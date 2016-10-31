#include "data_model.h"
#include <assert.h>
#include <stdlib.h>

// Initialize the common values used by both kinds of data models.
Data_Model::Data_Model(int alphabet_size, int _denominator_bits, int _order) {
    order = _order;
    denominator_bits = _denominator_bits;

    assert(alphabet_size > 0);
    assert(denominator_bits > 0);

    denominator = 1 << denominator_bits;
    num_symbols = alphabet_size + 1;    // Add a slot for the escape symbol.
    escape_symbol = alphabet_size;
}


Data_Model_1D::Data_Model_1D(int alphabet_size, int _denominator_bits, int order) 
: Data_Model(alphabet_size, _denominator_bits, 0) {
    symbols = new Symbol_Count[num_symbols];

    // Initialize all symbol counts to 0.
    int i;
    for (i = 0; i < num_symbols; i++) symbols[i].numerator = 0;
}

Data_Model_1D::~Data_Model_1D() {
    delete [] symbols;
}

void Data_Model_1D::process_training_symbol(int index) {
    assert(index >= 0);
    assert(index < num_symbols);

    // Increment the count for this symbol.  Right now 'numerator' is
    // just a count (you can think of the denominator as being 1).
    // Later we will rescale these numerators so that they are at
    // appropriate magnitudes for a denominator like 4096.
    symbols[index].numerator++;
}


// 
// finalize() does the rescaling of the symbol counts, and also
// computes the 'base_numerators' (used to look up the symbol at
// decode time).
//
void Data_Model_1D::finalize(float escape_probability_boost) {
    int sum = 0;

    int i;
    for (i = 0; i < num_symbols; i++) {
        sum += symbols[i].numerator;
    }

    assert(sum > 0);

    // Compute an escape probability based on the boost parameter
    // that was passed in.  (This can be 0!  If so, that's handled
    // later.)
    int escape_points = (int)(sum * escape_probability_boost);
    sum += escape_points;
    symbols[escape_symbol].numerator += escape_points;


    // Do the rescaling phase... We want to multiply each numerator
    // by (denominator / sum).

    int numerator_sum = 0;
    for (i = 0; i < num_symbols; i++) {
        unsigned long num = symbols[i].numerator;

        // If the number is low enough to not overflow when we multiply,
        // then we multiply first, then divide.  Otherwise we divide
        // first, then multiply.  On second thought this really isn't
        // necessary but hey, it's here.
        unsigned int limit = (1 << (32 - denominator_bits - 2)) - 1;

        if (num < limit) {
            num *= denominator;
            num /= sum;
        } else {
            num /= sum;
            num *= denominator;
        }

        symbols[i].numerator = num;
        numerator_sum += num;
    }


    // The round-off that happened during the rescaling phase
    // will have caused the sum of our fractions to be less than 1.
    // That is wasteful, we should allocate that extra space somewhere.
    // I just tack it onto the escape symbol,
    // because that's easy.  Though you could imagine ways to spread it 
    // around for an epsilon-more-efficient result.
    int leftovers = denominator - numerator_sum;
    numerator_sum += leftovers;

    // The escape symbol MUST have a count greater than 0, so that
    // it's possible to write it.  (We use the escape symbol to move
    // between contexts, when encoding a rare symbol that wasn't in the
    // training data.  We also use it to indicate end of file.)
    // We aren't guaranteed that this is nonzero yet, so we might as
    // well just add at least one to it, no matter what.  Adding
    // the leftovers from rounding is a good way to do this.  But
    // if we don't have any leftovers to give to the escape symbol,
    // we steal a point from someone arbitrary.
    if (leftovers == 0) {
        leech_from_some_symbol();
        leftovers = 1;
    }

    symbols[escape_symbol].numerator += leftovers;


    // Go through and initialize the 'base_numerator' values, now
    // that everything else is settled.

    sum = 0;
    for (i = 0; i < num_symbols; i++) {
        symbols[i].base_numerator = sum;
        sum += symbols[i].numerator;
    }
}

// Steal a point from some symbol, so we can allocate it elsewhere.
void Data_Model_1D::leech_from_some_symbol() {
    int i;
    for (i = 0; i < num_symbols; i++) {
        if (symbols[i].numerator == 0) continue;
        symbols[i].numerator--;

        assert(i < escape_symbol);
        return;
    }
}

// For the -1st context, we just want all probabilities to be equal.
void Data_Model_1D::set_all_probabilities_to_be_equal() {
    int i;
    for (i = 0; i < num_symbols; i++) {
        symbols[i].numerator = denominator / num_symbols;
    }
}



// Data_Model_2D is basically a holder for a bunch of 1D contexts.
Data_Model_2D::Data_Model_2D(int alphabet_size, int _denominator_bits)
: Data_Model(alphabet_size, _denominator_bits, 1) {
    last_symbol = 0;

    order_0_contexts = new Data_Model_1D *[num_symbols];

    int i;
    for (i = 0; i < num_symbols; i++) order_0_contexts[i] = NULL;
}

Data_Model_2D::~Data_Model_2D() {
    delete [] order_0_contexts;
}

void Data_Model_2D::process_training_symbol(int index) {
    assert(index >= 0);
    assert(index < num_symbols);

    // If we don't have a model for the current context, then create one.
    // (The current context is controlled by the last symbol we processed).

    if (!order_0_contexts[last_symbol]) {
        order_0_contexts[last_symbol] = new Data_Model_1D(num_symbols - 1, denominator_bits);
    }

    // Increment the count for this symbol in the current context.
    order_0_contexts[last_symbol]->symbols[index].numerator++;

    // Update the context.
    last_symbol = index;
}


void Data_Model_2D::finalize(float escape_probability_boost) {
    // Tell all the context models to finalize.

    int i;
    for (i = 0; i < num_symbols; i++) {
        // Really we want to compute the escape boost as a factor of what's
        // going on in this particular context, not as some arbitrary variable
        // passed in from above.  (The reason is, we want to optimize this value
        // based on how often we expect to escape from each particular context).
        if (order_0_contexts[i]) order_0_contexts[i]->finalize(escape_probability_boost);
    }

    reset_context();
}

void Data_Model_2D::reset_context() {
    // Reset 'last_symbol' so that when we actually encode/decode,
    // it starts from 0.
    last_symbol = 0;
}



// Set the appropriate boolean for this message_id, and increment
// the count of messages received.
void Data_Model_Set::mark_message_as_received(int message_id) {
    assert((message_id / MESSAGES_PER_BLOCK) == block_index);
    int index = message_id % MESSAGES_PER_BLOCK;
    if (got_message[index]) return;   // Already got this one -- it's a duplicate!

    got_message[index] = true;
    num_messages_received++;
}


// Have we gotten the entire block?
bool Data_Model_Set::got_entire_block() {
    if (num_messages_received == MESSAGES_PER_BLOCK) return true;
    return false;
}


Data_Model_Set::Data_Model_Set(int alphabet_size, int denominator_bits) {
    block_index = -1;
    order_0 = new Data_Model_1D(alphabet_size, denominator_bits);
    order_1 = new Data_Model_2D(alphabet_size, denominator_bits);

    int i;
    for (i = 0; i < MESSAGES_PER_BLOCK; i++) got_message[i] = 0;
    num_messages_received = 0;
}

Data_Model_Set::~Data_Model_Set() {
    delete order_0;
    delete order_1;
}


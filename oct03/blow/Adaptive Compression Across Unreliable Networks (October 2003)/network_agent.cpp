#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>  // For memcpy

#include "network_agent.h"
#include "arithmetic_coder.h"
#include "message_stats.h"

Network_Agent::Network_Agent() {
    stats = new Message_Stats();
    encoder = new Arithmetic_Encoder();
    decoder = new Arithmetic_Decoder();
    outgoing_message = NULL;
    incoming_message = NULL;
    static_set = NULL;

    next_outgoing_message_id = 0;

    build_static_models();

    int i;
    for (i = 0; i < MAX_DATA_MODELS; i++) consensual_models[i] = NULL;

    consensual_models[0] = static_set;
}

Network_Agent::~Network_Agent() {
    delete stats;
    delete encoder;
    delete decoder;

    delete static_set;
    delete order_negative_1;
    delete data_model_index_context;
}

void Network_Agent::build_static_models() {
    // Set up the table for order -1 (the context where all symbols have
    // the same probability... used as a fallback context when we encounter
    // situations that didn't happen in the training data.)
    order_negative_1 = new Data_Model_1D(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);
    order_negative_1->set_all_probabilities_to_be_equal();
    order_negative_1->finalize(0.0f);

    // Set up the table for order 0, by making a pass through the training
    // data and counting character frequencies.
    static_set = new Data_Model_Set(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);
    Data_Model_1D *order_0 = static_set->order_0;
    Data_Model_2D *order_1 = static_set->order_1;

    //
    // Load the training data...
    //
    char *training_filename = "training.txt";
    printf("Building static model from file '%s'\n", training_filename);
    FILE *f = fopen(training_filename, "rt");
    if (!f) {
        fprintf(stderr, "Unable to open training file '%s'!\n", training_filename);
        exit(1);
    }

    // Go through the training data one character at a time; use it to
    // train the order 0 and order 1 models.  The order 1 model keeps
    // 1 character of context locally, so we don't worry about that here.
    while (1) {
        int c = fgetc(f);
        if (c == EOF) break;
        if (c < 0) break;

        order_0->process_training_symbol(c);
        order_1->process_training_symbol(c);
    }

    fclose(f);

    //
    // Finalize the models, which just means: tell them that
    // no new training data will come, so they can do whatever
    // precomputation they need to do.
    //

    order_0->finalize(0.0f);
    order_1->finalize(0.0f);


    // Build contexts for the message types

    data_model_index_context = new Data_Model_1D(MAX_DATA_MODELS, DENOMINATOR_BITS);
    data_model_index_context->set_all_probabilities_to_be_equal();
    data_model_index_context->finalize(0.0f);
}

// Encode a symbol with a 0th order model (Data_Model_1D).
void Network_Agent::encode_single_symbol(Data_Model_1D *model, int symbol_index) {
    Symbol_Count *symbol = &model->symbols[symbol_index];
    if (symbol->numerator == 0) {
        // If the symbol has a 0 probability in this model, we need to
        // escape it and fall back to order -1 (where it is guaranteed
        // to have a nonzero probability).

        Symbol_Count *escape = &model->symbols[model->escape_symbol];
        encoder->pack(escape, model);

        encode_single_symbol(order_negative_1, symbol_index);

        return;
    }

    // The symbol had a nonzero probability, so we just pack it.
    encoder->pack(symbol, model);
}

// For order-1 modeling, find a context based on the last symbol.
Data_Model_1D *get_context(Data_Model_Set *model) {
    Data_Model_1D *context = model->order_1->order_0_contexts[model->order_1->last_symbol];

    // If the context is NULL (we never encountered it in training),
    // fall back to the 0th order model.
    if (!context) context = model->order_0;
    return context;
}

void Network_Agent::encode_single_symbol(Data_Model_Set *model, int symbol_index) {
    Data_Model_1D *context = get_context(model);
    encode_single_symbol(context, symbol_index);
    model->order_1->last_symbol = symbol_index;  // Update the context.
}

//
// To encode "done" (a.k.a. EOF) we escape all the way out of the bottom context.
//
void Network_Agent::encode_done_symbol(Data_Model_Set *model) {
    // Escape out of order 1 context
    Data_Model_1D *model_1 = get_context(model);
    encode_single_symbol(model_1, model->order_1->escape_symbol);

    // Escape out of current order 0 model
    encode_single_symbol(model->order_0, model->order_0->escape_symbol);

    // Escape out of order -1
    encode_single_symbol(order_negative_1, order_negative_1->escape_symbol);
}

// Decode a single symbol, when we haven't specified what order the model
// is.  (Again, this is just so we can use one test loop to test multiple
// models).
int Network_Agent::decode_single_symbol(Data_Model_1D *model, int *decoded_escape_symbol_return) {
    Data_Model_1D *context = model;
    if (decoded_escape_symbol_return) *decoded_escape_symbol_return = context->escape_symbol;

    // Unpack a symbol; if it's not the escape symbol, return it.
    int symbol = decoder->unpack((Data_Model_1D *)context);
    if (symbol != model->escape_symbol) return symbol;

    // Otherwise, we escaped to order -1...
    symbol = decoder->unpack(order_negative_1);
    if (decoded_escape_symbol_return) *decoded_escape_symbol_return = order_negative_1->escape_symbol;

    return symbol;
}
    
int Network_Agent::decode_single_symbol(Data_Model_Set *model, int *decoded_escape_symbol_return) {
    Data_Model_1D *context = get_context(model);
    int symbol = decode_single_symbol(context, decoded_escape_symbol_return);
    model->order_1->last_symbol = symbol;  // Update the context.
    return symbol;
}


// Transmit a packet.  Since this is a mock-up of a networked
// system, we don't actually send anything over a network.
// Instead we read the results out of the arithmetic encoder,
// put them into a network message data structure, and put
// that message in the 'outgoing_message' variable.  The
// main loop will pick it up from there and give it to the
// recipient.
void Network_Agent::transmit() {
    int len;
    unsigned char *data;

    encoder->get_result(&data, &len);

    assert(outgoing_message == NULL);
    Network_Message *message = new Network_Message;
    memcpy(message->data, data, len);
    message->length = len;

    outgoing_message = message;
}

// Which logical block number does this message belong to?
int Network_Agent::block_number_of_message(int message_id) {
    int block_number = message_id / MESSAGES_PER_BLOCK;
    return block_number;
}

// Get the appropriate accumulated data model for this block number.
Data_Model_Set *Network_Agent::get_message_stats(int message_id) {
    int block_number = block_number_of_message(message_id);
    Data_Model_Set *model = stats->get_stats_for_block(block_number);
    return model;
}

// Given a model index (of a complete data model), give us back the data model.
Data_Model_Set *Network_Agent::get_model_by_index(int index) {
    Data_Model_Set *model = consensual_models[index];
    assert(model != NULL);
    return model;
}


// Combine two data models into a result model.
// The stats of the input models are multiplied by WEIGHT_FOR_OLD_MODEL
// and WEIGHT_FOR_NEW_MODEL so that you can control the rate of
// convergence of the adaptation.
void combine(Data_Model_1D **result_ptr, Data_Model_1D *old, Data_Model_1D *recent) {
    if (!old && !recent) return;

    if (!*result_ptr) *result_ptr = new Data_Model_1D(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);

    Data_Model_1D *result = *result_ptr;

    int i;
    for (i = 0; i < result->num_symbols; i++) {
        int old_value = 0;
        int recent_value = 0;

        if (old) old_value = old->symbols[i].numerator;
        if (recent) recent_value = recent->symbols[i].numerator;
        result->symbols[i].numerator = old_value * WEIGHT_FOR_OLD_MODEL
                                     + recent_value * WEIGHT_FOR_NEW_MODEL;
    }
}

//
// Given an old model index, and a model describing the stats for
// some recent block of data, create a new consensual data model
// and put it in slot 'new_index' (destroying any data model that
// used to occupy that slot).
//
bool Network_Agent::synthesize_new_model(int old_index, Data_Model_Set *recent_model, int new_index) {
    Data_Model_Set *old_model = get_model_by_index(old_index);

    if (old_model == NULL) {
        // Something wacky happened... we ignore the client's request
        // for a new model.

        return false;
    }

    Data_Model_Set *new_model = new Data_Model_Set(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);
    recent_model->order_0->finalize(0.0f);
    recent_model->order_1->finalize(0.0f);

    combine(&new_model->order_0, old_model->order_0, recent_model->order_0);
    int i;
    for (i = 0; i < old_model->order_1->num_symbols; i++) {
        combine(&new_model->order_1->order_0_contexts[i],
                old_model->order_1->order_0_contexts[i],
                recent_model->order_1->order_0_contexts[i]);
    }

    new_model->order_0->finalize(0.0f);
    new_model->order_1->finalize(0.0f);

    if (consensual_models[new_index]) delete consensual_models[new_index];
    consensual_models[new_index] = new_model;

    return true;
}

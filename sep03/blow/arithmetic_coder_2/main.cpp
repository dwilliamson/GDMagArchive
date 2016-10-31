#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "arithmetic_coder.h"

const int NUM_ALPHABET_VALUES = 256;  // We are encoding bytes!

// DENOMINATOR_BITS = 12 means that probabilities are fractions
// between 1/4096 and 4096/4096.  There's a bit of a balance struck
// here; if the number of bits is too small, our probabilities
// become inaccurate, and we get poor compression.  If the number
// of bits is too big, the math in arithmetic_coder.cpp will cause
// a big loss of precision, resulting in poor compression.
const int DENOMINATOR_BITS = 12; 

// These are the data models we use for compressing the data.
Data_Model_2D *order_1;
Data_Model_1D *order_0;
Data_Model_1D *order_negative_1;

// For order-1 modeling, find a context based on the last symbol.
Data_Model_1D *get_context(Data_Model_2D *model) {
    Data_Model_1D *context = model->order_0_contexts[model->last_symbol];

    // If the context is NULL (we never encountered it in training),
    // fall back to the 0th order model.
    if (!context) context = order_0;
    return context;
}

// Encode a symbol with a 0th order model (Data_Model_1D).
void encode_single_symbol(Data_Model_1D *model, Arithmetic_Encoder *encoder, int symbol_index) {
    Symbol_Count *symbol = &model->symbols[symbol_index];
    if (symbol->numerator == 0) {
        // If the symbol has a 0 probability in this model, we need to
        // escape it and fall back to order -1 (where it is guaranteed
        // to have a nonzero probability).

        Symbol_Count *escape = &model->symbols[model->escape_symbol];
        encoder->pack(escape, model);

        encode_single_symbol(order_negative_1, encoder, symbol_index);

        return;
    }

    // The symbol had a nonzero probability, so we just pack it.
    encoder->pack(symbol, model);
}


// Encode a symbol with a 1st order model (Data_Model_2D).
// To do this we just get the appropriate 0th order context, and
// use that to pack.
void encode_single_symbol(Data_Model_2D *model, Arithmetic_Encoder *encoder, int symbol_index) {
    Data_Model_1D *context = get_context(model);
    encode_single_symbol(context, encoder, symbol_index);
    model->last_symbol = symbol_index;   // Update the context.
}


// Encode a symbol, where we haven't specified which order our model
// is.  (This is just so we can use the same basic loop to test the
// order 0 and order 1 models).  This function just checks the order
// of the model then casts up to the appropriate encoding function,
// above.
void encode_single_symbol(Data_Model *model, Arithmetic_Encoder *encoder, int symbol) {
    if (model->order == 1) {
        encode_single_symbol((Data_Model_2D *)model, encoder, symbol);
    } else {
        assert(model->order < 1);
        encode_single_symbol((Data_Model_1D *)model, encoder, symbol);
    }
}

//
// To encode "done" (a.k.a. EOF) we escape all the way out of the bottom context.
//
void encode_done_symbol(Data_Model *model, Arithmetic_Encoder *encoder) {
    assert(model->order <= 1);

    Data_Model *context = model;
    if (model->order == 1) {
        // Escape out of order 1
        encode_single_symbol(model, encoder, model->escape_symbol);
        context = get_context((Data_Model_2D *)model);
    }

    // Escape out of order 0
    encode_single_symbol(context, encoder, context->escape_symbol);

    // Escape out of order -1
    encode_single_symbol(order_negative_1, encoder, order_negative_1->escape_symbol);
}


// Decode a single symbol, when we haven't specified what order the model
// is.  (Again, this is just so we can use one test loop to test multiple
// models).
int decode_single_symbol(Data_Model *model, Arithmetic_Decoder *decoder) {
    Data_Model *context = model;
    if (model->order == 1) {
        // If 1st order, find the appropriate context.
        context = get_context((Data_Model_2D *)model);
    }

    assert(context->order < 1);

    // Unpack a symbol; if it's not the escape symbol, return it.
    int symbol = decoder->unpack((Data_Model_1D *)context);
    if (symbol != model->escape_symbol) return symbol;

    // Otherwise, we escaped to order -1...
    symbol = decoder->unpack(order_negative_1);

    return symbol;
}
    
void do_test(Data_Model *model, char *input_file_name, char *test_name) {
    printf("------------------------\n");
    printf("--- Beginning test '%s'\n", test_name);
    printf("------------------------\n");


    //
    // Open the input file and start packing it!
    //
    FILE *f = fopen(input_file_name, "rt");
    if (!f) {
        fprintf(stderr, "Unable to open file '%s' for input.\n", input_file_name);
        exit(1);
    }

    printf("\nCompressing file '%s'.\n", input_file_name);

    Arithmetic_Encoder *encoder = new Arithmetic_Encoder;
    int uncompressed_length = 0;
    // Pack one character at a time, and count them so we have some stats.
    while (1) {
        int c = fgetc(f);
        if (c == EOF) break;
        if (c < 0) break;

        uncompressed_length++;
        encode_single_symbol(model, encoder, c);
    }

    fclose(f);

    // Pack an EOF into the arithmetic coder...
    encode_done_symbol(model, encoder);


    printf("Original length: %d bytes.\n", uncompressed_length);

    unsigned char *data;
    int len;
    encoder->get_result(&data, &len);

    int compressed_length = len;

    printf("Compressed length: %d bytes.\n", compressed_length);

    printf("\nUncompressing...\n");
    printf("------------------\n");


    //
    // Now we ensure that we can actually decode the compressed data!
    //
    
    Arithmetic_Decoder *decoder = new Arithmetic_Decoder();
    decoder->set_data(data, len);

    // All the escape symbols should be the same... this is necessary
    // for the code below to work... we check 'symbol' against the
    // toplevel model's escape symbol in order to decide whether to
    // terminate, but it might actually be a lower level model's escape
    // symbol.  If we ever change the code so that escape symbols aren't
    // all the same, we can fix the loop by returning the context as well
    // as the symbol, and just asking that context for its escape
    // symbol, to compare with.
    assert(order_1->escape_symbol == order_0->escape_symbol);
    assert(order_0->escape_symbol == order_negative_1->escape_symbol);


    // If we're doing the 1st-order model, we need to tell it to 
    // forget any context it moved into while we were encoding.
    if (model->order == 1) ((Data_Model_2D *)model)->reset_context();


    // Decode the compressed data, counting each symbol so we have some stats.
    int decode_count = 0;
    while (1) {
        int symbol = decode_single_symbol(model, decoder);
        if (model->order == 1) ((Data_Model_2D *)model)->last_symbol = symbol;
        if (symbol == model->escape_symbol) break;
        printf("%c", symbol);
        decode_count++;
    }

    printf("------------------\n");
    printf("%d characters decoded.\n", decode_count);

    printf("Compressed length %d bytes; uncompressed length %d bytes\n",
           compressed_length, uncompressed_length);

    if (uncompressed_length > 0) {
        float compression_percent = ((uncompressed_length - compressed_length) / (float)uncompressed_length) * 100.0f;
        printf("Compression achieved: %2.1f%%\n", compression_percent + 0.05f);  // The 0.05f is to round to the nearest tenth of a percentage point (%2.1f won't round for us, it seems)
    }

    delete encoder;
    delete decoder;
}


void main(void) {
    int i;

    // Set up the table for order -1 (the context where all symbols have
    // the same probability... used as a fallback context when we encounter
    // situations that didn't happen in the training data.)
    order_negative_1 = new Data_Model_1D(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);
    order_negative_1->set_all_probabilities_to_be_equal();
    order_negative_1->finalize(0.0f);

    // Set up the table for order 0, by making a pass through the training
    // data and counting character frequencies.
    order_0 = new Data_Model_1D(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);
    order_1 = new Data_Model_2D(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);

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

    // Go through the training data one character at a time, and
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

    for (i = 0; i < order_0->num_symbols; i++) {
        int count = order_0->symbols[i].numerator;
        if (count) printf("Symbol %d count: %d\n", i, count);
    }

    //
    // Finalize the models, which just means: tell them that
    // no new training data will come, so they can do whatever
    // precomputation they need to do.
    //

    order_0->finalize(0.0f);

    // We pick a value of 1/20 for the escape probability here;
    // (the escape probability is the parameter to 'finalize'.
    // That is an arbitrary choice, and alternative values might
    // yield better compression.  It just all depends on how often
    // we need to escape from this context.  See the references for
    // some slightly more involved, but better, ways of choosing
    // the escape probability.
    order_1->finalize(0.05f);


    char *input_file_name = "input.txt";

    // Run the order 0 test.
    do_test(order_0, input_file_name, "Order 0");

    // Run the order 1 test.
    do_test(order_1, input_file_name, "Order 1");

    printf("Done!\n");

    delete order_negative_1;
    delete order_0;
    delete order_1;
}


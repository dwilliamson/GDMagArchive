#include <stdlib.h>
#include <stdio.h>
#include "client.h"
#include "message_stats.h"
#include "arithmetic_coder.h"

Client::Client() {
    last_requested_model_index = 0;
    highest_message_id_received = -1;
    num_messages_received = 0;
}

Client::~Client() {
}

//
// Create a new packet that tells the server to build a new data model,
// and transmit that packet.
//
void Client::send_new_model_request(int old_index, int new_index, int block_index) {
    encoder->reset();
    encode_single_symbol(data_model_index_context, old_index);
    encode_single_symbol(data_model_index_context, new_index);
    encoder->pack(block_index, BLOCK_INDEX_LIMIT);
    transmit();
}

// Find a model index that we can use for building a new model.
int Client::get_unused_model_index() {
    int index = last_requested_model_index + 1;
    if (index >= MAX_DATA_MODELS) {
        // We set index to 1 because I don't ever want to overwrite
        // the static data model, which is in slot 0.  Now, as this
        // demo stands, it never goes back and uses that model, so
        // we could just get rid of it.  But it doesn't seem unreasonable
        // to keep it around as a sort of fallback model, so that if
        // compression somehow becomes very poor, we reset to it.
        index = 1;
    }

    return index;
}

//
// update() gets called once per frame from the main loop.  We check
// to see if any packets have come in from the network; if they have,
// we process them.
//
void Client::update() {
    if (!incoming_message) return;

    // Feed the message data to the Arithmetic_Decoder.
    decoder->reset();
    decoder->set_data((unsigned char *)incoming_message->data, incoming_message->length);

    // Read the model index from the packet.
    int data_model_index = decode_single_symbol(data_model_index_context);

    // Read the message ID from the packet.
    int message_id = decoder->unpack(MESSAGE_ID_LIMIT);


    // Get the accumulator for the appropriate message block.
    // As we receive messages in this block, we add their stats
    // into the accumulator.  When we receive all messages for
    // a given block, we build a new data model, and tell the server
    // to build that model as well.
    Data_Model_Set *accumulator = get_message_stats(message_id);

    bool message_seems_valid = false;

    // Find the context we will use to decode the rest of the message.
    Data_Model_Set *context = consensual_models[data_model_index];
    if (context) context->order_1->reset_context();

    if (context) while (1) {
        // If we somehow got a corrupted packet, we want to make sure
        // we don't infinite loop... so we break here, if that happens.
        if (!decoder->might_there_be_any_data_left()) break;

        int escape_symbol;
        int symbol = decode_single_symbol(context, &escape_symbol);
        if (symbol == escape_symbol) {
            // If we get an escape symbol, that signifies the end of
            // the message.
            message_seems_valid = true;
            break;
        }


        // To add the symbol into the accumulator, we just tell
        // both its data models to train on that symbol.
        accumulator->order_0->process_training_symbol(symbol);
        accumulator->order_1->process_training_symbol(symbol);

        // Print the symbol to stdout, so we have something to look at.
        printf("%c", symbol);
    }

    if (message_seems_valid) {
        accumulator->mark_message_as_received(message_id);
        if (message_id > highest_message_id_received) highest_message_id_received = message_id;
        num_messages_received++;

        if (accumulator->got_entire_block()) {
            // It's time to build a new model!

            int old_index = last_requested_model_index;
            int new_index = get_unused_model_index();

            // Build the model on our end, so that when the server
            // starts using it, we can correctly decode the packets.
            synthesize_new_model(old_index, accumulator, new_index);

            // Tell the server to build this model for itself, and
            // to henceforth use it.
            send_new_model_request(old_index, new_index, accumulator->block_index);

            // We don't need this accumulator any more, so we discard it.
            stats->remove_model_fragment(accumulator);
            delete accumulator;

            // Record the model index we just requested; this helps us
            // pick the next one.
            last_requested_model_index = new_index;
        }
    } else {
        // Umm, throw away the data model since we corrupted it reading this message.
        stats->remove_model_fragment(accumulator);
        delete accumulator;
    }

    // Discard the message.
    delete incoming_message;
    incoming_message = NULL;
}

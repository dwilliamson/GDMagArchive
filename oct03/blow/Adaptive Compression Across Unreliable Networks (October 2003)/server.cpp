#include <stdlib.h>
#include <assert.h>
#include "server.h"
#include "message_stats.h"
#include "arithmetic_coder.h"

Server::Server() : Network_Agent() {
    char *filename = "to_transmit.txt";
    file_to_transmit = fopen(filename, "rt");
    done = false;

    num_bytes_transmitted = 0;
    num_data_bytes_precompressed = 0;
    current_model_index = 0;

    if (file_to_transmit == NULL) {
        fprintf(stderr, "Error: Unable to open file '%s' for reading.\n", filename);
        exit(1);
    }
}

Server::~Server() {
    if (file_to_transmit) fclose(file_to_transmit);
}


//
// In this demo, the client can only send one kind of packet to the
// server, which is a request for a new data model.  When we get one,
// we just need to extract out the items that tell us how to build
// the model.  Then we actually go build it, and make the model
// we use to transmit all new messages from now on (until we get
// a request for another new model!)
//
void Server::handle_new_model_request(Network_Message *message) {
    decoder->reset();
    decoder->set_data((unsigned char *)message->data, message->length);

    int old_index = decode_single_symbol(data_model_index_context);
    int new_index = decode_single_symbol(data_model_index_context);
    int block_index = decoder->unpack(BLOCK_INDEX_LIMIT);

    Data_Model_Set *recent_model = stats->get_stats_for_block(block_index);
    if (!recent_model) return;
    if (recent_model->num_messages_received != MESSAGES_PER_BLOCK) return;  // In case the client spoofed a synthesize message for data we don't have.

    bool success = synthesize_new_model(old_index, recent_model, new_index);
    if (success && !DISABLE_ADAPTATION) current_model_index = new_index;

    stats->remove_model_fragment(recent_model);
    delete recent_model;
}


//
// update() gets called once per cycle from the main loop.
//
void Server::update() {
    if (file_to_transmit == NULL) return;  // We're done transmitting!

    if (incoming_message) {
        // The only type of incoming message right now is a data model request.
        // So we handle it, then dispose of the message.
        handle_new_model_request(incoming_message);
        delete incoming_message;
        incoming_message = NULL;
    }

    //
    // Once per frame, we are going to transmit a packet with
    // more data in it from the 'file_to_transmit'.
    //

    // 'accumulator' holds statistics for all messages within a block.
    // Each time we send a new message that's a member of this block,
    // we add its statistics into 'accumulator'.  When the client gets
    // the whole block, it will add 'accumulator' to the current data 
    // model we're using for transmission, to build a new model; it will
    // tell us to do the same.  So, we need to track the accumulator here,
    // so we can accomodate the client's request when the time comes.
    Data_Model_Set *accumulator = get_message_stats(next_outgoing_message_id);
    accumulator->mark_message_as_received(next_outgoing_message_id);

    encoder->reset();

    // Tell the client which data model we're using.
    encode_single_symbol(data_model_index_context, current_model_index);

    // Tell the client the ID of this message (required so the client knows
    // which block it belongs to).
    encoder->pack(next_outgoing_message_id++, MESSAGE_ID_LIMIT);


    // Get the context we will use to encode the rest of this message.
    Data_Model_Set *context = consensual_models[current_model_index];
    context->order_1->reset_context();

    // The MESSAGE_LENGTH_MAX - 4 is because any single symbol that we 
    // pack using this arithmetic coder can't be longer than 4 bytes
    // (due to the implementation).  To signal end of packet, we need
    // to pack two escape symbols, so that means we can only proceed
    // if we have 12 bytes free in the packet.
    while (encoder->get_maximum_data_length() <= MESSAGE_LENGTH_MAX - 12) {
        int c = fgetc(file_to_transmit);
        num_data_bytes_precompressed++;

        if (c == EOF) {
            fclose(file_to_transmit);
            file_to_transmit = NULL;
            done = true;
            break;
        } else {
            assert(c >= 0);
            assert(c < 256);

            // To add the symbol into the accumulator, we just tell
            // both its data models to train on that symbol.
            accumulator->order_0->process_training_symbol(c);
            accumulator->order_1->process_training_symbol(c);

            // Actually put the symbol into the message.
            encode_single_symbol(context, c);
        }
    }

    // We need to encode a 'done' symbol so that the client knows
    // when to stop reading.
    assert(encoder->get_maximum_data_length() <= MESSAGE_LENGTH_MAX - 8);
    encode_done_symbol(context);

    // Transmit the message.
    transmit();

    if (outgoing_message) {
        num_bytes_transmitted += outgoing_message->length;
    }
} 


#pragma once
#ifndef __NETWORK_AGENT_H
#define __NETWORK_AGENT_H

struct Message_Stats;
struct Arithmetic_Encoder;
struct Arithmetic_Decoder;
struct Data_Model;
struct Data_Model_1D;
struct Data_Model_Set;

#include "config.h"

/*
  Network_Agent is the base class for Client and Server.  It handles
  particulars of encoding and decoding, and of managing probability tables.
*/

struct Network_Message {
    char data[MESSAGE_LENGTH_MAX];
    int length;
};

struct Network_Agent {
    Network_Agent();
    ~Network_Agent();

    Message_Stats *stats;   // Keeps data models for previously-encountered messages.

    Arithmetic_Encoder *encoder;
    Arithmetic_Decoder *decoder;

    Network_Message *incoming_message;
    Network_Message *outgoing_message;

    // These are the static models we start with for compressing the data.
    // 'static_set' will be computed from a training file, and it will
    // be the model that the adaptive scheme starts with.  'order_negative_1'
    // is the context we escape to whenever we're in an order-0 context
    // and we encounter a symbol with 0 probability.
    Data_Model_Set *static_set;
    Data_Model_1D *order_negative_1;

    // This is a special context made for the index of the data model.
    // Right now it treats all models as equal probability.  Probably
    // this wouldn't change in a real app, as you need to be able
    // to unambiguously decode this value, even if packets come in
    // out-of-order, etc.
    Data_Model_1D *data_model_index_context;

  protected:
    void build_static_models();
    void record_message_stats(Network_Message *);

    void encode_done_symbol(Data_Model_Set *model);
    void encode_single_symbol(Data_Model_1D *model, int symbol_index);
    void encode_single_symbol(Data_Model_Set *model, int symbol_index);

    int decode_single_symbol(Data_Model_1D *model, int *decoded_context_escape_symbol = NULL);
    int decode_single_symbol(Data_Model_Set *model, int *decoded_context_escape_symbol = NULL);
    void transmit();

    bool synthesize_new_model(int old_model_index, Data_Model_Set *recent_model, int new_model_index);

    int block_number_of_message(int message_id);
    Data_Model_Set *get_message_stats(int message_id);

    Data_Model_Set *get_model_by_index(int index);

    int next_outgoing_message_id;

    // 'consensual_models' is the set of all data models that can
    // be used to decode packets.  If an item is instantiated in this
    // array on the server, that means the client requested it;
    // since both sides used the same starting model, and the same
    // added ingredients, to build the new model, the new model
    // will be the same on the client and server.
    Data_Model_Set *consensual_models[MAX_DATA_MODELS];
};

#endif

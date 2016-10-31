#include "message_stats.h"
#include "data_model.h"

#include <stdlib.h>
#include <assert.h>

Message_Stats::Message_Stats() {
    highest_index_received = -1;
}

Message_Stats::~Message_Stats() {
    Data_Model_Set *fragment;
    Foreach(&data_model_fragments, fragment) {
        delete fragment;
    } Endeach;

    data_model_fragments.clean();
}

Data_Model_Set *Message_Stats::get_stats_for_block(int block_index) {
    // Look in our list and find an accumulator that matches this block index.
    Data_Model_Set *fragment;
    Foreach(&data_model_fragments, fragment) {
        if (fragment->block_index == block_index) return fragment;
    } Endeach;

    // We didn't find one, so create a new one, and add it to the list.
    Data_Model_Set *model = new Data_Model_Set(NUM_ALPHABET_VALUES, DENOMINATOR_BITS);
    model->block_index = block_index;
    data_model_fragments.add(model);
    return model;
}

void Message_Stats::remove_model_fragment(Data_Model_Set *to_remove) {
    // Search through the list, find the matching accumulator, and remove it.
    Data_Model_Set *fragment;
    Foreach(&data_model_fragments, fragment) {
        if (fragment == to_remove) {
            Foreach_Remove_Current_Element(&data_model_fragments);
            return;
        }
    } Endeach;

    // Uhh we couldn't find it... that is an error!
    assert(0);
}

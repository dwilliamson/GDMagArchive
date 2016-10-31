struct Data_Model_Set;

#include "list.h"

const int NUM_ALPHABET_VALUES = 256;  // We are encoding bytes!

// DENOMINATOR_BITS = 12 means that probabilities are fractions
// between 1/4096 and 4096/4096.  There's a bit of a balance struck
// here; if the number of bits is too small, our probabilities
// become inaccurate, and we get poor compression.  If the number
// of bits is too big, the math in arithmetic_coder.cpp will cause
// a big loss of precision, resulting in poor compression.
const int DENOMINATOR_BITS = 12; 

// Message_Stats is just a simple class that keeps a list of the
// active data model accumulators for each block.  It almost doesn't
// warrant being a separate class, in a demo this small.
struct Message_Stats {
    Message_Stats();
    ~Message_Stats();

    Data_Model_Set *get_stats_for_block(int block_index);
    void remove_model_fragment(Data_Model_Set *to_remove);

    int highest_index_received;
    List data_model_fragments;
};


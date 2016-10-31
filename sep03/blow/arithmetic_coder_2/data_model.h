// Symbol_Count is just our basic keeper of statistics on
// how many times we have seen a certain symbol in a file.
struct Symbol_Count {
    int numerator;      // The numerator of the probability fraction
    int base_numerator; // The sum of numerators of all probability fractions
                        // earlier than this one in the array.
};


//
// The base class for both kinds of data models that we currently support.
// It just contains some basic information.
//
struct Data_Model {
    Data_Model(int alphabet_size, int denominator_bits, int order);

    int order;      // In this demo, order will be -1, 0, or 1.

    int num_symbols;
    int escape_symbol;
    int denominator_bits;
    int denominator;
};


//
// Data_Model_1D is what we use for 0th order (context-free) modeling.
// It's called '1D' because it stores statistics as a 1-dimensional
// array, 1 entry per symbol.
//
// We also use this for the context of "order -1", which is just an
// order 0 context where each symbol has equal probability.  This is
// used to encode symbols that we didn't see in the training data.
//
struct Data_Model_1D : public Data_Model {
    Data_Model_1D(int alphabet_size, int denominator_bits, int order = 0);
    ~Data_Model_1D();

    Symbol_Count *symbols;

    void process_training_symbol(int symbol);      // Used when building stats.
    void finalize(float escape_probability_boost); // This finishes computing stats.

    void set_all_probabilities_to_be_equal();  // Used only to build the order -1 context.

  protected:
    void leech_from_some_symbol();
};


//
// Data_Model_2D is what we use for 1st order (1 symbol of context) modeling.
// It stores statistics as a 2-dimensional array, hence the '2D'.  The top-level
// array represents the current context (the last symbol that we saw); there
// is a Data_Model_1D for each possible context.
//
struct Data_Model_2D : public Data_Model {
    Data_Model_2D(int alphabet_size, int denominator_bits);
    ~Data_Model_2D();

    Data_Model_1D **order_0_contexts;
    int last_symbol;

    void reset_context();
    void process_training_symbol(int symbol);
    void finalize(float escape_probability_boost);
};


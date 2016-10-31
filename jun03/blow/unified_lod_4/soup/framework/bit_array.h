struct Bit_Array {
    Bit_Array(int num_bits);
    ~Bit_Array();

    void clear_all();
    int is_set(int index);
    void set(int index);
    void clear(int index);

    void copy_from(Bit_Array *other);

    int num_longs;
    unsigned long *data;


  protected:
    void get_slot(int index, int *bit_index_return, int *word_index_return);
};

inline void Bit_Array::get_slot(int bit_index, int *bit_mask_return, int *word_index_return) {
    assert(sizeof(unsigned long) == 4);

    int word_index = bit_index >> 5;   // 2**5 == 32
    int bit_index_within_word = bit_index & 31;
    int bit_mask = 1 << bit_index_within_word;

    assert(word_index >= 0);
    assert(word_index < num_longs);

    *bit_mask_return = bit_mask;
    *word_index_return = word_index;
}

inline int Bit_Array::is_set(int bit_index) {
    int bit_mask, word_index;
    get_slot(bit_index, &bit_mask, &word_index);
    return (data[word_index] & bit_mask);
}

inline void Bit_Array::set(int bit_index) {
    int bit_mask, word_index;
    get_slot(bit_index, &bit_mask, &word_index);
    data[word_index] |= bit_mask;
}

inline void Bit_Array::clear(int bit_index) {
    int bit_mask, word_index;
    get_slot(bit_index, &bit_mask, &word_index);
    data[word_index] &= ~bit_mask;
}


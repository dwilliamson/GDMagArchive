struct Memory_Arena {
    Memory_Arena(int size);
    ~Memory_Arena();

    void reset();

    int size;
    int size_remaining;
    char *data;
    char *cursor;
};

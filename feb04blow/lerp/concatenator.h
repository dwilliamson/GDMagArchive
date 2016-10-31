struct String_Bucket;

// The Concatenator allocates memory in chunks that are
// DEFAULT_CONCAT_BLOCK_SIZE bytes large.
const int DEFAULT_CONCAT_BLOCK_SIZE = 512;

struct Concatenator {
  public:
    // 'add' translates the argument into an ASCII string and
    // appends it to the current result.
    LEXPORT void add(char);  // Needed for packing binary code stuff!
    LEXPORT void add(char *);
    LEXPORT void add(int);
    LEXPORT void add(unsigned int);
    LEXPORT void add(double);
    LEXPORT void add(short);
    LEXPORT void add(long);

    // This 'printf' performs a callout to the mprintf() routine and then
    // appends the result; therefore you don't need to worry about the
    // array size (though mismatching the types of the printf format
    // arguments does have the potential for crashing the program, as
    // always with printf).
	LEXPORT void printf(char *fmt, ...);

    // 'add_u4b', 'add_u2b', and 'add_u1b' treat the argument as an integer
    // that you want to pack in binary form, as being 4 bytes large,
    // 2 bytes large, or 1 byte large.
    void add_u4b(int);
    void add_u2b(int);
    void add_u1b(int);

    void modify_2b(int location, int value);
    void modify_1b(int location, int value);

    // 'add_nozeroterm' adds raw binary data that is 'len' bytes long.
    void add_nozeroterm(char *s, unsigned int len);

    // 'length' tells you how many bytes long the current result is.
    unsigned int length();

    // 'get_result' allocates space for the current result, using new char[]
    // (it is your responsibility to free this memory).  It assumes the
    // result is an ASCII string, so it appends a 0 at the end to terminate
    // it.
    LEXPORT char *get_result();

    // 'get_nozeroterm_result' acts as 'get_result' but does not append
    // a 0 to the data.
    LEXPORT char *get_nozeroterm_result(unsigned int *len_return);

    // 'reset' erases the current result; calling this->length() will
    // then return 0.
    LEXPORT void reset();

    LEXPORT Concatenator(int block_size = DEFAULT_CONCAT_BLOCK_SIZE);
    LEXPORT ~Concatenator();

    int block_size;

  private:
    void expand(void);
    String_Bucket *seek(int location, int *base_return);

    String_Bucket *first;
    String_Bucket *last;
    unsigned int nbuckets;
};


typedef unsigned long u32;

struct Multiplication_Packer {
public:
    Multiplication_Packer();

    u32 accumulator;        // initialized to 0
    u32 composite_limit;    // initialized to 1

    void pack(u32 limit, u32 value);
    u32 get_result(u32 *limit_return = NULL);

    void set_accumulator(u32 accumulator);
    u32 unpack(u32 limit);
};


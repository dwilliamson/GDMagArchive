typedef unsigned char u8;
typedef unsigned long u32;

const int PACKER_BUFFER_SIZE = 1024;
struct Bit_Packer {
    Bit_Packer();

    void pack(u32 value, u32 num_bits_to_pack);
    void get_result(char **data_return, int *len_return);

    int next_bit_to_write;     // Initialized to 0
    u8 buffer[PACKER_BUFFER_SIZE];
};

struct Bit_Unpacker {
    Bit_Unpacker(char *data, int len);

    void set_input(char *data, int len);
    u32 unpack(u32 num_bits);


    int num_bits_remaining;
    int next_bit_to_read;

    u8 buffer[PACKER_BUFFER_SIZE];
};


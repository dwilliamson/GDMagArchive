#include "data_model.h"

// In this sample we use static buffers of some
// maximum size, in order to keep the code simple.
// For a real application, you probably want to use 
// auto-growing buffers or something.
const int AC_MAX_BUFFER_LEN = 16384;


// Arithmetic_Encoder packs a message together for transmission.

struct Arithmetic_Encoder {
    Arithmetic_Encoder();

    // Clear all data, start with a blank slate
    void reset();   

    // Get an array consisting of all the data we have packed.  The length
    // return value is measured in bytes.
    void get_result(unsigned char **data_return, int *length_return);

    void pack(Symbol_Count *symbol, Data_Model_1D *model);

    // Tells us whether the encoder is empty or not.
    bool has_anything_been_packed();

  protected:
    // See the .cpp file for an explanation of the things below.
    unsigned long low;
    unsigned long range;

    int num_bytes_output;

    void output_byte(unsigned int value);
    void output_bit(int value);

    void flush();
    void renormalize();
    void carry();

    unsigned char output_bytes[AC_MAX_BUFFER_LEN];
};


// Arithmetic_Decoder unpacks a message we've received.

struct Arithmetic_Decoder {
    Arithmetic_Decoder();
    ~Arithmetic_Decoder();

    // Start over with an empty slate.
    void reset();

    // Set the data to decode (e.g. the parameters come from a network message we received).
    void set_data(unsigned char *data, int length);

    unsigned long unpack(Data_Model_1D *model);

    // The following function is for sanity-checking; we can prevent
    // ourselves from infinite-looping in a case where there is no
    // data left but the program doesn't understand it's done.  (This
    // can happen if you receive garbage network messages that don't
    // conform to your protocol).
    bool might_there_be_any_data_left();

  protected:
    // See arithmetic_coder.cpp for explanations of the variables below!
    int num_input_bytes;
    int input_byte_cursor;

    unsigned long low;
    unsigned long range;
    unsigned long code;

    int input_next_byte();
    void flush();
    void renormalize();

    unsigned char input_bytes[AC_MAX_BUFFER_LEN];
};



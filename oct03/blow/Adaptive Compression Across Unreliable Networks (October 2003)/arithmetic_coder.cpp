#include "arithmetic_coder.h"

#include <assert.h>
#include <string.h>

/*
  This is an implementation of a "range coder".  Whereas the usual
  implementation of an arithmetic coder outputs 1 bit at a time, the
  range coder presented here outputs a byte at a time.  This translates
  into some potential CPU time savings, which can be important in a 
  server environment where you're encoding messages for many clients.
  Also, it makes for slightly simpler code, which I like.
*/


// NUM_HIGH_BITS is how many bits we shift out at a time (1 byte).
const int NUM_HIGH_BITS = 8;

// NUM_LOW_BITS is the machine word size, minus 1, minus the number of
// high bits.  The -1 happens because we need to represent numbers from
// 0 to 2**n, not just 0 to 2**n-1.  So the high bit is almost entirely
// unused, but not quite.  We could actually work around this, by making
// the code more complicated; in return we'd get 1 extra bit of precision
// in the math.  But since this is sample code, I have elected not to
// do that.
const int NUM_LOW_BITS = 31 - NUM_HIGH_BITS;


// If the range of the coding interval is less than RANGE_TOO_LOW, it's
// time to shift a byte into the output.  This gives us back some precision.
const unsigned int RANGE_TOO_LOW = (1 << NUM_LOW_BITS);


// We start out with the maximum range, since that means maximal math precision.
const unsigned long RANGE_INITIAL = 1UL << (NUM_LOW_BITS + NUM_HIGH_BITS);
const unsigned int OVERFLOW_MASK = 1UL << (NUM_LOW_BITS + NUM_HIGH_BITS);

void Arithmetic_Encoder::reset() {
    low = 0;
    range = RANGE_INITIAL;
    num_bytes_output = 0;
}

Arithmetic_Encoder::Arithmetic_Encoder() {
    reset();
}


// has_anything_been_packed() checks to see if we are still satisfying
// the initial conditions set by reset().  If so, nobody ever put anything
// into this coder.  (I use this in networking situations, where I run
// through a bunch of code that might pack data or might not; then at the
// end I check whether there's actually anything to send).
bool Arithmetic_Encoder::has_anything_been_packed() {
    if (low != 0) return true;
    if (range != RANGE_INITIAL) return true;
    if (num_bytes_output != 0) return true;

    return false;
}


// renormalize() is the routine that maintains math precision.  It
// checks to see whether 'range' is too low; if so, it shifts some
// data into the output, then scales up the coding interval.
void Arithmetic_Encoder::renormalize() {
    while (range <= RANGE_TOO_LOW) {
        assert((range >> NUM_LOW_BITS) <= 1);
        assert(range > 0);
        unsigned int output = low >> NUM_LOW_BITS;
        output_byte(output);
        range <<= 8;
        low <<= 8;
        low &= (OVERFLOW_MASK - 1);
    }
}


// output_byte() puts a byte into the output buffer.  Normally
// you'd expect the parameter to be between 0 and 255; however,
// we allow it to have an extra overflow bit on top, so that it
// can be as big as 511.  If it's got an overflow bit, we perform
// a carry into earlier data.  Putting the overflow checking
// here simplifies the rest of the code.
void Arithmetic_Encoder::output_byte(unsigned int value) {
    assert(value < 512);

    if (value >= 256) {
        carry();
        value -= 256;
    }

    assert(value < 256);
    assert(num_bytes_output < AC_MAX_BUFFER_LEN);
    output_bytes[num_bytes_output++] = value;
}


// When it's time to carry, it means we need to add 1 to the
// most recent byte we had output.  Usually that is all we
// need to do; but if that byte is already 255, the carry
// will make it 0, and create another carry bit, which
// we need to propagate one byte earlier in the output. 
// You might imagine this continuing for many output bytes,
// but such a circumstance would be extremely rare.

// This carry stuff is necessary because to maintain precision
// we need to output results whenever the range becomes small;
// but if the range happens to straddle 0.5, then we don't
// know whether the most significant bit will end up being
// 1 or 0.  But we need to shift it out anyway.  The carry
// mechanism is just a way of fixing that when we resolve
// which side of 0.5 we are on.
void Arithmetic_Encoder::carry() {
    int cursor = num_bytes_output;

    while (--cursor >= 0) {
        unsigned int value = output_bytes[cursor];
        if (value == 255) {
            output_bytes[cursor] = 0;
        } else {
            output_bytes[cursor] = value + 1;
            break;
        }
    }
}


// When it's time to put a new value into the encoder, we call pack().
void Arithmetic_Encoder::pack(Symbol_Count *symbol, Data_Model_1D *model) {
    unsigned long numerator = symbol->numerator;
    assert(numerator != 0);

    renormalize();

    // To get the new offset, multiply 'range' by the fraction for
    // the left-hand side of this symbol's probability interval.
    // This fraction is (symbol->base_numerator / (1 << model->denominator_bits)).
    // A slight rearrangement gives the expression below.
    unsigned int offset = (range >> model->denominator_bits) * symbol->base_numerator;

    // Move the low end of the interval by this offset.
    unsigned int old_low = low;
    low += offset;
    assert(low >= old_low);  // Don't overflow out of machine word!

    // Shrink the range by the factor (numerator / model->denominator_bits).
    range >>= model->denominator_bits;
    range *= numerator;
}

// This is the all-equal-probability version of 'pack' (as seen in part 1!)
void Arithmetic_Encoder::pack(int value, int limit) {
    assert(value >= 0);
    assert(value < limit);

    renormalize();

    range /= limit;
    unsigned int old_low = low;
    low += value * range;
    assert(low >= old_low);  // Don't overflow out of machine word!
}


void Arithmetic_Encoder::flush() {
    // This seems tricky, but hey... we want to not output
    // more information than we need, which means we need to output
    // enough bits to yield a number n, where low <= n <= high.
    // The most economical way to do this is output the MSB's of
    // (low + range - 1) until we output a bit that differs from 'low'.

    // But since we're outputting whole bytes at a time here, we want
    // to detect a byte that differs from 'low'.  This is the same as
    // saying, output up to and including the first byte for which
    // (range - 1) is nonzero.  The renormalize() call ensures that that
    // byte becomes the top byte.  Then we just output that top byte.
    // Unless of course 'low' is 0, in which case we get something
    // >= low just by doing nothing.

    // Khalid Sayood's book "Introduction to Data Compression, 2nd Edition" 
    // gives  an expression for computing how many bits you need ideally
    // (section 4.4.1).
    // But it's hard to implement this expression in a robust way,
    // and it also doesn't account for the inherent leakage due to
    // fixed precision math.  Thus our method here.

    renormalize();
    if (low == 0) return;

    unsigned int high = low + range - 1;

    output_byte(high >> NUM_LOW_BITS);
    assert((high >> NUM_LOW_BITS) > (low >> NUM_LOW_BITS));
}


// get_result() is just a wrapper to give us our buffer.
void Arithmetic_Encoder::get_result(unsigned char **data_return, int *length_return) {
    flush();

    *data_return = output_bytes;
    *length_return = num_bytes_output;
}




//
// That's all for the encoder!  Now comes the decoder.
//

void Arithmetic_Decoder::reset() {
    low = 0;
    code = 0;
    range = 1UL << (NUM_LOW_BITS + NUM_HIGH_BITS);

    input_byte_cursor = 0;
}

Arithmetic_Decoder::Arithmetic_Decoder() {
    reset();
}

Arithmetic_Decoder::~Arithmetic_Decoder() {
}


//
// Much like the Encoder's version of renormalize, we are just trying
// to duplicate the action of the encoder here.
//
void Arithmetic_Decoder::renormalize() {
    while (range <= RANGE_TOO_LOW) {
        assert((range >> NUM_LOW_BITS) <= 1);
        assert(range > 0);
        range <<= 8;
        low <<= 8;
        code <<= 8;
        low &= (OVERFLOW_MASK - 1);

        code &= (OVERFLOW_MASK - 1);
        if (code < low) {
            code |= OVERFLOW_MASK;  // Riddle me this.
        }

        code |= input_next_byte();
    }
}

// input_next_byte() pulls the next byte out of the buffer and
// hands it to us.  This code looks more complicated than it
// ought to be; the reason is that we're shifting in 8 bits
// at a time, but those 8 bits don't land exactly on a byte
// boundary.  They're shifted by a bit, as a consequence of the
// fact that our 'range' is only 31 bits.  We might shrink
// 'range' down to 24 bits for the decoder here, but that would
// cause the math to come out differently than on the encoder,
// which ruins the situation.
//
// The modification mentioned earlier that allows 'range' to be
// 32 bits in both the encoder and the decoder would solve this
// problem.
int Arithmetic_Decoder::input_next_byte() {
    int first_part, second_part;
    if ((input_byte_cursor == 0) || (input_byte_cursor > num_input_bytes)) {
        first_part = 0;
    } else {
        assert(input_byte_cursor - 1 < num_input_bytes) ;
        first_part = input_bytes[input_byte_cursor - 1] & 1;
    }

    if (input_byte_cursor >= num_input_bytes) {
        second_part = 0;
    } else {
        second_part = input_bytes[input_byte_cursor] & ~1;
    }

    // @Robustness: This assertion is loose; should figure out exactly
    // what it ought to be.
    assert(input_byte_cursor <= num_input_bytes + 5);

    input_byte_cursor++;
    return (first_part << 7) + (second_part >> 1);
}

// @Speed: find_value is a lot slower than it needs to be,
// because it was written to be simple.
//
// The goal of this function is just to find the correct symbol
// whose probability interval contains the current code value.
// For simplicity we search linearly.  To gain speed, we can do
// a binary search, or something more sophisticated.
unsigned long find_value(Data_Model_1D *model,
                         unsigned long point, unsigned long range) {
    int i;
    for (i = 0; i < model->num_symbols; i++) {
        unsigned long divided = range >> model->denominator_bits;
        unsigned long d0 = divided * model->symbols[i].base_numerator;
        unsigned long d1 = divided * (model->symbols[i].base_numerator + model->symbols[i].numerator);

        if (point >= d1) continue;
        if (point < d0) continue;

        return i;
    }

    assert(0);
    return model->num_symbols - 1;
}

// unpack() is just the entry point that gives us back a value.
unsigned long Arithmetic_Decoder::unpack(Data_Model_1D *model) {
    renormalize();

    // For an explnation of the math here, see Arithmetic_Coder::pack.
    unsigned long value = find_value(model, code - low, range);

    unsigned int offset = (range >> model->denominator_bits) * model->symbols[value].base_numerator;

    unsigned int old_low = low;
    low += offset;
    assert(low >= old_low);  // Don't overflow out of machine word!

    int numerator = model->symbols[value].numerator;

    range >>= model->denominator_bits;
    range *= numerator;

    return value;
}

// This is the all-equal-probability version of 'unpack' (as seen in part 1!)
unsigned long Arithmetic_Decoder::unpack(unsigned long limit) {
    assert(limit >= 1);

    renormalize();

    range /= limit;
    unsigned long value = (code - low) / range;
    assert(value < (unsigned long) limit);
    low += value * range;

    return value;
}


// set_data() is called with the data to decode, when we want to
// start decoding.  We copy the data into an internal buffer, and
// then we also shift the first 4 bytes of that buffer into the
// variable 'code' which represents the part of the number we
// are actively decoding.
void Arithmetic_Decoder::set_data(unsigned char *data, int length) {
    assert(length < AC_MAX_BUFFER_LEN);
    memcpy(input_bytes, data, length);
    num_input_bytes = length;

    int i;
    for (i = 0; i < 4; i++) {
        code <<= 8;
        code |= input_next_byte();
    }
}

// might_there_be_any_data_left() is another function I use to
// help me out when dealing with garbage packets; I wouldn't
// worry about it too much!
bool Arithmetic_Decoder::might_there_be_any_data_left() {
    if (input_byte_cursor < num_input_bytes + 4) return true;
    return false;
}

int Arithmetic_Encoder::get_maximum_data_length() {
    return num_bytes_output + 4;
}

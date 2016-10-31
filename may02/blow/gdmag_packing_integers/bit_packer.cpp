#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "bit_packer.h"

Bit_Packer::Bit_Packer() {
    memset(buffer, 0, PACKER_BUFFER_SIZE * sizeof(u8));
    next_bit_to_write = 0;
}


// Fast version of pack()
const int BITS_PER_WORD = 32;
void Bit_Packer::pack(u32 value, u32 num_bits_to_pack) {
	assert(num_bits_to_pack <= 32);
	assert((value & ((1 << num_bits_to_pack) - 1)) == value);

    // Scoot the value bits up to the top of the word; this makes
    // them easier to work with.

    value <<= (BITS_PER_WORD - num_bits_to_pack);

    // First we do the hard part: pack bits into the first u8,
    // which may already have bits in it.

    int byte_index = (next_bit_to_write / 8);
    int bit_index = (next_bit_to_write % 8);
    int empty_space_this_byte = (8 - bit_index) & 0x7;

    // Update next_bit_to_write for the next call; we don't need 
    // the old value any more.

    next_bit_to_write += num_bits_to_pack;

    u8 *dest = buffer + byte_index;

    if (empty_space_this_byte) {
        int to_copy = empty_space_this_byte;
	    int align = 0;

	    if (to_copy > num_bits_to_pack) {
	        // We don't have enough bits to fill up this u8.
	        align = to_copy - num_bits_to_pack;
	        to_copy = num_bits_to_pack;
	    }

	    u32 fill_bits = value >> (BITS_PER_WORD - empty_space_this_byte);
	    *dest |= fill_bits;

	    num_bits_to_pack -= to_copy;
	    dest++;
	    value <<= to_copy;
    }

    // Now we do the fast and easy part for what is hopefully
    // the bulk of the data.

    while (value) {
        *dest++ = value >> (BITS_PER_WORD - 8);
	    value <<= 8;
    }
}

void Bit_Packer::get_result(char **data_return, int *len_return) {
    int len_in_bytes = (next_bit_to_write + 7) / 8;
    *len_return = len_in_bytes;

    assert(*len_return <= PACKER_BUFFER_SIZE);

    *data_return = (char *)buffer;
}

// Slow version of pack()
/*
void Bit_Packer::pack(u32 value, u32 num_bits_to_pack) {
	assert(num_bits_to_pack <= 32);
	assert((value & ((1 << num_bits_to_write) - 1)) == value);

	while (num_bits_to_write > 0) {
		u32 byte_index = (next_bit_to_write / 8);
		u32 bit_index =  (next_bit_to_write % 8);
		
		u32 src_mask = (1 << (num_bits_to_write - 1));
		u8 dest_mask = (1 << (7 - bit_index));
		
		if (value & src_mask) buffer[byte_index] |= dest_mask;
		next_bit_to_write++;
		num_bits_to_write--;
    }
}
*/


Bit_Unpacker::Bit_Unpacker(char *data, int len) {
    assert(len <= PACKER_BUFFER_SIZE);

    memcpy(buffer, data, len);
    num_bits_remaining = len * 8;
    next_bit_to_read = 0;
}

u32 Bit_Unpacker::unpack(u32 num_bits_to_unpack) {
	u32 result = 0;

    assert(num_bits_to_unpack <= num_bits_remaining);

	while (num_bits_to_unpack) {
		u32 byte_index = (next_bit_to_read / 8);
		u32 bit_index = (next_bit_to_read % 8);
		
		u32 src_mask = (1 << (7 - bit_index));
		u32 dest_mask = (1 << (num_bits_to_unpack - 1));

		if (buffer[byte_index] & src_mask) result |= dest_mask;
		num_bits_to_unpack--;
		next_bit_to_read++;
	}

    num_bits_remaining -= num_bits_to_unpack;

    return result;
}


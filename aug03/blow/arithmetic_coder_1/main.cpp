#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "arithmetic_coder.h"

const int NUM_VALUES_TO_PACK = 100;


// Simple wrapper functions that print out what we're putting into the
// arithmetic coder, and what we're taking out.
void pack_and_report(Arithmetic_Encoder *encoder, int value, int limit) {
	printf("Packing %d (limit %d)\n", value, limit);
	encoder->pack((unsigned long)value, (unsigned long)limit);
}

int unpack_and_report(Arithmetic_Decoder *decoder, unsigned long limit) {
	unsigned long value = decoder->unpack(limit);
	printf("Unpacking a value between 0 and %d... it's %d.\n",
		   limit - 1, value);
    return (int)value;
}

// Test the coder by encoding a bunch of arbitrary numbers;
// then give those numbers to a decoder and unpack them.  Check
// that what we get out matches what we put in, and complain if
// this for some reason is not so!
void main(void) {
	printf("Arithmetic Coder test:\n\n");

    int limit_array[NUM_VALUES_TO_PACK];      // The range of each item we pack
    int value_to_pack[NUM_VALUES_TO_PACK];    // The actual value to pack (between 0 and limit_array[i] - 1)

    // This following process of "first pick a range, then pick a value"
    // might seem a little weird, but all we're doing here is simulating
    // conditions that you'd have in a real program.  You might have one
    // enumerated type WEAPON_TYPE with 50 possibilities, and then another 
    // CHARACTER_CLASS with 4 possibilities, then you might want to pack the 
    // number of hit points that some player has, which is a number between 
    // 0 and 100.  These ranges would all be stored in "limit_array".  If
    // the current weapon is WEAPON_SWORD=7, and the character class is
    // CHARACTER_CLASS_THIEF=2, and the player has 87 hit points, you
    // would pack the following sequence of numbers:

    // value =  7, limit = 50        //// The weapon type is 7
    // value =  2, limit = 4         //// The character class is 2
    // value = 87, limit = 100       //// The player has 87 hit points

    int i;
    for (i = 0; i < NUM_VALUES_TO_PACK; i++) {
        int limit = (rand() % 2000) + 1;      // Pick a random range for each item.
        limit_array[i] = limit;
        value_to_pack[i] = rand() % limit;    // Pick a number within that range.
    }


    // Actually pack all these values we just chose.

	Arithmetic_Encoder encoder;
    for (i = 0; i < NUM_VALUES_TO_PACK; i++) {
        pack_and_report(&encoder, value_to_pack[i], limit_array[i]);
    }


    // Get the result out of the arithmetic coder (as an array of bytes
    // that we can put into a file or transmit over the network).
	unsigned char *data;
	int len;
	encoder.get_result(&data, &len);
	
    printf("\nThe packed result was %d bytes long.\n\n", len);

	// At this point we would, in a real app, transmit this string
	// over the network.  We're going to pretend we just transmitted
	// it, and just received it, and now we'll put it into an
	// Arithmetic_Decoder for processing.

	Arithmetic_Decoder decoder;
    decoder.set_data(data, len);
	
    for (i = 0; i < NUM_VALUES_TO_PACK; i++) {
        int value = unpack_and_report(&decoder, limit_array[i]);
        assert(value == value_to_pack[i]);
    }

	printf("\nTest complete.\n\n\n");
}

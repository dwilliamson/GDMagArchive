#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "bit_packer.h"
#include "multiplication_packer.h"

//
// Bit_Packer utility functions here.
//

u32 necessary_bits(u32 range_max) {
	u32 result = 1;
	while (1) {
		assert(result < 32);

		u32 max_value = (u32)(1 << result) - 1;
		if (max_value >= range_max) return result;

		result++;
	}
}

void pack_and_report(Bit_Packer *packer, u32 range_max) {
	u32 num_bits = necessary_bits(range_max);
	u32 value = rand() % range_max;

	printf("Packing a value between 0 and %d (%d bits)... chose %d.\n",
		   range_max - 1, num_bits, value);

	packer->pack(value, num_bits);
}

void unpack_and_report(Bit_Unpacker *unpacker, u32 range_max) {
	u32 num_bits = necessary_bits(range_max);
	u32 value = unpacker->unpack(num_bits);
	printf("Unpacking a value between 0 and %d (%d bits)... it was %d.\n",
		   range_max - 1, num_bits, value);
}


//
// Multiplication_Packer utility functions here.
//
void pack_and_report(Multiplication_Packer *packer, u32 range_max) {
	u32 value = rand() % range_max;

	printf("Choosing and packing an integer between 0 and %d... it was %d.\n",
		   range_max - 1, value);
	
	packer->pack(range_max, value);
}

void unpack_and_report(Multiplication_Packer *packer, u32 range_max) {
	int value = packer->unpack(range_max);
	printf("Unpacking an integer fbetween 0 and %d... it was %d.\n",
		   range_max - 1, value);
}

void test_bit_packer() {
	printf("Bit Packer test:\n\n");

	Bit_Packer packer;
	pack_and_report(&packer, 15);
	pack_and_report(&packer, 38);
	pack_and_report(&packer, 3);
	pack_and_report(&packer, 2);
	pack_and_report(&packer, 3);
	pack_and_report(&packer, 7);
	pack_and_report(&packer, 118);

	char *data;
	int len;
	packer.get_result(&data, &len);
	
    printf("\nThe packed result was %d bytes long.\n\n", len);

	// At this point we would, in a real app, transmit this string
	// over the network.  We're going to pretend we just transmitted
	// it, and just received it, and now we'll put it into a 
	// Bit_Unpacker for processing.

	Bit_Unpacker unpacker(data, len);
	
	unpack_and_report(&unpacker, 15);
	unpack_and_report(&unpacker, 38);
	unpack_and_report(&unpacker, 3);
	unpack_and_report(&unpacker, 2);
	unpack_and_report(&unpacker, 3);
	unpack_and_report(&unpacker, 7);
	unpack_and_report(&unpacker, 118);

	printf("\nBit Packer test complete.\n\n\n");
}

void test_multiplication_packer() {
	printf("Multiplication Packer test:\n\n");

	Multiplication_Packer packer;
	pack_and_report(&packer, 12);
	pack_and_report(&packer, 69);
	pack_and_report(&packer, 105);
	pack_and_report(&packer, 2);
	pack_and_report(&packer, 3);
	pack_and_report(&packer, 3);

	u32 result, result_limit;
	result = packer.get_result(&result_limit);

	printf("\nThe result is %d; the limit is %d.\n\n", result, result_limit);
	
	// At this point we would, in a real app, transmit 'result'
	// over the network ('result_limit' is one plus the
	// maximum value of 'result', which tells us how much
	// bandwidth to use.)  We're going to pretend we just transmitted
	// it, and just received it, and now we'll put it into a different
	// Multiplication_Packer for unpacking.

	Multiplication_Packer unpacker;
	unpacker.set_accumulator(result);
	
	unpack_and_report(&unpacker, 3);
	unpack_and_report(&unpacker, 3);
	unpack_and_report(&unpacker, 2);
	unpack_and_report(&unpacker, 105);
	unpack_and_report(&unpacker, 69);
	unpack_and_report(&unpacker, 12);

	printf("\nMultiplication Packer test complete.\n");
}

void main(void) {
	test_bit_packer();
	test_multiplication_packer();
}

#include "../framework.h"
#include "bit_array.h"

Bit_Array::Bit_Array(int num_bits) {
    assert(num_bits >= 0);

    num_longs = num_bits / sizeof(unsigned long);
    int remainder = num_bits % sizeof(unsigned long);
    if (remainder) num_longs++;

    data = new unsigned long[num_longs];
}

Bit_Array::~Bit_Array() {
    delete [] data;
}

void Bit_Array::clear_all() {
    int i;
    for (i = 0; i < num_longs; i++) data[i] = 0;
}

void Bit_Array::copy_from(Bit_Array *other) {
    assert(num_longs == other->num_longs);
    int i;
    for (i = 0; i < num_longs; i++) data[i] = other->data[i];
}

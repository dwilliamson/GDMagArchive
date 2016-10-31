#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "multiplication_packer.h"

Multiplication_Packer::Multiplication_Packer() {
    accumulator = 0;
    composite_limit = 1;
}

void Multiplication_Packer::pack(u32 limit, u32 value) {
    u32 new_composite_limit = composite_limit * limit;
    assert(new_composite_limit >= composite_limit);
    composite_limit = new_composite_limit;
    assert(value < limit);
    accumulator = (limit * accumulator) + value;
}

u32 Multiplication_Packer::get_result(u32 *limit_return) {
    if (limit_return) *limit_return = composite_limit;
    return accumulator;
}

u32 Multiplication_Packer::unpack(u32 limit) {
    u32 quotient = accumulator / limit;
    u32 remainder = accumulator % limit;
    accumulator = quotient;
    return remainder;
}

void Multiplication_Packer::set_accumulator(u32 a) {
    accumulator = a;
}

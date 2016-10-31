#include "general.h"
#include "bytecode.h"

Lerp_Bytecode::Lerp_Bytecode() {
    data = NULL;
    type = ARG_BYTECODE;
}

Lerp_Bytecode::~Lerp_Bytecode() {
    delete [] data;
}

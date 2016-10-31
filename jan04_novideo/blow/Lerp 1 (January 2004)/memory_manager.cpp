#include "general.h"
#include "interp.h"

#include "bytecode.h"
#include "bytecode_runner.h"  // For Lerp_Call_Record
#include "memory_manager_private.h"
#include <stdlib.h>

// @Refactor: These are some temporary things to help me decide what to do; take them out (for formalize them better!)
static int num_allocs = 0;
static int num_releases = 0;
static int num_depth = 0;
static int num_premade = 0;

int num_1_len = 0;
int num_other_len = 0;

const int POOL_ALLOCATION_SIZE = 1024 * 1024 * 4;
const int NUM_CACHED_CALL_RECORDS = 50;
const int CACHED_CALL_RECORD_NUM_REGISTERS = 150;  // @Memory: should be able to reduce this!

struct Lerp_Call_Record_Cached {
    Lerp_Call_Record_Cached *next;
};

Lerp_Memory_Manager::Lerp_Memory_Manager(Lerp_Interp *_interp) {
    int arena_size = 16 * 1024 * 1024;
    memory_oldspace = new Memory_Arena(arena_size);
    memory_newspace = new Memory_Arena(arena_size);

    num_pointers_to_follow = 0;

    interp = _interp;
    cached_call_records = NULL;

    gc_color_black = GC_COLOR_BLACK_OR_WHITE_1;
    gc_color_white = GC_COLOR_BLACK_OR_WHITE_2;
    allocation_color = gc_color_white;

    int i;
    for (i = 0; i < NUM_TYPE_ENUMS; i++) {
        allocation_tracker[i] = 0;
        allocation_count[i] = 0;
    }
/*
    for (i = 0; i < NUM_CACHED_CALL_RECORDS; i++) {
        create_cached_call_record();
    }
*/
}

Lerp_Memory_Manager::~Lerp_Memory_Manager() {
    delete memory_oldspace;
    delete memory_newspace;
}

void *Lerp_Memory_Manager::allocate(int num_bytes, int type_ident) {
    allocation_tracker[type_ident] += num_bytes;
    allocation_count[type_ident]++;


    Memory_Arena *space = memory_newspace;
    assert(space->size_remaining >= num_bytes);

    char *mem = space->cursor;
    space->cursor += num_bytes;
    space->size_remaining -= num_bytes;

    First_Class *result = (First_Class *)mem;
    result->gc_color = gc_color_white;  // We allocate white for the hell of it
    return (void *)result;
}

void Lerp_Memory_Manager::print_allocations() {
    int sum = 0;
    int i;
    for (i = 0; i < NUM_TYPE_ENUMS; i++) {
        printf("%4d: (%8d): %d\n", i, allocation_count[i], allocation_tracker[i]);
        sum += allocation_tracker[i];
    }

    printf("--- Sum: %d ---  (allocs %d, releases %d, depth %d, premade %d)   len1: %d   other: %d\n", sum, num_allocs, num_releases, num_depth, num_premade, num_1_len, num_other_len);
}

Lerp_Call_Record *Lerp_Memory_Manager::create_call_record(int num_registers) {
    Lerp_Call_Record *result;

    if (cached_call_records && (num_registers <= CACHED_CALL_RECORD_NUM_REGISTERS)) {
        result = (Lerp_Call_Record *)cached_call_records;
        cached_call_records = cached_call_records->next;
        num_depth--;
    } else {
        result = create_call_record_really(num_registers);
    }

    // @Speed: In some cases we will go and just re-write some of these
    // values anyway, so maybe we want a constructor that doesn't bother
    // to initialize them.  For now though I am being safe, due to gc 
    // pointer-traversal concerns.
    result->type = ARG_IMPERATIVE_CALL_RECORD;
    result->bytecode = NULL;
    result->register_for_return_value = -1;
    result->num_registers = num_registers;
    result->this_pointer = NULL;
    result->previous_context = NULL;
    result->namespace_stack = NULL;

    int i;
    for (i = 0; i < num_registers; i++) {
        result->registers[i] = NULL;
    }
    
    return result;
}

Lerp_Call_Record *Lerp_Memory_Manager::create_call_record_really(int num_registers) {
    int size = Lerp_Call_Record::allocation_size(num_registers);
    Lerp_Call_Record *result = (Lerp_Call_Record *)allocate(size, TYPE_ENUM_Lerp_Call_Record);
    result->cached = 0;
    num_allocs++;
    
    return result;
}

Value_Array *Lerp_Memory_Manager::create_value_array(int num_values) {
    int size = Value_Array::allocation_size(num_values);
    Value_Array *result = (Value_Array *)allocate(size, TYPE_ENUM_Value_Array);
    result->type = ARG_VALUE_ARRAY;
    result->num_values = num_values;

    return result;
}

void Lerp_Memory_Manager::create_cached_call_record() {
    Lerp_Call_Record *record = create_call_record_really(CACHED_CALL_RECORD_NUM_REGISTERS);
    record->cached = 1;

    Lerp_Call_Record_Cached *cached = (Lerp_Call_Record_Cached *)record;
    cached->next = cached_call_records;
    cached_call_records = cached;
    num_depth++;
    num_premade++;
}

void Lerp_Memory_Manager::release_call_record(Lerp_Call_Record *record) {
    if (record->cached) {
        Lerp_Call_Record_Cached *cached = (Lerp_Call_Record_Cached *)record;
        cached->next = cached_call_records;
        cached_call_records = cached;
        num_depth++;
        num_releases++;
    } else {
        //
        // @GC
        // We just leak it and let the gc find it!  Instead, at some point we
        // could explicitly free this... might be worthwhile, it's unclear.
        //
    }
}

Decl_Expression *Lerp_Memory_Manager::create_decl_expression(int num_arguments) {
    assert(num_arguments < 65536);
    assert(num_arguments >= 0);

    int size = Decl_Expression::allocation_size(num_arguments);
    void *mem = allocate(size, TYPE_ENUM_Decl_Expression);

    Decl_Expression *expression = new (mem) Decl_Expression();

    expression->num_arguments = num_arguments;
    return expression;
}

Lerp_Bytecode *Lerp_Memory_Manager::create_bytecode(int num_constants) {
    assert(num_constants < 65536);
    assert(num_constants >= 0);

    int size = Lerp_Bytecode::allocation_size(num_constants);
    void *mem = allocate(size, TYPE_ENUM_Bytecode);

    Lerp_Bytecode *bytecode = new (mem) Lerp_Bytecode();

    bytecode->num_constants = num_constants;
    return bytecode;
}



Memory_Arena::Memory_Arena(int _size) {
    size = _size;
    size_remaining = size;
    data = new char[size];
    cursor = data;
}

Memory_Arena::~Memory_Arena() {
    delete [] data;
}

void Memory_Arena::reset() {
    cursor = data;
    size_remaining = size;
}


char *Lerp_Memory_Manager::get_allocation_pointer() {
    Memory_Arena *space = memory_newspace;
    return space->cursor;
}

void Lerp_Memory_Manager::set_allocation_pointer_with_alignment(char *pointer) {
    Memory_Arena *space = memory_newspace;

    // @WordSize need to make this 64-bit safe at some point.

    // Round the pointer forward to the next multiple of 4.
    unsigned long ipointer = (unsigned long)pointer;
    if (ipointer & 0x3) {
        ipointer += 4;
        ipointer &= ~0x3;
        pointer = (char *)ipointer;
    }
    
    space->cursor = pointer;
    assert(space->cursor >= space->data);
    assert(space->cursor <= space->data + space->size);
    
    int size_remaining = space->size - (space->cursor - space->data);
    space->size_remaining = size_remaining;
}

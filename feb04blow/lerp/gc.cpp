#include "general.h"
#include "interp.h"

#include "bytecode_runner.h"  // For Lerp_Call_Record
#include "schema.h"
#include "memory_manager_private.h"
#include "bytecode.h"
#include "hash_table.h"
#include <stdlib.h>

// XXXXXXXX Must do custom types

// XXXXXXXX Reactivate red floats, integers, call records

// XXXXXXXX formalize db_constraints


#define Newspace(T)   T *oldspace = (T *)oldspace_fc;  T *newspace = (T *)allocate(sizeof(T), 0); newspace->type = oldspace->type;
#define Newspace2(T, n)   T *oldspace = (T *)oldspace_fc;  T *newspace = (T *)allocate(T::allocation_size(n), 0); newspace->type = oldspace->type;

#define newspace_allocate allocate

template <class T> 
inline int Barrier <T>::get_gc_color() {
    return (int)gc_color;
}

void Lerp_Memory_Manager::swap_gc_colors() {
    int tmp = gc_color_black;
    gc_color_black = gc_color_white;
    gc_color_white = tmp;
}

void Lerp_Memory_Manager::swap_gc_spaces() {
    Memory_Arena *mtmp = memory_oldspace;
    memory_oldspace = memory_newspace;
    memory_newspace = mtmp;

    memory_newspace->reset();
#ifdef _DEBUG
    memset(memory_newspace->data, 0xdd, memory_newspace->size);
#endif
}

#define POINTER(name) handle_pointer((Barrier <First_Class *> *)(oldspace->name), (First_Class **)(&newspace->name));
#define COPY(name) newspace->name = oldspace->name;

inline First_Class *Lerp_Memory_Manager::get_easy_pointer(Barrier <First_Class *> *barrier) {
    switch (barrier->type) {
    case ARG_FORWARDING_POINTER:
        return barrier->forwarding_pointer;
    case ARG_BLOB_UNMANAGED:
    case ARG_ATOM:
    case ARG_VARIABLE:
    case DECL_QUALIFIER_DOMAIN: {
        return (First_Class *)barrier;  // They're not forwarding pointers so we look at them directly!
    }
    }

    First_Class *fc = (First_Class *)barrier;  // We can do this since we know it's not a forwarding pointer... we took care of that case above.

    int color = barrier->get_gc_color();
    if (color == gc_color_black) return fc;
    if (color == GC_COLOR_RED) return fc;
    
    assert((color == gc_color_white) || (color == GC_COLOR_GRAY));

    return NULL;
}

void Lerp_Memory_Manager::handle_pointer(Barrier <First_Class *> *oldspace_barrier, First_Class **newspace_ptr) {
    if (oldspace_barrier == NULL) {
        *newspace_ptr = NULL;
        return;
    }

    First_Class *easy = get_easy_pointer(oldspace_barrier);
    if (easy) {
        *newspace_ptr = easy;
        return;
    }

    // Note an extra pointer...

    assert(num_pointers_to_follow < MAX_POINTERS_TO_FOLLOW);
    *newspace_ptr = (First_Class *)oldspace_barrier;
    pointers_to_follow[num_pointers_to_follow++] = (Barrier <First_Class *> **)newspace_ptr;
}

void breaky() {
}

First_Class *Lerp_Memory_Manager::gc_move(First_Class *oldspace_fc) {
    First_Class *result = NULL;

    int type = oldspace_fc->type;  // For debugging; it gets overwritten.

    if (oldspace_fc == (First_Class *)0x008d056c) breaky();

    switch (type) {
    case ARG_INTEGER: {
        Newspace(Integer);
        COPY(value);
        result = newspace;
        break;
    }
        
    case ARG_FLOAT: {
        Newspace(Float);
        COPY(value);
        result = newspace;
        break;
    }
    case ARG_STRING: {
        // @GC @Robustness
        // For now we make the bald assumption that the string will fit in newspace.
        // (This has to be true for now since both spaces are the same size and we don't
        // create anything bigger than what was in the oldspace).  So we just plow ahead
        // and don't check for overflow.  Of course we can't have infinitely long strings,
        // but that was already true.  In future, a more thorough string implementation
        // will chunk strings into pieces so that they can fit reasonably in GC pages,
        // or something like that.
        String *oldspace = (String *)oldspace_fc;
        String *newspace = (String *)get_allocation_pointer();
        newspace->type = oldspace->type;

        char *s = oldspace->value;
        char *t = newspace->value;
        while (*t++ = *s++);

        set_allocation_pointer_with_alignment(t);

        result = newspace;
        break;
    }
    case ARG_DATABASE: {
        Newspace(Database);
        POINTER(schema);
        POINTER(assertions);
        result = newspace;
        break;
    }
    case ARG_SCHEMA: {
        Newspace(Schema);
        POINTER(named_slots);
        POINTER(type_name);

        COPY(instantiation_count);
        COPY(type_tag);
        COPY(flags);

        COPY(db_constraints);  // XXXXXXXXXXXXXx

        result = newspace;
        break;
    }
    case ARG_DECL_ASSERTION: {
        Newspace(Decl_Assertion);
        POINTER(next);
        POINTER(expression);
        POINTER(conditionals);
        COPY(num_variables);
        COPY(variables);  // XXX @Memory @GC
        COPY(flags);
        COPY(scalar_result_index);

        result = newspace;
        break;
    }
    case ARG_DECL_EXPRESSION: {
        Newspace2(Decl_Expression, oldspace->num_arguments);
        POINTER(next);
        COPY(flags);
        COPY(num_arguments);
        int i;
        for (i = 0; i < newspace->num_arguments; i++) {
            POINTER(arguments[i]);
        }
        result = newspace;
        break;
    }
    case ARG_VALUE_ARRAY: {
        Newspace2(Value_Array, oldspace->num_values);
        COPY(num_values);
        int i;
        for (i = 0; i < newspace->num_values; i++) {
            POINTER(values[i]);
        }
        result = newspace;
        break;
    }
    case ARG_VALUE_PAIR: {
        Newspace(Value_Pair);
        POINTER(left);
        POINTER(right);
        result = newspace;
        break;
    }
    case ARG_IMPERATIVE_PROCEDURE: {
        Newspace(Procedure);
        COPY(name);
        COPY(proc);
        POINTER(bytecode);
        COPY(hash_key);
        result = newspace;
        break;
    }
    case ARG_IMPERATIVE_CALL_RECORD: {
        Newspace2(Lerp_Call_Record, oldspace->num_registers);
        POINTER(previous_context);
        POINTER(bytecode);
        POINTER(this_pointer);
        POINTER(namespace_stack);

        newspace->register_for_return_value = oldspace->register_for_return_value;
        newspace->cached = oldspace->cached;
        newspace->program_counter = oldspace->program_counter;
        newspace->num_registers = oldspace->num_registers;

        int i;
        for (i = 0; i < newspace->num_registers; i++) {
            POINTER(registers[i]);
        }
        result = newspace;
        break;
    }
    case ARG_BYTECODE: {
        Newspace2(Lerp_Bytecode, oldspace->num_constants);
        COPY(num_arguments);
        COPY(num_registers);
        COPY(name);
        COPY(data);
        COPY(length);
        COPY(debug_info);
        COPY(num_constants);

        POINTER(argument_info);
        POINTER(procedure);

        int i;
        for (i = 0; i < newspace->num_constants; i++) {
            POINTER(constants[i]);
        }

        result = newspace;
        break;
    }

    case ARG_UNINITIALIZED:
    case ARG_FORWARDING_POINTER:
    case ARG_BLOB_UNMANAGED:
    case ARG_ATOM:
    case ARG_VARIABLE:
        assert(0);
        break;
    }

    assert(result != NULL);

    Barrier <First_Class *> *barrier = ToBarrier(oldspace_fc);
    barrier->type = ARG_FORWARDING_POINTER;
    barrier->forwarding_pointer = result;

    result->gc_color = gc_color_black;

    return result;
}

First_Class *Lerp_Memory_Manager::gc_consider(void *ptr) {
    if (ptr == NULL) return NULL;

    Barrier <First_Class *> *barrier = (Barrier <First_Class *> *)ptr;
    First_Class *easy = get_easy_pointer(barrier);
    if (easy) return easy;

    // Looks like we have to copy this guy!
    return gc_move((First_Class *)ptr);
}

void Lerp_Memory_Manager::gc() {
    //
    // Swap spaces
    //
    swap_gc_spaces();
    allocation_color = gc_color_black;  // What we allocate now is known to be reachable...

    //
    // Enumerate root set
    //
    interp->enumerate_gc_roots();

    //
    // Start tracing all the pointers that we acquired from the root set...
    //

    while (num_pointers_to_follow) {
        Barrier <First_Class *> **ptr = pointers_to_follow[--num_pointers_to_follow];
        First_Class *result = gc_consider(*ptr);
        *ptr = ToBarrier(result);
    }

    //
    // We're done... finish up...
    //

    swap_gc_colors();

    // Switching back to allocation by the rest of the system... so it allocates white.
    allocation_color = gc_color_white;
}


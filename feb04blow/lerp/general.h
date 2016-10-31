#include "lerp_os_specific.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include "data_structures.h"
#include "memcpy.h"

#include <new.h>  // Annoying thing to make placement new work in Visual C++

struct Decl_Assertion;
struct Lerp_Bytecode;
struct Lerp_Interp;
struct Lerp_Call_Record;
struct Schema;

typedef char Text_Utf8;
Text_Utf8 *copy_string(char *name);

const int LERP_SYSTEM_VERSION = 0;

enum Argument_Type {
    ARG_ATOM = 0,
    ARG_VARIABLE,
    ARG_INTEGER,
    ARG_FLOAT,
    ARG_STRING,
    ARG_DATABASE,                //  = 5
    ARG_SCHEMA,
    ARG_DECL_ASSERTION,
    ARG_DECL_EXPRESSION,
    ARG_DECL_EXPRESSION_PROCEDURAL,
    ARG_IMPERATIVE_PROCEDURE,
    ARG_BLOB_UNMANAGED,
    ARG_BYTECODE,

    // @Refactor: Mark these as internal...
    DECL_QUALIFIER_DOMAIN,
    ARG_IMPERATIVE_CALL_RECORD,
    ARG_BINDING,              
    ARG_MATCHED_GOAL,
    ARG_VALUE_ARRAY,
    ARG_FORWARDING_POINTER,
    ARG_VALUE_PAIR,

    ARG_UNINITIALIZED,

    ARG_NUM_TYPES
};



//
// (Atom *) and (Variable *) can't move right now, since they are
// stored in external storage... so they don't require a barrier.
// Other types do...
//

struct First_Class {
    unsigned short type;
    unsigned short gc_color;
};

//
// Defining the Barrier type...
// It's not true that everything is really a Barrier, we are just
// using this as a device to force us to follow forwarding pointers
// when necessary (The garbage collector may require this, as it moves
// things around).

template <class T>
struct Barrier {
    unsigned short type;
    unsigned short gc_color;

    First_Class *forwarding_pointer;

    int get_gc_color();
    T read();
};

template <class T> 
inline T Barrier <T>::read() {
    // @Speed: We can try to eliminate the check for NULL here but we will
    // have to go code review everyone who checks the read barrier to make
    // sure they are not NULL, and that is a pain in the ass and hard
    // to get right...
    if (this == NULL) return NULL;
    if (type != ARG_FORWARDING_POINTER) return (T)this;

    // A forwarding pointer must always point to valid data!
    assert(forwarding_pointer->type != ARG_FORWARDING_POINTER);
    return (T)forwarding_pointer;
}


template <class T>
Barrier <T> *ToBarrier(T ptr) {
    return (Barrier <T> *)ptr;
}

inline Barrier <First_Class *> *ToBarrierF(First_Class *ptr) {
    return (Barrier <First_Class *> *)ptr;
}


struct Value_Pair : public First_Class {
    Value_Pair();

    Barrier <First_Class *> *left;
    Barrier <First_Class *> *right;
};

struct Value_Array : public First_Class {
    unsigned long num_values;
    Barrier <First_Class *> *values[1];

    static int allocation_size(int num_values);
};

inline int Value_Array::allocation_size(int num_values) {
    int extra_bytes = (num_values - 1) * sizeof(First_Class *);  // This could be -4!
    int size = sizeof(Value_Array) + extra_bytes;
    return size;
}


inline Value_Pair::Value_Pair() {
    // The caller must initialize left and right (for performance reasons)
    type = ARG_VALUE_PAIR;
}

struct Atom : public First_Class {
    int scratch_marker;  // Intended for localized, short-term use.
    Text_Utf8 *name;
};

struct Integer : public First_Class {
    int value;
};

struct Float : public First_Class {
    double value;
};

struct String : public First_Class {
    Text_Utf8 value[1];       // We dynamically allocate past the end of the structure...
};

struct Variable : public First_Class {  // XXXXX Is this type even used?
    int scratch_marker;  // Intended for localized, short-term use.
    Text_Utf8 *name;   // XXXXX Should this be Atom?
};

struct Procedure : public First_Class {  // @Incomplete: Need typechecking info for arguments to be stored here... also perhaps put the 'arguments' stuff on Bytecode here instead.
    Procedure();

    Atom *name;
    void (*proc)(Lerp_Interp *interp, Lerp_Call_Record *record);
    Barrier <Lerp_Bytecode *> *bytecode;
    unsigned long hash_key;
};

inline Procedure::Procedure() {
    hash_key = 0;  // We set this so that we can check later that it has really been initialized.
                   // @Speed:  Make this happen only in debug build.

    name = (Atom *)0xbebac0ca;
}

enum {  // Flags for Decl_Expression
    DECL_EXPR_FLAGS_WILDCARDED = 0x1,

    DECL_EXPR_JUNK,     // This is just here so we can get DECL_EXPR_LAST_FLAG without explicitly naming an argument (that would encourage code rot)
    DECL_EXPR_LAST_FLAG = DECL_EXPR_JUNK - 1
};

struct Decl_Expression : public First_Class {
    Decl_Expression();

    Barrier <Decl_Expression *> *next;
    unsigned short flags;
    unsigned short num_arguments;

    Barrier <First_Class *> *arguments[1];

    void initialize_slot(int n, First_Class *value);
    static int allocation_size(int num_arguments);
};

inline Decl_Expression::Decl_Expression() {
    type = ARG_DECL_EXPRESSION;
    flags = 0;
    next = NULL;
}

inline void Decl_Expression::initialize_slot(int index, First_Class *value) {
    // @GC @WriteBarrier: do we need to do something special here?  Probably not
    // since this is an initialization, not a modification... but it wouldn't
    // hurt to be safe.  Probably.

    arguments[index] = ToBarrier(value);
}

inline int Decl_Expression::allocation_size(int num_arguments) {
    int extra_bytes = (num_arguments - 1) * sizeof(First_Class *);  // This could be -4!
    int size = sizeof(Decl_Expression) + extra_bytes;
    return size;
}

struct Decl_Expression_Procedural : public First_Class {
    Decl_Expression_Procedural();

    Barrier <Decl_Expression *> *next;  // Must be in the same place as it is for Decl_Expression!
    Lerp_Bytecode *proc;
};

inline Decl_Expression_Procedural::Decl_Expression_Procedural() {
    type = ARG_DECL_EXPRESSION_PROCEDURAL;
}

struct Decl_Assertion : public First_Class {
    Decl_Assertion();

    Barrier <Decl_Assertion *> *next;
    Barrier <Decl_Expression *> *expression;
    Barrier <Decl_Expression *> *conditionals;

    int num_variables;
    Variable **variables;   // XXX @Memory @GC external storage
    unsigned short flags;
    unsigned short scalar_result_index;
};

struct Binding : public First_Class {
//    Barrier <Variable *> *variable;
    Variable *variable;
    Barrier <First_Class *> *bound_value;
    Barrier <Binding *> *next;

    void assign(First_Class *value);
};

inline void Binding::assign(First_Class *value) {
    bound_value = ToBarrier(value);
}


struct Matched_Goal : public First_Class {
    Barrier <Decl_Assertion *> *assertion;
    Barrier <Binding *> *bindings;
    Barrier <Matched_Goal *> *next;

    ~Matched_Goal();
};

struct Database : public First_Class {
    Database();

    LEXPORT void clear();
    LEXPORT void add_assertion(Decl_Assertion *assertion);
    LEXPORT void add_assertion(Lerp_Interp *interp, Decl_Expression *expression);

    LEXPORT void add_assertion(Lerp_Interp *interp, First_Class *arg1);
    LEXPORT void add_assertion(Lerp_Interp *interp, First_Class *arg1, First_Class *arg2);
    LEXPORT void add_assertion(Lerp_Interp *interp, First_Class *arg1, First_Class *arg2, First_Class *arg3);

    LEXPORT void assign_named_slot(Lerp_Interp *, Atom *name, Barrier <First_Class *> *value);
    LEXPORT void assign_named_slot(Lerp_Interp *, Atom *name, First_Class *value);
    LEXPORT First_Class *lookup_named_slot(Lerp_Interp *, Atom *name);

    Barrier <Schema *> *schema;
    Barrier <Decl_Assertion *> *assertions;
};

inline Database::Database() {
    type = ARG_DATABASE;
    assertions = NULL;
    schema = NULL;
}

inline void Database::add_assertion(Decl_Assertion *assertion) {
    assertion->next = assertions;
    assertions = ToBarrier(assertion);
}

enum {
    ASSERTION_FLAGS_HAS_SCALAR_RESULT = 0x1,
    ASSERTION_FLAGS_CONST = 0x2
};

inline Decl_Assertion::Decl_Assertion() {
    type = ARG_DECL_ASSERTION;
    expression = NULL;
    conditionals = NULL;
    flags = 0;
    next = NULL;
}


struct Blob : public First_Class {
    Blob();
    void *get_data_pointer();

    int size_in_bytes;
};

inline Blob::Blob() {
    type = ARG_BLOB_UNMANAGED;
}

inline void *Blob::get_data_pointer() {
    char *base = (char *)&size_in_bytes;
    base += sizeof(size_in_bytes);

    return (void *)base;
}



struct Uninitialized : public First_Class {
    Uninitialized();
};

inline Uninitialized::Uninitialized() {
    type = ARG_UNINITIALIZED;
}


struct Integer_Constraint_Info {
    int min, max;
    bool min_defined;
    bool max_defined;
};

struct Float_Constraint_Info {
    float min, max;
    int num_quanta;
    bool min_defined;
    bool max_defined;
    bool quanta_defined;
};

struct Type_Constraint {
    Atom *type_name;
    union {
        Integer_Constraint_Info integer_constraint;
        Float_Constraint_Info float_constraint;
    } info;
};


struct Decl_Constraint {
    Decl_Constraint();

    List arguments;
    List modifiers;

    int num_domain_arguments;
};

inline Decl_Constraint::Decl_Constraint() {
    num_domain_arguments = 0;
}

/*
struct Decl_Qualifier_Domain : public First_Class {
    Decl_Qualifier_Domain();
    int num_domain_items;
    int *domain_indices;  // Allocated separately
};

inline Decl_Qualifier_Domain::Decl_Qualifier_Domain() {
    type = DECL_QUALIFIER_DOMAIN;
}
*/

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "data_structures.h"

char *copy_string(char *name);




enum Argument_Type {
    ARG_ATOM = 0,
    ARG_VARIABLE,
    ARG_INTEGER_CONSTANT,
    ARG_PRIMITIVE_FUNCTION,
    
    ARG_DECL_EXPRESSION_SEQUENCE,
    ARG_DECL_EXPRESSION,
    ARG_DECL_ASSERTION
};

struct First_Class {
    Argument_Type type;
};

struct Atom : public First_Class {
    char *name;
};

struct Integer : public First_Class {
    int value;
};

struct Variable : public First_Class {
    char *name;
};

struct Primitive_Function : public First_Class {
    char *name;
    void (*proc)();
};

struct Decl_Expression : public First_Class {
    Decl_Expression();
    List arguments;
};

struct Decl_Expression_Sequence : public First_Class {
    Decl_Expression_Sequence();
    List expressions;
};

struct Decl_Assertion : public First_Class {
    Decl_Assertion();
    Decl_Expression *expression;
    Decl_Expression_Sequence *conditionals;
};

struct Binding {
    Variable *variable;
    First_Class *bound_value;
};

struct Matched_Goal {
    Decl_Assertion *assertion;
    List bindings;

    ~Matched_Goal();
};


inline Decl_Assertion::Decl_Assertion() {
    type = ARG_DECL_ASSERTION;
    conditionals = NULL;
}

inline Decl_Expression::Decl_Expression() {
    type = ARG_DECL_EXPRESSION;
}

inline Decl_Expression_Sequence::Decl_Expression_Sequence() {
    type = ARG_DECL_EXPRESSION_SEQUENCE;
}


struct Ast_Lexical_Scope;

enum Ast_Type {
    // AST_BLOCK = ARG_NUM_TYPES + 1,
    // Set it to 100 instead so I can figure stuff out in the debugger...

    AST_BLOCK = 100,
    AST_STATEMENT,
    AST_UNARY_EXPRESSION,
    AST_BINARY_EXPRESSION,
    AST_PROCEDURE_CALL_EXPRESSION,
    AST_PROC_DECLARATION,
    AST_IMPLICIT_VARIABLE,
    AST_WHILE,
    AST_IF,
    AST_BREAK,
    AST_EACH_HARDENED,
    AST_EACH_PLACEHOLDER,
    AST_TYPE_INSTANTIATION,
    AST_ARRAY_SUBSCRIPT,

    AST_DECLARATIVE_EXPRESSION_PROCEDURAL,
    AST_DECLARATIVE_EXPRESSION,            // 115
    AST_DECLARATIVE_ASSERTION,

//    AST_TYPE_VALUE

    AST_LEXICAL_SCOPE
};

struct Ast : public First_Class {  // Not really First_Class!!  XXX
};

struct Ast_Declarative : public Ast {
};

/*
struct Ast_Declarative_Database_Entry : public Ast {
    Ast_Declarative_Database_Entry();
    Decl_Assertion *assertion;
};

inline Ast_Declarative_Database_Entry::Ast_Declarative_Database_Entry() {
    type = AST_DECLARATIVE_DATABASE_ENTRY;
}
*/

struct Ast_Block : public Ast {  
    Ast_Block();

    List scopes;      // Each of these is an Atom.
    List statements;  // Each of these is an Ast_Statement.
};


inline Ast_Block::Ast_Block() {
    type = AST_BLOCK;
//    scope = NULL;
}

/*
struct Ast_Type_Value : public Ast {
    Ast_Type_Value();
    Atom *name;
};

inline Ast_Type_Value::Ast_Type_Value() {
    type = AST_TYPE_VALUE;
};

struct Struct_Declaration : public Ast {
    Struct_Declaration();

    Atom *name;
    List members;
};

inline Struct_Declaration::Struct_Declaration() {
    type = AST_STRUCT_DECLARATION;
}

struct Struct_Member {
    Atom *name;
    Atom *type_name;
};
*/

struct Ast_Expression : public Ast {
};

struct Ast_Unary_Expression : public Ast_Expression {  
    Ast_Unary_Expression();
    int operator_token_type;
    Ast_Expression *subexpression;
};


inline Ast_Unary_Expression::Ast_Unary_Expression() {
    type = AST_UNARY_EXPRESSION;
}


struct Ast_Binary_Expression : public Ast_Expression {  
    Ast_Binary_Expression();
    int operator_token_type;
    Ast_Expression *left;
    Ast_Expression *right;
};


inline Ast_Binary_Expression::Ast_Binary_Expression() {
    type = AST_BINARY_EXPRESSION;
}


struct Ast_Procedure_Call_Expression : public Ast_Expression {  
    Ast_Procedure_Call_Expression();
    Ast_Expression *procedure_expression;
    Ast_Expression *owner_expression;  // NULL if this is not a method call...
    List argument_list;  // Each of these is an Expression
};


inline Ast_Procedure_Call_Expression::Ast_Procedure_Call_Expression() {
    type = AST_PROCEDURE_CALL_EXPRESSION;
    owner_expression = NULL;
}


struct Ast_Statement : public Ast {  
    Ast_Statement();

    Ast_Expression *expression;
    int starting_line_number;
};


inline Ast_Statement::Ast_Statement() {
    type = AST_STATEMENT;
}


struct Ast_Proc_Declaration : public Ast {  
    Ast_Proc_Declaration();

    Atom *proc_name;
    int operator_number;  // -1 if this is not an operator

    Ast_Block *body;
    List argument_list;  // Each of these is an Atom... redundant info is also present in body->scope (though that one has local variable declarations as well)
};


inline Ast_Proc_Declaration::Ast_Proc_Declaration() {
    type = AST_PROC_DECLARATION;
}


struct Ast_Implicit_Variable : public Ast {  
    Ast_Implicit_Variable();
    int value;
};


inline Ast_Implicit_Variable::Ast_Implicit_Variable() {
    type = AST_IMPLICIT_VARIABLE;
}


struct Ast_While : public Ast {
    Ast_While();
    Ast_Expression *conditional;
    Ast_Block *block;
};

inline Ast_While::Ast_While() {
    type = AST_WHILE;
}

struct Ast_If : public Ast {
    Ast_If();
    Ast_Expression *conditional;
    Ast_Block *then_block;
    Ast_Block *else_block;
};

inline Ast_If::Ast_If() {
    type = AST_IF;
}

struct Ast_Break : public Ast {
    Ast_Break();
};

inline Ast_Break::Ast_Break() {
    type = AST_BREAK;
}


enum Each_Flags {
    EACH_BINOP = 0x1
};

struct Ast_Each : public Ast {
    Ast_Each();

    Ast_Expression *list_expression;
    Ast *body;  // Could be a block, or an expression.
    
    unsigned long flags;
    unsigned long operator_token_type;
    Ast_Expression *starter;
};

inline Ast_Each::Ast_Each() {
    type = AST_EACH_HARDENED;
    flags = 0;
}



struct Ast_Type_Instantiation : public Ast {
    Ast_Type_Instantiation();

    Atom *type_name;
    Atom *variable_name;
    Ast_Expression *initializer;
    Ast_Type_Instantiation *next;
};

inline Ast_Type_Instantiation::Ast_Type_Instantiation() {
    type = AST_TYPE_INSTANTIATION;
    initializer = NULL;
}


struct Ast_Scope_Record {
    Atom *atom;
    int register_number;
};

struct Ast_Lexical_Scope : public Ast {
    Ast_Lexical_Scope();

    Ast_Lexical_Scope *parent;
    List variables;  // This is a list of Ast_Scope_Record
};

inline Ast_Lexical_Scope::Ast_Lexical_Scope() {
    type = AST_LEXICAL_SCOPE;
    parent = NULL;
}

enum {
    EXPR_FLAGS_HAS_SCALAR_RESULT = DECL_EXPR_LAST_FLAG * 2
};

struct Ast_Declarative_Expression : public Ast_Declarative {
    Ast_Declarative_Expression();
    Ast_Declarative_Expression *next;  // Must match Ast_Declarative_Expression_Procedural

    List arguments;

    unsigned short flags;
    unsigned short scalar_result_index;
};

inline Ast_Declarative_Expression::Ast_Declarative_Expression() {
    type = AST_DECLARATIVE_EXPRESSION;
    flags = 0;
    scalar_result_index = 0;
    next = NULL;
}

struct Ast_Declarative_Expression_Procedural : public Ast_Declarative {
    Ast_Declarative_Expression_Procedural();
    Ast_Declarative_Expression *next;   // Must be in same place as on Ast_Declarative_Expression!
    Ast_Expression *imperative_expression;
};

inline Ast_Declarative_Expression_Procedural::Ast_Declarative_Expression_Procedural() {
    type = AST_DECLARATIVE_EXPRESSION_PROCEDURAL;
    next = NULL;
}

struct Ast_Declarative_Assertion : public Ast_Declarative {
    Ast_Declarative_Assertion();
    Ast_Declarative_Expression *expression;
    Ast_Declarative_Expression *conditionals;

    int num_variables;
    Variable **variables;
    unsigned short flags;
    unsigned short scalar_result_index;
};

inline Ast_Declarative_Assertion::Ast_Declarative_Assertion() {
    type = AST_DECLARATIVE_ASSERTION;
    expression = NULL;
    conditionals = NULL;
    flags = 0;
    scalar_result_index = 0;
}

struct Ast_Array_Subscript : public Ast_Expression {
    Ast_Array_Subscript();
    Ast_Expression *left;
    Ast_Declarative_Assertion *right;
};

inline Ast_Array_Subscript::Ast_Array_Subscript() {
    type = AST_ARRAY_SUBSCRIPT;
}

// AST_NEW will be redefined to use a memory pool at some point...
// Uses local variable 'interp' which must be declared.
#define AST_NEW(type) new (GET_MEMORY((interp), type)) type()

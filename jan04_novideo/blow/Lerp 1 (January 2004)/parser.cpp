#include "general.h"
#include "parser.h"

#include "lexer.h"
#include "hash_table.h"

#include "parser_private.h"
#include "interp.h"
#include "tree_changer.h"
#include "unicode.h"

void break_here() {
}

Atom *Parser::make_atom(char *name) {
    Atom *result = NULL;
    atom_hash->find(name, (void **)&result);
    if (result) return result;

//    result = GC_NEW(Atom);  XXXXXXXX @Memory @GC
    result = new Atom();
    result->type = ARG_ATOM;
    result->name = copy_string(name);
    result->scratch_marker = 0;
    atom_hash->add(name, result);

    return result;
}

Integer *Parser::make_integer_really(int value) {
    // @Memory: Don't make a separate integer every fricking time!
    Integer *result;

    result = GC_NEW(Integer);
    result->type = ARG_INTEGER;
    result->value = value;

    return result;
}

Integer *Parser::make_integer(int value) {
/*
    if ((value >= PRE_MADE_INTEGER_MIN) && (value <= PRE_MADE_INTEGER_MAX)) {
        int slot = value - PRE_MADE_INTEGER_MIN;
        return pre_made_integers[slot];
    }
*/
    return make_integer_really(value);
}

Ast *Parser::make_implicit_variable(int value) {
    // @Memory: Don't make a separate var every fricking time!
    Ast_Implicit_Variable *result;
    result = GC_NEW(Ast_Implicit_Variable);
    result->value = value;

    return result;
}

Float *Parser::make_float_really(double value) {
    // @Memory: Don't make a separate float every fricking time!
    Float *result;
    result = GC_NEW(Float);
    result->type = ARG_FLOAT;
    result->value = value;

    return result;
}

Float *Parser::make_float(double value) {
/*
    if (value == 0.0) return float_zero;
    if (value == 1.0) return float_one;
*/
    return make_float_really(value);
}

String *Parser::make_string(Text_Utf8 *value) {
    // @Memory: Don't make a separate float every fricking time!

    int value_size = Unicode::size_in_bytes(value);
    int size = sizeof(String) - 1 * sizeof(Text_Utf8) + value_size;

    void *mem = interp->memory_manager->allocate(size, TYPE_ENUM_String);

    String *result;
    result = new (mem) String();
    result->type = ARG_STRING;
    memcpy(result->value, value, value_size);

    return result;
}

int Parser::get_operator_precedence(int operator_type) {
    switch (operator_type) {
    case TOKEN_PLUSEQUALS:
    case TOKEN_MINUSEQUALS:
    case TOKEN_TIMESEQUALS:
    case TOKEN_DIVEQUALS:
    case TOKEN_ISEQUAL:
    case TOKEN_ISNOTEQUAL:
    case TOKEN_LESSEQUALS:
    case TOKEN_GREATEREQUALS:
    case TOKEN_DOTPLUS:
    case TOKEN_DOTMINUS:
        return -1;
    case '*':
    case '/':
        return 2;
    case '.':
        return 3;
    default:
        return 0;
    }
}

Procedure *Parser::get_primitive_function(char *name) {
    Procedure *result = NULL;
    primitive_function_hash->find(name, (void **)&result);
    return result;
}

Procedure *Parser::make_procedure(Atom *name) {
    Procedure *result;
    result = GC_NEW(Procedure);
    result->type = ARG_IMPERATIVE_PROCEDURE;
    result->name = name;
    result->bytecode = NULL;
    primitive_function_hash->add(name->name, result);

    return result;
}

Procedure *Parser::make_procedure(char *name) {
    return make_procedure(make_atom(name));
}

Variable *Parser::make_variable(char *name) {
    Variable *result = NULL;
    variable_hash->find(name, (void **)&result);
    if (result) return result;

//    result = GC_NEW(Variable);  // @Memory @GC XXXXXXXXXX
    result = new Variable();
    result->type = ARG_VARIABLE;
    result->name = copy_string(name);
    result->scratch_marker = 0;
    variable_hash->add(name, result);
    return result;
}

Parser::Parser(Lerp_Interp *_interp) {
    interp = _interp;
    lexer = new Lexer(interp);
    tree_changer = new Tree_Changer(interp);

    atom_hash = new String_Hash_Table(300);
    variable_hash = new String_Hash_Table(300);
    primitive_function_hash = new String_Hash_Table(20);

    atom_domain = make_atom("domain");

/*
    float_zero = make_float_really(0.0);
    float_one = make_float_really(1.0);
    float_zero->gc_color = GC_COLOR_RED;
    float_one->gc_color = GC_COLOR_RED;

    int i;
    for (i = PRE_MADE_INTEGER_MIN; i <= PRE_MADE_INTEGER_MAX; i++) {
        int slot = i - PRE_MADE_INTEGER_MIN;

        pre_made_integers[slot] = make_integer_really(i);
        pre_made_integers[slot]->gc_color = GC_COLOR_RED;
    }
*/
}

Parser::~Parser() {
    delete lexer;

    delete atom_hash;
    delete variable_hash;
    delete primitive_function_hash;

    // XXX @Leak: We have leaked the contents of the hash tables.
}

void Parser::set_input_from_string(char *input) {
    lexer->set_input_from_string(input);
}

void Parser::set_input_from_file(void *file) {
    lexer->set_input_from_file(file);
}

bool Parser::expect_and_eat(int token_type) {
    Token *token = lexer->peek_next_token();
    if (token->type != token_type) {
        interp->report_error(token, "'%c' expected.\n", token_type);
        break_here();
        return false;
    } else {
        lexer->eat_token();
        return true;
    }
}


Ast *Parser::parse_toplevel() {
    Token *token = lexer->peek_next_token();

    while (token->type == ';') {   // Eat spare semicolons; we don't care about them!
        lexer->eat_token();
        token = lexer->peek_next_token();
    }

    if (token->type == '[') {
        return parse_declarative_toplevel();
    } else {
        return parse_imperative_toplevel();
    }
}



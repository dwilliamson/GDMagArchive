#include "general.h"
#include "parser.h"

#include "lexer.h"
#include "hash_table.h"

Atom *Parser::make_atom(char *name) {
    Atom *result = NULL;
    atom_hash->find(name, (void **)&result);
    if (result) return result;

    result = new Atom();
    result->type = ARG_ATOM;
    result->name = copy_string(name);
    atom_hash->add(name, result);

    return result;
}

Primitive_Function *Parser::get_primitive_function(char *name) {
    Primitive_Function *result = NULL;
    primitive_function_hash->find(name, (void **)&result);
    return result;
}

Primitive_Function *Parser::make_primitive_function(char *name) {
    Primitive_Function *result;
    result = new Primitive_Function();
    result->type = ARG_PRIMITIVE_FUNCTION;
    result->name = copy_string(name);
    primitive_function_hash->add(name, result);

    return result;
}

Variable *Parser::make_variable(char *name) {
    Variable *result = NULL;
    variable_hash->find(name, (void **)&result);
    if (result) return result;
    
    result = new Variable();
    result->type = ARG_VARIABLE;
    result->name = copy_string(name);
    variable_hash->add(name, result);
    return result;
}

Parser::Parser() {
    lexer = new Lexer();

    atom_hash = new String_Hash_Table(300);
    variable_hash = new String_Hash_Table(300);
    primitive_function_hash = new String_Hash_Table(20);

    Primitive_Function *fun;
    fun = make_primitive_function("notequal");
    parse_error = false;
}

Parser::~Parser() {
    delete lexer;

    delete atom_hash;
    delete variable_hash;
    delete primitive_function_hash;

    // @Leak: We have leaked the contents of the hash tables.
}

void Parser::set_input_from_string(char *input) {
    lexer->set_input_from_string(input);
}

void Parser::set_input_from_file(void *file) {
    lexer->set_input_from_file(file);
}

Decl_Assertion *Parser::parse_assertion() {
    parse_error = false;

    Token *token = lexer->peek_next_token();
    if (token->type != '(') return NULL;

    Decl_Expression *expression = parse_expression();
    if (expression == NULL) return NULL;

    Decl_Assertion *assertion = new Decl_Assertion();
    assertion->expression = expression;
    
    token = lexer->peek_next_token();
    if (token->type == ';') {
        lexer->eat_token();
        assertion->conditionals = NULL;
    } else if (token->type == TOKEN_IMPLIED_BY) {
        lexer->eat_token();
        assertion->conditionals = parse_expression_sequence();
        if (assertion->conditionals == NULL) {
            fprintf(stderr, "Expected expression sequence after '->'.\n");
            parse_error = true;
            delete assertion;    // @Leak expression is leaked if assertion destructor doesn't delete it
            return NULL;
        }
    } else {
        delete assertion;
        assertion = NULL;
    }

    return assertion;
}

Decl_Expression_Sequence *Parser::parse_expression_sequence() {
    parse_error = false;

    Token *token = lexer->peek_next_token();
    if (token->type != '(') return NULL;

    Decl_Expression_Sequence *sequence = new Decl_Expression_Sequence;
    while (1) {
        Decl_Expression *expression = parse_expression();
        if (!expression) break;

        sequence->expressions.add(expression);

        Token *token = lexer->peek_next_token();
        if (token->type == ',') {
            lexer->eat_token();
            continue;
        }

        if (token->type != '(') {
            break;
        }
    }

    token = lexer->peek_next_token();
    if (token->type == ';') {
        lexer->eat_token();
    } else if (token->type == TOKEN_END_OF_INPUT) {
    } else {
        fprintf(stderr, "Parse error: Expression sequence should end with ';'\n");
        parse_error = true;
    }

    if (parse_error) {
        printf("Parsing failed due to an error.\n");
        return NULL;
    }

    return sequence;
}

Decl_Expression *Parser::parse_expression() {
    Token *token = lexer->peek_next_token();
    assert(token->type == '(');  // Else we should not have been called!

    lexer->eat_token();

    Decl_Expression *result = Parser::parse_expression_interior();

    if (!result) {
        fprintf(stderr, "Parse error of some kind!!!!\n");
        parse_error = true;
    }

    token = lexer->peek_next_token();
    if (token->type != ')') {
        fprintf(stderr, "Expected to see a ')'.\n");
        parse_error = true;
        delete result;
        result = NULL;
    }

    lexer->eat_token();

    return result;
}

Decl_Expression *Parser::parse_expression_interior() {
    Decl_Expression *expression = new Decl_Expression();

    bool done = false;
    while (!done) {
        Token *token = lexer->peek_next_token();
        if (token == NULL) break;
/*
        if (token->type == TOKEN_IDENT) printf("Got one IDENT %s\n", token->name);
        if (token->type == TOKEN_VARIABLE) printf("Got one VAR %s\n", token->name);
*/
        switch (token->type) {
        case TOKEN_IDENT: {
            First_Class *argument = get_primitive_function(token->name);
            if (!argument) argument = make_atom(token->name);
            expression->arguments.add(argument);
            lexer->eat_token();
            break;
        }
        case TOKEN_VARIABLE:
            expression->arguments.add(make_variable(token->name));
            lexer->eat_token();
            break;
        default:
            done = true;
            break;
        }
    }

    return expression;
}

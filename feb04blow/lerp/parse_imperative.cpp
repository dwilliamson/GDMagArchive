#include "general.h"
#include "parser.h"

#include "lexer.h"

#include "parser_private.h"
#include "interp.h"
#include "schema.h"
#include "tree_changer.h"

extern void break_here();

void get_binop_from_token(int operator_token_type, int *operator_type_return, bool *assignment_return);
void get_unop_from_token(int operator_token_type, int *operator_type_return);


bool Parser::is_unary_operator(Token *token) {
    switch (token->type) {
    case '-':
    case '+':
    case '!':
        return true;
    default:
        return false;
    }
}

bool Parser::is_binary_operator(Token *token) {
    switch (token->type) {
    case '-':
    case '+':
    case '*':
    case '/':
    case TOKEN_PLUSEQUALS:
    case TOKEN_MINUSEQUALS:
    case TOKEN_TIMESEQUALS:
    case TOKEN_DIVEQUALS:
    case '.':
    case '=':
    case TOKEN_ISEQUAL:
    case TOKEN_ISNOTEQUAL:
    case '<':
    case '>':
    case TOKEN_LESSEQUALS:
    case TOKEN_GREATEREQUALS:
    case TOKEN_DOTPLUS:
    case TOKEN_DOTMINUS:
    case TOKEN_DOUBLE_DOT:
        return true;
    default:
        return false;
    }
}

Ast_Block *Parser::parse_scopey_block(Atom *starting_name) {
    List scopes;
    if (starting_name) scopes.add(starting_name);

    while (1) {
        Token *token = lexer->peek_next_token();
        if (token->type != TOKEN_IDENT) break;
        Atom *name = token->name;

        lexer->eat_token();
        token = lexer->peek_next_token();

        if (token->type != ':') {
            interp->report_error(token, "Expected to see a : for continued scope specifiers.\n");
            if (token->type == '{') break;  // Try to parse the block anyway.
            return NULL;
        }

        lexer->eat_token();
        scopes.add(name);  // We got another scope!
    }

    Token *token = lexer->peek_next_token();
    if (token->type != '{') {
        interp->report_error(token, "Error: expected to see a '{' to open a new block.\n");
        return NULL;
    }

    Ast_Block *block = parse_imperative_block();
    if (block == NULL) return NULL;

    block->scopes = scopes;

    scopes.head = scopes.tail = NULL;
    scopes.items = 0;

    return block;
}

Ast_Expression *Parser::parse_imperative_subexpression() {
    Token *token = lexer->peek_next_token();
    switch (token->type) {
    case TOKEN_IDENT: {
        First_Class *result = token->name;
        lexer->eat_token();

        token = lexer->peek_next_token();
        if (token->type == ':') {
            lexer->eat_token();
            return (Ast_Expression *)parse_scopey_block((Atom *)result);
        }

        return (Ast_Expression *)result;
    }
    case TOKEN_INTEGER: {
        First_Class *result = make_integer(token->integer_value);
        lexer->eat_token();
        return (Ast_Expression *)result;
    }
    case TOKEN_FLOAT: {
        First_Class *result = make_float(token->float_value);
        lexer->eat_token();
        return (Ast_Expression *)result;
    }
    case TOKEN_STRING: {
        First_Class *result = make_string(token->string_value);
        lexer->eat_token();
        return (Ast_Expression *)result;
    }
    case TOKEN_IMPERATIVE_IMPLICIT_VARIABLE: {
        Ast *result = make_implicit_variable(token->integer_value);
        lexer->eat_token();
        return (Ast_Expression *)result;
    }
    }

    return NULL;
}

Ast_Expression *Parser::parse_embedded_each() {
    Token *token = lexer->peek_next_token();
    lexer->eat_token();

    Ast_Expression *list = parse_imperative_expression();
    if (list == NULL) return NULL;

    Ast_Expression *body = parse_imperative_expression();
    // 'body' is allowed to be NULL in this case.
    // If it is, we just fall through and assign it to each->body.

    Ast_Each *each = AST_NEW(Ast_Each);
    each->type = AST_EACH_PLACEHOLDER;
    each->body = body;
    each->list_expression = list;

    return (Ast_Expression *)each;
}

static bool is_obvious_procedure_type(Ast_Expression *expression) {
    while (expression->type == AST_BINARY_EXPRESSION) {
        Ast_Binary_Expression *binexp = (Ast_Binary_Expression *)expression;
        if (binexp->operator_token_type != '.') break;
        expression = binexp->right;
    }

    if (expression->type == ARG_ATOM) return true;
    return false;
}

Ast_Expression *Parser::parse_imperative_expression_helper() {
    Token *token = lexer->peek_next_token();

    Ast_Expression *left;

    if (token->type == '(') {
        lexer->eat_token();
        left = parse_imperative_expression();
        expect_and_eat(')');
        // Tag this expression as parenthesized so that we don't try to
        // re-associate its terms...
        left->flags |= AST_PARENTHESIZED;
    } else if (is_unary_operator(token)) {
        Ast_Unary_Expression *unary = AST_NEW(Ast_Unary_Expression);
        unary->type = AST_UNARY_EXPRESSION;
        unary->operator_token_type = token->type;
        lexer->eat_token();
        unary->subexpression = parse_imperative_expression();

        left = unary;
    } else if (token->type == '[') {
        Ast_Declarative_Assertion *expression = parse_declarative_query();
        left = (Ast_Expression *)expression;  // @Refactor: Bogus cast
    } else if (token->type == TOKEN_KEYWORD_EACH) {
        left = parse_embedded_each();
    } else {
        left = parse_imperative_subexpression();
    }
    
    if (left == NULL) return NULL;
    if (left->type == AST_BLOCK) return left;

    token = lexer->peek_next_token();

    //
    // @Design: We call 'is_obvious_procedure_type' below in order to prevent
    // a parsing ambiguity from biting us too hard.  We will say that a '(' to the
    // right of an expression designates a procedure call when the thing right
    // before it involves an identifier lookup, but that it denotes a new expression
    // otherwise.  This will cause us problems later when we want to call arbitrary
    // expressions that evaluate to procedures... maybe some other kind of syntax
    // will be necessary for that case.
    if ((token->type == '(') && (is_obvious_procedure_type(left))) {
        Ast_Procedure_Call_Expression *result = AST_NEW(Ast_Procedure_Call_Expression);
        result->procedure_expression = left;
        parse_imperative_procedure_call_arguments(&result->argument_list);

        token = lexer->peek_next_token();
        left = result;
    }

    if (token->type == '[') {
        // A bracket at this point indicates the "array subscript" syntactic sugar.
        Ast_Binary_Expression *result = AST_NEW(Ast_Binary_Expression);
        result->operator_token_type = TOKEN_OPERATOR_ARRAY_SUBSCRIPT;
        result->left = left;
        Ast_Declarative_Assertion *dexp = parse_declarative_query();
        if (dexp == NULL) return NULL;

        result->right = reinterpret_cast <Ast_Expression *>(dexp);

        token = lexer->peek_next_token();

        if (dexp->expression) {
            interp->report_error(token, "Cannot use '->' in conjunction with array subscript!\n");
        }

        if ((!dexp->conditionals) || (dexp->conditionals->next)) {
            interp->report_error(token, "Weird error in array subscript.  Get it right next time!\n");
        }

        left = result;
    }

    if (is_binary_operator(token)) {
        int binary_token_type = token->type;
        Token copy = *token;

        lexer->eat_token();
        Ast_Expression *right = parse_imperative_expression();
        if (right == NULL) {
            interp->report_error(&copy, "Unable to see rhs of binop.\n");
            return left;
        }

        if (binary_token_type == '.') {
            if (right->type == AST_PROCEDURE_CALL_EXPRESSION) {
                // If we have a dot and then a procedure call, for example: a.b(x);
                // that indicates a method call.  Special stuff must be done during
                // code generation, so instead of leaving the dot and procedure call
                // as separate AST nodes, we're gonna combine them to make things
                // easier to deal with later on.

                Ast_Procedure_Call_Expression *call = (Ast_Procedure_Call_Expression *)right;
                call->owner_expression = left;
                return call;
            }
        }

        Ast_Binary_Expression *result = AST_NEW(Ast_Binary_Expression);
        result->operator_token_type = binary_token_type;
        result->left = left;
        result->right = right;

        //
        // Now fiddle with things based on the precedences of things...
        //
        if (right->type == AST_BINARY_EXPRESSION) {
            Ast_Binary_Expression *bin_r = static_cast <Ast_Binary_Expression *>(right);
            if (!(bin_r->flags & AST_PARENTHESIZED)) {
                int my_precedence = get_operator_precedence(result->operator_token_type);
                int his_precedence = get_operator_precedence(bin_r->operator_token_type);
                if (my_precedence > his_precedence) {
                    // Fiddle around with the terms!
                    result->right = bin_r->left;
                    bin_r->left = result;
                    return bin_r;
                }
            }
        }

        return result;
    }

    return left;
/*
    interp->report_error(token, "Unexpected token '%c'...\n", token->type);  // XXX Make a real error.

    return NULL;
*/
}

Ast_Expression *Parser::parse_imperative_expression() {
    // This function just calls parse_imperative_expression_helper() to do
    // the parsing.  But upon return, we always pass the expression through
    // the postprocessor... that way we don't have to do the bug-prone
    // design of trying to do the postprocess at every return point
    // of the parsing routine.

    Ast_Expression *exp = parse_imperative_expression_helper();
    if (!exp) return NULL;

    return tree_changer->postprocess(exp);
}

void Parser::parse_separated_expression_list(int separator_type, List *results) {
    Token *token = lexer->peek_next_token();
    while (1) {
        Ast_Expression *expression = parse_imperative_expression();
        if (!expression) break;

        results->add(expression);

        token = lexer->peek_next_token();
        if (token->type != separator_type) break;

        lexer->eat_token();
    }
}


void Parser::parse_imperative_procedure_call_arguments(List *results) {
    expect_and_eat('(');
    parse_separated_expression_list(',', results);
    expect_and_eat(')');
}

Ast_Type_Instantiation *Parser::parse_type_instantiation_list() {
    Ast_Type_Instantiation *prev = NULL;


    Token *token = lexer->peek_next_token();
    assert(token->type = TOKEN_IDENT);
    Atom *type_name = token->name;
    lexer->eat_token();

    while (1) {
        token = lexer->peek_next_token();
        if (token->type != TOKEN_IDENT) {
            interp->report_error(token, "Ident expected in type declaration.\n");
            return NULL;
        }

        Atom *variable_name = token->name;
        lexer->eat_token();

        Ast_Type_Instantiation *inst = AST_NEW(Ast_Type_Instantiation);
        inst->type_name = type_name;
        inst->variable_name = variable_name;
        inst->next = prev;
        prev = inst;

        token = lexer->peek_next_token();
        if (token->type == '=') {
            // There's an initial assignment...
            lexer->eat_token();

            Ast_Expression *exp = parse_imperative_expression();
            inst->initializer = exp;

            token = lexer->peek_next_token();
        }

        if (token->type == ';') break;
        if (token->type == ',') {
            lexer->eat_token();
        }  // Otherwise it will case an error in next loop-through.
    }

    return prev;
}

Ast_Statement *Parser::parse_imperative_statement(bool eat_semicolon) {
    Token *token = lexer->peek_next_token();
    int starting_line_number = token->line_number;

    Ast_Expression *expression;
    if (token->type == TOKEN_KEYWORD_RETURN) {
        lexer->eat_token();

        Ast_Expression *subexpression = parse_imperative_expression();
        if (subexpression == NULL) {
            token = lexer->peek_next_token();
            // Circumstances under which we could just have a bald 'return'
            // (to return void)...
            if ((token->type == ';') || (token->type == TOKEN_KEYWORD_ELSE)) {
                subexpression = (Ast_Expression *)interp->uninitialized;
            } else {
                // This must be a parse error!
                interp->report_error(token, "Syntax error in 'return'.\n");
                return NULL;
            }
        }

        // 
        // RETURN is stored in a Unary_Expression for now because I didn't want
        // to make a new class.  If this kind of thing becomes widespread I will
        // make a more generalized way of dealing with this stuff.
        //
        Ast_Unary_Expression *unary = AST_NEW(Ast_Unary_Expression);
        unary->type = AST_UNARY_EXPRESSION;
        unary->operator_token_type = TOKEN_KEYWORD_RETURN;
        unary->subexpression = subexpression;

        expression = unary;
    } else if (token->type == TOKEN_KEYWORD_IF) {
        lexer->eat_token();
        Ast_Expression *conditional = parse_imperative_expression();
        if (conditional == NULL) return NULL;

        // For now, 'then' is optional... we'll see how that works.
        Token *token = lexer->peek_next_token();
        if (token->type == TOKEN_KEYWORD_THEN) lexer->eat_token();

        Ast_Block *then_block = parse_imperative_block_or_statement();
        if (then_block == NULL) return NULL;

        token = lexer->peek_next_token();
        while (token->type == ';') {
            lexer->eat_token();
            token = lexer->peek_next_token();
        }

        Ast_Block *else_block = NULL;
        if (token->type == TOKEN_KEYWORD_ELSE) {
            lexer->eat_token();
            else_block = parse_imperative_block_or_statement();
        }

        Ast_If *ast_if = AST_NEW(Ast_If);

        ast_if->conditional = conditional;
        ast_if->then_block = then_block;
        ast_if->else_block = else_block;

        expression = (Ast_Expression *)ast_if;  // XXX Cast not strictly true, probably we just want this function to return AST
    } else if (token->type == TOKEN_KEYWORD_WHILE) {
        lexer->eat_token();

        Ast_Expression *conditional = parse_imperative_expression();
        if (conditional == NULL) return NULL;
        Ast_Block *block = parse_imperative_block_or_statement();
        if (block == NULL) return NULL;

        Ast_While *ast_while = AST_NEW(Ast_While);
        ast_while->conditional = conditional;
        ast_while->block = block;
        
        expression = (Ast_Expression *)ast_while;  // XXX See note on AST_IF
    } else if (token->type == TOKEN_KEYWORD_BREAK) {
        lexer->eat_token();

        Ast_Break *ast_break = AST_NEW(Ast_Break);

        expression = (Ast_Expression *)ast_break;  // XXX See note on ast_if
    } else if (token->type == TOKEN_KEYWORD_EACH) {
        lexer->eat_token();
        Ast_Expression *list = parse_imperative_expression();
        if (list == NULL) return NULL;
        
        Ast_Block *block = parse_imperative_block_or_statement();
        if (block == NULL) return NULL;

        Ast_Each *each = AST_NEW(Ast_Each);
        each->type = AST_EACH_HARDENED;
        each->list_expression = list;
        each->body = block;

        expression = (Ast_Expression *)each;
    } else if (token->type == TOKEN_IDENT) {
        // We look one extra token ahead... if it is also an IDENT, then we
        // treat this as a type instantiation...
        Token *token2 = lexer->peek_token(1);
        if (token2->type == TOKEN_IDENT) {
            expression = (Ast_Expression *)parse_type_instantiation_list();
        } else {
            // Fall back to default... it's an expression...
            expression = parse_imperative_expression();
        }
    } else if (token->type == '{') {
        expression = (Ast_Expression *)parse_imperative_block();
    } else {
        expression = parse_imperative_expression();
    }

    if (expression == NULL) return NULL;

    Ast_Statement *statement = AST_NEW(Ast_Statement);
    statement->expression = expression;
    statement->starting_line_number = starting_line_number;

// @Refactor: Is eat_semicolon deprecated... we just chew it up in the outer loop?
//    if (eat_semicolon) expect_and_eat(';');
    return statement;
}

void Parser::parse_imperative_proc_declaration_arguments(List *results) {
    Token *token = lexer->peek_next_token();
    if (token->type != '(') return;  // No arguments!
    lexer->eat_token();

    while (1) {
        Token *token = lexer->peek_next_token();

        if (token->type == TOKEN_IDENT) {  // This is either a type name or a variable name, let's see...
            Atom *type_name = NULL;
            Atom *variable_name = NULL;

            Atom *first_ident = token->name;
            lexer->eat_token();

            token = lexer->peek_next_token();
            if (token->type == TOKEN_IDENT) {
                type_name = first_ident;
                variable_name = token->name;
                lexer->eat_token();
            } else {
                variable_name = first_ident;
            }

            //
            // For now we just ditch the type info and add the variable name...
            bool added = results->add_unique(variable_name);  // @Speed: Could use the atom scratch_marker instead to speed this up

            if (!added) {
                interp->report_error(token, "Identifier '%s' used more than once in argument list.  Redundant declarations ignored.\n", variable_name->name);
            }

            results->add(type_name);  // May be NULL.

            token = lexer->peek_next_token();
            if (token->type == ',') {
                lexer->eat_token();
                continue;
            } else {
                break;
            }
        } else {
            // We got something we did not want... break out and bitch if it's not ')'.
            break;
        }
    }

    token = lexer->peek_next_token();

    if (token->type == ')') {
        lexer->eat_token();
        return;
    }

    interp->report_error(token, "Unexpected token found in proc declaration.\n");  // XXX make a real error
    break_here();
}

Ast_Proc_Declaration *Parser::parse_imperative_proc_declaration() {
    expect_and_eat(TOKEN_KEYWORD_PROC);

    Ast_Proc_Declaration *proc = AST_NEW(Ast_Proc_Declaration);
    proc->operator_number = -1;

    Token *token = lexer->peek_next_token();
    Token save_me;
    bool is_operator = false;

    if (token->type == TOKEN_IDENT) {
        proc->proc_name = token->name;
        lexer->eat_token();
    } else if (is_binary_operator(token) || is_unary_operator(token)) {
        proc->proc_name = interp->parser->make_atom("_operator");
        save_me = *token;
        is_operator = true;
        lexer->eat_token();
    } else {
        interp->report_error(token, "Procedure name expected.\n");  // XXX
        return NULL;
    }


    parse_imperative_proc_declaration_arguments(&proc->argument_list);

    proc->body = parse_scopey_block(NULL);

    if (!is_operator) return proc;

    //
    // If it was an operator procedure, do the right thing...
    // we need to discern whether it was unary or binary so that
    // we can give it the right operator ID which is currently
    // numeric (probably a more elegant way of doing this could be devised!)
    //

    int num_arguments = proc->argument_list.items / 2;
    assert(num_arguments * 2 == proc->argument_list.items);  // Verify it's not an odd number!

    if (num_arguments == 2) {
        if (!is_binary_operator(&save_me)) {
            interp->report_error("Attempt to declare a 2-argument proc for an operator that isn't binary.\n");  // @UI: Tell them which operator.
            return NULL;
        }

        int binop_operator;
        bool binop_assignment;
        get_binop_from_token(save_me.type, &binop_operator, &binop_assignment);

        proc->operator_number = binop_operator;
        if (binop_assignment) {
            interp->report_error("Definition of operators containing '=' is not currently supported.  Just work with the version of the operator without '='.\n");
            return NULL;
        }       
    } else if (num_arguments == 1) {
        if (!is_unary_operator(&save_me)) {
            interp->report_error("Attempt to declare a 1-argument proc for an operator that isn't unary.\n");  // @UI: Tell them which operator.
            return NULL;
        }

        int unop_operator;
        get_unop_from_token(save_me.type, &unop_operator);
        proc->operator_number = unop_operator;
    }

    return proc;
}

Ast *Parser::parse_imperative_toplevel() {
    Token *token = lexer->peek_next_token();

    if (token->type == TOKEN_KEYWORD_PROC) {
        return parse_imperative_proc_declaration();
    } else if (token->type == TOKEN_KEYWORD_STRUCT) {
        Schema *decl = parse_struct_declaration();
        return (Ast *)decl;  // XXX Cast not strictly true
    }

    if (token->type == TOKEN_END_OF_INPUT) return NULL;

    interp->report_error(token, "Unexpected thingy at toplevel.\n");
    break_here();

    return NULL;
}

Ast_Block *Parser::parse_imperative_block_or_statement() {
    Token *token = lexer->peek_next_token();
    if (token->type == '{') return parse_imperative_block();

    Ast_Statement *statement = parse_imperative_statement(false);
    if (!statement) {
        // XXX Make a real error.  @Incomplete (Or maybe parse_imperative_statement always gives an error?)
        return NULL;
    }

    //
    // Make a block containing only this one statement.
    //

    Ast_Block *result = AST_NEW(Ast_Block);
    result->statements.add(statement);

    return result;
}

Ast_Block *Parser::parse_imperative_block() {
    Token *token = lexer->peek_next_token();
    if (token->type != '{') return NULL;

    lexer->eat_token();

    Ast_Block *result = AST_NEW(Ast_Block);

    while (1) {
        Token *token = lexer->peek_next_token();

        while (token->type == ';') {   // Eat spare semicolons; we don't care about them!
            lexer->eat_token();
            token = lexer->peek_next_token();
        }

        if (token->type == '}') {
            lexer->eat_token();
            break;
        }

        if (token->type == TOKEN_END_OF_INPUT) {
            interp->report_error(token, "Unexpected end of input in code block.\n");
            return NULL;
        }

        if (token->type == TOKEN_KEYWORD_ELSE) {
            // Help 'em out if they goofed...
            interp->report_error(token, "'else' without 'if'.\n");
            return NULL;
        }

        Ast_Statement *statement = parse_imperative_statement();
        if (statement == NULL) {
            interp->report_error(token, "Unexpected input.\n");
            return NULL;
        }

        result->statements.add(statement);
    }

    return result;
}

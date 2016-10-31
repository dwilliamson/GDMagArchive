#include "general.h"
#include "parser.h"

#include "lexer.h"

#include "parser_private.h"
#include "interp.h"

#include "schema.h"

#define CHECK_FOR_WILDCARD() if (expression->flags & DECL_EXPR_FLAGS_WILDCARDED) interp->report_error(token, "It's unsupported to put anything in a declarative expression, after a wildcard marker (?*).\n");

void add_expression(Ast_Declarative_Assertion *assertion, Ast_Declarative_Expression *expression) {
    Ast_Declarative_Expression **cursor = &assertion->conditionals;
    while (*cursor) cursor = &((*cursor)->next);
    *cursor = expression;
}


Ast_Declarative_Expression *Parser::parse_declarative_expression_interior() {
    Ast_Declarative_Expression *expression = AST_NEW(Ast_Declarative_Expression);

    bool done = false;
    while (!done) {
        Token *token = lexer->peek_next_token();

        switch (token->type) {
/*
        case TOKEN_IDENT: {
            First_Class *argument = get_primitive_function(token->name->name);
            if (!argument) argument = token->name;
            expression->arguments.add(argument);
            lexer->eat_token();
            break;
        }

        // @Memory, @Speed: When we parse a variable, first we make an atom for the
        // variable name, and now we put it in a separate variable hash... well...
        // that duplicates the memory for the name, and it's slower... we could 
        // do better.
*/
        case TOKEN_QUERY_VARIABLE:
            CHECK_FOR_WILDCARD();
            expression->arguments.add(make_variable(token->name->name));
            lexer->eat_token();
            break;
        case TOKEN_DOUBLE_QUESTION_MARK: {
            CHECK_FOR_WILDCARD();
            if (expression->flags & EXPR_FLAGS_HAS_SCALAR_RESULT) {
                interp->report_error(token, "Attempt to use ?? more than once in a query expression.\n");
            } else {
                expression->flags |= EXPR_FLAGS_HAS_SCALAR_RESULT;
                expression->scalar_result_index = expression->arguments.items;
            }

            expression->arguments.add(make_variable("?"));
            lexer->eat_token();

            break;
        }
        case TOKEN_QUERY_WILDCARD: {
            lexer->eat_token();
            if (expression->flags & DECL_EXPR_FLAGS_WILDCARDED) {
                interp->report_error(token, "More than 1 wildcard in expression -- illegal!\n");
            } else {
                expression->flags |= DECL_EXPR_FLAGS_WILDCARDED;
            }

            break;
        }
        default: {
            CHECK_FOR_WILDCARD();
            First_Class *sub = parse_imperative_expression();
            if (sub) expression->arguments.add(sub);
            else done = true;
            break;
        }
        case ']':
            done = true;
            break;
        }
    }

    return expression;
}

Ast_Declarative_Expression *Parser::parse_declarative_expression() {
    expect_and_eat('[');

    Ast_Declarative_Expression *result = parse_declarative_expression_interior();

    if (!result) {
        Token *token = lexer->peek_next_token();  // So we have some context to report.
        interp->report_error(token, "Parse error of some kind!!!!\n");
    }

    expect_and_eat(']');

    return result;
}

Ast_Declarative_Assertion *Parser::parse_declarative_assertion() {
    Token *token = lexer->peek_next_token();
    if (token->type != '[') return NULL;

    Ast_Declarative_Expression *expression = parse_declarative_expression();
    if (expression == NULL) return NULL;

    Ast_Declarative_Assertion *assertion = AST_NEW(Ast_Declarative_Assertion);
    assertion->expression = expression;
    
    token = lexer->peek_next_token();
    if (token->type == TOKEN_IMPLIED_BY) {
        lexer->eat_token();
        parse_declarative_expression_sequence(assertion);
        if (assertion->conditionals == NULL) {
            interp->report_error(token, "Expected expression sequence after '->'.\n");
        }
    }

    postprocess_declarative_assertion(assertion);
    return assertion;
}

Ast_Declarative_Assertion *Parser::parse_declarative_query() {
    Token *token = lexer->peek_next_token();
    if (token->type != '[') return NULL;

    Ast_Declarative_Assertion *assertion = AST_NEW(Ast_Declarative_Assertion);

    parse_declarative_expression_sequence(assertion);
    Ast_Declarative_Expression *left = assertion->conditionals;  // @Refactor: This is a bit of a hack.
    assertion->conditionals = NULL;

    Ast_Declarative_Expression *right = NULL;

    token = lexer->peek_next_token();
    if (token->type == TOKEN_IMPLIED_BY) {
        lexer->eat_token();
        parse_declarative_expression_sequence(assertion);
        if (assertion->conditionals == NULL) {
            interp->report_error(token, "Expected expression sequence after '->'.\n");
        }

        right = assertion->conditionals;  // @Refactor: This is a bit of a hack.
    }

    if (left && !right) {
        assertion->conditionals = left;
        assertion->expression = NULL;
    } else {
        assertion->expression = left;
        assertion->conditionals = right;
    }

    postprocess_declarative_assertion(assertion);
    return assertion;
}

void Parser::postprocess_declarative_assertion(Ast_Declarative_Assertion *assertion) {
    //
    // Postprocess the sequence.  Basically we zip through it and figure
    // out what all the variables are, making an array of them.
    //
    
    List variables;
    Ast_Declarative_Expression *expression = assertion->conditionals;
    for (; expression; expression = expression->next) {
        if (expression->type == AST_DECLARATIVE_EXPRESSION_PROCEDURAL) continue;

        First_Class *argument;
        Foreach(&expression->arguments, argument) {
            if (argument->type != ARG_VARIABLE) continue;
            Variable *variable = (Variable *)argument;

            // Check for uniqueness; we only want to add each variable once.
            if (variable->scratch_marker) continue;
            variable->scratch_marker = 1;

            variables.add(variable);
        } Endeach;
    }

    assertion->num_variables = variables.items;
    if (assertion->num_variables) {
        assertion->variables = new Variable *[assertion->num_variables]; // XXXXXXX @Memory UNMANAGED MEMORY
    } else {
        assertion->variables = NULL;
    }

    Variable *scalar_query_var = interp->parser->make_variable("?");  // @Speed: Should use precomputed pointer to this guy

    int index = 0;
    Variable *variable;
    Foreach(&variables, variable) {
        assert(variable->type == ARG_VARIABLE);

        if (variable == scalar_query_var) {
            assert(assertion->flags & ASSERTION_FLAGS_HAS_SCALAR_RESULT);
            assertion->scalar_result_index = index;
        }

        assertion->variables[index++] = variable;
    } Endeach;

    // Undo all the scratch markers...
    int i;
    for (i = 0; i < assertion->num_variables; i++) {
        assertion->variables[i]->scratch_marker = 0;
    }

    //
    // We are now done post-processing the assertion.
    //
}

void Parser::parse_declarative_expression_sequence(Ast_Declarative_Assertion *assertion) {
    while (1) {
        Token *token = lexer->peek_next_token();
        if (token->type != '[') {
            Ast_Expression *expression = parse_imperative_expression();
            if (expression == NULL) return;
            Ast_Declarative_Expression_Procedural *epro = AST_NEW(Ast_Declarative_Expression_Procedural);
            epro->imperative_expression = expression;
            add_expression(assertion, (Ast_Declarative_Expression *)epro);
        } else {
            Ast_Declarative_Expression *expression = parse_declarative_expression();
            if (!expression) return;

            if (expression->flags & EXPR_FLAGS_HAS_SCALAR_RESULT) {
                if (assertion->flags & ASSERTION_FLAGS_HAS_SCALAR_RESULT) {
                    interp->report_error(token, "Attempt to use '??' more than once in an expression sequence.\n");
                } else {
                    assertion->flags |= ASSERTION_FLAGS_HAS_SCALAR_RESULT;  // We will figure out which one it is, later, when we collect the query variables.
                }
            }

            add_expression(assertion, expression);
        }

        token = lexer->peek_next_token();
        if (token->type == '&') {
            lexer->eat_token();
            continue;
        }

        break;
    }
}

Type_Constraint *Parser::parse_db_constraint_range() {
    First_Class *low = NULL;
    First_Class *high = NULL;

    Token *token = lexer->peek_next_token();
    assert(token->type == '(');
    lexer->eat_token();

    token = lexer->peek_next_token();
    if (token->type != TOKEN_DOUBLE_DOT) {
        Ast_Expression *left = parse_imperative_subexpression();
        if (left == NULL) return NULL;

        // @Refactor: All this stuff should probably be in parse_imperative_expression
        // anyway... argh

        token = lexer->peek_next_token();
        if (token->type != TOKEN_DOUBLE_DOT) {
            interp->report_error(token, "Expected double dot!!!\n");
            return NULL;
        }

        if (left->type != ARG_INTEGER) {
            interp->report_error(token, "Left-hand side of range must be an integer constant.\n");
            return NULL;
        }

        low = left;
    }

    lexer->eat_token();  // Eat the double-dot

    token = lexer->peek_next_token();
    Ast_Expression *right = NULL;
    if (token->type != ')') {
        right = parse_imperative_subexpression();
        if (!right) return NULL;
        if (right->type != ARG_INTEGER) {
            interp->report_error(token, "Right-hand side of range must be an integer constant.\n");
            return NULL;
        }
    
        high = right;
    }

    expect_and_eat(')');

    Type_Constraint *constraint = new Type_Constraint();  // XXX @Memory unmanaged memory
    Integer_Constraint_Info *info = &constraint->info.integer_constraint;

    if (low) {
        info->min_defined = true;
        info->min = ((Integer *)low)->value;
    }

    if (high) {
        info->max_defined = true;
        info->min = ((Integer *)high)->value;
    }

    return constraint;
}

/*
bool Parser::parse_db_constraint_qualifier(Decl_Constraint *constraint, List *results) {
    expect_and_eat(':');
    Token *token = lexer->peek_next_token();
    if (token->type != TOKEN_IDENT) {
        interp->report_error(token, "Expected an identifier after ':'\n");
        return false;
    }

    Atom *qualifier_name = token->name;
    lexer->eat_token();

    if (qualifier_name == atom_domain) {
        bool success;
        success = expect_and_eat('(');
        if (!success) return false;

        List domain;
        parse_separated_expression_list(',', &domain);

        success = expect_and_eat(')');
        if (!success) return false;

        //
        // Check to make sure every item in 'domain' is an Implicit Variable,
        // that the ranges are appropriate, etc etc.
        //
        // @Refactor: If we ever have a separate "semantic checking of AST" phase
        // that is distinct from parsing, this ought to be moved there.
        //
        int constraint_length = constraint->arguments.items;

        Decl_Qualifier_Domain *result = GC_NEW(Decl_Qualifier_Domain);
        result->num_domain_items = domain.items;
        result->domain_indices = new int[constraint_length];  // XXXXXXX @Memory: Unmanaged memory.

        int index = 0;
        First_Class *arg;
        bool okay = true;
        Foreach(&domain, arg) {
            if (arg->type != AST_IMPLICIT_VARIABLE) {
                interp->report_error(token, "Argument %d of 'domain' is not an implicit variable.\n", index+1);
                okay = false;
            } else {
                Ast_Implicit_Variable *var = (Ast_Implicit_Variable *)arg;
                if ((var->value >= constraint_length) || (var->value < 1)) {
                    interp->report_error(token, "Argument %d of 'domain' is out of range.\n", index+1);
                    okay = false;
                } else {
                    result->domain_indices[index] = var->value - 1;
                }
            }
            index++;
        } Endeach;

        if (!okay) return false;

        results->add(result);
        return true;
    } else {
        interp->report_error(token, "Unknown qualifier name '%s'\n", qualifier_name->name);
        return false;
    }

    return true;
}
*/


Decl_Constraint *Parser::parse_db_constraint_interior() {
//    Decl_Constraint *expression = GC_NEW(Decl_Constraint);  // XXXX @Memory Decl_Constraint is not currently a managed type
    Decl_Constraint *expression = new Decl_Constraint();  // XXXX @Memory Decl_Constraint is not currently a managed type

    bool got_domain_marker = false;
    bool done = false;
    while (!done) {
        Token *token = lexer->peek_next_token();

        switch (token->type) {
        case TOKEN_QUERY_VARIABLE: {
            Type_Constraint *constraint = new Type_Constraint; // @Memory XXXX Unmanaged memory
            memset(&constraint->info, 0, sizeof(constraint->info));
            constraint->type_name = token->name;
            lexer->eat_token();

            expression->arguments.add(constraint);
            break;
        }
        case '(': {
            Type_Constraint *constraint = parse_db_constraint_range();
            if (constraint) {
                expression->arguments.add(constraint);
            }

            break;
        }
        case '|': {
            lexer->eat_token();
            if (got_domain_marker) {
                interp->report_error(token, "Multiple domain markers '|' in constraint... extra ignored.\n");
            } else {
                expression->num_domain_arguments = expression->arguments.items;
            }

            break;
        }
        default:
            done = true;
            break;
        }
    }

    return expression;
}


bool Parser::parse_db_constraint_item(Decl_Constraint *constraint, List *results) {
    Token *token = lexer->peek_next_token();
/*
    if (token->type == ':') {
        bool success = parse_db_constraint_qualifier(constraint, results);
        return success;
    } else {
*/
        Ast_Expression *expression = parse_imperative_expression();
        if (expression) assert(0);
        return false;
//    }
}

void Parser::parse_struct_db_constraint(Schema *space) {
    expect_and_eat('[');
    Decl_Constraint *constraint = parse_db_constraint_interior();

    if (!constraint) {
        Token *token = lexer->peek_next_token();  // So we have some context to report.
        interp->report_error(token, "Parse error of some kind!!!!\n");
        return;
    }

    expect_and_eat(']');

    // 
    // That's the constraint header; now we look for modifiers.
    //
    while (1) {
        Token *token = lexer->peek_next_token();
        if (token->type == ',') {
            lexer->eat_token();
        } else if (token->type != ':') {
            // We allow ':'-delineated items to not require comma separation... 
            break;
        }

        bool success = parse_db_constraint_item(constraint, &constraint->modifiers);  // XXX
        if (!success) break;
    }

    // 
    // XXX Add expression and constraint to 'results'
    //
    space->db_constraints.add(constraint);
}

bool Parser::parse_struct_members(Schema *space) {
    Token *token = lexer->peek_next_token();
    if (token->type == '[') {
        parse_struct_db_constraint(space);
        return true;
    }

    if (token->type != TOKEN_IDENT) return false;

    Atom *type_name = token->name;
    lexer->eat_token();

    List member_names;

    // There may be many identifiers here, separated by commas...
    while (1) {
        token = lexer->peek_next_token();
        if (token->type != TOKEN_IDENT) break;

        Atom *slot_name = token->name;
        lexer->eat_token();

        member_names.add(slot_name);

        token = lexer->peek_next_token();

        if (token->type == ',') {
            lexer->eat_token();
            continue;
        } else {
            break;
        }
    }

    if (member_names.items > 0) {
        int num_slots = member_names.items;
        Value_Array *array = interp->memory_manager->create_value_array(num_slots * 2);
        space->named_slots = ToBarrier(array);
        
        int cursor = 0;
        Atom *name;
        Foreach(&member_names, name) {
            array->values[cursor++] = ToBarrierF(name);  // @WriteBarrier
            array->values[cursor++] = ToBarrierF(interp->uninitialized);  // XXX @Incomplete default value!!
        } Endeach;

        member_names.clean();
    }

    expect_and_eat(';');

    return true;
}

Schema *Parser::parse_struct_declaration() {
    Token *token = lexer->peek_next_token();
    assert(token->type == TOKEN_KEYWORD_STRUCT);
    lexer->eat_token();

    token = lexer->peek_next_token();
    if (token->type != TOKEN_IDENT) {
        // XXX Parse error
        return NULL;
    }

    Atom *name = token->name;

    lexer->eat_token();
    expect_and_eat('{');

    Schema *result = GC_NEW(Schema);
    result->type_name = name;

    while (1) {
        token = lexer->peek_next_token();
        if (token->type == ';') {
            lexer->eat_token();
            continue;
        }

        if (token->type == '}') break;

        bool success = parse_struct_members(result);
        if (!success) break;

        // @Robustness: Check for an error being flagged and break
        // ... this will have to wait til we actually flag errors.
        // (If this is not done we can infinite-loop here).
    }

    // XXX Leak members
    expect_and_eat('}');

    return result;
}

Ast_Declarative *Parser::parse_declarative_toplevel() {
    Token *token = lexer->peek_next_token();
    assert(token->type == '[');

    return NULL;  // XXX
//    return (Ast_Declarative *)parse_declarative_assertion();  // Actually it's really a child of First_Class but who gives a crap.
}

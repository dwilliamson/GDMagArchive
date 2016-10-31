#include "general.h"
#include "parser.h"
#include "parser_private.h"
#include "tree_changer.h"
#include "interp.h"

#include "concatenator.h"
#include "lexer.h"  // For operator_token_type enums

Tree_Changer::Tree_Changer(Lerp_Interp *_interp) {
    interp = _interp;
    next_iterator_variable_index = 0;
}

bool is_liftable_iteration(Ast_Expression *expression) {
    if (expression->type == AST_EACH_PLACEHOLDER) return true;
    return false;
}


Ast_Expression *Tree_Changer::postprocess(Ast_Unary_Expression *expression) {
    if (!is_liftable_iteration(expression->subexpression)) return expression;

    bool change = false;
    int left_value;

    switch (expression->operator_token_type) {
    case '+':
    case '-':
        change = true;
        left_value = 0;
        break;
    default:
        change = false;
        break;
    }

    if (!change) return expression;

    //
    // Make a binary expression out of this unary one... it will still have the 'each'
    // in it... then we call a different 'postprocess' routine on the binary
    // expression...
    //
    Ast_Binary_Expression *result = AST_NEW(Ast_Binary_Expression);
    result->operator_token_type = expression->operator_token_type;
    result->left = (Ast_Expression *)interp->parser->make_integer(left_value);
    result->right = expression->subexpression;

    return postprocess(result);
}

inline bool should_left_associate(int token) {
    if (token == '.') return true;
    if (token == TOKEN_OPERATOR_ARRAY_SUBSCRIPT) return true;
    return false;
}

Ast_Binary_Expression *Tree_Changer::left_associate(Ast_Binary_Expression *expression) {
    int token = expression->operator_token_type;
    if (should_left_associate(token)) {
        Ast_Expression *right = expression->right;
        if (right->flags & AST_PARENTHESIZED) return expression;

        if (right->type == AST_BINARY_EXPRESSION) {
            Ast_Binary_Expression *bin_r = static_cast <Ast_Binary_Expression *>(right);
            if (should_left_associate(bin_r->operator_token_type)) {
                // If we have a.b.c or such, we need to change this from being 
                // right-associative to left-associative.  
                    
                expression->right = bin_r->left;
                bin_r->left = postprocess(expression);
                
                return static_cast <Ast_Binary_Expression *> (postprocess(bin_r));  // XXXX Unnecessary?
            }
        }
    }

    return expression;
}

/*
// @Refactor: This is basically cut-and-paste from left_associate of Binary_Expression above.
// Array subscript REALLY should just be a binary expression node.  This would simplify
// several things, I think.  In fact this could just go away since operator precedence
// would take over.
Ast_Binary_Expression *Tree_Changer::left_associate(Ast_Array_Subscript *expression) {
    if (expression->flags & AST_PARENTHESIZED) return expression;
    if (expression->left->type != AST_BINARY_EXPRESSION) return expression;

    Ast_Binary_Expression *left = static_cast <Ast_Binary_Expression *>(expression->left);
    if (left->operator_token_type != '.') return expression;

    expression->
        if (Ast_Binary_Expression *right = expression->right;

        if ((right->type == AST_BINARY_EXPRESSION) && !(right->flags & AST_PARENTHESIZED)) {
            Ast_Binary_Expression *bin_r = static_cast <Ast_Binary_Expression *>(right);
            if (bin_r->operator_token_type == '.') {
                // If we have a.b.c or such, we need to change this from being 
                // right-associative to left-associative.  
                    
                expression->right = bin_r->left;
                bin_r->left = postprocess(expression);
                
                return static_cast <Ast_Binary_Expression *> (postprocess(bin_r));
            }
        }
    }

    return expression;
}
*/

Ast_Expression *Tree_Changer::postprocess(Ast_Binary_Expression *expression) {
    expression = left_associate(expression);
    if (expression->type != AST_BINARY_EXPRESSION) return expression;  // It might have changed!  @Refactor: Make note of this, have the return value not be Ast_Binary_Expression.

    bool left_liftable = is_liftable_iteration(expression->left);
    bool right_liftable = is_liftable_iteration(expression->right);
    if (!left_liftable && !right_liftable) return expression;

    if (left_liftable && right_liftable) {
        interp->report_bytecode_error("Weird situation where both sides of binop are liftable... DISSED.\n");
        return expression;
    }

    Ast_Each *each;
    Ast_Expression *other;
    if (left_liftable) {
        each = (Ast_Each *)expression->left;
        other = expression->right;
    } else {
        each = (Ast_Each *)expression->right;
        other = expression->left;
    }

    // @Incomplete:
    // The left_liftable case is not handled right now so I will just bail.
    // Not sure what the semantics should be.
    if (left_liftable) {
        interp->report_bytecode_error("'each' on left-hand side of a binary operator is not yet supported.\n");
        return NULL;
    }

    Ast_Each *result = GC_NEW(Ast_Each);
    result->type = AST_EACH_HARDENED;
    result->list_expression = each->list_expression;
    result->body = each->body;
    result->flags |= EACH_BINOP;
    result->operator_token_type = expression->operator_token_type;
    result->starter = other;

    return (Ast_Expression *)result;
}

Atom *Tree_Changer::make_unique_variable() {
    Concatenator c;
    c.add("each var ");  // Has spaces in it so it can't conflict with user variables
    c.add(next_iterator_variable_index);

    next_iterator_variable_index++;

    char *name = c.get_result();
    return interp->parser->make_atom(name);
    delete [] name;
}


Ast *Tree_Changer::prepend_to_body(Ast *body, Ast_Statement *statement) {
    if (body->type != AST_BLOCK) {
        if (body->type != AST_STATEMENT) {
            Ast_Statement *statement = AST_NEW(Ast_Statement);
            statement->expression = (Ast_Expression *)body;
            body = statement;
        }

        // Make a singleton block out of it, so we can then prepend the statement.

        Ast_Block *block = AST_NEW(Ast_Block);
        block->statements.add(body);

        body = block;
    }

    assert(body->type == AST_BLOCK);
    Ast_Block *block = (Ast_Block *)body;
    block->statements.add_at_head(statement);

    return block;
}

//
// @Incomplete, @Refactor: Right now we create some global variables to hold the
// iteration value... when we have locals implemented properly, we should just be
// able to open a new scope and use a local.
//
Ast_Expression *Tree_Changer::postprocess(Ast_Procedure_Call_Expression *expression) {
    // Find an argument that's an iterator...

    Atom *substitution_variable = NULL;
    Ast_Each *each = NULL;

    Ast_Expression *arg;
    Foreach(&expression->argument_list, arg) {
        if (is_liftable_iteration(arg)) {
            substitution_variable = make_unique_variable();
            each = (Ast_Each *)arg;
            __n->data = substitution_variable;
            break;
        }
    } Endeach;

    if (each == NULL) return expression;
    
    //
    // We call 'postprocess' again, in case there are any other arguments
    // that need lifting.
    //
    Ast_Expression *new_body = postprocess(expression);

    //
    // We are gonna modify the 'each' to lift it...
    //
    each->type = AST_EACH_HARDENED;
    if (each->body) {
        // If the each has a body, that is currently not supported... we should
        // support this eventually!  (I am too tired to figure out the exact
        // solution right now... argh)  @Incomplete

        interp->report_bytecode_error("'each' with a body is not supported as a procedure call argument.  (Will be soon, the code is just incomplete!)\n");
        return expression;
    }

    // 
    // Now we prepend something to this body that sets the temporary
    // variable to $0...
    //
    {
        Ast_Binary_Expression *binary = AST_NEW(Ast_Binary_Expression);
        binary->type = AST_BINARY_EXPRESSION;
        binary->operator_token_type = '=';
        binary->left = (Ast_Expression *)substitution_variable;
        binary->right = (Ast_Expression *)interp->parser->make_implicit_variable(0);
//        binary->right = (Ast_Expression *)interp->parser->make_integer(42);
        
        Ast_Statement *statement = AST_NEW(Ast_Statement);
        statement->expression = binary;

        new_body = (Ast_Expression *)prepend_to_body(new_body, statement);
    }

    each->body = new_body;
    
    return (Ast_Expression *)each;
}

Ast_Expression *Tree_Changer::postprocess(Ast_Expression *expression) {
    switch (expression->type) {
    case AST_UNARY_EXPRESSION:
        return postprocess((Ast_Unary_Expression *)expression);
    case AST_BINARY_EXPRESSION:
        return postprocess((Ast_Binary_Expression *)expression);
    case AST_PROCEDURE_CALL_EXPRESSION:
        return postprocess((Ast_Procedure_Call_Expression *)expression);
    default:
        return expression;
    }
}

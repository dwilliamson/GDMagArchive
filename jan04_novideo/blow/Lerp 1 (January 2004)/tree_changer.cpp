#include "general.h"
#include "parser.h"
#include "parser_private.h"
#include "tree_changer.h"
#include "interp.h"

#include "concatenator.h"

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

Ast_Expression *Tree_Changer::postprocess(Ast_Binary_Expression *expression) {
    bool change = false;
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

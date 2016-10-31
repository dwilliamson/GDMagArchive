/*
  This file implements all the primitive queries.  (Primitives are implemented
  via C callbacks, as opposed to matching against the database).  Right now there
  is only one primitive, "notequal".
*/

#include "general.h"

#include <stdlib.h>
#include <ctype.h>

First_Class *find_binding(List *bindings, First_Class *variable);
bool arguments_match(First_Class *assertion_arg, First_Class *goal_arg, List *downward_bindings);

First_Class *prim_notequal_get_arg(Decl_Expression *exp, int index, List *bindings) {
    First_Class *goal_arg = (First_Class *)exp->arguments.nth(index);
    
    if (goal_arg->type == ARG_VARIABLE) {
        First_Class *orig_goal_arg = goal_arg;

        goal_arg = find_binding(bindings, goal_arg);
        if (goal_arg == NULL) {
            fprintf(stderr, "Primitive notequal: Comparison with unbound variable is not valid (unbound variable was '%s')\n", ((Primitive_Function *)orig_goal_arg)->name);
        }
    }

    return goal_arg;
}

// Evaluate the primitive 'notequal'...
void eval_primitive_notequal(Decl_Expression *goal_expression, List *results, List *bindings) {
    // Check the number of arguments.
    if (goal_expression->arguments.items != 3) {
        fprintf(stderr, "Primitive notequal: incorrect number of arguments (must be 2!)\n");
        return;
    }

    // Retrieve the two arguments that we want to compare.
    First_Class *arg1, *arg2;
    arg1 = prim_notequal_get_arg(goal_expression, 1, bindings);
    arg2 = prim_notequal_get_arg(goal_expression, 2, bindings);
    if (arg1 == NULL) return;
    if (arg2 == NULL) return;

    // If the arguments are equal, there is no solution to (notequal ?x ?y), so
    // we return without producing any results.
    if (arguments_match(arg1, arg2, NULL)) return;

    // Otherwise, there's a valid solution... it doesn't involve binding any new
    // variables though.  So we just create a new goal, without any new bindings,
    // and add it to the results.
    Matched_Goal *matched_goal = new Matched_Goal();
    results->add(matched_goal);
}

// Report whether 'expression' is a primitive or not... (if it's a primitive then
// we call a built-in C function to evaluate it, we don't treat it as a database
// match).
bool is_primitive_goal(Decl_Expression *expression) {
    assert(expression->arguments.items);
    First_Class *argument = (First_Class *)expression->arguments.nth(0);
    if (argument->type == ARG_PRIMITIVE_FUNCTION) return true;
    return false;
}


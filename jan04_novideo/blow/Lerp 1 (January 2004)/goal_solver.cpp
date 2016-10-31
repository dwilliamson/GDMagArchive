#include "general.h"
#include "goal_solver.h"
#include "interp.h"
#include "unicode.h"

Goal_Solver::Goal_Solver(Lerp_Interp *_interp) {
    interp = _interp;
}

bool Goal_Solver::arguments_match(First_Class *goal_arg, First_Class *assertion_arg, Binding **downward_bindings) {
    switch (assertion_arg->type) {
    case ARG_ATOM:
        return (goal_arg == assertion_arg);
    case ARG_INTEGER:
        return ((Integer *)goal_arg)->value == ((Integer *)assertion_arg)->value;
    case ARG_FLOAT:
        return ((Float *)goal_arg)->value == ((Float *)assertion_arg)->value;
    case ARG_STRING: {
        Text_Utf8 *s1 = ((String *)goal_arg)->value;
        Text_Utf8 *s2 = ((String *)assertion_arg)->value;
        return Unicode::strings_match(s1, s2);
    }
    case ARG_VARIABLE: {
        if (downward_bindings) {
            if (goal_arg->type != ARG_VARIABLE) {
                Binding *binding = get_binding();
                binding->variable = (Variable *)assertion_arg;
                binding->assign(goal_arg);
                
                binding->next = ToBarrier(*downward_bindings);
                *downward_bindings = binding;
            }
        }
        return true;
    }

    }

    return false;
}

First_Class *Goal_Solver::find_binding(Binding *binding, First_Class *variable) {
    if (binding == NULL) return NULL;
    if (variable == interp->the_anonymous_variable) return NULL;

    assert(variable->type == ARG_VARIABLE);

    while (binding) {
        if (binding->variable == variable) {
            return binding->bound_value->read();
        }

        binding = binding->next->read();
    }

    return NULL;
}

Matched_Goal::~Matched_Goal() {
}

Matched_Goal *Goal_Solver::xref_goal(Matched_Goal *orig_goal, Binding *sub_binding_list) {
    Matched_Goal *result = get_matched_goal();
    result->assertion = orig_goal->assertion;
    
    Binding *binding = orig_goal->bindings->read();
    while (binding) {
        Binding *new_binding = get_binding();
        new_binding->variable = binding->variable;

        First_Class *rvalue = binding->bound_value->read();
        if (rvalue->type == ARG_VARIABLE) {
            First_Class *substitute = find_binding(sub_binding_list, rvalue);

            // @Refactor @ThinkAboutThis: If we can't find a binding for this variable,
            // it means the variable was used as an argument of the declaration but
            // is not constrained in any way by the declaration itself.. which I guess
            // means the declaration is true e.g. "For all X".  For now the way we
            // represent this is by leaving the "?x" in the answer, representing
            // a wildcard... who knows if that's the right thing though.

            if (substitute) rvalue = substitute;
        }

        new_binding->assign(rvalue);

        new_binding->next = result->bindings;
        result->bindings = ToBarrier(new_binding);

        binding = binding->next->read();
    }

    return result;
}

Matched_Goal *Goal_Solver::get_matched_goal() {
    Matched_Goal *goal = GS_NEW(Matched_Goal);
    goal->type = ARG_MATCHED_GOAL;
    goal->next = NULL;
    goal->bindings = NULL;

    return goal;
}

Binding *Goal_Solver::get_binding() {
    Binding *binding = GS_NEW(Binding);
    binding->type = ARG_BINDING;
    binding->next = NULL;
    binding->variable = NULL;
    return binding;
}

void Goal_Solver::duplicate_and_xref_goal(Matched_Goal *orig_goal,
                                          Binding *sub_matches, Matched_Goal **results) {
    Binding *binding_list_list = sub_matches;
    while (binding_list_list) {
        Binding *binding_list = (Binding *)binding_list_list->bound_value->read();
        assert(binding_list->type == ARG_BINDING);

        Matched_Goal *xref = xref_goal(orig_goal, binding_list);

        Matched_Goal *old = *results;
        xref->next = ToBarrier(old);
        *results = xref;

        binding_list_list = binding_list_list->next->read();
    }
}

// @Speed:  add_binding is a slow O(n)!
void add_binding(Binding *new_binding, Binding **bindings_ptr) {
    if (*bindings_ptr == NULL) {
        *bindings_ptr = new_binding;
        return;
    }

    Binding *cursor = *bindings_ptr;
    while (cursor->next) cursor = cursor->next->read();
    cursor->next = ToBarrier(new_binding);
}

//
// @Refactor: it seems to me there are too many damn Bindings lists going on below,
// figure that out! 
//
Matched_Goal *Goal_Solver::match_two_expressions(Decl_Expression *a, Decl_Assertion *assertion, Binding **downward_bindings, Binding *extant_bindings) {
    Matched_Goal *matched_goal = NULL;
    Decl_Expression *b = assertion->expression->read();
//    if (b == NULL) b = assertion->conditionals;  // @Refactor, required for find_direct_literal_fact
    assert(b);


    int goal_index;
    for (goal_index = 0; goal_index < a->num_arguments; goal_index++) {
        First_Class *goal_arg = a->arguments[goal_index]->read();
        First_Class *assertion_arg = b->arguments[goal_index]->read();

        switch (goal_arg->type) {
        default:
            if (!arguments_match(goal_arg, assertion_arg, downward_bindings)) {
                return NULL;
            }
            break;
        case ARG_VARIABLE:
        {
            First_Class *bound = find_binding(extant_bindings, goal_arg);
            if (!bound && matched_goal) bound = find_binding(matched_goal->bindings->read(), goal_arg);

            if (bound) {
                if (!arguments_match(bound, assertion_arg, downward_bindings)) {
                    return NULL;
                }
            } else {
                if (!matched_goal) matched_goal = get_matched_goal();  // XXX @Memory: Unmanaged memory.  This can get leaked if the match fails later.
                Binding *binding = get_binding();
                binding->variable = (Variable *)goal_arg;
                binding->assign(assertion_arg);

                add_binding(binding, (Binding **)&matched_goal->bindings);
                break;
            }
        }
        }
    }

    if (!matched_goal) matched_goal = get_matched_goal();
    matched_goal->assertion = ToBarrier(assertion);

    return matched_goal;
}

void beeb() {
}

bool Goal_Solver::match_two_expressions_cheaply(Decl_Expression *a, Decl_Expression *b) {
    Binding *bindings = NULL;

    if (a->flags & DECL_EXPR_FLAGS_WILDCARDED) {
        if (a->num_arguments > b->num_arguments) return false;
    } else {
        if (a->num_arguments != b->num_arguments) return false;
    }

    int goal_index;
    for (goal_index = 0; goal_index < a->num_arguments; goal_index++) {
        First_Class *goal_arg = a->arguments[goal_index]->read();
        First_Class *assertion_arg = b->arguments[goal_index]->read();

        if ((a->num_arguments == 4) && (goal_index == 1)) beeb();

        switch (goal_arg->type) {
        default:
            if (!arguments_match(goal_arg, assertion_arg, NULL)) return false;
            break;
        case ARG_VARIABLE:
        {
            First_Class *bound = find_binding(bindings, goal_arg);

            if (bound) {
                if (!arguments_match(bound, assertion_arg, NULL)) return false;
            } else {
                Binding *binding = get_binding();
                binding->variable = (Variable *)goal_arg;
                binding->assign(assertion_arg);

                add_binding(binding, &bindings);
                break;
            }
        }
        }
    }

    assert(goal_index == a->num_arguments);

/*
    Binding *binding;
    Foreach(&bindings, binding) {
        delete binding;
    } Endeach;
*/
    return true;
}



void Goal_Solver::single_direct_match(Database *db, Decl_Expression *goal_expression, Decl_Assertion *assertion, Matched_Goal **results, Binding *extant_bindings) {
    Decl_Expression *ass_expression = assertion->expression->read();

    if (goal_expression->flags & DECL_EXPR_FLAGS_WILDCARDED) {
        if (ass_expression->num_arguments < goal_expression->num_arguments) return;
    } else {
        if (ass_expression->num_arguments != goal_expression->num_arguments) return;
    }

    Binding *downward_bindings = NULL;

    Matched_Goal *matched_goal = match_two_expressions(goal_expression, assertion, &downward_bindings, extant_bindings);
    if (!matched_goal) return;

    // If there are conditions attached to this goal, see whether we meet them.
    if (assertion->conditionals) {
        Binding *binding_list_list = NULL;
        Decl_Expression *cursor = assertion->conditionals->read();
        match_conjunction_series(db, assertion, cursor, downward_bindings, &binding_list_list);

        // XXX What if we don't exactly match?
        duplicate_and_xref_goal(matched_goal, binding_list_list, results);
    } else {
        // Add to list
        matched_goal->next = ToBarrier(*results);
        *results = matched_goal;
    }

    //
    // If this had a wildcard, we are going to go through and add to the bindings,
    // representing all the anonymous entries that the '?*' matched.
    //
    if (goal_expression->flags & DECL_EXPR_FLAGS_WILDCARDED) {
        int index = goal_expression->num_arguments;

        Decl_Expression *ass_expression = assertion->expression->read();
        while (index < ass_expression->num_arguments) {
            First_Class *arg = ass_expression->arguments[index]->read();
            if (arg->type == ARG_VARIABLE) {
                First_Class *looked_up = find_binding(downward_bindings, arg);
                if (looked_up) arg = looked_up;
            }

            Binding *binding = get_binding();
            binding->variable = interp->the_anonymous_variable;
            binding->assign(arg);

            add_binding(binding, (Binding **)&matched_goal->bindings);

            index++;
        }
    }
}

void Goal_Solver::direct_match(Database *db, Decl_Expression *goal_expression, Matched_Goal **results, Binding *extant_bindings) {
    *results = NULL;
    if (goal_expression->num_arguments == 0) return;

    Decl_Assertion *assertion = db->assertions->read();
    while (assertion) {
        single_direct_match(db, goal_expression, assertion, results, extant_bindings);
        assertion = assertion->next->read();
    }
}

Binding *Goal_Solver::copy_binding(Binding *old_binding) {
    Binding *result = get_binding();
    *result = *old_binding;  // XXX Assumes we can just copy string ptr
    return result;
}

Binding *Goal_Solver::merge_bindings(Binding *a, Binding *b) {
    if (a == NULL) return b;

    Binding *cursor = a;
    while (cursor->next) cursor = cursor->next->read();
    cursor->next = ToBarrier(b);
    return a;
}

void Goal_Solver::match_conjunction_series(Database *db, Decl_Assertion *sequence, Binding **results) {
    *results = NULL;
    Binding *bindings = NULL;
    match_conjunction_series(db, sequence, sequence->conditionals->read(), bindings, results);
}

bool Goal_Solver::match_conjunction_series(Database *db, Decl_Assertion *sequence, Decl_Expression *cursor, Binding *bindings, Binding **results) {
    if (cursor == NULL) {
        // Add this binding_list to the binding_list_list
        Binding *binding = get_binding();
        
        assert(binding->next == NULL);
        binding->next = ToBarrier(*results);
        binding->bound_value = ToBarrier((First_Class *)bindings);
        *results = binding;

        return true;
    }

    Matched_Goal *matched_goals = NULL;
    direct_match(db, cursor, &matched_goals, bindings);

    Matched_Goal *matched_goal = matched_goals;
    while (matched_goal) {
        Binding *new_bindings = merge_bindings(matched_goal->bindings->read(), bindings);
        match_conjunction_series(db, sequence, cursor->next->read(), new_bindings, results);
        matched_goal = matched_goal->next->read();
    }

    return false;
}





int Goal_Solver::remove_direct_facts(Database *db, Decl_Expression *goal_expression, bool *by_implication_return) {
    int num_removed = 0;
    if (goal_expression->num_arguments == 0) return num_removed;

    Decl_Assertion *assertion = db->assertions->read();
    Decl_Assertion **assertion_ptr = &assertion;
    while (assertion) {
        Binding *extant_bindings = NULL;
        Matched_Goal *matched_goal = match_two_expressions(goal_expression, assertion, NULL, extant_bindings);
        if (matched_goal) {
            if (assertion->conditionals) {
                *by_implication_return = true;
                continue;
            } 

            num_removed++;
            *assertion_ptr = assertion->next->read();   // @WriteBarrier!?!
        }

        assertion_ptr = (Decl_Assertion **)&assertion->next;
        assertion = assertion->next->read();
    }

    return num_removed;
}


int Goal_Solver::find_direct_literal_fact(Database *db, Decl_Expression *goal_expression, Decl_Assertion **results, bool *by_implication_return) {

    if (goal_expression->num_arguments == 0) return 0;

    Decl_Assertion *assertion = db->assertions->read();
    while (assertion) {
        assert(assertion->expression);
        bool matched = match_two_expressions_cheaply(goal_expression, assertion->expression->read());
        if (matched) {
            if (assertion->conditionals) {
                *by_implication_return = true;
                return 0;
            } 

            *results = assertion;
            return 1;
        }

        assertion = assertion->next->read();
    }

    return 0;
}

int Goal_Solver::find_direct_literal_fact_from_one_value(Database *db, First_Class *value, Decl_Assertion **results, bool *by_implication_return) {

    Decl_Assertion *assertion = db->assertions->read();
    for (; assertion; assertion = assertion->next->read()) {
        assert(assertion->expression);
        Decl_Expression *expression = assertion->expression->read();

        if (expression->num_arguments < 1) continue;
        if (!arguments_match(value, expression->arguments[0]->read(), NULL)) continue;

        if (assertion->conditionals) {
            *by_implication_return = true;
            return 0;
        } 

        *results = assertion;
        return 1;
    }

    return 0;
}

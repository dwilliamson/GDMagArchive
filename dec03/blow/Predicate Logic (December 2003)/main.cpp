#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

//
// The Goal_Solver basically holds the entire database 
// and performs all the rule-matchings.
//
struct Goal_Solver {
    // The list 'assertions' contains the entire database (facts as well as inference
    // rules).  We do not care about speed yet so we are not organizing this database
    // in any way.  All queries just iterate linearly over this list in attempting
    // to match.
    List assertions;

    // match_conjunction_series() takes a list of expressions, all of which the user
    // wants to be true at once (i.e. they are connected by logical ANDs).  We iterate
    // over this series matching the expressions one by one, binding variables as
    // necessary to make each match.  In 'results' we return every combination of 
    // facts from the database that can satisfy our query.
    void match_conjunction_series(Decl_Expression_Sequence *sequence, List *results);

  protected:
    // direct_match() matches an expression against the database, putting the matches
    // into the list 'results'.  If you choose to pass some bindings into this function,
    // they will be used to substitute for variables in 'expression'.
    void direct_match(Decl_Expression *expression, List *results, List *bindings = NULL);

    // Helper function for direct_match
    void single_direct_match(Decl_Expression *goal_expression, Decl_Assertion *assertion, List *results, List *bindings = NULL);

    // Helper function for the other match_conjunction_series
    bool match_conjunction_series(Decl_Expression_Sequence *sequence, int cursor, List *bindings, List *results);
};


Goal_Solver *goal_solver;
Parser *the_parser;

bool is_primitive_goal(Decl_Expression *expression);  // Defined in primitives.cpp
void eval_primitive_notequal(Decl_Expression *goal_expression, List *results, List *bindings);


// Given a first-class value, print it to stdout.
void print_value(First_Class *value) {
    switch (value->type) {
    case ARG_ATOM:
        printf("%s", ((Atom *)value)->name);
        break;
    case ARG_VARIABLE:
        printf("?%s", ((Variable *)value)->name);
        break;
    case ARG_INTEGER_CONSTANT:
        printf("%d", ((Integer *)value)->value);
        break;
    default:
        assert(0);
        break;
    }
}

// Given an expression, print it to stdout.
void print_expression(Decl_Expression *expression) {
    printf("(");

    bool first = true;
    First_Class *argument;
    Foreach(&expression->arguments, argument) {
        if (!first) printf(" ");
        first = false;
        print_value(argument);
    } Endeach;

    printf(")");
}

// Given an assertion, print it to stdout.
void print_assertion(Decl_Assertion *assertion) {
    print_expression(assertion->expression);
    printf("\n");
}

// Given a sequence of expressions, print it to stdout.
void print_expression_sequence(Decl_Expression_Sequence *sequence) {
    bool first = true;
    Decl_Expression *expression;
    Foreach(&sequence->expressions, expression) {
        if (!first) printf(", ");
        first = false;

        print_expression(expression);
    } Endeach;
}



// Match assertion_arg to goal_arg.  If goal_arg is a variable, and we are making new
// bindings (downward_bindings is non-NULL), the arguments will 
// always match (and we make a new binding).
bool arguments_match(First_Class *assertion_arg, First_Class *goal_arg, List *downward_bindings) {
    switch (goal_arg->type) {
    case ARG_ATOM:
        return (goal_arg == assertion_arg);
    case ARG_VARIABLE: {
        if (assertion_arg->type != ARG_VARIABLE) {
            if (downward_bindings) {
                Binding *binding = new Binding;
                binding->variable = (Variable *)goal_arg;
                binding->bound_value = assertion_arg;
                downward_bindings->add(binding);
            }
        }
        return true;
    }

    }

    return false;
}

// If this variable has a binding inside this binding list, return the binding, else NULL.
First_Class *find_binding(List *bindings, First_Class *variable) {
    if (bindings == NULL) return NULL;

    assert(variable->type == ARG_VARIABLE);

    Binding *binding;
    Foreach(bindings, binding) {
        if (binding->variable == variable) {
            return binding->bound_value;
        }
    } Endeach;

    return NULL;
}

// Delete all bindings in a list.
void clean_bindings(List *bindings) {
    if (!bindings) return;

    Binding *binding;
    Foreach(bindings, binding) {
        delete binding;
    } Endeach;
}

Matched_Goal::~Matched_Goal() {
    clean_bindings(&bindings);
}


//
// The job of xref_goal() is to re-map solutions from the namespace of an
// inference rule into the namespace of the query.  The reason we need this is,
// suppose we have a rule 
//
// (grandparent ?x ?y) <- (parent ?x ?p), (parent ?p ?y);
//
// And we perform the query: (grandparent michael ?who)
//
// When we first match the left-hand side of this rule, we will get bindings
// ?x = michael, ?y = ?who.  We can now go solve the right-hand side, which
// will give us an answer for ?y.  (It can't give us an answer for ?x since
// that is already constrained).  Once we're done, we have an answer for
// ?y on the right-hand side, but we need to make that into an answer for ?who
// in the query.  That's what this function does.
//
Matched_Goal *xref_goal(Matched_Goal *orig_goal, List *sub_binding_list) {
    Matched_Goal *result = new Matched_Goal;
    result->assertion = orig_goal->assertion;
    
    Binding *binding;
    Foreach(&orig_goal->bindings, binding) {
        Binding *new_binding = new Binding;
        new_binding->variable = binding->variable;

        First_Class *rvalue = binding->bound_value;
        if (rvalue->type == ARG_VARIABLE) {
            First_Class *substitute = find_binding(sub_binding_list, rvalue);
            assert(substitute);
            if (substitute) rvalue = substitute;
        }

        new_binding->bound_value = rvalue;
        result->bindings.add(new_binding);
    } Endeach;

    return result;
}

// Given a 'Matched_Goal' (just a particular assertion in the database that we
// decided to investigate), and a set of sub-matches (which represent
// different steps that take us further to the goal), we need to generate
// one solution per sub-match.  To do that we need to map the variables in
// the sub-binding list back into the namespace of the query.
void duplicate_and_xref_goal(Matched_Goal *orig_goal,
                             List *sub_matches, List *results) {
    List *binding_list;
    Foreach(sub_matches, binding_list) {
        Matched_Goal *xref = xref_goal(orig_goal, binding_list);
        results->add(xref);
    } Endeach;

    delete orig_goal;
}

//
// 'single_direct_match' is in a sense the "meat" of this whole file, where
// the nucleus of the work gets done.
//
void Goal_Solver::single_direct_match(Decl_Expression *goal_expression, Decl_Assertion *assertion, List *results, List *bindings) {

    // The 'goal' is the query we are trying to match; the 'assertion' is the item
    // in the database we are comparing it against.

    // If the goal and the assertion have different numbers of arguments, they
    // cannot possibly match.
    if (assertion->expression->arguments.items != goal_expression->arguments.items) return;

    Matched_Goal *matched_goal = NULL;

    List downward_bindings_store;
    List *downward_bindings = NULL;

    //
    // If the assertion has conditionals, that means it's an inference rule.
    // (The code treats inference rules and database assertions as much the same;
    // a direct assertion is just an inference rule that is true always, i.e.
    // its list of conditions is empty).
    //
    // If it's an inference rule, that means we will have to recurse, evaluating
    // its conditons.  Because we're in the middle of a solution, we may have some
    // bindings that already constrain the search.  (These are the "downward bindings",
    // because they flow downward through the recursion tree, imposing constraints on
    // the child paths of the search).
    //

    if (assertion->conditionals) downward_bindings = &downward_bindings_store;

    First_Class *goal_arg;
    int goal_index = 0;
    Foreach(&goal_expression->arguments, goal_arg) {
        First_Class *assertion_arg = (First_Class *)assertion->expression->arguments.nth(goal_index);

        switch (goal_arg->type) {
        default:
            // If the goal arg is not a variable (i.e. it's a data value or label):

            // If one of the arguments doesn't match, well, then the match has failed.
            // Delete our current bindings (they represent a partial solution that will
            // not be completed) and return.
            if (!arguments_match(goal_arg, assertion_arg, downward_bindings)) {
                clean_bindings(downward_bindings);
                return;
            }
            break;
        case ARG_VARIABLE:
        {
            // If the goal arg is a variable... either it's already bound, or it's not.

            First_Class *bound = find_binding(bindings, goal_arg);

            // If it's already bound, we need to check the bound value against the assertion.
            // If they don't match, we bail.  If they do match, we just continue as usual.
            if (bound) {
                if (!arguments_match(bound, assertion_arg, downward_bindings)) {
                    clean_bindings(downward_bindings);
                    return;
                }
            } else {

                // Since it's not already bound, we create a new binding for it.
                // Henceforth, the rest of the search must obey this binding.

                if (!matched_goal) matched_goal = new Matched_Goal();  // This can get leaked if the match fails later... but we don't care about leaks.
                Binding *binding = new Binding;
                binding->variable = (Variable *)goal_arg;
                binding->bound_value = assertion_arg;
                matched_goal->bindings.add(binding);
                break;
            }
        }
        }

        goal_index++;
    } Endeach;

    if (!matched_goal) matched_goal = new Matched_Goal();
    matched_goal->assertion = assertion;

    // All that stuff above was just about matching the left-hand-side of the assertion or fact.
    // If there are conditions attached to this goal (i.e. it's an inference rule), we need
    // to ensure that we meet them.
    if (assertion->conditionals) {
        List matched;

        // Calling match_conjunction_series, we pass in the downward_bindings which
        // constrain the search.
        match_conjunction_series(assertion->conditionals, 0, downward_bindings, &matched);

        // We get back a set of matches.  For each item in 'matched', we need to fork
        // the goal and customize it for that particular match's set of bindings.
        // That's what duplicate_and_xref_goal() does.
        duplicate_and_xref_goal(matched_goal, &matched, results);
    } else {
        // There were no conditions, so this is just a clean match, yay.
        results->add(matched_goal);
    }

    clean_bindings(downward_bindings);
}

// 'direct_match' matches one expression against the database.  (You can pass in a list
// of 'bindings' already in effect, which constrain the match).
void Goal_Solver::direct_match(Decl_Expression *goal_expression, List *results, List *bindings) {
    if (goal_expression->arguments.items == 0) return;

    if (is_primitive_goal(goal_expression)) {
        // This is basically a lazy hack.  There is only one primitive defined in the system
        // right now, 'notequal'.  So if any goal is marked as a primitive, we know it
        // must be 'notequal' so we just directly call eval_primitive_notequal.  Of course
        // this will have to change in future.
        eval_primitive_notequal(goal_expression, results, bindings);
        return;
    }

    // It wasn't a primitive.  So for every assertion in the database, attempt
    // to match this expression, and see what happens.
    Decl_Assertion *assertion;
    Foreach(&assertions, assertion) {
        single_direct_match(goal_expression, assertion, results, bindings);
    } Endeach;
}


// Copy a single binding.
Binding *copy_binding(Binding *old_binding) {
    Binding *result = new Binding();
    *result = *old_binding;
    return result;
}

// Given a list of bindings, create a new list that contains copies of
// those bindings.
List *copy_binding_list(List *list) {
    List *results = new List();

    Binding *old_binding;
    Foreach(list, old_binding) {
        Binding *binding = copy_binding(old_binding);
        results->add(binding);
    } Endeach;

    return results;
}

// Given two lists of bindings, create a new list that contains copies
// of all those bindings.
List *merge_bindings(List *a, List *b) {
    List *results = new List();

    Binding *old_binding;
    Foreach(a, old_binding) {
        Binding *binding = copy_binding(old_binding);
        results->add(binding);
    } Endeach;

    Foreach(b, old_binding) {
        Binding *binding = copy_binding(old_binding);
        results->add(binding);
    } Endeach;

    return results;
}

// Delete all bindings in a list...
void delete_bindings(List *list) {
    Binding *binding;
    Foreach(list, binding) {
        delete binding;
    } Endeach;
}

//
// match_conjunction_series() takes a list of expressions, all of which the user
// wants to be true at once (i.e. they are connected by logical ANDs).  We iterate
// over this series matching the expressions one by one, binding variables as
// necessary to make each match.  In 'results' we return every combination of 
// facts from the database that can satisfy our query.
//
void Goal_Solver::match_conjunction_series(Decl_Expression_Sequence *sequence, List *results) {
    List *bindings = new List();
    match_conjunction_series(sequence, 0, bindings, results);
}

// This version of the function does the "real work" for the entry point above.
bool Goal_Solver::match_conjunction_series(Decl_Expression_Sequence *sequence, int cursor, List *bindings, List *results) {
    List *terms = &sequence->expressions;

    // If we're done iterating, that means we must have matched each expression
    // successfully.  Add the bindings (which were created from those matches)
    // to the result list.
    if (cursor == terms->items) {
        results->add(bindings);
        return true;
    }

    // Get the expression that we will now try to match (controlled by 'cursor').
    Decl_Expression *expression = (Decl_Expression *)terms->nth(cursor);
        
    // Match that expression, assuming that the variables in 'bindings' are already
    // bound.  The list 'matched_goals' contains any successful matches.
    List matched_goals;
    direct_match(expression, &matched_goals, bindings);


    // So far, the list 'bindings' represents a partial solution, i.e. there are
    // some variables in there that go some way toward being a solution (but not
    // all the way).  Now, given that those bindings are already in effect,
    // we have gotten back a list of 'matched_goals' any of which could represent
    // a correct answer.

    // So, we copy that binding list, one copy for each of the new 'matched_goals',
    // and then we merge the new bindings from each new match into each copy of
    // the binding list.  So, basically we are "forking" the old solution into
    // a number of new, closer solutions.

    // You could try to do this more optimally (by making data structures that point
    // back to the old set of bindings, rather than copying it) but hey, we're
    // just trying to be simple here.
    Matched_Goal *matched_goal;
    Foreach(&matched_goals, matched_goal) {
        List *new_bindings = merge_bindings(bindings, &matched_goal->bindings);
        bool kept = match_conjunction_series(sequence, cursor + 1, new_bindings, results);
        if (!kept) delete_bindings(new_bindings);
    } Endeach;

    return false;
}

// 'do_sequence' just takes an expression sequence (basically, a bunch
// of expressions connected by logical ANDs) and tries to match it against
// the databse, telling you the results.  Simple queries will have only
// one expression, in which case the sequence is only 1 item long.
void do_sequence(Decl_Expression_Sequence *sequence) {
    List conjunct_results;

    //
    // Ask the goal solver for a match.
    //
    goal_solver->match_conjunction_series(sequence, &conjunct_results);

    // If we didn't get one, print a brief "no results" report.
    if (conjunct_results.items == 0) {
        printf("0 solutions: {}\n");
        return;
    }

    // Print out the number of solutions.
    printf("%d solutions: {\n", conjunct_results.items);

    //
    // Print out each solution...
    //
    List *binding_list;
    Foreach(&conjunct_results, binding_list) {
        printf("    {");
        bool first = true;
        Binding *binding;
        Foreach(binding_list, binding) {
            char *name = binding->variable->name;
            if (!name[0]) continue;  // Ignore the anonymous "?"

            printf("%s%s = %s", first ? "" : ", ",
                   binding->variable->name, 
                   ((Atom *)binding->bound_value)->name);  // XXX Bad
            first = false;
        } Endeach;

        printf("}\n");
    } Endeach;

    printf("}\n");
}



int main(int argc, char *argv[]) {
    // Init stuff

    goal_solver = new Goal_Solver();
    the_parser = new Parser();

    char *db_name = "db.logic";
    char *queries_name = "queries.logic";

    //
    // Start whacking away...
    // If the user supplied an alternate filename for the database or the
    // default query set, use those.
    //

    if (argc > 1) db_name = argv[1];
    if (argc > 2) queries_name = argv[2];

    //
    // Try to open the database file...
    //
    FILE *db_file = fopen(db_name, "rt");
    if (db_file == NULL) {
        fprintf(stderr, "Unable to open file '%s' for reading.\n", db_name);
        exit(1);
    }

    the_parser->set_input_from_file((void *)db_file);

    while (1) {
        // For each assertion we can parse in the database source file, add that 
        // assertion to the database in memory.
        Decl_Assertion *assertion = the_parser->parse_assertion();
        if (!assertion) break;
        goal_solver->assertions.add(assertion);
    }

    fclose(db_file);


    //
    // Try to open the query file...
    //
    FILE *query_file = fopen(queries_name, "rt");
    if (query_file) {
        printf("--- Running queries from file '%s' ---\n", queries_name);

        the_parser->set_input_from_file((void *)query_file);
        while (1) {
            //
            // For each query we can parse, run it and display the results.
            //
            Decl_Expression_Sequence *sequence = the_parser->parse_expression_sequence();
            if (!sequence) break;
            printf("\nRunning query: ");
            print_expression_sequence(sequence);
            printf("\n");
            do_sequence(sequence);
        }
            
        fclose(query_file);
    }

    // 
    // Go into an interactive mode where we parse queries from the standard input,
    // running them and displaying the results.
    //

    printf("\nEntering interactive mode.\n\n");
    
    int turn_index = 1;
    const int INPUT_LEN_MAX = 1000;
    char input_buffer[INPUT_LEN_MAX];
    while (1) {
        // Print prompt
        printf("%d > ", turn_index);
        fflush(stdout);

        // Read input
        fgets(input_buffer, INPUT_LEN_MAX, stdin);
        char *s = input_buffer;
        while (isspace(*s)) s++;
        if (!*s) continue;

        the_parser->set_input_from_string(s);
        
        // Parse input
        Decl_Expression_Sequence *sequence = the_parser->parse_expression_sequence();
        if (!sequence) continue;

        // Run the query
        do_sequence(sequence);

        turn_index++;
    }

    return 0;
}

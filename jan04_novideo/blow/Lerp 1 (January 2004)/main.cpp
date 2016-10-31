#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "goal_solver.h"
#include "interp.h"
#include "schema.h"

#include "parser_private.h"  // XXX Refactor
#include "bytecode.h"  // XXX This too
#include "bytecode_builder.h"  // XXX This too
#include "bytecode_runner.h"  // XXX This too

#include "printer.h"

void do_one_file(Lerp_Interp *interp, char *filename) {
    FILE *f = fopen(filename, "rt");
    if (f == NULL) {
        fprintf(stderr, "Unable to open file '%s' for reading.\n", filename);
        exit(1);
    }

    interp->parser->set_input_from_file((void *)f);

    while (1) {
        Ast *toplevel = interp->parser->parse_toplevel();
        if (!toplevel) break;

        if (interp->parse_error) break;

        switch (toplevel->type) {
        case AST_PROC_DECLARATION: {
            Ast_Proc_Declaration *decl = (Ast_Proc_Declaration *)toplevel;

            Lerp_Bytecode *bytecode = interp->bytecode_builder->build(decl);
            if (bytecode) {
                Procedure *proc = interp->parser->make_procedure(decl->proc_name);
                proc->bytecode = ToBarrier(bytecode);
                bytecode->procedure = ToBarrier(proc);

                if (decl->operator_number == -1) {
                    // Not an operator, just file it under its usual name
                    interp->global_database->assign_named_slot(interp, proc->name, ToBarrierF(proc));
                } else {
                    // It's an operator... make a tuple for this guy.

                    int num_operator_arguments = bytecode->num_arguments;
                    assert((num_operator_arguments == 1) || (num_operator_arguments == 2));
                    int num_elements = num_operator_arguments + 3;

                    Atom *first_type = (Atom *)bytecode->argument_info->read()->values[0]->read();
                    Atom *second_type = NULL;
                    if (num_operator_arguments > 1) second_type = (Atom *)bytecode->argument_info->read()->values[2]->read();
                    assert(first_type->type == ARG_ATOM);
                    if (second_type) assert(second_type->type == ARG_ATOM);

                    Decl_Expression *expression = interp->memory_manager->create_decl_expression(num_elements);
                    expression->initialize_slot(0, decl->proc_name);  // This will just be atom "_operator"
                    expression->initialize_slot(1, interp->parser->make_integer(decl->operator_number));
                    expression->initialize_slot(2, proc);
                    expression->initialize_slot(3, first_type);
                    if (second_type) expression->initialize_slot(4, second_type);

                    interp->global_database->add_assertion(interp, expression);
                }
            }

            break;
        }

        case ARG_DECL_ASSERTION: {
            interp->add_assertion((Decl_Assertion *)toplevel);
            break;
        }

        case ARG_SCHEMA: {
            Schema *space = (Schema *)toplevel;
            interp->global_database->assign_named_slot(interp, space->type_name, ToBarrierF(space));
            break;
        }
/*
        case AST_STRUCT_DECLARATION: {
            // @Incomplete: register this guy somehow (or ignore!)
            break;
        }
*/
/*
        case AST_UNARY_EXPRESSION:
        case AST_BINARY_EXPRESSION:
        case AST_PROCEDURE_CALL_EXPRESSION: {
            Lerp_Bytecode *bytecode = interp->bytecode_builder->build(
            assert(0);
            break;
        }
*/
        default:
            assert(0);  // This will not do, of course!
            break;
        }
    }

    fclose(f);
}


int main(int argc, char *argv[]) {
    // Init stuff

    Lerp_Interp *interp = new Lerp_Interp();

    void register_primitives(Lerp_Interp *interp);
    register_primitives(interp);

    if (argc < 2) {
        fprintf(stderr, "You need to give an argument of a file to process!  Sorry.\n");
        exit(1);
    }

    //
    // Start whacking away
    //

    int i;
    for (i = 1; i < argc; i++) {
        char *filename = argv[i];
        do_one_file(interp, filename);
    }



    if (interp->parse_error) {
        fprintf(stderr, "There were parse errors; we cannot run.\n");
        exit(1);
    }

    if (interp->bytecode_error) {
        fprintf(stderr, "There were bytecode errors; we cannot run.\n");
        exit(1);
    }

    Atom *proc_name = interp->parser->make_atom("main");
    First_Class *fc_proc = interp->global_database->lookup_named_slot(interp, proc_name);
    if (!fc_proc) {
        fprintf(stderr, "There is no definition for 'main'... we can't run.\n");
        exit(1);
    }

    if (fc_proc->type != ARG_IMPERATIVE_PROCEDURE) {
        fprintf(stderr, "'main' is defined but its value is not a procedure!  We can't run.\n");
        exit(1);
    }
    
    Procedure *proc = (Procedure *)fc_proc;

    printf("\n\n\n\n\n\n\n\n");  // This is just to help me spot the new run in a window with lots of scrollback...

    assert(proc->bytecode);
    interp->bytecode_runner->run(proc);

    // interp->printer->print_namespace(interp->global_namespace);

#ifdef NOT
    printf("Entering interactive mode.\n\n");
    
    int turn_index = 1;
    const int INPUT_LEN_MAX = 1000;
    char input_buffer[INPUT_LEN_MAX];
    while (1) {
        printf("%d > ", turn_index);
        fflush(stdout);
        
        fgets(input_buffer, INPUT_LEN_MAX, stdin);
        char *s = input_buffer;
        while (isspace(*s)) s++;
        if (!*s) continue;

        interp->parser->set_input_from_string(s);
        
        Decl_Assertion *sequence = the_parser->parse_assertion();
        if (!sequence) continue;

        do_sequence(sequence);

        turn_index++;
    }
#endif NOT
    return 0;
}

#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "goal_solver.h"
#include "interp.h"

#include "code_manager.h"
#include "profiler_ticks.h"  // @Refactor

#include "bytecode.h"  // @Refactor  ... put thread loop stuff in the right place.
#include "bytecode_runner.h"
#include "thread.h"

int main(int argc, char *argv[]) {
    // Init stuff

    Lerp_Interp *interp = new Lerp_Interp();
    interp->code_manager->load_default_modules();

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
        interp->code_manager->add_file_to_load(NULL, interp->parser->make_string(filename));
    }

    interp->code_manager->perform_loads();


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

    Lerp_Thread *thread = interp->create_thread(proc);
    interp->profiler->set_current_thread(thread);  // @Refactor; just use Bytecode_Runner current_thread?
    interp->profiler->enter_procedure(proc);       // @Refactor

    while (interp->threads.items) {
        interp->run();

        Lerp_Thread *thread = interp->bytecode_runner->current_thread;
        if (thread->flags & THREAD_SHOULD_COMPLETE) {
            interp->threads.remove(thread);  // @Incomplete: Put in inactive thread list!!
        }
    }

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

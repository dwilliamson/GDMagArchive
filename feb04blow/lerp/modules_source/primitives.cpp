#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"
#include "goal_solver.h"
#include "printer.h"
#include "memory_manager_private.h"
#include "profiler_ticks.h"

#include "unicode.h"  // For string stuff
#include "module_helper.h"
#include "thread.h"

#include <math.h>

extern "C" {
    DLL_EXPORT void get_module_info(Lerp_Module_Init_Info *info_return);
    DLL_EXPORT Database *instantiate(Lerp_Interp *);
    DLL_EXPORT void init(Lerp_Interp *);
    DLL_EXPORT void shutdown(Lerp_Interp *);
    DLL_EXPORT void enumerate_gc_roots(Lerp_Interp *);
};






void proc_sleep(Lerp_Interp *interp, Lerp_Call_Record *record); // Defined in primitives_win32.cpp

void proc_print(Lerp_Interp *interp, Lerp_Call_Record *record) {
    int i;
    for (i = 1; i < record->num_registers; i++) {
        interp->printer->print_value(record->registers[i]->read());
    }

    interp->printer->output_buffer();

    // Return the number of items we printed.
    Return(interp->parser->make_integer(record->num_registers));
}

void proc_concat(Lerp_Interp *interp, Lerp_Call_Record *record) {
    int i;
    for (i = 1; i < record->num_registers; i++) {
        interp->printer->print_value(record->registers[i]->read());
    }

    char *data = interp->printer->get_buffer();
    First_Class *result = interp->parser->make_string(data);
    delete [] data;
    Return(result);
}




void proc_get_root_database(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 0);
    if (!success) return;

    Return(interp->global_database);
}

void proc_sqrt(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 1);
    if (!success) return;
    
    double value;
    success = coerce_to_float(interp, record, 0, &value);
    if (!success) return;
    
    if (value < 0) {
        interp->report_error("In sqrt: argument must be >= 0.\n");
        return;
    }

    Return(interp->parser->make_float(sqrt(value)));
}

void proc_cos(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 1);
    if (!success) return;
    
    double value;
    success = coerce_to_float(interp, record, 0, &value);
    if (!success) return;

    Return(interp->parser->make_float(cos(value)));
}

void proc_sin(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 1);
    if (!success) return;
    
    double value;
    success = coerce_to_float(interp, record, 0, &value);
    if (!success) return;

    Return(interp->parser->make_float(sin(value)));
}


void proc_seconds_since_startup(Lerp_Interp *interp, Lerp_Call_Record *record) {
    double seconds = interp->get_time_in_seconds();
	Return(interp->parser->make_float(seconds));
}


void proc_string_length(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 1);
    if (!success) return;

    char *string;
    success = get_string(interp, record, 0, &string);
    if (!success) {
        interp->report_error("Argument must be a string.\n");
        return;
    }

    Return(interp->parser->make_integer(Unicode::length_in_characters(string)));
}

void proc_string_nth_as_character(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 2);
    if (!success) return;

    char *string;
    success = get_string(interp, record, 0, &string);
    if (!success) {
        interp->report_error("Argument 1 must be a string.\n");
        return;
    }

    int index;
    success = coerce_to_integer(interp, record, 1, &index);
    if (!success) {
        interp->report_error("Argument 2 must be an integer.\n");
        return;
    }

    if (index < 1) {
        interp->report_error("Index must be greater than or equal to 1.\n");
        return;
    }

    unsigned char *cursor = (unsigned char *)string;
    int cursor_character_index = 0;
    while (1) {
        if (*cursor == '\0') {
            interp->report_error("Index is beyond the end of the string (string is %d characters long).\n", cursor_character_index);
            return;
        }

        cursor_character_index++;
        if (cursor_character_index == index) {
            UTF32 utf32;
            Conversion_Result result = Unicode::character_utf8_to_utf32(cursor, &utf32);
            if (result != UNICODE_CONVERSION_OK) {
                interp->report_error("String contains invalid characters.\n");
                return;
            }

            Return(interp->parser->make_integer(utf32));
        }

        int forward = 1 + Unicode::trailingBytesForUTF8[*cursor];
        cursor += forward;
    }
}

void proc_string_nth_as_string(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 2);
    if (!success) return;

    char *string;
    success = get_string(interp, record, 0, &string);
    if (!success) {
        interp->report_error("Argument 1 must be a string.\n");
        return;
    }

    int index;
    success = coerce_to_integer(interp, record, 1, &index);
    if (!success) {
        interp->report_error("Argument 2 must be an integer.\n");
        return;
    }

    int len_max = Unicode::length_in_characters(string);  // @Speed: store this!?!
    if (index < 1) {
        interp->report_error("Index must be greater than or equal to 1.\n");
        return;
    }

    if (index > len_max) {
        interp->report_error("Index is too high (string is %d characters long, but you tried to index it at %d.)\n", len_max, index);
        return;
    }

    Return(interp->parser->make_integer(string[index-1]));
}

void proc_thread_create(Lerp_Interp *interp, Lerp_Call_Record *record) {
    Lerp_Thread *new_thread = interp->create_thread();
    Lerp_Thread *old_thread = interp->bytecode_runner->current_thread;

    assert(old_thread);
    new_thread->context = ToBarrier(interp->bytecode_runner->copy_context(old_thread->context->read()));

    // Put a 0 into the return value of old_thread, and a 1 into the
    // return value of new_thread.  That way we can tell them apart at
    // runtime.
    Lerp_Call_Record *old_context = old_thread->context->read();
    Lerp_Call_Record *new_context = new_thread->context->read();

    // We return the value from the main thread by writing it into 'record'
    // (the caller will take care of transferring it into the old context).
    // We stuff it into register 0 of 'record'.
    record->registers[0] = ToBarrierF(interp->parser->make_integer(0));

    // The caller doesn't know how to deal with any new thread, so we
    // pre-return the value manually... the proper slot has been stuffed into
    // the 'register_for_return_value' slot of 'record'.
    int index = record->register_for_return_value;
    new_context->registers[index] = ToBarrierF(interp->parser->make_integer(1));
}

void proc_thread_yield(Lerp_Interp *interp, Lerp_Call_Record *record) {
    Lerp_Thread *thread = interp->bytecode_runner->current_thread;
    assert(thread);
    thread->flags |= THREAD_SHOULD_YIELD;
}

void proc_thread_end(Lerp_Interp *interp, Lerp_Call_Record *record) {
    Lerp_Thread *thread = interp->bytecode_runner->current_thread;
    assert(thread);
    thread->flags |= THREAD_SHOULD_COMPLETE;
}


void proc_print_allocations(Lerp_Interp *interp, Lerp_Call_Record *record) {
    interp->memory_manager->print_allocations();
}

void proc_gc(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 0);
    if (!success) return;

    interp->memory_manager->gc();

/*
    Memory_Arena *oldspace = interp->memory_manager->memory_oldspace;
    Memory_Arena *newspace = interp->memory_manager->memory_newspace;

    int s0 = oldspace->size - oldspace->size_remaining;
    int s1 = newspace->size - newspace->size_remaining;

    printf("Before GC: %d bytes used, after GC: %d bytes used (%d bytes freed)\n", s0, s1, s0 - s1);
*/
}

void proc_profiler_update(Lerp_Interp *interp, Lerp_Call_Record *record) {
    bool success = argument_count(interp, record, 0);
    if (!success) return;

    interp->profiler->update();
}





void get_module_info(Lerp_Module_Init_Info *info) {
    info->system_version = LERP_SYSTEM_VERSION;
    info->default_name = "Primitives";
};

void init(Lerp_Interp *interp) {
}

Database *instantiate(Lerp_Interp *interp) {
    Database *db = GC_NEW(Database);

    Register(print);
    Register(concat);
    Register(sleep);
    Register(get_root_database);
    Register(seconds_since_startup);

    Register(sqrt);
    Register(sin);
    Register(cos);
    Register(print_allocations);
    Register(gc);

    Register(string_length);
    Register(string_nth_as_character);
    Register(string_nth_as_string);

    Register(thread_yield);
    Register(thread_create);
    Register(thread_end);

    // Make the Profiler type...
    {
        Schema *ns = make_schema(interp, 15);
        int schema_cursor = 0;

        MEMBER_PROC2(update, profiler_update);
        ns->type_name = interp->parser->make_atom("Profiler_Type");
        trim_schema(interp, ns, schema_cursor);
        First_Class *profiler = interp->instantiate(ns);
        Bind("Profiler", profiler);
    }

    return db;
}

void shutdown(Lerp_Interp *) {
}

void enumerate_gc_roots(Lerp_Interp *) {
}


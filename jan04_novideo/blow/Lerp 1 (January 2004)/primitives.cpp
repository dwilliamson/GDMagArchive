#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"
#include "primitives_private.h"
#include "goal_solver.h"
#include "printer.h"
#include "memory_manager_private.h"
#include "profiler_ticks.h"

#include "unicode.h"  // For string stuff
#include "module_macros.h"

#include <math.h>

void proc_sleep(Lerp_Interp *interp, Lerp_Call_Record *record);
void proc_seconds_since_startup(Lerp_Interp *interp, Lerp_Call_Record *record);

bool argument_count(Lerp_Interp *interp, Lerp_Call_Record *record, int count) {
    if (count != record->num_registers - 1) {
        interp->report_error("Incorrect number of arguments (to unknown), wanted %d, got %d\n", count, record->num_registers-1);  // @Robustness: replace 'unknown'
        return false;
    }

    return true;
}

bool coerce_to_integer(Lerp_Interp *interp, Lerp_Call_Record *record, int index, int *result) {
    First_Class *value = record->registers[index+1]->read();
    if (value->type != ARG_INTEGER) {
        interp->report_error("Type error, expected INTEGER (WHERE????)\n");
        return false;
    }

    *result = ((Integer *)value)->value;
    return true;
}

bool coerce_to_float(Lerp_Interp *interp, Lerp_Call_Record *record, int index, double *result) {
    First_Class *value = record->registers[index+1]->read();

    if (value->type == ARG_INTEGER) {
        *result = (float)(((Integer *)value)->value);
        return true;
    }

    if (value->type == ARG_FLOAT) {
        *result = (float)(((Float *)value)->value);
        return true;
    }

    interp->report_error("Type error, expected INTEGER (WHERE????)\n");
    return false;
}

bool coerce_to_float(Lerp_Interp *interp, Lerp_Call_Record *record, int index, float *result) {
    double dvalue;
    bool success = coerce_to_float(interp, record, index, &dvalue);
    if (!success) return false;

    *result = (float)dvalue;
    return true;
}


bool get_string(Lerp_Interp *interp, Lerp_Call_Record *record, int index, char **result) {
    First_Class *value = record->registers[index+1]->read();

    if (value->type == ARG_STRING) {
        *result = ((String *)value)->value;
        return true;
    }

    interp->report_error("Type error, expected STRING (WHERE????)\n");
    return false;
}





//
// @Robustness: Do we want to have get_integer_member and get_string_member
// spit error messages?
//
 
bool get_integer_member(Lerp_Interp *interp, Lerp_Call_Record *record, Atom *name, int *result) {
    Database *owner = record->this_pointer->read();
    First_Class *ptr = owner->lookup_named_slot(interp, name);
    if (!ptr) return false;

    if (ptr->type != ARG_INTEGER) return false;

    Integer *arg = (Integer *)ptr;
    *result = arg->value;
    return true;
}

bool get_string_member(Lerp_Interp *interp, Lerp_Call_Record *record, Atom *name, char **result) {
    Database *owner = record->this_pointer->read();
    First_Class *ptr = owner->lookup_named_slot(interp, name);
    if (!ptr) return false;

    if (ptr->type != ARG_STRING) return false;

    String *arg = (String *)ptr;
    *result = arg->value;
    return true;
}



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
        interp->report_error("Index is too high (string is %d characters long, but you tried to index it at %d.\n", len_max, index);
        return;
    }

    Return(interp->parser->make_integer(string[index-1]));
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



Procedure *make_procedure(Lerp_Interp *interp, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *)) {
    Procedure *result;

    Atom *name = interp->parser->make_atom(name_string);

    result = GC_NEW(Procedure);
    result->type = ARG_IMPERATIVE_PROCEDURE;
    result->name = name;
    result->proc = proc;
    result->bytecode = NULL;

    return result;
}

void register_proc(Lerp_Interp *interp, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *)) {

    Procedure *result = make_procedure(interp, name_string, proc);
    interp->global_database->assign_named_slot(interp, result->name, ToBarrierF(result));
}

extern void register_opengl_primitives(Lerp_Interp *interp);

void register_primitives(Lerp_Interp *interp) {
    void primitives_os_init();
    primitives_os_init();

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
//    Register(register_number);

    Register(string_length);
    Register(string_nth_as_character);
    Register(string_nth_as_string);


    {
        Schema *ns = make_schema(interp, 15);
        int schema_cursor = 0;

        MEMBER_PROC2(update, profiler_update);
        ns->type_name = interp->parser->make_atom("Profiler_Type");
        trim_schema(ns, schema_cursor);
        First_Class *profiler = interp->instantiate(ns);
        bind_value(interp, "Profiler", profiler);
    }


    register_opengl_primitives(interp);
}

void trim_schema(Schema *schema, int length) {
    Value_Array *array = schema->named_slots->read();
    assert(array);

    length *= 2;
    assert(array->num_values >= length);
    array->num_values = length;
}


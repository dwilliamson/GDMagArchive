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

#include <math.h>

LEXPORT bool argument_count(Lerp_Interp *interp, Lerp_Call_Record *record, int count) {
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

void register_proc(Lerp_Interp *interp, Database *db, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *)) {

    Procedure *result = make_procedure(interp, name_string, proc);
    db->assign_named_slot(interp, result->name, ToBarrierF(result));
}

Procedure *make_procedure(Lerp_Interp *interp, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *)) {
    Procedure *result;

    Atom *name = interp->parser->make_atom(name_string);

    result = GC_NEW(Procedure);
    result->type = ARG_IMPERATIVE_PROCEDURE;
    result->name = name;
    result->proc = proc;
    result->bytecode = NULL;
    result->hash_key = interp->make_procedure_hash_key();

    return result;
}


void trim_schema(Lerp_Interp *interp, Schema *schema, int length) {
    Value_Array *array = schema->named_slots->read();
    assert(array);

    length *= 2;
    assert(array->num_values >= length);
    array->num_values = length;
}


Schema *make_schema(Lerp_Interp *interp, int num_slots) {
    Schema *schema = GC_NEW(Schema);
    if (num_slots) {
        Value_Array *array = interp->memory_manager->create_value_array(num_slots * 2);

        int i;
        for (i = 0; i < num_slots * 2; i++) array->values[i] = NULL;
        schema->named_slots = ToBarrier(array);
    }
    ;
    return schema;
}

void bind_value(Lerp_Interp *interp, Database *db, char *name_string, First_Class *value) {
    Atom *name = interp->parser->make_atom(name_string);
    db->assign_named_slot(interp, name, value);
}


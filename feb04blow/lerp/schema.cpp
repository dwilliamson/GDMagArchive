#include "general.h"
#include "schema.h"
#include "interp.h"

void Schema::initialize_slot(int index, Atom *name, First_Class *value) {
    // @WriteBarrier
    
    Value_Array *slots = named_slots->read();
    assert(index*2+1 < slots->num_values);
    
    slots->values[index*2+0] = ToBarrierF(name);
    slots->values[index*2+1] = ToBarrier(value);
}

/*
Lvalue *Schema::find_lvalue(Lerp_Interp *interp, Atom *name) {
    Lvalue *lvalue = bindings->read();
    while (lvalue) {
        if (lvalue->name == name) return lvalue;
        lvalue = lvalue->next->read();
    }

    return NULL;
}

Lvalue *Schema::find_or_create_lvalue(Lerp_Interp *interp, Atom *name) {
    Lvalue *lvalue = find_lvalue(interp, name);
    if (lvalue) return lvalue;

    lvalue = GC_NEW(Lvalue);
    lvalue->type = ARG_LVALUE;
    lvalue->name = name;
    lvalue->bound_value = NULL;
    lvalue->type_atom = NULL;

    num_bindings++;
    lvalue->next = bindings;
    bindings = ToBarrier(lvalue);

    return lvalue;
}
*/

void Database::clear() {
    assertions = NULL;
}

static Decl_Expression *find_var_expression(Lerp_Interp *interp, Database *db, Atom *name) {
    Decl_Assertion *assertion = db->assertions->read();
    while (assertion) {
        Decl_Expression *exp = assertion->expression->read();
        if (exp && exp->num_arguments == 3) {
            if ((exp->arguments[0]->read() == interp->member_atom) &&
                (exp->arguments[1]->read() == name)) {
                return exp;
            }
        }

        assertion = assertion->next->read();
    }

    return NULL;
}

void Database::assign_named_slot(Lerp_Interp *interp, Atom *name, First_Class *value) {
    Decl_Expression *expr = find_var_expression(interp, this, name);
    if (!expr) {
        expr = interp->memory_manager->create_decl_expression(3);
        expr->initialize_slot(0, interp->member_atom);
        expr->initialize_slot(1, name);
        expr->initialize_slot(2, value);
        add_assertion(interp, expr);
    } else {
        expr->initialize_slot(2, value);
    }
}

void Database::assign_named_slot(Lerp_Interp *interp, Atom *name, Barrier <First_Class *> *value) {
    assign_named_slot(interp, name, value->read());
}

First_Class *Database::lookup_named_slot(Lerp_Interp *interp, Atom *name) {
    Decl_Expression *expr = find_var_expression(interp, this, name);
    if (expr) return expr->arguments[2]->read();
    return NULL;
}


void Database::add_assertion(Lerp_Interp *interp, First_Class *arg1) {
    Decl_Expression *expression = interp->memory_manager->create_decl_expression(1);
    expression->initialize_slot(0, arg1);
    add_assertion(interp, expression);
}

void Database::add_assertion(Lerp_Interp *interp, First_Class *arg1, First_Class *arg2) {
    Decl_Expression *expression = interp->memory_manager->create_decl_expression(2);
    expression->initialize_slot(0, arg1);
    expression->initialize_slot(1, arg2);
    add_assertion(interp, expression);
}


void Database::add_assertion(Lerp_Interp *interp, First_Class *arg1, First_Class *arg2, First_Class *arg3) {
    Decl_Expression *expression = interp->memory_manager->create_decl_expression(3);
    expression->initialize_slot(0, arg1);
    expression->initialize_slot(1, arg2);
    expression->initialize_slot(2, arg3);
    add_assertion(interp, expression);
}

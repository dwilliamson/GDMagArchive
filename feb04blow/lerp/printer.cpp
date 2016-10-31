#include "general.h"
#include "printer.h"
#include "schema.h"
#include "interp.h"
#include "concatenator.h"

char *argument_type_names[ARG_NUM_TYPES] = {
    "atom",
    "variable",
    "integer",
    "float",
    "string",
    "database",
    "lvalue",
    "namespace",
    "blob",
    "assertion",
    "expression",
    "assertion",
    "imperative_procedure",
    "imperative_calling_record"
};

extern void print_value(Lerp_Interp *interp, First_Class *value);

Printer::Printer(Lerp_Interp *_interp) {
    indent_level = 0;
    interp = _interp;
    concatenator = new Concatenator();
    debugger_mode = false;
}

Printer::~Printer() {
    delete concatenator;
}

void Printer::print_indent() {
    int i;
    for (i = 0; i < indent_level; i++) {
        concatenator->add(" ");
    }
}

void Printer::indent(int num_spaces) {
    indent_level += num_spaces;
    assert(indent_level >= 0);
}

void Printer::print_namespace(Schema *space) {
    if (!space->named_slots) return;

    Value_Array *array = space->named_slots->read();
    concatenator->printf("Schema has %d bindings:\n", array->num_values);
    indent(4);

/*
    Lvalue *lvalue;
    Foreach(&space->bindings, lvalue) {
        print_indent();
        concatenator->add(lvalue->name->name);
        concatenator->add(": ");

        First_Class *value = lvalue->bound_value->read();
        print_value(value);
        if (value->type < ARG_FIRST_CUSTOM_TYPE) {
            concatenator->printf(" (%s)\n", argument_type_names[value->type]);
        }
    } Endeach;
*/
    indent(-4);
}







void Printer::print_expression(Decl_Expression *expression) {
    concatenator->add("[");

    First_Class *argument;
    int i;
    for (i = 0; i < expression->num_arguments; i++) {
        argument = expression->arguments[i]->read();
        if (i) concatenator->add(" ");
        interp->printer->print_value(argument);
    }

    concatenator->add("]");
}

void Printer::print_assertion(Decl_Assertion *assertion) {
    if (assertion->expression) {
        print_expression(assertion->expression->read());
        if (assertion->conditionals) {
            concatenator->add(" <- ");
        }
    }

    Decl_Expression *expression = assertion->conditionals->read();
    bool first = true;
    while (expression) {
        if (!first) concatenator->add(", ");
        first = false;

        print_expression(expression);
        expression = expression->next->read();
    }
}

void Printer::print_constraint(Decl_Constraint *constraint) {
    concatenator->add("*** one constraint\n");
}

void Printer::print_database(Database *db) {
    Schema *space = db->schema->read();

    if (space) {
//        print_indent();
        char *type_name = NULL;
        if (space->type_name) type_name = space->type_name->name;
        else type_name = "*unknown*";

        concatenator->printf("struct %s", type_name);
//        indent(4);
    }

/* Ignoring the actual schema contents for now....
        int cursor = 0;
        Lvalue *lvalue = space->bindings->read();
        while (lvalue) {
            print_indent();
            concatenator->printf("%s: ", lvalue->name->name);
            First_Class *value = read_nth_member(fc, cursor);
            print_value(value);
            concatenator->add("\n");
            cursor++;
            lvalue = lvalue->next->read();
        } 

        print_indent();
        concatenator->add("DATABASE CONSTRAINTS:\n");
        indent(4);

        Decl_Constraint *constraint;
        Foreach(&space->db_constraints, constraint) {
            print_indent();
            print_constraint(constraint);
        } Endeach;

        indent(-4);
        print_indent();
        concatenator->add("\n");
*/


/*
    print_indent();
    concatenator->add("ASSERTIONS:\n");
    indent(2);

    Decl_Assertion *assertion = db->assertions->read();
    while (assertion) {
        print_indent();
        print_assertion(assertion);
        concatenator->add("\n");
        assertion = assertion->next->read();
    }

    indent(-2);

    if (space) {
        indent(-4);
        print_indent();
        concatenator->add("}\n");
    }
*/
}

void Printer::print_value(First_Class *value) {
    switch (value->type) {
    case ARG_ATOM:
        concatenator->printf("%s", ((Atom *)value)->name);
        break;
    case ARG_VARIABLE:
        concatenator->printf("?%s", ((Variable *)value)->name);
        break;
    case ARG_INTEGER:
        concatenator->printf("%d", ((Integer *)value)->value);
        break;
    case ARG_FLOAT:
        concatenator->printf("%lf", ((Float *)value)->value);
        break;
    case ARG_STRING:
        if (debugger_mode) concatenator->printf("\"%s\"", ((String *)value)->value);
        else concatenator->printf("%s", ((String *)value)->value);
        break;
    case ARG_DECL_ASSERTION:
        print_assertion((Decl_Assertion *)value);
        break;
    case ARG_DECL_EXPRESSION:
        print_expression((Decl_Expression *)value);
        break;
    case ARG_IMPERATIVE_PROCEDURE:
        concatenator->add("#imp_pro");
        break;
    case ARG_IMPERATIVE_CALL_RECORD:
        concatenator->add("#imp_call_rec");
        break;
    case ARG_DATABASE:
        print_database((Database *)value);
        break;
    case ARG_SCHEMA:
        concatenator->add("#namespace");
        break;
    case ARG_BLOB_UNMANAGED:
        concatenator->add("#blob");
        break;
    case ARG_UNINITIALIZED:
        concatenator->add("--- UNINITIALIZED ---");
        break;
    default:
        assert(0);
        break;
    }
}

void Printer::output_buffer() {
    char *data;
    
    data = concatenator->get_result();
    concatenator->reset();

    fputs(data, stdout);

    delete [] data;
}


char *Printer::get_buffer() {
    char *data;
    
    data = concatenator->get_result();
    concatenator->reset();

    return data;
}

#include "general.h"
#include "goal_solver.h"
#include "parser.h"
#include "interp.h"
#include "bytecode_builder.h"
#include "bytecode_runner.h"
#include "schema.h"
#include "lexer.h"
#include "hash_table.h"
#include "printer.h"

#include "mprintf.h"
#include "bytecode.h"  // So we can do stack dump
#include "disassembler.h"

#include "profiler_ticks.h"

#include <stdlib.h>

Lerp_Interp::Lerp_Interp() {
    memory_manager = new Lerp_Memory_Manager(this);
    goal_solver = new Goal_Solver(this);
    parser = new Parser(this);
    bytecode_builder = new Lerp_Bytecode_Builder(this);
    bytecode_runner = new Lerp_Bytecode_Runner(this);
    disassembler = new Lerp_Disassembler(this);
    profiler = new Lerp_Profiler(this);

    Lerp_Interp *interp = this;  // For GC_NEW below

//    uninitialized = GC_NEW(Uninitialized); XXXXXXX @Memory @GC
    uninitialized = new Uninitialized();
    uninitialized->gc_color = GC_COLOR_RED;

//    the_null_blob = GC_NEW(Blob);   // @GC
    the_null_blob = new Blob();
    the_null_blob->size_in_bytes = sizeof(the_null_blob);

    the_anonymous_variable = parser->make_variable("");

    global_database = GC_NEW(Database);


    printer = new Printer(this);

    
    next_custom_type_tag = 1;  // 0 is reserved for "none".
    custom_types_by_id = new Integer_Hash_Table(50);


    int i;
    for (i = 0; i < ARG_NUM_TYPES; i++) {
        type_atoms[i] = NULL;
    }

    member_atom = parser->make_atom("_member");

    type_atoms[ARG_ATOM] = parser->make_atom("Atom");
    type_atoms[ARG_VARIABLE] = parser->make_atom("Variable");
    type_atoms[ARG_INTEGER] = parser->make_atom("Integer");
    type_atoms[ARG_FLOAT] = parser->make_atom("Float");
    type_atoms[ARG_STRING] = parser->make_atom("String");
    type_atoms[ARG_DATABASE] = parser->make_atom("Database");
    type_atoms[ARG_SCHEMA] = parser->make_atom("Schema");
    type_atoms[ARG_BLOB] = parser->make_atom("Blob");
    type_atoms[ARG_DECL_ASSERTION] = parser->make_atom("Decl_Assertion");
    type_atoms[ARG_DECL_EXPRESSION] = parser->make_atom("Decl_Expression");
    type_atoms[ARG_DECL_EXPRESSION_PROCEDURAL] = parser->make_atom("Decl_Expression_Procedural");
    type_atoms[ARG_DECL_ASSERTION] = parser->make_atom("Decl_Assertion");
    type_atoms[ARG_IMPERATIVE_PROCEDURE] = parser->make_atom("Procedure");
    type_atoms[ARG_IMPERATIVE_CALL_RECORD] = parser->make_atom("Call_Record");
    type_atoms[DECL_QUALIFIER_DOMAIN] = parser->make_atom("Decl_Qualifier_Domain");
    type_atoms[ARG_UNINITIALIZED] = parser->make_atom("Uninitialized");
    type_atoms[ARG_FORWARDING_POINTER] = parser->make_atom("!!!!ForwardingPointer!!!!");
    type_atoms[ARG_BYTECODE] = parser->make_atom("Bytecode");
    type_atoms[ARG_BINDING] = parser->make_atom("Binding");
    type_atoms[ARG_MATCHED_GOAL] = parser->make_atom("Matched_Goal");
    type_atoms[ARG_VALUE_ARRAY] = parser->make_atom("Value_Array");
    type_atoms[ARG_VALUE_PAIR] = parser->make_atom("Value_Pair");

    for (i = 0; i < ARG_NUM_TYPES; i++) {
        assert(type_atoms[i] != NULL);
    }

    parse_error = false;
    runtime_error = false;
    bytecode_error = false;

    generate_debug_info = true;
}

Lerp_Interp::~Lerp_Interp() {
    delete goal_solver;
    delete parser;
    delete bytecode_builder;
    delete bytecode_runner;
    delete custom_types_by_id;
    delete printer;
}

void Lerp_Interp::add_assertion(Decl_Assertion *assertion) {
    assertion->next = ToBarrier(global_database->assertions->read());
    global_database->assertions = ToBarrier(assertion);
}

Database *Lerp_Interp::instantiate(Schema *space) {
    if (space->type_tag == 0) {
        unsigned long tag = next_custom_type_tag++;
        assert(next_custom_type_tag > tag);  // No overflow!!

        space->type_tag = tag;
        custom_types_by_id->add(tag, space);
    }

    int old_count = space->instantiation_count;
    space->instantiation_count++;
    assert(space->instantiation_count > old_count);  // No overflow!

    Lerp_Interp *interp = this;  // For GC_NEW
    Database *result = GC_NEW(Database);
    result->schema = ToBarrier(space);

    Value_Array *array = space->named_slots->read();
    if (array) {
        int array_length = array->num_values;
    
    // @GC We are not really being careful here about preserving the
    // read barrier across gc...

        int cursor = 0;
        while (cursor < array_length) {
            Atom *name = (Atom *)array->values[cursor]->read();
            assert(name->type == ARG_ATOM);

            result->assign_named_slot(interp, name, array->values[cursor + 1]);

            cursor += 2;
        }
    }

    return result;
}

/*
First_Class **Lerp_Interp::get_address_of_member(First_Class *owner, Schema *owner_namespace, Atom *member) {
    int index = -1;
    int cursor = 0;

    Lvalue *lvalue = owner_namespace->bindings->read();
    while (lvalue) {
        if (lvalue->name == member) {
            index = cursor;
            break;
        }

        cursor++;
        lvalue = lvalue->next->read();
    }

    if (index == -1) return NULL;

    First_Class **base = ((First_Class **)owner) + 1;
    return base + index;
}

Database *Lerp_Interp::get_database_pointer(First_Class *value, Schema *space) {
    int index = space->num_bindings;
    First_Class **base = ((First_Class **)value) + 1;

    First_Class *result = *(base + index);

    assert(result->type == ARG_DATABASE);
    return (Database *)result;
}

Database *Lerp_Interp::get_database(First_Class *value) {
    Schema *space = get_owner_namespace(value);
    if (!space) return NULL;
    if (!(space->flags & NS_FLAG_HAS_IMPLICIT_DATABASE)) return NULL;

    Database *db = get_database_pointer(value, space);
    assert(db);

    return db;
}
*/

void error_hook() {
}

void Lerp_Interp::stack_dump_one_context(Lerp_Call_Record *context) {
    if (!context->bytecode) {
        printf("    (Weird no bytecode)\n");
        return;
    }

    printf("    %s\n", context->bytecode->read()->name->name);
}

void Lerp_Interp::stack_dump() {
    printf("Stack:\n");
    Lerp_Call_Record *context = bytecode_runner->current_context->read();
    assert(context);

    while (context) {
        stack_dump_one_context(context);
//        disassembler->disassemble(context);
        context = context->previous_context->read();
    }
}

int Lerp_Interp::find_current_line_number() {
    if (!bytecode_runner->current_context) return -1;

    Lerp_Bytecode *bytecode = bytecode_runner->current_context->read()->bytecode->read();
    if (!bytecode) return -1;
    if (!bytecode->debug_info) return -1;

    int pc = bytecode_runner->current_context->read()->program_counter;

    int line_number = -1;
    Statement_Debug_Info *info;
    Foreach(bytecode->debug_info, info) {
        if (info->program_counter >= pc) break;
        line_number = info->line_number;
    } Endeach;

    return line_number;
}

void Lerp_Interp::report_error(char *format, ...) {
    fprintf(stderr, "--------\n");

    int line = find_current_line_number();
    if (line == -1) {
        fprintf(stderr, "Runtime error:\n");
    } else {
        fprintf(stderr, "Runtime error at line %d:\n", line);
    }

    va_list ap;
    va_start(ap, format);

    char *result = mprintf_valist(format, ap);
    va_end(ap);

    fputs(result, stderr);
    fprintf(stderr, "--------\n");

    delete [] result;
    runtime_error = true;

    stack_dump();

    error_hook();
}

void Lerp_Interp::report_bytecode_error(char *format, ...) {
    fprintf(stderr, "--------\n");

    if (bytecode_builder->statement_debug_info_list.items) {
        Statement_Debug_Info *info = (Statement_Debug_Info *)bytecode_builder->statement_debug_info_list.peek_at_tail();
        fprintf(stderr, "Bytecode error at line %d:\n",
                info->line_number);
    } else {
        fprintf(stderr, "Bytecode error:\n");
    }

    va_list ap;
    va_start(ap, format);

    char *result = mprintf_valist(format, ap);
    va_end(ap);

    fputs(result, stderr);
    delete [] result;

    fprintf(stderr, "\n--------\n");
    bytecode_error = true;

    error_hook();
}

void Lerp_Interp::report_error(Token *token, char *format, ...) {
    fprintf(stderr, "--------\n");

    int line, character;
    if (token) {
        line = token->line_number;
        character = token->character_index;
    } else {
        line = parser->lexer->current_line_number;
        character = parser->lexer->current_character_index;
    }

    fprintf(stderr, "Parse error (line %d, character %d):\n", line, character);

    va_list ap;
    va_start(ap, format);

    char *result = mprintf_valist(format, ap);
    va_end(ap);

    fputs(result, stderr);
    fprintf(stderr, "--------\n");

    delete [] result;
    parse_error = true;

    error_hook();
}


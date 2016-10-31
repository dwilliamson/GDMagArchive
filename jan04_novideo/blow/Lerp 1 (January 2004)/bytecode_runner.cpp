#include "general.h"
#include "bytecode_runner.h"
#include "bytecode.h"
#include "interp.h"
#include "parser.h"
#include "schema.h"
#include "goal_solver.h"  // Gotta do database searches!

#include "parser_private.h"  // To get at ASSERTION_FLAGS... @Refactor
#include "printer.h"  // For debugging

#include "unicode.h"
#include "profiler_ticks.h"

//
// Note: Should I do something about the C++ = vs ==?
//

void add_db_assertion(Database *db, Decl_Assertion *assertion) {
    Decl_Expression *expression = assertion->expression->read();
    assert(expression);
    assert(expression->num_arguments > 0);
    assertion->next = ToBarrier(db->assertions->read());
    db->assertions = ToBarrier(assertion);
}

Lerp_Bytecode_Runner::Lerp_Bytecode_Runner(Lerp_Interp *_interp) {
    interp = _interp;
    current_context = NULL;
}

Lerp_Bytecode_Runner::~Lerp_Bytecode_Runner() {
}


int Lerp_Bytecode_Runner::unpack_register(int value) {
    char *ptr = current_context->read()->bytecode->read()->data + value;
    short *index_ptr = (short *)ptr;
    int result = *index_ptr;
    return result;
}

int Lerp_Bytecode_Runner::unpack_byte(int value) {
    char *ptr = current_context->read()->bytecode->read()->data + value;
    return *ptr;
}

First_Class *Lerp_Bytecode_Runner::read_register(int register_index) {
    Lerp_Call_Record *context = current_context->read();
    assert(register_index < context->num_registers);

    return context->registers[register_index]->read();
}

void Lerp_Bytecode_Runner::write_register(int register_index, First_Class *value) {
    assert(value != NULL);

    Lerp_Call_Record *context = current_context->read();
    assert(register_index < context->num_registers);

    context->registers[register_index] = ToBarrier(value);
}




inline bool get_integer(First_Class *fc, int *result) {
    if (fc->type != ARG_INTEGER) return false;
    *result = ((Integer *)fc)->value;
    return true;
}

bool Lerp_Bytecode_Runner::get_integer_math_args(First_Class *a1, First_Class *a2,
                                                 int *i1_return, int *i2_return) {
    bool success;
    success = get_integer(a1, i1_return);
    if (!success) return false;

    success = get_integer(a2, i2_return);
    if (!success) return false;

    return true;
}


inline bool get_float(First_Class *fc, double *result) {
    if (fc->type == ARG_INTEGER) {
        *result = (double)(((Integer *)fc)->value);
        return true;
    }

    if (fc->type != ARG_FLOAT) return false;
    *result = ((Float *)fc)->value;
    return true;
}

bool Lerp_Bytecode_Runner::get_float_math_args(First_Class *a1, First_Class *a2,
                                               double *f1_return, double *f2_return) {
    bool success;
    success = get_float(a1, f1_return);
    if (!success) return false;

    success = get_float(a2, f2_return);
    if (!success) return false;

    return true;
}

inline First_Class *Lerp_Bytecode_Runner::make_integer(int value) {
    return interp->parser->make_integer(value);
}

inline First_Class *Lerp_Bytecode_Runner::make_float(double value) {
    return interp->parser->make_float(value);
}

void Lerp_Bytecode_Runner::run_unop() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 6;

    int operator_type = current_context->read()->bytecode->read()->data[pc + 1];
    assert(operator_type < LERP_UNOP_LIMIT);

    int result_register = unpack_register(pc + 2);
    int source_register = unpack_register(pc + 4);

    First_Class *source = read_register(source_register);
    First_Class *result = NULL;

    // @Semantics: There is a bit of an inconsistency here, I am checking for
    // user-specified unops before doing the default integer/float thing, but checking
    // afterward for binops (meaning you can't override binop +, etc for primitive
    // types).  This ought to get cleaned up one way or the other, eventually.


    if (operator_type == LERP_UNOP_MINUS) {
        double float_value;
        int int_value;

        bool success = get_integer(source, &int_value);
        if (!success) {
            success = get_float(source, &float_value);
            if (!success) {
                success = do_user_defined_unop(source, operator_type, result_register);
                if (success) return;

                interp->report_error("Error in unary math operator.\n");
                return;
            }

            result = make_float(-float_value);
        } else {
            result = make_integer(-int_value);
        }
    } else {
        assert(operator_type == LERP_UNOP_NOT);
        if (evaluates_as_true(source)) {
            result = make_integer(0);
        } else {
            result = make_integer(1);
        }
    }

    assert(result != NULL);
    write_register(result_register, result);
}

Atom *Lerp_Bytecode_Runner::get_type_atom(First_Class *value) {
    if (value->type == ARG_DATABASE) {
        Database *db = (Database *)value;
        Schema *space = db->schema->read();
        if (space) return space->type_name;
    }

    assert(value->type < ARG_NUM_TYPES);
    return interp->type_atoms[value->type];
}

// This function should be renamed since it only does db_add
void Lerp_Bytecode_Runner::do_database_binop(Database *db, First_Class *right,
                                            int operator_type, int result_register) {
    assert(operator_type == LERP_BINOP_DB_ADD);

    if (right->type != ARG_DECL_ASSERTION) {
        interp->report_error("Type error -- expected an assertion on rhs\n");
        return;
    }

    Decl_Assertion *assertion = (Decl_Assertion *)right;
    if (assertion->expression == NULL) {
        // @Robustness @Incomplete: I sure hope this wasn't const!
        // @WriteBarrier?  (If we are being anal)
        assertion->expression = assertion->conditionals;
        assertion->conditionals = NULL;
    }

    if (assertion->expression == NULL) {
        interp->report_error("Attempt to assert an empty expression... wtf?\n");
        return;
    }

    Decl_Expression *expression = assertion->expression->read();

    if (expression->next) {
        interp->report_error("Attempt to assert a conjunction... not supported.\n");
        return;
    }

    add_db_assertion(db, assertion);
    write_register(result_register, right);
}

void Lerp_Bytecode_Runner::do_non_numeric_equality(int result_register,
                                                   First_Class *source1,
                                                   First_Class *source2, int register_1, int register_2) {
    First_Class *result = NULL;
    if (source1->type == ARG_UNINITIALIZED) {
        if (source2->type == ARG_UNINITIALIZED) result = interp->parser->make_integer(1);
        else result = interp->parser->make_integer(0);
    } else if (source2->type == ARG_UNINITIALIZED) {
        result = interp->parser->make_integer(0);
    } else if (source1->type != source2->type) {
        result = interp->parser->make_integer(0);
    } else {
        // All types must be the same from here down...
        if (source1->type == ARG_STRING) {
            String *s1 = (String *)source1;
            String *s2 = (String *)source2;
            bool match = Unicode::strings_match(s1->value, s2->value);
            if (match) result = make_integer(1);
            else result = make_integer(0);
        } else if ((source1->type == ARG_INTEGER) || (source1->type == ARG_FLOAT)) {
            bool success;
            int int1, int2;
            success = get_integer_math_args(source1, source2, &int1, &int2);
            if (success) {
                result = make_integer((int1 == int2) ? 1 : 0);
            } else {
                double f1, f2;
                success = get_float_math_args(source1, source2, &f1, &f2);
                if (success) result = make_integer((f1 == f2) ? 1 : 0);
            }
        } else if (source1 == source2) {
            result = make_integer(1);
        } else {
            result = make_integer(0);
        }
    }

    if (result) {
        write_register(result_register, result);
    } else {
        interp->report_error("Incompatible types in operator '==' (registers %d and %d).\n", register_1, register_2);
    }
}

Atom *get_type_name(Lerp_Interp *interp, First_Class *value) {
    if (value->type == ARG_DATABASE) {
        Database *db = (Database *)value;
        Schema *schema = db->schema->read();
        if (!schema) return interp->type_atoms[value->type];
        return schema->type_name;
    } else {
        assert(value->type < ARG_NUM_TYPES);
        return interp->type_atoms[value->type];
    }
}


bool Lerp_Bytecode_Runner::find_and_run_operator_function(Decl_Expression *expression,
                                                          int result_register,
                                                          First_Class *arg1, First_Class *arg2) {

    // XXXXXXXXXXXXXX For now we just check the global namespace, in fact we should
    // cascade through the namespaces here... @Incomplete

    Database *db = interp->global_database;
    Decl_Assertion *results = NULL;
    bool by_implication = false;
    int got_result = interp->goal_solver->find_direct_literal_fact(db, expression, &results, &by_implication);
    
    if (!got_result) return false;
    assert(results);


    Decl_Expression *result_expression = results->expression->read();
    Procedure *proc = (Procedure *)result_expression->arguments[2]->read();
    if (proc->type != ARG_IMPERATIVE_PROCEDURE) return false;
    if (!proc->bytecode) return false;

    // @Refactor: The code below should be shared with other procedure-calling code...
    {
        int num_registers = 2;  // 1 for at least 1 argument; 1 for register 0
        if (arg2) num_registers++;   // 1 more if this is a binary operator.

        Lerp_Bytecode *bytecode = proc->bytecode->read();
        if (bytecode->num_registers > num_registers) num_registers = bytecode->num_registers;
        
        Lerp_Call_Record *new_context = interp->memory_manager->create_call_record(num_registers);
        new_context->bytecode = proc->bytecode;
        new_context->register_for_return_value = result_register;

        new_context->previous_context = current_context;
        new_context->program_counter = 0;

        new_context->registers[1] = ToBarrier(arg1);
        if (arg2) new_context->registers[2] = ToBarrier(arg2);

        current_context = ToBarrier(new_context);

        execution_loop();
    }

    return true;
}


    
bool Lerp_Bytecode_Runner::do_user_defined_binop(First_Class *left, First_Class *right,
                                                 int operator_type, int result_register) {
    // @Speed: Re-use a work tuple here

    Atom *left_type_name = get_type_name(interp, left);
    Atom *right_type_name = get_type_name(interp, right);

    Decl_Expression *expression = interp->memory_manager->create_decl_expression(5);
    expression->initialize_slot(0, interp->parser->make_atom("_operator"));
    expression->initialize_slot(1, interp->parser->make_integer(operator_type));
    expression->initialize_slot(2, interp->the_anonymous_variable);
    expression->initialize_slot(3, left_type_name);
    expression->initialize_slot(4, right_type_name);

    bool success = find_and_run_operator_function(expression, result_register, left, right);
    return success;
}

bool Lerp_Bytecode_Runner::do_user_defined_unop(First_Class *arg,
                                                int operator_type, int result_register) {
    // @Speed: Re-use a work tuple here

    Atom *arg_type_name = get_type_name(interp, arg);

    Decl_Expression *expression = interp->memory_manager->create_decl_expression(4);
    expression->initialize_slot(0, interp->parser->make_atom("_operator"));
    expression->initialize_slot(1, interp->parser->make_integer(operator_type));
    expression->initialize_slot(2, interp->the_anonymous_variable);
    expression->initialize_slot(3, arg_type_name);

    bool success = find_and_run_operator_function(expression, result_register, arg, NULL);
    return success;
}


void Lerp_Bytecode_Runner::run_binop() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 8;

    int operator_type = current_context->read()->bytecode->read()->data[pc + 1];
    int result_register = unpack_register(pc + 2);
    int source1_register = unpack_register(pc + 4);
    int source2_register = unpack_register(pc + 6);

    First_Class *source1 = read_register(source1_register);
    First_Class *source2 = read_register(source2_register);

    if (source1 == NULL) {
        interp->report_error("Strange internal error! (Left argument of a binop is NULL.)\n");
        return;
    }

    if (operator_type == LERP_BINOP_ISEQUAL) { 
        do_non_numeric_equality(result_register, source1, source2,
                                source1_register, source2_register);
        return;
    }

    if ((source1->type == ARG_DATABASE) && (operator_type == LERP_BINOP_DB_ADD)) {
        do_database_binop((Database *)source1, source2, operator_type, result_register);
        return;
    }

    bool success;
    bool use_floats = false;
    int int1, int2;
    double float1, float2;

    success = get_integer_math_args(source1, source2, &int1, &int2);

    First_Class *result = NULL;

    if (!success) {
        success = get_float_math_args(source1, source2, &float1, &float2);

        if (!success) {
            success = do_user_defined_binop(source1, source2, operator_type, result_register);
            if (!success) interp->report_error("Invalid arguments to binary math operator (registers %d and %d).\n", source1_register, source2_register);
            return;
        }

        //
        // We are doing float...
        //

        switch (operator_type) {
        case LERP_BINOP_PLUS:
            result = make_float(float1 + float2);
            break;
        case LERP_BINOP_MINUS:
            result = make_float(float1 - float2);
            break;
        case LERP_BINOP_TIMES:
            result = make_float(float1 * float2);
            break;
        case LERP_BINOP_DIV:
            assert(float2 != 0);  // XXXXX @Robustness: Throw exception here
            result = make_float(float1 / float2);
            break;
        case LERP_BINOP_ISEQUAL:
            result = make_integer(float1 == float2);
            break;
        case LERP_BINOP_ISNOTEQUAL:
            result = make_integer(float1 != float2);
            break;
        case LERP_BINOP_GREATER:
            result = make_integer(float1 > float2);
            break;
        case LERP_BINOP_LESS:
            result = make_integer(float1 < float2);
            break;
        case LERP_BINOP_GREATEREQUAL:
            result = make_integer(float1 >= float2);
            break;
        case LERP_BINOP_LESSEQUAL:
            result = make_integer(float1 <= float2);
            break;
        default:
            assert(0);
            break;
        } 
    } else {
        //
        // We are doing integer...
        //
        switch (operator_type) {
        case LERP_BINOP_PLUS:
            result = make_integer(int1 + int2);
            break;
        case LERP_BINOP_MINUS:
            result = make_integer(int1 - int2);
            break;
        case LERP_BINOP_TIMES:
            result = make_integer(int1 * int2);
            break;
        case LERP_BINOP_DIV:
            assert(int2 != 0);  // XXXXX @Robustness: Throw exception here
            result = make_integer(int1 / int2);
            break;
        case LERP_BINOP_ISEQUAL:
            result = make_integer(int1 == int2);
            break;
        case LERP_BINOP_ISNOTEQUAL:
            result = make_integer(int1 != int2);
            break;
        case LERP_BINOP_GREATER:
            result = make_integer(int1 > int2);
            break;
        case LERP_BINOP_LESS:
            result = make_integer(int1 < int2);
            break;
        case LERP_BINOP_GREATEREQUAL:
            result = make_integer(int1 >= int2);
            break;
        case LERP_BINOP_LESSEQUAL:
            result = make_integer(int1 <= int2);
            break;
        default:
            assert(0);
            break;
        }
    }

    assert(result != NULL);
    write_register(result_register, result);
}

void Lerp_Bytecode_Runner::run_copy_register() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int dest_register = unpack_register(pc + 1);
    int source_register = unpack_register(pc + 3);
    
    write_register(dest_register, read_register(source_register));
}

void Lerp_Bytecode_Runner::run_make_calling_record() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int dest_register = unpack_register(pc + 1);
    int proc_register = unpack_register(pc + 3);
    int num_arguments = unpack_register(pc + 5);

    Procedure *proc = (Procedure *)read_register(proc_register);
    int num_registers = num_arguments + 1;  // Since register 0 is reserved... @RegisterConvention

    if (proc->type != ARG_IMPERATIVE_PROCEDURE) {
        interp->report_error("Attempt to call a non-procedure.\n");
        return;
    }

    Lerp_Bytecode *bytecode = proc->bytecode->read();
    if (bytecode) {
        if (num_arguments != bytecode->num_arguments) {
            interp->report_error("Incorrect number of arguments to procedure '%s' (wanted %d, got %d)\n", bytecode->name->name, bytecode->num_arguments, num_arguments);
            return;
        }

        num_registers = proc->bytecode->read()->num_registers;
    }

    Lerp_Call_Record *record = interp->memory_manager->create_call_record(num_registers);
    record->num_arguments = num_arguments;

    write_register(dest_register, record);
}

void Lerp_Bytecode_Runner::run_poke_into_calling_record() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int record_register = unpack_register(pc + 1);
    int poke_index = unpack_register(pc + 3);
    int arg_register = unpack_register(pc + 5);

    First_Class *fc = read_register(record_register);
    assert(fc->type == ARG_IMPERATIVE_CALL_RECORD);
    Lerp_Call_Record *record = (Lerp_Call_Record *)fc;
    assert(poke_index > 0);
    assert(poke_index <= record->num_arguments);
    assert(poke_index < record->num_registers);

    record->registers[poke_index] = ToBarrier(read_register(arg_register));
}

void Lerp_Bytecode_Runner::run_set_this_pointer_on_calling_record() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int record_register = unpack_register(pc + 1);
    int arg_register = unpack_register(pc + 3);

    First_Class *fc = read_register(record_register);
    assert(fc->type == ARG_IMPERATIVE_CALL_RECORD);
    Lerp_Call_Record *record = (Lerp_Call_Record *)fc;

    Database *db = (Database *)read_register(arg_register);
    assert(db->type == ARG_DATABASE);
    record->this_pointer = ToBarrier(db);
}

bool Lerp_Bytecode_Runner::evaluates_as_true(First_Class *fc) {
    if (fc->type == ARG_UNINITIALIZED) return false;
    if (fc->type == ARG_DATABASE) {
        Database *db = (Database *)fc;
        if (db->assertions == NULL) return false;
        return true;
    }

    if (fc->type == ARG_FLOAT) {
        Float *ffloat = (Float *)fc;
        if (ffloat->value) return true;
        return false;
    }

    if (fc->type != ARG_INTEGER) return true;
    Integer *integer = (Integer *)fc;
    if (integer->value) return true;

    return false;
}
    
void Lerp_Bytecode_Runner::run_goto() {
    int pc = current_context->read()->program_counter;
    int destination = unpack_register(pc + 1);

    current_context->read()->program_counter = destination;
}

void Lerp_Bytecode_Runner::run_goto_if_false() {
    int pc = current_context->read()->program_counter;
    int destination = unpack_register(pc + 1);
    int condition = unpack_register(pc + 3);

    if (!evaluates_as_true(read_register(condition))) {
        current_context->read()->program_counter = destination;
    } else {
        current_context->read()->program_counter = pc + 5;
    }
}

/*
bool Lerp_Bytecode_Runner::push_arguments_into_context(Lerp_Call_Record *context, Lerp_Call_Record *passed_arguments, Procedure *proc) {
    assert(proc->bytecode);  // Procs with NULL bytecode should never survive to runtime.

    int num_arguments = passed_arguments->num_registers;
    Lerp_Bytecode *bytecode = proc->bytecode->read();
    if (num_arguments != bytecode->num_arguments) {
        interp->report_error("Incorrect number of arguments (expected %d, got %d)\n",
                bytecode->num_arguments, num_arguments);
        return false;
    }

    int i;
    for (i = 0; i < num_arguments; i++) {
        Atom *argument = proc->bytecode->read()->arguments[i];
        int register_index = i + 1;  // @Register_Convention
//        int register_index = proc->bytecode->argument_registers[i];

        assert(register_index < context->num_registers);
        context->registers[register_index] = passed_arguments->registers[i];
    }

    return true;
}
*/

void Lerp_Bytecode_Runner::run_call_procedure() {

    int pc = current_context->read()->program_counter;
//    printf("--> pc was %d\n", pc);
    current_context->read()->program_counter += 7;
//    printf("<-- pc is %d\n", current_context->read()->program_counter);


    int result_register = unpack_register(pc + 1);
    int proc_register = unpack_register(pc + 3);
    int record_register = unpack_register(pc + 5);

    First_Class *fc_proc = read_register(proc_register);
    First_Class *fc_record = read_register(record_register);

    if (fc_proc->type != ARG_IMPERATIVE_PROCEDURE) {
        interp->report_error("Attempt to call something that is not a procedure.\n");
        write_register(result_register, interp->parser->make_integer(0));
        return;
    }

    assert(fc_record->type == ARG_IMPERATIVE_CALL_RECORD);
    Lerp_Call_Record *record = (Lerp_Call_Record *)fc_record;

    Procedure *proc = (Procedure *)fc_proc;
    Procedure *caller = current_context->read()->bytecode->read()->procedure->read();
    assert(caller);

    interp->profiler->enter_procedure(proc, caller);

    if (proc->bytecode) {
        // This procedure is written in Lerp... do the normal stuff to call it.
        Lerp_Call_Record *new_context = record;
        assert(new_context->num_registers == proc->bytecode->read()->num_registers);
        new_context->bytecode = proc->bytecode;
        new_context->register_for_return_value = result_register;

        new_context->previous_context = current_context;
        new_context->program_counter = 0;

        current_context = ToBarrier(new_context);

        execution_loop();
        return;  // Maybe this makes tail-recursion more obvious to the compiler, I don't know.
    } else {
        // This procedure is written in C++... we don't actually do a real call.
        proc->proc(interp, record);
        First_Class *return_value = record->registers[0]->read();
        if (!return_value) return_value = interp->uninitialized;
        write_register(result_register, return_value);
        interp->memory_manager->release_call_record(record);
    }

    interp->profiler->exit_procedure(proc, caller);
}

static Binding *nth_binding(Binding *bindings, int index) {
    while (index) {
        bindings = bindings->next->read();
        index--;
    }

    return bindings;
}

// @Speed: count_bindings is slow, we should do better!
static int count_bindings(Binding *bindings) {
    int count = 0;
    while (bindings) {
        bindings = bindings->next->read();
        count++;
    }

    return count;
}

Lerp_Call_Record *Lerp_Bytecode_Runner::make_record_for_procedure(Lerp_Bytecode *bytecode) {
    assert(bytecode);
    
    Lerp_Call_Record *record = interp->memory_manager->create_call_record(bytecode->num_registers);
    record->bytecode = ToBarrier(bytecode);

    return record;
}

// @Refactor: it seems that if we had a MOV instruction, return could currently
// be implemented by a MOV and a GOTO.  Not sure if we really want that though.
void Lerp_Bytecode_Runner::run_return() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter = current_context->read()->bytecode->read()->length;  // Because we are returning!
    int result_register = unpack_register(pc + 1);
    
    First_Class *result = read_register(result_register);
    write_register(0, result);
}

void Lerp_Bytecode_Runner::run_load_constant() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int result_register = unpack_register(pc + 1);
    int constant_index = unpack_register(pc + 3);

    First_Class *result = read_constant(constant_index);
    write_register(result_register, result);
}

int get_variable_index(Decl_Assertion *query, Variable *variable) {
    int i;
    for (i = 0; i < query->num_variables; i++) {
        if (variable == query->variables[i]) return i;
    }

    return -1;
}


Decl_Assertion *Lerp_Bytecode_Runner::make_solution_tuple(Decl_Assertion *query, Binding *bindings) {
    Decl_Assertion *assertion = GC_NEW(Decl_Assertion);
    assertion->conditionals = NULL;

    int num_bindings = count_bindings(bindings);
    Decl_Expression *expression = interp->memory_manager->create_decl_expression(num_bindings);
    assertion->expression = ToBarrier(expression);
    
    // @SolutionTuple
    int num_bound = 0;
    int index;

    Binding *binding = bindings;
    while (binding) {
        if (query) {
            // @Speed: We must do this n^2 thing to find where the variable goes
            // in the solution tuple, right now.  We ought to be able to do something
            // faster.
            index = get_variable_index(query, binding->variable);
            if (index == -1) {
                assert(binding->variable->name[0] == '\0');
                index = num_bound;
            }
        } else {
            index = num_bound;
        }

        expression->arguments[index] = ToBarrier(binding->bound_value->read());
        binding = binding->next->read();
        num_bound++;
    }

    assert(num_bound == num_bindings);
    return assertion;
}

void print_result_list(Lerp_Interp *interp, Binding *binding_list_list) {
    while (binding_list_list) {
        Binding *binding_list = (Binding *)binding_list_list->bound_value->read();
        assert(binding_list->type == ARG_BINDING);

        while (binding_list) {
            printf("    (%08x) %s: ", binding_list->variable, binding_list->variable->name);
            interp->printer->print_value(binding_list->bound_value->read());
            printf("\n");
            binding_list = binding_list->next->read();
        }

        binding_list_list = binding_list_list->next->read();
    }
}

Database *Lerp_Bytecode_Runner::make_db_from_results(Decl_Assertion *query, Binding *binding_list_list, int expected) {
    Database *database = GC_NEW(Database);

    while (binding_list_list) {
        Binding *binding_list = (Binding *)binding_list_list->bound_value->read();
        assert(binding_list->type == ARG_BINDING);

        Decl_Assertion *assertion = make_solution_tuple(query, binding_list);
        add_db_assertion(database, assertion);

        binding_list_list = binding_list_list->next->read();
    }

    return database;
}

Database *Lerp_Bytecode_Runner::make_db_from_results2(Matched_Goal *goals) {
    Database *database = GC_NEW(Database);

    Matched_Goal *goal = goals;
    while (goal) {
        Decl_Assertion *assertion = make_solution_tuple(NULL, goal->bindings->read());
        add_db_assertion(database, assertion);
        goal = goal->next->read();
    }

    return database;
}

void blix() {
}

void Lerp_Bytecode_Runner::run_run_query() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 8;

    int num_expected_variables = unpack_byte(pc + 1);  // @Speed: This is only for debugging
    int result_register = unpack_register(pc + 3);
    int query_register = unpack_register(pc + 5);
    int use_owner = unpack_byte(pc + 7);

    First_Class *owner = NULL;
    if (use_owner) {
        current_context->read()->program_counter += 2;
        int owner_register = unpack_register(pc + 8);
        owner = read_register(owner_register);
    }

    Decl_Assertion *query = (Decl_Assertion *)read_register(query_register);
    assert(query->type == ARG_DECL_ASSERTION);

    Database *source_db = NULL;
    if (!owner) {
        source_db = interp->global_database;
    } else {
        if (owner->type == ARG_DATABASE) {
            source_db = (Database *)owner;
        }
    }

    if (source_db == NULL) {
        interp->report_error("Error: Owner of '.' operator is not a database.\n");
        return;
    }

    //
    // Allocate a new Database to hold the results, then iterate over this list
    // and create database tuples.  This is of course going to be a bit 
    // memory-chewy and slow.  In future, maybe we will make the matcher just
    // return such tuples to begin with.  That would make a lot of sense, and it'd
    // be nice to keep all the results in the realm of first-class objects.
    //

    Binding *conjunct_results;
    interp->goal_solver->match_conjunction_series(source_db, query, &conjunct_results);

    if (query->flags & ASSERTION_FLAGS_HAS_SCALAR_RESULT) {
        blix();
        int index = query->scalar_result_index;
        assert(index < query->num_variables);

        if (conjunct_results == NULL) {
            // There were no results to the query, so we return 'uninitialized'...
            write_register(result_register, interp->uninitialized);
            return;
        }

        if (conjunct_results->next != NULL) {
            interp->report_error("Attempt to return a scalar result, for a query that returned multiple results.\n");
            interp->report_error("Here were the results:\n");
            print_result_list(interp, conjunct_results);
            write_register(result_register, interp->uninitialized);
            return;
        }

        //
        // We successfully returned with 1 result, so we return that result value
        // as a scalar.  Yay!
        //

        Binding *binding = (Binding *)conjunct_results->bound_value->read();
        assert(binding->type == ARG_BINDING);

        while (binding) {
            if (binding->variable->name[0] == '?') break; // @Refactor XXX Hack formalize this!!!
            binding = binding->next->read();
        }

        if (binding == NULL) {
            interp->report_error("Weird error!!!\n");
            return;
        }
            
        write_register(result_register, binding->bound_value->read());
        return;
    }

    Database *database = make_db_from_results(query, conjunct_results, num_expected_variables);
    write_register(result_register, database);
}

static Decl_Constraint *find_constraint(Schema *space) {
    Decl_Constraint *constraint;
    Foreach(&space->db_constraints, constraint) {
        return constraint;
    } Endeach;

    return NULL;
}

void Lerp_Bytecode_Runner::run_run_query_domain_specified() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int result_register = unpack_register(pc + 1);
    int left_register = unpack_register(pc + 3);
    int right_register = unpack_register(pc + 5);


    First_Class *left_arg = read_register(left_register);
    First_Class *right_arg = read_register(right_register);


    if (left_arg->type != ARG_DATABASE) {
        interp->report_error("Attempt to perform subscript query on a non-struct.\n");
        return;
    }

    Database *db = (Database *)left_arg;

    Schema *space = db->schema->read();
    if (!space) {
        interp->report_error("Weird error.\n");
        return;
    }

    Decl_Constraint *constraint = find_constraint(space);
    if (!constraint || (constraint->num_domain_arguments == 0)) {
        interp->report_error("struct does not have a domain qualifier!\n");
        return;
    }


    First_Class *return_value = NULL;

    if (right_arg->type != ARG_DECL_ASSERTION) {
        // SHORTCUT. @Refactor, contains some cutnpaste from below.
        int query_len = constraint->arguments.items;
        int num_domain_items = 1;
        int result_tuple_len = query_len - num_domain_items;

        Decl_Assertion *result = NULL;
        bool by_implication = false;
        interp->goal_solver->find_direct_literal_fact_from_one_value(db, right_arg, &result, &by_implication);
        if (by_implication) {
            interp->report_error("Attempt to subscript something that fulfills by implication (not implemented yet, but it will be!\n");
            return;
        }

        if (!result) {
            return_value = interp->uninitialized;
        } else {
            // XXXXXXXXXXXXX cutnpaste from above @Refactor
            if (result_tuple_len == 1) {
                Decl_Expression *expression = result->expression->read();
                return_value = expression->arguments[expression->num_arguments - 1]->read();
            } else {
                return_value = make_integer(1);  // XXXXX @Incomplete
            }
        }

        assert(return_value != NULL);
        write_register(result_register, return_value);
        return;
    }

    Decl_Assertion *assertion = (Decl_Assertion *)right_arg;
    assert(assertion->type == ARG_DECL_ASSERTION);
    
    int num_domain_items = constraint->num_domain_arguments;

    assert(assertion->expression == NULL);
    assert(assertion->conditionals);
    assert(assertion->conditionals->read()->next == NULL);

    Decl_Expression *tuple = (Decl_Expression *)assertion->conditionals;
    if (num_domain_items != tuple->num_arguments) {
        interp->report_error("Subscript is incorrect size for struct's domain (required %d items, but got %d)\n", num_domain_items, tuple->num_arguments);
        return;
    }

    // @Robustness: We must make sure that it's always okay to just set this flag on
    // the tuple (i.e. this piece of data will never be used in a context where that
    // flag is not wanted)

    // @Refactor: This should be done at compile time?
    tuple->flags |= DECL_EXPR_FLAGS_WILDCARDED;

    
    if (assertion->num_variables) {
        Matched_Goal *matched_goals;
        interp->goal_solver->direct_match(db, tuple, &matched_goals);

        if (matched_goals == NULL) {
            // Return 'uninitialized'
            return_value = interp->uninitialized;
        } else if (matched_goals->next) {
            // Too many answers... uhh that is an error!
//        interp->report_error("Domain subscripting produced multiple results... this should not happen!\n");
            return_value = make_db_from_results2(matched_goals); // XXX Make this more well-defined
        } else {
            // There must be exactly one binding list, so let's get that...
            Binding *bindings = matched_goals->bindings->read();

        //
        // Now we have to pull the data out of this binding list and return it...
        //

        // @Robustness: I am assuming that these items come back in the correct sorted
        // order.  If not... well... I will have to debug this at some point in the future,
        // won't that be fun!

        // @Semantics: Okay, if there's only 1 item in the query, I am just gonna
        // return that guy without a tuple... otherwise I make a tuple... how annoying!
            int query_len = constraint->arguments.items;
            int result_tuple_len = query_len - num_domain_items;
        
            return_value = NULL;
            if (result_tuple_len == 1) {
                Binding *binding = bindings;
                while (binding) {
                    if (binding->variable->name[0] == '\0') {  // This is an implicit variable
                        return_value = binding->bound_value->read();
                        break;
                    }
                    binding = binding->next->read();
                }

                assert(return_value != NULL);
            } else {
                return_value = make_integer(0);  // XXXXX @Incomplete
            }
        }
    } else {
        int query_len = constraint->arguments.items;
        int result_tuple_len = query_len - num_domain_items;

        Decl_Assertion *result = NULL;
        bool by_implication = false;
        interp->goal_solver->find_direct_literal_fact(db, tuple, &result, &by_implication);
        if (by_implication) {
            interp->report_error("Attempt to subscript something that fulfills by implication (not implemented yet, but it will be!\n");
            return;
        }

        if (!result) {
            return_value = interp->uninitialized;
        } else {
            // XXXXXXXXXXXXX cutnpaste from above @Refactor
            if (result_tuple_len == 1) {
                Decl_Expression *expression = result->expression->read();
                return_value = expression->arguments[expression->num_arguments - 1]->read();
            } else {
                return_value = make_integer(1);  // XXXXX @Incomplete
            }
        }
    }

    assert(return_value != NULL);
    write_register(result_register, return_value);
}

Decl_Expression *Lerp_Bytecode_Runner::copy_expression(Decl_Expression *old) {
    if (old == NULL) return NULL;
    Decl_Expression *result = interp->memory_manager->create_decl_expression(old->num_arguments);
    result->next = ToBarrier(copy_expression(old->next->read()));
    result->flags = old->flags;

    int i;
    for (i = 0; i < old->num_arguments; i++) result->arguments[i] = old->arguments[i];

    return result;
}

Decl_Assertion *Lerp_Bytecode_Runner::copy_assertion(Decl_Assertion *old) {
    Decl_Assertion *result = GC_NEW(Decl_Assertion);
    result->flags = old->flags;
    result->num_variables = old->num_variables;
    result->variables = old->variables;  // XXX @Memory copy of unmanaged memory
    result->scalar_result_index = old->scalar_result_index;

    result->expression = ToBarrier(copy_expression(old->expression->read()));
    result->conditionals = ToBarrier(copy_expression(old->conditionals->read()));
    
    return result;
}

void Lerp_Bytecode_Runner::run_assign_array_subscript() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int left_register = unpack_register(pc + 1);
    int right_register = unpack_register(pc + 3);
    int rvalue_register = unpack_register(pc + 5);


    First_Class *left_arg = read_register(left_register);
    First_Class *right_arg = read_register(right_register);


    if (left_arg->type != ARG_DATABASE) {
        interp->report_error("Attempt to perform subscript assignment on a non-struct.\n");
        return;
    }

    Database *db = (Database *)left_arg;
    Schema *space = db->schema->read();
    if (!space) {
        interp->report_error("Weird error.\n");
        return;
    }

    Decl_Constraint *constraint;
    constraint = find_constraint(space);
    if (!constraint || (constraint->num_domain_arguments == 0)) {
        interp->report_error("struct does not have a domain qualifier!\n");
        return;
    }

    //
    // Check to see that they put the proper number of arguments inside the
    // array subscript.
    //

    Decl_Assertion *assertion = NULL;
    Decl_Expression *tuple = NULL;
    int num_domain_items;
    if (right_arg->type == ARG_DECL_ASSERTION) {
        assertion = (Decl_Assertion *)right_arg;
        tuple = (Decl_Expression *)assertion->conditionals;
        num_domain_items = tuple->num_arguments;
    } else {
        num_domain_items = 1;
    }

    if (num_domain_items != constraint->num_domain_arguments) {
        interp->report_error("Subscript is incorrect size for struct's domain (required %d items, but got %d)\n", num_domain_items, tuple->num_arguments);
        return;
    }


    //
    // Measure how many items we are attempting to glue to the RHS, and then
    // verify that that matches the constraint.
    //
    First_Class *rvalue_arg = read_register(rvalue_register);
    Decl_Expression *rvalue_expression = NULL;
    int rvalue_length;
    if (rvalue_arg->type == ARG_DECL_ASSERTION) {
        Decl_Assertion *assertion = (Decl_Assertion *)rvalue_arg;
        if ((assertion->expression) || (!assertion->conditionals) || (assertion->conditionals->read()->next)) {
            interp->report_error("Can only assign a simple tuple!!!\n");
            return;
        }
        rvalue_expression = assertion->conditionals->read();
        rvalue_length = rvalue_expression->num_arguments;
    } else {
        rvalue_length = 1;
    }

    int query_len = constraint->arguments.items;

    int needed = query_len - num_domain_items;
    assert(needed >= 0);
    if (needed != rvalue_length) {
        interp->report_error("Improper length of rvalue. (Wanted %d, got %d)\n",
                needed, rvalue_length);
        return;
    }



    //
    // Remove the old facts having to do with this domain (hopefully we
    // remove only 0 or 1!   If more, something fishy is happening...
    //

    // @Robustness: We must make sure that it's always okay to just set this flag on
    // the tuple (i.e. this piece of data will never be used in a context where that
    // flag is not wanted)

    bool by_implication = false;
    Decl_Assertion *found_fact = NULL;
    int num_facts;
    if (tuple) {
        tuple->flags |= DECL_EXPR_FLAGS_WILDCARDED;  

        num_facts = interp->goal_solver->find_direct_literal_fact(db, tuple, &found_fact, &by_implication);
    } else {
        num_facts = interp->goal_solver->find_direct_literal_fact_from_one_value(db, right_arg, &found_fact, &by_implication);
    }

    if (by_implication) {
        interp->report_error("Attempt to assign a value that is already produced by implication... yuck!\n");
        return;
    }

    int num_range_items = num_domain_items + rvalue_length;

    Decl_Assertion *new_assertion;
    if (found_fact) {
        new_assertion = found_fact;
    } else if (!assertion) {
        new_assertion = GC_NEW(Decl_Assertion);
        Decl_Expression *expression = interp->memory_manager->create_decl_expression(num_range_items);
        expression->arguments[0] = ToBarrier(right_arg);  // Because it was a non-assertion value, there must be only 1.
        new_assertion->expression = ToBarrier(expression);
    } else if (!(assertion->flags & ASSERTION_FLAGS_CONST)) {
        new_assertion = assertion;
    } else {
        new_assertion = copy_assertion(assertion);
        new_assertion->flags &= ~ASSERTION_FLAGS_CONST;
    }

    if (new_assertion->expression == NULL) {
        new_assertion->expression = new_assertion->conditionals;
        new_assertion->conditionals = NULL;
    }

    // We are re-defining the meaning of 'tuple' here to mean the result tuple,
    // not the rvalue tuple any more.  (@Refactor: introduce new variable name
    // to make it cleaner).
    tuple = new_assertion->expression->read();
    assert(tuple);
    assert(tuple->num_arguments >= num_domain_items);
    if (tuple->num_arguments != num_range_items) {
        Decl_Expression *expr = interp->memory_manager->create_decl_expression(num_range_items);
        expr->flags = tuple->flags;

        int i;
        for (i = 0; i < num_domain_items; i++) {
            expr->arguments[i] = tuple->arguments[i];
        }

        new_assertion->expression = ToBarrier(expr);
        tuple = expr;
    }

    if (rvalue_expression) {
        First_Class *value;
        int i;
        for (i = 0; i < rvalue_expression->num_arguments; i++) {
            value = rvalue_expression->arguments[i]->read();
            // @WriteBarrier?
            tuple->arguments[num_domain_items + i] = ToBarrier(value);
        }
    } else {
        tuple->arguments[num_domain_items + 0] = ToBarrier(rvalue_arg);
    }

    if (!found_fact) add_db_assertion(db, new_assertion);
}

//
// @Speed: We are linearly searching down this assertion list each time!
// So an 'each' loop is acutally n^2.  Maybe something can be done about this.
//
static Decl_Assertion *get_nth_assertion(Database *database, int n) {
    Decl_Assertion *assertion = database->assertions->read();
    while (assertion && n) {
        assertion = assertion->next->read();
        n--;
    }

    return assertion;
}

void Lerp_Bytecode_Runner::each_helper(int iterator_status_register, int scratch_register,
                                       Database *database) {
    Integer *cursor = (Integer *)read_register(scratch_register);
    assert(cursor->type == ARG_INTEGER);

    Decl_Assertion *assertion = get_nth_assertion(database, cursor->value);
    if (assertion == NULL) {
        // We are done looping... 'uninitialized' goes into the status register.
        write_register(iterator_status_register, interp->uninitialized);
        return;
    }

    // We're not done... the database goes into the status register.
    write_register(iterator_status_register, database);
    
    // Put the next value into register 0.
    write_register(0, assertion);

    // Increment the iteration cursor.
    write_register(scratch_register, make_integer(cursor->value + 1));
}

Database *Lerp_Bytecode_Runner::make_singleton_database(First_Class *value) {
    // Make a 'Solution' tuple (@Volatile: This may change in the near future...)
    Decl_Expression *expression = interp->memory_manager->create_decl_expression(1);  // @SolutionTuple
    expression->arguments[0] = ToBarrier(value);
    
    Decl_Assertion *assertion = GC_NEW(Decl_Assertion);
    assertion->expression = ToBarrier(expression);

    Database *database = GC_NEW(Database);
    add_db_assertion(database, assertion);
    return database;
}

/*
void Lerp_Bytecode_Runner::singleton_each_helper(int iterator_status_register, int scratch_register,
                                                 First_Class *value) {
    // We're not done... 1 goes into the status register.
    write_register(iterator_status_register, make_integer(1));
    
    // Put the next value into register 0.
    write_register(0, make_singleton_assertion(value));

    // Increment the iteration cursor.
    write_register(scratch_register, make_integer(1));
}
*/

void Lerp_Bytecode_Runner::run_each_begin() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int iterator_status_register = unpack_register(pc + 1);
    int scratch_register = unpack_register(pc + 3);
    int list_value_register = unpack_register(pc + 5);

    First_Class *list = read_register(list_value_register);
    First_Class *orig = list;
    if (list->type != ARG_DATABASE) {
        list = make_singleton_database(list);
    }
    
/*    
    if (list->type != ARG_DATABASE) {
        interp->report_error("Type mismatch in 'each' iteration... must be a list.\n");
        return;
    }
*/
    write_register(scratch_register, make_integer(0));

    assert(list->type == ARG_DATABASE);
    Database *database = (Database *)list;
    each_helper(iterator_status_register, scratch_register, database);
}

//
// @Refactor: combine run_each_begin() and run_each_next() so there's not so much
// pasting?  Let's see how they evolve as the feature becomes more complicated...
//
void Lerp_Bytecode_Runner::run_each_next() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int iterator_status_register = unpack_register(pc + 1);
    int scratch_register = unpack_register(pc + 3);

    First_Class *list = read_register(iterator_status_register);

    if (!list || (list->type != ARG_DATABASE)) {
        interp->report_error("Type mismatch in 'each' iteration... must be a list.\n");
        return;
    }

    Database *database = (Database *)list;
    each_helper(iterator_status_register, scratch_register, database);
}

void Lerp_Bytecode_Runner::run_tuple_peek() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int result_register = unpack_register(pc + 1);
    int tuple_register = unpack_register(pc + 3);
    int index_to_extract = unpack_register(pc + 5);

    First_Class *value = read_register(tuple_register);
    if (value->type != ARG_DECL_ASSERTION) {
        interp->report_error("Type mismatch.... ought to have an assertion.\n");
        write_register(result_register, make_integer(0));
        return;
    }

    Decl_Assertion *assertion = (Decl_Assertion *)value;
    assert(assertion->conditionals == NULL);

    Decl_Expression *expression = assertion->expression->read();
    assert(index_to_extract < expression->num_arguments);

    First_Class *result = expression->arguments[index_to_extract]->read();
    write_register(result_register, result);
}

void Lerp_Bytecode_Runner::run_tuple_poke() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int tuple_register = unpack_register(pc + 1);
    int value_register = unpack_register(pc + 3);
    int tuple_index = unpack_register(pc + 5);

    First_Class *tuple_fc = read_register(tuple_register);
    if (tuple_fc->type != ARG_DECL_EXPRESSION) {
        interp->report_error("Type mismatch.... ought to have a Decl_Expression.\n");
        return;
    }

    Decl_Expression *expression = (Decl_Expression *)tuple_fc;
    assert(expression->num_arguments >= tuple_index);

    First_Class *value = read_register(value_register);
    expression->arguments[tuple_index] = ToBarrier(value);
}

void Lerp_Bytecode_Runner::run_tuple_make() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int result_register = unpack_register(pc + 1);
    int length = unpack_register(pc + 3);
    int flags = unpack_register(pc + 5);

    Decl_Expression *expression = interp->memory_manager->create_decl_expression(length);
    expression->flags = flags;

    write_register(result_register, expression);
}

void Lerp_Bytecode_Runner::run_sequence_make() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;
    
    int result_register = unpack_register(pc + 1);
    int prototype_constant = unpack_register(pc + 3);

    Decl_Assertion *proto = (Decl_Assertion *)read_constant(prototype_constant);
    assert(proto->type == ARG_DECL_ASSERTION);

    Decl_Assertion *sequence = GC_NEW(Decl_Assertion);
    sequence->num_variables = proto->num_variables;
    sequence->variables = proto->variables;  // XXX @Memory Potentially unsafe memory thing
    sequence->flags = proto->flags;
    sequence->scalar_result_index = proto->scalar_result_index;

    write_register(result_register, sequence);
}

void Lerp_Bytecode_Runner::run_sequence_prepend() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int sequence_register = unpack_register(pc + 1);
    int item_register = unpack_register(pc + 3);

    Decl_Expression *item = (Decl_Expression *)read_register(item_register);
    Decl_Assertion *sequence = (Decl_Assertion *)read_register(sequence_register);

    assert(sequence->type == ARG_DECL_ASSERTION);
    assert(item->type == ARG_DECL_EXPRESSION);

    item->next = ToBarrier(sequence->conditionals->read());
    sequence->conditionals = ToBarrier(item);
}

void Lerp_Bytecode_Runner::run_push_namespace() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 3;

    int name_register = unpack_register(pc + 1);
    Atom *name = (Atom *)read_register(name_register);
    assert(name->type == ARG_ATOM);

    //
    // @Incomplete Actually lookup the namespace here, don't just put the atom in
    //
    Database *space = (Database *)interp->global_database->lookup_named_slot(interp, name);
    if ((!space) || (space->type != ARG_DATABASE)) {
        interp->report_error("Attempt to push namespace '%s' which is not mounted!\n", name->name);
        return;
    }

    Value_Pair *pair = GC_NEW(Value_Pair);
    pair->left = ToBarrierF(space);
    pair->right = ToBarrierF(current_context->read()->namespace_stack->read());

    // @WriteBarrier
    current_context->read()->namespace_stack = ToBarrier(pair);
}

void Lerp_Bytecode_Runner::run_pop_namespaces() {
    Lerp_Call_Record *context = current_context->read();

    int pc = context->program_counter;
    context->program_counter += 3;

    int num_spaces_to_pop = unpack_register(pc + 1);

    assert(num_spaces_to_pop >= 0);
    while (num_spaces_to_pop) {
        if (context->namespace_stack == NULL) {
            interp->report_error("Namespace stack underflow (this is a hard error to get!).\n");
            return;
        }

        num_spaces_to_pop--;

        // @WriteBarrier
        context->namespace_stack = ToBarrier((Value_Pair *)context->namespace_stack->read()->right->read());
    }
}

First_Class *Lerp_Bytecode_Runner::read_constant(int index) {
    Lerp_Bytecode *bytecode = current_context->read()->bytecode->read();
    assert(index >= 0);
    assert(index < bytecode->num_constants);

    return bytecode->constants[index]->read();
}


First_Class *Lerp_Bytecode_Runner::lookup_nonlocal_name(Atom *name) {
    First_Class *result = NULL;

    Lerp_Call_Record *context = current_context->read();
    Value_Pair *pair = context->namespace_stack->read();
    while (pair) {
        Database *db = (Database *)pair->left->read();
        assert(db->type == ARG_DATABASE);

        result = db->lookup_named_slot(interp, name);
        if (result) return result;
        pair = (Value_Pair *)pair->right->read();
    }

    result = interp->global_database->lookup_named_slot(interp, name);
    return result;
}

void Lerp_Bytecode_Runner::run_lookup_rvalue() {
    int pc = current_context->read()->program_counter;
    int which_kind = unpack_byte(pc + 1);

    int result_register = unpack_register(pc + 2);
    int constant_index = unpack_register(pc + 4);
    int owner_register = -1;

    if (which_kind) {
        owner_register = unpack_register(pc + 6);
        current_context->read()->program_counter += 8;
    } else {
        current_context->read()->program_counter += 6;
    }


    //
    // @Incomplete: We ignore 'owner' for now; all lookups are lookups
    // of a global variable.
    //

    First_Class *fc_atom = read_constant(constant_index);
    assert(fc_atom->type == ARG_ATOM);
    Atom *atom = (Atom *)fc_atom;

    First_Class *rvalue = NULL;
    if (owner_register == -1) {
        rvalue = lookup_nonlocal_name(atom);
        if (!rvalue) {
            interp->report_error("Attempt to lookup undefined variable '%s'.\n", atom->name);
            return;
        }
    } else {
        First_Class *owner = read_register(owner_register);
        if (owner == NULL) {
            interp->report_error("REALLY BAD PROBLEM\n");
            return;
        }

        if (owner->type != ARG_DATABASE) {
            interp->report_error("Left-hand side of '.' is not a valid struct!\n");
            return;
        } else {
            Database *db = (Database *)owner;
            rvalue = db->lookup_named_slot(interp, atom);
            if (!rvalue) {
                interp->report_error("Trying to access a member that doesn't exist.\n");
                return;
            }
        }
    }

    write_register(result_register, rvalue);
}


void Lerp_Bytecode_Runner::run_assign() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int constant_index = unpack_register(pc + 1);
    int rvalue_register = unpack_register(pc + 3);
    int owner = -1;

    //
    // @Incomplete: We ignore 'owner' for now; all lookups are lookups
    // of a global variable.
    //

    First_Class *fc_atom = read_constant(constant_index);
    assert(fc_atom->type == ARG_ATOM);
    Atom *atom = (Atom *)fc_atom;

    First_Class *rvalue = read_register(rvalue_register);
    interp->global_database->assign_named_slot(interp, atom, rvalue);
}

void Lerp_Bytecode_Runner::run_assign_struct_member() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 7;

    int owner_register = unpack_register(pc + 1);
    int member_name = unpack_register(pc + 3);
    int rvalue_register = unpack_register(pc + 5);

    First_Class *rvalue = read_register(rvalue_register);
    First_Class *owner = read_register(owner_register);
    if (owner->type != ARG_DATABASE) {
        interp->report_error("Attempt to assign a struct member on something that's not a database.\n");
        return;
    }

    Atom *member = (Atom *)read_constant(member_name);
    assert(member->type == ARG_ATOM);

    Database *db = (Database *)owner;
    db->assign_named_slot(interp, member, rvalue);

    //
    // Don't need to write into a result register since rvalue_register
    // already has our result.
    //
}

void Lerp_Bytecode_Runner::run_instantiate() {
    int pc = current_context->read()->program_counter;
    current_context->read()->program_counter += 5;

    int result_register = unpack_register(pc + 1);
    int type_name_index = unpack_register(pc + 3);

    Atom *type_name = (Atom *)read_constant(type_name_index);
    assert(type_name->type == ARG_ATOM);

    // XXX Always global namespace right now... probably not the right thing.
    First_Class *type_fc = lookup_nonlocal_name(type_name);
    Schema *space = NULL;
    if (type_fc) {
        if (type_fc->type != ARG_SCHEMA) {
            interp->report_error("Attempt to instantiate a type '%s' that's bound to something other than a schema (?!?)\n", type_name->name);
            return;
        }

        space = (Schema *)type_fc;
    }

    if (!space) {
        interp->report_error("Attempt to instantiate unknown type '%s'\n", type_name->name);
        return;
    }

    if (!space || (space->type != ARG_SCHEMA)) {
        interp->report_error("Attempt to instantiate an unknown type ('%s')...\n", type_name->name);
        write_register(0, interp->parser->make_integer(0));
        return;
    }

    Database *result = interp->instantiate(space);
    write_register(result_register, result);
}

void Lerp_Bytecode_Runner::execution_loop() {
    int pc = current_context->read()->program_counter;
    Lerp_Bytecode *bytecode = current_context->read()->bytecode->read();

    while (pc < bytecode->length) {
        if (interp->runtime_error) return;

        int old_pc = pc;

        int opcode = bytecode->data[pc];

        switch (opcode) {
        case LERP_BYTECODE_COPY_REGISTER:
            run_copy_register();
            break;
        case LERP_BYTECODE_UNOP:
            run_unop();
            break;
        case LERP_BYTECODE_BINOP:
            run_binop();
            break;
        case LERP_BYTECODE_ASSIGN:
            run_assign();
            break;
        case LERP_BYTECODE_LOOKUP_RVALUE:
            run_lookup_rvalue();
            break;
        case LERP_BYTECODE_PUSH_NAMESPACE:
            run_push_namespace();
            break;
        case LERP_BYTECODE_POP_NAMESPACES:
            run_pop_namespaces();
            break;
        case LERP_BYTECODE_LOAD_CONSTANT:
            run_load_constant();
            break;
        case LERP_BYTECODE_MAKE_CALLING_RECORD:
            run_make_calling_record();
            break;
        case LERP_BYTECODE_POKE_INTO_CALLING_RECORD:
            run_poke_into_calling_record();
            break;
        case LERP_BYTECODE_CALL_PROCEDURE:
            run_call_procedure();
            bytecode = current_context->read()->bytecode->read();
            pc = bytecode->length;  // This is just a way to make us break out of the loop.
            break;
        case LERP_BYTECODE_GOTO:
            run_goto();
            break;
        case LERP_BYTECODE_GOTO_IF_FALSE:
            run_goto_if_false();
            break;
        case LERP_BYTECODE_RETURN:
            run_return();
            break;
        case LERP_BYTECODE_RUN_QUERY:
            run_run_query();
            break;
        case LERP_BYTECODE_RUN_QUERY_DOMAIN_SPECIFIED:
            run_run_query_domain_specified();
            break;
        case LERP_BYTECODE_EACH_BEGIN:
            run_each_begin();
            break;
        case LERP_BYTECODE_EACH_NEXT:
            run_each_next();
            break;
        case LERP_BYTECODE_TUPLE_PEEK:
            run_tuple_peek();
            break;
        case LERP_BYTECODE_TUPLE_POKE:
            run_tuple_poke();
            break;
        case LERP_BYTECODE_TUPLE_MAKE:
            run_tuple_make();
            break;
        case LERP_BYTECODE_SEQUENCE_MAKE:
            run_sequence_make();
            break;
        case LERP_BYTECODE_SEQUENCE_PREPEND:
            run_sequence_prepend();
            break;
        case LERP_BYTECODE_ASSIGN_STRUCT_MEMBER:
            run_assign_struct_member();
            break;
        case LERP_BYTECODE_ASSIGN_ARRAY_SUBSCRIPT:
            run_assign_array_subscript();
            break;
        case LERP_BYTECODE_SET_THIS_POINTER_ON_CALLING_RECORD:
            run_set_this_pointer_on_calling_record();
            break;
        case LERP_BYTECODE_INSTANTIATE:
            run_instantiate();
            break;
        default:
            assert(0);
        }

        assert(current_context->read()->type == ARG_IMPERATIVE_CALL_RECORD);
        pc = current_context->read()->program_counter;
//        printf("   pc %d, old_pc %d\n", pc, old_pc);
        if (!interp->runtime_error) assert(pc != old_pc);  // If program doesn't advance, there is a bug!
    }

    // Re-read these things because we might have gc'd!!
    pc = current_context->read()->program_counter;
    bytecode = current_context->read()->bytecode->read();

    // Did we exit because we're really done, or because we called a procedure?
    if (pc < bytecode->length) {
        // We called a procedure... just tail-recurse.
        execution_loop();
        return;
    } else {
        Lerp_Call_Record *old_context = current_context->read();
        Lerp_Call_Record *new_context = old_context->previous_context->read();
        if (!new_context) return;  // We are done!

        int return_register = old_context->register_for_return_value;
        First_Class *return_value = old_context->registers[0]->read();
        if (!return_value) return_value = interp->uninitialized;

        current_context = ToBarrier(new_context);
        interp->memory_manager->release_call_record(old_context);
        write_register(return_register, return_value);
    }
}

void Lerp_Bytecode_Runner::run(Lerp_Bytecode *bytecode) {
    Lerp_Call_Record *record = make_record_for_procedure(bytecode);
    record->program_counter = 0;

    current_context = ToBarrier(record);
    write_register(0, make_integer(0));

    execution_loop();
}

void Lerp_Bytecode_Runner::run(Procedure *procedure) {
    interp->profiler->enter_procedure(procedure);
    run(procedure->bytecode->read());
    interp->profiler->exit_procedure(procedure);
}

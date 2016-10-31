struct Lerp_Bytecode;
struct Lerp_Interp;
struct Schema;
struct Lerp_Thread;

struct Lerp_Call_Record : public First_Class {   // @Memory: Doesn't need to be First_Class...
    Barrier <Lerp_Call_Record *> *previous_context;
    Barrier <Lerp_Bytecode *> *bytecode;

    Barrier <Value_Pair *> *namespace_stack;
    Barrier <Database *> *this_pointer;

    unsigned short register_for_return_value;
    unsigned short cached;

    int program_counter;
    unsigned short num_registers;
    unsigned short num_arguments;
    Barrier <First_Class *> *registers[1];

    static int allocation_size(int num_registers);
};

inline int Lerp_Call_Record::allocation_size(int num_registers) {
    int extra_bytes = (num_registers - 1) * sizeof(First_Class *);  // This could be -4!
    int size = sizeof(Lerp_Call_Record) + extra_bytes;
    return size;
}

struct Lerp_Bytecode_Runner {
    Lerp_Bytecode_Runner(Lerp_Interp *);
    ~Lerp_Bytecode_Runner();

    Lerp_Interp *interp;

    Lerp_Thread *current_thread;
    void run(Lerp_Thread *);
    Lerp_Call_Record *make_record_for_procedure(Lerp_Bytecode *bytecode);

    LEXPORT void write_register(int register_index, First_Class *value);
    LEXPORT Lerp_Call_Record *copy_context(Lerp_Call_Record *context);
    Lerp_Call_Record *current_context();

  protected:

    void run_copy_register();
    void run_unop();
    void run_binop();
    void run_make_calling_record();
    void run_poke_into_calling_record();
    void run_set_this_pointer_on_calling_record();
    void run_call_procedure();
    void run_return();
    void run_eval();
    void run_lookup_rvalue();
    void run_push_namespace();
    void run_pop_namespaces();
    void run_load_constant();
    void run_goto();
    void run_goto_if_false();
    void run_run_query();
    void run_run_query_domain_specified();
    void run_each_begin();
    void run_each_next();
    void run_tuple_peek();
    void run_tuple_poke();
    void run_tuple_make();
    void run_sequence_make();
    void run_sequence_prepend();
    void run_assign();
    void run_assign_struct_member();
    void run_assign_array_subscript();
    void run_instantiate();
    
    void do_non_numeric_equality(int result_register,
                                 First_Class *source1,
                                 First_Class *source2,
                                 int register_1, int register_2);

    void do_database_binop(Database *db, First_Class *right,
                           int operator_type, int result_register);
    bool do_user_defined_binop(First_Class *left, First_Class *right,
                               int operator_type, int result_register);
    bool do_user_defined_unop(First_Class *arg,
                              int operator_type, int result_register);
    bool find_and_run_operator_function(Decl_Expression *expression,
                                        int result_register,
                                        First_Class *arg1, First_Class *arg2);
    Atom *get_type_atom(First_Class *);

    Decl_Assertion *make_solution_tuple(Decl_Assertion *query, Binding *bindings);

    void each_helper(int iterator_status_register, int scratch_register,
                     Database *database);

    First_Class *make_integer(int value);
    First_Class *make_float(double value);

    int unpack_register(int value);
    int unpack_byte(int value);

    First_Class *read_register(int register_index);

    First_Class *read_constant(int constant_index);

    bool push_arguments_into_context(Lerp_Call_Record *context, Lerp_Call_Record *passed_arguments, Procedure *proc);

    void execution_loop();

    Database *make_db_from_results(Decl_Assertion *query, Binding *binding_list_list, int expected = -1);
    Database *make_db_from_results2(Matched_Goal *matched_goals);

    bool get_integer_math_args(First_Class *a1, First_Class *a2,
                               int *i1_return, int *i2_return);
    bool get_float_math_args(First_Class *a1, First_Class *a2,
                             double *i1_return, double *i2_return);

    First_Class *lookup_nonlocal_name(Atom *name);

    bool evaluates_as_true(First_Class *fc);
    Decl_Expression *copy_expression(Decl_Expression *old);
    Decl_Assertion *copy_assertion(Decl_Assertion *old);

//    Decl_Expression *make_query_expression(Decl_Constraint *constraint, Decl_Expression *tuple);

//    Decl_Assertion *make_singleton_assertion(First_Class *value);
    Database *make_singleton_database(First_Class *value);
    void singleton_each_helper(int iterator_status_register, int scratch_register,
                               First_Class *value);
};


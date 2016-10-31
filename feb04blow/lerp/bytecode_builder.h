struct Lerp_Bytecode;
struct Lerp_Bytecode_Builder;
struct Concatenator;

struct Ast;
struct Ast_Block;
struct Ast_Expression;
struct Ast_Unary_Expression;
struct Ast_Binary_Expression;
struct Ast_Procedure_Call_Expression;
struct Ast_Proc_Declaration;
struct Ast_If;
struct Ast_While;
struct Ast_Break;
struct Ast_Each;
struct Ast_Type_Instantiation;
struct Ast_Array_Subscript;
struct Ast_Statement;
struct Ast_Lexical_Scope;

struct Ast_Declarative_Expression;
struct Ast_Declarative_Assertion;

struct First_Class;


struct Loop_Patch_Address;  // Used to know where the 'break' instructions are...
struct Loop_Patch_List;

struct Lerp_Bytecode_Builder {
    Lerp_Bytecode_Builder(Lerp_Interp *interp);
    ~Lerp_Bytecode_Builder();

    Lerp_Bytecode *build(Ast_Proc_Declaration *declaration);

    int registers_allocated;
    Concatenator *code_buffer;
    Lerp_Interp *interp;

    List statement_debug_info_list;
  protected:
    void emit_code_for_block(Ast_Block *block, int num_arguments = 0, Value_Array *argument_info = NULL);

    int emit_code_for_expression(Ast_Expression *expression);

    int emit_code_for_unary(Ast_Unary_Expression *);
    int emit_code_for_binary(Ast_Binary_Expression *);
    int emit_code_for_procedure_call(Ast_Procedure_Call_Expression *);
    int emit_code_for_implicit_variable(First_Class *);
    int emit_code_for_constant(First_Class *);
    int emit_code_for_lookup(Atom *expression);
    int emit_code_for_lookup(Atom *expression, int owner);
    int emit_code_for_assignment(Ast_Expression *lvalue_expr, int value_register);
    int emit_code_for_return(Ast_Unary_Expression *);
    int emit_code_for_if(Ast_If *);
    int emit_code_for_while(Ast_While *);
    int emit_code_for_break(Ast_Break *);
    int emit_code_for_run_query(Ast_Declarative_Assertion *expression, Ast_Expression *owner = NULL);
    int emit_code_for_lookup(Ast_Binary_Expression *expression);
    int emit_code_for_each(Ast_Each *each);
    int emit_code_for_integer_range_each(Ast_Each *each);
    int emit_code_for_type_instantiation(Ast_Type_Instantiation *inst);
    int emit_code_for_sequence_formation(Ast_Declarative_Assertion *sequence);
    int emit_code_for_optimized_sequence_formation(Ast_Declarative_Assertion *sequence);
    int emit_code_for_sequence_piece(Ast_Declarative_Expression *expression, int sequence_register);
    int emit_code_for_array_subscript(Ast_Binary_Expression *sub);
    int emit_code_for_declarative_expression(Ast_Declarative_Expression *expression);

//    void get_binop_from_token(int operator_token_type, int *operator_type_return, bool *assignment_return);

    int collect_explicit_variables(Ast *expression, Variable ***varaibles_return);
    int add_to_constant_store(First_Class *, bool *new_slot_created_return = NULL);
    int allocate_output_register();
    void pack_register(int register_index);
    void patchup_2b(int location, int value);

    void add_variable_to_scope(Atom *name, int register_number);
    void push_scope();
    void pop_scope();

    void push_patch_list();
    void pop_patch_list(int patch_destination);

    void output_debug_info_for_statement(Ast_Statement *);

    bool type_is_constable(int type);
    bool expression_is_constable(Ast_Declarative_Expression *expression);
    bool sequence_is_constable(Ast_Declarative_Assertion *sequence);
    void finish_constant_sequence_prototype(Ast_Declarative_Assertion *sequence, Decl_Assertion *proto);
    Decl_Expression *convert_ast_to_decl(Ast_Declarative_Expression *ast_expression);

    int note_bytecode_location();
    int find_local_variable(Atom *);

    Ast_Lexical_Scope *current_scope;
    Loop_Patch_List *current_patch_list;

    Atom **arguments;
    int num_arguments;

    List block_stack;  // For blocks whose code we are generating...
    List loop_patch_stack;

    Auto_Array <First_Class *> constants;
    Auto_Array <int> register_for_each_constant;
};

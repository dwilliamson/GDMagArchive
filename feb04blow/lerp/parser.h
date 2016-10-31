struct Lexer;
struct String_Hash_Table;

struct Ast;
struct Ast;
struct Ast_Block;
struct Ast_Statement;
struct Ast_Expression;
struct Ast_Unary_Expression;
struct Ast_Binary_Expression;
struct Ast_Procedure_Call_Expression;
struct Ast_Proc_Declaration;
struct Ast_Type_Instantiation;

struct Ast_Declarative;
struct Ast_Declarative_Assertion;
struct Ast_Declarative_Expression;
struct Ast_Declarative_Assertion;

struct Decl_Constraint;

struct Tree_Changer;

struct Struct_Declaration;
struct Struct_Member;

struct Token;
struct Lerp_Interp;
struct Schema;
struct Type_Constraint;

const int PRE_MADE_INTEGER_MIN = -1;
const int PRE_MADE_INTEGER_MAX = 100;

struct Parser {
    Parser(Lerp_Interp *interp);
    ~Parser();

    void set_input_from_string(char *string);
    void set_input_from_file(void *file);  // The (void *) is actually a (FILE *)
                                           // but we don't want to require stdio... damn C++.

    void parse_declarative_expression_sequence(Ast_Declarative_Assertion *assertion);
    Ast_Declarative_Assertion *parse_declarative_assertion();
    Ast_Declarative_Expression *parse_declarative_expression();
    Ast_Declarative_Expression *parse_declarative_expression_interior();
    Schema *parse_struct_declaration();
    bool parse_struct_members(Schema *space);


    LEXPORT Atom *make_atom(Text_Utf8 *name);
    LEXPORT Variable *make_variable(Text_Utf8 *name);
    LEXPORT Integer *make_integer(int value);
    LEXPORT Float *make_float(double value);
    LEXPORT String *make_string(Text_Utf8 *value);
    LEXPORT String *make_string(int length_in_bytes);
    LEXPORT Ast *make_implicit_variable(int value);


    Procedure *get_primitive_function(char *name);

    String_Hash_Table *atom_hash;
    String_Hash_Table *variable_hash;
    String_Hash_Table *primitive_function_hash;

    Lerp_Interp *interp;
    Lexer *lexer;
    Tree_Changer *tree_changer;

    Ast *parse_toplevel();
    Procedure *make_procedure(Atom *name);

  protected:
    Integer *make_integer_really(int value);
    Float *make_float_really(double value);


    Procedure *make_procedure(char *name);

    Ast *parse_directive_toplevel();
    Ast *parse_imperative_toplevel();
    Ast_Declarative *parse_declarative_toplevel();

    Ast_Block *parse_imperative_block();
    Ast_Block *parse_imperative_block_or_statement();
    Ast_Statement *parse_imperative_statement(bool eat_semicolon = true);
    Ast_Type_Instantiation *parse_type_instantiation_list();

    Ast_Expression *parse_imperative_expression();
    Ast_Expression *parse_imperative_expression_helper();
    Ast_Expression *parse_imperative_subexpression();
    Ast_Expression *parse_embedded_each();

    Ast_Proc_Declaration *parse_imperative_proc_declaration();
    void parse_imperative_proc_declaration_arguments(List *results);

    void parse_imperative_procedure_call_arguments(List *results);

    Barrier <Value_Pair *> *parse_directive_string_list();

    Ast_Declarative_Assertion *parse_declarative_query();

    bool expect_and_eat(int token_type);
    int get_operator_precedence(int operator_type);

    void postprocess_declarative_assertion(Ast_Declarative_Assertion *assertion);
    Ast_Block *parse_scopey_block(Atom *starting_name);

    bool parse_db_constraint_item(Decl_Constraint *constraint, List *results);
    Type_Constraint *parse_db_constraint_range();
//    bool parse_db_constraint_qualifier(Decl_Constraint *constraint, List *results);
    void parse_struct_db_constraint(Schema *space);
    Decl_Constraint *parse_db_constraint_interior();
    void parse_separated_expression_list(int separator_type, List *results);

    bool is_unary_operator(Token *);
    bool is_binary_operator(Token *);

    Atom *atom_domain;

    Float *float_zero;
    Float *float_one;
    Integer *pre_made_integers[PRE_MADE_INTEGER_MAX - PRE_MADE_INTEGER_MIN];
};


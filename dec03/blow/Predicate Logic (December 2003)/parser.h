/*
  This implements a recursive descent parser; the output is an AST.
  It's not central to the idea of this month's code so I will not yet
  comment it heavily.
*/

struct Lexer;
struct String_Hash_Table;

struct Parser {
    Parser();
    ~Parser();

    void set_input_from_string(char *string);
    void set_input_from_file(void *file);  // The (void *) is actually a (FILE *)
                                           // but we don't want to require stdio... damn C++.

    Decl_Expression_Sequence *parse_expression_sequence();
    Decl_Expression *parse_expression();
    Decl_Assertion *parse_assertion();

    Atom *make_atom(char *name);
    Variable *make_variable(char *name);

    Primitive_Function *get_primitive_function(char *name);

    String_Hash_Table *atom_hash;
    String_Hash_Table *variable_hash;
    String_Hash_Table *primitive_function_hash;

    Lexer *lexer;

    bool parse_error;
  protected:
    Primitive_Function *make_primitive_function(char *name);

    Decl_Expression *parse_expression_interior();
};


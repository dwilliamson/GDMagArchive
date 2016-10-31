struct Schema;
struct Concatenator;

struct Printer {
    Printer(Lerp_Interp *interp);
    ~Printer();

    void indent(int num_spaces);

    LEXPORT void print_value(First_Class *value);
    LEXPORT void print_namespace(Schema *space);
    LEXPORT void print_expression(Decl_Expression *expression);
    LEXPORT char *get_buffer();
    LEXPORT void output_buffer();

    int indent_level;
    Lerp_Interp *interp;

    Concatenator *concatenator;
    bool debugger_mode;
  protected:
    void print_indent();

    void print_assertion(Decl_Assertion *assertion);
    void print_constraint(Decl_Constraint *constraint);
    void print_custom_type(First_Class *fc);
    void print_database(Database *db);
};

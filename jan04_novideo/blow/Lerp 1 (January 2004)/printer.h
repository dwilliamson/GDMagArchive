struct Schema;
struct Concatenator;

struct Printer {
    Printer(Lerp_Interp *interp);
    ~Printer();

    void indent(int num_spaces);

    void print_value(First_Class *value);
    void print_namespace(Schema *space);
    void print_expression(Decl_Expression *expression);

    int indent_level;
    Lerp_Interp *interp;

    char *get_buffer();
    void output_buffer();

    Concatenator *concatenator;
    bool debugger_mode;
  protected:
    void print_indent();

    void print_assertion(Decl_Assertion *assertion);
    void print_constraint(Decl_Constraint *constraint);
    void print_custom_type(First_Class *fc);
    void print_database(Database *db);
};

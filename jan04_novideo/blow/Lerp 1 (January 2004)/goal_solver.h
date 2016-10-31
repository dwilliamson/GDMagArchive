struct Goal_Solver {
    Goal_Solver(Lerp_Interp *interp);

    void direct_match(Database *db, Decl_Expression *expression, Matched_Goal **results, Binding *extant_bindings = NULL);
    void match_conjunction_series(Database *database, Decl_Assertion *sequence, Binding **results);
    int remove_direct_facts(Database *db, Decl_Expression *expression, bool *by_implication_return);
    int find_direct_literal_fact(Database *db, Decl_Expression *expression, Decl_Assertion **results, bool *by_implication_return);
    int find_direct_literal_fact_from_one_value(Database *db, First_Class *value, Decl_Assertion **results, bool *by_implication_return);

  protected:
    Lerp_Interp *interp;

    void single_direct_match(Database *db, Decl_Expression *goal_expression, Decl_Assertion *assertion, Matched_Goal **results, Binding *extant_bindings = NULL);
    bool match_conjunction_series(Database *db, Decl_Assertion *sequence, Decl_Expression *cursor, Binding *bindings, Binding **results);
    Matched_Goal *match_two_expressions(Decl_Expression *a, Decl_Assertion *b_assertion, Binding **downward_bindings, Binding *extant_bindings);
    bool match_two_expressions_cheaply(Decl_Expression *a, Decl_Expression *b);

    First_Class *find_binding(Binding *bindings, First_Class *variable);
    Matched_Goal *xref_goal(Matched_Goal *orig_goal, Binding *sub_binding_list);
    void duplicate_and_xref_goal(Matched_Goal *orig_goal,
                                 Binding *sub_matches, Matched_Goal **results);


    Matched_Goal *get_matched_goal();
    Binding *get_binding();
    bool arguments_match(First_Class *goal_arg, First_Class *assertion_arg, Binding **downward_bindings);
    Binding *copy_binding(Binding *old_binding);
//    List *copy_binding_list(List *list);
    Binding *merge_bindings(Binding *a, Binding *b);
};

#define GS_NEW(type) new (GET_MEMORY((interp), type)) type()



struct Tree_Changer {
    Tree_Changer(Lerp_Interp *);

    Lerp_Interp *interp;

    Ast_Expression *postprocess(Ast_Expression *);

  protected:
    Ast_Expression *postprocess(Ast_Unary_Expression *);
    Ast_Expression *postprocess(Ast_Binary_Expression *);
    Ast_Expression *postprocess(Ast_Procedure_Call_Expression *);

    Ast_Binary_Expression *left_associate(Ast_Binary_Expression *expression);

    Atom *make_unique_variable();

    Ast *prepend_to_body(Ast *body, Ast_Statement *statement);

    int next_iterator_variable_index;
};

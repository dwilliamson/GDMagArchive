struct Ast_Directive_Module : public Ast {  
    Ast_Directive_Module();
    
    Barrier <Value_Pair *> *module_list;
};


inline Ast_Directive_Module::Ast_Directive_Module() : Ast() {
    type = AST_DIRECTIVE_MODULE;
}




struct Ast_Directive_Load : public Ast {  
    Ast_Directive_Load();
    
    Barrier <Value_Pair *> *file_list;
};


inline Ast_Directive_Load::Ast_Directive_Load() : Ast() {
    type = AST_DIRECTIVE_LOAD;
}




struct Ast_Directive_Import : public Ast {  
    Ast_Directive_Import();
    Ast_Expression *source_value;
};


inline Ast_Directive_Import::Ast_Directive_Import() : Ast() {
    type = AST_DIRECTIVE_IMPORT;
}


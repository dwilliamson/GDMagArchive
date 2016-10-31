struct Lerp_Interp;
struct Module_Load_Info;
struct File_Load_Info;
struct Ast_Directive_Import;

struct Code_Manager {
    Code_Manager(Lerp_Interp *interp);
    ~Code_Manager();

    void load_default_modules();
    void enumerate_gc_roots();

    void add_file_to_load(String *path, String *string);
    void add_module_to_load(String *path, String *string);
    void process_import_directive(Ast_Directive_Import *import);

    void perform_loads();

  protected:
    bool module_already_considered(Database *db, String *string);
    bool file_already_considered(Database *db, String *string);

    bool load_module(Module_Load_Info *info);
    bool load_file(File_Load_Info *info);

    Module_Load_Info *map_module(char *name);
    void map_module(Module_Load_Info *info);
    void map_module_specifying_path(Module_Load_Info *module, String *path);

    void copy_definitions_into_global_namespace(Database *db);

    void trace_module_db(Database **db_ptr);
    void trace_file_db(Database **db_ptr);

    void do_one_import(Value_Pair *import);

    String *concatenate(String *a, String *b);
    Value_Pair *validate_import_source(Ast_Expression *source);

    Database *modules_to_load;
    Database *files_to_load;

    Database *modules_loaded;
    Database *files_loaded;

    Database *import_directives;

    String *current_path_name;
    Lerp_Interp *interp;
};

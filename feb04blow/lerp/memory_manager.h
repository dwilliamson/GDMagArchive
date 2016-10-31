struct Lerp_Call_Record_Cached;
struct Memory_Arena;


enum {
    TYPE_ENUM_MISC = 0,
    TYPE_ENUM_Atom,
    TYPE_ENUM_Variable,
    TYPE_ENUM_Integer,
    TYPE_ENUM_Float,
    TYPE_ENUM_String,
    TYPE_ENUM_Database,
    TYPE_ENUM_Lvalue,
    TYPE_ENUM_Schema,
    TYPE_ENUM_Blob,
    TYPE_ENUM_Decl_Assertion,      // 10
    TYPE_ENUM_Decl_Expression,
    TYPE_ENUM_Decl_Expression_Procedural,
    TYPE_ENUM_Procedure,
    
    TYPE_ENUM_Instance,  // For custom types...

    TYPE_ENUM_Lerp_Call_Record,

    TYPE_ENUM_Matched_Goal,
    TYPE_ENUM_Binding,

    TYPE_ENUM_Value_Array,
    TYPE_ENUM_Value_Pair,
    TYPE_ENUM_Bytecode,
    TYPE_ENUM_Decl_Constraint,

    TYPE_ENUM_Ast_Each,
    TYPE_ENUM_Ast_Procedure_Call_Expression,
    TYPE_ENUM_Ast_Unary_Expression,
    TYPE_ENUM_Ast_Binary_Expression,
    TYPE_ENUM_Ast_Type_Instantiation,
    TYPE_ENUM_Ast_If,
    TYPE_ENUM_Ast_While,
    TYPE_ENUM_Ast_Break,
    TYPE_ENUM_Ast_Statement,
    TYPE_ENUM_Ast_Proc_Declaration,
    TYPE_ENUM_Ast_Block,
    TYPE_ENUM_Ast_Declarative_Expression,
    TYPE_ENUM_Ast_Declarative_Expression_Procedural,
    TYPE_ENUM_Ast_Declarative_Assertion,
    TYPE_ENUM_Ast_Scope_Record,
    TYPE_ENUM_Ast_Array_Subscript,
    TYPE_ENUM_Ast_Implicit_Variable,

    TYPE_ENUM_Loop_Patch_List,
    TYPE_ENUM_Loop_Patch_Address,

    TYPE_ENUM_Ast_Directive_Load,
    TYPE_ENUM_Ast_Directive_Module,
    TYPE_ENUM_Ast_Directive_Import,

    TYPE_ENUM_File_Load_Info,
    TYPE_ENUM_Module_Load_Info,

    TYPE_ENUM_Uninitialized,

    NUM_TYPE_ENUMS
};


const int GC_COLOR_BLACK_OR_WHITE_1 = 0x1000;
const int GC_COLOR_BLACK_OR_WHITE_2 = 0x1010;
const int GC_COLOR_RED              = 0x1101;
const int GC_COLOR_GRAY             = 0x0110;

const int MAX_POINTERS_TO_FOLLOW = 1000;   // @Robustness @Memory: Make a real system for this


struct Lerp_Memory_Manager {
    Lerp_Memory_Manager(Lerp_Interp *interp);
    ~Lerp_Memory_Manager();

    LEXPORT void *allocate(int num_bytes, int type_id = 0);
    LEXPORT void print_allocations();

    LEXPORT Lerp_Call_Record *create_call_record(int num_registers);
    LEXPORT void release_call_record(Lerp_Call_Record *record);

    LEXPORT Decl_Expression *create_decl_expression(int num_arguments);
    LEXPORT Lerp_Bytecode *create_bytecode(int num_constants);

    LEXPORT Value_Array *create_value_array(int num_values);

    LEXPORT void gc();

    LEXPORT First_Class *gc_consider(void *ptr);

    Memory_Arena *memory_oldspace, *memory_newspace;

  protected:
    char *get_allocation_pointer();
    void set_allocation_pointer_with_alignment(char *pointer);

    int num_pointers_to_follow;
    Barrier <First_Class *> **pointers_to_follow[MAX_POINTERS_TO_FOLLOW];

    int gc_color_black;
    int gc_color_white;
    int allocation_color;

    int allocation_tracker[NUM_TYPE_ENUMS];
    int allocation_count[NUM_TYPE_ENUMS];
    Lerp_Call_Record *create_call_record_really(int num_registers);
    void create_cached_call_record();

    Lerp_Interp *interp;
    Lerp_Call_Record_Cached *cached_call_records;

    void swap_gc_spaces();
    void swap_gc_colors();

    First_Class *gc_move(First_Class *oldspace);
    void handle_pointer(Barrier <First_Class *> *oldspace_barrier, First_Class **newspace_ptr);
    First_Class *get_easy_pointer(Barrier <First_Class *> *barrier);
};

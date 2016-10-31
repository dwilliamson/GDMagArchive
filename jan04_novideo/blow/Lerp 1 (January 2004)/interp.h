struct Goal_Solver;
struct Parser;
struct Lerp_Bytecode;
struct Lerp_Bytecode_Builder;
struct Lerp_Bytecode_Runner;
struct Schema;
struct Blob;
struct Integer_Hash_Table;
struct Printer;
struct Token;
struct Lerp_Memory_Manager;
struct Lerp_Disassembler;
struct Lerp_Profiler;

#include "memory_manager.h"

struct Lerp_Interp {
    Lerp_Interp();
    ~Lerp_Interp();

    Lerp_Memory_Manager *memory_manager;
    Goal_Solver *goal_solver;
    Parser *parser;
    Lerp_Bytecode_Builder *bytecode_builder;
    Lerp_Bytecode_Runner *bytecode_runner;
    Database *global_database;  // @Refactor: Should be rolled into global_namespace?

    First_Class *uninitialized;
    Printer *printer;
    Lerp_Disassembler *disassembler;
    Lerp_Profiler *profiler;

    Blob *the_null_blob;
    Variable *the_anonymous_variable;

    void add_assertion(Decl_Assertion *assertion);
    Database *instantiate(Schema *space);

    Integer_Hash_Table *custom_types_by_id;

    unsigned long next_custom_type_tag;

    Atom *type_atoms[ARG_NUM_TYPES];  // The name of each type, in Atom form.
    Atom *member_atom;

    void report_error(char *format, ...);
    void report_error(Token *token, char *format, ...);
    void report_bytecode_error(char *format, ...);
    
    void stack_dump_one_context(Lerp_Call_Record *context);
    void stack_dump();


    bool runtime_error;
    bool parse_error;
    bool bytecode_error;

    bool generate_debug_info;

  protected:
    int find_current_line_number();
};


// GC_NEW, for convenience, uses local variable 'interp' which must be declared.

#define GET_MEMORY(interp, type) ((interp)->memory_manager->allocate(sizeof(type), TYPE_ENUM_##type))
#define GC_NEW(type) new (GET_MEMORY((interp), type)) type()

inline void Database::add_assertion(Lerp_Interp *interp, Decl_Expression *expression) {
    Decl_Assertion *assertion = GC_NEW(Decl_Assertion);
    assertion->expression = ToBarrier(expression);
    add_assertion(assertion);
}

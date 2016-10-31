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
struct Code_Manager;
struct Lerp_Interp_Os_Specific;
struct Lerp_Thread;
struct Random_Generator;

#include "memory_manager.h"

struct Lerp_Call_Record;

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
    Code_Manager *code_manager;

    Blob *the_null_blob;
    Variable *the_anonymous_variable;

    List threads;
    void add_assertion(Decl_Assertion *assertion);
    void enumerate_gc_roots();
    Lerp_Thread *create_thread(Procedure *procedure);
    LEXPORT Lerp_Thread *create_thread();
    void run();


    Random_Generator *random_generator;
    Integer_Hash_Table *custom_types_by_id;

    unsigned long next_custom_type_tag;

    Atom *type_atoms[ARG_NUM_TYPES];  // The name of each type, in Atom form.
    Atom *member_atom;

    LEXPORT Database *instantiate(Schema *space);
    LEXPORT double get_time_in_seconds();

    LEXPORT void report_error(char *format, ...);
    LEXPORT void report_error(Token *token, char *format, ...);
    LEXPORT void report_bytecode_error(char *format, ...);
    
    LEXPORT void process_os_events();
    LEXPORT unsigned long make_procedure_hash_key();

    void stack_dump_one_context(Lerp_Call_Record *context);
    void stack_dump();


    bool parse_error;
    bool bytecode_error;
    bool runtime_error;

    bool generate_debug_info;
    bool profiling;

  protected:
    int next_thread_id;

    int find_current_line_number();

    void init_os_specific();
    Lerp_Interp_Os_Specific *os_specific;
};


// GC_NEW, for convenience, uses local variable 'interp' which must be declared.

#define GET_MEMORY(interp, type) ((interp)->memory_manager->allocate(sizeof(type), TYPE_ENUM_##type))
#define GC_NEW(type) new (GET_MEMORY((interp), type)) type()

inline void Database::add_assertion(Lerp_Interp *interp, Decl_Expression *expression) {
    Decl_Assertion *assertion = GC_NEW(Decl_Assertion);
    assertion->expression = ToBarrier(expression);
    add_assertion(assertion);
}

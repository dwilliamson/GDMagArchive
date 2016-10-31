struct Lerp_Thread;
struct Procedure_Profiling_Info;
struct Thread_Profiling_Info;
struct Integer_Hash_Table;

// @Refactor @Port: This is a highly machine-specific file,
// so we ought to factor it into a subdirectory.

typedef __int64 Profiling_Int64;

struct Lerp_Profiler {
    Lerp_Profiler(Lerp_Interp *interp);

    void set_current_thread(Lerp_Thread *thread);
    void enumerate_gc_roots();

    Lerp_Interp *interp;
    LEXPORT void update();

    double last_update_time;
    Profiling_Int64 last_update_ticks;
    Barrier <Lerp_Thread *> *current_thread;

    void enter_procedure(Procedure *old_procedure, Procedure *new_procedure);
    void enter_procedure(Procedure *new_procedure);
    void exit_procedure(Procedure *old_procedure, Procedure *new_procedure);
    void exit_procedure(Procedure *procedure);

  protected:
    Procedure_Profiling_Info *get_profiling_info(Procedure *procedure);
    Thread_Profiling_Info *get_thread_info(Lerp_Thread *thread);

    Integer_Hash_Table *thread_info_hash;
};



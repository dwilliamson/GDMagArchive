// @Refactor @Port: This is a highly machine-specific file,
// so we ought to factor it into a subdirectory.

struct Lerp_Profiler {
    Lerp_Profiler(Lerp_Interp *interp);

    Lerp_Interp *interp;
    void update();

    double last_update_time;
    Profiling_Int64 last_update_ticks;

    inline void enter_procedure(Procedure *old_procedure, Procedure *new_procedure);
    inline void enter_procedure(Procedure *new_procedure);
    inline void exit_procedure(Procedure *old_procedure, Procedure *new_procedure);
    inline void exit_procedure(Procedure *procedure);
};

inline void os_fast_get_integer_timestamp(Profiling_Int64 *result) {
    int *dest_low = (int *)(result);
    int *dest_high = dest_low + 1;

    __asm {
        rdtsc;
        mov    ebx, dest_low;
        mov    ecx, dest_high;
        mov    [ebx], eax;
        mov    [ecx], edx;
    }
}


inline void Lerp_Profiler::enter_procedure(Procedure *old_procedure,
                                           Procedure *new_procedure) {
    os_fast_get_integer_timestamp(&new_procedure->t_self_start);
    old_procedure->total_self_ticks += new_procedure->t_self_start - old_procedure->t_self_start;
    new_procedure->total_entry_count++;
}

inline void Lerp_Profiler::enter_procedure(Procedure *new_procedure) {
    os_fast_get_integer_timestamp(&new_procedure->t_self_start);
    new_procedure->total_entry_count++;
}

inline void Lerp_Profiler::exit_procedure(Procedure *old_procedure,
                                          Procedure *new_procedure) {
    Profiling_Int64 t_end;
    os_fast_get_integer_timestamp(&t_end);

    old_procedure->total_self_ticks += (t_end - old_procedure->t_self_start);
    new_procedure->t_self_start = t_end;
}

inline void Lerp_Profiler::exit_procedure(Procedure *procedure) {
    Profiling_Int64 t_end;
    os_fast_get_integer_timestamp(&t_end);

    procedure->total_self_ticks += (t_end - procedure->t_self_start);
}



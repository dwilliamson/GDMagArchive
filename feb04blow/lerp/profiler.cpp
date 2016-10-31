#include "general.h"
#include "profiler_ticks.h"
#include "interp.h"
//#include "gc.h"  
#include <math.h>

#include "hash_table.h"
#include "thread.h"

// @Speed:
// Profiling stuff is currently SLOW.  Used to be inlined methods and all that,
// but then decided I really wanted the language to be all threaded and, well,
// here we go.



#define Hash_Foreach(t, v) {\
void **__p = (void **)&(v);\
int __i;\
for (__i  =0; __i < (t)->table_size; __i++) {\
  List *__l = &(t)->lists[__i];\
  Integer_Hash_Info *__hi;\
  Foreach(__l, __hi) {\
    *__p = __hi->value;

#define Hash_Endeach } Endeach }}


struct Thread_Profiling_Info {
    Thread_Profiling_Info();
//    Barrier <Lerp_Thread *> *thread;
    Lerp_Thread *thread;
    Integer_Hash_Table hash;
};

struct Procedure_Profiling_Info {
    Procedure_Profiling_Info();
    Barrier <Procedure *> *procedure;

    Profiling_Int64 t_self_start;
    Profiling_Int64 total_self_ticks;
    int total_entry_count;
    float average_seconds;
};

/*
    t_self_start = 0;
    total_self_ticks = 0;
    total_entry_count = 0;
    average_seconds = 0;
*/

Thread_Profiling_Info::Thread_Profiling_Info() : hash(50) {
    thread = NULL;
}

Procedure_Profiling_Info::Procedure_Profiling_Info() {
    procedure = NULL;
    t_self_start = 0;
    total_self_ticks = 0;
    total_entry_count = 0;
    average_seconds = 0;
}

void os_fast_get_integer_timestamp(Profiling_Int64 *result) {
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

Lerp_Profiler::Lerp_Profiler(Lerp_Interp *_interp) {
    interp = _interp;
    last_update_time = 0;
    last_update_ticks = 0;
    current_thread = NULL;

    thread_info_hash = new Integer_Hash_Table(10);  // @Leak
}

void Lerp_Profiler::set_current_thread(Lerp_Thread *thread) {
    current_thread = ToBarrier(thread);
}

void Lerp_Profiler::enumerate_gc_roots() {
    current_thread = ToBarrier((Lerp_Thread *)interp->memory_manager->gc_consider(current_thread));

    Thread_Profiling_Info *thread_info;
    Hash_Foreach(thread_info_hash, thread_info) {
        Procedure_Profiling_Info *proc_info;
        Hash_Foreach(&thread_info->hash, proc_info) {
            proc_info->procedure = ToBarrier((Procedure *)interp->memory_manager->gc_consider(proc_info->procedure));
        } Hash_Endeach;
    } Hash_Endeach;
}

Thread_Profiling_Info *Lerp_Profiler::get_thread_info(Lerp_Thread *thread) {
    Thread_Profiling_Info *info = NULL;
    thread_info_hash->find(thread->thread_id, (void **)&info);
    if (!info) {
        info = new Thread_Profiling_Info();
        info->thread = thread; // ToBarrier(thread);
        thread_info_hash->add(thread->thread_id, info);
    } 

    return info;
}


Procedure_Profiling_Info *Lerp_Profiler::get_profiling_info(Procedure *procedure) {
    Thread_Profiling_Info *thread_info = get_thread_info(current_thread->read());
    assert(thread_info);

    
    unsigned long key;
    if (procedure) {
        key = procedure->hash_key;
    } else {
        key = 1;  // NULL is a valid procedure argument; 0 is an invalid key.
    }

    assert(key != 0);

    Procedure_Profiling_Info *proc_info = NULL;
    thread_info->hash.find(key, (void **)&proc_info);
    if (!proc_info) {
        proc_info = new Procedure_Profiling_Info();
        thread_info->hash.add(key, proc_info);
    }

    return proc_info;
}

void Lerp_Profiler::update() {
    if (!interp->profiling) return;

    float time_to_reach_90_percent = 0.3f;   // .3 seconds

    double now = interp->get_time_in_seconds();
    if (last_update_time == 0) {
        last_update_time = now;
        return;
    }

    double dt = now - last_update_time;
    last_update_time = now;

    double k = pow(0.1, dt / time_to_reach_90_percent);


    Profiling_Int64 now_ticks;
    os_fast_get_integer_timestamp(&now_ticks);
    Profiling_Int64 tick_difference = now_ticks - last_update_ticks;

    if (tick_difference == 0) return;
    double ticks_to_seconds = dt / tick_difference;
    last_update_ticks = now_ticks;


    Database *db = interp->global_database;
    Decl_Assertion *assertion;
    assertion = db->assertions->read();

    for (; assertion; assertion = assertion->next->read()) {
        Decl_Expression *expression = assertion->expression->read();
        if (expression->num_arguments != 3) continue;
        if (expression->arguments[0]->read() != interp->member_atom) continue;

        Procedure *proc = (Procedure *)expression->arguments[2]->read();
        if (proc->type != ARG_IMPERATIVE_PROCEDURE) continue;

        // We have a procedure member!
        Procedure_Profiling_Info *info = get_profiling_info(proc);
        float old_value = info->average_seconds;
        float new_value = info->total_self_ticks * ticks_to_seconds;
        info->average_seconds = old_value * k + new_value * (1 - k);
    }
}


void Lerp_Profiler::enter_procedure(Procedure *old_procedure,
                                    Procedure *new_procedure) {
    if (!interp->profiling) return;
    Procedure_Profiling_Info *old_info = get_profiling_info(old_procedure);
    Procedure_Profiling_Info *new_info = get_profiling_info(new_procedure);

    os_fast_get_integer_timestamp(&new_info->t_self_start);
    old_info->total_self_ticks += new_info->t_self_start - old_info->t_self_start;
    new_info->total_entry_count++;
}

void Lerp_Profiler::enter_procedure(Procedure *new_procedure) {
    if (!interp->profiling) return;
    Procedure_Profiling_Info *new_info = get_profiling_info(new_procedure);
    os_fast_get_integer_timestamp(&new_info->t_self_start);
    new_info->total_entry_count++;
}

void Lerp_Profiler::exit_procedure(Procedure *old_procedure,
                                   Procedure *new_procedure) {
    if (!interp->profiling) return;
    Procedure_Profiling_Info *old_info = get_profiling_info(old_procedure);
    Procedure_Profiling_Info *new_info = get_profiling_info(new_procedure);

    Profiling_Int64 t_end;
    os_fast_get_integer_timestamp(&t_end);

    old_info->total_self_ticks += (t_end - old_info->t_self_start);
    new_info->t_self_start = t_end;
}

void Lerp_Profiler::exit_procedure(Procedure *procedure) {
    if (!interp->profiling) return;
    Profiling_Int64 t_end;
    os_fast_get_integer_timestamp(&t_end);

    Procedure_Profiling_Info *info = get_profiling_info(procedure);
    info->total_self_ticks += (t_end - info->t_self_start);
}



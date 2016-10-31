#include "general.h"
#include "profiler_ticks.h"
#include "interp.h"

#include <math.h>

Lerp_Profiler::Lerp_Profiler(Lerp_Interp *_interp) {
    interp = _interp;
    last_update_time = 0;
    last_update_ticks = 0;
}

double get_time_in_seconds();

void Lerp_Profiler::update() {
    float time_to_reach_90_percent = 0.3f;   // .3 seconds

    double now = get_time_in_seconds();
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
        float old_value = proc->average_seconds;
        float new_value = proc->total_self_ticks * ticks_to_seconds;
        proc->average_seconds = old_value * k + new_value * (1 - k);
    }
}

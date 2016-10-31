#include "app_shell.h"
#include "profiler_lowlevel.h"

Program_Zone the_uncharted_zone;

int Profiling::current_zone = 0;
int Profiling::stack_pos = 0;
int Profiling::num_zones = 0;
int Profiling::zone_stack[MAX_PROFILING_STACK_DEPTH];
Program_Zone *Profiling::zone_pointers_by_index[MAX_PROFILING_ZONES];

Program_Zone::Program_Zone(char *_name) {
    initialized = 1;
    name = _name;
    index = ++(Profiling::num_zones);
    assert(index < MAX_PROFILING_ZONES);
    Profiling::zone_pointers_by_index[index] = this;

    total_self_ticks = 0.0;
    total_hier_ticks = 0.0;
    t_self_start = 0.0;
    t_hier_start = 0.0;

    total_entry_count = 0;
}

Program_Zone::Program_Zone() {
    initialized = 1;
    name = "*uncharted*";
    index = 0;
    assert(index < MAX_PROFILING_ZONES);
    assert(Profiling::zone_pointers_by_index[index] == NULL);
    Profiling::zone_pointers_by_index[index] = this;

    total_self_ticks = 0.0;
    total_hier_ticks = 0.0;
    t_self_start = 0.0;
    t_hier_start = 0.0;

    total_entry_count = 0;
}

void gamelib_profile_init() {
    Program_Zone *zone = Profiling::zone_pointers_by_index[0];
    assert(zone != NULL);
    
    os_fast_get_integer_timestamp(&zone->t_self_start);
    zone->t_hier_start = zone->t_self_start;
}

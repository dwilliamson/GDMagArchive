/*
  This file is about associating regions of your program with
  a certain name, so that the memory and CPU-time profiling systems
  can do a reasonable job of telling you where resources are being
  spent.

  This works by declaring some classes that, when they construct,
  push a zone marker onto a hidden stack somewhere; then you run
  a bunch of code in your function; then, when they destruct (you
  leave the scope of the function or code block), they pull
  themselves off that stack.  

  To declare a new zone marker, do this somewhere in your global scope:

    Define_Zone(zone_marker_name);

  To declare that a particular function is in this profiling zone:

    Profile_Scope(zone_marker_name);

  Of course this takes a little bit of CPU time whenever you enter
  or leave a scope.  If you don't put these in heavily trafficked
  functions, then the impact is minimal.  But you can turn them
  entirely off, so that they compile out to nothing, by setting
  the value of PROFILE_HOOKS to 0.

  If PROFILE_HOOKS is 1, all this stuff will be installed.  The
  generated data is accessible at runtime, and can be displayed
  from a HUD or whatnot.
*/

typedef __int64 Profiling_Int64;

struct Program_Zone {
    char *name;
    int initialized;
    int index;

    Profiling_Int64 total_self_ticks;
    Profiling_Int64 total_hier_ticks;

    Profiling_Int64 t_self_start;
    Profiling_Int64 t_hier_start;

    int total_entry_count;

    Program_Zone(char *name);
    Program_Zone();
};

const int MAX_PROFILING_ZONES = 512;
const int MAX_PROFILING_STACK_DEPTH = 16384;

struct Profiling {
    static int current_zone;
    static int stack_pos;
    static int num_zones;
    static int zone_stack[MAX_PROFILING_STACK_DEPTH];
    static Program_Zone *zone_pointers_by_index[MAX_PROFILING_ZONES];

    inline static void Enter_Zone(Program_Zone &zone);
    inline static void Exit_Zone();
};


#define PROFILE_HOOKS 1

inline void os_fast_get_integer_timestamp(Profiling_Int64 *result) {
    Profiling_Int64 val;
    int *dest_low = (int *)(&val);
    int *dest_high = dest_low + 1;

    __asm {
        rdtsc;
        mov    ebx, dest_low;
        mov    ecx, dest_high;
        mov    [ebx], eax;
        mov    [ecx], edx;
    }

    *result = val;
}

#ifdef PROFILE_HOOKS

inline void Profiling::Enter_Zone(Program_Zone &zone) {
    Program_Zone *old_zone = zone_pointers_by_index[current_zone];
    zone_stack[stack_pos] = current_zone;
    current_zone = zone.index;

    os_fast_get_integer_timestamp(&zone.t_self_start);
    zone.t_hier_start = zone.t_self_start;

    old_zone->total_self_ticks += zone.t_self_start - old_zone->t_self_start;
    zone.total_entry_count++;

    stack_pos++;
}

inline void Profiling::Exit_Zone() {
    Program_Zone *zone = zone_pointers_by_index[current_zone];

    --stack_pos;

    Profiling_Int64 t_end;
    os_fast_get_integer_timestamp(&t_end);

    zone->total_self_ticks += (t_end - zone->t_self_start);
    zone->total_hier_ticks += (t_end - zone->t_hier_start);
   
    current_zone = zone_stack[stack_pos];

    Program_Zone *new_zone = zone_pointers_by_index[current_zone];
    new_zone->t_self_start = t_end;
}


struct Profile_Scope_Var {
    inline Profile_Scope_Var(Program_Zone &zone);
    inline ~Profile_Scope_Var();
};

inline Profile_Scope_Var::Profile_Scope_Var(Program_Zone &zone) {
    Profiling::Enter_Zone(zone);
}

inline Profile_Scope_Var::~Profile_Scope_Var() {
    Profiling::Exit_Zone();
}

#define Profile_Scope(x) Profile_Scope_Var scope_##__LINE__(__ ## x)
#define Define_Zone(name) Program_Zone __ ## name(#name);

#else  // if notdef PROFILE_HOOKS

#define Profile_Scope(x)
#define Define_Zone(name)

#endif

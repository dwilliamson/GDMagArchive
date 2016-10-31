#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "scripting.h"
#include "script_compiler.h"

#include "worlds.h"

#include <math.h>
#include <gl/glu.h>


const int MAX_COMPILED_SCRIPTS = 10;
Compiled_Script *compiled_scripts[MAX_COMPILED_SCRIPTS];

char *app_name = "Timescript";
int current_world_index;
double last_app_shell_time = 0;

Scripting_System *scripting_system;

void draw_scene() {
    int world_index = current_world_index;

    double now = app_shell->get_time();
    double dt;
    if (last_app_shell_time == 0) {
        dt = 0;
    } else {
        dt = now - last_app_shell_time;
    }

    last_app_shell_time = now;

    Compiled_Script *script = compiled_scripts[world_index];
    if (script) scripting_system->variable_binding_list = script->global_variables;
    world_update(world_index, dt);

    if (script) scripting_system->run_script(compiled_scripts[world_index]);

    world_draw(world_index);
}


void handle_keydown(int key) {
    if (key == '1') current_world_index = 1;
    if (key == '2') current_world_index = 2;
    if (key == '3') current_world_index = 3;

    world_handle_key_down(current_world_index, key);
}

void handle_keyup(int) {
}

void app_init() {
	scripting_system = new Scripting_System();
	scripting_system->init();
    
    Script_Compiler *script_compiler = new Script_Compiler();
    int i;
    for (i = 1; i <= NUM_WORLDS; i++) {
        char index_letter = '0' + i;
        char buf[BUFSIZ];
        sprintf(buf, "data\\script%c", index_letter);

        FILE *f = fopen(buf, "r");
        if (f) {
            compiled_scripts[i] = script_compiler->compile_script(f);
            fclose(f);
            scripting_system->variable_binding_list = compiled_scripts[i]->global_variables;
        }

        world_init(i);
    }

    current_world_index = 1;
}


void Scripting_System::report_error(char *s) {
    if (log_file == NULL) {
        log_file = fopen("error_log_from_runner.txt", "wt");
        if (log_file == NULL) return;
    }

    fprintf(log_file, "Script #%d, Error in line %d: \n    %s\n", current_world_index, line_number, s);
    fflush(log_file);
}


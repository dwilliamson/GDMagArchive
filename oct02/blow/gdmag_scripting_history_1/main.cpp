#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "scripting.h"

#include <math.h>
#include <gl/glu.h>

char *app_name = "Timescript";

Scripting_System *scripting_system;

const float NUM_STANDARD_DEVIATIONS = 1.7f;
bool extra_slowness = false;

// Everything this function does is 2-dimensional; I am just using
// a Vector3 for the vector operations to avoid having to do all kinds
// of code-writing for the Vector2 class.  The 'z' coordinate of these
// 3-vectors is always 0.
void draw_one_ellipse(Vector2 mean, Covariance2 *covariance, float scale) {
    Vector3 axis[2];
    float lambda[2];

    covariance->find_eigenvectors(lambda, axis);
    
    float len0 = sqrt(lambda[0]) * NUM_STANDARD_DEVIATIONS;
    float len1 = sqrt(lambda[1]) * NUM_STANDARD_DEVIATIONS;

    Vector3 axis0 = axis[0];
    Vector3 axis1 = axis[1];

    axis0.scale(len0 * scale);
    axis1.scale(len1 * scale);

    app_shell->triangle_mode_begin();

    const int NUM_VERTICES = 50;
    static Vector3 vertices[NUM_VERTICES];

    // Generate the vertex coordinates for the ellipse.

    int j;
    for (j = 0; j < NUM_VERTICES; j++) {
        double theta = 2 * M_PI * (j / (double)NUM_VERTICES);
        double ct = cos(theta);
        double st = sin(theta);

        Vector3 a0 = axis0;
        Vector3 a1 = axis1;
        a0.scale(ct);
        a1.scale(st);

        Vector3 pos = a0.add(a1);
        pos.x += mean.x;
        pos.y += mean.y;
        
        vertices[j] = pos;
    }

    // Draw the ellipse.

    glBegin(GL_TRIANGLES);
    for (j = 0; j < NUM_VERTICES; j++) {
        int n0 = j;
        int n1 = (j + 1) % NUM_VERTICES;
        glVertex2f(mean.x, mean.y);
        glVertex2f(vertices[n0].x, vertices[n0].y);
        glVertex2f(vertices[n1].x, vertices[n1].y);
    }
    glEnd();

    app_shell->triangle_mode_end();
}


void draw_variables() {
	Variable_Binding_List *vars = scripting_system->variable_binding_list;

    glColor4f(1, 1, 1, 1);

	int sx = 0;
	int sy = app_shell->screen_height - 40;
    int orig_sy = sy;

	app_shell->text_mode_begin();

	const int BUFFER_LEN = 1000;
	char buf[BUFFER_LEN]; // XXX static buffer size
	
	int i;
	for (i = 0; i < vars->num_bindings; i++) {
		Variable_Binding *binding = &vars->bindings[i];

		strcpy(buf, binding->name); // XXX overflow
		strcat(buf, "  ");
		int len = strlen(buf);
		char *pos = buf + len;
		int len_remaining = BUFFER_LEN - len - 2;

		scripting_system->print_value(binding, pos, len_remaining);
		
		app_shell->draw_text_line(&sx, &sy, buf);
	}

    if (extra_slowness) {
        glColor3f(1, 1, 0);
        sprintf(buf, "EXTRA SLOWNESS: ON (press 'S' to deactivate)");
        int len = strlen(buf);
        app_shell->draw_text_line(&sx, &sy, buf);
    }

	app_shell->text_mode_end();
}

void draw_scene() {
	int x, y;
	x = app_shell->mouse_pointer_x;
	y = app_shell->mouse_pointer_y;

	// We divide y by the screen width, not the height... this is
	// so that the y axis of the coordinate system won't be
	// scaled with respect to the x.
	float fx = x / (float)app_shell->screen_width;
	float fy = y / (float)app_shell->screen_width;

	double now = app_shell->get_time();

	Variable_Binding_List *vars = scripting_system->variable_binding_list;

	Vector2 mouse_pos;
	mouse_pos.set(fx, fy);
	Variable_Binding *mouse = vars->lookup("mouse");
	if (mouse) mouse->set(vars, mouse_pos);

	Variable_Binding *time = vars->lookup("time");
	if (time) time->set(vars, now);
	
	scripting_system->update(now);


    int i;
    float scale = app_shell->screen_width;

	if (mouse) {
        // Draw covariances.

		for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
			int index = NUM_HISTORY_SLOTS - 1 - i;
			assert(index >= 0);


            float scale_max = 0.85f;
            float scale_base = 1.0f - scale_max;
            float color_scale = (1.0f - index / (float)(NUM_HISTORY_SLOTS - 1)) * scale_max + scale_base;
            glColor4f(0, color_scale * 0.5f, color_scale, 1);

            Vector2 mean = mouse->vector2.history_values[index];

            Covariance2 *covariance = &mouse->vector2.history_covariances[index];
            Covariance2 relative;
            covariance->translate(&relative, -mean.x, -mean.y);

            mean.x *= scale;
            mean.y *= scale;
            draw_one_ellipse(mean, &relative, scale);
		}
        
 		app_shell->line_mode_begin();

		glColor3f(1, 1, 1);


        // Draw the raw data for mouse positions, linearly interpolated.
		glBegin(GL_LINE_STRIP);

		for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
			int index = NUM_HISTORY_SLOTS - 1 - i;
			assert(index >= 0);

            Vector2 pos;
            
			glVertex2f(mouse->vector2.history_values[index].x * scale,
					   mouse->vector2.history_values[index].y * scale);
		}

		glVertex2f(mouse->vector2.instantaneous_value.x * scale,
				   mouse->vector2.instantaneous_value.y * scale);
		glEnd();


		app_shell->line_mode_end();

	}

    draw_variables();
        
    if (extra_slowness) app_shell->sleep(0.08f);
}

void handle_keydown(int key) {
    if (key == 'S') extra_slowness = !extra_slowness;
}

void handle_keyup(int) {
}

void app_init() {
	scripting_system = new Scripting_System();
	scripting_system->init();

	Variable_Binding_List *vars = scripting_system->variable_binding_list;
	assert(vars != NULL);

	vars->declare("mouse", VARTYPE_VECTOR2);
	vars->declare("time", VARTYPE_SCALAR);
}



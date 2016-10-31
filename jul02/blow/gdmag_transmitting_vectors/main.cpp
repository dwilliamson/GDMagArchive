#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "app_shell.h"
#include "relaxer_vector3_unit.h"
#include "relaxer_vector2.h"
#include "relaxer.h"

char *app_name = "Vector Relaxation";

Loaded_Texture_Info spot_texture;

const int MODE_2D = 0;
const int MODE_3D_UNIT = 1;

int global_app_mode = -1;

double relax_timestep_2d = 0.00004f;
double relax_timestep_3d = 0.05f;
bool should_draw_cells = false;

Relaxer_Vector3_Unit *relaxer_vector3_unit;
Relaxer_Vector2 *relaxer_vector2;



void enter_mode(int mode) {
	if (relaxer_vector3_unit) delete relaxer_vector3_unit;
	if (relaxer_vector2) delete relaxer_vector2;

	relaxer_vector3_unit = NULL;
	relaxer_vector2 = NULL;

    global_app_mode = mode;

	if (mode == MODE_2D) {
		relaxer_vector2 = new Relaxer_Vector2();
		relaxer_vector2->init(200);
	}
	
	if (mode == MODE_3D_UNIT) {
		relaxer_vector3_unit = new Relaxer_Vector3_Unit();
		relaxer_vector3_unit->init(256);
	}
}	

void save_vectors(int num_vectors, Vector3 *vectors, char *filename) {
	FILE *f = fopen(filename, "wt");
	if (f == NULL) return;

	fprintf(f, "%d\n", num_vectors);

	int i;
	for (i = 0; i < num_vectors; i++) {
		fprintf(f, "%f %f %f\n",
				vectors[i].x, vectors[i].y, vectors[i].z);
	}
	
	fclose(f);
}

void handle_keydown(int key) {
	if (global_app_mode == MODE_2D) {
		if (key == 'Q') relax_timestep_2d *= 0.5f;
		if (key == 'W') relax_timestep_2d *= 2.0f;
	}
	
	if (global_app_mode == MODE_3D_UNIT) {
		if (key == 'Q') relax_timestep_3d *= 0.5f;
		if (key == 'W') relax_timestep_3d *= 2.0f;
	}
	
	if (key == 'C') should_draw_cells = !should_draw_cells;
	
	if (key == '1') enter_mode(MODE_2D);
	if (key == '2') enter_mode(MODE_3D_UNIT);

	if (key == 'S') {
		if (global_app_mode == MODE_2D) {
			save_vectors(relaxer_vector2->num_vectors,
						 relaxer_vector2->vectors,
						 "results_vector2.txt");
		}
		
		if (global_app_mode == MODE_3D_UNIT) {
			save_vectors(relaxer_vector3_unit->num_vectors,
						 relaxer_vector3_unit->vectors,
						 "results_vector3_unit.txt");
		}
	}
}

void handle_keyup(int) {
}

void app_init() {
    app_shell->load_texture(&spot_texture, "white_dot.jpg");
    assert(spot_texture.loaded_successfully);

	enter_mode(MODE_2D);
}

void draw_scene() {
	if (global_app_mode == MODE_2D) {
		relaxer_vector2->relax_and_draw(relax_timestep_2d);
	}

	if (global_app_mode == MODE_3D_UNIT) {
		relaxer_vector3_unit->relax_and_draw(relax_timestep_3d);
	}
}








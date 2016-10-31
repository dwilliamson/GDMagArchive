#include "app_shell.h"
#include <math.h>

#include "scripting.h"

const char *variable_type_names[NUM_VARIABLE_TYPES] = {
	"Scalar", "Vector2"
};

void Variable_Binding_List::init() {
	num_bindings = 0;
}

Variable_Binding *Variable_Binding_List::prepare_binding(char *name) {
	assert(num_bindings < MAX_BINDINGS);

	Variable_Binding *binding = &bindings[num_bindings];
	binding->name = strdup(name);
	binding->type = VARTYPE_UNINITIALIZED;
	num_bindings++;

	return binding;
}

/*
void Variable_Binding_List::bind(char *name, float *value) {
	Variable_Binding *binding = prepare_binding(name);
	binding->type = VARTYPE_SCALAR;
	binding->pointer.scalar = value;
}

void Variable_Binding_List::bind(char *name, Vector2 *value) {
    Variable_Binding *binding = prepare_binding(name);
	binding->type = VARTYPE_VECTOR2;
	binding->pointer.vector2 = value;
}
*/

Variable_Binding *Variable_Binding_List::lookup(char *name) {
	int i;
	for (i = 0; i < num_bindings; i++) {
		Variable_Binding *binding = &bindings[i];
		if (strcmp(binding->name, name) == 0) return binding;
	}

	return NULL;
}

void Value_Scalar::init() {
	instantaneous_value = 0;

	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		history_values[i] = 0;
		history_covariances[i] = 0;
	}
}

void Value_Vector2::init() {
	instantaneous_value.set(0, 0);

	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		history_values[i].set(0, 0);
		history_covariances[i].reset();
	}
}

void Variable_Binding_List::declare(char *name, Variable_Type type) {
    Variable_Binding *binding = prepare_binding(name);
	binding->type = type;

	switch (type) {
  	case VARTYPE_SCALAR:
		binding->scalar.init();
		break;
  	case VARTYPE_VECTOR2:
		binding->vector2.init();
		break;
	default:
		assert(0);
		break;
	}		
}

void Variable_Binding::set(Variable_Binding_List *vars, const double &value_scalar) {
	assert(type == VARTYPE_SCALAR);
	scalar.instantaneous_value = value_scalar;
}

void Variable_Binding::set(Variable_Binding_List *vars, const Vector2 &value_vector2) {
	assert(type == VARTYPE_VECTOR2);
	vector2.instantaneous_value = value_vector2;
}

void Value_Scalar::update(double dt, Scripting_System *system) {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		float value = history_values[i];
		float factor = system->precomputed_exponents[i];
		history_values[i] = value * factor + instantaneous_value * (1 - factor);
	}
}

void Value_Vector2::update(double dt, Scripting_System *system) {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        float mx = history_values[i].x;  // XXX use pre-update or post-update mean?
        float my = history_values[i].y;

        // Average this guy into the mean for each history slot.
		float value_x = history_values[i].x;
		float value_y = history_values[i].y;

		float factor_a = system->precomputed_exponents[i];
		float factor_b = 1 - factor_a;
		history_values[i].x = value_x * factor_a + instantaneous_value.x * factor_b;
		history_values[i].y = value_y * factor_a + instantaneous_value.y * factor_b;

        // Do the covariance part.

        float new_x = instantaneous_value.x;
        float new_y = instantaneous_value.y;

       float a, b, c;
        a = new_x * new_x;
        b = new_x * new_y;
        c = new_y * new_y;


        float va = factor_a;
        float vb = factor_b;

        history_covariances[i].a = a * vb + history_covariances[i].a * va;
        history_covariances[i].b = b * vb + history_covariances[i].b * va;
        history_covariances[i].c = c * vb + history_covariances[i].c * va;
	}
}

#ifdef BEFORE_GLOBALITY
void Value_Vector2::update(double dt, Scripting_System *system) {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {

        // Average this guy into the mean for each history slot.
		float value_x = history_values[i].x;
		float value_y = history_values[i].y;

		float factor_a = system->precomputed_exponents[i];
		float factor_b = 1 - factor_a;
		history_values[i].x = value_x * factor_a + instantaneous_value.x * factor_b;
		history_values[i].y = value_y * factor_a + instantaneous_value.y * factor_b;

        // Do the covariance part.

        Vector2 relative_value;
        relative_value.x = instantaneous_value.x - history_values[i].x;
        relative_value.y = instantaneous_value.y - history_values[i].y;

        float a, b, c;
        a = relative_value.x * relative_value.x;
        b = relative_value.x * relative_value.y;
        c = relative_value.y * relative_value.y;



        history_covariances[i].a = a * factor_b + history_covariances[i].a * factor_a;
        history_covariances[i].b = b * factor_b + history_covariances[i].b * factor_a;
        history_covariances[i].c = c * factor_b + history_covariances[i].c * factor_a;


/*        
        float xp = sqrt(history_covariances[i].a);
        float xi = sqrt(a);
        float yp = sqrt(history_covariances[i].c);
        float yi = sqrt(c);

        float sign = 1;
//        if ((history_covariances[i].b < 0) && (b < 0)) sign = -1;
        if (b < 0) sign = -1; else sign = 1;
        

        history_covariances[i].a = a * factor_b*factor_b + history_covariances[i].a * factor_a*factor_a + factor_a*factor_b*(xp*xi*2);
        history_covariances[i].b = b * factor_b*factor_b + history_covariances[i].b * factor_a*factor_a + sign*factor_a*factor_b*(xp*yi + yp*xi);
        history_covariances[i].c = c * factor_b*factor_b + history_covariances[i].c * factor_a*factor_a + factor_a*factor_b*(yp*yi*2);
*/
	}
}
#endif


void Variable_Binding::update(double dt, Scripting_System *system) {
	switch (type) {
	case VARTYPE_SCALAR:
		scalar.update(dt, system);
		break;
	case VARTYPE_VECTOR2:
		vector2.update(dt, system);
		break;
	default:
		assert(0);
		break;
	}
}

Scripting_System::Scripting_System() {
	variable_binding_list = NULL;
    dt = 0;
}

Scripting_System::~Scripting_System() {
	delete variable_binding_list;
}

void Scripting_System::init() {
	variable_binding_list = new Variable_Binding_List();
	variable_binding_list->init();

	int index = 0;
	history_times_to_reach_90_percent[index++] = 0.01f;
	history_times_to_reach_90_percent[index++] = 0.1f;
	history_times_to_reach_90_percent[index++] = 0.3f;
	history_times_to_reach_90_percent[index++] = 1.0f;
	history_times_to_reach_90_percent[index++] = 2.0f;
	history_times_to_reach_90_percent[index++] = 5.0f;
	history_times_to_reach_90_percent[index++] = 10.0f;
	history_times_to_reach_90_percent[index++] = 20.0f;
	history_times_to_reach_90_percent[index++] = 60.0f;

	assert(index == NUM_HISTORY_SLOTS);
	
	first_update = true;
}

void Scripting_System::interpolate_vector2(Variable_Binding *binding, float time,
										   Vector2 *result_return) {
	assert(binding->type == VARTYPE_VECTOR2);
	
	Vector2 result;

	float *times = history_times_to_reach_90_percent;
	
	if (time < times[0]) {
		float ta, tb;
		Vector2 v0a, v0b;
		
		ta = 0;
		tb = times[0];
		v0a = binding->vector2.instantaneous_value;
		v0b = binding->vector2.history_values[0];

		float perc = (time - ta) / (tb - ta);
		result.x = v0a.x + perc * (v0b.x - v0a.x);
		result.y = v0a.y + perc * (v0b.y - v0a.y);
	} else if (time >= times[NUM_HISTORY_SLOTS - 1]) {
		// XXX punt!
		result = binding->vector2.history_values[NUM_HISTORY_SLOTS - 1];
	} else {
		int i;
		for (i = 1; i < NUM_HISTORY_SLOTS; i++) {
			if (time < times[i]) break;
		}

		assert(i != NUM_HISTORY_SLOTS);
		
		float ta, tb;
		Vector2 v0a, v0b;
		
		ta = times[i-1];
		tb = times[i];
		v0a = binding->vector2.history_values[i-1];
		v0b = binding->vector2.history_values[i];

		float perc = (time - ta) / (tb - ta);
		result.x = v0a.x + perc * (v0b.x - v0a.x);
		result.y = v0a.y + perc * (v0b.y - v0a.y);
	}

	*result_return = result;
}

void Scripting_System::precompute_exponents(double dt) {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		precomputed_exponents[i] = pow(0.1f, dt / history_times_to_reach_90_percent[i]);
	}
}

void Scripting_System::update(double now, bool test) {
	if (first_update) {
		dt = 0;
		first_update = false;
	} else {
		dt = now - last_update_time;
	}

	precompute_exponents(dt);

	// Actually go updating the variables
	if (test) {
		update_binding_list_test(variable_binding_list);
	} else {
		update_binding_list(variable_binding_list);
	}
	
	last_update_time = now;
}

/*
  This test function copies each variable and updates it twice in
  parallel.  The first copy is advanced forward in time by dt;
  the second copy is advanced 10 times by dt/10.  The idea is that
  if the underlying math is working right, the results will come
  out the same for each copy, within numerical roundoff error.
  This demonstrates that the system is frame-rate-independent.
*/
void Scripting_System::update_binding_list_test(Variable_Binding_List *vars) {
	int i;

	for (i = 0; i < vars->num_bindings; i++) {
		Variable_Binding *binding = &vars->bindings[i];
		Variable_Binding copy1 = *binding;
		Variable_Binding copy2 = *binding;

		precompute_exponents(dt);
		
		copy1.update(dt, this);

		const int NSTEPS = 10;

		precompute_exponents(dt / NSTEPS);
		int j;
		for (j = 0; j < NSTEPS; j++) {
			copy2.update(dt * (1.0f / (float)NSTEPS), this);
		}
	}
}

void Scripting_System::update_binding_list(Variable_Binding_List *vars) {
	int i;
	for (i = 0; i < vars->num_bindings; i++) {
		Variable_Binding *binding = &vars->bindings[i];
		binding->update(dt, this);
	}
}

void Scripting_System::print_value(Variable_Binding *binding, char *buf, int len_max) {
	// XXX overflows galore
	switch (binding->type) {
  	case VARTYPE_SCALAR:
		sprintf(buf, "%f", binding->scalar.instantaneous_value);
		break;
  	case VARTYPE_VECTOR2:
		sprintf(buf, "(%f, %f)",
				binding->vector2.instantaneous_value.x, binding->vector2.instantaneous_value.y);
		break;
	default:
		assert(0);
		break;
	}		
}


#include "app_shell.h"
#include <math.h>

#include "scripting.h"
#include "command_execution.h"

#include <float.h>

void instantiate_real_value(float value, Value_General *result);
void instantiate_vector_value(float value, Value_Vector2 *result);


void Value_Scalar::set(const float &value_scalar) {
	history_values[0] = value_scalar;
	history_variances[0] = value_scalar*value_scalar;
}

void Value_Scalar::set_for_all_history(const float &value_scalar) {
    int i;
    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        history_values[i] = value_scalar;
        history_variances[i] = value_scalar*value_scalar;
    }
}

void Value_Vector2::set(float x, float y) {
    float a = x*x;
    float b = x*y;
    float c = y*y;

	history_values[0].x = x;
	history_values[0].y = y;
	history_variances[0].a = a;
	history_variances[0].b = b;
	history_variances[0].c = c;
}

void Value_Vector2::set(const Vector2 &value_vector2) {
    set(value_vector2.x, value_vector2.y);
}

void Value_Vector2::set_for_all_history(const Vector2 &value_vector2) {
    float a = value_vector2.x * value_vector2.x;
    float b = value_vector2.x * value_vector2.y;
    float c = value_vector2.y * value_vector2.y;

    int i;
    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        history_values[i] = value_vector2;
        history_variances[i].a = a;
        history_variances[i].b = b;
        history_variances[i].c = c;
    }
}



void Variable_Binding_List::init() {
	num_bindings = 0;
}

Variable_Binding *Variable_Binding_List::prepare_binding(char *name) {
	assert(num_bindings < MAX_BINDINGS);

	Variable_Binding *binding = &bindings[num_bindings];
	binding->name = app_shell->strdup(name);
	binding->value.type = VARTYPE_UNINITIALIZED;
	num_bindings++;

	return binding;
}

Variable_Binding *Variable_Binding_List::lookup(char *name) {
	int i;
	for (i = 0; i < num_bindings; i++) {
		Variable_Binding *binding = &bindings[i];
		if (strcmp(binding->name, name) == 0) return binding;
	}

	return NULL;
}

void Value_Scalar::init() {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		history_values[i] = 0;
		history_variances[i] = 0;
	}
}

void Value_Vector2::init() {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		history_values[i].set(0, 0);
		history_variances[i].reset();
	}
}

void Value_Command::init() {
    proc = NULL;
}

Variable_Binding *Variable_Binding_List::declare(char *name, Variable_Type type) {
    Variable_Binding *binding = prepare_binding(name);
	binding->value.type = type;

	switch (type) {
  	case VARTYPE_SCALAR:
		binding->value.scalar.init();
		break;
  	case VARTYPE_VECTOR2:
		binding->value.vector2.init();
		break;
  	case VARTYPE_COMMAND:
		binding->value.command.init();
		break;
  	case VARTYPE_STRING:
		binding->value.string.data = app_shell->strdup("");
		break;
	default:
		assert(0);
		break;
	}		

    return binding;
}

void Value_Scalar::update(double dt, Scripting_System *system) {
	int i;
	for (i = 1; i < NUM_HISTORY_SLOTS; i++) {
		float value = history_values[i];
		float factor = system->precomputed_exponents[i];
		history_values[i] = value * factor + history_values[0] * (1 - factor);

        assert(_finite(history_values[i]));
        float variance = history_variances[i];
        history_variances[i] = variance * factor + history_variances[0] * (1 - factor);
	}
}

void Value_Vector2::update(double dt, Scripting_System *system) {
	int i;
	for (i = 1; i < NUM_HISTORY_SLOTS; i++) {
        float mx = history_values[i].x;
        float my = history_values[i].y;

        // Average this guy into the mean for each history slot.
		float value_x = history_values[i].x;
		float value_y = history_values[i].y;

		float factor_a = system->precomputed_exponents[i];
		float factor_b = 1 - factor_a;
		history_values[i].x = value_x * factor_a + history_values[0].x * factor_b;
		history_values[i].y = value_y * factor_a + history_values[0].y * factor_b;

        assert(_finite(history_values[i].x));

        // Do the covariance part.

        float va = factor_a;
        float vb = factor_b;

        history_variances[i].a = history_variances[0].a * vb + history_variances[i].a * va;
        history_variances[i].b = history_variances[0].b * vb + history_variances[i].b * va;
        history_variances[i].c = history_variances[0].c * vb + history_variances[i].c * va;
	}
}

void Variable_Binding::update(double dt, Scripting_System *system) {
	switch (value.type) {
	case VARTYPE_SCALAR:
		value.scalar.update(dt, system);
		break;
	case VARTYPE_VECTOR2:
		value.vector2.update(dt, system);
		break;
    case VARTYPE_STRING:
    case VARTYPE_LVALUE:
        // These types do not do anything over time!
        break;
	default:
		assert(0);
		break;
	}
}

Scripting_System::Scripting_System() {
	variable_binding_list = NULL;
    dt = 0;
    log_file = NULL;
    line_number = -1;
    current_script = NULL;

    current_return_value = new Value_General();
    instantiate_real_value(0, current_return_value);
}

Scripting_System::~Scripting_System() {
	delete variable_binding_list;
    delete current_return_value;
}

void Scripting_System::init() {
	variable_binding_list = NULL;
    init_standard_commands();

	int index = 0;
	history_times_to_reach_90_percent[index++] = 0.0f;
	history_times_to_reach_90_percent[index++] = 0.2f;
	history_times_to_reach_90_percent[index++] = 0.5f;
	history_times_to_reach_90_percent[index++] = 1.0f;
	history_times_to_reach_90_percent[index++] = 2.0f;
	history_times_to_reach_90_percent[index++] = 5.0f;
	history_times_to_reach_90_percent[index++] = 10.0f;
	history_times_to_reach_90_percent[index++] = 25.0f;
	history_times_to_reach_90_percent[index++] = 60.0f;
	history_times_to_reach_90_percent[index++] = 180.0f;

	assert(index == NUM_HISTORY_SLOTS);
	
	first_update = true;
}

void Scripting_System::interpolate_vector2(Value_Vector2 *vector2, float time,
										   Vector2 *result_return,
                                           Covariance2 *cov_return) {
	Vector2 result;

	float *times = history_times_to_reach_90_percent;
	float fraction;

    Covariance2 *c0a, *c0b;
    Covariance2 cov_tmp;

    Vector2 v0a, v0b;

    if (time < times[0]) time = times[0];

	if (time >= times[NUM_HISTORY_SLOTS - 1]) {
		// XXX This is basically a punt, could be handled better.
		*result_return = vector2->history_values[NUM_HISTORY_SLOTS - 1];
        if (cov_return) *cov_return = vector2->history_variances[NUM_HISTORY_SLOTS - 1];
        return;
	} else {
		int i;
		for (i = 1; i < NUM_HISTORY_SLOTS; i++) {
			if (time < times[i]) break;
		}

		assert(i != NUM_HISTORY_SLOTS);
		
		float ta, tb;
		
		ta = times[i-1];
		tb = times[i];
		v0a = vector2->history_values[i-1];
		v0b = vector2->history_values[i];

        c0a = &vector2->history_variances[i-1];
        c0b = &vector2->history_variances[i];

		fraction = (time - ta) / (tb - ta);
	}

    result_return->x = v0a.x + fraction * (v0b.x - v0a.x);
	result_return->y = v0a.y + fraction * (v0b.y - v0a.y);

    if (cov_return) {
        cov_return->a = c0a->a + fraction * (c0b->a - c0a->a);
        cov_return->b = c0a->b + fraction * (c0b->b - c0a->b);
        cov_return->c = c0a->c + fraction * (c0b->c - c0a->c);
    }
}

void Scripting_System::interpolate_scalar(Value_Scalar *scalar, float time,
										  double *result_return) {
    double result;

	float *times = history_times_to_reach_90_percent;
	
	if (time < times[0]) time = times[0];

	if (time >= times[NUM_HISTORY_SLOTS - 1]) {
		// XXX This is basically a punt, could be handled better.
		result = scalar->history_values[NUM_HISTORY_SLOTS - 1];
	} else {
		int i;
		for (i = 1; i < NUM_HISTORY_SLOTS; i++) {
			if (time < times[i]) break;
		}

		assert(i != NUM_HISTORY_SLOTS);
		
		float ta, tb;
        double v0a, v0b;
		
		ta = times[i-1];
		tb = times[i];
		v0a = scalar->history_values[i-1];
		v0b = scalar->history_values[i];

		float perc = (time - ta) / (tb - ta);
        result = v0a + perc * (v0b - v0a);
	}

	*result_return = result;
}

void Scripting_System::precompute_exponents(double dt) {
	int i;
	for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
		if (i == 0) {
            precomputed_exponents[i] = 0;
        } else {
            precomputed_exponents[i] = pow(0.1f, dt / history_times_to_reach_90_percent[i]);
        }
	}
}

void Scripting_System::update(double now) {
	if (first_update) {
		dt = 0;
		first_update = false;
	} else {
		dt = now - last_update_time;
        if (dt > 0.2) dt = 0.2;
	}

	precompute_exponents(dt);

	// Actually go updating the variables
    update_binding_list(variable_binding_list);
	
	last_update_time = now;
}

void instantiate_real_value(float value, Value_General *result) {
    result->type = VARTYPE_SCALAR;
    int i;
    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        result->scalar.history_values[i] = value;
        result->scalar.history_variances[i] = value*value;
    }
}

void instantiate_vector_value(float x, float y, Value_General *result) {
    result->type = VARTYPE_VECTOR2;
    int i;
    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        result->vector2.history_values[i].x = x;
        result->vector2.history_values[i].y = y;
        result->vector2.history_variances[i].a = x*x;
        result->vector2.history_variances[i].b = x*y;
        result->vector2.history_variances[i].c = y*y;
    }
}

void instantiate_string_value(char *data, Value_General *result) {
    result->type = VARTYPE_STRING;
    result->string.data = data;
}

bool Scripting_System::evaluate_argument(Command_Argument_Spec *unevaluated_argument,
                                         Value_General *result) {
    Command_Argument_Spec::Argument_Type type = unevaluated_argument->type;

    if ((type == Command_Argument_Spec::VARIABLE) || (type == Command_Argument_Spec::LVALUE)) {
        char *name = unevaluated_argument->variable_name;
        Variable_Binding *binding = variable_binding_list->lookup(name);
        if (binding == NULL) {
            char buf[BUFSIZ];
            sprintf(buf, "Undefined variable: %s\n", name);
            report_error(buf);
            return false;
        }

        if (type == Command_Argument_Spec::VARIABLE) {
            *result = binding->value;
        } else {
            result->type = VARTYPE_LVALUE;
            result->lvalue.binding = binding;
        }

        return true;
    } else if (type == Command_Argument_Spec::LITERAL_REAL) {
        instantiate_real_value(unevaluated_argument->real_value, result);
        return true;
    } else if (type == Command_Argument_Spec::LITERAL_STRING) {
        instantiate_string_value(unevaluated_argument->string_data, result);
        return true;
    } else {
        assert(0);
    }

    return false;
}

void Scripting_System::update_system_defined_globals(Variable_Binding_List *bindings, double now) {
	int x, y;
	x = app_shell->mouse_pointer_x;
	y = app_shell->mouse_pointer_y;

	// We divide y by the screen width, not the height... this is
	// so that the y axis of the coordinate system won't be
	// scaled with respect to the x.
	float fx = x / (float)app_shell->screen_width;
	float fy = y / (float)app_shell->screen_width;

	Vector2 mouse_pos;
	mouse_pos.set(fx, fy);
	Variable_Binding *mouse = bindings->lookup("mouse");
	if (mouse && (mouse->value.type == VARTYPE_VECTOR2)) mouse->value.vector2.set(mouse_pos);

	Variable_Binding *time = bindings->lookup("time");
	if (time && (time->value.type == VARTYPE_SCALAR)) time->value.scalar.set(now);

	Variable_Binding *frame_time = bindings->lookup("frame_time");
	if (frame_time && (frame_time->value.type == VARTYPE_SCALAR)) frame_time->value.scalar.set(dt);
}

void Scripting_System::run_script(Compiled_Script *script) {

    current_script = script;
    variable_binding_list = script->global_variables;

	double now = app_shell->get_time();
    update_system_defined_globals(variable_binding_list, now);
	update(now);   // Update all the variables


    Command_Calling_Data calling_data;

    bool success = true;

    program_counter = 0;
    while (program_counter < script->commands->num_commands) {
        Script_Command *command = &script->commands->commands[program_counter];

        line_number = command->line_number;  // This is used for error reporting.

        calling_data.scripting_system = this;
        calling_data.num_arguments = command->argc;
        calling_data.arguments = static_argument_pool;

        int j;
        for (j = 0; j < command->argc; j++) {
            success = evaluate_argument(&command->arguments[j], &calling_data.arguments[j]);
            if (!success) break;
        }

        if (!success) break;

        Variable_Binding *binding = command_binding_list->lookup(command->command_name);
        if (binding == NULL) {
            char buf[BUFSIZ];
            sprintf(buf, "Command '%s' not found.\n", command->command_name);
            report_error(buf);
            break;
        }

        assert(binding->value.type == VARTYPE_COMMAND);

        binding->value.command.proc(&calling_data);
        program_counter++;
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
	// XXX We are using lots of static buffers here, which can
    // lead to buffer overflows, corruption, crashes, and other
    // bad stuff.  Don't do this in real software, please.
	switch (binding->value.type) {
  	case VARTYPE_SCALAR:
		sprintf(buf, "%f", binding->value.scalar.history_values[0]);
		break;
  	case VARTYPE_VECTOR2:
		sprintf(buf, "(%f, %f)",
				binding->value.vector2.history_values[0].x, binding->value.vector2.history_values[0].y);
		break;
    case VARTYPE_LVALUE:
        sprintf(buf, "[lvalue]");
        break;
    case VARTYPE_STRING:
        sprintf(buf, "\"%s\"", binding->value.string.data);
        break;
	default:
		assert(0);
		break;
	}		
}

Value_General *Scripting_System::configure_return_type(Variable_Type type) {
    if (current_return_value->type == VARTYPE_STRING) {
        free(current_return_value->string.data);
        current_return_value->string.data = NULL;
    }

    current_return_value->type = type;

    // Put in some default values so there isn't garbage memory there.
    switch (type) {
    case VARTYPE_SCALAR:
        instantiate_real_value(0, current_return_value);
        break;
    case VARTYPE_VECTOR2:
        instantiate_vector_value(0, 0, current_return_value);
        break;
    case VARTYPE_STRING:
        break;
    default:
        assert(0);
        break;
    }

    return current_return_value;
}

void Vector2::normalize() {
    float len = sqrt(x*x + y*y);
    if (len) {
        x /= len;
        y /= len;
    }
}

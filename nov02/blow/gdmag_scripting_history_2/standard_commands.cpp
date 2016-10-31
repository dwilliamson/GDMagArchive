#include "app_shell.h"
#include "app_shell/os_specific_opengl_headers.h"

#include "scripting.h"
#include "script_compiler.h"
#include "command_execution.h"
#include "standard_commands.h"

#include <math.h>
#include <float.h>

void c_fire_mortar(Command_Calling_Data *data);

const char *variable_type_names[NUM_VARIABLE_TYPES] = {
	"Scalar", "Vector2", "String"
};


bool check_number_of_arguments(Command_Calling_Data *data, int num_desired) {
    if (data->num_arguments == num_desired) return true;

    char buf[BUFSIZ];
    sprintf(buf, "Incorrect number of arguments (wanted %d, got %d)\n",
            num_desired, data->num_arguments);
    data->scripting_system->report_error(buf);

    return false;
}

Value_General *get_value(Command_Calling_Data *data, int index) {
    assert(index < data->num_arguments);
    return &data->arguments[index];
}

Value_Scalar *get_scalar(Command_Calling_Data *data, int index) {
    Value_General *value = get_value(data, index);
    assert(value->type == VARTYPE_SCALAR);
    return &value->scalar;
}

Value_Vector2 *get_vector2(Command_Calling_Data *data, int index) {
    Value_General *value = get_value(data, index);
    assert(value->type == VARTYPE_VECTOR2);

    return &value->vector2;
}

Value_String *get_string(Command_Calling_Data *data, int index) {
    Value_General *value = get_value(data, index);
    assert(value->type == VARTYPE_STRING);
    return &value->string;
}

Value_Lvalue *get_lvalue(Command_Calling_Data *data, int index) {
    Value_General *value = get_value(data, index);
    assert(value->type == VARTYPE_LVALUE);
    return &value->lvalue;
}

bool ensure_argument_is_lvalue(Command_Calling_Data *data, int index) {
    assert(data->num_arguments > index);
    Value_General *value = get_value(data, index);

    if (value->type == VARTYPE_LVALUE) return true;

    char buf[BUFSIZ];
    sprintf(buf, "Type mismatch in argument %d (wanted an lvalue).\n",
            index + 1);
    data->scripting_system->report_error(buf);
    return false;
}


bool ensure_argument_is_scalar_or_vector(Command_Calling_Data *data, int index) {
    assert(data->num_arguments > index);
    Value_General *value = get_value(data, index);

    if (value->type == VARTYPE_SCALAR) return true;
    if (value->type == VARTYPE_VECTOR2) return true;

    char buf[BUFSIZ];
    sprintf(buf, "Type mismatch in argument %d (wanted a scalar or vector).\n",
            index + 1);
    data->scripting_system->report_error(buf);
    return false;
}

bool ensure_argument_is_vector2(Command_Calling_Data *data, int index) {
    assert(data->num_arguments > index);
    Value_General *value = get_value(data, index);

    if (value->type == VARTYPE_VECTOR2) return true;

    char buf[BUFSIZ];
    sprintf(buf, "Type mismatch in argument %d (wanted a vector).\n",
            index + 1);
    data->scripting_system->report_error(buf);
    return false;
}

bool ensure_argument_is_scalar(Command_Calling_Data *data, int index) {
    assert(data->num_arguments > index);
    Value_General *value = get_value(data, index);

    if (value->type == VARTYPE_SCALAR) return true;

    char buf[BUFSIZ];
    sprintf(buf, "Type mismatch in argument %d (wanted a vector).\n",
            index + 1);
    data->scripting_system->report_error(buf);
    return false;
}

bool ensure_argument_is_string(Command_Calling_Data *data, int index) {
    assert(data->num_arguments > index);
    Value_General *value = get_value(data, index);

    if (value->type == VARTYPE_STRING) return true;

    char buf[BUFSIZ];
    sprintf(buf, "Type mismatch in argument %d (wanted a string).\n",
            index + 1);
    data->scripting_system->report_error(buf);
    return false;
}


bool do_core_math_operation_checks(Command_Calling_Data *data) {
    bool success;
    success = ensure_argument_is_scalar_or_vector(data, 0);
    if (!success) return false;
    success = ensure_argument_is_scalar_or_vector(data, 1);
    if (!success) return false;

    Value_General *v0 = get_value(data, 0);
    Value_General *v1 = get_value(data, 1);
    
    if (v0->type != v1->type) {
        data->scripting_system->report_error("Type mismatch between arguments 1 and 2.\n");
        return false;
    }

    return true;
}

bool do_math_operation_checks(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return false;

    success &= do_core_math_operation_checks(data);
    return success;
}





const float NUM_STANDARD_DEVIATIONS = 1.7f;

// Everything this function does is 2-dimensional; I am just using
// a Vector3 for the vector operations to avoid having to do all kinds
// of code-writing for the Vector2 class.  The 'z' coordinate of these
// 3-vectors is always 0.
void draw_one_ellipse(Vector2 *mean, Vector3 *axis0, Vector3 *axis1) {
    app_shell->triangle_mode_begin();

    const int NUM_VERTICES = 50;
    static Vector3 vertices[NUM_VERTICES];

    // Generate the vertex coordinates for the ellipse.

    int j;
    for (j = 0; j < NUM_VERTICES; j++) {
        double theta = 2 * M_PI * (j / (double)NUM_VERTICES);
        double ct = cos(theta);
        double st = sin(theta);

        Vector3 a0 = *axis0;
        Vector3 a1 = *axis1;
        a0.scale(ct);
        a1.scale(st);

        Vector3 pos = a0.add(a1);
        pos.x += mean->x;
        pos.y += mean->y;
        
        vertices[j] = pos;
    }

    // Draw the ellipse.

    glBegin(GL_TRIANGLES);
    for (j = 0; j < NUM_VERTICES; j++) {
        int n0 = j;
        int n1 = (j + 1) % NUM_VERTICES;
        glVertex2f(mean->x, mean->y);
        glVertex2f(vertices[n0].x, vertices[n0].y);
        glVertex2f(vertices[n1].x, vertices[n1].y);
    }
    glEnd();

    app_shell->triangle_mode_end();
}

void draw_one_ellipse(Vector2 mean, Covariance2 *covariance, float scale) {
    Vector3 axis[2];
    float lambda[2];

    covariance->find_eigenvectors(lambda, axis);
    assert(lambda[0] >= 0);
    assert(lambda[1] >= 0);
    
    float len0 = sqrt(lambda[0]) * NUM_STANDARD_DEVIATIONS;
    float len1 = sqrt(lambda[1]) * NUM_STANDARD_DEVIATIONS;

    Vector3 axis0 = axis[0];
    Vector3 axis1 = axis[1];

    axis0.scale(len0 * scale);
    axis1.scale(len1 * scale);

    draw_one_ellipse(&mean, &axis0, &axis1);
}






void c_add(Command_Calling_Data *data) {
    bool success;
    success = do_math_operation_checks(data);
    if (!success) return;

    Value_General *v0 = get_value(data, 0);
    Value_General *v1 = get_value(data, 1);
    
    Value_General *gresult = data->scripting_system->configure_return_type(v0->type);
    if (v0->type == VARTYPE_SCALAR) {
        Value_Scalar *scalar_result = &gresult->scalar;
        scalar_result->set(v0->scalar.history_values[0] + v1->scalar.history_values[0]);
    } else {
        Value_Vector2 *result = &gresult->vector2;
        assert(v0->type == VARTYPE_VECTOR2);
        Vector2 value;
        value.x = v0->vector2.history_values[0].x + v1->vector2.history_values[0].x;
        value.y = v0->vector2.history_values[0].y + v1->vector2.history_values[0].y;
        result->set(value);
    }

}

void c_subtract(Command_Calling_Data *data) {
    bool success;
    success = do_math_operation_checks(data);
    if (!success) return;

    Value_General *v0 = get_value(data, 0);
    Value_General *v1 = get_value(data, 1);

    Value_General *gresult = data->scripting_system->configure_return_type(v0->type);
    if (v0->type == VARTYPE_SCALAR) {
        Value_Scalar *scalar_result = &gresult->scalar;
        scalar_result->set(v0->scalar.history_values[0] - v1->scalar.history_values[0]);
    } else {
        Value_Vector2 *result = &gresult->vector2;
        assert(v0->type == VARTYPE_VECTOR2);
        Vector2 value;
        value.x = v0->vector2.history_values[0].x - v1->vector2.history_values[0].x;
        value.y = v0->vector2.history_values[0].y - v1->vector2.history_values[0].y;
        result->set(value);
    }
}

void c_draw_history(Command_Calling_Data *data) {
    int i;
    float scale = app_shell->screen_width;

    bool success;
    success = check_number_of_arguments(data, 5);
    if (!success) return;

    success = ensure_argument_is_vector2(data, 0);
    if (!success) return;

    Value_General *value = get_value(data, 0);

    // Draw covariances.

    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        int index = NUM_HISTORY_SLOTS - 1 - i;
        assert(index >= 0);

        float scale_max = 0.85f;
        float scale_base = 1.0f - scale_max;
        float color_scale = (1.0f - index / (float)(NUM_HISTORY_SLOTS - 1)) * scale_max + scale_base;
        glColor4f(0, color_scale * 0.5f, color_scale, 1);

        Vector2 mean = value->vector2.history_values[index];

        Covariance2 *covariance = &value->vector2.history_variances[index];
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
            
        glVertex2f(value->vector2.history_values[index].x * scale,
                   value->vector2.history_values[index].y * scale);
    }

    glVertex2f(value->vector2.history_values[0].x * scale,
               value->vector2.history_values[0].y * scale);
    glEnd();


    app_shell->line_mode_end();
}

void c_print_variable(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;
    success = ensure_argument_is_lvalue(data, 0);
    if (!success) return;
    success = ensure_argument_is_vector2(data, 1);
    if (!success) return;

    Value_General *variable = get_value(data, 0);
    Value_Vector2 *position = get_vector2(data, 1);

    glColor3f(1, 1, 1);
	app_shell->text_mode_begin();

	const int BUFFER_LEN = 1000;
	char buf[BUFFER_LEN]; // XXX static buffer size
	
    Variable_Binding *binding = variable->lvalue.binding;

    strcpy(buf, binding->name); // XXX overflow
    strcat(buf, "  ");
    int len = strlen(buf);
    char *pos = buf + len;
    int len_remaining = BUFFER_LEN - len - 2;

    data->scripting_system->print_value(binding, pos, len_remaining);
		

    float fx = position->history_values[0].x;
    float fy = position->history_values[0].y;
    int sx = (int)(fx * app_shell->screen_width + 0.5f);
    int sy = (int)(fy * app_shell->screen_height + 0.5f);
    app_shell->draw_text_line(&sx, &sy, buf);
    fy = sy / (float)app_shell->screen_height;
    
    // Write the new screen coordinates into the return value.
    
    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_VECTOR2);
    Value_Vector2 *result = &gresult->vector2;
    result->set(fx, fy);

	app_shell->text_mode_end();
}


void c_make_vector(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 0);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 1);
    if (!success) return;

    Value_Scalar *desired_x = get_scalar(data, 0);
    Value_Scalar *desired_y = get_scalar(data, 1);

    Value_General *value = data->scripting_system->configure_return_type(VARTYPE_VECTOR2);
    Value_Vector2 *vector2 = &value->vector2;

    Vector2 desired_value;
    desired_value.x = desired_x->history_values[0];
    desired_value.y = desired_y->history_values[0];

    value->vector2.set(desired_value);
}

void c_print_string(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;
    success = ensure_argument_is_string(data, 0);
    if (!success) return;
    success = ensure_argument_is_vector2(data, 1);
    if (!success) return;

    Value_Vector2 *position = get_vector2(data, 1);

    glColor3f(1, 1, 1);
	app_shell->text_mode_begin();

    Value_String *string = get_string(data, 0);

    float fx = position->history_values[0].x;
    float fy = position->history_values[0].y;
    int sx = (int)(fx * app_shell->screen_width + 0.5f);
    int sy = (int)(fy * app_shell->screen_height + 0.5f);
    app_shell->draw_text_line(&sx, &sy, string->data);
    fy = sy / (float)app_shell->screen_height;
    
    // Write the new screen coordinates into the return value.
    
    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_VECTOR2);
    gresult->vector2.set(fx, fy);

	app_shell->text_mode_end();
}

void c_draw_ellipse(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 3);
    if (!success) return;

    success = ensure_argument_is_vector2(data, 0);
    success &= ensure_argument_is_vector2(data, 1);
    success &= ensure_argument_is_vector2(data, 2);
    if (!success) return;

    Vector2 mean;
    Vector3 axis0, axis1;
    mean = get_vector2(data, 0)->history_values[0];
    Vector2 a0 = get_vector2(data, 1)->history_values[0];
    Vector2 a1 = get_vector2(data, 2)->history_values[0];
    axis0.set(a0.x, a0.y, 0);
    axis1.set(a1.x, a1.y, 0);

    axis0.scale(app_shell->screen_width);
    axis1.scale(app_shell->screen_width);
    mean.x *= app_shell->screen_width;
    mean.y *= app_shell->screen_width;

    glColor3f(.2, .2, .5);

    draw_one_ellipse(&mean, &axis0, &axis1);
}

void c_sample_tendency(Command_Calling_Data *data) {
    bool success;
    if ((data->num_arguments != 2) && (data->num_arguments != 4)) {
        data->scripting_system->report_error("'sample_tendency' requires 2 or 4 arguments.");
        return;
    }

    success = ensure_argument_is_scalar_or_vector(data, 0);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 1);
    if (!success) return;

    Value_General *v0 = get_value(data, 0);
    Scripting_System *system = data->scripting_system;
    if (v0->type == VARTYPE_SCALAR) {
        if (data->num_arguments != 2) {
            system->report_error("Scalar version of 'sample_tendency' requires only 2 arguments.");
            return;
        }
    }

    Value_Lvalue *lvalue_axis_1 = NULL;
    Value_Lvalue *lvalue_axis_2 = NULL;
    if (data->num_arguments == 4) {
        success = ensure_argument_is_lvalue(data, 2);
        if (!success) return;
        success = ensure_argument_is_lvalue(data, 3);
        if (!success) return;
        lvalue_axis_1 = get_lvalue(data, 2);
        lvalue_axis_2 = get_lvalue(data, 3);
        if (lvalue_axis_1->binding->value.type != VARTYPE_VECTOR2) {
            system->report_error("Argument 3 must be a pointer to a vector.\n");
            return;
        }
        if (lvalue_axis_2->binding->value.type != VARTYPE_VECTOR2) {
            system->report_error("Argument 4 must be a pointer to a vector.\n");
            return;
        }
    }

    // Boy that was a lot of argument checking.  Now we do
    // the actual work:

    Value_Scalar *age_scalar = get_scalar(data, 1);
    float age = age_scalar->history_values[0];


    Value_General *gresult = system->configure_return_type(v0->type);
    if (v0->type == VARTYPE_SCALAR) {
        double sampled_result;
        system->interpolate_scalar(&v0->scalar, age, &sampled_result);
        gresult->scalar.set(sampled_result);
    } else {
        assert(v0->type == VARTYPE_VECTOR2);
        Vector2 sampled_result;
        Covariance2 cov;
            
        system->interpolate_vector2(&v0->vector2, age, &sampled_result, &cov);
        assert(_finite(sampled_result.x));
        assert(_finite(sampled_result.y));

        gresult->vector2.set(sampled_result);

        if (lvalue_axis_1) {
            assert(lvalue_axis_2);
            float eigenvalues[2];
            Vector3 eigenvectors[2];

            Covariance2 local;
            cov.translate(&local, -sampled_result.x, -sampled_result.y);
            local.find_eigenvectors(eigenvalues, eigenvectors);
            assert(eigenvalues[0] >= 0);
            assert(eigenvalues[1] >= 0);

            eigenvectors[0].scale(sqrt(eigenvalues[0]) * NUM_STANDARD_DEVIATIONS);
            eigenvectors[1].scale(sqrt(eigenvalues[1]) * NUM_STANDARD_DEVIATIONS);
            Vector2 axis1, axis2;
            axis1.x = eigenvectors[0].x;
            axis1.y = eigenvectors[0].y;
            axis2.x = eigenvectors[1].x;
            axis2.y = eigenvectors[1].y;
            lvalue_axis_1->binding->value.vector2.set(axis1);
            lvalue_axis_2->binding->value.vector2.set(axis2);
        }
    }
}

void c_if(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 1);
    if (!success) return;

    success = ensure_argument_is_scalar(data, 0);
    if (!success) return;

    Value_Scalar *scalar = get_scalar(data, 0);
    if (!scalar->history_values[0]) {
        Scripting_System *system = data->scripting_system;
        Compiled_Script *script = system->current_script;
        while (system->program_counter < script->commands->num_commands) {
            Script_Command *command = &script->commands->commands[system->program_counter];
            if (!strcmp(command->command_name, "endif")) break;
            system->program_counter++;
        }

        if (system->program_counter == script->commands->num_commands) {
            system->report_error("Missing 'endif'");
        }
    }
}

void c_end(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 0);
    if (!success) return;

    data->scripting_system->program_counter = data->scripting_system->current_script->commands->num_commands;
}

void c_endif(Command_Calling_Data *data) {
}

void c_clamp(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 3);
    if (!success) return;

    success = ensure_argument_is_scalar(data, 0);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 1);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 2);
    if (!success) return;

    Value_Scalar *v0 = get_scalar(data, 0);
    Value_Scalar *v1 = get_scalar(data, 1);
    Value_Scalar *v2 = get_scalar(data, 2);
    float value = v0->history_values[0];
    if (value < v1->history_values[0]) value = v1->history_values[0];
    if (value > v2->history_values[0]) value = v2->history_values[0];

    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
    Value_Scalar *result = &gresult->scalar;
    result->set(value);
}

void c_multiply(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;

    success = ensure_argument_is_scalar_or_vector(data, 0);
    if (!success) return;
    success = ensure_argument_is_scalar_or_vector(data, 1);
    if (!success) return;

    Value_General *g0 = &data->arguments[0];
    Value_General *g1 = &data->arguments[1];

    if ((g0->type == VARTYPE_VECTOR2) && (g1->type == VARTYPE_VECTOR2)) {
        data->scripting_system->report_error("Cannot multiply two vectors!\n");
        return;
    }

    if (g1->type == VARTYPE_VECTOR2) {
        Value_General *tmp = g1;
        g1 = g0;
        g0 = tmp;
    }

    if (g0->type == VARTYPE_VECTOR2) {
        assert(g1->type == VARTYPE_SCALAR);

        float x = g0->vector2.history_values[0].x * g1->scalar.history_values[0];
        float y = g0->vector2.history_values[0].y * g1->scalar.history_values[0];

        Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_VECTOR2);
        gresult->vector2.set(x, y);
    } else {
        float value = g0->scalar.history_values[0] * g1->scalar.history_values[0];

        Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
        gresult->scalar.set(value);
    }
}

void c_and(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    success &= ensure_argument_is_scalar(data, 0);
    success &= ensure_argument_is_scalar(data, 1);
    if (!success) return;

    Value_General *g0 = &data->arguments[0];
    Value_General *g1 = &data->arguments[1];
    float value = g0->scalar.history_values[0] * g1->scalar.history_values[0];

    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
    gresult->scalar.set(value);
}

void c_or(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    success &= ensure_argument_is_scalar(data, 0);
    success &= ensure_argument_is_scalar(data, 1);
    if (!success) return;

    Value_General *g0 = &data->arguments[0];
    Value_General *g1 = &data->arguments[1];
    float value = g0->scalar.history_values[0];
    if (value < g1->scalar.history_values[0]) value = g1->scalar.history_values[0];

    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
    gresult->scalar.set(value);
}

void c_length(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 1);
    if (!success) return;

    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_SCALAR);

    Value_General *value = get_value(data, 0);
    float len;
    if (value->type == VARTYPE_STRING) {
        char *value = get_string(data, 0)->data;
        len = strlen(value);
    } else if (value->type == VARTYPE_SCALAR) {
        float value = get_scalar(data, 0)->history_values[0];
        len = fabs(value);
    } else {
        assert(value->type == VARTYPE_VECTOR2);

        Value_Vector2 *vector2 = get_vector2(data, 0);
        float x = vector2->history_values[0].x;
        float y = vector2->history_values[0].y;
        len = sqrt(x*x + y*y);
    }

    gresult->scalar.set(len);
}

void c_normalize(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 1);
    if (!success) return;

    success = ensure_argument_is_vector2(data, 0);
    if (!success) return;

    Value_Vector2 *vector2 = get_vector2(data, 0);
    float x = vector2->history_values[0].x;
    float y = vector2->history_values[0].y;
    float len = sqrt(x*x + y*y);
    if (len == 0) len = 1;

    Value_General *gresult = data->scripting_system->configure_return_type(VARTYPE_VECTOR2);
    gresult->vector2.set(x / len, y / len);
}

void assign_value(Value_General *result, Value_General *rvalue) {
    assert(rvalue->type == result->type);

    switch (rvalue->type) {
    case VARTYPE_SCALAR: {
        result->scalar.set(rvalue->scalar.history_values[0]);
        break;
    }
    case VARTYPE_VECTOR2: {
        Vector2 value_vector2 = rvalue->vector2.history_values[0];
        result->vector2.set(value_vector2);
        break;
    }
    case VARTYPE_STRING:
        result->string.data = app_shell->strdup(rvalue->string.data);
        break;
    }
}


void assign_value_full_copy(Value_General *result, Value_General *rvalue) {
    assert(rvalue->type == result->type);

    assert(rvalue->type != VARTYPE_STRING); // Should have been handled in assign_value().
    *result = *rvalue;
}


void assign_value(Value_Lvalue *lvalue, Value_General *rvalue) {
    assign_value(&lvalue->binding->value, rvalue);
}

void assign_value_full_copy(Value_Lvalue *lvalue, Value_General *rvalue) {
    if (rvalue->type == VARTYPE_STRING) {
        assign_value(&lvalue->binding->value, rvalue);
        return;
    }

    assign_value_full_copy(&lvalue->binding->value, rvalue);
}

void c_assign_return_value(Command_Calling_Data *data) {
    Scripting_System *system = data->scripting_system;
    Value_General *value = system->current_return_value;

    bool success;
    success = check_number_of_arguments(data, 1);
    if (!success) return;

    success = ensure_argument_is_lvalue(data, 0);
    if (!success) return;

    Value_Lvalue *lvalue = get_lvalue(data, 0);
    if (value->type != lvalue->binding->value.type) {
        system->report_error("Type mismatch between return value and destination variable.");
        return;
    }

    assign_value(lvalue, value);
}

void c_copy(Command_Calling_Data *data) {
    Scripting_System *system = data->scripting_system;
    Value_General *value = system->current_return_value;

    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;

    success = ensure_argument_is_lvalue(data, 0);
    if (!success) return;

    Value_Lvalue *lvalue = get_lvalue(data, 0);
    if (value->type != lvalue->binding->value.type) {
        system->report_error("Type mismatch between return value and destination variable.");
        return;
    }

    assign_value_full_copy(lvalue, value);
}

void c_differences(Command_Calling_Data *data) {
    Scripting_System *system = data->scripting_system;
    Value_General *value = system->current_return_value;

    bool success;
    success = check_number_of_arguments(data, 2);
    success &= ensure_argument_is_scalar_or_vector(data, 0);
    success &= ensure_argument_is_lvalue(data, 1);
    if (!success) return;

    Value_General *input = get_value(data, 0);
    Value_Lvalue *result_lvalue = get_lvalue(data, 1);
    if (result_lvalue->binding->value.type != input->type) {
        system->report_error("Type mismatch between input variable and return lvalue.\n");
        return;
    }

    Value_General *result = &result_lvalue->binding->value;

    int i;
    for (i = 1; i < NUM_HISTORY_SLOTS; i++) {
        float t1 = data->scripting_system->history_times_to_reach_90_percent[i-1];
        float t0 = data->scripting_system->history_times_to_reach_90_percent[i];
        float idt = 1.0 / (t0 - t1);

        // XXX Here I essentially say that there is no variance
        // in the derivative; is that correct?
        if (input->type == VARTYPE_SCALAR) {
            float v1 = input->scalar.history_values[i - 1];
            float v0 = input->scalar.history_values[i];

            float value = (v1 - v0) * idt;
            result->scalar.history_values[i] = value;
            result->scalar.history_variances[i] = value * value;
        } else {
            assert(input->type == VARTYPE_VECTOR2);
            Vector2 v1 = input->vector2.history_values[i - 1];
            Vector2 v0 = input->vector2.history_values[i];

            float x = (v1.x - v0.x) * idt;
            float y = (v1.y - v0.y) * idt;

            result->vector2.history_values[i].x = x;
            result->vector2.history_values[i].y = y;
            result->vector2.history_variances[i].a = x*x;
            result->vector2.history_variances[i].b = x*y;
            result->vector2.history_variances[i].c = y*y;
        }
    }

    if (input->type == VARTYPE_SCALAR) {
        result->scalar.history_values[0] = result->scalar.history_values[1];
        result->scalar.history_variances[0] = result->scalar.history_variances[1];
    } else {
        result->vector2.history_values[0] = result->vector2.history_values[1];
        result->vector2.history_variances[0] = result->vector2.history_variances[1];
    }
}

void beet() {
}

void make_double_outer_product(Covariance2 *result, Vector2 a, Vector2 b) {
    result->a = 2*a.x*b.x;
    result->b = a.x*b.y + a.y*b.x;
    result->c = 2*a.y*b.y;
}

void compute_endpoints(Value_Vector2 *vector2, int index, 
                       Vector2 *a0_return, Vector2 *a1_return) {
    Vector2 a0, a1;
    int a1_index = index - 2;
    if (a1_index < 0) a1_index = 0;
/*
    if (index < 2) a1_index = 0; else a1_index = 1;

    a1_index = 3;
*/
    extern Scripting_System *scripting_system;
    a1 = vector2->history_values[a1_index];
    float k = scripting_system->precomputed_exponents[a1_index];
    Vector2 a = vector2->history_values[index];

    float ik = 1.0 / k;
    a0.x = ik * (a.x - (1-k)*a1.x);
    a0.y = ik * (a.y - (1-k)*a1.y);

    *a0_return = a0;
    *a1_return = a1;
}

void c_history_subtract(Command_Calling_Data *data) {
    Scripting_System *system = data->scripting_system;
    Value_General *value = system->current_return_value;

    bool success;
    success = check_number_of_arguments(data, 3);
    success &= do_core_math_operation_checks(data);
    success &= ensure_argument_is_lvalue(data, 2);

    Value_General *v0 = get_value(data, 0);
    Value_General *v1 = get_value(data, 1);
    Value_Lvalue *lvalue = get_lvalue(data, 2);
    if (lvalue->binding->value.type != v0->type) {
        system->report_error("Type mismatch between input and output arguments.\n");
        success = false;
    }
    if (!success) return;

    Value_General *result = &lvalue->binding->value;

    int i;
    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        if (v0->type == VARTYPE_SCALAR) {
            float s0 = v0->scalar.history_values[i];
            float s1 = v1->scalar.history_values[i];
            float var0 = v0->scalar.history_variances[i];
            float var1 = v1->scalar.history_variances[i];
            
            result->scalar.history_values[i] = s0 - s1;
            result->scalar.history_variances[i] = var0 + var1; // Not a mistake (I think)
        } else {
            assert(v0->type == VARTYPE_VECTOR2);
            Covariance2 var_A = v0->vector2.history_variances[i];
            Covariance2 var_B = v1->vector2.history_variances[i];

            Vector2 a = v0->vector2.history_values[i];
            Vector2 b = v1->vector2.history_values[i];

            Vector2 a0, a1, b0, b1;

            compute_endpoints(&v0->vector2, i, &a0, &a1);
            compute_endpoints(&v1->vector2, i, &b0, &b1);

            Covariance2 a0b0;
            Covariance2 a1b1;

            make_double_outer_product(&a0b0, a0, b0);
            make_double_outer_product(&a1b1, a1, b1);

            extern Scripting_System *scripting_system;
            float k = scripting_system->precomputed_exponents[i];
            a0b0.scale(-k);
            a1b1.scale(-(1-k));

            Vector2 vec2;
            vec2.x = a.x - b.x;
            vec2.y = a.y - b.y;


            beet();

            Covariance2 var_C;
            var_C = var_A.add(var_B);

            var_C = var_C.add(a0b0);
            var_C = var_C.add(a1b1);


            result->vector2.history_values[i] = vec2;
            result->vector2.history_variances[i] = var_C;
        }
    }
}

#ifdef BEFORE_CORRELATION_THING

void c_history_subtract(Command_Calling_Data *data) {
    Scripting_System *system = data->scripting_system;
    Value_General *value = system->current_return_value;

    bool success;
    success = check_number_of_arguments(data, 3);
    success &= do_core_math_operation_checks(data);
    success &= ensure_argument_is_lvalue(data, 2);

    Value_General *v0 = get_value(data, 0);
    Value_General *v1 = get_value(data, 1);
    Value_Lvalue *lvalue = get_lvalue(data, 2);
    if (lvalue->binding->value.type != v0->type) {
        system->report_error("Type mismatch between input and output arguments.\n");
        success = false;
    }
    if (!success) return;

    Value_General *result = &lvalue->binding->value;

    int i;
    for (i = 0; i < NUM_HISTORY_SLOTS; i++) {
        if (v0->type == VARTYPE_SCALAR) {
            float s0 = v0->scalar.history_values[i];
            float s1 = v1->scalar.history_values[i];
            float var0 = v0->scalar.history_variances[i];
            float var1 = v1->scalar.history_variances[i];
            
            result->scalar.history_values[i] = s0 - s1;
            result->scalar.history_variances[i] = var0 + var1; // Not a mistake (I think)
        } else {
            assert(v0->type == VARTYPE_VECTOR2);
            Vector2 vec0 = v0->vector2.history_values[i];
            Vector2 vec1 = v1->vector2.history_values[i];
            Covariance2 var0 = v0->vector2.history_variances[i];
            Covariance2 var1 = v1->vector2.history_variances[i];
            
            Vector2 vec2;
            vec2.x = vec0.x - vec1.x;
            vec2.y = vec0.y - vec1.y;


            beet();
            Covariance2 test = var0;
            Covariance2 test2, test3;
            test.move_to_local_coordinates(&test2, vec0.x, vec0.y);
            test2.move_to_global_coordinates(&test3, vec0.x, vec0.y);


            Covariance2 var0prime, var1prime;
            var0.move_to_local_coordinates(&var0prime, vec0.x, vec0.y);
            var1.move_to_local_coordinates(&var1prime, vec1.x, vec1.y);

            Covariance2 var2prime;
            var2prime.a = var0prime.a + var1prime.a;
            var2prime.b = var0prime.b + var1prime.b;
            var2prime.c = var0prime.c + var1prime.c;

            Covariance2 var2;
            var2prime.move_to_global_coordinates(&var2, vec2.x, vec2.y);

            result->vector2.history_values[i] = vec2;
            result->vector2.history_variances[i] = var2;
        }
    }
}
#endif

void c_instantiate_literal(Command_Calling_Data *data) {
    Scripting_System *system = data->scripting_system;

    bool success;
    success = check_number_of_arguments(data, 1);
    if (!success) return;

    Value_General *value = get_value(data, 0);
    data->scripting_system->configure_return_type(value->type);
    assign_value(data->scripting_system->current_return_value, value);
}

void c_is_greater_than(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;

    success = ensure_argument_is_scalar(data, 0);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 1);
    if (!success) return;

    Value_Scalar *arg1 = get_scalar(data, 0);
    Value_Scalar *arg2 = get_scalar(data, 1);



    Value_General *value = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
    Value_Scalar *result = &value->scalar;

    float diff = arg1->history_values[0] - arg2->history_values[0];
    float fvalue = 0;
    if (diff > 0) fvalue = 1;
    result->set(fvalue);
}

// XXX c_is_less_than is currently one big cutnpaste of c_is_greater_than
void c_is_less_than(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 2);
    if (!success) return;

    success = ensure_argument_is_scalar(data, 0);
    if (!success) return;
    success = ensure_argument_is_scalar(data, 1);
    if (!success) return;

    Value_Scalar *arg1 = get_scalar(data, 0);
    Value_Scalar *arg2 = get_scalar(data, 1);


    Value_General *value = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
    Value_Scalar *result = &value->scalar;

    float diff = arg1->history_values[0] - arg2->history_values[0];
    float fvalue = 0;
    if (diff < 0) fvalue = 1;
    result->set(fvalue);
}

void bind_command_to_list(void (*proc)(Command_Calling_Data *), char *name, Variable_Binding_List *list) {
    Variable_Binding *binding = list->declare(name, VARTYPE_COMMAND);
    binding->value.command.proc = proc;
}

void c_is_between(Command_Calling_Data *data) {
    bool success;
    success = check_number_of_arguments(data, 3);
    success &= ensure_argument_is_scalar(data, 0);
    success &= ensure_argument_is_scalar(data, 1);
    success &= ensure_argument_is_scalar(data, 2);
    if (!success) return;

    Value_Scalar *arg1 = get_scalar(data, 0);
    Value_Scalar *arg2 = get_scalar(data, 1);
    Value_Scalar *arg3 = get_scalar(data, 2);

    Value_General *value = data->scripting_system->configure_return_type(VARTYPE_SCALAR);
    Value_Scalar *result = &value->scalar;

    float fvalue = 1;
    if (arg1->history_values[0] < arg2->history_values[0]) fvalue = 0;
    if (arg1->history_values[0] > arg3->history_values[0]) fvalue = 0;

    result->set(fvalue);
}

void Scripting_System::init_standard_commands() {
    command_binding_list = new Variable_Binding_List;
    command_binding_list->init();

    bind_command_to_list(c_assign_return_value, "assign_return_value", command_binding_list);
    bind_command_to_list(c_instantiate_literal, "instantiate_literal", command_binding_list);

    bind_command_to_list(c_add, "add", command_binding_list);
    bind_command_to_list(c_subtract, "subtract", command_binding_list);
    bind_command_to_list(c_history_subtract, "history_subtract", command_binding_list);


    bind_command_to_list(c_draw_history, "draw_history", command_binding_list);
    bind_command_to_list(c_make_vector, "make_vector", command_binding_list);
    bind_command_to_list(c_print_variable, "print_variable", command_binding_list);
    bind_command_to_list(c_print_string, "print_string", command_binding_list);
    bind_command_to_list(c_is_greater_than, "is_greater_than", command_binding_list);
    bind_command_to_list(c_is_less_than, "is_less_than", command_binding_list);
    bind_command_to_list(c_is_between, "is_between", command_binding_list);

    bind_command_to_list(c_sample_tendency, "sample_tendency", command_binding_list);
    bind_command_to_list(c_draw_ellipse, "draw_ellipse", command_binding_list);

    bind_command_to_list(c_if, "if", command_binding_list);
    bind_command_to_list(c_endif, "endif", command_binding_list);
    bind_command_to_list(c_end, "end", command_binding_list);

    bind_command_to_list(c_length, "length", command_binding_list);
    bind_command_to_list(c_multiply, "multiply", command_binding_list);
    bind_command_to_list(c_clamp, "clamp", command_binding_list);
    bind_command_to_list(c_normalize, "normalize", command_binding_list);

    bind_command_to_list(c_and, "and", command_binding_list);
    bind_command_to_list(c_or, "or", command_binding_list);

    bind_command_to_list(c_copy, "copy", command_binding_list);
    bind_command_to_list(c_differences, "differences", command_binding_list);


    // These commands are specific to the Mortar Game
    bind_command_to_list(c_fire_mortar, "fire_mortar", command_binding_list);
}

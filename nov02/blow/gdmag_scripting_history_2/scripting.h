struct Variable_Binding_List;
struct Variable_Binding;
struct Scripting_System;
struct Command_Calling_Data;


// Regarding this predefined limit, see comment at the top of script_compiler.h.
const int MAX_ARGUMENTS_PER_COMMAND = 100;

#include "covariance.h" // In case we haven't yet

enum Variable_Type {
	VARTYPE_UNINITIALIZED = 0,
	VARTYPE_SCALAR,
	VARTYPE_VECTOR2,
	VARTYPE_STRING,

    VARTYPE_COMMAND,
    VARTYPE_LVALUE,

	NUM_VARIABLE_TYPES
};

struct Vector2 {
	void set(float, float);
	void normalize();

	float x, y;
};

extern const char *variable_type_names[];

// Right now we have a maximum number of variables that
// can be declared, which is controlled by the value MAX_BINDINGS.
// In a real application, you would really want to put in the
// elbow grease to efficiently allow an unlimited number of
// variables.

const int MAX_BINDINGS = 100;

const int NUM_HISTORY_SLOTS = 10;

struct Value_Scalar {
	double history_values[NUM_HISTORY_SLOTS];
	double history_variances[NUM_HISTORY_SLOTS];
	
	void init();
	void update(double dt, Scripting_System *system);
	void set(const float &scalar_value);
	void set_for_all_history(const float &scalar_value);
};

struct Value_Vector2 {
	Vector2 history_values[NUM_HISTORY_SLOTS];
	Covariance2 history_variances[NUM_HISTORY_SLOTS];
	
	void set(const Vector2 &vector2_value);
	void set(float x, float y);
	void set_for_all_history(const Vector2 &vector2_value);
	
	void init();
	void update(double dt, Scripting_System *system);
};

struct Value_Command {
    void init();
    void (*proc)(Command_Calling_Data *data);
};

struct Value_Lvalue {
    Variable_Binding *binding;
};

struct Value_String {
    char *data;
};

// Note that Value_General is not very memory efficient; i.e.
// it unions very big stuff (like Value_Vector2) with very small
// stuff (like Value_Lvalue)... I would not abide this in a
// production system.
struct Value_General {
	Variable_Type type;

    union {
        Value_Scalar scalar;
        Value_Vector2 vector2;
        Value_Command command;
        Value_Lvalue lvalue;
        Value_String string;
    };
};

struct Variable_Binding {
	char *name;

	void update(double dt, Scripting_System *system);

    Value_General value;
};

struct Variable_Binding_List {
	void init();

	Variable_Binding *lookup(char *name);
	
	Variable_Binding *declare(char *name, Variable_Type type);
	
	Variable_Binding bindings[MAX_BINDINGS];
	int num_bindings;

  protected:
	Variable_Binding *prepare_binding(char *name);
};




struct Command_Argument_Spec {
    enum Argument_Type {
        VARIABLE = 0,
        LITERAL_REAL,
        LITERAL_STRING,
        LVALUE
    };

    Argument_Type type;

    union {
        char *variable_name;
        char *string_data;
        float real_value;
    };
};

struct Script_Command {
    char *command_name;
    int argc;
    int line_number;
    Command_Argument_Spec *arguments;
};

struct Script_Command_Sequence {
    int num_commands;
    Script_Command *commands;
};

struct Compiled_Script {
    Script_Command_Sequence *commands;
    Variable_Binding_List *global_variables;
};


struct Scripting_System {
	Scripting_System();
	~Scripting_System();

	void print_value(Variable_Binding *binding, char *buf, int len_max);
	
    Variable_Binding_List *command_binding_list;
	Variable_Binding_List *variable_binding_list;


	double last_update_time;
    int line_number;
    int program_counter;
    FILE *log_file;
	double dt;
    Compiled_Script *current_script;
    Value_General *current_return_value;

    Value_General static_argument_pool[MAX_ARGUMENTS_PER_COMMAND];

	void init();
	void update(double now);
    void run_script(Compiled_Script *script);

  // protected:

    void init_standard_commands();

    bool evaluate_argument(Command_Argument_Spec *unevaluated_argument,
                           Value_General *result);
    void update_system_defined_globals(Variable_Binding_List *bindings, double now);

	float history_times_to_reach_90_percent[NUM_HISTORY_SLOTS];
	float precomputed_exponents[NUM_HISTORY_SLOTS];

	void interpolate_vector2(Value_Vector2 *vector2, float time,
							 Vector2 *result_return,
                             Covariance2 *cov_return);

    void interpolate_scalar(Value_Scalar *scalar, float time,
                            double *result_return);
    Value_General *configure_return_type(Variable_Type type);

    void report_error(char *error_string);

	bool first_update;

  protected:
	void precompute_exponents(double dt);
	void update_binding_list(Variable_Binding_List *);
};

inline void Vector2::set(float _x, float _y) {
	x = _x;
	y = _y;
}

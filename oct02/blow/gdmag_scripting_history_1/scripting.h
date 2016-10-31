struct Variable_Binding_List;
struct Scripting_System;

#include "covariance.h" // In case we haven't yet

enum Variable_Type {
	VARTYPE_UNINITIALIZED = 0,
	VARTYPE_SCALAR,
	VARTYPE_VECTOR2,

	NUM_VARIABLE_TYPES
};

struct Vector2 {
	void set(float, float);
	
	float x, y;
};

extern const char *variable_type_names[];

// Right now we have a maximum number of variables that
// can be declared, which is controlled by the value MAX_BINDINGS.
// In a real application, you would really want to put in the
// elbow grease to efficiently allow an unlimited number of
// variables.

const int MAX_BINDINGS = 100;

const int NUM_HISTORY_SLOTS = 9;

struct Value_Scalar {
	double instantaneous_value;
	double history_values[NUM_HISTORY_SLOTS];

	double history_covariances[NUM_HISTORY_SLOTS];
	
	void init();
	void update(double dt, Scripting_System *system);
};

struct Value_Vector2 {
	Vector2 instantaneous_value;
	Vector2 history_values[NUM_HISTORY_SLOTS];

	Covariance2 history_covariances[NUM_HISTORY_SLOTS];
	
	void init();
	void update(double dt, Scripting_System *system);
};

struct Variable_Binding {
	char *name;
	Variable_Type type;

	void set(Variable_Binding_List *vars, const double &scalar_value);
	void set(Variable_Binding_List *vars, const Vector2 &vector2_value);
	
	void update(double dt, Scripting_System *system);

	union {
		Value_Scalar scalar;
		Value_Vector2 vector2;
	};
};

struct Variable_Binding_List {
	void init();

	Variable_Binding *lookup(char *name);
	
	void declare(char *name, Variable_Type type);
	
	Variable_Binding bindings[MAX_BINDINGS];
	int num_bindings;

  protected:
	Variable_Binding *prepare_binding(char *name);
};

struct Scripting_System {
	Scripting_System();
	~Scripting_System();

	void print_value(Variable_Binding *binding, char *buf, int len_max);
	
	Variable_Binding_List *variable_binding_list;
	double last_update_time;

	void init();
	void update(double now, bool test = false);

  // protected:

	double dt;
	float history_times_to_reach_90_percent[NUM_HISTORY_SLOTS];
	float precomputed_exponents[NUM_HISTORY_SLOTS];

	void interpolate_vector2(Variable_Binding *binding, float time,
							 Vector2 *result_return);

	bool first_update;

  protected:
	void precompute_exponents(double dt);
	void update_binding_list(Variable_Binding_List *);
	void update_binding_list_test(Variable_Binding_List *);
};

inline void Vector2::set(float _x, float _y) {
	x = _x;
	y = _y;
}

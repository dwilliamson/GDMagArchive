bool check_number_of_arguments(Command_Calling_Data *data, int num_desired);
Value_General *get_value(Command_Calling_Data *data, int index);
Value_Scalar *get_scalar(Command_Calling_Data *data, int index);
Value_Vector2 *get_vector2(Command_Calling_Data *data, int index);
Value_String *get_string(Command_Calling_Data *data, int index);
Value_Lvalue *get_lvalue(Command_Calling_Data *data, int index);
bool ensure_argument_is_lvalue(Command_Calling_Data *data, int index);
bool ensure_argument_is_scalar_or_vector(Command_Calling_Data *data, int index);
bool ensure_argument_is_vector2(Command_Calling_Data *data, int index);
bool ensure_argument_is_scalar(Command_Calling_Data *data, int index);
bool ensure_argument_is_string(Command_Calling_Data *data, int index);


bool do_math_operation_checks(Command_Calling_Data *data);



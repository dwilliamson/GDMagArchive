
bool argument_count(Lerp_Interp *interp, Lerp_Call_Record *record, int count);
bool coerce_to_integer(Lerp_Interp *interp, Lerp_Call_Record *record, int index, int *result);
bool coerce_to_float(Lerp_Interp *interp, Lerp_Call_Record *record, int index, double *result);
bool coerce_to_float(Lerp_Interp *interp, Lerp_Call_Record *record, int index, float *result);
bool get_string(Lerp_Interp *interp, Lerp_Call_Record *record, int index, char **result);

void bind_value(Lerp_Interp *interp, char *name_string, First_Class *value);
void register_proc(Lerp_Interp *interp, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *));
Procedure *make_procedure(Lerp_Interp *interp, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *));

bool get_integer_member(Lerp_Interp *interp, Lerp_Call_Record *record, Atom *name, int *result);
bool get_string_member(Lerp_Interp *interp, Lerp_Call_Record *record, Atom *name, char **result);

Schema *make_schema(Lerp_Interp *interp, int num_slots);
void trim_schema(Schema *schema, int length);
Database *install_module(Lerp_Interp *interp, Schema *schema, char *name);

//
// Also see file module_macros.h
//

#define Register(x) register_proc(interp, #x, proc_##x)
#define Bind(x, v) bind_value(interp, x, v)


// @WriteBarrier
#define Return(v) (record->registers[0] = ToBarrier((First_Class *)(v))); return;


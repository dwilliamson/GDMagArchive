struct Lerp_Module_Init_Info {
    unsigned long system_version;
    char *default_name;
};

LEXPORT bool argument_count(Lerp_Interp *interp, Lerp_Call_Record *record, int count);
LEXPORT bool coerce_to_integer(Lerp_Interp *interp, Lerp_Call_Record *record, int index, int *result);
LEXPORT bool coerce_to_float(Lerp_Interp *interp, Lerp_Call_Record *record, int index, double *result);
LEXPORT bool coerce_to_float(Lerp_Interp *interp, Lerp_Call_Record *record, int index, float *result);
LEXPORT bool get_string(Lerp_Interp *interp, Lerp_Call_Record *record, int index, char **result);

LEXPORT void bind_value(Lerp_Interp *interp, Database *db, char *name_string, First_Class *value);
LEXPORT void register_proc(Lerp_Interp *interp, Database *db, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *));
LEXPORT Procedure *make_procedure(Lerp_Interp *interp, char *name_string, void (*proc)(Lerp_Interp *, Lerp_Call_Record *));

LEXPORT bool get_integer_member(Lerp_Interp *interp, Lerp_Call_Record *record, Atom *name, int *result);
LEXPORT bool get_string_member(Lerp_Interp *interp, Lerp_Call_Record *record, Atom *name, char **result);

LEXPORT Schema *make_schema(Lerp_Interp *interp, int num_slots);
LEXPORT void trim_schema(Lerp_Interp *interp, Schema *schema, int length);
LEXPORT Database *install_module(Lerp_Interp *interp, Schema *schema, char *name);



#define Register(x) register_proc(interp, db, #x, proc_##x)
#define Bind(x, v) bind_value(interp, db, x, v)

// @WriteBarrier
#define Return(v) (record->registers[0] = ToBarrier((First_Class *)(v))); return;



#define IDENT(s) interp->parser->make_atom(#s)
#define MEMBER1(s) ns->initialize_slot(schema_cursor++, IDENT(s), (First_Class *)
#define MEMBER(s, type, value) MEMBER1(s) interp->parser->make_##type(value));
#define MEMBER_UNINIT(s) MEMBER1(s) interp->uninitialized);
#define MEMBER_PROC(s) MEMBER1(s) make_procedure(interp, #s, proc_##s));
#define MEMBER_PROC2(s, s2) MEMBER1(s) make_procedure(interp, #s, proc_##s2));
#define MEMBER_BLOB_NULL(s) MEMBER1(s) interp->the_null_blob);

#define DEFINE_STRING(s) s##_string = interp->parser->make_string(#s);
#define DEFINE_ATOM(s) s##_atom = interp->parser->make_atom(#s);

#define BEGIN_TRACING(interp) Lerp_Memory_Manager *_mm = (interp)->memory_manager;
#define TRACE(p) *(First_Class **)&(p) = _mm->gc_consider(p);
#define TRACE2(p, x) *(First_Class **)&(x->p) = _mm->gc_consider(x->p);

// @Refactor: We can get rid of TRACE2 if our policy is just that things live in the
// global namespace of the DLL... hmm maybe that isn't nice, though.


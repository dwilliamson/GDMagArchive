#define IDENT(s) interp->parser->make_atom(#s)
#define MEMBER1(s) ns->initialize_slot(schema_cursor++, IDENT(s), (First_Class *)
#define MEMBER(s, type, value) MEMBER1(s) interp->parser->make_##type(value));
#define MEMBER_UNINIT(s) MEMBER1(s) interp->uninitialized);
#define MEMBER_PROC(s) MEMBER1(s) make_procedure(interp, #s, proc_##s));
#define MEMBER_PROC2(s, s2) MEMBER1(s) make_procedure(interp, #s, proc_##s2));
#define MEMBER_BLOB_NULL(s) MEMBER1(s) interp->the_null_blob);

#define DEFINE_STRING(s) s##_string = interp->parser->make_string(#s);
#define DEFINE_ATOM(s) s##_atom = interp->parser->make_atom(#s);


// Requires module_helper.h to be included first, for definition of Lerp_Module_Init_Info.

struct Lerp_Dll_Module {
    Lerp_Dll_Module();

    void (*get_module_info)(Lerp_Module_Init_Info *system_version_return);
    Database *(*instantiate)(Lerp_Interp *);
    void (*init)(Lerp_Interp *);
    void (*shutdown)(Lerp_Interp *);
    void (*enumerate_gc_roots)(Lerp_Interp *);

    Lerp_Module_Init_Info module_init_info;
    bool loaded_successfully;
};

struct File_Load_Info : public Blob {
    File_Load_Info();

    String *name;
    String *path;
};

struct Module_Load_Info : public Blob {
    Module_Load_Info();

    String *name;
    String *path;
    Database *instance;   // @Memory:  This should be a weak pointer!  But we don't have a mechanism for weak pointers.  Hmm, argh.  Maybe we disallow unbinding a module at runtime for now.
    Lerp_Dll_Module dll_module;
};




/*
// NEW_BLOB/mark_blob are a little bit un-kosher as we first fill in the size
// member of the Blob, then we call its constructor afterward.  However, this
// should still be okay since we are in control of the memory.
inline void *mark_blob(void *memory, int size) {
    Blob *blob = reinterpret_cast <Blob *>(memory);
    blob->size_in_bytes = size;
    return (void *)blob;
}

#define NEW_BLOB(interp, type) new (mark_blob(GET_MEMORY(interp, type), sizeof(type))) type()
*/

#define NEW_BLOB(interp, type) new type()

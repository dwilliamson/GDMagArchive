#include "general.h"
#include "parser.h"
#include "interp.h"
#include "mprintf.h"
#include "module_helper.h"
#include "code_manager.h"
#include "code_manager_private.h"

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#define MAP(name) \
    {FARPROC proc = GetProcAddress(hmodule, #name); \
    *(FARPROC *)&(dll->name) = proc;}


void Code_Manager::map_module_specifying_path(Module_Load_Info *module, String *path) {
    char *debug_suffix = "";
#ifdef _DEBUG
    debug_suffix = ".debug";
#endif

    char *path_name;
    if (path) path_name = path->value;
    else path_name = "modules/";

    char *full_name = mprintf("%s%s%s.dll",
                              path_name,
                              module->name->value, debug_suffix);
    
    HMODULE hmodule = LoadLibrary(full_name);
    delete [] full_name;

    if (hmodule == NULL) return;

    Lerp_Dll_Module *dll = &module->dll_module;

    bool success = true;
    MAP(get_module_info);
    MAP(instantiate);
    MAP(init);
    MAP(shutdown);
    MAP(enumerate_gc_roots);

    if (!success) return;

    dll->get_module_info(&dll->module_init_info);
    if (dll->module_init_info.system_version != LERP_SYSTEM_VERSION) return;  // @Robustness: Report a good error message here...

    dll->init(interp);
    dll->loaded_successfully = true;
}

void Code_Manager::map_module(Module_Load_Info *module) {
    if (module->path) {
        map_module_specifying_path(module, module->path);
    }

    if (!module->dll_module.loaded_successfully) {
        // Try it without the path...
        map_module_specifying_path(module, NULL);
    }
}

Module_Load_Info *Code_Manager::map_module(char *name) {
    Module_Load_Info *module = NEW_BLOB(interp, Module_Load_Info);
    module->name = interp->parser->make_string(name);
    map_module(module);

    if (!module->dll_module.loaded_successfully) return NULL;
    return module;
}


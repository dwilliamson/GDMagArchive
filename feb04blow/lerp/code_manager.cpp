#include "general.h"
#include "parser.h"
#include "interp.h"
#include "code_manager.h"
#include "foreach.h"
#include "module_helper.h"
#include "code_manager_private.h"
#include "unicode.h"

#include "parser_private.h"
#include "ast_directive.h"
#include "bytecode.h"
#include "schema.h"
#include "bytecode_builder.h"
#include "mprintf.h"

#include <stdlib.h>

Code_Manager::Code_Manager(Lerp_Interp *_interp) {
    interp = _interp;

    modules_to_load = GC_NEW(Database);
    modules_loaded = GC_NEW(Database);

    files_to_load = GC_NEW(Database);
    files_loaded = GC_NEW(Database);
    
    import_directives = GC_NEW(Database);

    current_path_name = NULL;
}

Code_Manager::~Code_Manager() {
}

void Code_Manager::trace_module_db(Database **db_ptr) {
    Database *db = *db_ptr;

    BEGIN_TRACING(interp);

    TRACE(db);
    Database_Foreach(db) {
        if (expression->num_arguments < 1) continue;

        First_Class *fc = expression->arguments[0]->read();
        if (fc->type != ARG_BLOB_UNMANAGED) continue;

        // @Robustness: Make sure that nobody is able to write into
        // this database!  Argh.
        Module_Load_Info *info = reinterpret_cast <Module_Load_Info *>(fc);
        TRACE(info->name);
        if (info->path) TRACE(info->path);
        TRACE(info->instance);
        if (info->dll_module.loaded_successfully) info->dll_module.enumerate_gc_roots(interp);
    } Database_Endeach;

    *db_ptr = db;
}

void breaky();

void Code_Manager::trace_file_db(Database **db_ptr) {
    Database *db = *db_ptr;

    BEGIN_TRACING(interp);

    TRACE(db);
    Database_Foreach(db) {
        if (expression == (First_Class *)0x008d056c) breaky();
        if (expression->num_arguments < 1) continue;

        First_Class *fc = expression->arguments[0]->read();
        assert(fc->type == ARG_BLOB_UNMANAGED);

        // @Robustness: Make sure that nobody is able to write into
        // this database!  Argh.
        File_Load_Info *info = reinterpret_cast <File_Load_Info *>(fc);
        TRACE(info->name);
        if (info->path) TRACE(info->path);
    } Database_Endeach;

    *db_ptr = db;
}

void Code_Manager::enumerate_gc_roots() {
    BEGIN_TRACING(interp);

    trace_module_db(&modules_to_load);
    trace_module_db(&modules_loaded);
    trace_file_db(&files_to_load);
    trace_file_db(&files_loaded);

    if (current_path_name) TRACE(current_path_name);

    TRACE(import_directives);  // No blobs in here so we can just trace it directly.
}


void Code_Manager::copy_definitions_into_global_namespace(Database *db) {
    Database *destination = interp->global_database;

    Database_Foreach(db) {
        if (expression->num_arguments != 3) continue;

        destination->add_assertion(interp,
                                   expression->arguments[0]->read(),   // __member
                                   expression->arguments[1]->read(),   // variable name
                                   expression->arguments[2]->read());  // value
    } Database_Endeach;
}

void Code_Manager::load_default_modules() {
    Module_Load_Info *module = map_module("primitives_0");
    if (!module) {
        interp->report_error("Unable to load default module 'primitives_0'!\n");
        return;
    }

    Database *db = module->dll_module.instantiate(interp);
    if (db == NULL) return;

    copy_definitions_into_global_namespace(db);
}

bool Code_Manager::load_module(Module_Load_Info *info) {
    assert(info->name);
    map_module(info);
    if (!info->dll_module.loaded_successfully) {
        interp->report_error("Unable to load module: %s\n", info->name->value);
        return false;
    }

    Database *db = info->dll_module.instantiate(interp);
    assert(info->instance == NULL);
    info->instance = db;

    // @Incomplete: For now all modules get bound into the global namespace.
    Atom *atom = interp->parser->make_atom(info->dll_module.module_init_info.default_name);
    printf("Binding module as name: %s\n", atom->name);
    interp->global_database->assign_named_slot(interp, atom, db);

    return true;
}

Decl_Assertion *remove_first_assertion(Database *db) {
    if (db->assertions == NULL) return NULL;
    Decl_Assertion *result = db->assertions->read();
    db->assertions = result->next;  // @WriteBarrier
    return result;
}

File_Load_Info *get_file_load_info(Decl_Assertion *assertion) {
    Decl_Expression *expression = assertion->expression->read();
    if (expression->num_arguments < 1) return NULL;
    
    File_Load_Info *info = reinterpret_cast <File_Load_Info *> (expression->arguments[0]->read());
    return info;
}

Module_Load_Info *get_module_load_info(Decl_Assertion *assertion) {
    Decl_Expression *expression = assertion->expression->read();
    if (expression->num_arguments < 1) return NULL;
    
    Module_Load_Info *info = reinterpret_cast <Module_Load_Info *> (expression->arguments[0]->read());
    return info;
}

void Code_Manager::do_one_import(Value_Pair *import) {
    First_Class *fc = interp->global_database;
    Atom *dest_name = NULL;

    while (import) {
        if (fc->type != ARG_DATABASE) {
            interp->report_error("Import directive: Attempt to dereference through a non-database.\n");  // @Robustness @Incomplete: Lame error message, doesn't say where or what.
            return;
        }

        Atom *name = static_cast <Atom *>(import->left->read());
        assert(name->type == ARG_ATOM);
        dest_name = name;  // Keeps getting overwritten until the end.

        Database *db = static_cast <Database *>(fc);
        fc = db->lookup_named_slot(interp, name);
        if (fc == NULL) {
            interp->report_error("Import directive: Symbol '%s' not found.\n", name->name);   // @Robustness @Incomplete: Lame error message, doesn't say where or entirely what.
            return;
        }

        import = static_cast <Value_Pair *>(import->right->read());
    }

    assert(dest_name != NULL);
    assert(fc != NULL);

    // @Incomplete: For now, all imports go into the global database... this will change at some point.
    Database *dest_db = interp->global_database;
    dest_db->assign_named_slot(interp, dest_name, fc);
}

void Code_Manager::perform_loads() {
    int num_changes;

    do {
        num_changes = 0;

        while (1) {
            Decl_Assertion *assertion = remove_first_assertion(modules_to_load);
            if (assertion == NULL) break;

            Module_Load_Info *info = get_module_load_info(assertion);
            load_module(info);

            num_changes++;
            modules_loaded->add_assertion(assertion);
        }

        while (1) {
            Decl_Assertion *assertion = remove_first_assertion(files_to_load);
            if (assertion == NULL) break;

            File_Load_Info *info = get_file_load_info(assertion);
            files_loaded->add_assertion(assertion);
            load_file(info);

            num_changes++;
        }
    } while (num_changes);

    // Now we process all the import directives.
    Database_Foreach(import_directives) {
        assert(expression->num_arguments == 1);
        do_one_import(static_cast <Value_Pair *>(expression->arguments[0]->read()));
    } Database_Endeach;


    // If it's a debug build, gc a bunch of times to catch problems early...
#ifdef _DEBUG
    interp->memory_manager->gc();
    interp->memory_manager->gc();
    interp->memory_manager->gc();
#endif
}


bool Code_Manager::module_already_considered(Database *db, String *name) {
    Database_Foreach(db) {
        if (expression->num_arguments < 1) continue;
        Module_Load_Info *info = reinterpret_cast <Module_Load_Info *> (expression->arguments[0]->read());
        if (Unicode::strings_match(name->value, info->name->value)) return true;
    } Database_Endeach;

    return false;
}

bool Code_Manager::file_already_considered(Database *db, String *name) {
    Database_Foreach(db) {
        if (expression->num_arguments < 1) continue;
        File_Load_Info *info = reinterpret_cast <File_Load_Info *> (expression->arguments[0]->read());
        if (Unicode::strings_match(name->value, info->name->value)) return true;
    } Database_Endeach;

    return false;
}

void Code_Manager::add_file_to_load(String *path, String *name) {
    if (file_already_considered(files_to_load, name)) return;
    if (file_already_considered(files_loaded, name)) return;

    File_Load_Info *file = NEW_BLOB(interp, File_Load_Info);
    file->name = name;
    file->path = path;

    files_to_load->add_assertion(interp, file);
}

void Code_Manager::add_module_to_load(String *path, String *name) {
    if (module_already_considered(modules_to_load, name)) return;
    if (module_already_considered(modules_loaded, name)) return;

    Module_Load_Info *module = NEW_BLOB(interp, Module_Load_Info);

    module->name = name;
    module->path = path;
    modules_to_load->add_assertion(interp, module);
}


Module_Load_Info::Module_Load_Info(){
    name = NULL;
    path = NULL;
    instance = NULL;
}

File_Load_Info::File_Load_Info() {
    name = NULL;
    path = NULL;
}

Lerp_Dll_Module::Lerp_Dll_Module() {
    get_module_info = NULL;
    instantiate = NULL;
    shutdown = NULL;
    enumerate_gc_roots = NULL;
    loaded_successfully = false;
}


Value_Pair *Code_Manager::validate_import_source(Ast_Expression *source) {
    if (source == NULL) return NULL;

    First_Class *leaf;
    Ast_Binary_Expression *binary = NULL;

    if (source->type == AST_BINARY_EXPRESSION) {
        binary = static_cast <Ast_Binary_Expression *>(source);
        leaf = binary->left;
    } else if (source->type == ARG_ATOM) {
        leaf = static_cast <First_Class *>(source);
    } else {
        interp->report_error("Import directive: only a dotted identifier path is allowed.\n"); // @Incomplete @Robustness: This is a lame error message, it says nothing about where this occurred.
        return NULL;
    }

    // @Robustness: Right now, if we get an error partway through the path, we will
    // still be returning an incomplete path which will later cause a binding to
    // occur to the wrong thing (possibly generating other error messages?)  What we
    // want to do is, after returning from this, note whether an error occurred and
    // if so, don't bind anything.

    Value_Pair *pair = GC_NEW(Value_Pair);
    pair->left = ToBarrierF(leaf);

    if (binary) {
        pair->right = ToBarrierF(validate_import_source(binary->right));
    } else {
        pair->right = NULL;
    }

    return pair;
}

void Code_Manager::process_import_directive(Ast_Directive_Import *import) {
    Ast_Expression *source = import->source_value;
    Value_Pair *processed_import = validate_import_source(source);
    if (import) {
        import_directives->add_assertion(interp, processed_import);
    }
}

static void chop(Text_Utf8 *string) {
    Text_Utf8 *s = string;

    Text_Utf8 *last_slash = NULL;
    while (*s) {
        if ((*s == '/') || (*s == '\\')) {
            last_slash = s;
            *s = '/';  // Make sure it's a forward slash (in case we're on win32)...
        }

        int bytes_for_this_character = 1 + Unicode::trailingBytesForUTF8[*s];
        s += bytes_for_this_character;
    }

    if (last_slash) {
        // Put a 0 after the last_slash...
        Text_Utf8 *t = last_slash + 1;
        *t = '\0';
        return;
    }

    // We didn't find a directory separator, thus the path should be empty.
    // So we put a 0 into the first byte.
    *string = '\0';
}

String *Code_Manager::concatenate(String *a, String *b) {
    if (!a || (a->value[0] == '\0')) return b;

    int size_a = Unicode::size_in_bytes(a->value);
    int size_b = Unicode::size_in_bytes(b->value);
    int size_in_bytes = size_a + size_b - 1; // Eliminate zero-termination on a

    String *result = interp->parser->make_string(size_in_bytes);

    memcpy(result->value, a->value, size_a - 1);
    memcpy(result->value + size_a - 1, b->value, size_b);

    return result;
}

bool Code_Manager::load_file(File_Load_Info *info) {
    String *full_name = concatenate(info->path, info->name);

    char *filename = info->name->value;

    String *local_path = interp->parser->make_string(filename);  // Copy so we can clobber it
    chop(local_path->value);  // First we copy the whole string, then chop it... a tad wasteful but really who cares?
    current_path_name = concatenate(info->path, local_path);

    FILE *f = fopen(full_name->value, "rt");
    if (f == NULL) {
        interp->report_error("Unable to open file '%s' for reading.\n", full_name->value);
        return false;
    }

    interp->parser->set_input_from_file((void *)f);

    while (1) {
        Ast *toplevel = interp->parser->parse_toplevel();
        if (!toplevel) break;

        if (interp->parse_error) break;

        switch (toplevel->type) {
        case AST_PROC_DECLARATION: {
            Ast_Proc_Declaration *decl = (Ast_Proc_Declaration *)toplevel;

            Lerp_Bytecode *bytecode = interp->bytecode_builder->build(decl);
            if (bytecode) {
                Procedure *proc = interp->parser->make_procedure(decl->proc_name);
                proc->bytecode = ToBarrier(bytecode);
                bytecode->procedure = ToBarrier(proc);

                if (decl->operator_number == -1) {
                    // Not an operator, just file it under its usual name
                    interp->global_database->assign_named_slot(interp, proc->name, ToBarrierF(proc));
                } else {
                    // It's an operator... make a tuple for this guy.

                    int num_operator_arguments = bytecode->num_arguments;
                    assert((num_operator_arguments == 1) || (num_operator_arguments == 2));
                    int num_elements = num_operator_arguments + 3;

                    Atom *first_type = (Atom *)bytecode->argument_info->read()->values[0]->read();
                    Atom *second_type = NULL;
                    if (num_operator_arguments > 1) second_type = (Atom *)bytecode->argument_info->read()->values[2]->read();
                    assert(first_type->type == ARG_ATOM);
                    if (second_type) assert(second_type->type == ARG_ATOM);

                    Decl_Expression *expression = interp->memory_manager->create_decl_expression(num_elements);
                    expression->initialize_slot(0, decl->proc_name);  // This will just be atom "_operator"
                    expression->initialize_slot(1, interp->parser->make_integer(decl->operator_number));
                    expression->initialize_slot(2, proc);
                    expression->initialize_slot(3, first_type);
                    if (second_type) expression->initialize_slot(4, second_type);

                    interp->global_database->add_assertion(interp, expression);
                }
            }

            break;
        }

        case ARG_DECL_ASSERTION: {
            interp->add_assertion((Decl_Assertion *)toplevel);
            break;
        }

        case ARG_SCHEMA: {
            Schema *space = reinterpret_cast <Schema *> (toplevel);
            interp->global_database->assign_named_slot(interp, space->type_name, ToBarrierF(space));
            break;
        }

        case AST_DIRECTIVE_MODULE: {
            Ast_Directive_Load *load = reinterpret_cast <Ast_Directive_Load *>(toplevel);
            Value_Pair *pair = load->file_list->read();
            while (pair) {
                String *string = reinterpret_cast <String *>(pair->left->read());
                add_module_to_load(current_path_name, string);
                pair = reinterpret_cast <Value_Pair *>(pair->right->read());
            }
            break;
        }
             
        case AST_DIRECTIVE_IMPORT: {
            Ast_Directive_Import *import = reinterpret_cast <Ast_Directive_Import *>(toplevel);
            process_import_directive(import);
            break;
        }

        case AST_DIRECTIVE_LOAD: {
            Ast_Directive_Load *load = reinterpret_cast <Ast_Directive_Load *>(toplevel);
            Value_Pair *pair = load->file_list->read();
            while (pair) {
                String *string = reinterpret_cast <String *>(pair->left->read());
                add_file_to_load(current_path_name, string);
                pair = reinterpret_cast <Value_Pair *>(pair->right->read());
            }
            break;
        }
/*
        case AST_STRUCT_DECLARATION: {
            // @Incomplete: register this guy somehow (or ignore!)
            break;
        }
*/
/*
        case AST_UNARY_EXPRESSION:
        case AST_BINARY_EXPRESSION:
        case AST_PROCEDURE_CALL_EXPRESSION: {
            Lerp_Bytecode *bytecode = interp->bytecode_builder->build(
            assert(0);
            break;
        }
*/
        default:
            assert(0);  // This will not do, of course!
            break;
        }
    }

    fclose(f);

    current_path_name = NULL;

    return true;
}


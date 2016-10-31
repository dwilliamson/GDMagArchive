#include "general.h"
#include "module_helper.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"

#include "os_specific_opengl_headers.h"
#include "app_shell.h"

#include "jpeg_load.h"

#include "concatenator.h"
#include "gl_and_windows.h"


Window_Lib *glib;

extern "C" {
    DLL_EXPORT void get_module_info(Lerp_Module_Init_Info *info_return);
    DLL_EXPORT Database *instantiate(Lerp_Interp *interp);
    DLL_EXPORT void init(Lerp_Interp *interp);
    DLL_EXPORT void shutdown(Lerp_Interp *);
    DLL_EXPORT void enumerate_gc_roots(Lerp_Interp *);
};



const int WINDOW_DEFAULT_WIDTH = 100;
const int WINDOW_DEFAULT_HEIGHT = 100;

const char *APP_NAME = "Lerp Window System DLL";
const char *WINDOW_CLASS_NAME = "Lerp GL Window";

void proc_do_window_events(Lerp_Interp *interp, Lerp_Call_Record *record);


#define FILL_SLOT(owner, name)     (owner)->assign_named_slot(interp, interp->parser->make_atom(#name), interp->parser->make_integer(name));
#define FILL_SLOT_F(owner, name)     (owner)->assign_named_slot(interp, interp->parser->make_atom(#name), interp->parser->make_float(name));


Private_Window_Blob::Private_Window_Blob() : Blob() {
    size_in_bytes = sizeof(this);
}

bool is_window_pointer(Lerp_Interp *interp, First_Class *fc) {
    if (fc->type != ARG_DATABASE) return false;

    Database *db = (Database *)fc;
    Schema *space = db->schema->read();
    if (!space) return false;
    if (space->type_name != glib->Window_Type_atom) return false;

    return true;
}





void set_pixel_format(HDC dc) {
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),  // Size
		1,                              // Version
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,                              // 24-bit color 
		0, 0, 0, 0, 0, 0,                // A bunch of unused stuff.
		0, 0,
		0, 0, 0, 0, 0,
		32,                              // Z-buffer depth
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0 };
	
	int format = ChoosePixelFormat(dc, &pfd);
    assert(format != 0);

    PIXELFORMATDESCRIPTOR new_pfd;
    int desc_result = DescribePixelFormat(dc, format, sizeof(new_pfd), &new_pfd);
    assert(desc_result != NULL);

	BOOL result = SetPixelFormat(dc, format, &new_pfd);
    assert(result);
}

static int translate_ascii_code(WPARAM wParam) {
    switch (wParam) {
    // Hacky keybindings so that we don't have to define a bunch of
    // OS-independent constants for these, like we would if we
    // were writing a real game input system.
    case VK_LEFT:
    case VK_PRIOR:
        return App_Shell::ARROW_LEFT;
    case VK_RIGHT:
    case VK_NEXT:
        return App_Shell::ARROW_RIGHT;
    case VK_UP:
        return App_Shell::ARROW_UP;
    case VK_DOWN:
        return App_Shell::ARROW_DOWN;
    case VK_DELETE:
        return ')';
    default:
        return wParam;
    }
}

Schema *get_window_namespace() {
    return glib->Window_namespace;
}

void Window_Lib::update_window_focus(First_Class *win, bool has_focus) {
    Database *owner = glib->Windows;
    Schema *space = owner->schema->read();
    if (!space) return;

    if (has_focus) {
        owner->assign_named_slot(interp, glib->focus_window_atom, ToBarrier(win));
    } else {
        // We are assuming that Windows always passes the "lose focus" message
        // before the subsequent "gain focus" message (this seems to always be the
        // case) so we don't bother checking the current value of the focus window...
        // if we get a focus 0, then that means no window in our application has
        // focus until we hear otherwise.
        owner->assign_named_slot(interp, glib->focus_window_atom, ToBarrier(interp->uninitialized));
    }
}

Private_Window_Blob *get_blob(Lerp_Interp *interp, Database *owner) {
    First_Class *blob_fc = owner->lookup_named_slot(interp, glib->resources_atom);
    if (!blob_fc) return NULL;  // Uhhh... this would mean something very bad is going on.
    if (blob_fc->type != ARG_BLOB_UNMANAGED) return NULL;

    Private_Window_Blob *blob = (Private_Window_Blob *)blob_fc;
    return blob;
}

Database *find_window_record(HWND hwnd, Private_Window_Blob **blob_result = NULL) {
    Schema *space = get_window_namespace();

    Lerp_Interp *interp = glib->interp;
    List *list = &glib->list_of_windows;
    Database *owner;
    Foreach(list, owner) {
        assert(owner->type == ARG_DATABASE);
        
        Private_Window_Blob *blob = get_blob(interp, owner);
        if (blob->hwnd != hwnd) continue;

        //
        // We've got a match!!
        //
        if (blob_result) *blob_result = blob;

        return owner;
    } Endeach;

    return NULL;
}

void update_geometry(Database *owner, int width, int height) {
    Lerp_Interp *interp = glib->interp;
    Private_Window_Blob *blob = NULL;
    Schema *space = get_window_namespace();

    owner->assign_named_slot(interp, glib->width_atom, interp->parser->make_integer(width));
    owner->assign_named_slot(interp, glib->height_atom, interp->parser->make_integer(height));
}

void make_buffered_event(int keycode) {
    if (keycode < 0) return;
    if (keycode > NUM_KEY_CODES) return;
    String *name = glib->key_names[keycode];
    if (name == NULL) return;

    Database *db = glib->buffer_for_keyboard;
    db->add_assertion(glib->interp, glib->text_input_string, name); // XXX @Robustness: Want to handle non-English characters
}

LRESULT CALLBACK WindowProc(HWND    hwnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam) {
  	switch (message) {
    case WM_CREATE:{
          HDC dc = GetDC(hwnd);
          set_pixel_format(dc);
          HGLRC glrc = wglCreateContext(dc);
          assert(glrc != NULL);

          // Put these in temporaries so we can read them back when we get back into
          // the function that created the window.  Callbacks suck.
          glib->dc_tmp = dc;
          glib->glrc_tmp = glrc;

          break;
    }
    case WM_DESTROY: {
        Private_Window_Blob *blob = NULL;
        find_window_record(hwnd, &blob);
        
        if (blob) {
            wglMakeCurrent(blob->dc, NULL);
            wglDeleteContext(blob->glrc);

            if (blob->palette != NULL) DeleteObject(blob->palette);
            ReleaseDC(hwnd, blob->dc);

            // XXX PostQuitMessage(0);
        }

        break;
    }
	case WM_ACTIVATEAPP: {
        Private_Window_Blob *blob = NULL;
        First_Class *owner = find_window_record(hwnd, &blob);
        assert(owner);
        if (owner) {
            glib->update_window_focus(owner, wParam ? true : false);
        }
		break;
    }
	case WM_SIZE: {
		int width, height;

		width = LOWORD(lParam);
		height = HIWORD(lParam);

        Private_Window_Blob *blob = NULL;
        Database *owner = find_window_record(hwnd, &blob);
        
        if (blob) {
            if (blob->glrc == glib->current_opengl_glrc) {
                glViewport(0, 0, width, height);
            }

            update_geometry(owner, width, height);
        }

		break;
	}

	case WM_KEYDOWN: {
        // @Refactor: Keydown and Keyup were written to use an array, as
        // below, but later on the buffered system was created for WM_CHAR
        // so maybe these can be rewritten to use that (and we can ditch
        // the arrays)?  It all depends on what functionality we settle
        // on wanting from the arrays...
		int ascii_code;
		ascii_code = translate_ascii_code(wParam);
        if (ascii_code) glib->key_is_down[ascii_code] = true;
		break;
	}
	case WM_KEYUP: {
		int ascii_code;
		ascii_code = translate_ascii_code(wParam);
        if (ascii_code) glib->key_is_down[ascii_code] = false;
		break;
	}

    case WM_CHAR: {
        int keycode = wParam;
        if (wParam > 31) {
            make_buffered_event(wParam);
        }
        break;
    }
	case WM_PAINT: 
		ValidateRect(hwnd,NULL);
		break;

	case WM_QUERYNEWPALETTE: {
        Private_Window_Blob *blob = NULL;
        First_Class *owner = find_window_record(hwnd, &blob);
        
        if (blob && blob->palette) {
            int nRet;

            SelectPalette(blob->dc, blob->palette, FALSE);
            nRet = RealizePalette(blob->dc);
            InvalidateRect(hwnd, NULL, FALSE);
			
            return nRet;
        }

        break;
    }
	
	case WM_PALETTECHANGED: {
        Private_Window_Blob *blob = NULL;
        First_Class *owner = find_window_record(hwnd, &blob);
        
        if (blob && blob->palette && ((HWND)wParam != hwnd)) {
			SelectPalette(blob->dc, blob->palette, FALSE);
			RealizePalette(blob->dc);
			UpdateColors(blob->dc);
			return 0;
		}

		break;
    }
    case WM_LBUTTONDOWN:
//        app_shell->mouse_button_is_down = true;
        break;
    case WM_LBUTTONUP:
//        app_shell->mouse_button_is_down = false;
        break;
	default:
	    return DefWindowProc(hwnd, message, wParam, lParam);
	}

    return 0;
}

void init_window_class() {
	WNDCLASS *wc = &glib->window_class;

	wc->style                = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc->lpfnWndProc          = (WNDPROC)WindowProc;
	wc->cbClsExtra           = 0;
	wc->cbWndExtra           = 0;
	wc->hInstance            = (HINSTANCE)0;   // @Robustness: If we want to run on win98 we need the app hinstance here...
	wc->hIcon                = LoadIcon(NULL, "APPICON");
	wc->hCursor              = LoadCursor(NULL, IDC_ARROW);
	wc->hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	
	wc->lpszMenuName         = NULL; 
	wc->lpszClassName        = WINDOW_CLASS_NAME;

	// Register the window class
	if (RegisterClass(wc) == 0) return;

    glib->window_class_initted = true;
}

void ensure_gl_is_loaded() {
    // @Robustness: not a static global here!
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        HMODULE gl = LoadLibrary("opengl32.dll");
        assert(gl);
    }
}






// Wacky structures used in the font file format.

struct Glx_Glyph_Data {
    int a, b;
    int advance;
    float left, top, right, bottom;
};

struct Glx_Tex_Font_Data {
    int num_glyphs;
    int bmp_height, bmp_width;
    int character_height;
    Glx_Glyph_Data glyphs[94];
};


static bool load_character_map(Lerp_Interp *interp, Database *font, char *name) {
    Concatenator c;
    c.add(name);
    c.add(".font_map");
    char *filename = c.get_result();

    FILE *f = fopen(filename, "rb");
    if (!f) return false;

    Glx_Tex_Font_Data data;

    unsigned int low = fgetc(f);
    unsigned int high = fgetc(f);

    int num_chars_to_eat = (high << 8) | low;
    int i;
    for (i = 0; i < num_chars_to_eat; i++) {
        fgetc(f);
    }

    int len = fread(&data, sizeof(data), 1, f);
    delete [] filename;
    fclose(f);

    if (len != 1) return false;


    int character_range_low = 32;
    int num_characters = 94;
    int character_height = data.character_height;

    FILL_SLOT(font, character_height);

    String *char_string = glib->char_string;

    for (i = 0; i < num_characters; i++) {
        Glx_Glyph_Data *glyph = &data.glyphs[i];

        Database *info = interp->instantiate(glib->Font_Character_Info_namespace);

        double a = glyph->a;
        double b = glyph->b;
        double advance = glyph->advance;
        double u0 = glyph->left;
        double u1 = glyph->right;
        double v0 = 1.0 - glyph->bottom;
        double v1 = 1.0 - glyph->top;

        FILL_SLOT_F(info, a);
        FILL_SLOT_F(info, b);
        FILL_SLOT_F(info, advance);
        FILL_SLOT_F(info, u0);
        FILL_SLOT_F(info, u1);
        FILL_SLOT_F(info, v0);
        FILL_SLOT_F(info, v1);

        font->add_assertion(interp, char_string, interp->parser->make_integer(i + character_range_low), info);
    }

    return true;
}



unsigned char *do_dumb_alpha_thing(unsigned char *src, int width, int height) {
    int npixels = width * height;
    unsigned char *dest = (unsigned char *)malloc(npixels * 4);

    int i;
    for (i = 0; i < npixels; i++) {
        int r = src[i * 3 + 0];
        int g = src[i * 3 + 1];
        int b = src[i * 3 + 2];

        int sum = r + g + b;
        int a;
        if (sum) {
            a = 255;
        } else {
            a = 0;
        }

        dest[i * 4 + 0] = b;
        dest[i * 4 + 1] = g;
        dest[i * 4 + 2] = r;
        dest[i * 4 + 3] = a;
    }

    free(src);
    return dest;
}

unsigned char *do_dumb_alpha_thing_for_font(unsigned char *src, int width, int height) {
    int npixels = width * height;
    unsigned char *dest = (unsigned char *)malloc(npixels * 4);

    int i;
    for (i = 0; i < npixels; i++) {
        int r = src[i * 3 + 0];
        int g = src[i * 3 + 1];
        int b = src[i * 3 + 2];

        int a = r;
        r = 255;
        g = 255;
        b = 255;

        dest[i * 4 + 0] = r;
        dest[i * 4 + 1] = g;
        dest[i * 4 + 2] = b;
        dest[i * 4 + 3] = a;
    }

    free(src);
    return dest;
}

GLuint gl_texture_from_bitmap(char *bits, int width, int height) {
    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_BGRA_EXT,
                      GL_UNSIGNED_BYTE, (void *)bits);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    return texture_id;
}

bool texture_from_file(char *filename, GLuint *result,
                       int *width_result, int *height_result, bool for_font) {
    unsigned char *bitmap;
    int width, height;
    bool success = load_jpeg_file(filename, &bitmap, &width, &height);
    if (success == false) return false;

    if (for_font) {
        bitmap = do_dumb_alpha_thing_for_font(bitmap, width, height);
    } else {
        bitmap = do_dumb_alpha_thing(bitmap, width, height);
    }

    GLuint texture = gl_texture_from_bitmap((char *)bitmap, width, height);

    // Give the appropriate data values back to the caller.

    printf("Texture '%s' loaded; handle %d, width %d, height %d\n",
           filename, texture, width, height);

    *result = texture;
    *width_result = width;
    *height_result = height;

	// XXX
    // By the way... since we aren't remembering the pointer to 'bitmap'
    // anywhere, we can never deallocate it, so it gets leaked.  Not a big
    // deal in this demo, but for real software, you want to change that.

    return true;
}



void proc_load_font(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 1)) return;
    char *name;
    bool success = get_string(interp, record, 0, &name);
    if (!success) {
        interp->report_error("Argument must be a string.\n");
        return;
    }

    Concatenator c;
    c.add(name);
    c.add(".jpg");
    char *texture_name = c.get_result();

    int width, height;
    GLuint texture_handle;
    success = texture_from_file(texture_name, &texture_handle, &width, &height, true);
    delete [] texture_name;

    if (!success) {
        // @Robustness @UI: Report a better error here.... but only if the user wants it (they might intentionally try to load fonts that might not exist!)
        Return(interp->uninitialized);
    }
    
    printf("TEXTURE HANDLE IS %d; width %d, height %d\n", texture_handle, width, height);

    Database *result = interp->instantiate(glib->Font_namespace);
    FILL_SLOT(result, width);
    FILL_SLOT(result, height);
    FILL_SLOT(result, texture_handle);
    
    success = load_character_map(interp, result, name);
    if (!success) {
        // @Robustness @UI: Report a better error here.... but only if the user wants it (they might intentionally try to load fonts that might not exist!)
        Return(interp->uninitialized);
    }

    Return(result);
}

void proc_create_window(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 0)) return;

    ensure_gl_is_loaded();

    if (!glib->window_class_initted) {
        init_window_class();
    }

    if (!glib->window_class_initted) {
        interp->bytecode_runner->write_register(0, interp->parser->make_integer(0));
        return;
    }

	HWND hwnd;
    hwnd = CreateWindow(WINDOW_CLASS_NAME,
                        APP_NAME,
                        WS_OVERLAPPEDWINDOW,
                        0, 0,
                        WINDOW_DEFAULT_WIDTH,
                        WINDOW_DEFAULT_HEIGHT,
                        NULL,
                        NULL,
                        (HINSTANCE)0, // @Robustness: If we want to run on win98 we need the app hinstance here...
                        NULL);

    assert(glib->dc_tmp != 0);

    // Display the window
    UpdateWindow(hwnd);

	
    Schema *space = get_window_namespace();
    if (!space) return;

    Database *result = interp->instantiate(space);
    
    Private_Window_Blob *blob = new Private_Window_Blob();  // XXXXXXX @Memory Unmanaged memory
    blob->hwnd = hwnd;
    blob->dc = glib->dc_tmp;
    blob->glrc = glib->glrc_tmp;
    blob->palette = 0;

    result->assign_named_slot(interp, glib->resources_atom, blob);

    glib->list_of_windows.add(result);


    Decl_Expression *expression = interp->memory_manager->create_decl_expression(2);
    expression->initialize_slot(0, glib->window_string);
    expression->initialize_slot(1, result);

    glib->Windows->add_assertion(interp, expression);


    Return(result);


    // Do some GL stuff that we may actually not want to do in the long run.
    wglMakeCurrent(blob->dc, blob->glrc);
    glib->current_opengl_dc = blob->dc;
    glib->current_opengl_glrc = blob->glrc;
    glViewport(0, 0, WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
}



HWND get_window_pointer(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (record->this_pointer == NULL) {
        interp->report_error("Attempt to invoke a procedure without an appropriate 'this' pointer.\n");
        return 0;
    }

    Private_Window_Blob *blob = get_blob(interp, record->this_pointer->read());
    assert(blob != NULL);

    return blob->hwnd;
}

void proc_window_update(Lerp_Interp *interp, Lerp_Call_Record *record) {
    HWND hwnd = get_window_pointer(interp, record);
    if (!hwnd) return;

    if (!argument_count(interp, record, 0)) return;

    int width = 1;
    int height = 1;
    int x = 0;
    int y = 0;

    get_integer_member(interp, record, glib->x_atom, &x);
    get_integer_member(interp, record, glib->y_atom, &y);
    get_integer_member(interp, record, glib->width_atom, &width);
    get_integer_member(interp, record, glib->height_atom, &height);

    SetWindowPos(hwnd, HWND_TOP, x, y, width, height, 0);


    // Now do the title...
    char *title = "Window Title";
    get_string_member(interp, record, interp->parser->make_atom("title"), &title);

    SetWindowText(hwnd, title);
}

void proc_mouse_pointer_update(Lerp_Interp *interp, Lerp_Call_Record *record);

void proc_window_show(Lerp_Interp *interp, Lerp_Call_Record *record) {
    HWND hwnd = get_window_pointer(interp, record);
    if (!hwnd) return;

    if (!argument_count(interp, record, 1)) return;

    int should_show;
    bool success = coerce_to_integer(interp, record, 0, &should_show);

    if (should_show) {
        ShowWindow(hwnd, SW_SHOW);
    } else {
        ShowWindow(hwnd, SW_HIDE);
    }

    UpdateWindow(hwnd);
}






void proc_load_texture(Lerp_Interp *interp, Lerp_Call_Record *record) {

    ensure_gl_is_loaded();

    if (!argument_count(interp, record, 1)) return;
    char *name;
    bool success = get_string(interp, record, 0, &name);
    if (!success) {
        interp->report_error("Argument must be a string.\n");
        return;
    }
    
    int width, height;
    GLuint result;
    success = texture_from_file(name, &result, &width, &height, false);
    First_Class *return_value;
    if (success) {
        return_value = interp->parser->make_integer(result);
    } else {
        return_value = interp->uninitialized;
    }

    Return(return_value);
    //
    // @Memory: We just leak all the texture data here, etc!
    //
}


Decl_Assertion *make_singleton_assertion(Lerp_Interp *interp, Decl_Expression *expression) {
    Decl_Assertion *assertion = GC_NEW(Decl_Assertion);
    assertion->conditionals = NULL;
    assertion->expression = ToBarrier(expression);
    return assertion;
}

Database *make_singleton_database(Lerp_Interp *interp, Decl_Expression *expression) {
    Database *db = GC_NEW(Database);
    db->add_assertion(interp, expression);
    return db;
}

/*
void proc_get_cursor_position(Lerp_Interp *interp, Lerp_Call_Record *record) {
    if (!argument_count(interp, record, 0)) return;

    POINT cursor_point;
    BOOL success = GetCursorPos(&cursor_point);
    if (!success) {
        interp->report_error("Weird error whereby we couldn't read the cursor position.\n");
        return;
    }

    Decl_Expression *expression = interp->memory_manager->create_decl_expression(2);
    expression->initialize_slot(0, interp->parser->make_integer(cursor_point.x));
    expression->initialize_slot(1, interp->parser->make_integer(cursor_point.y));

    // @Memory: Later we can just return an assertion instead of a database...
    // save some memory!
    Database *result = make_singleton_database(interp, expression);

    Return(result);
}
*/
    

Window_Lib::Window_Lib(Lerp_Interp *_interp) {
    interp = _interp;
    String *unknown_string = interp->parser->make_string("*unknown*");
    int i;
    for (i = 0; i < NUM_KEY_CODES; i++) {
        key_is_down[i] = false;
        key_was_down[i] = false;
        char name[2];

        if (isprint(i)) {
            name[0] = i;
            name[1] = '\0';
            key_names[i] = interp->parser->make_string(name);
        } else if ((i == 13) || (i == 10)) {
            key_names[i] = interp->parser->make_string("enter");
        } else if (i == 8) {
            key_names[i] = interp->parser->make_string("backspace");
        } else {
            key_names[i] = unknown_string;
        }
    }

    DEFINE_STRING(key_held_down);
    DEFINE_STRING(key_pressed);
    DEFINE_STRING(key_released);
    DEFINE_STRING(text_input);
    DEFINE_STRING(window);
    DEFINE_STRING(char);

    DEFINE_ATOM(focus_window);
}

void Window_Lib::update_keyboard() {
    // First we plop the assertions from the buffer over onto the current
    // Keyboard (this performs two functions: erase all the current assertions,
    // and move over the stuff that's been waiting for us.)   Then we erase
    // the buffer...

    // @WriteBarrier
    glib->Keyboard->assertions = glib->buffer_for_keyboard->assertions;
    glib->buffer_for_keyboard->clear();

    Database *db = glib->Keyboard;

    int i;
    for (i = 0; i < NUM_KEY_CODES; i++) {
        if (key_is_down[i]) {
            db->add_assertion(interp, glib->key_held_down_string, key_names[i]);
            if (!key_was_down[i]) db->add_assertion(interp, glib->key_pressed_string, key_names[i]);
        } else {
            if (key_was_down[i]) db->add_assertion(interp, glib->key_released_string, key_names[i]);
        }

        key_was_down[i] = key_is_down[i];
    }
}


Database *install_module(Lerp_Interp *interp, Schema *schema, char *name) {
    Database *result = interp->instantiate(schema);
    interp->global_database->assign_named_slot(interp, interp->parser->make_atom(name), result);
    return result;
}


void proc_do_window_events(Lerp_Interp *interp, Lerp_Call_Record *record) {
    interp->process_os_events();

    extern Window_Lib *glib;

    // Update the mouse pointer info...
    POINT cursor_point;
    BOOL success = GetCursorPos(&cursor_point);
    Schema *ns = glib->Mouse_Pointer->schema->read();

    Database *mouse = glib->Mouse_Pointer;
    if (success && ns) {
        First_Class *x_fc = mouse->lookup_named_slot(interp, glib->x_atom);
        First_Class *y_fc = mouse->lookup_named_slot(interp, glib->y_atom);
        
        if (x_fc && y_fc && (x_fc->type == ARG_INTEGER) && (y_fc->type == ARG_INTEGER)) {
            Integer *x = (Integer *)x_fc;
            Integer *y = (Integer *)y_fc;

            // @WriteBarrier @GC?
            if (cursor_point.x != x->value) mouse->assign_named_slot(interp, glib->x_atom, ToBarrierF(interp->parser->make_integer(cursor_point.x)));
            if (cursor_point.y != y->value) mouse->assign_named_slot(interp, glib->y_atom, ToBarrierF(interp->parser->make_integer(cursor_point.y)));
        }
    }
}

void proc_mouse_pointer_update(Lerp_Interp *interp, Lerp_Call_Record *record) {
    Schema *ns = glib->Mouse_Pointer->schema->read();
    if (!ns) return;

    First_Class *x_fc = glib->Mouse_Pointer->lookup_named_slot(interp, glib->x_atom);
    First_Class *y_fc = glib->Mouse_Pointer->lookup_named_slot(interp, glib->y_atom);
    if (!x_fc) return;
    if (!y_fc) return;
    if (x_fc->type != ARG_INTEGER) return;
    if (y_fc->type != ARG_INTEGER) return;

    int x = ((Integer *)x_fc)->value;
    int y = ((Integer *)y_fc)->value;

/*
    POINT cursor_point;
    cursor_point.x = x;
    cursor_point.y = y;
*/
    SetCursorPos(x, y);


    First_Class *visible_fc = glib->Mouse_Pointer->lookup_named_slot(interp, glib->visible_atom);
    if (!visible_fc) return;
    if (visible_fc->type != ARG_INTEGER) return;

    Integer *visible = (Integer *)visible_fc;
    
    int cursor_visible = ShowCursor(FALSE);

    // Annoying win32 state-holding ShowCursor.... aaaaargh
    if (visible->value) {
        if (cursor_visible < 1) {
            ShowCursor(TRUE);
            ShowCursor(TRUE);
        }
    } else {
        if (cursor_visible < -1) ShowCursor(TRUE);
    }

    glib->update_keyboard();
}






void get_module_info(Lerp_Module_Init_Info *info) {
    info->system_version = LERP_SYSTEM_VERSION;
    info->default_name = "GL_and_Windows";
};

Database *instantiate(Lerp_Interp *interp) {
    Database *db = GC_NEW(Database);

    {
        //
        // Declare type "Window"
        //

        Schema *ns = make_schema(interp, 8);
        int schema_cursor = 0;

        MEMBER(x, integer, 0);
        MEMBER(y, integer, 0);
        MEMBER(width, integer, 0);
        MEMBER(height, integer, 0);
        MEMBER(title, string, "Window Title");
        ns->type_name = glib->Window_Type_atom;

        MEMBER_PROC2(update, window_update);
        MEMBER_PROC2(show, window_show);
        MEMBER_BLOB_NULL(resources);
        bind_value(interp, db, "Window", ns);

        glib->Window_namespace = ns;
    }

    {
        //
        // Declare type "Mouse_Pointer"
        //

        Schema *ns = make_schema(interp, 4);
        int schema_cursor = 0;

        MEMBER(x, integer, 0);
        MEMBER(y, integer, 0);
        MEMBER(visible, integer, 1);

        MEMBER_PROC2(update, mouse_pointer_update);
        //bind_value(interp, "Mouse_Pointer", ns);
        ns->type_name = interp->parser->make_atom("Mouse_Pointer_Type");
        glib->Mouse_Pointer = interp->instantiate(ns);
        bind_value(interp, db, "Mouse_Pointer", glib->Mouse_Pointer);
    }

    {
        //
        // Declare type "Font"
        //

        Schema *ns = make_schema(interp, 20);
        int schema_cursor = 0;

        MEMBER(character_height, integer, 0);
        MEMBER(texture_handle, integer, 0);
        MEMBER(name, string, "Font Name");
        ns->type_name = glib->font_type_atom;

        // In addition there will be a bunch of entries in the database,
        // one for each font character, giving the character info...
        // there is no way to represent that yet in the Schema.  @Incomplete
        bind_value(interp, db, "Font", ns);

        trim_schema(interp, ns, schema_cursor);
        glib->Font_namespace = ns;
    }

    {
        //
        // Declare type "Font_Character_Info"
        //

        Schema *ns = make_schema(interp, 20);
        int schema_cursor = 0;

        MEMBER(u0, float, 0.0);
        MEMBER(v0, float, 0.0);
        MEMBER(u1, float, 0.0);
        MEMBER(v1, float, 0.0);
        MEMBER(a, float, 0.0);
        MEMBER(b, float, 0.0);
        MEMBER(advance, float, 0.0);
        ns->type_name = glib->font_character_info_type_atom;

        // In addition there will be a bunch of entries in the database,
        // one for each font character, giving the character info...
        // there is no way to represent that yet in the Schema.  @Incomplete
        bind_value(interp, db, "Font_Character_Info", ns);

        trim_schema(interp, ns, schema_cursor);
        glib->Font_Character_Info_namespace = ns;
    }

    
    glib->buffer_for_keyboard = GC_NEW(Database);

    glib->Keyboard = GC_NEW(Database);
    bind_value(interp, db, "Keyboard", glib->Keyboard);

    {
        Schema *ns = make_schema(interp, 15);
        int schema_cursor = 0;

        MEMBER_UNINIT(focus_window);
        MEMBER_PROC(create_window);
        MEMBER_PROC(load_texture);
        MEMBER_PROC(do_window_events);
        MEMBER_PROC(load_font);

        trim_schema(interp, ns, schema_cursor);
        ns->type_name = interp->parser->make_atom("Windows_Type");

        glib->Windows = install_module(interp, ns, "Windows");
    }

    {
        Database *register_opengl_primitives(Lerp_Interp *interp);
        Database *gl = register_opengl_primitives(interp);
        bind_value(interp, db, "GL", gl);
    }
        
    return db;
}

void init(Lerp_Interp *interp) {

/*
    void lerp_window_os_init();
    lerp_window_os_init();
*/

    glib = new Window_Lib(interp);
    glib->width_atom = interp->parser->make_atom("width");
    glib->height_atom = interp->parser->make_atom("height");
    glib->resources_atom = interp->parser->make_atom("resources");
    glib->x_atom = interp->parser->make_atom("x");
    glib->y_atom = interp->parser->make_atom("y");
    glib->visible_atom = interp->parser->make_atom("visible");
    glib->Window_atom = interp->parser->make_atom("Window");
    glib->Windows_atom = interp->parser->make_atom("Windows");
    glib->window_class_initted = false;
    glib->Window_Type_atom = interp->parser->make_atom("Window_Type");
    glib->font_atom = interp->parser->make_atom("Font");
    glib->font_type_atom = interp->parser->make_atom("Font_Type");
    glib->font_character_info_atom = interp->parser->make_atom("Font_Character_Info");
    glib->font_character_info_type_atom = interp->parser->make_atom("Font_Character_Info_Type");

}

void shutdown(Lerp_Interp *) {
}

void enumerate_gc_roots(Lerp_Interp *interp) {
    BEGIN_TRACING(interp);

    TRACE2(Window_namespace, glib);
    TRACE2(Mouse_Pointer, glib);
    TRACE2(Keyboard, glib);
    TRACE2(Windows, glib);
    TRACE2(window_string, glib);
    TRACE2(key_held_down_string, glib);
    TRACE2(key_pressed_string, glib);
    TRACE2(key_released_string, glib);
    TRACE2(text_input_string, glib);
    TRACE2(char_string, glib);

    TRACE2(buffer_for_keyboard, glib);
    TRACE2(Font_namespace, glib);
    TRACE2(Font_Character_Info_namespace, glib);

    int i;
    for (i = 0; i < NUM_KEY_CODES; i++) {
//        TRACE(glib->key_names[i]);
        String *s = glib->key_names[i];
        if (s) glib->key_names[i] = (String *)_mm->gc_consider(s);
    }

    First_Class *win;
    Foreach(&glib->list_of_windows, win) {
        First_Class *result = _mm->gc_consider(win);
        __n->data = (void *)result;  // @WriteBarrier
    } Endeach;
}

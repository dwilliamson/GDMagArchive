struct Schema;

const int NUM_KEY_CODES = 256;

/// XXXXXXXXXXXX global_list_of_windows is not gc-safe!!
struct Window_Lib {
    Window_Lib(Lerp_Interp *interp);

    void update_keyboard();
    Lerp_Interp *interp;
    List list_of_windows;  // XXXXXX We want a better way of keeping track of this stuff....

    Atom *width_atom;
    Atom *height_atom;
    Atom *resources_atom;
    Atom *x_atom;
    Atom *y_atom;
    Atom *Window_atom;
    Atom *Windows_atom;
    Atom *Window_Type_atom;
    Atom *visible_atom;
    Atom *font_atom;
    Atom *font_type_atom;
    Atom *font_character_info_atom;
    Atom *font_character_info_type_atom;

    Schema *Window_namespace;
    Schema *Font_namespace;
    Schema *Font_Character_Info_namespace;

    Database *Mouse_Pointer;
    Database *Keyboard;
    Database *Windows;

    Database *buffer_for_keyboard;

    HDC dc_tmp;
    HGLRC glrc_tmp;

    HDC current_opengl_dc;
    HGLRC current_opengl_glrc;

    WNDCLASS window_class;

    Atom *focus_window_atom;

    String *window_string;
    String *key_held_down_string;
    String *key_pressed_string;
    String *key_released_string;
    String *text_input_string;
    String *char_string;

    String *key_names[NUM_KEY_CODES];
    bool key_is_down[NUM_KEY_CODES];
    bool key_was_down[NUM_KEY_CODES];

    bool window_class_initted;

//  protected:
    void update_window_focus(First_Class *window, bool has_focus);
  protected:
};

struct Private_Window_Blob : public Blob {
    Private_Window_Blob();
    HWND hwnd;
    HDC dc;
    HGLRC glrc;
    HPALETTE palette;
};

bool is_window_pointer(Lerp_Interp *interp, First_Class *fc);
void register_window_primitives(Lerp_Interp *interp);


extern Window_Lib *glib;  // A global... eventually this will be in a DLL so it's okay.


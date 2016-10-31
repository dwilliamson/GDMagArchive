#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct Font_Character_Info;
struct List;

const int DEFAULT_WIDTH = 1024;
const int DEFAULT_HEIGHT = 768;

struct Loaded_Texture_Info {
    int texture_handle;
    int width, height;
    bool loaded_successfully;
};


struct Font {
    int character_range_low;
    int character_range_high;

    int num_characters;
    int texture_handle;
    int texture_width, texture_height;

    int character_height;

    Font_Character_Info *character_info;

    float get_string_width_in_pixels(char *s);
};

struct App_Shell {
	bool init(int width, int height);
	
	int screen_width;
	int screen_height;
	void *hwnd;

	int mouse_pointer_x, mouse_pointer_y;
	int mouse_pointer_delta_x, mouse_pointer_delta_y;

	double get_time();
    double get_dt();

	void draw_text(Font *font, float x, float y, char *text, float red = 1, float green = 1, float blue = 1, float alpha = 1, float max_width = 0);
	void draw_texture_quad(int texture_handle, float x, float y, 
						   float width, float height);

	enum Draw_Mode {
        MODE_NONE, MODE_TEXT, MODE_LINE, MODE_TRIANGLE, MODE_BITMAP
    };

	void text_mode_begin(Font *font);
	void text_mode_end();

	void line_mode_begin();
	void line_mode_end();

	void triangle_mode_begin();
	void triangle_mode_end();

	void bitmap_mode_begin(int texture_handle);
	void bitmap_mode_end();

	Loaded_Texture_Info font_texture;
	void load_texture(Loaded_Texture_Info *info, char *name, bool for_font = false);
    Font *load_font(char *name);

    int find_or_load_texture(char *name, char *prefix);

    void sleep(float seconds);

    // Key codes
    enum {
        ARROW_UP = 1, ARROW_DOWN = 2, ARROW_LEFT = 3, ARROW_RIGHT = 4
    };

    Draw_Mode draw_mode;
    float dt;
    double last_frame_time;

    List *loaded_textures;

	bool mouse_button_1_is_down;
	bool mouse_button_2_is_down;
    bool show_mouse_pointer;
    bool center_mouse_pointer;

  protected:
	bool init_gl();
	void init_textures();
	void init_modelview_transform();

    bool load_character_map(Font *result, char *name);
};

const double M_PI = 3.14159265358979323846;
const float F_PI = (float)M_PI;

extern App_Shell *app_shell;


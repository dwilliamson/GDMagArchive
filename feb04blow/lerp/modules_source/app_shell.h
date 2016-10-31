#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

const int DEFAULT_WIDTH = 800;
const int DEFAULT_HEIGHT = 600;

struct Vector3 {
	Vector3(float x, float y, float z);
	Vector3();
	
	float x, y, z;

	float length_squared();
	float length();
	void normalize();
	void set(float x, float y, float z);
	Vector3 scale(float factor);
	Vector3 subtract(const Vector3 &other) const;
	Vector3 add(const Vector3 &other) const;
};

struct Loaded_Texture_Info {
    int texture_handle;
    int width, height;
    bool loaded_successfully;
};

struct Font_Character_Info;

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
	bool mouse_button_is_down;

	double get_time();

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

    char *strdup(char *string);

    void sleep(float seconds);

    // Key codes
    enum {
        ARROW_UP = 1, ARROW_DOWN = 2, ARROW_LEFT = 3, ARROW_RIGHT = 4
    };

    Draw_Mode draw_mode;

  protected:
	bool init_gl();
	void init_textures();
	void init_modelview_transform();

    bool load_character_map(Font *result, char *name);
};

const double M_PI = 3.14159265358979323846;
const float F_PI = (float)M_PI;

inline float Vector3::length_squared() {
	return x*x + y*y + z*z;
}

inline Vector3::Vector3() {
}

inline Vector3::Vector3(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
}

inline void Vector3::set(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
}

inline Vector3 Vector3::add(const Vector3 &other) const {
	Vector3 result;
	result.x = x + other.x;
	result.y = y + other.y;
	result.z = z + other.z;

	return result;
}

inline Vector3 Vector3::subtract(const Vector3 &other) const {
	Vector3 result;
	result.x = x - other.x;
	result.y = y - other.y;
	result.z = z - other.z;

	return result;
}

inline float dot_product(const Vector3 &a, const Vector3 &b) {
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline Vector3 Vector3::scale(float factor) {
    return Vector3(x * factor, y * factor, z * factor);
}

Vector3 cross_product(Vector3 a, Vector3 b);

extern App_Shell *app_shell;



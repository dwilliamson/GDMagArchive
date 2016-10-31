#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

const int DEFAULT_WIDTH = 800;
const int DEFAULT_HEIGHT = 600;

// XXX check value of PI, or something
const double M_PI = 3.141592653579;
const float F_PI = (float)M_PI;

struct Vector3 {
	Vector3(float x, float y, float z);
	Vector3();
	
	float x, y, z;

	float length_squared();
	float length();
	void normalize();
	void set(float x, float y, float z);
	void scale(float factor);
	Vector3 subtract(const Vector3 &other) const;
	Vector3 add(const Vector3 &other) const;
};

struct Loaded_Texture_Info {
    int texture_handle;
    int width, height;
    bool loaded_successfully;
};

struct App_Shell {
	bool init(int width, int height);
	
	int screen_width;
	int screen_height;
	void *hwnd;

	int mouse_pointer_x, mouse_pointer_y;
	
	double get_time();
	
	void draw_text_line(int *x, int *y, char *text);
	void draw_texture_quad(int texture_handle, float x, float y, 
						   float width, float height);
	
	void text_mode_begin();
	void text_mode_end();

	void line_mode_begin();
	void line_mode_end();

	void bitmap_mode_begin(int texture_handle);
	void bitmap_mode_end();

	Loaded_Texture_Info font_texture;

	void init_modelview_transform();

	void load_texture(Loaded_Texture_Info *info, char *name);
  protected:
	bool init_gl();
	void init_textures();
};


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

inline void Vector3::scale(float factor) {
	x *= factor;
	y *= factor;
	z *= factor;
}

inline void Vector3::normalize() {
	float ilen = 1.0 / sqrt(length_squared());
	scale(ilen);
}

inline float distance_squared(const Vector3 &v0, const Vector3 &v1) {
	float dx = v1.x - v0.x;
	float dy = v1.y - v0.y;
	float dz = v1.z - v0.z;
	return dx*dx + dy*dy + dz*dz;
}

extern App_Shell *app_shell;








#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"

#include <gl/glu.h>

#include "jpeg_load.h"
#include "app_shell.h"

#include "../profiler/profiler_lowlevel.h"
#include "glext.h"

Define_Zone(app_shell_main_loop);
Define_Zone(app_shell_window_proc);

App_Shell *app_shell;

// XXX I would like to ditch these globals at some point.
static HDC global_dc = 0;
static HGLRC global_gl_rc;
static HPALETTE global_palette = NULL;
static HINSTANCE global_hinstance;
static LARGE_INTEGER global_base_time; // XXX
bool app_is_active = false;


struct Font_Character_Info {
    float u0, v0, u1, v1;
    float a, b, advance;
};

LRESULT CALLBACK WindowProc(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam);

static LARGE_INTEGER get_time_reading() {
    LARGE_INTEGER freq;
    LARGE_INTEGER time;

    BOOL ok = QueryPerformanceFrequency(&freq);
    assert(ok == TRUE);

    freq.QuadPart = freq.QuadPart / 1000;

    ok = QueryPerformanceCounter(&time);
    assert(ok == TRUE);

    time.QuadPart = time.QuadPart / freq.QuadPart;

	return time;
}

void add_letter_quad(Font *font, Font_Character_Info *info, float xs, float ys, float iw) {
    float u0 = info->u0;
    float u1 = info->u1;

    float npix = (u1 - u0) * iw;
    float w = npix;

    float h = font->character_height;

    glTexCoord2f(u0, info->v0);
    glVertex2f(xs, ys);

    glTexCoord2f(u1, info->v0);
    glVertex2f(xs+w, ys);

    glTexCoord2f(u1, info->v1);
    glVertex2f(xs+w, ys+h);

    glTexCoord2f(u0, info->v1);
    glVertex2f(xs, ys+h);
}

inline int remap_letter(Font *font, unsigned int letter) {
    if (letter < font->character_range_low) return -1;
    if (letter > font->character_range_high) return -1;
    letter -= font->character_range_low;
    return letter;
}

float Font::get_string_width_in_pixels(char *s) {
    float sum = 0;
    while (*s) {
        int letter = remap_letter(this, *s);
        if (letter != -1) {
            Font_Character_Info *info = &character_info[letter];
            sum += info->advance;
        }

        s++;
    }

    return sum;
}

void App_Shell::draw_text(Font *font, float xs, float ys, char *s, float cr, float cg, float cb, float ca) {
    int len = strlen(s);

    glBegin(GL_QUADS);

    xs -= 0.5f;
    glColor4f(cr, cg, cb, ca);

    float iw = (float)font->texture_width;

    int i;
    for (i = 0; i < len; i++) {
        int letter = remap_letter(font, s[i]);
        if (letter == -1) continue;

        Font_Character_Info *info = &font->character_info[letter];
        add_letter_quad(font, info, xs, ys, iw);

		xs += info->advance;
    }

    glEnd();
}

void App_Shell::draw_texture_quad(int texture_handle, float x, float y, 
								  float width, float height) {

    bitmap_mode_begin(texture_handle);
    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f(1, 0);
    glVertex2f(x + width, y);

    glTexCoord2f(1, 1);
    glVertex2f(x + width, y + height);

    glTexCoord2f(0, 1);
    glVertex2f(x, y + height);

    glEnd();
    bitmap_mode_end();
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

    *result = texture;
    *width_result = width;
    *height_result = height;

	// XXX
    // By the way... since we aren't remembering the pointer to 'bitmap'
    // anywhere, we can never deallocate it, so it gets leaked.  Not a big
    // deal in this demo, but for real software, you want to change that.

    return true;
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

char *append(char *name, char *extension) {
    int len_total = strlen(name) + strlen(extension);

    char *filename = new char[len_total + 1];
    strcpy(filename, name);
    strcat(filename, extension);

    return filename;
}

bool App_Shell::load_character_map(Font *font, char *name) {
    char *filename = append(name, ".font_map");

    FILE *f = fopen(filename, "rb");
    delete [] filename;
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
    fclose(f);

    if (len != 1) return false;

    int num_characters = 94;
    font->character_info = new Font_Character_Info[num_characters];
    font->character_range_low = 32;
    font->character_range_high = font->character_range_low + num_characters - 1;
    font->character_height = data.character_height;

    for (i = 0; i < num_characters; i++) {
        Font_Character_Info *info = &font->character_info[i];
        Glx_Glyph_Data *glyph = &data.glyphs[i];
        info->a = glyph->a;
        info->b = glyph->b;
        info->advance = glyph->advance;
        info->u0 = glyph->left;
        info->u1 = glyph->right;
        info->v0 = 1.0f - glyph->bottom;
        info->v1 = 1.0f - glyph->top;
    }

    return true;
}

Font *App_Shell::load_font(char *name) {
    Loaded_Texture_Info info;

    char *texture_name = append(name, ".jpg");
    load_texture(&info, texture_name, true);
    delete [] texture_name;
    if (!info.loaded_successfully) return NULL;

    Font *result = new Font();
    result->texture_handle = info.texture_handle;
    result->texture_width = info.width;
    result->texture_height = info.height;

    bool success = load_character_map(result, name);
    if (!success) {
        delete result;
        return NULL;
    }

    return result;
}

void App_Shell::load_texture(Loaded_Texture_Info *info, char *name, bool for_font) {
	GLuint handle;
    bool success = texture_from_file(name, &handle,
                                     &info->width, &info->height, for_font);
	info->texture_handle = handle;
    info->loaded_successfully = success;
}

void App_Shell::init_textures() {
}


void App_Shell::line_mode_begin() {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void App_Shell::line_mode_end() {
}

void App_Shell::triangle_mode_begin() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);
}

void App_Shell::triangle_mode_end() {
}

void App_Shell::bitmap_mode_begin(int texture_handle) {
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App_Shell::bitmap_mode_end() {
    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
}

void App_Shell::text_mode_begin(Font *font) {
    if (font == NULL) return;

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, font->texture_handle);

//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);
    GLfloat ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
}

void App_Shell::text_mode_end() {
    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
}

bool App_Shell::init(int width, int height) {
	screen_width = width;
	screen_height = height;

	mouse_pointer_x = 0;
	mouse_pointer_y = 0;

	// Init time.
	global_base_time = get_time_reading();

	// Init other stuff.
	bool success;

	success = init_gl();
	if (!success) return false;
	
	init_textures();
	init_modelview_transform();

	return true;
}

void App_Shell::init_modelview_transform() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	float pos_x = 0;
	float pos_y = 0;
	float pos_z = 5;
    gluLookAt(pos_x, pos_y, pos_z, 0, 0, 0, 0, 0, 1);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, (float)screen_width / (float)screen_height,
				   0.1f, 1000.0f);
}





///// OpenGL stuff here

const char *window_class_name = "AppShellWindowClass";
HWND get_hwnd(App_Shell *shell) {
	return *(HWND *)&shell->hwnd;
}

bool App_Shell::init_gl() {
	WNDCLASS wc;

	wc.style                = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc          = (WNDPROC)WindowProc;
	wc.cbClsExtra           = 0;
	wc.cbWndExtra           = 0;
	wc.hInstance            = global_hinstance;
	wc.hIcon                = LoadIcon(NULL, "APPICON");
	wc.hCursor              = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	
	wc.lpszMenuName         = NULL; 
	wc.lpszClassName        = window_class_name;

	// Register the window class
	if (RegisterClass(&wc) == 0) return false;

	extern char *app_name;
	
	HWND app_hwnd;
    app_hwnd = CreateWindow(window_class_name,
							app_name,
							WS_OVERLAPPEDWINDOW,
							0, 0,
							screen_width, screen_height,
							NULL,
							NULL,
							global_hinstance,
							NULL);

    assert(global_dc != 0);

	hwnd = (void *)app_hwnd;

    // Display the window
    ShowWindow(app_hwnd, SW_SHOW);
    UpdateWindow(app_hwnd);

	
    wglMakeCurrent(global_dc, global_gl_rc);
    glViewport(0, 0, screen_width, screen_height);

	return true;
}


double App_Shell::get_time() {
    LARGE_INTEGER time = get_time_reading();
	time.QuadPart = time.QuadPart - global_base_time.QuadPart;

	return (double)time.QuadPart * 0.001;
}

void App_Shell::sleep(float seconds) {
    Sleep(seconds * 1000);
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
	SetPixelFormat(dc, format, &pfd);
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

LRESULT CALLBACK WindowProc(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam) {
    Profile_Scope(app_shell_window_proc);

  	switch (message) {
      case WM_CREATE:
          global_dc = GetDC(hWnd);

          set_pixel_format(global_dc);

          // Make a rendering context.
          global_gl_rc = wglCreateContext(global_dc);
          assert(global_gl_rc != NULL);

          BOOL succ;
          succ = wglMakeCurrent(global_dc, global_gl_rc);
          assert(succ);

          break;

    case WM_DESTROY:
        wglMakeCurrent(global_dc, NULL);
        wglDeleteContext(global_gl_rc);

        if (global_palette != NULL) DeleteObject(global_palette);
        ReleaseDC(hWnd, global_dc);

		PostQuitMessage(0);
		break;

	case WM_ACTIVATEAPP:
        if (wParam) app_is_active = true;
        else app_is_active = false;
		break;
		
	case WM_SIZE: {
		int width, height;

		width = LOWORD(lParam);
		height = HIWORD(lParam);

		wglMakeCurrent(global_dc, global_gl_rc);
		glViewport(0, 0, width, height);
		app_shell->screen_width = width;
		app_shell->screen_height = height;

		break;
	}

	case WM_KEYDOWN: {
		extern void handle_keydown(int);
		int ascii_code;
		ascii_code = translate_ascii_code(wParam);
		handle_keydown(ascii_code);
		break;
	}
	case WM_KEYUP: {
		extern void handle_keyup(int);
		int ascii_code;
		ascii_code = translate_ascii_code(wParam);
		handle_keyup(ascii_code);
		break;
	}

	case WM_PAINT: 
		ValidateRect(hWnd,NULL);
		break;

	case WM_QUERYNEWPALETTE:
        if (global_palette) {
            int nRet;

            SelectPalette(global_dc, global_palette, FALSE);
            nRet = RealizePalette(global_dc);
            InvalidateRect(hWnd, NULL, FALSE);
			
            return nRet;
        }

        break;

	
	case WM_PALETTECHANGED:
		if ((global_palette != NULL) && ((HWND)wParam != hWnd)) {
			SelectPalette(global_dc, global_palette, FALSE);
			RealizePalette(global_dc);
			UpdateColors(global_dc);
			return 0;
		}

		break;

	default:
	    return DefWindowProc(hWnd, message, wParam, lParam);
	}

    return 0;
}

int AppPaint(HWND, HDC) {
    Profile_Scope(app_shell_main_loop);

    float time_0 = app_shell->get_time();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    extern void draw_scene();
    draw_scene();
    SwapBuffers(global_dc);

    float time_1 = app_shell->get_time();
    float dt = time_1 - time_0;

    // Don't do anything with dt right now; other times
    // we might want to clamp our update rate.
    // Sleep(20); // Let us not eat all the CPU under any circumstances!

    return TRUE;
}

int PASCAL WinMain(HINSTANCE hinst, HINSTANCE,
                   LPSTR pCommandLine, int ShowMode) {
    global_hinstance = hinst;

    // get the user's specified display parameters, or defaults
    // the shortened names work because of the default value
    
	app_shell = new App_Shell();
	bool success = app_shell->init(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	assert(success);

	extern void app_init();
	app_init();
	
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    MSG msg;
    for (;;) {
		HWND hwnd = get_hwnd(app_shell);
        assert(hwnd);

		// Do mouse pointer stuff.
		POINT point;
		BOOL success = GetCursorPos(&point);
        assert(success);

		if (success) {
			RECT client_rect;
			GetClientRect(hwnd, &client_rect);

            POINT upper_left;
            upper_left.x = 0;
            upper_left.y = 0;

            POINT lower_right;
            lower_right.x = client_rect.right;
            lower_right.y = client_rect.bottom;

            ClientToScreen(hwnd, &upper_left);
            ClientToScreen(hwnd, &lower_right);

            int cursor_x = point.x - upper_left.x;
            int cursor_y = lower_right.y - point.y;

			app_shell->mouse_pointer_x = cursor_x;
			app_shell->mouse_pointer_y = cursor_y;
		}

		// Do normal event kind of stuff.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            if (app_is_active) {
                AppPaint(hwnd, global_dc);
            } else {
                WaitMessage();
            }
        }
    }

    return msg.wParam;
}

void Vector3::normalize() {
	float ilen = 1.0 / sqrt(length_squared());
	x *= ilen;
	y *= ilen;
	z *= ilen;
}




double get_freq() {  // temporary
    LARGE_INTEGER freq;

    BOOL ok = QueryPerformanceFrequency(&freq);
    assert(ok == TRUE);

    return (double)freq.QuadPart;
}


Vector3 cross_product(Vector3 v1, Vector3 v2) {
    Vector3 n;

    n.x = v1.y * v2.z - v1.z * v2.y;
    n.y = v1.z * v2.x - v1.x * v2.z;
    n.z = v1.x * v2.y - v1.y * v2.x;

    return n;
}

float Vector3::length() {
    return sqrt(length_squared());
}

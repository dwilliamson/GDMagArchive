#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#include "os_specific_opengl_headers.h"

#include <gl/glu.h>

#include "jpeg_load.h"
#include "app_shell.h"

App_Shell *app_shell;

const int LETTER_WIDTH = 11;
const int LETTER_HEIGHT = 13;
const int LETTER_PAD = 2;

// XXX I would like to ditch these globals at some point.
static HDC global_dc = 0;
static HGLRC global_gl_rc;
static HPALETTE global_palette = NULL;
static HINSTANCE global_hinstance;
static LARGE_INTEGER global_base_time; // XXX
bool app_is_active = false;


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

void compute_xy(Vector3 *result, int ix, int iy, int screen_i, int screen_j) {
    float fx = ix;
    float fy = iy;

    float Z_PLANE = -1;
    *result = Vector3(fx, fy, Z_PLANE);
}

void add_letter_quad(char *s,
					 int i, int xs, int ys, 
					 int letter_width, int letter_height,
					 float cr, float cg, float cb,
					 int screen_i, int screen_j) {

    unsigned int c = s[i];
    unsigned int ci = (c % 16);
    unsigned int cj = (c / 16);

    int w = letter_width;
    int h = letter_height;

    int n0 = i * 4 + 0;
    int n1 = i * 4 + 1;
    int n2 = i * 4 + 2;
    int n3 = i * 4 + 3;

	Vector3 v0, v1, v2, v3;
    compute_xy(&v0, xs+0, ys+0, screen_i, screen_j);
    compute_xy(&v1, xs+w, ys+0, screen_i, screen_j);
    compute_xy(&v2, xs+w, ys+h, screen_i, screen_j);
    compute_xy(&v3, xs+0, ys+h, screen_i, screen_j);

    glTexCoord2f(ci, cj + 1);
    glVertex2f(v0.x, v0.y);

    glTexCoord2f(ci + 1, cj + 1);
    glVertex2f(v1.x, v1.y);

    glTexCoord2f(ci + 1, cj);
    glVertex2f(v2.x, v2.y);

    glTexCoord2f(ci, cj);
    glVertex2f(v3.x, v3.y);
}

void App_Shell::draw_text_line(int *x, int *y, char *s) {
    int len = strlen(s);

    float cr = 1.0f;
    float cg = 1.0f;
    float cb = 1.0f;

    int xs = *x;
    int ys = *y;

    glBegin(GL_QUADS);

    int i;
    for (i = 0; i < len; i++) {
        add_letter_quad(s, i, xs, ys, LETTER_WIDTH, LETTER_HEIGHT,
                        cr, cg, cb, screen_width, screen_height);

		xs += LETTER_WIDTH;
    }

    glEnd();

    *y -= LETTER_HEIGHT - LETTER_PAD;
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

        dest[i * 4 + 0] = r;
        dest[i * 4 + 1] = g;
        dest[i * 4 + 2] = b;
        dest[i * 4 + 3] = a;
    }

    free(src);
    return dest;
}

bool texture_from_file(char *filename, GLuint *result,
                       int *width_result, int *height_result) {
    unsigned char *bitmap;
    int width, height;
    bool success = load_jpeg_file(filename, &bitmap, &width, &height);
    if (success == false) return false;

    bitmap = do_dumb_alpha_thing(bitmap, width, height);

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

void App_Shell::load_texture(Loaded_Texture_Info *info, char *name) {
	GLuint handle;
    bool success = texture_from_file(name, &handle,
                                     &info->width, &info->height);
	info->texture_handle = handle;
    info->loaded_successfully = success;
}

void App_Shell::init_textures() {
    load_texture(&font_texture, "data\\app_shell\\font.jpg");
	assert(font_texture.loaded_successfully);
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

void App_Shell::text_mode_begin() {
	if (!font_texture.loaded_successfully) return;
	
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(LETTER_WIDTH / 256.0, LETTER_HEIGHT / 256.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_width, 0, screen_height, 0, -100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthFunc(GL_ALWAYS);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, font_texture.texture_handle);
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
inline HWND get_hwnd(App_Shell *shell) {
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
        return '!';
    case VK_RIGHT:
    case VK_NEXT:
        return '@';
    case VK_UP:
        return '#';
    case VK_DOWN:
        return '$';
    default:
        return wParam;
    }
}

LRESULT CALLBACK WindowProc(HWND    hWnd,
							UINT    message,
							WPARAM  wParam,
							LPARAM  lParam) {
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    extern void draw_scene();
    draw_scene();

    SwapBuffers(global_dc);

    Sleep(20); // Let us not eat all the CPU under any circumstances!
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
	
    MSG msg;
    for (;;) {
		HWND hwnd = get_hwnd(app_shell);

		// Do mouse pointer stuff.
		POINT point;
		BOOL success = GetCursorPos(&point);
		if (success) {
			RECT client_rect;
			GetClientRect(hwnd, &client_rect);

			int orig_y = point.y;
			int dy = client_rect.bottom - client_rect.top;
			point.y = client_rect.top + (dy - (orig_y - client_rect.top));

			
			int cursor_x = point.x;
			//			int cursor_y = app_shell->screen_height - point.y;
			int cursor_y = point.y;

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


float Vector3::length() {
	return sqrt(length_squared());
}


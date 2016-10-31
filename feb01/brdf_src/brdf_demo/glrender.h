// Externs.h
#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

#define CAMERA_FOV	60.0f			// Default FOV
#define WORLD_SIZE_X	30.0f
#define WORLD_SIZE_Y	30.0f
#define WORLD_SIZE_Z	30.0f

typedef struct
{
	union {
		float x;
		float u;
		float r;
	};
	union {
		float y;
		float v;
		float g;
	};
	union {
		float z;
		float w;
		float b;
	};
} tVector;

typedef struct s_Camera
{
	tVector		rot;					// CURRENT ROTATION FACTORS
	tVector		trans;					// CURRENT TRANSLATION FACTORS
	float		fov;					// FIELD OF VIEW
} t_Camera;

// Camera for View System
extern t_Camera			g_POV;
void SetDCDepthPixelFormat(HDC hDC);
BOOL BuildClasses(void);
extern LPCTSTR lpszMainWndClass;
extern LPCTSTR lpszViewWndClass;
extern HWND AppWindow;
extern HDC g_hDC;
extern HGLRC  g_hRC;
extern HPALETTE	hPalette;
extern HINSTANCE hInstance;

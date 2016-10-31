// Externs.h
#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

#include "MathDefs.h"

#define MAX_ELEMENTS	50
#define MAX_PARTICLES	4096

#define M_PI			3.1415926
#define HALF_PI			1.5707963
#define PI_TIMES_TWO	6.2831852
/// Trig Macros ///////////////////////////////////////////////////////////////
#define DEGTORAD(A)	((A * M_PI) / 180.0f)
#define RADTODEG(A)	((A * 180.0f) / M_PI)
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b)   ((a < b) ? a : b)
#define SIGN(a)  ((a < 0) ? -1 : 1)
///////////////////////////////////////////////////////////////////////////////

// OpenGL DisplayList Definitions
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID
#define OGL_ELEMENT_DLIST	2		// OPENGL DISPLAY LIST ID

enum eElementTypes
{
	ELEMENT_UNIFORM,
	ELEMENT_SOURCE,
	ELEMENT_VORTEX,
	ELEMENT_DOUBLET
};

// Camera used for rendering
typedef struct s_Camera
{
	tVector		rot;					// CURRENT ROTATION FACTORS
	tVector		trans;					// CURRENT TRANSLATION FACTORS
	float		fov;					// FIELD OF VIEW
} t_Camera;

typedef struct s_Element
{
	int		type;
        bool    emitParticles;
	float	size;
	float	strength;
	tVector	pos;
} t_Element;

typedef struct s_Particle
{
	tVector rest_pos;		// Position of Particle
	tVector pos;		// Position of Particle
    tVector v;			// Velocity of Particle
	int		flags;		// So I can track things
} t_Particle;

// Camera for View System
extern t_Camera			g_POV;

// Variable for the system needed globally
extern t_Element		g_Element[MAX_ELEMENTS];
extern int				g_ElementCnt;
extern int				g_Picked;
extern int				g_ScreenWidth, g_ScreenHeight;

// Boolean Flags for display and settings
extern int				g_DrawParticles;
extern int				g_DrawInfluence;

// External handles to all the windows, palettes, etc.
extern HWND				hViewWnd;
extern HWND				hMainWnd;
extern HPALETTE		hPalette;
extern HDC			g_hDC;

/////////////////////////
// Functions declared in other source files
// Defined in RenderWorld.cpp
void InitRender(void);	
void FreeRender(void);
void RenderWorld(void);	
void EvaluateVelocity(t_Particle *particle);
void AddParticle(float xpos, float ypos);
void AdvanceParticles();
void KillParticle(int which);

// Defined in ViewWnd.cpp
void AddElement();
void DeleteElement();

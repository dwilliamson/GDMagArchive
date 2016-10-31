// Externs.h
#include "MathDefs.h"

#define		SYSTEM_COUNT	3
#define		MAX_CONTACTS	2048
#define		MAX_PARTICLES	2048
#define		GROUND_POINTS	80
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
#define EPSILON  0.00001f				// ERROR TERM
#define DEFAULT_DAMPING		0.002f
#define	DRAG_FORCE		5.0f		// For Mouse interaction

// OpenGL DisplayList Definitions
#define OGL_AXIS_DLIST		1		// OPENGL DISPLAY LIST ID

enum eParticleFlags
{
	NOT_COLLIDING = 1,
	PENETRATING = 2,
	COLLIDING_WITH_GROUND = 4
};

enum eIntegratorTypes
{
	EULER_INTEGRATOR,
	MIDPOINT_INTEGRATOR,
	RK4_INTEGRATOR,
	PC_INTEGRATOR
};

// TYPE FOR CONTACTS THAT ARE FOUND DURING SIM
typedef struct s_Contact
{
	int		particle,particle2;	
    CVector normal;		// Normal of Collision plane
	int		type;		// COLLIDING OR CONTACT
	float	Kr;			// Coefficient of restitution
} t_Contact;

// Camera used for rendering
typedef struct s_Camera
{
	CVector		rot;					// CURRENT ROTATION FACTORS
	CVector		trans;					// CURRENT TRANSLATION FACTORS
	float		fov;					// FIELD OF VIEW
} t_Camera;

typedef struct s_Particle
{
	CVector rest_pos;		// Position of Particle
	CVector pos;		// Position of Particle
    CVector v;			// Velocity of Particle
	CVector f;			// Force Acting on Particle
	CVector f_old;		// Force Acting on Particle from previous frame
	float	oneOverM;	// 1 / Mass of Particle
	int		flags;		// So I can track things
} t_Particle;

// Springs used for surface tension
typedef struct s_Spring
{
	int		p1,p2;		// PARTICLE INDEX FOR ENDS
	float	restLen;	// LENGTH OF SPRING AT REST
	float	Ks;			// SPRING CONSTANT
	float	Kd;			// SPRING DAMPING
	float	stress;		// STRESS CALCULATION 
	int		active;		// IS THE SPRING ACTIVE
} t_Spring;

// Camera for View System
extern t_Camera			g_POV;

// Variable for the system needed globally
extern t_Particle		*g_CurrentSys;
extern int				g_MouseForceActive;
extern float			g_ViewMatrix[];
extern int				g_Pick[2];
extern CVector			g_Ground[];
extern int				g_ParticleCount;
extern DWORD			g_Time;
extern float			g_Hour;
extern float			g_Kd;
extern float			g_Kr;	// Particle to Wall Coefficient of Restitution
extern float			g_Ksh;
extern float			g_Ksd;
extern float			g_YoungModulus;
extern float			g_FractureThreshold;
extern t_Particle			*g_GameSys[SYSTEM_COUNT];			// LIST OF PHYSICAL PARTICLES
extern t_Spring		*g_Spring;				// VALID SPRINGS IN SYSTEM
extern int				g_SpringCnt;		
extern CVector			vGravity;

// Boolean Flags for display and settings
extern int				g_UseFriction;				// Global to Select Friction
extern int				g_UseGravity;				// Global to Select Gravity
extern int				g_DrawSprings;
extern int				g_DrawCVs;
extern int				g_GridSnap;

// External handles to all the windows, palettes, etc.
extern HWND		hViewWnd;
extern HWND     hMainWnd;
extern HPALETTE hPalette;
extern HDC		g_hDC;

/////////////////////////
// Functions declared in other source files
// Defined in RenderWorld.cpp
void InitRender(void);	
void RenderWorld(void);	
float GetTime( void );
void GetNearestPoint(int x, int y);
void ChangeGround(int hitX, int hitY);
void AddParticle(int hitX, int hitY);
void AddSpring(int p1, int p2);

// Defines from SpringSim.cpp
BOOL InitSim(void);
void ResetSim();
void Simulate(float DeltaTime, BOOL running);
void DrawSimWorld(void);
void LoadSimulation();
void SaveSimulation();
void SetMouseForce(int deltaX,int deltaY, CVector *localX, CVector *localY);

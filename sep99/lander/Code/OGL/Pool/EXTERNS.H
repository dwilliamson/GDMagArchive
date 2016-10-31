// Externs.h
#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

#include "MathDefs.h"

// This sets up the table position in the world
// TODO: Should load with the model....?
#define TABLE_POSITION	3.45f			// Height of the virtual table off ground
#define LEFT_BUMPER		4.88f			// Left X coordinate of Table Bumper
#define	RIGHT_BUMPER	4.88f			// Right X coordinate of Table Bumper
#define TOP_BUMPER		2.3f			// Top Z coordinate of Table Bumper	-  Ooops model error
#define	BOTTOM_BUMPER	2.45f			// Bottom Z coordinate of Table Bumper

// Physical Dimension of the Ball
#define BALL_DIAMETER	0.1875f			// Diameter of Billiard Ball in feet 2.25 in

#define EPSILON  0.00001f				// ERROR TERM
#define DEFAULT_DAMPING		0.002f
#define	CUE_STICK_FORCE		5.0f		// For Mouse interaction

enum tCollisionTypes
{
	NOT_COLLIDING,
	PENETRATING,
	COLLIDING_WITH_WALL,
	COLLIDING_WITH_BALL
};

enum tIntegratorTypes
{
	EULER_INTEGRATOR,
	MIDPOINT_INTEGRATOR,
	RK4_INTEGRATOR
};

// TYPE FOR CONTACTS THAT ARE FOUND DURING SIM
typedef struct s_Contact
{
	int		ball,ball2;		// Ball Index Ball2 is only used in Ball 2 ball collisions
    tVector normal;		// Normal of Collision plane
	int		type;		// COLLIDING OR CONTACT
	float	Kr;			// Coefficient of restitution
} t_Contact;

// TYPE FOR A PLANE THAT THE SYSTEM WILL COLLIDE WITH
typedef struct s_CollisionPlane
{
	tVector normal;			// inward pointing
    float	d;				// ax + by + cz + d = 0
} t_CollisionPlane;

typedef struct s_Camera
{
	tVector		rot;					// CURRENT ROTATION FACTORS
	tVector		trans;					// CURRENT TRANSLATION FACTORS
	float		fov;					// FIELD OF VIEW
} t_Camera;

// Structure to Hold the Cue Stick
typedef struct s_Cue
{
	float	draw,old_draw;				// How much is the stick pulled back
	float	yaw;						// Direction
	tVector	pos;
	float	drawtime;
} t_CueStick;

// TYPE FOR A PHYSICAL PARTICLE IN THE SYSTEM
typedef struct s_Ball
{
	tVector pos;		// Position of Particle
    tVector v;			// Velocity of Particle
	tVector f;			// Force Acting on Particle
    tVector angMom;		// Angular Momentum
    tVector torque;		// Torque
	float	oneOverM;	// 1 / Mass of Particle
	tQuaternion	orientation;
	int		flags;		// So I can track things
} t_Ball;

#define POLY_SELECTED	1
#define POLY_TEXTURED	2

// Camera for View System
extern t_Camera			g_POV;

// Objects for the Game
extern t_CueStick		g_CueStick;
extern t_Ball			*g_CurrentSys;
extern int				g_CueHitBall;				// Set when a Cue hits the ball
extern tVector			g_CueForce;							
extern int				g_BallInPlay;
extern int				g_UseFriction;				// Global to Select Friction

// External handles to all the windows, palettes, etc.
extern HWND		hViewWnd;
extern HWND     hMainWnd;
extern HPALETTE hPalette;
extern HDC		g_hDC;

// Other shared variables
extern double	PI;

/////////////////////////
// Functions declared in other source files
BOOL InitGame(void);	// Defined in GameSim.c
void InitRender(void);	// Defined in RenderWorld.c
void RenderWorld(void);	// Defined in RenderWorld.c
void Simulate(float DeltaTime, BOOL running);	// In GameSim.c
float GetTime( void );

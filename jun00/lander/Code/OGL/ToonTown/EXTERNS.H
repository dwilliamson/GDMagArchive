// Externs.h
#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

#include "MathDefs.h"

// This sets up the table position in the world
// TODO: Should load with the model....?
#define FFD_STARTY		4.20f			// Height of the virtual table off ground

// Physical Dimension of the Ball
#define BALL_DIAMETER	0.1875f			// Diameter of Billiard Ball in feet 2.25 in

#define EPSILON  0.00001f				// ERROR TERM
#define DEFAULT_DAMPING		0.002f
#define	DRAG_FORCE		5.0f		// For Mouse interaction

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
	int		particle,particle2;	
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

// TYPE FOR A PHYSICAL PARTICLE IN THE SYSTEM
typedef struct s_Particle
{
	tVector rest_pos;		// Position of Particle
	tVector pos;		// Position of Particle
    tVector v;			// Velocity of Particle
	tVector f;			// Force Acting on Particle
    tVector angMom;		// Angular Momentum
    tVector torque;		// Torque
	float	oneOverM;	// 1 / Mass of Particle
	tQuaternion	orientation;
	int		flags;		// So I can track things
} t_Particle;

// TYPE FOR SPRINGS IN SYSTEM
typedef struct s_Spring
{
	int		p1,p2;		// PARTICLE INDEX FOR ENDS
	float	restLen;	// LENGTH OF SPRING AT REST
	float	Ks;			// SPRING CONSTANT
	float	Kd;			// SPRING DAMPING
} t_Spring;

/// Model Definitions ///////////////////////////////////////////////////////
struct t_VWeight
{
	float		*weight;
};

typedef struct
{
	long	v[3],n[3],t[3];
	int		mat;
} t_faceIndex;

typedef struct 
{
	int		dataFormat;
	tVector	*vertex;		// Vertex
	long	vertexCnt;		// NUMBER OF VERTICES IN VISUAL
	tVector	*normal;		// Vertex
	tVector	*deformData;	// DEFORMED VERTEX DATA
	tVector	*texture;		// Vertex
	t_faceIndex	*index;			
	long	faceCnt;		// NUMBER OF FACES IN VISUAL
	tVector *matColor;		// POINTER TO VECTOR
	int		matCnt;
	long	vPerFace;		// VERTICES PER FACE, EITHER 3 OR 4
	tColor  Ka,Kd,Ks;		// COLOR FOR OBJECT
	float	Ns;				// SPECULAR COEFFICIENT
	char    map[255];
	uint    glTex;
	float	*weightData;		//
	long	normalCnt;		// NUMBER OF VERTICES IN VISUAL
	long	uvCnt;		// NUMBER OF VERTICES IN VISUAL
	tVector bbox[8];		// BBOX COORDS
	tVector transBBox[8];	
} t_ToonVisual;

typedef struct 
{
	long	id;							// BONE ID
	char	name[80];					// BONE NAME
	long	flags;						// BONE FLAGS
	// HIERARCHY INFO
	void	*parent;					// POINTER TO PARENT BONE
	int 	childCnt;					// COUNT OF CHILD BONES
	void	*children;					// POINTER TO CHILDREN
	// TRANSFORMATION INFO
	tVector	b_scale;					// BASE SCALE FACTORS
	tVector	b_rot;						// BASE ROTATION FACTORS
	tVector	b_trans;					// BASE TRANSLATION FACTORS
	tVector	scale;						// CURRENT SCALE FACTORS
	tVector	rot;						// CURRENT ROTATION FACTORS
	tVector	trans;						// CURRENT TRANSLATION FACTORS
	tQuaternion quat;					// QUATERNION USEFUL FOR ANIMATION
	tMatrix matrix;						// PLACE TO STORE THE MATRIX

	// ANIMATION INFO
	DWORD	primChanType;				// WHAT TYPE OF PREIMARY CHAN IS ATTACHED
	float	*primChannel;				// POINTER TO PRIMARY CHANNEL OF ANIMATION
	float 	primFrameCount;				// FRAMES IN PRIMARY CHANNEL
	float	primSpeed;					// CURRENT PLAYBACK SPEED
	float	primCurFrame;				// CURRENT FRAME NUMBER IN CHANNEL
	DWORD	secChanType;				// WHAT TYPE OF SECONDARY CHAN IS ATTACHED
	float	*secChannel;				// POINTER TO SECONDARY CHANNEL OF ANIMATION
	float	secFrameCount;				// FRAMES IN SECONDARY CHANNEL
	float	secCurFrame;				// CURRENT FRAME NUMBER IN CHANNEL
	float	secSpeed;					// CURRENT PLAYBACK SPEED
	float	animBlend;					// BLENDING FACTOR (ANIM WEIGHTING)
	// DOF CONSTRAINTS
	float	min_rx, max_rx;				// ROTATION X LIMITS
	float	min_ry, max_ry;				// ROTATION Y LIMITS
	float	min_rz, max_rz;				// ROTATION Z LIMITS
	float	damp_width, damp_strength;	// DAMPENING SETTINGS
	// VISUAL ELEMENTS
	int		visualCnt;					// COUNT OF ATTACHED VISUAL ELEMENTS
	t_ToonVisual	*visuals;					// POINTER TO VISUALS/BITMAPS
	int		*CV_select;			// POINTER TO CONTROL VERTICES
	void	*CV_weight;					// POINTER TO ARRAY OF WEIGHTING VALUES
	// COLLISION ELEMENTS
	float	bbox[6];					// BOUNDING BOX (UL XYZ, LR XYZ)
	tVector	center;						// CENTER OF OBJECT (MASS)
	float	bsphere;					// BOUNDING SPHERE (RADIUS)  
	// PHYSICS
	tVector	length;						// BONE LENGTH VECTOR
	float	mass;						// MASS
	float	friction;					// STATIC FRICTION
	float	kfriction;					// KINETIC FRICTION
	tMatrix *curMatrix;					// STORE THE CURRENT MATRIX
} t_Mesh;

#define POLY_SELECTED	1
#define POLY_TEXTURED	2

// Camera for View System
extern t_Camera			g_POV;

// Objects for the Game
extern t_Particle		*g_CurrentSys;
extern int				g_UseFriction;				// Global to Select Friction
extern int				g_UseGravity;				// Global to Select Gravity
extern int				g_DrawSprings;
extern int				g_DrawCVs;
extern int				g_DrawMesh;
extern int				g_DrawInfluence;			// For display of vertex weights
extern int				g_MouseForceActive;
extern tMatrix			g_ViewMatrix;
extern int				g_Pick[2];

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
void DrawSimWorld(void);	// Defined in GameSim.c
void Simulate(float DeltaTime, BOOL running);	// In GameSim.c
void InitRender(void);	// Defined in RenderWorld.c
void RenderWorld(void);	// Defined in RenderWorld.c
float GetTime( void );
void GetNearestPoint(int x, int y);
void SetMouseForce(int deltaX,int deltaY, tVector *localX, tVector *localY);
void SetFFDWeights(t_ToonVisual *visual);
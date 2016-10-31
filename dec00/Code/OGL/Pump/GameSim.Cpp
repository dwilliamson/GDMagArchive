///////////////////////////////////////////////////////////////////////////////
//
// Game Simulation Implementation File			GameSim.c
//
// Purpose:	Implementation of Toon Physics
//			This is the meat of the physical simulation for the ToonTown
// Created:
//		JL 3/5/00	Created for the new sample app
//
// Modified:
//		JL 10/7/00	Changes to add volume preservation technique
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998-2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>	// Normal Windows stuff
#include <assert.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <math.h>
#include "externs.h"	// Data shared between files

#define		CUBE_NODE_COUNT	64
#define		CUBE_WIDTH	4
#define		CUBE_HEIGHT	4
#define		CUBE_DEPTH	4

#define		SYSTEM_COUNT	4
#define		MAX_CONTACTS	64

void			SetupWorld();
void ComputeObjLocalFrame(tMatrix *obj_matrix);

t_Particle		g_Particle[CUBE_NODE_COUNT];		

float			g_Kd;
float			g_Kr;	// Particle to Wall Coefficient of Restitution
float			g_Ksh;
float			g_Ksd;
float			g_Keh;
float			g_Ked;
float			g_Csf;
float			g_Ckf;
int				g_IntegratorType;
int				g_CollisionRootFinding = FALSE;		// ONLY SET WHEN FINDING A COLLISION
int				g_UseDamping = TRUE;				// Use a Damping force
int				g_UseFriction = FALSE;				// Use Friction
int				g_UseGravity = TRUE;				// Use Gravity
int				g_DrawSprings = TRUE;
int				g_DrawVolumePoints = TRUE;					// Draw the Volume Preserving point
int				g_MouseForceActive = FALSE;
int				g_DrawCVs = TRUE;
int				g_DrawMesh = TRUE;
int				g_Pick[2];

tVector			g_Gravity;
tVector			g_MouseDragPos[2];

t_Contact		g_Contact[MAX_CONTACTS];			// LIST OF POSSIBLE COLLISIONS
int				g_ContactCnt;						// COLLISION COUNT
t_CollisionPlane	g_CollisionPlane[5];		// LIST OF COLLISION PLANES
int					g_CollisionPlaneCnt;			
t_Particle			g_GameSys[SYSTEM_COUNT][CUBE_NODE_COUNT];			// LIST OF PHYSICAL PARTICLES
t_Particle			*g_CurrentSys,*g_TargetSys;
int				g_ParticleCount;
t_Spring		*g_Spring;				// VALID SPRINGS IN SYSTEM
int				g_SpringCnt;		
float			g_WorldSizeX,g_WorldSizeY,g_WorldSizeZ;
tVector			g_FFDmin,g_FFDmax;

tMatrix			g_ObjectMatrix;

///////////////////////////////////////////////////////////////////////////////
// Initialize the world and the objects in it.
BOOL InitGame(void)
{
	g_Pick[0] = -1;
	g_Pick[1] = -1;

	g_Kd	= 0.5f;	// DAMPING FACTOR
	g_Kr	= 0.6f;		// 1.0 = SUPERBALL BOUNCE 0.0 = DEAD WEIGHT
	g_Ksh	= 200.0f;		// HOOK'S SPRING CONSTANT
	g_Ksd	= 4.0f;		// SPRING DAMPING CONSTANT

	g_Keh	= 20.0f;		// ELASTIC RETURN CONSTANT
	g_Ked	= 0.2f;		// ELASTIC RETURN DAMPING CONSTANT

	g_Csf	= 0.2f;		// Default Static Friction
	g_Ckf	= 0.05f;	// Default Kinetic Friction

	g_Gravity.Set(0.0f, -16.0f, 0.0f);	// Feet per second

	// Pick an Integrator.
	g_IntegratorType = EULER_INTEGRATOR;

	g_CollisionRootFinding = FALSE;		// ONLY SET WHEN FINDING A COLLISION

	g_ContactCnt = 0;
	g_SpringCnt = 0;


	g_ParticleCount = CUBE_NODE_COUNT;

	g_CurrentSys = g_GameSys[0];
	g_TargetSys = g_GameSys[1];

	SetupWorld();


	g_CollisionPlaneCnt = 5;

	// CREATE THE SIZE FOR THE SIMULATION WORLD
	g_WorldSizeX = 15.0f;
	g_WorldSizeY = 15.0f;
	g_WorldSizeZ = 15.0f;

	// MAKE THE BOTTOM PLANE (FLOOR)
	g_CollisionPlane[0].normal.Set(0.0f, 1.0f, 0.0f);
	g_CollisionPlane[0].d = g_WorldSizeY / 2.0f;

	// MAKE THE LEFT PLANE
	g_CollisionPlane[1].normal.Set(-1.0f, 0.0f, 0.0f);
	g_CollisionPlane[1].d = g_WorldSizeX / 2.0f;

	// MAKE THE RIGHT PLANE
	g_CollisionPlane[2].normal.Set(1.0f, 0.0f, 0.0f);
	g_CollisionPlane[2].d = g_WorldSizeX / 2.0f;

	// MAKE THE FRONT PLANE
	g_CollisionPlane[3].normal.Set(0.0f, 0.0f, -1.0f);
	g_CollisionPlane[3].d = g_WorldSizeZ / 2.0f;

	// MAKE THE BACK PLANE
	g_CollisionPlane[4].normal.Set(0.0f, 0.0f, 1.0f);
	g_CollisionPlane[4].d = g_WorldSizeZ / 2.0f;


	return TRUE;
}

void AddSpring(int p1, int p2,t_Particle *sys, int type)
{
	t_Spring		*tempSprings;

	tempSprings = (t_Spring *)malloc(sizeof(t_Spring) * (g_SpringCnt + 1));
	if (g_SpringCnt > 0)
	{
		memcpy(tempSprings,g_Spring,sizeof(t_Spring) * g_SpringCnt);
		free(g_Spring);
	}
	g_Spring = tempSprings;
	tempSprings = &g_Spring[g_SpringCnt];
	g_SpringCnt = g_SpringCnt + 1;
	tempSprings->Ks = g_Ksh;
	tempSprings->Kd = g_Ksd;
	tempSprings->p1 = p1;
	tempSprings->p2 = p2;
	tempSprings->restLen = sys[p2].pos.VectorDistance(&sys[p1].pos);
	tempSprings->type = type;
}

void SetupWorld()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop, loop2,px,py,pz;
///////////////////////////////////////////////////////////////////////////////

	g_FFDmin.Set(9999.0f, 9999.0f, 9999.0f);
	g_FFDmax.Set(-9999.0f, -9999.0f, -9999.0f);
	// This is a bit screwy since it has a hard coded size and position for the FFD lattice
	for (loop = 0; loop < SYSTEM_COUNT; loop++)
	{
		for (loop2 = 0; loop2 < g_ParticleCount; loop2++)		//
		{				
			g_GameSys[loop][loop2].pos.Set(3.0f - ((loop2 % CUBE_WIDTH) * 2.0f), CUBE_STARTY - ((loop2 / (CUBE_WIDTH * CUBE_DEPTH)) * 2.0f), 3.0f - (((loop2 % (CUBE_WIDTH * CUBE_DEPTH)) / CUBE_WIDTH) * 2.0f));
			g_GameSys[loop][loop2].rest_pos = g_GameSys[loop][loop2].pos;
			g_GameSys[loop][loop2].v.Set(0.0f, 0.0f, 0.0f);
			g_GameSys[loop][loop2].f.Set(0.0f, 0.0f, 0.0f);
			g_GameSys[loop][loop2].oneOverM = 1.0f / 0.34f;
			g_GameSys[loop][loop2].angMom.Set(0.0f, 0.0f, 0.0f);
			g_GameSys[loop][loop2].torque.Set(0.0f, 0.0f, 0.0f);

			// Clear the Quaternion
			MAKEVECTOR(g_GameSys[loop][loop2].orientation,0.0f, 0.0f, 0.0f)
			g_GameSys[loop][loop2].orientation.w = 1.0f;	// Handle the W part

			g_GameSys[loop][loop2].flags = 0;
			if (g_GameSys[loop][loop2].pos.x < g_FFDmin.x)
				g_FFDmin.x = g_GameSys[loop][loop2].pos.x;
			if (g_GameSys[loop][loop2].pos.y < g_FFDmin.y)
				g_FFDmin.y = g_GameSys[loop][loop2].pos.y;
			if (g_GameSys[loop][loop2].pos.z < g_FFDmin.z)
				g_FFDmin.z = g_GameSys[loop][loop2].pos.z;

			if (g_GameSys[loop][loop2].pos.x > g_FFDmax.x)
				g_FFDmax.x = g_GameSys[loop][loop2].pos.x;
			if (g_GameSys[loop][loop2].pos.y > g_FFDmax.y)
				g_FFDmax.y = g_GameSys[loop][loop2].pos.y;
			if (g_GameSys[loop][loop2].pos.z > g_FFDmax.z)
				g_FFDmax.z = g_GameSys[loop][loop2].pos.z;
		}
	}

	g_SpringCnt = 0;
	// Setup the springs
	for (loop = 0; loop < g_ParticleCount; loop++)
	{	
		px = loop % CUBE_WIDTH;
		pz = (loop % (CUBE_WIDTH * CUBE_HEIGHT)) / CUBE_WIDTH;
		py = loop / (CUBE_WIDTH * CUBE_HEIGHT);

		// Structural Springs
		if (px < 3)
		{
			AddSpring(loop, loop + 1,&g_GameSys[0][0],SPRING_STRUCTURAL);
		}
		if (pz < 3)
		{
			AddSpring(loop, loop + 4,&g_GameSys[0][0],SPRING_STRUCTURAL);
		}
		if (py < 3)
		{
			AddSpring(loop, loop + 16,&g_GameSys[0][0],SPRING_STRUCTURAL);
		}
		// Shear Springs along X axis
		if (px < 3 && py < 3)
		{
			AddSpring(loop, loop + 17,&g_GameSys[0][0],SPRING_SHEAR);
		}
		if (px > 0 && py < 3)
		{
			AddSpring(loop, loop + 15,&g_GameSys[0][0],SPRING_SHEAR);
		}
		// Shear Springs along Z axis
		if (py < 3 && pz < 3)
		{
			AddSpring(loop, loop + 20,&g_GameSys[0][0],SPRING_SHEAR);
		}
		if (py < 3 && pz > 0)
		{
			AddSpring(loop, loop + 12,&g_GameSys[0][0],SPRING_SHEAR);
		}
		// Shear Springs along Y axis
		if (px < 3 && pz < 3)
		{
			AddSpring(loop, loop + 5,&g_GameSys[0][0],SPRING_SHEAR);
		}
		if (px > 0 && pz < 3)
		{
			AddSpring(loop, loop + 3,&g_GameSys[0][0],SPRING_SHEAR);
		}
		// Cross section volume springs
		if (py < 3 && pz > 0)
		{
			AddSpring(loop, loop + 12,&g_GameSys[0][0],SPRING_FLEXON);
		}
		if (px < 3 && py < 3 && pz < 3)
		{
			AddSpring(loop, loop + 21,&g_GameSys[0][0],SPRING_FLEXON);
		}
		if (px > 0 && py < 3 && pz < 3)
		{
			AddSpring(loop, loop + 19,&g_GameSys[0][0],SPRING_FLEXON);
		}
		if (px < 3 && py > 0 && pz < 3)
		{
			AddSpring(loop, loop - 11,&g_GameSys[0][0],SPRING_FLEXON);
		}
		if (px > 0 && py > 0 && pz < 3)
		{
			AddSpring(loop, loop - 13,&g_GameSys[0][0],SPRING_FLEXON);
		}
	}

	// Compute starting reference frame for FFD block
	ComputeObjLocalFrame(&g_ObjectMatrix);

	// Store the relative offsets of the rest matrix in a global storage.
	// I need to subtract from the calculated center of the object.
	t_Particle *tempParticle = g_GameSys[3];
	tVector pt;
	pt.Set(g_ObjectMatrix.m[12],g_ObjectMatrix.m[13],g_ObjectMatrix.m[14]);
	for (loop = 0; loop < g_ParticleCount; loop++)
	{	
		tempParticle->pos -= pt;
		tempParticle++;
	}

}

void DrawSimWorld()
{
	t_Particle		*tempParticle;
	t_Spring		*tempSpring;
	int				loop;

	// Compute the new local 3D frame for the deformable object
	ComputeObjLocalFrame(&g_ObjectMatrix);

    // draw floor
    glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
        glColor3f(0.3f,0.3f,0.3f);
        glVertex3f(-g_WorldSizeX/2.0f,-g_WorldSizeY/2.0f,-g_WorldSizeZ/2.0f);
        glVertex3f( g_WorldSizeX/2.0f,-g_WorldSizeY/2.0f,-g_WorldSizeZ/2.0f);
        glVertex3f( g_WorldSizeX/2.0f,-g_WorldSizeY/2.0f, g_WorldSizeZ/2.0f);
        glVertex3f(-g_WorldSizeX/2.0f,-g_WorldSizeY/2.0f, g_WorldSizeZ/2.0f);
    glEnd();
    glEnable(GL_CULL_FACE);


	glLineWidth(1.0f);
	if (g_CurrentSys)
	{
		if (g_Spring && g_DrawSprings)
		{
			glBegin(GL_LINES);
			glColor3f(0.0f,0.8f,0.8f);
			tempSpring = g_Spring;
			for (loop = 0; loop < g_SpringCnt; loop++)
			{
				if (tempSpring->type == SPRING_STRUCTURAL)
				{
					glVertex3fv((float *)&g_CurrentSys[tempSpring->p1].pos.x);
					glVertex3fv((float *)&g_CurrentSys[tempSpring->p2].pos.x);
				}
				tempSpring++;
			}
			if (g_MouseForceActive)	// DRAW MOUSESPRING FORCE
			{
				if (g_Pick[0] > -1)
				{
					glColor3f(0.8f,0.0f,0.8f);
					glVertex3fv((float *)&g_CurrentSys[g_Pick[0]].pos.x);
					glVertex3fv((float *)&g_MouseDragPos[0].x);
				}
				if (g_Pick[1] > -1)
				{
					glColor3f(0.8f,0.0f,0.8f);
					glVertex3fv((float *)&g_CurrentSys[g_Pick[1]].pos.x);
					glVertex3fv((float *)&g_MouseDragPos[1].x);
				}
			}
			glEnd();
		}
		if (g_DrawCVs)
		{
			glBegin(GL_POINTS);
			tempParticle = g_CurrentSys;
			for (loop = 0; loop < g_ParticleCount; loop++)
			{
				// Picked particles are green then red for 1 and 2
				if (loop == g_Pick[0])
					glColor3f(0.0f,0.8f,0.0f);
				else if (loop == g_Pick[1])
					glColor3f(0.8f,0.0f,0.0f);
				// If particles are in contact, Draw them in Orange
//				else if (tempParticle->contacting && g_UseFriction)
//					glColor3f(1.0f,0.5f,0.0f);
				// Normally Yellow
				else
					glColor3f(0.8f,0.0f,0.8f);

				glVertex3fv((float *)&tempParticle->pos.x);
				tempParticle++;
			}

			glEnd();
		}
	
		if (g_DrawVolumePoints)
		{
			glPushMatrix();
			// Draw the calculated orientation matrix
			glMultMatrixf( g_ObjectMatrix.m );
			glLineWidth(3.0f);
			glScalef(4.0f, 4.0f, 4.0f);
			// Draw the Axis
			glCallList(OGL_AXIS_DLIST);
			glPopMatrix();

			// Draw the Position of the Original positions rotated by the calculated orientation matrix
			glColor3f(0.0f,0.0f,0.8f);
			// This is where I stored those positions relative to the center
			tempParticle = g_GameSys[3];
			tVector pt;
			glBegin(GL_POINTS);
			for (loop = 0; loop < g_ParticleCount; loop++)
			{	
				pt = tempParticle->pos;
				pt.MultVectorByMatrix(g_ObjectMatrix.m);
				glVertex3fv((float *)&pt.x);
				tempParticle++;
			}
			glEnd();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CompareBuffer
// Purpose:		Check the feedback buffer to see if anything is hit
// Arguments:	Number of hits, pointer to buffer, point to test
///////////////////////////////////////////////////////////////////////////////
void CompareBuffer(int size, float *buffer,float x, float y)
{
/// Local Variables ///////////////////////////////////////////////////////////
	GLint count;
	GLfloat token,point[3];
	int loop,currentVertex,result = -1;
	long nearest = -1, dist;
///////////////////////////////////////////////////////////////////////////////
	count = size;
	while (count)
	{
		token = buffer[size - count];	// CHECK THE TOKEN
		count--;
		if (token == GL_PASS_THROUGH_TOKEN)	// VERTEX MARKER
		{
			currentVertex = (int)buffer[size - count]; // WHAT VERTEX
			count--;
		}
		else if (token == GL_POINT_TOKEN)
		{
			// THERE ARE THREE ELEMENTS TO A POINT TOKEN
			for (loop = 0; loop < 3; loop++)
			{
				point[loop] = buffer[size - count];
				count--;
			}
			dist = ((x - point[0]) * (x - point[0])) + ((y - point[1]) * (y - point[1]));
			if (result == -1 || dist < nearest)
			{
				nearest = dist;
				result = currentVertex;
			}
		}
	}

	if (nearest < 50.0f)
	{
		if (g_Pick[0] == -1)
			g_Pick[0] = result;
		else if (g_Pick[1] == -1)
			g_Pick[1] = result;
		else
		{
			g_Pick[0] = result;
			g_Pick[1] = -1;
		}
	}
}
////// CompareBuffer //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	GetNearestPoint
// Purpose:		Use OpenGL Feedback to find the closest point to a mouseclick
// Arguments:	Screen coordinates of the hit
///////////////////////////////////////////////////////////////////////////////
void GetNearestPoint(int x, int y)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float *feedBuffer;
	int hitCount;
	t_Particle *tempParticle;
	int loop;
///////////////////////////////////////////////////////////////////////////////
	// INITIALIZE A PLACE TO PUT ALL THE FEEDBACK INFO (3 DATA, 1 TAG, 2 TOKENS)
	feedBuffer = (float *)malloc(sizeof(GLfloat) * g_ParticleCount * 6);
	// TELL OPENGL ABOUT THE BUFFER
	glFeedbackBuffer(g_ParticleCount * 6,GL_3D,feedBuffer);
	(void)glRenderMode(GL_FEEDBACK);	// SET IT IN FEEDBACK MODE

	tempParticle = g_CurrentSys;
	for (loop = 0; loop < g_ParticleCount; loop++)
	{
		// PASS THROUGH A MARKET LETTING ME KNOW WHAT VERTEX IT WAS
		glPassThrough((float)loop);
		// SEND THE VERTEX
		glBegin(GL_POINTS);
		glVertex3fv((float *)&tempParticle->pos.x);
		glEnd();
		tempParticle++;
	}
	hitCount = glRenderMode(GL_RENDER); // HOW MANY HITS DID I GET
	CompareBuffer(hitCount,feedBuffer,(float)x,(float)y);		// CHECK THE HIT 
	free(feedBuffer);		// GET RID OF THE MEMORY
}
////// GetNearestPoint ////////////////////////////////////////////////////////


// Velocity Threshold that decides between Static and Kinetic Friction
#define STATIC_THRESHOLD	0.03f				

void ComputeForces( t_Particle	*system)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Particle		*curPart,*restPart,*p1,*p2;
	tVector		contactN;
	float		FdotN,VdotN,Vmag;		
	tVector		Vn,Vt;				// CONTACT RESOLUTION IMPULSE
	t_Spring		*spring;
	float		dist, Hterm, Dterm;
	tVector		springForce,deltaV,deltaP;
///////////////////////////////////////////////////////////////////////////////

	restPart = g_GameSys[3];
	curPart = system;
	for (loop = 0; loop < CUBE_NODE_COUNT; loop++)
	{
		curPart->f.Set(0.0f,0.0f,0.0f);			// Clear Force Vector
		curPart->torque.Set(0.0f, 0.0f, 0.0f);	// Clear Torque Vector

		if (g_UseGravity && curPart->oneOverM != 0) // && curParticle->type != CONTACTING)
		{
			curPart->f += (g_Gravity / curPart->oneOverM);
		}

		if (g_UseDamping)
		{
			curPart->f += (curPart->v * (-g_Kd));
		}
		else
		{
			curPart->f += (curPart->v * (-DEFAULT_DAMPING));
		}


		// Handle Friction forces for  points in Contact with the a surface
		if (g_UseFriction)
		{
			// Calculate Fn 
			FdotN = g_Gravity.y / curPart->oneOverM;		// Gravity
			// Calculate Vt Velocity Tangent to Normal Plane
			contactN.Set(0.0f, 1.0f, 0.0f);
			VdotN = contactN.DotProduct(&curPart->v);
			Vn = contactN * VdotN;
			Vt = curPart->v - Vn;
			Vmag = Vt.SquaredLength();
			// Check if Velocity is faster then threshold
			if (Vmag > STATIC_THRESHOLD)		// Use Kinetic Friction model
			{
				Vt.NormalizeVector();
				Vt = Vt * (FdotN * g_Ckf);
				curPart->f += Vt;
			}
			else	// Use Static Friction Model
			{
				Vmag = Vmag / STATIC_THRESHOLD;
				Vt.NormalizeVector();
				Vt = Vt * (FdotN * g_Csf * Vmag);
				curPart->f += Vt;
			}
		}

		// Handle the volume preserving force
		// This uses the body reference frame to transform the rest points.
		// This tells me where the deformable vertex should be
		// then a spring pulls the vertex towards that full volume point
		tVector transformed_rest = restPart->pos;
		transformed_rest.MultVectorByMatrix(g_ObjectMatrix.m);
		deltaP = curPart->pos - transformed_rest;
		dist = deltaP.Length();					// Magnitude of deltaP
		if (dist > 0)
		{
			Hterm = dist * g_Keh;	// Ks * (dist - rest) rest distance is 0
			
			deltaV = curPart->v;		// Delta Velocity Vector
			Dterm = (deltaV.DotProduct(&deltaP) * g_Ked) / dist; // Damping Term
			
			springForce = deltaP * (1.0f / dist);	// Normalize Distance Vector
			springForce *= -(Hterm + Dterm);	// Calc Force
			curPart->f += springForce;			// Apply volume force to Particle
		}

		curPart++;
		restPart++;
	}

	// now do the regular springs for the mesh
	spring = g_Spring;
	for (loop = 0; loop < g_SpringCnt; loop++)
	{
		p1 = &system[spring->p1];
		p2 = &system[spring->p2];
		deltaP = p1->pos - p2->pos;
		dist = deltaP.Length();					// Magnitude of deltaP

		Hterm = (dist - spring->restLen) * spring->Ks;	// Ks * (dist - rest)
		
		deltaV = p1->v - p2->v;		// Delta Velocity Vector
		Dterm = (deltaV.DotProduct(&deltaP) * spring->Kd) / dist; // Damping Term
		
		springForce = deltaP * (1.0f / dist);	// Normalize Distance Vector
		springForce *= -(Hterm + Dterm);	// Calc Force
		p1->f += springForce;			// Apply to Particle 1
		p2->f -= springForce;			// - Force on Particle 2
		spring++;					// DO THE NEXT SPRING
	}

	// APPLY THE MOUSE DRAG FORCES IF THEY ARE ACTIVE
	if (g_MouseForceActive)
	{
		// APPLY TO EACH PICKED PARTICLE
		if (g_Pick[0] > -1)
		{
			p1 = &system[g_Pick[0]];
			deltaP = g_MouseDragPos[0] - p1->pos;
			dist = deltaP.Length();					// Magnitude of deltaP

			if (dist != 0.0f)
			{
				Hterm = (dist) * 30.8f;					// Ks * dist

				springForce = deltaP * (1.0f / dist);	// Normalize Distance Vector
				springForce *= -(Hterm);	// Calc Force
				p1->f -= springForce;			// Apply to Particle 1
			}

		}
		if (g_Pick[1] > -1)
		{
			p1 = &system[g_Pick[1]];
			deltaP = g_MouseDragPos[1] - p1->pos;
			dist = deltaP.Length();					// Magnitude of deltaP

			if (dist != 0.0f)
			{
				Hterm = (dist) * 30.8f;					// Ks * dist

				springForce = deltaP * (1.0f / dist);	// Normalize Distance Vector
				springForce *= -(Hterm);	// Calc Force
				p1->f -= springForce;			// Apply to Particle 1
			}
		}
	}
}   


///////////////////////////////////////////////////////////////////////////////
// Function:	IntegrateSysOverTime 
// Purpose:		Does the Integration for all the points in a system
// Arguments:	Initial Position, Source and Target Particle Systems and Time
// Notes:		Computes a single integration step
///////////////////////////////////////////////////////////////////////////////
void IntegrateSysOverTime(t_Particle *initial,t_Particle *source, t_Particle *target, float deltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	float deltaTimeMass;
///////////////////////////////////////////////////////////////////////////////
	for (loop = 0; loop < CUBE_NODE_COUNT; loop++)
	{
		deltaTimeMass = deltaTime * initial->oneOverM;
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v = initial->v + (source->f * deltaTimeMass);

		target->oneOverM = initial->oneOverM;

		// SET THE NEW POSITION
		target->pos = initial->pos + (source->v * deltaTime);

		initial++;
		source++;
		target++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	EulerIntegrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses Euler's method
///////////////////////////////////////////////////////////////////////////////
void EulerIntegrate( float DeltaTime)
{
	// JUST TAKE A SINGLE STEP
	IntegrateSysOverTime(g_CurrentSys,g_CurrentSys, g_TargetSys,DeltaTime);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	MidPointIntegrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses the Midpoint method
///////////////////////////////////////////////////////////////////////////////
void MidPointIntegrate( float DeltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float		halfDeltaT;
///////////////////////////////////////////////////////////////////////////////
	halfDeltaT = DeltaTime / 2.0f;

	// TAKE A HALF STEP AND UPDATE VELOCITY AND POSITION
	IntegrateSysOverTime(g_CurrentSys,g_CurrentSys,&g_GameSys[2][0],halfDeltaT);

	// COMPUTE FORCES USING THESE NEW POSITIONS AND VELOCITIES
	ComputeForces(&g_GameSys[2][0]);

	// TAKE THE FULL STEP WITH THIS NEW INFORMATION
	IntegrateSysOverTime(g_CurrentSys,&g_GameSys[2][0],g_TargetSys,DeltaTime);
}

int CheckForCollisions( t_Particle	*system )
{
/// Local Variables ///////////////////////////////////////////////////////////
	int collisionState = NOT_COLLIDING;
    float const depthEpsilon = 0.001f;
    float const partDepthEpsilon = 0.0001f;
	int loop,planeIndex;
	t_Particle *curPart;
	t_CollisionPlane *plane;
	float axbyczd,relativeVelocity;
///////////////////////////////////////////////////////////////////////////////

	g_ContactCnt = 0;		// THERE ARE CURRENTLY NO CONTACTS

	curPart = system;
	for (loop = 0; (loop < CUBE_NODE_COUNT) && (collisionState != PENETRATING); 
			loop++,curPart++)
	{
        for(planeIndex = 0;(planeIndex < g_CollisionPlaneCnt) &&
            (collisionState != PENETRATING);planeIndex++)
        {
			plane = &g_CollisionPlane[planeIndex];

            axbyczd = curPart->pos.DotProduct(&plane->normal) + plane->d;

			// Used to look for exact collisions.  Now use penalty method and just go.
            if(axbyczd < depthEpsilon)
            {
                relativeVelocity = plane->normal.DotProduct(&curPart->v);
				if(relativeVelocity < 0.0f)
                {
                    collisionState = COLLIDING_WITH_WALL;
					g_Contact[g_ContactCnt].type = COLLIDING_WITH_WALL;
					g_Contact[g_ContactCnt].particle = loop; 
					g_Contact[g_ContactCnt].Kr = g_Kr;		// Particle to wall
					g_Contact[g_ContactCnt].normal = plane->normal;
					g_Contact[g_ContactCnt].depth = axbyczd;
					g_ContactCnt++;
                }
            }
        }
	}

    return collisionState;
}

// Handle the contact resolution
void ResolveCollisions( t_Particle	*system )
{
	t_Contact	*contact;
	t_Particle		*part;		// THE PARTICLE COLLIDING
	float		VdotN;		
	tVector		Vn,Vt;				// CONTACT RESOLUTION IMPULSE
	int			loop;

	contact = g_Contact;
	for (loop = 0; loop < g_ContactCnt; loop++,contact++)
	{
		part = &system[contact->particle];
		// CALCULATE Vn
		VdotN = contact->normal.DotProduct(&part->v);
		Vn = contact->normal * VdotN;
		// CALCULATE Vt
		Vt = part->v - Vn;

		// Check if it was a collision with a wall or particle
		if (contact->type == COLLIDING_WITH_WALL)
		{
			// SCALE Vn BY COEFFICIENT OF RESTITUTION
			Vn *= contact->Kr;
			// SET THE VELOCITY TO BE THE NEW IMPULSE
			part->v = Vt - Vn;
		}

	}
}

void Simulate(float DeltaTime, BOOL running)
{
    float		CurrentTime = 0.0f;
    float		TargetTime = DeltaTime;
	t_Particle		*tempSys;
	int			collisionState;

    while(CurrentTime < DeltaTime)
    {
		if (running)
		{
			ComputeForces(g_CurrentSys);

			// IN ORDER TO MAKE THINGS RUN FASTER, I HAVE THIS LITTLE TRICK
			// IF THE SYSTEM IS DOING A BINARY SEARCH FOR THE COLLISION POINT,
			// I FORCE EULER'S METHOD ON IT. OTHERWISE, LET THE USER CHOOSE.
			// THIS DOESN'T SEEM TO EFFECT STABILITY EITHER WAY
			if (g_CollisionRootFinding)
			{
				EulerIntegrate(TargetTime-CurrentTime);
			}
			else
			{
				switch (g_IntegratorType)
				{
				case EULER_INTEGRATOR:
					EulerIntegrate(TargetTime-CurrentTime);
					break;
				case MIDPOINT_INTEGRATOR:
					MidPointIntegrate(TargetTime-CurrentTime);
					break;
				}
			}
		}

		collisionState = CheckForCollisions(g_TargetSys);

        // either colliding or clear
        if(collisionState == COLLIDING_WITH_WALL)
        {
            int Counter = 0;
            do
            {
                ResolveCollisions(g_TargetSys);
                Counter++;
            } while((CheckForCollisions(g_TargetSys) >=
                        COLLIDING_WITH_WALL) && (Counter < 500));

            assert(Counter < 500);
			g_CollisionRootFinding = FALSE;	// FOUND THE COLLISION POINT BACK TO NORMAL
        }

        // we made a successful step, so swap configurations
        // to "save" the data for the next step
        
		CurrentTime = TargetTime;
		TargetTime = DeltaTime;

		// SWAP MY TWO SYSTEM BUFFERS SO I CAN DO IT AGAIN
		tempSys = g_CurrentSys;
		g_CurrentSys = g_TargetSys;
		g_TargetSys = tempSys;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SetMouseForce 
// Purpose:		Allows the user to interact with selected points by dragging
// Arguments:	Delta distance from clicked point, local x and y axes
///////////////////////////////////////////////////////////////////////////////
void SetMouseForce(int deltaX,int deltaY, tVector *localX, tVector *localY)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tVector tempX,tempY;
///////////////////////////////////////////////////////////////////////////////
	tempX = *localX;
	tempX *= (float)(deltaX * 0.03f);
	tempY = *localY;
	tempY = tempY * (-(float)deltaY * 0.03f);
	if (g_Pick[0] > -1)
	{
		g_MouseDragPos[0] = g_CurrentSys[g_Pick[0]].pos + tempX;
		g_MouseDragPos[0] = g_MouseDragPos[0] + tempY;
	}
	if (g_Pick[1] > -1)
	{
		g_MouseDragPos[1] = g_CurrentSys[g_Pick[1]].pos + tempX;
		g_MouseDragPos[1] = g_MouseDragPos[1] + tempY;
	}
}
/// SetMouseForce /////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	SetFFDWeights 
// Purpose:		Approximate an FFD by setting up control weights
// Arguments:	Pointer to base mesh visual structure
///////////////////////////////////////////////////////////////////////////////
void SetFFDWeights(t_ToonVisual *visual)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tVector *vertex;
	int loop,cvLoop;
	float	XBasis[4], YBasis[4], ZBasis[4];
	float	u, v, w;
	float	*vertexWeight;
	int		px,py,pz;
///////////////////////////////////////////////////////////////////////////////
	// Allocate the space for all the weights
	visual->weightData = (float *)malloc(visual->vertexCnt * CUBE_NODE_COUNT * sizeof(float));
	vertex = visual->vertex;
	// Go through all the vertices
	for (loop = 0; loop < visual->vertexCnt; loop++, vertex++)
	{
		// Find where each vertex is within the FFD grid 
		// Effectively scales each vertex to 0-1
		u = (vertex->x - g_FFDmin.x)/(g_FFDmax.x - g_FFDmin.x);
		v = (vertex->y - g_FFDmin.y)/(g_FFDmax.y - g_FFDmin.y);
		w = (vertex->z - g_FFDmin.z)/(g_FFDmax.z - g_FFDmin.z);

		// X Bezier Basis Functions
		XBasis[0] = (1.0f - u) * (1.0f - u) * (1.0f - u);
		XBasis[1] = 3.0f * u * (1.0f - u) * (1.0f - u);
		XBasis[2] = 3.0f * u * u * (1.0f - u);;
		XBasis[3] = u * u * u;

		// Y Bezier Basis Functions
		YBasis[0] = (1.0f - v) * (1.0f - v) * (1.0f - v);
		YBasis[1] = 3.0f * v * (1.0f - v) * (1.0f - v);
		YBasis[2] = 3.0f * v * v * (1.0f - v);;
		YBasis[3] = v * v * v;

		// Z Bezier Basis Functions
		ZBasis[0] = (1.0f - w) * (1.0f - w) * (1.0f - w);
		ZBasis[1] = 3.0f * w * (1.0f - w) * (1.0f - w);
		ZBasis[2] = 3.0f * w * w * (1.0f - w);;
		ZBasis[3] = w * w * w;

		// Pointer to Place to store weight data
		vertexWeight = &visual->weightData[loop * 64];
		// Go through the control vertices
		for (cvLoop = 0; cvLoop < CUBE_NODE_COUNT; cvLoop++,vertexWeight++)
		{
			// Some quick math to find the component indices
			px = CUBE_WIDTH - (cvLoop % CUBE_WIDTH) - 1;
			py = CUBE_HEIGHT - (cvLoop / (CUBE_WIDTH * CUBE_HEIGHT)) - 1;
			pz = CUBE_DEPTH - ((cvLoop % (CUBE_WIDTH * CUBE_HEIGHT)) / CUBE_WIDTH) - 1;
			
			// set the vertex weight for this CV
			*vertexWeight = (XBasis[px] * YBasis[py] * ZBasis[pz]);
		}
	}
}
/// SetFFDWeights /////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	ComputeObjLocalFrame 
// Purpose:		Computes a local transformation matrix for a deformable model
// Notes:		Makes use of vector structure for most operations
//				Assumes a relatively square boundary.  Works by averaging the sides
///////////////////////////////////////////////////////////////////////////////
void ComputeObjLocalFrame(tMatrix *obj_matrix)
{
/// Local Variables ///////////////////////////////////////////////////////////
    int i;
	tVector vObjectCenter;
	tVector vX_Axis,vY_Axis,vZ_Axis;
	tVector vX_Min,vX_Max,vY_Min,vY_Max;
///////////////////////////////////////////////////////////////////////////////
    // Compute the center of the object by averaging
    for (i = 0; i < g_ParticleCount; i++)
        vObjectCenter += g_CurrentSys[i].pos;
    vObjectCenter /= g_ParticleCount;	// Divide by count to get average

    // Compute the min and max of the X axis
    for (i = 0; i < CUBE_DEPTH * CUBE_HEIGHT; i++)
	{
        vX_Max += g_CurrentSys[i * CUBE_WIDTH].pos;
        vX_Min += g_CurrentSys[(i * CUBE_WIDTH) + (CUBE_WIDTH - 1)].pos;
	}
    vX_Max /= CUBE_DEPTH * CUBE_HEIGHT;	// Average the side
    vX_Min /= CUBE_DEPTH * CUBE_HEIGHT;

    // Compute the min and max of the Y axis
    for (i = 0; i < CUBE_WIDTH * CUBE_DEPTH; i++)
	{
        vY_Max += g_CurrentSys[i].pos;
        vY_Min += g_CurrentSys[i + (CUBE_WIDTH * CUBE_DEPTH * (CUBE_HEIGHT - 1))].pos;
	}
    vY_Max /= CUBE_WIDTH * CUBE_DEPTH;	// Average the side
    vY_Min /= CUBE_WIDTH * CUBE_DEPTH;

	vX_Axis = vX_Max - vX_Min;	// Calculate X-axis vector
	vX_Axis.NormalizeVector();

	vY_Axis = vY_Max - vY_Min;	// Calculate Y-axis vector
	vY_Axis.NormalizeVector();

	// Use Cross product to get the Z axis
	vZ_Axis.CrossProduct(&vX_Axis, &vY_Axis);

	// Make certain the X is orthogonal using Cross product
	vX_Axis.CrossProduct(&vY_Axis, &vZ_Axis);

	// Create the OpenGL ready matrix
	IdentityMatrix(obj_matrix);	// Reset it
	// X-Axis
	obj_matrix->m[0] = vX_Axis.x;
	obj_matrix->m[1] = vX_Axis.y;
	obj_matrix->m[2] = vX_Axis.z;

	// Y-Axis
	obj_matrix->m[4] = vY_Axis.x;
	obj_matrix->m[5] = vY_Axis.y;
	obj_matrix->m[6] = vY_Axis.z;

	// Z-Axis
	obj_matrix->m[8] = vZ_Axis.x;
	obj_matrix->m[9] = vZ_Axis.y;
	obj_matrix->m[10] = vZ_Axis.z;

	// Set the Center position
	obj_matrix->m[12] = vObjectCenter.x;
	obj_matrix->m[13] = vObjectCenter.y;
	obj_matrix->m[14] = vObjectCenter.z;
}

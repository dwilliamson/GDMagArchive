///////////////////////////////////////////////////////////////////////////////
//
// Spring Simulation
//
// Created:
//		JL 7/10/2002
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2002 Darwin 3D, LLC., All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>	// Normal Windows stuff
#include <assert.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <math.h>
#include <stdio.h>
#include "externs.h"	// Data shared between files

// Forward Declarations
void			SetupWorld();
float			g_Kd;
float			g_Kr;	// Particle to Wall Coefficient of Restitution
float			g_Ksh;
float			g_Ksd;
float			g_YoungModulus;						// Young's Modulus
float			g_FractureThreshold;				// Point of breaking
int				g_IntegratorType;
int				g_MouseForceActive = FALSE;	
int				g_UseDamping = TRUE;				// Use a Damping force
int				g_UseFriction = TRUE;				// Use Friction
int				g_UseGravity = TRUE;				// Use Gravity
int				g_DrawSprings = TRUE;				// Draw tension springs
int				g_DrawCVs = TRUE;					// Draw vertices
int				g_GridSnap = FALSE;
int				g_Pick[2];
float			g_GridStep_X;
float			g_GridStart_X;
float			g_WorldSizeX,g_WorldSizeY,g_WorldSizeZ;
CVector			g_MouseDragPos[2];

// Physical elements in the simulation
t_Contact		g_Contact[MAX_CONTACTS];			// LIST OF POSSIBLE COLLISIONS
int				g_ContactCnt;						// COLLISION COUNT
t_Particle			*g_GameSys[SYSTEM_COUNT];			// LIST OF PHYSICAL PARTICLES
t_Particle			*g_CurrentSys,*g_TargetSys;
int				g_ParticleCount;
CVector			g_Ground[GROUND_POINTS];
t_Spring		*g_Spring;				// VALID SPRINGS IN SYSTEM
int				g_SpringCnt;		
CVector			vGravity;

DWORD			g_Time;					// Store the base system time in ticks
DWORD			g_CurTime;				// Store the current system time in ticks
float			g_Hour;					// Convenience time in hours

///////////////////////////////////////////////////////////////////////////////
// Initialize the world and the objects in it.
BOOL InitSim(void)
{
	g_Pick[0] = -1;
	g_Pick[1] = -1;

	g_Kd	= 0.8f;	// DAMPING FACTOR
	g_Kr	= 0.6f;		// 1.0 = SUPERBALL BOUNCE 0.0 = DEAD WEIGHT
	g_Ksh	= 8.0f;		// HOOK'S SPRING CONSTANT
	g_Ksd	= 0.9f;		// SPRING DAMPING CONSTANT

	g_YoungModulus = 10.0f;	// Young's Modulus, Stiffness of stress check
	g_FractureThreshold = 0.7f;

	vGravity = CVector(0.0f, -0.5f, 0.0f);	// Feet per second low because of surface module

	// Pick an Integrator.  Either seem to work fine for this app
	// Euler is faster so....
	g_IntegratorType = PC_INTEGRATOR;
//	g_IntegratorType = MIDPOINT_INTEGRATOR;
//	g_IntegratorType = EULER_INTEGRATOR;

	g_ContactCnt = 0;
	g_SpringCnt = 0;

	// CREATE THE SIZE FOR THE SIMULATION WORLD
	g_WorldSizeX = 30.0f;
	g_WorldSizeY = 30.0f;
	g_WorldSizeZ = 30.0f;

	// Get the initial millisecond clock
	g_Time = timeGetTime();

	g_ParticleCount = 0;
	g_GridStep_X = 1.0f;

	// Allocate space for the system
	for (int i = 0; i < SYSTEM_COUNT; i++)
		g_GameSys[i] = (t_Particle *)malloc(sizeof(t_Particle) * MAX_PARTICLES);

	SetupWorld();

	g_CurrentSys = g_GameSys[0];
	g_TargetSys = g_GameSys[1];
	
	return TRUE;
}

void FreeSim(void)
{
	for (int i = 0; i < SYSTEM_COUNT; i++)
		free(g_GameSys[i]);

	if (g_SpringCnt > 0)
	{
		free(g_Spring);
	}
}

void AddSpring(int p1, int p2)
{
	t_Spring		*tempSpring;

	tempSpring = (t_Spring *)malloc(sizeof(t_Spring) * (g_SpringCnt + 1));
	if (g_SpringCnt > 0)
	{
		memcpy(tempSpring,g_Spring,sizeof(t_Spring) * g_SpringCnt);
		free(g_Spring);
	}
	g_Spring = tempSpring;
	tempSpring = &g_Spring[g_SpringCnt];
	g_SpringCnt = g_SpringCnt + 1;
	tempSpring->Ks = g_Ksh;
	tempSpring->Kd = g_Ksd;
	tempSpring->p1 = p1;
	tempSpring->p2 = p2;
	tempSpring->restLen = CVector(g_CurrentSys[p1].pos  - g_CurrentSys[p2].pos).Length();
	tempSpring->active = TRUE;
	tempSpring->stress = 0.0f;
}

void DeleteSpring(int which)
{
	if (which < g_SpringCnt - 1)
	{
		memcpy(&g_Spring[which],&g_Spring[which + 1],sizeof(t_Spring) * (g_SpringCnt - which - 1));
	}
	g_SpringCnt--;
}

void SetupWorld()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop2;
///////////////////////////////////////////////////////////////////////////////
	g_ParticleCount = 0;
	g_GridStep_X = g_WorldSizeX / (float)(GROUND_POINTS - 1);
	g_GridStart_X = (-g_WorldSizeX/2);

	// Initialize the Ground
	for (loop2 = 0; loop2 < GROUND_POINTS; loop2++)
	{	
		g_Ground[loop2] = CVector((-g_WorldSizeX/2) + (loop2 * g_GridStep_X),-7.0f,0.0f );
	}

	g_SpringCnt = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	ComputeStress
// Purpose:		Compute the stress value on a spring using Green Strain
// Arguments:	Spring to check
///////////////////////////////////////////////////////////////////////////////
float ComputeStress(t_Spring *spring)
{
	CVector deltaP;
	t_Particle *p1 = &g_CurrentSys[spring->p1];
	t_Particle *p2 = &g_CurrentSys[spring->p2];
	deltaP = p1->pos - p2->pos;	// Vector distance 
	float l = deltaP.Length();					// Magnitude of deltaP
	// Compute Green Strain
	float stress = ((l * l) - (spring->restLen * spring->restLen))/(2 * (spring->restLen * spring->restLen));
	return stress * g_YoungModulus;	// return Stress value
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CheckSpringFracture
// Purpose:		Check if each spring is under too much stress and should break
///////////////////////////////////////////////////////////////////////////////
void CheckSpringFracture(t_Spring *spring)
{
	// Calculate stress.  Positive is tension, negative is compression
	spring->stress = ComputeStress(spring);
	if (fabsf(spring->stress) > g_FractureThreshold)
		spring->active = FALSE;
}


void DrawSimWorld()
{
	t_Particle		*tempParticle;
	t_Spring		*tempSpring;
	int				loop;

	if (g_CurrentSys)
	{
		g_CurTime = timeGetTime() - g_Time;

    // draw ground floor
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glBegin(GL_QUADS);
		glColor3f(0.6f,0.2f,0.0f);
		for (loop = 0; loop < GROUND_POINTS - 1; loop++)
		{	
			glVertex3fv((float *)&g_Ground[loop].x);
			glVertex3fv((float *)&g_Ground[loop+1].x);
			glVertex3f(g_Ground[loop+1].x,g_Ground[loop+1].y - 30.0f,g_Ground[loop+1].z);
			glVertex3f(g_Ground[loop].x,g_Ground[loop].y - 30.0f,g_Ground[loop].z);
		}
		glEnd();
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		if (g_Spring && g_DrawSprings)
		{
			glBegin(GL_LINES);
			glColor3f(0.0f,0.8f,0.8f);
			tempSpring = g_Spring;
			for (loop = 0; loop < g_SpringCnt; loop++,tempSpring++)
			{
				if (tempSpring->active) {
					// Compute the stress on each spring and display it by color
					float stress = min(1.0f,fabsf(tempSpring->stress));
					glColor3f(1.0f,1.0f - fabsf(stress),1.0f - fabsf(stress));
					glVertex3fv((float *)&g_CurrentSys[tempSpring->p1].pos.x);
					glVertex3fv((float *)&g_CurrentSys[tempSpring->p2].pos.x);
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
				if (loop == g_Pick[0])
					glColor3f(0.0f,0.8f,0.0f);
				else if (loop == g_Pick[1])
					glColor3f(0.8f,0.0f,0.0f);
				// If particles are in contact, Draw them in Orange
				else if ((tempParticle->flags & COLLIDING_WITH_GROUND) && g_UseFriction)
					glColor3f(1.0f,0.5f,0.0f);
				// Normally Yellow
				else
					glColor3f(0.8f,0.0f,0.8f);

				glVertex3fv((float *)&tempParticle->pos.x);
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
	GLfloat token,point[3],dist;
	int loop,currentVertex,result = -1;
	GLfloat nearest = 99999.0f;
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
			g_Pick[1] = g_Pick[0];
			g_Pick[0] = result;
		}
	} else {
		g_Pick[0] = -1;
		g_Pick[1] = -1;
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

///////////////////////////////////////////////////////////////////////////////
// Function:	ComputeForces 
// Purpose:		Computes forces acting on a mesh control point
// Arguments:	Particle Systems
///////////////////////////////////////////////////////////////////////////////
void ComputeForces( t_Particle	*system)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	t_Particle		*curPart,*p1,*p2;
	CVector		contactN;
	CVector		Vn,Vt;				// CONTACT RESOLUTION IMPULSE
	t_Spring		*spring;
	float		dist, Hterm, Dterm;
	CVector		springForce,deltaV,deltaP;
///////////////////////////////////////////////////////////////////////////////

	curPart = system;
	for (loop = 0; loop < g_ParticleCount; loop++, curPart++)
	{
		curPart->f_old = curPart->f;				// store old force for predictor corrector
		curPart->f = CVector(0.0f,0.0f,0.0f);		// Clear Force Vector
		curPart->flags = 0;

		if (g_UseGravity && curPart->oneOverM != 0) // && curParticle->type != CONTACTING)
		{
			curPart->f += (vGravity / curPart->oneOverM);
		}

		if (g_UseDamping)
		{
			curPart->f += (curPart->v * (-g_Kd));
		}
		else
		{
			curPart->f += (curPart->v * (-DEFAULT_DAMPING));
		}
	}

	// NOW DO ALL THE SPRINGS
	spring = g_Spring;
	for (loop = 0; loop < g_SpringCnt; loop++)
	{
		if (spring->active) {
			p1 = &system[spring->p1];
			p2 = &system[spring->p2];
			deltaP = p1->pos  - p2->pos;					// Vector distance 
			dist = deltaP.Length();							// Magnitude of deltaP

			Hterm = (dist - spring->restLen) * spring->Ks;	// Ks * (dist - rest)
			
			deltaV = p1->v - p2->v;							// Delta Velocity Vector
			Dterm = (deltaV.Dot(&deltaP) * spring->Kd) / dist; // Damping Term
			
			springForce = deltaP * (1.0f / dist);			// Normalize Distance Vector
			springForce = springForce * -(Hterm + Dterm);	// Calc Force
			p1->f += springForce;							// Apply to Particle 1
			p2->f -= springForce;							// - Force on Particle 2
		}
		spring++;										// DO THE NEXT SPRING
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
	for (loop = 0; loop < g_ParticleCount; loop++)
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
// Function:	CreshawIntegrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses an predictor-corrector method by Crenshaw
///////////////////////////////////////////////////////////////////////////////
void CrenshawIntegrate( float deltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	float deltaTimeMass;
	t_Particle *initial;
	t_Particle *source;
	t_Particle *target;
///////////////////////////////////////////////////////////////////////////////
	initial = g_CurrentSys;
	source = g_CurrentSys;
	target = g_TargetSys;

	for (loop = 0; loop < g_ParticleCount; loop++)
	{
		deltaTimeMass = deltaTime * initial->oneOverM * 0.5f;
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v = initial->v + (((source->f * 3.0f) - source->f_old) * deltaTimeMass);

		target->oneOverM = initial->oneOverM;

		// SET THE NEW POSITION
		target->pos = initial->pos + ((target->v  + source->v) * 0.5f * deltaTimeMass);

		initial++;
		source++;
		target++;
	}
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


///////////////////////////////////////////////////////////////////////////////
// Function:	GetGroundDepth 
// Purpose:		Check the Ground depth
// Arguments:	Position to test
// Returns:		Depth of ground at the test point
///////////////////////////////////////////////////////////////////////////////
float GetGroundDepth(CVector *pos)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int f1, f2;
	float mod,y ;
///////////////////////////////////////////////////////////////////////////////
	f1 = (int)((pos->x - g_GridStart_X) / g_GridStep_X);
	if (f1 < 0) f1 = 0;
	f2 = f1 + 1;
	if (f2 == GROUND_POINTS)
	{
		y = g_Ground[f1].y;
	}
	else
	{
		mod = fmodf((pos->x - g_GridStart_X), g_GridStep_X);
		y  = (g_Ground[f1].y * (1.0f - mod)) + (g_Ground[f2].y * (mod));
	}
	return (pos->y - y);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CheckGroundCollision 
// Purpose:		Check for a collision with the ground
// Arguments:	Position to test
// Returns:		Collision normal and distance from ground
///////////////////////////////////////////////////////////////////////////////
BOOL CheckGroundCollision(CVector *pos,CVector *normal, float *dist)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int f1, f2;
	float mod,y ;
///////////////////////////////////////////////////////////////////////////////
	f1 = (int)((pos->x - g_GridStart_X) / g_GridStep_X);
	f2 = f1 + 1;
	if (f2 == GROUND_POINTS)
	{
		f2--;
		// Last point, set normal to straight up
		*normal = CVector(0.0f, 1.0f, 0.0f);
		y = g_Ground[f1].y;
		if (y > pos->y)
		{
			pos->y = y;
		}
	}
	else
	{
		mod = fmodf((pos->x - g_GridStart_X), g_GridStep_X);
		y  = (g_Ground[f1].y * (1.0f - mod)) + (g_Ground[f2].y * (mod));
		// Collision
		if (y > pos->y)
		{
			pos->y = y;
			*normal = (g_Ground[f2] - g_Ground[f1]);
			float x = normal->x;
			// Get the perp normal
			normal->x = -normal->y;
			normal->y = x;
		}
	}
	*dist = fabsf(pos->y - y);
	if (y > pos->y) return TRUE;
	else return FALSE;

}

int CheckForCollisions( t_Particle	*system )
{
/// Local Variables ///////////////////////////////////////////////////////////
	int collisionState = NOT_COLLIDING;
	int loop;
	t_Particle *curPart;
	CVector vCollisionNormal;
	float dist;
///////////////////////////////////////////////////////////////////////////////

	g_ContactCnt = 0;		// THERE ARE CURRENTLY NO CONTACTS

	curPart = system;
	for (loop = 0; (loop < g_ParticleCount);loop++,curPart++)
	{
		if (curPart->oneOverM > 0.0f)
		{
			if (CheckGroundCollision(&curPart->pos,&vCollisionNormal,&dist))
			{
				collisionState = COLLIDING_WITH_GROUND;
	//			curPart->pos.y = y;
	//			curPart->v.Set(0,0,0);
				g_Contact[g_ContactCnt].type = COLLIDING_WITH_GROUND;
				g_Contact[g_ContactCnt].particle = loop; 
				g_Contact[g_ContactCnt].Kr = g_Kr;		// Particle to Ground
				g_Contact[g_ContactCnt].normal = vCollisionNormal;
				g_ContactCnt++;
			}

			if (dist < 0.02f)
				curPart->flags |= COLLIDING_WITH_GROUND;
			else
				curPart->flags &= ~COLLIDING_WITH_GROUND;
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
	CVector		Vn,Vt;				// CONTACT RESOLUTION IMPULSE
	int			loop;

	contact = g_Contact;
	for (loop = 0; loop < g_ContactCnt; loop++,contact++)
	{
		part = &system[contact->particle];
		// CALCULATE Vn
		VdotN = contact->normal.Dot(&part->v);
		Vn = contact->normal * VdotN;
		// CALCULATE Vt
		Vt = part->v - Vn;

		// Check if it was a collision with a wall or particle
		if (contact->type == COLLIDING_WITH_GROUND)
		{
			// SCALE Vn BY COEFFICIENT OF RESTITUTION
			Vn = Vn * contact->Kr;
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

	if (running)
	{
		// Calculate the particle positions
		ComputeForces(g_CurrentSys);

		switch (g_IntegratorType)
		{
		case EULER_INTEGRATOR:
			EulerIntegrate(TargetTime-CurrentTime);
			break;
		case MIDPOINT_INTEGRATOR:
			MidPointIntegrate(TargetTime-CurrentTime);
			break;
		case PC_INTEGRATOR:
			CrenshawIntegrate(TargetTime-CurrentTime);
			break;
		}

	}

	collisionState = CheckForCollisions(g_TargetSys);

    // either colliding or clear
    if(collisionState == COLLIDING_WITH_GROUND)
    {
        ResolveCollisions(g_TargetSys);
    }

	t_Spring *spring = g_Spring;
	for (int i = 0; i < g_SpringCnt; i++,spring++)
		CheckSpringFracture(spring);

    // we made a successful step, so swap configurations
    // to "save" the data for the next step

	CurrentTime = TargetTime;
	TargetTime = DeltaTime;

	// SWAP MY TWO SYSTEM BUFFERS SO I CAN DO IT AGAIN
	tempSys = g_CurrentSys;
	g_CurrentSys = g_TargetSys;
	g_TargetSys = tempSys;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	AddParticle 
// Purpose:		Allows the user to add particle
// Arguments:	x,y point
///////////////////////////////////////////////////////////////////////////////
void AddParticle(int hitX, int hitY)
{
	double modelMatrix[16],projMatrix[16];
	int viewport[4];

	double picked[3];

	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);

	glPushMatrix();

	// Set root skeleton's orientation and position
	glTranslatef(-g_POV.trans.x, -g_POV.trans.y, -g_POV.trans.z);

	glRotatef(g_POV.rot.z, 0.0f, 0.0f, 1.0f);
	glRotatef(g_POV.rot.x, 1.0f, 0.0f, 0.0f); 
	glRotatef(g_POV.rot.y, 0.0f, 1.0f, 0.0f);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);

	glPopMatrix();

	// Get the place the mouse is clicking by using GL_UNPROJECT
	glGetIntegerv(GL_VIEWPORT,viewport);

	gluProject(0.0, 0.0, 0.0, modelMatrix, projMatrix, viewport, &picked[0],&picked[1],&picked[2]);
	gluUnProject((double)hitX, (double)hitY,picked[2], modelMatrix, projMatrix, viewport, &picked[0],&picked[1],&picked[2]);

	for (int loop = 0; loop < SYSTEM_COUNT; loop++)
	{
		t_Particle *part = &g_GameSys[loop][g_ParticleCount];
		part->pos = CVector((float)picked[0],(float)picked[1],0.0f );
		part->rest_pos = part->pos;
		part->v = CVector(0.0f, 0.0f, 0.0f);
		part->f = CVector(0.0f, 0.0f, 0.0f);
		// points added below the ground are considered anchors and have infinite mass
		if (GetGroundDepth(&part->pos) < 0.0f)
			part->oneOverM = 0.0f;
		else
			part->oneOverM = 1.0f;
		part->flags = 0;
	}

	g_Pick[1] = g_Pick[0];
	g_Pick[0] = g_ParticleCount;
	g_ParticleCount++;

}

///////////////////////////////////////////////////////////////////////////////
// Function:	ResetSim 
// Purpose:		Resets the sim to starting point
///////////////////////////////////////////////////////////////////////////////
void ResetSim()
{
	for (int loop = 0; loop < SYSTEM_COUNT; loop++)
	{
		for (int i = 0; i < g_ParticleCount; i++)
		{
			t_Particle *part = &g_GameSys[loop][i];
			part->pos = part->rest_pos;
			part->v = CVector(0.0f, 0.0f, 0.0f);
			part->f = CVector(0.0f, 0.0f, 0.0f);
			part->flags = 0;
		}
	}

	t_Spring *spring = g_Spring;
	for (loop = 0; loop < g_SpringCnt; loop++, spring++) {
		spring->active = TRUE;
		spring->stress = 0.0f;
		spring->Ks = g_Ksh;
		spring->Kd = g_Ksd;
	}

	g_ContactCnt = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SetMouseForce 
// Purpose:		Allows the user to interact with selected points by dragging
// Arguments:	Delta distance from clicked point, local x and y axes
///////////////////////////////////////////////////////////////////////////////
void SetMouseForce(int deltaX,int deltaY, CVector *localX, CVector *localY)
{
/// Local Variables ///////////////////////////////////////////////////////////
	CVector tempX,tempY;
///////////////////////////////////////////////////////////////////////////////
	tempX = *localX;
	tempX = tempX * ((float)deltaX * 0.03f);
	tempX.z = 0.0f;
	tempY = *localY;
	tempY = tempY * (-(float)deltaY * 0.03f);
	tempY.z = 0.0f;
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


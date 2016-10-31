///////////////////////////////////////////////////////////////////////////////
//
// PhysEnv.cpp : Physical World implementation file
//
// Purpose:	Implementation of Particle Physics System
//
// Created:
//		JL 12/1/98		
// Modified:
//		JL 3/6/99 - FIXED GRAVITY FORCE CALCULATION BUG
//		JL 3/8/99 - ADDED MORE POSSIBLE CONTACTS AS EACH VERTEX CAN CONTACT MORE 
//					THEN ONE COLLISION SURFACE (SHOULD IT BE DYNAMICALLY ALLOC'ED?)
//		JL 3/20/99 - ADDED THE MIDPOINT AND RK INTEGRATOR NEEDED TO ALLOC 5 TEMP PARTICLE ARRAYS
//		JL 6/01/99 - Modified for the Friction code with a Contact model 
//						Friction is added int the Apply forces code
//						Contact is determined in CheckForCollisions
//						World Damping was reduced so friction is more evident
//		JL 6/12/00 - Modified for the Dynamic Muscle System
//
// Notes:	A bit of this along with the organization comes from Chris Hecker's
//			Physics Articles from last year.  Hopefully this will get everyone
//			back up to speed before we dig deeper into the world of Dynamics.
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 2000 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <assert.h>
#include <math.h>

#include "Muscles.h"
#include "PhysEnv.h"
#include "SimProps.h"
#include "VertMass.h"
#include "MusDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning (disable:4244)      // I NEED TO CONVERT FROM DOUBLE TO FLOAT

/////////////////////////////////////////////////////////////////////////////
// CPhysEnv

// INITIALIZE THE SIMULATION WORLD
CPhysEnv::CPhysEnv()
{
	m_Pick[0] = -1;
	m_Pick[1] = -1;
	m_ParticleSys[0] = NULL;
	m_ParticleSys[1] = NULL;
	m_ParticleSys[2] = NULL;	// RESET BUFFER

	// THESE TEMP PARTICLE BUFFERS ARE NEEDED FOR THE MIDPOINT AND RK4 INTEGRATOR
	for (int i = 0; i < 5; i++)
		m_TempSys[i] = NULL;
	m_ParticleCnt = 0;
	m_Contact = NULL;
	m_Spring = NULL;
	m_SpringCnt = 0;		

	m_UseGravity = FALSE;
	m_UseFriction = TRUE;
	m_DrawSprings = TRUE;
	m_DrawMuscles = TRUE;
	m_DrawVertices	= TRUE;
	m_MouseForceActive = FALSE;
	m_MusclesFlexin = TRUE;

	MAKEVECTOR(m_Gravity, 0.0f, -0.2f, 0.0f)
	m_UserForceMag = 100.0;
	m_UserForceActive = FALSE;
	m_Kd	= 0.02f;	// DAMPING FACTOR
	m_Kr	= 0.8f;		// 1.0 = SUPERBALL BOUNCE 0.0 = DEAD WEIGHT
	m_Ksh	= 5.0f;		// HOOK'S SPRING CONSTANT
	m_Ksd	= 0.1f;		// SPRING DAMPING CONSTANT

	m_Csf	= 0.9f;		// Default Static Friction
	m_Ckf	= 0.7f;	// Default Kinetic Friction
	m_MouseForceKs = 0.08f;	// MOUSE SPRING CONSTANT

	// CREATE THE SIZE FOR THE SIMULATION WORLD
	m_WorldSizeX = 25.0f;
	m_WorldSizeY = 15.0f;
	m_WorldSizeZ = 25.0f;

	m_IntegratorType = EULER_INTEGRATOR;

	m_CollisionPlane = (tCollisionPlane	*)malloc(sizeof(tCollisionPlane) * 6);
	m_CollisionPlaneCnt = 6;

	// MAKE THE TOP PLANE (CEILING)
	MAKEVECTOR(m_CollisionPlane[0].normal,0.0f, -1.0f, 0.0f)
	m_CollisionPlane[0].d = m_WorldSizeY / 2.0f;

	// MAKE THE BOTTOM PLANE (FLOOR)
	MAKEVECTOR(m_CollisionPlane[1].normal,0.0f, 1.0f, 0.0f)
	m_CollisionPlane[1].d = m_WorldSizeY / 2.0f;

	// MAKE THE LEFT PLANE
	MAKEVECTOR(m_CollisionPlane[2].normal,-1.0f, 0.0f, 0.0f)
	m_CollisionPlane[2].d = m_WorldSizeX / 2.0f;

	// MAKE THE RIGHT PLANE
	MAKEVECTOR(m_CollisionPlane[3].normal,1.0f, 0.0f, 0.0f)
	m_CollisionPlane[3].d = m_WorldSizeX / 2.0f;

	// MAKE THE FRONT PLANE
	MAKEVECTOR(m_CollisionPlane[4].normal,0.0f, 0.0f, -1.0f)
	m_CollisionPlane[4].d = m_WorldSizeZ / 2.0f;

	// MAKE THE BACK PLANE
	MAKEVECTOR(m_CollisionPlane[5].normal,0.0f, 0.0f, 1.0f)
	m_CollisionPlane[5].d = m_WorldSizeZ / 2.0f;

	m_CollisionRootFinding = FALSE;		// ONLY SET WHEN FINDING A COLLISION

	// Make a little table of sin values to speed cycling of muscles
	float angle = 0.0f; 
	for (i = 0; i < 256; i++, angle += (M_PI / 256.0f))
	{
		m_SinTable[i] = sin(angle);
	}

	m_FrameCnt = 0;
}

CPhysEnv::~CPhysEnv()
{
	if (m_ParticleSys[0])
		free(m_ParticleSys[0]);
	if (m_ParticleSys[1])
		free(m_ParticleSys[1]);
	if (m_ParticleSys[2])
		free(m_ParticleSys[2]);
	for (int i = 0; i < 5; i++)
	{
		if (m_TempSys[i])
			free(m_TempSys[i]);
	}
	if (m_Contact)
		free(m_Contact);
	if (m_Spring)
		free(m_Spring);
	free(m_CollisionPlane);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	RenderWorld
// Purpose:		Draw the current system (particles, springs, userforces)
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::RenderWorld(tVector *vertices)
{
	tParticle	*tempParticle;
	tSpring		*tempSpring;
	tVector		*tempVertex;
	// FIRST DRAW THE WORLD CONTAINER  
	glColor3f(1.0f,1.0f,1.0f);

// JL - I DON'T WANT TO SEE THE WALLS RIGHT NOW
/*    // do a big linestrip to get most of edges
    glBegin(GL_LINE_STRIP);
        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
    glEnd();
    // fill in the stragglers
    glBegin(GL_LINES);
        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);

        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);

        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
    glEnd();
  */  
    // draw floor
    glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);
        glColor3f(0.0f,0.0f,0.5f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
    glEnd();
    glEnable(GL_CULL_FACE);


	if (m_ParticleSys)
	{
		if (m_Spring && m_DrawSprings)
		{
			tempSpring = m_Spring;
			for (int loop = 0; loop < m_SpringCnt; loop++)
			{
				if (tempSpring->isMuscle)
				{
					glColor3f(0.8f,0.0f,0.8f);
				}
				else
					glColor3f(0.0f,0.8f,0.8f);
				if (!tempSpring->isMuscle || m_DrawMuscles)
				{
					glBegin(GL_LINES);
					glVertex3fv((float *)&m_CurrentSys[tempSpring->p1].pos);
					glVertex3fv((float *)&m_CurrentSys[tempSpring->p2].pos);
					glEnd();
				}
				tempSpring++;
			}
			if (m_MouseForceActive)	// DRAW MOUSESPRING FORCE
			{
				glBegin(GL_LINES);
				if (m_Pick[0] > -1)
				{
					glColor3f(0.8f,0.4f,0.4f);
					glVertex3fv((float *)&m_CurrentSys[m_Pick[0]].pos);
					glVertex3fv((float *)&m_MouseDragPos[0]);
				}
				if (m_Pick[1] > -1)
				{
					glColor3f(0.8f,0.4f,0.4f);
					glVertex3fv((float *)&m_CurrentSys[m_Pick[1]].pos);
					glVertex3fv((float *)&m_MouseDragPos[1]);
				}
				glEnd();
			}
		}
		if (m_DrawVertices)
		{
			glBegin(GL_POINTS);
			tempParticle = m_CurrentSys;
			tempVertex = vertices;
			for (int loop = 0; loop < m_ParticleCnt; loop++,tempVertex++, tempParticle++)
			{
				// Picked particles are green then red for 1 and 2
				if (loop == m_Pick[0])
					glColor3f(0.0f,0.8f,0.0f);
				else if (loop == m_Pick[1])
					glColor3f(0.8f,0.0f,0.0f);
				// If particles are in contact, Draw them in Orange
				else if (tempParticle->contacting && m_UseFriction)
					glColor3f(1.0f,0.5f,0.0f);
				// Normally Yellow
				else
					glColor3f(0.8f,0.8f,0.0f);

				glVertex3fv((float *)&tempParticle->pos);
				// Copy the particle position back to the mesh so it deforms
				if (vertices) memcpy(tempVertex,&tempParticle->pos,sizeof(float) * 3);
			}
			glEnd();
		}
		else if (vertices)	// Still update the vertices
		{
			tempParticle = m_CurrentSys;
			tempVertex = vertices;
			for (int loop = 0; loop < m_ParticleCnt; loop++,tempVertex++, tempParticle++)
			{
				memcpy(tempVertex,&tempParticle->pos,sizeof(float) * 3);
			}
		}
	}

}
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	GetNearestPoint
// Purpose:		Use OpenGL Feedback to find the closest point to a mouseclick
// Arguments:	Screen coordinates of the hit
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::GetNearestPoint(int x, int y)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float *feedBuffer;
	int hitCount;
	tParticle *tempParticle;
	int loop;
///////////////////////////////////////////////////////////////////////////////
	// INITIALIZE A PLACE TO PUT ALL THE FEEDBACK INFO (3 DATA, 1 TAG, 2 TOKENS)
	feedBuffer = (float *)malloc(sizeof(GLfloat) * m_ParticleCnt * 6);
	// TELL OPENGL ABOUT THE BUFFER
	glFeedbackBuffer(m_ParticleCnt * 6,GL_3D,feedBuffer);
	(void)glRenderMode(GL_FEEDBACK);	// SET IT IN FEEDBACK MODE

	tempParticle = m_CurrentSys;
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// PASS THROUGH A MARKET LETTING ME KNOW WHAT VERTEX IT WAS
		glPassThrough((float)loop);
		// SEND THE VERTEX
		glBegin(GL_POINTS);
		glVertex3fv((float *)&tempParticle->pos);
		glEnd();
		tempParticle++;
	}
	hitCount = glRenderMode(GL_RENDER); // HOW MANY HITS DID I GET
	CompareBuffer(hitCount,feedBuffer,(float)x,(float)y);		// CHECK THE HIT 
	free(feedBuffer);		// GET RID OF THE MEMORY
}
////// GetNearestPoint ////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	CompareBuffer
// Purpose:		Check the feedback buffer to see if anything is hit
// Arguments:	Number of hits, pointer to buffer, point to test
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::CompareBuffer(int size, float *buffer,float x, float y)
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
		if (m_Pick[0] == -1)
			m_Pick[0] = result;
		else if (m_Pick[1] == -1)
			m_Pick[1] = result;
		else
		{
			m_Pick[0] = result;
			m_Pick[1] = -1;
		}
	}
}
////// CompareBuffer //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	SetWorldParticles
// Purpose:		Inform the System of the particles under control
// Arguments:	List of vertices and count
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::SetWorldParticles(tVector *coords,int particleCnt)
{
	tParticle *tempParticle;

	if (m_ParticleSys[0])
		free(m_ParticleSys[0]);
	if (m_ParticleSys[1])
		free(m_ParticleSys[1]);
	if (m_ParticleSys[2])
		free(m_ParticleSys[2]);
	for (int i = 0; i < 5; i++)
	{
		if (m_TempSys[i])
			free(m_TempSys[i]);
	}
	if (m_Contact)
		free(m_Contact);
	// THE SYSTEM IS DOUBLE BUFFERED TO MAKE THINGS EASIER
	m_CurrentSys = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	m_TargetSys = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	m_ParticleSys[2] = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	for (i = 0; i < 5; i++)
	{
		m_TempSys[i] = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	}
	m_ParticleCnt = particleCnt;

	// MULTIPLIED PARTICLE COUNT * 2 SINCE THEY CAN COLLIDE WITH MULTIPLE WALLS
	m_Contact = (tContact *)malloc(sizeof(tContact) * particleCnt * 2);
	m_ContactCnt = 0;

	tempParticle = m_CurrentSys;
	for (int loop = 0; loop < particleCnt; loop++)
	{
		MAKEVECTOR(tempParticle->pos, coords->x, coords->y, coords->z)
		MAKEVECTOR(tempParticle->v, 0.0f, 0.0f, 0.0f)
		MAKEVECTOR(tempParticle->f, 0.0f, 0.0f, 0.0f)
		tempParticle->oneOverM = 1.0f;							// MASS OF 1
		tempParticle->contacting = FALSE;
		tempParticle++;
		coords++;
	}

	// COPY THE SYSTEM TO THE SECOND ONE ALSO
	memcpy(m_TargetSys,m_CurrentSys,sizeof(tParticle) * particleCnt);
	// COPY THE SYSTEM TO THE RESET BUFFER ALSO
	memcpy(m_ParticleSys[2],m_CurrentSys,sizeof(tParticle) * particleCnt);

	m_ParticleSys[0] = m_CurrentSys;
	m_ParticleSys[1] = m_TargetSys;
}
////// SetWorldParticles //////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	FreeSystem
// Purpose:		Remove all particles and clear it out
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::FreeSystem()
{
	m_Pick[0] = -1;
	m_Pick[1] = -1;
	if (m_ParticleSys[0])
	{
		m_ParticleSys[0] = NULL;
		free(m_ParticleSys[0]);
	}
	if (m_ParticleSys[1])
	{
		free(m_ParticleSys[1]);
		m_ParticleSys[1] = NULL;
	}
	if (m_ParticleSys[2])
	{
		free(m_ParticleSys[2]);
		m_ParticleSys[2] = NULL;	// RESET BUFFER
	}
	for (int i = 0; i < 5; i++)
	{
		if (m_TempSys[i])
		{
			free(m_TempSys[i]);
			m_TempSys[i] = NULL;	// RESET BUFFER
		}
	}
	if (m_Contact)
	{
		free(m_Contact);
		m_Contact = NULL;
	}
	if (m_Spring)
	{
		free(m_Spring);
		m_Spring = NULL;
	}
	m_SpringCnt = 0;	
	m_ParticleCnt = 0;
}
////// FreeSystem //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	LoadData
// Purpose:		Load a simulation system 
// Arguments:	File pointer
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::LoadData(FILE *fp)
{
	fread(&m_UseGravity,sizeof(BOOL),1,fp);
	fread(&m_UseDamping,sizeof(BOOL),1,fp);
	fread(&m_UserForceActive,sizeof(BOOL),1,fp);
	fread(&m_Gravity,sizeof(tVector),1,fp);
	fread(&m_UserForce,sizeof(tVector),1,fp);
	fread(&m_UserForceMag,sizeof(float),1,fp);
	fread(&m_Kd,sizeof(float),1,fp);
	fread(&m_Kr,sizeof(float),1,fp);
	fread(&m_Ksh,sizeof(float),1,fp);
	fread(&m_Ksd,sizeof(float),1,fp);
	fread(&m_ParticleCnt,sizeof(int),1,fp);
	m_CurrentSys = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	m_TargetSys = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	m_ParticleSys[2] = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	for (int i = 0; i < 5; i++)
	{
		m_TempSys[i] = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	}
	m_ParticleSys[0] = m_CurrentSys;
	m_ParticleSys[1] = m_TargetSys;
	m_Contact = (tContact *)malloc(sizeof(tContact) * m_ParticleCnt);
	fread(m_ParticleSys[0],sizeof(tParticle),m_ParticleCnt,fp);
	fread(m_ParticleSys[1],sizeof(tParticle),m_ParticleCnt,fp);
	fread(m_ParticleSys[2],sizeof(tParticle),m_ParticleCnt,fp);
	fread(&m_SpringCnt,sizeof(int),1,fp);
	m_Spring = (tSpring *)malloc(sizeof(tSpring) * (m_SpringCnt));
	fread(m_Spring,sizeof(tSpring),m_SpringCnt,fp);
	fread(m_Pick,sizeof(int),2,fp);
}
////// LoadData //////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	SaveData
// Purpose:		Save a simulation system 
// Arguments:	File pointer
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::SaveData(FILE *fp)
{
	fwrite(&m_UseGravity,sizeof(BOOL),1,fp);
	fwrite(&m_UseDamping,sizeof(BOOL),1,fp);
	fwrite(&m_UserForceActive,sizeof(BOOL),1,fp);
	fwrite(&m_Gravity,sizeof(tVector),1,fp);
	fwrite(&m_UserForce,sizeof(tVector),1,fp);
	fwrite(&m_UserForceMag,sizeof(float),1,fp);
	fwrite(&m_Kd,sizeof(float),1,fp);
	fwrite(&m_Kr,sizeof(float),1,fp);
	fwrite(&m_Ksh,sizeof(float),1,fp);
	fwrite(&m_Ksd,sizeof(float),1,fp);
	fwrite(&m_ParticleCnt,sizeof(int),1,fp);
	fwrite(m_ParticleSys[0],sizeof(tParticle),m_ParticleCnt,fp);
	fwrite(m_ParticleSys[1],sizeof(tParticle),m_ParticleCnt,fp);
	fwrite(m_ParticleSys[2],sizeof(tParticle),m_ParticleCnt,fp);
	fwrite(&m_SpringCnt,sizeof(int),1,fp);
	fwrite(m_Spring,sizeof(tSpring),m_SpringCnt,fp);
	fwrite(m_Pick,sizeof(int),2,fp);
}
////// SaveData //////////////////////////////////////////////////////////////

// RESET THE SIM TO INITIAL VALUES
void CPhysEnv::ResetWorld()
{
	tSpring		*tempSpring;
	memcpy(m_CurrentSys,m_ParticleSys[2],sizeof(tParticle) * m_ParticleCnt);
	memcpy(m_TargetSys,m_ParticleSys[2],sizeof(tParticle) * m_ParticleCnt);
//	m_FrameCnt = 0;
	if (m_Spring)
	{
		tempSpring = m_Spring;
		for (int loop = 0; loop < m_SpringCnt; loop++)		
		{
			if (tempSpring->isMuscle)
				tempSpring->restLen = tempSpring->origRestLen;
			tempSpring++;
		}
	}
}

void CPhysEnv::SetWorldProperties()
{
	CSimProps	dialog;
	dialog.m_CoefRest = m_Kr;
	dialog.m_Damping = m_Kd;
	dialog.m_GravX = m_Gravity.x;
	dialog.m_GravY = m_Gravity.y;
	dialog.m_GravZ = m_Gravity.z;
	dialog.m_SpringConst = m_Ksh;
	dialog.m_SpringDamp = m_Ksd;
	dialog.m_UserForceMag = m_UserForceMag;
	dialog.m_MouseSpring = m_MouseForceKs;
	// Set up the Friction Values
	dialog.m_CoefStaticFriction = m_Csf;
	dialog.m_CoefKineticFriction = m_Ckf;

	if (dialog.DoModal() == IDOK)
	{
		m_Kr = dialog.m_CoefRest;
		m_Kd = dialog.m_Damping;
		m_Gravity.x = dialog.m_GravX;
		m_Gravity.y = dialog.m_GravY;
		m_Gravity.z = dialog.m_GravZ;
		m_UserForceMag = dialog.m_UserForceMag;
		m_Ksh = dialog.m_SpringConst;
		m_Ksd = dialog.m_SpringDamp;
		for (int loop = 0; loop < m_SpringCnt; loop++)
		{
			m_Spring[loop].Ks = m_Ksh;
			m_Spring[loop].Kd = m_Ksd;
		}
		m_Csf = dialog.m_CoefStaticFriction;
		m_Ckf = dialog.m_CoefKineticFriction;

		// GET THE NEW MOUSESPRING FORCE
		m_MouseForceKs = dialog.m_MouseSpring;
	}
}


void CPhysEnv::SetVertexMass()
{
	CVertMass	dialog;
	dialog.m_VertexMass = m_CurrentSys[m_Pick[0]].oneOverM;
	if (dialog.DoModal() == IDOK)
	{
		m_ParticleSys[0][m_Pick[0]].oneOverM = dialog.m_VertexMass;
		m_ParticleSys[0][m_Pick[1]].oneOverM = dialog.m_VertexMass;
		m_ParticleSys[1][m_Pick[0]].oneOverM = dialog.m_VertexMass;
		m_ParticleSys[1][m_Pick[1]].oneOverM = dialog.m_VertexMass;
		m_ParticleSys[2][m_Pick[0]].oneOverM = dialog.m_VertexMass;
		m_ParticleSys[2][m_Pick[1]].oneOverM = dialog.m_VertexMass;
	}
}

void CPhysEnv::EditMuscle()
{
	tSpring		*tempSpring;
	CMusDlg	dialog;
	int loop,sel = -1;

	// Find out if a spring is selected by checking the tagged vertices
	if (m_ParticleSys)
	{
		tempSpring = m_Spring;
		for (loop = 0; loop < m_SpringCnt; loop++)
		{
			if ((tempSpring->p1 == m_Pick[0] && tempSpring->p2 == m_Pick[1]) ||
				(tempSpring->p2 == m_Pick[0] && tempSpring->p1 == m_Pick[1]))
			{
				sel = loop;
				break;
			}
			tempSpring++;
		}
	}

	// If there is a selected muscle/spring edit the settings
	if (sel > -1)
	{
		dialog.m_Spring_Ks = tempSpring->Ks;
		dialog.m_Spring_Kd = tempSpring->Kd;
		dialog.m_IsMuscle = tempSpring->isMuscle;
		dialog.m_ContractOff = tempSpring->cycOff;
		dialog.m_ContractPercent = tempSpring->contract;
		if (dialog.DoModal() == IDOK)
		{
			tempSpring->isMuscle = dialog.m_IsMuscle;
			if (tempSpring->isMuscle)
				tempSpring->origRestLen = tempSpring->restLen;			
			tempSpring->Ks = dialog.m_Spring_Ks;	// Set new spring constant			
			tempSpring->Kd = dialog.m_Spring_Kd;	// Set Damping constant
			tempSpring->cycOff = dialog.m_ContractOff;
			tempSpring->contract = dialog.m_ContractPercent;
		}
	}
}

void CPhysEnv::ApplyUserForce(tVector *force)
{
	ScaleVector(force,  m_UserForceMag, &m_UserForce);
	m_UserForceActive = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SetMouseForce 
// Purpose:		Allows the user to interact with selected points by dragging
// Arguments:	Delta distance from clicked point, local x and y axes
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::SetMouseForce(int deltaX,int deltaY, tVector *localX, tVector *localY)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tVector tempX,tempY;
///////////////////////////////////////////////////////////////////////////////
	ScaleVector(localX,  (float)deltaX * 0.03f, &tempX);
	ScaleVector(localY,  -(float)deltaY * 0.03f, &tempY);
	if (m_Pick[0] > -1)
	{
		VectorSum(&m_CurrentSys[m_Pick[0]].pos,&tempX,&m_MouseDragPos[0]);
		VectorSum(&m_MouseDragPos[0],&tempY,&m_MouseDragPos[0]);
	}
	if (m_Pick[1] > -1)
	{
		VectorSum(&m_CurrentSys[m_Pick[1]].pos,&tempX,&m_MouseDragPos[1]);
		VectorSum(&m_MouseDragPos[1],&tempY,&m_MouseDragPos[1]);
	}
}
/// SetMouseForce /////////////////////////////////////////////////////////////

void CPhysEnv::AddSpring(BOOL isMuscle)
{
	tSpring	*spring;
	// MAKE SURE TWO PARTICLES ARE PICKED
	if (m_Pick[0] > -1 && m_Pick[1] > -1)
	{
		spring = (tSpring *)malloc(sizeof(tSpring) * (m_SpringCnt + 1));
		if (m_Spring != NULL)
			memcpy(spring,m_Spring,sizeof(tSpring) * m_SpringCnt);
		m_Spring = spring;
		spring = &m_Spring[m_SpringCnt++];
		spring->Ks = m_Ksh;
		spring->Kd = m_Ksd;
		spring->p1 = m_Pick[0];
		spring->p2 = m_Pick[1];
		spring->restLen = 
			sqrt(VectorSquaredDistance(&m_CurrentSys[m_Pick[0]].pos, 
									   &m_CurrentSys[m_Pick[1]].pos));
		spring->origRestLen = spring->restLen;
		if (isMuscle)
		{
			spring->isMuscle = TRUE;
			spring->contract = 0.5f;
			spring->cycOff = 0;
		}
		else
			spring->isMuscle = FALSE;

	}
}


// Velocity Threshold that decides between Static and Kinetic Friction
#define STATIC_THRESHOLD	0.03f				

void CPhysEnv::ComputeForces( tParticle	*system, BOOL duringIntegration )
{
	int loop;
	tParticle	*curParticle,*p1, *p2;
	tSpring		*spring;
	float		dist, Hterm, Dterm;
	tVector		springForce,deltaV,deltaP;
	float		FdotN,VdotN,Vmag;		
	tVector		Vn,Vt;				// CONTACT RESOLUTION IMPULSE

	curParticle = system;
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		MAKEVECTOR(curParticle->f,0.0f,0.0f,0.0f)		// CLEAR FORCE VECTOR

		if (m_UseGravity && curParticle->oneOverM != 0) // && curParticle->type != CONTACTING)
		{
			curParticle->f.x += (m_Gravity.x / curParticle->oneOverM);
			curParticle->f.y += (m_Gravity.y / curParticle->oneOverM);
			curParticle->f.z += (m_Gravity.z / curParticle->oneOverM);
		}

		if (m_UseDamping)
		{
			curParticle->f.x += (-m_Kd * curParticle->v.x);
			curParticle->f.y += (-m_Kd * curParticle->v.y);
			curParticle->f.z += (-m_Kd * curParticle->v.z);
		}
		else
		{
			curParticle->f.x += (-DEFAULT_DAMPING * curParticle->v.x);
			curParticle->f.y += (-DEFAULT_DAMPING * curParticle->v.y);
			curParticle->f.z += (-DEFAULT_DAMPING * curParticle->v.z);
		}

		// Handle Friction forces for Particles in contact with collision plane
		// Do not apply friction During integration phase
		if (curParticle->contacting && !duringIntegration && m_UseFriction)
		{
			// Calculate Fn 
			FdotN = DotProduct(&curParticle->contactN,&curParticle->f);
			// Calculate Vt Velocity Tangent to Normal Plane
			VdotN = DotProduct(&curParticle->contactN,&curParticle->v);
			ScaleVector(&curParticle->contactN, VdotN, &Vn);
			VectorDifference(&curParticle->v, &Vn, &Vt);
			Vmag = VectorSquaredLength(&Vt);
			// Check if Velocity is faster then threshold
			if (Vmag > STATIC_THRESHOLD)		// Use Kinetic Friction model
			{
				NormalizeVector(&Vt);
				ScaleVector(&Vt, (FdotN * m_Ckf), &Vt);
				VectorSum(&curParticle->f,&Vt,&curParticle->f);
			}
			else	// Use Static Friction Model
			{
				Vmag = Vmag / STATIC_THRESHOLD;
				NormalizeVector(&Vt);
				ScaleVector(&Vt, (FdotN * m_Csf * Vmag), &Vt);  // Scale Friction by Velocity
				VectorSum(&curParticle->f,&Vt,&curParticle->f);
			}
			curParticle->contacting = FALSE;
		}
		curParticle++;
	}

	// CHECK IF THERE IS A USER FORCE BEING APPLIED
	if (m_UserForceActive)
	{
		if (m_Pick[0] != -1)
		{
			VectorSum(&system[m_Pick[0]].f,&m_UserForce,&system[m_Pick[0]].f);
		}
		if (m_Pick[1] != -1)
		{
			VectorSum(&system[m_Pick[1]].f,&m_UserForce,&system[m_Pick[1]].f);
		}
		MAKEVECTOR(m_UserForce,0.0f,0.0f,0.0f);	// CLEAR USER FORCE
	}

	// NOW DO ALL THE SPRINGS
	spring = m_Spring;
	for (loop = 0; loop < m_SpringCnt; loop++)
	{

		// Handle All active springs = MUSCLES
		if (spring->isMuscle && m_MusclesFlexin)
		{
			// Multiplying the framecount by 2 to make it snappy
			float contraction = 1.0f - (m_SinTable[((m_FrameCnt * 2) + spring->cycOff) % 256] * spring->contract);
			spring->restLen = spring->origRestLen * contraction;
		}
		p1 = &system[spring->p1];
		p2 = &system[spring->p2];
		VectorDifference(&p1->pos,&p2->pos,&deltaP);	// Vector distance 
		dist = VectorLength(&deltaP);					// Magnitude of deltaP

		Hterm = (dist - spring->restLen) * spring->Ks;	// Ks * (dist - rest)
		
		VectorDifference(&p1->v,&p2->v,&deltaV);		// Delta Velocity Vector
		Dterm = (DotProduct(&deltaV,&deltaP) * spring->Kd) / dist; // Damping Term
		
		ScaleVector(&deltaP,1.0f / dist, &springForce);	// Normalize Distance Vector
		ScaleVector(&springForce,-(Hterm + Dterm),&springForce);	// Calc Force
		VectorSum(&p1->f,&springForce,&p1->f);			// Apply to Particle 1
		VectorDifference(&p2->f,&springForce,&p2->f);	// - Force on Particle 2
		spring++;					// DO THE NEXT SPRING
	}

	// APPLY THE MOUSE DRAG FORCES IF THEY ARE ACTIVE
	if (m_MouseForceActive)
	{
		// APPLY TO EACH PICKED PARTICLE
		if (m_Pick[0] > -1)
		{
			p1 = &system[m_Pick[0]];
			VectorDifference(&p1->pos,&m_MouseDragPos[0],&deltaP);	// Vector distance 
			dist = VectorLength(&deltaP);					// Magnitude of deltaP

			if (dist != 0.0f)
			{
				Hterm = (dist) * m_MouseForceKs;					// Ks * dist

				ScaleVector(&deltaP,1.0f / dist, &springForce);	// Normalize Distance Vector
				ScaleVector(&springForce,-(Hterm),&springForce);	// Calc Force
				VectorSum(&p1->f,&springForce,&p1->f);			// Apply to Particle 1
			}
		}
		if (m_Pick[1] > -1)
		{
			p1 = &system[m_Pick[1]];
			VectorDifference(&p1->pos,&m_MouseDragPos[1],&deltaP);	// Vector distance 
			dist = VectorLength(&deltaP);					// Magnitude of deltaP

			if (dist != 0.0f)
			{
				Hterm = (dist) * m_MouseForceKs;					// Ks * dist

				ScaleVector(&deltaP,1.0f / dist, &springForce);	// Normalize Distance Vector
				ScaleVector(&springForce,-(Hterm),&springForce);	// Calc Force
				VectorSum(&p1->f,&springForce,&p1->f);			// Apply to Particle 1
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
void CPhysEnv::IntegrateSysOverTime(tParticle *initial,tParticle *source, tParticle *target, float deltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	float deltaTimeMass;
///////////////////////////////////////////////////////////////////////////////
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		deltaTimeMass = deltaTime * initial->oneOverM;
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v.x = initial->v.x + (source->f.x * deltaTimeMass);
		target->v.y = initial->v.y + (source->f.y * deltaTimeMass);
		target->v.z = initial->v.z + (source->f.z * deltaTimeMass);

		target->oneOverM = initial->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = initial->pos.x + (deltaTime * source->v.x);
		target->pos.y = initial->pos.y + (deltaTime * source->v.y);
		target->pos.z = initial->pos.z + (deltaTime * source->v.z);

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
void CPhysEnv::EulerIntegrate( float DeltaTime)
{
	// JUST TAKE A SINGLE STEP
	IntegrateSysOverTime(m_CurrentSys,m_CurrentSys, m_TargetSys,DeltaTime);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	MidPointIntegrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses the Midpoint method
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::MidPointIntegrate( float DeltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float		halfDeltaT;
///////////////////////////////////////////////////////////////////////////////
	halfDeltaT = DeltaTime / 2.0f;

	// TAKE A HALF STEP AND UPDATE VELOCITY AND POSITION
	IntegrateSysOverTime(m_CurrentSys,m_CurrentSys,m_TempSys[0],halfDeltaT);

	// COMPUTE FORCES USING THESE NEW POSITIONS AND VELOCITIES
	ComputeForces(m_TempSys[0],TRUE);

	// TAKE THE FULL STEP WITH THIS NEW INFORMATION
	IntegrateSysOverTime(m_CurrentSys,m_TempSys[0],m_TargetSys,DeltaTime);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	RK4Integrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses the Runga-Kutta 4 method
//				This could use a generic function 4 times instead of unrolled
//				but it was easier for me to debug.  Fun for you to optimize.
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::RK4Integrate( float DeltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	float		halfDeltaT,sixthDeltaT;
	tParticle	*source,*target,*accum1,*accum2,*accum3,*accum4;
///////////////////////////////////////////////////////////////////////////////
	halfDeltaT = DeltaTime / 2.0f;		// SOME TIME VALUES I WILL NEED
	sixthDeltaT = 1.0f / 6.0f;

	// FIRST STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[1];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		accum1->f.x = halfDeltaT * source->f.x * source->oneOverM;
		accum1->f.y = halfDeltaT * source->f.y * source->oneOverM;
		accum1->f.z = halfDeltaT * source->f.z * source->oneOverM;

		accum1->v.x = halfDeltaT * source->v.x;
		accum1->v.y = halfDeltaT * source->v.y;
		accum1->v.z = halfDeltaT * source->v.z;
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE OVER 1/2 TIME
		target->v.x = source->v.x + (accum1->f.x);
		target->v.y = source->v.y + (accum1->f.y);
		target->v.z = source->v.z + (accum1->f.z);

		target->oneOverM = source->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = source->pos.x + (accum1->v.x);
		target->pos.y = source->pos.y + (accum1->v.y);
		target->pos.z = source->pos.z + (accum1->v.z);

		source++;
		target++;
		accum1++;
	}

	ComputeForces(m_TempSys[0],TRUE);  // COMPUTE THE NEW FORCES

	// SECOND STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[2];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		accum1->f.x = halfDeltaT * target->f.x * source->oneOverM;
		accum1->f.y = halfDeltaT * target->f.y * source->oneOverM;
		accum1->f.z = halfDeltaT * target->f.z * source->oneOverM;
		accum1->v.x = halfDeltaT * target->v.x;
		accum1->v.y = halfDeltaT * target->v.y;
		accum1->v.z = halfDeltaT * target->v.z;

		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v.x = source->v.x + (accum1->f.x);
		target->v.y = source->v.y + (accum1->f.y);
		target->v.z = source->v.z + (accum1->f.z);

		target->oneOverM = source->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = source->pos.x + (accum1->v.x);
		target->pos.y = source->pos.y + (accum1->v.y);
		target->pos.z = source->pos.z + (accum1->v.z);

		source++;
		target++;
		accum1++;
	}

	ComputeForces(m_TempSys[0],TRUE);  // COMPUTE THE NEW FORCES

	// THIRD STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[3];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// NOTICE I USE THE FULL DELTATIME THIS STEP
		accum1->f.x = DeltaTime * target->f.x * source->oneOverM;
		accum1->f.y = DeltaTime * target->f.y * source->oneOverM;
		accum1->f.z = DeltaTime * target->f.z * source->oneOverM;
		accum1->v.x = DeltaTime * target->v.x;
		accum1->v.y = DeltaTime * target->v.y;
		accum1->v.z = DeltaTime * target->v.z;

		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v.x = source->v.x + (accum1->f.x);
		target->v.y = source->v.y + (accum1->f.y);
		target->v.z = source->v.z + (accum1->f.z);

		target->oneOverM = source->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = source->pos.x + (accum1->v.x);
		target->pos.y = source->pos.y + (accum1->v.y);
		target->pos.z = source->pos.z + (accum1->v.z);

		source++;
		target++;
		accum1++;
	}

	ComputeForces(m_TempSys[0],TRUE);  // COMPUTE THE NEW FORCES

	// FOURTH STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[4];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// NOTICE I USE THE FULL DELTATIME THIS STEP
		accum1->f.x = DeltaTime * target->f.x * source->oneOverM;
		accum1->f.y = DeltaTime * target->f.y * source->oneOverM;
		accum1->f.z = DeltaTime * target->f.z * source->oneOverM;

		accum1->v.x = DeltaTime * target->v.x;
		accum1->v.y = DeltaTime * target->v.y;
		accum1->v.z = DeltaTime * target->v.z;

		// THIS TIME I DON'T NEED TO COMPUTE THE TEMPORARY POSITIONS
		source++;
		target++;
		accum1++;
	}

	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TargetSys;
	accum1 = m_TempSys[1];
	accum2 = m_TempSys[2];
	accum3 = m_TempSys[3];
	accum4 = m_TempSys[4];
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE USING RK4 FORMULA
		target->v.x = source->v.x + ((accum1->f.x + ((accum2->f.x + accum3->f.x) * 2.0f) + accum4->f.x) * sixthDeltaT);
		target->v.y = source->v.y + ((accum1->f.y + ((accum2->f.y + accum3->f.y) * 2.0f) + accum4->f.y) * sixthDeltaT);
		target->v.z = source->v.z + ((accum1->f.z + ((accum2->f.z + accum3->f.z) * 2.0f) + accum4->f.z) * sixthDeltaT);
		// DETERMINE THE NEW POSITION FOR THE PARTICLE USING RK4 FORMULA
		target->pos.x = source->pos.x + ((accum1->v.x + ((accum2->v.x + accum3->v.x) * 2.0f) + accum4->v.x) * sixthDeltaT);
		target->pos.y = source->pos.y + ((accum1->v.y + ((accum2->v.y + accum3->v.y) * 2.0f) + accum4->v.y) * sixthDeltaT);
		target->pos.z = source->pos.z + ((accum1->v.z + ((accum2->v.z + accum3->v.z) * 2.0f) + accum4->v.z) * sixthDeltaT);

		source++;
		target++;
		accum1++;
		accum2++;
		accum3++;
		accum4++;
	}

}

int CPhysEnv::CheckForCollisions( tParticle	*system )
{
    // be optimistic!
    int collisionState = NOT_COLLIDING;
    float const depthEpsilon = 0.001f;
    float const contactEpsilon = 0.01f;		// Threshold for particle in contact

	int loop;
	tParticle *curParticle;

	m_ContactCnt = 0;		// THERE ARE CURRENTLY NO CONTACTS

	curParticle = system;
	for (loop = 0; (loop < m_ParticleCnt) && (collisionState != PENETRATING); 
			loop++,curParticle++)
	{
        for(int planeIndex = 0;(planeIndex < m_CollisionPlaneCnt) &&
            (collisionState != PENETRATING);planeIndex++)
        {
			tCollisionPlane *plane = &m_CollisionPlane[planeIndex];

            float axbyczd = DotProduct(&curParticle->pos,&plane->normal) + plane->d;

            if(axbyczd < -depthEpsilon)
            {
				// ONCE ANY PARTICLE PENETRATES, QUIT THE LOOP
				collisionState = PENETRATING;
            }
            else
            if(axbyczd < depthEpsilon)
            {
                float relativeVelocity = DotProduct(&plane->normal,&curParticle->v);
				if(relativeVelocity < 0.0f)
                {
                    collisionState = COLLIDING;
					m_Contact[m_ContactCnt].type = COLLIDING;
					m_Contact[m_ContactCnt].particle = loop; 
					memcpy(&m_Contact[m_ContactCnt].normal,&plane->normal,sizeof(tVector));
					m_ContactCnt++;
                }
            }
			// Check if the Particles are in contact and need friction applied
			if (axbyczd < contactEpsilon)
			{
				curParticle->contacting = TRUE;
				// Save the contact normal for later
				memcpy(&curParticle->contactN,&plane->normal,sizeof(tVector));
			}
        }
	}

    return collisionState;
}

void CPhysEnv::ResolveCollisions( tParticle	*system )
{
	tContact	*contact;
	tParticle	*particle;		// THE PARTICLE COLLIDING
	float		VdotN;		
	tVector		Vn,Vt;				// CONTACT RESOLUTION IMPULSE
	contact = m_Contact;
	for (int loop = 0; loop < m_ContactCnt; loop++,contact++)
	{
		particle = &system[contact->particle];
		// CALCULATE Vn
		VdotN = DotProduct(&contact->normal,&particle->v);
		ScaleVector(&contact->normal, VdotN, &Vn);
		// CALCULATE Vt
		VectorDifference(&particle->v, &Vn, &Vt);
		// SCALE Vn BY COEFFICIENT OF RESTITUTION
		ScaleVector(&Vn, m_Kr, &Vn);
		// SET THE VELOCITY TO BE THE NEW IMPULSE
		VectorDifference(&Vt, &Vn, &particle->v);
	}
}

void CPhysEnv::Simulate(float DeltaTime, BOOL running)
{
    float		CurrentTime = 0.0f;
    float		TargetTime = DeltaTime;
	tParticle	*tempSys;
	int			collisionState;

    while(CurrentTime < DeltaTime)
    {
		if (running)
		{
			ComputeForces(m_CurrentSys, FALSE);

			// IN ORDER TO MAKE THINGS RUN FASTER, I HAVE THIS LITTLE TRICK
			// IF THE SYSTEM IS DOING A BINARY SEARCH FOR THE COLLISION POINT,
			// I FORCE EULER'S METHOD ON IT. OTHERWISE, LET THE USER CHOOSE.
			// THIS DOESN'T SEEM TO EFFECT STABILITY EITHER WAY
			if (m_CollisionRootFinding)
			{
				EulerIntegrate(TargetTime-CurrentTime);
			}
			else
			{
				switch (m_IntegratorType)
				{
				case EULER_INTEGRATOR:
					EulerIntegrate(TargetTime-CurrentTime);
					break;
				case MIDPOINT_INTEGRATOR:
					MidPointIntegrate(TargetTime-CurrentTime);
					break;
				case RK4_INTEGRATOR:
					RK4Integrate(TargetTime-CurrentTime);
					break;
				}
			}
		}

		collisionState = CheckForCollisions(m_TargetSys);

        if(collisionState == PENETRATING)
        {
			// TELL THE SYSTEM I AM LOOKING FOR A COLLISION SO IT WILL USE EULER
			m_CollisionRootFinding = TRUE;
            // we simulated too far, so subdivide time and try again
            TargetTime = (CurrentTime + TargetTime) / 2.0f;

            // blow up if we aren't moving forward each step, which is
            // probably caused by interpenetration at the frame start
            assert(fabs(TargetTime - CurrentTime) > EPSILON);
        }
        else
        {
            // either colliding or clear
            if(collisionState == COLLIDING)
            {
                int Counter = 0;
                do
                {
                    ResolveCollisions(m_TargetSys);
                    Counter++;
                } while((CheckForCollisions(m_TargetSys) ==
                            COLLIDING) && (Counter < 100));

                assert(Counter < 100);
				m_CollisionRootFinding = FALSE;	// FOUND THE COLLISION POINT BACK TO NORMAL
            }

            // we made a successful step, so swap configurations
            // to "save" the data for the next step
            
			CurrentTime = TargetTime;
			TargetTime = DeltaTime;

			// SWAP MY TWO PARTICLE SYSTEM BUFFERS SO I CAN DO IT AGAIN
			tempSys = m_CurrentSys;
			m_CurrentSys = m_TargetSys;
			m_TargetSys = tempSys;
        }
    }
}

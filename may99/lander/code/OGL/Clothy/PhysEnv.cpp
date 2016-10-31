///////////////////////////////////////////////////////////////////////////////
//
// PhysEnv.cpp : Physical World implementation file
//
// Purpose:	Implementation of Particle Physics System
//
// Created:
//		JL 2/10/99	Modified from Mass-Spring Particle demo to handle cloth
// Modified:
//		JL 3/6/99 - FIXED GRAVITY FORCE CALCULATION BUG
//		JL 3/8/99 - ADDED MORE POSSIBLE CONTACTS AS EACH VERTEX CAN CONTACT MORE 
//					THEN ONE COLLISION SURFACE (SHOULD IT BE DYNAMICALLY ALLOC'ED?)
//		JL 3/20/99 - ADDED THE MIDPOINT AND RK INTEGRATOR NEEDED TO ALLOC 5 TEMP PARTICLE ARRAYS
//
// Notes:	A bit of this along with the organization comes from Chris Hecker's
//			Physics Articles from last year www.d6.com.  Hopefully this will get 
//			everyone back up to speed before we dig deeper into the world of Dynamics.
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998-1999 Jeff Lander, All Rights Reserved.
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

#include "Clothy.h"
#include "PhysEnv.h"
#include "SimProps.h"
#include "SetVert.h"
#include "AddSpher.h"

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
	m_IntegratorType = EULER_INTEGRATOR;

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
	m_MouseForceActive = FALSE;

	m_UseGravity = TRUE;
	m_DrawSprings = TRUE;
	m_DrawStructural = TRUE;	// By default only draw structural springs
	m_DrawBend = FALSE;
	m_DrawShear = FALSE;
	m_DrawVertices	= TRUE;
	m_CollisionActive = TRUE;
	m_CollisionRootFinding = FALSE;		// I AM NOT LOOKING FOR A COLLISION RIGHT AWAY

	MAKEVECTOR(m_Gravity, 0.0f, -0.2f, 0.0f)
	m_UserForceMag = 100.0;
	m_UserForceActive = FALSE;
	m_MouseForceKs = 2.0f;	// MOUSE SPRING CONSTANT
	m_Kd	= 0.04f;	// DAMPING FACTOR
	m_Kr	= 0.1f;		// 1.0 = SUPERBALL BOUNCE 0.0 = DEAD WEIGHT
	m_Ksh	= 5.0f;		// HOOK'S SPRING CONSTANT
	m_Ksd	= 0.1f;		// SPRING DAMPING CONSTANT

	// CREATE THE SIZE FOR THE SIMULATION WORLD
	m_WorldSizeX = 15.0f;
	m_WorldSizeY = 15.0f;
	m_WorldSizeZ = 15.0f;

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

	m_SphereCnt = 0;

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
	free(m_CollisionPlane);
	free(m_Spring);

	free(m_Sphere);
}

void CPhysEnv::RenderWorld()
{
	tParticle	*tempParticle;
	tSpring		*tempSpring;

	// FIRST DRAW THE WORLD CONTAINER  
	glColor3f(1.0f,1.0f,1.0f);
    // do a big linestrip to get most of edges
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
			glBegin(GL_LINES);
			glColor3f(0.0f,0.8f,0.8f);
			tempSpring = m_Spring;
			for (int loop = 0; loop < m_SpringCnt; loop++)
			{
				// Only draw normal springs or the cloth "structural" ones
				if ((tempSpring->type == MANUAL_SPRING) ||
					(tempSpring->type == STRUCTURAL_SPRING && m_DrawStructural) ||
					(tempSpring->type == SHEAR_SPRING && m_DrawShear) ||
					(tempSpring->type == BEND_SPRING && m_DrawBend))
				{
					glVertex3fv((float *)&m_CurrentSys[tempSpring->p1].pos);
					glVertex3fv((float *)&m_CurrentSys[tempSpring->p2].pos);
				}
				tempSpring++;
			}
			if (m_MouseForceActive)	// DRAW MOUSESPRING FORCE
			{
				if (m_Pick[0] > -1)
				{
					glColor3f(0.8f,0.0f,0.8f);
					glVertex3fv((float *)&m_CurrentSys[m_Pick[0]].pos);
					glVertex3fv((float *)&m_MouseDragPos[0]);
				}
				if (m_Pick[1] > -1)
				{
					glColor3f(0.8f,0.0f,0.8f);
					glVertex3fv((float *)&m_CurrentSys[m_Pick[1]].pos);
					glVertex3fv((float *)&m_MouseDragPos[1]);
				}
			}
			glEnd();
		}
		if (m_DrawVertices)
		{
			glBegin(GL_POINTS);
			tempParticle = m_CurrentSys;
			for (int loop = 0; loop < m_ParticleCnt; loop++)
			{
				if (loop == m_Pick[0])
					glColor3f(0.0f,0.8f,0.0f);
				else if (loop == m_Pick[1])
					glColor3f(0.8f,0.0f,0.0f);
				else
					glColor3f(0.8f,0.8f,0.0f);
				glVertex3fv((float *)&tempParticle->pos);
				tempParticle++;
			}
			glEnd();
		}
	}

	if (m_SphereCnt > 0 && m_CollisionActive)
	{
		glColor3f(0.5f,0.0f,0.0f);
		for (int loop = 0; loop < m_SphereCnt; loop++)
		{
			glPushMatrix();
			glTranslatef(m_Sphere[loop].pos.x, m_Sphere[loop].pos.y, m_Sphere[loop].pos.z);
			glScalef(m_Sphere[loop].radius,m_Sphere[loop].radius,m_Sphere[loop].radius);
			glCallList(OGL_AXIS_DLIST);
			glPopMatrix();
		}
	}
}

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

void CPhysEnv::SetWorldParticles(tTexturedVertex *coords,int particleCnt)
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
	fread(&m_SphereCnt,sizeof(int),1,fp);
	m_Sphere = (tCollisionSphere *)malloc(sizeof(tCollisionSphere) * (m_SphereCnt));
	fread(m_Sphere,sizeof(tCollisionSphere),m_SphereCnt,fp);
}

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
	fwrite(&m_SphereCnt,sizeof(int),1,fp);
	fwrite(m_Sphere,sizeof(tCollisionSphere),m_SphereCnt,fp);
}

// RESET THE SIM TO INITIAL VALUES
void CPhysEnv::ResetWorld()
{
	memcpy(m_CurrentSys,m_ParticleSys[2],sizeof(tParticle) * m_ParticleCnt);
	memcpy(m_TargetSys,m_ParticleSys[2],sizeof(tParticle) * m_ParticleCnt);
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
	}
}

void CPhysEnv::SetVertexProperties()
{
	CSetVert	dialog;
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


void CPhysEnv::AddSpring()
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
		spring->type = 	MANUAL_SPRING;
	}
}

void CPhysEnv::AddSpring(int v1, int v2,float Ksh,float Ksd, int type)
{
	tSpring	*spring;
	// MAKE SURE TWO PARTICLES ARE PICKED
	if (v1 > -1 && v2 > -1)
	{
		spring = (tSpring *)malloc(sizeof(tSpring) * (m_SpringCnt + 1));
		if (m_Spring != NULL)
		{
			memcpy(spring,m_Spring,sizeof(tSpring) * m_SpringCnt);
			free(m_Spring);
		}
		m_Spring = spring;
		spring = &m_Spring[m_SpringCnt++];
		spring->type = type;
		spring->Ks = Ksh;
		spring->Kd = Ksd;
		spring->p1 = v1;
		spring->p2 = v2;
		spring->restLen = 
			sqrt(VectorSquaredDistance(&m_CurrentSys[v1].pos, 
									   &m_CurrentSys[v2].pos));
	}
}

void CPhysEnv::ComputeForces( tParticle	*system )
{
	int loop;
	tParticle	*curParticle,*p1, *p2;
	tSpring		*spring;
	float		dist, Hterm, Dterm;
	tVector		springForce,deltaV,deltaP;

	curParticle = system;
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		MAKEVECTOR(curParticle->f,0.0f,0.0f,0.0f)		// CLEAR FORCE VECTOR

		if (m_UseGravity && curParticle->oneOverM != 0)
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
	ComputeForces(m_TempSys[0]);

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

	ComputeForces(m_TempSys[0]);  // COMPUTE THE NEW FORCES

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

	ComputeForces(m_TempSys[0]);  // COMPUTE THE NEW FORCES

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

	ComputeForces(m_TempSys[0]);  // COMPUTE THE NEW FORCES

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

	int loop;
	tParticle *curParticle;

	m_ContactCnt = 0;		// THERE ARE CURRENTLY NO CONTACTS

	curParticle = system;
	for (loop = 0; (loop < m_ParticleCnt) && (collisionState != PENETRATING); 
			loop++,curParticle++)
	{
		// CHECK THE MAIN BOUNDARY PLANES FIRST
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
					m_Contact[m_ContactCnt].particle = loop; 
					memcpy(&m_Contact[m_ContactCnt].normal,&plane->normal,sizeof(tVector));
					m_ContactCnt++;
                }
            }
        }
		if (m_CollisionActive)
		{
			// THIS IS NEW FROM MARCH - ADDED SPHERE BOUNDARIES
			// CHECK ANY SPHERE BOUNDARIES
			for(int sphereIndex = 0;(sphereIndex < m_SphereCnt) &&
				(collisionState != PENETRATING);sphereIndex++)
			{
				tCollisionSphere *sphere = &m_Sphere[sphereIndex];

				tVector	distVect;

				VectorDifference(&curParticle->pos, &sphere->pos, &distVect);

				float radius = VectorSquaredLength(&distVect);
				// SINCE IT IS TESTING THE SQUARED DISTANCE, SQUARE THE RADIUS ALSO
				float dist = radius - (sphere->radius * sphere->radius);

				if(dist < -depthEpsilon)
				{
					// ONCE ANY PARTICLE PENETRATES, QUIT THE LOOP
					collisionState = PENETRATING;
				}
				else
				if(dist < depthEpsilon)
				{
					// NORMALIZE THE VECTOR
					NormalizeVector(&distVect);

					float relativeVelocity = DotProduct(&distVect,&curParticle->v);

					if(relativeVelocity < 0.0f)
					{
						collisionState = COLLIDING;
						m_Contact[m_ContactCnt].particle = loop; 
						memcpy(&m_Contact[m_ContactCnt].normal,&distVect,sizeof(tVector));
						m_ContactCnt++;
					}
				}
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
			ComputeForces(m_CurrentSys);

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
				m_CollisionRootFinding = FALSE;  // FOUND THE COLLISION POINT
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

///////////////////////////////////////////////////////////////////////////////
// Function:	AddCollisionSphere 
// Purpose:		Add a collision sphere to the system
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::AddCollisionSphere()
{
/// Local Variables ///////////////////////////////////////////////////////////
	tCollisionSphere *temparray;
	CAddSpher		dialog;
///////////////////////////////////////////////////////////////////////////////
	dialog.m_Radius = 2.0f;
	dialog.m_XPos = 0.0f;
	dialog.m_YPos = -3.0f;
	dialog.m_ZPos = 0.0f;
	if (dialog.DoModal())
	{
		temparray = (tCollisionSphere *)malloc(sizeof(tCollisionSphere) * (m_SphereCnt+1)); 	
		if (m_SphereCnt > 0)
		{
			memcpy(temparray,m_Sphere,sizeof(tCollisionSphere) * m_SphereCnt);
			free(m_Sphere);
		}
		
		MAKEVECTOR(temparray[m_SphereCnt].pos,dialog.m_XPos,dialog.m_YPos, dialog.m_ZPos)
		temparray[m_SphereCnt].radius = dialog.m_Radius;
	
		m_Sphere = temparray;
		m_SphereCnt++;
	}
}
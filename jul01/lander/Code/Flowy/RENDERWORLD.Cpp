/////////////////////////////////////////////////////////////////////////////////////
// RenderWorld.c
// This file actually renders the world complete
// 
// Created:
//		JL 2/15/01		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2001 Darwin 3D, LLC, All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>	// Normal Windows stuff
#include <math.h>
#include <stdio.h>
#include <gl/gl.h>		// Core OpenGL functions
#include <gl/glu.h>		// OpenGL Utility functions
#include <gl/glaux.h>
#include "externs.h"	// Data shared between files

/// Application Definitions ///////////////////////////////////////////////////
int			g_DrawInfluence = true;
float		g_ViewMatrix[16];
tVector		g_TestPoint;
int			g_DrawParticles = true;
DWORD		g_Time;
t_Element	g_Element[MAX_ELEMENTS];
int			g_ElementCnt;
t_Particle	*g_Particle;
int			g_PartCnt;
float		g_DeltaTime = 0.5f;	// Default time for each step

////////////////////////////////////////////////////////////////////////////////
// Initialize the Render System
////////////////////////////////////////////////////////////////////////////////
void InitRender(void)
{
/// Local Variables ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

	g_ElementCnt = 0;

	glNewList(OGL_AXIS_DLIST,GL_COMPILE);
		glPushMatrix();
		glBegin(GL_LINES);
			glVertex3f( 0.0f, -10.0f,  0.0f);
			glVertex3f( 0.0f, 10.0f,   0.0f);
			glVertex3f( 0.0f,  10.0f, 0.0f);	// TOP PIECE OF ARROWHEAD
			glVertex3f( 4.0f,  3.5f, 0.0f);
			glVertex3f( 0.0f,  10.0f, 0.0f);	// BOTTOM PIECE OF ARROWHEAD
			glVertex3f( -4.0f,  3.5f, 0.0f);
		glEnd();
		glPopMatrix();
	glEndList();

	// Draw the circular shape of a object
	glNewList(OGL_ELEMENT_DLIST,GL_COMPILE);
		glBegin(GL_LINES);
		for (float angle = 0; angle < PI_TIMES_TWO; angle += (float)PI_TIMES_TWO / 32.0f)
		{
			glVertex3f(10.0 * sin(angle),10.0 * cos(angle), 0.0f);
			glVertex3f(10.0 * sin(angle + (PI_TIMES_TWO / 32.0f)),10.0 * cos(angle + (PI_TIMES_TWO / 32.0f)), 0.0f);
		}
		glEnd();
	glEndList();

	g_Particle = (t_Particle *)malloc(sizeof(t_Particle) * MAX_PARTICLES);
	g_PartCnt = 0;
}

void FreeRender(void)
{
	free(g_Particle);
}

////////////////////////////////////////////////////////////////////////////////
// Render the entire simulation environment
////////////////////////////////////////////////////////////////////////////////
void DrawSimWorld(void)
{
	if (g_ElementCnt > 0 && g_DrawInfluence)
	{
		for (int loop = 0; loop < g_ElementCnt; loop++)
		{
			glPushMatrix();
			glTranslatef(g_Element[loop].pos.x,g_Element[loop].pos.y,g_Element[loop].pos.z);
			if (g_Element[loop].type == 0)
			{
				glScalef(g_Element[loop].size,g_Element[loop].size,g_Element[loop].size);
				glColor3f(1.0f, 1.0f, 1.0f);	// drawn white
				if (loop == g_Picked)
					glColor3f(1.0f, 0.0f, 1.0f);
				glRotatef(-g_Element[loop].strength,0,0,1);
				glCallList(OGL_AXIS_DLIST);
			}
			else
			{
				glScalef(g_Element[loop].size,g_Element[loop].size,g_Element[loop].size);
				switch (g_Element[loop].type)
				{
					case ELEMENT_SOURCE:
						glColor3f(1.0f, 1.0f, 0.0f);
						break;
					case ELEMENT_VORTEX:
						glColor3f(0.0f, 1.0f, 1.0f);
						break;
					case ELEMENT_DOUBLET:
						glColor3f(0.0f, 1.0f, 0.0f);
						break;
				}
				if (loop == g_Picked)
					glColor3f(1.0f, 0.0f, 1.0f);
				glCallList(OGL_ELEMENT_DLIST);
			}

			glPopMatrix();
		}
	}
	if (g_PartCnt > 0 && g_DrawParticles)
	{
		glColor3f(1.0f, 1.0f, 1.0f);	// drawn white
		t_Particle *particle = g_Particle;
		for (int loop = 0; loop < g_PartCnt; loop++, particle++)
		{
			// Draw the particle as a single triangle strip
			glBegin(GL_TRIANGLE_STRIP);
				glVertex2f(particle->pos.x - 4.0f,particle->pos.y - 4.0f);
				glVertex2f(particle->pos.x + 4.0f,particle->pos.y - 4.0f);
				glVertex2f(particle->pos.x - 4.0f,particle->pos.y + 4.0f);
				glVertex2f(particle->pos.x + 4.0f,particle->pos.y + 4.0f);
			glEnd();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Render the entire scene
////////////////////////////////////////////////////////////////////////////////
void RenderWorld(void)
{
	AdvanceParticles();		// Run the simulation

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	// Clear the window with current clearing color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	DrawSimWorld();

	glPopMatrix();

	SwapBuffers(g_hDC);
}

////////////////////////////////////////////////////////////////////////////////
// Procedure:	EvaluateVelocity
// Purpose:		Evalutate the potential Velocity field for a given point
// Arguments:	Position is screen space to evaluate
////////////////////////////////////////////////////////////////////////////////
void EvaluateVelocity(t_Particle *particle)
{
/// Local Variable /////////////////////////////////////////////////////////////
	tVector vAcc,vTemp;
////////////////////////////////////////////////////////////////////////////////
	vAcc.Set(0.0f, 0.0f, 0.0f);
	vTemp.Set(0.0f, 0.0f, 0.0f);

	if (g_ElementCnt > 0)
	{
		t_Element *pElement = g_Element;
		for (int loop = 0; loop < g_ElementCnt; loop++, pElement++)
		{
			float distX = particle->pos.x - pElement->pos.x;
			float distY = particle->pos.y - pElement->pos.y;
			// Handle each of the different potential velocity 
			switch (pElement->type)
			{
			case ELEMENT_UNIFORM:
				// Size = flow rate   strength = flow angle
				vTemp.x = pElement->size * sin(DEGTORAD(pElement->strength));
				vTemp.y = pElement->size * cos(DEGTORAD(pElement->strength));
				break;
			case ELEMENT_SOURCE:
				// size = flow rate
				if (distY != 0.0f || distX != 0.0f)
				{
					vTemp.x = (pElement->strength)*(pElement->size)*(distX/(distX*distX + distY*distY));
					vTemp.y = (pElement->strength)*(pElement->size)*(distY/(distX*distX + distY*distY));
				}
				break;
			case ELEMENT_VORTEX:
				// size = rotation rate
				if (distY != 0.0f || distX != 0.0f)
				{
					vTemp.x = (pElement->strength)*(pElement->size)*(distY/(distX*distX + distY*distY));
					vTemp.y = -(pElement->strength)*(pElement->size)*(distX/(distX*distX + distY*distY));
				}
				break;
			case ELEMENT_DOUBLET:
				// Size = flow rate   strength = length
				if (distY != 0.0f || distX != 0.0f)
				{
					float dist_sq_sq = (distX*distX + distY*distY) * (distX*distX + distY*distY);
					vTemp.x = -(pElement->size * pElement->strength * pElement->strength)*((distX*distX - distY*distY)/dist_sq_sq);
					vTemp.y = -(pElement->size * pElement->strength * pElement->strength)*(distX*distY/dist_sq_sq);
				}
				break;
			}
			// Clamp it so that there is at least some measurable force
			if (fabs(vTemp.x) < 0.1f) vTemp.x = 0.0f;
			if (fabs(vTemp.y) < 0.1f) vTemp.y = 0.0f;

			vAcc += vTemp;
		}
	}
	particle->v = vAcc;  // Set the accumulated velocity
}

void AddParticle(float x, float y)
{
	if (g_PartCnt < MAX_PARTICLES)
	{
		t_Particle *particle = &g_Particle[g_PartCnt];
		particle->pos.Set(x,y,0.0f);
		particle->rest_pos = particle->pos;
		particle->v.Set(0.0f,0.0f,0.0f);
		g_PartCnt++;
	}
}

void KillParticle(int which)
{
	if (which < g_PartCnt-1)
	{
		memcpy(&g_Particle[which],&g_Particle[which+1],sizeof(t_Particle) * (g_PartCnt - which));
	}
	g_PartCnt--;
}

////////////////////////////////////////////////////////////////////////////////
// Procedure:	AdvanceParticles
// Purpose:		Advance all the particles in the environment
// Notes:		Integration is explicit Euler with the velocities determined 
//				by the velocity field in the EvaluateVelocity function
////////////////////////////////////////////////////////////////////////////////
void AdvanceParticles()
{
	t_Particle *particle = g_Particle;
	for (int i = 0; i < g_PartCnt; i++, particle++)
	{
		EvaluateVelocity(particle);
		particle->pos += (particle->v * g_DeltaTime);	// Simple Euler Integration step
		if (particle->pos.x < 0.0f || particle->pos.y < 0.0f ||
			particle->pos.x > g_ScreenWidth || particle->pos.y > g_ScreenHeight)
		{
			KillParticle(i);
			i--;
		}
	}
        // Handle any Particle emittions from the elements
	if (g_ElementCnt > 0)
	{
		t_Element *pElement = g_Element;
		for (int loop = 0; loop < g_ElementCnt; loop++, pElement++)
		{
                    if (pElement->emitParticles)    // I need to emit a particle from this element
                    {
                        if (rand() < (RAND_MAX / 8)) // Only emit once in a while
                        {
                            // Find a random position within the element by creating a random angle and distance from center
                            float angle = DEGTORAD((float)(rand() % 360));
                            float dist = pElement->size * 10 * ((float)rand() / (float)RAND_MAX);
                            if (dist == 0) dist = 0.1f; // avoid the center 
                            float xpos = pElement->pos.x + (dist * sin(angle));
                            float ypos = pElement->pos.y + (dist * cos(angle));
                            AddParticle(xpos,ypos);
                        }
                    }
                }
        }
}

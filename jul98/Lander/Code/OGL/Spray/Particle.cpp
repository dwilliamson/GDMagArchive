///////////////////////////////////////////////////////////////////////////////
//
// Particle.cpp : Particle System support file
//
// Purpose:	Structure Support routines for Particle System
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
// Revisions:
//		Integrated into Particle Demo		4/18/98
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Spray.h"
#include "Particle.h"
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

///////////////////////////////////////////////////////////////////////////////
//
// Particle.cpp : Particle System structure implementation
//
// Purpose:	Implementation code of Particle System
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 4/1/98		
// Revisions:
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#include <time.h>

/// Global Definitions ////////////////////////////////////////////////////////
const int HALF_RAND = (RAND_MAX / 2);
///////////////////////////////////////////////////////////////////////////////

/// Global Member Variables ///////////////////////////////////////////////////
// REALLY ARE PRIVATE MEMBER VARIABLES OF PARTICLE CLASS
tParticle *m_ParticlePool;		// POOL TO PULL PARTICLES FROM
///////////////////////////////////////////////////////////////////////////////

float RandomNum()
{
	int rn;
	rn = rand();
	return ((float)(rn - HALF_RAND) / (float)HALF_RAND);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	RotationToDirection
// Purpose:		Convert a Yaw and Pitch to a direction vector
///////////////////////////////////////////////////////////////////////////////
void RotationToDirection(float pitch,float yaw,tVector *direction)
{
	direction->x = (float)(-sin(yaw) * cos(pitch));
	direction->y = (float)sin(pitch);
	direction->z = (float)(cos(pitch) * cos(yaw));
}
/// initParticleSystem ////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	initParticleSystem
// Purpose:		Initialize the particle system
// Notes:		This is really the CREATOR for the particle class
//				Since I am doing it C, I need to call it
///////////////////////////////////////////////////////////////////////////////
BOOL initParticleSystem()
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
///////////////////////////////////////////////////////////////////////////////
	srand( (unsigned)time( NULL ) );

	// SO I DON'T NEED TO DYNAMICALLY ALLOC THE PARTICLES IN THE RUNTIME
	// I WANT TO PULL ALREADY CREATED PARTICLES FROM A GLOBAL POOL.
	m_ParticlePool = (tParticle *)malloc(MAX_PARTICLES * sizeof(tParticle));
	// THIS IS A LINKED LIST OF PARTICLES, SO I NEED TO ESTABLISH LINKS
	for (loop = 0; loop < MAX_PARTICLES - 1; loop++)
	{
		m_ParticlePool[loop].next = &m_ParticlePool[loop + 1];
	}	
	// SET THE LAST PARTICLE TO POINT TO NULL
	m_ParticlePool[MAX_PARTICLES - 1].next = NULL;

	return TRUE;
}
/// initParticleSystem ////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	setDefaultEmitter
// Purpose:		Set up some default settings
// Arguments:	The emitter to setup
///////////////////////////////////////////////////////////////////////////////
BOOL setDefaultEmitter(tEmitter *emitter)
{
	emitter->id = 0;		// UNUSED
	strcpy(emitter->name, "Emitter");
	emitter->flags = NULL;
	emitter->prev = NULL;				// NOT USED
	emitter->next = NULL;					// NOT USED
	emitter->pos.x = 0.0f;					// XYZ POSITION
	emitter->pos.y = -0.5f;					// XYZ POSITION
	emitter->pos.z = -5.0f;					// XYZ POSITION

	emitter->yaw = DEGTORAD(0.0f);
	emitter->yawVar = DEGTORAD(360.0f);
	emitter->pitch = DEGTORAD(90.0f);
	emitter->pitchVar = DEGTORAD(40.0f);
	emitter->speed = 0.05f;
	emitter->speedVar = 0.01f;

	emitter->totalParticles	= 4000;
	emitter->particleCount	= 0;
	emitter->emitsPerFrame	= 100;
	emitter->emitVar	= 15;
	emitter->life = 60;
	emitter->lifeVar = 15;
	emitter->startColor.r = 0.6f;
	emitter->startColor.g = 0.6f;
	emitter->startColor.b = 0.8f;
	emitter->startColorVar.r = 0.1f;
	emitter->startColorVar.g = 0.1f;
	emitter->startColorVar.b = 0.1f;
	emitter->endColor.r = 0.0f;
	emitter->endColor.g = 0.0f;
	emitter->endColor.b = 0.8f;
	emitter->endColorVar.r = 0.1f;
	emitter->endColorVar.g = 0.1f;
	emitter->endColorVar.b = 0.2f;

	emitter->force.x = 0.000f;
	emitter->force.y = -0.001f;
	emitter->force.z = 0.0f;
	return TRUE;
}
/// setDefaultEmitter /////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Function:	initEmitter
// Purpose:		Initialize an emitter in the system
// Arguments:	The emitter to initialize
///////////////////////////////////////////////////////////////////////////////
BOOL initEmitter(tEmitter *emitter)
{
	setDefaultEmitter(emitter);
	emitter->particle	= NULL;					// NULL TERMINATED LINKED LIST
	return TRUE;
}
/// initEmitter ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	addParticle
// Purpose:		add a particle to an emitter
// Arguments:	The emitter to add to
///////////////////////////////////////////////////////////////////////////////
BOOL addParticle(tEmitter *emitter)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tParticle *particle;
	tColor	start,end;
	float yaw,pitch,speed;
///////////////////////////////////////////////////////////////////////////////
	// IF THERE IS AN EMITTER AND A PARTICLE IN THE POOL
	// AND I HAVEN'T EMITTED MY MAX
	if (emitter != NULL && m_ParticlePool != NULL && 
		emitter->particleCount < emitter->totalParticles)
	{
		particle = m_ParticlePool;		// THE CURRENT PARTICLE 
		m_ParticlePool = m_ParticlePool->next;	// FIX THE POOL POINTERS

		if (emitter->particle != NULL)
			emitter->particle->prev = particle; // SET BACK LINK
		particle->next = emitter->particle;	// SET ITS NEXT POINTER
		particle->prev = NULL;				// IT HAS NO BACK POINTER
		emitter->particle = particle;		// SET IT IN THE EMITTER

		particle->pos.x = 0.0f;	// RELATIVE TO EMITTER BASE
		particle->pos.y = 0.0f;
		particle->pos.z = 0.0f;

		particle->prevPos.x = 0.0f;	// USED FOR ANTI ALIAS
		particle->prevPos.y = 0.0f;
		particle->prevPos.z = 0.0f;

		// CALCULATE THE STARTING DIRECTION VECTOR
		yaw = emitter->yaw + (emitter->yawVar * RandomNum());
		pitch = emitter->pitch + (emitter->pitchVar * RandomNum());

		// CONVERT THE ROTATIONS TO A VECTOR
		RotationToDirection(pitch,yaw,&particle->dir);
		
		// MULTIPLY IN THE SPEED FACTOR
		speed = emitter->speed + (emitter->speedVar * RandomNum());
		particle->dir.x *= speed;
		particle->dir.y *= speed;
		particle->dir.z *= speed;

		// CALCULATE THE COLORS
		start.r = emitter->startColor.r + (emitter->startColorVar.r * RandomNum());
		start.g = emitter->startColor.g + (emitter->startColorVar.g * RandomNum());
		start.b = emitter->startColor.b + (emitter->startColorVar.b * RandomNum());
		end.r = emitter->endColor.r + (emitter->endColorVar.r * RandomNum());
		end.g = emitter->endColor.g + (emitter->endColorVar.g * RandomNum());
		end.b = emitter->endColor.b + (emitter->endColorVar.b * RandomNum());

		particle->color.r = start.r;
		particle->color.g = start.g;
		particle->color.b = start.b;

		// CALCULATE THE LIFE SPAN
		particle->life = emitter->life + (int)((float)emitter->lifeVar * RandomNum());

		// CREATE THE COLOR DELTA
		particle->deltaColor.r = (end.r - start.r) / particle->life;
		particle->deltaColor.g = (end.g - start.g) / particle->life;
		particle->deltaColor.b = (end.b - start.b) / particle->life;

		emitter->particleCount++;	// A NEW PARTICLE IS BORN
		return TRUE;
	}
	return FALSE;
}
/// addParticle ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	updateParticle
// Purpose:		updateParticle settings
// Arguments:	The particle to update and the emitter it came from
///////////////////////////////////////////////////////////////////////////////
BOOL updateParticle(tParticle *particle,tEmitter *emitter)
{
	// IF THIS IS AN VALID PARTICLE
	if (particle != NULL && particle->life > 0)
	{
		// SAVE ITS OLD POS FOR ANTI ALIASING
		particle->prevPos.x = particle->pos.x;
		particle->prevPos.y = particle->pos.y;
		particle->prevPos.z = particle->pos.z;

		// CALCULATE THE NEW
		particle->pos.x += particle->dir.x;
		particle->pos.y += particle->dir.y;
		particle->pos.z += particle->dir.z;

		// APPLY GLOBAL FORCE TO DIRECTION
		particle->dir.x += emitter->force.x;
		particle->dir.y += emitter->force.y;
		particle->dir.z += emitter->force.z;

		// SAVE THE OLD COLOR
		particle->prevColor.r = particle->color.r;
		particle->prevColor.g = particle->color.g;
		particle->prevColor.b = particle->color.b;

		// GET THE NEW COLOR
		particle->color.r += particle->deltaColor.r;
		particle->color.g += particle->deltaColor.g;
		particle->color.b += particle->deltaColor.b;

		particle->life--;	// IT IS A CYCLE OLDER
		return TRUE;
	}
	else if (particle != NULL && particle->life == 0)
	{
		// FREE THIS SUCKER UP BACK TO THE MAIN POOL
		if (particle->prev != NULL)
			particle->prev->next = particle->next;
		else
			emitter->particle = particle->next;
		// FIX UP THE NEXT'S PREV POINTER IF THERE IS A NEXT
		if (particle->next != NULL)
			particle->next->prev = particle->prev;
		particle->next = m_ParticlePool;
		m_ParticlePool = particle;	// NEW POOL POINTER
		emitter->particleCount--;	// ADD ONE TO POOL
	}
	return FALSE;
}
/// updateParticle ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	updateEmitter
// Purpose:		updateEmitter setting
// Arguments:	The Emitter to update
// Notes:		This is called once per frame to update the emitter
///////////////////////////////////////////////////////////////////////////////
BOOL updateEmitter(tEmitter *emitter)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop,emits;
	tParticle *particle, *next;
///////////////////////////////////////////////////////////////////////////////
	// IF THERE IS AN EMITTER
	if (emitter != NULL)
	{
		if (emitter->particle != NULL)
		{
			// GO THROUGH THE PARTICLES AND UPDATE THEM
			particle = emitter->particle;
			while (particle)
			{
				next = particle->next;	// SAVE THIS BECAUSE IT MAY CHANGE UNDER ME
				updateParticle(particle,emitter);
				particle = next;
			}
		}

		// EMIT PARTICLES FOR THIS FRAME
		emits = emitter->emitsPerFrame + (int)((float)emitter->emitVar * RandomNum());
		for (loop = 0; loop < emits; loop++)
			addParticle(emitter);
		
		return TRUE;
	}
	return FALSE;
}
/// updateEmitter ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	renderEmitter
// Purpose:		render particle system
// Arguments:	The Emitter to render
// Notes:		This is called once per frame to render the emitter
///////////////////////////////////////////////////////////////////////////////
BOOL renderEmitter(tEmitter *emitter, BOOL antiAlias)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tParticle *particle;
///////////////////////////////////////////////////////////////////////////////
	// IF THERE IS AN EMITTER
	if (emitter != NULL)
	{
		if (emitter->particle != NULL)
		{
			particle = emitter->particle;
			// DON'T ANTIALIAS FIRST FRAME
			if (antiAlias)
				glBegin(GL_LINES);
			else
				glBegin(GL_POINTS);
			while (particle)
			{
				if (antiAlias)
				{
					glColor3f(particle->prevColor.r, particle->prevColor.g, particle->prevColor.b);
					glVertex3f(particle->prevPos.x,particle->prevPos.y,particle->prevPos.z);
				}
				glColor3f(particle->color.r, particle->color.g, particle->color.b);
				glVertex3f(particle->pos.x,particle->pos.y,particle->pos.z);
				particle = particle->next;
			}
			glEnd();
		}

		return TRUE;
	}
	return FALSE;
}
/// renderEmitter ///////////////////////////////////////////////////////////////


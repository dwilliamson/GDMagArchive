///////////////////////////////////////////////////////////////////////////////
//
// Particle.h : Particle System structure definition file
//
// Purpose:	Structure Definition of Particle System
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

#if !defined(PARTICLE_H__INCLUDED_)
#define PARTICLE_H__INCLUDED_

/// Particle Definitions //////////////////////////////////////////////////////
#define MAX_PARTICLES		4000		// MAXIMUM NUMBER OF PARTICLES
///////////////////////////////////////////////////////////////////////////////

#include "MathStuff.h"		// GET THE TYPE FOR MATH STRUCTURES

/// Structure Definitions ///////////////////////////////////////////////////////

// TODO: Could add Alpha component here to allow for blending
struct tColor
{
	float	r,g,b;
};

struct tParticle
{
	tParticle *prev,*next;				// LINK
	tVector	pos;						// CURRENT POSITION
	tVector	prevPos;					// PREVIOUS POSITION
	tVector	dir;						// CURRENT DIRECTION WITH SPEED
	int		life;						// HOW LONG IT WILL LAST
	tColor	color;						// CURRENT COLOR OF PARTICLE
	tColor	prevColor;					// LAST COLOR OF PARTICLE
	tColor  deltaColor;					// CHANGE OF COLOR
};

struct tEmitter
{
	long		id;							// EMITTER ID
	char		name[80];					// EMITTER NAME
	long		flags;						// EMITTER FLAGS
	// LINK INFO
	tEmitter	*prev;						// POINTER TO PARENT BONE
	tEmitter	*next;						// POINTER TO CHILDREN
	// TRANSFORMATION INFO
	tVector		pos;						// XYZ POSITION
	float		yaw, yawVar;				// YAW AND VARIATION
	float		pitch, pitchVar;			// PITCH AND VARIATION
	float		speed,speedVar;
	// Particle
	tParticle	*particle;					// NULL TERMINATED LINKED LIST
	int			totalParticles;				// TOTAL EMITTED AT ANY TIME
	int			particleCount;				// TOTAL EMITTED RIGHT NOW
	int			emitsPerFrame, emitVar;		// EMITS PER FRAME AND VARIATION
	int			life, lifeVar;				// LIFE COUNT AND VARIATION
	tColor		startColor, startColorVar;	// CURRENT COLOR OF PARTICLE
	tColor		endColor, endColorVar;		// CURRENT COLOR OF PARTICLE
	// Physics
	tVector		force;
};

///////////////////////////////////////////////////////////////////////////////

/// Support Function Definitions //////////////////////////////////////////////
BOOL initParticleSystem();
BOOL setDefaultEmitter(tEmitter *emitter);
BOOL initEmitter(tEmitter *emitter);
BOOL updateEmitter(tEmitter *emitter);		// DRAW THE SYSTEM FOR A FRAME
BOOL renderEmitter(tEmitter *emitter, BOOL antiAlias);
///////////////////////////////////////////////////////////////////////////////

#endif // !defined(PARTICLE_H__INCLUDED_)

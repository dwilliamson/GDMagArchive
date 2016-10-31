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

/// Structure Definitions ///////////////////////////////////////////////////////

struct tParticle
{
	tParticle *prev,*next;				// Link
	tVector	pos;						// Current Position
	tVector	rot;						// Euler angle orientation
	float	size;						// Render Scale
	int		texture;					// Texture Pointer
	tVector	color;						// Modulated Color
};

#endif // !defined(PARTICLE_H__INCLUDED_)

///////////////////////////////////////////////////////////////////////////////
//
// MathDefs.h : Math Structure Header File
//
// Purpose:	Declare Basic Math Structures
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
// Revisions:
//		Integrated into Kine Demo		8/18/98
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(MATHDEFS_H__INCLUDED_)
#define MATHDEFS_H__INCLUDED_

#define M_PI        3.14159265358979323846f
//#define HALF_PI	    1.57079632679489661923f
#define PI_TIMES_TWO	6.2831852
/// Trig Macros ///////////////////////////////////////////////////////////////
#define DEGTORAD(A)	((A * M_PI) / 180.0f)
#define RADTODEG(A)	((A * 180.0f) / M_PI)
#define MAX(a,b) ((a > b) ? a : b)
#define MIN(a,b)   ((a < b) ? a : b)
///////////////////////////////////////////////////////////////////////////////
typedef unsigned char uchar;

typedef int		BOOL;
typedef unsigned int	uint;
typedef unsigned short  ushort;
typedef unsigned char	byte;

typedef struct
{
	float u,v;
} t2DCoord;

struct tVector  
{
public:
	tVector ();

	union
	{
		float	x;
		float	r;
		float	pitch;
	};
	union
	{
		float	y;
		float	g;
		float	yaw;
	};
	union
	{
		float	z;
		float	b;
		float	roll;
	};

	void	Set( float fX, float fY, float fZ );
	void	CrossProduct(tVector *v1, tVector *v2);
	float	VectorSquaredDistance(tVector *v);
	float	VectorDistance(tVector *v);
	float	SquaredLength();
	float	Length(); 
	void	NormalizeVector();
	void	Lerp(tVector *v,float factor);
	float	DotProduct(tVector *v);
	float	DotProduct(float vx, float vy, float vz);
	void	MultVectorByMatrix(float *mat);
	void	MultVectorByRotMatrix(float *mat);
	tVector operator+( tVector arg );
	tVector operator-( tVector arg );
	tVector operator=( tVector arg );
	tVector operator=( float *arg );
	tVector operator=( float arg );
	tVector operator+=( tVector arg );
	tVector operator-=( tVector arg );
	tVector operator*( float arg );
	tVector operator/( float arg );
	tVector operator*=( float arg );
	tVector operator/=( float arg );
};

typedef struct
{
	float r;
	float g;
	float b;
	float a;
} tColor;

// NOT DECLARED AS float[4][4] BECAUSE OPENGL ACCESSES THIS STRANGLY
typedef struct
{
	float m[16];
} tMatrix;

// SOME STRUCTURES TO HELP ME ACCESS VERTEX DATA IN AN ARRAY
typedef struct
{
	float r,g,b;
	float x,y,z;
} tColoredVertex;

typedef struct
{
	float u,v;
	float x,y,z;
} tTexturedVertex;

typedef struct
{
	float u,v;
	float r,g,b;
	float x,y,z;
} tTexturedColoredVertex;

/// Quaternion Definitions ////////////////////////////////////////////////////
typedef struct
{
	float x,y,z,w;
} tQuaternion;
///////////////////////////////////////////////////////////////////////////////

#define MAKEVECTOR(a,vx,vy,vz)	a.x = vx; a.y = vy; a.z = vz;

void	IdentityMatrix(tMatrix *mat);

#endif // !defined(MATH_H__INCLUDED_)


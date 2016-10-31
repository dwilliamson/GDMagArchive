///////////////////////////////////////////////////////////////////////////////
//
// MathDefs.h : Math Structure Header File
//
// Purpose:	Declare Basic Math Structures
//
// Created:
//		JL 9/1/97		
// Revisions:
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) 2002 Darwin 3D, LLC., All Rights Reserved.
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(MATHDEFS_H__INCLUDED_)
#define MATHDEFS_H__INCLUDED_

#define M_PI			3.1415926
#define HALF_PI			1.5707963
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

class CVector  
{
public:
	CVector();
	CVector(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
	virtual ~CVector();

public:
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

	void	CrossProduct(CVector *v1, CVector *v2);
	float	VectorSquaredDistance(CVector *v);
	float	VectorDistance(CVector *v);
	float	SquaredLength();
	float	Length(); 
	void	NormalizeVector();
	void	Lerp(CVector *v,float factor);
	float	Dot(CVector *v);
	float	Dot(float vx, float vy, float vz);
	void	MultVectorByMatrix(float *mat);
	void	MultVectorByRotMatrix(float *mat);
	CVector operator+( CVector arg );
	CVector operator-( CVector arg );
	CVector operator=( CVector arg );
	CVector operator=( float *arg );
	CVector operator+=( CVector arg );
	CVector operator-=( CVector arg );
	CVector operator*( float arg );
	CVector operator/( float arg );
	CVector operator*=( float arg );
	CVector operator/=( float arg );
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


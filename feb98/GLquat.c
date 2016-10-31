/*SDOC*********************************************************************

  $Header$

  Module:	GLquat.c

  Author:	Nick Bobic

  Copyright (c) 1997 Nick Bobic 
                                               
  Description: Common (and not so common) Quaternion Functions

*********************************************************************EDOC*/

/*SDOC*********************************************************************

  Revision Record

	Date		Auth	Changes
	====		====	=======

	16AUG97		NB		Created file
	17AUG97		NB		Added some exotic functions

**********************************************************************EDOC*/




// includes
#include "GLquat.h"
#include <math.h>

// some math.h do not have M_PI definition
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419716939937510f
#endif


#define DELTA 1e-6     // error tolerance

#if defined (WIN32)
#pragma warning (disable:4244)	// disable conversion warnings (dbl -> fl)
#endif


/*SDOC***********************************************************************

  Name:		gluQuatToMat_EXT

  Action:   Converts quaternion representation of a rotation to a matrix
			representation

  Params:   GL_QUAT* (our quaternion), GLfloat (4x4 matrix)

  Returns:  nothing

  Comments: remember matrix (in OGL) is represented in COLUMN major form

***********************************************************************EDOC*/
void APIENTRY gluQuatToMat_EXT(GL_QUAT * quat, GLfloat m[4][4])
{

  GLfloat wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

  x2 = quat->x + quat->x; y2 = quat->y + quat->y; z2 = quat->z + quat->z;
  xx = quat->x * x2;   xy = quat->x * y2;   xz = quat->x * z2;
  yy = quat->y * y2;   yz = quat->y * z2;   zz = quat->z * z2;
  wx = quat->w * x2;   wy = quat->w * y2;   wz = quat->w * z2;

  m[0][0] = 1.0 - (yy + zz);
  m[0][1] = xy - wz;
  m[0][2] = xz + wy;
  m[0][3] = 0.0;
 
  m[1][0] = xy + wz;
  m[1][1] = 1.0 - (xx + zz);
  m[1][2] = yz - wx;
  m[1][3] = 0.0;

  m[2][0] = xz - wy;
  m[2][1] = yz + wx;
  m[2][2] = 1.0 - (xx + yy);
  m[2][3] = 0.0;

  m[3][0] = 0;
  m[3][1] = 0;
  m[3][2] = 0;
  m[3][3] = 1;

}


/*SDOC***********************************************************************

  Name:		gluEulerToQuat_EXT

  Action:   Converts representation of a rotation from Euler angles to
			quaternion representation

  Params:   GLfloat (roll), GLfloat (pitch), GLfloat (yaw), GL_QUAT* (quat)

  Returns:  nothing

  Comments: remember:	roll  - rotation around X axis
						pitch - rotation around Y axis
						yaw   - rotation around Z axis
			
			rotations are performed in the following order:
					yaw -> pitch -> roll

			Qfinal = Qyaw Qpitch Qroll

***********************************************************************EDOC*/
void APIENTRY gluEulerToQuat_EXT(GLfloat roll, GLfloat pitch, GLfloat yaw, 
													GL_QUAT * quat)
{
	
	GLfloat cr, cp, cy, sr, sp, sy, cpcy, spsy;

	cr = cos(roll/2);
	cp = cos(pitch/2);
	cy = cos(yaw/2);

	sr = sin(roll/2);
	sp = sin(pitch/2);
	sy = sin(yaw/2);
	
	cpcy = cp * cy;
	spsy = sp * sy;

	quat->w = cr * cpcy + sr * spsy;
	quat->x = sr * cpcy - cr * spsy;
	quat->y = cr * sp * cy + sr * cp * sy;
	quat->z = cr * cp * sy - sr * sp * cy;

}



/*SDOC***********************************************************************

  Name:		gluMatToQuat_EXT

  Action:   Converts matrix representation of a rotation to a quaternion
			representation

  Params:   GLfloat (matrix), GL_QUAT* (quat)

  Returns:  nothing

  Comments: remember matrix (in OGL) is represented in COLUMN major form

***********************************************************************EDOC*/
void APIENTRY gluMatToQuat_EXT(GLfloat m[4][4], GL_QUAT * quat)
{

  GLfloat  tr, s;
  GLfloat  q[4];
  GLint    i, j, k;

  int nxt[3] = {1, 2, 0};

  tr = m[0][0] + m[1][1] + m[2][2];

  // check the diagonal

  if (tr > 0.0) 
  {
    s = sqrt (tr + 1.0);

    quat->w = s / 2.0;
    
	s = 0.5 / s;

    quat->x = (m[1][2] - m[2][1]) * s;
    quat->y = (m[2][0] - m[0][2]) * s;
    quat->z = (m[0][1] - m[1][0]) * s;

  } else {		
	  
	  // diagonal is negative
    
	  i = 0;

      if (m[1][1] > m[0][0]) i = 1;
	  if (m[2][2] > m[i][i]) i = 2;
    
	  j = nxt[i];
      k = nxt[j];

      s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0);
      
	  q[i] = s * 0.5;

      if (s != 0.0) s = 0.5 / s;

	  q[3] = (m[j][k] - m[k][j]) * s;
      q[j] = (m[i][j] + m[j][i]) * s;
      q[k] = (m[i][k] + m[k][i]) * s;

	  quat->x = q[0];
	  quat->y = q[1];
	  quat->z = q[2];
	  quat->w = q[3];
  }

}


/*SDOC***********************************************************************

  Name:		gluQuatSlerp_EXT

  Action:	Smoothly (spherically, shortest path on a quaternion sphere) 
			interpolates between two UNIT quaternion positions

  Params:   GLQUAT (first and second quaternion), GLfloat (interpolation
			parameter [0..1]), GL_QUAT (resulting quaternion; inbetween)

  Returns:  nothing

  Comments: Most of this code is optimized for speed and not for readability

			As t goes from 0 to 1, qt goes from p to q.
		slerp(p,q,t) = (p*sin((1-t)*omega) + q*sin(t*omega)) / sin(omega)

***********************************************************************EDOC*/
void APIENTRY gluQuatSlerp_EXT(GL_QUAT * from, GL_QUAT * to, GLfloat t, 
															GL_QUAT * res)
{
        GLfloat           to1[4];
        GLdouble          omega, cosom, sinom;
        GLdouble          scale0, scale1;

        // calc cosine
        cosom = from->x * to->x + from->y * to->y + from->z * to->z
			       + from->w * to->w;

        // adjust signs (if necessary)
        if ( cosom < 0.0 )
		{
			cosom = -cosom;

			to1[0] = - to->x;
			to1[1] = - to->y;
			to1[2] = - to->z;
			to1[3] = - to->w;

        } else  {

			to1[0] = to->x;
			to1[1] = to->y;
			to1[2] = to->z;
			to1[3] = to->w;

        }

        // calculate coefficients

        if ( (1.0 - cosom) > DELTA ) 
		{
                // standard case (slerp)
                omega = acos(cosom);
                sinom = sin(omega);
                scale0 = sin((1.0 - t) * omega) / sinom;
                scale1 = sin(t * omega) / sinom;

        } else {        
			    // "from" and "to" quaternions are very close 
			    //  ... so we can do a linear interpolation

                scale0 = 1.0 - t;
                scale1 = t;
        }

		// calculate final values
		res->x = scale0 * from->x + scale1 * to1[0];
		res->y = scale0 * from->y + scale1 * to1[1];
		res->z = scale0 * from->z + scale1 * to1[2];
		res->w = scale0 * from->w + scale1 * to1[3];

}



/*SDOC***********************************************************************

  Name:		gluQuatLerp_EXT

  Action:   Linearly interpolates between two quaternion positions

  Params:   GLQUAT (first and second quaternion), GLfloat (interpolation
			parameter [0..1]), GL_QUAT (resulting quaternion; inbetween)

  Returns:  nothing

  Comments: fast but not as nearly as smooth as Slerp

***********************************************************************EDOC*/
void APIENTRY gluQuatLerp_EXT(GL_QUAT * from, GL_QUAT * to, GLfloat t, 
															GL_QUAT * res)
{
        GLfloat           to1[4];
        GLdouble          cosom;
        GLdouble          scale0, scale1;

        // calc cosine
        cosom = from->x * to->x + from->y * to->y + from->z * to->z
			       + from->w * to->w;

        // adjust signs (if necessary)
        if ( cosom < 0.0 )
		{
			to1[0] = - to->x;
			to1[1] = - to->y;
			to1[2] = - to->z;
			to1[3] = - to->w;

        } else  {

			to1[0] = to->x;
			to1[1] = to->y;
			to1[2] = to->z;
			to1[3] = to->w;

        }

 
		// interpolate linearly
        scale0 = 1.0 - t;
        scale1 = t;
 
		// calculate final values
		res->x = scale0 * from->x + scale1 * to1[0];
		res->y = scale0 * from->y + scale1 * to1[1];
		res->z = scale0 * from->z + scale1 * to1[2];
		res->w = scale0 * from->w + scale1 * to1[3];

}



/*SDOC***********************************************************************

  Name:		gluQuatNormalize_EXT

  Action:   Normalizes quaternion (i.e. w^2 + x^2 + y^2 + z^2 = 1)

  Params:   GL_QUAT* (quaternion)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatNormalize_EXT(GL_QUAT *quat)
{
    GLfloat	dist, square;

	square = quat->x * quat->x + quat->y * quat->y + quat->z * quat->z
		+ quat->w * quat->w;
	
	if (square > 0.0)
		dist = (GLfloat)(1.0 / sqrt(square));
	else dist = 1;

    quat->x *= dist;
    quat->y *= dist;
    quat->z *= dist;
    quat->w *= dist;
}



/*SDOC***********************************************************************

  Name:		gluQuatGetValue_EXT

  Action:   Disassembles quaternion to an axis and an angle

  Params:   GL_QUAT* (quaternion), GLfloat* (x, y, z - axis), GLfloat (angle)

  Returns:  nothing

  Comments: NOTE: vector has been split into x, y, z so that you do not have
			to change your vector library (i.e. greater portability)

			NOTE2: angle is in RADIANS

***********************************************************************EDOC*/
void APIENTRY gluQuatGetValue_EXT(GL_QUAT *quat, GLfloat *x, GLfloat *y, 
											GLfloat *z, GLfloat *radians)
{
    GLfloat	len;
    GLfloat tx, ty, tz;

	// cache variables
	tx = quat->x;
	ty = quat->y;
	tz = quat->z;

	len = tx * tx + ty * ty + tz * tz;

    if (len > DELTA) 
	{
		*x = tx * (1.0f / len);
		*y = ty * (1.0f / len);
		*z = tz * (1.0f / len);
	    *radians = (GLfloat)(2.0 * acos(quat->w));
    }

    else {
		*x = 0.0;
		*y = 0.0;
		*z = 1.0;
	    *radians = 0.0;
    }
}


/*SDOC***********************************************************************

  Name:		gluQuatSetValue_EXT

  Action:   Assembles quaternion from an axis and an angle

  Params:   GL_QUAT* (quaternion), GLfloat (x, y, z - axis), GLfloat (angle)

  Returns:  nothing

  Comments: NOTE: vector has been split into x, y, z so that you do not have
			to change your vector library (i.e. greater portability)

			NOTE2: angle has to be in RADIANS

***********************************************************************EDOC*/
void APIENTRY gluQuatSetValue_EXT(GL_QUAT *quat, GLfloat x, GLfloat y, 
												GLfloat z, GLfloat angle)
{
	GLfloat temp, dist;

	// normalize
	temp = x*x + y*y + z*z;

    dist = (GLfloat)(1.0 / sqrt(temp));

    x *= dist;
    y *= dist;
    z *= dist;

	quat->x = x;
	quat->y = y;
	quat->z = z;

	quat->w = (GLfloat)cos(angle / 2.0f);
	
}



/*SDOC***********************************************************************

  Name:		gluQuatScaleAngle_EXT

  Action:   Scales the rotation angle of a quaternion

  Params:   GL_QUAT* (quaternion), GLfloat (scale value)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatScaleAngle_EXT(GL_QUAT * quat, GLfloat scale)
{
    GLfloat x, y, z;	// axis
    GLfloat angle;		// and angle

	gluQuatGetValue_EXT(quat, &x, &y, &z, &angle);

    gluQuatSetValue_EXT(quat, x, y, z, (angle * scale));
}



/*SDOC***********************************************************************

  Name:		gluQuatInverse_EXT

  Action:   Inverts quaternion's rotation ( q^(-1) )

  Params:   GL_QUAT* (quaternion)

  Returns:  nothing

  Comments: none
Returns the inverse of the quaternion (1/q).  check conjugate
***********************************************************************EDOC*/
void APIENTRY gluQuatInverse_EXT(GL_QUAT *quat)
{
	GLfloat norm, invNorm;

	norm = quat->x * quat->x + quat->y * quat->y + quat->z * quat->z
		               + quat->w * quat->w;
	
	invNorm = (GLfloat) (1.0 / norm);
	
	quat->x = -quat->x * invNorm;
    quat->y = -quat->y * invNorm;
    quat->z = -quat->z * invNorm;
    quat->w =  quat->w * invNorm;

}


/*SDOC***********************************************************************

  Name:		gluQuatSetFromAx_EXT

  Action:   Constructs quaternion to rotate from one direction vector to 
			another

  Params:   GLfloat (x1, y1, z1 - from vector), 
			GLfloat (x2, y2, z2 - to vector), GL_QUAT* (resulting quaternion)

  Returns:  nothing

  Comments: Two vectors have to be UNIT vectors (so make sure you normalize
			them before calling this function
			Some parts are heavily optimized so readability is not so great :(
***********************************************************************EDOC*/
void APIENTRY gluQuatSetFromAx_EXT(GLfloat x1,GLfloat y1, GLfloat z1, 
						 GLfloat x2,GLfloat y2, GLfloat z2, GL_QUAT *quat)

{
    GLfloat tx, ty, tz, temp, dist;

    GLfloat	cost, len, ss;

	// get dot product of two vectors
    cost = x1 * x2 + y1 * y2 + z1 * z2;

    // check if parallel
    if (cost > 0.99999f) {
	quat->x = quat->y = quat->z = 0.0f;
	quat->w = 1.0f;
	return;
    }
    else if (cost < -0.99999f) {		// check if opposite

	// check if we can use cross product of from vector with [1, 0, 0]
	tx = 0.0;
	ty = x1;
	tz = -y1;

	len = sqrt(ty*ty + tz*tz);

	if (len < DELTA)
	{
		// nope! we need cross product of from vector with [0, 1, 0]
		tx = -z1;
		ty = 0.0;
		tz = x1;
	}

	// normalize
	temp = tx*tx + ty*ty + tz*tz;

    dist = (GLfloat)(1.0 / sqrt(temp));

    tx *= dist;
    ty *= dist;
    tz *= dist;
	
	quat->x = tx;
	quat->y = ty;
	quat->z = tz;
	quat->w = 0.0;

	return;
    }

	// ... else we can just cross two vectors

	tx = y1 * z2 - z1 * y2;
	ty = z1 * x2 - x1 * z2;
	tz = x1 * y2 - y1 * x2;

	temp = tx*tx + ty*ty + tz*tz;

    dist = (GLfloat)(1.0 / sqrt(temp));

    tx *= dist;
    ty *= dist;
    tz *= dist;


    // we have to use half-angle formulae (sin^2 t = ( 1 - cos (2t) ) /2)
	
	ss = (float)sqrt(0.5f * (1.0f - cost));

	tx *= ss;
	ty *= ss;
    tz *= ss;

    // scale the axis to get the normalized quaternion
    quat->x = tx;
    quat->y = ty;
    quat->z = tz;

    // cos^2 t = ( 1 + cos (2t) ) / 2
    // w part is cosine of half the rotation angle
    quat->w = (float)sqrt(0.5f * (1.0f + cost));

}




/*SDOC***********************************************************************

  Name:		gluQuatMul_EXT

  Action:   Multiplies two quaternions

  Params:   GL_QUAT ( q1 * q2 = res)

  Returns:  nothing

  Comments: NOTE: multiplication is not commutative

***********************************************************************EDOC*/
void APIENTRY gluQuatMul_EXT(GL_QUAT* q1, GL_QUAT* q2, GL_QUAT* res)
{

	res->x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
	res->y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
	res->z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
	res->w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;


	// make sure the resulting quaternion is a unit quat.
	gluQuatNormalize_EXT(res);

}


/*SDOC***********************************************************************

  Name:		gluQuatAdd_EXT

  Action:   Adds two quaternions

  Params:   GL_QUAT* (q1 + q2 = res)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatAdd_EXT(GL_QUAT* q1, GL_QUAT* q2, GL_QUAT* res)
{
	res->x = q1->x + q2->x;
	res->y = q1->y + q2->y;
	res->z = q1->z + q2->z;
	res->w = q1->w + q2->w;

	// make sure the resulting quaternion is a unit quat.
	gluQuatNormalize_EXT(res);
}


/*SDOC***********************************************************************

  Name:		gluQuatSub_EXT

  Action:   Subtracts two quaternions

  Params:   GL_QUAT* (q1 - q2 = res)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatSub_EXT(GL_QUAT* q1, GL_QUAT* q2, GL_QUAT* res)
{
	res->x = q1->x - q2->x;
	res->y = q1->y - q2->y;
	res->z = q1->z - q2->z;
	res->w = q1->w - q2->w;

	// make sure the resulting quaternion is a unit quat.
	gluQuatNormalize_EXT(res);
}


/*SDOC***********************************************************************

  Name:		gluQuatDiv_EXT

  Action:   Divide two quaternions

  Params:   GL_QUAT* (q1 / q2 = res)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatDiv_EXT(GL_QUAT* q1, GL_QUAT* q2, GL_QUAT* res)
{
	GL_QUAT q, r, s;

	gluQuatCopy_EXT(q2, &q);

	// invert vector
    q.x = -q.x;
    q.y = -q.y;
    q.z = -q.z;
	
	gluQuatMul_EXT(q1, &q, &r);
	gluQuatMul_EXT(&q, &q, &s);

	res->x = r.x / s.w;
	res->y = r.y / s.w;
	res->z = r.z / s.w;
	res->w = r.w / s.w;

}


/*SDOC***********************************************************************

  Name:		gluQuatCopy_EXT

  Action:   copies q1 into q2

  Params:   GL_QUAT* (q1 and q2)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatCopy_EXT(GL_QUAT* q1, GL_QUAT* q2)
{
	q2->x = q1->x;
	q2->y = q1->y;
	q2->z = q1->z;
	q2->w = q1->w;
}



/*SDOC***********************************************************************

  Name:		gluQuatSquare_EXT

  Action:   Square quaternion

  Params:   GL_QUAT* (q1 * q1 = res)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatSquare_EXT(GL_QUAT* q1, GL_QUAT* res)
{
	GLfloat  tt;


	tt = 2 * q1->w;
	res->x = tt * q1->x;
	res->y = tt * q1->y;
	res->z = tt * q1->z;
	res->w = (q1->w * q1->w - q1->x * q1->x - q1->y * q1->y - q1->z * q1->z);
}


/*SDOC***********************************************************************

  Name:		gluQuatSqrt_EXT

  Action:   Find square root of a quaternion

  Params:   GL_QUAT* (sqrt(q1) = res)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatSqrt_EXT(GL_QUAT* q1, GL_QUAT* res)
{
	GLfloat  length, m, r1, r2;
	GL_QUAT r;

	length = sqrt (q1->w * q1->w + q1->x * q1->x + q1->y * q1->y);
	if (length != 0.0) 
		length = 1.0 / length; 
	else length = 1.0;

	r.x = q1->x * length;
	r.y = q1->z * length;
	r.z = 0.0f;
	r.w = q1->w * length;

	m = 1.0 / sqrt (r.w * r.w + r.x * r.x);
	r1 = sqrt ((1.0 + r.y) * 0.5);
	r2 = sqrt ((1.0 - r.y) * 0.5);

	res->x = sqrt (length) * r2 * r.x * m;
	res->y = sqrt (length) * r1;
	res->z = q1->z;
	res->w = sqrt (length) * r1 * r.w * m;

}


/*SDOC***********************************************************************

  Name:		gluQuatDot_EXT

  Action:   Computes the dot product of two unit quaternions

  Params:   GL_QUAT (first and second quaternion)

  Returns:  (GLfloat) Dot product

  Comments: Quaternion has to be normalized (i.e. it's a unit quaternion)

***********************************************************************EDOC*/
GLfloat APIENTRY gluQuatDot_EXT(GL_QUAT* q1, GL_QUAT* q2)
{
  return (GLfloat)(q1->w * q2->w + q1->x * q2->x + q1->y * q2->y+q1->z*q2->z);
}


/*SDOC***********************************************************************

  Name:		gluQuatLength_EXT

  Action:   Calculates the length of a quaternion

  Params:   GL_QUAT* (quaternion)

  Returns:  GLfloat (length)

  Comments: none

***********************************************************************EDOC*/
GLfloat APIENTRY gluQuatLength_EXT(GL_QUAT* q1)
{
  return sqrt (q1->w * q1->w + q1->x * q1->x + q1->y * q1->y + q1->z * q1->z);
}


/*SDOC***********************************************************************

  Name:		gluQuatNegate_EXT

  Action:   Negates vector part of a quaternion

  Params:   GL_QUAT (source and destination quaternion)

  Returns:  nothing

  Comments: Source quaternion does NOT have to be normalized 

***********************************************************************EDOC*/
void APIENTRY gluQuatNegate_EXT(GL_QUAT* q1, GL_QUAT* q2)
{
	gluQuatCopy_EXT(q1, q2);

	gluQuatNormalize_EXT(q2);
	q2->x = -q2->x;
	q2->y = -q2->y;
	q2->z = -q2->z;
}

/*SDOC***********************************************************************

  Name:		gluQuatExp_EXT

  Action:   Calculates exponent of a quaternion

  Params:   GL_QUAT* (Source and destination quaternion)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatExp_EXT(GL_QUAT* q1, GL_QUAT* q2)
{
	GLfloat  len1, len2;

	len1 = (GLfloat) sqrt (q1->x * q1->x + q1->y * q1->y + q1->z * q1->z);
	if (len1 > 0.0) 
		len2 = (GLfloat)sin(len1) / len1; 
	else 
		len2 = 1.0;

	q2->x = q1->x * len2;
	q2->y = q1->y * len2;
	q2->z = q1->z * len2;
	q2->w = cos (len1);
}


/*SDOC***********************************************************************

  Name:		gluQuatLog_EXT

  Action:   Calculates natural logarithm of a quaternion

  Params:   GL_QUAT* (Source and destination quaternion)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatLog_EXT(GL_QUAT* q1, GL_QUAT* q2)
{
	GLfloat  length;

	length = sqrt (q1->x * q1->x + q1->y * q1->y + q1->z * q1->z);

	//make sure we do not divide by 0
	if (q1->w != 0.0) 
		length = atan (length / q1->w); 
	else length = (GLfloat)M_PI/2;

	q2->w = 0.0f;
	q2->x = q1->x * length;
	q2->y = q1->y * length;
	q2->z = q1->z * length;
}

/*SDOC***********************************************************************

  Name:		gluQuatLnDif_EXT

  Action:   Computes the "natural log difference" of two quaternions,
			q1 and q2 as  ln(qinv(q1)*q2)

  Params:   GL_QUAT* (Source quaternions  and a destination quaternion)

  Returns:  nothing

  Comments: none

***********************************************************************EDOC*/
void APIENTRY gluQuatLnDif_EXT(GL_QUAT *q1, GL_QUAT *q2, GL_QUAT *res)
{

	GL_QUAT inv, dif, temp;
	GLfloat  len, len1, s;

	qt_inverse (a, &inv);
	qt_mul (&inv, b, &dif);
	len = sqrt (dif.x*dif.x + dif.y*dif.y + dif.z*dif.z);
	s = qt_dot (a, b);
	if (s != 0.0) len1 = atan (len / s); else len1 = M_PI/2;
	if (len != 0.0) len1 /= len;
	temp.w = 0.0;
	temp.x = dif.x * len1;
	temp.y = dif.y * len1;
	temp.z = dif.z * len1;
	qt_copy (&temp, out);
}




// cleanup stuff we changed
#if defined (WIN32)
#pragma warning( default : 4244 )	// set it to default again
#endif
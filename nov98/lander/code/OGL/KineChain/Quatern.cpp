///////////////////////////////////////////////////////////////////////////////
//
// Quaternion.cpp : Quaternion System structure implementation file
//
// Purpose:	Quaternion Conversion and Evaluation Functions
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
// ALSO NOT TOTALLY OPTIMIZED AND TRICKED OUT FOR CLARITY
//
// Created:
//		JL 9/1/97		
//
// Sources:
//	Shoemake, Ken, "Animating Rotations with Quaternion Curves"
//		Computer Graphics 85, pp. 245-254
//	Watt and Watt, Advanced Animation and Rendering Techniques
//		Addison Wesley, pp. 360-368
//  Shoemake, Graphic Gems II.
//
// Notes:
//			There are a couple of methods of conversion here so it
// can be played around with a bit.  One is more clear and the other
// is a bit faster.  
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1997 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <math.h>
#include "MathDefs.h"
#include "skeleton.h"
#include "quatern.h"

///////////////////////////////////////////////////////////////////////////////
// Function:	CopyVector
// Purpose:		Copy a vector
// Arguments:	pointer to destination and source
///////////////////////////////////////////////////////////////////////////////
void CopyVector(tVector *dest, tVector *src)
{
	dest->x = src->x;
	dest->y = src->y;
	dest->z = src->z;
}
//// CopyVector ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	ScaleVector
// Purpose:		Scale a vector
// Arguments:	pointer to vector and scale factor
///////////////////////////////////////////////////////////////////////////////
void ScaleVector(tVector *vect, float scale)
{
	vect->x *= scale;
	vect->y *= scale;
	vect->z *= scale;
}
//// ScaleVector ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	AddVectors
// Purpose:		Add two vectors
// Arguments:	pointer to vectors and dest
///////////////////////////////////////////////////////////////////////////////
void AddVectors(tVector *vect1, tVector *vect2, tVector *dest)
{
	dest->x = vect1->x + vect2->x;
	dest->y = vect1->y + vect2->y;
	dest->z = vect1->z + vect2->z;
}
//// AddVectors ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	DotVectors
// Purpose:		Compute the dot product of two vectors
// Arguments:	pointer to vectors
// Returns:		Dot product
///////////////////////////////////////////////////////////////////////////////
float DotVectors(tVector *vect1, tVector *vect2)
{
	return	(vect1->x * vect2->x) + 
			(vect1->y * vect2->y) + 
			(vect1->z * vect2->z);
}
//// DotVectors ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	CrossVectors
// Purpose:		Computes the cross product of two vectors
// Arguments:	pointer to vectors and dest
///////////////////////////////////////////////////////////////////////////////
void CrossVectors(tVector *vect1, tVector *vect2, tVector *dest)
{
	// COMPUTE THE CROSS PRODUCT
	dest->x = (vect1->y * vect2->z) - (vect1->z * vect2->y);
	dest->y = (vect1->z * vect2->x) - (vect1->x * vect2->z);
	dest->z = (vect1->x * vect2->y) - (vect1->y * vect2->x);
}
//// CrossVectors /////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	MultQuaternions
// Purpose:		Computes the product of two quaternions
// Arguments:	pointer to quaternions and dest
///////////////////////////////////////////////////////////////////////////////
void MultQuaternions(tQuaternion *quat1, tQuaternion *quat2, tQuaternion *dest)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tQuaternion v1,v2,v3,vf;
///////////////////////////////////////////////////////////////////////////////

	CopyVector((tVector *)&v1, (tVector *)quat1);		// COPY OFF THE VECTOR PART OF THE QUAT1
	ScaleVector((tVector *)&v1,quat2->w);	// MULTIPLY IT BY THE SCALAR PART OF QUAT2

	CopyVector((tVector *)&v2, (tVector *)quat2);		// COPY OFF THE VECTOR PART OF THE QUAT1
	ScaleVector((tVector *)&v2,quat1->w);	// MULTIPLY IT BY THE SCALAR PART OF QUAT2

	CrossVectors((tVector *)quat2,(tVector *)quat1,(tVector *)&v3);

	AddVectors((tVector *)&v1, (tVector *)&v2, (tVector *)&vf);
	AddVectors((tVector *)&v3, (tVector *)&vf, (tVector *)&vf);

	vf.w = (quat1->w * quat2->w) - DotVectors((tVector *)quat1,(tVector *)quat2);

	dest->x = vf.x;
	dest->y = vf.y;
	dest->z = vf.z;
	dest->w = vf.w;
}
//// MultQuaternions //////////////////////////////////////////////////////////

/* AN OPTIMIZATION/REORGANIZATION OF ABOVE CODE - NOT AS CLEAR 
   I THINK THIS IS SIMILAR TO GRAPHIC GEMS THOUGH I DON'T HAVE THE REF HANDY
   THE MATH CHECKS OUT THOUGH */
///////////////////////////////////////////////////////////////////////////////
// Function:	MultQuaternions2
// Purpose:		Computes the product of two quaternions
// Arguments:	pointer to quaternions and dest
///////////////////////////////////////////////////////////////////////////////
void MultQuaternions2(tQuaternion *quat1, tQuaternion *quat2, tQuaternion *dest)
{
    tQuaternion tmp;
    tmp.x =		quat2->w * quat1->x + quat2->x * quat1->w +
				quat2->y * quat1->z - quat2->z * quat1->y;
    tmp.y  =	quat2->w * quat1->y + quat2->y * quat1->w +
				quat2->z * quat1->x - quat2->x * quat1->z;
    tmp.z  =	quat2->w * quat1->z + quat2->z * quat1->w +
				quat2->x * quat1->y - quat2->y * quat1->x;
    tmp.w  =	quat2->w * quat1->w - quat2->x * quat1->x -
				quat2->y * quat1->y - quat2->z * quat1->z;
    dest->x = tmp.x; dest->y = tmp.y;
    dest->z = tmp.z; dest->w = tmp.w;
}
//// MultQuaternions2 //////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Function:	NormalizeQuaternion
// Purpose:		Normalize a Quaternion
// Arguments:	a quaternion to set
// Discussion:  Quaternions must follow the rules of x^2 + y^2 + z^2 + w^2 = 1
//				This function insures this
///////////////////////////////////////////////////////////////////////////////
void NormalizeQuaternion(tQuaternion *quat)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float magnitude;
///////////////////////////////////////////////////////////////////////////////
	// FIRST STEP, FIND THE MAGNITUDE
	magnitude = (quat->x * quat->x) + 
				(quat->y * quat->y) + 
				(quat->z * quat->z) + 
				(quat->w * quat->w);

	// DIVIDE BY THE MAGNITUDE TO NORMALIZE
	quat->x = quat->x / magnitude;
	quat->y = quat->y / magnitude;
	quat->z = quat->z / magnitude;
	quat->w = quat->w / magnitude;
}
// NormalizeQuaternion  ///////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// THESE TWO PROCEDURES ARE FUNCTIONALLY EQUIVALENT.  TWO METHODS TO CONVERT
// A SERIES OF ROTATIONS TO QUATERNIONS.  
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	EulerToQuaternion
// Purpose:		Convert a set of Euler angles to a Quaternion
// Arguments:	A rotation set of 3 angles, a quaternion to set
// Discussion:  As the order of rotations is important I am
//				using the Quantum Mechanics convention of (X,Y,Z)
//				a Yaw-Pitch-Roll (Y,X,Z) system would have to be
//				adjusted.  It is more efficient this way though.
///////////////////////////////////////////////////////////////////////////////
void EulerToQuaternion(tVector *rot, tQuaternion *quat)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float rx,ry,rz,tx,ty,tz,cx,cy,cz,sx,sy,sz,cc,cs,sc,ss;
///////////////////////////////////////////////////////////////////////////////
	// FIRST STEP, CONVERT ANGLES TO RADIANS
	rx =  (rot->x * (float)M_PI) / (360 / 2);
	ry =  (rot->y * (float)M_PI) / (360 / 2);
	rz =  (rot->z * (float)M_PI) / (360 / 2);
	// GET THE HALF ANGLES
	tx = rx * (float)0.5;
	ty = ry * (float)0.5;
	tz = rz * (float)0.5;
	cx = (float)cos(tx);
	cy = (float)cos(ty);
	cz = (float)cos(tz);
	sx = (float)sin(tx);
	sy = (float)sin(ty);
	sz = (float)sin(tz);

	cc = cx * cz;
	cs = cx * sz;
	sc = sx * cz;
	ss = sx * sz;

	quat->x = (cy * sc) - (sy * cs);
	quat->y = (cy * ss) + (sy * cc);
	quat->z = (cy * cs) - (sy * sc);
	quat->w = (cy * cc) + (sy * ss);

	// INSURE THE QUATERNION IS NORMALIZED
	// PROBABLY NOT NECESSARY IN MOST CASES
	NormalizeQuaternion(quat);
}
// EulerToQuaternion  /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	EulerToQuaternion2
// Purpose:		Convert a set of Euler angles to a Quaternion
// Arguments:	A rotation set of 3 angles, a quaternion to set
// Discussion:  This is a second variation.  It creates a
//				Series of quaternions and multiplies them together
//				It would be easier to extend this for other rotation orders
///////////////////////////////////////////////////////////////////////////////
void EulerToQuaternion2(tVector *rot, tQuaternion *quat)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float rx,ry,rz,ti,tj,tk;
	tQuaternion qx,qy,qz,qf;
///////////////////////////////////////////////////////////////////////////////
	// FIRST STEP, CONVERT ANGLES TO RADIANS
	rx =  (rot->x * (float)M_PI) / (360 / 2);
	ry =  (rot->y * (float)M_PI) / (360 / 2);
	rz =  (rot->z * (float)M_PI) / (360 / 2);
	// GET THE HALF ANGLES
	ti = rx * (float)0.5;
	tj = ry * (float)0.5;
	tk = rz * (float)0.5;

	qx.x = (float)sin(ti); qx.y = 0.0; qx.z = 0.0; qx.w = (float)cos(ti);
	qy.x = 0.0; qy.y = (float)sin(tj); qy.z = 0.0; qy.w = (float)cos(tj);
	qz.x = 0.0; qz.y = 0.0; qz.z = (float)sin(tk); qz.w = (float)cos(tk);

	MultQuaternions(&qx,&qy,&qf);
	MultQuaternions(&qf,&qz,&qf);

// ANOTHER TEST VARIATION
//	MultQuaternions2(&qx,&qy,&qf);
//	MultQuaternions2(&qf,&qz,&qf);

	// INSURE THE QUATERNION IS NORMALIZED
	// PROBABLY NOT NECESSARY IN MOST CASES
	NormalizeQuaternion(&qf);

	quat->x = qf.x;
	quat->y = qf.y;
	quat->z = qf.z;
	quat->w = qf.w;

}
// EulerToQuaternion2  /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	QuatToAxisAngle
// Purpose:		Convert a Quaternion to Axis Angle representation
// Arguments:	A quaternion to convert, a axisAngle to set
///////////////////////////////////////////////////////////////////////////////
void QuatToAxisAngle(tQuaternion *quat,tQuaternion *axisAngle)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float scale,tw;
///////////////////////////////////////////////////////////////////////////////
	tw = (float)acos(quat->w) * 2;
	scale = (float)sin(tw / 2.0);
	axisAngle->x = quat->x / scale;
	axisAngle->y = quat->y / scale;
	axisAngle->z = quat->z / scale;

	// NOW CONVERT THE ANGLE OF ROTATION BACK TO DEGREES
	axisAngle->w = (tw * (360 / 2)) / (float)M_PI;
}
// QuatToAxisAngle  /////////////////////////////////////////////////////////

// THIS ROUTINE IS REALLY FOR THE APRIL ARTICLE BUT IF YOU WANT TO PLAY
// AROUND WITH IT, HERE IT IS.

#define DELTA	0.0001		// DIFFERENCE AT WHICH TO LERP INSTEAD OF SLERP
///////////////////////////////////////////////////////////////////////////////
// Function:	SlerpQuat
// Purpose:		Spherical Linear Interpolation Between two Quaternions
// Arguments:	Two Quaternions, blend factor, result quaternion
// Notes:		The comments explain the handling of the special cases.
//				The comments in the magazine were wrong.  Adjust the
//				DELTA constant to play with the LERP vs. SLERP level
///////////////////////////////////////////////////////////////////////////////
void SlerpQuat(tQuaternion *quat1,tQuaternion *quat2,float slerp, tQuaternion *result)
{
/// Local Variables ///////////////////////////////////////////////////////////
	double omega,cosom,sinom,scale0,scale1;
///////////////////////////////////////////////////////////////////////////////
	// USE THE DOT PRODUCT TO GET THE COSINE OF THE ANGLE BETWEEN THE
	// QUATERNIONS
	cosom = quat1->x * quat2->x + 
			quat1->y * quat2->y + 
			quat1->z * quat2->z + 
			quat1->w * quat2->w; 

	// CHECK A COUPLE OF SPECIAL CASES. 
	// MAKE SURE THE TWO QUATERNIONS ARE NOT EXACTLY OPPOSITE? (WITHIN A LITTLE SLOP)
	if ((1.0 + cosom) > DELTA)
	{
		// ARE THEY MORE THAN A LITTLE BIT DIFFERENT? AVOID A DIVIDED BY ZERO AND LERP IF NOT
		if ((1.0 - cosom) > DELTA) {
			// YES, DO A SLERP
			omega = acos(cosom);
			sinom = sin(omega);
			scale0 = sin((1.0 - slerp) * omega) / sinom;
			scale1 = sin(slerp * omega) / sinom;
		} else {
			// NOT A VERY BIG DIFFERENCE, DO A LERP
			scale0 = 1.0 - slerp;
			scale1 = slerp;
		}
		result->x = scale0 * quat1->x + scale1 * quat2->x;
		result->y = scale0 * quat1->y + scale1 * quat2->y;
		result->z = scale0 * quat1->z + scale1 * quat2->z;
		result->w = scale0 * quat1->w + scale1 * quat2->w;
	} else {
		// THE QUATERNIONS ARE NEARLY OPPOSITE SO TO AVOID A DIVIDED BY ZERO ERROR
		// CALCULATE A PERPENDICULAR QUATERNION AND SLERP THAT DIRECTION
		result->x = -quat2->y;
		result->y = quat2->x;
		result->z = -quat2->w;
		result->w = quat2->z;
		scale0 = sin((1.0 - slerp) * (float)HALF_PI);
		scale1 = sin(slerp * (float)HALF_PI);
		result->x = scale0 * quat1->x + scale1 * result->x;
		result->y = scale0 * quat1->y + scale1 * result->y;
		result->z = scale0 * quat1->z + scale1 * result->z;
		result->w = scale0 * quat1->w + scale1 * result->w;
	}

}
// SlerpQuat  /////////////////////////////////////////////////////////////////

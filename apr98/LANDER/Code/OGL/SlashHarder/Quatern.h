///////////////////////////////////////////////////////////////////////////////
//
// Quaternion.h : Quaternion System structure definition file
//
// Purpose:	Quaternion Conversion and Evaluation Functions
//
// I DIDN'T PUT THESE IN A C++ CLASS FOR CROSS PLATFORM COMPATIBILITY
// SINCE THE ENGINE MAY BE IMPLEMENTED ON CONSOLES AND OTHER SYSTEMS
//
// Created:
//		JL 9/1/97		
//
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1997 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(QUATERN_H__INCLUDED_)
#define QUATERN_H__INCLUDED_

/// Quaternion Definitions ////////////////////////////////////////////////////
typedef struct
{
	float x,y,z,w;
} tQuaternion;

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Function:	EulerToQuaternion
// Purpose:		Convert a set of Euler angles to a Quaternion
// Arguments:	A rotation set of 3 angles, a quaternion to set
///////////////////////////////////////////////////////////////////////////////
void EulerToQuaternion(tVector *rot, tQuaternion *quat);

///////////////////////////////////////////////////////////////////////////////
// Function:	EulerToQuaternion2
// Purpose:		Convert a set of Euler angles to a Quaternion
// Arguments:	A rotation set of 3 angles, a quaternion to set
///////////////////////////////////////////////////////////////////////////////
void EulerToQuaternion2(tVector *rot, tQuaternion *quat);

///////////////////////////////////////////////////////////////////////////////
// Function:	QuatToAxisAngle
// Purpose:		Convert a Quaternion to Axis Angle representation
// Arguments:	A quaternion to convert, a axisAngle to set
// Discussion:  As the order of rotations is important I am
//				using the Quantum Mechanics convention of (X,Y,Z)
//				a Yaw-Pitch-Roll (Y,X,Z) system would have to be
//				adjusted
///////////////////////////////////////////////////////////////////////////////
void QuatToAxisAngle(tQuaternion *quat,tQuaternion *axisAngle);

///////////////////////////////////////////////////////////////////////////////
// Function:	SlerpQuat
// Purpose:		Spherical Linear Interpolation Between two Quaternions
// Arguments:	Two Quaternions, blend factor, result quaternion
///////////////////////////////////////////////////////////////////////////////
void SlerpQuat(tQuaternion *quat1,tQuaternion *quat2,float slerp, tQuaternion *result);

#endif // !defined(QUATERN_H__INCLUDED_)

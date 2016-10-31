// IawMatrix.cpp App Wizard Version 2.0 Beta 1
// ----------------------------------------------------------------------
// 
// Copyright © 2001 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative works of this 
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  This software is provided "AS IS." 
//
// Intel specifically disclaims all warranties, express or implied, and all liability,
// including consequential and other indirect damages, for the use of this software, 
// including liability for infringement of any proprietary rights, and including the 
// warranties of merchantability and fitness for a particular purpose.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// ----------------------------------------------------------------------
//
// PURPOSE:
//    
// IawMatrix.cpp: implementation of the CIawMatrix class
//
// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#include "..\IawFrameWork.h"

//
// Vector class
//

// Constructors...
IawVector::IawVector()
{
}

IawVector::IawVector(const IawVector& rVector)
{
	mX = rVector.mX;
	mY = rVector.mY;
	mZ = rVector.mZ;
}

IawVector::IawVector(float f)
{
	mX = mY = mZ = f;
}

IawVector::IawVector(float x, float y, float z)
{
	mX = x;
	mY = y;
	mZ = z;
}

// Destructor
IawVector::~IawVector()
{
}

// Assignment operators...
IawVector IawVector::operator= (const float f)
{
	mX = f;
	mY = f;
	mZ = f;
	return *this;
}

IawVector IawVector::operator+= (const IawVector rV)
{
	mX += rV.mX;
	mY += rV.mY;
	mZ += rV.mZ;
	return *this;
}

IawVector IawVector::operator-= (const IawVector rV)
{
	mX -= rV.mX;
	mY -= rV.mY;
	mZ -= rV.mZ;
	return *this;
}

IawVector IawVector::operator*= (const IawVector rV)
{
	mX *= rV.mX;
	mY *= rV.mY;
	mZ *= rV.mZ;
	return *this;
}

IawVector IawVector::operator/= (const IawVector rV)
{
	mX /= rV.mX;
	mY /= rV.mY;
	mZ /= rV.mZ;
	return *this;
}

IawVector IawVector::operator*= (float s)
{
	mX *= s;
	mY *= s;
	mZ *= s;
	return *this;
}

IawVector IawVector::operator/= (float s)
{
	mX /= s;
	mY /= s;
	mZ /= s;
	return *this;
}

// Unary operators...
IawVector operator+ (const IawVector& rV)
{
	IawVector temp = rV;
	return temp;
}

IawVector operator- (const IawVector& rV)
{
	IawVector temp = rV;
	temp.mX = -rV.mX;
	temp.mY = -rV.mY;
	temp.mZ = -rV.mZ;
	return temp;
}

IawVector operator+ (const IawVector& rV1, const IawVector& rV2)
{
	IawVector temp;
	temp.mX = rV1.mX + rV2.mX;
	temp.mY = rV1.mY + rV2.mY;
	temp.mZ = rV1.mZ + rV2.mZ;
	return temp;
}

IawVector operator- (const IawVector& rV1, const IawVector& rV2)
{ 
	IawVector temp;
	temp.mX = rV1.mX - rV2.mX;
	temp.mY = rV1.mY - rV2.mY;
	temp.mZ = rV1.mZ - rV2.mZ;
	return temp;
}

IawVector operator* (const IawVector& rV, float f)
{ 
	IawVector temp;
	temp.mX = rV.mX * f;
	temp.mY = rV.mY * f;
	temp.mZ = rV.mZ * f;
	return temp;
}

IawVector operator* (float f, const IawVector& rV)
{ 
	IawVector temp;
	temp.mX = rV.mX * f;
	temp.mY = rV.mY * f;
	temp.mZ = rV.mZ * f;
	return temp;
}

IawVector operator/ (const IawVector& rV, float f)
{ 
	IawVector temp;
	float invf = 1.0f / f;
	temp.mX = rV.mX * invf;
	temp.mY = rV.mY * invf;
	temp.mZ = rV.mZ * invf;
	return temp;
}

// Memberwise multiplication and division
IawVector operator* (const IawVector& rV1, const IawVector& rV2)
{ 
	IawVector temp;
	temp.mX = rV1.mX * rV2.mX;
	temp.mY = rV1.mY * rV2.mY;
	temp.mZ = rV1.mZ * rV2.mZ;
	return temp;
}

IawVector operator/ (const IawVector& rV1, const IawVector& rV2)
{ 
	IawVector temp;
	temp.mX = rV1.mX / rV2.mX;
	temp.mY = rV1.mY / rV2.mY;
	temp.mZ = rV1.mZ / rV2.mZ;
	return temp;
}

float Magnitude(const IawVector& rV)
{
	return (float)sqrt((rV.mX * rV.mX) + (rV.mY * rV.mY) + (rV.mZ * rV.mZ));
}

IawVector Normalize(const IawVector& rV)
{
	IawVector temp;
	float invMag = 1.0f/(float)sqrt((rV.mX * rV.mX) + (rV.mY * rV.mY) + (rV.mZ * rV.mZ));
	temp.mX = rV.mX * invMag;
	temp.mY = rV.mY * invMag;
	temp.mZ = rV.mZ * invMag;
	return temp;
}

float DotProduct(const IawVector& rV1, const IawVector& rV2)
{
	return ((rV1.mX * rV2.mX) + (rV1.mY * rV2.mY) + (rV1.mZ * rV2.mZ));
}

IawVector CrossProduct(const IawVector& rV1, const IawVector& rV2)
{
	IawVector temp;
	temp.mX = (rV1.mY * rV2.mZ - rV1.mZ * rV2.mY);
	temp.mY = (rV1.mX * rV2.mZ - rV1.mZ * rV2.mX);
	temp.mZ = (rV1.mX * rV2.mY - rV1.mY * rV2.mX);
	return temp;
}


//
// Matrix class
//

// Constructors...
IawMatrix::IawMatrix(bool setIdentity)
{
	if (setIdentity)
		SetIdentity();
}

IawMatrix::IawMatrix(IawMatrix& rMx)
{
	mMx = rMx.mMx;
}

// Destructor...
IawMatrix::~IawMatrix()
{
}

// Set the Identity matrix
void IawMatrix::SetIdentity()
{
	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			mMx(i,j) = (i == j) ? 1.0f : 0.0f;
}

// Pre-multiply the matrix by rMx
void IawMatrix::PreMultiply(IawMatrix& rMx)
{
	int i, j, k;
	IawMatrix mx_temp = *this;

	memset(&mMx, 0, sizeof(float) * 16);

	for( i=0; i<4; i++ )
		for (j=0; j<4; j++ )
			for (k=0; k<4; k++)
				mMx(i,j) += rMx.mMx(i,k) * mx_temp.mMx(k,j);
}

// Post-multiply the matrix by rMx
void IawMatrix::PostMultiply(IawMatrix& rMx)
{
	IawMatrix mx_temp = *this;

	memset( &mMx, 0, sizeof( float ) * 16 );

	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			for (int k=0; k<4; k++)
				mMx(i,j) += mx_temp.mMx(i,k) * rMx.mMx(k,j);
}

// Assignment operators...
IawMatrix IawMatrix::operator= (IawMatrix mx)
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			mMx(i,j) = mx.mMx(i,j);
		}
	}
	return *this;
}

IawMatrix IawMatrix::operator= (D3DMATRIX& rMx1)
{
	mMx(0,0) = rMx1._11;
	mMx(0,1) = rMx1._12;
	mMx(0,2) = rMx1._13;
	mMx(0,3) = rMx1._14;

	mMx(1,0) = rMx1._21;
	mMx(1,1) = rMx1._22;
	mMx(1,2) = rMx1._23;
	mMx(1,3) = rMx1._24;

	mMx(2,0) = rMx1._31;
	mMx(2,1) = rMx1._32;
	mMx(2,2) = rMx1._33;
	mMx(2,3) = rMx1._34;

	mMx(3,0) = rMx1._41;
	mMx(3,1) = rMx1._42;
	mMx(3,2) = rMx1._43;
	mMx(3,3) = rMx1._44;

	return *this;
}


IawMatrix operator*(const IawMatrix& rMx1, const IawMatrix& rMx2)
{
	int i, j, k;
	IawMatrix mx_temp;

	memset(&mx_temp.mMx, 0, sizeof(float) * 16);

	for (i=0; i<4; i++)
		for (j=0; j<4; j++)
			for (k=0; k<4; k++)
				mx_temp.mMx(i,j) += rMx2.mMx(i,k) * rMx1.mMx(k,j);

	return mx_temp;
}

IawVector operator*(const IawVector& rV, const IawMatrix& rMx)
{
	IawVector temp;

	temp = 0.0f;

	temp.mX = (rV.mX * rMx.mMx(0,0)) + (rV.mY * rMx.mMx(1,0)) + (rV.mZ * rMx.mMx(2,0)) + (/*rV.w * */ rMx.mMx(3,0));
	temp.mY = (rV.mX * rMx.mMx(0,1)) + (rV.mY * rMx.mMx(1,1)) + (rV.mZ * rMx.mMx(2,1)) + (/*rV.w * */ rMx.mMx(3,1));
	temp.mZ = (rV.mX * rMx.mMx(0,2)) + (rV.mY * rMx.mMx(1,2)) + (rV.mZ * rMx.mMx(2,2)) + (/*rV.w * */ rMx.mMx(3,2));

	return temp;
}

// Invert a matrix
void IawMatrix::Invert()
{
	// Note:
	// mMx._14, mMx._24, mMx._34 must be 0.0
	// and mMx._44 must be 1.0 for the inverse
	// operation to succeed

    float fInvDeterm = 1.0f / ( mMx._11 * ( mMx._22 * mMx._33 - mMx._23 * mMx._32 ) -
                                mMx._12 * ( mMx._21 * mMx._33 - mMx._23 * mMx._31 ) +
                                mMx._13 * ( mMx._21 * mMx._32 - mMx._22 * mMx._31 ) );

    float mx11 =  fInvDeterm * ( mMx._22 * mMx._33 - mMx._23 * mMx._32 );
    float mx12 = -fInvDeterm * ( mMx._12 * mMx._33 - mMx._13 * mMx._32 );
    float mx13 =  fInvDeterm * ( mMx._12 * mMx._23 - mMx._13 * mMx._22 );
    float mx21 = -fInvDeterm * ( mMx._21 * mMx._33 - mMx._23 * mMx._31 );
    float mx22 =  fInvDeterm * ( mMx._11 * mMx._33 - mMx._13 * mMx._31 );
    float mx23 = -fInvDeterm * ( mMx._11 * mMx._23 - mMx._13 * mMx._21 );
    float mx31 =  fInvDeterm * ( mMx._21 * mMx._32 - mMx._22 * mMx._31 );
    float mx32 = -fInvDeterm * ( mMx._11 * mMx._32 - mMx._12 * mMx._31 );
    float mx33 =  fInvDeterm * ( mMx._11 * mMx._22 - mMx._12 * mMx._21 );
    float mx41 = -( mMx._41 * mx11 + mMx._42 * mx21 + mMx._43 * mx31 );
    float mx42 = -( mMx._41 * mx12 + mMx._42 * mx22 + mMx._43 * mx32 );
    float mx43 = -( mMx._41 * mx13 + mMx._42 * mx23 + mMx._43 * mx33 );

	mMx._11 = mx11;	mMx._12 = mx12;	mMx._13 = mx13;	mMx._14 = 0.0f;
	mMx._21 = mx21;	mMx._22 = mx22;	mMx._23 = mx23;	mMx._24 = 0.0f;
	mMx._31 = mx31;	mMx._32 = mx32;	mMx._33 = mx33;	mMx._34 = 0.0f;
	mMx._41 = mx41;	mMx._42 = mx42;	mMx._43 = mx43;	mMx._44 = 1.0f;
}

// Rotation funcitons...
void IawMatrix::RotateX(float angle)
{
	IawMatrix mx_rotate(true);
	
	mx_rotate.mMx(1,1) = mx_rotate.mMx(2,2) = (float)cos(angle);
	mx_rotate.mMx(1,2) = (float)sin(angle);
	mx_rotate.mMx(2,1) = -mx_rotate.mMx(1,2);

	PreMultiply(mx_rotate);
}

void IawMatrix::RotateY(float angle)
{
	IawMatrix mx_rotate(true);
	
	mx_rotate.mMx(0,0) = mx_rotate.mMx(2,2) = (float)cos(angle);
	mx_rotate.mMx(0,2) = (float)sin(-angle);
	mx_rotate.mMx(2,0) = -mx_rotate.mMx(0,2);

	PreMultiply(mx_rotate);
}

void IawMatrix::RotateZ(float angle)
{
	IawMatrix mx_rotate(true);
	
	mx_rotate.mMx(0,0) = mx_rotate.mMx(1,1) = (float)cos(angle);
	mx_rotate.mMx(0,1) = (float)sin(angle);
	mx_rotate.mMx(1,0) = - mx_rotate.mMx(0,1);

	PreMultiply(mx_rotate);
}

void IawMatrix::Rotate(float x, float y, float z, float angle)
{
	IawVector axis(x, y, z);
	Rotate(axis, angle);
}

void IawMatrix::Rotate(IawVector axis, float angle)
{
	float angle_cos = (float)cos(angle);
	float angle_sin = (float)sin(angle);
	float x, y, z;
	IawMatrix mx_rotate;

	// Normalize the vector
	axis = Normalize(axis);
	x = axis.mX;
	y = axis.mY;
	z = axis.mZ;

	mx_rotate.mMx(0,0) = (x * x) * (1.0f - angle_cos) + angle_cos;
	mx_rotate.mMx(0,1) = (x * y) * (1.0f - angle_cos) - (z * angle_sin);
	mx_rotate.mMx(0,2) = (x * z) * (1.0f - angle_cos) + (y * angle_sin);

	mx_rotate.mMx(1,0) = (y * x) * (1.0f - angle_cos) + (z * angle_sin);
	mx_rotate.mMx(1,1) = (y * y) * (1.0f - angle_cos) + angle_cos ;
	mx_rotate.mMx(1,2) = (y * z) * (1.0f - angle_cos) - (x * angle_sin);

	mx_rotate.mMx(2,0) = (z * x) * (1.0f - angle_cos) - (y * angle_sin);
	mx_rotate.mMx(2,1) = (z * y) * (1.0f - angle_cos) + (x * angle_sin);
	mx_rotate.mMx(2,2) = (z * z) * (1.0f - angle_cos) + angle_cos;

	mx_rotate.mMx(0,3) = mx_rotate.mMx(1,3) = mx_rotate.mMx(2,3) = 0.0f;
	mx_rotate.mMx(3,0) = mx_rotate.mMx(3,1) = mx_rotate.mMx(3,2) = 0.0f;
	mx_rotate.mMx(3,3) = 1.0f;

	PreMultiply(mx_rotate);
}

// Translation
void IawMatrix::Translate(float x, float y, float z)
{
	IawMatrix mx_translate(true);

	mx_translate.mMx(3,0) = x;
	mx_translate.mMx(3,1) = y;
	mx_translate.mMx(3,2) = z;

	PreMultiply(mx_translate);
}

// Scaling
void IawMatrix::Scale(float x, float y, float z)
{
	IawMatrix mx_scale(true);

	mx_scale.mMx(0,0) = x;
	mx_scale.mMx(1,1) = y;
	mx_scale.mMx(2,2) = z;

	PreMultiply(mx_scale);
}

// Set up a view matrix
void IawMatrix::SetView(IawVector from, IawVector to, IawVector up)
{
	// Create the View vector
	IawVector mx_view = to - from;

	float length = Magnitude(mx_view);
	if(length < 1e-6f)
		return;

	// Normalize the view vector
	mx_view /= length;

	// Get the dot product, and calculate the projection of the view
	// vector onto the up vector. The projection is the y basis vector.
	float dot_product = DotProduct(up, mx_view);

	up = up - dot_product * mx_view;

	// If this vector has near-zero length because the input specified a
	// bogus up vector, let's try a default up vector
	if(1e-6f > (length = Magnitude(up)))
	{
		up = IawVector(0.0f, 1.0f, 0.0f) - mx_view.mY * mx_view;

		// If we still have near-zero length, resort to a different axis.
		if(1e-6f > (length = Magnitude(up)))
		{
			up = IawVector(0.0f, 0.0f, 1.0f) - mx_view.mZ * mx_view;

			if(1e-6f > (length = Magnitude(up)))
				return;
		}
	}

	// Normalize the y basis vector
	up /= length;

	// The x basis vector is found simply with the cross product of the y
	// and z basis vectors
	IawVector right_vector = CrossProduct(up, mx_view);

	SetIdentity();
	mMx(0,0) = right_vector.mX;	mMx(0,1) = up.mX;	mMx(0,2) = mx_view.mX;
	mMx(1,0) = right_vector.mY;	mMx(1,1) = up.mY;	mMx(1,2) = mx_view.mY;
	mMx(2,0) = right_vector.mZ;	mMx(2,1) = up.mZ;	mMx(2,2) = mx_view.mZ;

    // Do the translation values (rotations are still about the eyepoint)
	mMx(3,0) = - DotProduct(from, right_vector);
	mMx(3,1) = - DotProduct(from, up);
	mMx(3,2) = - DotProduct(from, mx_view);

	return;
}

// Set up a projection matrix
void IawMatrix::SetProjection(
							  float FOV, 
							  float aspectRatio, 
							  float nearClip, 
							  float farClip)
{
	if(fabs(farClip-nearClip) < 0.01f)
		return;
	if(fabs(sin(FOV/2)) < 0.01f)
		return;

	float w = aspectRatio * (float)(cos(FOV/2)/sin(FOV/2));
	float h =   1.0f  * (float)(cos(FOV/2)/sin(FOV/2));
	float Q = farClip / (farClip - nearClip);

	SetIdentity();
	mMx(0,0) = w;
	mMx(1,1) = h;
	mMx(2,2) = Q;
	mMx(2,3) = 1.0f;
	mMx(3,2) = -Q * nearClip;
	mMx(3,3) = 0.0f;
}


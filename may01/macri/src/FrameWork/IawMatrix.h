// IawMatrix.h App Wizard Version 2.0 Beta 1
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
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#if !defined(IawMatrix_h)
#define IawMatrix_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * A math vector class.
 *
 * This class greatly simplifies mathematical vector manipulation
 */
class IawVector
{
public:
	/** Components of a math vector. */
	float mX, mY, mZ;

	/** Default Constructor */
	IawVector();

	/** Copy Constructor */
	IawVector(const IawVector& rVector);

	/**
	 * Scalar constructor.
	 * Constructor to assign the same value to all componenets.
	 */
	IawVector(float f);

	/** 
	 * Components constructor.
	 * Constructor to explicitly define each component of a vector.
	 */
	IawVector(float x, float y, float z);

	/** Default Destructor */
	~IawVector();

	//@{
	/** Memberwise assignment operator */
	IawVector operator= (const float f);
	IawVector operator+= (const IawVector v);
	IawVector operator-= (const IawVector v);
	IawVector operator*= (const IawVector v);
	IawVector operator/= (const IawVector v);
	IawVector operator*= (float s);
	IawVector operator/= (float s);
	//@}

	//@{
	/** Unary assignment operator */
	friend IawVector operator+ (const IawVector& rV);
	friend IawVector operator- (const IawVector& rV);
	//@}

	//@{
	/** Binary assignment operator */
	friend IawVector operator+ (const IawVector& rV1, const IawVector& rV2); 
	friend IawVector operator- (const IawVector& rV1, const IawVector& rV2);
	friend IawVector operator* (const IawVector& rV, float s);
	friend IawVector operator* (float s, const IawVector& rV);
	friend IawVector operator/ (const IawVector& rV, float s);
	friend IawVector operator* (const IawVector& rV1, const IawVector& rV2);
	friend IawVector operator/ (const IawVector& rV1, const IawVector& rV2);
	//@}

	/**
	 * Length related function.
	 * Calculate the magnitude of a vector.
	 * @return the square root of the sum of the squared components.
	 */
	friend float Magnitude(const IawVector& rV);

	/**
	 * Normalize the vector to a unit vector.
	 * @return a unitized vector with same direction and unit length
	 */
	friend IawVector Normalize (const IawVector& rV);

	/** Dot product */
	friend float DotProduct (const IawVector& rV1, const IawVector& rV2);

	/** Cross product */
	friend IawVector CrossProduct (const IawVector& rV1, const IawVector& rV2);

}; // IawVector



/**
 * A math matrix class.
 *
 * This class simplifies mathematical matrix manipulation.
 */
class IawMatrix
{
public:
	D3DMATRIX mMx; /**< A DX8 3D matrix */

	/**
	 * Default Constructor.
	 * @param setIdentity Set up as the Identity matrix on construction.
	 */
	IawMatrix(bool setIdentity = false);

	/** Copy Constructor */
	IawMatrix(IawMatrix& rMx);

	/** Default Destructor */
	virtual ~IawMatrix();

	/** Set the matrix to the Identity matrix */
	void SetIdentity();

	/**
	 * Pre-multiply by the referenced matrix.
	 * This is used for applying transformations.
	 */
	void PreMultiply(IawMatrix& rMx);

	/**
	 * Pos-multiply by the referenced matrix.
	 * This is especially useful for doing nonuniform transformations.
	 */
	void PostMultiply(IawMatrix& rMx);

	//@{
	/** Memberwise ssignment operator */
	IawMatrix operator= (IawMatrix mx);
	IawMatrix operator= (D3DMATRIX& rMx);
	//@}

	/** Invert the matrix */
	void Invert();

	/** Rotate about X */
	void RotateX(float angle);

	/** Rotate about Y */
	void RotateY(float angle);

	/** Rotate about Z */
	void RotateZ(float angle);

	/**
	 * Rotate about an arbitrary axis.
	 * @param x x component of vector to rotate about.
	 * @param y y component of vector to rotate about.
	 * @param z z component of vector to rotate about.
	 * @param angle the degree of rotation.
	 */
	void Rotate(float x, float y, float z, float angle);

	/**
	 * Rotate about an arbitrary axis.
	 * @param axis the arbitrary axis to rotate about.
	 * @param angle the degree of rotation.
	 */
	void Rotate(IawVector axis, float angle);

	/** Translation in x, y, and z */
	void Translate(float x, float y, float z);

	/** Scaling in x, y, and z */
	void Scale(float x, float y, float z);

	/**
	 * Set up a view matrix.
	 * @param from the point in space you want to view from
	 * @param to the point in space you want to look at
	 * @param up the "up" vector (usually the Y axis)
	 */
	void SetView(IawVector from, IawVector to, IawVector up);

	/**
	 * Set up a projection matrix.
	 * @param FOV field of view.
	 * @param aspectRatio window aspect ratio.
	 * @param nearClip distance from the eye point to the near clipping plane.
	 * @param farClip distance from the eye point to the far clipping plane.
	 */
	void SetProjection(float FOV, float aspectRaio, float nearClip, float farClip);

	//@{
	/** Binary operators */
	friend IawMatrix operator* (const IawMatrix& rMx1, const IawMatrix &rMx2);
	friend IawVector operator* (const IawVector& rV, const IawMatrix &rMx);
	//@}
}; // IawMatrix


#endif // !defined(IawMATRIX_h)


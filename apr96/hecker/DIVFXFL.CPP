/*

This material is Copyright 1995, 1996 Chris Hecker, All Rights Reserved.
It's for you to read and learn from, not to put in your own articles
or books or on your website, etc.  Thank you.

Chris Hecker
checker@netcom.com

*/

/******** Perspective texture mapper *********/

#include <math.h>
#include <assert.h>
#include "mappers.h"

/******** structures, inlines, and function declarations **********/

struct gradients_fx_fl {
	gradients_fx_fl( POINT3D const *pVertices );
	float aOneOverZ[3];				// 1/z for each vertex
	float aUOverZ[3];				// u/z for each vertex
	float aVOverZ[3];				// v/z for each vertex
	float dOneOverZdX, dOneOverZdY;	// d(1/z)/dX, d(1/z)/dY
	float dUOverZdX, dUOverZdY;		// d(u/z)/dX, d(u/z)/dY
	float dVOverZdX, dVOverZdY;		// d(v/z)/dX, d(v/z)/dY
};

struct edge_fx_fl {
	edge_fx_fl(gradients_fx_fl const &Gradients, POINT3D const *pVertices, int Top,
			int Bottom );
	inline int Step( void );

	long X, XStep, Numerator, Denominator;			// DDA info for x
	long ErrorTerm;
	int Y, Height;					// current y and vertical count
	float OneOverZ, OneOverZStep, OneOverZStepExtra;// 1/z and step
	float UOverZ, UOverZStep, UOverZStepExtra;		// u/z and step
	float VOverZ, VOverZStep, VOverZStepExtra;		// v/z and step
};

inline int edge_fx_fl::Step( void ) {
	X += XStep; Y++; Height--;
	UOverZ += UOverZStep; VOverZ += VOverZStep; OneOverZ += OneOverZStep;

	ErrorTerm += Numerator;
	if(ErrorTerm >= Denominator) {
		X++;
		ErrorTerm -= Denominator;
		OneOverZ += OneOverZStepExtra;
		UOverZ += UOverZStepExtra; VOverZ += VOverZStepExtra;
	}
	return Height;
}	

void DrawScanLine( dib_info const &Dest, gradients_fx_fl const &Gradients,
	edge_fx_fl *pLeft, edge_fx_fl *pRight, dib_info const &Texture );


/******** TextureMapTriangle **********/

void TextureMapTriangle_div_fx_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture )
{
	int Top, Middle, Bottom, MiddleForCompare, BottomForCompare;
	fixed28_4 Y0 = pVertices[0].fxfl.Y, Y1 = pVertices[1].fxfl.Y,
				Y2 = pVertices[2].fxfl.Y;

	// sort vertices in y
	if(Y0 < Y1) {
		if(Y2 < Y0) {
			Top = 2; Middle = 0; Bottom = 1;
			MiddleForCompare = 0; BottomForCompare = 1;
		} else {
			Top = 0;
			if(Y1 < Y2) {
				Middle = 1; Bottom = 2;
				MiddleForCompare = 1; BottomForCompare = 2;
			} else {
				Middle = 2; Bottom = 1;
				MiddleForCompare = 2; BottomForCompare = 1;
			}
		}
	} else {
		if(Y2 < Y1) {
			Top = 2; Middle = 1; Bottom = 0;
			MiddleForCompare = 1; BottomForCompare = 0;
		} else {
			Top = 1;
			if(Y0 < Y2) {
				Middle = 0; Bottom = 2;
				MiddleForCompare = 3; BottomForCompare = 2;
			} else {
				Middle = 2; Bottom = 0;
				MiddleForCompare = 2; BottomForCompare = 3;
			}
		}
	}

	gradients_fx_fl Gradients(pVertices);
	edge_fx_fl TopToBottom(Gradients,pVertices,Top,Bottom);
	edge_fx_fl TopToMiddle(Gradients,pVertices,Top,Middle);
	edge_fx_fl MiddleToBottom(Gradients,pVertices,Middle,Bottom);
	edge_fx_fl *pLeft, *pRight;
	int MiddleIsLeft;

	// the triangle is clockwise, so if bottom > middle then middle is right
	if(BottomForCompare > MiddleForCompare) {
		MiddleIsLeft = 0;
		pLeft = &TopToBottom; pRight = &TopToMiddle;
	} else {
		MiddleIsLeft = 1;
		pLeft = &TopToMiddle; pRight = &TopToBottom;
	}

	int Height = TopToMiddle.Height;

	while(Height--) {
		DrawScanLine(Dest,Gradients,pLeft,pRight,Texture);
		TopToMiddle.Step(); TopToBottom.Step();
	}

	Height = MiddleToBottom.Height;

	if(MiddleIsLeft) {
		pLeft = &MiddleToBottom; pRight = &TopToBottom;
	} else {
		pLeft = &TopToBottom; pRight = &MiddleToBottom;
	}
	
	while(Height--) {
		DrawScanLine(Dest,Gradients,pLeft,pRight,Texture);
		MiddleToBottom.Step(); TopToBottom.Step();
	}
}

/********** gradients_fx_fl constructor **********/

gradients_fx_fl::gradients_fx_fl( POINT3D const *pVertices )
{
	int Counter;

	fixed28_4 X1Y0 = Fixed28_4Mul(pVertices[1].fxfl.X - pVertices[2].fxfl.X,
							pVertices[0].fxfl.Y - pVertices[2].fxfl.Y);
	fixed28_4 X0Y1 = Fixed28_4Mul(pVertices[0].fxfl.X - pVertices[2].fxfl.X,
							pVertices[1].fxfl.Y - pVertices[2].fxfl.Y);
	float OneOverdX = 1.0 / Fixed28_4ToFloat(X1Y0 - X0Y1);

	float OneOverdY = -OneOverdX;

	for(Counter = 0;Counter < 3;Counter++)
	{
		float const OneOverZ = 1/pVertices[Counter].fxfl.Z;
		aOneOverZ[Counter] = OneOverZ;
		aUOverZ[Counter] = pVertices[Counter].fxfl.U * OneOverZ;
		aVOverZ[Counter] = pVertices[Counter].fxfl.V * OneOverZ;
	}

	dOneOverZdX = OneOverdX * (((aOneOverZ[1] - aOneOverZ[2]) *
			Fixed28_4ToFloat(pVertices[0].fxfl.Y - pVertices[2].fxfl.Y)) -
			((aOneOverZ[0] - aOneOverZ[2]) *
			Fixed28_4ToFloat(pVertices[1].fxfl.Y - pVertices[2].fxfl.Y)));
	dOneOverZdY = OneOverdY * (((aOneOverZ[1] - aOneOverZ[2]) *
			Fixed28_4ToFloat(pVertices[0].fxfl.X - pVertices[2].fxfl.X)) -
			((aOneOverZ[0] - aOneOverZ[2]) *
			Fixed28_4ToFloat(pVertices[1].fxfl.X - pVertices[2].fxfl.X)));

	dUOverZdX = OneOverdX * (((aUOverZ[1] - aUOverZ[2]) *
			Fixed28_4ToFloat(pVertices[0].fxfl.Y - pVertices[2].fxfl.Y)) -
			((aUOverZ[0] - aUOverZ[2]) *
			Fixed28_4ToFloat(pVertices[1].fxfl.Y - pVertices[2].fxfl.Y)));
	dUOverZdY = OneOverdY * (((aUOverZ[1] - aUOverZ[2]) *
			Fixed28_4ToFloat(pVertices[0].fxfl.X - pVertices[2].fxfl.X)) -
			((aUOverZ[0] - aUOverZ[2]) *
			Fixed28_4ToFloat(pVertices[1].fxfl.X - pVertices[2].fxfl.X)));

	dVOverZdX = OneOverdX * (((aVOverZ[1] - aVOverZ[2]) *
			Fixed28_4ToFloat(pVertices[0].fxfl.Y - pVertices[2].fxfl.Y)) -
			((aVOverZ[0] - aVOverZ[2]) *
			Fixed28_4ToFloat(pVertices[1].fxfl.Y - pVertices[2].fxfl.Y)));
	dVOverZdY = OneOverdY * (((aVOverZ[1] - aVOverZ[2]) *
			Fixed28_4ToFloat(pVertices[0].fxfl.X - pVertices[2].fxfl.X)) -
			((aVOverZ[0] - aVOverZ[2]) *
			Fixed28_4ToFloat(pVertices[1].fxfl.X - pVertices[2].fxfl.X)));
}

/********** handle floor divides and mods correctly ***********/

inline void FloorDivMod( long Numerator, long Denominator, long &Floor,
				long &Mod )
{
	assert(Denominator > 0);		// we assume it's positive
	if(Numerator >= 0) {
		// positive case, C is okay
		Floor = Numerator / Denominator;
		Mod = Numerator % Denominator;
	} else {
		// Numerator is negative, do the right thing
		Floor = -((-Numerator) / Denominator);
		Mod = (-Numerator) % Denominator;
		if(Mod) {
			// there is a remainder
			Floor--; Mod = Denominator - Mod;
		}
	}
}

/********** edge_fx_fl constructor ***********/

edge_fx_fl::edge_fx_fl( gradients_fx_fl const &Gradients, POINT3D const *pVertices, int Top,
		int Bottom )
{
	Y = Ceil28_4(pVertices[Top].fxfl.Y);
	int YEnd = Ceil28_4(pVertices[Bottom].fxfl.Y);
	Height = YEnd - Y;

	if(Height)
	{
		long dN = pVertices[Bottom].fxfl.Y - pVertices[Top].fxfl.Y;
		long dM = pVertices[Bottom].fxfl.X - pVertices[Top].fxfl.X;
	
		long InitialNumerator = dM*16*Y - dM*pVertices[Top].fxfl.Y +
					dN*pVertices[Top].fxfl.X - 1 + dN*16;
		FloorDivMod(InitialNumerator,dN*16,X,ErrorTerm);
		FloorDivMod(dM*16,dN*16,XStep,Numerator);
		Denominator = dN*16;
	
		float YPrestep = Fixed28_4ToFloat(Y*16 - pVertices[Top].fxfl.Y);
		float XPrestep = Fixed28_4ToFloat(X*16 - pVertices[Top].fxfl.X);
	
		OneOverZ = Gradients.aOneOverZ[Top]
						+ YPrestep * Gradients.dOneOverZdY
						+ XPrestep * Gradients.dOneOverZdX;
		OneOverZStep = XStep * Gradients.dOneOverZdX
						+ Gradients.dOneOverZdY;
		OneOverZStepExtra = Gradients.dOneOverZdX;
	
		UOverZ = Gradients.aUOverZ[Top]
						+ YPrestep * Gradients.dUOverZdY
						+ XPrestep * Gradients.dUOverZdX;
		UOverZStep = XStep * Gradients.dUOverZdX
						+ Gradients.dUOverZdY;
		UOverZStepExtra = Gradients.dUOverZdX;
	
		VOverZ = Gradients.aVOverZ[Top]
						+ YPrestep * Gradients.dVOverZdY
						+ XPrestep * Gradients.dVOverZdX;
		VOverZStep = XStep * Gradients.dVOverZdX
						+ Gradients.dVOverZdY;
		VOverZStepExtra = Gradients.dVOverZdX;
	}
}

/********** DrawScanLine ************/

void DrawScanLine( dib_info const &Dest, gradients_fx_fl const &Gradients,
	edge_fx_fl *pLeft, edge_fx_fl *pRight, dib_info const &Texture )
{
	int XStart = pLeft->X;
	int Width = pRight->X - XStart;

	char unsigned *pDestBits = Dest.pBits;
	char unsigned * const pTextureBits = Texture.pBits;
	pDestBits += pLeft->Y * Dest.DeltaScan + XStart;
	long TextureDeltaScan = Texture.DeltaScan;

	float OneOverZ = pLeft->OneOverZ;
	float UOverZ = pLeft->UOverZ;
	float VOverZ = pLeft->VOverZ;

	while(Width-- > 0)
	{
		float Z = 1/OneOverZ;
		int U = UOverZ * Z;
		int V = VOverZ * Z;

		*(pDestBits++) = *(pTextureBits + U + (V * TextureDeltaScan));

		OneOverZ += Gradients.dOneOverZdX;
		UOverZ += Gradients.dUOverZdX;
		VOverZ += Gradients.dVOverZdX;
	}
}

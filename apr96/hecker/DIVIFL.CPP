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

struct gradients_i_fl {
	gradients_i_fl( POINT3D const *pVertices );
	float aOneOverZ[3];				// 1/z for each vertex
	float aUOverZ[3];				// u/z for each vertex
	float aVOverZ[3];				// v/z for each vertex
	float dOneOverZdX, dOneOverZdY;	// d(1/z)/dX, d(1/z)/dY
	float dUOverZdX, dUOverZdY;		// d(u/z)/dX, d(u/z)/dY
	float dVOverZdX, dVOverZdY;		// d(v/z)/dX, d(v/z)/dY
};

struct edge_i_fl {
	edge_i_fl( gradients_i_fl const &Gradients, POINT3D const *pVertices, int Top,
			int Bottom );
	inline int Step( void );

	long X, XStep, Numerator, Denominator;			// DDA info for x
	long ErrorTerm;
	int Y, Height;					// current y and vertical count
	float OneOverZ, OneOverZStep, OneOverZStepExtra;// 1/z and step
	float UOverZ, UOverZStep, UOverZStepExtra;		// u/z and step
	float VOverZ, VOverZStep, VOverZStepExtra;		// v/z and step
};

inline int edge_i_fl::Step( void ) {
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

void DrawScanLine( dib_info const &Dest, gradients_i_fl const &Gradients,
	edge_i_fl *pLeft, edge_i_fl *pRight, dib_info const &Texture );

/******** TextureMapTriangle **********/

void TextureMapTriangle_div_i_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture )
{
	int Top, Middle, Bottom, MiddleForCompare, BottomForCompare;
	int Y0 = pVertices[0].ifl.Y, Y1 = pVertices[1].ifl.Y,
					Y2 = pVertices[2].ifl.Y;

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

	gradients_i_fl Gradients(pVertices);
	edge_i_fl TopToBottom(Gradients,pVertices,Top,Bottom);
	edge_i_fl TopToMiddle(Gradients,pVertices,Top,Middle);
	edge_i_fl MiddleToBottom(Gradients,pVertices,Middle,Bottom);
	edge_i_fl *pLeft, *pRight;
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

/********** gradients_i_fl constructor **********/

gradients_i_fl::gradients_i_fl( POINT3D const *pVertices )
{
	int Counter;
	
	float OneOverdX = 1.0 / (((pVertices[1].ifl.X - pVertices[2].ifl.X) *
			(pVertices[0].ifl.Y - pVertices[2].ifl.Y)) -
			((pVertices[0].ifl.X - pVertices[2].ifl.X) *
			(pVertices[1].ifl.Y - pVertices[2].ifl.Y)));

	float OneOverdY = -OneOverdX;

	for(Counter = 0;Counter < 3;Counter++)
	{
		float const OneOverZ = 1.0/pVertices[Counter].ifl.Z;
		aOneOverZ[Counter] = OneOverZ;
		aUOverZ[Counter] = pVertices[Counter].ifl.U * OneOverZ;
		aVOverZ[Counter] = pVertices[Counter].ifl.V * OneOverZ;
	}

	dOneOverZdX = OneOverdX * (((aOneOverZ[1] - aOneOverZ[2]) *
			(pVertices[0].ifl.Y - pVertices[2].ifl.Y)) -
			((aOneOverZ[0] - aOneOverZ[2]) *
			(pVertices[1].ifl.Y - pVertices[2].ifl.Y)));
	dOneOverZdY = OneOverdY * (((aOneOverZ[1] - aOneOverZ[2]) *
			(pVertices[0].ifl.X - pVertices[2].ifl.X)) -
			((aOneOverZ[0] - aOneOverZ[2]) *
			(pVertices[1].ifl.X - pVertices[2].ifl.X)));

	dUOverZdX = OneOverdX * (((aUOverZ[1] - aUOverZ[2]) *
			(pVertices[0].ifl.Y - pVertices[2].ifl.Y)) -
			((aUOverZ[0] - aUOverZ[2]) *
			(pVertices[1].ifl.Y - pVertices[2].ifl.Y)));
	dUOverZdY = OneOverdY * (((aUOverZ[1] - aUOverZ[2]) *
			(pVertices[0].ifl.X - pVertices[2].ifl.X)) -
			((aUOverZ[0] - aUOverZ[2]) *
			(pVertices[1].ifl.X - pVertices[2].ifl.X)));

	dVOverZdX = OneOverdX * (((aVOverZ[1] - aVOverZ[2]) *
			(pVertices[0].ifl.Y - pVertices[2].ifl.Y)) -
			((aVOverZ[0] - aVOverZ[2]) *
			(pVertices[1].ifl.Y - pVertices[2].ifl.Y)));
	dVOverZdY = OneOverdY * (((aVOverZ[1] - aVOverZ[2]) *
			(pVertices[0].ifl.X - pVertices[2].ifl.X)) -
			((aVOverZ[0] - aVOverZ[2]) *
			(pVertices[1].ifl.X - pVertices[2].ifl.X)));
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

/********** edge_i_fl constructor ***********/

edge_i_fl::edge_i_fl( gradients_i_fl const &Gradients, POINT3D const *pVertices, int Top,
		int Bottom )
{
	Y = pVertices[Top].ifl.Y;
	Height = pVertices[Bottom].ifl.Y - Y;
	int Width = pVertices[Bottom].ifl.X - pVertices[Top].ifl.X;

	if(Height) {
		// this isn't necessary because we always start at TopY, but if you
		// want to start somewhere else you'd make Y your start
		FloorDivMod(Width * (Y - pVertices[Top].ifl.Y) - 1,Height,X,ErrorTerm);
		X += pVertices[Top].ifl.X + 1;
	
		FloorDivMod(Width,Height,XStep,Numerator);
		Denominator = Height;
	
		OneOverZ = Gradients.aOneOverZ[Top];
		OneOverZStep = XStep * Gradients.dOneOverZdX + Gradients.dOneOverZdY;
		OneOverZStepExtra = Gradients.dOneOverZdX;
	
		UOverZ = Gradients.aUOverZ[Top];
		UOverZStep = XStep * Gradients.dUOverZdX + Gradients.dUOverZdY;
		UOverZStepExtra = Gradients.dUOverZdX;
	
		VOverZ = Gradients.aVOverZ[Top];
		VOverZStep = XStep * Gradients.dVOverZdX + Gradients.dVOverZdY;
		VOverZStepExtra = Gradients.dVOverZdX;
	}
}

/********** DrawScanLine ************/

void DrawScanLine( dib_info const &Dest, gradients_i_fl const &Gradients,
	edge_i_fl *pLeft, edge_i_fl *pRight, dib_info const &Texture )
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

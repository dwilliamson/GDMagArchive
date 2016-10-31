/*

This material is Copyright 1995, 1996 Chris Hecker, All Rights Reserved.
It's for you to read and learn from, not to put in your own articles
or books or on your website, etc.  Thank you.

Chris Hecker
checker@netcom.com

*/

/******** Perspective texture mapper *********/

#include <math.h>
#include "mappers.h"

/******** structures, inlines, and function declarations **********/

struct gradients_fl_fl {
	gradients_fl_fl( POINT3D const *pVertices );
	float aOneOverZ[3];				// 1/z for each vertex
	float aUOverZ[3];				// u/z for each vertex
	float aVOverZ[3];				// v/z for each vertex
	float dOneOverZdX, dOneOverZdY;	// d(1/z)/dX, d(1/z)/dY
	float dUOverZdX, dUOverZdY;		// d(u/z)/dX, d(u/z)/dY
	float dVOverZdX, dVOverZdY;		// d(v/z)/dX, d(v/z)/dY
};

struct edge_fl_fl {
	edge_fl_fl(gradients_fl_fl const &Gradients, POINT3D const *pVertices, int Top,
			int Bottom );
	inline int Step( void );

	float X, XStep;					// fractional x and dX/dY
	int Y, Height;					// current y and vertical count
	float OneOverZ, OneOverZStep;	// 1/z and step
	float UOverZ, UOverZStep;		// u/z and step
	float VOverZ, VOverZStep;		// v/z and step
};

inline int edge_fl_fl::Step( void ) {
	X += XStep; Y++; Height--;
	UOverZ += UOverZStep; VOverZ += VOverZStep; OneOverZ += OneOverZStep;
	return Height;
}

void DrawScanLine( dib_info const &Dest, gradients_fl_fl const &Gradients,
	edge_fl_fl *pLeft, edge_fl_fl *pRight, dib_info const &Texture );

/******** TextureMapTriangle **********/

void TextureMapTriangle_div_fl_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture )
{
	int Top, Middle, Bottom, MiddleForCompare, BottomForCompare;
	float Y0 = pVertices[0].flfl.Y, Y1 = pVertices[1].flfl.Y,
					Y2 = pVertices[2].flfl.Y;

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

	gradients_fl_fl Gradients(pVertices);
	edge_fl_fl TopToBottom(Gradients,pVertices,Top,Bottom);
	edge_fl_fl TopToMiddle(Gradients,pVertices,Top,Middle);
	edge_fl_fl MiddleToBottom(Gradients,pVertices,Middle,Bottom);
	edge_fl_fl *pLeft, *pRight;
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

/********** gradients_fl_fl constructor **********/

gradients_fl_fl::gradients_fl_fl( POINT3D const *pVertices )
{
	int Counter;
	
	float OneOverdX = 1 / (((pVertices[1].flfl.X - pVertices[2].flfl.X) *
			(pVertices[0].flfl.Y - pVertices[2].flfl.Y)) -
			((pVertices[0].flfl.X - pVertices[2].flfl.X) *
			(pVertices[1].flfl.Y - pVertices[2].flfl.Y)));

	float OneOverdY = -OneOverdX;

	for(Counter = 0;Counter < 3;Counter++)
	{
		float const OneOverZ = 1/pVertices[Counter].flfl.Z;
		aOneOverZ[Counter] = OneOverZ;
		aUOverZ[Counter] = pVertices[Counter].flfl.U * OneOverZ;
		aVOverZ[Counter] = pVertices[Counter].flfl.V * OneOverZ;
	}

	dOneOverZdX = OneOverdX * (((aOneOverZ[1] - aOneOverZ[2]) *
			(pVertices[0].flfl.Y - pVertices[2].flfl.Y)) -
			((aOneOverZ[0] - aOneOverZ[2]) *
			(pVertices[1].flfl.Y - pVertices[2].flfl.Y)));
	dOneOverZdY = OneOverdY * (((aOneOverZ[1] - aOneOverZ[2]) *
			(pVertices[0].flfl.X - pVertices[2].flfl.X)) -
			((aOneOverZ[0] - aOneOverZ[2]) *
			(pVertices[1].flfl.X - pVertices[2].flfl.X)));

	dUOverZdX = OneOverdX * (((aUOverZ[1] - aUOverZ[2]) *
			(pVertices[0].flfl.Y - pVertices[2].flfl.Y)) -
			((aUOverZ[0] - aUOverZ[2]) *
			(pVertices[1].flfl.Y - pVertices[2].flfl.Y)));
	dUOverZdY = OneOverdY * (((aUOverZ[1] - aUOverZ[2]) *
			(pVertices[0].flfl.X - pVertices[2].flfl.X)) -
			((aUOverZ[0] - aUOverZ[2]) *
			(pVertices[1].flfl.X - pVertices[2].flfl.X)));

	dVOverZdX = OneOverdX * (((aVOverZ[1] - aVOverZ[2]) *
			(pVertices[0].flfl.Y - pVertices[2].flfl.Y)) -
			((aVOverZ[0] - aVOverZ[2]) *
			(pVertices[1].flfl.Y - pVertices[2].flfl.Y)));
	dVOverZdY = OneOverdY * (((aVOverZ[1] - aVOverZ[2]) *
			(pVertices[0].flfl.X - pVertices[2].flfl.X)) -
			((aVOverZ[0] - aVOverZ[2]) *
			(pVertices[1].flfl.X - pVertices[2].flfl.X)));
}

/********** edge_fl_fl constructor ***********/

edge_fl_fl::edge_fl_fl( gradients_fl_fl const &Gradients, POINT3D const *pVertices, int Top,
		int Bottom )
{
	Y = ceil(pVertices[Top].flfl.Y);
	int YEnd = ceil(pVertices[Bottom].flfl.Y);
	Height = YEnd - Y;

	float YPrestep = Y - pVertices[Top].flfl.Y;
	
	float RealHeight = pVertices[Bottom].flfl.Y - pVertices[Top].flfl.Y;
	float RealWidth = pVertices[Bottom].flfl.X - pVertices[Top].flfl.X;

	X = ((RealWidth * YPrestep)/RealHeight) + pVertices[Top].flfl.X;
	XStep = RealWidth/RealHeight;
	float XPrestep = X - pVertices[Top].flfl.X;

	OneOverZ = Gradients.aOneOverZ[Top] + YPrestep * Gradients.dOneOverZdY
					+ XPrestep * Gradients.dOneOverZdX;
	OneOverZStep = XStep * Gradients.dOneOverZdX + Gradients.dOneOverZdY;

	UOverZ = Gradients.aUOverZ[Top] + YPrestep * Gradients.dUOverZdY
					+ XPrestep * Gradients.dUOverZdX;
	UOverZStep = XStep * Gradients.dUOverZdX + Gradients.dUOverZdY;

	VOverZ = Gradients.aVOverZ[Top] + YPrestep * Gradients.dVOverZdY
					+ XPrestep * Gradients.dVOverZdX;
	VOverZStep = XStep * Gradients.dVOverZdX + Gradients.dVOverZdY;
}

/********** DrawScanLine ************/

void DrawScanLine( dib_info const &Dest, gradients_fl_fl const &Gradients,
	edge_fl_fl *pLeft, edge_fl_fl *pRight, dib_info const &Texture )
{
	int XStart = ceil(pLeft->X);
	float XPrestep = XStart - pLeft->X;

	char unsigned *pDestBits = Dest.pBits;
	char unsigned * const pTextureBits = Texture.pBits;
	pDestBits += pLeft->Y * Dest.DeltaScan + XStart;
	long TextureDeltaScan = Texture.DeltaScan;

	int Width = ceil(pRight->X) - XStart;

	float OneOverZ = pLeft->OneOverZ + XPrestep * Gradients.dOneOverZdX;
	float UOverZ = pLeft->UOverZ + XPrestep * Gradients.dUOverZdX;
	float VOverZ = pLeft->VOverZ + XPrestep * Gradients.dVOverZdX;

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

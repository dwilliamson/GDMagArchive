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

struct gradients_fx_fl_a {
	gradients_fx_fl_a( POINT3D const *pVertices );
	float aOneOverZ[3];				// 1/z for each vertex
	float aUOverZ[3];				// u/z for each vertex
	float aVOverZ[3];				// v/z for each vertex
	float dOneOverZdX, dOneOverZdY;	// d(1/z)/dX, d(1/z)/dY
	float dUOverZdX, dUOverZdY;		// d(u/z)/dX, d(u/z)/dY
	float dVOverZdX, dVOverZdY;		// d(v/z)/dX, d(v/z)/dY
	fixed16_16 dUdXModifier;
	fixed16_16 dVdXModifier;
};

struct edge_fx_fl_a {
	edge_fx_fl_a(gradients_fx_fl_a const &Gradients, POINT3D const *pVertices, int Top,
			int Bottom );
	inline int Step( void );

	long X, XStep, Numerator, Denominator;			// DDA info for x
	long ErrorTerm;
	int Y, Height;					// current y and vertical count
	float OneOverZ, OneOverZStep, OneOverZStepExtra;// 1/z and step
	float UOverZ, UOverZStep, UOverZStepExtra;		// u/z and step
	float VOverZ, VOverZStep, VOverZStepExtra;		// v/z and step
};

inline int edge_fx_fl_a::Step( void ) {
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

void DrawScanLine_suba( dib_info const &Dest, gradients_fx_fl_a const &Gradients,
	edge_fx_fl_a *pLeft, edge_fx_fl_a *pRight, dib_info const &Texture );


/******** TextureMapTriangle **********/

void TextureMapTriangle_suba_fx_fl( dib_info const &Dest,
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

	gradients_fx_fl_a Gradients(pVertices);
	edge_fx_fl_a TopToBottom(Gradients,pVertices,Top,Bottom);
	edge_fx_fl_a TopToMiddle(Gradients,pVertices,Top,Middle);
	edge_fx_fl_a MiddleToBottom(Gradients,pVertices,Middle,Bottom);
	edge_fx_fl_a *pLeft, *pRight;
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
		DrawScanLine_suba(Dest,Gradients,pLeft,pRight,Texture);
		TopToMiddle.Step(); TopToBottom.Step();
	}

	Height = MiddleToBottom.Height;

	if(MiddleIsLeft) {
		pLeft = &MiddleToBottom; pRight = &TopToBottom;
	} else {
		pLeft = &TopToBottom; pRight = &MiddleToBottom;
	}
	
	while(Height--) {
		DrawScanLine_suba(Dest,Gradients,pLeft,pRight,Texture);
		MiddleToBottom.Step(); TopToBottom.Step();
	}
}

/********** gradients_fx_fl_a constructor **********/

gradients_fx_fl_a::gradients_fx_fl_a( POINT3D const *pVertices )
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

	//
	// set up rounding modifiers
	//
	// see gradient.txt for the detailed description
	//

	fixed16_16 const Half = 0x8000;
	fixed16_16 const PosModifier = Half;
	fixed16_16 const NegModifier = Half - 1;

	float dUdXIndicator = dUOverZdX * aOneOverZ[0] - aUOverZ[0] * dOneOverZdX;

	if(dUdXIndicator > 0)
	{
		dUdXModifier = PosModifier;
	}
	else
	if(dUdXIndicator < 0)
	{
		dUdXModifier = NegModifier;
	}
	else
	{
		// dUdX == 0

		float dUdYIndicator = dUOverZdY * aOneOverZ[0] -
								aUOverZ[0] * dOneOverZdY;

		if(dUdYIndicator >= 0)
		{
			dUdXModifier = PosModifier;
		}
		else
		{
			dUdXModifier = NegModifier;
		}
	}

	float dVdXIndicator = dVOverZdX * aOneOverZ[0] - aVOverZ[0] * dOneOverZdX;

	if(dVdXIndicator > 0)
	{
		dVdXModifier = PosModifier;
	}
	else
	if(dVdXIndicator < 0)
	{
		dVdXModifier = NegModifier;
	}
	else
	{
		// dVdX == 0

		float dVdYIndicator = dVOverZdY * aOneOverZ[0] -
								aVOverZ[0] * dOneOverZdY;

		if(dVdYIndicator >= 0)
		{
			dVdXModifier = PosModifier;
		}
		else
		{
			dVdXModifier = NegModifier;
		}
	}
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

/********** edge_fx_fl_a constructor ***********/

edge_fx_fl_a::edge_fx_fl_a( gradients_fx_fl_a const &Gradients, POINT3D const *pVertices, int Top,
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

void DrawScanLine_suba( dib_info const &Dest,
	gradients_fx_fl_a const &Gradients,
	edge_fx_fl_a *pLeft, edge_fx_fl_a *pRight, dib_info const &Texture )
{
	int XStart = pLeft->X;
	int Width = pRight->X - XStart;

	char unsigned *pDestBits = Dest.pBits;
	char unsigned * const pTextureBits = Texture.pBits;
	pDestBits += pLeft->Y * Dest.DeltaScan + XStart;
	long TextureDeltaScan = Texture.DeltaScan;

	int const AffineLength = 8;

	float OneOverZLeft = pLeft->OneOverZ;
	float UOverZLeft = pLeft->UOverZ;
	float VOverZLeft = pLeft->VOverZ;

	float dOneOverZdXAff = Gradients.dOneOverZdX * AffineLength;
	float dUOverZdXAff = Gradients.dUOverZdX * AffineLength;
	float dVOverZdXAff = Gradients.dVOverZdX * AffineLength;

	float OneOverZRight = OneOverZLeft + dOneOverZdXAff;
	float UOverZRight = UOverZLeft + dUOverZdXAff;
	float VOverZRight = VOverZLeft + dVOverZdXAff;

	float ZLeft = 1/OneOverZLeft;
	float ULeft = ZLeft * UOverZLeft;
	float VLeft = ZLeft * VOverZLeft;

	float ZRight, URight, VRight;
	fixed16_16 U, V, DeltaU, DeltaV;

	if(Width > 0)
	{
		int Subdivisions = Width / AffineLength;
		int WidthModLength = Width % AffineLength;

		if(!WidthModLength)
		{
			Subdivisions--;
			WidthModLength = AffineLength;
		}

		while(Subdivisions-- > 0)
		{
			ZRight = 1/OneOverZRight;
			URight = ZRight * UOverZRight;
			VRight = ZRight * VOverZRight;
	
			U = FloatToFixed16_16(ULeft) + Gradients.dUdXModifier;
			V = FloatToFixed16_16(VLeft) + Gradients.dVdXModifier;
			DeltaU = FloatToFixed16_16(URight - ULeft) / AffineLength;
			DeltaV = FloatToFixed16_16(VRight - VLeft) / AffineLength;
	
			for(int Counter = 0;Counter < AffineLength;Counter++)
			{
				int UInt = U>>16;
				int VInt = V>>16;

				*(pDestBits++) = *(pTextureBits + UInt +
						(VInt * TextureDeltaScan));

				U += DeltaU;
				V += DeltaV;
			}
	
			ZLeft = ZRight;
			ULeft = URight;
			VLeft = VRight;
	
			OneOverZRight += dOneOverZdXAff;
			UOverZRight += dUOverZdXAff;
			VOverZRight += dVOverZdXAff;
		}
	
		if(WidthModLength)
		{
			ZRight = 1/(pRight->OneOverZ - Gradients.dOneOverZdX);
			URight = ZRight * (pRight->UOverZ - Gradients.dUOverZdX);
			VRight = ZRight * (pRight->VOverZ - Gradients.dVOverZdX);

			U = FloatToFixed16_16(ULeft) + Gradients.dUdXModifier;
			V = FloatToFixed16_16(VLeft) + Gradients.dVdXModifier;

			if(--WidthModLength)
			{
				// guard against div-by-0 for 1 pixel lines
				DeltaU = FloatToFixed16_16(URight - ULeft) / WidthModLength;
				DeltaV = FloatToFixed16_16(VRight - VLeft) / WidthModLength;
			}
	
			for(int Counter = 0;Counter <= WidthModLength;Counter++)
			{
				int UInt = U>>16;
				int VInt = V>>16;
	
				*(pDestBits++) = *(pTextureBits + UInt +
						(VInt * TextureDeltaScan));
		
				U += DeltaU;
				V += DeltaV;
			}
		}
	}
}


/*----------------------------------------------------------------------------

mappers header - this file contains the various platform-independent
	structures for the texture mappers

Chris Hecker (checker@netcom.com)

*/

/*

This material is Copyright 1995, 1996 Chris Hecker, All Rights Reserved.
It's for you to read and learn from, not to put in your own articles
or books or on your website, etc.  Thank you.

Chris Hecker
checker@netcom.com

*/

/*----------------------------------------------------------------------------

types

*/

typedef long fixed28_4;
typedef long fixed16_16;

struct P3D_fl_fl
{
	float X, Y;
	float Z, U, V;
};

struct P3D_i_fl
{
	long X, Y;
	float Z, U, V;
};

struct P3D_fx_fl
{
	fixed28_4 X, Y;
	float Z, U, V;
};

struct P3D_fx_fx
{
	fixed28_4 X, Y;
	fixed16_16 Z, U, V;
};

struct POINT3D
{
	union
	{
		P3D_fl_fl flfl;
		P3D_i_fl ifl;
		P3D_fx_fl fxfl;
		P3D_fx_fx fxfx;
	};
};

struct dib_info
{
	char unsigned *pBits;
	long Width;
	long Height;
	long DeltaScan;
};

/*----------------------------------------------------------------------------

declarations

*/

void TextureMapTriangle_div_fl_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture );

void TextureMapTriangle_div_i_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture );

void TextureMapTriangle_div_fx_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture );

void TextureMapTriangle_suba_fx_fl( dib_info const &Dest,
		POINT3D const *pVertices, dib_info const &Texture );

/*----------------------------------------------------------------------------

functions

*/

inline fixed28_4 FloatToFixed28_4( float Value ) {
	return Value * 16;
}
inline float Fixed28_4ToFloat( fixed28_4 Value ) {
	return Value / 16.0;
}
inline fixed16_16 FloatToFixed16_16( float Value ) {
	return Value * 65536;
}
inline float Fixed16_16ToFloat( fixed16_16 Value ) {
	return Value / 65536.0;
}
inline fixed28_4 Fixed28_4Mul( fixed28_4 A, fixed28_4 B ) {
	// could make this asm to prevent overflow
	return (A * B) / 16;	// 28.4 * 28.4 = 24.8 / 16 = 28.4
}
inline long Ceil28_4( fixed28_4 Value ) {
	long ReturnValue;
	long Numerator = Value - 1 + 16;
	if(Numerator >= 0) {
		ReturnValue = Numerator/16;
	} else {
		// deal with negative numerators correctly
		ReturnValue = -((-Numerator)/16);
		ReturnValue -= ((-Numerator) % 16) ? 1 : 0;
	}
	return ReturnValue;
}


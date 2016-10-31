/**************************************************************************
	dumb3d.hpp - A simple linear algebra library for 3D.

 **************************************************************************/
/**************************************************************************

		(C) Copyright 1994 Microsoft Corp.	All rights reserved.

		You have a royalty-free right to use, modify, reproduce and 
		distribute the Sample Files (and/or any modified version) in 
		any way you find useful, provided that you agree that 
		Microsoft has no warranty obligations or liability for any 
		Sample Application Files which are modified. 

 **************************************************************************/

#if !defined(DUMB3D_HPP)
#define DUMB3D_HPP

#pragma warning (disable:4244)		// int to float conversion

/*----------------------------------------------------------------------------

This header contains the declarations for the dumb3d functions.

*/


// real type

typedef float real;


#define M_PI						3.14159265358979323846


// forward declarations

class point_4;
class vector_4;
class matrix_4x4;

/*----------------------------------------------------------------------------

globally useful functions.

*/

inline vector_4 operator-( point_4 const &Operand1, point_4 const &Operand2 );

inline vector_4 operator+( vector_4 const &Operand1,
		vector_4 const &Operand2 );

inline vector_4 operator-( vector_4 const &Operand1,
		vector_4 const &Operand2 );

inline vector_4 operator*( vector_4 const &Multiplicand,
		real const &Multiplier );

inline vector_4 operator*( real const &Multiplier,
		vector_4 const &Multiplicand );
	  
inline vector_4 operator/( vector_4 const &Quotient,
		real const &Divisor );

inline point_4 operator+( point_4 const &Operand1, vector_4 const &Operand2 );
inline point_4 operator-( point_4 const &Operand1, vector_4 const &Operand2 );
inline point_4 operator+( vector_4 const &Operand2, point_4 const &Operand1 );

inline vector_4 operator-( vector_4 const &Operand1 );

inline vector_4 CrossProduct( vector_4 const &Operand1,
		vector_4 const &Operand2 );

inline vector_4 VectorFrom( point_4 const &Point );
inline point_4 PointFrom( vector_4 const &Vector );

inline real DotProduct( vector_4 const &Operand1, vector_4 const &Operand2 );

real AngleBetween( vector_4 const &Operand1, vector_4 const &Operand2 );
real AngleBetweenUnit( vector_4 const &Unit1, vector_4 const &Unit2 );

inline real RadiansFrom( real Degrees );
inline real DegreesFrom( real Radians );

matrix_4x4 operator*( matrix_4x4 const &Multiplicand,
		matrix_4x4 const &Multiplier );

vector_4 operator*( matrix_4x4 const &Multiplicand,
		vector_4 const &Multiplier );

point_4 operator*( matrix_4x4 const &Multiplicand,
		point_4 const &Multiplier );

void SetViewDistance( matrix_4x4 &Transform, real Distance );


/*----------------------------------------------------------------------------

quadruple.	Base class for homogeneous vectors and points.

*/

class quadruple
{
public:

	inline real GetElement( int Row ) const;

	inline real GetX( void ) const;
	inline real GetY( void ) const;
	inline real GetZ( void ) const;
	inline real GetW( void ) const;

	inline void SetElement( int Row, real Value );

	inline void SetX( real Value );
	inline void SetY( real Value );
	inline void SetZ( real Value );
	inline void SetW( real Value );


protected:

	inline quadruple( void );
	inline quadruple( real X, real Y, real Z, real W );
	inline quadruple( quadruple const & );
	inline quadruple &operator=( quadruple const & );
  
	real aElements[4];
};


/*----------------------------------------------------------------------------

point_4.	This class represents a homogeneous 3D point.

*/

class point_4 :
	public quadruple
{
public:

	inline point_4( void );
	inline point_4( real X, real Y, real Z );

	inline void Homogenize( void );
};

/*----------------------------------------------------------------------------

vector_4.  This class represents a homogeneous 3D vector.

*/

class vector_4 :
	public quadruple
{
public:

	inline vector_4( void );
	inline vector_4( real X, real Y, real Z );

	vector_4 &Normalize( void );
	real Length( void );
};



/*----------------------------------------------------------------------------

matrix_4x4.  This class represents row major 4x4 homogeneous matrices.

*/

class matrix_4x4
{
public:

	matrix_4x4( void );
	inline matrix_4x4( vector_4 const &Axis, real AngleInRadians );
	inline matrix_4x4( point_4 const &Origin, vector_4 const &XAxis,
				vector_4 const &YAxis, vector_4 const &ZAxis );

	matrix_4x4 &ConcatenateXRotation( real Radians );
	matrix_4x4 &ConcatenateYRotation( real Radians );
	matrix_4x4 &ConcatenateZRotation( real Radians );

	matrix_4x4 &ConcatenateXTranslation( real Distance );
	matrix_4x4 &ConcatenateYTranslation( real Distance );
	matrix_4x4 &ConcatenateZTranslation( real Distance );

	matrix_4x4 &ConcatenateXScale( real Distance );
	matrix_4x4 &ConcatenateYScale( real Distance );
	matrix_4x4 &ConcatenateZScale( real Distance );

	inline real GetElement( int Row, int Column ) const;
	inline matrix_4x4 &SetElement( int Row, int Column, real Value );

  
protected:

	enum do_not_initialize { DoNotInitialize };

	inline matrix_4x4( do_not_initialize );

	void InitWith( vector_4 const &Axis, real AngleInRadians );
	void InitWith( point_4 const &Origin, vector_4 const &XAxis,
				vector_4 const &YAxis, vector_4 const &ZAxis );

	real aElements[4][4];
};

/*----------------------------------------------------------------------------

view transform.

*/

class view_transform :
	public matrix_4x4
{
public:

	view_transform( point_4 const &Viewpoint, vector_4 const &ViewDirection,
		vector_4 const &Up );
};


/*----------------------------------------------------------------------------

inline function definitions.

*/


inline real RadiansFrom( real Degrees )
{
	return (Degrees / real(360)) * real(M_PI) * real(2);
}

inline real DegreesFrom( real Radians )
{
	return real(360) * (Radians / (real(M_PI) * real(2)));
}

inline vector_4 operator-( point_4 const &Operand1, point_4 const &Operand2 )
{
	return vector_4(Operand1.GetX() - Operand2.GetX(),
					Operand1.GetY() - Operand2.GetY(),
					Operand1.GetZ() - Operand2.GetZ());
}

inline vector_4 operator+( vector_4 const &Operand1,
		vector_4 const &Operand2 )
{
	return vector_4(Operand1.GetX() + Operand2.GetX(),
					Operand1.GetY() + Operand2.GetY(),
					Operand1.GetZ() + Operand2.GetZ());
}

inline vector_4 operator-( vector_4 const &Operand1,
		vector_4 const &Operand2 )
{
	return vector_4(Operand1.GetX() - Operand2.GetX(),
					Operand1.GetY() - Operand2.GetY(),
					Operand1.GetZ() - Operand2.GetZ());
}

inline vector_4 operator-( vector_4 const &Operand1 )
{
	return vector_4(-Operand1.GetX(),-Operand1.GetY(),-Operand1.GetZ());
}

inline vector_4 operator*( vector_4 const &Multiplicand,
		real const &Multiplier )
{
	return vector_4(Multiplicand.GetX() * Multiplier,
					Multiplicand.GetY() * Multiplier,
					Multiplicand.GetZ() * Multiplier);
}

inline vector_4 operator*( real const &Multiplier,
		vector_4 const &Multiplicand ) 
{
	return Multiplicand * Multiplier;
}
	  
inline vector_4 operator/( vector_4 const &Quotient,
		real const &Divisor )
{
	return Quotient * (real(1.0)/Divisor);
}
	  

inline point_4 operator+( point_4 const &Operand1, vector_4 const &Operand2 )
{
	return point_4(Operand1.GetX() + Operand2.GetX(),
					Operand1.GetY() + Operand2.GetY(),
					Operand1.GetZ() + Operand2.GetZ());
}

inline point_4 operator-( point_4 const &Operand1, vector_4 const &Operand2 )
{
	return point_4(Operand1.GetX() - Operand2.GetX(),
					Operand1.GetY() - Operand2.GetY(),
					Operand1.GetZ() - Operand2.GetZ());
}

inline point_4 operator+( vector_4 const &Operand1, point_4 const &Operand2 )
{
	return Operand2 + Operand1;
}

inline vector_4 CrossProduct( vector_4 const &Operand1,
		vector_4 const &Operand2 )
{
	real X = Operand1.GetY() * Operand2.GetZ() -
				Operand1.GetZ() * Operand2.GetY();
	real Y = Operand1.GetZ() * Operand2.GetX() -
				Operand1.GetX() * Operand2.GetZ();
	real Z = Operand1.GetX() * Operand2.GetY() -
				Operand1.GetY() * Operand2.GetX();

	return vector_4(X,Y,Z);
}

inline vector_4 VectorFrom( point_4 const &Point )
{
	return vector_4(Point.GetX(),Point.GetY(),Point.GetZ());
}

inline point_4 PointFrom( vector_4 const &Vector )
{
	return point_4(Vector.GetX(),Vector.GetY(),Vector.GetZ());
}

inline real DotProduct( vector_4 const &Operand1, vector_4 const &Operand2 )
{
	return Operand1.GetX() * Operand2.GetX() +
			Operand1.GetY() * Operand2.GetY() +
			Operand1.GetZ() * Operand2.GetZ();
}


inline real quadruple::GetElement( int Row ) const
{
	return aElements[Row];
}

inline real quadruple::GetX( void ) const
{
	return aElements[0];
}

inline real quadruple::GetY( void ) const
{
	return aElements[1];
}

inline real quadruple::GetZ( void ) const
{
	return aElements[2];
}

inline real quadruple::GetW( void ) const
{
	return aElements[3];
}

inline void quadruple::SetElement( int Row, real Value )
{
	aElements[Row] = Value;
}

inline void quadruple::SetX( real Value )
{
	aElements[0] = Value;
}

inline void quadruple::SetY( real Value )
{
	aElements[1] = Value;
}

inline void quadruple::SetZ( real Value )
{
	aElements[2] = Value;
}

inline void quadruple::SetW( real Value )
{
	aElements[3] = Value;
}

inline void point_4::Homogenize( void )
{
	aElements[0] = aElements[0] / aElements[3];
	aElements[1] = aElements[1] / aElements[3];
	aElements[2] = aElements[2] / aElements[3];
}

inline quadruple::quadruple( void )
{
	aElements[0] = aElements[1] = aElements[2] = aElements[3] = 0;
}

inline quadruple::quadruple( real X, real Y, real Z, real W )
{
	aElements[0] = X;
	aElements[1] = Y;
	aElements[2] = Z;
	aElements[3] = W;
}

inline quadruple::quadruple( quadruple const &Source )
{
	aElements[0] = Source.aElements[0];
	aElements[1] = Source.aElements[1];
	aElements[2] = Source.aElements[2];
	aElements[3] = Source.aElements[3];
}

inline quadruple &quadruple::operator=( quadruple const &Source )
{
	aElements[0] = Source.aElements[0];
	aElements[1] = Source.aElements[1];
	aElements[2] = Source.aElements[2];
	aElements[3] = Source.aElements[3];

	return *this;
}

inline point_4::point_4( void ) :
	quadruple(0,0,0,1)
{

}

inline point_4::point_4( real X, real Y, real Z ) :
	quadruple(X,Y,Z,1)
{
#if 0
	char aBuffer[100];
	sprintf(aBuffer,"X: %f Y: %f Z: %f",X,Y,Z);
	MessageBox(0,aBuffer,"foobar",MB_OK);
	sprintf(aBuffer,"X: %f Y: %f Z: %f W:%f",aElements[0],aElements[1],
		aElements[2],aElements[3]);
	MessageBox(0,aBuffer,"foobar",MB_OK);
#endif
}

inline vector_4::vector_4( void ) :
	quadruple(0,0,0,0)
{

}

inline vector_4::vector_4( real X, real Y, real Z ) :
	quadruple(X,Y,Z,0)
{

}

inline real matrix_4x4::GetElement( int Row, int Column ) const
{
	return aElements[Row][Column];
}

inline matrix_4x4 &matrix_4x4::SetElement( int Row, int Column, real Value )
{
	aElements[Row][Column] = Value;

	return *this;
}

inline matrix_4x4::matrix_4x4( do_not_initialize )
{

}

inline matrix_4x4::matrix_4x4( vector_4 const &Axis, real AngleInRadians )
{
	InitWith(Axis,AngleInRadians);
}

inline matrix_4x4::matrix_4x4( point_4 const &Origin, vector_4 const &XAxis,
			vector_4 const &YAxis, vector_4 const &ZAxis )
{
	InitWith(Origin,XAxis,YAxis,ZAxis);
}



#pragma warning (disable:4514)		// unreferenced inline removed
#pragma warning (default:4244)		// int to float conversion

#endif

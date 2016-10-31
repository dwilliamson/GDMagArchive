/*----------------------------------------------------------------------------

	dumb3d.cpp - A simple linear algebra library for 3D.

	Various parts Copyright Microsoft and Chris Hecker
*/

#include "dumb3d.hpp"

//*** We're intentionally converting doubles to floats all over...
#pragma warning ( disable : 4136 )
#pragma warning ( disable : 4244 )

#include <math.h>


/*----------------------------------------------------------------------------

matrix multiplication.

*/

matrix_4x4 operator*( matrix_4x4 const &Multiplicand,
		matrix_4x4 const &Multiplier )
{
	matrix_4x4 ReturnMatrix;

	for(int i = 0;i < 4;i++)
	{
		for(int j = 0;j < 4;j++)
		{
			real Value = 0;
		  
			for(int k = 0;k < 4;k++)
			{
				Value += Multiplicand.GetElement(i,k) *
						Multiplier.GetElement(k,j);
			}

			ReturnMatrix.SetElement(i,j,Value);
		}
	}

	return ReturnMatrix;
}

vector_4 operator*( matrix_4x4 const &Multiplicand,
		vector_4 const &Multiplier )
{
	vector_4 ReturnPoint;

	for(int i = 0;i < 4;i++)
	{
		real Value = 0;
	  
		for(int k = 0;k < 4;k++)
		{
			Value += Multiplicand.GetElement(i,k) *
					Multiplier.GetElement(k);
		}

		ReturnPoint.SetElement(i,Value);
	}

	return ReturnPoint;
}

point_4 operator*( matrix_4x4 const &Multiplicand,
		point_4 const &Multiplier )
{
	point_4 ReturnPoint;

	for(int i = 0;i < 4;i++)
	{
		real Value = 0;
	  
		for(int k = 0;k < 4;k++)
		{
			Value += Multiplicand.GetElement(i,k) *
					Multiplier.GetElement(k);
		}

		ReturnPoint.SetElement(i,Value);
	}

	return ReturnPoint;
}


/*----------------------------------------------------------------------------

constructors.

*/

matrix_4x4::matrix_4x4( void )
{
	for(int Counter = 0;Counter < 16;Counter++)
	{
		aElements[0][Counter] = 0;
	}

	aElements[0][0] = aElements[1][1] = aElements[2][2] = aElements[3][3] = 1;
}

void matrix_4x4::InitWith( vector_4 const &Vector, real Radians )
{
	real Sin = sin(Radians), Cos = cos(Radians);

	real Vx = Vector.GetX(), Vx2 = Vx * Vx;
	real Vy = Vector.GetY(), Vy2 = Vy * Vy;
	real Vz = Vector.GetZ(), Vz2 = Vz * Vz;

	aElements[0][0] = Vx2 + Cos * (1 - Vx2);
	aElements[0][1] = Vx * Vy * (1 - Cos) - Vz * Sin;
	aElements[0][2] = Vz * Vx * (1 - Cos) + Vy * Sin;
	aElements[0][3] = 0;
	
	aElements[1][0] = Vx * Vy * (1 - Cos) + Vz * Sin;
	aElements[1][1] = Vy2 + Cos * (1 - Vy2);
	aElements[1][2] = Vy * Vz * (1 - Cos) - Vx * Sin;
	aElements[1][3] = 0;
	
	aElements[2][0] = Vz * Vx * (1 - Cos) - Vy * Sin;
	aElements[2][1] = Vy * Vz * (1 - Cos) + Vx * Sin;
	aElements[2][2] = Vz2 + Cos * (1 - Vz2);
	aElements[2][3] = 0;

	aElements[3][0] = aElements[3][1] = aElements[3][2] = 0;
	aElements[3][3] = 1;
}

void matrix_4x4::InitWith( point_4 const &Origin, vector_4 const &XAxis,
			vector_4 const &YAxis, vector_4 const &ZAxis )
{
	// put in rotation

	for(int Counter = 0;Counter < 3;Counter++)
	{
		SetElement(0,Counter,XAxis.GetElement(Counter));
	}

	for(Counter = 0;Counter < 3;Counter++)
	{
		SetElement(1,Counter,YAxis.GetElement(Counter));
	}

	for(Counter = 0;Counter < 3;Counter++)
	{
		SetElement(2,Counter,ZAxis.GetElement(Counter));
	}

	// put in pre-translation to origin

	vector_4 OriginVector(Origin.GetX(),Origin.GetY(),Origin.GetZ());

	aElements[0][3] = DotProduct(XAxis,OriginVector);
	aElements[1][3] = DotProduct(YAxis,OriginVector);
	aElements[2][3] = DotProduct(ZAxis,OriginVector);
	
	aElements[3][0] = aElements[3][1] = aElements[3][2] = 0;
	aElements[3][3] = 1;
}	

	
/*----------------------------------------------------------------------------

Rotations.

*/

matrix_4x4 &matrix_4x4::ConcatenateXRotation( real Radians )
{
	real Temp10, Temp11, Temp12, Temp13;
	real Temp20, Temp21, Temp22, Temp23;

	real Sin = sin(Radians), Cos = cos(Radians);

	Temp10 = aElements[1][0] * Cos + aElements[2][0] * -Sin;
	Temp11 = aElements[1][1] * Cos + aElements[2][1] * -Sin;
	Temp12 = aElements[1][2] * Cos + aElements[2][2] * -Sin;
	Temp13 = aElements[1][3] * Cos + aElements[2][3] * -Sin;

	Temp20 = aElements[1][0] * Sin + aElements[2][0] * Cos;
	Temp21 = aElements[1][1] * Sin + aElements[2][1] * Cos;
	Temp22 = aElements[1][2] * Sin + aElements[2][2] * Cos;
	Temp23 = aElements[1][3] * Sin + aElements[2][3] * Cos;


	aElements[1][0] = Temp10;
	aElements[1][1] = Temp11;
	aElements[1][2] = Temp12;
	aElements[1][3] = Temp13;
	aElements[2][0] = Temp20;
	aElements[2][1] = Temp21;
	aElements[2][2] = Temp22;
	aElements[2][3] = Temp23;

	return *this;
}

matrix_4x4 &matrix_4x4::ConcatenateYRotation( real Radians )
{
	real Temp00, Temp01, Temp02, Temp03;
	real Temp20, Temp21, Temp22, Temp23;

	real Sin = sin(Radians), Cos = cos(Radians);

	Temp00 = aElements[0][0] * Cos + aElements[2][0] * Sin;
	Temp01 = aElements[0][1] * Cos + aElements[2][1] * Sin;
	Temp02 = aElements[0][2] * Cos + aElements[2][2] * Sin;
	Temp03 = aElements[0][3] * Cos + aElements[2][3] * Sin;

	Temp20 = aElements[0][0] * -Sin + aElements[2][0] * Cos;
	Temp21 = aElements[0][1] * -Sin + aElements[2][1] * Cos;
	Temp22 = aElements[0][2] * -Sin + aElements[2][2] * Cos;
	Temp23 = aElements[0][3] * -Sin + aElements[2][3] * Cos;

	aElements[0][0] = Temp00;
	aElements[0][1] = Temp01;
	aElements[0][2] = Temp02;
	aElements[0][3] = Temp03;
	aElements[2][0] = Temp20;
	aElements[2][1] = Temp21;
	aElements[2][2] = Temp22;
	aElements[2][3] = Temp23;

	return *this;
}

matrix_4x4 &matrix_4x4::ConcatenateZRotation( real Radians )
{
	real Temp00, Temp01, Temp02, Temp03;
	real Temp10, Temp11, Temp12, Temp13;

	real Sin = sin(Radians), Cos = cos(Radians);

	Temp00 = aElements[0][0] * Cos + aElements[1][0] * -Sin;
	Temp01 = aElements[0][1] * Cos + aElements[1][1] * -Sin;
	Temp02 = aElements[0][2] * Cos + aElements[1][2] * -Sin;
	Temp03 = aElements[0][3] * Cos + aElements[1][3] * -Sin;

	Temp10 = aElements[0][0] * Sin + aElements[1][0] * Cos;
	Temp11 = aElements[0][1] * Sin + aElements[1][1] * Cos;
	Temp12 = aElements[0][2] * Sin + aElements[1][2] * Cos;
	Temp13 = aElements[0][3] * Sin + aElements[1][3] * Cos;

	aElements[0][0] = Temp00;
	aElements[0][1] = Temp01;
	aElements[0][2] = Temp02;
	aElements[0][3] = Temp03;
	aElements[1][0] = Temp10;
	aElements[1][1] = Temp11;
	aElements[1][2] = Temp12;
	aElements[1][3] = Temp13;

	return *this;
}

/*----------------------------------------------------------------------------

Translations.

*/

matrix_4x4 &matrix_4x4::ConcatenateXTranslation( real Distance )
{
	aElements[0][0] = aElements[0][0] + Distance * aElements[3][0];
	aElements[0][1] = aElements[0][1] + Distance * aElements[3][1];
	aElements[0][2] = aElements[0][2] + Distance * aElements[3][2];
	aElements[0][3] = aElements[0][3] + Distance * aElements[3][3];

	return *this;
}

matrix_4x4 &matrix_4x4::ConcatenateYTranslation( real Distance )
{
	aElements[1][0] = aElements[1][0] + Distance * aElements[3][0];
	aElements[1][1] = aElements[1][1] + Distance * aElements[3][1];
	aElements[1][2] = aElements[1][2] + Distance * aElements[3][2];
	aElements[1][3] = aElements[1][3] + Distance * aElements[3][3];

	return *this;
}

matrix_4x4 &matrix_4x4::ConcatenateZTranslation( real Distance )
{
	aElements[2][0] = aElements[2][0] + Distance * aElements[3][0];
	aElements[2][1] = aElements[2][1] + Distance * aElements[3][1];
	aElements[2][2] = aElements[2][2] + Distance * aElements[3][2];
	aElements[2][3] = aElements[2][3] + Distance * aElements[3][3];

	return *this;
}

/*----------------------------------------------------------------------------

Scales.

*/

matrix_4x4 &matrix_4x4::ConcatenateXScale( real Distance )
{
	aElements[0][0] = aElements[0][0] * Distance;
	aElements[0][1] = aElements[0][1] * Distance;
	aElements[0][2] = aElements[0][2] * Distance;
	aElements[0][3] = aElements[0][3] * Distance;

	return *this;
}

matrix_4x4 &matrix_4x4::ConcatenateYScale( real Distance )
{
	aElements[1][0] = aElements[1][0] * Distance;
	aElements[1][1] = aElements[1][1] * Distance;
	aElements[1][2] = aElements[1][2] * Distance;
	aElements[1][3] = aElements[1][3] * Distance;

	return *this;
}

matrix_4x4 &matrix_4x4::ConcatenateZScale( real Distance )
{
	aElements[2][0] = aElements[2][0] * Distance;
	aElements[2][1] = aElements[2][1] * Distance;
	aElements[2][2] = aElements[2][2] * Distance;
	aElements[2][3] = aElements[2][3] * Distance;

	return *this;
}

/*----------------------------------------------------------------------------

vector length and normalize.

*/

real vector_4::Length( void )
{
	return sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ());
}

vector_4 &vector_4::Normalize( void )
{
	real MyLength = Length();

	SetX(GetX() / MyLength);
	SetY(GetY() / MyLength);
	SetZ(GetZ() / MyLength);

	return *this;
}
  
real AngleBetween( vector_4 const &Operand1, vector_4 const &Operand2 )
{
	vector_4 Unit1 = Operand1, Unit2 = Operand2;

	Unit1.Normalize();
	Unit2.Normalize();
	
	return acos(DotProduct(Unit1,Unit2));
}

real AngleBetweenUnit( vector_4 const &Unit1, vector_4 const &Unit2 )
{
	return acos(DotProduct(Unit1,Unit2));
}


void SetViewDistance( matrix_4x4 &Transform, real Distance )
{
	real InverseDistance = 1.0 / Distance;
	for(int Counter = 0;Counter < 4;Counter++)
	{
		Transform.SetElement(3,Counter,
			Transform.GetElement(2,Counter) * InverseDistance);
	}
}

/*----------------------------------------------------------------------------

view transform ctor.

*/

view_transform::view_transform( point_4 const &Viewpoint,
	vector_4 const &ViewDirection, vector_4 const &Up )
{
	// translate the viewpoint to the origin

	this->ConcatenateXTranslation(-Viewpoint.GetX());
	this->ConcatenateYTranslation(-Viewpoint.GetY());
	this->ConcatenateZTranslation(-Viewpoint.GetZ());

	// get view vectors set up

	vector_4 Right = -CrossProduct(ViewDirection,Up);
	vector_4 ReallyUp = CrossProduct(Right,ViewDirection);
	  
	matrix_4x4 LookDownZ;

	// @todo this should use the new matrix_4x4 ctor

	for(int Counter = 0;Counter < 3;Counter++)
	{
		LookDownZ.SetElement(0,Counter,Right.GetElement(Counter));
	}

	for(Counter = 0;Counter < 3;Counter++)
	{
		LookDownZ.SetElement(1,Counter,ReallyUp.GetElement(Counter));
	}

	for(Counter = 0;Counter < 3;Counter++)
	{
		LookDownZ.SetElement(2,Counter,ViewDirection.GetElement(Counter));
	}

	this->matrix_4x4::operator=(LookDownZ * *this);
}

//*** Turn on float conversion warning
#pragma warning ( default : 4136 )
#pragma warning ( disable : 4514 ) // unused inline function


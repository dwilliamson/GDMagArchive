// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : camera.cpp
// Description : Handle the basic functions of the object "camera"
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------
#include "camera.hpp"
#include <math.h>

void Camera::SetPosition(float px, float py, float pz)
{
	Orient.SetPosition( Hvector(px, py, pz) );
//	x = px;
//	y = py;
//	z = pz;
}

void Camera::SetPosition(Hvector &v)
{
	Orient.SetPosition( v );
//	x = px;
//	y = py;
//	z = pz;
}

//void		Target(float, float, float); // set_orient
void Camera::Euler(float a, float b, float c)
{
	Orient.Euler( (long)a, (long)b, (long)c );
}



void Camera::Rotate(int Direction, int Value)
{
	Hmatrix4x4 a,b;
	Camera &camera = *this;

	switch(Direction)
	{
		case LEFT:
			a = camera.GetRotation();
			b.Euler(0,Value,0);
			a = Hproduct(a,b);
			camera.SetRotation(a);
			break;
		case RIGHT:
			a = camera.GetRotation();
			b.Euler(0,-Value,0);
			a = Hproduct(a,b);
			camera.SetRotation(a);
			break;
		case UP:
			a = camera.GetRotation();
			b.Euler(-Value,0,0);
			a = Hproduct(a,b);
			camera.SetRotation(a);
			break;
		case DOWN:
			a = camera.GetRotation();
			b.Euler(Value,0,0);
			a = Hproduct(a,b);
			camera.SetRotation(a);
			break;
		case ROLL_RIGHT:
			a = camera.GetRotation();
			b.Euler(0,0,Value);
			a = Hproduct(a,b);
			camera.SetRotation(a);
			break;
		case ROLL_LEFT:
			a = camera.GetRotation();
			b.Euler(0,0,-Value);
			a = Hproduct(a,b);
			camera.SetRotation(a);
			break;
/*	m = camera.coosystem.gmat;
	m1.fromEuler(0,0,10);
	m = Hproduct(m1, m);
	camera.coosystem.gmat=m;	 */

	}
}



Hmatrix4x4 Camera::GetRotation()
{
	return(Orient);
}


void Camera::SetRotation(Hmatrix4x4 &m)
{
	Orient = m;
}

Hvector	Camera::GetPosition()
{
	return Orient.GetPosition();
}

Hvector	Camera::GetZAxis()
{
	return Orient.GetZAxis();
}

void Camera::Move(int Direction, int Value)
{
	float Speed = (float) Value;
	Hvector Axis;
	Hvector Pos;
	
	Camera &camera = *this;

	switch(Direction)
	{
		case BACKWARD:
			Axis = camera.GetZAxis();
			Pos = camera.GetPosition();
			Pos -= Axis * Speed;
			camera.SetPosition( Pos );
			break;

		case FORWARD:
			Axis = camera.GetZAxis();
			Pos = camera.GetPosition();
			Pos += Axis * Speed;
			camera.SetPosition( Pos );
			break;
	}
}


void Camera::SetViewport(float w, float h)
{
	Width = w;
	Height = h;
}


void Camera::SetFocus(float f)
{
	float w = Width/2;
	float h = Height/2;
	float fovrad = (float) ((2*3.14*f) / 360);	// conversion en radians

	Focus = ((float)(w*2) / (float)(2*tan(fovrad/2)));
}

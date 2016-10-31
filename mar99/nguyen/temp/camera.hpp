#ifndef __CAMERA
#define __CAMERA

#include "Hmatrix4x4.hpp"

class Camera
{
public:
	float Width;
	float Height;
	float Focus;

	enum {
		LEFT=0,
		RIGHT,
		UP,
		DOWN,
		FORWARD,
		BACKWARD,
		ROLL_RIGHT,
		ROLL_LEFT
	} ROTATIONTYPE;

	Hmatrix4x4  Orient;

	Hmatrix4x4  GetRotation(void);
	void		SetRotation(Hmatrix4x4 &);
	void		SetPosition(float, float, float);
	void		SetPosition(Hvector &);
	Hvector		GetPosition();
	Hvector		GetZAxis();
	void		Target(float, float, float); // set_orient
	void		Euler(float, float, float);

	// control
	void		Rotate(int, int);
	void		Move(int, int);
	void		SetViewport(float, float);
	void		SetFocus(float);
	Camera()
	{
		Width = 320;
		Height = 240;
		Focus = 400;
	}
};


#endif
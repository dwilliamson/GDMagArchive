/*	#ifndef HMATRIX
		#include "Hmatrix.hpp"
	#endif*/

#ifndef HVECTOR
#define HVECTOR

class Hvector
{
public:
	float x,y,z;
	Hvector ();	
	Hvector (float , float , float );

	void setx(float value) {x=value;}
	void sety(float value) {y=value;}
	void setz(float value) {z=value;}
	void setvect(float vx, float vy, float vz) { x=vx; y=vy; z=vz;}
	void getvect(float *fx, float *fy, float *fz) {*fx=x; *fy=y; *fz=z;}

//	Hvector rotate(const Hmatrix &) const;

	Hvector operator + (const Hvector & ) const;
	Hvector operator - (const Hvector & ) const;
	Hvector operator * (float scalefactor) const;
	Hvector operator / (float scalefactor) const;

	Hvector& operator += (const Hvector & );
	Hvector& operator -= (const Hvector& b);
	Hvector& operator *= (float scalefactor);
};


//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector::Hvector(float fx, float fy, float fz)
{
	x = fx;	
	y = fy;
	z = fz;
}




//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector::Hvector()
{
	x = (float)0;	
	y = (float)0;
	z = (float)0;
}


//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector Hvector::operator + (const Hvector& b) const
{
	return	Hvector	(
					(x + b.x),
					(y + b.y),
					(z + b.z) );
}

//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector& Hvector::operator += (const Hvector& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
	return(*this);
}



//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector& Hvector::operator -= (const Hvector& b)
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	return(*this);
}



//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector Hvector::operator - (const Hvector& b) const
{
	return Hvector(	x - b.x,
			y - b.y,
			z - b.z );
}


//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector Hvector::operator * (float scalefactor) const
{
	return Hvector (	
			x*scalefactor,
			y*scalefactor,
			z*scalefactor);
}

//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector Hvector::operator / (float scalefactor) const
{
	float oos = 1/scalefactor;
	return Hvector (x*oos, y*oos, z*oos);
}


//-----------------------------------------------
//|												|
//-----------------------------------------------
inline Hvector& Hvector::operator *= (float scalefactor)
{
	x *= scalefactor;
	y *= scalefactor;
	z *= scalefactor;
	return(*this);
}

#endif



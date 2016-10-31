// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : tools.cpp
// Description : various functions needed by the example...
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------

#include "tools.hpp"



//-------------------------------------------------
//|                                               |
//-------------------------------------------------
float Dot_product(Hvector a, Hvector b)
{
  return ( (a.x*b.x) + (a.y*b.y) + (a.z*b.z) );
}


//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hvector Product(Hmatrix4x4 &m, Hvector &v)
{
	return Hvector (v.x*m.xx + v.y*m.yx + v.z*m.zx,	// x
					v.x*m.xy + v.y*m.yy + v.z*m.zy,	// y
					v.x*m.xz + v.y*m.yz + v.z*m.zz);// 
}


//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hvector Product(const Hvector &a, const Hvector &b)
{
	return(	Hvector ( 
			(a.y*b.z) - (a.z*b.y),
			(a.z*b.x) - (a.x*b.z),
			(a.x*b.y) - (a.y*b.x)
			)
		);
}

//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hvector Normalize(Hvector vo)
{
	float l;
	Hvector v;

	l = (float) sqrt(vo.x*vo.x + vo.y*vo.y + vo.z*vo.z);
	if (l==0.0)
	{
		v.x = (float)0;
		v.y = (float)0;
		v.z = (float)0;
	}
	else
	{
		float temp = (float) (1.0 / l);
		v.x = vo.x * temp;	
		v.y = vo.y * temp;
		v.z = vo.z * temp;
	}
	return(v);
}


//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hvector NormalVector(const Hvector &v0, const Hvector &v1, const Hvector &v2)
{
	Hvector n, a, b;

	a = v1-v0;
	b = v2-v1;
	
	n = Product(a,b);
	n= Normalize(n);
	
	return(n);
}



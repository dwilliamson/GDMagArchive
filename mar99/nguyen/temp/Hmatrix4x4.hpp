#ifndef HMATRIX4X4
#define HMATRIX4X4

#ifndef HSINCOS
	#include "Hsincos.hpp"
#endif


#include "Hvector.hpp"


class Hmatrix4x4
{
private:
	static		Hsincos sc;

public:
	float		xx,xy,xz,xw;
	float		yx,yy,yz,yw;
	float		zx,zy,zz,zw;
	float		tx,ty,tz,tw;

	Hmatrix4x4();

	void		Euler(long rx, long ry, long rz);

	void		GetPosition(Hvector &);
	Hvector		GetPosition(void);
	void		SetPosition(Hvector &);
	Hmatrix4x4	GetRotation(void);
	void		SetRotation(Hmatrix4x4 &);
	Hvector		GetZAxis(void);

	void		Identity(void);
	void		Inverse(void);
	void		InverseRotation(void);
};


Hmatrix4x4 Hproduct(Hmatrix4x4 &c, Hmatrix4x4 &p);


#endif
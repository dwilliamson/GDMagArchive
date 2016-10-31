// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : Hmatrix4x4.cpp
// Description : Basic functions to handle generation of a rotation matrix
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------
#include "Hmatrix4x4.hpp"


Hsincos Hmatrix4x4::sc;


//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hmatrix4x4::Hmatrix4x4()
{
	Identity();
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void Hmatrix4x4::Identity(void)
{
	xy=xz=xw= (float)0.0;
	yx=yz=yw= (float)0.0;
	zx=zy=zw= (float)0.0;
	tx=ty=tz= (float)0.0;

	xx=yy=zz=tw= (float)1.0;
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void Hmatrix4x4::SetPosition(Hvector &v)
{
	tx = v.x;
	ty = v.y;
	tz = v.z;
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void Hmatrix4x4::GetPosition(Hvector &v)
{
	v.x = tx;
	v.y = ty;
	v.z = tz;
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hvector Hmatrix4x4::GetPosition(void)
{
	return Hvector(tx, ty, tz);
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hmatrix4x4 Hmatrix4x4::GetRotation(void)
{
	Hmatrix4x4 tmp;

	tmp = *this;
	tmp.tx = 0;
	tmp.ty = 0;
	tmp.tz = 0;
	tmp.tw = 1;
	tmp.xw = 0;
	tmp.yw = 0;
	tmp.zw = 0;
	return(tmp);
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void Hmatrix4x4::SetRotation(Hmatrix4x4 &r)
{
	xx = r.xx;	xy = r.xy;	xz = r.xz;
	yx = r.yx;	yy = r.yy;	yz = r.yz;
	zx = r.zx;	zy = r.zy;	zz = r.zz;
}





//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void Hmatrix4x4::Euler(long rx, long ry, long rz)
{
	float  coszrot;
	float  sinzrot;
	float  cosyrot;
	float  sinyrot;
	float  cosxrot;
	float  sinxrot;
	float  sinPicthSinxrot;
	float  coszrotCosxrot;
	float  sinzrotCosxrot;
	// Hmatrix4x4 m;
	rx &= 4096-1;
	ry &= 4096-1;
	rz &= 4096-1;

	sinzrot   = sc.tsin [rz];
	coszrot   = sc.tcos [rz];
	sinyrot   = sc.tsin [ry];
	cosyrot   = sc.tcos [ry];
	sinxrot   = sc.tsin [rx];
	cosxrot   = sc.tcos [rx];

	sinPicthSinxrot  = sinyrot * sinxrot;
	coszrotCosxrot   = coszrot * cosxrot;
	sinzrotCosxrot   = sinzrot * cosxrot;

	// Order of rotation: zrot, yrot, xrot
	xx = coszrot * cosyrot;
	xy = coszrot * sinPicthSinxrot - sinzrotCosxrot;
	xz = coszrotCosxrot * sinyrot  + sinzrot * sinxrot;
	yx = sinzrot * cosyrot;
	yy = coszrotCosxrot + sinzrot * sinPicthSinxrot;
	yz = sinzrotCosxrot * sinyrot  - coszrot * sinxrot;
	zx = - sinyrot;
	zy = cosyrot * sinxrot;
	zz = cosyrot * cosxrot;
}





//-------------------------------------------------
//;|xx xy xz|
//;|yx yy yz|
//;|zx zy zz|
//     ->
//;|xx yx zx|
//;|xy yy zy|
//;|xz yz zz|
//-------------------------------------------------
void Hmatrix4x4::Inverse(void)
{
	float	txx, txy, txz,
			tyx, tyy, tyz,
			tzx, tzy, tzz,
			ttx, tty, ttz;

	txx = xx;	txy = yx;	txz = zx;
	tyx = xy;	tyy = yy;	tyz = zy;
	tzx = xz;	tzy = yz;	tzz = zz;
	ttx = -tx;  tty = -ty;  ttz = -tz;

	xx = txx;	xy = txy;	xz = txz;
	yx = tyx;	yy = tyy;	yz = tyz;
	zx = tzx;	zy = tzy;	zz = tzz;
	tx = ttx;	ty = tty;	tz = ttz;
}




//-------------------------------------------------
//;|xx xy xz|
//;|yx yy yz|
//;|zx zy zz|
//     ->
//;|xx yx zx|
//;|xy yy zy|
//;|xz yz zz|
//-------------------------------------------------
void Hmatrix4x4::InverseRotation(void)
{
	float	txx, txy, txz,
			tyx, tyy, tyz,
			tzx, tzy, tzz,
			ttx, tty, ttz;

	txx = xx;	txy = yx;	txz = zx;
	tyx = xy;	tyy = yy;	tyz = zy;
	tzx = xz;	tzy = yz;	tzz = zz;
	//ttx = -tx;  tty = -ty;  ttz = -tz;

	xx = txx;	xy = txy;	xz = txz;
	yx = tyx;	yy = tyy;	yz = tyz;
	zx = tzx;	zy = tzy;	zz = tzz;
	//tx = ttx;	ty = tty;	tz = ttz;
}



//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Hvector Hmatrix4x4::GetZAxis(void)
{
	return Hvector(zx,zy,zz);
}





//------------------------------------
// parent, child
//------------------------------------
Hmatrix4x4 Hproduct(Hmatrix4x4 &p, Hmatrix4x4 &c)
{
	Hmatrix4x4 res;

	res.xx = c.xx*p.xx + c.xy*p.yx + c.xz*p.zx;
	res.xy = c.xx*p.xy + c.xy*p.yy + c.xz*p.zy;
	res.xz = c.xx*p.xz + c.xy*p.yz + c.xz*p.zz;
	res.xw = 0;

	res.yx = c.yx*p.xx + c.yy*p.yx + c.yz*p.zx;
	res.yy = c.yx*p.xy + c.yy*p.yy + c.yz*p.zy;
	res.yz = c.yx*p.xz + c.yy*p.yz + c.yz*p.zz;
	res.yw = 0;

	res.zx = c.zx*p.xx + c.zy*p.yx + c.zz*p.zx;
	res.zy = c.zx*p.xy + c.zy*p.yy + c.zz*p.zy;
	res.zz = c.zx*p.xz + c.zy*p.yz + c.zz*p.zz;
	res.zw = 0;

	res.tx = c.tx*p.xx + c.ty*p.yx + c.tz*p.zx + p.tx;
	res.ty = c.tx*p.xy + c.ty*p.yy + c.tz*p.zy + p.ty;
	res.tz = c.tx*p.xz + c.ty*p.yz + c.tz*p.zz + p.tz;

	res.tw = 1;

	return (res);
}




/*
				pere
				xx' xy' xz' 0
				yx' yy' yz' 0
				zx' zy' zz' 0
				tx' ty' tz' 1
fils
xx xy xz 0		xx² xy² xz² xw²
yx yy yz 0		yx² yy² yz² yw²
zx zy zz 0		zx² zy² zz² zw²
tx ty tz 1		tx² ty² tz² tw²

xx² = xx*xx' + xy*yx' + xz*zx'
xy² = xx*xy' + xy*yy' + xz*zy'
xz² = xx*xz' + xy*yz' + xz*zz'
xw² = 0

yx² = yx*xx' + yy*yx' + yz*zx'
yy² = yx*xy' + yy*yy' + yz*zy'
yz² = yx*xz' + yy*yz' + yz*zz'
yw² = 0

zx² = zx*xx' + zy*yx' + zz*zx'
zy² = zx*xy' + zy*yy' + zz*zy'
zz² = zx*xz' + zy*yz' + zz*zz'
zw² = 0

tx² = tx*xx' + ty*yx' + tz*zx' + 1*tx'
ty² = tx*xy' + ty*yy' + tz*zy' + 1*ty'
tz² = tx*xz' + ty*yz' + tz*zz' + 1*tz'
tw² = 1

  */
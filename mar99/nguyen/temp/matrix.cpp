#include <math.h>

#include "matrix.hpp"



//-------------------------------------------------
//|      Matrice a partir                         |
//|  d'1 angle et d'1 vecteur                     |
//|                                               |
//-------------------------------------------------
/*void Matrix::setorient(Hvector *vect, long angle)
{
  float racine,Un_racine;
  float a,b,c;
  
  *vect = Hnormalize(*vect);

  a = vect->x;
  b = vect->y;
  c = vect->z;
  racine=(float) sqrt(1.0- (b*b) );
  Un_racine=1.0/racine;

  xx = (c*Un_racine);
  xy = (-a*b*Un_racine);
  xz = vect->x;

  yx = (float)0.0;
  yy = racine;
  yz = vect->y;

  zx = (-a*Un_racine);
  zy = (-b*c*Un_racine);
  zz = vect->z;
}*/






//-----------------------------------------------
//|												|
//-----------------------------------------------
Matrix::Matrix()
{
	xx=(float)1;xy=(float)0;xz=(float)0;
	yx=(float)0;yy=(float)1;yz=(float)0;
	zx=(float)0;zy=(float)0;zz=(float)1;
}



//-----------------------------------------------
//|												|
//-----------------------------------------------
void Matrix::Identity()
{	
	xx=(float)1;xy=(float)0;xz=(float)0;
	yx=(float)0;yy=(float)1;yz=(float)0;
	zx=(float)0;zy=(float)0;zz=(float)1;
}



//-------------------------------------------------
//|                                               |
//-------------------------------------------------
void Matrix::Euler(float rz, float ry, float rx)
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
 Matrix m;

 sinzrot   = (float)sin(rz);
 coszrot   = (float)cos(rz);
 sinyrot   = (float)sin(ry);
 cosyrot   = (float)cos(ry);
 sinxrot   = (float)sin(rx);
 cosxrot   = (float)cos(rx);

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
Matrix Matrix::Inverse(void)
{
 Matrix mt;
 
 mt.xx = xx;
 mt.xy = yx;
 mt.xz = zx;
 mt.yx = xy;
 mt.yy = yy;
 mt.yz = zy;
 mt.zx = xz;
 mt.zy = yz;
 mt.zz = zz;

 return(mt);
}



//-------------------------------------------------
//|                                               |
//-------------------------------------------------
Matrix Matrix::Product(const Matrix&b) const
{
  Matrix result;

  result.xx = xx * b.xx + xy * b.yx + xz * b.zx; // xx
  result.yx = yx * b.xx + yy * b.yx + yz * b.zx; // yx
  result.zx = zx * b.xx + zy * b.yx + zz * b.zx; // zx

  result.xy = xx * b.xy + xy * b.yy + xz * b.zy; // xy
  result.yy = yx * b.xy + yy * b.yy + yz * b.zy; // yy
  result.zy = zx * b.xy + zy * b.yy + zz * b.zy; // zy

  result.xz = xx * b.xz + xy * b.yz + xz * b.zz; // xz
  result.yz = yx * b.xz + yy * b.yz + yz * b.zz; // yz
  result.zz = zx * b.xz + zy * b.yz + zz * b.zz; // zz

  return(result);
}





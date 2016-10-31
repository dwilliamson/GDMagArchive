// -----------------------------------------------------------------
// -----------------------------------------------------------------
// Subject : Casting shadows on volumes - Game Developer, march 1999 issue
// Author : Nguyen Huu Hubert (nguyenyoda@aol.com)
//
// File : Hsincos.cpp
// Description : A sin / cos table. May be I should remove this...
//
// -----------------------------------------------------------------
// -----------------------------------------------------------------
#include <math.h>

#include "Hsincos.hpp"


//-----------------------------------------------
//|												|
//|												|
//|												|
//|												|
//-----------------------------------------------
Hsincos::Hsincos()
{
  long i;

  for (i=0; i<TRIG_RANGE; i++)
  {
   tcos[i] = (float)(cos(i*2*3.141592654/TRIG_RANGE));
   tsin[i] = (float)(sin(i*2*3.141592654/TRIG_RANGE));
  }
}


//-----------------------------------------------
//|												|
//|												|
//|												|
//|												|
//-----------------------------------------------
float Hsincos::getsine(long value)
{
	if ((value>TRIG_RANGE) || (value<0)) return((float)2);
	return(tsin[value]);
}


//-----------------------------------------------
//|												|
//|												|
//|												|
//|												|
//-----------------------------------------------
float Hsincos::getcosine(long value)
{
	if ((value>TRIG_RANGE) || (value<0)) return((float)2);
	return(tcos[value]);
}


//-----------------------------------------------
//|												|
//|												|
//|												|
//|												|
//-----------------------------------------------
long Hsincos::getrange(void)
{
	return(TRIG_RANGE);
}



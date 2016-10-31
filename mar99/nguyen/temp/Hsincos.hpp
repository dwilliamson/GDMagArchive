#ifndef __HSINCOS

#define TRIG_RANGE 4096
#define __HSINCOS

class Hsincos
{
public:
	float tcos[TRIG_RANGE];
	float tsin[TRIG_RANGE];

	Hsincos ();
	float getsine(long value);
	float getcosine(long value);
	long getrange(void);
};

#endif
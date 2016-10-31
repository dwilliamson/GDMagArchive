class Hrect
{
public:
	long top;
	long bottom;
	long left;
	long right;

	Hrect();
	Hrect(long, long, long, long);
};

inline Hrect::Hrect()
{
	top=bottom=left=right=0;
}

inline Hrect::Hrect(long t, long l, long r, long b)
{
	top = t;
	right = r;
	left = l;
	bottom = b;
}

void  copy2video(unsigned short *output, Hrect &src, Hrect &dst, int bpp);
void  Hset_copy2videohwnd(int h);


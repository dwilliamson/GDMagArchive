#include "Hvector.hpp"
#include "camera.hpp"
#include "Hmatrix4x4.hpp"

class Vertex
{
public:
	enum {
		CLIPLEFT	= 1,
		CLIPRIGHT	= 2,
		CLIPTOP		= 4,
		CLIPBOTTOM	= 8,
		CLIPZ		= 16,
	} VERTEXFLAGS;

	float			x,y,z;
	float			px,py,pz;
	float			u,v;
	unsigned long	flags;

	Vertex(float fx, float fy, float fz)
	{
		x = fx; 
		y = fy; 
		z = fz;
	}

	Vertex()
	{
		x = y = z = px = py = pz = u = v = 0;
		flags = 0;
	}
};


class Mesh
{
public:
	enum {
		XPLUS=0,
		XLESS,
		YPLUS,
		YLESS,
		ZPLUS,
		ZLESS,
	} MOVETYPE;

	Hmatrix4x4	Local;
	Hmatrix4x4	Global;
	int			nVertices;
	int			nTriangles;

	Vertex *	VertexArray;
	int*		TriangleArray;
	Hvector*	TriangleNormalArray;
	char *		FlagArray;

	Mesh();
	~Mesh();
	void		Create(int nv, int nt, float *va, int *ta);
	void		Rotate(Camera &);
	void		SetPosition(float, float, float);
	void		SetPosition(Hvector &);
	Hvector		GetPosition(void);
	void		Euler(long, long, long);
	void		Move(int, float);
};
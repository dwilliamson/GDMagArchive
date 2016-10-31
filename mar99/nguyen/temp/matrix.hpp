#ifndef HMATRIX
#define HMATRIX

class Matrix
{
public:
	
	float xx,xy,xz;
	float yx,yy,yz;
	float zx,zy,zz;
	
	Matrix ();		
	void Identity();
	void Euler(float, float, float);
	Matrix Inverse(void);
	Matrix Product(const Matrix&) const;
//	void setorient(Hvector *, long );
};


#endif
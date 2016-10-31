// SPLINE.CPP
//
// classes used to represent various splines and spline patches
//
// Author: Kai Martin

class Vector3D
{
	Vector3D(){};
	~Vector3D(){};

	float x, y, z;
};


class BSplineBasis
{
public:
	BSplineBasis( int order = 0, int numControlPoints = 0 );
	~BSplineBasis();

	// returns value given order k, index i, and curve parameter u
	float calcBasis ( int i, int order, float u );

	int k;  // order of basis function
	int n;  // n + 1 = # of control points influenced by basis function
	int *vKnot;  // knot vector for basis function
};


class UniformBSplineCurve
{
public:
	UniformBSplineCurve ( int order = 0, Vector3D *controlPoints = 0, 
					 int numControlPoints = 0 );
	~UniformBSplineCurve();

	// evaluate a number of points along the spline (includes the starting
	// point and end point of the curve)
	Vector3D* evaluate ( int numPoints );

	Vector3D* curveFit ( int order, int numControlPoints, 
			  		     Vector3D *dataPoints, int numDataPoints );

	// calculate the parameter values along the curve at each data point, 
	// which is used in the curve fitting
	float* calcCurveParameterValues ( int n, int numDataPoints, Vector3D *dataPoints );

	Vector3D *p;  // array of control points
	BSplineBasis *N;
};


class UniformBSplineSurface
{
public:
	enum { _UBS_CALC_ALONG_X, _UBS_CALC_ALONG_Y, };

	UniformBSplineSurface( int orderX = 0, int orderY = 0 );
	
	UniformBSplineSurface( int orderX, int orderY, Vector3D *controlPoints, 
						   int numControlPointsX, int numControlPointsY );

	~UniformBSplineSurface();

	Vector3D* evaluate ( int numPointsX, int numPointsY );
	Vector3D* surfaceFit ( int orderX, int orderY, 
						   int numControlPointsX, int numControlPointsY, 
						   Vector3D *dataPoints, int numDataPointsX, int numDataPointsY );

	// calculate the parameter values in the X or Y direction (dictated by calcFlag, which 
	// should be either _UBS_CALC_ALONG_X or _UBS_CALC_ALONG_Y) at each data point, 
	// which is used in surface fitting
	float* calcSurfaceParameterValues ( int row, int col,  
							     int numDataPointsX, int numDataPointsY, 
								 Vector3D *dataPoints, int calcFlag );

	Vector3D *p; // 1-d array representing a 2-d grid of control points
	BSplineBasis *N, *M;
};

BSplineBasis::BSplineBasis( int order, int numControlPoints )
{
	k = order;
	n = numControlPoints - 1;
	vKnot = (int *) malloc ( sizeof ( int ) * (n + k + 1) );

	for ( int i = 0; i < n + k + 1; i++ )
	{
		if ( i > n )
			vKnot[i] = n - k + 2;
		else
		if ( i > k - 1 )
			vKnot[i] = i - k + 1;
		else
			vKnot[i] = 0;
	}
}


BSplineBasis::~BSplineBasis()
{
	free ( vKnot );
}


float BSplineBasis::calcBasis ( int i, int order, float u )
{
	if ( order == 1 )
	{
		if ( u >= vKnot[i] && u < vKnot[i + 1] )
			return 1.0f;
		else 
			return 0.0f;
	} else {
		if ( (vKnot[i + order - 1] - vKnot[i]) <= 0 
			&& (vKnot[i + order] - vKnot[i + 1]) <= 0 )
			return 0.0f;
		else
		if ( (vKnot[i + order - 1] - vKnot[i]) <= 0 )
			return (vKnot[i + order] - u) * calcBasis ( i + 1, order - 1, u ) / (vKnot[i + order] - vKnot[i + 1]);
		else
		if ( vKnot[i + order] - vKnot[i + 1] <= 0 )
			return (u - vKnot[i]) * calcBasis ( i, order - 1, u ) / (vKnot[i + order - 1] - vKnot[i]);
		else
			return (vKnot[i + order] - u) * calcBasis ( i + 1, order - 1, u ) / (vKnot[i + order] - vKnot[i + 1])
					+ (u - vKnot[i]) * calcBasis ( i, order - 1, u ) / (vKnot[i + order - 1] - vKnot[i]);
				
	}
}


UniformBSplineCurve::UniformBSplineCurve ( int order, Vector3D *controlPoints, 
							     int numControlPoints )
{
	N = new BSplineBasis ( order, numControlPoints );
	p = (Vector3D *) malloc ( sizeof ( Vector3D ) * numControlPoints );
	memcpy ( p, controlPoints, sizeof ( Vector3D ) * numControlPoints );
}


UniformBSplineCurve::~UniformBSplineCurve()
{
	delete N;
}


Vector3D* UniformBSplineCurve::evaluate ( int numPoints )
{
	Vector3D *ptArray = (Vector3D *) malloc ( sizeof ( Vector3D ) * numPoints );

	float du = (float)(N->n - N->k + 2) / (numPoints - 1);
	float u = 0;

	for ( int idx = 0; idx < numPoints - 1; idx++ )
	{
		Vector3D Q( 0, 0, 0 );

		for ( int i = 0; i <= N->n; i++ )
		{
			float weight = N->calcBasis ( i, N->k, u );
			Q.x += weight * p[i].x;
			Q.y += weight * p[i].y;
			Q.z += weight * p[i].z;
		}

		ptArray[idx] = Q;

		u += du;
	}

	ptArray[numPoints - 1] = p[N->n];

	return ptArray;
}


Vector3D* UniformBSplineCurve::curveFit ( int order, int numControlPoints, 
			  								Vector3D *dataPoints, int numDataPoints )
{
	Vector3D *ptArray;

	return ptArray;
}


UniformBSplineSurface::UniformBSplineSurface( int orderX, int orderY, Vector3D *controlPoints, 
									  	      int numControlPointsX, int numControlPointsY )
{
	N = new BSplineBasis ( orderX, numControlPointsX );
	M = new BSplineBasis ( orderY, numControlPointsY );

	if ( numControlPointsX && numControlPointsY && controlPoints != NULL )
	{
		p = (Vector3D *) malloc ( sizeof ( Vector3D ) * numControlPointsX * numControlPointsY );
		memcpy ( p, controlPoints, sizeof ( Vector3D ) * numControlPointsX * numControlPointsY );
	}
}


UniformBSplineSurface::UniformBSplineSurface( int orderX, int orderY )
{
}


UniformBSplineSurface::~UniformBSplineSurface()
{
}


Vector3D* UniformBSplineSurface::evaluate ( int numPointsX, int numPointsY )
{
	
	float minZ = p[0].z; float maxZ = 0;
	for ( int k = 0; k < (N->n + 1) * (M->n + 1); k++ )
	{
		if ( p[k].z < minZ )
			minZ = p[k].z;

		if ( p[k].z > maxZ )
			maxZ = p[k].z;
	}

	Vector3D *ptArray = (Vector3D *) malloc ( sizeof ( Vector3D ) * numPointsX * numPointsY );

	float du = (float)(N->n - N->k + 2) / numPointsX;
	float dw = (float)(M->n - M->k + 2) / numPointsY;

	float u = 0, w = 0;

	float **basisValuesX = (float **) malloc ( sizeof ( float *) * numPointsX );
	float **basisValuesY = (float **) malloc ( sizeof ( float *) * numPointsY );

	for ( int i = 0; i < numPointsX; i++ )
	{
		basisValuesX[i] = (float *) malloc ( sizeof ( float ) * (N->n + 1) );

		for ( int a = 0; a <= N->n; a++ )
		{
			basisValuesX[i][a] = N->calcBasis ( a, N->k, u );
		}

		u += du;
	}

	for ( i = 0; i < numPointsY; i++ )
	{
		basisValuesY[i] = (float *) malloc ( sizeof ( float ) * (M->n + 1) );

		for ( int a = 0; a <= M->n; a++ )
		{
			basisValuesY[i][a] = M->calcBasis ( a, M->k, w );
		}

		w += dw;
	}


	int idx;
	int controlPtIdx;
	
	w = 0;

	float weightSumX, weightSumY, weightSumXY;
	Vector3D R, Q;

	int min, max = 0;
	int foundMin = 0;

	for ( int y = 0; y < numPointsY; y++ )
	{
		u = 0;

		for ( int x = 0; x < numPointsX; x++ )
		{
			R.x = 0.0f;
			R.y = 0.0f;
			R.z = 0.0f;

			weightSumY = 0;
			weightSumXY = 0;

			for ( int j = 0; j <= M->n; j++ )
			{
				Q.x = 0.0f;
				Q.y = 0.0f;
				Q.z = 0.0f;

				float weightY = basisValuesY[y][j];

				if ( weightY )
				{
					for ( int i = 0; i <= N->n; i++ )
					{
						float weightX = basisValuesX[x][i];

						if ( weightX )
						{
							controlPtIdx = j * (N->n + 1) + i;
							Q.x += weightX * p[controlPtIdx].x;
							Q.y += weightX * p[controlPtIdx].y;
							Q.z += weightX * p[controlPtIdx].z;
						}
					}

					Q.x *= weightY;
					Q.y *= weightY;
					Q.z *= weightY;

					R.x += Q.x;
					R.y += Q.y;
					R.z += Q.z;
				}
			}

			idx = y * numPointsX + x;

			if ( R.z > maxZ + 1 || R.z + 1 < minZ  )
				alert ( "Evaluated point is outside of convex hull!  Val = %f, (u, w) = (%f, %f), Min = %f, Max = %f",
						 R.z, u, w, minZ, maxZ );

			ptArray[idx] = R;

			if ( !foundMin )
			{
				min = ptArray[idx].z;
				foundMin++;
			}

			if ( ptArray[idx].z < min )
				min = ptArray[idx].z;

			if ( ptArray[idx].z > max )
				max = ptArray[idx].z;
			
			u += du;
		}

		w += dw;
	}

 	return ptArray;
}

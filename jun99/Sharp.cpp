/**** LISTING 1 (BEGIN) ****/
void genCubicFunction()
{
    // Do this so we can treat each endpoint and tangent vector as a separate array.
    float* p0 = points;
    float* p1 = points + 3;
    float* v0 = tangents;
    float* v1 = tangents + 3;

    // Do this so we can treat each vector coefficient of the function as a separate array.
    float* a = functionCoeffs;
    float* b = functionCoeffs + 3;
    float* c = functionCoeffs + 6;
    float* d = functionCoeffs + 9;

    // Now, generate each coefficient from the endpoints, tangents, and the predefined basis functions.
    // Note that we loop once each for the x, y, and z components of the vector function.
    for (int lcv = 0; lcv < 3; lcv++)
    {
        // a = 2p0 - 2p1 + v0 + v1
        a[ lcv ] = (p0[ lcv ] + p0[ lcv ]) - (p1[ lcv ] + p1[ lcv ]) + v0[ lcv ] + v1[ lcv ];

        // b = -3p0 + 3p1 - 2v0 - v1
        b[ lcv ] = - (p0[ lcv ] + p0[ lcv ] + p0[ lcv ]) + (p1[ lcv ] + p1[ lcv ] + p1[ lcv ]) - (v0[ lcv ] + v0[ lcv ]) - v1[ lcv ];

        // c = v0
        c[ lcv ] = v0[ lcv ];

        // d = p0
        d[ lcv ] = p0[ lcv ];
    }
}
/**** LISTING 1 (END) ****/

/**** LISTING 2 (BEGIN) ****/
// This function simply computes au^3 + bu^2 + cu + d for a 
// specific u and stores the vector result in out.
void evaluateAt(float u, float* out)
{
    // Do this so we can treat each vector coefficient of the function as a separate array.
    float* a = functionCoeffs;
    float* b = functionCoeffs + 3;
    float* c = functionCoeffs + 6;
    float* d = functionCoeffs + 9;

    // Note that we use Horner's rule for computing polynomials (which is the way
    // we nest the multiplies and adds to minimize the computation we need.)
    out[ 0 ] = ( ( ( a[ 0 ] ) * u + b[ 0 ] ) * u + c[ 0 ] ) * u + d[ 0 ];
    out[ 1 ] = ( ( ( a[ 1 ] ) * u + b[ 1 ] ) * u + c[ 1 ] ) * u + d[ 1 ];
    out[ 2 ] = ( ( ( a[ 2 ] ) * u + b[ 2 ] ) * u + c[ 2 ] ) * u + d[ 2 ];
}
/**** LISTING 2 (END) ****/

/**** LISTING 3 (BEGIN) ****/
void UniformCurveTessellator::tessellate( const std::vector< CurvePoint >& controls, 
                                     const std::vector< BasisFunction >& bases ) const
{
  // We break the curve into 100 even increments of u.
  int numSteps = 100;

  // We can multiply by this in our loop instead of dividing by (numSteps-1) every time.
  double invTotalSteps = 1.0f / (numSteps - 1);

  // Start drawing our curve.
  ::glBegin( GL_LINE_STRIP );

  for ( int step = 0; step < numSteps; step++ )
  {
    // Generate the parameter for this step of the curve.
    float u = step * invTotalSteps;

    // This holds the point we're working on as we add control points' contributions to it.
    float curPt[ 3 ] = { 0, 0, 0 };

    // Generate a point on the curve for this step.
    for ( int pt = 0; pt <= 3 ; pt++ )
    {
      // Get the value of this basis function at the current parameter value.
      float basisVal = bases[ pt ]( u );

      // Add this control point's contribution onto the current point.
      curPt[ 0 ] += controls[ pt ].getX() * basisVal;
      curPt[ 1 ] += controls[ pt ].getY() * basisVal;
      curPt[ 2 ] += controls[ pt ].getZ() * basisVal;
    }

    // Draw this point.
    ::glVertex3fv( curPt );
  }

  ::glEnd();
}
/**** LISTING 3 (END) ****/

/**** LISTING 4 (BEGIN) ****/
void UniformPatchTessellator::tessellate( const std::vector< std::vector< CurvePoint > >& controls, 
                                     const std::vector< BasisFunction >& bases ) const
{
  // We break the patch into a numSteps x numSteps array of points.
  int numSteps = 10;

  // First, we need to make the basis functions for our normals, which just involves
  // taking the derivative of each basis function.
  std::vector< BasisFunction > bases_deriv( bases );
  for ( int i = 0; i <= 3; i++ )
  {
    bases_deriv[ i ].differentiate();
  }

  // Now we generate the points and normals.
  float* verts = new float[ 3 * numSteps * numSteps ];
  float* norms = new float[ 3 * numSteps * numSteps ];

  double invTotalSteps = 1.0f / (numSteps - 1);

  for ( int stepU = 0; stepU < numSteps; stepU++ )
  {
    // Generate the parameter for this step of the curve.
    float u = stepU * invTotalSteps;

    for ( int stepV = 0; stepV < numSteps; stepV++ )
    {
      // Generate the parameter for this step of the curve.
      float v = stepV * invTotalSteps;

      // This holds the point we're working on as we add control points' contributions to it.
      float curPt[ 3 ] = { 0, 0, 0 };
      float curNorm[ 3 ] = { 0, 0, 0 };

      float curUTan[ 3 ] = { 0, 0, 0 };
      float curVTan[ 3 ] = { 0, 0, 0 };

      // Generate a point on the curve for this step.
      for ( i = 0; i <= 3; i++ )
      {
        for ( j = 0; j <= 3; j++ )
        {
          // Get a few basis function values and products thereof that we'll need.
          float bu = bases[ i ]( u );
          float bv = bases[ j ]( v );
          float dbu = bases_deriv[ i ]( u );
          float dbv = bases_deriv[ j ]( v );
          float bu_bv = bu * bv;
          float bu_dbv = bu * dbv;
          float dbu_bv = dbu * bv;

          // Add this control point's contribution onto the current point.
          curPt[ 0 ] += controls[ i ][ j ].getX() * bu_bv;
          curPt[ 1 ] += controls[ i ][ j ].getY() * bu_bv;
          curPt[ 2 ] += controls[ i ][ j ].getZ() * bu_bv;

          // Add this point's contribution to our u-tangent.
          curUTan[ 0 ] += controls[ i ][ j ].getX() * dbu_bv;
          curUTan[ 1 ] += controls[ i ][ j ].getY() * dbu_bv;
          curUTan[ 2 ] += controls[ i ][ j ].getZ() * dbu_bv;

          // Add this point's contribution to our v-tangent.
          curVTan[ 0 ] += controls[ i ][ j ].getX() * bu_dbv;
          curVTan[ 1 ] += controls[ i ][ j ].getY() * bu_dbv;
          curVTan[ 2 ] += controls[ i ][ j ].getZ() * bu_dbv;
        }
      }

      // Now get our normal as the cross-product of the u and v tangents.
      curNorm[ 0 ] = curUTan[ 1 ] * curVTan[ 2 ] - curUTan[ 2 ] * curVTan[ 1 ];
      curNorm[ 1 ] = curUTan[ 2 ] * curVTan[ 0 ] - curUTan[ 0 ] * curVTan[ 2 ];
      curNorm[ 2 ] = curUTan[ 0 ] * curVTan[ 1 ] - curUTan[ 1 ] * curVTan[ 0 ];

      // Normalize our normal (ouch!)
      float rInv = 1.0f / sqrt( curNorm[ 0 ] * curNorm[ 0 ] + curNorm[ 1 ] * curNorm[ 1 ] + curNorm[ 2 ] * curNorm[ 2 ] );
      curNorm[ 0 ] *= rInv;
      curNorm[ 1 ] *= rInv;
      curNorm[ 2 ] *= rInv;

      // Store these.
      memcpy( verts + ( stepU + ( stepV * numSteps ) ) * 3, curPt, 3 * sizeof(float) );
      memcpy( norms + ( stepU + ( stepV * numSteps ) ) * 3, curNorm, 3 * sizeof(float) );
    }
  }

  // We render each strip of the surface out as a triangle strip.
  for ( int stepV = 0; stepV < numSteps - 1; stepV++ )
  {
    int y0 = stepV;
    int y1 = stepV + 1;

    ::glBegin( GL_TRIANGLE_STRIP );

    for ( int stepU = 0; stepU < numSteps; stepU++ )
    {
      int x0 = stepU;

      glNormal3fv( norms + ( x0 + ( y0 * numSteps ) ) * 3 );
      glVertex3fv( verts + ( x0 + ( y0 * numSteps ) ) * 3 );

      glNormal3fv( norms + ( x0 + ( y1 * numSteps ) ) * 3 );
      glVertex3fv( verts + ( x0 + ( y1 * numSteps ) ) * 3 );
    }

    ::glEnd();
  }

  // Clean up after ourselves.
  delete[] verts;
  delete[] norms;
}
/**** LISTING 4 (END) ****/


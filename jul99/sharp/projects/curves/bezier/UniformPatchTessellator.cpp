#include <curves/bezier/UniformPatchTessellator.h>
#include <curves/bezier/BezierBasis.h>
#include <curves/Polynomial.h>
#include <curves/OpenGL.h>
#include <gl/glut.h>

#include <iostream>

#include <math.h>

// Stupid STL warning (more than 255 characters of garbage!  Truncated!)
#pragma warning (disable: 4786)

UniformPatchTessellator::UniformPatchTessellator() 
: BezierPatchTessellator()
{
  verts = 0;
  norms = 0;
}

UniformPatchTessellator::UniformPatchTessellator( const UniformPatchTessellator& source )
: BezierPatchTessellator( source )
{
  verts = 0;
  norms = 0;
}

UniformPatchTessellator& UniformPatchTessellator::operator=( const UniformPatchTessellator& source )
{
  BezierPatchTessellator::operator=( source );
  verts = 0;
  norms = 0;

  return *this;
}

UniformPatchTessellator::~UniformPatchTessellator()
{
  delete[] verts;
  delete[] norms;
}

void UniformPatchTessellator::tessellate( const std::vector< std::vector< CurvePoint > >& controls, 
                                     const std::vector< BasisFunction >& basesU, 
                                     const std::vector< BasisFunction >& basesV ) const
{
  // Quick check: don't try to draw degenerate patches.
  if ( controls.size() == 0 )
  {
    return;
  }

  // This is just a tweaked scalar that works well.  So, if the granularity is 1.0, we do
  // a 10x10 tessellation, increasing to 11x11 if it's 1.1, etc.
  int numSteps = 10.0f * getGranularity();

  // Make points and normals given the points and the bases and the number of steps.
  generatePoints( numSteps, controls, basesU, basesV );
}

void UniformPatchTessellator::draw(bool multitexture) const
{
  // We don't re-tessellate here, just draw what we've got.
  int numSteps = 10.0f * getGranularity();

  // Okay, draw each vertex.
  ::glPolygonMode( GL_FRONT_AND_BACK, getMode() );

  float invTotalSteps = 1.0f / numSteps;

  for ( int stepV = 0; stepV < numSteps - 1; stepV++ )
  {
    // Generate the parameter for this step of the curve.
    float v0 = stepV * invTotalSteps;
    float v1 = (stepV+1) * invTotalSteps;

    int y0 = stepV;
    int y1 = stepV + 1;

    ::glBegin( GL_TRIANGLE_STRIP );

    for ( int stepU = 0; stepU < numSteps; stepU++ )
    {
      // Generate the parameter for this step of the curve.
      float u = stepU * invTotalSteps;

      int x0 = stepU;

      if (multitexture && OpenGL::getNumMultiTextures() > 1)
      {
        OpenGL::glMultiTexCoord2fARB( GL_TEXTURE0_ARB, u, v0 );
        OpenGL::glMultiTexCoord2fARB( GL_TEXTURE1_ARB, u, v0 );
        glNormal3fv( norms + ( x0 + ( y0 * numSteps ) ) * 3 );
        glVertex3fv( verts + ( x0 + ( y0 * numSteps ) ) * 3 );

        OpenGL::glMultiTexCoord2fARB( GL_TEXTURE0_ARB, u, v1 );
        OpenGL::glMultiTexCoord2fARB( GL_TEXTURE1_ARB, u, v1 );
        glNormal3fv( norms + ( x0 + ( y1 * numSteps ) ) * 3 );
        glVertex3fv( verts + ( x0 + ( y1 * numSteps ) ) * 3 );
      }
      else
      {
        glTexCoord2f( u, v0 );
        glNormal3fv( norms + ( x0 + ( y0 * numSteps ) ) * 3 );
        glVertex3fv( verts + ( x0 + ( y0 * numSteps ) ) * 3 );

        glTexCoord2f( u, v1 );
        glNormal3fv( norms + ( x0 + ( y1 * numSteps ) ) * 3 );
        glVertex3fv( verts + ( x0 + ( y1 * numSteps ) ) * 3 );
      }
    }

    ::glEnd();
  }

//  ::glBegin( GL_LINES );
//
//  // Now draw each normal.
//  for ( stepU = 0; stepU < numSteps; stepU++ )
//  {
//    for ( int stepV = 0; stepV < numSteps; stepV++ )
//    {
//      float* vert = verts + ( stepU + ( stepV * numSteps ) ) * 3;
//      float* norm = norms + ( stepU + ( stepV * numSteps ) ) * 3;
//
//      glVertex3fv( vert );
//      glVertex3f( norm[ 0 ] + vert[ 0 ], norm[ 1 ] + vert[ 1 ], norm[ 2 ] + vert[ 2 ] );
//    }
//  }
//
//  ::glEnd();
}

int UniformPatchTessellator::generateLightmap( int size,
                                               const std::vector< std::vector< CurvePoint > >& controls,
                                               const std::vector< BasisFunction >& basesU,
                                               const std::vector< BasisFunction >& basesV ) const
{
  // We want a huge projection so that it won't clip anything, and we want
  // it orthographic to save on transformation cost.
  ::glMatrixMode( GL_PROJECTION );
  ::glLoadIdentity();
  ::glOrtho( -10000, 10000, -10000, 10000, 1, 20000 );

  ::glMatrixMode( GL_MODELVIEW );
  ::glLoadIdentity();
  ::gluLookAt( 0,0,10000, 0,0,0, 1,0,0 );

  // Position the light.
  ::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  ::glEnable(GL_LIGHTING);
  float position[] = {20.0f,0.0f,5.0f, 0.0f};
  ::glLightfv(GL_LIGHT0, GL_POSITION, position);
  
  generatePoints( size, controls, basesU, basesV );

  float* feedBuffer = new float[ size * size * 8 ];

  ::glFeedbackBuffer( size * size * 8, GL_3D_COLOR, feedBuffer );
  ::glRenderMode( GL_FEEDBACK );

  ::glColor3f( 1, 1, 1 );
  ::glBegin( GL_POINTS );
  for ( int v = 0; v < size; v++ )
  {
    for ( int u = 0; u < size; u++ )
    {
      ::glNormal3fv( norms + (u + (v*size))*3 );
      ::glVertex3fv( verts + (u + (v*size))*3 );
    }
  }
  ::glEnd();

  int count = ::glRenderMode( GL_RENDER );

  int texPos = 0;
  float* texData = new float[ 3 * size * size ];

  for( int x = 0; x < count; x++ )
  {
    if ( feedBuffer[ x ] == GL_POINT_TOKEN )
    {
      texData[ texPos + 0 ] = feedBuffer[ x + 4 ];
      texData[ texPos + 1 ] = feedBuffer[ x + 5 ];
      texData[ texPos + 2 ] = feedBuffer[ x + 6 ];

      texPos += 3;
      x += 7;
    }
    else
    {
      std::cout << "ERROR parsing feedback buffer array." << std::endl;
      delete[] texData;
      return 0;
    }
  }
  
  unsigned int texNum;
  glGenTextures( 1, &texNum ); 
  glBindTexture( GL_TEXTURE_2D, texNum );
  
  // Set the tiling mode.  We don't want lightmaps to repeat, or we get odd borders.
  // If we can't clamp to edge, hope clamping does the job.  Otherwise, driver problem.
  if (OpenGL::getSupportsClampToEdge())
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL::GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL::GL_CLAMP_TO_EDGE);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  }
  
  // Set the filtering.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

  gluBuild2DMipmaps( GL_TEXTURE_2D, 3, size, size, GL_RGB, GL_FLOAT, texData );

  // Uncomment this to write the lightmap our to a raw file.
//  char* imgData = new char[ size * size * 3 ];
//  for ( x = 0; x < size * size * 3; x++ )
//  {
//    imgData[ x ] = (texData[ x ] * 255.0f);
//  }
//
//  FILE* outfile = fopen( "lightmap.raw", "wb" );
//  fwrite( imgData, 1, size * size * 3, outfile );
//  fclose( outfile );
//  delete[] imgData;

  delete[] texData;

  return texNum;
}

void UniformPatchTessellator::generatePoints( int numSteps, 
                                              const std::vector< std::vector< CurvePoint > >& controls,
                                              const std::vector< BasisFunction >& basesU,
                                              const std::vector< BasisFunction >& basesV ) const
{
  // First, we need to make the basis functions for our normals, which just involves
  // taking the derivative of each basis function.
  std::vector< BasisFunction > basesUdu( basesU );
  std::vector< BasisFunction > basesVdv( basesV );

  for ( int i = 0; i < basesUdu.size(); i++ )
  {
    basesUdu[ i ].differentiate();
  }
  for ( int j = 0; j < basesVdv.size(); j++ )
  {
    basesVdv[ j ].differentiate();
  }

  // Now we generate the points and normals.
  delete[] verts;
  delete[] norms;
  verts = new float[ 3 * numSteps * numSteps ];
  norms = new float[ 3 * numSteps * numSteps ];

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
      for ( i = 0; i < controls.size(); i++ )
      {
        for ( j = 0; j < controls[ 0 ].size(); j++ )
        {
          // Get a few basis function values and products thereof that we'll need.
          float bu = basesU[ i ]( u );
          float bv = basesV[ j ]( v );
          float dbu = basesUdu[ i ]( u );
          float dbv = basesVdv[ j ]( v );
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
}
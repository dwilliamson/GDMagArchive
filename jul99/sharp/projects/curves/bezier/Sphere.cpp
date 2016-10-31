#include <curves/bezier/Sphere.h>
#include <curves/OpenGL.h>
#include <math.h>

#define SPHERE_RADIUS 0.3f
#ifndef PI
#define PI 3.141593
#endif

#pragma warning (disable: 4305)

#define SETMAGNITUDE(x, mag) \
    rinv = mag/sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);\
    x[0] *= rinv; \
    x[1] *= rinv; \
    x[2] *= rinv;

Sphere::Sphere(int numPolys, unsigned int texName)
{
  mTris = NULL;
  mVertexData = NULL;
//  mNormalData = NULL;
  mTexCoordData = NULL;
  mNumVerts = 0;
  mNumTris = 0;

  setNumPolys(numPolys);
  initResolution();

  textureName = texName;
}

void Sphere::draw(unsigned int lightmap, float u, float v)
{
  // If we're drawing both of them, do it in one pass if we have multitexturing.
  if (OpenGL::getNumMultiTextures() > 1)
  {
    glDisable(GL_BLEND);

    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, lightmap);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture(GL_TEXTURE_2D, textureName);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Tell the patch to draw.  Tell it to multitexture.
    drawSelf(true, u, v);
  }

  // Make sure we don't leave the second unit on, as that can elicit a bug in nVidia's driver.
  if (OpenGL::getNumMultiTextures() > 1)
  {
    OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
  }

  // Now, if we can't multitexture, or we only want to draw one of the passes, do that here.
  if (OpenGL::getNumMultiTextures() <= 1)
  {
    // Turn on texturing and load in the lightmap and draw the surface.
    glEnable(GL_TEXTURE_2D);

    // Draw us in two passes.  First pass (lightmap):
    glBindTexture(GL_TEXTURE_2D, lightmap);
    glDepthFunc(GL_LESS);

    // Tell the patch to draw.
    drawSelf(false, u, v);

    // Now set up the blending and z-buffering for the base texture pass, assuming we're
    // doing two-pass drawing.
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glDepthFunc(GL_EQUAL);

    // Now we're drawing the base texture, either as a second pass or by itself.
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textureName);

    // Again, draw.
    drawSelf(true);

    // Now clean up.
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthFunc(GL_LESS);
  }

  if (OpenGL::getNumMultiTextures() > 1)
  {
    OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
    glDisable(GL_TEXTURE_2D);

    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
  }
//  glDisable(GL_TEXTURE_2D);
//  glColor3f(0,1,0);
//  glBegin(GL_LINES);
//  for (int x=0; x<mNumVerts; x++)
//  {
//    glVertex3f(mVertexData[3*x+0], mVertexData[3*x+1], mVertexData[3*x+2]);
//    glVertex3f(mVertexData[3*x+0]+mNormalData[3*x+0], mVertexData[3*x+1]+mNormalData[3*x+1], mVertexData[3*x+2]+mNormalData[3*x+2]);
//  }
//  glEnd();
}

void Sphere::drawSelf(bool useTexCoords, float u, float v)
{
//  // Okay, draw each vertex.
//  // If we're multitexturing, setup our arrays accordingly.
//  if (OpenGL::getNumMultiTextures() > 1)
//  {
//    OpenGL::glClientActiveTextureARB(GL_TEXTURE0_ARB);
//    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//    OpenGL::glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
//
//    OpenGL::glClientActiveTextureARB(GL_TEXTURE1_ARB);
//    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//    glTexCoordPointer(2, GL_FLOAT, 0, mTexCoordData);
//  }
//  else
//  {
//    if (useTexCoords)
//    {
//      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//      glTexCoordPointer(2, GL_FLOAT, 0, mTexCoordData);
//    }
//    else
//    {
//      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//      glTexCoord2f(u,v);
//    }
//  }
//  glEnableClientState(GL_VERTEX_ARRAY);
//  glVertexPointer(3, GL_FLOAT, 0, mVertexData);
//
//  // If it supports compiled vertex arrays, by all means, use them.
//  if (OpenGL::getSupportsCompiledVertexArrays())
//  {
//    OpenGL::glLockArraysEXT(0,mNumVerts);
//  }
//
//  // Draw us.
//  glDrawElements(GL_TRIANGLES, mNumTris*3, GL_UNSIGNED_INT, mTris);
//
//  // Clean up after ourselves.
//  if (OpenGL::getSupportsCompiledVertexArrays())
//  {
//    OpenGL::glUnlockArraysEXT();
//  }
  // Okay, draw each vertex.
  // If we're multitexturing, setup our arrays accordingly.
  if (OpenGL::getNumMultiTextures() > 1)
  {
    glBegin(GL_TRIANGLES);
    for (int x=0; x<mNumTris; x++)
    {
      int i0 = mTris[x*3+0];
      int i1 = mTris[x*3+1];
      int i2 = mTris[x*3+2];
      OpenGL::glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
      OpenGL::glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexCoordData[2*i0+0], mTexCoordData[2*i0+1]);
      glVertex3fv(mVertexData + i0*3);
      OpenGL::glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
      OpenGL::glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexCoordData[2*i1+0], mTexCoordData[2*i1+1]);
      glVertex3fv(mVertexData + i1*3);
      OpenGL::glMultiTexCoord2fARB(GL_TEXTURE0_ARB, u, v);
      OpenGL::glMultiTexCoord2fARB(GL_TEXTURE1_ARB, mTexCoordData[2*i2+0], mTexCoordData[2*i2+1]);
      glVertex3fv(mVertexData + i2*3);
    }
    glEnd();
  }
  else
  {
    if (useTexCoords)
    {
      glBegin(GL_TRIANGLES);
      for (int x=0; x<mNumTris; x++)
      {
        int i0 = mTris[x*3+0];
        int i1 = mTris[x*3+1];
        int i2 = mTris[x*3+2];
        glTexCoord2f(mTexCoordData[2*i0+0], mTexCoordData[2*i0+1]);
        glVertex3fv(mVertexData + i0*3);
        glTexCoord2f(mTexCoordData[2*i1+0], mTexCoordData[2*i1+1]);
        glVertex3fv(mVertexData + i1*3);
        glTexCoord2f(mTexCoordData[2*i2+0], mTexCoordData[2*i2+1]);
        glVertex3fv(mVertexData + i2*3);
      }
      glEnd();
    }
    else
    {
      glBegin(GL_TRIANGLES);
      for (int x=0; x<mNumTris; x++)
      {
        int i0 = mTris[x*3+0];
        int i1 = mTris[x*3+1];
        int i2 = mTris[x*3+2];
        glTexCoord2f(u,v);
        glVertex3fv(mVertexData + i0*3);
        glVertex3fv(mVertexData + i1*3);
        glVertex3fv(mVertexData + i2*3);
      }
      glEnd();
    }
  }
}

int Sphere::addVertex(float vx, float vy, float vz)
{
  float rinv = SPHERE_RADIUS/sqrt(vx*vx + vy*vy + vz*vz);    // Set it so the sphere has the right radius
  vx *= rinv;
  vy *= rinv;
  vz *= rinv;
  
  for (int x=0; x<mNumVerts; x++)
  {
    if ((mVertexData[3*x+0] == vx) &&
        (mVertexData[3*x+1] == vy) &&
        (mVertexData[3*x+2] == vz))
    {
      return x;
    }
  }
  
  mVertexData[(3*mNumVerts)+0] = vx;
  mVertexData[(3*mNumVerts)+1] = vy;
  mVertexData[(3*mNumVerts)+2] = vz;
  
  vx /= SPHERE_RADIUS;
  vy /= SPHERE_RADIUS;
  vz /= SPHERE_RADIUS;

//  mNormalData[(3*mNumVerts)+0] = vx;
//  mNormalData[(3*mNumVerts)+1] = vy;
//  mNormalData[(3*mNumVerts)+2] = vz;
  
  float u = (atan2(vx, -vy) + PI) / (2*PI);
  float v = (asin(vz) + (PI/2)) / (PI);
  
  mTexCoordData[(2*mNumVerts)+0] = u;
  mTexCoordData[(2*mNumVerts)+1] = v;
  
  unsigned int returnVal = mNumVerts;
  mNumVerts++;
  return returnVal; // Return the current value, then increment it.
}

void Sphere::addTri(int* indices)
{
  mTris[mNumTris*3+0] = indices[0];
  mTris[mNumTris*3+1] = indices[1];
  mTris[mNumTris*3+2] = indices[2];
  
  mNumTris++;
}

void Sphere::subdivide(float* v1, float* v2, float* v3, long depth)
{
  GLfloat v12[3], v23[3], v31[3];
  GLint i;
  float rinv;
  
  static int triIndices[3];
  if (depth == 0)
  {
    triIndices[2] = addVertex(v1[0], v1[1], v1[2]);
    triIndices[1] = addVertex(v2[0], v2[1], v2[2]);
    triIndices[0] = addVertex(v3[0], v3[1], v3[2]);
    
    if ((triIndices[0] != -1) && (triIndices[1] != -1) && (triIndices[2] != -1))
    {
      addTri(triIndices);
    }
    
    return;
  }
  
  for (i=0; i<3; i++)
  {
    v12[i] = v1[i] + v2[i];
    v23[i] = v2[i] + v3[i];
    v31[i] = v3[i] + v1[i];
  }
  
  SETMAGNITUDE(v12, 1);
  SETMAGNITUDE(v23, 1);
  SETMAGNITUDE(v31, 1);
  
  subdivide( v1, v12, v31, depth-1);
  subdivide( v2, v23, v12, depth-1);
  subdivide( v3, v31, v23, depth-1);
  subdivide(v12, v23, v31, depth-1);
}

void Sphere::setNumPolys(int np) 
{
  numPolys = np;
}

void Sphere::initResolution() 
{
#define X .525731112119133606
#define Z .850650808352039932
  
  int depth;
  if (numPolys >= 1280)
  {
    depth = 3;
    reserveVerts(642);
    reserveTris(1280);
  }
  else if (numPolys >= 320)
  {
    depth = 2;
    reserveVerts(162);
    reserveTris(320);
  }
  else if (numPolys > 80)
  {
    depth = 1;
    reserveVerts(42);
    reserveTris(80);
  }
  else
  {
    depth = 0;
    reserveVerts(12);
    reserveTris(20);
  }
  
  GLfloat vdata[12][3] = {
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
  };
  
  GLint triIndices[20][3] = {
    {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},
    {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3}, 
    {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
    {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11}, 
  };
  
  for (int i=0; i<20; i++)
  {
    subdivide(&vdata[triIndices[i][0]][0], &vdata[triIndices[i][1]][0], &vdata[triIndices[i][2]][0], depth);
  }
}

void Sphere::reserveVerts(int numVerts)
{
  delete[] mVertexData;
  delete[] mTexCoordData;
//  delete[] mNormalData;
  mNumVerts = 0;
  mVertexData = new float[numVerts * 3];
//  mNormalData = new float[numVerts * 3];
  mTexCoordData = new float[numVerts * 2];
}

void Sphere::reserveTris(int numTris)
{
  delete[] mTris;
  mNumTris = 0;
  mTris = new int[numTris * 3];
}

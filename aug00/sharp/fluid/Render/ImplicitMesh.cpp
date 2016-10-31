#include <fluid/Render/ImplicitMesh.h>
#include <harness/OpenGL.h>
#include <gl/glut.h>
#include <harness/GlobalCamera.h>
#include <math/Math.h>
#include <assert.h>

ImplicitMesh::ImplicitMesh() : vertices(), normals(), indices()
{
  useNormals = true;
  useColors = true;
  useTexCoords = true;
}

void ImplicitMesh::render() const
{
  // Hand it all off to OpenGL.  Yeah, I'm mildly abusing the STL by assuming vector stores everything
  // as a contiguous array... but it's a pain otherwise.
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 16, &(vertices[0]));

  if (useNormals)
  {
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 16, &(normals[0]));
  }
  else
  {
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  if (useColors)
  {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, 4, &(colors[0]));
  }
  else
  {
    glDisableClientState(GL_COLOR_ARRAY);
  }

  if (useTexCoords)
  {
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 8, &(texCoords[0]));
  }
  else
  {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  if (OpenGL::getSupportsCompiledVertexArrays())
  {
    OpenGL::glLockArraysEXT(0,vertices.size()/4);
  }

//  glEnable(GL_BLEND);
//  glDepthMask(GL_FALSE);
//  glBlendFunc(GL_DST_COLOR, GL_ZERO);
//  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &(indices[0]));
 
//  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//  glBlendFunc(GL_ONE, GL_ONE);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &(indices[0]));
//  glDepthMask(GL_TRUE);
//  glDisable(GL_BLEND);

  if (OpenGL::getSupportsCompiledVertexArrays())
  {
    OpenGL::glUnlockArraysEXT();
  }
}

void ImplicitMesh::renderNormals() const
{
  glPushAttrib(GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_LIGHTING);

  glColor3f(1,0,1);
  glBegin(GL_LINES);
  for (int x=0; x<indices.size(); x++)
  {
    glVertex3fv(&vertices[4*indices[x]]);
    glVertex3f(vertices[4*indices[x]]+normals[4*indices[x]], vertices[4*indices[x]+1]+normals[4*indices[x]+1], vertices[4*indices[x]+2]+normals[4*indices[x]+2]);
  }
  glEnd();
  
  glPopAttrib();
}

void ImplicitMesh::clear()
{
  vertices.clear();
  normals.clear();
  indices.clear();
  texCoords.clear();
  colors.clear();
}

// Used to tell this guy which arrays to pass.  I assume vertex arrays should always
// be enabled. :-P
void ImplicitMesh::setUseNormalArrays(bool val) 
{ 
  useNormals = val; 
}

void ImplicitMesh::setUseColorArrays(bool val)
{
  useColors = val;
}

void ImplicitMesh::setUseTexCoordArrays(bool val)
{
  useTexCoords = val;
}

bool ImplicitMesh::getUseNormalArrays() const
{ 
  return useNormals; 
}

int ImplicitMesh::addVertex(const Vector& newVert)
{
  vertices.push_back(newVert.x);
  vertices.push_back(newVert.y);
  vertices.push_back(newVert.z);
  vertices.push_back(0);

  return (vertices.size() / 4) - 1;
}

int ImplicitMesh::addVertex(float* newVert)
{
  vertices.push_back(newVert[0]);
  vertices.push_back(newVert[1]);
  vertices.push_back(newVert[2]);
  vertices.push_back(0);

  return (vertices.size() / 4) - 1;
}

int ImplicitMesh::addVertex(float newX, float newY, float newZ)
{
  vertices.push_back(newX);
  vertices.push_back(newY);
  vertices.push_back(newZ);
  vertices.push_back(0);

  return (vertices.size() / 4) - 1;
}

// Adds a normal and returns its index.  If you don't always add a normal when you
// add a vertex everything's going to get out of sync.
int ImplicitMesh::addNormal(const Vector& newNormal)
{
  normals.push_back(newNormal.x);
  normals.push_back(newNormal.y);
  normals.push_back(newNormal.z);
  normals.push_back(0);

  return (normals.size() / 4) - 1;
}

int ImplicitMesh::addNormal(float* newNormal)
{
  normals.push_back(newNormal[0]);
  normals.push_back(newNormal[1]);
  normals.push_back(newNormal[2]);
  normals.push_back(0);

  return (normals.size() / 4) - 1;
}

int ImplicitMesh::addNormal(float newX, float newY, float newZ)
{
  normals.push_back(newX);
  normals.push_back(newY);
  normals.push_back(newZ);
  normals.push_back(0);

  return (normals.size() / 4) - 1;
}

// Adds a color.  Same restrictions as for adding everything else (i.e. add one of everything
// every time or things get out of sync.)
int ImplicitMesh::addColor(unsigned char* color)
{
  colors.push_back(color[0]);
  colors.push_back(color[1]);
  colors.push_back(color[2]);
  colors.push_back(color[3]);

  return (colors.size() / 4) - 1;
}

// Adds a texcoord pair.  Same restrictions as for adding everything else (i.e. add one of everything
// every time or things get out of sync.)
int ImplicitMesh::addTexCoords(float* tc)
{
  texCoords.push_back(tc[0]);
  texCoords.push_back(tc[1]);

  return (texCoords.size() / 2) - 1;
}

void ImplicitMesh::addTri(unsigned int index0, unsigned int index1, unsigned int index2)
{
  indices.push_back(index0);
  indices.push_back(index1);
  indices.push_back(index2);
}

// This causes this guy to compute his normals (a simple geometric normal calculation) and then
// to subsequently generate texcoords from sphere-mapping and/or lighting colors.
void ImplicitMesh::generateNormals()
{
  normals.resize(vertices.size());
  for (int x=0; x<normals.size(); x++)
  {
    normals[x] = 0;
  }

  for (x=0; x<indices.size(); x+=3)
  {
    int ind0 = indices[x+0];
    int ind1 = indices[x+1];
    int ind2 = indices[x+2];

    Vector v0(vertices[4*ind0+0], vertices[4*ind0+1], vertices[4*ind0+2]);
    Vector v1(vertices[4*ind1+0], vertices[4*ind1+1], vertices[4*ind1+2]);
    Vector v2(vertices[4*ind2+0], vertices[4*ind2+1], vertices[4*ind2+2]);

    v1 -= v0;
    v2 -= v0;
    v1.cross(v2,v0);

    v0.fastNormalize();

    normals[4*ind0+0] += v0.x;
    normals[4*ind0+1] += v0.y;
    normals[4*ind0+2] += v0.z;
    
    normals[4*ind1+0] += v0.x;
    normals[4*ind1+1] += v0.y;
    normals[4*ind1+2] += v0.z;
    
    normals[4*ind2+0] += v0.x;
    normals[4*ind2+1] += v0.y;
    normals[4*ind2+2] += v0.z;
  }

  for (x=0; x<normals.size(); x+=4)
  {
    Vector normal(normals[x+0], normals[x+1], normals[x+2]);
    normal.fastNormalize();
    normals[x+0] = normal.x;
    normals[x+1] = normal.y;
    normals[x+2] = normal.z;
  }
}

void ImplicitMesh::doLight()
{
  if (getUseColorArrays())
  {
    colors.resize(vertices.size());

    // Light position and color (yeah, yeah, hardcoded...)
    const Vector lightDir(-0.099f,-0.099f,-0.99f);
    float diffuse [] = {0.7f, 0.6f, 1.0f, 1.0f};
    const float lightColor[3] = {100.0f, 120.0f, 255.0f};
//    const float lightColor[3] = {255.0f, 255.0f, 255.0f};
    const unsigned char ambient[3] = {50, 50, 50};
  
    for (int x=0; x<colors.size(); x+=4)
    {
      Vector normal(normals[x+0], normals[x+1], normals[x+2]);

      float dotProd = -(lightDir.dot(normal));
      colors[x+0] = ambient[0];
      colors[x+1] = ambient[1];
      colors[x+2] = ambient[2];
      colors[x+3] = 192;
      if (dotProd > 0)
      {
        colors[x+0] += Math::minOf(255-ambient[0], (int)(lightColor[0]*dotProd));
        colors[x+1] += Math::minOf(255-ambient[1], (int)(lightColor[1]*dotProd));
        colors[x+2] += Math::minOf(255-ambient[2], (int)(lightColor[2]*dotProd));
      }
    }
  }
}

void ImplicitMesh::doSphereMap()
{
  if (getUseTexCoordArrays())
  {
    texCoords.resize(vertices.size()/2);
    const Position& camPos = GlobalCamera::Instance()->getPosition();
    
    for (int x=0; x<texCoords.size(); x+=2)
    {
      Vector cameraLoc(-camPos);

      cameraLoc.x += vertices[2*x + 0];
      cameraLoc.y += vertices[2*x + 1];
      cameraLoc.z += vertices[2*x + 2];
      
      cameraLoc.fastNormalize();

      // Got our camera vector, we know the normal is normalized, get the reflection vector and then do
      // the crezzy sphere_map thang.
      Vector normal(normals[2*x+0], normals[2*x+1], normals[2*x+2]);
      float normalDotCamera = normal.dot(cameraLoc);
      Vector reflection = cameraLoc;
      reflection.x -= 2 * normal.x * normalDotCamera;
      reflection.y -= 2 * normal.y * normalDotCamera;
      reflection.z -= 2 * normal.z * normalDotCamera;

      // Rotate around x to get our sphere map world-aligned just right.
      float holdZ = reflection.z;
      reflection.z = -reflection.y;
      reflection.y = holdZ;

      assert(fabs(reflection.getMagnitudeSquared()-1.0f) < 0.1);

      if (reflection.z > -1)
      {
        float invM = 0.5f*Math::fastInverseSqrt(2*reflection.z + 2);
        texCoords[x+0] = 0.5f + reflection.x * invM;
        texCoords[x+1] = 0.5f + reflection.y * invM;
      }
      else
      {
        // Put them  somewhere remotely intelligible.
        texCoords[x+0] = 0.5f;
        texCoords[x+1] = 0.5f;
      }
    }
  }
}


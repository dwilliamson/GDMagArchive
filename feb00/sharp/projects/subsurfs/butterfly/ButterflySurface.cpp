// Stupid STL name-mangling length warning.  Why, God, why?  They don't generate warnings
// when you mask class data with function-local data, leaving you to ponder vague compiler
// errors for hours, but they warn you about the name-mangling?!?!  MSVC: AUGH!!!
//
// *ahem*
#pragma warning (disable: 4786)

#include <subsurfs/butterfly/ButterflySurface.h>
#include <gl/glut.h>
#include <harness/OpenGL.h>
#include <assert.h>
#include <math.h>
#include <map>
#include <harness/PlatformSpecific.h>
#include <harness/GlobalCamera.h>
#include <math/Vector.h>
#include <iostream>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef PI
#define PI 3.14159265359
#endif

bool ButterflyEdge::operator==(const ButterflyEdge& cmp) const
{
  if (v[0] == cmp.v[0] && v[1] == cmp.v[1])
  {
    return true;
  }
  return false;
}

bool ButterflyEdge::operator<(const ButterflyEdge& cmp) const
{
  // Since they're sorted, say it's less if the components are less.
  if (v[0] < cmp.v[0])
  {
    return true;
  }
  if ((v[0] == cmp.v[0]) && (v[1] < cmp.v[1]))
  {
    return true;
  }

  return false;
}

VertexEdges::VertexEdges() { numEdges = 0; }
VertexEdges::VertexEdges(const VertexEdges& source)
{
  numEdges = source.numEdges;
  for (int x=0; x<numEdges; x++)
  {
    edges[x] = source.edges[x];
  }
}

VertexEdges& VertexEdges::operator=(const VertexEdges& source)
{
  numEdges = source.numEdges;
  for (int x=0; x<numEdges; x++)
  {
    edges[x] = source.edges[x];
  }

  return *this;
}

ButterflySurface::ButterflySurface()
{
  numControlVerts = 0;
  controlVerts = 0;
  controlTexCoords = 0;
  controlColors = 0;

  numControlFaces = 0;
  controlFaces = 0;
  controlFaceEdges = 0;

  numControlEdges = 0;
  controlEdges = 0;
  
  numVerts = 0;
  vertCapacity = 0;
  verts = 0;
  texCoords = 0;
  envTexCoords = 0;
  colors = 0;

  numFaces = 0;
  faceCapacity = 0;
  faces = 0;
  faceEdges = 0;

  numEdges = 0;
  edgeCapacity = 0;
  edges = 0;

  // Set these to invalid until told otherwise.
  baseTex = -1;
  glossTex = -1;
  envTex = -1;

  drawBaseTex = false;
  drawEnvTex = false;
  drawGlossTex = false;

  curveThreshold = 0;

  initializeBlendingWeights();
}

// This copies the information from the specified net into this surface.
// You can query to get the vertices back to move them around, and you're
// guaranteed that they'll be in the same order that they are in the argument
// control net.
void ButterflySurface::setControlNet(const ControlNet& net)
{
  // Clean us out.
  clear();

  // Copy their textures.
  setBaseTexture(net.baseTex);
  setGlossMap(net.glossTex);
  setEnvironmentMap(net.envTex);

  // Seed the random generator for now since we're randomly picking colors.
  srand(timeGetTime());

  // First just copy stuff over.
  numControlVerts = net.numVerts;
  controlVerts = new float[4*numControlVerts];
  controlVertEdges = new VertexEdges[numControlVerts];
  controlTexCoords = new float[2*numControlVerts];
  controlColors = new unsigned char[4*numControlVerts];

  int x,y;
  for (x=0; x<numControlVerts; x++)
  {
    controlVerts[4*x+0] = net.verts[4*x+0];
    controlVerts[4*x+1] = net.verts[4*x+1];
    controlVerts[4*x+2] = net.verts[4*x+2];

    controlTexCoords[2*x+0] = 0;
    controlTexCoords[2*x+1] = 0;
    if (net.texCoords != 0)
    {
      controlTexCoords[2*x+0] = net.texCoords[2*x+0];
      controlTexCoords[2*x+1] = net.texCoords[2*x+1];
    }

    controlColors[4*x+0] = 255.0f * (0.0f + (rand() / (float)RAND_MAX)*1.0f);
    controlColors[4*x+1] = 255.0f * (0.0f + (rand() / (float)RAND_MAX)*1.0f);
    controlColors[4*x+2] = 255.0f * (0.0f + (rand() / (float)RAND_MAX)*1.0f);
    controlColors[4*x+3] = 255.0f * (1);
    if (net.colors != 0)
    {
      controlColors[4*x+0] = 255.0f * (net.colors[4*x+0]);
      controlColors[4*x+1] = 255.0f * (net.colors[4*x+1]);
      controlColors[4*x+2] = 255.0f * (net.colors[4*x+2]);
      controlColors[4*x+3] = 255.0f * (net.colors[4*x+3]);
    }
  }

  // Got all the vertex data copied.  Copy the faces.
  std::vector<int> indices;

  // Triangulate this guy.
  for (x=0; x<net.numFaces; x++)
  {
    for (int y=0; y<net.faces[x].numVerts-2; y++)
    {
      indices.push_back(net.faces[x].verts[0]);
      indices.push_back(net.faces[x].verts[y+1]);
      indices.push_back(net.faces[x].verts[y+2]);
    }
  }

  // Now copy that into our faces.
  numControlFaces = indices.size() / 3;
  controlFaces = new int[3*numControlFaces];
  controlFaceEdges = new int[3*numControlFaces];

  for (x=0; x<indices.size(); x++)
  {
    controlFaces[x] = indices[x];
  }

  // Okay, now for the fun.  Need to generate edge data from all this.
  std::vector<ButterflyEdge> buildEdges;

  for (x=0; x<numControlFaces; x++)
  {
    for (int y=0; y<3; y++)
    {
      int vert0 = controlFaces[3*x+y];
      int vert1 = controlFaces[3*x + ((y+1)%3)];
      ButterflyEdge addEdge;
      addEdge.v[0] = min(vert0, vert1);
      addEdge.v[1] = max(vert0, vert1);

      // We can't just use find() because we wouldn't get the index back.
      bool foundEdge = false;
      for (int z=0; z<buildEdges.size(); z++)
      {
        if (buildEdges[z] == addEdge)
        {
          controlFaceEdges[3*x+y] = z;
          foundEdge = true;
        }
      }
      if (!foundEdge)
      {
        buildEdges.push_back(addEdge);
        controlFaceEdges[3*x+y] = buildEdges.size()-1;
      }
    }
  }

  // Got the edges, copy 'em in.
  numControlEdges = buildEdges.size();
  controlEdges = new ButterflyEdge[numControlEdges];
  for (x=0; x<numControlEdges; x++)
  {
    controlEdges[x] = buildEdges[x];
  }

  // Now we have to genenerate the edge list for the vertices.

  // First generate all the vertex normals.
  std::vector<Vector> vertNormals;
  vertNormals.resize(numControlVerts, Vector(0,0,0));

  for (x=0; x<numControlFaces; x++)
  {
    Vector facetNormal(0,0,0);
    Vector center(controlVerts[4*controlFaces[3*x+0]+0], controlVerts[4*controlFaces[3*x+0]+1], controlVerts[4*controlFaces[3*x+0]+2]);
    Vector v0(controlVerts[4*controlFaces[3*x+1]+0], controlVerts[4*controlFaces[3*x+1]+1], controlVerts[4*controlFaces[3*x+1]+2]);
    Vector v1(controlVerts[4*controlFaces[3*x+2]+0], controlVerts[4*controlFaces[3*x+2]+1], controlVerts[4*controlFaces[3*x+2]+2]);

    v0 -= center;
    v1 -= center;

    v0.cross(v1, facetNormal);
    facetNormal.normalize();

    for (y=0; y<3; y++)
    {
      vertNormals[controlFaces[3*x+y]] += facetNormal;
    }
  }

  // Normalize the vert normals.
  for (x=0; x<numControlVerts; x++)
  {
    vertNormals[x].normalize();
  }

  // Now aggregate a list of the edges going to each vertex.
  std::vector<std::vector<int> > buildVertEdges;
  buildVertEdges.resize(numControlVerts, std::vector<int>());

  for (x=0; x<numControlEdges; x++)
  {
    buildVertEdges[controlEdges[x].v[0]].push_back(x);
    buildVertEdges[controlEdges[x].v[1]].push_back(x);
  }

  // Now the somewhat harder part -- sort them by angle.  Pick an arbitrary one to start
  // with.
  for (x=0; x<numControlVerts; x++)
  {
    assert(buildVertEdges[x].size() >= 3 && buildVertEdges[x].size() <= MAX_VERTEX_VALENCE);

    // We'll use these.  A lot.
    Vector normal = vertNormals[x];
    Vector vertex(controlVerts[4*x+0], controlVerts[4*x+1], controlVerts[4*x+2]);

    // Use this to sort them.
    std::map<float, int> sortedEdges;

    // Just start with the first one, all the others will be stored relative to it, in
    // CCW-wound order.
    int edgeNum = buildVertEdges[x][0];
    sortedEdges[0.0f] = edgeNum;

    int otherVert = controlEdges[edgeNum].v[0] == x ? controlEdges[edgeNum].v[1] : controlEdges[edgeNum].v[0];
    Vector refVector(controlVerts[4*otherVert+0], controlVerts[4*otherVert+1], controlVerts[4*otherVert+2]);
    refVector -= vertex;

    // Project the reference (angle == 0) vector into the tangent plane.
    Vector projVec(normal);
    projVec *= refVector.dot(normal);
    refVector -= projVec;
    refVector.normalize();

    for (y=1; y<buildVertEdges[x].size(); y++)
    {
      edgeNum = buildVertEdges[x][y];

      // Find the vector from the vertex along this edge.
      otherVert = controlEdges[edgeNum].v[0] == x ? controlEdges[edgeNum].v[1] : controlEdges[edgeNum].v[0];
      Vector vertVector(controlVerts[4*otherVert+0], controlVerts[4*otherVert+1], controlVerts[4*otherVert+2]);
      vertVector -= vertex;

      // Find the vector projected into the tangent plane.
      projVec = normal;
      projVec *= vertVector.dot(normal);
      vertVector -= projVec;
      vertVector.normalize();

      // Find the angle between it and the refence vector.  
      // Remember that v1 dot v2 = |v1||v2|cos(theta) = cos(theta) for normalized vectors.
      float cosTheta = vertVector.dot(refVector);
      if (cosTheta < -1.0f) cosTheta = -1.0f;
      if (cosTheta > 1.0f) cosTheta = 1.0f;
      float angle = acos(cosTheta);
      angle *= 180.0f / PI;

      // To find the sign, it's a clockwise (negative) angle if the cross product points 
      // away from the tangent plane normal, so map it properly.
      Vector crossProd;
      vertVector.cross(refVector, crossProd);
      if (crossProd.dot(normal) < 0)
      {
        angle = 360 - angle;
      }

      sortedEdges[angle] = edgeNum;
    }

    // Okay, it's all sorted, put it into the vertex's edge list in the right order.
    std::map<float, int>::iterator it;
    for (it=sortedEdges.begin(); it!=sortedEdges.end(); it++)
    {
      controlVertEdges[x].edges[controlVertEdges[x].numEdges] = it->second;
      controlVertEdges[x].numEdges++;
    }
  }

  // All set!  Just copy the data into our arrays.
  for (x=0; x<numControlVerts; x++)
  {
    addVert();
    verts[4*x+0] = controlVerts[4*x+0];
    verts[4*x+1] = controlVerts[4*x+1];
    verts[4*x+2] = controlVerts[4*x+2];
    vertEdges[x] = controlVertEdges[x];
    texCoords[2*x+0] = controlTexCoords[2*x+0];
    texCoords[2*x+1] = controlTexCoords[2*x+1];
    colors[4*x+0] = controlColors[4*x+0];
    colors[4*x+1] = controlColors[4*x+1];
    colors[4*x+2] = controlColors[4*x+2];
    colors[4*x+3] = controlColors[4*x+3];
  }

  for (x=0; x<numControlFaces; x++)
  {
    addFace();
    faces[3*x+0] = controlFaces[3*x+0];
    faces[3*x+1] = controlFaces[3*x+1];
    faces[3*x+2] = controlFaces[3*x+2];
    faceEdges[3*x+0] = controlFaceEdges[3*x+0];
    faceEdges[3*x+1] = controlFaceEdges[3*x+1];
    faceEdges[3*x+2] = controlFaceEdges[3*x+2];
  }

  for (x=0; x<numControlEdges; x++)
  {
    addEdge();
    edges[x] = controlEdges[x];
  }

  // Done!
}

void ButterflySurface::clear()
{
  numControlVerts = 0;
  delete[] controlVerts;
  delete[] controlTexCoords;
  delete[] controlColors;
  numControlFaces = 0;
  delete[] controlFaces;
  delete[] controlFaceEdges;
  numControlEdges = 0;
  delete[] controlEdges;
  numVerts = 0;
  vertCapacity = 0;
  delete[] verts;
  delete[] texCoords;
  delete[] envTexCoords;
  delete[] colors;
  numFaces = 0;
  faceCapacity = 0;
  delete[] faces;
  delete[] faceEdges;
  numEdges = 0;
  edgeCapacity = 0;
  delete[] edges;
}

float* ButterflySurface::getControlVerts()
{
  return controlVerts;
}

// You need to call this when you've modified the model by changing the data that
// getControlVerts() points to, or else it won't be taken into account.
void ButterflySurface::changedVertices()
{
}

// This'll draw the surface using the GlobalCamera to determine LOD issues.
void ButterflySurface::draw()
{
  tessellate();

  // Okay, now if we have a base texture, turn it on, otherwise just draw it
  // diffuse-lit like normal.
  if (baseTex != -1 && drawBaseTex)
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, baseTex);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
  }
  else
  {
    glDisable(GL_TEXTURE_2D);
  }
//  glEnable(GL_LIGHTING);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  glVertexPointer(3, GL_FLOAT, 16, verts);
  glNormalPointer(GL_FLOAT, 16, vertNorms);
  glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
  if (OpenGL::getSupportsCompiledVertexArrays())
  {
    OpenGL::glLockArraysEXT(0,numVerts);
  }
  glDrawElements(GL_TRIANGLES, 3*numFaces, GL_UNSIGNED_INT, faces);
  if (OpenGL::getSupportsCompiledVertexArrays())
  {
    OpenGL::glUnlockArraysEXT();
  }

  // For now we're pretty picky -- only envmap if we also have a gloss map and can
  // multitexture it.
  if ((envTex != -1 && drawEnvTex) &&
      ((glossTex != -1) || (!drawGlossTex)) &&
      (OpenGL::getNumMultiTextures() > 1))
  {
    // Generate the texture coords.
    generateEnvTexCoords();

    // Setup the state we need for blending.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f);

    int lightingOn = glIsEnabled(GL_LIGHTING);
    glDisable(GL_LIGHTING);

    // Set our arrays up.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // Pass the stuff in...
    glVertexPointer(3, GL_FLOAT, 16, verts);
    glNormalPointer(GL_FLOAT, 16, vertNorms);
    glColor3f(0.5f,0.5f,0.5f);

    // Get the two texture mapping units set up.
    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
    OpenGL::glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glBindTexture(GL_TEXTURE_2D, envTex);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, envTexCoords);

    if (drawGlossTex && glossTex != -1)
    {
      OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
      OpenGL::glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D, glossTex);
      glEnable(GL_TEXTURE_2D);
      glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    }

    if (OpenGL::getSupportsCompiledVertexArrays())
    {
      OpenGL::glLockArraysEXT(0,numVerts);
    }
    glDrawElements(GL_TRIANGLES, 3*numFaces, GL_UNSIGNED_INT, faces);
    if (OpenGL::getSupportsCompiledVertexArrays())
    {
      OpenGL::glUnlockArraysEXT();
    }

    if (drawGlossTex && glossTex != -1)
    {
      OpenGL::glActiveTextureARB(GL_TEXTURE1_ARB);
      OpenGL::glClientActiveTextureARB(GL_TEXTURE1_ARB);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glDisable(GL_TEXTURE_2D);
    }

    OpenGL::glActiveTextureARB(GL_TEXTURE0_ARB);
    OpenGL::glClientActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);

    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    if (lightingOn == 1)
    {
      glEnable(GL_LIGHTING);
    }
  }

  // Okay, this'll re-copy the faces and edges over from the originals and set
  // the counts on things to effectively drop the generated data.
  resetGeometryForFrame();
}

void ButterflySurface::generateEnvTexCoords()
{
  // Vector math!  Fun fun fun!
  Vector ori = GlobalCamera::Instance()->getPosition().getOrientation();
  ori.normalize();

  Vector up = GlobalCamera::Instance()->getPosition().getUpVector();
  up.normalize();

  Vector negEyePos = GlobalCamera::Instance()->getPosition();
  negEyePos *= -1;

  for (int x=0; x<numVerts; x++)
  {
    Vector eyeToVert(negEyePos);
    eyeToVert.x += verts[4*x+0];
    eyeToVert.y += verts[4*x+1];
    eyeToVert.z += verts[4*x+2];
    eyeToVert.normalize();
    Vector normal(vertNorms[4*x+0], vertNorms[4*x+1], vertNorms[4*x+2]);
    Vector nCrossU;
    normal.cross(eyeToVert, nCrossU);
    Vector nCrossNCrossU;
    normal.cross(nCrossU, nCrossNCrossU);
    eyeToVert *= -1;
    nCrossNCrossU *= -2;
    eyeToVert += nCrossNCrossU;

    eyeToVert.normalize();
    float r = ori.dot(eyeToVert);

    eyeToVert.x -= ori.x * r;
    eyeToVert.y -= ori.y * r;
    eyeToVert.z -= ori.z * r;

    eyeToVert.normalize();
    float cosTheta = up.dot(eyeToVert);
    
    // Reuse this.
    up.cross(eyeToVert, nCrossU);
    float sinTheta = nCrossU.getMagnitude();

    r = -r;
    if (r < 0) r = 0;
    r = 1.0f - r;

    envTexCoords[2*x+0] = (r*cosTheta)*0.5f + 0.5f;
    envTexCoords[2*x+1] = (r*sinTheta)*0.5f + 0.5f;
  }                        
}

void ButterflySurface::resetGeometryForFrame()
{
  // Set the numbers right, first.
  numVerts = numControlVerts;
  numFaces = numControlFaces;
  numEdges = numControlEdges;

  // Copy over the original vertex edge counts.
  int x;
  for (x=0; x<numControlVerts; x++)
  {
    vertEdges[x] = controlVertEdges[x];
  }

  // Reset the rest of them.
  for (; x<vertCapacity; x++)
  {
    vertEdges[x].numEdges = 0;
  }

  // Copy the faces and edges over since they'll have changed.
  for (x=0; x<numControlFaces; x++)
  {
    faces[3*x+0] = controlFaces[3*x+0];
    faces[3*x+1] = controlFaces[3*x+1];
    faces[3*x+2] = controlFaces[3*x+2];
    faceEdges[3*x+0] = controlFaceEdges[3*x+0];
    faceEdges[3*x+1] = controlFaceEdges[3*x+1];
    faceEdges[3*x+2] = controlFaceEdges[3*x+2];
  }

  for (x=0; x<numControlEdges; x++)
  {
    edges[x] = controlEdges[x];
  }
}

// Use these hooks to control the scale, bias, and max detail of the surface.
void ButterflySurface::setRecursionDepth(int depth)
{
  maxRecursion = depth;
}

int ButterflySurface::getRecursionDepth() const
{
  return maxRecursion;
}

// This controls the bias -- the threshold of curvature.
void ButterflySurface::setCurveThreshold(float val)
{
  curveThreshold = val;
}

float ButterflySurface::getCurveThreshold() const
{
  return curveThreshold;
}

// You can give the surface textures here.
void ButterflySurface::setBaseTexture(int base)
{
  baseTex = base;
  if (baseTex != -1)
  {
    drawBaseTex = true;
  }
}

void ButterflySurface::setGlossMap(int gloss)
{
  glossTex = gloss;
  if (glossTex != -1)
  {
    drawGlossTex = true;
  }
}

void ButterflySurface::setEnvironmentMap(int env)
{
  envTex = env;
  if (envTex != -1)
  {
    drawEnvTex = true;
  }
}

void ButterflySurface::setDrawEnvironmentMap(bool draw)
{
  drawEnvTex = draw;
}

bool ButterflySurface::getDrawEnvironmentMap() const
{
  return drawEnvTex;
}

void ButterflySurface::setDrawGlossMap(bool draw)
{
  drawGlossTex = draw;
}

bool ButterflySurface::getDrawGlossMap() const
{
  return drawGlossTex;
}

void ButterflySurface::setDrawTexture(bool draw)
{
  drawBaseTex = draw;
}

bool ButterflySurface::getDrawTexture() const
{
  return drawBaseTex;
}

// The surface uses these hooks to manage array sizes.
int ButterflySurface::addVert()
{
  // Check and see if we have no space.  If so, just make one element.
  if (vertCapacity == 0)
  {
    verts = new float[4];
    vertNorms = new float[4];
    vertEdges = new VertexEdges[1];
    texCoords = new float[2];
    envTexCoords = new float[2];
    colors = new unsigned char[4];
    vertCapacity = 1;
    numVerts = 1;
    return 0;
  }

  // Do we need to double?
  if (numVerts == vertCapacity)
  {
    float* newVerts = new float[4*vertCapacity*2];
    float* newVertNorms = new float[4*vertCapacity*2];
    VertexEdges* newVertEdges = new VertexEdges[vertCapacity*2];
    float* newTexCoords = new float[2*vertCapacity*2];
    float* newEnvTexCoords = new float[2*vertCapacity*2];
    unsigned char* newColors = new unsigned char[4*vertCapacity*2];

    int x;
    for (x=0; x<numVerts; x++)
    {
      newVerts[4*x+0] = verts[4*x+0];
      newVerts[4*x+1] = verts[4*x+1];
      newVerts[4*x+2] = verts[4*x+2];
      newVertNorms[4*x+0] = vertNorms[4*x+0];
      newVertNorms[4*x+1] = vertNorms[4*x+1];
      newVertNorms[4*x+2] = vertNorms[4*x+2];
      newVertEdges[x] = vertEdges[x];
      newTexCoords[2*x+0] = texCoords[2*x+0];
      newTexCoords[2*x+1] = texCoords[2*x+1];
      newEnvTexCoords[2*x+0] = envTexCoords[2*x+0];
      newEnvTexCoords[2*x+1] = envTexCoords[2*x+1];
      newColors[4*x+0] = colors[4*x+0];
      newColors[4*x+1] = colors[4*x+1];
      newColors[4*x+2] = colors[4*x+2];
      newColors[4*x+3] = colors[4*x+3];
    }

    delete[] verts;
    delete[] vertNorms;
    delete[] vertEdges;
    delete[] texCoords;
    delete[] envTexCoords;
    delete[] colors;

    verts = newVerts;
    vertNorms = newVertNorms;
    vertEdges = newVertEdges;
    texCoords = newTexCoords;
    envTexCoords = newEnvTexCoords;
    colors = newColors;

    vertCapacity *= 2;
  }

  numVerts++;
  return numVerts-1;
}

int ButterflySurface::addFace()
{
  // Check and see if we have no space.  If so, just make one element.
  if (faceCapacity == 0)
  {
    faces = new int[3];
    faceEdges = new int[3];
    faceCapacity = 1;
    numFaces = 1;
    return 0;
  }

  // Do we need to double?
  if (numFaces == faceCapacity)
  {
    int* newFaces = new int[3*faceCapacity*2];
    int* newFaceEdges = new int[3*faceCapacity*2];

    int x;
    for (x=0; x<numFaces; x++)
    {
      newFaces[3*x+0] = faces[3*x+0];
      newFaces[3*x+1] = faces[3*x+1];
      newFaces[3*x+2] = faces[3*x+2];
      newFaceEdges[3*x+0] = faceEdges[3*x+0];
      newFaceEdges[3*x+1] = faceEdges[3*x+1];
      newFaceEdges[3*x+2] = faceEdges[3*x+2];
    }

    delete[] faces;
    delete[] faceEdges;
    
    faces = newFaces;
    faceEdges = newFaceEdges;

    faceCapacity *= 2;
  }

  numFaces++;
  return numFaces-1;
}

int ButterflySurface::addEdge()
{
  // Check and see if we have no space.  If so, just make one element.
  if (edgeCapacity == 0)
  {
    edges = new ButterflyEdge[1];
    edgeCapacity = 1;
    numEdges = 1;
    return 0;
  }

  // Do we need to double?
  if (numEdges == edgeCapacity)
  {
    ButterflyEdge* newEdges = new ButterflyEdge[edgeCapacity*2];

    int x;
    for (x=0; x<numEdges; x++)
    {
      newEdges[x] = edges[x];
    }

    delete[] edges;
    
    edges = newEdges;

    edgeCapacity *= 2;
  }

  numEdges++;
  return numEdges-1;
}

// This tessellates the surface.
void ButterflySurface::tessellate()
{
  // Loop controls.
  int x;

  // Not adaptive for now.
  for (int level=0; level<maxRecursion; level++)
  {
    // This is how we later find the new vertices created along edges.
    int* edgeVertMap = new int[numEdges];
    for (x=0; x<numEdges; x++)
    {
      edgeVertMap[x] = -1;
    }
    
    // This is how we find the new other half-edge made when the edge is split.
    int* edgeEdgeMap = new int[numEdges];
    for (x=0; x<numEdges; x++)
    {
      edgeEdgeMap[x] = -1;
    }

    tessellateEdges(edgeVertMap, edgeEdgeMap);
    buildNewFaces(edgeVertMap, edgeEdgeMap);     

    delete[] edgeVertMap;
    delete[] edgeEdgeMap;
  }

  // Only at the end here do we generate our normals.
  generateVertexNormals();
}

// These are smaller portions of the above function, broken apart for clarity.
void ButterflySurface::tessellateEdges(int* edgeVertMap, int* edgeEdgeMap)
{
  // For each edge (the value changes inside the loop so we have to save the initial val.)
  int startingEdgeCount = numEdges;
  for (int x=0; x<startingEdgeCount; x++)
  {
    // Make a new vertex along it.
    int newVertLoc = addVert();

    // Figure out which edge number we are for each of our neighbor verts.
    int v0EdgeNum = findEdge(edges[x].v[1], vertEdges[edges[x].v[0]].edges, vertEdges[edges[x].v[0]].numEdges);
    int v1EdgeNum = findEdge(edges[x].v[0], vertEdges[edges[x].v[1]].edges, vertEdges[edges[x].v[1]].numEdges);

    assert(v0EdgeNum != -1 && v1EdgeNum != -1);

    verts[4*newVertLoc+0] = 0;
    verts[4*newVertLoc+1] = 0;
    verts[4*newVertLoc+2] = 0;
                     
    // Okay, if both verts are 6-valence, ask them each to contribute.
    // Otherwise if one is extraordinary, just use it.  Or if both are extraordinary,
    // use the average.
    int vert0 = edges[x].v[0];
    int vert1 = edges[x].v[1];
    int valence0 = vertEdges[vert0].numEdges;
    int valence1 = vertEdges[vert1].numEdges;
    if (valence0 == 6 && valence1 == 6)
    {
      getContributionToVert(vert0, v0EdgeNum, verts, verts+(4*newVertLoc));
      getContributionToVert(vert1, v1EdgeNum, verts, verts+(4*newVertLoc));
//      getContributionToVert(vert0, v0EdgeNum, colors, colors+(4*newVertLoc));
//      getContributionToVert(vert1, v1EdgeNum, colors, colors+(4*newVertLoc));
    }
    else if (valence0 == 6)
    {
      getContributionToVert(vert1, v1EdgeNum, verts, verts+(4*newVertLoc));
//      getContributionToVert(vert1, v1EdgeNum, colors, colors+(4*newVertLoc));
    }
    else if (valence1 == 6)
    {
      getContributionToVert(vert0, v0EdgeNum, verts, verts+(4*newVertLoc));
//      getContributionToVert(vert0, v0EdgeNum, colors, colors+(4*newVertLoc));
    }
    else
    {
      getContributionToVert(vert0, v0EdgeNum, verts, verts+(4*newVertLoc));
      getContributionToVert(vert1, v1EdgeNum, verts, verts+(4*newVertLoc));
      verts[4*newVertLoc+0] *= 0.5f;
      verts[4*newVertLoc+1] *= 0.5f;
      verts[4*newVertLoc+2] *= 0.5f;
//      getContributionToVert(vert0, v0EdgeNum, colors, colors+(4*newVertLoc));
//      getContributionToVert(vert1, v1EdgeNum, colors, colors+(4*newVertLoc));
//      colors[4*newVertLoc+0] *= 0.5f;
//      colors[4*newVertLoc+1] *= 0.5f;
//      colors[4*newVertLoc+2] *= 0.5f;
    }

    // Um... so the curvature of the surface appears to be very similar all over for most
    // surfaces... until I can find a good example where this helps, I'm not going to put 
    // it in, since there are bugs in the one- and two- level face building functions.
//    // Now find the midpoint so we can take the distance between them to decide whether it's
//    // worth tessellating here or not.
//    float nonlin[3];
//    nonlin[0] = (verts[4*vert0+0] + verts[4*vert1+0])*0.5f;
//    nonlin[1] = (verts[4*vert0+1] + verts[4*vert1+1])*0.5f;
//    nonlin[2] = (verts[4*vert0+2] + verts[4*vert1+2])*0.5f;
//
//    nonlin[0] -= verts[4*newVertLoc+0];
//    nonlin[1] -= verts[4*newVertLoc+1];
//    nonlin[2] -= verts[4*newVertLoc+2];
//
//    // Find the distance...
//    float dist = sqrt(nonlin[0]*nonlin[0] + nonlin[1]*nonlin[1] + nonlin[2]*nonlin[2]);
//
//    if (dist < curveThreshold)
//    {
//      // Back down the number of verts so our temp doesn't punch a whole in the array.
//      numVerts--;
//      return;
//    }
//    else
//    {
//      std::cout << "Passed with dist == " << dist << std::endl;
//    }

    // Just linearly interpolate these.
    texCoords[2*newVertLoc+0] = (texCoords[2*vert0+0] + texCoords[2*vert1+0]) * 0.5f;
    texCoords[2*newVertLoc+1] = (texCoords[2*vert0+1] + texCoords[2*vert1+1]) * 0.5f;
    colors[4*newVertLoc+0] = (colors[4*vert0+0] + colors[4*vert1+0]) * 0.5f;
    colors[4*newVertLoc+1] = (colors[4*vert0+1] + colors[4*vert1+1]) * 0.5f;
    colors[4*newVertLoc+2] = (colors[4*vert0+2] + colors[4*vert1+2]) * 0.5f;
    colors[4*newVertLoc+3] = (colors[4*vert0+3] + colors[4*vert1+3]) * 0.5f;

    // Map the edge to the vertex so we can find it later when building faces.
    edgeVertMap[x] = newVertLoc;

    // Break the current edge in half.
    int saveEdge = edges[x].v[1];

    // Add another edge for the other half.
    int newEdgeLoc = addEdge();
    edges[newEdgeLoc].v[0] = saveEdge;
    edges[newEdgeLoc].v[1] = newVertLoc;

    // Map us to that edge, too, for the same reason.
    edgeEdgeMap[x] = newEdgeLoc;
  }
  // Done tessellating edges.

  // Now go update the old edges that should have been split and the vertices that
  // got the new edge-halves.
  for (x=0; x<startingEdgeCount; x++)
  {
    if (edgeVertMap[x] != -1)
    {
      // Update the vert now hanging onto the new edge that it's not us anymore.
      int saveEdge = edges[x].v[1];
      for (int y=0; y<vertEdges[saveEdge].numEdges; y++)
      {
        if (vertEdges[saveEdge].edges[y] == x)
        {
          vertEdges[saveEdge].edges[y] = edgeEdgeMap[x];
        }
      }

      edges[x].v[1] = edgeVertMap[x];
    }
  }
}

// This is used to abstract the way a vertex computes its contribution to nearby new
// vertices, hiding the work of various valences and sharpness schemes.
void ButterflySurface::getContributionToVert(int vertNum, int newVertEdge, float* in, float* out)
{
  // Get our edges.
  VertexEdges& vEdge = vertEdges[vertNum];

  for (int x=0; x<vEdge.numEdges; x++)
  {
    // We start at newVertEdge and wrap.
    int edgeNum = (x+newVertEdge) % vEdge.numEdges;
    int otherVert = edges[vEdge.edges[edgeNum]].v[0] == vertNum ? edges[vEdge.edges[edgeNum]].v[1] : edges[vEdge.edges[edgeNum]].v[0];

    // Add it in with appropriate blending.
    out[0] += in[4*otherVert+0]*vertBlendingWeights[vEdge.numEdges][x];
    out[1] += in[4*otherVert+1]*vertBlendingWeights[vEdge.numEdges][x];
    out[2] += in[4*otherVert+2]*vertBlendingWeights[vEdge.numEdges][x];
  }

  if (vEdge.numEdges != 6)
  {
    out[0] += in[4*vertNum+0]*0.75f;
    out[1] += in[4*vertNum+1]*0.75f;
    out[2] += in[4*vertNum+2]*0.75f;
  }
}
void ButterflySurface::getContributionToVert(int vertNum, int newVertEdge, unsigned char* in, unsigned char* out)
{
  // Get our edges.
  VertexEdges& vEdge = vertEdges[vertNum];

  for (int x=0; x<vEdge.numEdges; x++)
  {
    // We start at newVertEdge and wrap.
    int edgeNum = (x+newVertEdge) % vEdge.numEdges;
    int otherVert = edges[vEdge.edges[edgeNum]].v[0] == vertNum ? edges[vEdge.edges[edgeNum]].v[1] : edges[vEdge.edges[edgeNum]].v[0];

    // Add it in with appropriate blending.
    out[0] += in[4*otherVert+0]*vertBlendingWeights[vEdge.numEdges][x];
    out[1] += in[4*otherVert+1]*vertBlendingWeights[vEdge.numEdges][x];
    out[2] += in[4*otherVert+2]*vertBlendingWeights[vEdge.numEdges][x];
  }

  if (vEdge.numEdges != 6)
  {
    out[0] += in[4*vertNum+0]*0.75f;
    out[1] += in[4*vertNum+1]*0.75f;
    out[2] += in[4*vertNum+2]*0.75f;
  }
}

void ButterflySurface::buildNewFaces(int* edgeVertMap, int* edgeEdgeMap)
{
  // For each face (the value changes in the loop as we're adding faces, so save the value.)
  int startingFaceCount = numFaces;
  for (int x=0; x<startingFaceCount; x++)
  {
    // Check and see how many of our edges were tessellated...
    int numEdgesTessellated = 0;
    
    // This stores either the edge tessellated if only one was tessellated, or
    // the edge not tessellated if none were tessellated.
    int edgeNotTessellated, edgeTessellated;
    
    for (int y=0; y<3; y++)
    {
      if (edgeVertMap[faceEdges[3*x+y]] != -1)
      {
        edgeTessellated = y;
        numEdgesTessellated++;
      }
      else 
      {
        edgeNotTessellated = y;
      }
    }

    if (numEdgesTessellated == 0)
    {
      return;
    }
    else if (numEdgesTessellated == 1)
    {
      buildLevelOneTriangles(x, edgeTessellated, edgeVertMap, edgeEdgeMap);
    }
    else if (numEdgesTessellated == 2)
    {
      buildLevelTwoTriangles(x, edgeNotTessellated, edgeVertMap, edgeEdgeMap);
    }
    else
    {
      buildLevelThreeTriangles(x, edgeVertMap, edgeEdgeMap);
    }
  }
  // Done building new faces.
}

void ButterflySurface::buildLevelOneTriangles(int x, int tessEdge, int* edgeVertMap, int* edgeEdgeMap)
{
  int fverts[4];
  int localVerts[3];

  // Find the vertex opposite the untessellated edge.
  int tessVert0 = edges[faceEdges[3*x+tessEdge]].v[0];
  int tessVert1 = edges[faceEdges[3*x+tessEdge]].v[1];

  // Our goal is to have the array be packed with the tess vert at the end.
  // So we arrange the verts differently so the one we start with is always the
  // one circularly right after the tess vert.
  if (faces[3*x+0] != tessVert0 && faces[3*x+0] != tessVert1)
  {
    fverts[0] = faces[3*x+2];
    fverts[1] = faces[3*x+0];
    fverts[2] = faces[3*x+1];
    localVerts[0] = 2;
    localVerts[1] = 0;
    localVerts[2] = 1;
  }
  else if (faces[3*x+1] != tessVert0 && faces[3*x+1] != tessVert1)
  {
    fverts[0] = faces[3*x+0];
    fverts[1] = faces[3*x+1];
    fverts[2] = faces[3*x+2];
    localVerts[0] = 0;
    localVerts[1] = 1;
    localVerts[2] = 2;
  }
  else if (faces[3*x+2] != tessVert0 && faces[3*x+2] != tessVert1)
  {
    fverts[0] = faces[3*x+1];
    fverts[1] = faces[3*x+2];
    fverts[2] = faces[3*x+0];
    localVerts[0] = 1;
    localVerts[1] = 2;
    localVerts[2] = 0;
  }

  // Now the tessellated guy.
  fverts[3] = edgeVertMap[tessEdge];

  // Now put the edges in order.
  int connectedEdges[2][2];
  connectedEdges[0][0] = (fverts[0] < fverts[2]) ? tessEdge : edgeEdgeMap[tessEdge];
  connectedEdges[1][1] = (fverts[0] < fverts[2]) ? edgeEdgeMap[tessEdge] : tessEdge;

  int otherEdge0 = (tessEdge == 0) ? 1 : 0;
  int otherEdge1 = (tessEdge == 0) ? 2 : ((tessEdge == 1) ? 2 : 1);
  if (edges[faceEdges[3*x+otherEdge0]].v[0] == fverts[0] || edges[faceEdges[3*x+otherEdge0]].v[1] == fverts[0]) 
  {
    connectedEdges[0][1] = faceEdges[3*x+otherEdge0];
    connectedEdges[1][0] = faceEdges[3*x+otherEdge1];
  }
  else
  {
    connectedEdges[1][0] = faceEdges[3*x+otherEdge0];
    connectedEdges[0][1] = faceEdges[3*x+otherEdge1];
  }

  // Now add our new edge.
  int edgeLoc = addEdge();
  edges[edgeLoc].v[0] = min(fverts[1], fverts[3]);
  edges[edgeLoc].v[1] = max(fverts[1], fverts[3]);

  // Now make our triangles, [3,0,1] and [3,1,2]

  // Build [3,0,1] in our spot...
  faces[3*x+0] = fverts[3];
  faces[3*x+1] = fverts[0];
  faces[3*x+2] = fverts[1];
  faceEdges[3*x+0] = edgeLoc;
  faceEdges[3*x+1] = connectedEdges[0][0];
  faceEdges[3*x+2] = connectedEdges[0][1];
  
  // Add [3,1,2]...
  int triLoc = addFace();
  faces[3*triLoc+0] = fverts[3];
  faces[3*triLoc+1] = fverts[1];
  faces[3*triLoc+2] = fverts[2];
  faceEdges[3*triLoc+0] = connectedEdges[1][1];
  faceEdges[3*triLoc+1] = edgeLoc;
  faceEdges[3*triLoc+2] = connectedEdges[1][0];

  // Now build the edge information about the new vertex in the right order.
  buildVertexEdgeInfo(fverts[3], connectedEdges[0][0], -1, edgeLoc, connectedEdges[1][1]);

  // Now we have to add the new edge to corner vert 1.
  addCornerEdge(fverts[1], connectedEdges[1][0], edgeLoc);
}

void ButterflySurface::buildLevelTwoTriangles(int x, int untessEdge, int* edgeVertMap, int* edgeEdgeMap)
{
  int fverts[5];
  int localVerts[3];

  // Find the vertex opposite the untessellated edge.
  int untessVert0 = edges[faceEdges[3*x+untessEdge]].v[0];
  int untessVert1 = edges[faceEdges[3*x+untessEdge]].v[1];

  int connectedEdges[3][2];

  // Our goal is to have the array be packed with the hole going at the end.
  // So we arrange the verts differently so the one we start with is always the
  // one circularly right after the hole.
  if (faces[3*x+0] != untessVert0 && faces[3*x+0] != untessVert1)
  {
    fverts[0] = faces[3*x+2];
    fverts[2] = faces[3*x+0];
    fverts[4] = faces[3*x+1];
    localVerts[0] = 2;
    localVerts[1] = 0;
    localVerts[2] = 1;
  }
  else if (faces[3*x+1] != untessVert0 && faces[3*x+1] != untessVert1)
  {
    fverts[0] = faces[3*x+0];
    fverts[2] = faces[3*x+1];
    fverts[4] = faces[3*x+2];
    localVerts[0] = 0;
    localVerts[1] = 1;
    localVerts[2] = 2;
  }
  else if (faces[3*x+2] != untessVert0 && faces[3*x+2] != untessVert1)
  {
    fverts[0] = faces[3*x+1];
    fverts[2] = faces[3*x+2];
    fverts[4] = faces[3*x+0];
    localVerts[0] = 1;
    localVerts[1] = 2;
    localVerts[2] = 0;
  }

  // Now fit the tessellated verts in there, too.
  int otherEdge0 = (untessEdge == 0) ? 1 : 0;
  int otherEdge1 = (untessEdge == 0) ? 2 : ((untessEdge == 1) ? 2 : 1);
  if (edges[faceEdges[3*x+otherEdge0]].v[0] == fverts[0] || edges[edgeEdgeMap[faceEdges[3*x+otherEdge0]]].v[0] == fverts[0]) 
  {
    fverts[1] = edgeVertMap[faceEdges[3*x+otherEdge0]];
    fverts[3] = edgeVertMap[faceEdges[3*x+otherEdge1]];
  }
  else
  {
    fverts[1] = edgeVertMap[faceEdges[3*x+otherEdge1]];
    fverts[3] = edgeVertMap[faceEdges[3*x+otherEdge0]];
    int swap = otherEdge0;
    otherEdge0 = otherEdge1;
    otherEdge1 = swap;
  }

  // Fill in the connectivity.
  connectedEdges[localVerts[0]][0] = faceEdges[3*x+untessEdge];
  connectedEdges[localVerts[2]][1] = faceEdges[3*x+untessEdge];
  if (fverts[0] < fverts[2])
  {
    connectedEdges[localVerts[0]][1] = faceEdges[3*x+otherEdge0];
    connectedEdges[localVerts[1]][0] = edgeEdgeMap[faceEdges[3*x+otherEdge0]];
  }
  else
  {
    connectedEdges[localVerts[1]][0] = faceEdges[3*x+otherEdge0];
    connectedEdges[localVerts[0]][1] = edgeEdgeMap[faceEdges[3*x+otherEdge0]];
  }

  if (fverts[2] < fverts[4])
  {
    connectedEdges[localVerts[1]][1] = faceEdges[3*x+otherEdge1];
    connectedEdges[localVerts[2]][0] = edgeEdgeMap[faceEdges[3*x+otherEdge1]];
  }
  else
  {
    connectedEdges[localVerts[2]][0] = faceEdges[3*x+otherEdge1];
    connectedEdges[localVerts[1]][1] = edgeEdgeMap[faceEdges[3*x+otherEdge1]];
  }

  // Now we need to create three triangles.  Basically, referring to fverts, we have
  // [1,2,3], [4,1,3], [4,0,1].

  // First add the two new edges we'll need.
  int edgeLoc[2];
  edgeLoc[0] = addEdge();
  edges[edgeLoc[0]].v[0] = min(fverts[1], fverts[3]);
  edges[edgeLoc[0]].v[1] = max(fverts[1], fverts[3]);
  edgeLoc[1] = addEdge();
  edges[edgeLoc[1]].v[0] = min(fverts[1], fverts[4]);
  edges[edgeLoc[1]].v[1] = max(fverts[1], fverts[4]);

  // Build the corner triangle in our spot out of [1,2,3]
  faces[3*x+0] = fverts[1];
  faces[3*x+1] = fverts[2];
  faces[3*x+2] = fverts[3];
  faceEdges[3*x+0] = edgeLoc[0];
  faceEdges[3*x+1] = connectedEdges[1][0];
  faceEdges[3*x+2] = connectedEdges[1][1];

  // Now [4,1,3].
  int triLoc = addFace();
  faces[3*triLoc+0] = fverts[4];
  faces[3*triLoc+1] = fverts[1];
  faces[3*triLoc+2] = fverts[3];
  faceEdges[3*triLoc+0] = connectedEdges[2][0];
  faceEdges[3*triLoc+1] = edgeLoc[1];
  faceEdges[3*triLoc+2] = edgeLoc[0];

  // Now [4,0,1].
  triLoc = addFace();
  faces[3*triLoc+0] = fverts[4];
  faces[3*triLoc+1] = fverts[0];
  faces[3*triLoc+2] = fverts[1];
  faceEdges[3*triLoc+0] = edgeLoc[0];
  faceEdges[3*triLoc+1] = connectedEdges[2][1];
  faceEdges[3*triLoc+2] = connectedEdges[0][1];

  // Now, also, we want to make sure the verts have their edge information correct.

  // For edge number 1, there's two mid-edges coming from it.
  int firstEdge = connectedEdges[1][0];
  int secondEdge = edgeLoc[0];
  int thirdEdge = edgeLoc[1];
  int fourthEdge = connectedEdges[0][1];
  buildVertexEdgeInfo(fverts[1], firstEdge, secondEdge, thirdEdge, fourthEdge);

  // For edge 2, there's only one new edge coming from it.
  firstEdge = connectedEdges[2][0];
  secondEdge = -1;
  thirdEdge = edgeLoc[0];
  fourthEdge = connectedEdges[1][1];
  buildVertexEdgeInfo(fverts[3], firstEdge, secondEdge, thirdEdge, fourthEdge);

  // Now we have to add the new edge to corner vert 4.
  addCornerEdge(fverts[4], connectedEdges[2][1], edgeLoc[1]);
}

void ButterflySurface::buildLevelThreeTriangles(int x, int* edgeVertMap, int* edgeEdgeMap)
{
  int fverts[6];
  fverts[0] = faces[3*x+0];
  fverts[2] = faces[3*x+1];
  fverts[4] = faces[3*x+2];

  int connectedEdges[3][2];

  for (int y=0; y<3; y++)
  {
    // We know the first is smaller than the second by the way we split the edges.
    int v0 = edges[faceEdges[3*x+y]].v[0];
    int v1 = edges[edgeEdgeMap[faceEdges[3*x+y]]].v[0];
    
    if (v0 == min(fverts[0], fverts[2]) && v1 == max(fverts[0], fverts[2]))
    {
      fverts[1] = edgeVertMap[faceEdges[3*x+y]];

      // Now figure out which vert these edges go with.
      if (fverts[0] < fverts[2])
      {
        connectedEdges[0][1] = faceEdges[3*x+y];
        connectedEdges[1][0] = edgeEdgeMap[faceEdges[3*x+y]];
      }
      else
      {
        connectedEdges[0][1] = edgeEdgeMap[faceEdges[3*x+y]];
        connectedEdges[1][0] = faceEdges[3*x+y];
      }
    }
    if (v0 == min(fverts[2], fverts[4]) && v1 == max(fverts[2], fverts[4]))
    {
      fverts[3] = edgeVertMap[faceEdges[3*x+y]];

      // Now figure out which vert these edges go with.
      if (fverts[2] < fverts[4])
      {
        connectedEdges[1][1] = faceEdges[3*x+y];
        connectedEdges[2][0] = edgeEdgeMap[faceEdges[3*x+y]];
      }
      else
      {
        connectedEdges[1][1] = edgeEdgeMap[faceEdges[3*x+y]];
        connectedEdges[2][0] = faceEdges[3*x+y];
      }
    }
    if (v0 == min(fverts[4], fverts[0]) && v1 == max(fverts[4], fverts[0]))
    {
      fverts[5] = edgeVertMap[faceEdges[3*x+y]];

      // Now figure out which vert these edges go with.
      if (fverts[4] < fverts[0])
      {
        connectedEdges[2][1] = faceEdges[3*x+y];
        connectedEdges[0][0] = edgeEdgeMap[faceEdges[3*x+y]];
      }
      else
      {
        connectedEdges[2][1] = edgeEdgeMap[faceEdges[3*x+y]];
        connectedEdges[0][0] = faceEdges[3*x+y];
      }
    }
  }

  // Add 3 new edges that we didn't know about until now, let the center
  //   know about them.
  int edgeLoc[3];
  edgeLoc[0] = addEdge();
  edges[edgeLoc[0]].v[0] = min(fverts[5], fverts[1]);
  edges[edgeLoc[0]].v[1] = max(fverts[5], fverts[1]);
  edgeLoc[1] = addEdge();
  edges[edgeLoc[1]].v[0] = min(fverts[1], fverts[3]);
  edges[edgeLoc[1]].v[1] = max(fverts[1], fverts[3]);
  edgeLoc[2] = addEdge();
  edges[edgeLoc[2]].v[0] = min(fverts[3], fverts[5]);
  edges[edgeLoc[2]].v[1] = max(fverts[3], fverts[5]);

  // Build the center triangle in our spot out of the three edge verts.
  faces[3*x+0] = fverts[1];
  faces[3*x+1] = fverts[3];
  faces[3*x+2] = fverts[5];
  faceEdges[3*x+0] = edgeLoc[0];
  faceEdges[3*x+1] = edgeLoc[1];
  faceEdges[3*x+2] = edgeLoc[2];

  // Now for each of our original corner verts
  for (y=0; y<3; y++)
  {
    // We know its new interior edge is edgeLoc[y] because that's how we built them.
    // Make a new triangle from that information.
    int triLoc = addFace();
    faces[3*triLoc+0] = edges[connectedEdges[y][0]].v[1];
    faces[3*triLoc+1] = fverts[2*y];
    faces[3*triLoc+2] = edges[connectedEdges[y][1]].v[1];

    faceEdges[3*triLoc+0] = connectedEdges[y][0];
    faceEdges[3*triLoc+1] = connectedEdges[y][1];
    faceEdges[3*triLoc+2] = edgeLoc[y];
  }

  // Now, also, we want to make sure the verts have their edge information correct.
  for (y=0; y<3; y++)
  {
    // This edge is from the mid-vert to the corner vert one step forward in our winding
    // order.
    int firstEdge = connectedEdges[(y+1)%3][0];

    // This edge is the edge facing / closest-to the first corner vert.
    int secondEdge = edgeLoc[(y+1)%3];

    // This edge is the edge facing / closest-to the opposite endpoint vert.
    int thirdEdge = edgeLoc[y];

    // This edge is the edge from the mid-vert to the opposite (previous, in our winding
    // order) corner vert.
    int fourthEdge = connectedEdges[y][1];

    buildVertexEdgeInfo(fverts[2*y+1], firstEdge, secondEdge, thirdEdge, fourthEdge);
  }
}

// This assembles, in correct CCW-wound order, a new vert's edges.  Note that it'll be
// called once by the two faces that share it, so we only add the original corner-vert
// edges on the first call.
void ButterflySurface::buildVertexEdgeInfo(int vertNum, int edge0, int edge1, int edge2, int edge3)
{
  // Are we the first?
  bool firstPoly = (vertEdges[vertNum].numEdges == 0);
  
  // If any of the edges are -1, it means there's no edge there, so don't put it in.
  if (firstPoly)
  {
    vertEdges[vertNum].edges[vertEdges[vertNum].numEdges] = edge0;
    vertEdges[vertNum].numEdges++;
  }
  if (edge1 != -1)
  {
    vertEdges[vertNum].edges[vertEdges[vertNum].numEdges] = edge1;
    vertEdges[vertNum].numEdges++;
  }
  if (edge2 != -1)
  {
    vertEdges[vertNum].edges[vertEdges[vertNum].numEdges] = edge2;
    vertEdges[vertNum].numEdges++;
  }
  if (firstPoly)
  {
    vertEdges[vertNum].edges[vertEdges[vertNum].numEdges] = edge3;
    vertEdges[vertNum].numEdges++;
  }
}

// Now we have to add the new edge to corner vert 4.
void ButterflySurface::addCornerEdge(int vertNum, int priorEdge, int addEdge)
{
  // Find that edge in the vert's edges.
  int priorEdgeIndex = 0;
  while (priorEdgeIndex < vertEdges[vertNum].numEdges && vertEdges[vertNum].edges[priorEdgeIndex] != priorEdge)
  {
    priorEdgeIndex++;
  }

  // Okay, make sure we found the edge.
  assert(priorEdgeIndex < vertEdges[vertNum].numEdges);

  // Hope we don't hit this limit!
  assert(vertEdges[vertNum].numEdges < MAX_VERTEX_VALENCE);

  // Now copy everything forward and then add the new vert.
  for (int x=vertEdges[vertNum].numEdges-1; x>priorEdgeIndex; x--)
  {
    vertEdges[vertNum].edges[x+1] = vertEdges[vertNum].edges[x];
  }

  vertEdges[vertNum].edges[priorEdgeIndex+1] = addEdge;
}

void ButterflySurface::generateVertexNormals()
{
  // Zero out the normals first.
  for (int x=0; x<numVerts; x++)
  {
    vertNorms[4*x+0] = 0;
    vertNorms[4*x+1] = 0;
    vertNorms[4*x+2] = 0;
  }

  // First generate all the vertex normals denormalized...
  for (x=0; x<numFaces; x++)
  {
    Vector facetNormal(0,0,0);
    Vector center(verts[4*faces[3*x+0]+0], verts[4*faces[3*x+0]+1], verts[4*faces[3*x+0]+2]);
    Vector v0(verts[4*faces[3*x+1]+0], verts[4*faces[3*x+1]+1], verts[4*faces[3*x+1]+2]);
    Vector v1(verts[4*faces[3*x+2]+0], verts[4*faces[3*x+2]+1], verts[4*faces[3*x+2]+2]);

    v0 -= center;
    v1 -= center;

    v0.cross(v1, facetNormal);

    for (int y=0; y<3; y++)
    {
      vertNorms[4*faces[3*x+y]+0] += facetNormal.x;
      vertNorms[4*faces[3*x+y]+1] += facetNormal.y;
      vertNorms[4*faces[3*x+y]+2] += facetNormal.z;
    }
  }

  // Normalize the vert normals.
  for (x=0; x<numVerts; x++)
  {
    float invSqrt = 1.0f / sqrt(vertNorms[4*x+0]*vertNorms[4*x+0] + vertNorms[4*x+1]*vertNorms[4*x+1] + vertNorms[4*x+2]*vertNorms[4*x+2]);
    vertNorms[4*x+0] *= invSqrt;
    vertNorms[4*x+1] *= invSqrt;
    vertNorms[4*x+2] *= invSqrt;
  }
}

// This is a lookup table of the blending values for vertices 
// of valence up to MAX_VERTEX_VALENCE;
void ButterflySurface::initializeBlendingWeights()
{
  // Taken from Zorin et al, "Interpolating Subdivision for Meshes with Arbitrary Topology."
  // SIGGRAPH '96.
  vertBlendingWeights[3][0] = 5.0f / 12.0f;
  vertBlendingWeights[3][1] = -1.0f / 12.0f;
  vertBlendingWeights[3][2] = -1.0f / 12.0f;

  vertBlendingWeights[4][0] = 3.0f / 8.0f;
  vertBlendingWeights[4][1] = 0.0f;
  vertBlendingWeights[4][2] = -1.0f / 8.0f;
  vertBlendingWeights[4][3] = 0.0f;
//  vertBlendingWeights[4][0] = 1;
//  vertBlendingWeights[4][1] = 0.0f;
//  vertBlendingWeights[4][2] = -1.0f / 3.0f;
//  vertBlendingWeights[4][3] = 0.0f;

  for(int K=5; K<=MAX_VERTEX_VALENCE; K++)
  {
    for (int j=0; j<K; j++)
    {
      vertBlendingWeights[K][j] = (0.25f + cos(2*PI*j/(float)K) + 0.5f*cos(4*PI*j/(float)K)) / (float)K;
    }
  }

  // Note that the default butterfly case of a 6-vertex is special.
  // w can be adjusted but Zorin et al just use w = 0.
  float w = 0;
  vertBlendingWeights[6][0] = (1.0f / 2.0f) - w;
  vertBlendingWeights[6][1] = (1.0f / 16.0f) + w;
  vertBlendingWeights[6][2] = (-1.0f / 16.0f) - w;
  vertBlendingWeights[6][3] = w;
  vertBlendingWeights[6][4] = (-1.0f / 16.0f) - w;
  vertBlendingWeights[6][5] = (1.0f / 16.0f) + w;
}

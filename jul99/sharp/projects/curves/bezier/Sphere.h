#ifndef SPHERE_H
#define SPHERE_H

class Sphere
{
public:
  Sphere(int numPolys, unsigned int texName);

  void draw(unsigned int lightmap, float u, float v);

protected:
  int addVertex(float, float, float);
  void addTri(int* indices);
  void subdivide(float*, float*, float*, long);
  void setNumPolys(int);
  void initResolution();

  void reserveVerts(int numVerts);
  void reserveTris(int numTris);

  void drawSelf(bool useTexCoords = true, float u = 0, float v = 0);

  int* mTris;
  int mNumTris;

  float* mVertexData;
//  float* mNormalData;
  float* mTexCoordData;
  int mNumVerts;
  
  int numPolys;

  unsigned int textureName;
};

#endif // SPHERE_H
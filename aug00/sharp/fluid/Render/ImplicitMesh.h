#ifndef IMPLICITMESH_H
#define IMPLICITMESH_H

//
// ImplicitMesh
//
// ImplicitMesh's job is simple: store polygons and render them later.  It's the repository
// for all polygons generated during surface tessellation, which it then renders out.  It's
// pretty straightforward.
//
// ImplicitMesh is the only one who knows about the rendering subsystem (OpenGL) but other
// than that it's pretty stupid.  CubePolygonizer depends on it as a place to dump polys.
//

#include <vector>
#include <math/Vector.h>

typedef std::vector<float> FloatArray;
typedef std::vector<unsigned int> UIntArray;
typedef std::vector<unsigned char> UCharArray;

class ImplicitMesh
{
public:
  ImplicitMesh();

  void render() const;

  void renderNormals() const;

  void clear();

  // Used to tell this guy which arrays to pass.  I assume vertex arrays should always
  // be enabled. :-P
  void setUseNormalArrays(bool);
  void setUseColorArrays(bool);
  void setUseTexCoordArrays(bool);
  
  bool getUseNormalArrays() const;

  // We call these in the inner loop to determine whether to light or spheremap, hence the inline.
  // We always generate normals (unless neither we nor OpenGL are doing either texturing or lighting,
  // which is a kind of boring case) so we always generate normals.
  __inline bool getUseColorArrays() const;
  __inline bool getUseTexCoordArrays() const;

  // Adds a vertex and returns its index.
  int addVertex(const Vector& newVert);
  int addVertex(float* newVert);
  int addVertex(float newX, float newY, float newZ);

  // Adds a normal and returns its index.  If you don't always add a normal when you
  // add a vertex everything's going to get out of sync.
  int addNormal(const Vector& newNormal);
  int addNormal(float* newNormal);
  int addNormal(float newX, float newY, float newZ);

  // Adds a color.  Same restrictions as for adding everything else (i.e. add one of everything
  // every time or things get out of sync.)
  int addColor(unsigned char*);

  // Adds a texcoord pair.  Same restrictions as for adding everything else (i.e. add one of everything
  // every time or things get out of sync.)
  int addTexCoords(float*);

  // Adds three indices (one triangle.)
  void addTri(unsigned int index0, unsigned int index1, unsigned int index2);

  // This causes this guy to compute his normals (a simple geometric normal calculation) and then
  // to subsequently generate texcoords from sphere-mapping and/or lighting colors.
  void generateNormals();
  void doLight();
  void doSphereMap();

  // Statistics stuff.
  int getNumTris() { return indices.size()/3; }
  int getNumVertices() { return vertices.size()/4; }

private:
  FloatArray vertices;
  FloatArray normals;
  FloatArray texCoords;
  UCharArray colors;
  UIntArray indices;

  bool useNormals, useColors, useTexCoords;
};

__inline bool ImplicitMesh::getUseColorArrays() const
{ 
  return useColors; 
}

__inline bool ImplicitMesh::getUseTexCoordArrays() const 
{ 
  return useTexCoords; 
}

#endif //IMPLICITMESH_H
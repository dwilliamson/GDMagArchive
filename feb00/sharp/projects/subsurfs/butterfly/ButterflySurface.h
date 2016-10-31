#ifndef BUTTERFLYSURFACE_H
#define BUTTERFLYSURFACE_H

#include <subsurfs/ControlNet.h>

#define MAX_VERTEX_VALENCE 16

class VertexEdges
{
public:
  VertexEdges();
  VertexEdges(const VertexEdges& source);
  VertexEdges& operator=(const VertexEdges& source);
  int numEdges;
  int edges[MAX_VERTEX_VALENCE];
};


class ButterflyEdge
{
public:
  bool operator==(const ButterflyEdge& cmp) const;
  bool operator<(const ButterflyEdge& cmp) const;
  int v[2];
};

class ButterflySurface
{
public:
  ButterflySurface();

  // This copies the information from the specified net into this surface.
  // You can query to get the vertices back to move them around, and you're
  // guaranteed that they'll be in the same order that they are in the argument
  // control net.
  virtual void setControlNet(const ControlNet& net);
  virtual float* getControlVerts();

  // You need to call this when you've modified the model by changing the data that
  // getControlVerts() points to, or else it won't be taken into account.
  void changedVertices();

  // This'll draw the surface using the GlobalCamera to determine LOD issues.
  void draw();

  // Use these hooks to control the scale, bias, and max detail of the surface.
  void setRecursionDepth(int depth);
  int getRecursionDepth() const;

  // This controls the bias -- the threshold of curvature.
  void setCurveThreshold(float val);
  float getCurveThreshold() const;

  // You can give the surface textures here.
  void setBaseTexture(int baseTex);
  void setGlossMap(int glossMap);
  void setEnvironmentMap(int envMap);

  void setDrawEnvironmentMap(bool draw);
  bool getDrawEnvironmentMap() const;
  void setDrawGlossMap(bool draw);
  bool getDrawGlossMap() const;
  void setDrawTexture(bool draw);
  bool getDrawTexture() const;

protected:
  // Empty us out.
  void clear();

  // This is called by draw() to flush the tessellation and set the state back to the
  // original control net data.
  void resetGeometryForFrame();

  // The surface uses these hooks to manage array sizes.
  int addVert();
  int addFace();
  int addEdge();

  // Helper function to find an edge to a particular vertex in an array of edges.
  inline int findEdge(int destVert, int* edges, int numEdges);

  // This tessellates the surface.
  void tessellate();

  // These are smaller portions of the above function, broken apart for clarity.
  void tessellateEdges(int* edgeVertMap, int* edgeEdgeMap);
  void buildNewFaces(int* edgeVertMap, int* edgeEdgeMap);
  void generateVertexNormals();

  // This is used if we're environment mapping.
  void generateEnvTexCoords();

  // This is used to abstract the way a vertex computes its contribution to nearby new
  // vertices, hiding the work of various valences and sharpness schemes.
  void getContributionToVert(int vertNum, int newVertEdge, float* in, float* out);
  void getContributionToVert(int vertNum, int newVertEdge, unsigned char* in, unsigned char* out);

  // These are used to generate new faces from a face given that a certain number of
  // its edges have been tessellated.  When we support adaptive subdivision, we'll
  // need to support one and two edges tessellated, but for now all three will be.
  void buildLevelOneTriangles(int faceNum, int tessEdge, int* edgeVertMap, int* edgeEdgeMap);
  void buildLevelTwoTriangles(int faceNum, int untessEdge, int* edgeVertMap, int* edgeEdgeMap);
  void buildLevelThreeTriangles(int faceNum, int* edgeVertMap, int* edgeEdgeMap);

  // Used to accumulate the edge information about new vertices as the faces are
  // being put together.
  void buildVertexEdgeInfo(int vertNum, int edge0, int edge1, int edge2, int edge3);
  void addCornerEdge(int vertNum, int priorEdge, int addEdge);

  // Configuration data.
  int maxRecursion;

  // These are the originals we keep around to let them modify and have us
  // recopy into our arrays.
  int numControlVerts;
  float* controlVerts;
  VertexEdges* controlVertEdges;
  float* controlTexCoords;
  unsigned char* controlColors;

  // All faces are triangulated.
  int numControlFaces;
  int* controlFaces;
  int* controlFaceEdges;

  // Connectivity information, needed for tessellating.
  int numControlEdges;
  ButterflyEdge* controlEdges;

  int numVerts;
  int vertCapacity;
  float* verts;
  float* vertNorms;
  VertexEdges* vertEdges;
  float* texCoords;
  float* envTexCoords;
  unsigned char* colors;

  // All faces are triangles.
  int numFaces;
  int faceCapacity;
  int* faces;
  int* faceEdges;

  // Connectivity information, needed for tessellating.
  int numEdges;
  int edgeCapacity;
  ButterflyEdge* edges;

  // This is a lookup table of the blending values for vertices 
  // of valence up to MAX_VERTEX_VALENCE;
  void initializeBlendingWeights();
  float vertBlendingWeights[MAX_VERTEX_VALENCE+1][MAX_VERTEX_VALENCE];

  // These are our three textures, the base texture, gloss-map, and environment map.
  // The way we apply them is: gloss*envmap + lit verts * base.  The envmap is sphere-mapped
  // onto the model.  That way, you can modulate the shininess of the surface, and you
  // get a lit base texture, too.
  int baseTex;
  int glossTex;
  int envTex;
  bool drawBaseTex;
  bool drawEnvTex;
  bool drawGlossTex;
  float curveThreshold;
};

int ButterflySurface::findEdge(int destVert, int* edgeArray, int numEdges)
{
  for (int x=0; x<numEdges; x++)
  {
    if (edges[edgeArray[x]].v[0] == destVert || edges[edgeArray[x]].v[1] == destVert)
    {
      return x;
    }
  }

  // Oops, didn't find it.
  return -1;
}

#endif //BUTTERFLYSURFACE_H
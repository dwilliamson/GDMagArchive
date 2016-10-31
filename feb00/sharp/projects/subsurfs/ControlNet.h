#ifndef CONTROLNET_H
#define CONTROLNET_H

#include <math/Vector.h>
#include <vector>
#include <map>
#include <assert.h>

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

class ControlFace
{
public:
  ControlFace()
  {
    verts = 0;
    numVerts = 0;
  }
  
  ControlFace(const ControlFace& source)
  {
    verts = 0;
    setNumVerts(source.numVerts);
    for (int x=0; x<numVerts; x++)
    {
      verts[x] = source.verts[x];
    }
  }

  ControlFace& operator=(const ControlFace& source)
  {
    setNumVerts(source.numVerts);
    for (int x=0; x<numVerts; x++)
    {
      verts[x] = source.verts[x];
    }
    return *this;
  }

  void setNumVerts(int newNumVerts)
  {
    delete[] verts;
    numVerts = newNumVerts;
    verts = new int[numVerts];
  }

  int numVerts;
  int* verts;
};

// This class represents a control net for a generalized subdivision surface (it's not
// particular to the scheme used, be it Loop or Catmull-Clark or Doo-Sabin.)
class ControlNet
{
public:
  ControlNet();
  ControlNet(const ControlNet&);
  ControlNet(const std::vector<Vector>& verts, const std::vector<std::pair<float, float> > texCoords, const std::vector<float> colors, const std::vector<std::vector<int> >& faces);
  ControlNet(FILE* infile);

  ControlNet& operator=(const ControlNet&);

  bool setData(FILE* infile);
  void setData(const std::vector<Vector>& verts, const std::vector<std::pair<float, float> > texCoords, const std::vector<float> colors, const std::vector<std::vector<int> >& faces);

  void clear();

  // Utility functions to get the net in the right place.
  void center();
  void scaleTo(float size);
  void genTexCoords();

  int numVerts, numFaces;
  float* verts;
  float* texCoords;
  float* colors;
  ControlFace* faces;

  int baseTex;
  int glossTex;
  int envTex;
};

#endif //CONTROLNET_H
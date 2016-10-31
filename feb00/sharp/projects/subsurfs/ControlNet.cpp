#include <subsurfs/ControlNet.h>
#include <harness/TextureManager.h>
#include <set>
#include <assert.h>
#include <stdio.h>

// Stupid STL name-mangling warning.
#pragma warning (disable: 4786)

ControlNet::ControlNet()
{
  numVerts = 0;
  numFaces = 0;
  verts = 0;
  texCoords = 0;
  colors = 0;
  faces = 0;

  baseTex = -1;
  glossTex = -1;
  envTex = -1;
}

ControlNet::ControlNet(const ControlNet& source)
{
  numVerts = source.numVerts;
  numFaces = source.numFaces;

  verts = new float[numVerts*4];
  faces = new ControlFace[numFaces];

  int x;
  for (x=0; x<numVerts*4; x++)
  {
    verts[x] = source.verts[x];
  }
  for (x=0; x<numFaces; x++)
  {
    faces[x] = source.faces[x];
  }
}

ControlNet& ControlNet::operator=(const ControlNet& source)
{
  clear();

  numVerts = source.numVerts;
  numFaces = source.numFaces;

  verts = new float[numVerts*4];
  faces = new ControlFace[numFaces];

  int x;
  for (x=0; x<numVerts*4; x++)
  {
    verts[x] = source.verts[x];
  }
  for (x=0; x<numFaces; x++)
  {
    faces[x] = source.faces[x];
  }

  return *this;
}

ControlNet::ControlNet(const std::vector<Vector>& newVerts, const std::vector<std::pair<float, float> > newTexCoords, const std::vector<float> colors, const std::vector<std::vector<int> >& newFaces)
{
  numVerts = 0;
  numFaces = 0;
  verts = 0;
  faces = 0;

  setData(newVerts, newTexCoords, colors, newFaces);
}

ControlNet::ControlNet(FILE* infile)
{
  numVerts = 0;
  numFaces = 0;
  verts = 0;
  faces = 0;
  setData(infile);
}

bool ControlNet::setData(FILE* infile)
{
  float v0, v1, v2, v3;
  int i[10];
  char texName[80];

  std::vector<Vector> vertices;
  std::vector<std::pair<float, float> > texcoords;
  std::vector<float> colors;
  std::vector<std::vector<int> > indices;

  char buf[81];
  bool done = false;
  while (!done)
  {
    fgets(buf, 80, infile);

    if (feof(infile))
    {
      done = true;
    }
    else if (sscanf(buf, "BaseTexture %s %d %d", texName, i+0, i+1) == 3)
    {
      baseTex = TextureManager::instance()->addTexture(std::string(texName), i[0], i[1], false);
    }
    else if (sscanf(buf, "GlossTexture %s %d %d", texName, i+0, i+1) == 3)
    {
      glossTex = TextureManager::instance()->addTexture(std::string(texName), i[0], i[1], false);
    }
    else if (sscanf(buf, "EnvTexture %s %d %d", texName, i+0, i+1) == 3)
    {
      envTex = TextureManager::instance()->addTexture(std::string(texName), i[0], i[1], false);
    }
    else
    {
      switch (buf[0])
      {
      case 'v':
        {
          if (sscanf(buf, "v %f %f %f", &v0, &v1, &v2) != 3)
          {
            return false;
          }
          vertices.push_back(Vector(v0,v1,v2)); 
        }
        break;

      case 't':
        {
          if (sscanf(buf, "t %f %f", &v0, &v1) != 2)
          {
            return false;
          }
          texcoords.push_back(std::pair<float,float>(v0,v1)); 
        }
        break;

      case 'c':
        {
          if (sscanf(buf, "c %f %f %f %f", &v0, &v1, &v2, &v3) != 4)
          {
            return false;
          }
          colors.push_back(v0);
          colors.push_back(v1);
          colors.push_back(v2);
          colors.push_back(v3);
        }
        break;

      case 'f':
        {
          // We only support up to ten vertices per face so we don't really
          // have to make this arbitrarily large.  But they have to be triangles, at least.
          int faceVerts;
          if ((faceVerts = sscanf(buf, "f %d %d %d %d %d %d %d %d %d %d", i+0, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9)) < 3)
          {
            return false;
          }
          indices.push_back(std::vector<int>());
          for (int x=0; x<faceVerts; x++)
          {
            indices[indices.size()-1].push_back(i[x]);
          }
        }
        break;

      default:
        {
          // Ignore lines that we don't recognize.
//          return false;
        }
        break;
      }
    }
  }

  // If they're non-empty they should be equal.
  int vertCt = vertices.size();
  int texCCt = texcoords.size();
  int colrCt = colors.size() / 4;

  if ((vertCt != texCCt) && (texCCt != 0))
  {
    return false;
  }
  if ((vertCt != colrCt) && (colrCt != 0))
  {
    return false;
  }

  // Okay, got all the data.  Transfer it to the arrays for easy access.
  setData(vertices, texcoords, colors, indices);
  return true;
}

void ControlNet::setData(const std::vector<Vector>& newVerts, const std::vector<std::pair<float, float> > newTexCoords, const std::vector<float> newColors, const std::vector<std::vector<int> >& newFaces)
{
  int x, y;

  // Copy the vertices in.
  delete[] verts;
  numVerts = newVerts.size();
  verts = new float[numVerts*4];
  for (x=0; x<numVerts; x++)
  {
    verts[4*x+0] = newVerts[x].x;
    verts[4*x+1] = newVerts[x].y;
    verts[4*x+2] = newVerts[x].z;
  }

  // Copy in the texture coords and colors if there are any.
  delete[] texCoords;
  if (newTexCoords.size() != 0)
  {
    texCoords = new float[2*numVerts];
    for (x=0; x<numVerts; x++)
    {
      texCoords[2*x+0] = newTexCoords[x].first;
      texCoords[2*x+1] = newTexCoords[x].second;
    }
  }

  delete[] colors;
  if (newColors.size() != 0)
  {
    colors = new float[4*numVerts];
    for (x=0; x<4*numVerts; x++)
    {
      colors[x] = newColors[x];
    }
  }

  // Copy the faces in.
  delete[] faces;
  numFaces = newFaces.size();
  faces = new ControlFace[numFaces];
  for (x = 0; x<numFaces; x++)
  {
    faces[x].setNumVerts(newFaces[x].size());
    for (y=0; y<faces[x].numVerts; y++)
    {
      faces[x].verts[y] = newFaces[x][y];
    }
  }
}

void ControlNet::clear()
{
  numFaces = 0;
  numVerts = 0;
  delete[] faces;
  delete[] verts;
  delete[] texCoords;
  delete[] colors;
  verts = 0;
  faces = 0;
  texCoords = 0;
  colors = 0;
}

// Utility functions to get the net in the right place.
void ControlNet::center()
{
  if (numVerts == 0)
  {
    return;
  }

  float mins[3], maxs[3];

  mins[0] = maxs[0] = verts[0];
  mins[1] = maxs[1] = verts[1];
  mins[2] = maxs[2] = verts[2];

  int x, y;

  for (x=1; x<numVerts; x++)
  {
    for (y=0; y<3; y++)
    {
      mins[y] = min(mins[y], verts[4*x+y]);
      maxs[y] = max(maxs[y], verts[4*x+y]);
    }
  }

  // Now subtract off the geometric center.
  for (x=0; x<numVerts; x++)
  {
    for (y=0; y<3; y++)
    {
      verts[4*x+y] -= (mins[y]+maxs[y])*0.5f;
    }
  }
}

void ControlNet::scaleTo(float size)
{
  if (numVerts == 0)
  {
    return;
  }

  float maxs[3];

  maxs[0] = verts[0];
  maxs[1] = verts[1];
  maxs[2] = verts[2];

  int x, y;

  for (x=1; x<numVerts; x++)
  {
    for (y=0; y<3; y++)
    {
      maxs[y] = max(maxs[y], fabs(verts[4*x+y]));
    }
  }

  // Now scale to not more than that value.
  float maxDim = max(maxs[0], max(maxs[1], maxs[2]));
  float scaleFactor = size / maxDim;

  // Now subtract off the geometric center.
  for (x=0; x<numVerts; x++)
  {
    for (y=0; y<3; y++)
    {
      verts[4*x+y] *= scaleFactor;
    }
  }
}

void ControlNet::genTexCoords()
{
  float mins[3], maxs[3];

  mins[0] = maxs[0] = verts[0];
  mins[1] = maxs[1] = verts[1];
  mins[2] = maxs[2] = verts[2];

  int x, y;

  for (x=1; x<numVerts; x++)
  {
    for (y=0; y<3; y++)
    {
      mins[y] = min(mins[y], verts[4*x+y]);
      maxs[y] = max(maxs[y], verts[4*x+y]);
    }
  }

  float yScale = 1;
  float yBias = 0;
  float xScale = 1;
  float xBias = 0;

  for (x=1; x<numVerts; x++)
  {
    texCoords[2*x+0] = ((verts[4*x+1] - mins[1]) / (maxs[1]-mins[1])) * yScale + yBias;
    texCoords[2*x+1] = ((verts[4*x+0] - mins[0]) / (maxs[0]-mins[0])) * xScale + xBias;
  }
}

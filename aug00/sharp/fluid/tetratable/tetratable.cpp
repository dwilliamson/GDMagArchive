//
// Marching Cube Lookup Table Generator
//
// This program is used to generate a pair of lookup tables used in polygonizing a cubelet
// by breaking it into 6 tetrahedrons and polygonizing them.
//

#pragma warning (disable: 4786)

#include <math/Math.h>
#include <math/Vector.h>

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <assert.h>

// Class representing a pair of edges; a comparison with another one succeeds even if they face
// the other direction.
class IntPair
{
public:
  int first, second;

  IntPair(int one, int two) { first = one; second = two; }

  bool operator<(const IntPair& cmp) const
  {
    int minL = Math::minOf(first,second);
    int maxL = Math::maxOf(first,second);
    int minR = Math::minOf(cmp.first,cmp.second);
    int maxR = Math::maxOf(cmp.first,cmp.second);

    // Are we entirely to one side or the other?
    if (maxL < minR) return true;
    if (maxR < minL) return false;

    // Okay, base it off the mins if we overlap.
    if (minL < minR) return true;
    if (minL > minR) return false;

    // Mins are equal.  Maxes?
    if (maxL < maxR) return true;
    if (maxL > maxR) return false;

    // We're equal.
    return false;
  }

  bool operator==(const IntPair& cmp) const
  {
    if (first == cmp.first && second == cmp.second) return true;
    if (second == cmp.first && first == cmp.second) return true;
    return false;
  }
};

std::map<IntPair, int> edgeIndexTable;

void computeTetraPolys(int a, int b, int c, int d, int bits, std::vector<int>& polysOut);

int main(int argc, char** argv)
{
  // So the idea is this: for each of the 256 possible cube combinations (where each of the vertices
  // can be either inside or outside the surface) we ask each tetrahedron of the cubelet to add whatever
  // polygons it contains to the list.
  std::vector<std::vector<int> > triIndices;
  std::vector<int> edgesNeeded;
  triIndices.resize(256);
  edgesNeeded.resize(256);

  // Another thing... we define the edge table in terms of the edges it connects.  Only some connections
  // are valid, though.
  edgeIndexTable[IntPair(0,1)] = 0;
  edgeIndexTable[IntPair(1,2)] = 1;
  edgeIndexTable[IntPair(2,3)] = 2;
  edgeIndexTable[IntPair(3,0)] = 3;
  
  edgeIndexTable[IntPair(4,5)] = 4;
  edgeIndexTable[IntPair(5,6)] = 5;
  edgeIndexTable[IntPair(6,7)] = 6;
  edgeIndexTable[IntPair(7,4)] = 7;

  edgeIndexTable[IntPair(0,4)] = 8;
  edgeIndexTable[IntPair(1,5)] = 9;
  edgeIndexTable[IntPair(2,6)] = 10;
  edgeIndexTable[IntPair(3,7)] = 11;

  edgeIndexTable[IntPair(1,4)] = 12;
  edgeIndexTable[IntPair(2,5)] = 13;
  edgeIndexTable[IntPair(2,7)] = 14;
  edgeIndexTable[IntPair(3,4)] = 15;

  edgeIndexTable[IntPair(1,3)] = 16;
  edgeIndexTable[IntPair(5,7)] = 17;

  edgeIndexTable[IntPair(3,5)] = 18;

  // So build the list of tris needed for each value in the lookup table.
  int maxNumTris = 0;
  for (int x=0; x<256; x++)
  {
    // The cubelet breaks up into 6 tetrahedrons with the specified vertices...
    computeTetraPolys(0,3,1,4, x, triIndices[x]);
    computeTetraPolys(1,3,2,5, x, triIndices[x]);
    computeTetraPolys(1,3,5,4, x, triIndices[x]);
    computeTetraPolys(2,5,3,7, x, triIndices[x]);
    computeTetraPolys(2,6,5,7, x, triIndices[x]);
    computeTetraPolys(3,5,4,7, x, triIndices[x]);

    maxNumTris = Math::maxOf(maxNumTris, triIndices[x].size()/3);
  }

  // Got the list of tris.  Now compute which edges are needed for each.
  for (x=0; x<256; x++)
  {
    int neededEdgeBits = 0;

    for (int y=0; y<triIndices[x].size(); y++)
    {
      neededEdgeBits |= (1 << triIndices[x][y]);
    }

    edgesNeeded[x] = neededEdgeBits;
  }

  // Verify those edges fairly rigorously.
  for (x=0; x<8; x++)
  {
    for (int y=0; y<8; y++)
    {
      IntPair edge(x,y);
      if (edgeIndexTable.find(edge) != edgeIndexTable.end())
      {
        for (int z=0; z<256; z++)
        {
          if (((z & (1<<x)) ? true:false) != ((z & (1<<y)) ? true:false))
          {
            if (!(edgesNeeded[z] & (1<<(edgeIndexTable[edge]))))
            {
              int breakpoint = 0;
              assert(false);
            }
          }
          else
          {
            if (edgesNeeded[z] & (1<<(edgeIndexTable[edge])))
            {
              int breakpoint = 0;
              assert(false);
            }
          }
        }
      }
    }
  }

  // Output them in a useful format...
  std::cout << "int edgeTable[256] = " << std::endl << "{" << std::endl;
  for (x=0; x<32; x++)
  {
    std::cout << "  ";
    for (int y=0; y<8; y++)
    {
      std::cout << edgesNeeded[x*8 + y] << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << "};" << std::endl << std::endl;

  std::cout << "int triTable[256][" << (maxNumTris*3+1) << "] = " << std::endl << "{" << std::endl;
  for (x=0; x<256; x++)
  {
    std::cout << "  {";
    for (int y=0; y<maxNumTris*3; y++)
    {
      if (triIndices[x].size() > y) std::cout << triIndices[x][y] << ", ";
      else std::cout << "-1, ";
    }
    std::cout << "-1}," << std::endl;
  }
  std::cout << "};" << std::endl;

  return 0;
}



// This function computes, for a tetrahedron with vertices (a,b,c,d), where a, b, c, and d are
// all in the range [0,7], what polygons are produced for the given set of bits.
void computeTetraPolys(int a, int b, int c, int d, int bits, std::vector<int>& polysOut)
{
  Vector corners[8] = {Vector(0,0,0), Vector(1,0,0), Vector(1,1,0), Vector(0,1,0), 
                       Vector(0,0,1), Vector(1,0,1), Vector(1,1,1), Vector(0,1,1)};

  Vector edges[19] = {(corners[0]+corners[1])*0.5f, (corners[1]+corners[2])*0.5f, 
                      (corners[2]+corners[3])*0.5f, (corners[3]+corners[0])*0.5f, 
                      (corners[4]+corners[5])*0.5f, (corners[5]+corners[6])*0.5f, 
                      (corners[6]+corners[7])*0.5f, (corners[7]+corners[4])*0.5f, 
                      (corners[0]+corners[4])*0.5f, (corners[1]+corners[5])*0.5f, 
                      (corners[2]+corners[6])*0.5f, (corners[3]+corners[7])*0.5f, 
                      (corners[1]+corners[4])*0.5f, (corners[2]+corners[5])*0.5f, 
                      (corners[2]+corners[7])*0.5f, (corners[3]+corners[4])*0.5f, 
                      (corners[1]+corners[3])*0.5f, (corners[5]+corners[7])*0.5f, 
                      (corners[3]+corners[5])*0.5f };


  int vertFlags = 0;
  vertFlags |= (bits & (1 << a)) ? 1 : 0;
  vertFlags |= (bits & (1 << b)) ? 2 : 0;
  vertFlags |= (bits & (1 << c)) ? 4 : 0;
  vertFlags |= (bits & (1 << d)) ? 8 : 0;

  // Now use those as their own bitfields into a lookup table; there are 16 possible configurations,
  // each with their own polygonization, designated in this lookup table.
#define TERM IntPair(-1,-1)
#define TRI(in0, out0, out1, out2) IntPair(in0,out0), IntPair(in0,out1), IntPair(in0,out2),TERM,TERM,TERM
#define QUAD(in0, in1, out0, out1) IntPair(in0,out0), IntPair(in0,out1), IntPair(in1,out0), \
                                   IntPair(in1,out0), IntPair(in0,out1), IntPair(in1,out1)

  IntPair triTable[16][6] = {{TERM,TERM,TERM,TERM,TERM,TERM},
                             {TRI (a,  b,c,d)},
                             {TRI (b,  d,c,a)},
                             {QUAD(a,b,  c,d)},
                             {TRI (c,  a,b,d)},
                             {QUAD(b,d,  a,c)},
                             {QUAD(b,c,  a,d)},
                             {TRI (d,  a,b,c)},
                             {TRI (d,  c,b,a)},
                             {QUAD(a,d,  b,c)},
                             {QUAD(a,c,  b,d)},
                             {TRI (c,  d,b,a)},
                             {QUAD(c,d,  a,b)},
                             {TRI (b,  a,c,d)},
                             {TRI (a,  d,c,b)},
                             {TERM,TERM,TERM,TERM,TERM,TERM}};

  // If there's one triangle, add it.
  if (triTable[vertFlags][0].first != -1)
  {
    // Make sure they're there.
    assert(edgeIndexTable.find(triTable[vertFlags][0]) != edgeIndexTable.end());
    assert(edgeIndexTable.find(triTable[vertFlags][1]) != edgeIndexTable.end());
    assert(edgeIndexTable.find(triTable[vertFlags][2]) != edgeIndexTable.end());

    assert(edgeIndexTable.find(triTable[vertFlags][0])->first == triTable[vertFlags][0]);
    assert(edgeIndexTable.find(triTable[vertFlags][1])->first == triTable[vertFlags][1]);
    assert(edgeIndexTable.find(triTable[vertFlags][2])->first == triTable[vertFlags][2]);

    int ind0 = edgeIndexTable[triTable[vertFlags][0]];
    int ind1 = edgeIndexTable[triTable[vertFlags][1]];
    int ind2 = edgeIndexTable[triTable[vertFlags][2]];

    polysOut.push_back(ind0);
    polysOut.push_back(ind1);
    polysOut.push_back(ind2);

    // Verify that this triangle is pointing the right direction.
    Vector polyNorm;
    Vector v0 = edges[ind0];
    Vector v1 = edges[ind1];
    Vector v2 = edges[ind2];
    Vector centroid = (v0+v1+v2) * (1.0f/3.0f);
    Vector v0v1 = v1-v0;
    Vector v0v2 = v2-v0;
    v0v1.cross(v0v2, polyNorm);
    int causeCornerIndex = triTable[vertFlags][0].first;
    Vector causeCorner = corners[causeCornerIndex];
    Vector polyToCorner = causeCorner - centroid;
    
    if (bits & (1<<causeCornerIndex))
    {
      if (polyNorm.dot(polyToCorner) > 0)
      {
        int breakpoint = 0;
        assert(false);
      }
    }
    else
    {
      if (polyNorm.dot(polyToCorner) < 0)
      {
        int breakpoint = 0;
        assert(false);
      }
    }

    // Furthermore, was there a second triangle?
    if (triTable[vertFlags][3].first != -1)
    {
      // Make sure they're there.
      assert(edgeIndexTable.find(triTable[vertFlags][3]) != edgeIndexTable.end());
      assert(edgeIndexTable.find(triTable[vertFlags][4]) != edgeIndexTable.end());
      assert(edgeIndexTable.find(triTable[vertFlags][5]) != edgeIndexTable.end());

      int ind3 = edgeIndexTable[triTable[vertFlags][3]];
      int ind4 = edgeIndexTable[triTable[vertFlags][4]];
      int ind5 = edgeIndexTable[triTable[vertFlags][5]];

      polysOut.push_back(ind3);
      polysOut.push_back(ind4);
      polysOut.push_back(ind5);

      // Verify that this triangle is pointing the right direction.
      Vector polyNorm;
      Vector v0 = edges[ind3];
      Vector v1 = edges[ind4];
      Vector v2 = edges[ind5];
      Vector centroid = (v0+v1+v2) * (1.0f/3.0f);
      Vector v0v1 = v1-v0;
      Vector v0v2 = v2-v0;
      v0v1.cross(v0v2, polyNorm);
      int causeCornerIndex = triTable[vertFlags][3].first;
      Vector causeCorner = corners[causeCornerIndex];
      Vector polyToCorner = causeCorner - centroid;
    
      if (bits & (1<<causeCornerIndex))
      {
        if (polyNorm.dot(polyToCorner) > 0)
        {
          int breakpoint = 0;
          assert(false);
        }
      }
      else
      {
        if (polyNorm.dot(polyToCorner) < 0)
        {
          int breakpoint = 0;
          assert(false);
        }
      }
    }
  }

  // Done!
}

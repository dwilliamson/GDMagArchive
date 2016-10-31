#ifndef INTVECTOR_H
#define INTVECTOR_H

//
// IntVector
//
// This is a small helper class used to track cubelet cells and other integer-valued N-vectors, like
// tracking cell corners to keep a map of function values we've already computed and the like.  This
// is used, for instance, by CubePolygonizer and also ContinuousTessellator to track cubelet info.
//

#include <memory.h>

class IntVector
{
public:
  IntVector() {}
  IntVector(const unsigned short cellIn[3]) { memcpy(cell, cellIn, 3*sizeof(unsigned short)); }
  void setLoc(const unsigned short cellIn[3]) { memcpy(cell, cellIn, 3*sizeof(unsigned short)); }

  unsigned short cell[3];
  __inline bool operator<(const IntVector& cmp) const
  {
    // Compare in order of precedence -- x matters more than y matters more than z, etc.
    if (cell[0] > cmp.cell[0]) return false;
    if (cell[0] < cmp.cell[0]) return true;

    if (cell[1] > cmp.cell[1]) return false;
    if (cell[1] < cmp.cell[1]) return true;
    
    if (cell[2] > cmp.cell[2]) return false;
    if (cell[2] < cmp.cell[2]) return true;

    // They're equal.
    return false;
  }
};

#endif // INTVECTOR_H
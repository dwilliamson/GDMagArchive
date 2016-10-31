// Don't include this, as it includes us! (we're a template.)
//#include <fluid/Utility/IntHash.h>

#include <gl/glut.h>
#include <math/Math.h>

template <class Value>
IntHash<Value>::IntHash()
{
  // This seemed to fix an MSVC 5.0 SP3 bug where if I didn't have this line it was just crashing
  // upon deleting arrays of type HashPack<Value>.  I guess it wasn't generating the sizeof correctly or something.
  // I love compiler bugs.
  int packSize = sizeof(HashPack<Value>);

  // Allocate one space for each table cell for starters.
  for (int x=0; x<NUM_HASH_BINS; x++)
  {
    table[x] = new HashPack<Value>[2];
    tableCapacity[x] = 2;
    tableSize[x] = 0;
  }
}

template <class Value>
IntHash<Value>::~IntHash()
{
  // Empty us out, we're going away.
  for (int x=0; x<NUM_HASH_BINS; x++)
  {
    delete[] table[x];
    tableCapacity[x] = 0;
    tableSize[x] = 0;
  }
}

// Wipes the hash table of values.
template <class Value>
void IntHash<Value>::clear()
{
  for (int x=0; x<NUM_HASH_BINS; x++)
  {
    tableSize[x] = 0;
  }
}

// Only call this for the value-less hashtables.
template <class Value>
__inline void IntHash<Value>::insert(unsigned short x, unsigned short y, unsigned short z)
{
  int cell = makeHashKey(x,y,z);
  int loc = makeRoom(cell);
  table[cell][loc].key = makeFullKey(x,y,z);
}

// Only call this for the valued hashtables (i.e. the ones with non-null value objects.)
template <class Value>
__inline void IntHash<Value>::insert(unsigned short x, unsigned short y, unsigned short z, const Value& val)
{
  int cell = makeHashKey(x,y,z);
  int loc = makeRoom(cell);
  table[cell][loc].key = makeFullKey(x,y,z);
  table[cell][loc].value = val;
}

// Fast lookup to see merely if it exists.
template <class Value>
__inline bool IntHash<Value>::doesExist(unsigned short x, unsigned short y, unsigned short z)
{
  int cell = makeHashKey(x,y,z);
  LargeInt key = makeFullKey(x,y,z);
  for (int lcv=0; lcv<tableSize[cell]; lcv++)
  {
    if (table[cell][lcv].key == key)
    {
      return true;
    }
  }
  return false;
}

// Lookup to also get the value of a lookup back.  Returns null if not found.
template <class Value>
__inline Value* IntHash<Value>::find(unsigned short x, unsigned short y, unsigned short z)
{
  int cell = makeHashKey(x,y,z);
  LargeInt key = makeFullKey(x,y,z);
  for (int lcv=0; lcv<tableSize[cell]; lcv++)
  {
    if (table[cell][lcv].key == key)
    {
      return &(table[cell][lcv].value);
    }
  }
  return 0;
}

// Used to form the hash key.
template <class Value>
__inline int IntHash<Value>::makeHashKey(unsigned short x, unsigned short y, unsigned short z)
{
  // We take a third the bits from each component.  We use the lower-order bits so it's like a mod-table
  // instead of clustering nearby cubelets into the same table location.
  int key;

  key = x & COMP_MASK;
  key <<= BITS_PER_COMP;

  key |= y & COMP_MASK;
  key <<= BITS_PER_COMP;

  key |= z & COMP_MASK;

  return key;
}

// Used to make the packed full key.
template <class Value>
__inline LargeInt IntHash<Value>::makeFullKey(unsigned short x, unsigned short y, unsigned short z)
{
  LargeInt key = x;
  key <<= 16;
  key |= y;
  key <<= 16;
  key |= z;
  return key;
}

// Used to grow a hash cell when needed.
template <class Value>
__inline int IntHash<Value>::makeRoom(int cellNum)
{
  if (tableSize[cellNum] >= tableCapacity[cellNum])
  {
    tableCapacity[cellNum] <<= 1;
    HashPack<Value>* newCell = new HashPack<Value>[tableCapacity[cellNum]];

    memcpy(newCell, table[cellNum], sizeof(HashPack<Value>)*tableSize[cellNum]);

    delete[] table[cellNum];
    table[cellNum] = newCell;
  }

  tableSize[cellNum]++;

  return tableSize[cellNum]-1;
}

// Used to display debug information about the hashtable (a small line-diagram of distribution.)
template <class Value>
void IntHash<Value>::drawDebugGraph(int x0, int y0, int x1, int y1)
{
  glPushAttrib   (GL_ENABLE_BIT | GL_TRANSFORM_BIT);
  
  glDisable      (GL_LIGHTING);
  glDisable(GL_BLEND);
  glDisable      (GL_DEPTH_TEST);
  glDisable      (GL_TEXTURE_2D);
  glDisable      (GL_TEXTURE_GEN_S);
  glDisable      (GL_TEXTURE_GEN_T);
  
  glBegin(GL_LINES);

  // 4 pixels == 2 pixels padding to the left and right of the bar graph for the outline border.
  int barRange = x1-x0-4;

  for (int x=0; x<barRange; x++)
  {
    int curLoc = (float)(x * getNumHashBins() / ((float)barRange));
    int nextLoc = (float)((x+1) * getNumHashBins() / ((float)barRange));

    nextLoc = Math::maxOf(nextLoc, getNumHashBins());

    float avgValue = 0;
    for (int y=curLoc; y<nextLoc; y++)
    {
      avgValue += getHashBucketSize(y);
    }
    avgValue /= (float)(nextLoc-curLoc);

    // Some semi-arbitrary scalar to make the bars big enough to see well.
    avgValue *= 32;

    // Make the bars be of alternating color so it doesn't look like one green blob.
    if (x % 4 == 0)
    {
      glColor3f(0,0,1);
    }
    if (x % 8 == 0)
    {
      glColor3f(0,1,0);
    }

    glVertex3f(x1 - x - 2, y0 + 2, 0);
    glVertex3f(x1 - x - 2, y0 + 2 + (int)avgValue, 0);
  }

  glColor3f(1,1,1);

  glVertex3f(x1,y0, 0);
  glVertex3f(x1,y1, 0);
  
  glVertex3f(x1,y0, 0);
  glVertex3f(x0,y0, 0);

  glVertex3f(x0,y0, 0);
  glVertex3f(x0,y1, 0);

  glEnd();
  
  glPopAttrib    () ;
}
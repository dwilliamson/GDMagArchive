#ifndef INTHASH_H
#define INTHASH_H

//
// IntHash
//
// IntHash is an extremely specific kind of hashtable -- it's written to hash values specified as a three-
// vector of unsigned shorts, used to specify locations in the cubelet-cell grid space.  These can then
// be easily reduced to a hash key used to do lookups into the growable array-buckets stored.
//
// When an array-bucket is too small, IntHash simply doubles its size.  It never deallocates memory.
// Wasteful?  Perhaps... but it should reach a maximum per-frame value and stabilize there; it won't just 
// keep growing.
//
// WARNING: The hash table doesn't check to see if a key already exists when it's told to insert it.
// If you insert the same key multiple times you could get weirdness.  Because the implicit surface stuff
// doesn't do this, it's not a problem, and this is far from a general-purpose container!!
//

// To maintain some semblance of cross-platform compatibility, I use this typedef to refer to a 64-bit
// int.  If your platform doesn't have such a thing, this code basically won't work.
typedef __int64 LargeInt;

template <class Value>
class HashPack
{
public:
  LargeInt key;
  Value value;
};

// Hardcode the size of the hash table; must be a multiple of 3 -- handy because then each of the x,y, and z
// can contribute the same amount (i.e. a 12-bit hash key is 4 bits from each component.)
const int NUM_HASH_BITS = 12;
const int NUM_HASH_BINS = 1<<NUM_HASH_BITS;

const int BITS_PER_COMP = NUM_HASH_BITS/3;
const int COMP_MASK = (1<<BITS_PER_COMP)-1;

// The hashtable class.
template <class Value>
class IntHash
{
public:
  IntHash();
  ~IntHash();

  // Wipes the hash table of values.
  void clear();

  // Utility functions used to build info about the distribution of items in the hash table.
  int getNumHashBins() { return NUM_HASH_BINS; }
  int getHashBucketSize(int index)
  {
    assert(index < NUM_HASH_BINS && index >= 0);
    return tableSize[index];
  }
  void drawDebugGraph(int x0, int y0, int x1, int y1);

  // Insert that doesn't set the value (the resulting value at this node is undefined.)
  __inline void insert(unsigned short x, unsigned short y, unsigned short z);

  // Value-setting insert (slower but sets the value.)
  __inline void insert(unsigned short x, unsigned short y, unsigned short z, const Value& val);

  // Fast lookup to see merely if it exists.
  __inline bool doesExist(unsigned short x, unsigned short y, unsigned short z);

  // Lookup to also get the value of a lookup back.  Returns null if not found.
  __inline Value* find(unsigned short x, unsigned short y, unsigned short z);

protected:
  // Used to form the hash key.
  __inline int makeHashKey(unsigned short x, unsigned short y, unsigned short z);

  // Used to make the packed full key.
  __inline LargeInt makeFullKey(unsigned short x, unsigned short y, unsigned short z);

  // Used to grow a hash cell when needed.
  __inline int makeRoom(int cellNum);

  int tableSize[NUM_HASH_BINS];
  int tableCapacity[NUM_HASH_BINS];
  HashPack<Value>* table[NUM_HASH_BINS];
};

// Argh, love templates.  At least this way I can put the inlines in the .cpp...
#include <fluid/Utility/IntHash.cpp>

#endif //INTHASH_H
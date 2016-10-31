/*
 *  dm_names - data manager NAMES
 *
 *  This is basically just a hash table from 
 *  string names to unique integer IDs
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <assert.h>

#include "dm_internal.h"
#include "dm_names.h"

// simple packed string allocator (unfreeable strings),
// which leave a 4-byte-aligned pointer right before the
// string

static Name **namelist;
static char *name_storage;
static int num_names, max_names, storage_left;

static Name *addName(char *str)
{
   Name *nwd;

   int n = strlen(str) + sizeof(Name);
   if (storage_left < n) {
      storage_left = 65536;
      if (storage_left < n) storage_left = n;
      name_storage = malloc(storage_left);
   }
   if (num_names >= max_names) {
      if (max_names == 0) {
         max_names = 1024;
         namelist = malloc(sizeof(*namelist) * max_names);
      } else {
         max_names *= 2;
         namelist = realloc(namelist, sizeof(*namelist) * max_names);
      }
   }

   nwd = (Name *) name_storage;

   strcpy(nwd->name, str);
   nwd->cached = NULL;
   nwd->tracked = NULL;

   namelist[num_names] = nwd;

   n = (n+3) & ~3;    // align pointer field of next allocation
   
   name_storage += n;
   storage_left -= n;

   ++num_names;

   return nwd;
}

char *dmStringFromID(Name *n)
{
   return n->name;
}

///////////////////////////////////////////////////////////

// simple string hash table layered on top of above

#ifdef HASH_POWER_OF_TWO
static int hash_table_size = (1 << 10);
#else
static int hash_table_size = 1087;  // use 3x-4 generator sequence, valid to 7,118,687
#endif

static Name **hash_table;
static int hash_table_filled;
static int hash_table_limit;

// circular shift hash -- produces good results if modding by a prime;
// longword at a time would be faster (need alpha-style "is any byte 0"),
// or just use the first longword
static unsigned int hash(char *str)
{
   unsigned char *n = (unsigned char *) str;
   unsigned int acc = 0x55555555;
   while (*n)
      acc = (acc >> 27) + (acc << 5) + *n++;
   return acc;
}


#ifdef HASH_POWER_OF_TWO
   #define HASH_MOD(x,size)   ((x) & ((size)-1))
   #define STEP(h, size)      ((((x) >> 8) ^ (x)) & ((size)-2))
#else
   #define HASH_MOD(x,size)   ((x) % (size))
   #define STEP(h, size)      ((x) % ((size)-1) + 1)
#endif

void dmNamesInit(void)
{
   int s;
   int i;
   hash_table_filled = 0;
   hash_table_limit = (int) (hash_table_size * 0.8f);
   s = sizeof(*hash_table) * hash_table_size;
   hash_table = malloc(s);
   for (i=0; i < hash_table_size; ++i)
      hash_table[i] = NULL;
}

void dmNamesIter(void (*func)(Name *, void *), void *data)
{
   int i;
   for (i=0; i < hash_table_size; ++i)
      if (hash_table[i] != NULL)
         func(hash_table[i], data);
}

static void growHashTable(void);
static Name *findString(char *str, Name *slot)
{
   unsigned int h = hash(str);
   unsigned int x, step;
   Name *z;

   x = HASH_MOD(h, hash_table_size);

   z = hash_table[x];
   if (z != NULL) {
      if (strcmp(z->name, str) == 0)
         return z;

      // don't bother computing the hash step size for internal chaining
      // until it's needed
      step = STEP(h, hash_table_size);
      while(1) {
         x = HASH_MOD(x+step, hash_table_size);
         z = hash_table[x];
         if (z == NULL) break;
         if (strcmp(z->name, str) == 0)
            return z;
      }
   }

   // add to string table if not already there
   // [it's already there if we're inside nested down a level
   // inside the call to growHashTable() below]
   if (slot == NULL)
      slot = addName(str);

   // insert at x
   hash_table[x] = slot;

   ++hash_table_filled;
   if (hash_table_filled >= hash_table_limit)
      growHashTable();

   return slot;
}

static void growHashTable(void)
{
   int x,n = hash_table_size;
   Name **old_hash = hash_table;

   #ifdef HASH_POWER_OF_TWO
      hash_table_size *= 2;
   #else
      hash_table_size = (hash_table_size * 3 - 4); // 3x-4 generator sequence
      assert(hash_table_size <= 7118687);
   #endif

   dmNamesInit(); // reallocate hash table to new size

   // copy old hash table entries to new one
   for (x=0; x < n; ++x) {
      Name *z = old_hash[x];
      if (z)
         findString(z->name, z);
   }
   free(old_hash);
}
   
Name *dmStringID(char *str)
{
   return findString(str, NULL);
}

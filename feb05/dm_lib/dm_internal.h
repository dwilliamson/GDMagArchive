#ifndef INC_DM_INTERNAL_H
#define INC_DM_INTERNAL_H

#include "dm_hand.h"

typedef struct st_Name Name;

typedef struct
{
   int typecode;
   int dependent_typecode;
   int extension;
   void * (*loadFromName)(char *name);
   void * (*loadFromFile)(FILE *f, int len);
   void * (*postloadProcessing)(void *data, int src_len, void *param);
   void   (*unload)(void *data);
} DMFormatHandler;

// if we try to free a cached data item by name, we won't
// know which directory it comes from. but that's fine, because
// we won't be untracking it, just unloading it

typedef struct st_DMCachedData DMCachedData;
typedef struct st_DMTrackedData DMTrackedData;

struct st_DMCachedData
{
   void *p;
   DMFormatHandler *h;
   DMTrackedData *source;
   int lock;     // @TODO refcount active references to this... not implemented

   DMCachedData *next_by_name, *prev_by_name;
   DMCachedData *next_by_type, *prev_by_type;
};

// for every tracked item, we need to store its Name (so we can
// unthread it from its namelist if we close the tracked directory),
// its directory (so we can find it), its extension (so we can open
// it as a file and infer its type), and its offset (to avoid searching
// if it's in a packed directory)
//
// ok, we can get by without storing the name if we keep the linked
// list of all tracked copies of this name circular? that's a problem
// for deleting the header, unless we keep an extra sentinel tracked item
// around, but that's even MORE wasteful of space I imagine.
//
// note size matters here since we track _all_ visible data, not just
// the loaded ones.

struct st_DMTrackedData
{
   Name *name;
   struct st_DMTrackedData *next_ofname;  // next item with this same name
   unsigned short dir;          // internal code for directory/packed directory file
   unsigned short extension;    // internal unique code for extensions
   unsigned short off;          // index into compressed directory of packed file
   short update_code;           // callback handle if data changes
};

// each directory will keep an array of DMTrackedData pointers,
// rather than a threaded linked list, since this isn't normally
// updated on the fly (it is on hotloading, but that's not the
// normal user path)

struct st_Name
{
   DMTrackedData *tracked;
   DMCachedData *cached;
   char name[1];
};

// dm_hand
extern DMCachedData *dmLoadDataAsType(char *file, int type, int ext, void *param);
extern DMCachedData *dmLoadUntracked(Name *name, int typecode, void *param);
extern void          dmUnload(DMCachedData *z);

// dm_track
typedef void (*DMCallback)(char *str);
extern DMCachedData *dmFindDataObject(Name *name, int typecode, void *param);
extern int           dmExtensionID(char *ext);
extern int           dmGetCallbackHandle(DMCallback func);

// dm_names
void dmNamesIter(void (*func)(Name *, void *), void *);

#endif

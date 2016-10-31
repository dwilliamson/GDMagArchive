/*
 *  dm_track - data manager TRACKER
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <assert.h>
#include <ctype.h>

#include "dm.h"
#include "dm_internal.h"

#define MAX_DIRECTORIES 256    // I am a lazy C programmer

enum DMDirectoryType
{
   DM_DIR_DIRECTORY,
   DM_DIR_PACKED
};

#define MAX_EXTENSIONS  256
static char extensions[MAX_EXTENSIONS][6];
static int num_extensions;

int dmExtensionID(char *ext)
{
   int i;
   for (i=0; i < num_extensions; ++i)
      if (strcmp(extensions[i], ext) == 0)
         return i;
   assert(num_extensions < MAX_EXTENSIONS);
   strcpy(extensions[num_extensions], ext);
   return num_extensions++;
}

typedef struct Directory
{
   enum DMDirectoryType type;
   char *dir;
   int num_items;
   int max_items;   // while scanning
   DMTrackedData *itemlist[1];
} Directory;

static Directory *directories[MAX_DIRECTORIES];
static int num_directories;

static Directory *allocDirectory(enum DMDirectoryType t, int n)
{
   Directory *d = malloc(sizeof(*d) + (n-1) * sizeof(d->itemlist[0]));
   d->max_items = n;
   d->num_items = 0;
   d->dir = NULL;
   d->type = t;
   return d;
}

static Directory *growDirectory(Directory *d)
{
   Directory *e;
   int n = d->max_items * 2;
   if (n <= 0) n = 2;
   e = realloc(d, sizeof(*e) + (n-1) * sizeof(d->itemlist[0]));
   e->max_items = n;
   return e;
}

static void lowercase(char *s)
{
   while (*s) {
      *s = tolower(*s);
      ++s;
   }
}

enum
{
   ID_INVALID,
   ID_VALID,
   ID_PACKED_FILE,
   ID_TEXT,
};

static int filenameToIDs(Name **nameid, int *extensionid, char *file)
{
   char name[_MAX_FNAME], ext_data[_MAX_EXT], *ext;
   _splitpath(file, NULL, NULL, name, ext_data);
   lowercase(name);
   lowercase(ext_data);
   ext = ext_data;
   if (ext[0] == '.') ++ext;
   if (ext[0] == 0) return ID_INVALID;
   if (strcmp(ext, "box") == 0) {
      return ID_PACKED_FILE;
   }

   *nameid = dmStringID(name);
   *extensionid = dmExtensionID(ext);
   return ID_VALID;
}

static Directory *addFileToDirectory(Directory *d, char *file, int dir)
{
   DMTrackedData *t;
   int extensionid, result;
   Name *nameid;

   result = filenameToIDs(&nameid, &extensionid, file);

   if (result == ID_INVALID) {
      return d;
   }

   if (result == ID_PACKED_FILE) {
      // @TODO: recursively parse the packed file
      // flag it for future reference to rescan it too
      return d;
   }

   t = malloc(sizeof(*t));

   t->dir       = dir;
   t->name      = nameid;
   t->extension = extensionid;
   t->off       = 0xffff;
   t->update_code = -1;

   if (d->num_items >= d->max_items)
      d = growDirectory(d);

   d->itemlist[d->num_items++] = t;
   return d;
}

static Directory * scanFilesystemDirectory(char *dir, int id)
{
   char buffer[256];
   struct _finddata_t f;
   Directory *d;
   long z;

   sprintf(buffer, "%s/*.*", dir);
   z = _findfirst(buffer, &f);
   if (z == -1) return NULL;

   d = allocDirectory(DM_DIR_DIRECTORY, 16);
   d->dir = _strdup(dir);
   
   do {
      if ((f.attrib & _A_SUBDIR) == 0)
         d = addFileToDirectory(d, f.name, id); 
      // @TODO: could recurse down subdirectories... I don't bother to because
      // I want more control, but on a larger project it's probably the right
      // thing to do... you'll and to call dmOpenDirectory, but you may need to
      // be careful about how they use global variables to allocate things. the
      // current system grabs this unique id 'id' and scans using it, but doesn't
      // _allocate_ the id until it sees the scan succeeded. a naive recursive scan
      // would keep reallocating the same one. just change it to allocating before
      // calling this
   } while (_findnext(z, &f) == 0);

   _findclose(z);

   return d;
}

static void addTrackedItem(DMTrackedData *t)
{
   t->next_ofname = t->name->tracked;
   t->name->tracked = t;
}

static void removeTrackedItem(DMTrackedData *t)
{
   DMTrackedData **z = &t->name->tracked;
   while (*z != t)
      z = &((*z)->next_ofname);
   *z = t->next_ofname;
}

static DMCachedData *loadTrackedAsType(DMTrackedData *p, int type, void *param)
{
   DMCachedData *c;
   char buffer[256];
   // @TODO: box file support
   sprintf(buffer, "%s/%s.%s", directories[p->dir]->dir, dmStringFromID(p->name), extensions[p->extension]);
   c = dmLoadDataAsType(buffer, type, p->extension, param);
   if (c != NULL)
      c->source = p;
   return c;
}

static int trackedCompare(const void *p, const void *q)
{
   DMTrackedData *a = (DMTrackedData *) p;
   DMTrackedData *b = (DMTrackedData *) q;
   if (a->dir < b->dir) return -1;
   return a->dir > b->dir;
}

DMCachedData *dmFindDataObject(Name *name, int type, void *param)
{
   DMTrackedData *t;
   DMCachedData *p;
   
   for (t = name->tracked; t != NULL; t = t->next_ofname) {
      p = loadTrackedAsType(t, type, param);
      if (p) return p;
   }

   p = dmLoadUntracked(name, type, param);

   return p;
}

static void addDirectoryToGlobalHash(Directory *d)
{
   int i;
   for (i=0; i < d->num_items; ++i)
      addTrackedItem(d->itemlist[i]);
}

static void removeDirectoryFromGlobalHash(Directory *d)
{
   int i;
   for (i=0; i < d->num_items; ++i)
      removeTrackedItem(d->itemlist[i]);
}

int dmOpenDirectory(char *dir)
{
   int z = num_directories;
   Directory *q = scanFilesystemDirectory(dir, z);
   if (q == NULL) return -1;
   ++num_directories;  // allocate the id if the directory wasn't empty
   directories[z] = q;
   addDirectoryToGlobalHash(q);

#ifdef WIN32
   {
      extern void osMonitorDirectory(int z, char *dir);
      osMonitorDirectory(z, dir);
   }
#endif
   return z;
}

#define MAX_CALLBACKS 32
static DMCallback callbacks[MAX_CALLBACKS];

int dmGetCallbackHandle(DMCallback func)
{
   int i;
   if (func == NULL) return -1;
   for (i=0; i < MAX_CALLBACKS; ++i) {
      if (callbacks[i] == NULL) callbacks[i] = func;
      if (callbacks[i] == func)
         return i;
   }
   assert(i != MAX_CALLBACKS);
   return -1;
}

void dmUpdateFile(int dir_handle, char *filename)
{
   int result, ext;
   Name *name;
   DMTrackedData *t;
   DMCachedData *c;
   result = filenameToIDs(&name, &ext, filename);
   if (result != ID_VALID) return;

   for (t = name->tracked; t != NULL; t = t->next_ofname)
      if (t->dir == dir_handle && t->extension == ext)
         break;
   if (t == NULL) {
      // @TODO: add a new resource
      return;
   }

   // free existing entries with this filename and extension (which
   // will force the manager to lazily load it from disk next time it's
   // used)

   // @TODO: free anything that is of the same type even if it has
   // a different extension--this is needed so if you add a new
   // resource that overrides what's already loaded, either a diff
   // file extension or in a different dir
   for (c = name->cached; c != NULL; c = c->next_by_name) {
      if (c->source->dir == dir_handle && c->source->extension == ext) {
         dmFreeID(name, c->h->typecode);
         return;
      }
   }
   if (t->update_code >= 0)
      callbacks[t->update_code](dmStringFromID(name));
}

// load data yourself bypassing the cache
void *dmUncachedLoad(char *name, int typecode, void *param)
{
   DMCachedData *c = dmFindDataObject(dmStringID(name), typecode, param);
   void *p;
   if (c == NULL) return NULL;
   p = c->p;
   free(c);
   return p;
}

// load data yourself and install your own callback to get called when the data changes
void *dmUncachedLoadCallback(char *name, int typecode, void *param, DMCallback func)
{
   DMCachedData *c = dmFindDataObject(dmStringID(name), typecode, param);
   void *p;
   if (c == NULL) return NULL;
   if (c->source)  // there's no source if it's untracked (algorithmic)
      c->source->update_code = dmGetCallbackHandle(func);
   p = c->p;
   free(c);
   return p;
}

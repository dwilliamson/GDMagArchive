/*
 *  dm_hand - data manager handler
 *
 *  This is "resource manager" which loads data resources
 *  into memory via name or handle
 *
 *     handler -- this allows different resource types to be loaded
 *                and managed differently. the swapper indirects through
 *                the handler in a fixed way, determined at tracking time
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "dm_internal.h"
#include "dm_hand.h"
#include "dm_names.h"
#include "dm_swap.h"

#define MAX_FORMATS     64    // 64 different loaders ought to be enough for anyone
                              // (I am a lazy C programmer)

static DMFormatHandler formats[MAX_FORMATS];
static int num_formats;

void dmInit(void)
{
   dmNamesInit();
}

void dmRegisterFormatHandler(int typecode, int dependent_typecode,
                             char *extension,
                             dmLoadFromNameFP loadFromName,
                             dmLoadFromFileFP loadFromFile,
                             dmPostloadProcessingFP postloadProcessing,
                             dmUnloadFP unload)
{
   int n;
   assert(num_formats < MAX_FORMATS);
   n = num_formats++;

   formats[n].typecode = typecode;
   formats[n].dependent_typecode = dependent_typecode;
   formats[n].extension = extension ? dmExtensionID(extension) : -1;
   formats[n].loadFromName = loadFromName;
   formats[n].loadFromFile = loadFromFile;
   formats[n].postloadProcessing = postloadProcessing;
   formats[n].unload = unload;
}

static int filelength(char *n)
{
   FILE *f = fopen(n, "rb");
   int z;
   if (!f) return -1;
   fseek(f, 0, SEEK_END);
   z = ftell(f);
   fclose(f);
   return z;
}

int dmLoadFileToBuffer(char *filename, void **buffer)
{
   int len = filelength(filename);
   if (len >= 0) {
      FILE *f = fopen(filename, "rb");
      *buffer = malloc(len+1);
      fread(*buffer, 1, len, f);
      fclose(f);
   }
   return len;
}

static void *doLoad(DMFormatHandler *h, FILE *f, int len, void *param)
{
   void *p;
   if (h->loadFromFile) { 
      // if the manager has a loader from an open file, pass that in
      p = h->loadFromFile(f, len);
   } else {
      // otherwise malloc a temporary buffer, and read it into that
      p = malloc(len+1);    // add extra space for a sentinel
      fread(p, len, 1, f);
      // most likely the loader will actually convert the data in postloadProcessing,
      // but if the app just wants to use it naively, we're all done
   }
   if (p == NULL) return NULL;
   if (h->postloadProcessing) {
      // postload must free old data if necessary!
      p = h->postloadProcessing(p, len, param);
   }
   return p;
}

// this internal function creates an entry to remember we have cached data for p
static DMCachedData *makeCached(DMFormatHandler *h, void *p)
{
   DMCachedData *z = malloc(sizeof(*z));
   z->h = h;
   z->p = p;
   z->source = NULL;
   return z;
}

// load data and create a cached-data entry for it...
// this is where you would read from a packed file (well, you'd
// need a different interface than 'filename', so it would actually
// be the caller of this that would choose)
static DMCachedData *loadDataAsFormat(char *filename, DMFormatHandler *h, void *param)
{
   void *q;
   FILE *f = fopen(filename, "rb");
   if (!f) return NULL;
   q = doLoad(h, f, filelength(filename), param);
   fclose(f);
   if (q)
      return makeCached(h, q);
   return NULL;
}

static void doUnload(DMFormatHandler *h, void *p)
{
   if (h->unload)
      h->unload(p);
   else
      free(p);
}

void dmUnload(DMCachedData *z)
{
   doUnload(z->h, z->p);
   free(z);
}

// used to have lists of extensions per loader, but got rid of that code
static int isExtensionInList(int ext, DMFormatHandler *f)
{
   if (f->extension == ext) return 1;
   return 0;
}

// load a dependent datatype. we've loaded the underlying data into z.
// note that we don't actually cache that data separately, despite what
// I wrote in my column. Oops. Instead, we load it uncached (in the next
// file), then we build the dependent data from it, then we unload the
// old one, then we overwrite it with the dependent data.
static DMCachedData *loadDependent(DMCachedData *z, DMFormatHandler *h, char *file, void *param)
{
   void *p = z->p, *q;
   assert(h->postloadProcessing);
   q = h->postloadProcessing(p, file ? filelength(file) : -1, param);
   if (q != p)
       doUnload(z->h, z->p);
   z->h = h;
   z->p = q;
   return z;
}

// iterate through all the loaders looking for one that matches. no need
// for speed since this is the uncached path.
DMCachedData *dmLoadDataAsType(char *file, int type, int ext, void *param)
{
   int i;
   for (i=0; i < num_formats; ++i) {
      if (formats[i].typecode == type) {
         if (formats[i].dependent_typecode > 0) {
            // if it's dependent, do a recursive load to try to get the underlying item
            DMCachedData *z = dmLoadDataAsType(file, formats[i].dependent_typecode, ext, NULL);
            if (z)
               return loadDependent(z, &formats[i], file, param);
         } else if (isExtensionInList(ext, &formats[i])) {
            DMCachedData *z = loadDataAsFormat(file, &formats[i], param);
            if (z) return z;
         }
      } 
   }
   return NULL;
}

// iterate through all the loaders looking for one that matches. no need
// for speed since this is the uncached path.
DMCachedData *dmLoadUntracked(Name *name, int typecode, void *param)
{
   int i;
   for (i=0; i < num_formats; ++i) {
      if (formats[i].typecode == typecode && formats[i].loadFromName) {
         DMFormatHandler *h = &formats[i];
         void *p = h->loadFromName(name->name);
         if (p)
            return makeCached(h, p);
      }
   }
   for (i=0; i < num_formats; ++i) {
      if (formats[i].typecode == typecode && formats[i].dependent_typecode > 0) {
         DMCachedData *z = dmLoadUntracked(name, formats[i].dependent_typecode, NULL);
         if (z) {
            return loadDependent(z, &formats[i], NULL, param);
         }
      }
   }
   return NULL;
}

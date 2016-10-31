#include <stdlib.h>
#include <assert.h>
#include "dm_internal.h"
#include "dm_track.h"
#include "dm_names.h"
#include "dm_swap.h"

static DMCachedData *findDataObject(Name *name, int type, void *param)
{
   DMCachedData *q;
   
   for (q = name->cached; q != NULL; q = q->next_by_name) {
      if (q->h->typecode == type)
         return q;
   }

   q = dmFindDataObject(name, type, param);
   if (q == NULL) return q;

   q->next_by_name = name->cached;
   q->prev_by_name = NULL;
   if (name->cached != NULL)
      name->cached->prev_by_name = q;
   name->cached = q;

   q->next_by_type = q->prev_by_type = NULL;
   q->lock = 0;

   return q;
}

static void freeDataObject(Name *name, int type, int unload)
{
   DMCachedData *z;
   for (z = name->cached; z != NULL; z = z->next_by_name) {
      if (z->h->typecode == type) {
         if (z->prev_by_name) z->prev_by_name->next_by_name = z->next_by_name;
         else name->cached = z->next_by_name;
         if (z->next_by_name) z->next_by_name->prev_by_name = z->prev_by_name;

         if (unload)
            dmUnload(z);
         return;
      }
   }
}

static void dmFreeTypeRaw(Name *name, void *p)
{
   freeDataObject(name, *(int *)p, 1);
}

void dmFreeType(int type)
{
   dmNamesIter(dmFreeTypeRaw, &type);
}

void *dmFindID(Name *name, int type)
{
   DMCachedData *p = findDataObject(name, type, NULL);
   if (p == NULL) return NULL;
   return p->p;
}

void *dmFind(char *name, int type)
{
   return dmFindID(dmStringID(name), type);
}

void *dmFindIDExtra(Name *name, int type, void *param)
{
   DMCachedData *p = findDataObject(name, type, param);
   if (p == NULL) return NULL;
   return p->p;
}

void *dmFindExtra(char *name, int type, void *param)
{
   return dmFindIDExtra(dmStringID(name), type, param);
}

void dmFreeID(Name *name, int type)
{
   freeDataObject(name, type, 1);
}

void dmFree(char *name, int type)
{
   dmFreeID(dmStringID(name), type);
}

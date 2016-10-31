// DMTYPES.H
//
//   This "user" file defines the data types exposed by
//   the datamanager. It's necessary to have this global header
//   file so that enum DatamanTypes is actually unique. Maybe
//   a more clever way to do this would be to use Jon Blow's
//   profiling technique: declare a global variable for each
//   unique type, use the address of that global variables as
//   the symbolic name, and rely on the linker to generate the
//   unique names.

#ifndef INC_DMTYPES_H
#define INC_DMTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dm.h"

enum DatamanTypes
{
   DT_NULL=0,

   DT_FONT,
   DT_IMAGE,
   DT_TEX,
   DT_MODEL,
   DT_STRINGS,
};

extern void stringsFree(char **str);  // not one single block, sadly

// the loader function for DT_STRINGS, without going through the dataman,
extern char **loadStringsFromFile(char *filename);

// C-style forward declares
typedef struct st_Model Model;
typedef struct st_Image Image;
typedef struct st_Font Font;

#define getModel(x)    ((Model   *) dmFindID((x), DT_MODEL))
#define getTexture(x)  ((GLuint   ) dmFindID((x), DT_TEX  ))
#define getImage(x)    ((Image   *) dmFind((x), DT_IMAGE))
#define getFont(x)     ((Font    *) dmFind((x), DT_FONT ))
#define getStrings(x)  ((char   **) dmFind((x), DT_STRING_ARRAY)

// this is an optional data structure used to alter models while
// they're being loaded. it could probably be done later in the
// pipeline, but this allows the whole thing to be subsumed into
// the data manager pipeline, so you can just use the model somewhere
// and it's automatically altered appropriately. Otherwise we'd have
// to have somewhere that explicitly loads the models and "preps" them.

typedef struct
{
   float scale;
   float x_scale, y_scale, z_scale;

   float pitch;      // rotation about x axis in degrees

   int x_flip, y_flip, z_flip;
   int reverse_winding;

   int share_normals;  // facetify or not
   int raise_z;

   int max_submodels;

} ModelLoadData;

#ifdef __cplusplus
}
#endif

#endif

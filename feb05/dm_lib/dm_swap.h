// Data Manager swapping subsystem
// this is what you call to actually get at data


#ifndef INC_DM_SWAP_H
#define INC_DM_SWAP_H


typedef struct st_Name Name;

// this interface will hash-lookup the string every time you call
extern  void *  dmFind       (char *name, int type);
extern  void *  dmFindExtra  (char *name, int type, void *param);

// this interface allows you to pre-hash-lookup the string
extern  void *  dmFindID     (Name *name, int type);
extern  void *  dmFindIDExtra(Name *name, int type, void *param);

// this interface forces an object to unload
extern  void    dmFree       (char *name, int type);
extern  void    dmFreeID     (Name *name, int type);

// free all objects of this datatype
// (e.g. free all textures on graphics mode changes)
extern void   dmFreeType(int type);

#endif

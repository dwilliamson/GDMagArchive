#ifndef INC_DM_TRACK_H

typedef struct st_Name Name;

// open a directory as a resource source; returns an internal handle
extern int dmOpenDirectory(char *dir);

// tell the dm system that a file has changed in the given directory
// to allow on-the-fly resource updating (used internally by the
// win32 file-watching code, but reusable on other OSes)
extern void dmUpdateFile(int dir_handle, char *filename);

// load a resource without going through the cache (must free it yourself)
// optionally with a callback that's called with the resource name if
// it updates on disk
typedef void (*DMCallback)(char *str);
extern void *dmUncachedLoad        (char *name, int typecode, void *param);
extern void *dmUncachedLoadCallback(char *name, int typecode, void *param, DMCallback func);

// utility function: load a file into an allocated buffer
extern int   dmLoadFileToBuffer(char *filename, void **buffer);

#endif

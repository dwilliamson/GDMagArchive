#ifdef WIN32

#include <assert.h>
#define _WIN32_WINNT 0x450
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "dm_track.h"

typedef unsigned char uint8;


#define MAX_DIRECTORIES  128

static uint8 buffer[MAX_DIRECTORIES][256];
static HANDLE hDir[MAX_DIRECTORIES];
static OVERLAPPED overlapped_buffer[MAX_DIRECTORIES];

static void startMonitoring(int z);

VOID CALLBACK monitorRoutine(DWORD error, DWORD numBytes, LPOVERLAPPED overlapped)
{
   char filename[128];
   int d = (int) overlapped->hEvent;
   FILE_NOTIFY_INFORMATION *p = (FILE_NOTIFY_INFORMATION *) buffer[d];
   assert(error == 0);

   while (1) {
      if (p->Action == FILE_ACTION_MODIFIED || p->Action == FILE_ACTION_ADDED || p->Action == FILE_ACTION_RENAMED_NEW_NAME) {
         unsigned int i;
         // moronic unicode(?) conversion
         for (i=0; i < p->FileNameLength/2; ++i)
            filename[i] = (char) p->FileName[i];
         filename[i] = 0;
         dmUpdateFile(d, filename);
      }
      // what to do if it's deleted? we should probably uncache it too, but let's
      // just bail instead

      if (p->NextEntryOffset == 0)
         break;
      p = (FILE_NOTIFY_INFORMATION *) ((char *) p + p->NextEntryOffset);
   }

   // have to restart watching this directory
   startMonitoring(d);
} 

static void startMonitoring(int z)
{
   BOOL temp;
   overlapped_buffer[z].hEvent = (void *) z;
   temp = ReadDirectoryChangesW(hDir[z], buffer[z], sizeof(buffer[z]), FALSE,
                 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                 NULL, &overlapped_buffer[z], monitorRoutine);
}

void osMonitorDirectory(int z, char *dir)
{
   assert(z < MAX_DIRECTORIES);
   hDir[z] = CreateFile(
      dir,                                // pointer to the file name
      FILE_LIST_DIRECTORY,                // access (read-write) mode
      FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,  // share mode
      NULL,                               // security descriptor
      OPEN_EXISTING,                      // how to create
      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,         // file attributes
      NULL                                // file with attributes to copy
   );
   startMonitoring(z); 
}

#endif

// Win32 version

#ifdef _USRDLL
#define LEXPORT
#define LEXPORT_DATA __declspec(dllimport)
#else
#define LEXPORT __declspec(dllexport) 
#define LEXPORT_DATA __declspec(dllexport) 
#endif

#define DLL_EXPORT __declspec(dllexport)

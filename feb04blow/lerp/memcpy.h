// I include mempcy and memset here, instead of including string.h.
// That's because hey, we want to use memcpy and memset, but we want to
// NOT include the definitions of things like strlen().  If we include
// those we might accidentally use them, and the goal is to keep this
// program Unicode-clean.

// @OS: These are made to match what's in Windows' <string.h>; eventually
// we will fork this file for each platform.

extern "C" {
void *  __cdecl memcpy(void *, const void *, size_t);
void *  __cdecl memset(void *, int, size_t);
};


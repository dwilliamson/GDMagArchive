// mprintf()
//
// malloc()s a required amount of space for a string, where
// the initial guess is expanded until the string fits.  
//
// mprintf  -->  returns NULL on failure
//
// remember to free the result!

const int MPRINTF_INITIAL_GUESS = 256;

#include <stdarg.h>
char *mprintf(char *fmt, ...);

// If you are calling this via indirection from another varargs routine...
char *mprintf_valist(char *fmt, va_list arg_list);

// You can also pass a differing initial guess, if you want.
char *mprintf(int initial_guess, char *fmt, ...);



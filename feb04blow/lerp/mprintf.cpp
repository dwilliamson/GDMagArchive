#include "general.h"

#include "mprintf.h"
#include <stdio.h>
#include <stdarg.h>


// @Refactor: This vsnprintf definition is Windows-specific
#define vsnprintf _vsnprintf

/*
// SPECIAL NOTE: vsnprintf is specified slightly 
// differently on different systems.  This is why
// the check for success is so explicit....
*/

char *mprintf(char *fmt, ...) {
    char *res = NULL;
    int size = MPRINTF_INITIAL_GUESS;

    while (1) {
        res = new char[size];
		if (!res) return NULL;

		va_list ap;
		va_start(ap, fmt);

		int len = vsnprintf(res, size, fmt, ap);
		va_end(ap);

		if ((len >= 0) && (size >= len + 1)) {
			size = len;
			break;
		}

		delete [] res;
		size *= 2;							
    }

    return res;
}

char *mprintf_valist(char *fmt, va_list ap_orig) {
    char *res = NULL;
    int size = MPRINTF_INITIAL_GUESS;

    while (1) {
        res = new char[size];
		if (!res) return NULL;

		va_list ap;
        ap = ap_orig;

		int len = vsnprintf(res, size, fmt, ap);
		va_end(ap);

		if ((len >= 0) && (size >= len + 1)) {
			size = len;
			break;
		}

		delete [] res;
		size *= 2;							
    }

    return res;
}

// The below function is mostly a cut-and-paste of the above.
// That ought to be cleaned up at some point.  XXX
char *mprintf(int size, char *fmt, ...) {
    assert(size > 0);

    char *res = NULL;

    while (1) {
        res = new char[size];
		if (!res) return NULL;

		va_list ap;
		va_start(ap, fmt);

		int len = vsnprintf(res, size, fmt, ap);
		va_end(ap);

		if ((len >= 0) && (size >= len + 1)) {
			size = len;
			break;
		}

		delete [] res;
		size *= 2;							
    }

    return res;
}


#include "unicode.h"

typedef char Text_Utf8;  // @Refactor: This is also defined in general.h but I didn't want to include that.

// This code is split into a separate file from unicode.cpp to help keep
// copyright issues clean.

bool Unicode::strings_match(Text_Utf8 *a, Text_Utf8 *b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++;
        b++;
    }

    if (*a != *b) return false;
    return true;
}

int Unicode::size_in_bytes(Text_Utf8 *s) {
    char *t = s;
    while (*t) t++;
    return (t - s) + 1;  // Include the zero-termination.
}

int Unicode::length_in_characters(Text_Utf8 *s) {
    int length = 0;

    unsigned char *t = (unsigned char *)s;  // So we can use it to index an array nicely...

    while (*t) {
        int bytes_for_this_character = 1 + Unicode::trailingBytesForUTF8[*t];
        length++;
        t += bytes_for_this_character;
    }

    return length;
}

/*
 * Copyright 2001 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
	Author: Mark E. Davis, 1994.
	Rev History: Rick McGowan, fixes & updates May 2001.
	Sept 2001: fixed const & error conditions per
		mods suggested by S. Parent & A. Lillich.

------------------------------------------------------------------------ */


#include "unicode.h"
#ifdef CVTUTF_DEBUG
#include <stdio.h>
#endif

static const int halfShift	= 10; /* used for shifting by 10 bits */

static const UTF32 halfBase	= 0x0010000UL;
static const UTF32 halfMask	= 0x3FFUL;

#define UNI_SUR_HIGH_START	(UTF32)0xD800
#define UNI_SUR_HIGH_END	(UTF32)0xDBFF
#define UNI_SUR_LOW_START	(UTF32)0xDC00
#define UNI_SUR_LOW_END		(UTF32)0xDFFF
#define false			0
#define true			1


/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 */

const char Unicode::trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
					 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... six byte sequence.)
 */
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };



/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */

Conversion_Result Unicode::string_utf16_to_utf8(
    const UTF16** sourceStart, const UTF16* sourceEnd, 
    UTF8** target_start, UTF8* targetEnd, ConversionFlags flags) {


	Conversion_Result result = UNICODE_CONVERSION_OK;
	const UTF16* source = *sourceStart;
	UTF8 *target = *target_start;
	while (source < sourceEnd) {
		UTF32 ch;
		unsigned short bytes_to_write = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80; 
		const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
		ch = *source++;
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END && source < sourceEnd) {
			UTF32 ch2 = *source;
			if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
				ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
					+ (ch2 - UNI_SUR_LOW_START) + halfBase;
				++source;
			} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
				--source; /* return to the illegal value itself */
				result = UNICODE_SOURCE_ILLEGAL;
				break;
			}
		} else if ((flags == strictConversion) && (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END)) {
			--source; /* return to the illegal value itself */
			result = UNICODE_SOURCE_ILLEGAL;
			break;
		}
		/* Figure out how many bytes the result will require */
		if (ch < (UTF32)0x80) {			bytes_to_write = 1;
		} else if (ch < (UTF32)0x800) {		bytes_to_write = 2;
		} else if (ch < (UTF32)0x10000) {	bytes_to_write = 3;
		} else if (ch < (UTF32)0x200000) {	bytes_to_write = 4;
		} else {				bytes_to_write = 2;
							ch = UNI_REPLACEMENT_CHAR;
		}

		target += bytes_to_write;
		if (target > targetEnd) {
			source = oldSource; /* Back up source pointer! */
			target -= bytes_to_write; result = UNICODE_TARGET_EXHAUSTED; break;
		}
		switch (bytes_to_write) {	/* note: everything falls through. */
			case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 1:	*--target =  ch | firstByteMark[bytes_to_write];
		}
		target += bytes_to_write;
	}
	*sourceStart = source;
	*target_start = target;
	return result;
}



/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *	length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

static Boolean isLegalUTF8(const UTF8 *source, int length) {
	UTF8 a;
	const UTF8 *srcptr = source+length;
	switch (length) {
	default: return false;
		/* Everything else falls through when "true"... */
	case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 2: if ((a = (*--srcptr)) > 0xBF) return false;
		switch (*source) {
		    /* no fall-through in this inner switch */
		    case 0xE0: if (a < 0xA0) return false; break;
		    case 0xF0: if (a < 0x90) return false; break;
		    case 0xF4: if (a > 0x8F) return false; break;
		    default:  if (a < 0x80) return false;
		}
    	case 1: if (*source >= 0x80 && *source < 0xC2) return false;
		if (*source > 0xF4) return false;
	}
	return true;
}



/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(const UTF8 *source, const UTF8 *sourceEnd) {
	int length = Unicode::trailingBytesForUTF8[*source]+1;
	if (source+length > sourceEnd) {
	    return false;
	}
	return isLegalUTF8(source, length);
}



Conversion_Result Unicode::string_utf8_to_utf16(
    const UTF8** sourceStart, const UTF8* sourceEnd, 
    UTF16** target_start, UTF16* targetEnd, ConversionFlags flags) {


	Conversion_Result result = UNICODE_CONVERSION_OK;
	const UTF8 *source = *sourceStart;
	UTF16* target = *target_start;
	while (source < sourceEnd) {
		UTF32 ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			result = UNICODE_SOURCE_EXHAUSTED; break;
		}
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1)) {
			result = UNICODE_SOURCE_ILLEGAL;
			break;
		}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			source -= (extraBytesToRead+1);	/* Back up source pointer! */
			result = UNICODE_TARGET_EXHAUSTED; break;
		}
		if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
			if ((flags == strictConversion) && (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)) {
				source -= (extraBytesToRead+1); /* return to the illegal value itself */
				result = UNICODE_SOURCE_ILLEGAL;
				break;
			} else {
			    *target++ = ch;	/* normal case */
			}
		} else if (ch > UNI_MAX_UTF16) {
			if (flags == strictConversion) {
				result = UNICODE_SOURCE_ILLEGAL;
				source -= (extraBytesToRead+1); /* return to the start */
				break; /* Bail out; shouldn't continue */
			} else {
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		} else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target + 1 >= targetEnd) {
				source -= (extraBytesToRead+1);	/* Back up source pointer! */
				result = UNICODE_TARGET_EXHAUSTED; break;
			}
			ch -= halfBase;
			*target++ = (ch >> halfShift) + UNI_SUR_HIGH_START;
			*target++ = (ch & halfMask) + UNI_SUR_LOW_START;
		}
	}
	*sourceStart = source;
	*target_start = target;
	return result;
}



Conversion_Result Unicode::character_utf32_to_utf8(UTF32 source, UTF8 **target_start, int target_length, bool strict_conversion) {
    unsigned short bytes_to_write = 0;
    const UTF32 byteMask = 0xBF;
    const UTF32 byteMark = 0x80; 

    UTF32 ch = source;

    /* surrogates of any stripe are not legal UTF32 characters */
    if (strict_conversion) {
        if ((ch >= UNI_SUR_HIGH_START) && (ch <= UNI_SUR_LOW_END)) {
            return UNICODE_SOURCE_ILLEGAL;
        }
    }

    /* Figure out how many bytes the result will require */
    if (ch < (UTF32)0x80) {
        bytes_to_write = 1;
    } else if (ch < (UTF32)0x800) {
		bytes_to_write = 2;
    } else if (ch < (UTF32)0x10000) {
        bytes_to_write = 3;
    } else if (ch < (UTF32)0x200000) {
        bytes_to_write = 4;
    } else {
        bytes_to_write = 2;
        ch = UNI_REPLACEMENT_CHAR;
    }
		
    if (bytes_to_write > target_length) {
        return UNICODE_TARGET_EXHAUSTED;
    }

	UTF8 *target = *target_start;
    *target_start += bytes_to_write;


    target += bytes_to_write; // Wow... this is ass-backwards...

    switch (bytes_to_write) {	/* note: everything falls through. */
        case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
        case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
        case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
        case 1:	*--target =  ch | firstByteMark[bytes_to_write];
    }

	return UNICODE_CONVERSION_OK;
}



Conversion_Result Unicode::character_utf8_to_utf32(UTF8 *source_start, int source_length, UTF32 *target) {
	const UTF8 *source = source_start;

    UTF32 ch = 0;
    unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
    if (extraBytesToRead + 1 > source_length) {
        return UNICODE_SOURCE_EXHAUSTED;
    }

    /* Do this check whether lenient or strict */
    if (!isLegalUTF8(source, extraBytesToRead+1)) {
        return UNICODE_SOURCE_ILLEGAL;
    }

    /*
     * The cases all fall through. See "Note A" below.
     */
    switch (extraBytesToRead) {
    case 3:	ch += *source++; ch <<= 6;
    case 2:	ch += *source++; ch <<= 6;
    case 1:	ch += *source++; ch <<= 6;
    case 0:	ch += *source++;
    }

    ch -= offsetsFromUTF8[extraBytesToRead];

    if (ch <= UNI_MAX_UTF32) {
        *target = ch;
    } else { /* i.e., ch > UNI_MAX_UTF32 */
        *target = UNI_REPLACEMENT_CHAR;
    }

	return UNICODE_CONVERSION_OK;
}

Conversion_Result Unicode::character_utf8_to_utf32(UTF8 *source_start, UTF32 *target) {
	const UTF8 *source = source_start;

    UTF32 ch = 0;
    unsigned short extraBytesToRead = trailingBytesForUTF8[*source];

    /* Do this check whether lenient or strict */
    if (!isLegalUTF8(source, extraBytesToRead+1)) {
        return UNICODE_SOURCE_ILLEGAL;
    }

    /*
     * The cases all fall through. See "Note A" below.
     */
    switch (extraBytesToRead) {
    case 3:	ch += *source++; ch <<= 6;
    case 2:	ch += *source++; ch <<= 6;
    case 1:	ch += *source++; ch <<= 6;
    case 0:	ch += *source++;
    }

    ch -= offsetsFromUTF8[extraBytesToRead];

    if (ch <= UNI_MAX_UTF32) {
        *target = ch;
    } else { /* i.e., ch > UNI_MAX_UTF32 */
        *target = UNI_REPLACEMENT_CHAR;
    }

	return UNICODE_CONVERSION_OK;
}

/* ---------------------------------------------------------------------

	Note A.
	The fall-through switches in UTF-8 reading code save a
	temp variable, some decrements & conditionals.  The switches
	are equivalent to the following loop:
		{
			int tmpBytesToRead = extraBytesToRead+1;
			do {
				ch += *source++;
				--tmpBytesToRead;
				if (tmpBytesToRead) ch <<= 6;
			} while (tmpBytesToRead > 0);
		}
	In UTF-8 writing code, the switches on "bytes_to_write" are
	similarly unrolled loops.

   --------------------------------------------------------------------- */



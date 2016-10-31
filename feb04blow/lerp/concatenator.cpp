#include "lerp_os_specific.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "concatenator.h"
#include "mprintf.h"
#include "unicode.h"
#include "memcpy.h"

typedef void **pointer;

struct String_Bucket {
    String_Bucket *next;
    char *point;
    char *start;
    char *end;
    char data[sizeof(pointer)];
};

#define SBMALLOCSIZ(c)  (c->block_size - 16)
#define SBUCKETSIZ (sizeof(struct String_Bucket) - sizeof(pointer))
#define STRING_BUCKET_LEN(c) (SBMALLOCSIZ(c) - SBUCKETSIZ)

static void reset_string_bucket(String_Bucket *b) {
    b->next = NULL;
    b->start = b->data;
    b->point = b->start;
}

static String_Bucket *make_String_Bucket(Concatenator *c) {
    String_Bucket *b;

    b = (String_Bucket *)malloc(SBMALLOCSIZ(c));
    b->end = b->data + STRING_BUCKET_LEN(c);

    reset_string_bucket(b);

    return b;
}

Concatenator::Concatenator(int _block_size) {
    block_size = _block_size;
    nbuckets = 1;
    first = last = make_String_Bucket(this);
}

Concatenator::~Concatenator(void) {
    String_Bucket *b, *c;
    
    for (b = first; b != NULL; b = c) {
        c = b->next;
        delete [] ((char *)b);
    }
}

#define IsFull(b) (b->point == b->end)
#define SpaceLeft(b) (b->end - b->point)

void Concatenator::add(char c) {
    if (IsFull(last)) expand();

    *(last->point++) = c;
}

void Concatenator::add_nozeroterm(char *s, unsigned int len) {
    int left;
    int amount;

    left = SpaceLeft(last);
    assert (left >= 0);

    if (left == 0) {
        expand();
        left = SpaceLeft(last);
    }

    amount = (left > len) ? len : left;

    memcpy(last->point, s, amount);
    last->point += amount;

    if (amount < len) add_nozeroterm(s + amount, len - amount);
}

#include <string.h>
void Concatenator::add(char *s) {
    add_nozeroterm(s, Unicode::size_in_bytes(s) - 1);
}

unsigned int Concatenator::length(void) {
    unsigned int tally; 
    String_Bucket *b;

    tally = 0;
    for (b = first; b != NULL; b = b->next) {
        tally += b->point - b->start;
    }

    return tally;
}

static void result_into_mem(String_Bucket *b, char *s) {
    unsigned int len;

    if (b == NULL) return;
    len = b->point - b->data;
    memcpy(s, b->data, len);
    result_into_mem(b->next, s + len);
}

char *Concatenator::get_result(void) {
    unsigned int len;
    char *result;

    len = length();
    result = new char[len + 1];

    result_into_mem(first, result);
    result[len] = '\0';

    return result;
}

char *Concatenator::get_nozeroterm_result(unsigned int *len_return) {
    unsigned int len;
    char *result;

    len = length();
    result = new char[len];

    result_into_mem(first, result);
    *len_return = len;
    return result;
}

/*
 * In the functions below we assume that BUFSIZ is big enough to hold
 * the representation of integers or doubles.  This is hopefully a safe
 * assumption.
 *
 */
void Concatenator::add(int i) {
    const int BUFFER_SIZE = 512;
    char buf[BUFFER_SIZE];  // XXXXX replace!

    sprintf(buf, "%d", i);
    add(buf);
}

void Concatenator::add(long l) {
    char buf[BUFSIZ];

    sprintf(buf, "%ld", l);
    add(buf);
}

void Concatenator::add(short i) {
    char buf[BUFSIZ];
    sprintf(buf, "%d", i);
    add(buf);
}

void Concatenator::add(unsigned int i) {
    char buf[BUFSIZ];

    sprintf(buf, "%u", i);
    add(buf);
}

void Concatenator::add(double d) {
    char buf[BUFSIZ];

    sprintf(buf, "%f", d);
    add(buf);
}

void Concatenator::add_u4b(int value) {
    unsigned char c0, c1, c2, c3;

    c0 = (value & 0x000000ff) >>  0;
    c1 = (value & 0x0000ff00) >>  8;
    c2 = (value & 0x00ff0000) >> 16;
    c3 = (value & 0xff000000) >> 24;

    int len = length();

    add((char)c0);
    add((char)c1);
    add((char)c2);
    add((char)c3);

    int len2 = length();
    assert(len2 == len + 4);
}

void Concatenator::add_u2b(int value) {
    unsigned char c0, c1;

    c0 = (value & 0x00ff) >>  0;
    c1 = (value & 0xff00) >>  8;

    add((char)c0);
    add((char)c1);
}

void Concatenator::add_u1b(int value) {
    unsigned char c0;
    c0 = (value & 0x00ff) >>  0;
    add((char)c0);
}

void Concatenator::expand(void) {
    String_Bucket *b;

    b = make_String_Bucket(this);
    assert(b != NULL);

    last->next = b;
    last = b;
    nbuckets++;
}

void Concatenator::reset(void) {
    String_Bucket *buck, *buck2;

    buck = first->next;

    while (buck != NULL) {
        buck2 = buck->next;

        delete [] ((char *)buck);
        buck = buck2;
    }

    reset_string_bucket(first);
    last = first;
    nbuckets = 1;
}


String_Bucket *Concatenator::seek(int location, int *base_return) {
    int base = 0;

    String_Bucket *bucket = first;
    while (bucket) {
        int bucket_size = bucket->end - bucket->start;
        if (base + bucket_size > location) {
            *base_return = base;
            return bucket;
        }

        base += bucket_size;
        bucket = bucket->next;
    }

    return NULL;
}

void Concatenator::modify_1b(int location, int value) {
    int base;
    String_Bucket *bucket = seek(location, &base);
    assert(bucket);

    int offset = location - base;
    bucket->start[offset] = value;
}

void Concatenator::modify_2b(int location, int value) {
    // @Speed: Just did a silly implementation here to start with; of course
    // we can make this faster.

    int c0, c1;
    c0 = (value & 0x00ff) >>  0;
    c1 = (value & 0xff00) >>  8;

    modify_1b(location, c0);
    modify_1b(location + 1, c1);
}

void Concatenator::printf(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	char *result = mprintf_valist(fmt, ap);

	va_end(ap);

	if (result) {
		add(result);
		delete [] (result);
	}
}

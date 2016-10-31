#include <stdio.h>

inline void put_u4b(int value, FILE *f) {
    unsigned char c0, c1, c2, c3;

    c0 = (value & 0x000000ff) >>  0;
    c1 = (value & 0x0000ff00) >>  8;
    c2 = (value & 0x00ff0000) >> 16;
    c3 = (value & 0xff000000) >> 24;

    fputc(c0, f);
    fputc(c1, f);
    fputc(c2, f);
    fputc(c3, f);
}

inline void put_u2b(int value, FILE *f) {
    unsigned char c0, c1;

    c0 = (value & 0x00ff) >>  0;
    c1 = (value & 0xff00) >>  8;

    fputc(c0, f);
    fputc(c1, f);
}

inline void put_f32(float value, FILE *f) {
    unsigned long ivalue = *(unsigned long *)&value;
    put_u4b(ivalue, f);
}

inline void put_u1b(int value, FILE *f) {
    fputc(value, f);
}

inline void put_string(char *value, FILE *f) {
    int len = strlen(value);
    assert(len < 65536);

    put_u2b(len, f);
    while (*value) {
        fputc(*value, f);
        value++;
    }
}

inline void put_vector3(Vector3 *value, FILE *f) {
    put_f32(value->x, f);
    put_f32(value->y, f);
    put_f32(value->z, f);
}


inline void put_quaternion(Quaternion *value, FILE *f) {
    put_f32(value->w, f);
    put_f32(value->x, f);
    put_f32(value->y, f);
    put_f32(value->z, f);
}







inline void get_u4b(FILE *f, int *result, bool *error) {
    assert(sizeof(int) >= 4);

    int c0, c1, c2, c3;
    c0 = fgetc(f);
    c1 = fgetc(f);
    c2 = fgetc(f);
    c3 = fgetc(f);

    if (c3 == EOF) *error = true;

    unsigned int sum = 0;

    sum += ((unsigned int)c0) <<  0;
    sum += ((unsigned int)c1) <<  8;
    sum += ((unsigned int)c2) << 16;
    sum += ((unsigned int)c3) << 24;

    *result = (int)sum;
}

inline void get_u2b(FILE *f, int *result, bool *error) {
    assert(sizeof(int) >= 2);

    int c0, c1;
    c0 = fgetc(f);
    c1 = fgetc(f);

    if (c1 == EOF) *error = true;

    unsigned int sum = 0;

    sum += ((unsigned int)c0) <<  0;
    sum += ((unsigned int)c1) <<  8;

    *result = (int)sum;
}

inline void get_u1b(FILE *f, int *result, bool *error) {
    assert(sizeof(int) >= 1);

    int c0;
    c0 = fgetc(f);

    if (c0 == EOF) *error = true;

    *result = c0;
}

inline void get_f32(FILE *f, float *result, bool *error) {
    int ivalue;
    get_u4b(f, &ivalue, error);
    *result = *(float *)&ivalue;
}

inline void get_vector3(FILE *f, Vector3 *result, bool *error) {
    get_f32(f, &result->x, error);
    get_f32(f, &result->y, error);
    get_f32(f, &result->z, error);
}

inline void get_quaternion(FILE *f, Quaternion *result, bool *error) {
    get_f32(f, &result->w, error);
    get_f32(f, &result->x, error);
    get_f32(f, &result->y, error);
    get_f32(f, &result->z, error);
}

inline void get_string(FILE *f, char **result, bool *error) {
    int len;
    get_u2b(f, &len, error);
    if (*error) return;

    char *data = new char[len + 1];
    *result = data;
    data[len] = '\0';
    
    int i;
    for (i = 0; i < len; i++) {
        data[i] = fgetc(f);
    }

    if (len && (data[len - 1] == EOF)) *error = true;
}


inline void get_u4b_reversed(FILE *f, int *result, bool *error) {
    assert(sizeof(int) >= 4);

    int c0, c1, c2, c3;
    c0 = fgetc(f);
    c1 = fgetc(f);
    c2 = fgetc(f);
    c3 = fgetc(f);

    if (c3 == EOF) *error = true;

    unsigned int sum = 0;

    sum += ((unsigned int)c0) << 24;
    sum += ((unsigned int)c1) << 16;
    sum += ((unsigned int)c2) <<  8;
    sum += ((unsigned int)c3) <<  0;

    *result = (int)sum;
}

inline void get_u2b_reversed(FILE *f, int *result, bool *error) {
    assert(sizeof(int) >= 2);

    int c0, c1;
    c0 = fgetc(f);
    c1 = fgetc(f);

    if (c1 == EOF) *error = true;

    unsigned int sum = 0;

    sum += ((unsigned int)c0) <<  8;
    sum += ((unsigned int)c1) <<  0;

    *result = (int)sum;
}


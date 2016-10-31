/*
header to 3d stuff

Z3d.h
*/
#pragma once
#ifndef Z3D_H
#define FRONT 1
#define BACK 2
#define COPLANAR 3
#define TRAVERSE 4
#define LEAF 5
#define NODE 6
#define VALID 7
#define INVALID 8
#define EPSILON 0.001
#define PI 3.14159
extern float sinlook[32767],coslook[32767];

typedef struct{
	float v[3];
} vert3d_t, *LPvert3d_t;

typedef struct{
	vert3d_t norm;
	float d;
} plane_t, *LPplane_t;

typedef struct{
	int v[3];
} tri_t, *LPtri_t;

typedef struct{
	vert3d_t v[16];
	vert3d_t c[16];
	int f[16];
	int nverts;
	int color;
} poly_t, *LPpoly_t;

typedef struct {
    double   x, y;
} point2D_t;

typedef struct {
    int         color;
    int         numverts;
    point2D_t   verts[8];
} polygon2D_t;


typedef struct {
    int xleft, xright;
} span_t;

extern inline float DotProduct3D(vert3d_t *a, vert3d_t *b);
extern inline void CrossProduct3D(vert3d_t *u,vert3d_t *v, vert3d_t *normal);
extern inline void Normalize(vert3d_t *v);
extern inline void BackRotateVector(vert3d_t *in, vert3d_t *out, vert3d_t *axis);
extern void BuildLookupTables(void);
extern inline void ProjectVert(vert3d_t *vert);
extern inline void RotateVerts(vert3d_t *in, vert3d_t *out, vert3d_t *axis, int nverts);
extern inline void TranslateVerts(vert3d_t *vert, vert3d_t *disp,int nverts);
extern inline void RotateUnitVectors(vert3d_t *axis, int *vals);
extern void ProjectVerts(vert3d_t *verts, int nverts);
extern inline int TestVert(plane_t *plane,vert3d_t *vert);
extern void AssignVert(vert3d_t *vert, float x, float y, float z);
extern int ClipPoly(poly_t *in, plane_t *plane, poly_t *out);
extern void FillPolygon2D(polygon2D_t *ppoly);
extern void ProjectPolygon (poly_t *ppoly, polygon2D_t *ppoly2D);

#define Z3D_H
#endif
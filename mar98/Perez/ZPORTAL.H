/*
portal rendering Build 0

zportal.h
*/
#include "Ztypes.h"
#include "newengg.h"
#include "Zfile.h"
#include "Z3d.h"


#ifndef ZPORTAL_H

#define MAXCELLPOLYS 20  //we're not going for ultra-complex :)
#define MAXCELLS 8      //make it the size of our data set for simplicity
#define MAXFRUSPLANES 10//no way it'll get more complex than this.

typedef struct{
	int nfrus;
	plane_t p[MAXFRUSPLANES];
} ZPfrus_t, *LPZPfrus_t;

typedef struct{
	int type;//-1 is normal, else index of the cell portal goes to.
	poly_t poly;
	plane_t plane;
} ZPpoly_t, *LPZPpoly_t;

typedef struct{
	int npolys;
	ZPpoly_t p[MAXCELLPOLYS];
} ZPcell_t, *LPZPcell_t;


typedef class ZPortal {
public:
	// Constructors / Destructors
	ZPortal();
	~ZPortal();
	//variables
	int ncells,flag;
	int currcell;//the cell that the camera is in
	ZPcell_t c[MAXCELLS];
	vert3d_t oh;//view above the model
	vert3d_t oaxis[3];
	// Other functions
	void GenPlane(ZPpoly_t *node);
	void LoadB2(char *filename);//we'll need cells for this to work, right?
	void FindCameraCell();//goes through all cells and finds the one the camera's in.
	void DrawCell(ZPcell_t *cell,ZPfrus_t *frus);//draws a cell against a frustum
	void BuildFrus(poly_t *poly,ZPfrus_t *frus);//builds frus from poly.
	void DrawPoly(poly_t *poly);//flag says which camera to draw from.
} ZPortal, *LPZPortal;

#define ZPORTAL_H
#endif

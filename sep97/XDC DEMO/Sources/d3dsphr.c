/*==========================================================================
 *
 *  Copyright (C) 1995, 1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File: d3dsphr.c
 *
 ***************************************************************************/

#include <math.h>
#include <d3d.h>

#define PI 3.14159265358979323846f

/*
 * Generates a sphere around the y-axis centered at the origin including
 * normals and texture coordiantes.  Returns TRUE on success and FALSE on
 * failure.
 *     sphere_r     Radius of the sphere.
 *     num_rings    Number of full rings not including the top and bottom
 *                  caps.
 *     num_sections Number of sections each ring is divided into.  Each
 *                  section contains two triangles on full rings and one 
 *                  on top and bottom caps.
 *     sx, sy, sz   Scaling along each axis.  Set each to 1.0 for a 
 *                  perfect sphere. 
 *     plpv         On exit points to the vertices of the sphere.  The
 *                  function allocates this space.  Not allocated if
 *                  function fails.
 *     plptri       On exit points to the triangles of the sphere which 
 *                  reference vertices in the vertex list.  The function
 *                  allocates this space. Not allocated if function fails.
 *     pnum_v       On exit contains the number of vertices.
 *     pnum_tri     On exit contains the number of triangles.
 */


BOOL 
GenerateSphereClose(float sphere_r, int num_rings, int num_sections, float sx, 
               float sy, float sz, LPD3DVERTEX* plpv, 
               LPD3DTRIANGLE* plptri, int* pnum_v, int* pnum_tri)
{

    float theta, phi;    /* Angles used to sweep around sphere */
    float dtheta, dphi;  /* Angle between each section and ring */
    float x, y, z, v, rsintheta; /* Temporary variables */
    float cos_v , sin_v, cos_u ,sin_u , Nx, Ny ,Nz ,Nsize ,r_2;
	int i, j, n, m;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */

    /*
     * Check the parameters to make sure they are valid.
     */
    if ((sphere_r <= 0) || (num_rings < 1) || (num_sections < 3) ||
        (sx <= 0) || (sy <= 0) || (sz <= 0))
        return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
    num_tri = (num_rings + 1) * num_sections * 2;
    num_v = (num_rings + 1) * (num_sections +1) + 2;
    *plpv = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    lpv = *plpv;
    lptri = *plptri;
    *pnum_v = num_v;
    *pnum_tri = num_tri;
     r_2 = sphere_r * sphere_r;
    /*
     * Generate vertices at the top and bottom points.
     */
    lpv[0].x = D3DVAL(0.0);
    lpv[0].y = D3DVAL(sy * sphere_r);
    lpv[0].z = D3DVAL(0.0);
    lpv[0].nx = D3DVAL(0.0);
    lpv[0].ny = D3DVAL(1.0);
    lpv[0].nz = D3DVAL(0.0);
    lpv[0].tu = D3DVAL(0.0);
    lpv[0].tv = D3DVAL(0.0);
    lpv[num_v - 1].x = D3DVAL(0.0);
    lpv[num_v - 1].y = D3DVAL(sy * -sphere_r);
    lpv[num_v - 1].z = D3DVAL(0.0);
    lpv[num_v - 1].nx = D3DVAL(0.0);
    lpv[num_v - 1].ny = D3DVAL(-1.0);
    lpv[num_v - 1].nz = D3DVAL(0.0);
    lpv[num_v - 1].tu = D3DVAL(0.0);
    lpv[num_v - 1].tv = D3DVAL(1.0);


    /*
     * Generate vertex points for rings
     */
    dtheta = (float)(PI / (double)(num_rings + 2));
    dphi = (float)(2.0 * PI / (double) num_sections);
    n = 1; /* vertex being generated, begins at 1 to skip top point */
    theta = dtheta;
    for (i = 0; i <= num_rings; i++) {
        y = sphere_r * (float)cos(theta); /* y is the same for each ring */
        v = theta / (float)PI;     /* v is the same for each ring */
        rsintheta = sphere_r * (float)sin(theta);
        phi = (float)0.0;
        for (j = 0; j <= num_sections; j++) {
            x = rsintheta * (float)sin(phi);
            z = rsintheta * (float)cos(phi);
            lpv[n].x = D3DVAL(sx * x);
            lpv[n].z = D3DVAL(-1.0f * sz * z);
            lpv[n].y = D3DVAL(sy * y);
	        
			cos_v  = (float)cos(theta); sin_v = (float)sin(theta);
			cos_u  = (float)cos(phi);    sin_u = (float)sin(phi);
		
            Nx    = ( r_2  * sin_v * sin_v * sin_u);
		    Ny   =  ( r_2  * sin_v * cos_v);
		    Nz   =  (r_2  * sin_v * sin_v * cos_u);
            Nsize = (float)sqrt(Nx * Nx + Ny * Ny + Nz * Nz);            
			lpv[n].nx = D3DVAL(Nx / Nsize);
            lpv[n].ny = D3DVAL(Ny / Nsize);
            lpv[n].nz = D3DVAL(-1.0f * Nz / Nsize);
            lpv[n].tv = D3DVAL(v);
            lpv[n].tu = D3DVAL((float)(1.0 - phi / (2.0 * PI)));
            phi += dphi;
            ++n;
        }
        theta += dtheta;
    }

    /*
     * Generate triangles for top and bottom caps.
     */
    if (num_sections < 30) {
    /*
     * we can put the whole cap in a tri fan.
     */
        for (i = 0; i < num_sections; i++) {
            lptri[i].v1 = 0;
            lptri[i].v2 = i + 1;
            lptri[i].v3 = 1 + ((i + 1) % num_sections);
            
            lptri[num_tri - num_sections + i].v1 = num_v - 1;
            lptri[num_tri - num_sections + i].v2 = num_v - 2 - i;
            lptri[num_tri - num_sections + i].v3 = num_v - 2 - 
                    ((1 + i) % num_sections);
                    
                   
            /*
             * Enable correct edges.
             */
            lptri[i].wFlags = D3DTRIFLAG_EDGEENABLE1 |
                              D3DTRIFLAG_EDGEENABLE2;
                              
            lptri[num_tri - num_sections + i].wFlags= D3DTRIFLAG_EDGEENABLE1 |
                                                      D3DTRIFLAG_EDGEENABLE2;
            /*
             * build fans.
             */
            if (i == 0) {
                lptri[i].wFlags |= D3DTRIFLAG_START;
                lptri[num_tri - num_sections + i].wFlags |= D3DTRIFLAG_START;
            } else {
                lptri[i].wFlags |= D3DTRIFLAG_EVEN;
                lptri[num_tri - num_sections + i].wFlags |= D3DTRIFLAG_EVEN;
            }
            
        }
    } else {
        for (i = 0; i < num_sections; i++) {
            lptri[i].v1 = 0;
            lptri[i].v2 = i + 1;
            lptri[i].v3 = 1 + ((i + 1) % num_sections);
            lptri[i].wFlags = D3DTRIFLAG_EDGEENABLE1;
                              D3DTRIFLAG_EDGEENABLE2;
            lptri[num_tri - num_sections + i].v1 = num_v - 1;
            lptri[num_tri - num_sections + i].v2 = num_v - 2 - i;
            lptri[num_tri - num_sections + i].v3 = num_v - 2 - 
                    ((1 + i) % num_sections);
            lptri[num_tri - num_sections + i].wFlags= D3DTRIFLAG_EDGEENABLE1 |
                                                      D3DTRIFLAG_EDGEENABLE2;
        }
    }

    /*
     * Generate triangles for the rings
     */
    m = 1; /* first vertex in current ring,begins at 1 to skip top point*/
    n = num_sections; /* triangle being generated, skip the top cap */
        for (i = 0; i < num_rings; i++) {
        for (j = 0; j < num_sections; j++) {
            lptri[n].v1 = m + j;
            lptri[n].v2 = m + num_sections + j;
            lptri[n].v3 = m + num_sections + ((j + 1) % num_sections);
            lptri[n].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
            
            /*
             * Start a two triangle flat fan for each face.
             */
                    
            lptri[n].wFlags = D3DTRIFLAG_STARTFLAT(1);
            
            /*
             * only need two edges for wireframe.
             */ 
            lptri[n].wFlags |= D3DTRIFLAG_EDGEENABLE1 |
                               D3DTRIFLAG_EDGEENABLE2;
        
            
            lptri[n + 1].v1 = lptri[n].v1;
            lptri[n + 1].v2 = lptri[n].v3;
            lptri[n + 1].v3 = m + ((j + 1) % num_sections);
            
            lptri[n + 1].wFlags = D3DTRIFLAG_EVEN;
            /*
             * only need two edges for wireframe.
             */ 
            lptri[n + 1].wFlags |= D3DTRIFLAG_EDGEENABLE2 |
                                   D3DTRIFLAG_EDGEENABLE3;
            n += 2;
        }
        m += num_sections;
    }
    return TRUE;
}

/**************************************************************************************/
BOOL 
GenerateCone(float cone_r, float cone_h , int num_rings, int num_sections, 
               LPD3DVERTEX* plpv, 
               LPD3DTRIANGLE* plptri, int* pnum_v, int* pnum_tri)
{

    float phi,dy , ddy;    /* Angles used to sweep around sphere */
    float dphi;  /* Angle between each section and ring */
    float x, y, z; /* Temporary variables */
    int i, j, n; // , m;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */
    float sy,sz,sx;
    sy =  sz = sx = (float)1.0;
	/*
     * Check the parameters to make sure they are valid.
     */
    if ((cone_r <= 0) || (num_rings < 1) || (num_sections < 3) ||
        (sx <= 0) || (sy <= 0) || (sz <= 0))
        return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
    num_tri = num_rings  * num_sections * 2;
    num_v = (num_rings +1) * (num_sections +1);
    *plpv = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    lpv = *plpv;
    lptri = *plptri;
    *pnum_v = num_v;
    *pnum_tri = num_tri;

    /* 
	* Generate vertex points for rings
     */
    dphi = (float)(2.0 * PI / (double) num_sections);
    n = 0; /* vertex being generated, begins at 1 to skip top point */
    dy = (float)0.0 ; 
	ddy  =  (float)1.0 / (float)num_rings;
	for (i = 0; i <= num_rings; i++) {
        y = -(cone_h * (float)0.5) +  dy  * cone_h ;
        phi = (float)0.0;
        for (j = 0; j <= num_sections; j++) {
            x = cone_r * (float)cos(phi);
            z = cone_r * (float)sin(phi);
            lpv[n].x = D3DVAL(sx * x) ;
            lpv[n].z = D3DVAL(sz * z) ;
            lpv[n].y = D3DVAL(sy * y);
            lpv[n].nx = D3DVAL( x / cone_r);
            lpv[n].nz = D3DVAL( z / cone_r);
            lpv[n].ny = D3DVAL(y);
            lpv[n].tv = D3DVAL(ddy*i);
            lpv[n].tu = D3DVAL((float)(phi / (2.0 * PI)));
            phi += dphi;
           	n++;
        }
        dy += ddy;
    }

	
/*	
   m = 0; // first vertex in current ring,begins at 1 to skip top point
   n  = 0; //  triangle being generated, skip the top cap 
  for (i = 0; i < num_rings; i++) {
        for (j = 0; j < num_sections; j++)  {
            lptri[n].v1 = m + j;
            lptri[n].v2 = m + num_sections + j;
            lptri[n].v3 = m + num_sections + ((j + 1) % num_sections);
            lptri[n].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
            lptri[n].wFlags = D3DTRIFLAG_STARTFLAT(1);
            lptri[n].wFlags |= D3DTRIFLAG_EDGEENABLE1 |
                                      D3DTRIFLAG_EDGEENABLE2;
            
			lptri[n + 1].v1 = lptri[n].v1;
            lptri[n + 1].v2 = lptri[n].v3;
            lptri[n + 1].v3 = m + ((j + 1) % num_sections);
            
            lptri[n + 1].wFlags = D3DTRIFLAG_EVEN;
            lptri[n + 1].wFlags |= D3DTRIFLAG_EDGEENABLE2 |
                                   D3DTRIFLAG_EDGEENABLE3;
			n += 2;
        }
        m += num_sections;
    }
*/
return TRUE;
}

/*****************************************************************************/
BOOL 
GenerateTorus(float tor_big_r, float tor_small_r,
			           int num_rings,  int num_sections, 
					   float sx, float sy, float sz, 
					   LPD3DVERTEX* plpv, LPD3DTRIANGLE* plptri, 
					   int* pnum_v, int* pnum_tri)
{

    float theta, phi;    /* Angles used to sweep around sphere */
    float dtheta, dphi;  /* Angle between each section and ring */
    float x, y, z, v; /* Temporary variables */
    float a, b, bcostheta;
	float  a_pls_bcosv, cos_v , sin_v, cos_u ,sin_u , Nx, Ny ,Nz ,Nsize;
	int i, j, n ;//, m;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */

    /*
     * Check the parameters to make sure they are valid.
     */
    if ((tor_big_r <= 0) || (num_rings < 1) || (num_sections < 3) ||
        (sx <= 0) || (sy <= 0) || (sz <= 0))
        return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
     num_tri = (num_rings ) * (num_sections) * 2;
	 num_v  = (num_rings + 1) * (num_sections + 1);
    *plpv    = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    // *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    
	 lpv = *plpv;
     lptri = *plptri;
     *pnum_v = num_v;
    *pnum_tri = num_tri;
	
	a = tor_big_r;
	b = tor_small_r;
	
     /*
     * Generate vertex points for rings
     */
    dtheta = (float)(2.0 * PI / (double)(num_rings ));
    dphi    = (float)(2.0 * PI / (double) num_sections);
    n = 0; /* vertex being generated, begins at 1 to skip top point */
    theta = 0.0f;
    for (i = 0; i <= num_rings; i++) {
        y = tor_small_r * (float)sin(theta); /* y is the same for each ring */
        v = theta / (2.0f * PI);		    /* v is the same for each ring */
        bcostheta = b * (float)cos(theta);
        phi = (float)0.0;
        for (j = 0; j <=  num_sections; j++) {
            x = (a + bcostheta) * (float)cos(phi);
            z = (a + bcostheta) * (float)sin(phi);
            
			
			lpv[n].x = D3DVAL(sx * x );
            lpv[n].z = D3DVAL(sz * z * -1.0f);
            lpv[n].y = D3DVAL(sy * y);
            
			// normal calculation
			cos_v  = (float)cos(theta); sin_v = (float)sin(theta);
			cos_u  = (float)cos(phi);    sin_u = (float)sin(phi);
			a_pls_bcosv = a + b * cos_v;
            
			Nx   =  (a_pls_bcosv  * b  * cos_v * cos_u);
		    Ny   =  (a_pls_bcosv  * b  * sin_v  );
		    Nz   =  -1.0f * (a_pls_bcosv  * b  * cos_v * sin_u);
            /*
			Nx   =  (lpv[n].x - ((a + b)  * cos_u));
		    Ny   =  (lpv[n].y) ;
		    Nz   =  (lpv[n].z - ((a + b)  * sin_u) );
			*/
			Nsize = (float)sqrt(Nx * Nx + Ny * Ny + Nz * Nz);
            lpv[n].nx = D3DVAL(Nx / Nsize);
            lpv[n].ny = D3DVAL(Ny / Nsize);
            lpv[n].nz = D3DVAL(Nz / Nsize);
            /*
			if( (v < 0.15f) || (v > 0.85f) ) {
			lpv[n].x += lpv[n].nx;
            lpv[n].z += lpv[n].nz;
            lpv[n].y += lpv[n].ny;
			}
			*/
			lpv[n].tv = D3DVAL(v);
            lpv[n].tu = D3DVAL((float)(1.0 - phi / (2.0 * PI)));
            
			
			
			phi += dphi;
            ++n;
        }
        theta += dtheta;
    }
    /*
     * Generate triangles for the rings
     */
    /*
	m = 0; // first vertex in current ring,begins at 1 to skip top point
    n = 0;  // triangle being generated, skip the top cap 
    for (i = 0; i < num_rings; i++) {
        for (j = 0; j < num_sections; j++) {
            lptri[n].v1 = m + j;
            lptri[n].v2 = m + (num_sections +1) + j;
            lptri[n].v3 = m + (num_sections + 1) + (j + 1);
			
			lptri[n + 1].v1 = lptri[n].v1;
            lptri[n + 1].v2 = lptri[n].v3;
            lptri[n + 1].v3 = m + (j + 1);
            
			n += 2;
        }
        m = m + (num_sections +1);
    }
   */
    return TRUE;
}
/*****************************************************************************/
BOOL 
GenerateEllipsoid(float a, float b, float c,
			           int num_rings,  int num_sections, 
					   LPD3DVERTEX* plpv, LPD3DTRIANGLE* plptri, 
					   int* pnum_v, int* pnum_tri)
{

    float theta, phi;    /* Angles used to sweep around sphere */
    float dtheta, dphi;  /* Angle between each section and ring */
    float x, y, z, v; /* Temporary variables */
	float  cos_v , sin_v, cos_u ,sin_u , Nx, Ny ,Nz ,Nsize;
	int i, j, n;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */

    /*
     * Check the parameters to make sure they are valid.
     */
    if ((num_rings < 1) || (num_sections < 3) ) return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
     num_tri = (num_rings + 1) * (num_sections  + 1) * 2;
	 num_v  = (num_rings + 1) * (num_sections + 1);
    *plpv    = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    
	 lpv = *plpv;
     lptri = *plptri;
     *pnum_v = num_v;
    *pnum_tri = num_tri;
	
     /*
     * Generate vertex points for rings
     */
    dtheta = (float)(2.0 * PI / (double)(num_rings ));
    dphi    = (float)(2.0 * PI / (double) num_sections);
    n = 0; /* vertex being generated, begins at 1 to skip top point */
    phi = 0.0f;
    for (i = 0; i <= num_rings; i++) {
        z = c  * (float)cos(phi);           /* z is the same for each ring */
        v = phi / (2.0f * PI);	   /* v is the same for each ring */
        theta = 0.0f;
        for (j = 0; j <=  num_sections; j++) {
            x = a * (float)(cos(theta)*sin(phi));
            y = b * (float)(sin(theta) *sin(phi));
            lpv[n].x = D3DVAL( x);
            lpv[n].z = D3DVAL( -1.0f * z);
            lpv[n].y = D3DVAL( y);
            // normal calculation
            cos_v  = (float)cos(theta); sin_v = (float)sin(theta);
			cos_u  = (float)cos(phi);    sin_u = (float)sin(phi);
            
			Nx   = - (b  * c * cos_v * sin_u * sin_u);
		    Ny   = - (a  * c * sin_v  * sin_u * sin_u);
		    Nz   = - (a  * b * cos_u * sin_u);
            Nsize = (float)sqrt(Nx * Nx + Ny * Ny + Nz * Nz);
            lpv[n].nx = D3DVAL(Nx / Nsize);
            lpv[n].ny = D3DVAL(Ny / Nsize);
            lpv[n].nz = D3DVAL(Nz / Nsize);
            lpv[n].tv = D3DVAL(v);
            lpv[n].tu = D3DVAL((float)(1.0 - theta / (2.0 * PI)));
            theta += dtheta;
            ++n;
        }
        phi += dphi;
    }

    return TRUE;
}

/******************************************************************************/
BOOL 
GenerateHyperboloid(float a, float b, float c,
			           int num_rings,  int num_sections, 
					   float sx, float sy, float sz, 
					   LPD3DVERTEX* plpv, LPD3DTRIANGLE* plptri, 
					   int* pnum_v, int* pnum_tri)
{

    float theta, phi;    /* Angles used to sweep around sphere */
    float dtheta, dphi;  /* Angle between each section and ring */
    float x, y, z, v; /* Temporary variables */
	float  cos_v , sin_v, cos_u ,sin_u , Nx, Ny ,Nz ,Nsize;
	int i, j, n;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */

    /*
     * Check the parameters to make sure they are valid.
     */
    if ((a <= 0) || (num_rings < 1) || (num_sections < 3) ||
        (sx <= 0) || (sy <= 0) || (sz <= 0))
        return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
     num_tri = (num_rings + 1) * (num_sections  + 1) * 2;
	 num_v  = (num_rings + 1) * (num_sections + 1);
    *plpv    = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    
	 lpv = *plpv;
     lptri = *plptri;
     *pnum_v = num_v;
    *pnum_tri = num_tri;
	
     /*
     * Generate vertex points for rings
     */
    dtheta = (float)(2.0 * PI / (double)(num_rings ));
    dphi    = (float)(2.0 * PI / (double) num_sections);
    n = 0; /* vertex being generated, begins at 1 to skip top point */
    phi = -3.141592653589793f;
    for (i = 0; i <= num_rings; i++) {
        y = b  * (float)sinh(phi);           /* z is the same for each ring */
        v = (phi + PI) / ((float)2.0 * PI);	   /* v is the same for each ring */
        theta = 0.0f;
        for (j = 0; j <=  num_sections; j++) {
            x = a * (float)(cos(theta)*cosh(phi));
            z = c * (float)(sin(theta) *sinh(phi));
            lpv[n].x = D3DVAL(sx * x);
            lpv[n].z = D3DVAL(sz * z);
            lpv[n].y = D3DVAL(sy * y);
            // normal calculation
            
			cos_v  = (float)cos(theta); sin_v = (float)sin(theta);
			cos_u  = (float)cos(phi);    sin_u = (float)sin(phi);
            /*
			Nx   =  (a_pls_bcosv  * b  * cos_v * cos_u);
		    Ny   =  (a_pls_bcosv  * b  * sin_v  );
		    Nz   =  (a_pls_bcosv  * b  * cos_v * sin_u);
            */
			Nx   =  (lpv[n].x - ((a + b)  * cos_u));
		    Ny   =  (lpv[n].y) ;
		    Nz   =  (lpv[n].z - ((a + b)  * sin_u) );
			
			Nsize = (float)sqrt(Nx * Nx + Ny * Ny + Nz * Nz + 0.0001f);
            lpv[n].nx = D3DVAL(Nx / Nsize);
            lpv[n].ny = D3DVAL(Ny / Nsize);
            lpv[n].nz = D3DVAL(Nz / Nsize);
            lpv[n].tv = D3DVAL(v);
            lpv[n].tu = D3DVAL((float)(1.0 - theta / (2.0 * PI)));
            theta += dtheta;
            ++n;
        }
        phi += dphi;
    }

    return TRUE;
}

/******************************************************************************/
BOOL 
GenerateSaddle(
			           int num_rings,  int num_sections, 
					   float sx, float sy, float sz, 
					   LPD3DVERTEX* plpv, LPD3DTRIANGLE* plptri, 
					   int* pnum_v, int* pnum_tri)
{

    float du, dv;  /* Angle between each section and ring */
    float x, y, z,u, v; /* Temporary variables */
    //float bcostheta ;
	float  Nx, Ny ,Nz ,Nsize;
	int i, j, n; // , m;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */

    /*
     * Check the parameters to make sure they are valid.
     */
    if ( (num_rings < 1) || (num_sections < 3) ||
        (sx <= 0) || (sy <= 0) || (sz <= 0))
        return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
     num_tri = (num_rings + 1) * (num_sections  + 1) * 2;
	 num_v  = (num_rings + 1) * (num_sections + 1);
    *plpv    = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    
	 lpv = *plpv;
     lptri = *plptri;
     *pnum_v = num_v;
    *pnum_tri = num_tri;
	
     /*
     * Generate vertex points for rings
     */
    du = (float)(2.0 / (double)num_rings);
    dv = (float)(2.0 / (double)num_sections);
    n = 0; /* vertex being generated, begins at 1 to skip top point */
    v = -1.0f;
    for (i = 0; i <= num_rings; i++) {
        u = -1.0f;	   /* v is the same for each ring */
        y  = u * v ;  /* z is the same for each ring */
        for (j = 0; j <=  num_sections; j++) {
            x = u;
            z = v;
            lpv[n].x = D3DVAL(sx * x);
            lpv[n].z = D3DVAL(sz * z);
            lpv[n].y = D3DVAL(sy * y);
            // normal calculation
            Nx   =  (lpv[n].x);
		    Ny   =  (lpv[n].y);
		    Nz   =  (lpv[n].z );
			Nsize = (float)sqrt(Nx * Nx + Ny * Ny + Nz * Nz + 0.00001f);
            lpv[n].nx = D3DVAL(Nx / Nsize);
            lpv[n].ny = D3DVAL(Ny / Nsize);
            lpv[n].nz = D3DVAL(Nz / Nsize);
            lpv[n].tv = D3DVAL((v +1.0f) / 2.0f);
            lpv[n].tu = D3DVAL((u + 1.0f) / 2.0f);
            u += du;
            ++n;
        }
        v += dv;
    }

    return TRUE;
}

BOOL 
GenerateSphereOld(float sphere_r, int num_rings, int num_sections, float sx, 
               float sy, float sz, LPD3DVERTEX* plpv, 
               LPD3DTRIANGLE* plptri, int* pnum_v, int* pnum_tri)
{

    float theta, phi;    /* Angles used to sweep around sphere */
    float dtheta, dphi;  /* Angle between each section and ring */
    float x, y, z, v, rsintheta; /* Temporary variables */
    int i, j, n, m;      /* counters */
    int num_v, num_tri;  /* Internal vertex and triangle count */
    LPD3DVERTEX lpv;     /* Internal pointer for vertices */
    LPD3DTRIANGLE lptri; /* Internal pointer for trianlges */

    /*
     * Check the parameters to make sure they are valid.
     */
    if ((sphere_r <= 0) || (num_rings < 1) || (num_sections < 3) ||
        (sx <= 0) || (sy <= 0) || (sz <= 0))
        return FALSE;
    /*
     * Generate space for the required triangles and vertices.
     */
    num_tri = (num_rings + 1) * num_sections * 2;
    num_v = (num_rings + 1) * (num_sections) + 2;
    *plpv = (LPD3DVERTEX) malloc(sizeof(D3DVERTEX) * num_v);
    *plptri = (LPD3DTRIANGLE) malloc(sizeof(D3DTRIANGLE) * num_tri);
    lpv = *plpv;
    lptri = *plptri;
    *pnum_v = num_v;
    *pnum_tri = num_tri;

    /*
     * Generate vertices at the top and bottom points.
     */
    lpv[0].x = D3DVAL(0.0);
    lpv[0].y = D3DVAL(sy * sphere_r);
    lpv[0].z = D3DVAL(0.0);
    lpv[0].nx = D3DVAL(0.0);
    lpv[0].ny = D3DVAL(1.0);
    lpv[0].nz = D3DVAL(0.0);
    lpv[0].tu = D3DVAL(0.0);
    lpv[0].tv = D3DVAL(0.0);
    lpv[num_v - 1].x = D3DVAL(0.0);
    lpv[num_v - 1].y = D3DVAL(sy * -sphere_r);
    lpv[num_v - 1].z = D3DVAL(0.0);
    lpv[num_v - 1].nx = D3DVAL(0.0);
    lpv[num_v - 1].ny = D3DVAL(-1.0);
    lpv[num_v - 1].nz = D3DVAL(0.0);
    lpv[num_v - 1].tu = D3DVAL(0.0);
    lpv[num_v - 1].tv = D3DVAL(1.0);


    /*
     * Generate vertex points for rings
     */
    dtheta = (float)(PI / (double)(num_rings + 2));
    dphi = (float)(2.0 * PI / (double) num_sections);
    n = 1; /* vertex being generated, begins at 1 to skip top point */
    theta = dtheta;
    for (i = 0; i <= num_rings; i++) {
        y = sphere_r * (float)cos(theta); /* y is the same for each ring */
        v = theta / (float)PI;     /* v is the same for each ring */
        rsintheta = sphere_r * (float)sin(theta);
        phi = (float)0.0;
        for (j = 0; j <  num_sections; j++) {
            x = rsintheta * (float)sin(phi);
            z = rsintheta * (float)cos(phi);
            lpv[n].x = D3DVAL(sx * x);
            lpv[n].z = D3DVAL(sz * z);
            lpv[n].y = D3DVAL(sy * y);
            lpv[n].nx = D3DVAL(x / sphere_r);
            lpv[n].ny = D3DVAL(y / sphere_r);
            lpv[n].nz = D3DVAL(z / sphere_r);
            lpv[n].tv = D3DVAL(v);
            lpv[n].tu = D3DVAL((float)(1.0 - phi / (2.0 * PI)));
            phi += dphi;
            ++n;
        }
        theta += dtheta;
    }

    /*
     * Generate triangles for top and bottom caps.
     */
    if (num_sections < 30) {
    /*
     * we can put the whole cap in a tri fan.
     */
        for (i = 0; i < num_sections; i++) {
            lptri[i].v1 = 0;
            lptri[i].v2 = i + 1;
            lptri[i].v3 = 1 + ((i + 1) % num_sections);
            
            lptri[num_tri - num_sections + i].v1 = num_v - 1;
            lptri[num_tri - num_sections + i].v2 = num_v - 2 - i;
            lptri[num_tri - num_sections + i].v3 = num_v - 2 - 
                    ((1 + i) % num_sections);
                    
                   
            /*
             * Enable correct edges.
             */
            lptri[i].wFlags = D3DTRIFLAG_EDGEENABLE1 |
                              D3DTRIFLAG_EDGEENABLE2;
                              
            lptri[num_tri - num_sections + i].wFlags= D3DTRIFLAG_EDGEENABLE1 |
                                                      D3DTRIFLAG_EDGEENABLE2;
            /*
             * build fans.
             */
            if (i == 0) {
                lptri[i].wFlags |= D3DTRIFLAG_START;
                lptri[num_tri - num_sections + i].wFlags |= D3DTRIFLAG_START;
            } else {
                lptri[i].wFlags |= D3DTRIFLAG_EVEN;
                lptri[num_tri - num_sections + i].wFlags |= D3DTRIFLAG_EVEN;
            }
            
        }
    } else {
        for (i = 0; i < num_sections; i++) {
            lptri[i].v1 = 0;
            lptri[i].v2 = i + 1;
            lptri[i].v3 = 1 + ((i + 1) % num_sections);
            lptri[i].wFlags = D3DTRIFLAG_EDGEENABLE1;
                              D3DTRIFLAG_EDGEENABLE2;
            lptri[num_tri - num_sections + i].v1 = num_v - 1;
            lptri[num_tri - num_sections + i].v2 = num_v - 2 - i;
            lptri[num_tri - num_sections + i].v3 = num_v - 2 - 
                    ((1 + i) % num_sections);
            lptri[num_tri - num_sections + i].wFlags= D3DTRIFLAG_EDGEENABLE1 |
                                                      D3DTRIFLAG_EDGEENABLE2;
        }
    }

    /*
     * Generate triangles for the rings
     */
    m = 1; /* first vertex in current ring,begins at 1 to skip top point*/
    n = num_sections; /* triangle being generated, skip the top cap */
        for (i = 0; i < num_rings; i++) {
        for (j = 0; j < num_sections; j++) {
            lptri[n].v1 = m + j;
            lptri[n].v2 = m + num_sections + j;
            lptri[n].v3 = m + num_sections + ((j + 1) % num_sections);
            lptri[n].wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
            
            /*
             * Start a two triangle flat fan for each face.
             */
                    
            lptri[n].wFlags = D3DTRIFLAG_STARTFLAT(1);
            
            /*
             * only need two edges for wireframe.
             */ 
            lptri[n].wFlags |= D3DTRIFLAG_EDGEENABLE1 |
                               D3DTRIFLAG_EDGEENABLE2;
        
            
            lptri[n + 1].v1 = lptri[n].v1;
            lptri[n + 1].v2 = lptri[n].v3;
            lptri[n + 1].v3 = m + ((j + 1) % num_sections);
            
            lptri[n + 1].wFlags = D3DTRIFLAG_EVEN;
            /*
             * only need two edges for wireframe.
             */ 
            lptri[n + 1].wFlags |= D3DTRIFLAG_EDGEENABLE2 |
                                   D3DTRIFLAG_EDGEENABLE3;
            n += 2;
        }
        m += num_sections;
    }
    return TRUE;
}

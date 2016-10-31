// --------------------
//
// collision.cpp
//
// Copyright (C) 1995-2001 Volition, Inc.
// All rights reserved
//
// This code is meant for illustrative purposes only.  Volition takes 
//  no responsibility for any Bad Things(TM) that happen as a result 
//  of using it.  Use at your own risk.
//
// --------------------

#include <math.h>
#include "vector.h"


// --------------------
//
// Defines
//
// --------------------

#define CLIP_RIGHT	(1<<0)	// cohen-sutherland clipping outcodes
#define CLIP_LEFT		(1<<1)
#define CLIP_TOP		(1<<2)
#define CLIP_BOTTOM	(1<<3)
#define CLIP_FRONT	(1<<4)
#define CLIP_BACK		(1<<5)


// --------------------
//
// Helper Macros
//
// --------------------


// --------------------
//
// Enumerated Types
//
// --------------------


// --------------------
//
// Structures
//
// --------------------


// --------------------
//
// Classes
//
// --------------------


// --------------------
//
// Global Variables
//
// --------------------


// --------------------
//
// Local Variables
//
// --------------------


// --------------------
//
// Internal Functions
//
// --------------------

// calculates the cohen-sutherland outcode for a point and a bounding box.
//
// bbox_min:	min vector of the bounding box
// bbox_max:	max vector of the bounding box
// pnt:			the point to check
//
// returns:		the outcode
//
static ulong calc_outcode( vector &bbox_min, vector &bbox_max, vector &pnt )
{
	ulong outcode = 0;

	if( pnt.x > bbox_max.x ) {
		outcode |= CLIP_RIGHT;
	} else if( pnt.x < bbox_min.x ) {
		outcode |= CLIP_LEFT;
	}
	if( pnt.y > bbox_max.y ) {
		outcode |= CLIP_TOP;
	} else if( pnt.y < bbox_min.y ) {
		outcode |= CLIP_BOTTOM;
	}
	if( pnt.z > bbox_max.z ) {
		outcode |= CLIP_BACK;
	} else if( pnt.z < bbox_min.z ) {
		outcode |= CLIP_FRONT;
	}

	return outcode;
}


// --------------------
//
// External Functions
//
// --------------------

// checks if a line crosses from the front of a plane to the back
//
// start:	start point of the line
// dir:	direction of the line, does not need to be normalized
// p:		the plane
// fraction: (OUT) how far along the line it crossed the plane
//
// returns: true if the line crossed from front to back
//
bool collide_line_plane( vector &start, vector &dir, plane &p, float &fraction )
{
	float dist, len;

	dist = start * p.normal + p.offset;
	if( dist < 0 ) {
		// behind plane
		return false;
	}

	len = -(dir * p.normal);
	if( len < dist ) {
		// moving away from plane or point too far away
		return false;
	}

	// 
	fraction = dist / len;
	return true;
}


// checks if a sphere crosses from the front of a plane to the back
//
// start:	start point of the line
// dir:	direction of the line, does not need to be normalized
// radius:	the radius of the sphere
// p:		the plane
// fraction: (OUT) how far along the line it crossed the plane
//
// returns: true if the sphere crossed from front to back
//
bool collide_sphere_plane( vector &start, vector &dir, float radius, plane &p, float &fraction, vector &hit_point )
{
	// find the closest point on the sphere to the plane
	vector sphere_bottom = start - (p.normal * radius);

	// collide the point like a regular ray
	if( !collide_line_plane(sphere_bottom, dir, p, fraction) ) {
		// no dice
		return false;
	}

	// we have a winner
	hit_point = sphere_bottom + dir * fraction;
	return true;
}

						
// see if a point in inside a face by projecting into 2d.
//
// hit_point:	the point to test
// verts:		polygon vertices
// norm1:		polygon normal
//
// returns: true if hit point is within polygon
//
bool point_in_face( vector &hit_point, vector *verts[], vector &norm1 )
{
	// given largest component of normal, return i & j
	static int ij_table[3][2] = {	{2,1},	// norm x biggest
									{0,2},	// norm y biggest
									{1,0} };// norm z biggest	int i0, i1,i2;

	float u0, u1, u2, v0, v1, v2;
	int i0, i1, i2;
	float alpha, beta;

	// find largest component of normal
	float abs_x = (float)fabs(norm1.x); 
	float abs_y = (float)fabs(norm1.y); 
	float abs_z = (float)fabs(norm1.z);
	if( abs_x > abs_y ) {
		if( abs_x > abs_z ) {
			i0 = 0; 
		} else {
			i0 = 2;
		}
	} else {
		if( abs_y > abs_z ) {
			i0 = 1;
		} else {
			i0 = 2;
		}
	}

	// pretend that norm1 is an array of three floats.
	float *norm1_xyz = &norm1.x;
	// figure out the two dimensions we are going to use
	if( norm1_xyz[i0] > 0.0f ) {
		i1 = ij_table[i0][0];
		i2 = ij_table[i0][1];
	} else {
		i1 = ij_table[i0][1];
		i2 = ij_table[i0][0];
	}

	// pretend that hit_point and *(verts[N]) are arrays of three floats.
	float *hit_point_xyz = &hit_point.x;
	float *verts0_xyz = &(verts[0]->x);
	float *verts1_xyz = &(verts[1]->x);
	float *verts2_xyz = &(verts[2]->x);

	// get deltas for hitpoint
	u0 = hit_point_xyz[i1] - verts0_xyz[i1];
	v0 = hit_point_xyz[i2] - verts0_xyz[i2];
	// ditto edge 1
	u1 = verts1_xyz[i1] - verts0_xyz[i1]; 
	v1 = verts1_xyz[i2] - verts0_xyz[i2];
	// ditto edge 2
	u2 = verts2_xyz[i1] - verts0_xyz[i1];
	v2 = verts2_xyz[i2] - verts0_xyz[i2];

	// calculate alpha and beta
	if( (u1 > -0.0001f) && (u1 < 0.0001f) ) {
		// special case to guard against divide by zero
		beta = u0 / u2;
		if( (beta >= 0.0f) && (beta <= 1.0f) ) {
			alpha = (v0 - beta*v2) / v1;
			return ((alpha >= 0.0f) && (alpha+beta <= 1.0f));
		}
	} else {
		beta = (v0*u1 - u0*v1) / (v2*u1 - u2*v1);
		if( (beta >= 0.0f) && (beta <= 1.0f) )	{
			alpha = (u0 - beta*u2) / u1;
			return ((alpha >= 0.0f) && (alpha+beta <= 1.0f));
		}
	}

	// not inside polygon
	return false;
}


// given an edge of a polygon and a moving sphere, find the first contact the sphere 
//  makes with the edge, if any.  note that hit_time must be primed with a  value of 1
//  before calling this function the first time.  it will then maintain the closest 
//  collision in subsequent calls.
//
// xs0:			start point (center) of sphere
// vs: 			path of sphere during frame
// rad:			radius of sphere
// v0:			vertex #1 of the edge
// v1:			vertex #2 of the edge
// hit_time:	(OUT) time at which sphere collides with polygon edge
// hit_point:	(OUT) point on edge that is hit
//
// returns - whether the edge (or it's vertex) was hit
bool collide_edge_sphereline( const vector &xs0, const vector &vs, float rad, const vector &v0, const vector &v1, float &hit_time, vector &hit_point )
{
	static vector temp_sphere_hit;
	bool try_vertex = false; // Assume we don't have to try the vertices.

	vector ve = v1 - v0;
	vector delta = xs0 - v0;
	float delta_dot_ve = delta * ve;
	float delta_dot_vs = delta * vs;
	float delta_sqr = delta.mag_squared();
	float ve_dot_vs = ve * vs;
	float ve_sqr = ve.mag_squared();
	float vs_sqr = vs.mag_squared();

	float temp;

	// position of the collision along the edge is given by: xe = v0 + ve*s, where s is
	//  in the range [0,1].  position of sphere along its path is given by: 
	//  xs = xs + vs*t, where t is in the range [0,1].  t is time, but s is arbitrary.
	//
	// solve simultaneous equations
	// (1) distance between edge and sphere center must be sphere radius
	// (2) line between sphere center and edge must be perpendicular to edge
	//
	// (1) (xe - xs)*(xe - xs) = rad*rad
	// (2) (xe - xs) * ve = 0
	//
	// then apply mathematica

	float A, B, C, root, discriminant;
	float root1 = 0.0f;
	float root2 = 0.0f;
	A = ve_dot_vs * ve_dot_vs - ve_sqr * vs_sqr;
	B = 2 * (delta_dot_ve * ve_dot_vs - delta_dot_vs * ve_sqr);
	C = delta_dot_ve * delta_dot_ve + rad * rad * ve_sqr - delta_sqr * ve_sqr;

	if( A > -0.0001f && A < 0.0001f ) {
		// degenerate case, sphere is traveling parallel to edge
		try_vertex = true;
	} else {
		discriminant = B*B - 4*A*C;
		if( discriminant > 0 ) {
			root = (float)sqrt(discriminant);
			root1 = (-B + root) / (2 * A);
			root2 = (-B - root) / (2 * A);

			// sort root1 and root2, use the earliest intersection.  the larger root 
			//  corresponds to the final contact of the sphere with the edge on its 
			//  way out.
			if( root2 < root1 ) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// root1 is a time, check that it's in our currently valid range
			if( (root1 < 0) || (root1 >= hit_time) ) {
				return false;
			}

			// find sphere and edge positions
			temp_sphere_hit = xs0 + vs * root1;

			// check if hit is between v0 and v1
			float s_edge = ((temp_sphere_hit - v0) * ve) / ve_sqr;
			if( (s_edge >= 0) && (s_edge <= 1) ) {
				// bingo
				hit_time = root1;
				hit_point = v0 + ve * s_edge;
				return true;
			}
		} else {
			// discriminant negative, sphere passed edge too far away
			return false;
		}
	}

	// sphere missed the edge, check for a collision with the first vertex.  note
	//  that we only need to check one vertex per call to check all vertices.
		A = vs_sqr;
		B = 2 * delta_dot_vs;
		C = delta_sqr - rad * rad;

		discriminant = B*B - 4*A*C;
		if( discriminant > 0 ) {
			root = (float)sqrt(discriminant);
			root1 = (-B + root) / (2 * A);
			root2 = (-B - root) / (2 * A);

			// sort the solutions
			if( root1 > root2 ) {
				temp = root1;
				root1 = root2;
				root2 = temp;
			}

			// check hit vertex is valid and earlier than what we already have
			if( (root1 < 0) || (root1 >= hit_time) ) {
				return false;
			}
		} else {
			// discriminant negative, sphere misses vertex too
			return false;
		}

	// bullseye
	hit_time = root1;
	hit_point = v0;
	return true;
}


// tests two axis-aligned bounding boxes for intersection
//
// min1:	- min vector of bounding box 1
// max1:	- max vector of bounding box 1
// min2:	- min vector of bounding box 2
// max2:	- max vector of bounding box 2
//
// returns true if boxes intersect
//
bool ix_box_box_aligned( vector &min1, vector &max1, vector &min2, vector &max2 )
{
	if( min1.x > max2.x || min1.y > max2.y || min1.z > max2.z || max1.x < min2.x || max1.y < min2.y || max1.z < min2.z ) {
		return false;
	}	

	return true;
}


// determines if a linesegment intersects a bounding box. this is based on
//  the cohen-sutherland line-clipping algorithm.
//
// bbox_min:	bounding box min vector
// bbox_max:	bounding box max vector
// p1:			end point of line segment
// p2:			other end point
// intercept:	(out) the point in/on the bounding box where the intersection 
//				  occured.  note that this point may not be on the surface of the box.
//
// returns:		true if the segment and box intersect.
//
bool collide_linesegment_boundingbox( vector &bbox_min, vector &bbox_max, vector &p1, vector &p2, vector &intercept )
{
	ulong outcode1, outcode2;

	outcode1 = calc_outcode( bbox_min, bbox_max, p1 );
	if( outcode1 == 0 ) {
		// point inside bounding box
		intercept = p1;
		return true;
	}

	outcode2 = calc_outcode( bbox_min, bbox_max, p2 );
	if( outcode2 == 0 ) {
		// point inside bounding box
		intercept = p2;
		return true;
	}

	if( (outcode1 & outcode2) > 0 ) {
		// both points on same side of box
		return false;
	}

	// check intersections
	if( outcode1 & (CLIP_RIGHT | CLIP_LEFT) ) {
		if( outcode1 & CLIP_RIGHT ) {
			intercept.x = bbox_max.x;
		} else {
			intercept.x = bbox_min.x;
		}
		float x1 = p2.x - p1.x;
		float x2 = intercept.x - p1.x;
		intercept.y = p1.y + x2 * (p2.y - p1.y) / x1;
		intercept.z = p1.z + x2 * (p2.z - p1.z) / x1;

		if( intercept.y <= bbox_max.y && intercept.y >= bbox_min.y && intercept.z <= bbox_max.z && intercept.z >= bbox_min.z ) {
			return true;
		}
	}

	if( outcode1 & (CLIP_TOP | CLIP_BOTTOM) ) {
		if( outcode1 & CLIP_TOP ) {
			intercept.y = bbox_max.y;
		} else {
			intercept.y = bbox_min.y;
		}
		float y1 = p2.y - p1.y;
		float y2 = intercept.y - p1.y;
		intercept.x = p1.x + y2 * (p2.x - p1.x) / y1;
		intercept.z = p1.z + y2 * (p2.z - p1.z) / y1;

		if( intercept.x <= bbox_max.x && intercept.x >= bbox_min.x && intercept.z <= bbox_max.z && intercept.z >= bbox_min.z ) {
			return true;
		}
	}

	if( outcode1 & (CLIP_FRONT | CLIP_BACK) ) {
		if( outcode1 & CLIP_BACK ) {
			intercept.z = bbox_max.z;
		} else {
			intercept.z = bbox_min.z;
		}
		float z1 = p2.z - p1.z;
		float z2 = intercept.z - p1.z;
		intercept.x = p1.x + z2 * (p2.x - p1.x) / z1;
		intercept.y = p1.y + z2 * (p2.y - p1.y) / z1;

		if( intercept.x <= bbox_max.x && intercept.x >= bbox_min.x && intercept.y <= bbox_max.y && intercept.y >= bbox_min.y ) {
			return true;
		}
	}

	// nothing found
	return false;
}


// determines a collision between a ray and a sphere
//
// ray_start:	the start pos of the ray
// ray_dir:		the normalized direction of the ray
// length:		length of ray to check
// sphere_pos:	sphere position
// sphere_rad:	sphere redius
// hit_time:	(OUT) if a collision, contains the distance from ray.pos
// hit_pos:		(OUT) if a collision, contains the world point of the collision
//
// returns: true if a collision occurred
//
bool collide_ray_sphere( vector &ray_start, vector &ray_dir, float length, vector &sphere_pos, float sphere_rad, float &hit_time, vector &hit_pos )
{
	// get the offset vector
	vector offset = sphere_pos - ray_start;

	// get the distance along the ray to the center point of the sphere
	float ray_dist = ray_dir * offset;
	if( ray_dist <= 0 || (ray_dist - length) > sphere_rad) {
		// moving away from object or too far away
		return false;
	}

	// get the squared distances
	float off2 = offset * offset;
	float rad2 = sphere_rad * sphere_rad;
	if( off2 <= rad2 ) {
		// we're in the sphere
		hit_pos = ray_start;
		hit_time = 0;
		return true;
	}

	// find hit distance squared
	float d = rad2 - (off2 - ray_dist * ray_dist);
	if( d < 0 ) {
		// ray passes by sphere without hitting
		return false;
	}

	// get the distance along the ray
	hit_time = (float)(ray_dist - sqrt( d ));
	if( hit_time > length ) {
		// hit point beyond length
		return false;
	}

	// sort out the details
	hit_pos = ray_start + ray_dir * hit_time;
	hit_time /= length;
	return true;
}



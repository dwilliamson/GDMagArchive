// MissingDX7Stuff.h App Wizard Version 2.0 Beta 1
// ----------------------------------------------------------------------
// 
// Copyright © 2001 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative works of this 
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  This software is provided "AS IS." 
//
// Intel specifically disclaims all warranties, express or implied, and all liability,
// including consequential and other indirect damages, for the use of this software, 
// including liability for infringement of any proprietary rights, and including the 
// warranties of merchantability and fitness for a particular purpose.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// ----------------------------------------------------------------------
// Authors: Kim Pallister,Dean Macri - Intel Technology Diffusion Team
// ----------------------------------------------------------------------

#if !defined(IawMissingDx7Stuff_h)
#define IawMissingDX7Stuff_h

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
#ifndef Dx8Shell_inc
#define Dx8Shell_inc

#define STRICT
#define D3D_OVERLOADS 1

*/

#if !defined(D3D_OVERLOADS)
#define D3D_OVERLOADS
#endif

typedef float D3DVALUE;
#define DIRECT3D_VERSION 0x0800

//#include <windows.h>
//#include <mmsystem.h>
//#include <ddraw.h>
//#include <d3d.h>

//#include <d3d8.h>

//some other stuff that didn't get defined in DX8 that was there in DX7
#define D3DFVF_VERTEX ( D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 )
#define D3DFVF_LVERTEX ( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )
#define D3DFVF_TLVERTEX ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1 )

#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
    union {
    D3DVALUE x;
    D3DVALUE dvX;
    };
    union {
    D3DVALUE y;
    D3DVALUE dvY;
    };
    union {
    D3DVALUE z;
    D3DVALUE dvZ;
    };
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)

public:

    // =====================================
    // Constructors
    // =====================================

    _D3DVECTOR() { }
    _D3DVECTOR(D3DVALUE f);
    _D3DVECTOR(D3DVALUE _x, D3DVALUE _y, D3DVALUE _z);
    _D3DVECTOR(const D3DVALUE f[3]);

    // =====================================
    // Access grants
    // =====================================

    const D3DVALUE&operator[](int i) const;
    D3DVALUE&operator[](int i);

    // =====================================
    // Assignment operators
    // =====================================

    _D3DVECTOR& operator += (const _D3DVECTOR& v);
    _D3DVECTOR& operator -= (const _D3DVECTOR& v);
    _D3DVECTOR& operator *= (const _D3DVECTOR& v);
    _D3DVECTOR& operator /= (const _D3DVECTOR& v);
    _D3DVECTOR& operator *= (D3DVALUE s);
    _D3DVECTOR& operator /= (D3DVALUE s);

    // =====================================
    // Unary operators
    // =====================================

    friend _D3DVECTOR operator + (const _D3DVECTOR& v);
    friend _D3DVECTOR operator - (const _D3DVECTOR& v);


    // =====================================
    // Binary operators
    // =====================================

    // Addition and subtraction
        friend _D3DVECTOR operator + (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR operator - (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
    // Scalar multiplication and division
        friend _D3DVECTOR operator * (const _D3DVECTOR& v, D3DVALUE s);
        friend _D3DVECTOR operator * (D3DVALUE s, const _D3DVECTOR& v);
        friend _D3DVECTOR operator / (const _D3DVECTOR& v, D3DVALUE s);
    // Memberwise multiplication and division
        friend _D3DVECTOR operator * (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR operator / (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

    // Vector dominance
        friend int operator < (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend int operator <= (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

    // Bitwise equality
        friend int operator == (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

    // Length-related functions
        friend D3DVALUE SquareMagnitude (const _D3DVECTOR& v);
        friend D3DVALUE Magnitude (const _D3DVECTOR& v);

    // Returns vector with same direction and unit length
        friend _D3DVECTOR Normalize (const _D3DVECTOR& v);

    // Return min/max component of the input vector
        friend D3DVALUE Min (const _D3DVECTOR& v);
        friend D3DVALUE Max (const _D3DVECTOR& v);

    // Return memberwise min/max of input vectors
        friend _D3DVECTOR Minimize (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR Maximize (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

    // Dot and cross product
        friend D3DVALUE DotProduct (const _D3DVECTOR& v1, const _D3DVECTOR& v2);
        friend _D3DVECTOR CrossProduct (const _D3DVECTOR& v1, const _D3DVECTOR& v2);

#endif
#endif // DIRECT3D_VERSION >= 0x0500 
} D3DVECTOR;
#define D3DVECTOR_DEFINED
#endif
typedef struct _D3DVECTOR *LPD3DVECTOR;



#ifndef D3DMATRIX_DEFINED
typedef struct _D3DMATRIX {
#if(DIRECT3D_VERSION >= 0x0500)
#if (defined __cplusplus) && (defined D3D_OVERLOADS)
    union {
        struct {
#endif

#endif // DIRECT3D_VERSION >= 0x0500 
            D3DVALUE        _11, _12, _13, _14;
            D3DVALUE        _21, _22, _23, _24;
            D3DVALUE        _31, _32, _33, _34;
            D3DVALUE        _41, _42, _43, _44;

#if(DIRECT3D_VERSION >= 0x0500)
			
#if (defined __cplusplus) && (defined D3D_OVERLOADS)

        };
        D3DVALUE m[4][4];
    };
    _D3DMATRIX() { }
    _D3DMATRIX( D3DVALUE _m00, D3DVALUE _m01, D3DVALUE _m02, D3DVALUE _m03,
                D3DVALUE _m10, D3DVALUE _m11, D3DVALUE _m12, D3DVALUE _m13,
                D3DVALUE _m20, D3DVALUE _m21, D3DVALUE _m22, D3DVALUE _m23,
                D3DVALUE _m30, D3DVALUE _m31, D3DVALUE _m32, D3DVALUE _m33
        )
        {
                m[0][0] = _m00; m[0][1] = _m01; m[0][2] = _m02; m[0][3] = _m03;
                m[1][0] = _m10; m[1][1] = _m11; m[1][2] = _m12; m[1][3] = _m13;
                m[2][0] = _m20; m[2][1] = _m21; m[2][2] = _m22; m[2][3] = _m23;
                m[3][0] = _m30; m[3][1] = _m31; m[3][2] = _m32; m[3][3] = _m33;
        }

    D3DVALUE& operator()(int iRow, int iColumn) { return m[iRow][iColumn]; }
    const D3DVALUE& operator()(int iRow, int iColumn) const { return m[iRow][iColumn]; }
#if(DIRECT3D_VERSION >= 0x0600)
    friend _D3DMATRIX operator* (const _D3DMATRIX&, const _D3DMATRIX&);
#endif // DIRECT3D_VERSION >= 0x0600 
#endif
#endif // DIRECT3D_VERSION >= 0x0500 
} D3DMATRIX;
#define D3DMATRIX_DEFINED
#endif

#undef DIRECT3D_VERSION

/*

*/

#endif // IawMissingDX7Stuff_h

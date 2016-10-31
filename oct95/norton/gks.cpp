// Module:   gks.cpp
// rev.  :   1.00       10/31/93
//
// Compilers: Microsoft Visual C++ 1.5, 2.0, Watcom 10.0
// Environments:  16 bit {all}; 32 bit {DOS4GW, PHARLAP TNT}
// Author:    Michael Norton
//  Functionality: 
//         Graphics Kernal System (GKS)
//         This is the header for a mini-GKS object.
//         GKS defines the objects Point and Rect.
//         Point is the basic class for defining 
//         a Rect class.  
//         2 Point objects comprise a Line object.
//
//         pixelDyne  copyright 1993, 1994, 1995
//////////////////////////////////////////////////////////////////////////////  

// I N C L U D E S ///////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

#include "gks.hpp"


// FUNCTIONS /////////////////////////////////////////////////////////////////

Point::Point( unsigned int px, unsigned int py )
{
    x = px;
    y = py;
}

//////////////////////////////////////////////////////////////////////////////  

Point::Point( const Point& P)
{
    x = P.x;
    y = P.y;
}

//////////////////////////////////////////////////////////////////////////////  

Point& Point::operator=(const Point& P)
{
    x = P.x;
    y = P.y;

    return *this;
}

//////////////////////////////////////////////////////////////////////////////  

void Point::show()
{
    printf(" x = %d\n", x);
    printf(" y = %d\n", y);
}

//////////////////////////////////////////////////////////////////////////////  

Line::Line( unsigned int px1, unsigned int py1, 
               unsigned int px2, unsigned int py2 )
{
    x1 = px1; y1 = py1; x2 = px2; y2 = py2;
}

//////////////////////////////////////////////////////////////////////////////  

Line::Line( const Point& P1, const Point& P2 )
{
    x1 = P1.x; y1 = P1.y;
    x2 = P2.x; y2 = P2.y;
}

//////////////////////////////////////////////////////////////////////////////  

Line::Line(const Line& l)
{
    x1 = l.x1; y1 = l.y1; 
    x2 = l.x2; y2 = l.y2;
}

//////////////////////////////////////////////////////////////////////////////  

Line& Line::operator=(const Line& l)
{
    x1 = l.x1; y1 = l.y1; x2 = l.x2; y2 = l.y2;
    return *this;
}

//////////////////////////////////////////////////////////////////////////////  

Rect::Rect( unsigned int l, unsigned int t, unsigned int r,unsigned int b )
{
    left = l; top = t; right = r; bottom = b;
}

//////////////////////////////////////////////////////////////////////////////  

Rect::Rect( const Point& P1, const Point& P2 )
{
    left = P1.x;
    top = P1.y;
    right = P2.x;
    bottom = P2.y;
}

//////////////////////////////////////////////////////////////////////////////  

Rect::Rect( const Rect& r )
{
    left = r.left;
    top = r.top;
    right = r.right;
    bottom = r.bottom;
}

//////////////////////////////////////////////////////////////////////////////  

Rect& Rect::operator=(const Rect& r)
{
    left = r.left;
    top = r.top;
    right = r.right;
    bottom = r.bottom;

    return *this;
}

//////////////////////////////////////////////////////////////////////////////  

void Rect::setRect( unsigned int l, unsigned int t, unsigned int r,unsigned int b )
{
    left = l; top = t; right = r; bottom = b;
}

//////////////////////////////////////////////////////////////////////////////  

void Rect::setRect( const Point& P1, const Point& P2 )
{
    left = P1.x;
    top = P1.y;
    right = P2.x;
    bottom = P2.y;
}

//////////////////////////////////////////////////////////////////////////////  


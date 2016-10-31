// Header:   gks.hpp
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
#ifndef _GKS_HPP
#define _GKS_HPP

// CLASS BODY DEFINITION /////////////////////////////////////////////////////

class Point {
public:
    unsigned int x, y;

    Point(){ x = y = 0; };
    Point( unsigned int, unsigned int );
    // copy constructor
    Point( const Point& );
    Point& operator=( const Point& );
    void show();
    // destructor
    ~Point(){};
};

//////////////////////////////////////////////////////////////////////////////

class Line {
    // data members
    unsigned int x1, y1, x2, y2;

    Line(){ x1 = y1 = x2 = y2 = 0;};
    Line( unsigned int, unsigned int, unsigned int, unsigned int );
    Line( const Point&, const Point& );
    // copy constructor
    Line( const Line& );
    Line& operator=( const Line& );
    ~Line(){};
};

//////////////////////////////////////////////////////////////////////////////

class Rect { 
public:
    // data members
    unsigned int left, top, right, bottom;

    // constructor;
    Rect(){ left = top = right = bottom = 0;};
    Rect( unsigned int, unsigned int, unsigned int, unsigned int );
    Rect( const Point&, const Point& );
    Rect( const Rect& );
    Rect& operator=( const Rect& );
    void setRect( unsigned int, unsigned int, unsigned int, unsigned int );
    void setRect( const Point&, const Point& );
    // destructor
    ~Rect(){};
};

//////////////////////////////////////////////////////////////////////////////

#endif

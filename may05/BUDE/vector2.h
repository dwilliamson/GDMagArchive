/////////////////////////////////////////////////////////////////////////////////////////
// Vector2.h
// A simple 2D floating point vector class
// allowing for basic vector arithmatic
//
// Copyright (c) 2005 Mick West
// http://mickwest.com/PlayerControl
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.


class Vector2
{
    public:

            Vector2         ();    
   			Vector2         ( const Vector2& v );
   			Vector2         ( float xx, float yy );
			Vector2& 		operator=	( const Vector2& v );
			bool			operator==  ( const Vector2& v ) const;
			bool			operator!=  ( const Vector2& v ) const;
        	Vector2&		operator+=	( const Vector2& v );
        	Vector2&		operator-=	( const Vector2& v );
        	Vector2&		operator*=	( const float s );
			Vector2&		operator/=	( float s );
            float           Length2() { return x*x + y*y;}
            float           Length() { return sqrtf(Length2());}
            Vector2         Normal();

// The actual vector is stored as two floats.
// could be private, but it's a lot simpler if it's not  
            float   x,y;
        
};

Vector2		operator+ ( const Vector2& v1, const Vector2& v2 );
Vector2		operator- ( const Vector2& v1, const Vector2& v2 );
Vector2		operator* ( const Vector2& v1, const Vector2& v2 );
Vector2		operator* ( const Vector2& v, const float s );
Vector2		operator* ( const float s, const Vector2& v );
Vector2		operator/ ( const Vector2& v, const float s );
Vector2		operator- ( const Vector2& v );

float       DotProduct( const Vector2& v1, const Vector2& v2 ) {return v1.x*v2.x + v1.y*v2.y;} 

inline Vector2::Vector2 ( ) 
{
    
}

inline Vector2::Vector2 ( float xx, float yy ) 
{
	x = xx;
    y = yy;
}

inline Vector2::Vector2 ( const Vector2& v ) 
{
	*this = v;
}

inline Vector2&		Vector2::operator=	( const Vector2& v )
{	
	x = v.x;
	y = v.y;
	return *this;
}

inline bool		  	Vector2::operator== ( const Vector2& v ) const
{	
	return (( x == v.x ) && ( y == v.y ));
}

inline bool		  	Vector2::operator!= ( const Vector2& v ) const
{	
	return !( *this == v );
}

inline Vector2&		Vector2::operator+= ( const Vector2& v )
{
	x += v.x;
	y += v.y;
	return *this;
}		

inline Vector2&		Vector2::operator-= ( const Vector2& v )
{
	x -= v.x;
	y -= v.y;
	return *this;
}

inline Vector2&		Vector2::operator*= ( const float s )
{
	x *= s;
	y *= s;
	return *this;
}

inline Vector2&		Vector2::operator/= ( float s )
{
	x /= s;
	y /= s;
	return *this;
}

inline	Vector2	operator+	( const Vector2& v1, const Vector2& v2 )
{	
	return Vector2 ( v1.x + v2.x, v1.y + v2.y );
}

inline	Vector2	operator-	( const Vector2& v1, const Vector2& v2 )
{
	return Vector2 ( v1.x - v2.x, v1.y - v2.y );
}

inline	Vector2	operator*	( const Vector2& v1, const Vector2& v2 )
{
	return Vector2 ( v1.x * v2.x, v1.y * v2.y );
}


inline	Vector2	operator*	( const Vector2& v, const float s )
{
	return Vector2 ( v.x * s, v.y * s );
}

inline	Vector2	operator*	( const float s, const Vector2& v )
{
	return v * s;
}

inline	Vector2	operator/	( const Vector2& v, const float s )
{
	return v * (1.0f / s) ;
}

inline	Vector2	operator-	( const Vector2& v )
{
	return Vector2 ( -v.x, -v.y );
}

inline Vector2 Vector2::Normal()
{
    Vector2 t;
    float len = Length();
    t.x = x/len;
    t.y = y/len;
    return t;
}

// End of Vector2
/////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////
// ICol2D.h  - Interface to 2D collision
//
//
#ifndef __ICOL2D_H__
#define __ICOL2D_H__

// ICol2D is an interface class for collision detection
class	ICol2D
{
    public:
                    ICol2D();
                    ICol2D(Vector2 &start, Vector2 &end);
        void        Set(Vector2 &start, Vector2 &end);        
        virtual bool        CheckCollision() = 0;
        bool        CheckCollision(Vector2 &start, Vector2 &end);        
        Vector2     GetNormal() {return m_normal;}
        Vector2     GetPoint()  {return m_point;}
        Vector2     GetStart()  {return m_start;}
        Vector2     GetEnd()  {return m_end;}
    protected:
        // Input data
        Vector2     m_start, m_end;
        // Output/Calculated Data
        Vector2     m_point, m_normal;
};

inline ICol2D::ICol2D()
{
    m_start.x = 0.0f;
    m_start.y = 0.0f;
    m_end.x = 1.0f;
    m_start.y = 1.0f;
	//m_point = m_normal = Vector2(0,0);
}

inline ICol2D::ICol2D(Vector2 &start, Vector2 &end)
{
    m_start = start;
    m_end = end;
}

inline void ICol2D::Set(Vector2 &start, Vector2 &end)
{
    m_start = start;
    m_end = end;
}

inline bool ICol2D::CheckCollision(Vector2 &start, Vector2 &end)
{
    Set(start,end);
    return CheckCollision(); 
}


#endif

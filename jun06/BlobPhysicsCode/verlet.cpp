// verlet.h
// Copyright (c) 2006 Mick West
// http://mickwest.com/
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

#include "verlet.h"
#include "assert.h"
#define	ASSERT assert

// Some debuggery
typedef unsigned long       DWORD;
void    debug_log( const char* text, ...);
void DrawLine(Vector2 start , Vector2 end, DWORD color);


CVerletPoint::CVerletPoint()
{
	m_force = Vector2(0.0f,0.0f);
	m_mass = 1.0f;
}

void	CVerletPoint::Set(Vector2 point)
{
	m_pos = m_last_pos = point;	
}

CRigidConstraint::CRigidConstraint(CVerletPoint*  p_other_point, float radius)
{
	mp_other_verlet = p_other_point;
	m_radius = radius;
}

Vector2 CRigidConstraint::GetForce(CVerletPoint*  p_other_point)
{
	return Vector2(0,0);
}


// apply this constraint to a verlet (usually the one that owns this constraint).
void	CRigidConstraint::Satisfy(CVerletPoint* p_verlet)
{
	ASSERT(mp_other_verlet != 0);
	// Get vector from other verlet to me
	Vector2	to_me = p_verlet->GetPos() - mp_other_verlet->GetPos();
	Vector2	mid = (p_verlet->GetPos() + mp_other_verlet->GetPos())/2.0f;
	// handle case where points are the same
	if (to_me.Length() < 0.0001)
	{
		to_me = Vector2(1.0f,0.0f);
	}
	// Scale it to the required radius
	to_me = m_radius * to_me.Normal();
	// and set the point
#if 0
	p_verlet->SetPos(mp_other_verlet->GetPos() + to_me);
#else
	p_verlet->SetPos(mid + to_me/2.0f);
	mp_other_verlet->SetPos(mid - to_me/2.0f);
	//debug_log("(p_verlet(%.3f,%.3f), mp_other(%.3f,%.3f)",p_verlet->GetPos().x,p_verlet->GetPos().y,mp_other_verlet->GetPos().x,mp_other_verlet->GetPos().y);
	//debug_log("(to_me(%.3f,%.3f), (len= %.3f) ",to_me.x,to_me.y,to_me.Length());
#endif
}

void	CRigidConstraint::debug_render(CVerletPoint* p_verlet)
{
	DrawLine(p_verlet->GetPos(),mp_other_verlet->GetPos(),0xffff0000);
}


CSemiRigidConstraint::CSemiRigidConstraint(CVerletPoint*  p_other_point, float min, float mid, float max, float force)
{
	mp_other_verlet = p_other_point;
	m_min = min;
	m_mid = mid;
	m_max = max;
	m_force = force;
}

Vector2	CSemiRigidConstraint::GetForce(CVerletPoint* p_verlet)
{
	Vector2	to_me = p_verlet->GetPos() - mp_other_verlet->GetPos();
	// handle case where points are the same
	if (to_me.Length() < 0.000001)
	{
		to_me = Vector2(1.0f,0.0f);
	}
	Vector2	mid = mp_other_verlet->GetPos() + to_me.Normal()*m_mid;
	Vector2	to_mid = mid-p_verlet->GetPos() ;
	//debug_log("(p_verlet(%.3f,%.3f), mp_other(%.3f,%.3f)",p_verlet->GetPos().x,p_verlet->GetPos().y,mp_other_verlet->GetPos().x,mp_other_verlet->GetPos().y);
	//debug_log("(to_me(%.3f,%.3f), (len= %.3f) %.3f,%.3f",to_me.x,to_me.y,to_me.Length(),to_mid.x,to_mid.y);

	return to_mid*m_force;  // --- ????
}

// apply this constraint to a verlet (usually the one that owns this constraint).
void	CSemiRigidConstraint::Satisfy(CVerletPoint* p_verlet)
{
	ASSERT(mp_other_verlet != 0);
	// Get vector from other verlet to me
	Vector2	to_me = p_verlet->GetPos() - mp_other_verlet->GetPos();
	Vector2	mid = (p_verlet->GetPos() + mp_other_verlet->GetPos())/2.0f;
	// handle case where points are the same
	if (to_me.Length() == 0.0f)
	{
		to_me = Vector2(1.0f,0.0f);
	}
	float radius = to_me.Length();

	
	if (radius < m_min) radius = m_min;
	if (radius > m_max) radius = m_max;

	// Scale it to the required radius
	to_me = radius * to_me.Normal();
	// and set the point
#if 0
	p_verlet->SetPos(mp_other_verlet->GetPos() + to_me);
#else
	//debug_log("Before (%.3f,%.3f), (%.3f,%.3f)", p_verlet->GetPos().x,p_verlet->GetPos().y,mp_other_verlet->GetPos().x,mp_other_verlet->GetPos().y);
	p_verlet->SetPos(mid + to_me/2.0f);
	mp_other_verlet->SetPos(mid - to_me/2.0f);
	//debug_log("After (%.3f,%.3f), (%.3f,%.3f)", p_verlet->GetPos().x,p_verlet->GetPos().y,mp_other_verlet->GetPos().x,mp_other_verlet->GetPos().y);
#endif
}


void	CSemiRigidConstraint::debug_render(CVerletPoint* p_verlet)
{
	DrawLine(p_verlet->GetPos(),mp_other_verlet->GetPos(),0xff000000);
}

// perform the verlet integration step
void	CVerletPoint::Integrate(float t)
{
	Vector2 last = m_pos;
	// Standard verlet integration step
	//m_pos += 0.992f * (m_pos - m_last_pos) + m_force/m_mass * t * t;
	//debug_log("Before (%.5f,%.5f)  Diff %f,%f", GetPos().x,GetPos().y);
	//m_pos += (m_pos - m_last_pos);
	m_pos += (m_pos - m_last_pos) + m_force/m_mass * t * t;
	//debug_log("After (%.5f,%.5f)", GetPos().x,GetPos().y);
	m_last_pos = last;


	// possible time corrected version
	// is at http://www.gamedev.net/reference/programming/features/verlet/
	// and requires you scale distance by ratio of time step for this and last frame
	// not implemented here, since I'm not anticipating a variable frame rate
	// for my tests.  But worth considering.
}

// Get constraint forces
void	CVerletPoint::GatherForces()
{

	vector<CVerletConstraint*>::iterator i;
	for (i = mp_constraints.begin(); i != mp_constraints.end(); i++)
	{
		m_force += (*i)->GetForce(this);
	}

}


// apply all the constraints
void	CVerletPoint::SatisfyConstraints()
{

	vector<CVerletConstraint*>::iterator i;
	for (i = mp_constraints.begin(); i != mp_constraints.end(); i++)
	{
		(*i)->Satisfy(this);
	}

}

// apply all the collision constraints
void	CVerletPoint::SatisfyCollisionConstraints()
{

	vector<CVerletConstraint*>::iterator i;
	for (i = mp_collision_constraints.begin(); i != mp_collision_constraints.end(); i++)
	{
		(*i)->Satisfy(this);
	}

}

// apply all the collision2 constraints
void	CVerletPoint::SatisfyCollision2Constraints()
{

	vector<CVerletConstraint*>::iterator i;
	for (i = mp_collision2_constraints.begin(); i != mp_collision2_constraints.end(); i++)
	{
		(*i)->Satisfy(this);
	}

}

// debug_render all the constraints
void	CVerletPoint::debug_render()
{

	vector<CVerletConstraint*>::iterator i;
	for (i = mp_constraints.begin(); i != mp_constraints.end(); i++)
	{
		(*i)->debug_render(this);
	}

}


CVerletPoint* CVerletSystem::CreatePoint(Vector2 pos)
{
	CVerletPoint *p = new CVerletPoint();
	p->Set(pos);
	Add(p);
	return p;
}


// Add mutual rigid constraints between two points
void	RigidConstraint(CVerletPoint* p_v1, CVerletPoint* p_v2, float r)
{
	CRigidConstraint *p_r1 = new CRigidConstraint(p_v2, r);
	p_v1->AddConstraint(p_r1);	
	
	CRigidConstraint *p_r2 = new CRigidConstraint(p_v1, r );
	p_v2->AddConstraint(p_r2);
}

void	SemiRigidConstraint(CVerletPoint* p_v1, CVerletPoint* p_v2, float min, float mid, float max, float force)
{
	CSemiRigidConstraint *p_r1 = new CSemiRigidConstraint(p_v2,  min,  mid,  max,  force);
	p_v1->AddConstraint(p_r1);	
	
	CSemiRigidConstraint *p_r2 = new CSemiRigidConstraint(p_v1,  min,  mid,  max,  force );
	p_v2->AddConstraint(p_r2);
}


// Test should:
// create a verlet system
// create three verlets at known positions
// create constraints for them
// add them to the system
// update the system (Integrate and satisfy constraints)
// verify that
bool	UnitTestVerlets()
{


	return true;
}
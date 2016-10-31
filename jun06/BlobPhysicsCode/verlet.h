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
#ifndef	__VERLET_H__
#define __VERLET_H__

#include	"vector2.h"
#include <vector>

using namespace std;

class	CVerletPoint;

// A verlet constraint is an abstract base class
// allowing you to add arbitary constraints to a verlet point with a common contraint interface
class	CVerletConstraint
{
public:
	virtual void	Satisfy(CVerletPoint* p_verlet) = 0;		// pure virtual function
	virtual Vector2	GetForce(CVerletPoint* p_verlet) = 0;
	virtual	void	debug_render(CVerletPoint* p_verlet) = 0;
};


class CVerletPoint
{
public:
	CVerletPoint();
	void	Set(Vector2 point);

	Vector2	GetPos()	{return m_pos;}
	Vector2	GetLastPos()	{return m_last_pos;}
	void	SetPos(Vector2 pos)	{m_pos = pos;}
	void	SetForce(Vector2 v) {m_force = v;};
	void	AddForce(Vector2 v) {m_force += v;};
	Vector2	GetForce() {return m_force;}
	float	GetMass() {return m_mass;}
	void	SetMass(float m) {m_mass = m;}
	void	AddConstraint(CVerletConstraint* p_c) {mp_constraints.push_back(p_c);}
	void	AddCollisionConstraint(CVerletConstraint* p_c) {mp_collision_constraints.push_back(p_c);}
	void	AddCollision2Constraint(CVerletConstraint* p_c) {mp_collision2_constraints.push_back(p_c);}
	void	Integrate(float t);
	void	SatisfyConstraints();
	void	SatisfyCollisionConstraints();
	void	SatisfyCollision2Constraints();

	void	GatherForces();

	void	debug_render();

private:
	Vector2	m_pos;
	Vector2	m_last_pos;
	Vector2	m_force;
	float	m_mass;

	// using a vector of constraint pointers
	vector<CVerletConstraint*> mp_constraints;
	// seperate list for collision constraints, so they can be applied seperately
	vector<CVerletConstraint*> mp_collision_constraints;
	// and yet another for line segment collisions
	vector<CVerletConstraint*> mp_collision2_constraints;
	

};


// A rigid constrain is a specific type of constraint
// that says this point must be a fixed radius from some other verlet point
class	CRigidConstraint : public CVerletConstraint
{
public:
	CRigidConstraint(CVerletPoint*  p_other_point, float radius);
	virtual void				Satisfy(CVerletPoint* p_verlet);
	virtual	void				debug_render(CVerletPoint* p_verlet);
	virtual Vector2				GetForce(CVerletPoint* p_verlet);
private:
	CVerletPoint*				mp_other_verlet;
	float						m_radius;
};


class	CSemiRigidConstraint : public CVerletConstraint
{
public:
	CSemiRigidConstraint(CVerletPoint*  p_other_point, float min, float mid, float max, float force);
	virtual void				Satisfy(CVerletPoint* p_verlet);
	virtual	void				debug_render(CVerletPoint* p_verlet);
	virtual Vector2				GetForce(CVerletPoint* p_verlet);
private:
	CVerletPoint*				mp_other_verlet;
	float						m_min, m_mid, m_max, m_force;
};

class	CVerletSystem
{
public:
	void	Update();
	void	Add(CVerletPoint* p_verlet) {m_system.push_back(p_verlet);}
	CVerletPoint*	CreatePoint(Vector2 v);
	vector<CVerletPoint*>&	System() {return m_system;}
private:
	vector<CVerletPoint*>		m_system;
};


// Helper functions
void	RigidConstraint(CVerletPoint* p1, CVerletPoint* p2, float r);
void	SemiRigidConstraint(CVerletPoint* p1, CVerletPoint* p2, float min, float mid, float max, float force);


bool	UnitTestVerlets();




#endif
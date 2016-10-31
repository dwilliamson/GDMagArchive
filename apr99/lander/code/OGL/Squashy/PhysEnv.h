#if !defined(AFX_PhysEnv_H__3DC11AC3_95FB_11D2_9D83_00105A124906__INCLUDED_)
#define AFX_PhysEnv_H__3DC11AC3_95FB_11D2_9D83_00105A124906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhysEnv.h : header file
//
#include "MathDefs.h"

#define EPSILON  0.00001f				// ERROR TERM
#define DEFAULT_DAMPING		0.002f

enum tCollisionTypes
{
	NOT_COLLIDING,
	PENETRATING,
	COLLIDING,
	CONTACTING
};

enum tIntegratorTypes
{
	EULER_INTEGRATOR,
	MIDPOINT_INTEGRATOR,
	RK4_INTEGRATOR
};


// TYPE FOR A PLANE THAT THE SYSTEM WILL COLLIDE WITH
struct tCollisionPlane
{
	tVector normal;			// inward pointing
    float	d;				// ax + by + cz + d = 0
};

// TYPE FOR A PHYSICAL PARTICLE IN THE SYSTEM
struct tParticle
{
	tVector pos;		// Position of Particle
    tVector v;			// Velocity of Particle
	tVector f;			// Force Acting on Particle
	float	oneOverM;	// 1 / Mass of Particle
};

// TYPE FOR CONTACTS THAT ARE FOUND DURING SIM
struct tContact
{
	int		particle;	// Particle Index
    tVector normal;		// Normal of Collision plane
	int		type;		// COLLIDING OR CONTACT
};

// TYPE FOR SPRINGS IN SYSTEM
struct tSpring
{
	int		p1,p2;		// PARTICLE INDEX FOR ENDS
	float	restLen;	// LENGTH OF SPRING AT REST
	float	Ks;			// SPRING CONSTANT
	float	Kd;			// SPRING DAMPING
};

class CPhysEnv
{
// Construction
public:
	CPhysEnv();
	void RenderWorld();
	void SetWorldParticles(tVector *coords,int particleCnt);
	void ResetWorld();
	void Simulate(float DeltaTime,BOOL running);
	void ApplyUserForce(tVector *force);
	void SetMouseForce(int deltaX,int deltaY, tVector *localX, tVector *localY);
	void GetNearestPoint(int x, int y);
	void AddSpring();
	void SetVertexMass();
	void SetWorldProperties();
	void FreeSystem();
	void LoadData(FILE *fp);
	void SaveData(FILE *fp);
	BOOL				m_UseGravity;			// SHOULD GRAVITY BE ADDED IN
	BOOL				m_UseDamping;			// SHOULD DAMPING BE ON
	BOOL				m_UserForceActive;		// WHEN USER FORCE IS APPLIED
	BOOL				m_DrawSprings;			// DRAW THE SPRING LINES
	BOOL				m_DrawVertices;			// DRAW VERTICES
	BOOL				m_MouseForceActive;		// MOUSE DRAG FORCE
	BOOL				m_CollisionRootFinding;	// TRYING TO FIND A COLLISION
	int					m_IntegratorType;

// Attributes
private:
	float				m_WorldSizeX,m_WorldSizeY,m_WorldSizeZ;
	tVector				m_Gravity;				// GRAVITY FORCE VECTOR
	tVector				m_UserForce;			// USER FORCE VECTOR
	float				m_UserForceMag;			// MAGNITUDE OF USER FORCE
	float				m_Kd;					// DAMPING FACTOR
	float				m_Kr;					// COEFFICIENT OF RESTITUTION
	float				m_Ksh;					// HOOK'S SPRING CONSTANT
	float				m_Ksd;					// SPRING DAMPING
	float				m_MouseForceKs;			// MOUSE SPRING COEFFICIENT
	tCollisionPlane		*m_CollisionPlane;		// LIST OF COLLISION PLANES
	int					m_CollisionPlaneCnt;			
	tContact			*m_Contact;				// LIST OF POSSIBLE COLLISIONS
	int					m_ContactCnt;			// COLLISION COUNT
	tParticle			*m_ParticleSys[3];		// LIST OF PHYSICAL PARTICLES
	tParticle			*m_CurrentSys,*m_TargetSys;
	tParticle			*m_TempSys[5];			// SETUP FOR TEMP PARTICLES USED WHILE INTEGRATING
	int					m_ParticleCnt;
	tSpring				*m_Spring;				// VALID SPRINGS IN SYSTEM
	int					m_SpringCnt;		
	int					m_Pick[2];				// INDEX COUNTERS FOR SELECTING
	tVector				m_MouseDragPos[2];		// POSITION OF DRAGGED MOUSE VECTOR
// Operations
private:
	inline void	IntegrateSysOverTime(tParticle *initial,tParticle *source, tParticle *target, float deltaTime);
	void	RK4Integrate( float DeltaTime);
	void	MidPointIntegrate( float DeltaTime);
	void	EulerIntegrate( float DeltaTime);
	void	ComputeForces( tParticle	*system );
	int		CheckForCollisions( tParticle	*system );
	void	ResolveCollisions( tParticle	*system );
	void	CompareBuffer(int size, float *buffer,float x, float y);

// Implementation
public:
	virtual ~CPhysEnv();

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PhysEnv_H__3DC11AC3_95FB_11D2_9D83_00105A124906__INCLUDED_)

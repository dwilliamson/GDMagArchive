/*
	Oxygen3D 
	(c)Copyright 1999-2000 Mystic Game Development
	All rights reserved worldwide.

	Particle classes

        Game Developer Magazine Readers Note:
        -----------------------------------------------------------------
        This code has not yet been optimized.
        Use it as an example of how to build an advanced particle system.
        But don't just copy paste:
        "Innovate, don't imitate" :-)

        If you got any comments/questions or just wanna chat/mail about
        whatever, just mail me at john@mysticgd.com or add me to your
        ICQ list at 40937910
*/

#ifndef __O_PARTICLES_H
#define __O_PARTICLES_H

#include <pigs.h>
L_USE_NAMESPACE

#include "o_node.h"
#include "o_system.h"


// system classes
DECLARE_CLASS(O3D_Particle)
DECLARE_CLASS(O3D_ParticleShape)
DECLARE_CLASS(O3D_ParticleMgr)

// particle systems
DECLARE_CLASS(O3D_ParticleSystem)	// PS_Manual
DECLARE_CLASS(O3D_Sparks)			// PS_Sparks
DECLARE_CLASS(O3D_Smoke)			// PS_Smoke
DECLARE_CLASS(O3D_FlareLight)		// PS_FlareLight
DECLARE_CLASS(O3D_BitmapExplosion)	// PS_BitmapExplosion
DECLARE_CLASS(O3D_Bubbles)			// PS_Bubbles
DECLARE_CLASS(O3D_Snow)				// PS_Snow


// Particle system types
enum ParticleSystemType
{
	PS_Manual		= 0,
	PS_Sparks,
	PS_Smoke,
	PS_FlareLight,
	PS_BitmapExplosion,
	PS_Bubbles,
	PS_Snow,
};


//-------------------------------------------------------------------------------------------------
// a particle
class O3D_Particle
{
	public:
		// constructor and destructor (empty for speed)
		O3D_Particle()	{ }
		~O3D_Particle()	{ }

		// public attributes
		bool			alive;
		Vector3			position;
		Vector3			oldPos;
		Vector3			velocity;
		RGBAColor		color;
		int				energy;
		float			size;
};


//-------------------------------------------------------------------------------------------------
// particle system class
class O3D_ParticleSystem : public O3D_Node
{	
	friend class O3D_Particle;
	friend class O3D_ParticleMgr;

	protected:
		class O3D_Texture		*texture;
		BlendMode				blendMode;
		ParticleSystemType		systemType;


	public:
		// constructor and destructor
		O3D_ParticleSystem(int nr, rcVector3 centerPos, BlendMode blend=Blend_AddAlpha, rcString filename="Effects/Particles/green_particle", ParticleSystemType type=PS_Manual);
		virtual ~O3D_ParticleSystem();

		// public members
		Array<O3D_Particle>			particles;
		Array<O3D_ParticleShape>	shapes;
		int							nrAlive;	// alive particles
		BoundingBox3				boundingBox;


		// inline functions
		f_inline void		SetNumParticles(int nr)				{ particles.Resize(nr); shapes.Resize(nr); }
		f_inline void		SetTexture(class O3D_Texture *tex)	{ texture = tex;	}
		f_inline class O3D_Texture*	GetTexture() const			{ return texture;	}
		f_inline void		AddDefaultParticle()				{ O3D_Particle p; SetParticleDefaults(p); particles.Add(p); }
		f_inline void		SetupShapes()						{ for (int i=0; i<shapes.Length(); i++) if (particles[i].alive) SetupShape(i);	}
		f_inline bool		IsAlive() const						{ return (nrAlive>0); }
		f_inline ParticleSystemType GetSystemType()	const		{ return systemType; }
		f_inline int		GetNumAlive() const					{ return nrAlive; }	

		// main functions
		virtual void		SetParticleDefaults(O3D_Particle& p);
		virtual bool		TransformSystem();
		virtual void		ProcessAI();
		virtual void		FillBuckets();
		virtual void		SetupShape(int nr);
		virtual void		SetDefaults()						{ nrAlive=0; for (int i=0; i<particles.Length(); i++) SetParticleDefaults(particles[i]);  }
};



//-------------------------------------------------------------------------------------------------
// particle shape descriptor
class O3D_ParticleShape
{
	public:
		// constructor and destructor
		O3D_ParticleShape()	{}
		~O3D_ParticleShape(){}

		// attributes
		Vector3		vertex[4];		// positions of 4 corners
		RGBAColor	color;			// colors of the corners
};


//-------------------------------------------------------------------------------------------------
// particle manager (o_particles.cpp)
class O3D_ParticleMgr
{
	friend class O3D_ParticleSystem;
	friend class O3D_Particle;

	private:
		// a particle bucket
		struct Bucket				{ BlendMode blendMode; class O3D_Texture* texture; Array<O3D_ParticleShape*> polys; ParticleSystemType systemType; };

		// private attributes
		Array<O3D_ParticleSystem*>	systems;		// the particle systems	
		String						version;		// manager version
		pVertexArray				verts;			// vertex array
		IndexArray					vertsIndex;		// index array
		Array<Bucket>				buckets;		// normal buckets


	public:
		// constructor and destructor
		O3D_ParticleMgr()	{ Init(); }
		~O3D_ParticleMgr()	{ Exit(); }

		// misc functions 
		rcString	GetVersion()	{ return version; }

		// main functions
		void	Init();								// initialize, called by constructor
		void	Exit();								// release all allocated memory and delete all systems etc, called by destructor
		void	ProcessHierarchy();					// process hierarchy
		void	Transform();						// transform (update) all systems
		void	ProcessAI();						// update stuff
		void	FillBuckets();						// fill render buckets
		void	RenderNormalBuckets();				// render non-transparent buckets (ALL BUCKETS ARE TRANSPARENT!)
		void	RenderTransparentBuckets();			// render transparent buckets
		void	DeleteBuckets();					// delete all buckets (free memory)
		void	RemoveSystem(O3D_ParticleSystem *sys);	// removes a system		

		// inline functions
		f_inline int							GetNumBuckets() const						{ return buckets.Length(); }
		f_inline int							GetNumSystems() const						{ return systems.Length(); }
		f_inline int							GetNumAlive() const							{ int total=0; for (int i=0; i<systems.Length(); i++) total+=systems[i]->GetNumAlive(); return total; }
		f_inline bool							DoesExist(O3D_ParticleSystem *sys) const	{ return (systems.Find(sys)!=-1); }
		f_inline O3D_ParticleSystem*			GetSystem(int nr) const						{ assert(nr<systems.Length()); return systems[nr]; }
		f_inline O3D_ParticleSystem*			AddSystem(O3D_ParticleSystem *newSystem)	{ assert(newSystem); systems.Add(newSystem); return systems.Back(); }
		f_inline Array<O3D_ParticleSystem*>&	GetSystems()								{ return systems; }
};



//-------------------------------------------------------------------------------------------------
// Particle Effect Classes
//-------------------------------------------------------------------------------------------------
// a sparks system
class O3D_Sparks : public O3D_ParticleSystem
{
	public:
		struct InitInfo
		{
			bool		stretch;		// stretching particles?	[default=false]
			class O3D_Light *light;		// pointer to the light		[default=NULL]
			float 		speed;			// start speed multiplier	[default=1]
			float		size;			// start size multiplier	[default=1]
			bool		bDoSmoke;		// add smoke?				[default=false]
			bool		bDoCollision;	// collision detection?		[default=false]
			bool		bUseDirection;	// use direction vector?	[default=false]
			bool		bAlwaysAlive;	// keep system alive?		[default=false]
			Vector3		direction;		// direction vector			[default=Vector3(0,1,0)]
			float		spread;			// direction spread			[default=4.0f]
			float		gravity;		// gravity value			[default=-0.2f]
		};

		// constructor
		O3D_Sparks(int nr, rcVector3 pos, InitInfo *initInfo=NULL, BlendMode blendMode=Blend_AddAlpha, rcString texture="Effects/Particles/fire_particle", ParticleSystemType type=PS_Sparks);
		~O3D_Sparks();

		// inline functions
		f_inline InitInfo& GetInfo()	{ return info;	}

		// main functions
		static void		DefaultInitInfo(InitInfo &initInfo);
		void			SetParticleDefaults(O3D_Particle& p);
		void			SetupShape(int nr);
		bool			TransformSystem();

	private:
		class O3D_Light	*light;
		InitInfo		info;
};



// smoke
class O3D_Smoke : public O3D_ParticleSystem
{
	public:
		struct InitInfo
		{
			float		scaleSpeed;			// scale increase speed
			float		energySpeed;		// energy decrease speed
			float		windDistortFactor;	// distortion factor (velocity = direction + (randomVector*distortion))
			Vector3		windDirection;		// direction of the wind
			bool		bAlwaysAlive;		// keep this smoke system alive? (for constant smoke) [default=false]
		};

		// constructor
		O3D_Smoke(int nr, rcVector3 pos, InitInfo *initInfo=NULL, BlendMode blendMode=Blend_AddAlpha, rcString texture="Effects/Particles/smoke", ParticleSystemType type=PS_Smoke);

		// inline functions
		f_inline InitInfo& GetInfo()	{ return info;		}

		// main functions
		static void	DefaultInitInfo(InitInfo &initInfo);
		void		SetParticleDefaults(O3D_Particle& p);
		bool		TransformSystem();


	private:
		InitInfo		info;
};


// FlareLight class
// Please note that this is not a lensflare. This can be seen as a lensflare without a
// lenseffect. Usable for car lights etc (which will not emit light). Just for the effect.
class O3D_FlareLight : public O3D_ParticleSystem
{
	private:
		int		intensity;		// real intensity, used in rendering
		int		tempIntensity;	// desired intensity (used for smooth fading)
		float	fadeSpeed;		// fade speed facter [default=0.2] [range=0..1]
		bool	bBlocked;		// blocked? (used for fade)


	public:
		struct InitInfo
		{
			float			size;			// size of the light polys (size x size) [default=10.0]
			Vector3			direction;		// the direction where the light is looking at (must be normalized) [default=given from constructor]
			bool			alwaysActive;	// do not energy based on angle etc? [default=false]
			unsigned char	minEnergy;		// minimum energy (should always be >=1) [default=1]
			unsigned char	maxEnergy;		// maximum energy (upperlimit=255) [default=255]
			unsigned char	sensivity;		// sensivity of the angle change [default=255]
			bool			doubleSided;	// double sided light?
		};

		// constructor
		O3D_FlareLight(rcVector3 pos, rcVector3 dir, InitInfo *initInfo=NULL, BlendMode blendMode=Blend_AddAlpha, rcString texture="Effects/flares/red_particle", ParticleSystemType type=PS_FlareLight);

		// inline functions
		f_inline InitInfo& GetInfo()	{ return info;	}

		// main functions
		static void	DefaultInitInfo(InitInfo &initInfo);
		void		SetParticleDefaults(O3D_Particle& p);
		bool		TransformSystem();
		int			GetIntensity(class O3D_Camera *cam); // return the intensity of the flarelight, seen from camera cam

		// inline
		f_inline	void			SetSensivity(unsigned char sens)			{ info.sensivity= sens;		}
		f_inline	void			SetMaxEnergy(unsigned char maxEn)			{ info.maxEnergy= maxEn;	}
		f_inline	void			SetMinEnergy(unsigned char minEn)			{ info.minEnergy= minEn;	}
		f_inline	void			SetSize(float newSize)						{ info.size		= newSize;	}
		f_inline	void 			SetDirection(rcVector3 dir)					{ info.direction= dir;		}
		f_inline	void			SetAlwaysActive(bool alw)					{ info.alwaysActive = alw;	}
		f_inline	void			Die()										{ info.size=0;				} // kill and remove from memory

		f_inline	rcVector3		GetDirection() const						{ return info.direction;	}
		f_inline	bool			IsAlwaysActive() const						{ return info.alwaysActive; }
		f_inline	unsigned char	GetMinEnergy() const						{ return info.minEnergy;	}
		f_inline	unsigned char	GetMaxEnergy() const						{ return info.maxEnergy;	}
		f_inline	unsigned char	GetSensivity() const						{ return info.sensivity;	}
		f_inline	float			GetSize() const								{ return info.size;			}

		f_inline	bool			IsBlocked() const							{ return bBlocked;			}
		f_inline	void			SetBlocked(bool b)							{ bBlocked = b;				}
		f_inline	bool			IsDoubleSided() const						{ return info.doubleSided;	}
		f_inline	void			SetDoubleSided(bool b)						{ info.doubleSided = b;		}

		f_inline	unsigned char	GetRealIntensity() const					{ return intensity;			}
		f_inline	unsigned char	GetDesiredIntensity() const					{ return tempIntensity;		}
		f_inline	float			GetFadeSpeed() const						{ return fadeSpeed;			}

		f_inline	void			SetRealIntensity(unsigned char intens)		{ intensity		= intens;	}
		f_inline	void			SetDesiredIntensity(unsigned char intens)	{ tempIntensity	= intens;}
		f_inline	void			SetFadeSpeed(float factor)					{ fadeSpeed		= factor;	}


	private:
		InitInfo		info;
};


// Bitmap explosion
class O3D_BitmapExplosion : public O3D_ParticleSystem
{
	public:
		struct InitInfo
		{
			float		size;				// size
			float		energySpeed;		// energy decrease speed
			float		animSpeed;			// animation speed (1.0 is normal and default)
		};

		// constructor
		O3D_BitmapExplosion(rcVector3 pos, InitInfo *initInfo=NULL, rcString filePrefix="Effects/Explosions/medium_", BlendMode blendMode=Blend_AddAlpha, ParticleSystemType type=PS_BitmapExplosion);

		// inline functions
		f_inline	InitInfo& GetInfo()		{ return info;	}

		// main functions
		static void	DefaultInitInfo(InitInfo &initInfo);
		void		SetParticleDefaults(O3D_Particle& p);
		bool		TransformSystem();


	private:
		InitInfo	info;
		String		prefix;
		float		frame;
};



// water bubbles
class O3D_Bubbles : public O3D_ParticleSystem
{
	public:
		struct InitInfo
		{
			bool		bAlwaysAlive;		// keep the system alive? [default=true]
			float		maxDist;			// maximum distance from start (particle system position)
			float		windDistortFactor;	// distortion factor (velocity = direction + (randomVector*distortion))
			Vector3		windDirection;		// direction of the wind
		};

		// constructor
		O3D_Bubbles(int nr, rcVector3 pos, InitInfo *initInfo=NULL, BlendMode blendMode=Blend_AddAlpha, rcString texture="Effects/Particles/bubble", ParticleSystemType type=PS_Bubbles);

		// inline functions
		f_inline InitInfo& GetInfo()	{ return info;	}

		// main functions
		static void	DefaultInitInfo(InitInfo &initInfo);
		void		SetParticleDefaults(O3D_Particle& p);
		bool		TransformSystem();


	private:
		InitInfo		info;
};


// the snow class
class O3D_Snow : public O3D_ParticleSystem
{
	public:
		struct InitInfo
		{
			float		radius;				// radius of snowflake falling				[default=250]
			float		top;				// height where the snow starts to fall		[default=230]
			float		bottom;				// height where the snow respawns			[default=0]
			float		distortion;			// snow woggle distortion					[default=0.2]
			float		speed;				// maximum drop speed						[default=0.4]
			Vector3		wind;				// wind direction vector					[default=Vector3(0,0,0)=no wind]
		};

		// constructor
		O3D_Snow(int nr, rcVector3 pos, InitInfo *initInfo=NULL, BlendMode blendMode=Blend_AlphaBlend, rcString texture="Effects/Particles/snow_particle", ParticleSystemType type=PS_Snow);

		// inline functions
		f_inline InitInfo& GetInfo()	{ return info;	}

		// main functions
		static void	DefaultInitInfo(InitInfo &initInfo);
		void		SetParticleDefaults(O3D_Particle& p);
		bool		TransformSystem();


	private:
		InitInfo		info;
};


#endif

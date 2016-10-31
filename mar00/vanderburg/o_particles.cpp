/*
	Oxygen3D 
        (c)Copyright 1999-2000 Mystic Game Development
	All rights reserved worldwide.

	Particle implementation

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

#include "o_system.h"
#include "o_specialFX.h"
#include "o_particles.h"
#include "o_texture.h"
#include "o_world.h"
#include "o_nodeMgr.h"
#include "o_node.h"
#include "o_raytrace.h"
#include "o_portal.h"
#include "o_light.h"
#include "o_camera.h"
#include "o_keyframer.h"

//--[ Particle Systems ]---------------------------------------------------------------------------

// constructor (nr=number of particles, centerPos=center position of the system in worldspace)
O3D_ParticleSystem::O3D_ParticleSystem(int nr, rcVector3 centerPos, BlendMode blend, rcString filename, ParticleSystemType type) : O3D_Node()
{
	SetType(O3D_Node::ParticleSystem);
	SetLocalPos(centerPos);

	String		textureFile;
	texture		= NULL;
	blendMode	= blend;
	systemType  = type;
	nrAlive		= 0;

	//boundingBox.Init();

	assert(nr>=0);
	SetNumParticles(nr);
	SetDefaults();

	gEngine->PushMipStatus();
	gEngine->SetGenMipsOnTheFly(false);

	String fname = filename;
	if (gEngine->GetEngineTexture(fname))
	{
		texture = gTexCache.Get(fname);
	}
	else
	{
		gEngine->TraceConsole( StringF("Cannot load particle texture '%s' [mtx/pcx/tga/bmp]", filename.GetReadPtr()) );
		texture = NULL;
	}

	gEngine->PopMipStatus();
}


// destructor
O3D_ParticleSystem::~O3D_ParticleSystem()
{
	particles.Clear();
	shapes.Clear();
}


// set particle on default values for this system type
void O3D_ParticleSystem::SetParticleDefaults(O3D_Particle& p)
{
	p.position	= GetLocalPos();				// set particle position to system center
	p.alive     = true;							// particle is alive
	p.energy	= rnd.Rand()%200+55;			// full energy
	p.color		= gRGB(255, 255, 255, p.energy);// original colors at full intensity
	p.velocity.Set(0, 0, 0);					// don't move!
	p.size		= (rnd.RandF()*10) + 1;			// set the size
}


// update system
void O3D_ParticleSystem::ProcessAI()
{
	if (DoKeyframing())
	{
		if (nodeKF)
			nodeKF->Update();
	}

	CalcLocalTM();
}

// transform system
bool O3D_ParticleSystem::TransformSystem()
{
	nrAlive = 0;
	boundingBox.Init();
	for (int i=0; i<particles.Length(); i++)
	{
		if (!particles[i].alive) continue;

		if (particles[i].energy<=0 || particles[i].size<=0)
			particles[i].alive=false;
		else
			nrAlive++;
	}

	return (nrAlive>0);
}


// fill the render buckets
void O3D_ParticleSystem::FillBuckets()
{
	// first setup the shapes of all particles
	SetupShapes();

	// create shortcut to the buckets
	Array<O3D_ParticleMgr::Bucket>	&buckets = gFX->GetParticleMgr()->buckets;

	for (int b=0; b<buckets.Length(); b++) // check all buckets, if there is a bucket created for this shape already
	{
		if (buckets[b].blendMode==blendMode && buckets[b].texture==texture && systemType==buckets[b].systemType) // if we found the bucket
		{
			int oldSize = buckets[b].polys.Length();	// store old size
			buckets[b].polys.Resize(oldSize+nrAlive);	// resize bucket array

			int current=0;
			for (int s=0; s<shapes.Length(); s++)				// add all shapes to buckets
			{
				if (particles[s].alive)
				{
					buckets[b].polys[oldSize+current] = &shapes[s];
					current++;
				}
			}

			return;
		}
	}

	// if we are here, it means we haven't found a valid bucket, so we need to create a new one
	O3D_ParticleMgr::Bucket newBucket;
	newBucket.texture   = texture;			// set the texture for this bucket
	newBucket.blendMode	= blendMode;		// set the blendmode for this bucket
	newBucket.systemType= systemType;		// set system type
	buckets.Add(newBucket);					// add the new bucket to the bucket list

	// add the shapes to this bucket
	buckets.Back().polys.Resize(nrAlive);
	
	int current = 0;
	for (int s=0; s<shapes.Length(); s++)				// add all shapes to buckets
	{
		if (particles[s].alive)
		{
			buckets.Back().polys[current] = &shapes[s];
			current++;
		}
	}
}


// setup the shape for particle number <nr>
void O3D_ParticleSystem::SetupShape(int nr)
{
	assert(nr < shapes.Length());	// make sure we don't try to shape anything we don't have

	// calc projection point
	Vector3 proj = gCamera->GetViewMatrix() * particles[nr].position;

	// setup shape corner positions
	shapes[nr].vertex[0] = proj + Vector3(-particles[nr].size,  particles[nr].size, 0);
	shapes[nr].vertex[1] = proj + Vector3( particles[nr].size,  particles[nr].size, 0);
	shapes[nr].vertex[2] = proj + Vector3( particles[nr].size, -particles[nr].size, 0);
	shapes[nr].vertex[3] = proj + Vector3(-particles[nr].size, -particles[nr].size, 0);

	// setup color
	shapes[nr].color = gRGB( gRed(particles[nr].color), gGreen(particles[nr].color), gBlue(particles[nr].color), particles[nr].energy);
}


//--[ Particle Manager ]---------------------------------------------------------------------------

// initialize manager, called by constructor
void O3D_ParticleMgr::Init() 
{
	version		= "0.02";
	verts		= NULL;
	vertsIndex.Clear();
	buckets.Clear();
}


// shutdown all systems (free all allocated memory etc), called by destructor
void O3D_ParticleMgr::Exit()
{
	// remove all systems
	while (systems.Length())
	{
		delete systems.Back();
		systems.PopBack();
	}
	systems.Clear();

	// remove vertex and index arrays
	delete verts; verts=NULL;
	vertsIndex.Clear();

	// empty and delete all buckets (including transparent buckets)
	DeleteBuckets();
}


// removes a system
void O3D_ParticleMgr::RemoveSystem(O3D_ParticleSystem *sys)
{
	int pos = systems.Find(sys);

	if (pos!=-1)
	{
		delete sys; sys=NULL;
		systems.SwapRemove(pos);
	}
}


// process hierarchy
void O3D_ParticleMgr::ProcessHierarchy()
{
	for (int i=0; i<GetNumSystems(); i++)
	{
		if (systems[i]->IsRootNode())
		{
			systems[i]->SetNodeTM( systems[i]->GetLocalTM() );
			systems[i]->ProcessHierarchy();
		}
	}
}


// process AI of all systems (update keyframers etc)
void O3D_ParticleMgr::ProcessAI()
{
	// process all systems
	for (int i=0; i<systems.Length(); i++)
		systems[i]->ProcessAI();
}



// transform all particle systems (update all particle positions etc)
void O3D_ParticleMgr::Transform()
{
	// process all systems
	for (int i=0; i<systems.Length();)
	{
		if (!systems[i]->TransformSystem()) // if system died, delete it
		{
			delete systems[i];
			systems.SwapRemove(i);
		}
		else
			i++;
	}
}


// fill all render buckets
void O3D_ParticleMgr::FillBuckets()
{
	// clear all buckets
	DeleteBuckets();

	// fill all buckets
	for (int i=0; i<systems.Length(); i++)
	{
		if (systems[i]->systemType==PS_FlareLight)
		{
			bool byPortal = false;
			O3D_Ray ray(gCamera->GetPos(), systems[i]->GetPos());

			if (gWorld->viewFrustum.PartiallyContains(systems[i]->boundingBox))
			{
				O3D_FlareLight *fl = (O3D_FlareLight*)systems[i];

				// check antiportals
				for (int a=0; a<gWorld->GetNumPortals(); a++)
				{
					if (gWorld->nodeMgr->GetPortal(a)->CompletelyContains(systems[i]->boundingBox))
					{
						fl->SetBlocked(true);
						fl->SetDesiredIntensity( fl->GetMinEnergy() );
						byPortal=true;
						goto done1;
					}
				}

				if (ray.IsBlockedByVisible())
				{
					fl->SetBlocked(true);
					fl->SetDesiredIntensity( fl->GetMinEnergy() );
				}
				else
					fl->SetBlocked(false);

				
done1: // FIXME : remove the goto, it's ugly
				if ( fl->GetRealIntensity() > fl->GetMinEnergy())					
					if (!byPortal) systems[i]->FillBuckets();
			}
		}
		else
		{
			if (gWorld->viewFrustum.PartiallyContains(systems[i]->boundingBox))
			{
				// check antiportals
				for (int a=0; a<gWorld->GetNumPortals(); a++)
					if (gWorld->nodeMgr->GetPortal(a)->CompletelyContains(systems[i]->boundingBox)) goto done2;

done2:; // FIXME : remove the goto, it's ugly
				systems[i]->FillBuckets();
			}
		}
	}
}


// delete all render buckets (free the memory)
void O3D_ParticleMgr::DeleteBuckets()
{
	for (int i=0; i<buckets.Length(); i++)
		buckets[i].polys.Clear();

	buckets.Clear();
}


// render normal (non-transparent) buckets
void O3D_ParticleMgr::RenderNormalBuckets()
{
	// There are no normal particles, because all particles need to be drawn as last
	// so they are all transparent
}


// render transparent buckets
void O3D_ParticleMgr::RenderTransparentBuckets()
{
	int indexPos, vtxPos;

	if (buckets.Length()==0) return;

	// disable the fog
	if (gWorld->IsFogEnabled()) gWorld->DisableFog();

	// set viewmatrix to NULL
	g->PushMatrix(g->View);
	g->SetMatrix(g->View, NULL);
	g->DepthWrite(false);
	//g->DepthFunc(g->Less);
	//g->AlphaFunc(g->Greater, 0.0);
	//g->PixelBlend(g->PB_Add);
	g->CullMode(g->CullDisable);
	g->Filter(g->TriLinear);

	// process all buckets
	for (int b=0; b<buckets.Length(); b++)
	{
		gWorld->nrVisPolys += buckets[b].polys.Length()*2;	// 2 triangles
		vtxPos = indexPos = 0;
		int bucketVerts   = buckets[b].polys.Length() * 4;
		int bucketIndices = (bucketVerts*3) / 2;

		// if the vertex array is not allocated yet, or if the vertex array is too small
		if (verts==NULL || verts->Length() < bucketVerts)
		{
			int vertsWithBuffer = (buckets[b].polys.Length() + 100) * 4;
			int indicesWithBuffer = (vertsWithBuffer*3) / 2;

			delete verts;
			verts = g->NewVertexArray( vertsWithBuffer ); assert(verts);
			vertsIndex.Resize( indicesWithBuffer ); assert(vertsIndex.Length()>0);
			//trace("vertex array upscaled to %d", bucketVerts);
			//trace("index array upscaled to %d", bucketIndices);
		} 
		else
		{
			if (verts->Length() - bucketVerts > 5000)
			{
				delete verts;
				verts = g->NewVertexArray( bucketVerts ); assert(verts);
				vertsIndex.Resize( bucketIndices ); assert(vertsIndex.Length()>0);
				//trace("vertex array downscaled to %d", bucketVerts);
				//trace("index array downscaled to %d", bucketIndices);
			}
		}

		verts->Lock(VtxWrite);

		for (int s=0; s<buckets[b].polys.Length(); s++)	// for all shapes in this bucket
		{
			verts->Set(vtxPos  , buckets[b].polys[s]->vertex[0], Vector2(0,0), buckets[b].polys[s]->color);
			verts->Set(vtxPos+1, buckets[b].polys[s]->vertex[1], Vector2(1,0), buckets[b].polys[s]->color);
			verts->Set(vtxPos+2, buckets[b].polys[s]->vertex[2], Vector2(1,1), buckets[b].polys[s]->color);
			verts->Set(vtxPos+3, buckets[b].polys[s]->vertex[3], Vector2(0,1), buckets[b].polys[s]->color);

			vertsIndex[indexPos  ] = vtxPos;
			vertsIndex[indexPos+1] = vtxPos+1;
			vertsIndex[indexPos+2] = vtxPos+3;
			vertsIndex[indexPos+3] = vtxPos+1;
			vertsIndex[indexPos+4] = vtxPos+2;
			vertsIndex[indexPos+5] = vtxPos+3;

			vtxPos+=4;
			indexPos+=6;
		}

		verts->Unlock();


		// setup render state
		if (buckets[b].systemType==PS_FlareLight)
			g->DepthFunc(g->Always);
		else
			g->DepthFunc(g->Less);

		if (buckets[b].texture)
			g->SelectTexture(buckets[b].texture->GetTexHandle());
		else
			g->SelectTexture(NULL);

		gEngine->SetBlendMode(buckets[b].blendMode);
		
		// transform and render
		g->IndexedPrimitive(g->Triangles, verts, vertsIndex.GetPtr(), indexPos);
	}

	// restore viewmatrix
	g->PopMatrix(g->View);

	// restore fog
	if (gWorld->IsFogEnabled()) gWorld->EnableFog();
}



//-------------------------------------------------------------------------------------------------
// Particle Effect Classes
//-------------------------------------------------------------------------------------------------


// spark system
O3D_Sparks::O3D_Sparks(int nr, rcVector3 pos, InitInfo *initInfo, BlendMode blendMode, rcString texture, ParticleSystemType type) : O3D_ParticleSystem(nr, pos, blendMode, texture, type)
{
	light = NULL;

	// set system info
	if (initInfo)
		info = *initInfo;
	else
		DefaultInitInfo(info);

	// if we need to create a light
	if (info.light)
		gWorld->GetNodeMgr()->AddNode( info.light );


	// add smoke
	if (info.bDoSmoke)
	{
		O3D_Smoke::InitInfo info;
		O3D_Smoke::DefaultInitInfo(info);	
		info.windDirection = rnd.RandVector3()*0.2f;
		info.windDirection.y = rnd.RandF()*0.3f;
		info.scaleSpeed*=0.5f;
		gParticleMgr->AddSystem( new O3D_Smoke(2, pos, &info) );
	}

	// set particles defaults
	SetDefaults();
}


// destructor
O3D_Sparks::~O3D_Sparks()
{
	if (light)
		gWorld->GetNodeMgr()->RemoveNode(light);
}


// default the init info
void O3D_Sparks::DefaultInitInfo(InitInfo &initInfo)
{
	initInfo.stretch		= false;
	initInfo.speed			= 1;
	initInfo.light			= NULL;
	initInfo.bDoSmoke		= false;
	initInfo.bDoCollision	= false;
	initInfo.size			= 1;
	initInfo.bUseDirection  = false;
	initInfo.bAlwaysAlive   = false;
	initInfo.direction		= Vector3(0,1,0);
	initInfo.spread         = 4;
	initInfo.gravity        = -0.2;
}


// set particle on default values for this system type
void O3D_Sparks::SetParticleDefaults(O3D_Particle& p)
{
	p.alive     = true;
	p.position	= position;
	p.oldPos    = position;
	p.energy	= rnd.Rand(8);
	p.color		= gRGB(255, 255, 255, p.energy);
	p.velocity	= rnd.RandVector3() * info.speed;
	p.size		= (rnd.RandF() + 0.2f)*info.size;;

	if (info.bUseDirection)
	{
		Matrix4x4 tm;
		tm.Identity();
		tm.SetRotationForward( info.direction, g->GetCoordSys() );
		
		p.velocity = tm * Vector3(0,0,1) * info.speed;
		p.velocity += rnd.RandVector3() * info.spread;
	}
}



// setup the shape for particle number <nr>
void O3D_Sparks::SetupShape(int nr)
{
	assert(nr < shapes.Length());	// make sure we don't try to shape anything we don't have

	if (!info.stretch)
	{
		// calc projection point
		Vector3 proj = gCamera->GetViewMatrix() * particles[nr].position;

		// setup shape corner positions
		shapes[nr].vertex[0] = proj + Vector3(-particles[nr].size,  particles[nr].size, 0);
		shapes[nr].vertex[1] = proj + Vector3( particles[nr].size,  particles[nr].size, 0);
		shapes[nr].vertex[2] = proj + Vector3( particles[nr].size, -particles[nr].size, 0);
		shapes[nr].vertex[3] = proj + Vector3(-particles[nr].size, -particles[nr].size, 0);
	}
	else
	{
		// Push view matrix and set identity, only translate origin of sprite, use offsets for rest of them
		Vector3 begProj = gCamera->GetViewMatrix()*particles[nr].oldPos;
		Vector3 endProj = gCamera->GetViewMatrix()*particles[nr].position;

		float	dx		= endProj.x - begProj.x;
		float	dy		= endProj.y - begProj.y;
		float	oolen	= ffFOneOverSqrt(dx*dx+dy*dy)*0.5;
		Vector3 persp	= Vector3(-dy*oolen, +dx*oolen, 0);

		if (abs(dx)>1) begProj.x-=dx;
		if (abs(dy)>1) begProj.y-=dy;

		// setup shape corner positions
		shapes[nr].vertex[0] = Vector3(endProj.x-persp.x, endProj.y-persp.y, endProj.z);
		shapes[nr].vertex[1] = Vector3(endProj.x+persp.x, endProj.y+persp.y, endProj.z);
		shapes[nr].vertex[2] = Vector3(begProj.x+persp.x, begProj.y+persp.y, begProj.z);
		shapes[nr].vertex[3] = Vector3(begProj.x-persp.x, begProj.y-persp.y, begProj.z);
	}

	// setup color
	shapes[nr].color = gRGB( gRed(particles[nr].color), gGreen(particles[nr].color), gBlue(particles[nr].color), particles[nr].energy);
}


// update the sparks
bool O3D_Sparks::TransformSystem()
{
	float rate;
	
	if (!gEngine->InMovieRecordMode())
		rate = gEngine->GetFrameTimeRate();
	else
		rate = 1.0f / gEngine->GetDesiredRate();
	
	nrAlive    = 0;

	boundingBox.Init();
	for (int i=0; i<particles.Length(); i++)
	{
		O3D_Particle& part = particles[i];

		if (!part.alive) continue;

		part.oldPos		 = part.position;			// store old position
		part.position	+= part.velocity * rate;	// move particle
		part.velocity	*= 0.95f;					// damping
		part.velocity.y += info.gravity * rate;		// gravity
		part.energy		-= 4 * rate;				// decrease energy


		// do collision
		if (info.bDoCollision)
		{
			O3D_Ray ray(part.oldPos, part.position);

			Vector3 reflect, intersect, normal;	
			for (int i=0; i<gWorld->GetNumStaticObjects(); i++)
			{
				if (ray.IntersectsNode( (class O3D_GeomNode*)gWorld->GetStaticObject(i), NULL, &intersect, &normal, &reflect))
				{
					part.position = intersect - normal*0.1;
					part.velocity = -reflect/*.FNormalized()*/ * 0.7f;
					continue;
				}
			}
		}


		boundingBox.Encapsulate(part.position);

		if (!info.stretch)
			part.size   -= 0.08f * rate;			// decrease size, if not stretching particles


		if (!info.bAlwaysAlive)
		{
			if (part.energy<=0 || part.size<=0) 
				part.alive=false;
			else
				nrAlive++;
		}
		else
		{
			if (part.energy<=0 || part.size<=0) 
				SetParticleDefaults(part);
		}
	}

	if (info.light)
	{
		if (!info.bAlwaysAlive)
			info.light->SetRadius( info.light->GetRadius() - 10.0f*rate );

		info.light->SetLocalPos(position);

		if (info.light->GetRadius()<=0)
		{
			gWorld->GetNodeMgr()->RemoveNode(info.light);
			info.light=NULL;
		}
	}

	if (info.bAlwaysAlive)
		nrAlive = particles.Length();

	boundingBox.Widen(5);

	return (nrAlive>0);
}


//--[ Smoke ]--------------------------------------------------------------------------------------

// smoke
O3D_Smoke::O3D_Smoke(int nr, rcVector3 pos, InitInfo *initInfo, BlendMode blendMode, rcString texture, ParticleSystemType type) : O3D_ParticleSystem(nr, pos, blendMode, texture, type)
{
	// set system info
	if (initInfo)
		info = *initInfo;
	else
		DefaultInitInfo(info);

	SetDefaults();
}


// default the init info
void O3D_Smoke::DefaultInitInfo(InitInfo &initInfo)
{
	initInfo.energySpeed  = 2;				// energy decrease speed
	initInfo.scaleSpeed   = 0.3;			// scale speed
	initInfo.windDirection.Set(0, 0.5, 0);	// wind direction
	initInfo.windDistortFactor = 0.1;		// wind distortion (velocity = winddirection + randomVector*windDistortFactor)
	initInfo.bAlwaysAlive = false;			// keep smoke alive? (for constant smoke)
}


// set particle on default values for this system type
void O3D_Smoke::SetParticleDefaults(O3D_Particle& p)
{
	p.alive     = true;
	p.position	= position;
	p.energy	= rnd.Rand(8);
	p.color		= gRGB(255, 255, 255, p.energy);
	p.size		= (rnd.RandF()*2) + 1;
	p.velocity  = info.windDirection + (rnd.RandVector3()*info.windDistortFactor);
}


// update the smoke
bool O3D_Smoke::TransformSystem()
{
	float rate;
	if (!gEngine->InMovieRecordMode())
		rate = gEngine->GetFrameTimeRate();
	else
		rate = (1.0f / gEngine->GetDesiredRate()) * 1.5;

	nrAlive=0;

	boundingBox.Init();
	for (int i=0; i<particles.Length(); i++)
	{
		O3D_Particle& part = particles[i];
		if (!part.alive) continue;

		part.position	+= particles[i].velocity * rate;	// update position
		part.size		+= info.scaleSpeed * rate;			// update size
		part.energy		-= info.energySpeed * rate;		// update energy

		boundingBox.Encapsulate(part.position);

		if (!info.bAlwaysAlive)
		{
			if (part.energy<=0 || part.size<=0) 
				part.alive=false;
			else
				nrAlive++;
		}
		else
		{
			if (part.energy<=0 || part.size<=0) 
				SetParticleDefaults(part);
		}
	}

	if (info.bAlwaysAlive)
		nrAlive = particles.Length();

	boundingBox.Widen(10);

	return (nrAlive>0);
}







//--[ FlareLight ]----------------------------------------------------------------------------------

// constructor
O3D_FlareLight::O3D_FlareLight(rcVector3 pos, rcVector3 dir, InitInfo *initInfo, BlendMode blendMode, rcString texture, ParticleSystemType type) : O3D_ParticleSystem(1, pos, blendMode, texture, type)
{
	// set system info
	if (initInfo)
		info = *initInfo;
	else
		DefaultInitInfo(info);

	info.direction = dir;
	SetDefaults();

	SetDesiredIntensity(255);
	SetRealIntensity(255);
	SetFadeSpeed(0.2);
	SetBlocked(false);
}


// default the init info
void O3D_FlareLight::DefaultInitInfo(InitInfo &initInfo)
{
	initInfo.size			= 10.0;		// size of the light (size x size)
	initInfo.alwaysActive	= false;	// is the flarelight always active?
	initInfo.maxEnergy		= 255;		// the maximum energy
	initInfo.minEnergy		= 1;		// the minimum energy (>=1)
	initInfo.sensivity		= 255;		// sensivity when angle is changed
	initInfo.doubleSided	= true;		// double sided light?
}


// return the intensity of the flarelight, seen from camera cam
int O3D_FlareLight::GetIntensity(class O3D_Camera* cam)
{
	int distFade = (position - cam->GetPos()).Length()*0.5;

	if (!info.alwaysActive)
	{
		Vector3 vec	= (cam->GetTarget() - cam->GetPos()).FNormalized();
		return clamp((int)abs(info.sensivity*info.direction.Dot(vec))-distFade, (int)info.minEnergy, (int)info.maxEnergy);
	}

	return clamp((int)(info.maxEnergy-distFade), (int)info.minEnergy, (int)info.maxEnergy);
}


// set particle on default values for this system type
void O3D_FlareLight::SetParticleDefaults(O3D_Particle& p)
{
	p.alive     = true;
	p.position	= GetLocalPos();
	p.energy	= GetIntensity(gCamera);
	p.color		= gRGB(255, 255, 255, p.energy);
	p.size		= info.size;
	p.velocity  = Vector3(0,0,0);
}


// update the flarelight
bool O3D_FlareLight::TransformSystem()
{
	nrAlive=0;

	boundingBox.Init();

	O3D_Particle& part = particles[0];
	
	if (part.alive)
	{
		if (!bBlocked) SetDesiredIntensity(GetIntensity(gCamera));

		int diff = tempIntensity - intensity; // desired - current (negative if we will fade out, positive if we will fade in)
		if (diff>0) SetRealIntensity( clamp((int)(intensity+(diff*fadeSpeed)), (int)info.minEnergy, (int)tempIntensity) ); // if fading in
			else SetRealIntensity( clamp((int)(intensity+(diff*fadeSpeed*2)), (int)tempIntensity, (int)info.maxEnergy) ); // fading out

		part.position	= position;
		part.energy		= intensity;
		part.size		= info.size;
		boundingBox.Encapsulate(part.position);

		if (part.energy<=0 || part.size<=0) 
			part.alive=false;
		else
			nrAlive++;
	}

	boundingBox.Widen(info.size);
	return (nrAlive>0);
}





//--[ BitmapExplosion ]----------------------------------------------------------------------------

// bitmap explosion
O3D_BitmapExplosion::O3D_BitmapExplosion(rcVector3 pos, InitInfo *initInfo, rcString filePrefix, BlendMode blendMode, ParticleSystemType type) : O3D_ParticleSystem(1, pos, blendMode, filePrefix+"00", type)
{
	// set system info
	if (initInfo)
		info = *initInfo;
	else
		DefaultInitInfo(info);

	SetDefaults();

	prefix = filePrefix;
	boundingBox.Init();
	boundingBox.Encapsulate(pos);
	boundingBox.Widen(info.size);
	frame = 0;
}


// default the init info
void O3D_BitmapExplosion::DefaultInitInfo(InitInfo &initInfo)
{
	initInfo.size			= 15;	// size of the explosion
	initInfo.energySpeed	= 1;	// energy decrease speed
	initInfo.animSpeed		= 1.0;	// animation speed (1.0 is normal speed)
}


// set particle on default values for this system type
void O3D_BitmapExplosion::SetParticleDefaults(O3D_Particle& p)
{
	p.alive     = true;
	p.position	= position;
	p.energy	= 255;
	p.color		= gRGB(255, 255, 255, p.energy);
	p.size		= info.size;
}


// update the explosion
bool O3D_BitmapExplosion::TransformSystem()
{
	nrAlive = 0;

	O3D_Particle& part = particles[0];

	frame += gEngine->GetFrameTimeRate();

	String fname;
	fname.Format("%s%.2d", prefix.GetReadPtr(), (int)frame);

	if (gEngine->GetEngineTexture(fname))
	{
		gEngine->PushMipStatus();
		gEngine->SetGenMipsOnTheFly(false);
		texture = gTexCache.Get(fname);
		gEngine->PopMipStatus();
	}
	else
	{
		part.alive=false;
		texture = NULL;
		gEngine->PopMipStatus();
		return false;
	}


	part.energy	-= info.energySpeed;		// update energy
	if (part.energy<=0 || part.size<=0) 
		part.alive=false;
	else
		nrAlive++;

	return (part.energy>0 && part.size>0);
}



//--[ Bubbles ]--------------------------------------------------------------------------------------

// water bubbles
O3D_Bubbles::O3D_Bubbles(int nr, rcVector3 pos, InitInfo *initInfo, BlendMode blendMode, rcString texture, ParticleSystemType type) : O3D_ParticleSystem(nr, pos, blendMode, texture, type)
{
	// set system info
	if (initInfo)
		info = *initInfo;
	else
		DefaultInitInfo(info);

	SetDefaults();
}


// default the init info
void O3D_Bubbles::DefaultInitInfo(InitInfo &initInfo)
{
	initInfo.bAlwaysAlive = true;
	initInfo.maxDist = 30.0;				// maximum distance from start point
	initInfo.windDirection.Set(0, rnd.RandF()+0.25, 0);	// wind direction
	initInfo.windDistortFactor = 0.1;		// wind distortion (velocity = winddirection + randomVector*windDistortFactor)
}


// set particle on default values for this system type
void O3D_Bubbles::SetParticleDefaults(O3D_Particle& p)
{
	p.alive     = true;
	p.position	= GetLocalPos();
	p.energy	= rnd.Rand(8);
	p.color		= gRGB(255, 255, 255, p.energy);
//	p.size		= (rnd.RandF()*0.5)+0.1;
	p.size		= (rnd.RandF()*0.5)+0.3;
	p.velocity  = info.windDirection + (rnd.RandVector3()*info.windDistortFactor);
	//p.velocity.y= rnd.RandF()+0.25;

	p.position.y+= rnd.RandF()*info.maxDist/2;
}


// update the bubbles
bool O3D_Bubbles::TransformSystem()
{
	float rate;
	if (!gEngine->InMovieRecordMode())
		rate = gEngine->GetFrameTimeRate();
	else
		rate = (1.0f / gEngine->GetDesiredRate()) * 1.5;

	nrAlive=0;

	boundingBox.Init();
	for (int i=0; i<particles.Length(); i++)
	{
		O3D_Particle& part = particles[i];
		if (!part.alive) continue;

		part.velocity  = (info.windDirection + (rnd.RandVector3()*info.windDistortFactor)) * rate;

		part.position	+= particles[i].velocity;	// update position
		if ((position-part.position).Length() > info.maxDist)
			part.energy-= 2.0f * rate;

		boundingBox.Encapsulate(part.position);

		if (!info.bAlwaysAlive)
		{
			if (part.energy<=0 || part.size<=0) 
				part.alive=false;
			else
				nrAlive++;
		}
		else
		{
			if (part.energy<=0 || part.size<=0) 
				SetParticleDefaults(part);
		}
	}

	if (info.bAlwaysAlive)
		nrAlive = particles.Length();

	boundingBox.Widen(10);

	return (nrAlive>0);
}




//--[ Snow ]--------------------------------------------------------------------------------------

// Snow
O3D_Snow::O3D_Snow(int nr, rcVector3 pos, InitInfo *initInfo, BlendMode blendMode, rcString texture, ParticleSystemType type) : O3D_ParticleSystem(nr, pos, blendMode, texture, type)
{
	// set system info
	if (initInfo)
		info = *initInfo;
	else
		DefaultInitInfo(info);

	SetDefaults();
}


// default the init info
void O3D_Snow::DefaultInitInfo(InitInfo &initInfo)
{
	initInfo.radius		= 250.0f;
	initInfo.top		= 230.0f;
	initInfo.bottom		= 0.0f;
	initInfo.distortion	= 0.2f;
	initInfo.speed		= 0.4f;
//	initInfo.wind		= Vector3(0,0,0);
	initInfo.wind		= Vector3(0.1,0,0);
}


// set particle on default values for this system type
void O3D_Snow::SetParticleDefaults(O3D_Particle& p)
{
	Vector3 r	= rnd.RandVector3() * info.radius;
	r.y			= - rnd.RandF()*(info.top - info.bottom);
	p.alive		= true;

	p.position	= position + r;
	p.energy	= 255;
	p.color		= gRGB(255, 255, 255, p.energy);
	p.size		= (rnd.RandF()*0.7)+0.3;
	p.velocity  = Vector3(0, -((rnd.RandF()*info.speed) + 0.1), 0);
}


// update the snowflakes
bool O3D_Snow::TransformSystem()
{
	float rate;
	if (!gEngine->InMovieRecordMode())
		rate = gEngine->GetFrameTimeRate();
	else
		rate = (1.0f / gEngine->GetDesiredRate());

	boundingBox.Init();
	boundingBox.Encapsulate(position);

	Vector3 distort, r;
	for (int i=0; i<particles.Length(); i++)
	{
		O3D_Particle& part		= particles[i];

		distort				=  rnd.RandUnitVector3() * info.distortion; distort.y=0;
		part.position		+= ((particles[i].velocity*rate) + distort) + (info.wind*rate);

		// respawn
		if (part.position.y < 0)
		{
			r				= rnd.RandVector3() * info.radius;	r.y = 0;
			part.position	= position + Vector3(r.x, r.y, r.z);
		}
	}

	boundingBox.Widen( info.radius + 10);

	nrAlive = particles.Length();
	return true;
}


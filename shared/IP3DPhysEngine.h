//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics core engine class interfaces
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"

#include "IP3DPhysBody.h"


//-------------------------------------------------- COLLISION GROUPS --------------------------------------------------
// Pokud dojde ke zmene tohoto enumu je treba projit kod a hledat PhysCollisionGroups a puvodni enumy a znova nastavit.
// Urcite do toho patri CollisionGroups.cpp v game projektu
// Pri zmene skupin taky treba rebuild Phys DLL Modulu!!! Takze se zmenou tohoto enumu zmenit verzi ifacu IP3DPhysEngine
enum PhysCollisionType
{
	// prvni skupina musi byt 1
	// --------------------------------------------------
	// -- pohyblive - nastavi se v CollisionGroups.cpp --
	PHYSGROUP_PLAYER=1,
	PHYSGROUP_ENEMY,
	PHYSGROUP_VEHICLE,
	PHYSGROUP_PARTICLE,
	// -- nepohyblive -----------------------------------
	PHYSGROUP_WATER,
	PHYSGROUP_WINDOW,
	PHYSGROUP_FOLIAGE,
	// ... mozno pridavat
	// ---------------------------------------------------
	PHYSGROUP_NON_COLLIDABLE, // DON'T CHANGE, COLLIDES WITH NOTHING
	PHYSGROUP_COLLIDABLE // DON'T CHANGE, COLLIDES WITH EVERYTHING
};

struct PhysCollision
{
	/** Source collision group */
	WORD group;
	/** Collision group group is collidable with these groups (true or false). Array must be set for indexes from PhysCollisionType without last two. */
	bool bCollide[PHYSGROUP_NON_COLLIDABLE];
	/** Set all to non-collidable */
	void Reset(){ for(int i=1; i<PHYSGROUP_NON_COLLIDABLE; i++) bCollide[i]=false; };
};


//---------------------------------
class IP3DPhysEngine : public IP3DBaseInterface
{
public:
	virtual bool Initialize() = 0;
	/** This starts physical simulation. After this call, simulation will be done on second thread, so you can do something useful on first app thread. After you will be ready, call SimulationEnd. */
	virtual void SimulateBegin(float deltaTime) = 0;
	/** Ends simulation and wait for all calculation on rigid bodies will be finished. */
	virtual void SimulateEnd() = 0;
	virtual void RenderDebug()=0;
	/** Is using Hardware Physics Processor Unit? */
	virtual bool IsHW()const=0;
	/** Get nearest body which collided with this ray. \param collParam Percentage of end-start line. This is intersect point in this line. For example collParam=0.5 means that intersect point is start+(end-start)*0.5 (exactly between start and end point) */
	virtual IP3DPhysBody* GetNearestWorldRayCastHit(P3DXVector3D start, P3DXVector3D end, float &collParam)=0;
	/** Determines if these two collision group are collidable. Use only if you know what are you doing. Instead of this use SetCollision(). \see SetCollision */
	virtual void SetGroupCollision(WORD collGroup1, WORD collGroup2, bool bCollide)=0;
	virtual void SetCollision(PhysCollision collision)=0;
	/** Should be called before creating new physics scene, but after destroying old one. This method frees some unreferenced convex meshes and similar resources... */
	virtual void BeginNewScene()=0;
};
#define IP3DPHYS_PHYSENGINE "P3DPhysEngine_1" // nazev ifacu

//---------------------------------
class IP3DPhysCharacter : public IP3DBaseInterface
{
public:
	virtual bool CreateCharacter(float fHeight=175.0f, float fRadius=30.0f, float fEyeOffset=10.0f)=0;
	/** Retrieve filtered character position */
	virtual void GetPosition(P3DXVector3D &pos)const=0;
	/** Can character collide with environment? */
	virtual void SetCollision(bool bEnable)=0;
	/** Change character height - usable for crouching. Returns false when cannot be done (blocked by an environment) */
	virtual bool SetHeight(float newHeight)=0;
	virtual void Move(P3DXVector3D &moveVec)=0;
	virtual void SetPosition(P3DXVector3D &pos)=0;
};
#define IP3DPHYS_CHARACTER "P3DPhysCharacter_1" // nazev ifacu
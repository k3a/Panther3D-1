//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (4.8.2007)
// Purpose:	Collision groups interaction
//-----------------------------------------------------------------------------
#include "CollisionGroups.h"
#include "common.h"

void SetCollisonGroups()
{
	PhysCollision coll;

	// PHYSGROUP_PLAYER
	coll.Reset();
	coll.group = PHYSGROUP_PLAYER;
	coll.bCollide[PHYSGROUP_PLAYER]=true;
	coll.bCollide[PHYSGROUP_ENEMY]=true;
	coll.bCollide[PHYSGROUP_VEHICLE]=true;
	coll.bCollide[PHYSGROUP_WATER]=false;
	coll.bCollide[PHYSGROUP_WINDOW]=true;
	coll.bCollide[PHYSGROUP_FOLIAGE]=false;
	coll.bCollide[PHYSGROUP_PARTICLE]=false; // aspon zatim ne
	g_pPhysEngine->SetCollision(coll);

	// PHYSGROUP_ENEMY
	coll.Reset();
	coll.group = PHYSGROUP_ENEMY;
	coll.bCollide[PHYSGROUP_PLAYER]=true;
	coll.bCollide[PHYSGROUP_ENEMY]=true;
	coll.bCollide[PHYSGROUP_VEHICLE]=true;
	coll.bCollide[PHYSGROUP_WATER]=false;
	coll.bCollide[PHYSGROUP_WINDOW]=true;
	coll.bCollide[PHYSGROUP_FOLIAGE]=false;
	coll.bCollide[PHYSGROUP_PARTICLE]=true; // na zkousku ;)
	g_pPhysEngine->SetCollision(coll);

	// PHYSGROUP_VEHICLE
	coll.Reset();
	coll.group = PHYSGROUP_VEHICLE;
	coll.bCollide[PHYSGROUP_PLAYER]=true;
	coll.bCollide[PHYSGROUP_ENEMY]=true;
	coll.bCollide[PHYSGROUP_VEHICLE]=true;
	coll.bCollide[PHYSGROUP_WATER]=false;
	coll.bCollide[PHYSGROUP_WINDOW]=true;
	coll.bCollide[PHYSGROUP_FOLIAGE]=false;
	coll.bCollide[PHYSGROUP_PARTICLE]=true;
	g_pPhysEngine->SetCollision(coll);

	// PHYSGROUP_PARTICLE
	coll.Reset();
	coll.group = PHYSGROUP_PARTICLE;
	coll.bCollide[PHYSGROUP_PLAYER]=false; // aspon zatim ne
	coll.bCollide[PHYSGROUP_ENEMY]=true; // na zkousku ;)
	coll.bCollide[PHYSGROUP_VEHICLE]=true;
	coll.bCollide[PHYSGROUP_WATER]=false;
	coll.bCollide[PHYSGROUP_WINDOW]=true;
	coll.bCollide[PHYSGROUP_FOLIAGE]=false;
	coll.bCollide[PHYSGROUP_PARTICLE]=false;
	g_pPhysEngine->SetCollision(coll);
}
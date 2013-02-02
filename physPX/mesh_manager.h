//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (15.9.2007)
// Purpose:	Cooking and mesh management class
//-----------------------------------------------------------------------------
#pragma once

#define NOMINMAX
#define MEMORY_DEBUG 1

#include "NxPhysics.h"
#include "NxCooking.h"
#include <vector>

using namespace std;

struct CYLINDER
{
	float height;
	float radius;
	NxConvexMesh* pMesh;
};
struct CMESH
{
	char path[64];
	NxConvexMesh* pMesh;
};

class CP3DPMeshManager
{
public:
	void Initialize();
	void Shutdown();
	NxConvexMesh* LoadPhysMesh(const char* path);
	NxConvexMesh* LoadCylinder(float height, float radius);
private:
	vector<CMESH> m_arrMesh;
	vector<CYLINDER> m_arrCylinder;
};

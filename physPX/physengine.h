//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics core engine
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "IP3DPhysEngine.h"

class MyStream : public NxUserOutputStream
{
public:
	void  reportError (NxErrorCode code, const char *message, const char *file, int line){CON(MSG_CON_ERR, "Phys: %s", message);};
	NxAssertResponse  reportAssertViolation (const char *message, const char *file, int line){CON(MSG_CON_ERR, "Phys: Assert Violation %s", message); return NX_AR_CONTINUE;};
	void  print (const char *message){CON(MSG_CON_INFO, "Phys: %s", message);};
};

class CP3DPhysEngine : public IP3DPhysEngine
{
public:
	~CP3DPhysEngine();
	bool Init();
	bool Initialize();
	void SimulateBegin(float deltaTime);
	void SimulateEnd();
	void RenderDebug();
	bool IsHW()const{ return bHW; };
	IP3DPhysBody* GetNearestWorldRayCastHit(P3DXVector3D start, P3DXVector3D end, float &collParam);
	void SetGroupCollision(WORD collGroup1, WORD collGroup2, bool bCollide){ m_pScene->setGroupCollisionFlag(collGroup1, collGroup2, bCollide); };
	void SetCollision(PhysCollision collision);
	void BeginNewScene();
private:
	NxPhysicsSDK* m_pSDK;
	NxScene* m_pScene;
	//
	bool bHW;
};
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics body class
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "IP3DPhysBody.h"

#define CH() if(!bInitialized) return
#define CHN() if(!bInitialized) return NULL;

class CP3DPhysBody : public IP3DPhysBody
{
public:
	CP3DPhysBody();
	~CP3DPhysBody();
	void AddShapeBox(const PhysShapeDesc &desc, float dimX, float dimY, float dimZ);
	void AddShapeSphere(const PhysShapeDesc &desc, float radius);
	void AddShapePlane(const PhysShapeDesc &desc, const P3DXVector3D &normal, float distFromOrigin);
	void AddShapeCapsule(const PhysShapeDesc &desc, float height, float radius);
	void AddShapeCylinder(const PhysShapeDesc &desc, float height, float radius);
	void AddShapeTrimesh(const PhysShapeDesc &desc, const PhysTrimeshShapeDesc &tridesc, bool bSmoothCollision=false);
	void AddShapeConvexmesh(const PhysShapeDesc &desc, const char* sPhysFile);
	void RemoveAllShapes();
	//
	bool CreateBody(bool bDynamic);
	void SetUserData(void *data){ m_pUserData=data; };
	void* GetUserData()const{ return m_pUserData; };
	void SetKinematic(bool bKinematic){ CH(); bKinematic ? m_pActor->raiseBodyFlag(NX_BF_KINEMATIC) : m_pActor->clearBodyFlag(NX_BF_KINEMATIC); };
	void SetVisualize(bool bVisualize){ CH(); bVisualize ? m_pActor->raiseBodyFlag(NX_BF_VISUALIZATION) : m_pActor->clearBodyFlag(NX_BF_VISUALIZATION); };
	void SetCollision(bool bCollision){ CH(); m_pActor->wakeUp(); bCollision ? m_pActor->clearActorFlag(NX_AF_DISABLE_COLLISION) : m_pActor->raiseActorFlag(NX_AF_DISABLE_COLLISION); };
	//
	void SetVelocity(P3DXVector3D vel){CH(); m_pActor->setLinearVelocity(NxVec3(vel.x, vel.y, vel.z)); };
	void SetMass(float mass){ CH(); /*m_pActor->setMass(mass);*/ m_pActor->updateMassFromShapes(0, mass); };
	void SetCentreOfMass(P3DXVector3D vec){ CH(); m_pActor->setCMassOffsetLocalPosition(NxVec3(vec.x, vec.y, vec.z));};
	void SetMatrix(P3DXMatrix &mat){ CH(); m_pActor->setGlobalPose(Mat2Nx(mat)); };
	void MoveMatrix(P3DXMatrix &mat){ CH(); m_pActor->moveGlobalPose(Mat2Nx(mat)); };
private:
	NxActorDesc m_desc;
	NxActor *m_pActor;
	void* m_pUserData;
	//
	bool bShapes; // ma shapes
	bool bInitialized; // je vytvoren?
};


//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (21.7.2007)
// Purpose:	Physics character controller
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "IP3DPhysEngine.h"
#include "NxCapsuleController.h"

// -----------------------------------------------------------------------------

class CP3DPhysCharacter : public IP3DPhysCharacter
{
public:
	CP3DPhysCharacter();
	~CP3DPhysCharacter();
	bool CreateCharacter(float fHeight, float fRadius, float fEyeOffset);
	void GetPosition(P3DXVector3D &pos)const;
	void SetCollision(bool bEnable){ if(bLoaded) m_pChar->setCollision(bEnable); };
	bool SetHeight(float newHeight){ if(bLoaded) return m_pChar->setHeight(newHeight); else return false; };
	void Move(P3DXVector3D &moveVec);
	void SetPosition(P3DXVector3D &pos){ 
		if (bLoaded) m_pChar->setPosition(NxExtendedVec3(pos.x, pos.y-m_fHeight/2.0f+m_fEyeOffset, pos.z)); 
	};
private:
	bool bLoaded;
	NxCapsuleController *m_pChar;
	//
	float m_fHeight;
	float m_fRadius;
	float m_fEyeOffset;
};

// ------------------------------------------------------------------------------

class ControllerHitReport : public NxUserControllerHitReport
{
	public:
	virtual NxControllerAction  onShapeHit(const NxControllerShapeHit& hit)
	{
//		renderTerrainTriangle(hit.faceID);

		if(1 && hit.shape)
		{
			NxCollisionGroup group = hit.shape->getGroup();
			if(g_pScene->getGroupCollisionFlag(group, PHYSGROUP_PLAYER))
			{
				NxActor& actor = hit.shape->getActor();
				if(actor.isDynamic())
				{
					if (abs(hit.dir.y) < 0.9f) // normal interaction
					{
						NxVec3 dir = hit.dir;
						//dir.y /= 10;
						NxF32 coeff = actor.getMass() * hit.length;
						actor.addForceAtLocalPos(dir*coeff, NxVec3(0,0,0), NX_IMPULSE);
//						actor.addForceAtPos(hit.dir*coeff, hit.controller->getPosition(), NX_IMPULSE);
//						actor.addForceAtPos(hit.dir*coeff, hit.worldPos, NX_IMPULSE);
					}
					else // y is almost -1 or 1, so up or down. It can be standing
					{
						NxVec3 dir;
						dir = hit.worldPos - hit.controller->getPosition();
						dir.normalize();
						dir += hit.dir;
						dir.normalize();
						dir.y /= 15; // aby moc neskakalo

						NxVec3 vel = hit.controller->getActor()->getLinearVelocity();
						vel *=  0.004f; // protoze kopeme a netlacime
						NxF32 coeff = actor.getMass() * hit.length * vel.distance(NxVec3(0,0,0));
						actor.addForceAtLocalPos(dir*coeff, NxVec3(0,0,0), NX_IMPULSE);
					}
				}
			}
		}

		return NX_ACTION_NONE;
	}

	virtual NxControllerAction  onControllerHit(const NxControllersHit& hit)
	{
		return NX_ACTION_NONE;
	}

} g_ControllerHitReport;
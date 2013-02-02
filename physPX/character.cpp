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
#include "character.h"

REGISTER_SINGLE_CLASS(CP3DPhysCharacter, IP3DPHYS_CHARACTER); // TODO: jako nesingleton - moznost pouzit jako AI charactera treba

CP3DPhysCharacter::CP3DPhysCharacter()
{
	m_pChar = NULL;
}

CP3DPhysCharacter::~CP3DPhysCharacter()
{
	if (m_pChar) g_pCtrlMgr->releaseController(*m_pChar);
}

bool CP3DPhysCharacter::CreateCharacter(float fHeight, float fRadius, float fEyeOffset)
{
	m_fHeight = fHeight; m_fRadius = fRadius; m_fEyeOffset = fEyeOffset;

	bLoaded = false;

	NxCapsuleControllerDesc desc;
	desc.height = m_fHeight - 2*m_fRadius - 15.0f; // - step offset
	desc.radius = m_fRadius;
	desc.slopeLimit = 0.7f; // asi v rad
	desc.stepOffset = 35.0f;
	desc.upDirection = NX_Y;
	desc.callback = &g_ControllerHitReport;

	m_pChar = (NxCapsuleController*)g_pCtrlMgr->createController(g_pScene, desc);

	int nmbShp = m_pChar->getActor()->getNbShapes();
	NxShape *const * pShp = m_pChar->getActor()->getShapes();
	for (int i=0;i<nmbShp;i++)
		pShp[i]->setGroup(PHYSGROUP_PLAYER);

	if(!m_pChar)
	{
		CON(MSG_CON_ERR, "Can't create capsule character controller!");
		return false;
	}

	bLoaded = true;
	return true;
}

void CP3DPhysCharacter::GetPosition(P3DXVector3D &pos)const
{
	if (!bLoaded) return;
	const NxExtendedVec3 vec = m_pChar->getFilteredPosition();
	pos = P3DXVector3D((float)vec.x, (float)vec.y+m_fHeight/2.0f-m_fEyeOffset, (float)vec.z);
}

void CP3DPhysCharacter::Move(P3DXVector3D &moveVec)
{
	if (!bLoaded) return;

	P3DXVector3D vec(0, -9.8f, 0);
	vec += moveVec;
	vec.Normalize();
	vec *= 5000.0f * GetTimeDelta();
	UINT u;
	m_pChar->move(NxVec3(vec.x, vec.y, vec.z), 0xFFFFFFFF, 0.000001f, u );
}
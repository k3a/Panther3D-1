//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	NULL body - entity class
//-----------------------------------------------------------------------------
#include "ePhys_null.h"

REGISTER_ENTITY(ePhys_null, "phys_null", false, false, false);

ePhys_null::ePhys_null()
{
	ENT_DATA("body", m_nBody, TYPE_POINTER, 0, E_INTERNAL, "");
}

ePhys_null::~ePhys_null()
{
	SAFE_DELETE(m_nBody);
}

bool ePhys_null::Spawn(geometryData_s& gdata)
{
	GetWorldPos(m_vPos);

	g_pNewtonWorld = (IP3DPhysWorld*)I_GetClass(IP3DPHYS_PHYSWORLD);
	if (!g_pNewtonWorld) return false;

	m_nBody = (IP3DRigidBody*)I_GetClass(IP3DPHYS_RIGIDBODY);
	if (m_nBody == NULL) return false;

	NewtonCollision *cColl;
	cColl = g_pNewtonWorld->CollisionCreateNull();
	if (!cColl) return false;

	if(!m_nBody->Initialize(cColl)) return false;

	P3DXMatrix mat;
	mat.SetIdentityMatrix();
	mat.m_posit.x = m_vPos.x;
	mat.m_posit.y = m_vPos.y;
	mat.m_posit.z = m_vPos.z;

	m_nBody->SetTransformMatrix(mat);

	return true;
}
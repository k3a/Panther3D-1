//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	NULL body - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class ePhys_null : public eBase
{
public:
	ePhys_null();
	~ePhys_null();
	bool Spawn(geometryData_s& gdata);
private:
	P3DVector3D m_vPos;
	IP3DRigidBody *m_nBody;
};
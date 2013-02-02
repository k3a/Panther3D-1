//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Rotation. Entity class declaration.
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "eBase.h"
#include "ip3dphysicsengine.h"

class eFunc_rotation : public eBase
{
public:
	eFunc_rotation();
	~eFunc_rotation(){SAFE_DELETE_ARRAY(m_szTarget);};
	bool Spawn(geometryData_s& gdata);
	void Finalize();
	void Think(float deltaTime);
	void Activate(eBase* pCaller, int nValue){m_bEnabled=nValue;};
private:
	P3DVector3D m_vOmega;
	//
	IP3DRigidBody *m_pBody;
	char* m_szTarget;
	//
	bool m_bEnabled;
};
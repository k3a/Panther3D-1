//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Corkscrew phys. constraint - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class ePhys_corkscrew : public eBase
{
public:
	ePhys_corkscrew();
	~ePhys_corkscrew();
	void Finalize();
	void Think(float deltaTime);
	void Break(eBase* pCaller, int nValue);
	bool Spawn(geometryData_s& gdata);
private:
	P3DVector3D m_vPos;
	P3DVector3D m_vDir;
	char *m_pszTarget1;
	char *m_pszTarget2;
	float m_fMaxForce;
	//
	NewtonJoint* m_pJoint;
	// udalosti
	EntEvent OnBreak;
};
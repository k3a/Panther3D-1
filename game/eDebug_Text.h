//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (5.9.2006)
// Purpose:	Debug point entity - draws text in the screen space
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eDebug_text : public eBase
{
public:
	eDebug_text();
	~eDebug_text();
	bool Spawn(geometryData_s& gdata);
	void Render(float deltaTime);
	bool Command(eBase* pCaller, int cmdID, ECMD_PARAM *params, int numParams);
private:
	P3DXVector3D m_vPos;
	char *m_pszText;
	int m_nMaxDist;
};
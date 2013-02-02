//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (20.10.2006)
// Purpose:	Environmental sound player ;)
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eEnv_sound : public eBase
{
public:
	eEnv_sound();
	~eEnv_sound();
	bool Spawn(geometryData_s& gdata);
	void Think(float deltaTime);
	void Activate(eBase* pCaller, int nValue);
private:
	P3DVector3D m_vPos;
	char* strSndStart;
	char* strSnd;
	char* strSndEnd;
	int nLoop; // 0 - prehrat jednou, 1-opakovat jednou, n-opakovat nkrat
	int play; // 0 - nehraje nic, 1 - hraje begin, 2 - hraje zvuk, 3 - hraje end
	//	
	EntEvent OnFinish;
	EntEvent OnBegin;
	EntEvent OnStop;
	EntEvent OnPause;
};
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (22.10.2006)
// Purpose:	Shake entity (camera shake)
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"

class eEnv_shake : public eBase
{
public:
	eEnv_shake();
	void Activate(eBase* pCaller, int nValue);
private:
	int m_nIntensity;
};
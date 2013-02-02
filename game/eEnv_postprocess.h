//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Postprocess entity - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

#define MAX_POSTPROCESS_NUM	10

class eEnv_postprocess : public eBase
{
public:
	eEnv_postprocess();
	~eEnv_postprocess();
	bool Spawn(geometryData_s& gdata);
	void Reset(eBase* pCaller, int nValue);
	void Activate(eBase* pCaller, int nValue);
private:
	char *m_pszPost[MAX_POSTPROCESS_NUM];
};
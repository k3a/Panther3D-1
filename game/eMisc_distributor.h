//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (22.10.2006)
// Purpose:	Distributor entity (if you fire this entity it fires first, 
//			then next, next, ...)
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"

class eMisc_distributor : public eBase
{
public:
	eMisc_distributor();
	void Activate(eBase* pCaller, int nValue);
private:
	EntEvent On1Activate;
	EntEvent On2Activate;
	EntEvent On3Activate;
	EntEvent On4Activate;
	EntEvent On5Activate;
	EntEvent On6Activate;
	EntEvent On7Activate;
	EntEvent On8Activate;
	EntEvent On9Activate;
	EntEvent On10Activate;
	//
	int m_fActItem;
	bool m_bLoop;
};
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (20.10.2006)
// Purpose:	Entity branch (if you fire this entity it fires other)
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"

class eMisc_branch : public eBase
{
public:
	eMisc_branch();
	void Think(float deltaTime);
	void Finalize();
	void Activate(eBase* pCaller, int nValue);
private:
	EntEvent OnAction;
	float m_fWait;
	float m_fActTime;
	eBase* m_pCaller;
};
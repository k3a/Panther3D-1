//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	IF function - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eMisc_if : public eBase
{
public:
	eMisc_if();
	void Toggle(eBase* pCaller, int nValue)
	{
		m_nValue = !m_nValue;
		if (nValue) // provest test
		{
			if (m_nValue) 
				OnTrue.RaiseEvent(this);
			else
				OnFalse.RaiseEvent(this);
		}
	};
	void Value(eBase* pCaller, int nValue){m_nValue = nValue;};
	void Activate(eBase* pCaller, int nValue)
	{ 
		if (m_nValue) 
			OnTrue.RaiseEvent(this);
		else
			OnFalse.RaiseEvent(this);
	};
private:
	int m_nValue;
	//
	EntEvent OnTrue;
	EntEvent OnFalse;
};
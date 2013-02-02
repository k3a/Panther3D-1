//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (20.10.2006)
// Purpose:	Entity branch (if you fire this entity it fires other)
//-----------------------------------------------------------------------------
#include "eMisc_branch.h"

REGISTER_ENTITY(eMisc_branch, "misc_branch", false, true, false);

eMisc_branch::eMisc_branch()
{
	ENT_DATA("wait", m_fWait, TYPE_FLOAT, 0, E_KEY_DONT_SAVE, "#entities_edWait");
	ENT_DATA("akt_time", m_fActTime, TYPE_FLOAT, 0, E_SAVE, "");

	ENT_EVENT("OnAction", OnAction, "#entities_eeOnAction");
}

void eMisc_branch::Finalize()
{
	m_fActTime = m_fWait+1.0f;
}

void eMisc_branch::Think(float deltaTime)
{
	if (m_fActTime < m_fWait) // pokud jeste nebylo aktivovano
	{
		m_fActTime+=deltaTime; // pricti
		if (m_fActTime >= m_fWait) // FIRE!
			OnAction.RaiseEvent(m_pCaller);
	}
}

void eMisc_branch::Activate(eBase* pCaller, int nValue)
{
	m_pCaller = pCaller;
	m_fActTime = -0.001f; // - kvuli mozne nuly u wait
}
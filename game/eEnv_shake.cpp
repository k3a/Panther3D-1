//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (22.10.2006)
// Purpose:	Shake entity (camera shake)
//-----------------------------------------------------------------------------
#include "eEnv_shake.h"

REGISTER_ENTITY(eEnv_shake, "env_shake", false, false, false);

eEnv_shake::eEnv_shake()
{
	ENT_DATA("intensity", m_nIntensity, TYPE_INT, 1, E_KEY_DONT_SAVE, "#entities_edIntensity"); // stupen zatreseni (dal bych asi 1-3)
}

void eEnv_shake::Activate(eBase* pCaller, int nValue)
{
	// TODO: zatrast kamerou
}
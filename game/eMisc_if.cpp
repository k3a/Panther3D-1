//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	IF function - entity class
//-----------------------------------------------------------------------------
#include "eMisc_if.h"

REGISTER_ENTITY(eMisc_if, "misc_if", false, false, false);

eMisc_if::eMisc_if()
{
	ENT_DATA("value", m_nValue, TYPE_INT, 0, E_KEY_SAVE, "#entities_edValue");

	ENT_EVENT("OnTrue", OnTrue, "");
	ENT_EVENT("OnFalse", OnFalse, "");
}
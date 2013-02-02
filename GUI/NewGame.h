//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Dialog New game
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"

class CP3DGUI_NewGame : public CWindow
{
public:
	CP3DGUI_NewGame();
private:
	CLabel m_lblDesc;
	CLabel m_lblDifficulty;
	COption m_optLow;
	COption m_optMedium;
	COption m_optHigh;
	COption m_optExtraHigh;

	CButton m_btnPlay;
	CButton m_btnCancel;
};
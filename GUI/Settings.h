//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Dialog Settings
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"

class CP3DGUI_Settings : public CWindow
{
public:
	CP3DGUI_Settings();
private:
	CTabStrip m_tabMain;

	CFrame m_frmGame;
	 CLabel m_lblDifficulty;
	  COption m_optLow;
	  COption m_optMedium;
	  COption m_optHigh;
	  COption m_optExtraHigh;
	 CLabel m_lblLanguage;
	  CComboBox m_cmbLanguage;
	CFrame m_frmControls;
	 CLabel m_lblTodo;
	CFrame m_frmDisplay;
	 CLabel m_lblResolution;
	 CComboBox m_cmbResolution;
	 CCheckBox m_chkWindowed;
};
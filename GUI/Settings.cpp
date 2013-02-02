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
#include "Settings.h"

CP3DGUI_Settings::CP3DGUI_Settings()
{
	CreateWindow(g_pLocaliz->Translate_const("#GUI_Settings"), 
		0, 0, 500, 400, P3DWS_CAPTION | P3DWS_CLOSEBTN);
	CenterScreen();

	m_tabMain.CreateTabStrip(10, 10, 480, 380, this);
		m_frmGame.CreateFrame(NULL, 0, 22, 480, 360, false, &m_tabMain);
		  m_lblDifficulty.CreateLabel(g_pLocaliz->Translate_const("#GUI_Difficulty"), 30, 30, 180, 13, false, FA_RIGHT, &m_frmGame);
		    m_optLow.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyLow"), 220, 30, 180, 20, false, &m_frmGame);
		    m_optMedium.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyMedium"), 220, 50, 180, 20, false, &m_frmGame);
		    m_optHigh.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyHigh"), 220, 70, 180, 20, false, &m_frmGame);
		    m_optExtraHigh.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyExtraHigh"), 220, 90, 180, 20, false, &m_frmGame);
		  m_lblLanguage.CreateLabel(g_pLocaliz->Translate_const("#GUI_Language"), 30, 110, 180, 13, false, FA_RIGHT, &m_frmGame);
		    m_cmbLanguage.CreateComboBox(220, 110, 100, 20, &m_frmGame);
		      m_cmbLanguage.AddItem("Èesky");
			  m_cmbLanguage.AddItem("English");
		m_frmControls.CreateFrame(NULL, 0, 22, 480, 360, false, &m_tabMain);
		  // TODO: novy gui prvek na nastavovani klaves
		  m_lblTodo.CreateLabel("Již brzy ;)", 30, 30, 180, 13, false, FA_RIGHT, &m_frmControls);
		m_frmDisplay.CreateFrame(NULL, 0, 22, 480, 360, false, &m_tabMain);
		  m_lblResolution.CreateLabel(g_pLocaliz->Translate_const("#GUI_Resolution"), 30, 30, 180, 13, false, FA_RIGHT, &m_frmDisplay);
		    m_cmbResolution.CreateComboBox(220, 30, 100, 20, &m_frmDisplay);
		      m_cmbResolution.AddItem("640 x 480");
		      m_cmbResolution.AddItem("800 x 600");
		      m_cmbResolution.AddItem("1024 x 768");
		      m_cmbResolution.AddItem("1152 x 864");
		      m_cmbResolution.AddItem("1280 x 960");
		    m_chkWindowed.CreateCheckBox(g_pLocaliz->Translate_const("#GUI_Windowed"), 220, 55, 180, 13, false, &m_frmDisplay);
		// pridej do tabstripu
		m_tabMain.AddTab(g_pLocaliz->Translate_const("#GUI_SettGame"), &m_frmGame);
		m_tabMain.AddTab(g_pLocaliz->Translate_const("#GUI_SettControls"), &m_frmControls);
		m_tabMain.AddTab(g_pLocaliz->Translate_const("#GUI_SettDisplay"), &m_frmDisplay);
		m_tabMain.AddTab(g_pLocaliz->Translate_const("#GUI_SettSound"), &m_frmGame);
		m_tabMain.AddTab(g_pLocaliz->Translate_const("#GUI_SettPerf"), &m_frmGame);
}
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
#include "NewGame.h"

#define INIT_ELEMENTx(uniqueName) uniqueName .m_szName = #uniqueName

CP3DGUI_NewGame::CP3DGUI_NewGame()
{
	CreateWindow(g_pLocaliz->Translate_const("#GUI_NewGame"), 
		0, 0, 300, 177, P3DWS_CAPTION | P3DWS_CLOSEBTN);
	CenterScreen();

		m_lblDesc.CreateLabel(g_pLocaliz->Translate_const("#GUI_NewGameDesc"), 10, 10, 280, 45,
			false, LA_LEFT, this);
		m_lblDifficulty.CreateLabel(g_pLocaliz->Translate_const("#GUI_Difficulty"), 10, 55, 280, 13, 
			true, FA_WORDBREAK, this);
		m_optLow.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyLow"), 10, 78, 280, 20, true, this);
		m_optMedium.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyMedium"), 10, 103, 280, 20, true, this);
		m_optHigh.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyHigh"), 10, 128, 280, 20, true, this);
		m_optExtraHigh.CreateOption(g_pLocaliz->Translate_const("#GUI_DifficultyExtraHigh"), 10, 153, 280, 20, true, this);

		m_btnPlay.CreateButton(g_pLocaliz->Translate_const("#GUI_Play"), 218, 119, 70, 20, this);
		m_btnCancel.CreateButton(g_pLocaliz->Translate_const("#GUI_Cancel"), 218, 144, 70, 20, this);
}
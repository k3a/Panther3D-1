//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI System DLL
//-----------------------------------------------------------------------------
#pragma once
#include "IP3DGUI.h"
#include "IP3DMaterialManager.h"
#include "common.h"
#include "MainMenu.h"

//dialogy
#include "NewGame.h"
#include "Settings.h"

class CGUISystem : public IP3DGUISystem
{
public:
	~CGUISystem();
	bool InitGUISystem();
	bool Render(float deltaTime); // pokud vrati false nema se vykreslovat a aktualizovat hra
	void Update(float deltaTime);
private:
	// zakladni dialogy
	CP3DGUI_NewGame* g_pDia_NewGame;
	CP3DGUI_Settings* g_pDia_Settings;
};

//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI SYSTEM
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"

class IP3DGUISystem : public IP3DBaseInterface
{
public:
	virtual bool InitGUISystem()=0;
	virtual bool Render(float deltaTime)=0; // pokud vrati false nema se vykreslovat a aktualizovat hra
	virtual void Update(float deltaTime)=0;
};

#define IP3DGUI_GUISYSTEM "P3DGUISystem_1" // nazev ifacu
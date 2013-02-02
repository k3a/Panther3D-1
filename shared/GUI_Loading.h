//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI_Loading interface
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"

class IP3DGUI_Loading : public IP3DBaseInterface
{
public:
	virtual void SetProgress(float fValue)=0;
};

#define IP3DGUI_LOADING "P3DGUI_Loading_1" // nazev ifacu
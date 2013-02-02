//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Game language string manager - iface
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

class IP3DLocalization : public IP3DBaseInterface
{
public:
	virtual void Shutdown()=0;
	virtual bool Translate(char* szInOutString)=0; // pokud bude buffer maly, pokusi se ho manager zvetsit
	virtual const char* Translate_const(const char* szInString)=0; // NEPOUZIVAT JAKO UKAZATEL, VZDY JEN ZKOPIROVAT VRACENY STRING!
};

#define IP3DENGINE_LOCALIZATION "P3DLocalization_1" // nazev ifacu
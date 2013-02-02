//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Stereoscopic manager (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

class IP3DStereoscopic : public IP3DBaseInterface
{
public:
	virtual void Begin (int eye) = 0; // 0 - 1
	virtual void End (int eye) = 0; // 0 - 1
};

#define IP3DRENDERER_STEREOSCOPIC "IP3DStereoscopic_1" // nazev ifacu
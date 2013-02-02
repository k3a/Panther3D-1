//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Sky rendering interface (engine)
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"

class IP3DSky : public IP3DBaseInterface
{
public:
	virtual bool CreateSkybox(const char* strSkyTexturePath, const char* strCloudsTexturePath, float fSkyAngle=0.0f)=0;
	virtual void Render(bool drawSky, bool drawClouds)=0;
};

#define IP3DRENDERER_SKY "P3DSky_3" // nazev ifacu

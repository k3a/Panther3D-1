//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (9.12.2006)
// Purpose:	Water Manager Iface (Helper functions) (Renderer)
//-----------------------------------------------------------------------------
#pragma once


#include "interface.h"
#include "types.h"


class IP3DWaterManager : public IP3DBaseInterface
{
public:
	virtual void BeginRefractiveScene() = 0; // Vola se jako prvni!
	virtual void BeginReflectiveScene(P3DXVector3D *reflCameraPos=NULL) = 0; // Vola se jako druhe!
	virtual void BeginWaterSurface() = 0; // Vola se jako treti. Spusti pocatek shaderu
	virtual void EndWaterSurface() = 0; // ukonci aplikaci shader
};

#define IP3DRENDERER_WATERMANAGER "P3DWaterManager_1" // nazev ifacu
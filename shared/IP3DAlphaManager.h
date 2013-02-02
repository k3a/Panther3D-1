//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Alpha manager interface (renderer)
//-----------------------------------------------------------------------------
#pragma once


#include "interface.h"
#include "types.h"
#include "IP3DMesh.h"

class IP3DAlphaManager : public IP3DBaseInterface
{
public:
	virtual void UnloadAll() = 0;
	virtual bool CreateGlassWindow (P3DXVector3D *points, char* szAlphaTexture = NULL, char* szCubeMapTexture = NULL) = 0;
	virtual bool CreateGrassBunch (P3DXVector3D origin, float fRadius, WORD quadCount, \
				float fFadeDistance, float fMinGrassQuadSize, float fMaxGrassQuadSize, \
				char *szTexture, bool bBillboarded, bool bAnimated) = 0;
 	virtual DWORD CreateDynamicModel (IP3DMesh *pInitialMesh, P3DSphere &bsphere) = 0;
	virtual void UpdateDynamicModel(DWORD dwDynModel, IP3DMesh *pNewMesh, P3DMatrix &mat, bool bVisible, bool bIsLighted, SHADER_DYNLIGHT_INPUT &shader_input)=0;
	virtual void Loop () = 0;
	virtual void Render () = 0;
	virtual void BreakWindow (unsigned long Idx) = 0;
};

#define IP3DRENDERER_ALPHAMANAGER "P3DAlphaManager_3" // nazev ifacu
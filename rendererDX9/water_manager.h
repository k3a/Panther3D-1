//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (9.12.2006)
// Purpose:	Water Manager(Helper functions)
//-----------------------------------------------------------------------------
#pragma once

#include "IP3DWaterManager.h"
#include "common.h"

class CP3DWaterManager : public IP3DWaterManager
{
public:
	CP3DWaterManager();
	~CP3DWaterManager();
	bool Init();
	void BeginRefractiveScene(); // Vola se jako prvni!
	void BeginReflectiveScene(P3DXVector3D *reflCameraPos); // Vola se jako druhe!
	void BeginWaterSurface(); // Vola se jako treti. Spusti pocatek shaderu
	void EndWaterSurface(); // ukonci aplikaci shader
	//
	void SetShaderParameters();
private:
	// Refract
	LPDIRECT3DTEXTURE9 m_pTexRefract;
	LPDIRECT3DSURFACE9 m_pSurfRefract;
	
	// Reflect
	LPDIRECT3DTEXTURE9 m_pTexReflect;
	LPDIRECT3DSURFACE9 m_pSurfReflect;
	
	// Original
	LPDIRECT3DSURFACE9 m_pBackBufferSurf;
	LPDIRECT3DSURFACE9 m_pDepthStencil;
	LPDIRECT3DSURFACE9 m_pWaterDepthStencil;
	
	// Mat
	DWORD m_dwMatWater; // material na vodni polygony
	DWORD m_dwMatClipmask; // material na vytvoreni clipmasky pro refraction
	D3DXMATRIX m_matViewOld;
};
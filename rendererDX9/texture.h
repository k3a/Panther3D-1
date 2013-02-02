//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Texture map (jpg, tga, ....) class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dtexture.h"
#include "d3dx9.h"

class CP3DTexture : public IP3DTexture
{
public:
	CP3DTexture();
	~CP3DTexture();
	bool Load(const char* FileName, TexType texType, bool bForceToCreateNew = false);
	void UnLoad ();
	void Use(unsigned long Stage = 0);
	bool Create(unsigned int Width, unsigned int Height,unsigned int MipLevels, TexFormat format, bool renderTarget);
	void GetSurfaceData(unsigned int SurfaceLevel = 0, P3DLOCKED_RECT *pRect = NULL);
	void SaveSurfaceData();
	void Filter(unsigned long ulFilter);
	void *GetD3DTexture();
	void *GetD3DSurface(unsigned int nSurfaceLevel);
	void GetDesc(TexDesc &desc);
	bool SetAsRenderTarget(DWORD rtIndex);
	bool CaptureCurrentBackBuffer();
private:
	bool FindExtension(char *strFileName);
	union
	{
		LPDIRECT3DTEXTURE9 m_pTexture;
		LPDIRECT3DCUBETEXTURE9 m_pCubeTexture;
		LPDIRECT3DVOLUMETEXTURE9 m_pVolumeTexture;
	};
	LPDIRECT3DSURFACE9	m_pSurface;
	TexDesc m_texDesc;
};

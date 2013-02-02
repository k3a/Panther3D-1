//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Texture manager class declaration - ensures that each texture is loading only once
//-----------------------------------------------------------------------------

#pragma once

#include "d3dx9.h"
#include "common.h"
#include "symbols.h"
#include "IP3DTexture.h"
#include "ip3dfilesystem.h"


#define BUF_DEFAULT_SIZE		100
#define BUF_ENLARGE_STEP		25


//---------------------------------
struct TEXTURE_RECORD {
	union {
		LPDIRECT3DTEXTURE9			pTexture2D;
		LPDIRECT3DCUBETEXTURE9		pTextureCube;
		LPDIRECT3DVOLUMETEXTURE9	pTextureVolume;
	};
	TexType	type;
	char		*szTextureName;
	int			intRefNum;		// pocet referencii -> ak==0 tak sa polozka rusi
	bool		bInUse;			// pouzivane pri StartCaching(), StopCaching()
};


//---------------------------------
class CP3DTextureLoader
{
private:
	TEXTURE_RECORD		*m_pTextures;			// tabulka s texturami
	DWORD				m_dwTextureCount;		// pocet prvkov v m_pTextures
	TEXTURE_RECORD		*m_pUniqueTextures;		// tabulka s texturami ktore boli vytvorene s bForceToCtreateNew
	DWORD				m_dwUniqueTextureCount;	// pocet prvkov v m_pUniqueTextures

	LONG EnlargeBuffer();
	LONG EnlargeUniqueBuffer();
	void* Load (LPCSTR szFileName, TexType type, bool bForceToCreateNew);

public:
	CP3DTextureLoader();
	~CP3DTextureLoader();
	bool Init ();
	LPDIRECT3DTEXTURE9 CreateTexture (LPCSTR szFileName, bool bForceToCtreateNew = false);
	LPDIRECT3DCUBETEXTURE9 CreateCubeTexture (LPCSTR szFileName, bool bForceToCtreateNew = false);
	LPDIRECT3DVOLUMETEXTURE9 CreateVolumeTexture (LPCSTR szFileName, bool bForceToCtreateNew = false);
	bool AddRef (void* pTextureToAddRef);
	bool UnloadTexture (void* pTextureToUnload);
	bool UnloadAll ();
};

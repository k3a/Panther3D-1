
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Texture loader class definition
//-----------------------------------------------------------------------------


#include "texture_loader.h"

extern IP3DFileSystem		*g_pFS;
extern LPDIRECT3DDEVICE9	g_pD3DDevice;


//#define DEBUG_LOG_TEXTURE_INFO		// vypise info o nacitanych texturach do konzole

//---------------------------------
CP3DTextureLoader::CP3DTextureLoader()
{
	m_pTextures = NULL;
	m_dwTextureCount = 0;
	m_pUniqueTextures = NULL;
	m_dwUniqueTextureCount = 0;
}

//---------------------------------
CP3DTextureLoader::~CP3DTextureLoader()
{
	UnloadAll ();
	SAFE_DELETE_ARRAY (m_pTextures)
	SAFE_DELETE_ARRAY (m_pUniqueTextures)
}

// zvacsi buffer o BUF_ENLARGE_STEP poloziek, vrati index najnizsej volnej polozky, -1 v pripade chyby
//---------------------------------
LONG CP3DTextureLoader::EnlargeBuffer()
{
	DWORD dwNewBufferMax, i;
	TEXTURE_RECORD	*pTempBuffer;

	if (m_pTextures == NULL)
		return -1;
	dwNewBufferMax = m_dwTextureCount + BUF_ENLARGE_STEP;
	pTempBuffer = new TEXTURE_RECORD[dwNewBufferMax];
	if (pTempBuffer == NULL)
		return -1;

	memset (pTempBuffer, 0, sizeof(TEXTURE_RECORD) * dwNewBufferMax);
	memcpy (pTempBuffer, m_pTextures, sizeof(TEXTURE_RECORD) * m_dwTextureCount);
	SAFE_DELETE_ARRAY (m_pTextures)
	m_pTextures = pTempBuffer;
	m_dwTextureCount = dwNewBufferMax;
	// vyhladanie volneho indexu
	for (i=0; i<m_dwTextureCount; i++)
		if (m_pTextures[i].pTexture2D == NULL)
			return (LONG)i;

	return -1;		// neznama chyba
}

// zvacsi buffer o BUF_ENLARGE_STEP poloziek, vrati index najnizsej volnej polozky, -1 v pripade chyby
//---------------------------------
LONG CP3DTextureLoader::EnlargeUniqueBuffer()
{
	DWORD dwNewBufferMax, i;
	TEXTURE_RECORD	*pTempBuffer;

	if (m_pUniqueTextures == NULL)
		return -1;
	dwNewBufferMax = m_dwUniqueTextureCount + BUF_ENLARGE_STEP;
	pTempBuffer = new TEXTURE_RECORD[dwNewBufferMax];
	if (pTempBuffer == NULL)
		return -1;

	memset (pTempBuffer, 0, sizeof(TEXTURE_RECORD) * dwNewBufferMax);
	memcpy (pTempBuffer, m_pUniqueTextures, sizeof(TEXTURE_RECORD) * m_dwUniqueTextureCount);
	SAFE_DELETE_ARRAY (m_pUniqueTextures)
	m_pUniqueTextures = pTempBuffer;
	m_dwUniqueTextureCount = dwNewBufferMax;
	// vyhladanie volneho indexu
	for (i=0; i<m_dwUniqueTextureCount; i++)
		if (m_pUniqueTextures[i].pTexture2D == NULL)
			return (LONG)i;

	return -1;		// neznama chyba
}

// inicializacia, alokovanie pamate pre buffer uchovavajuci info o jednotlivych texturach
// v pripade potreby  sa buffer automaticky zvacsi
//---------------------------------
bool CP3DTextureLoader::Init ()
{
	m_pTextures = new TEXTURE_RECORD[BUF_DEFAULT_SIZE];
	if (m_pTextures == NULL)
		return false;

	m_pUniqueTextures = new TEXTURE_RECORD[BUF_ENLARGE_STEP];
	if (m_pUniqueTextures == NULL)
		{delete[] m_pTextures; return false;}

	memset (m_pTextures, 0, sizeof(TEXTURE_RECORD) * BUF_DEFAULT_SIZE);
	memset (m_pUniqueTextures, 0, sizeof(TEXTURE_RECORD) * BUF_ENLARGE_STEP);
	m_dwTextureCount = BUF_DEFAULT_SIZE;
	m_dwUniqueTextureCount = BUF_ENLARGE_STEP;
	return true;
}

// vrati objekt textury, NULL v pripade chyby
//---------------------------------
void* CP3DTextureLoader::Load (LPCSTR szFileName, TexType type, bool bForceToCreateNew)
{
	DWORD		i;
	void*		pRet = NULL;
	LONG		lFreeIdx = -1;
	char		szTextureName[MAX_PATH] = {0};

	if ((m_pTextures == NULL) || (m_pUniqueTextures == NULL) || (szFileName == NULL) || (szFileName[0] == 0))
		return NULL;

	strcpy (szTextureName, szFileName);
	if (szTextureName[strlen (szTextureName)-4] != '.')
		strcat (szTextureName, ".dds");

	// ak textura s rovnakym menom uz existuje potom sa vrati odkaz na tento objekt
	if (!bForceToCreateNew)
	{
		for (i=0; i<m_dwTextureCount; i++)
			if (m_pTextures[i].szTextureName && m_pTextures[i].pTexture2D)
			{
				if ((m_pTextures[i].type == type) && (stricmp (m_pTextures[i].szTextureName, szTextureName) == 0))
				{
					// existujuci zaznam bol najdeny
					m_pTextures[i].intRefNum++;		// zvysenie referencii
					return m_pTextures[i].pTexture2D;
				}
			}
			else if (lFreeIdx < 0)
				lFreeIdx = i;
	}
	else for (i=0; i<m_dwUniqueTextureCount; i++)		// vyhladanie volneho prvku
		if (m_pUniqueTextures[i].pTexture2D == NULL)
			{lFreeIdx = i; break;}
	
	// je potrebne zvacsit bufer
	if (lFreeIdx < 0)
		if (bForceToCreateNew)
			{if ((lFreeIdx = EnlargeUniqueBuffer()) < 0) return NULL;}
		else
			if ((lFreeIdx = EnlargeBuffer()) < 0) return NULL;

	if (!bForceToCreateNew)
	{
		// alokovanie miesta a kopirovanie szTextureName
		DWORD dwTextLen = (DWORD)strlen (szTextureName);
		if ((dwTextLen == 0) || (dwTextLen > MAX_PATH-20))
			return NULL;
		dwTextLen++;
		m_pTextures[lFreeIdx].szTextureName = new char[dwTextLen];
		if (m_pTextures[lFreeIdx].szTextureName == NULL)
			return NULL;
		memcpy (m_pTextures[lFreeIdx].szTextureName, szTextureName, dwTextLen);
	}

	TEXTURE_RECORD *pTempRecord;
	if (bForceToCreateNew)
		pTempRecord = &m_pUniqueTextures[lFreeIdx];
	else
		pTempRecord = &m_pTextures[lFreeIdx];

	//////////////////////////////////////////////////////////////////////////
	// vytvorenie textury
	FSFILE	*pFile;
	BYTE		*pData;
	DWORD		dwSize;

	if (pFile = g_pFS->Load (szTextureName, pData, dwSize))
	{
		switch (type)
		{
		case TT_2D:
			if (SUCCEEDED (D3DXCreateTextureFromFileInMemory (g_pD3DDevice, (LPCVOID)pData, dwSize, &pTempRecord->pTexture2D)))
			{
				if (pTempRecord->pTexture2D->GetLevelCount() < 2)
					pTempRecord->pTexture2D->GenerateMipSubLevels ();
				pTempRecord->intRefNum = 1;
				pTempRecord->type = TT_2D;
				pRet = (void*) pTempRecord->pTexture2D;

				D3DSURFACE_DESC desc;
				for (unsigned long i=0; i<pTempRecord->pTexture2D->GetLevelCount(); i++)
				{
					pTempRecord->pTexture2D->GetLevelDesc(i, &desc);
					STAT(STAT_TEXTURE_ALL_TEXELS, desc.Width*desc.Height);
				}
			}
			break;
		case TT_CUBE:
			if (SUCCEEDED (D3DXCreateCubeTextureFromFileInMemory (g_pD3DDevice, (LPCVOID)pData, dwSize, &pTempRecord->pTextureCube)))
			{
				if (pTempRecord->pTextureCube->GetLevelCount() < 2)
					pTempRecord->pTextureCube->GenerateMipSubLevels ();
				pTempRecord->intRefNum = 1;
				pTempRecord->type = TT_CUBE;
				pRet = (void*) pTempRecord->pTextureCube;

				D3DSURFACE_DESC desc;
				for (unsigned long i=0; i<pTempRecord->pTexture2D->GetLevelCount(); i++)
				{
					pTempRecord->pTexture2D->GetLevelDesc(i, &desc); // desc. jen pro jednu stranu!!!
					STAT(STAT_TEXTURE_ALL_TEXELS, 6*desc.Width*desc.Height);
				}
			}
			break;
		case TT_VOLUME:
			if (SUCCEEDED (D3DXCreateVolumeTextureFromFileInMemory (g_pD3DDevice, (LPCVOID)pData, dwSize, &pTempRecord->pTextureVolume)))
			{
				if (pTempRecord->pTextureVolume->GetLevelCount() < 2)
					pTempRecord->pTextureVolume->GenerateMipSubLevels ();
				pTempRecord->intRefNum = 1;
				pTempRecord->type = TT_VOLUME;
				pRet = (void*) pTempRecord->pTextureVolume;

				// FIXME: Nejspise spatny vypocet!!!!
				D3DVOLUME_DESC desc;
				for (unsigned long i=0; i<pTempRecord->pTextureVolume->GetLevelCount(); i++)
				{
					pTempRecord->pTextureVolume->GetLevelDesc(i, &desc);
					STAT(STAT_TEXTURE_ALL_TEXELS, desc.Width*desc.Height);
				}
			}
		}

		g_pFS->UnLoad (pFile, pData);
	}

	if (pRet)
	{
#ifdef DEBUG_LOG_TEXTURE_INFO
		CON(MSG_CON_DEBUG, "-- DEBUG -- TextureLoader: texture %s successfully loaded", szFileName);
#endif
		return pRet;
	}

	// chyba
	pTempRecord->pTexture2D = NULL;
	SAFE_DELETE_ARRAY (pTempRecord->szTextureName)
	pTempRecord->type = TT_2D;
	pTempRecord->intRefNum = 0;
	if (g_pConsole)
		CON(MSG_CON_DEBUG, "Texture Loader...can't create texture:  %s", szFileName);
	return NULL;
}

//---------------------------------
LPDIRECT3DTEXTURE9 CP3DTextureLoader::CreateTexture (LPCSTR szFileName, bool bForceToCreateNew)
{
	return (LPDIRECT3DTEXTURE9)Load (szFileName, TT_2D, bForceToCreateNew);
}

//---------------------------------
LPDIRECT3DCUBETEXTURE9 CP3DTextureLoader::CreateCubeTexture (LPCSTR szFileName, bool bForceToCreateNew)
{
	return (LPDIRECT3DCUBETEXTURE9)Load (szFileName, TT_CUBE, bForceToCreateNew);
}

//---------------------------------
LPDIRECT3DVOLUMETEXTURE9 CP3DTextureLoader::CreateVolumeTexture (LPCSTR szFileName, bool bForceToCreateNew)
{
	return (LPDIRECT3DVOLUMETEXTURE9)Load (szFileName, TT_VOLUME, bForceToCreateNew);
}

// zvysenie poctu referencii textury
//---------------------------------
bool CP3DTextureLoader::AddRef (void* pTextureToAddRef)
{
	DWORD i;
	if (pTextureToAddRef == NULL) return false;
	if (m_pTextures )
		for (i=0; i<m_dwTextureCount; i++)
			if (m_pTextures[i].pTexture2D == pTextureToAddRef)
				{m_pTextures[i].intRefNum++; return true;}
	if (m_pUniqueTextures)
		for (i=0; i<m_dwUniqueTextureCount; i++)
			if (m_pUniqueTextures[i].pTexture2D == pTextureToAddRef)
				{m_pUniqueTextures[i].intRefNum++; return true;}
	return false;
}

// znizenie poctu referencii textury, ak ref. dosiahne 0 rusi sa cela polozka
//---------------------------------
bool CP3DTextureLoader::UnloadTexture (void* pTextureToUnload)
{
	if (pTextureToUnload == NULL) return false;

	if (m_pTextures)
		for (DWORD i=0; i<m_dwTextureCount; i++)
			if (m_pTextures[i].pTexture2D == pTextureToUnload)
			{
				m_pTextures[i].intRefNum--;
				if (m_pTextures[i].intRefNum == 0)
				{
					switch (m_pTextures[i].type)
					{
					case TT_2D: 
						{
							D3DSURFACE_DESC desc;
							for (unsigned long ii=0; ii<m_pTextures[i].pTexture2D->GetLevelCount(); ii++)
							{
								m_pTextures[i].pTexture2D->GetLevelDesc(ii, &desc);
								STAT(STAT_TEXTURE_ALL_TEXELS, -(int)(desc.Width*desc.Height));
							}
						}
						m_pTextures[i].pTexture2D->Release(); 
						break;
					case TT_CUBE: 
						{
							D3DSURFACE_DESC desc;
							for (unsigned long ii=0; ii<m_pTextures[i].pTextureCube->GetLevelCount(); ii++)
							{
								m_pTextures[i].pTextureCube->GetLevelDesc(ii, &desc); // desc. jen pro jednu stranu!!!
								STAT(STAT_TEXTURE_ALL_TEXELS, -(int)(6*desc.Width*desc.Height));
							}
						}
						m_pTextures[i].pTextureCube->Release();
						break;
					case TT_VOLUME: 
						{
							D3DVOLUME_DESC desc;
							for (unsigned long ii=0; ii<m_pTextures[i].pTextureVolume->GetLevelCount(); ii++)
							{
								m_pTextures[i].pTextureVolume->GetLevelDesc(ii, &desc);
								STAT(STAT_TEXTURE_ALL_TEXELS, -(int)(desc.Width*desc.Height));
							}
						}
						m_pTextures[i].pTextureVolume->Release();
					}

					m_pTextures[i].pTexture2D = NULL;
					SAFE_DELETE_ARRAY (m_pTextures[i].szTextureName)
				}
				return true;
			}

	if (m_pUniqueTextures)
		for (DWORD i=0; i<m_dwUniqueTextureCount; i++)
			if (m_pUniqueTextures[i].pTexture2D == pTextureToUnload)
			{
				m_pUniqueTextures[i].intRefNum--;
				if (m_pUniqueTextures[i].intRefNum == 0)
				{
					switch (m_pUniqueTextures[i].type)
					{
					case TT_2D: m_pUniqueTextures[i].pTexture2D->Release(); break;
					case TT_CUBE: m_pUniqueTextures[i].pTextureCube->Release(); break;
					case TT_VOLUME: m_pUniqueTextures[i].pTextureVolume->Release();
					}
					m_pUniqueTextures[i].pTexture2D = NULL;
					SAFE_DELETE_ARRAY (m_pUniqueTextures[i].szTextureName)
				}
				return true;
			}

	return false;
}


// uvolnenie vsetkych textur
//---------------------------------
bool CP3DTextureLoader::UnloadAll ()
{
	if (m_pTextures)
		for (DWORD i=0; i<m_dwTextureCount; i++)
		{
			if (m_pTextures[i].pTexture2D)
			{
				switch (m_pTextures[i].type)
				{
				case TT_2D: m_pTextures[i].pTexture2D->Release(); break;
				case TT_CUBE: m_pTextures[i].pTextureCube->Release(); break;
				case TT_VOLUME: m_pTextures[i].pTextureVolume->Release();
				}
				m_pTextures[i].pTexture2D = NULL;
			}
			m_pTextures[i].intRefNum = 0;
			SAFE_DELETE_ARRAY (m_pTextures[i].szTextureName)
		}

	if (m_pUniqueTextures)
		for (DWORD i=0; i<m_dwUniqueTextureCount; i++)
		{
			if (m_pUniqueTextures[i].pTexture2D)
			{
				switch (m_pUniqueTextures[i].type)
				{
				case TT_2D: m_pUniqueTextures[i].pTexture2D->Release(); break;
				case TT_CUBE: m_pUniqueTextures[i].pTextureCube->Release(); break;
				case TT_VOLUME: m_pUniqueTextures[i].pTextureVolume->Release();
				}
				m_pUniqueTextures[i].pTexture2D = NULL;
			}
			m_pUniqueTextures[i].intRefNum = 0;
			SAFE_DELETE_ARRAY (m_pUniqueTextures[i].szTextureName)
		}

	return true;
}

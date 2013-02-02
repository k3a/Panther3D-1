	
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DMaterialManager definition
//-----------------------------------------------------------------------------


#include "material_manager.h"


REGISTER_SINGLE_CLASS (CP3DMaterialManager, IP3DRENDERER_MATERIALMANAGER); // zaregistruj tuto tøídu


// #define DEBUG_LOG_MATERIAL_INFO				// vypise info o nacitanych materialoch do konzole
// #define DEBUG_USE_VS_SHADER_VERSION	(D3DVS_VERSION(2,0))	// debug urcenie najvyssej pouzitej verzie shaderov
// #define DEBUG_USE_PS_SHADER_VERSION	(D3DPS_VERSION(2,0))


unsigned char		*m_pIncludes;
DWORD				m_dwIncludesSize;

// s touto tabulkou upravit vzdy aj enum FXSemantics !
//---------------------------------
const SemanticsInfo g_SemanticsInfoTable [SEM_NUM] = 
{
	SEMTYPE_FLOAT4x4, true, "View",
	SEMTYPE_FLOAT4x4, true, "ViewTranspose",
	SEMTYPE_FLOAT4x4, true, "ViewInverse",
	SEMTYPE_FLOAT4x4, true, "ViewInverseTranspose",
	SEMTYPE_FLOAT4x4, true, "ViewProjection",
	SEMTYPE_FLOAT4x4, true, "ViewProjectionTranspose",
	SEMTYPE_FLOAT4x4, true, "ViewProjectionInverse",
	SEMTYPE_FLOAT4x4, true, "ViewProjectionInverseTranspose",
	SEMTYPE_FLOAT4x4, true, "Projection",
	SEMTYPE_FLOAT4x4, true, "ProjectionInverse",
	SEMTYPE_FLOAT4x4, true, "ProjectionInverseTranspose",
	SEMTYPE_FLOAT3,	true, "CameraPosition",
	SEMTYPE_FLOAT3,	true, "CameraDirection",
	SEMTYPE_FLOAT2,	true,	"ViewportPixelSize",
	//////////////////////////////////////////////////////////////////////////
	SEMTYPE_FLOAT4x4, false, "World",
	SEMTYPE_FLOAT4x4, false, "WorldTranspose",
	SEMTYPE_FLOAT4x4, false, "WorldInverse",
	SEMTYPE_FLOAT4x4, false, "WorldInverseTranspose",
	SEMTYPE_FLOAT4x4, false, "WorldView",
	SEMTYPE_FLOAT4x4, false, "WorldViewTranspose",
	SEMTYPE_FLOAT4x4, false, "WorldViewInverse",
	SEMTYPE_FLOAT4x4, false, "WorldViewInverseTranspose",
	SEMTYPE_FLOAT4x4, false, "WorldViewProjection",
	SEMTYPE_FLOAT4x4, false, "WorldViewProjectionInverse",
	SEMTYPE_FLOAT4x4, false, "WorldViewProjectionInverseTranspose",

	SEMTYPE_UNKNOWN, false, "AnyValue",
	SEMTYPE_FLOAT1, false, "CustomValue1",
	SEMTYPE_FLOAT1, false, "CustomValue2",
	SEMTYPE_FLOAT1, false, "CustomValue3",
	SEMTYPE_FLOAT1, false, "CustomValue4",

	SEMTYPE_INT1, false, "NumBones",
	SEMTYPE_FLOAT4x3, false, "SkinMatrixArray",

	SEMTYPE_TEXTURE_2D, false, "TextureLayer0",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer1",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer2",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer3",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer4",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer5",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer6",
	SEMTYPE_TEXTURE_2D, false, "TextureLayer7",
	SEMTYPE_TEXTURE_CUBE, false, "TextureCubeMap",
	SEMTYPE_TEXTURE_VOLUME, false, "TextureVolumeMap",

	SEMTYPE_FLOAT3x3, false, "LightGridInfo",

	SEMTYPE_HALF4x4, false, "DynLightTransform",
	SEMTYPE_HALF1, false, "DynLightRadius",
	SEMTYPE_HALF3, false, "DynLightColor",
	SEMTYPE_BOOL1, false, "DynLightUseTexture",
	SEMTYPE_INT1, false, "DynLightCount",
	SEMTYPE_TEXTURE_CUBE, false, "DynLightTexture0",
	SEMTYPE_TEXTURE_CUBE, false, "DynLightTexture1",
	SEMTYPE_TEXTURE_CUBE, false, "DynLightTexture2",
	SEMTYPE_TEXTURE_CUBE, false, "DynLightTexture3",

	//////////////////////////////////////////////////////////////////////////
	SEMTYPE_FLOAT1, false, "Time",
	SEMTYPE_FLOAT1, false, "TimeDelta"
};

//trieda cHelpFXInclude
//---------------------------------
HRESULT cHelpFXInclude::Open (D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, \
							  LPCVOID * ppData, UINT * pBytes)
{
	*ppData = NULL; *pBytes = 0;

	if ((m_pIncludes == NULL) || (stricmp (pFileName, "common.fxh") != 0))
		return E_FAIL;

	*ppData = m_pIncludes;
	*pBytes = m_dwIncludesSize;
	return S_OK;
};

//---------------------------------
HRESULT cHelpFXInclude::Close (LPCVOID pData)
{
	return S_OK;
};

// trieda CP3DMaterialManager
//---------------------------------
CP3DMaterialManager::CP3DMaterialManager()
{
	material_table = NULL;
	m_dwMaterialCount = 0;
	effect_table = NULL;
	m_dwEffectCount = 0;
	m_pParameterEffect = NULL;
	m_pEffectPool = NULL;
	m_dwVSVersionHWSupport = 0;
	m_dwPSVersionHWSupport = 0;
	m_dwVSVersionUsed = 0;
	m_dwPSVersionUsed = 0;
	m_dwActiveMaterial = 0xFFFFFFFF;
	m_dwActivePass = 0xFFFFFFFF;
	m_dwNumPasses = 0;
	m_pActiveTechnique = NULL;
	m_dwDefaultLMmatID = 0xFFFFFFFF; m_dwDefaultVLmatID = 0xFFFFFFFF;
	memset (m_ParamHandleTable, 0, sizeof(D3DXHANDLE) * SEM_World);
}

//---------------------------------
CP3DMaterialManager::~CP3DMaterialManager()
{
	UnloadAll (false);
	SAFE_RELEASE (m_pParameterEffect)
	SAFE_RELEASE (m_pEffectPool)
	SAFE_DELETE_ARRAY (effect_table)
	SAFE_DELETE_ARRAY (material_table)
	SAFE_DELETE_ARRAY (m_pIncludes)
}

//---------------------------------
bool CP3DMaterialManager::Init ()
{
	if ((g_pD3DDevice == NULL) && (g_pFS == NULL)) return false;
	
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	if (g_pConsole == NULL) 
	{
		CON(MSG_ERR_FATAL, "MaterialManager: Can't obtain console class!");
		return false;
	}
	
	D3DCAPS9 caps;
	if (FAILED (g_pD3DDevice->GetDeviceCaps (&caps)))
	{
		CON(MSG_ERR_FATAL, "MaterialManager: Can't obtain device capabilities!");
		return false;
	}

	m_dwVSVersionUsed = m_dwVSVersionHWSupport = caps.VertexShaderVersion;
	m_dwPSVersionUsed = m_dwPSVersionHWSupport = caps.PixelShaderVersion;

#ifdef DEBUG_USE_VS_SHADER_VERSION
	m_dwVSVersionUsed = DEBUG_USE_VS_SHADER_VERSION;
#endif
#ifdef DEBUG_USE_PS_SHADER_VERSION
	m_dwPSVersionUsed = DEBUG_USE_PS_SHADER_VERSION;
#endif

	if (!ReallocateMaterialTable()) 
	{
		CON(MSG_ERR_FATAL, "MaterialManager: Can't reallocate material table!");
		return false;
	}
	if (!ReallocateEffectTable()) 
	{
		CON(MSG_ERR_FATAL, "MaterialManager: Can't reallocate effect table!");
		return false;
	}
	if (FAILED (D3DXCreateEffectPool (&m_pEffectPool)))
	{
		CON(MSG_ERR_FATAL, "MaterialManager: Can't create effect pool!");
		return false;
	}
	// nacitanie include suboru common.fxh
	//////////////////////////////////////////////////////////////////////////
	FSFILE	*pFile;
	BYTE	*pData = NULL;
	DWORD	dwFileSize = 0;
	if ((pFile = g_pFS->Load ("shaders\\common.fxh", pData, dwFileSize)) == NULL)
	{
		CON(MSG_ERR_FATAL, "FX include file (shaders/common.fxh) doesn't exist!");
		return false;
	}
	m_dwIncludesSize = dwFileSize;
	m_pIncludes = new unsigned char[m_dwIncludesSize];
	if (m_pIncludes)
		memcpy (m_pIncludes, pData, m_dwIncludesSize);
	g_pFS->UnLoad (pFile, pData);

	// nacitanie globals.fx so zdielanymi premennymi
	//////////////////////////////////////////////////////////////////////////
	pData = NULL; dwFileSize = 0;
	if ((pFile = g_pFS->Load ("shaders/globals.fx", pData, dwFileSize)) == NULL)
	{
		CON(MSG_ERR_FATAL, "Global shader (shaders/globals.fx) doesn't exist!");
		return false;
	}
	LPD3DXBUFFER pErrorBuf=NULL;
	HRESULT hr = D3DXCreateEffect (g_pD3DDevice, pData, dwFileSize, NULL, NULL, \
		D3DXFX_NOT_CLONEABLE, m_pEffectPool, &m_pParameterEffect, &pErrorBuf); 
	g_pFS->UnLoad (pFile, pData);
	if (FAILED (hr))
	{
		if (pErrorBuf)
		{
			char *szError = (char*)pErrorBuf->GetBufferPointer ();
			if (szError)
			{
				char szbuf[512] = {0};
				strncpy (szbuf, szError, 510);
				CON(MSG_ERR_FATAL, "MaterialManager: Can't create/compile globals.fx shader!\n\r\n\r%s", szbuf);
			}
			pErrorBuf->Release ();
		}
		CON(MSG_ERR_FATAL, "MaterialManager: Can't create/compile globals.fx!");
		return false;
	}
	// ziskanie handlov na globalne parametre
	//////////////////////////////////////////////////////////////////////////
	for (DWORD i=0; i<SEM_World; i++)
		m_ParamHandleTable[i] = m_pParameterEffect->GetParameterBySemantic (NULL, g_SemanticsInfoTable[i].szSemanticName);

	// preloading default shaderov
	m_dwDefaultVLmatID = LoadMaterial ("materials\\common\\bspvl_def.7mt");
	m_dwDefaultLMmatID = LoadMaterial ("materials\\common\\bsplm_def.7mt");
	if ((m_dwDefaultLMmatID == 0xFFFFFFFF) || (m_dwDefaultVLmatID == 0xFFFFFFFF))
	{
		CON(MSG_ERR_FATAL, "MaterialManager: Can't load default materials (bsplm_def.7mt, bspvl_def.7mt)!");
		return false;
	}

	return true;
}

// uvolni vsetky materialy
//---------------------------------
void CP3DMaterialManager::UnloadAll (bool bSaveDefaultMaterials)
{
	DWORD i, j;

	// unload efects
	if (effect_table)
		for (i=0; i<m_dwEffectCount; i++)
		{
			if (bSaveDefaultMaterials)
				if ((material_table[m_dwDefaultLMmatID].pEffect == effect_table[i].pEffect) || \
					(material_table[m_dwDefaultVLmatID].pEffect == effect_table[i].pEffect))
					continue;
			SAFE_RELEASE (effect_table[i].pEffect)
			SAFE_DELETE_ARRAY (effect_table[i].szFileName)
			SAFE_DELETE_ARRAY (effect_table[i].szTechniqueName)
		}

	// unload materials
	if (material_table)
	{
		for (i=0; i<m_dwMaterialCount; i++)
		{
			if (bSaveDefaultMaterials)
				if ((i == m_dwDefaultLMmatID) || (i == m_dwDefaultVLmatID))
					continue;
			material_table[i].pEffect = NULL;
			SAFE_DELETE_ARRAY (material_table[i].szFileName)
			if (material_table[i].requiredParameters)
			{
				for (j=0; j<material_table[i].wParamsNum; j++)
					if (material_table[i].requiredParameters[j].pValue)
					{
						FXSemanticsType	semType;
						void					*pVoid;
						pVoid = material_table[i].requiredParameters[j].pValue;
						semType = g_SemanticsInfoTable[material_table[i].requiredParameters[j].SemanticID].SemType;
						if ((semType >= SEMTYPE_TEXTURE_2D) && (semType <= SEMTYPE_TEXTURE_VOLUME))
						{
							// ak ide o texturu tak unload textury
							LPDIRECT3DTEXTURE9	*ppTexture = (LPDIRECT3DTEXTURE9*)pVoid;
							g_TextureLoader.UnloadTexture (*ppTexture);
						}
						delete[] pVoid;
					}
				delete[] material_table[i].requiredParameters;
				material_table[i].requiredParameters = NULL;
			}
			material_table[i].wParamsNum = 0;
		}
	}

	if (!bSaveDefaultMaterials)
	{
		m_dwDefaultLMmatID = 0xFFFFFFFF;
		m_dwDefaultVLmatID = 0xFFFFFFFF;
	}
}

//---------------------------------
bool CP3DMaterialManager::ReallocateMaterialTable()
{
	if (material_table == NULL)
	{
		material_table = new P3DMATERIAL[MATERIAL_TABLE_SIZE];
		if (material_table == NULL) return false;
		m_dwMaterialCount = MATERIAL_TABLE_SIZE;
		return true;
	}
	else
	{
		P3DMATERIAL *pTempTable = new P3DMATERIAL[m_dwMaterialCount + MATERIAL_TABLE_SIZE];
		if (pTempTable == NULL) return false;
		memcpy (pTempTable, material_table, sizeof(P3DMATERIAL) * m_dwMaterialCount);
		delete[] material_table;
		material_table = pTempTable;
		m_dwMaterialCount += MATERIAL_TABLE_SIZE;
		return true;
	}
}

//---------------------------------
bool CP3DMaterialManager::ReallocateEffectTable()
{
	if (effect_table == NULL)
	{
		effect_table = new EFFECT_INFO[MATERIAL_TABLE_SIZE];
		if (effect_table == NULL) return false;
		m_dwEffectCount = MATERIAL_TABLE_SIZE;
		return true;
	}
	else
	{
		EFFECT_INFO *pTempTable = new EFFECT_INFO[m_dwEffectCount + MATERIAL_TABLE_SIZE];
		if (pTempTable == NULL) return false;
		memcpy (pTempTable, effect_table, sizeof(EFFECT_INFO) * m_dwEffectCount);
		delete[] effect_table;
		effect_table = pTempTable;
		m_dwEffectCount += MATERIAL_TABLE_SIZE;
		return true;
	}
}

//---------------------------------
const char* CP3DMaterialManager::SemanticToString (FXSemantics semantic)
{
	if (semantic >= SEM_NUM) return NULL;
	return g_SemanticsInfoTable[semantic].szSemanticName;
}

//---------------------------------
FXSemantics CP3DMaterialManager::StringToSemantic (const char* szSemantic)
{
	if (szSemantic == NULL) return SEM_Unknown;

	for (DWORD i=0; i<SEM_NUM; i++)
		if (stricmp (g_SemanticsInfoTable[i].szSemanticName, szSemantic) == 0)
			return (FXSemantics)i;

	CON(MSG_CON_DEBUG, "CP3DMaterialManager: Unknown semantic '%s'!", szSemantic);
	return SEM_Unknown;
}

//---------------------------------
void* CP3DMaterialManager::GetPoolHandle()
{
	return (void*)m_pEffectPool;
}

//---------------------------------
LPD3DXEFFECT CP3DMaterialManager::LoadEffect (IN const char* szFile, IN const char* szTechnique, OUT D3DXHANDLE &pTechnique, OUT D3DXHANDLE &pTechniqueLighted)
{
	DWORD		i, dwFileSize;
	HRESULT	hr;
	char szTechniqueLighted[128] = {0};

	pTechnique = NULL; pTechniqueLighted = NULL;
	if ((szFile == NULL) || (szTechnique == NULL))
		return NULL;
	strncpy (szTechniqueLighted, szTechnique, sizeof(szTechniqueLighted)-1);
	strcat (szTechniqueLighted, "_lighted");

	// vyhladanie v tabulke uz existujucich efektov
	for (i=0; i<m_dwEffectCount; i++)
		if ((effect_table[i].szFileName != NULL) && (effect_table[i].szTechniqueName != NULL) && (effect_table[i].pEffect != NULL))
			if (stricmp (effect_table[i].szFileName, szFile) == 0)
				if (stricmp (effect_table[i].szTechniqueName, szTechnique) == 0)
				{
					pTechnique = effect_table[i].pEffect->GetTechniqueByName (szTechnique);
					pTechniqueLighted = effect_table[i].pEffect->GetTechniqueByName (szTechniqueLighted);
					if (pTechniqueLighted == NULL) pTechniqueLighted = pTechnique;
					return effect_table[i].pEffect;
				}

	// nenachadza sa v tabulke, efekt je potrebne nahrat
	FSFILE	*pFile;
	BYTE		*pData;
	if ((pFile = g_pFS->Load (szFile, pData, dwFileSize)) == NULL)
		return NULL;

	// flagy pre shader debugging	D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_DEBUG | D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT | D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT
	LPD3DXEFFECT	pEffect = NULL;
	LPD3DXBUFFER pErrorBuf = NULL;
	cHelpFXInclude FXInclude;

	hr = D3DXCreateEffect (g_pD3DDevice, pData, dwFileSize, NULL, &FXInclude, \
								D3DXFX_NOT_CLONEABLE, m_pEffectPool, &pEffect, &pErrorBuf);

	g_pFS->UnLoad (pFile, pData);

	if (FAILED (hr))
	{
		CON(MSG_CON_ERR, "Can't load effect file: %s", szFile);
		if (pErrorBuf)
		{
			char *szError = (char*)pErrorBuf->GetBufferPointer ();
			if (szError)
			{
				char szbuf[512] = {0};
				strncpy (szbuf, szError, 510);
				CON(MSG_CON_ERR, "%s", szbuf);
			}
			pErrorBuf->Release ();
		}
		return NULL;
	}

	// ziskanie pozadovanej technique
	if ((pTechnique = pEffect->GetTechniqueByName (szTechnique)) == NULL)	// dana technique neexistuje
		goto quit_hand;
	if ((pTechniqueLighted = pEffect->GetTechniqueByName (szTechniqueLighted)) == NULL)
		pTechniqueLighted = pTechnique;

jump01:
	// pridanie do cache tabulky
	for (i=0; i<m_dwEffectCount; i++)
		if (effect_table[i].pEffect == NULL)
		{
			effect_table[i].pEffect = pEffect;
			effect_table[i].szFileName = new char [strlen (szFile) + 1];
			strcpy (effect_table[i].szFileName, szFile);
			effect_table[i].szTechniqueName = new char [strlen (szTechnique) + 1];
			strcpy (effect_table[i].szTechniqueName, szTechnique);
#ifdef DEBUG_LOG_MATERIAL_INFO
			CON(MSG_CON_DEBUG, "-- DEBUG -- MaterialManager: shader %s successfully loaded", szFile);
#endif
			return pEffect;
		}
	if (i == m_dwEffectCount)
		if (ReallocateEffectTable ())
			goto jump01;

quit_hand:
	SAFE_RELEASE (pEffect)
	pTechnique = NULL; pTechniqueLighted = NULL;
	return NULL;
}

//---------------------------------
void* CP3DMaterialManager::CreateDefaultParameter (const char *szDefValue, FXSemanticsType semanticType)
{
	char	*pStr;
	WORD	wSize = 0;
	void	*pRet = NULL;
	float	fVector[4] = {0.0f};
	int		intVector[4] = {0};
	char	szTextureFile[MAX_PATH] = {0};

	if ((szDefValue == NULL) || (szDefValue[0] == 0)) return NULL;
	char szBuf[512] = {0};
	strncpy (szBuf, szDefValue, sizeof(szBuf)-1);

	// zatial tu su iba floaty, int, textury a matice, dalsie parametre sa pridaju ak to bude potrebne
	switch (semanticType)
	{
	// float
	case SEMTYPE_FLOAT4:
	case SEMTYPE_HALF4:
		wSize = 4;
		if ((pStr = strrchr (szBuf, ',')) == NULL) break;
		fVector[3] = (float)atof (pStr+1);
		*pStr = 0;
	case SEMTYPE_FLOAT3:
	case SEMTYPE_HALF3:
		if (!wSize) wSize = 3;
		if ((pStr = strrchr (szBuf, ',')) == NULL) break;
		fVector[2] = (float)atof (pStr+1);
		*pStr = 0;
	case SEMTYPE_FLOAT2:
	case SEMTYPE_HALF2:
		if (!wSize) wSize = 2;
		if ((pStr = strrchr (szBuf, ',')) == NULL) break;
		fVector[1] = (float)atof (pStr+1);
		*pStr = 0;
	case SEMTYPE_FLOAT1:
	case SEMTYPE_HALF1:
		if (!wSize) wSize = 1;
		fVector[0] = (float)atof (szBuf);

		if (pRet = new float[wSize])
			memcpy (pRet, fVector, sizeof(float) * wSize);
		return pRet;
	//////////////////////////////////////////////////////////////////////////
	// int
	case SEMTYPE_INT4:
		wSize = 4;
		if ((pStr = strrchr (szBuf, ',')) == NULL) break;
		intVector[3] = atoi (pStr+1);
		*pStr = 0;
	case SEMTYPE_INT3:
		if (!wSize) wSize = 3;
		if ((pStr = strrchr (szBuf, ',')) == NULL) break;
		intVector[2] = atoi (pStr+1);
		*pStr = 0;
	case SEMTYPE_INT2:
		if (!wSize) wSize = 2;
		if ((pStr = strrchr (szBuf, ',')) == NULL) break;
		intVector[1] = atoi (pStr+1);
		*pStr = 0;
	case SEMTYPE_INT1:
		if (!wSize) wSize = 1;
		intVector[0] = atoi (szDefValue);

		if (pRet = new int[wSize])
			memcpy (pRet, intVector, sizeof(int) * wSize);
		return pRet;
	//////////////////////////////////////////////////////////////////////////
	// matice
	case SEMTYPE_FLOAT4x4:
	case SEMTYPE_HALF4x4:
		wSize = 16;
	case SEMTYPE_FLOAT3x4:
	case SEMTYPE_HALF3x4:
	case SEMTYPE_FLOAT4x3:
	case SEMTYPE_HALF4x3:
		if (!wSize) wSize = 12;
	case SEMTYPE_FLOAT3x3:
	case SEMTYPE_HALF3x3:
		if (!wSize) wSize = 9;

		float *pMatrix;
		if (pMatrix = new float[wSize])
		{
			memset (pMatrix, 0, sizeof(float)*wSize);
			char *pStr = szBuf;
			for (DWORD i=0; (i<wSize) && (*pStr!=0); i++)
			{
				pMatrix[i] = (float)atof (pStr);
				for (; *pStr!=0; pStr++)
					if (*pStr==',')
						{pStr++; break;}
			}
		}
		return pMatrix;
	//////////////////////////////////////////////////////////////////////////
	// textures
	case SEMTYPE_TEXTURE_2D:
		LPDIRECT3DTEXTURE9 *ppTexture;
		if (ppTexture = new LPDIRECT3DTEXTURE9[1])
		{
			strcat (szTextureFile, szDefValue);
			if (szTextureFile[strlen(szTextureFile)-4]!='.') strcat (szTextureFile, ".dds"); //def. pripona
			*ppTexture = g_TextureLoader.CreateTexture (szTextureFile);
			if (*ppTexture == NULL)
				CON(MSG_CON_DEBUG, "Material manager can't load default parameter: %s", szTextureFile);
			pRet = ppTexture;
		}
		return pRet;
	case SEMTYPE_TEXTURE_CUBE:
		LPDIRECT3DCUBETEXTURE9 *ppCubeTexture;
		if (ppCubeTexture = new LPDIRECT3DCUBETEXTURE9[1])
		{
			strcat (szTextureFile, szDefValue);
			if (szTextureFile[strlen(szTextureFile)-4]!='.') strcat (szTextureFile, ".dds"); //def. pripona
			*ppCubeTexture = g_TextureLoader.CreateCubeTexture (szTextureFile);
			if (*ppCubeTexture == NULL)
				CON(MSG_CON_DEBUG, "Material manager can't load default parameter: %s", szTextureFile);
			pRet = ppCubeTexture;
		}
		return pRet;
	case SEMTYPE_TEXTURE_VOLUME:
		LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture;
		if (ppVolumeTexture = new LPDIRECT3DVOLUMETEXTURE9[1])
		{
			strcat (szTextureFile, szDefValue);
			if (szTextureFile[strlen(szTextureFile)-4]!='.') strcat (szTextureFile, ".dds"); //def. pripona
			*ppVolumeTexture = g_TextureLoader.CreateVolumeTexture (szTextureFile);
			if (*ppVolumeTexture == NULL)
				CON(MSG_CON_DEBUG, "Material manager can't load default parameter: %s", szTextureFile);
			pRet = ppVolumeTexture;
		}
		return pRet;
	}

	return NULL;
}

// v pripade chyby vrati 0xFFFFFFFF
//---------------------------------
DWORD CP3DMaterialManager::LoadMaterial (const char *szFileName)
{
	DWORD i, j, k, l;
	char	*pFileBuf;
	LPD3DXEFFECT	pEffect = NULL;

	if ((szFileName == NULL) || (szFileName[0] == NULL))
		return 0xFFFFFFFF;
	// vyhladanie v tabulke uz existujucich materialov
	for (i=0; i<m_dwMaterialCount; i++)
		if ((material_table[i].pEffect != NULL) && (material_table[i].szFileName != NULL))
			if (stricmp (material_table[i].szFileName, szFileName) == 0)
				return i;

	FSFILE	*pFile;
	DWORD	dwFileSize = 0;
	BYTE	*pData = NULL;
	if (((pFile = g_pFS->Load (szFileName, pData, dwFileSize, true)) == NULL) || (pData == NULL))
	{
		CON(MSG_CON_ERR, "MaterialManager: 7mt material file doesn't exist %s!", szFileName);
		return 0xFFFFFFFF;
	}
	pFileBuf = (char*)pData;

	//////////////////////////////////////////////////////////////////////////
	// parsing 7mt suboru

	char *strStart, *strStop, *strBuf, *strBestShader = NULL;
	DWORD LastGoodVSversion = 0, LastGoodPSversion = 0;
	// vyhladanie najlepsieho shaderu podla podporovanej HW verzie
	for (strStart = pFileBuf; ; strStart = strStop)
	{
		if ((strStart = strchr (strStart, '<')) == NULL)
			break;
		if ((strStop = strchr (strStart, '>')) == NULL)
			break;
		strBuf = strstr (strStart, "\r\n[");
		if ((strBuf == NULL) || (strBuf > strStop))
			{strStart = strStop; continue;}

		DWORD VSversion, PSversion;
		int VSmajor, VSminor, PSmajor, PSminor;
		if ((VSmajor = atoi(strBuf+3)) == 0)
			{strStart = strStop; continue;}
		if ((strBuf = strchr (strBuf, '.')) == NULL)
			{strStart = strStop; continue;}
		VSminor = atoi(++strBuf);
		VSversion = D3DVS_VERSION (VSmajor, VSminor);

		if ((strBuf = strchr (strBuf, ',')) == NULL)
			{strStart = strStop; continue;}
		if ((PSmajor = atoi(++strBuf)) == 0)
			{strStart = strStop; continue;}
		if ((strBuf = strchr (strBuf, '.')) == NULL)
			{strStart = strStop; continue;}
		PSminor = atoi(++strBuf);
		PSversion = D3DPS_VERSION (PSmajor, PSminor);

		if ((VSversion <= m_dwVSVersionUsed) && (PSversion <= m_dwPSVersionUsed))
			if ((VSversion >= LastGoodVSversion) && (PSversion >= LastGoodPSversion))
			{
				LastGoodVSversion = VSversion; LastGoodPSversion = PSversion;
				strBestShader = strStart;
			}
	}

	if (strBestShader == NULL)		// nespravny format 7mt suboru
	{
		CON(MSG_CON_ERR, "MaterialManager: Incorrect material definition %s!", szFileName);
		goto quit_hand;
	}

	//////////////////////////////////////////////////////////////////////////
	// parsing vybraneho 7mt materialu

	char szEffectFile[MAX_PATH] = "shaders/";
	char szTechniqueName[128] = {0};

	// shader name
	strStart = strBestShader;
	if ((strStop = strstr (++strStart, "\r\n")) == NULL)
		goto quit_hand;
	strBuf = strchr (strStart, '=');
	if ((strBuf == NULL) || (strStop - strBuf <= 1))
	{
		CON(MSG_CON_ERR, "MaterialManager: Material %s without technique name?", szFileName);
		goto quit_hand;
	}
	*strBuf = 0;
	strcat (szEffectFile, strStart);
	strcat (szEffectFile, ".fx");
	if (!g_pFS->Exists (szEffectFile))
	{
		CON(MSG_CON_ERR, "MaterialManager: Effect file %s doesn't exist!", szEffectFile );
		goto quit_hand;		// zatial sa nenacita
	}
	// shader technique
	*strStop = 0;
	strncpy (szTechniqueName, ++strBuf, sizeof(szTechniqueName)-1);

	// parsing parametrov
	bool bParsingSuccess = false;
	WORD wNumParameters = 0;
	MATERIAL_PARAMETER	param_table[SEM_NUM];

	if ((strStop = strstr (++strStop, "\r\n")) == NULL)
		goto quit_hand;
	strStart = strStop + 2;
	while ((unsigned long)(strStart - pFileBuf) < dwFileSize)
	{
		char *strDefParam = NULL;
		if (*strStart == '>')
			{bParsingSuccess = true; break;}	// parsing ukonceny
		if ((strStop = strstr (strStart, "\r\n")) == NULL)
			break;
		*strStop = 0;
		if (strDefParam = strchr (strStart, '='))		// urcenie default. parametrov
			{*strDefParam = 0; strDefParam++;}
		param_table[wNumParameters].SemanticID = StringToSemantic (strStart);
		if (strDefParam)
			strncpy (param_table[wNumParameters].szTextValue, strDefParam, sizeof(param_table[wNumParameters].szTextValue)-1);
		wNumParameters++;

		strStart = strStop + 2;
	}

	if (!bParsingSuccess) goto quit_hand;

	// kontrola na zhodu s uz existujucimi materialmi
	for (i=0; i<m_dwMaterialCount; i++)
		if (material_table[i].pEffect != NULL)
		{
			for (j=0; j<m_dwEffectCount; j++)
				if (effect_table[j].pEffect == material_table[i].pEffect)
				{
					if (stricmp (effect_table[j].szFileName, szEffectFile) == 0)
						if (stricmp (effect_table[j].szTechniqueName, szTechniqueName) == 0)
							if (material_table[i].wParamsNum == wNumParameters)
							{
								bool bParamFounded = false;
								for (k=0; k<wNumParameters; k++, bParamFounded = false)
								{
									for (l=0; l<wNumParameters; l++)
										if (material_table[i].requiredParameters[k].SemanticID == param_table[l].SemanticID)
										{
											if ( 0 == strnicmp (material_table[i].requiredParameters[k].szTextValue, param_table[l].szTextValue, sizeof(param_table[l].szTextValue)))
												bParamFounded = true;
											break;
										}
									if (bParamFounded == false)
										goto jump01;
								}
								return i;	// dany material je uz vytvoreny
							}
					break;
				}
		}

jump01:

	// nacitanie efektu
	D3DXHANDLE pTechnique, pTechniqueLighted;
	pEffect = LoadEffect (szEffectFile, szTechniqueName, pTechnique, pTechniqueLighted);
	if (pEffect == NULL) goto quit_hand;

	// ulozenie materialu do cache
jump02:
	for (i=0; i<m_dwMaterialCount; i++)
		if (material_table[i].pEffect == NULL)
		{
			material_table[i].pEffect = pEffect;
			material_table[i].pTechnique = pTechnique;
			material_table[i].pTechniqueLighted = pTechniqueLighted;

			D3DXHANDLE pNonLitTechnique = pEffect->GetTechnique (0);
			D3DXTECHNIQUE_DESC	TechniqueDesc;
			pEffect->GetTechniqueDesc (pNonLitTechnique, &TechniqueDesc);
			material_table[i].dwBasicPassNum = TechniqueDesc.Passes;

			material_table[i].szFileName = new char[strlen (szFileName) + 1];
			strcpy (material_table[i].szFileName, szFileName);

			// parsing parametrov
			if (wNumParameters == 0)
				material_table[i].requiredParameters = NULL;
			else
			{
				D3DXHANDLE paramHandle;
				material_table[i].requiredParameters = new MATERIAL_PARAMETER[wNumParameters];
				if (material_table[i].requiredParameters)
				{
					for (j=0; j<wNumParameters; j++)
						if (param_table[j].SemanticID < SEM_NUM)
							if (paramHandle = pEffect->GetParameterBySemantic (NULL, SemanticToString (param_table[j].SemanticID)))
							{
								material_table[i].requiredParameters[j].paramHandle = (void*)paramHandle;
								material_table[i].requiredParameters[j].SemanticID = param_table[j].SemanticID;
								if (param_table[j].szTextValue[0] != 0)		// parsing def. parametru
								{
									strncpy (material_table[i].requiredParameters[j].szTextValue, param_table[j].szTextValue, sizeof(param_table[j].szTextValue)-1);
									material_table[i].requiredParameters[j].pValue = \
										CreateDefaultParameter (param_table[j].szTextValue, g_SemanticsInfoTable[param_table[j].SemanticID].SemType);
								}
							}
					material_table[i].wParamsNum = wNumParameters;
					// zoradenie tabulky
					SortMaterialParameters (material_table[i].requiredParameters, material_table[i].wParamsNum, material_table[i].wSpecificParamsNum);
				}
			}

			// ziskanie dynamic light parametrov
			if (pTechniqueLighted != pTechnique)
				for (j=0; j<DYNLIGHT_SEMANTICS; j++)
					material_table[i].dynlightParameters[j].paramHandle = (void*)pEffect->GetParameterBySemantic (NULL, SemanticToString (material_table[i].dynlightParameters[j].SemanticID));

			g_pFS->UnLoad (pFile, (BYTE*)pFileBuf);

#ifdef DEBUG_LOG_MATERIAL_INFO
			CON(MSG_CON_DEBUG, "-- DEBUG -- MaterialManager: material %s successfully loaded", szFileName);
#endif
			return i;
		}
	if (i == m_dwMaterialCount)
		if (ReallocateMaterialTable ())
			goto jump02;

quit_hand:
	g_pFS->UnLoad (pFile, (BYTE*)pFileBuf);
	SAFE_RELEASE (pEffect)
	CON(MSG_CON_ERR, "MaterialManager: Unhandled exception while loading material %s!", szFileName);
	return 0xFFFFFFFF;
}

//---------------------------------
void CP3DMaterialManager::SortMaterialParameters (MATERIAL_PARAMETER *pParameters, WORD &wParamNum, WORD &wSpecificParamNum)
{
	int i, j;
	MATERIAL_PARAMETER matParamBuf;
	FXSemantics semantic;
	bool bContinue, bSort;
	int intLoop = wParamNum-1;

	// tabulka by mala mat taketo poradie prvkov:
	// Adjustable
	// Predefined (default hodnoty definovane v 7mt)
	// General (Time)
	// Fault (SEM_Unknown)

	for (i=0; i<3; i++)
	{
		do 
		{
			bContinue = false;
			for (j=intLoop; j>=0; j--)
			{
				semantic = pParameters[j].SemanticID;
				switch (i)
				{
				case 0: bSort = (semantic == SEM_Unknown); break;			// "vylucenie" chybovych parametrov z tabulky
				case 1: bSort = (GENERAL_SEMANTICS (semantic)); break;	// general parametre na koniec
				case 2: bSort = (pParameters[j].pValue != NULL);			// preddefinovane parametre na koniec (pred general)
				}

				if (bSort)
				{
					matParamBuf = pParameters[j];
					pParameters[j] = pParameters[intLoop];
					pParameters[intLoop] = matParamBuf;
					intLoop--;
					bContinue = true;
					break;
				}
			}
		} while(bContinue);
	}

	intLoop = wParamNum;
	wParamNum = 0;
	wSpecificParamNum = 0;

	for (i=0; i<intLoop; i++)
	{
		if (pParameters[i].SemanticID == SEM_Unknown)
			break;

		if (GENERAL_SEMANTICS(pParameters[i].SemanticID) || (pParameters[i].pValue != NULL))
			if (wSpecificParamNum == 0)
				wSpecificParamNum = i;
	}
	wParamNum = i;
	if (wSpecificParamNum == 0)
		wSpecificParamNum = wParamNum;
}

//---------------------------------
void CP3DMaterialManager::QueryParameters (IN DWORD matID, IN bool bQueryDynlightParams, \
														OUT MATERIAL_PARAMETER pParamTable[SEM_NUM], \
														OUT WORD &wAllParamsNum, OUT WORD &wParamsToSetNum)
{
	wAllParamsNum = material_table[matID].wParamsNum;
	wParamsToSetNum = material_table[matID].wSpecificParamsNum;

	if ((!bQueryDynlightParams) || (material_table[matID].dynlightParameters[0].paramHandle == NULL))
	{
		if (wAllParamsNum)
			memcpy (pParamTable, material_table[matID].requiredParameters, sizeof(MATERIAL_PARAMETER) * wAllParamsNum);
	}
	else
	{
		if (wParamsToSetNum)
			memcpy (pParamTable, material_table[matID].requiredParameters, sizeof(MATERIAL_PARAMETER) * wParamsToSetNum);
		memcpy (&pParamTable[wParamsToSetNum], material_table[matID].dynlightParameters, sizeof(MATERIAL_PARAMETER) * DYNLIGHT_SEMANTICS);
		wAllParamsNum += DYNLIGHT_SEMANTICS;
		wParamsToSetNum += DYNLIGHT_SEMANTICS;
		if (wAllParamsNum > wParamsToSetNum)
			memcpy (&pParamTable[wParamsToSetNum], \
					&material_table[matID].requiredParameters[wParamsToSetNum-DYNLIGHT_SEMANTICS], \
					sizeof(MATERIAL_PARAMETER) * (wAllParamsNum - wParamsToSetNum));
	}
}

//---------------------------------
bool CP3DMaterialManager::SetParameters (DWORD matID, const MATERIAL_PARAMETER pParamTable[SEM_NUM], WORD wNumParameters)
{
	Prof(RENDERER_CP3DMaterialManager__SetParameters);
	LPD3DXEFFECT pEffect = material_table[matID].pEffect;
	if (pEffect == NULL) return false;

	for (register DWORD i=0; i<wNumParameters; i++)
		if (pParamTable[i].paramHandle)
		{
			if (pParamTable[i].pValue)
			{
				STAT(STAT_SHADER_SET_VALUE, 1);
				pEffect->SetValue ((D3DXHANDLE)pParamTable[i].paramHandle, pParamTable[i].pValue, D3DX_DEFAULT);
			}
			// nastavenie specifickych parametrov
			else
			{
				float fValue;
				switch (pParamTable[i].SemanticID)
				{
				case SEM_Time:								// cas
					fValue = (g_pTimer->GetTickCount_s ());
					STAT(STAT_SHADER_SET_VALUE, 1);
					pEffect->SetValue ((D3DXHANDLE)pParamTable[i].paramHandle, &fValue, D3DX_DEFAULT);
					break;
				case SEM_TimeDelta:							// cas framu
					fValue = ((float)g_pTimer->GetTimeDelta_s ());
					STAT(STAT_SHADER_SET_VALUE, 1);
					pEffect->SetValue ((D3DXHANDLE)pParamTable[i].paramHandle, &fValue, D3DX_DEFAULT);
					break;
				}
			}
		}

	if (m_dwActiveMaterial == matID)
		pEffect->CommitChanges ();
	return true;
}

//---------------------------------
bool CP3DMaterialManager::SetMatrixArray (DWORD matID, void* paramHandle, const P3DMatrix *matrixArray, UINT dwMatrixCount)
{
	LPD3DXEFFECT pEffect;
	if ((paramHandle == NULL) || (matrixArray == NULL) || (dwMatrixCount == 0))
		return false;
	if ((pEffect = material_table[matID].pEffect) == NULL)
		return false;

	HRESULT hr = pEffect->SetMatrixArray ((D3DXHANDLE)paramHandle, (D3DXMATRIX*)matrixArray, dwMatrixCount);
	if (m_dwActiveMaterial == matID)
		pEffect->CommitChanges ();
	return SUCCEEDED (hr);
}

//---------------------------------
DWORD CP3DMaterialManager::GetLMmaterialID()
{
	return m_dwDefaultLMmatID;
}

//---------------------------------
void CP3DMaterialManager::Establish_LM_Material (void* diffTexture, void* lmTexture)
{
	Prof(RENDERER_CP3DMaterialManager__Establish_LM_Material);
	LPD3DXEFFECT pEffect = material_table[m_dwDefaultLMmatID].pEffect;

	// nastavenie parametrov
	if (diffTexture)
		pEffect->SetTexture ((D3DXHANDLE)material_table[m_dwDefaultLMmatID].requiredParameters[0].paramHandle, \
								(LPDIRECT3DBASETEXTURE9) diffTexture);
	if (lmTexture)
		pEffect->SetTexture ((D3DXHANDLE)material_table[m_dwDefaultLMmatID].requiredParameters[1].paramHandle, \
								(LPDIRECT3DBASETEXTURE9) lmTexture);

	// nastavenie shaderu
	//////////////////////////////////////////////////////////////////////////
	if (m_dwActiveMaterial == m_dwDefaultLMmatID)	// pozadovany material je prave nastaveny
	{
		if (diffTexture || lmTexture)
			pEffect->CommitChanges ();
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	else if (m_dwActiveMaterial == 0xFFFFFFFF)		// nie je nastaveny ziaden material
	{
		pEffect->Begin ((UINT*)&m_dwNumPasses, D3DXFX_DONOTSAVESTATE);
		STAT(STAT_SHADER_BEGIN_PASS, 1);
		pEffect->BeginPass (0);
		m_dwActivePass = 0;
		m_dwActiveMaterial = m_dwDefaultLMmatID;
	}
	//////////////////////////////////////////////////////////////////////////
	else											// zmena materialu na pozadovany
	{
		material_table[m_dwActiveMaterial].pEffect->EndPass ();
		material_table[m_dwActiveMaterial].pEffect->End ();
		pEffect->Begin ((UINT*)&m_dwNumPasses, D3DXFX_DONOTSAVESTATE);
		STAT(STAT_SHADER_BEGIN_PASS, 1);
		pEffect->BeginPass (0);
		m_dwActivePass = 0;
		m_dwActiveMaterial = m_dwDefaultLMmatID;
	}
}

//---------------------------------
void CP3DMaterialManager::GetShaderVersion (OUT DWORD &vertexVersion, OUT DWORD &pixelVersion)
{
	vertexVersion = m_dwVSVersionUsed;
	pixelVersion = m_dwPSVersionUsed;
}

//---------------------------------
void CP3DMaterialManager::SetMaxShaderVersion (DWORD VSmajor, DWORD VSminor, DWORD PSmajor, DWORD PSminor)
{
	m_dwVSVersionUsed = D3DVS_VERSION(VSmajor, VSminor);
	m_dwPSVersionUsed = D3DPS_VERSION(PSmajor, PSminor);
}

//---------------------------------
DWORD CP3DMaterialManager::GetVertexLightMaterialID()
{
	return m_dwDefaultVLmatID;
}

//---------------------------------
void CP3DMaterialManager::Establish_VertexLight_Material (void* diffTexture)
{
	Prof(RENDERER_CP3DMaterialManager__Establish_VLMaterial);
	LPD3DXEFFECT pEffect = material_table[m_dwDefaultVLmatID].pEffect;

	// nastavenie parametrov
	if (diffTexture)
		pEffect->SetTexture ((D3DXHANDLE)material_table[m_dwDefaultVLmatID].requiredParameters[0].paramHandle, \
								(LPDIRECT3DBASETEXTURE9) diffTexture);

	// nastavenie shaderu
	//////////////////////////////////////////////////////////////////////////
	if (m_dwActiveMaterial == m_dwDefaultVLmatID)	// pozadovany material je prave nastaveny
	{
		if (diffTexture)
			pEffect->CommitChanges ();
		return;
	}
	//////////////////////////////////////////////////////////////////////////
	else if (m_dwActiveMaterial == 0xFFFFFFFF)		// nie je nastaveny ziaden material
	{
		pEffect->Begin ((UINT*)&m_dwNumPasses, D3DXFX_DONOTSAVESTATE);
		STAT(STAT_SHADER_BEGIN_PASS, 1);
		pEffect->BeginPass (0);
		m_dwActivePass = 0;
		m_dwActiveMaterial = m_dwDefaultVLmatID;
	}
	//////////////////////////////////////////////////////////////////////////
	else											// zmena materialu na pozadovany
	{
		material_table[m_dwActiveMaterial].pEffect->EndPass ();
		material_table[m_dwActiveMaterial].pEffect->End ();
		pEffect->Begin ((UINT*)&m_dwNumPasses, D3DXFX_DONOTSAVESTATE);
		STAT(STAT_SHADER_BEGIN_PASS, 1);
		pEffect->BeginPass (0);
		m_dwActivePass = 0;
		m_dwActiveMaterial = m_dwDefaultVLmatID;
	}
}

//---------------------------------
bool CP3DMaterialManager::SetMaterial (DWORD matID, DWORD dwDynLightsCount)
{
	Prof(RENDERER_CP3DMaterialManager__SetMaterial);
	LPD3DXEFFECT pEffect = material_table[matID].pEffect;

	//////////////////////////////////////////////////////////////////////////
	if (m_dwActiveMaterial == matID)		// pozadovany material je prave nastaveny
	{
		if (dwDynLightsCount == 0)		// static lighting
		{
			if (m_pActiveTechnique != material_table[matID].pTechnique)
			{
				pEffect->EndPass ();
				pEffect->End ();
				m_pActiveTechnique = material_table[matID].pTechnique;
				STAT(STAT_SHADER_SET_TECHNIQUE, 1);
				pEffect->SetTechnique (m_pActiveTechnique);
				pEffect->Begin ((UINT*)&m_dwNumPasses, 0);
				STAT(STAT_SHADER_BEGIN_PASS, 1);
				pEffect->BeginPass (0);
				m_dwActivePass = 0;
			}
		}
		else									// dyn. lighting
		{
			if (m_pActiveTechnique != material_table[matID].pTechniqueLighted)
			{
				pEffect->EndPass ();		// mierne chaoticke ???
				pEffect->End ();
				m_pActiveTechnique = material_table[matID].pTechniqueLighted;
				STAT(STAT_SHADER_SET_TECHNIQUE, 1);
				pEffect->SetTechnique (m_pActiveTechnique);
				pEffect->Begin ((UINT*)&m_dwNumPasses, 0);
				// jeden pass pre klasicke vykreslenie, vsetky ostatne pre dyn. svetla
				m_dwNumPasses = min (m_dwNumPasses, dwDynLightsCount + material_table[matID].dwBasicPassNum);
				STAT(STAT_SHADER_BEGIN_PASS, 1);
				STAT(STAT_SHADER_DYNLIGHT_PASS, m_dwNumPasses-1);
				pEffect->BeginPass (0);
				m_dwActivePass = 0;
			}
		}
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	else if (m_dwActiveMaterial != 0xFFFFFFFF)		// zmena materialu na pozadovany
	{
		material_table[m_dwActiveMaterial].pEffect->EndPass ();
		material_table[m_dwActiveMaterial].pEffect->End ();
	}

	if (dwDynLightsCount == 0)
		m_pActiveTechnique = material_table[matID].pTechnique;
	else
		m_pActiveTechnique = material_table[matID].pTechniqueLighted;
	if (m_pActiveTechnique != pEffect->GetCurrentTechnique ())
	{
		STAT(STAT_SHADER_SET_TECHNIQUE, 1);
		pEffect->SetTechnique (m_pActiveTechnique);
	}

	pEffect->Begin ((UINT*)&m_dwNumPasses, 0);
	if (dwDynLightsCount)
	{
		m_dwNumPasses = min (m_dwNumPasses, dwDynLightsCount + material_table[matID].dwBasicPassNum);
		STAT(STAT_SHADER_DYNLIGHT_PASS, m_dwNumPasses-1);
	}
	STAT(STAT_SHADER_BEGIN_PASS, 1);
	pEffect->BeginPass (0);
	m_dwActivePass = 0;
	m_dwActiveMaterial = matID;

	return true;
}

//---------------------------------
bool CP3DMaterialManager::NextPass (DWORD matID)
{
	if (m_dwNumPasses == 1)
		return false;

	LPD3DXEFFECT pEffect = material_table[matID].pEffect;
	if (pEffect == NULL) return false;

	if (++m_dwActivePass >= m_dwNumPasses)				// posledny pass
	{
		pEffect->EndPass ();
		pEffect->End ();
		m_dwActiveMaterial = 0xFFFFFFFF;
		m_dwActivePass = 0xFFFFFFFF;
		m_pActiveTechnique = NULL;
		m_dwNumPasses = 0;
		return false;
	}
	pEffect->EndPass ();
	STAT(STAT_SHADER_BEGIN_PASS, 1);
	pEffect->BeginPass (m_dwActivePass);
	return true;
}

//---------------------------------
void CP3DMaterialManager::TurnOffShaders ()
{
	Prof(RENDERER_CP3DMaterialManager__TurnOffShaders);
	if (m_dwActiveMaterial != 0xFFFFFFFF)
	{
		material_table[m_dwActiveMaterial].pEffect->EndPass ();
		material_table[m_dwActiveMaterial].pEffect->End ();
		m_dwActivePass = 0xFFFFFFFF;
		m_dwActiveMaterial = 0xFFFFFFFF;
		m_pActiveTechnique = NULL;
		m_dwNumPasses = 0;
	}
	// OPTIM: TODO: dost narocne - da sa optimalizovat ???
	g_pD3DDevice->SetVertexShader (NULL);
	g_pD3DDevice->SetPixelShader (NULL);
	// OPTIM
}

//---------------------------------
void CP3DMaterialManager::OnViewMatrixChange (P3DXVector3D *camLookAt)
{
	D3DXMATRIX	matTemp;
	if (m_pParameterEffect == NULL) return;
	matTemp = g_matView;

	// TODO: vsetky tieto matice asi nebude treba

	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_View], &matTemp);

	// pouzit SetMatrixTranspose() ???
	D3DXMatrixTranspose (&matTemp, &matTemp);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewTranspose], &matTemp);

	D3DXMatrixInverse (&matTemp, NULL, &g_matView);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewInverse], &matTemp);

	D3DXMatrixTranspose (&matTemp, &matTemp);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewInverseTranspose], &matTemp);

	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjection], &g_matViewProj);

	D3DXMatrixTranspose (&matTemp, &g_matViewProj);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjectionTranspose], &matTemp);

	D3DXMatrixInverse (&matTemp, NULL, &g_matViewProj);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjectionInverse], &matTemp);

	D3DXMatrixTranspose (&matTemp, &matTemp);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjectionInverseTranspose], &matTemp);

	m_pParameterEffect->SetVector (m_ParamHandleTable[SEM_CameraPosition], (D3DXVECTOR4*)&g_cameraPosition);
	if (camLookAt)
	{
		P3DXVector3D vecTemp(*camLookAt);
		vecTemp -= g_cameraPosition;
		vecTemp.Normalize ();
		m_pParameterEffect->SetVector (m_ParamHandleTable[SEM_CameraDirection], (D3DXVECTOR4*)&vecTemp);
	}

	if (m_dwActiveMaterial != 0xFFFFFFFF)
		m_pParameterEffect->CommitChanges ();
}

//---------------------------------
void CP3DMaterialManager::OnProjectionMatrixChange ()
{
	D3DXMATRIX	matTemp;
	if (m_pParameterEffect == NULL) return;

	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjection], &g_matViewProj);

	D3DXMatrixTranspose (&matTemp, &g_matViewProj);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjectionTranspose], &matTemp);

	D3DXMatrixInverse (&matTemp, NULL, &g_matViewProj);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjectionInverse], &matTemp);

	D3DXMatrixTranspose (&matTemp, &matTemp);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ViewProjectionInverseTranspose], &matTemp);

	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_Projection], &g_matProj);

	D3DXMatrixInverse (&matTemp, NULL, &g_matProj);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ProjectionInverse], &matTemp);

	D3DXMatrixTranspose (&matTemp, &matTemp);
	m_pParameterEffect->SetMatrix (m_ParamHandleTable[SEM_ProjectionInverseTranspose], &matTemp);

	if (m_dwActiveMaterial != 0xFFFFFFFF)
		m_pParameterEffect->CommitChanges ();
}

//---------------------------------
void CP3DMaterialManager::OnViewportSizeChange (float ViewportSize[2])
{
	if (m_pParameterEffect == NULL) return;
	m_pParameterEffect->SetVector (m_ParamHandleTable[SEM_ViewportPixelSize], (D3DXVECTOR4*)ViewportSize);
	if (m_dwActiveMaterial != 0xFFFFFFFF)
		m_pParameterEffect->CommitChanges ();
}

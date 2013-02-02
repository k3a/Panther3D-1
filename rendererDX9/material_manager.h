
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DMaterialManager declaration
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "..\shared\helpers.h"

//---------------------------------
struct SemanticsInfo{
	FXSemanticsType	SemType;
	bool			InThePool;		// true ak je parameter zdielany vsetkymi efektmi
	const char*		szSemanticName;
};

//---------------------------------
class cHelpFXInclude : public ID3DXInclude
{
	STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
	STDMETHOD(Close)(LPCVOID pData);
};

//---------------------------------
struct P3DMATERIAL{
	LPD3DXEFFECT		pEffect;
	D3DXHANDLE			pTechnique;
	D3DXHANDLE			pTechniqueLighted;	// dynamic light technique
	DWORD				dwBasicPassNum;		// pocet pass-ov v technique, ktora nepouziva dynamicke svetla
	char				*szFileName;
	MATERIAL_PARAMETER	dynlightParameters[DYNLIGHT_SEMANTICS];	// tabulka parametrov dynamickeho osvetlenia
	MATERIAL_PARAMETER	*requiredParameters;		// tabulka povinnych vstupnych parametrov
	WORD				wParamsNum;					// pocet (vsetkych) poloziek v requiredParameters
	WORD				wSpecificParamsNum;			// pocet poloziek, ktore musi program nastavit
	P3DMATERIAL() : pEffect(NULL), pTechnique(NULL), pTechniqueLighted(NULL), dwBasicPassNum(1), \
					szFileName(NULL), requiredParameters(NULL), wParamsNum(0), wSpecificParamsNum(0)
	{
		dynlightParameters[0].SemanticID = SEM_DynLightTransform;
		dynlightParameters[1].SemanticID = SEM_DynLightRadius;
		dynlightParameters[2].SemanticID = SEM_DynLightColor;
		dynlightParameters[3].SemanticID = SEM_DynLightUseTexture;
		dynlightParameters[4].SemanticID = SEM_DynLightCount;
		dynlightParameters[5].SemanticID = SEM_DynLightTexture0;
		dynlightParameters[6].SemanticID = SEM_DynLightTexture1;
		dynlightParameters[7].SemanticID = SEM_DynLightTexture2;
		dynlightParameters[8].SemanticID = SEM_DynLightTexture3;
	}
};

//---------------------------------
struct EFFECT_INFO{
	LPD3DXEFFECT	pEffect;
	char			*szFileName;
	char			*szTechniqueName;
	EFFECT_INFO() : pEffect(NULL), szFileName(NULL), szTechniqueName(NULL) {}
};


#define	MATERIAL_TABLE_SIZE	50		// v pripade potreby automaticky rastie

//---------------------------------
class CP3DMaterialManager : public IP3DMaterialManager
{
private:
	P3DMATERIAL			*material_table;		// tabulka vytvorenych 7mt materialov
	DWORD				m_dwMaterialCount;
	EFFECT_INFO			*effect_table;			// tabulka vytvorenych .fx shaderov
	DWORD				m_dwEffectCount;

	LPD3DXEFFECT		m_pParameterEffect;
	D3DXHANDLE			m_ParamHandleTable[SEM_World];
	LPD3DXEFFECTPOOL	m_pEffectPool;
	DWORD				m_dwVSVersionHWSupport, m_dwPSVersionHWSupport;
	DWORD				m_dwVSVersionUsed, m_dwPSVersionUsed;
	DWORD				m_dwActiveMaterial, m_dwActivePass, m_dwNumPasses;
	D3DXHANDLE			m_pActiveTechnique;

	DWORD				m_dwDefaultLMmatID;
	DWORD				m_dwDefaultVLmatID;

	bool ReallocateMaterialTable();
	bool ReallocateEffectTable();
	void* CreateDefaultParameter (const char *szDefValue, FXSemanticsType semanticType);
	FXSemantics StringToSemantic (const char* szSemantic);
	const char* SemanticToString (FXSemantics semantic);
	void SortMaterialParameters (IN OUT MATERIAL_PARAMETER *pParameters, IN OUT WORD &wParamNum, OUT WORD &wSpecificParamNum);

	LPD3DXEFFECT LoadEffect (IN const char* szFile, IN const char* szTechnique, OUT D3DXHANDLE &pTechnique, OUT D3DXHANDLE &pTechniqueLighted);
public:
	CP3DMaterialManager();
	~CP3DMaterialManager();
	bool Init ();
	void UnloadAll (bool bSaveDefaultMaterials = true);
	void* GetPoolHandle();
	DWORD LoadMaterial (const char *szFileName);
	void QueryParameters (IN DWORD matID, IN bool bQueryDynlightParams, OUT MATERIAL_PARAMETER pParamTable[SEM_NUM], OUT WORD &wAllParamsNum, OUT WORD &wParamsToSetNum);
	bool SetParameters (DWORD matID, const MATERIAL_PARAMETER pParamTable[SEM_NUM], WORD wNumParameters);
	bool SetMatrixArray (DWORD matID, void* paramHandle, const P3DMatrix *matrixArray, UINT dwMatrixCount);
	DWORD GetLMmaterialID();
	DWORD GetVertexLightMaterialID();
	void GetShaderVersion (OUT DWORD &vertexVersion, OUT DWORD &pixelVersion);
	void SetMaxShaderVersion (DWORD VSmajor, DWORD VSminor, DWORD PSmajor, DWORD PSminor);

	void Establish_LM_Material (void* diffTexture, void* lmTexture);
	void Establish_VertexLight_Material (void* diffTexture);
	bool SetMaterial (DWORD matID, DWORD dwDynLightsCount = 0);
	bool NextPass (DWORD matID);
	void TurnOffShaders ();
//	bool Reload();		// TODO: pridat znovunacitanie shaderov

	void OnViewMatrixChange (P3DXVector3D *camLookAt = NULL);
	void OnProjectionMatrixChange ();
	void OnViewportSizeChange (float ViewportSize[2]);
};

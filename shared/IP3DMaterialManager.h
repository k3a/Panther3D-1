
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	IP3DMaterialManager interface
//-----------------------------------------------------------------------------


#pragma once

#include "interface.h"
#include "HP3DVector.h"
#include "IP3DLightManager.h"

#define	DYNLIGHT_SEMANTICS		9		// pocet semantics pouzitych pre dynamicke osvetlenie (SEM_DynLightTransform az SEM_DynLightTexture3)

//---------------------------------
enum FXSemanticsType
{
	SEMTYPE_FLOAT1, SEMTYPE_FLOAT2, SEMTYPE_FLOAT3, SEMTYPE_FLOAT4,
	SEMTYPE_FLOAT4x4, SEMTYPE_FLOAT3x3, SEMTYPE_FLOAT3x4, SEMTYPE_FLOAT4x3,
	SEMTYPE_TEXTURE_2D, SEMTYPE_TEXTURE_CUBE, SEMTYPE_TEXTURE_VOLUME,
	SEMTYPE_INT1, SEMTYPE_INT2, SEMTYPE_INT3, SEMTYPE_INT4,
	SEMTYPE_HALF1, SEMTYPE_HALF2, SEMTYPE_HALF3, SEMTYPE_HALF4,
	SEMTYPE_HALF4x4, SEMTYPE_HALF3x3, SEMTYPE_HALF3x4, SEMTYPE_HALF4x3,
	SEMTYPE_DOUBLE1, SEMTYPE_DOUBLE2, SEMTYPE_DOUBLE3, SEMTYPE_DOUBLE4,
	SEMTYPE_BOOL1, SEMTYPE_BOOL2, SEMTYPE_BOOL3, SEMTYPE_BOOL4,
	SEMTYPE_UNKNOWN
};

// s touto tabulkou upravit vzdy aj g_SemanticsInfoTable !
//---------------------------------
enum FXSemantics
{
	// tieto parametre su zdielane cez pool pre vsetky efekty
	// nastavuju sa automaticky pri zmene, t.j. napr. pri zmene view transformacie 
	// v engine sa nastavia vsetky matView*
	SEM_View,
	SEM_ViewTranspose,
	SEM_ViewInverse,
	SEM_ViewInverseTranspose,
	SEM_ViewProjection,
	SEM_ViewProjectionTranspose,
	SEM_ViewProjectionInverse,
	SEM_ViewProjectionInverseTranspose,
	SEM_Projection,
	SEM_ProjectionInverse,
	SEM_ProjectionInverseTranspose,
	SEM_CameraPosition,
	SEM_CameraDirection,
	SEM_ViewportPixelSize,

	// nasledujuce parametre sa uz musia nastavovat vzdy pre kazdy efekt zvlast:
   // tieto parametre by mal nastavit program (mesh/bsp triedy)
	// (na prvom mieste tejto casti tabulky by mal vzdy zostat SEM_World!)
	SEM_World,
	SEM_WorldTranspose,
	SEM_WorldInverse,
	SEM_WorldInverseTranspose,
	SEM_WorldView,
	SEM_WorldViewTranspose,
	SEM_WorldViewInverse,
	SEM_WorldViewInverseTranspose,
	SEM_WorldViewProjection,
	SEM_WorldViewProjectionInverse,
	SEM_WorldViewProjectionInverseTranspose,

	SEM_AnyValue,
	SEM_CustomValue1,
	SEM_CustomValue2,
	SEM_CustomValue3,
	SEM_CustomValue4,
	// skinning
	SEM_NumBones,
	SEM_SkinMatrixArray,
	// textury
	SEM_TextureLayer0,
	SEM_TextureLayer1,
	SEM_TextureLayer2,
	SEM_TextureLayer3,
	SEM_TextureLayer4,
	SEM_TextureLayer5,
	SEM_TextureLayer6,
	SEM_TextureLayer7,
	SEM_TextureCubeMap,
	SEM_TextureVolumeMap,
	// lightgrid osvetlenie
	SEM_LightGridInfo,
	// dynamicke svetlo
	SEM_DynLightTransform,
	SEM_DynLightRadius,
	SEM_DynLightColor,
	SEM_DynLightUseTexture,
	SEM_DynLightCount,
	SEM_DynLightTexture0,
	SEM_DynLightTexture1,
	SEM_DynLightTexture2,
	SEM_DynLightTexture3,

	// nasledujuce parametre 'prednastavuje' material manager
	// (na prvom mieste tejto casti tabulky by mal vzdy zostat SEM_Time!)
	SEM_Time,
	SEM_TimeDelta,

	// koniec tabulky
	SEM_NUM,
	SEM_Unknown
};

// makra ohranicujuce jednotlive casti tabulky, vzdy pri zmene tabulky upravit aj tieto makra
// v zdrojakoch pouzivat ako hranicne hodnoty tieto makra
#define SHARED_SEMANTICS(p)			(p < SEM_World)
#define ADJUSTABLE_SEMANTICS(p)		((p >= SEM_World) && (p < SEM_Time))
#define GENERAL_SEMANTICS(p)		((p >= SEM_Time) && (p < SEM_NUM))


struct MATERIAL_PARAMETER{
	FXSemantics			SemanticID;
	void				*paramHandle;
	void				*pValue;
	char				szTextValue[128];
	MATERIAL_PARAMETER() : SemanticID(SEM_Unknown), paramHandle(NULL), pValue(NULL) \
		{memset (szTextValue, 0, sizeof(szTextValue));}
};

//							IP3DMaterialManager
//---------------------------------
class IP3DMaterialManager : public IP3DBaseInterface
{
public:
	virtual bool Init () = 0;
	virtual void UnloadAll (bool bSaveDefaultMaterials = true) = 0;
	virtual void* GetPoolHandle() = 0;
	virtual DWORD LoadMaterial (const char *szFileName) = 0;
	virtual void QueryParameters (IN DWORD matID, IN bool bQueryDynlightParams, OUT MATERIAL_PARAMETER pParamTable[SEM_NUM], OUT WORD &wAllParamsNum, OUT WORD &wParamsToSetNum) = 0;
	virtual bool SetParameters (DWORD matID, const MATERIAL_PARAMETER pParamTable[SEM_NUM], WORD wNumParameters) = 0;
	virtual bool SetMatrixArray (DWORD matID, void* paramHandle, const P3DMatrix *matrixArray, UINT dwMatrixCount) = 0;
	virtual DWORD GetLMmaterialID() = 0;
	virtual DWORD GetVertexLightMaterialID() = 0;
	virtual void GetShaderVersion (OUT DWORD &vertexVersion, OUT DWORD &pixelVersion) = 0;
	virtual void SetMaxShaderVersion (DWORD VSmajor, DWORD VSminor, DWORD PSmajor, DWORD PSminor) = 0;

	virtual void Establish_LM_Material (void* diffTexture, void* lmTexture) = 0;
	virtual void Establish_VertexLight_Material (void* diffTexture) = 0;
	virtual bool SetMaterial (DWORD matID, DWORD dwDynLightsCount = 0) = 0;
	virtual bool NextPass (DWORD matID) = 0;
	virtual void TurnOffShaders () = 0;

	virtual void OnViewMatrixChange (P3DXVector3D *camLookAt = NULL) = 0;
	virtual void OnProjectionMatrixChange () = 0;
	virtual void OnViewportSizeChange (float ViewportSize[2]) = 0;

};

#define IP3DRENDERER_MATERIALMANAGER "P3DMaterialManager_1" // nazev ifacu

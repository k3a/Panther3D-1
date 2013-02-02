
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	.X files managing
//-----------------------------------------------------------------------------


/**************************************************************************************************


 CP3DMeshLoader - vytvara objekty CP3DMesh a CP3DMeshHierarchy. Vsetky modely nahrane z jedneho suboru
 a s rovnakymi flagmi zdielaju prakticky vsetky data (caching modelov). Iba pre objekty CP3DMeshHierarchy
 sa vzdy vytvara vlastny AnimationController.
 
 CP3DMesh - bezny mesh nahravany fciou D3DXLoadMeshFromX, neobsahuje pohyblive casti

 CP3DMeshHierarchy - mesh s kostrou nahravany fciou D3DXLoadMeshHierarchyFromX, prehrava 
 animacie, podporuje skinning. Kosti su reprezentovane stromovou strukturou objektov
 D3DXFRAME_DERIVED. Kazdy vertex by mal mat (v 3DS Max) "priradeny"
 co najmensi pocet kosti, maximalne 4. Skinning bezi kompletne cez shadery.
 Maximalny pocet kosti na jeden model je MAX_SKIN_BONES v sucasnosti 82.

**************************************************************************************************/

#pragma once


#include <windows.h>
#include "common.h"
#include "mesh_support.h"

bool CalculateMeshTangents (LPD3DXBASEMESH pMesh, DWORD dwTextureCoordsIndex);


#define	DEF_ANIM_TRANSITION_TIME		0.25f	// cas potrebny pre (plynulu) zmenu jednej animacie na druhu
#define	MAX_MESH_MATERIALS				100		// sto materialov pre jeden mesh bude urcite stacit
// First 9 constants are used for other data.  Each 4x3 matrix takes up 3 constants.
// (96 - 9) /3 i.e. Maximum constant count - used constants 
#define MAX_SKIN_BONES	82		// predtym 26 (sm 1.1)

#define	MAX_ANIM_TRACKS_NUM		4		// pocet animacii, ktore je mozne sucasne prehravat (miesat)
#define	ANIMATION_SAMPLING_RATE	30

#define	UNIQUE_MODEL_IDENT		(0xFFFFFFFF - 1)		// unique models vratia vzdy ako sort_key tuto hodnotu

#define MAKE_MESH_OPTIMIZATION

class CP3DMesh;
class CP3DMeshHierarchy;


#define	INIT_CACHE_SIZE	50
struct MODEL_CACHE {					// polozka cache tabulky mesh_loadera
	char				*filename;
	IP3DBaseInterface	*pMeshObject;	// IP3DMesh, alebo IP3DMeshHierarchy
	MESH_LOAD_FLAGS		LoadFlags;
	DWORD				dwMeshID;
	bool				bInUse;			// pouzivane pri StartCaching(), StopCaching()
};


//			CP3DMeshLoader
//---------------------------------
class CP3DMeshLoader : public IP3DMeshLoader
{
private:
	MODEL_CACHE		*m_ModelCache;
	DWORD			m_dwCacheSize;			// pocet poloziek v m_ModelCache
	DWORD			m_dwNextFreeID;
	bool			m_bInCachingState;		// true ak je program "medzi" volaniami StartCaching() a StopCaching()

	bool ReallocateCacheBuffer();
	void UnloadFrameHierarchyRecursive (D3DXFRAME_DERIVED *pFrame);
	void UnloadCacheItem (DWORD dwIdx);
	DWORD Fetch7mt (const char *szFileName, MESH_MATERIAL_ASSOCIATION TableToFill[], DWORD dwTableSize);
	bool IsolateAnimations (const char *szFile, const LPD3DXANIMATIONCONTROLLER pAnimController);
	bool RemoveIsolatedAnimation (const char* szAnimSetName, const char* szAnimName, const LPD3DXANIMATIONCONTROLLER pAnimController);
	void SetSkinMatrixPtrs (D3DXFRAME_DERIVED *pFrameRoot, D3DXFRAME_DERIVED *pFrameBase = NULL);
	IP3DBaseInterface* MeshLoading (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures, DWORD dwUseThisPainting, bool bForceToCreateNew);
	CP3DMesh* DuplicateMesh (CP3DMesh *pMeshOrigin, DWORD dwUseThisPainting, ezxml_t xmlTextures);
	CP3DMesh* LoadMeshFromFile (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures, DWORD dwUseThisPainting);
	CP3DMeshHierarchy* DuplicateMeshHierarchy (CP3DMeshHierarchy *pMeshOrigin, DWORD dwUseThisPainting, ezxml_t xmlTextures);
	DWORD RemoveUnusedFrames (D3DXFRAME_DERIVED &pFrame, bool bFirstCall = true);
	CP3DMeshHierarchy* LoadMeshHierarchyFromFile (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures, DWORD dwUseThisPainting);
public:
	CP3DMeshLoader();
	~CP3DMeshLoader();
	void UnloadAll ();
	void StartCaching ();
	void StopCaching ();
	bool PreCache (const char *szFileName, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures = NULL, DWORD dwUseThisPainting = 0);
	bool UnCache (const char *szFileName, MESH_LOAD_FLAGS MeshLoadFlags);
	IP3DMesh* LoadMesh (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags = MESH_LOAD, ezxml_t xmlTextures = NULL, DWORD dwUseThisPainting = 0, bool bForceToCreateNew = false);
	IP3DMeshHierarchy* LoadMeshHierarchy (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags = MESH_LOAD_HIERARCHY, ezxml_t xmlTextures = NULL, DWORD dwUseThisPainting = 0, bool bForceToCreateNew = false);
};

//			CP3DMesh
//---------------------------------
class CP3DMesh : public IP3DMesh
{
	friend class CP3DMeshLoader;
private:
	D3DXFRAME_DERIVED			*m_pFrameRoot;
	MESH_LOAD_FLAGS				m_LoadFlags;
	DWORD						m_dwMeshID;
	DWORD						m_dwPaintingVersion;
	DWORD						m_dwForceToUseThisMaterial;	// ak je nastaveny tak sa pouzije tento shader material, pre cely mesh
	bool						m_bUniqueModel;				// ak je model nahravany s bForceToCreateNew
	const BSP_LIGHTCELL_INFO	*m_pLightingInfo;

	void SetLightInfoPointer (BSP_LIGHTCELL_INFO *pLightInfo);
	void SetShaderParameters (DWORD dwMaterialID, const D3DXMESHCONTAINER_DERIVED *pContainer, DWORD dwMeshMaterialID, SHADER_DYNLIGHT_INPUT *pShaderDynlightsInput);
public:
	CP3DMesh();
	~CP3DMesh();
	DWORD	GetVertexCount();
	bool	GetVertexPositionCloud (IN OUT P3DXVector3D *pVertexData, IN DWORD dwVertexCount);
	void	MeshMove (const P3DMatrix &matWorld);
	void	Render (bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input);
	void	ForceToUseThisMaterial (DWORD dw7mtMaterialID);
	void*	GetTopFrame();
	void	GetSortKey (UINT &SortKeyMajor, UINT &SortKeyMinor);
	bool	SaveXFile (const char* filename);
};




// polozka track tabulky - pouzivane programom, nie DX
//---------------------------------
struct ANIM_TRACK_TABLE_ITEM{
	DWORD		dwAnimationIndex;		// 0xffffffff pre ziadnu animaciu
	bool		bHighOrLow;				// urcuje ci sa aktivna animacia nachadza v dolnej alebo hornej polovici anim. track-ov
};


//		CP3DMeshHierarchy
//---------------------------------
class CP3DMeshHierarchy : public IP3DMeshHierarchy
{
	friend class CP3DMeshLoader;
private:
	D3DXFRAME_DERIVED			*m_pFrameRoot;
	LPD3DXANIMATIONCONTROLLER	m_animController;

	float						m_currentTime;			// current animation time
	ANIM_TRACK_TABLE_ITEM		m_TrackTable[MAX_ANIM_TRACKS_NUM];	// animation track tabulka
//	DWORD						m_dwMaxAnimationTracks;	// pocet poloziek v track tabulke
	DWORD						m_dwAnimationSetNum;	// pocet animacii, ktore .x subor obsahuje
	DWORD						m_dwMeshID;
	DWORD						m_dwPaintingVersion;
	DWORD						m_dwForceToUseThisMaterial;	// ak je nastaveny tak sa pouzije tento shader material, pre cely mesh
	MESH_LOAD_FLAGS				m_LoadFlags;
	const BSP_LIGHTCELL_INFO	*m_pLightingInfo;
	bool						m_bUniqueModel;			// ak je model nahravany s bForceToCreateNew
	bool						m_bLocalTransformsNotSet;	// true ak su vsetky matLocalTansform matice jednotkove

	void DrawFrame (D3DXFRAME_DERIVED *frame, bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input);
	void DrawFrameSkinned (D3DXFRAME_DERIVED *pFrame, bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input);
	void SetShaderParameters (DWORD dw7mtMaterialID, const D3DXFRAME_DERIVED *pFrame, DWORD dwMeshMaterialID, SHADER_DYNLIGHT_INPUT *pShaderDynlightsInput);
	void SetCombinedMatrices (D3DXFRAME_DERIVED *frameBase, const D3DXMATRIX *parentMatrix);
	// animacie:
	const LPD3DXANIMATIONSET GetAnimationSet(DWORD index) const;

public:
	CP3DMeshHierarchy();
	~CP3DMeshHierarchy();
	void MeshMove (float elapsedTime, const P3DMatrix &matWorld);
	void Render(bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input);
	void ForceToUseThisMaterial (DWORD dw7mtMaterialID);
	bool Save (const char* filename);

	void* GetTopFrame();
	void GetSortKey (UINT &SortKeyMajor, UINT &SortKeyMinor);
	void SetLightInfoPointer (BSP_LIGHTCELL_INFO *pLightInfo);
	void ResetBonesLocalTransforms();
	void SetBoneLocalTransform (const char *szFrameName, const P3DMatrix &frameMatrix);
	// animacie:
	void SetAnimation (DWORD dwTrackIndex, DWORD dwAnimSetIndex);
	void ResetTime ();
};

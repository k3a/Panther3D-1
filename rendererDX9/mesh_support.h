
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Mesh support interfaces and stuctures
//-----------------------------------------------------------------------------


#pragma once


struct MESH_RAW_MATERIAL {		// definicia materialu ako je ulozeny v x subore
	float rDif, gDif, bDif, aDif;
	float specPower;
	float rSpec, gSpec, bSpec;
	float rEm, gEm, bEm;
};

struct MESH_MATERIAL_ASSOCIATION {
	char				*sz7mtMaterialName;
	char				*szTextureName;
	MESH_RAW_MATERIAL	material;
	bool				bAlreadyUsed;

	MESH_MATERIAL_ASSOCIATION()
		{memset (this, 0, sizeof(MESH_MATERIAL_ASSOCIATION));}
	~MESH_MATERIAL_ASSOCIATION()
	{
		SAFE_DELETE_ARRAY (sz7mtMaterialName)
		SAFE_DELETE_ARRAY (szTextureName)
	}
};

struct ALLOC_HIERARCHY_INITIALIZERS {
	BOOL		bLoadSkinInfo;
	MESH_MATERIAL_ASSOCIATION *Mesh7mtMaterials;
	DWORD		dw7mtMaterialsNum;
	ezxml_t		xmlTextures;
	DWORD		dwUseThisPainting;
	ALLOC_HIERARCHY_INITIALIZERS() : bLoadSkinInfo(false), Mesh7mtMaterials(NULL), dw7mtMaterialsNum(0), \
									xmlTextures(NULL), dwUseThisPainting(0) {};
};


// pouzivane vo fcii D3DXSaveMeshHierarchyToFile
//---------------------------------
class CSaveUserData : public ID3DXSaveUserData
{
	STDMETHOD (AddFrameChildData)(CONST D3DXFRAME *pFrame, LPD3DXFILESAVEOBJECT pXofSave, LPD3DXFILESAVEDATA pXofFrameData)
		{return S_OK;}
	STDMETHOD (AddMeshChildData)(CONST D3DXMESHCONTAINER *pMeshContainer, LPD3DXFILESAVEOBJECT pXofSave, LPD3DXFILESAVEDATA pXofMeshData)
		{return S_OK;}
	// NOTE: this is called once per Save.  All top level objects should be added using the
	// provided interface.  One call adds objects before the frame hierarchy, the other after
	STDMETHOD (AddTopLevelDataObjectsPre)(LPD3DXFILESAVEOBJECT pXofSave);
	STDMETHOD (AddTopLevelDataObjectsPost)(LPD3DXFILESAVEOBJECT pXofSave)
		{return S_OK;}
	// callbacks for the user to register and then save templates to the XFile
	STDMETHOD (RegisterTemplates)(LPD3DXFILE pXFileApi);
	STDMETHOD (SaveTemplates)(LPD3DXFILESAVEOBJECT pXofSave)
		{return S_OK;}
};


// pouzivane vo fcii D3DXLoadMeshHierarchyFromX
//---------------------------------
class CLoadUserData : public ID3DXLoadUserData 
{
	STDMETHOD (LoadTopLevelData)(LPD3DXFILEDATA pXofChildData);
	STDMETHOD (LoadFrameChildData)(LPD3DXFRAME pFrame,LPD3DXFILEDATA pXofChildData)
		{return S_OK;}
	STDMETHOD (LoadMeshChildData)(LPD3DXMESHCONTAINER pMeshContainer, LPD3DXFILEDATA pXofChildData)
		{return S_OK;}
};


// alokuje casti meshov
//---------------------------------
class CAllocateHierarchy  : public ID3DXAllocateHierarchy
{
private:
	ALLOC_HIERARCHY_INITIALIZERS	m_localData;
public:
	CAllocateHierarchy::CAllocateHierarchy (ALLOC_HIERARCHY_INITIALIZERS	&Initializers) : m_localData(Initializers) {};
	STDMETHOD (CreateFrame)(LPCSTR Name, LPD3DXFRAME *ppNewFrame);
	STDMETHOD (CreateMeshContainer) (LPCSTR Name, const D3DXMESHDATA *pMeshData, \
					const D3DXMATERIAL *pMaterials,const D3DXEFFECTINSTANCE *pEffectInstances, \
					DWORD NumMaterials, const DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, \
					LPD3DXMESHCONTAINER *ppNewMeshContainer);
	STDMETHOD (DestroyFrame) (LPD3DXFRAME pFrameToFree)
		{return S_OK;}
	STDMETHOD (DestroyMeshContainer) (LPD3DXMESHCONTAINER pMeshContainerToFree)
		{return S_OK;}
};

// uvolnuje casti meshov
//---------------------------------
class CDeallocateHierarchy  : public ID3DXAllocateHierarchy
{
public:
	STDMETHOD (CreateFrame)(LPCSTR Name, LPD3DXFRAME *ppNewFrame)
		{ppNewFrame = NULL; return E_FAIL;}
	STDMETHOD (CreateMeshContainer) (LPCSTR Name, const D3DXMESHDATA *pMeshData, \
				const D3DXMATERIAL *pMaterials,const D3DXEFFECTINSTANCE *pEffectInstances, \
				DWORD NumMaterials, const DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, \
				LPD3DXMESHCONTAINER *ppNewMeshContainer)
		{ppNewMeshContainer = NULL; return E_FAIL;}
	STDMETHOD (DestroyFrame) (LPD3DXFRAME pFrameToFree);
	STDMETHOD (DestroyMeshContainer) (LPD3DXMESHCONTAINER pMeshContainerToFree);
};


//---------------------------------
struct D3DXMESHCONTAINER_DERIVED: public D3DXMESHCONTAINER
{
	// textury
	LPDIRECT3DTEXTURE9	*pTextures;	// 3-rozmerna tabulka textur s rozmermi [NumMaterials] [NumCoordinates] [NumPaintings]
	DWORD				dwNumCoords;
	DWORD				dwNumPaintings;
	// shader materials
	DWORD				*p7MTmaterials;				// pouzite 7mt materialy, 0xFFFFFFFF pre ziaden material
	// skin info
	D3DXMATRIX			**ppBoneTransformMatrices;	// smerniky na transform. matice prislusnych kosti (frame-ov) - konstantne
	D3DXMATRIX			*pBoneInverseMatrices;		// inverzne matice pre danu kost - nemenne
	DWORD				NumInfl;
	DWORD				NumAttributeGroups;
	LPD3DXBUFFER		pBoneCombinationBuf;
	DWORD				NumPaletteEntries;

	// ***************************************
	D3DXMESHCONTAINER_DERIVED();
	~D3DXMESHCONTAINER_DERIVED();
};

//---------------------------------
struct D3DXFRAME_DERIVED: public D3DXFRAME
{
	// kombinovana matica
	D3DXMATRIX	matCombined;
	// matica pre transformaciu kosti, nastavuje sa programovo, nie animaciou
	D3DXMATRIX	matLocalTansform;
	bool		bIdentityLocalTransform;	// true ak matLocalTansform je jednotkova matica, pre urychlenie vypoctov
	//***************************************
	D3DXFRAME_DERIVED();
	~D3DXFRAME_DERIVED();
	D3DXFRAME_DERIVED *Find(const char *FrameName);
};

//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Alpha manager class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "IP3DAlphaManager.h"
#include "IP3DPhysEngine.h"
#include "IP3DBSPMap.h"
#include "IP3DFrustum.h"
#include "common.h"
#include "sorting_alg.h"


#define INIT_TABLE_SIZE				30
// grass constants
#define MAX_GRASS_BILLBOARDS		120		// musi byt zhodne s hodnotou v shadery
#define MIN_GRASS_BILLBOARDS		5
#define MAX_GRASS_RADIUS			500.0f
#define MIN_GRASS_RADIUS			50.0f


#define GWIN_FVF		(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define GRASS_FVF		(D3DFVF_XYZ | D3DFVF_TEX1)

// vrati nahodnu hodnotu z intervalu 0 az 1
inline float GetRndValue()
	{return (float(rand()) / float(RAND_MAX));}


struct GWIN_VERTEX_FORMAT {
	P3DXVector3D	position;
	P3DXVector3D	normal;
	float			u, v;
};

struct GRASS_VERTEX_FORMAT {
	P3DXVector3D	position;
	float			u, v;
};


struct ALPHA_SORT_TABLE {
	short		Idx;
	float		fDistance;
	// operatory pre sorting
	bool operator<(const ALPHA_SORT_TABLE &B) const {return (this->fDistance < B.fDistance);}
	bool operator>(const ALPHA_SORT_TABLE &B) const {return (this->fDistance > B.fDistance);}
	bool operator<=(const ALPHA_SORT_TABLE &B) const {return (this->fDistance <= B.fDistance);}
	bool operator>=(const ALPHA_SORT_TABLE &B) const {return (this->fDistance >= B.fDistance);}
	bool operator==(const ALPHA_SORT_TABLE &B) const {return (this->fDistance == B.fDistance);}
	bool operator!=(const ALPHA_SORT_TABLE &B) const {return (this->fDistance != B.fDistance);}
};



struct GLASS_WINDOW {
	LPDIRECT3DVERTEXBUFFER9		pVB;
	LPDIRECT3DTEXTURE9			p_Texture;
	LPDIRECT3DCUBETEXTURE9		p_CubeTexture;
	IP3DPhysBody				*p_RigidBody;
	GLASS_WINDOW () {memset (this, 0, sizeof(GLASS_WINDOW));}
	~GLASS_WINDOW ()
	{
		SAFE_RELEASE (pVB)
		if (p_Texture) g_TextureLoader.UnloadTexture (p_Texture);
		if (p_CubeTexture) g_TextureLoader.UnloadTexture (p_CubeTexture);
		SAFE_DELETE (p_RigidBody)
	}
};

struct GRASS_BUNCH {
	LPDIRECT3DTEXTURE9	p_Texture;
	D3DXVECTOR4			*quad_light_size;
	D3DXVECTOR4			*quad_position_rot;
	ALPHA_SORT_TABLE	*sort_list;
	float				fSquaredFadeDistance;
	float				fSquaredFadeDistanceVisTest;
	WORD				wQuadCount;
	bool				bBillboarded;
	bool				bAnimated;

	GRASS_BUNCH () {memset (this, 0, sizeof(GRASS_BUNCH));}
	~GRASS_BUNCH ()
	{
		SAFE_DELETE_ARRAY (quad_light_size);
		SAFE_DELETE_ARRAY (quad_position_rot);
		SAFE_DELETE_ARRAY (sort_list);
		if (p_Texture)
			g_TextureLoader.UnloadTexture (p_Texture);
	}
};

struct DYN_MODEL_ALPHAMAN {
	IP3DMesh				*pMesh;
	bool					bIsLighted;
	SHADER_DYNLIGHT_INPUT	shader_input;
	bool					bVisible;
	P3DMatrix				matrix;
	DYN_MODEL_ALPHAMAN() : pMesh(NULL), bVisible(false), bIsLighted(false) {}
};

enum ALPHA_OBJECT_TYPE {
	alphaObj_none,
	alphaObj_window,
	alphaObj_grass,
	alphaObj_dynamic_model
};

struct OBJECT_TABLE {
	ALPHA_OBJECT_TYPE	type;
	P3DSphere		boundSphere;
	BSPCLUSTER_LIST		clusterList;
	union {
		GLASS_WINDOW		*pGlassWindow;
		GRASS_BUNCH			*pGrassBunch;
		DYN_MODEL_ALPHAMAN	*pDynamicModel;
		// pripadne dalsie !!!
		// ...
	};
	OBJECT_TABLE() {type = alphaObj_none, pGlassWindow = NULL;};
};

//---------------------------------
class CP3DAlphaManager : public IP3DAlphaManager
{
private:
	OBJECT_TABLE		*m_pObjects;
	DWORD				m_dwMaxObjectCount;		// pocet poloziek v m_pObjects, podla potreby rastie
	DWORD				m_dwVisibleObjects;
	DWORD				m_dwDynamicModels;
	ALPHA_SORT_TABLE	*m_pSortTable;
	LPD3DXEFFECT		m_pWindowShader;			// shader pre windows
	D3DXHANDLE			m_pWindowParams[3];
	LPD3DXEFFECT		m_pGrassShader;			// billboard shader pre grass
	D3DXHANDLE			m_pGrassParams[7];

	IP3DBSPMap			*m_pBspMap;
	IP3DPhysEngine		*m_pPhysEngine;
	IP3DFrustum			*m_pFrustum;
	LPDIRECT3DVERTEXDECLARATION9	m_pWindowsVertDec;
	LPDIRECT3DVERTEXDECLARATION9	m_pGrassVertDec;
	LPDIRECT3DVERTEXBUFFER9			m_pGrassVertexBuf;
	LPDIRECT3DINDEXBUFFER9			m_pGrassIndexBuf;

	bool ReallocateAlphaRecords ();
	void SortGrassBunch (GRASS_BUNCH *pGrassBunch);
public:
	CP3DAlphaManager();
	~CP3DAlphaManager();
	bool Init();
	void UnloadAll();
	bool CreateGlassWindow (P3DXVector3D *points, char* szAlphaTexture = NULL, char* szCubeMapTexture = NULL);
	bool CreateGrassBunch (P3DXVector3D origin, float fRadius, WORD quadCount, \
		float fFadeDistance, float fMinGrassQuadSize, float fMaxGrassQuadSize, \
		char *szTexture, bool bBillboarded, bool bAnimated);
 	DWORD CreateDynamicModel (IP3DMesh *pInitialMesh, P3DSphere &bsphere);
 	void UpdateDynamicModel (DWORD dwDynModel, IP3DMesh *pNewMesh, P3DMatrix &mat, bool bVisible, bool bIsLighted, SHADER_DYNLIGHT_INPUT &shader_input);
	void Loop ();
	void Render ();
	void BreakWindow (unsigned long Idx);
};

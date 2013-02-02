//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Renderable part of base entity
//-----------------------------------------------------------------------------
#pragma once
#include "IP3DMesh.h"
#include "IP3DPhysengine.h"

class eBase; // dopredna deklarace, je rozsirena pomoci include v cpp

struct LOD_MESH_TABLE {
	IP3DMesh*	pMesh;
	float			fmaxDistance;

	LOD_MESH_TABLE() : pMesh(NULL), fmaxDistance(0) {};
	~LOD_MESH_TABLE() {SAFE_DELETE(pMesh)}
};


class eBaseRenderable
{
public:
	~eBaseRenderable(); // TODO: InitBaseRenderable predelat na konstruktor a iniciovat v konstruktoru eBase
	void InitBaseRenderable(eBase* thisBase);
	bool LoadModel(const char* szModelPath, int paintVer=0);
	bool UnloadModel();
	void RecalculateLightingAtNewPosition();
	bool IsModelEntity(){return m_bLoaded;};
	void RenderableRender(float deltaTime);
	const char* GetModelPath(){return const_cast<const char*>(modelPath);};
private:
	bool LoadFromXml (const ezxml_t modelXml, DWORD dwUseThisPainting);
private:
	eBase* base;
	char *modelPath;
	//
	LOD_MESH_TABLE*			m_Lods;
	DWORD					m_dwNumLods;
	DWORD					m_dwActualLodIndex;
	bool					m_bAlphaUsed;
	DWORD					m_dwAlphaModelID;
	P3DXMatrix				m_TransformMatrix;
	BSP_LIGHTCELL_INFO		m_cellLightInfo;
	P3DSphere				m_boundsphereActual;	// aktualny bounding sphere vo world space
	P3DXVector3D				m_LastDeltaPosition;

	bool					m_bLoaded;
	//data entit
	SHADER_DYNLIGHT_INPUT	m_dynLightingData;
	bool					m_bIsLighted;	// true ak je model osvetleny dynamickym svetlom
	bool					m_bDynLightStateChanged;
};

//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	IP3DMeshLoader, IP3DMesh and IP3DMeshHierarchy interfaces
//-----------------------------------------------------------------------------


#pragma once

#include "interface.h"
#include "IP3DXML.h"
#include "IP3DLightManager.h"


enum MESH_LOAD_FLAGS{
	MESH_LOAD,				// non-hierarchy meshe
	MESH_LOAD_HIERARCHY,	// hierarchy meshe
	MESH_LOAD_SKINNED		// hierarchy skinned meshe
};


class IP3DMesh;
class IP3DMeshHierarchy;



//			IP3DMeshLoader
//---------------------------------
class IP3DMeshLoader : public IP3DBaseInterface
{
public:
	virtual void UnloadAll () = 0;
	virtual void StartCaching () = 0;
	virtual void StopCaching () = 0;
	virtual bool PreCache (const char *szFileName, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures = NULL, DWORD dwUseThisPainting = 0) = 0;
	virtual bool UnCache (const char *szFileName, MESH_LOAD_FLAGS MeshLoadFlags) = 0;
	virtual IP3DMesh* LoadMesh (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags = MESH_LOAD, ezxml_t xmlTextures = NULL, DWORD dwUseThisPainting = 0, bool bForceToCreateNew = false) = 0;
	virtual IP3DMeshHierarchy* LoadMeshHierarchy (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags = MESH_LOAD_HIERARCHY, ezxml_t xmlTextures = NULL, DWORD dwUseThisPainting = 0, bool bForceToCreateNew = false) = 0;

};
#define IP3DRENDERER_MESHLOADER "P3DMeshLoader_1" // nazev ifacu

//			IP3DMesh
//---------------------------------
class IP3DMesh : public IP3DBaseInterface
{
public:
	virtual void SetLightInfoPointer (BSP_LIGHTCELL_INFO *pLightInfo) = 0;
	virtual DWORD GetVertexCount() = 0;
	virtual bool GetVertexPositionCloud (IN OUT P3DXVector3D *pVertexData, IN DWORD dwVertexCount) = 0;
	virtual void MeshMove (const P3DMatrix &matWorld) = 0;
	virtual void Render (bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input) = 0;
	virtual void ForceToUseThisMaterial (DWORD dw7mtMaterialID) = 0;
	virtual void* GetTopFrame() = 0;
	virtual void GetSortKey (UINT &SortKeyMajor, UINT &SortKeyMinor) = 0;
	virtual bool SaveXFile (const char* filename) = 0;
};

//		IP3DMeshHierarchy
//---------------------------------
class IP3DMeshHierarchy : public IP3DBaseInterface
{
public:
	virtual void MeshMove (float elapsedTime, const P3DMatrix &matWorld) = 0;
	virtual void Render (bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input) = 0;
	virtual void ForceToUseThisMaterial (DWORD dw7mtMaterialID) = 0;
	virtual bool Save (const char* filename) = 0;
	virtual void SetLightInfoPointer (BSP_LIGHTCELL_INFO *pLightInfo)=0;
	virtual void ResetBonesLocalTransforms() = 0;
	virtual void SetBoneLocalTransform (const char *szFrameName, const P3DMatrix &frameMatrix) = 0;
	virtual void SetAnimation (DWORD dwTrackIndex, DWORD dwAnimSetIndex) = 0;
	virtual void ResetTime () = 0;
	virtual void* GetTopFrame() = 0;
	virtual void GetSortKey (UINT &SortKeyMajor, UINT &SortKeyMinor) = 0;
};

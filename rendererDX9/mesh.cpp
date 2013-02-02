
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DMesh class definition
//-----------------------------------------------------------------------------

#include "mesh.h"


D3DXMATRIX	g_matBuf[10];		// 10 - pocet moznych matic na vstupe shaderu, v pripade potreby upravit pocet


//////////////////////////////////////////////////////////////////////////
//											CP3DMesh
//////////////////////////////////////////////////////////////////////////


//---------------------------------
CP3DMesh::CP3DMesh()
{
	m_pFrameRoot = NULL;
	m_LoadFlags = MESH_LOAD;
	m_dwMeshID = 0xFFFFFFFF;
	m_dwPaintingVersion = 0;
	m_dwForceToUseThisMaterial = 0xFFFFFFFF;
	m_bUniqueModel = false;
	m_pLightingInfo = NULL;
}

//---------------------------------
CP3DMesh::~CP3DMesh()
{
	// ak ide o ForceToCreateNew model tak sa kompletne uvolni
	if (m_bUniqueModel && (m_pFrameRoot != NULL))
	{
		CDeallocateHierarchy Dealloc;
		D3DXFrameDestroy (m_pFrameRoot, &Dealloc);
	}
}

//---------------------------------
void* CP3DMesh::GetTopFrame ()
{
	return m_pFrameRoot;
}

//---------------------------------
void CP3DMesh::GetSortKey (UINT &SortKeyMajor, UINT &SortKeyMinor)
{
	SortKeyMajor = m_dwMeshID;
	SortKeyMinor = m_dwPaintingVersion;
}

//---------------------------------
void CP3DMesh::SetLightInfoPointer (BSP_LIGHTCELL_INFO *pLightInfo)
{
	m_pLightingInfo = pLightInfo;
}

//---------------------------------
DWORD CP3DMesh::GetVertexCount()
{
	if (m_pFrameRoot == NULL) return 0;
	LPD3DXMESH pMesh = m_pFrameRoot->pMeshContainer->MeshData.pMesh;
	if (pMesh)
		return pMesh->GetNumVertices ();
	return 0;
}

// vrati pozicie vertexov, kazdy vertex je vrateny ako <floatX, floatY, floatZ>
// pamat v pVertexData musi byt dostatocna!
//---------------------------------
bool CP3DMesh::GetVertexPositionCloud (IN OUT P3DXVector3D *pVertexData, IN DWORD dwVertexCount)
{
	DWORD dwVertexStride;
	LPBYTE pSourceData;
	LPD3DXMESH pMesh;

	if ((m_pFrameRoot == NULL) || (pVertexData == NULL)) return false;
	if (IsBadWritePtr (pVertexData, dwVertexCount * sizeof(P3DXVector3D))) return false;
	pMesh = m_pFrameRoot->pMeshContainer->MeshData.pMesh;
	if (pMesh == NULL) return false;

	dwVertexCount = min (dwVertexCount, pMesh->GetNumVertices ());
	dwVertexStride = pMesh->GetNumBytesPerVertex ();

	if (FAILED (pMesh->LockVertexBuffer (D3DLOCK_NOOVERWRITE | D3DLOCK_READONLY, (LPVOID*)&pSourceData)))
		return false;
	for (DWORD i=0; i<dwVertexCount; i++)			// kopirovanie pozicie vertexov
		pVertexData[i] = *((P3DXVector3D*)(pSourceData + i * dwVertexStride));
	pMesh->UnlockVertexBuffer ();

	return true;
}

// pohybuje meshom - nastavuje matCombined maticu
//---------------------------------
void CP3DMesh::MeshMove (const P3DMatrix &matWorld)
{
	m_pFrameRoot->matCombined = *((D3DXMATRIX*)&matWorld);
}

// nastavit hodnotu 0xFFFFFFFF pre "unforcing back"
//---------------------------------
void CP3DMesh::ForceToUseThisMaterial (DWORD dw7mtMaterialID)
{
	m_dwForceToUseThisMaterial = dw7mtMaterialID;
}

//---------------------------------
void CP3DMesh::SetShaderParameters (DWORD dw7mtMaterialID, const D3DXMESHCONTAINER_DERIVED *pContainer, DWORD dwMeshMaterialID, SHADER_DYNLIGHT_INPUT *pShaderDynlightsInput)
{
	WORD wAllParams, wParamsToSet;
	DWORD dwFreeMatrixIdx = 0;
	DWORD dwTextureBaseIdx = dwMeshMaterialID * pContainer->dwNumCoords * pContainer->dwNumPaintings + m_dwPaintingVersion;

	g_pMaterialManager->QueryParameters (dw7mtMaterialID, (pShaderDynlightsInput!=NULL),  g_paramTable, wAllParams, wParamsToSet);

	for (DWORD i=0; i<wParamsToSet; i++)
	{
		switch (g_paramTable[i].SemanticID)
		{
		case SEM_TextureLayer0:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx];
			break;
		case SEM_World:
			g_paramTable[i].pValue = &m_pFrameRoot->matCombined;
			break;
		case SEM_WorldViewProjection:
			D3DXMatrixMultiply (&g_matBuf[dwFreeMatrixIdx], &m_pFrameRoot->matCombined, &g_matViewProj); 
			g_paramTable[i].pValue = &g_matBuf[dwFreeMatrixIdx];
			dwFreeMatrixIdx++;
			break;
		case SEM_TextureLayer1:
			if (pContainer->dwNumCoords > 1)
			{
				STAT(STAT_SHADER_TEXTURES, 1);
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings];
			}
			break;

		// lightgrid osvetlenie
		case SEM_LightGridInfo:
			if (m_pLightingInfo)
				g_paramTable[i].pValue = (void*)m_pLightingInfo;
			break;
		// dynamicke osvetlenie
		case SEM_DynLightTransform:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_pos_rot[0];
			break;
		case SEM_DynLightRadius:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_radius[0];
			break;
		case SEM_DynLightColor:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_color[0];
			break;
		case SEM_DynLightUseTexture:
			BOOL bUseTextures[MAX_DYN_LIGHTS_PER_FACE];
			for (int j=0; j<pShaderDynlightsInput->light_count; j++)
				bUseTextures[j] = (pShaderDynlightsInput->light_textures[j] != NULL);
			g_paramTable[i].pValue = &bUseTextures[0];
			break;
		case SEM_DynLightCount:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_count;
			break;
		case SEM_DynLightTexture0:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[0];
			break;
		case SEM_DynLightTexture1:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[1];
			break;
		case SEM_DynLightTexture2:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[2];
			break;
		case SEM_DynLightTexture3:
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[3];
			break;

		// zvysne textury
		case SEM_TextureLayer2:
			if (pContainer->dwNumCoords > 2)
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings*2];
			break;
		case SEM_TextureLayer3:
			if (pContainer->dwNumCoords > 3)
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings*3];
			break;
		case SEM_TextureLayer4:
			if (pContainer->dwNumCoords > 4)
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings*4];
			break;
		case SEM_TextureLayer5:
			if (pContainer->dwNumCoords > 5)
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings*5];
			break;
		case SEM_TextureLayer6:
			if (pContainer->dwNumCoords > 6)
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings*6];
			break;
		case SEM_TextureLayer7:
			if (pContainer->dwNumCoords > 7)
				g_paramTable[i].pValue = &pContainer->pTextures[dwTextureBaseIdx + pContainer->dwNumPaintings*7];
			break;
		// zvysne transformacie
		case SEM_WorldView:
			D3DXMatrixMultiply (&g_matBuf[dwFreeMatrixIdx], &m_pFrameRoot->matCombined, &g_matView);
			g_paramTable[i].pValue = &g_matBuf[dwFreeMatrixIdx];
			dwFreeMatrixIdx++;
			break;
		}
	}

	g_pMaterialManager->SetParameters (dw7mtMaterialID, g_paramTable, wAllParams);
}

// funkcia renderuje cely mesh
//---------------------------------
void CP3DMesh::Render (bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input)
{
	DWORD						dwMaterialID;
	bool						bFixedWorldTransformSetting = false;
	D3DXMESHCONTAINER_DERIVED	*meshContainer = (D3DXMESHCONTAINER_DERIVED*)m_pFrameRoot->pMeshContainer;

	// vykreslenie meshu
	for (DWORD i=0; i < meshContainer->NumMaterials; i++)
	{
		// shaders
		//////////////////////////////////////////////////////////////////////////
		if (m_dwForceToUseThisMaterial != 0xFFFFFFFF)
			dwMaterialID = m_dwForceToUseThisMaterial;
		else
			dwMaterialID = meshContainer->p7MTmaterials[i];

		if (dwMaterialID != 0xFFFFFFFF)
		{
			SetShaderParameters (dwMaterialID, meshContainer, i, (bIsDynLighted ? &shader_input : NULL)); // nastavenie parametrov shaderu

			if (g_pMaterialManager->SetMaterial (dwMaterialID, (bIsDynLighted ? (DWORD)shader_input.light_count : 0)))
				do {
					STAT(STAT_DRAW_CALLS, 1);
					STAT(STAT_DRAW_VERTS, meshContainer->MeshData.pMesh->GetNumVertices());
					meshContainer->MeshData.pMesh->DrawSubset(i);
				}while (g_pMaterialManager->NextPass (dwMaterialID));
		}
		// fixed function pipeline - legacy feature! vsetko by malo ist cez shadery
		//////////////////////////////////////////////////////////////////////////
		else if (m_pLightingInfo)
		{
			// zapnutie DX fixed pipeline svetla
			P3DLIGHT9 light;
			memset (&light, 0, sizeof(P3DLIGHT9));
			light.Type = RLT_DIRECTIONAL;
			light.Ambient.r = m_pLightingInfo->Ambient.r;
			light.Ambient.g = m_pLightingInfo->Ambient.g;
			light.Ambient.b = m_pLightingInfo->Ambient.b;
			light.Ambient.a = 1.0f;
			light.Diffuse.r = m_pLightingInfo->Directional.r;
			light.Diffuse.g = m_pLightingInfo->Directional.g;
			light.Diffuse.b = m_pLightingInfo->Directional.b;
			light.Diffuse.a = 1.0f;
			light.Direction = m_pLightingInfo->Direction;

			g_pRenderer->EnableLighting (true);
			g_pRenderer->SetLight (0, light);
			g_pRenderer->LightEnable (0, true);

			// nastavenie transformácie daného framu
			if (!bFixedWorldTransformSetting)
			{
				g_pD3DDevice->SetTransform(D3DTS_WORLD, &m_pFrameRoot->matCombined);
				bFixedWorldTransformSetting = true;
			}
			g_pMaterialManager->TurnOffShaders ();
			g_pRenderer->SetTexture (meshContainer->pTextures[i * meshContainer->dwNumCoords * meshContainer->dwNumPaintings + m_dwPaintingVersion]);
			g_pD3DDevice->SetMaterial( &meshContainer->pMaterials[i].MatD3D );
			STAT(STAT_DRAW_CALLS, 1);
			STAT(STAT_DRAW_VERTS, meshContainer->MeshData.pMesh->GetNumVertices());
			meshContainer->MeshData.pMesh->DrawSubset(i);

			// vypnutie fixed pipeline svetla
			g_pRenderer->EnableLighting (false);
			g_pRenderer->LightEnable (0, false);
		}
	}
	// TODO: odstranit - debug !!!
	if (meshContainer->pNextMeshContainer)
		MessageBox (NULL, "meshContainer->pNextMeshContainer != NULL", "", MB_SYSTEMMODAL);
	// debug !!!
}

// ulozi mesh
//---------------------------------
bool CP3DMesh::SaveXFile (const char* filename)
{
	if (filename == NULL) return false;
	CSaveUserData CUserData;
	//D3DXF_FILEFORMAT_BINARY, D3DXF_FILEFORMAT_TEXT
	if (FAILED (D3DXSaveMeshHierarchyToFile (filename, D3DXF_FILEFORMAT_BINARY, m_pFrameRoot, NULL, &CUserData)))
		return false;
	return true;

}


//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DMeshHierarchy class definition
//-----------------------------------------------------------------------------


#include "mesh.h"


//premenné
//P3DMatrix4x3	g_SkinShaderInputMatrixArray[MAX_SKIN_BONES];
P3DMatrix		g_SkinShaderInputMatrixArray[MAX_SKIN_BONES];

extern D3DXMATRIX	g_matBuf[10];

//////////////////////////////////////////////////////////////////////////
//										CP3DMeshHierarchy
//////////////////////////////////////////////////////////////////////////


//---------------------------------
CP3DMeshHierarchy::CP3DMeshHierarchy()
{
	m_pFrameRoot = NULL;
	m_animController = NULL;
	m_currentTime = 0.0f;
	m_dwAnimationSetNum = 0;
	m_dwMeshID = 0xFFFFFFFF;
	m_dwPaintingVersion = 0;
	m_dwForceToUseThisMaterial = 0xFFFFFFFF;
	m_LoadFlags = MESH_LOAD_HIERARCHY;
	m_pLightingInfo = NULL;
	m_bUniqueModel = false;
	m_bLocalTransformsNotSet = true;
}

//---------------------------------
CP3DMeshHierarchy::~CP3DMeshHierarchy()
{
	SAFE_RELEASE (m_animController)

	// ak ide o ForceToCreateNew model tak sa kompletne uvolni
	if (m_bUniqueModel)
	{
		if (m_pFrameRoot)
		{
			CDeallocateHierarchy Dealloc;
			D3DXFrameDestroy (m_pFrameRoot, &Dealloc);
		}
	}
}

//---------------------------------
void* CP3DMeshHierarchy::GetTopFrame ()
{
	return m_pFrameRoot;
}

//---------------------------------
void CP3DMeshHierarchy::GetSortKey (UINT &SortKeyMajor, UINT &SortKeyMinor)
{
	SortKeyMajor = m_dwMeshID;
	SortKeyMinor = m_dwPaintingVersion;
}

//---------------------------------
void CP3DMeshHierarchy::SetLightInfoPointer (BSP_LIGHTCELL_INFO *pLightInfo)
{
	m_pLightingInfo = pLightInfo;
}

// rekurzívne volaná pri renderovaní celej hierarchie daného framu
// volané z funkcie Render -> DrawFrame(m_pFrameRoot);
//---------------------------------
void CP3DMeshHierarchy::DrawFrame (D3DXFRAME_DERIVED *pFrame, bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input)
{
	DWORD						dwMaterialID;
	bool						bFixedWorldTransformSetting = false;
	D3DXMESHCONTAINER_DERIVED	*meshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

	// vykreslenie meshu
	if (meshContainer)
	{
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
				SetShaderParameters (dwMaterialID, pFrame, i, (bIsDynLighted ? &shader_input : NULL)); // nastavenie parametrov shaderu

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
					g_pD3DDevice->SetTransform(D3DTS_WORLD, &pFrame->matCombined);
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
			MessageBox (NULL, "meshContainer->pNextMeshContainer != NULL", "", 0);
		// debug !!!
	}


	if (pFrame->pFrameSibling)
		DrawFrame((D3DXFRAME_DERIVED*)pFrame->pFrameSibling, bIsDynLighted, shader_input);

	if (pFrame->pFrameFirstChild)
		DrawFrame((D3DXFRAME_DERIVED*)pFrame->pFrameFirstChild, bIsDynLighted, shader_input);
}

//---------------------------------
void CP3DMeshHierarchy::DrawFrameSkinned (D3DXFRAME_DERIVED *pFrame, bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input)
{
	DWORD						dwMaterialID;
	D3DXMESHCONTAINER_DERIVED	*pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

	// vykreslenie meshu
	if (pMeshContainer && pMeshContainer->pBoneCombinationBuf)
	{
		for (DWORD i=0; i < pMeshContainer->NumMaterials; i++)
		{
			if (m_dwForceToUseThisMaterial != 0xFFFFFFFF)
				dwMaterialID = m_dwForceToUseThisMaterial;
			else
				dwMaterialID = pMeshContainer->p7MTmaterials[i];

			if (dwMaterialID != 0xFFFFFFFF)
			{
				SetShaderParameters (dwMaterialID, pFrame, i, (bIsDynLighted ? &shader_input : NULL)); // nastavenie parametrov shaderu

				if (g_pMaterialManager->SetMaterial (dwMaterialID, (bIsDynLighted ? (DWORD)shader_input.light_count : 0)))
					do {
						STAT(STAT_DRAW_CALLS, 1);
						STAT(STAT_DRAW_VERTS, pMeshContainer->MeshData.pMesh->GetNumVertices());
						pMeshContainer->MeshData.pMesh->DrawSubset(i);
					}while (g_pMaterialManager->NextPass (dwMaterialID));
			}
		}
	}

	// TODO: odstranit - debug !!!
	if (pMeshContainer && pMeshContainer->pNextMeshContainer)
		MessageBox (NULL, "pMeshContainer->pNextMeshContainer != NULL", "", 0);
	// debug !!!

	// rekurzia
	if (pFrame->pFrameSibling)
		DrawFrameSkinned((D3DXFRAME_DERIVED*)pFrame->pFrameSibling, bIsDynLighted, shader_input);

	if (pFrame->pFrameFirstChild)
		DrawFrameSkinned((D3DXFRAME_DERIVED*)pFrame->pFrameFirstChild, bIsDynLighted, shader_input);
}

// rekurzivne volana pre celu hierarchiu - nastavuje matCombined matice
// matCombined matice predstavuju aktualnu svetovu transformaciu jednotlivych kosti
//---------------------------------
void CP3DMeshHierarchy::SetCombinedMatrices (D3DXFRAME_DERIVED *frameBase, const D3DXMATRIX *parentMatrix)
{
	// aplikovanie "custom" (lokalnej) transformacie frameBase->matLocalTansform
	// frameBase->TransformationMatrix predstavuje transformaciu sposobenu animaciou a nastavuje
	// ju iba animation controller, nie priamo program
	if (frameBase->bIdentityLocalTransform)
		frameBase->matCombined = frameBase->TransformationMatrix;	// local transform sa nepouziva
	else
		frameBase->matCombined = frameBase->matLocalTansform * frameBase->TransformationMatrix;
	// ak existuje rodièovská matica, tak sa nasobi - m_pFrameRoot sa posunie do svetoveho priestoru
	// ostatne child kosti zdedia transformaciu od svojich rodicov
	if (parentMatrix)
		frameBase->matCombined *= (*parentMatrix);

	// rekurzia pre surodencov
	if (frameBase->pFrameSibling)
		SetCombinedMatrices ((D3DXFRAME_DERIVED*)frameBase->pFrameSibling, parentMatrix);
	// rekurzia pre synov
	if (frameBase->pFrameFirstChild)
		SetCombinedMatrices ((D3DXFRAME_DERIVED*)frameBase->pFrameFirstChild, &frameBase->matCombined);
}

// pohybuje meshom - prehrava animacie a nastavuje matCombined matice celej hierarchie
//---------------------------------
void CP3DMeshHierarchy::MeshMove (float elapsedTime, const P3DMatrix &matWorld)
{
	//prehranie animacie
	if (elapsedTime)
		if (m_animController)
		{
			// TODO: pridat profiling
			m_animController->AdvanceTime (elapsedTime, NULL);		//advance the time
			m_currentTime += elapsedTime;
		}

	//update the model matrices
	SetCombinedMatrices (m_pFrameRoot, (D3DXMATRIX*)&matWorld);
}


//---------------------------------
void CP3DMeshHierarchy::SetShaderParameters (DWORD dw7mtMaterialID, const D3DXFRAME_DERIVED *pFrame, DWORD dwMeshMaterialID, SHADER_DYNLIGHT_INPUT *pShaderDynlightsInput)
{
	WORD wAllParams, wParamsToSet;
	int	intNumBones;
	DWORD dwFreeMatrixIdx = 0;
	const D3DXMESHCONTAINER_DERIVED *pContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;
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
			g_paramTable[i].pValue = (void*)&pFrame->matCombined;
			break;
		case SEM_WorldViewProjection:
			D3DXMatrixMultiply (&g_matBuf[dwFreeMatrixIdx], &pFrame->matCombined, &g_matViewProj); 
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

		// skinning
		case SEM_NumBones:
			intNumBones = max (0, int(pContainer->NumInfl - 1));
			g_paramTable[i].pValue = &intNumBones;		// bones count
			break;
		case SEM_SkinMatrixArray:						// bones matrix array
			// pouzitie cez g_pMaterialManager->SetMatrixArray()
			if (pContainer->NumAttributeGroups == 1)
			{
				DWORD dwMatrixIndex;
				LPD3DXBONECOMBINATION pBoneComb;

				pBoneComb = (LPD3DXBONECOMBINATION)pContainer->pBoneCombinationBuf->GetBufferPointer();
				// calculate all the world matrices
				for (DWORD dwPaletteEntry = 0; dwPaletteEntry < pContainer->NumPaletteEntries; dwPaletteEntry++)
					if ((dwMatrixIndex = pBoneComb->BoneId[dwPaletteEntry]) != UINT_MAX)
						g_SkinShaderInputMatrixArray[dwPaletteEntry] = *(P3DMatrix*)&(pContainer->pBoneInverseMatrices[dwMatrixIndex] * (*pContainer->ppBoneTransformMatrices[dwMatrixIndex]));

				if (pContainer->NumPaletteEntries)
				{
					g_pMaterialManager->SetMatrixArray (dw7mtMaterialID, g_paramTable[i].paramHandle, g_SkinShaderInputMatrixArray, pContainer->NumPaletteEntries);
					// posun parametrov v tabulke
					if (int(wAllParams - i - 1) > 0)
						memcpy (&g_paramTable[i], &g_paramTable[i+1], (wAllParams-i- 1) * sizeof(MATERIAL_PARAMETER));
					i--; wAllParams--; wParamsToSet--;
				}
			}
			// TODO: debug - odstranit !!!
			else
				MessageBox (NULL, "pContainer->NumAttributeGroups != 1", "Error", 0);
			// NumAttributeGroups je vacsie ako 1 ak max pocet kosti nepostacuje pre skinning (pravdepodobne v ramci jedneho materialu)
			// debug !!!

/*
			// pouzitie cez g_pMaterialManager->SetParameters()
			if (pContainer->NumAttributeGroups == 1)
			{
				LPD3DXBONECOMBINATION pBoneComb;
				DWORD dwMatrixIndex;
				pBoneComb = (LPD3DXBONECOMBINATION)pContainer->pBoneCombinationBuf->GetBufferPointer();
				// calculate all the world matrices
				for (DWORD dwPaletteEntry = 0; dwPaletteEntry < pContainer->NumPaletteEntries; dwPaletteEntry++)
				{
					if ((dwMatrixIndex = pBoneComb->BoneId[dwPaletteEntry]) != UINT_MAX)
						g_SkinShaderInputMatrixArray[dwPaletteEntry] = *(P3DMatrix*)&(pContainer->pBoneInverseMatrices[dwMatrixIndex] * (*pContainer->ppBoneTransformMatrices[dwMatrixIndex]));
				}
				// FIXME: TODO: asi by bolo rychlejsie dodavat matrix array funkciou effect->SetMatrixArray() ???
				g_paramTable[i].pValue = g_SkinShaderInputMatrixArray;
			}
			// TODO: debug - odstranit !!!
			else
				MessageBox (NULL, "pContainer->NumAttributeGroups != 1", "Error", 0);
			// NumAttributeGroups je vacsie ako 1 ak max pocet kosti nepostacuje pre skinning (pravdepodobne v ramci jedneho materialu)
			// debug !!!
*/
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
			D3DXMatrixMultiply (&g_matBuf[dwFreeMatrixIdx], &pFrame->matCombined, &g_matView);
			g_paramTable[i].pValue = &g_matBuf[dwFreeMatrixIdx];
			dwFreeMatrixIdx++;
			break;
		}
	}

	g_pMaterialManager->SetParameters (dw7mtMaterialID, g_paramTable, wAllParams);
}

// funkcia renderuje cely mesh
//---------------------------------
void CP3DMeshHierarchy::Render(bool bIsDynLighted, SHADER_DYNLIGHT_INPUT &shader_input)
{
	if (m_LoadFlags == MESH_LOAD_HIERARCHY)
		DrawFrame (m_pFrameRoot, bIsDynLighted, shader_input);
	else if (m_LoadFlags == MESH_LOAD_SKINNED)
		DrawFrameSkinned (m_pFrameRoot, bIsDynLighted, shader_input);
}

// nastavit hodnotu 0xFFFFFFFF pre "unforcing back"
//---------------------------------
void CP3DMeshHierarchy::ForceToUseThisMaterial (DWORD dw7mtMaterialID)
{
	m_dwForceToUseThisMaterial = dw7mtMaterialID;
}

// ulozi .x subor spolu so vsetkymi animaciami a "custom" datami
//---------------------------------
bool CP3DMeshHierarchy::Save (const char *filename)
{
	if (filename == NULL) return false;
	CSaveUserData CUserData;
	//D3DXF_FILEFORMAT_BINARY, D3DXF_FILEFORMAT_TEXT
	if (FAILED (D3DXSaveMeshHierarchyToFile (filename, D3DXF_FILEFORMAT_BINARY, m_pFrameRoot, m_animController, &CUserData)))
		return false;
	return true;
}


// rekurzia pre fciu ResetBonesLocalTransforms
//---------------------------------
void ResetBonesLocalTransformRecur (D3DXFRAME_DERIVED *frameBase)
{
	D3DXMatrixIdentity ((D3DXMATRIX*)&frameBase->matLocalTansform);
	frameBase->bIdentityLocalTransform = true;

	if (frameBase->pFrameSibling)
		ResetBonesLocalTransformRecur ((D3DXFRAME_DERIVED*)frameBase->pFrameSibling);
	if (frameBase->pFrameFirstChild)
		ResetBonesLocalTransformRecur ((D3DXFRAME_DERIVED*)frameBase->pFrameFirstChild);
}

// nastavi vsetky matice matLocalTransform v meshi na jednotkove
// je vhodne ju volat pred prehravanim akejkolvek animacie
// samozrejme funkciu nevolat ak sa prave pouziva SetBoneLocalTransform()
//---------------------------------
void CP3DMeshHierarchy::ResetBonesLocalTransforms()
{
	if (m_bLocalTransformsNotSet) return;
	ResetBonesLocalTransformRecur (m_pFrameRoot);
	m_bLocalTransformsNotSet = true;
}


/*fcia SetBoneLocalTransform sa pouziva na nastavenie matice matLocalTansform vo frame szFrameName
program moze pomocou matic matLocalTransform pohybovat castou meshu bez pouzitia animacii
vysledna transformacna matica pre frame sa pocita vo fcii SetCombinedMatrices ako:
matCombined = matLocalTansform * TransformationMatrix * parentMatrix
kde:
matLocalTansform - nastavovana programom vo fcii SetBoneLocalTransform(), je to lokalna transformacia pre cast objektu
TransformationMatrix - nastavovana iba AnimationControlerom
parentMatrix - nastavovana programom, aplikovanie svetovej transformacie pre cely mesh

pre skinned meshe sa transformacie pocitaju inym sposobom!
fciu SetBoneLocalTransform je potrebne volat pred fciou CP3DMeshHierarchy::MeshMove
vacsinou nie je vhodne hybat kostou, ktorou tiez hybe prave beziaca animacia*/
//---------------------------------
void CP3DMeshHierarchy::SetBoneLocalTransform (const char *szFrameName, const P3DMatrix &frameMatrix)
{
	D3DXFRAME_DERIVED *pFrame;
	if ((szFrameName == NULL) || (m_pFrameRoot == NULL)) return;
	if (pFrame = m_pFrameRoot->Find (szFrameName))
	{
		pFrame->matLocalTansform = *((D3DXMATRIX*)&frameMatrix);
		pFrame->bIdentityLocalTransform = false;
		m_bLocalTransformsNotSet = false;
	}
}



//////////////////////////////////////////////////////////////////////////
//								CP3DMeshHierarchy - Animacie
//////////////////////////////////////////////////////////////////////////



// vrati "animation set" podla indexu
//---------------------------------
const LPD3DXANIMATIONSET CP3DMeshHierarchy::GetAnimationSet (DWORD index) const
{
	LPD3DXANIMATIONSET ppAnimSet;
	if (m_animController == NULL) return NULL;
	if (FAILED (m_animController->GetAnimationSet(index, &ppAnimSet)))
		return NULL;
	return ppAnimSet;
}


/*
// vrati "animation set" podla nazvu
//---------------------------------
const LPD3DXANIMATIONSET CP3DMeshHierarchy::GetAnimationSet (char *szAnimsetName) const
{
	LPD3DXANIMATIONSET ppAnimSet;
	if ((m_animController == NULL) || (szAnimsetName == NULL))
		return NULL;
	if (FAILED (m_animController->GetAnimationSetByName (szAnimsetName, &ppAnimSet)))
		return NULL;
	return ppAnimSet;
}
*/



/*
// prida animation set z meshu pSourceMesh do akutalneho meshu
// obaja musia mat rovnaku hierarch. stukturu!
//---------------------------------
bool CP3DMeshHierarchy::AddAnimationSet (CP3DMeshHierarchy *pSourceMesh)
{
	DWORD numToCopy = pSourceMesh->GetNumberOfAnimationSets();

	if (numToCopy==0)
		return true;

	//overenie èi má controller dostatok vo¾ného miesta
	DWORD numPossible = m_animController->GetMaxNumAnimationSets();
	DWORD numAlready = m_animController->GetNumAnimationSets();

	if (numPossible - numAlready < numToCopy)
	{
		//controller nemá dostatok miesta, miesto sa nedá dynamicky zväèšova,
		//je treba vytvori nový controller funkciou CloneAnimationController	
		LPD3DXANIMATIONCONTROLLER newController = NULL;
		if (FAILED( m_animController->CloneAnimationController(
					m_animController->GetMaxNumAnimationOutputs(),
					numAlready+numToCopy,
					2,	// number of required tracks
					m_animController->GetMaxNumEvents(),
					&newController)))
			return false;

		// Release existing controller
		SAFE_RELEASE(m_animController);

		// Now set this to new one just created
		m_animController = newController;
	}

	// Copy all the new animation sets - note: often just one
	for (DWORD i=0; i<numToCopy; i++)
	{
		LPD3DXANIMATIONSET animSet = pSourceMesh->GetAnimationSet(i);
		if (animSet == NULL)
			return false;

		// Add it to our controller
		HRESULT hr = m_animController->RegisterAnimationSet(animSet);
		if (FAILED(hr))
			return false;
	}

	return true;
}
*/


// nastavi pozadovany anim. set do daneho anim. tracku. Ak dany track uz prehrava rovnaku animaciu
// nenastane ziadna zmena, ak prehrava inu animaciu potom plynule prejde do novej animacie
// pre zastavenie animacie v niektorom z anim. trackov je potrebne zadat dwAnimSetIndex = 0xffffffff
//---------------------------------
void CP3DMeshHierarchy::SetAnimation (DWORD dwTrackIndex, DWORD dwAnimSetIndex)
{
	DWORD dwNewTrackIndex, dwOldTrackIndex;

	// kontrola
	if ((dwTrackIndex >= MAX_ANIM_TRACKS_NUM) || (m_animController == NULL))
		return;

	// ziadna zmena
	if (m_TrackTable[dwTrackIndex].dwAnimationIndex == dwAnimSetIndex)
		return;

	// zrusenie animacie
	if (dwAnimSetIndex == 0xffffffff)
	{
		if (m_TrackTable[dwTrackIndex].bHighOrLow)
			dwNewTrackIndex = dwTrackIndex + MAX_ANIM_TRACKS_NUM;
		else
			dwNewTrackIndex = dwTrackIndex;
		m_animController->SetTrackEnable (dwNewTrackIndex, FALSE);
		m_TrackTable[dwTrackIndex].dwAnimationIndex = 0xffffffff;
		return;
	}
	else if (dwAnimSetIndex >= m_dwAnimationSetNum)
		return;		// kontrola

	// true ak je potrebny prechod animacii
	bool bTransitionRequired = (m_TrackTable[dwTrackIndex].dwAnimationIndex != 0xffffffff);
	if (bTransitionRequired)
	{
		if (m_TrackTable[dwTrackIndex].bHighOrLow)
			{dwNewTrackIndex = dwTrackIndex; dwOldTrackIndex = dwTrackIndex + MAX_ANIM_TRACKS_NUM;}
		else
			{dwNewTrackIndex = dwTrackIndex + MAX_ANIM_TRACKS_NUM; dwOldTrackIndex = dwTrackIndex;}
	}
	else
		dwNewTrackIndex = dwTrackIndex;

	// Get the animation set
	LPD3DXANIMATIONSET animset;
	if (FAILED (m_animController->GetAnimationSet (dwAnimSetIndex, &animset)))
		return;
	// Assign to our track
	m_animController->SetTrackAnimationSet (dwNewTrackIndex, animset);
	animset->Release();
	m_animController->SetTrackEnable (dwNewTrackIndex, TRUE);


	// plynuly prechod z jednej anim. na druhu
	if (bTransitionRequired)
	{
		// Clear any track events currently assigned to our two tracks
		m_animController->UnkeyAllTrackEvents (dwNewTrackIndex);
		m_animController->UnkeyAllTrackEvents (dwOldTrackIndex);
		m_animController->SetTrackEnable (dwOldTrackIndex, TRUE);

		// Add an event key to disable the currently playing track DEF_ANIM_TRANSITION_TIME seconds in the future
		m_animController->KeyTrackEnable (dwOldTrackIndex, FALSE, m_currentTime + DEF_ANIM_TRANSITION_TIME );
		// Add an event key to change the speed right away so the animation completes in DEF_ANIM_TRANSITION_TIME seconds
		m_animController->KeyTrackSpeed (dwOldTrackIndex, 0.0f, m_currentTime, DEF_ANIM_TRANSITION_TIME, D3DXTRANSITION_LINEAR );
		// Add an event to change the weighting of the current track (the effect it has blended with the secon track)
		m_animController->KeyTrackWeight (dwOldTrackIndex, 0.0f, m_currentTime, DEF_ANIM_TRANSITION_TIME, D3DXTRANSITION_LINEAR );

		// Add an event key to set the speed of the track
		m_animController->KeyTrackSpeed (dwNewTrackIndex, 1.0f, m_currentTime, DEF_ANIM_TRANSITION_TIME, D3DXTRANSITION_LINEAR );
		// Add an event to change the weighting of the current track (the effect it has blended with the first track)
		// As you can see this will go from 0 effect to total effect(1.0f) in DEF_ANIM_TRANSITION_TIME seconds and the
		// first track goes from total to 0.0f in the same time.
		m_animController->KeyTrackWeight (dwNewTrackIndex, 1.0f, m_currentTime, DEF_ANIM_TRANSITION_TIME, D3DXTRANSITION_LINEAR );
	}
	else
	{
		m_animController->SetTrackSpeed (dwNewTrackIndex, 1.0f);
		m_animController->SetTrackWeight(dwNewTrackIndex, 0.5f);
	}

	// update m_TrackTable tabulky
	if (bTransitionRequired)
		m_TrackTable[dwTrackIndex].bHighOrLow = !m_TrackTable[dwTrackIndex].bHighOrLow;
	m_TrackTable[dwTrackIndex].dwAnimationIndex = dwAnimSetIndex;

}


//---------------------------------
void CP3DMeshHierarchy::ResetTime ()
{
	m_currentTime = 0;
	if (m_animController)
	{
		m_animController->ResetTime();
		m_animController->AdvanceTime (0, NULL);
	}
}

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
#include "eBaseRenderable.h"
#include "common.h"

#include "eBase.h"

static ConVar *pCVr_dyn_lighting=NULL;

void eBaseRenderable::InitBaseRenderable(eBase* thisBase)
{
	////
	base=thisBase;
	////

	modelPath = NULL;
	m_bLoaded = false;
	memset (&m_dynLightingData, 0, sizeof(SHADER_DYNLIGHT_INPUT));
	m_bIsLighted = false;
	m_bDynLightStateChanged = false;
	m_Lods = NULL;
	m_dwNumLods = 0;
	m_bAlphaUsed = false;
	m_dwActualLodIndex = 0;
	m_TransformMatrix.SetIdentityMatrix ();

	UnloadModel();

	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	pCVr_dyn_lighting = g_pConsole->FindConVar("r_dyn_lighting");
}

//---------------------------------
eBaseRenderable::~eBaseRenderable()
{
	UnloadModel();
}

//---------------------------------
bool eBaseRenderable::LoadModel(const char* szModelPath, int paintVer)
{
	UnloadModel();

	modelPath = new char[strlen(szModelPath)+1];
	strcpy(modelPath, szModelPath);

	// test na vstupni parametry
	if (szModelPath == NULL || szModelPath[0]==0)
	{
		g_pConsole->Message(MSG_CON_ERR, "LoadModel(const char* szModelPath, int paintVer): szModelPath == NULL!");
		return false;
	}

	char szPath[MAX_PATH] = "N/A";
	ezxml_t modelXml;

	g_pXml = (IP3DXML*)I_GetClass(IP3DENGINE_XML);
	if (g_pXml == NULL) return false;

	strcpy(szPath, szModelPath);
	if (szPath[strlen(szPath)-1]=='x') 
	{
		g_pConsole->Message(MSG_CON_ERR, "LoadModel(): Model with .x extension (should be .xml)! %s", szPath);
		return false;
	}
	if (!stricmp(&szPath[strlen(szPath)-4], ".jpg")) strcpy(&szPath[strlen(szPath)-4], ".xml");

	modelXml = g_pXml->Parse_file(szPath);
	if (modelXml == NULL)
	{
		g_pConsole->Message(MSG_CON_ERR, "LoadModel(): Can't parse xml description file '%s'!", szPath);
		goto quit_hand;
	}

	if (!LoadFromXml (modelXml, paintVer))
		goto quit_hand;

	g_pXml->Free(modelXml); // ukonèi xml objekt

	// nastavenie sort keys
	UINT nSortMajor, nSortMinor;
	if ((m_Lods == NULL) || (m_dwNumLods == 0) || (m_Lods[0].pMesh == NULL))
		goto quit_hand;
	m_Lods[0].pMesh->GetSortKey (nSortMajor, nSortMinor);
	base->SetSortKeys (nSortMajor, nSortMinor);

	m_bLoaded = true;
	return true;

quit_hand:
	if (modelXml)
		g_pXml->Free(modelXml);
	SAFE_DELETE_ARRAY (m_Lods)
	g_pConsole->Message(MSG_CON_ERR, "LoadModel(): Error while loading %s model!", szPath);
	return false;
}

//---------------------------------
bool eBaseRenderable::UnloadModel()
{
	SAFE_DELETE_ARRAY (modelPath);
	SAFE_DELETE_ARRAY (m_Lods)
	m_bLoaded = false;
	return true;
}

//---------------------------------
void eBaseRenderable::RecalculateLightingAtNewPosition ()
{
	if (!m_bLoaded) return; // tanhle entita nema model...

	// tahle metoda se vola jen pri zmene pozice, takze staci tady nastavovat novou pozici meshe
	base->GetWorldMatrix(m_TransformMatrix);

	// urcenie aktualneho osvetlenia podla pozicie objektu
	g_pBSPMap->GetLightFromPoint (m_TransformMatrix.m_posit3, m_cellLightInfo);
}

//---------------------------------
bool eBaseRenderable::LoadFromXml (const ezxml_t modelXml, DWORD dwUseThisPainting)
{
	float v1, v2, v3; DWORD dwBuf; const char *szBuf; // pomocné promìnné
	ezxml_t part; ezxml_t partChild;
	P3DXMatrix matTransform;
	P3DSphere m_boundsphereBase;

	g_pMeshLoader = (IP3DMeshLoader*)I_GetClass(IP3DRENDERER_MESHLOADER);
	if ((!modelXml) || (g_pMeshLoader == NULL)) return false;

	// model type solid
	part = g_pXml->Child(modelXml, "model");
	if ((szBuf = g_pXml->Attr(part, "type")) == NULL) return false;
	if (stricmp (szBuf, "solid"))
		return false;

	// pouzivaju sa alpha materialy?
	if (szBuf = g_pXml->Attr(part, "alpha_used"))
	{
		int intBuf = atoi (szBuf);
		m_bAlphaUsed = (intBuf!=0);
	}
	// nastavenie bounding boxu
	if (partChild = g_pXml->Child(part, "AABB"))
	{
		float x, y, z;
		if (szBuf = g_pXml->Attr(partChild, "x")) x = (float) atof(szBuf)/2.0f;
		if (szBuf = g_pXml->Attr(partChild, "y")) y = (float) atof(szBuf)/2.0f;
		if (szBuf = g_pXml->Attr(partChild, "z")) z = (float) atof(szBuf)/2.0f;
		P3DAABB aabb;
		aabb.min.x = -x; aabb.min.y = -y; aabb.min.z = -z; 
		aabb.max.x = x; aabb.max.y = y; aabb.max.z = z; 
		base->SetRelAABB(aabb);
	}
	// nastavenie bounding sphere
	if (partChild = g_pXml->Child(part, "boundingsphere"))
	{
		if (szBuf = g_pXml->Attr(partChild, "r"))
			m_boundsphereBase.fRadius = (float) atof(szBuf);
		if (partChild = g_pXml->Child(partChild, "offset"))
		{
			sscanf (part->txt, "%f %f %f", &v1, &v2, &v3);
			m_boundsphereBase.pointOrigin.x = v1;
			m_boundsphereBase.pointOrigin.y = v2;
			m_boundsphereBase.pointOrigin.z = v3;
		}
		else
			memset (&m_boundsphereBase.pointOrigin, 0, sizeof(P3DXVector3D));

		base->SetRelSphere(m_boundsphereBase);
	}

	// nacitanie Lod
	ezxml_t lod;
	if ((part = g_pXml->Child(modelXml, "lods")) == NULL) 
	{
		CON(MSG_CON_ERR, "XML model without LOD block!");
		return false;
	}
	for (dwBuf = 0, lod = g_pXml->Child(part, "lod"); lod; lod = lod->next)
		dwBuf++;
	m_Lods = new LOD_MESH_TABLE[dwBuf];
	if (m_Lods == NULL) return false;

	ezxml_t pXmlTextures = g_pXml->Child(modelXml, "textures");
	for (dwBuf = 0, lod = g_pXml->Child(part, "lod"); lod; lod = lod->next, dwBuf++)
	{
		if (szBuf = g_pXml->Attr(lod, "distance"))
			m_Lods[dwBuf].fmaxDistance = (float)atof (szBuf);

		// MESH_LOAD, MESH_LOAD_HIERARCHY, MESH_LOAD_SKINNED
		m_Lods[dwBuf].pMesh = g_pMeshLoader->LoadMesh (lod->txt, MESH_LOAD, pXmlTextures, dwUseThisPainting);

		if (m_Lods[dwBuf].pMesh == NULL)		// debug info
			g_pConsole->Message(MSG_CON_ERR, "LoadModel(): Can't load %i. lod-version of model '%s'!", dwBuf, lod->txt);
		else
			m_Lods[dwBuf].pMesh->SetLightInfoPointer (&m_cellLightInfo);
	}
	m_dwNumLods = dwBuf;

	base->GetWorldPos(m_TransformMatrix.m_posit3);
	base->RecalculateIntersectClusters(); // mame novou sphere
	RecalculateLightingAtNewPosition();	// urcenie osvetlenia


	if (m_bAlphaUsed)
	{
		P3DSphere sph;
		base->GetRelSphere(sph);
		m_dwAlphaModelID = g_pAlphaMan->CreateDynamicModel (m_Lods[0].pMesh, sph);
		if (m_dwAlphaModelID == 0xFFFFFFFF)
		{
			CON(MSG_CON_ERR, "Can't assign transparent renderable model to aplha manager!");
			return false;
		}
	}

	return true;
}

void eBaseRenderable::RenderableRender(float deltaTime)
{
	if (!m_bLoaded) return;

	//////////////////////////////////////////////////////////////////////////
	// update dynamic lighting info
	if (pCVr_dyn_lighting->GetBool())
	{
		if (m_bIsLighted || (!m_bDynLightStateChanged))
			m_bIsLighted = g_pLightManager->GetModelLightInfo (m_boundsphereActual, m_dynLightingData);
		else if (base->WasMatrixChanged())
		{
			P3DXVector3D vecPosDiff(m_LastDeltaPosition);
			vecPosDiff -= m_TransformMatrix.m_posit3;
			// 15cm ako minimalna zmena pozicie na prepocitanie dyn. osvetlenia
			if (vecPosDiff.Abs().GetMax() > 15.0f)
			{
				m_bIsLighted = g_pLightManager->GetModelLightInfo (m_boundsphereActual, m_dynLightingData);
				m_LastDeltaPosition = m_TransformMatrix.m_posit3;
			}
		}
		m_bDynLightStateChanged = true;
	}
	else
	{m_bIsLighted = false; m_bDynLightStateChanged = false;}

	// urcenie spravnej lod-verzie
	P3DXVector3D pointTemp;
	float fFOV, fDistanceFromCamera;
	g_pRenderer->GetCameraInfo (pointTemp, fFOV);
	pointTemp -= m_TransformMatrix.m_posit3;
	// pri urceni lod-verzie sa zapocitava aj aktualna hodnota FOV, iba odhad, ak treba tak upravit
	fDistanceFromCamera = pointTemp.Length() * fFOV * (1.0f / DEFAULT_FIELD_OF_VIEW);
	m_dwActualLodIndex = 0;
	for (DWORD j=m_dwNumLods; j>0; --j)
		if (fDistanceFromCamera > m_Lods[j-1].fmaxDistance)
			{m_dwActualLodIndex = j; break;}
	if (m_dwActualLodIndex >= m_dwNumLods) return;	// objekt sa nachadza za maximalnou hranicou, nevykresluje sa
	if (m_Lods[m_dwActualLodIndex].pMesh == NULL) return; // OPTIM: lze nacist nejaky maly model symoblizujici chybejici model

	if (m_bAlphaUsed) // rendering is done by alpha manager
	{
		g_pAlphaMan->UpdateDynamicModel(m_dwAlphaModelID, m_Lods[m_dwActualLodIndex].pMesh, m_TransformMatrix, base->IsVisible(),
			m_bIsLighted, m_dynLightingData);
		return;
	}

	// set current matrix for rendering
	m_Lods[m_dwActualLodIndex].pMesh->MeshMove (m_TransformMatrix);

	m_Lods[m_dwActualLodIndex].pMesh->Render (m_bIsLighted, m_dynLightingData);
}
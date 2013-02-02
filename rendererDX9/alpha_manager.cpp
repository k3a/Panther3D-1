//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Alpha manager class definition
//-----------------------------------------------------------------------------

#include "alpha_manager.h"


REGISTER_SINGLE_CLASS (CP3DAlphaManager, IP3DRENDERER_ALPHAMANAGER); // zaregistruj tuto tøídu

D3DXVECTOR4		sorted_quad_light_size[MAX_GRASS_BILLBOARDS];
D3DXVECTOR4		sorted_quad_position_rot[MAX_GRASS_BILLBOARDS];


//---------------------------------
CP3DAlphaManager::CP3DAlphaManager()
{
	m_pObjects = NULL;
	m_dwMaxObjectCount = 0;
	m_dwVisibleObjects = 0;
	m_pSortTable = NULL;
	m_pWindowShader = NULL;
	m_pWindowParams[0] = NULL; m_pWindowParams[1] = NULL; m_pWindowParams[2] = NULL;
	m_pGrassParams[0] = NULL; m_pGrassParams[1] = NULL; m_pGrassParams[2] = NULL;
	m_pGrassParams[3] = NULL; m_pGrassParams[4] = NULL; m_pGrassParams[5] = NULL; m_pGrassParams[6] = NULL;
	m_pGrassShader = NULL;
	m_pBspMap = NULL;
	g_pPhysEngine = NULL;
	m_pFrustum = NULL;
	m_pWindowsVertDec = NULL;
	m_pGrassVertDec = NULL;
	m_pGrassVertexBuf = NULL;
	m_pGrassIndexBuf = NULL;
}

//---------------------------------
CP3DAlphaManager::~CP3DAlphaManager()
{
	UnloadAll ();
	SAFE_DELETE_ARRAY (m_pObjects)
	SAFE_DELETE_ARRAY (m_pSortTable)
	SAFE_RELEASE (m_pWindowsVertDec)
	SAFE_RELEASE (m_pGrassVertDec)
	SAFE_RELEASE (m_pGrassVertexBuf)
	SAFE_RELEASE (m_pGrassIndexBuf)
	SAFE_RELEASE (m_pWindowShader)
	SAFE_RELEASE (m_pGrassShader)
}

//---------------------------------
void CP3DAlphaManager::UnloadAll()
{
	if (m_pObjects)
	{
		for (DWORD i=0; i<m_dwMaxObjectCount; i++)
			if (m_pObjects[i].pGlassWindow)
			{
				if (m_pObjects[i].type == alphaObj_window)				// window
					SAFE_DELETE (m_pObjects[i].pGlassWindow)
				else if (m_pObjects[i].type == alphaObj_grass)			// grass
					SAFE_DELETE (m_pObjects[i].pGrassBunch)
				else if (m_pObjects[i].type == alphaObj_dynamic_model)	// dynamic model
					SAFE_DELETE (m_pObjects[i].pDynamicModel)
			}
			memset (m_pObjects, 0, sizeof(OBJECT_TABLE) * m_dwMaxObjectCount);
	}
}

//---------------------------------
bool CP3DAlphaManager::ReallocateAlphaRecords ()
{
	if (m_pObjects == NULL)
	{
		m_pObjects = new OBJECT_TABLE[INIT_TABLE_SIZE];
		m_pSortTable = new ALPHA_SORT_TABLE[INIT_TABLE_SIZE];
		if ((m_pObjects == NULL) || (m_pSortTable == NULL))
			return false;
		memset (m_pObjects, 0, sizeof(OBJECT_TABLE) * INIT_TABLE_SIZE);
		m_dwMaxObjectCount = INIT_TABLE_SIZE;
	}
	else
	{
		OBJECT_TABLE		*pNewRecords;
		ALPHA_SORT_TABLE	*pNewSortTable;
		DWORD					dwNewWinCount;
		dwNewWinCount = DWORD(float(m_dwMaxObjectCount) * 1.5f);
		pNewRecords = new OBJECT_TABLE[dwNewWinCount];
		pNewSortTable = new ALPHA_SORT_TABLE[dwNewWinCount];
		if ((pNewRecords == NULL) || (pNewSortTable == NULL))
			return false;
		memset (pNewRecords, 0, sizeof(OBJECT_TABLE) * dwNewWinCount);
		memcpy (pNewRecords, m_pObjects, sizeof(OBJECT_TABLE) * m_dwMaxObjectCount);

		delete[] m_pObjects;
		m_pObjects = pNewRecords;
		delete[] m_pSortTable;
		m_pSortTable = pNewSortTable;
		m_dwMaxObjectCount = dwNewWinCount;
	}

	for (short i=0; i<(short)m_dwMaxObjectCount; i++)
	{
		m_pSortTable[i].Idx = i;
		m_pSortTable[i].fDistance = -1.0f;
	}
	return true;
}

//---------------------------------
bool CP3DAlphaManager::Init()
{
	if (!ReallocateAlphaRecords())
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Failed to reallocate AplhaRecords!");
		return false;
	}

	// ziskanie singletonov NewtonWorld a BSPMap
	m_pBspMap = (IP3DBSPMap*)I_GetClass(IP3DRENDERER_BSPMAP);
	g_pPhysEngine = (IP3DPhysEngine*)I_GetClass(IP3DPHYS_PHYSENGINE);
	m_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM);
	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	if ((m_pBspMap == NULL) || (g_pPhysEngine == NULL) || (m_pFrustum == NULL) || (g_pMaterialManager == NULL))
		return false;

	// nacteni do pameti
	BYTE *tmpData; ULONG tmpDataSize=0;
	FSFILE* tmpFile;
	tmpFile = g_pFS->Load("shaders\\spec_glass_window.fx", tmpData, tmpDataSize);
	if (!tmpFile) 
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Can't load spec_glass_window.fx shader to memory!");
		return false;
	}

	// nacitanie window efektu
	if (FAILED (D3DXCreateEffectEx (g_pD3DDevice, tmpData, (UINT)tmpDataSize, \
		NULL, NULL, NULL, D3DXFX_NOT_CLONEABLE, (LPD3DXEFFECTPOOL)g_pMaterialManager->GetPoolHandle (), \
		&m_pWindowShader, NULL)))
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Can't compile spec_glass_window.fx effect!");
		g_pFS->UnLoad(tmpFile, tmpData);
		return false;
	}

	g_pFS->UnLoad(tmpFile, tmpData);


	m_pWindowParams[0] = m_pWindowShader->GetParameterBySemantic (NULL, "AnyValue");
	m_pWindowParams[1] = m_pWindowShader->GetParameterBySemantic (NULL, "TextureLayer0");
	m_pWindowParams[2] = m_pWindowShader->GetParameterBySemantic (NULL, "TextureCubeMap");

	// nacti do pameti
	tmpFile = g_pFS->Load("shaders\\spec_grass.fx", tmpData, tmpDataSize);
	if (!tmpFile) 
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Can't load spec_grass.fx effect to memory!");
		return false;
	}

	// nacitanie grass efektu
	if (FAILED (D3DXCreateEffectEx (g_pD3DDevice, tmpData, (UINT)tmpDataSize, \
		NULL, NULL, NULL, D3DXFX_NOT_CLONEABLE, (LPD3DXEFFECTPOOL)g_pMaterialManager->GetPoolHandle (), \
		&m_pGrassShader, NULL)))
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Can't compile spec_grass.fx effect!");
		g_pFS->UnLoad(tmpFile, tmpData);
		return false;
	}

	g_pFS->UnLoad(tmpFile, tmpData);


	m_pGrassParams[0] = m_pGrassShader->GetParameterBySemantic (NULL, "LightSize");
	m_pGrassParams[1] = m_pGrassShader->GetParameterBySemantic (NULL, "QuadPositionRot");
	m_pGrassParams[2] = m_pGrassShader->GetParameterBySemantic (NULL, "GrassTexture");
	m_pGrassParams[3] = m_pGrassShader->GetParameterBySemantic (NULL, "Billboarded");
	m_pGrassParams[4] = m_pGrassShader->GetParameterBySemantic (NULL, "Animated");
	m_pGrassParams[5] = m_pGrassShader->GetParameterBySemantic (NULL, "Time");
	m_pGrassParams[6] = m_pGrassShader->GetParameterBySemantic (NULL, "SquaredFadeDistance");

	DWORD dwVertVer, dwPixVer;
	g_pMaterialManager->GetShaderVersion (dwVertVer, dwPixVer);
	STAT(STAT_SHADER_SET_TECHNIQUE, 1);
	if (D3DSHADER_VERSION_MAJOR (dwVertVer) >= 3)
		m_pGrassShader->SetTechnique ("spec_grass_model3");
	else
		m_pGrassShader->SetTechnique ("spec_grass_model2");

	// vytvorenie VB a IB pre grass billboardy
	if (FAILED (g_pD3DDevice->CreateVertexBuffer (sizeof(GRASS_VERTEX_FORMAT) * 4 * MAX_GRASS_BILLBOARDS, \
		0, GRASS_FVF, D3DPOOL_MANAGED, &m_pGrassVertexBuf, NULL)))
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Failed to create grass vertex buffer!");
		return false;
	}

	void	*pData;
	GRASS_VERTEX_FORMAT	*pVertexData;
	DWORD	i;

	GRASS_VERTEX_FORMAT	grass_quad_buf[4] = {	{P3DXVector3D(-0.5f, 1.0f, 0.0f), 0.0f, 0.0f},
	{P3DXVector3D(0.5f, 1.0f, 0.0f), 1.0f, 0.0f},
	{P3DXVector3D(-0.5f, 0.0f, 0.0f), 0.0f, 1.0f},
	{P3DXVector3D(0.5f, 0.0f, 0.0f), 1.0f, 1.0f}
	};

	if (SUCCEEDED (m_pGrassVertexBuf->Lock (0, 0, &pData, 0)))
	{
		pVertexData = (GRASS_VERTEX_FORMAT*)pData;
		for (i=0; i<MAX_GRASS_BILLBOARDS; i++)
		{
			pVertexData[i*4] = grass_quad_buf[0];
			pVertexData[i*4].position.z = float(i);	// z sa pouziva ako index pre shader instancing
			pVertexData[i*4+1] = grass_quad_buf[1];
			pVertexData[i*4+1].position.z = float(i);
			pVertexData[i*4+2] = grass_quad_buf[2];
			pVertexData[i*4+2].position.z = float(i);
			pVertexData[i*4+3] = grass_quad_buf[3];
			pVertexData[i*4+3].position.z = float(i);
		}
		m_pGrassVertexBuf->Unlock ();
	}

	// index buffer - grass
	if (FAILED (g_pD3DDevice->CreateIndexBuffer (sizeof(WORD) * 6 * MAX_GRASS_BILLBOARDS, \
		0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pGrassIndexBuf, NULL)))
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Failed to create index buffer!");
		return false;
	}
	if (SUCCEEDED (m_pGrassIndexBuf->Lock (0, 0, &pData, 0)))
	{
		unsigned short *pIndexData = (unsigned short*)pData;
		for (i=0; i<MAX_GRASS_BILLBOARDS; i++)
		{
			pIndexData[i*6] = (unsigned short) i*4;
			pIndexData[i*6+1] = (unsigned short) i*4 + 1;
			pIndexData[i*6+2] = (unsigned short) i*4 + 2;
			pIndexData[i*6+3] = (unsigned short) i*4 + 2;
			pIndexData[i*6+4] = (unsigned short) i*4 + 1;
			pIndexData[i*6+5] = (unsigned short) i*4 + 3;
		}
		m_pGrassIndexBuf->Unlock ();
	}


	// window vertex declarations
	D3DVERTEXELEMENT9	vertex_elements[4] = {0};
	vertex_elements[0].Offset = 0;
	vertex_elements[0].Type = D3DDECLTYPE_FLOAT3;
	vertex_elements[0].Usage = D3DDECLUSAGE_POSITION;
	vertex_elements[1].Offset = sizeof (float) * 3;
	vertex_elements[1].Type = D3DDECLTYPE_FLOAT3;
	vertex_elements[1].Usage = D3DDECLUSAGE_NORMAL;
	vertex_elements[2].Offset = sizeof (float) * 6;
	vertex_elements[2].Type = D3DDECLTYPE_FLOAT2;
	vertex_elements[2].Usage = D3DDECLUSAGE_TEXCOORD;
	// ukoncovaci element
	vertex_elements[3].Stream = 0xFF;
	vertex_elements[3].Type = D3DDECLTYPE_UNUSED;

	if (FAILED (g_pD3DDevice->CreateVertexDeclaration (vertex_elements, &m_pWindowsVertDec)))
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Failed to create windows vertex declaration!");
		return false;
	}

	// grass vertex declarations
	memset (vertex_elements, 0, sizeof(vertex_elements));
	vertex_elements[0].Offset = 0;
	vertex_elements[0].Type = D3DDECLTYPE_FLOAT3;
	vertex_elements[0].Usage = D3DDECLUSAGE_POSITION;
	vertex_elements[1].Offset = sizeof (float) * 3;
	vertex_elements[1].Type = D3DDECLTYPE_FLOAT2;
	vertex_elements[1].Usage = D3DDECLUSAGE_TEXCOORD;
	// ukoncovaci element
	vertex_elements[2].Stream = 0xFF;
	vertex_elements[2].Type = D3DDECLTYPE_UNUSED;

	if (FAILED (g_pD3DDevice->CreateVertexDeclaration (vertex_elements, &m_pGrassVertDec)))
	{
		CON(MSG_ERR_FATAL, "AlphaManager: Failed to create gress vertex declaration!");
		return false;
	}

	return true;
}

//---------------------------------
bool CP3DAlphaManager::CreateGlassWindow (P3DXVector3D *points, char* szAlphaTexture, char* szCubeMapTexture)
{
	if (g_pD3DDevice == NULL)
		return false;

	// najdenie volneho prvku
	P3DXVector3D	BBmin, BBmax;
	DWORD	i, dwFreeIdx = 0xFFFFFFFF;
loop0:
	for (i=0; i<m_dwMaxObjectCount; i++)
		if (m_pObjects[i].pGlassWindow == NULL)
			{dwFreeIdx = i; break;}
	if (dwFreeIdx == 0xFFFFFFFF)
		if (ReallocateAlphaRecords ())
			goto loop0;
		else
		{
			CON(MSG_CON_ERR, "AlphaManager: Failed to reallocate AlphaRecords!");
			return false;
		}

	if ((m_pObjects[dwFreeIdx].pGlassWindow = new GLASS_WINDOW) == NULL)
	{
		CON(MSG_CON_ERR, "AlphaManager: Failed to allocate memory for a new glass window object!");
		return false;
	}
	memset (m_pObjects[dwFreeIdx].pGlassWindow, 0, sizeof(GLASS_WINDOW));

	// nastavenie vertex dat
	GWIN_VERTEX_FORMAT	VertexData[4];
	VertexData[0].position = points[3];
	VertexData[0].u = 0.0f; VertexData[0].v = 0.0f;
	VertexData[1].position = points[0];
	VertexData[1].u = 1.0f; VertexData[1].v = 0.0f;
	VertexData[2].position = points[1];
	VertexData[2].u = 0.0f; VertexData[2].v = 1.0f;
	VertexData[3].position = points[2];
	VertexData[3].u = 1.0f; VertexData[3].v = 1.0f;
	// nastavenie normal
	BBmin = VertexData[1].position - VertexData[0].position;
	BBmax = VertexData[2].position - VertexData[0].position;
	BBmin %= BBmax;
	BBmin.Normalize ();		// normala vertexov
	VertexData[0].normal = BBmin;
	VertexData[1].normal = BBmin;
	VertexData[2].normal = BBmin;
	VertexData[3].normal = BBmin;

	if (FAILED (g_pD3DDevice->CreateVertexBuffer (sizeof(GWIN_VERTEX_FORMAT) * 4, 0, GWIN_FVF, \
		D3DPOOL_MANAGED, &m_pObjects[dwFreeIdx].pGlassWindow->pVB, NULL)))
	{
		SAFE_DELETE (m_pObjects[dwFreeIdx].pGlassWindow)
			CON(MSG_CON_ERR, "AlphaManager: Failed to create glass vertex buffer!");
		return false;
	}

	void *pVertexData;
	if (FAILED (m_pObjects[dwFreeIdx].pGlassWindow->pVB->Lock (0, 0, &pVertexData, 0)))
	{
		m_pObjects[dwFreeIdx].pGlassWindow->pVB->Release ();
		SAFE_DELETE (m_pObjects[dwFreeIdx].pGlassWindow)
			CON(MSG_CON_ERR, "AlphaManager: Failed to lock glass vertex buffer!");
		return false;
	}

	memcpy (pVertexData, VertexData, sizeof(GWIN_VERTEX_FORMAT) * 4);
	m_pObjects[dwFreeIdx].pGlassWindow->pVB->Unlock ();

	// nacitanie textur
	if (szAlphaTexture)
		m_pObjects[dwFreeIdx].pGlassWindow->p_Texture = g_TextureLoader.CreateTexture (szAlphaTexture);
	if (szCubeMapTexture)
		m_pObjects[dwFreeIdx].pGlassWindow->p_CubeTexture = g_TextureLoader.CreateCubeTexture (szCubeMapTexture);

	// urcenie bounding sphere
	P3DSphere bSphere;
	BBmin.x = 1000000; BBmin.y = 1000000; BBmin.z = 1000000;
	BBmax.x = -1000000; BBmax.y = -1000000; BBmax.z = -1000000;
	for (i=0; i<4; i++)
	{
		BBmin.x = min (BBmin.x, VertexData[i].position.x);
		BBmin.y = min (BBmin.y, VertexData[i].position.y);
		BBmin.z = min (BBmin.z, VertexData[i].position.z);
		BBmax.x = max (BBmax.x, VertexData[i].position.x);
		BBmax.y = max (BBmax.y, VertexData[i].position.y);
		BBmax.z = max (BBmax.z, VertexData[i].position.z);
	}
	BBmax = (BBmax - BBmin) * 0.5f;
	bSphere.pointOrigin = BBmin + BBmax;

	float fBuf, fMaxLen = -1.0f;
	for (i=0; i<4; i++)
	{
		BBmax = bSphere.pointOrigin;
		BBmax -= VertexData[i].position;
		fBuf = BBmax.Length ();
		if (fBuf > fMaxLen) fMaxLen = fBuf;
	}
	bSphere.fRadius = fMaxLen;

	// bounding sphere pre sorting a frustum culling
	m_pObjects[dwFreeIdx].boundSphere = bSphere;

	m_pBspMap->CalculateIntersectClusters (bSphere, m_pObjects[dwFreeIdx].clusterList);

	// vytvorenie fyzikalnej kolizie
	m_pObjects[dwFreeIdx].pGlassWindow->p_RigidBody = (IP3DPhysBody*)I_GetClass(IP3DPHYS_PHYSBODY);

	// urcenie uhlov podla normaly, pouziva sa iba rotacia okolo Y a rotacia okolo specifickej osi
	// WARN: mozno nebude vzdy fungovat spravne
	float fAngleY, fAngle2;
	P3DXVector3D	rotAxis;
	D3DXVECTOR3		DXrotAxis;
	BBmin = VertexData[0].normal;
	BBmin.y = 0;
	BBmin.Normalize ();
	fAngleY = acosf (BBmin * P3DXVector3D(0.0f, 0.0f, -1.0f));	// uhol rotacie okolo Y
	fAngle2 = acosf (BBmin * VertexData[0].normal);				// uhol rotacie okolo danej osi
	rotAxis = (BBmin % P3DXVector3D(0.0f, 1.0f, 0.0f));			// os druhej rotacie
	DXrotAxis.x = rotAxis.x;
	DXrotAxis.y = rotAxis.y;
	DXrotAxis.z = rotAxis.z;

	D3DXMATRIX	mat, matBuf;
	D3DXMatrixRotationY (&mat, fAngleY);
	D3DXMatrixRotationAxis (&matBuf, &DXrotAxis, fAngle2);
	mat *= matBuf;
	D3DXMatrixTranslation (&matBuf, bSphere.pointOrigin.x, bSphere.pointOrigin.y, bSphere.pointOrigin.z);
	mat *= matBuf;		// vysledna matica

	// urcenie rozmerov boxu
	P3DXVector3D	PhysBoxDims;
	BBmin = VertexData[0].position - VertexData[1].position;
	PhysBoxDims.x = BBmin.Length() - 10.0f;		// FIXME: ubrat z okrajov 5cm ???
	BBmin = VertexData[0].position - VertexData[2].position;
	PhysBoxDims.y = BBmin.Length() - 10.0f;
	PhysBoxDims.z = 3.0f;		// 3 cm

	PhysShapeDesc desc;
	m_pObjects[dwFreeIdx].pGlassWindow->p_RigidBody->AddShapeBox(desc, PhysBoxDims.x, PhysBoxDims.y, PhysBoxDims.z);
	m_pObjects[dwFreeIdx].pGlassWindow->p_RigidBody->SetMatrix ((P3DXMatrix&)mat);

	m_pObjects[dwFreeIdx].type = alphaObj_window;
	return true;
}

//---------------------------------
bool CP3DAlphaManager::CreateGrassBunch (P3DXVector3D origin, float fRadius, WORD quadCount, \
										 float fFadeDistance, float fMinGrassQuadSize, float fMaxGrassQuadSize, \
										 char *szTexture, bool bBillboarded, bool bAnimated)
{
	if ((g_pD3DDevice == NULL) || (szTexture == NULL))
		return false;

	// najdenie volneho prvku
	GRASS_BUNCH		*pGrassBunchOut;
	P3DXVector3D	BBmin, BBmax;
	DWORD	i, dwFreeIdx = 0xFFFFFFFF;
loop1:
	for (i=0; i<m_dwMaxObjectCount; i++)
		if (m_pObjects[i].pGlassWindow == NULL)
			{dwFreeIdx = i; break;}
	if (dwFreeIdx == 0xFFFFFFFF)
		if (ReallocateAlphaRecords ())
			goto loop1;
		else
		{
			CON(MSG_CON_ERR, "AlphaManager: Failed to reallocate AlphaRecords!");
			return false;
		}

	if ((pGrassBunchOut = new GRASS_BUNCH) == NULL)
	{
		CON(MSG_CON_ERR, "AlphaManager: Can't allocate memory for new grass bunch!");
		return false;
	}
	// clamping
	quadCount = min (quadCount, MAX_GRASS_BILLBOARDS);
	quadCount = max (quadCount, MIN_GRASS_BILLBOARDS);
	fRadius = min (fRadius, MAX_GRASS_RADIUS);
	fRadius = max (fRadius, MIN_GRASS_RADIUS);

	pGrassBunchOut->quad_light_size = new D3DXVECTOR4[quadCount];
	pGrassBunchOut->quad_position_rot = new D3DXVECTOR4[quadCount];
	pGrassBunchOut->sort_list = new ALPHA_SORT_TABLE[quadCount];
	pGrassBunchOut->wQuadCount = quadCount;
	pGrassBunchOut->bBillboarded = bBillboarded;
	pGrassBunchOut->bAnimated = bAnimated;
	pGrassBunchOut->fSquaredFadeDistance = fFadeDistance * fFadeDistance;
	pGrassBunchOut->fSquaredFadeDistanceVisTest = (fFadeDistance + fRadius) * (fFadeDistance + fRadius);

	if ((pGrassBunchOut->quad_light_size == NULL) || (pGrassBunchOut->quad_position_rot == NULL) || \
			(pGrassBunchOut->sort_list == NULL))
		{delete pGrassBunchOut; CON(MSG_CON_ERR, "AlphaManager: Can't allocate quad memory!"); return false;}

	// bounding sphere
	m_pObjects[dwFreeIdx].boundSphere.pointOrigin = origin;
	m_pObjects[dwFreeIdx].boundSphere.fRadius =  fRadius + fMaxGrassQuadSize;

	// cluster list
	m_pBspMap->CalculateIntersectClusters (m_pObjects[dwFreeIdx].boundSphere, m_pObjects[dwFreeIdx].clusterList);

	// nacitanie textury
	pGrassBunchOut->p_Texture = g_TextureLoader.CreateTexture (szTexture);
	if (pGrassBunchOut->p_Texture == NULL) 
	{
		CON(MSG_CON_ERR, "AlphaManager: Can't load grass texture %s!", szTexture);
		return false;
	}

	BSP_LIGHTCELL_INFO	light_cell;
	for (i=0; i<quadCount; i++)
	{
		// quad_position_rot
		pGrassBunchOut->quad_position_rot[i].x = origin.x + (GetRndValue() * 2.0f - 1.0f) * fRadius;
		pGrassBunchOut->quad_position_rot[i].y = origin.y;
		pGrassBunchOut->quad_position_rot[i].z = origin.z + (GetRndValue() * 2.0f - 1.0f) * fRadius;
		// nahodne natocenie statickeho quadu
		pGrassBunchOut->quad_position_rot[i].w = GetRndValue() * P3D_PI;
		// falling onto ground
		IP3DPhysBody	*pRigidBody;
		P3DXVector3D		pointStart, pointEnd;
		float				fIntersectParam;
		pointStart.x = pGrassBunchOut->quad_position_rot[i].x;
		pointStart.y = pGrassBunchOut->quad_position_rot[i].y;
		pointStart.z = pGrassBunchOut->quad_position_rot[i].z;
		pointEnd = pointStart;
		pointEnd.y -= 1000.0f;	// 10m smerom nadol

		pRigidBody = g_pPhysEngine->GetNearestWorldRayCastHit (pointStart, pointEnd, fIntersectParam);
		if (pRigidBody)
			pGrassBunchOut->quad_position_rot[i].y -= (pointStart.y - pointEnd.y) * fIntersectParam;

		// quad_light_size
		pGrassBunchOut->quad_light_size[i] = D3DXVECTOR4 (1.0f, 1.0f, 1.0f, 1.0f);
		pGrassBunchOut->quad_light_size[i].w = fMinGrassQuadSize + ((fMaxGrassQuadSize - fMinGrassQuadSize) * GetRndValue());
		P3DXVector3D	lighPoint;
		lighPoint.x = pGrassBunchOut->quad_position_rot[i].x;
		lighPoint.y = pGrassBunchOut->quad_position_rot[i].y + (pGrassBunchOut->quad_light_size[i].w / 2);
		lighPoint.z = pGrassBunchOut->quad_position_rot[i].z;

		if (m_pBspMap->GetLightFromPoint (lighPoint, light_cell))
		{
			pGrassBunchOut->quad_light_size[i].x = light_cell.Ambient.r + light_cell.Directional.r * 0.6f;	// 0.6f - korekcia, je mozne odstranit
			pGrassBunchOut->quad_light_size[i].y = light_cell.Ambient.g + light_cell.Directional.g * 0.6f;
			pGrassBunchOut->quad_light_size[i].z = light_cell.Ambient.b + light_cell.Directional.b * 0.6f;
		}
		pGrassBunchOut->quad_light_size[i].x = min (pGrassBunchOut->quad_light_size[i].x, 1.0f);
		pGrassBunchOut->quad_light_size[i].y = min (pGrassBunchOut->quad_light_size[i].y, 1.0f);
		pGrassBunchOut->quad_light_size[i].z = min (pGrassBunchOut->quad_light_size[i].z, 1.0f);
		// sort_list
		pGrassBunchOut->sort_list[i].Idx = (short)i;
		pGrassBunchOut->sort_list[i].fDistance = -1.0f;
	}

	m_pObjects[dwFreeIdx].pGrassBunch = pGrassBunchOut;
	m_pObjects[dwFreeIdx].type = alphaObj_grass;

	return true;
}

//---------------------------------
inline void CP3DAlphaManager::SortGrassBunch (GRASS_BUNCH *pGrassBunch)
{
	if (pGrassBunch == NULL) return;

	float x, z;
	DWORD	i, dwQuadCount = (DWORD)pGrassBunch->wQuadCount;

	// vypocet vzdialenosti
	for (i=0; i<dwQuadCount; i++)
	{
		D3DXVECTOR4	&vec4 = pGrassBunch->quad_position_rot[pGrassBunch->sort_list[i].Idx];
		x = g_cameraPosition.x - vec4.x;
		z = g_cameraPosition.z - vec4.z;
		pGrassBunch->sort_list[i].fDistance = x*x + z*z;
	}
	// sorting
	BidirectBubbleSort<ALPHA_SORT_TABLE> (pGrassBunch->sort_list, dwQuadCount);
}

//---------------------------------
DWORD CP3DAlphaManager::CreateDynamicModel (IP3DMesh *pInitialMesh, P3DSphere &bsphere)
{
	// najdenie volneho prvku
	DWORD	dwFreeIdx = 0xFFFFFFFF;
loop1:
	for (DWORD i=0; i<m_dwMaxObjectCount; i++)
		if (m_pObjects[i].pDynamicModel == NULL)
			{dwFreeIdx = i; break;}
	if (dwFreeIdx == 0xFFFFFFFF)
		if (ReallocateAlphaRecords ())
			goto loop1;
		else
			return 0xFFFFFFFF;

	m_pObjects[dwFreeIdx].pDynamicModel = new DYN_MODEL_ALPHAMAN;
	if (m_pObjects[dwFreeIdx].pDynamicModel == NULL) return 0xFFFFFFFF;
	m_pObjects[dwFreeIdx].type = alphaObj_dynamic_model;
	m_pObjects[dwFreeIdx].pDynamicModel->pMesh = pInitialMesh;
	m_pObjects[dwFreeIdx].boundSphere = bsphere;

	return dwFreeIdx;
}

//---------------------------------
void CP3DAlphaManager::UpdateDynamicModel (DWORD dwDynModel, IP3DMesh *pNewMesh, P3DMatrix &mat, bool bVisible, bool bIsLighted, SHADER_DYNLIGHT_INPUT &shader_input)
{
	m_pObjects[dwDynModel].pDynamicModel->pMesh = pNewMesh;
	m_pObjects[dwDynModel].boundSphere.pointOrigin = mat.m_posit3;
	m_pObjects[dwDynModel].pDynamicModel->bVisible = bVisible;
	m_pObjects[dwDynModel].pDynamicModel->bIsLighted = bIsLighted;
	m_pObjects[dwDynModel].pDynamicModel->shader_input = shader_input;
	m_pObjects[dwDynModel].pDynamicModel->matrix = mat;
}

//---------------------------------
void CP3DAlphaManager::Loop ()
{
	DWORD i;
	bool bSortGrassBunch = false;
	static DWORD dwLastGrassBunchSortTimeStamp;
	#define GRASSBUNCH_SORT_PERIOD	50	// 20 krat za sekundu

	if (g_pTimer->GetTickCount_ms() >= dwLastGrassBunchSortTimeStamp)
	{
		bSortGrassBunch = true;
		dwLastGrassBunchSortTimeStamp = g_pTimer->GetTickCount_ms() + GRASSBUNCH_SORT_PERIOD;
	}

	m_dwVisibleObjects = 0;

	// urcenie viditelnosti a zoradenie podla vzdialenosti
	for (i=0; i<m_dwMaxObjectCount; i++)
		if (m_pObjects[i].pGlassWindow)
		{
			float	x, y, z;
			bool	bVisible = false;

			//////////////////////////////////////////////////////////////////////////
			if (m_pObjects[i].type == alphaObj_window)		// window
			{
				// frustum test
				P3DSphere	&bSphere = m_pObjects[i].boundSphere;
				if (!m_pFrustum->SphereInFrustum (bSphere.pointOrigin.x, bSphere.pointOrigin.y, \
						bSphere.pointOrigin.z, bSphere.fRadius))
					continue;		// nevyhovuje pre frustum

				// bsp culling
				for (DWORD j=0; j<m_pObjects[i].clusterList.dwNumClusters; j++)
					if (m_pBspMap->ClusterVisible (m_pObjects[i].clusterList.intCluster[j]))
						{bVisible = true; break;}
				if (!bVisible) continue;		// nevyhovuje pre bsp culling

				P3DXVector3D	&pTempVector = m_pObjects[i].boundSphere.pointOrigin;
				x = g_cameraPosition.x - pTempVector.x;
				y = g_cameraPosition.y - pTempVector.y;
				z = g_cameraPosition.z - pTempVector.z;
				m_pSortTable[m_dwVisibleObjects].fDistance = x*x + y*y + z*z;
				m_pSortTable[m_dwVisibleObjects].Idx = (short)i;
				m_dwVisibleObjects++;
			}

			//////////////////////////////////////////////////////////////////////////
			else if (m_pObjects[i].type == alphaObj_grass)		// grass
			{
				P3DXVector3D	&pTempVector = m_pObjects[i].boundSphere.pointOrigin;
				x = g_cameraPosition.x - pTempVector.x;
				y = g_cameraPosition.y - pTempVector.y;
				z = g_cameraPosition.z - pTempVector.z;
				float fDistance = x*x + y*y + z*z;
				if (fDistance > m_pObjects[i].pGrassBunch->fSquaredFadeDistanceVisTest)
					continue;		// grass entita je za maximalnou hranicou vykreslovania

				// frustum culling
				P3DSphere	&bSphere = m_pObjects[i].boundSphere;
				if (!m_pFrustum->SphereInFrustum (bSphere.pointOrigin.x, bSphere.pointOrigin.y, \
						bSphere.pointOrigin.z, bSphere.fRadius))
					continue;		// nevyhovuje pre frustum

				// bsp culling
				bool	bVisible = false;
				for (DWORD j=0; j<m_pObjects[i].clusterList.dwNumClusters; j++)
					if (m_pBspMap->ClusterVisible (m_pObjects[i].clusterList.intCluster[j]))
						{bVisible = true; break;}

				if (!bVisible) continue;		// nevyhovuje pre bsp culling

				m_pSortTable[m_dwVisibleObjects].fDistance = fDistance;
				m_pSortTable[m_dwVisibleObjects].Idx = (short)i;
				m_dwVisibleObjects++;
				if (bSortGrassBunch)
					SortGrassBunch (m_pObjects[i].pGrassBunch);
			}

			//////////////////////////////////////////////////////////////////////////
			else if (m_pObjects[i].type == alphaObj_dynamic_model)		// dynamic model
			{
				if (m_pObjects[i].pDynamicModel->bVisible)
				{
					P3DXVector3D &pTempVector = m_pObjects[i].boundSphere.pointOrigin;
					x = g_cameraPosition.x - pTempVector.x;
					y = g_cameraPosition.y - pTempVector.y;
					z = g_cameraPosition.z - pTempVector.z;
					m_pSortTable[m_dwVisibleObjects].fDistance = x*x + y*y + z*z;
					m_pSortTable[m_dwVisibleObjects].Idx = (short)i;
					m_dwVisibleObjects++;
					m_pObjects[i].pDynamicModel->bVisible = false;
				}
			}
		}

		if (m_dwVisibleObjects == 0) return;	// ani jeden objekt nepresiel cez visibility test

		// zoradenie alpha objektov
		QuickSort<ALPHA_SORT_TABLE> (m_pSortTable, 0, m_dwVisibleObjects - 1);
}

//---------------------------------
void CP3DAlphaManager::Render ()
{
	if (m_dwVisibleObjects == 0) return;

	g_pMaterialManager->TurnOffShaders ();

	STAT(STAT_SET_RENDER_STATE, 2);
	g_pD3DDevice->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pD3DDevice->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//////////////////////////////////////////////////////////////////////////
	// rendering

	UINT					passes;
	BOOL					features[2];
	ALPHA_OBJECT_TYPE	lastObjectType = alphaObj_none;
	LPD3DXEFFECT		pLastEffect = NULL;
	float					fCurrentTime = ((float)g_pTimer->GetTickCount_ms()) * 0.001f;

	for (int j=m_dwVisibleObjects-1; j>=0; --j)		// opacne poradie
	{
		int intIdx = m_pSortTable[j].Idx;

		//////////////////////////////////////////////////////////////////////////
		if (m_pObjects[intIdx].type == alphaObj_window)				// WINDOW
		{
			if ((lastObjectType != alphaObj_window) && (lastObjectType != alphaObj_grass))
			{
				lastObjectType = alphaObj_window;
				STAT(STAT_SET_RENDER_STATE, 3);
				STAT(STAT_SET_SAMPLER_STATE, 2);
				g_pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
				g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);
				g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
				g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_NONE);
			}
			g_pD3DDevice->SetStreamSource (0, m_pObjects[intIdx].pGlassWindow->pVB, 0, sizeof(GWIN_VERTEX_FORMAT));
			// nastavenie parametrov
			features[0] = (m_pObjects[intIdx].pGlassWindow->p_Texture != NULL);
			features[1] = (m_pObjects[intIdx].pGlassWindow->p_CubeTexture != NULL);
			m_pWindowShader->SetBoolArray (m_pWindowParams[0], features, 2);
			if (features[0])
				m_pWindowShader->SetTexture (m_pWindowParams[1], m_pObjects[intIdx].pGlassWindow->p_Texture);
			if (features[1])
				m_pWindowShader->SetTexture (m_pWindowParams[2], m_pObjects[intIdx].pGlassWindow->p_CubeTexture);

			if (pLastEffect != m_pWindowShader)
			{
				if (pLastEffect)
				{
					pLastEffect->EndPass ();
					pLastEffect->End ();
				}
				g_pD3DDevice->SetVertexDeclaration (m_pWindowsVertDec);
				m_pWindowShader->Begin (&passes, D3DXFX_DONOTSAVESTATE);
				STAT(STAT_SHADER_BEGIN_PASS, 1);
				m_pWindowShader->BeginPass (0);
				pLastEffect = m_pWindowShader;
			}
			else
				m_pWindowShader->CommitChanges ();
			// vykreslenie
			STAT(STAT_DRAW_CALLS, 1);
			STAT(STAT_DRAW_VERTS, 2);
			g_pD3DDevice->DrawPrimitive (D3DPT_TRIANGLESTRIP, 0, 2);
		}

		//////////////////////////////////////////////////////////////////////////
		else if (m_pObjects[intIdx].type == alphaObj_grass)		// GRASS
		{
			if ((lastObjectType != alphaObj_window) && (lastObjectType != alphaObj_grass))
			{
				lastObjectType = alphaObj_grass;
				STAT(STAT_SET_RENDER_STATE, 3);
				STAT(STAT_SET_SAMPLER_STATE, 2);
				g_pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, TRUE);
				g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, FALSE);
				g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
				g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
				g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_NONE);
			}
			// usporiadanie prvkov
			DWORD dwQuadCount = (DWORD)m_pObjects[intIdx].pGrassBunch->wQuadCount;
			for (register int j=0; j<(int)dwQuadCount; ++j)
			{
				short Idx = m_pObjects[intIdx].pGrassBunch->sort_list[dwQuadCount - j - 1].Idx;
				sorted_quad_light_size[j] = m_pObjects[intIdx].pGrassBunch->quad_light_size[Idx];
				sorted_quad_position_rot[j] = m_pObjects[intIdx].pGrassBunch->quad_position_rot[Idx];
			}

			m_pGrassShader->SetVectorArray (m_pGrassParams[0], sorted_quad_light_size, dwQuadCount);
			m_pGrassShader->SetVectorArray (m_pGrassParams[1], sorted_quad_position_rot, dwQuadCount);
			m_pGrassShader->SetTexture (m_pGrassParams[2], m_pObjects[intIdx].pGrassBunch->p_Texture);
			m_pGrassShader->SetBool (m_pGrassParams[3], (BOOL)m_pObjects[intIdx].pGrassBunch->bBillboarded);
			m_pGrassShader->SetBool (m_pGrassParams[4], (BOOL)m_pObjects[intIdx].pGrassBunch->bAnimated);
			m_pGrassShader->SetFloat (m_pGrassParams[5], fCurrentTime);
			m_pGrassShader->SetFloat (m_pGrassParams[6], m_pObjects[intIdx].pGrassBunch->fSquaredFadeDistance);

			if (pLastEffect != m_pGrassShader)
			{
				if (pLastEffect)
				{
					pLastEffect->EndPass ();
					pLastEffect->End ();
				}
				g_pD3DDevice->SetStreamSource (0, m_pGrassVertexBuf, 0, sizeof(GRASS_VERTEX_FORMAT));
				g_pD3DDevice->SetIndices (m_pGrassIndexBuf);
				g_pD3DDevice->SetVertexDeclaration (m_pGrassVertDec);
				m_pGrassShader->Begin (&passes, D3DXFX_DONOTSAVESTATE);
				STAT(STAT_SHADER_BEGIN_PASS, 1);
				m_pGrassShader->BeginPass (0);
				pLastEffect = m_pGrassShader;
			}
			else
				m_pGrassShader->CommitChanges ();
			// vykreslenie
			STAT(STAT_DRAW_CALLS, 1);
			STAT(STAT_DRAW_VERTS, dwQuadCount * 2);
			g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, 0, 0, dwQuadCount * 4, 0, dwQuadCount * 2);
		}

		//////////////////////////////////////////////////////////////////////////
		else if (m_pObjects[intIdx].type == alphaObj_dynamic_model)		// DYNAMIC MODEL
		{
			if (lastObjectType != alphaObj_dynamic_model)
			{
				lastObjectType = alphaObj_dynamic_model;
				STAT(STAT_SET_RENDER_STATE, 3);
				STAT(STAT_SET_SAMPLER_STATE, 2);
				g_pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
				g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
				g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
				g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW);
			}
			// vykreslit konecne ve spravnem poradi alpha mesh
			if (m_pObjects[intIdx].pDynamicModel->bVisible)
				m_pObjects[intIdx].pDynamicModel->pMesh->Render(m_pObjects[intIdx].pDynamicModel->bIsLighted, m_pObjects[intIdx].pDynamicModel->shader_input);
		}
	}

	if (pLastEffect)
	{
		pLastEffect->EndPass ();
		pLastEffect->End ();
	}

	STAT(STAT_SET_SAMPLER_STATE, 2);
	STAT(STAT_SET_RENDER_STATE, 3);
	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	g_pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
	g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW);
	g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
}

//---------------------------------
void CP3DAlphaManager::BreakWindow (unsigned long Idx)
{
	if ((m_pObjects == NULL) || (m_pObjects[Idx].pGlassWindow->pVB == NULL))
		return;

	SAFE_RELEASE(m_pObjects[Idx].pGlassWindow->pVB)
		if (m_pObjects[Idx].pGlassWindow->p_Texture)
			g_TextureLoader.UnloadTexture (m_pObjects[Idx].pGlassWindow->p_Texture);
	if (m_pObjects[Idx].pGlassWindow->p_CubeTexture)
		g_TextureLoader.UnloadTexture (m_pObjects[Idx].pGlassWindow->p_CubeTexture);
	if (m_pObjects[Idx].pGlassWindow->p_RigidBody)
		delete m_pObjects[Idx].pGlassWindow->p_RigidBody;
	memset (&m_pObjects[Idx], 0, sizeof(OBJECT_TABLE));
	// TODO: pridat graf. efekt rozbiteho skla ???
}

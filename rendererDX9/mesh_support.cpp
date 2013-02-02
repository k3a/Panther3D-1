
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Mesh support interfaces and stuctures definition -
//							D3DXFRAME_DERIVED
//							D3DXMESHCONTAINER_DERIVED
//							CalculateMeshTangents
//							CSaveUserData
//							CLoadUserData
//							CAllocateHierarchy
//							CDeallocateHierarchy
//-----------------------------------------------------------------------------


#include "mesh.h"
#include <initguid.h>


// GUID je mozne generovat programom GUIDGEN.EXE - sucast MS VS
//DEFINE_GUID (tempGuid, 0x4c944580, 0x9e9a, 0x11cf, 0xab, 0x43, 0x01, 0x20, 0xaf, 0x71, 0xe4, 0x33);


bool FindTextureIndexTagByName (ezxml_t xmlTextures, char* szTextureName, DWORD &dwIdx);


//-----------------------------------------------------------------------------
// Desc: konstruktor, destruktor a Find pre D3DXFRAME_DERIVED
//-----------------------------------------------------------------------------
D3DXFRAME_DERIVED::D3DXFRAME_DERIVED()
{
	Name = NULL;
	pMeshContainer = NULL;
	pFrameSibling = pFrameFirstChild = NULL;
	D3DXMatrixIdentity(&matCombined);
	matLocalTansform = matCombined;
	TransformationMatrix = matCombined;
	bIdentityLocalTransform = true;
}

D3DXFRAME_DERIVED::~D3DXFRAME_DERIVED()
{
	SAFE_DELETE_ARRAY (Name)
}

D3DXFRAME_DERIVED* D3DXFRAME_DERIVED::Find(const char *szFrameName)
{
	D3DXFRAME_DERIVED *pFrame;
	if ( (Name != NULL) && (strcmp(szFrameName, Name) == 0) )
		return this;
	if (pFrameSibling)
		if (pFrame = ((D3DXFRAME_DERIVED*)pFrameSibling)->Find (szFrameName))
			return pFrame;
	if (pFrameFirstChild)
		if (pFrame = ((D3DXFRAME_DERIVED*)pFrameFirstChild)->Find (szFrameName))
			return pFrame;
	return NULL;
}

//-----------------------------------------------------------------------------
// Desc: konstruktor, destruktor a Find pre D3DXMESHCONTAINER_DERIVED
//-----------------------------------------------------------------------------
D3DXMESHCONTAINER_DERIVED::D3DXMESHCONTAINER_DERIVED()
{
	memset (this, 0, sizeof(D3DXMESHCONTAINER_DERIVED));
}

D3DXMESHCONTAINER_DERIVED::~D3DXMESHCONTAINER_DERIVED()
{
	SAFE_DELETE_ARRAY (Name)
	if (pMaterials)
		for (DWORD i=0; i<NumMaterials; i++)
			SAFE_DELETE_ARRAY (pMaterials[i].pTextureFilename)
	SAFE_DELETE_ARRAY (pMaterials)
	SAFE_DELETE_ARRAY (pAdjacency)
	SAFE_DELETE_ARRAY (ppBoneTransformMatrices)
	SAFE_DELETE_ARRAY (pBoneInverseMatrices)
	SAFE_DELETE_ARRAY (p7MTmaterials)
	SAFE_RELEASE (MeshData.pMesh)
	SAFE_RELEASE (pSkinInfo)
	SAFE_RELEASE (pBoneCombinationBuf)
	SAFE_DELETE_ARRAY (pEffects)

	if (pTextures)
	{
		DWORD dwTexturesNum = NumMaterials * dwNumCoords * dwNumPaintings;
		for (DWORD i=0; i < dwTexturesNum; i++)
			if (pTextures[i])
				g_TextureLoader.UnloadTexture (pTextures[i]);
	}
	SAFE_DELETE_ARRAY (pTextures)
}


//////////////////////////////////////////////////////////////////////////
//							CalculateMeshTangents
//
//				custom funkcia na vypocet tangent a binormal
//////////////////////////////////////////////////////////////////////////

bool CalculateMeshTangents (LPD3DXBASEMESH pMesh, DWORD dwTextureCoordsIndex)
{
	long vertexCount;
	long triangleCount;

	DWORD dwPosOffset = 0xFFFFFFFF, dwNormalOffset = 0xFFFFFFFF, dwTangentOffset = 0xFFFFFFFF;
	DWORD dwBinormalOffset = 0xFFFFFFFF, dwTexccordOffset = 0xFFFFFFFF;
	DWORD i, dwVertexStride;
	bool b16bitIndices;
	unsigned short	*indicex16bit;
	unsigned long	*indicex32bit;

	if (pMesh == NULL) return false;

	vertexCount = pMesh->GetNumVertices ();
	triangleCount = pMesh->GetNumFaces ();
	dwVertexStride = pMesh->GetNumBytesPerVertex();
	b16bitIndices = !(pMesh->GetOptions() & D3DXMESH_32BIT);

	D3DVERTEXELEMENT9 meshDecl[MAX_FVF_DECL_SIZE] = {0};
	pMesh->GetDeclaration (meshDecl);
	for (i=0; ;i++)
	{
		switch (meshDecl[i].Usage)
		{
		case D3DDECLUSAGE_POSITION: dwPosOffset = meshDecl[i].Offset; break;
		case D3DDECLUSAGE_NORMAL: dwNormalOffset = meshDecl[i].Offset; break;
		case D3DDECLUSAGE_TANGENT: dwTangentOffset = meshDecl[i].Offset; break;
		case D3DDECLUSAGE_BINORMAL: dwBinormalOffset = meshDecl[i].Offset; break;
		case D3DDECLUSAGE_TEXCOORD:
			if (meshDecl[i].UsageIndex == (BYTE)dwTextureCoordsIndex)
				dwTexccordOffset = meshDecl[i].Offset;
		}
		// posledny element
		if ((i == MAX_FVF_DECL_SIZE) || ((meshDecl[i].Stream == 0xFF) && (meshDecl[i].Type == D3DDECLTYPE_UNUSED)))
			break;
	}

	if (	(dwPosOffset==0xFFFFFFFF) || (dwNormalOffset==0xFFFFFFFF) || \
			(dwTangentOffset==0xFFFFFFFF) || (dwBinormalOffset==0xFFFFFFFF) || \
			(dwTexccordOffset==0xFFFFFFFF))
		return false;


	//////////////////////////////////////////////////////////////////////////

	LPVOID pVertexData;
	LPBYTE pVertexDataByte;
	if (FAILED (pMesh->LockVertexBuffer (0, &pVertexData)))
		return false;
	pVertexDataByte = (LPBYTE)pVertexData;

	LPVOID pIndexData;
	if (FAILED (pMesh->LockIndexBuffer (0, &pIndexData)))
	{
		pMesh->UnlockVertexBuffer();
		return false;
	}
	indicex16bit = (unsigned short*)pIndexData;
	indicex32bit = (unsigned long*)pIndexData;

	P3DXVector3D *tan1 = new P3DXVector3D[vertexCount * 2];
	P3DXVector3D *tan2 = tan1 + vertexCount;
	memset (tan1, 0, vertexCount * sizeof(P3DXVector3D) * 2);

	for (long a = 0; a < triangleCount; a++)
	{
		long i1, i2, i3;

		if (b16bitIndices)
		{
			i1 = indicex16bit[a*3];
			i2 = indicex16bit[a*3 + 1];
			i3 = indicex16bit[a*3 + 2];
		}
		else
		{
			i1 = indicex32bit[a*3];
			i2 = indicex32bit[a*3 + 1];
			i3 = indicex32bit[a*3 + 2];
		}

		const P3DXVector3D &v1 = (P3DXVector3D&) pVertexDataByte[i1*dwVertexStride + dwPosOffset];
		const P3DXVector3D &v2 = (P3DXVector3D&) pVertexDataByte[i2*dwVertexStride + dwPosOffset];
		const P3DXVector3D &v3 = (P3DXVector3D&) pVertexDataByte[i3*dwVertexStride + dwPosOffset];

		const P3DPoint2D &w1 = (P3DPoint2D&) pVertexDataByte[i1*dwVertexStride + dwTexccordOffset];
		const P3DPoint2D &w2 = (P3DPoint2D&) pVertexDataByte[i2*dwVertexStride + dwTexccordOffset];
		const P3DPoint2D &w3 = (P3DPoint2D&) pVertexDataByte[i3*dwVertexStride + dwTexccordOffset];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float r = 1.0F / (s1 * t2 - s2 * t1);
		P3DXVector3D sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		P3DXVector3D tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (long a = 0; a < vertexCount; a++)
	{
		const P3DXVector3D &n = (P3DXVector3D&) pVertexDataByte[a*dwVertexStride + dwNormalOffset];
		const P3DXVector3D& t = tan1[a];

		// Gram-Schmidt orthogonalize
		P3DXVector3D vecTangent = t - n * (n * t);
		vecTangent.Normalize ();
		// set tangent
		P3DXVector3D &tangent = (P3DXVector3D&) pVertexDataByte[a*dwVertexStride + dwTangentOffset];
		tangent = vecTangent;

		// Calculate handedness
		float handedness = (((n % t) * tan2[a]) < 0.0F) ? -1.0F : 1.0F;
		// binormal = normal x tangent
		P3DXVector3D vecBinormal = (n % vecTangent) * handedness;
		vecBinormal.Normalize();
		// set binormal
		P3DXVector3D &binormal = (P3DXVector3D&) pVertexDataByte[a*dwVertexStride + dwBinormalOffset];
		binormal = vecBinormal;
	}

	delete[] tan1;
	pMesh->UnlockIndexBuffer();
	pMesh->UnlockVertexBuffer();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//								CSaveUserData
//
//				callback fcie volane z D3DXSaveMeshHierarchyToFile
//////////////////////////////////////////////////////////////////////////

//---------------------------------
HRESULT CSaveUserData::AddTopLevelDataObjectsPre (LPD3DXFILESAVEOBJECT pXofSave)
{
/*
// priklad ulozenia top level objektu do .x suboru
	HRESULT hr;
	LPD3DXFILE				pFile;
	LPD3DXFILESAVEDATA	pData;
	struct MyStruct{
		DWORD dwSize;
		DWORD dwField[10];
	};
	MyStruct MyData;
	MyData.dwSize = 10;
	memset (MyData.dwField, 255, 10 * sizeof(DWORD));

	hr = pXofSave->AddDataObject (tempGuid, "MyData", NULL, sizeof(MyData), &MyData, &pData);
	pData->Release ();
*/
	return S_OK;
}

//---------------------------------
HRESULT CSaveUserData::RegisterTemplates (LPD3DXFILE pXFileApi)
{
/*
	// priklad registracie vlastnej sablony - registracia je potrebna pre akykolvek format,
	// ktoreho data sa neskor ulozia napr. cez CSaveUserData::AddTopLevelDataObjectsPre
	char *Template = 
		"xof 0303txt 0032 \
		template CustomTemplate { \
		<4c944580-9e9a-11cf-ab43-0120af71e433> \
		DWORD Length; \
		array DWORD Values[Length]; \
		}";

	HRESULT hr;
	hr = pXFileApi->RegisterTemplates (Template, strlen (Template));
*/
	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
//							CLoadUserData
//
//			callback fcie volane z D3DXLoadMeshHierarchyFromX
//////////////////////////////////////////////////////////////////////////

HRESULT CLoadUserData::LoadTopLevelData (LPD3DXFILEDATA pXofChildData)
{
/*
	// priklad nahratia custom top level datovej jednotky z .x suboru
	HRESULT hr;
	DWORD dwSize;
	char szName[64] = {0};

	dwSize = sizeof(szName);
	hr = pXofChildData->GetName (szName, &dwSize);
	GUID guid;
	hr = pXofChildData->GetType (&guid);				// template identifier 

	LPVOID pData;
	hr = pXofChildData->Lock (&dwSize, (LPCVOID*)&pData);
	// nacitanie dat ...
	hr = pXofChildData->Unlock ();
*/
	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
//							CAllocateHierarchy
//////////////////////////////////////////////////////////////////////////


HRESULT CAllocateHierarchy::CreateFrame (LPCSTR Name, LPD3DXFRAME *ppNewFrame)
{
	D3DXFRAME_DERIVED *pFrame;

	*ppNewFrame = NULL;
	pFrame = new D3DXFRAME_DERIVED;
	if (pFrame == NULL)
		return E_OUTOFMEMORY;

	if ((Name != NULL) && (Name[0] != 0))
	{//alokovanie miesta pre meno rámca
		int nNameSize = (int)strlen(Name)+1;
		pFrame->Name = new char[nNameSize];
		if (pFrame->Name == NULL)
			{delete pFrame; return E_OUTOFMEMORY;}
		memcpy(pFrame->Name, Name, nNameSize);
	}

	*ppNewFrame = pFrame;
	return S_OK;
}

//---------------------------------
HRESULT CAllocateHierarchy::CreateMeshContainer (LPCSTR Name, const D3DXMESHDATA *pMeshData, \
		const D3DXMATERIAL *pMaterials,const D3DXEFFECTINSTANCE *pEffectInstances, \
		DWORD NumMaterials, const DWORD *pAdjacency, LPD3DXSKININFO pSkinInfo, \
		LPD3DXMESHCONTAINER *ppNewMeshContainer)
{
	DWORD i, j;
	D3DXMESHCONTAINER_DERIVED *pMeshContainer;
	LPD3DXMESH pMesh, pTempMesh;
	HRESULT hr = E_FAIL;

	*ppNewMeshContainer = NULL;

	//podporované su iba bezne meshe
	if (pMeshData->Type != D3DXMESHTYPE_MESH)
		return E_FAIL;

	//alokácia derivovanej štruktúry kvôli návratovej hodnote - LPD3DXMESHCONTAINER
	pMeshContainer = new D3DXMESHCONTAINER_DERIVED;
	if (pMeshContainer == NULL)
		return E_OUTOFMEMORY;

	pMesh = pMeshData->pMesh;
	pMesh->AddRef();

	pMeshContainer->NumMaterials = max (1, NumMaterials);
	pMeshContainer->dwNumPaintings = 1;
	pMeshContainer->dwNumCoords = 1;
	if (m_localData.xmlTextures)
	{
		const char *szTemp;
		if (szTemp = g_pXML->Attr (m_localData.xmlTextures, "paint_num"))
			pMeshContainer->dwNumPaintings = (DWORD)atoi (szTemp);
		if (szTemp = g_pXML->Attr (m_localData.xmlTextures, "coord_num"))
			pMeshContainer->dwNumCoords = (DWORD)atoi (szTemp);
	}
	pMeshContainer->dwNumPaintings = max (1, pMeshContainer->dwNumPaintings);
	pMeshContainer->dwNumCoords = max (1, pMeshContainer->dwNumCoords);
	if (pMeshContainer->dwNumPaintings <= m_localData.dwUseThisPainting)
		m_localData.dwUseThisPainting = 0;		// chyba, pozadovana painting-verzia nie je v xml definovana

	//////////////////////////////////////////////////////////////////////////
	// mesh cloning - tangent, binormal, tex. koordinaty - pocitaju sa len pre modely s viac ako jednou texturovou vrstvou
	if (pMeshContainer->dwNumCoords > 1)
	{
		const DWORD dwRealNumCoords = (pMesh->GetFVF() & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
		const int intTexCoordsToAdd = pMeshContainer->dwNumCoords - dwRealNumCoords;
		DWORD dwVertexStride = pMesh->GetNumBytesPerVertex ();

		D3DVERTEXELEMENT9 meshDeclaration[MAX_FVF_DECL_SIZE];
		if (SUCCEEDED (pMesh->GetDeclaration (meshDeclaration)))
		{
			DWORD dwNumDeclarations = 0;
			for (i=0; (i < MAX_FVF_DECL_SIZE) && (meshDeclaration[i].Stream != 0xFF); i++)
				dwNumDeclarations++;
			// pridanie deklaracie pre tangent
			meshDeclaration[dwNumDeclarations].Stream = 0;
			meshDeclaration[dwNumDeclarations].Offset = (WORD)dwVertexStride;
			meshDeclaration[dwNumDeclarations].Type = D3DDECLTYPE_FLOAT3;
			meshDeclaration[dwNumDeclarations].Method = D3DDECLMETHOD_DEFAULT;
			meshDeclaration[dwNumDeclarations].Usage = D3DDECLUSAGE_TANGENT;
			meshDeclaration[dwNumDeclarations].UsageIndex = 0;
			dwNumDeclarations++;
			// pridanie deklaracie pre binormal
			meshDeclaration[dwNumDeclarations].Stream = 0;
			meshDeclaration[dwNumDeclarations].Offset = (WORD)(dwVertexStride + 3*sizeof(float));
			meshDeclaration[dwNumDeclarations].Type = D3DDECLTYPE_FLOAT3;
			meshDeclaration[dwNumDeclarations].Method = D3DDECLMETHOD_DEFAULT;
			meshDeclaration[dwNumDeclarations].Usage = D3DDECLUSAGE_BINORMAL;
			meshDeclaration[dwNumDeclarations].UsageIndex = 0;
			dwNumDeclarations++;
			// pridanie texturovych koordinatov
			for (int k=0; k<intTexCoordsToAdd; k++)
			{
				meshDeclaration[dwNumDeclarations].Stream = 0;
				meshDeclaration[dwNumDeclarations].Offset = (WORD)(dwVertexStride + 6*sizeof(float) + k*2*sizeof(float));
				meshDeclaration[dwNumDeclarations].Type = D3DDECLTYPE_FLOAT2;
				meshDeclaration[dwNumDeclarations].Method = D3DDECLMETHOD_DEFAULT;
				meshDeclaration[dwNumDeclarations].Usage = D3DDECLUSAGE_TEXCOORD;
				meshDeclaration[dwNumDeclarations].UsageIndex = BYTE(k + dwRealNumCoords);
				dwNumDeclarations++;
			}
			// ukoncovaci element
			memset (&meshDeclaration[dwNumDeclarations], 0, sizeof(D3DVERTEXELEMENT9));
			meshDeclaration[dwNumDeclarations].Stream = 0xFF;
			meshDeclaration[dwNumDeclarations].Type = D3DDECLTYPE_UNUSED;

			if (FAILED (pMesh->CloneMesh (pMesh->GetOptions(), meshDeclaration, g_pD3DDevice, &pTempMesh)))
				goto e_Exit;

			pMesh->Release ();
			pMesh = pTempMesh;
		}

		//////////////////////////////////////////////////////////////////////////
		const DWORD	dwVertexCount = pMesh->GetNumVertices ();
		dwVertexStride = pMesh->GetNumBytesPerVertex ();

		// uprava koordinatov, ak je to potrebne
		for (i=dwRealNumCoords; i < pMeshContainer->dwNumCoords; i++)
		{
			LPVOID	pData;
			DWORD	dwUseThisCoords = 0xFFFFFFFF,dwTexCoordOffsetSrc = 0xFFFFFFFF, dwTexCoordOffsetDst = 0xFFFFFFFF;
			if (m_localData.xmlTextures)
			{
				ezxml_t pTextureTag, pCoordsTag;
				const char *szTemp;
				char szCoordsTag[16];
				if (pTextureTag = g_pXML->Child (m_localData.xmlTextures, "texture0"))
				{
					sprintf (szCoordsTag, "coords%u", i);
					if (pCoordsTag = g_pXML->Child (pTextureTag, szCoordsTag))
						if (szTemp = g_pXML->Attr (pCoordsTag, "usecoords"))
						{
							dwUseThisCoords = (DWORD) atoi (szTemp);
							if (dwUseThisCoords >= i) dwUseThisCoords = 0;
						}
				}
			}
			if ((i > dwRealNumCoords) && (dwUseThisCoords == 0xFFFFFFFF))
				dwUseThisCoords = 0;

			// ak je v xml definovany usecoords, tak kopirovat koordinaty
			if (dwUseThisCoords != 0xFFFFFFFF)
			{
				for (j=0; (j < MAX_FVF_DECL_SIZE) && (meshDeclaration[j].Stream != 0xFF); j++)
					if (meshDeclaration[j].Usage == D3DDECLUSAGE_TEXCOORD)
						if (meshDeclaration[j].UsageIndex == dwUseThisCoords)
							dwTexCoordOffsetSrc = meshDeclaration[j].Offset;
						else if (meshDeclaration[j].UsageIndex == i)
							dwTexCoordOffsetDst = meshDeclaration[j].Offset;
				if ((dwTexCoordOffsetSrc == 0xFFFFFFFF) || (dwTexCoordOffsetDst == 0xFFFFFFFF))
					continue;	// chyba

				if (SUCCEEDED (pMesh->LockVertexBuffer (0, &pData)))
				{
					LPBYTE pVertexData = (LPBYTE)pData;
					// kopirovanie koordinatov z vrstvy dwUseThisCoords
					for (j=0; j<dwVertexCount; j++)
					{
						memcpy ((pVertexData + dwTexCoordOffsetDst), (pVertexData + dwTexCoordOffsetSrc), 2*sizeof(float));
						pVertexData += dwVertexStride;
					}
					pMesh->UnlockVertexBuffer ();
				}
			}
		}
		// vypocet tangent a binormal
		if (!CalculateMeshTangents (pMesh, 1))
			g_pConsole->Message(MSG_CON_ERR, "MeshLoader: CalculateMeshTangents() function failed");
	}

	//////////////////////////////////////////////////////////////////////////
	// nacitanie informacii o materialoch a texturach z x suboru
	pMeshContainer->pMaterials	= new D3DXMATERIAL[pMeshContainer->NumMaterials];
	pMeshContainer->pTextures	= new LPDIRECT3DTEXTURE9 [pMeshContainer->NumMaterials * pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings];
	pMeshContainer->p7MTmaterials = new DWORD [pMeshContainer->NumMaterials];

	if ((pMeshContainer->pMaterials == NULL) || (pMeshContainer->pTextures == NULL) || \
														(pMeshContainer->p7MTmaterials == NULL))
		{hr = E_OUTOFMEMORY; goto e_Exit;}
	memset (pMeshContainer->pMaterials, 0, sizeof(D3DXMATERIAL) * pMeshContainer->NumMaterials);
	memset (pMeshContainer->pTextures, 0, sizeof(LPDIRECT3DTEXTURE9) * pMeshContainer->NumMaterials * pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings);
	memset (pMeshContainer->p7MTmaterials, 0xFF, sizeof(DWORD) * pMeshContainer->NumMaterials);

	// nacitanie materialov
	if (NumMaterials > 0)
	{
		for (i=0; i<NumMaterials; i++)
		{
			pMeshContainer->pMaterials[i].MatD3D = pMaterials[i].MatD3D;
			// do x suboru sa neexportuje udaj o ambient zlozke materialov, upravit podla potreby
			pMeshContainer->pMaterials[i].MatD3D.Ambient = D3DXCOLOR (1.0f, 1.0f, 1.0f, 1.0f);
			// vypnut Power ak nie je zadana Specular zlozka
			D3DCOLORVALUE &colorSpec = pMeshContainer->pMaterials[i].MatD3D.Specular;
			if ((colorSpec.r == 0) && (colorSpec.g == 0) && (colorSpec.b == 0))
				pMeshContainer->pMaterials[i].MatD3D.Power = 0.0f;

			// nacitanie 7mt materialu, ak je k danemu povrchu priradeny
			#define MATERIAL_FLOAT_TOLERANCE		0.001f
			for (j=0; j < m_localData.dw7mtMaterialsNum; j++)
				if (m_localData.Mesh7mtMaterials[j].sz7mtMaterialName != NULL)
				{
					// porovnanie textur ak su dostupne
					if ((m_localData.Mesh7mtMaterials[j].szTextureName != NULL) && (pMaterials[i].pTextureFilename != NULL))
						if (stricmp (m_localData.Mesh7mtMaterials[j].szTextureName, pMaterials[i].pTextureFilename) != 0)
							continue;
					// porovnanie materialov
					if (fabs (m_localData.Mesh7mtMaterials[j].material.rDif - pMaterials[i].MatD3D.Diffuse.r) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.gDif - pMaterials[i].MatD3D.Diffuse.g) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.bDif - pMaterials[i].MatD3D.Diffuse.b) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.aDif - pMaterials[i].MatD3D.Diffuse.a) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.rSpec - pMaterials[i].MatD3D.Specular.r) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.gSpec - pMaterials[i].MatD3D.Specular.g) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.bSpec - pMaterials[i].MatD3D.Specular.b) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.rEm - pMaterials[i].MatD3D.Emissive.r) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.gEm - pMaterials[i].MatD3D.Emissive.g) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.bEm - pMaterials[i].MatD3D.Emissive.b) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (m_localData.Mesh7mtMaterials[j].material.specPower - pMaterials[i].MatD3D.Power) > MATERIAL_FLOAT_TOLERANCE) continue;

					// materialy su zhodne
					char szBuf[MAX_PATH] = "materials\\models\\";
					strcat (szBuf, m_localData.Mesh7mtMaterials[j].sz7mtMaterialName);
					// 7mt material loading
					pMeshContainer->p7MTmaterials[i] = g_pMaterialManager->LoadMaterial (szBuf);
					break;
				}
			// #######################################################
			// naèítanie textúr
			if ((pMaterials[i].pTextureFilename != NULL) && (pMaterials[i].pTextureFilename[0] != 0))
			{
				char szTag[16];
				ezxml_t	pTextureTag = NULL;
				DWORD dwTextureIdx = 0xFFFFFFFF;

				pMeshContainer->pMaterials[i].pTextureFilename = new char [strlen (pMaterials[i].pTextureFilename) + 1];
				strcpy (pMeshContainer->pMaterials[i].pTextureFilename, pMaterials[i].pTextureFilename);

				if (FindTextureIndexTagByName (m_localData.xmlTextures, pMaterials[i].pTextureFilename, dwTextureIdx))
				{
					sprintf (szTag, "texture%u", dwTextureIdx);
					pTextureTag = g_pXML->Child (m_localData.xmlTextures, szTag);
				}

				for (j=0; j < pMeshContainer->dwNumCoords; j++)		// pre jednotlive koordinaty
				{
					char szBuf[MAX_PATH] = "textures\\models\\";
					if (pTextureTag == NULL)		// pouzit texturu z x-suboru
					{
						strcat (szBuf, pMaterials[i].pTextureFilename);
						pMeshContainer->pTextures[ i * pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings + j*pMeshContainer->dwNumPaintings + m_localData.dwUseThisPainting] = g_TextureLoader.CreateTexture (szBuf);
					}
					else
					{
						// ziskanie nazvu textury z xml suboru
						ezxml_t	pCoordsTag, pPaintingTag;
						sprintf (szTag, "coords%u", j);
						if (pCoordsTag = g_pXML->Child (pTextureTag, szTag))
						{
							sprintf (szTag, "painting%u", m_localData.dwUseThisPainting);
							if (pPaintingTag = g_pXML->Child (pCoordsTag, szTag))
								if (pPaintingTag->txt)
								{
									strcat (szBuf, pPaintingTag->txt);
									pMeshContainer->pTextures[ i * pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings + j*pMeshContainer->dwNumPaintings + m_localData.dwUseThisPainting] = g_TextureLoader.CreateTexture (szBuf);
								}
						}
					}
				}
			}
		}
	}
	else	// nastavenie defaultneho materialu - Diffuse = 0.8, Specular = Emissive = 0.0, Ambient = 1.0
	{
		pMeshContainer->pMaterials[0].pTextureFilename = NULL;
		pMeshContainer->pMaterials[0].MatD3D.Ambient = D3DXCOLOR (1.0f, 1.0f, 1.0f, 1.0f);
		pMeshContainer->pMaterials[0].MatD3D.Diffuse = D3DXCOLOR (0.8f, 0.8f, 0.8f, 1.0f);
		pMeshContainer->pMaterials[0].MatD3D.Emissive = D3DXCOLOR (0.0f, 0.0f, 0.0f, 1.0f);
		pMeshContainer->pMaterials[0].MatD3D.Specular = pMeshContainer->pMaterials[0].MatD3D.Emissive;
		pMeshContainer->pMaterials[0].MatD3D.Power = 0.0f;
	}

/*
		// alokovanie pamäte pre materiály, textúry
		pMeshContainer->pMaterials = new D3DXMATERIAL[pMeshContainer->NumMaterials];
		pMeshContainer->pTextures = new LPDIRECT3DTEXTURE9[pMeshContainer->NumMaterials];
		if ((pMeshContainer->pMaterials == NULL) || (pMeshContainer->pTextures == NULL))
			{hr = E_OUTOFMEMORY; goto e_Exit;}
	
		//inicializácia textúr a materiálov
		memset(pMeshContainer->pTextures, 0, sizeof(LPDIRECT3DTEXTURE9) * pMeshContainer->NumMaterials);
		memset(pMeshContainer->pMaterials, 0, sizeof(D3DXMATERIAL) * pMeshContainer->NumMaterials);
	
		//ak sú dostupné materiály, tak sa kopírujú
		if (NumMaterials > 0)
		{
			for (UINT iMaterial = 0; iMaterial < NumMaterials; iMaterial++)
			{
				pMeshContainer->pMaterials[iMaterial].MatD3D = pMaterials[iMaterial].MatD3D;
				pMeshContainer->pMaterials[iMaterial].MatD3D.Ambient = D3DXCOLOR (1.0f, 1.0f, 1.0f, 1.0f);
				D3DCOLORVALUE &colorSpec = pMeshContainer->pMaterials[iMaterial].MatD3D.Specular;
				if ((colorSpec.r == 0) && (colorSpec.g == 0) && (colorSpec.b == 0))
					pMeshContainer->pMaterials[iMaterial].MatD3D.Power = 0.0f;
				if (pMaterials[iMaterial].pTextureFilename != NULL)
				{
					wsprintf (szBuf, "textures\\models\\%s", pMaterials[iMaterial].pTextureFilename);
					pMeshContainer->pTextures[iMaterial] = g_TextureLoader.CreateTexture (szBuf);
				}
			}
		}
		else		// ak nie je definovany žiaden materiál, potom sa nastavý defaultný
		{
			//Diffuse = Specular = 0.8, Ambient = 1.0
			pMeshContainer->pMaterials[0].pTextureFilename = NULL;
			pMeshContainer->pMaterials[0].MatD3D.Ambient = D3DXCOLOR (1.0f, 1.0f, 1.0f, 1.0f);
			pMeshContainer->pMaterials[0].MatD3D.Diffuse = D3DXCOLOR (0.8f, 0.8f, 0.8f, 1.0f);
			pMeshContainer->pMaterials[0].MatD3D.Emissive = D3DXCOLOR (0.0f, 0.0f, 0.0f, 1.0f);
			pMeshContainer->pMaterials[0].MatD3D.Specular = pMeshContainer->pMaterials[0].MatD3D.Diffuse;
			pMeshContainer->pMaterials[0].MatD3D.Power = 5.0f;
		}
*/
	

	//////////////////////////////////////////////////////////////////////////
	// nacitanie skin infa
	if (m_localData.bLoadSkinInfo && (pSkinInfo != NULL))
	{
		DWORD	dwNumBones;
		pMeshContainer->pSkinInfo = pSkinInfo;
		pSkinInfo->AddRef();
		// alokovanie pamate pre skin-info matice
		dwNumBones = pSkinInfo->GetNumBones();
		if (dwNumBones > 0)
		{
			pMeshContainer->pBoneInverseMatrices = new D3DXMATRIX[dwNumBones];
			pMeshContainer->ppBoneTransformMatrices = new D3DXMATRIX*[dwNumBones];
			if ((pMeshContainer->pBoneInverseMatrices == NULL) || \
						(pMeshContainer->ppBoneTransformMatrices == NULL))
				{hr = E_OUTOFMEMORY; goto e_Exit;}
			// nacitanie bone-offset matic
			for (DWORD iBone=0; iBone < dwNumBones; iBone++)
				pMeshContainer->pBoneInverseMatrices[iBone] = *(pSkinInfo->GetBoneOffsetMatrix(iBone));

			memset (pMeshContainer->ppBoneTransformMatrices, 0, sizeof(LPD3DXMATRIX) * dwNumBones);
		}

		//////////////////////////////////////////////////////////////////////////
		// upravy pre shader skinning
		pMeshContainer->NumPaletteEntries = min(MAX_SKIN_BONES, pMeshContainer->pSkinInfo->GetNumBones());

		if (FAILED (pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh (
												pMesh,
												D3DXMESH_MANAGED | D3DXMESHOPT_VERTEXCACHE,
												pMeshContainer->NumPaletteEntries,
												pAdjacency,
												NULL, NULL, NULL,
												&pMeshContainer->NumInfl,
												&pMeshContainer->NumAttributeGroups,
												&pMeshContainer->pBoneCombinationBuf,
												&pTempMesh)))
			goto e_Exit;

		pMesh->Release ();
		pMesh = pTempMesh;
		pTempMesh = NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	// optimalizacia
#ifdef MAKE_MESH_OPTIMIZATION
	if (pAdjacency)
	{
//		pMesh->GenerateAdjacency ()	 // ???
		pMesh->OptimizeInplace ( \
			D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, \
			pAdjacency, NULL, NULL, NULL);
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	pMeshContainer->MeshData.pMesh = pMesh;
	pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

	*ppNewMeshContainer = pMeshContainer;
	return S_OK;

e_Exit:
	pMesh->Release ();
	SAFE_DELETE (pMeshContainer)
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//							CDeallocateHierarchy
//////////////////////////////////////////////////////////////////////////

HRESULT CDeallocateHierarchy::DestroyFrame (LPD3DXFRAME pFrameToFree)
{
	D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameToFree;
	SAFE_DELETE (pFrame);
	return S_OK; 
}

//---------------------------------
HRESULT CDeallocateHierarchy::DestroyMeshContainer (LPD3DXMESHCONTAINER pMeshContainerToFree)
{
	D3DXMESHCONTAINER_DERIVED *pContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerToFree;
	SAFE_DELETE (pContainer);
	return S_OK;
}

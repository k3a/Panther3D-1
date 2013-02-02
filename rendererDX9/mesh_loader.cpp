
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	CP3DMeshLoader class definition
//-----------------------------------------------------------------------------


#include "mesh.h"
#include <Dxfile.h>
#include <rmxftmpl.h>
#include <rmxfguid.h>

// prototypes
void CheckTextures (D3DXFRAME_DERIVED *pFrame, DWORD dwUseThisPainting, ezxml_t xmlTextures);
bool ConvertToKeyframedAnimations (D3DXFRAME_DERIVED *pFrameRoot, LPD3DXANIMATIONCONTROLLER pAnimController);


REGISTER_SINGLE_CLASS (CP3DMeshLoader, IP3DRENDERER_MESHLOADER); // zaregistruj tuto tøídu


bool FindTextureIndexTagByName (ezxml_t xmlTextures, char* szTextureName, DWORD &dwIdx);


ConVar pCVr_use_loadcaching("r_use_loadcaching", "1", CVAR_ARCHIVE | CVAR_NOT_STRING, "Model|Texture|Shader caching on/off");


//---------------------------------
CP3DMeshLoader::CP3DMeshLoader ()
{
	m_ModelCache = NULL;
	m_dwCacheSize = 0;
	m_dwNextFreeID = 0;
	m_bInCachingState = false;
}

//---------------------------------
CP3DMeshLoader::~CP3DMeshLoader ()
{
	UnloadAll ();
	SAFE_DELETE_ARRAY (m_ModelCache)
}

// v pripade potreby cache buffer rastie vzdy o INIT_CACHE_SIZE poloziek
//---------------------------------
bool CP3DMeshLoader::ReallocateCacheBuffer()
{
	if (m_ModelCache == NULL)
	{
		m_ModelCache = new MODEL_CACHE[INIT_CACHE_SIZE];
		if (m_ModelCache == NULL) return false;
		memset (m_ModelCache, 0, sizeof(MODEL_CACHE) * INIT_CACHE_SIZE);
		m_dwCacheSize = INIT_CACHE_SIZE;
		return true;
	}
	else
	{
		MODEL_CACHE	*pTempCache = new MODEL_CACHE[m_dwCacheSize + INIT_CACHE_SIZE];
		if (pTempCache == NULL) return false;
		memset (pTempCache, 0, sizeof(MODEL_CACHE) * (m_dwCacheSize + INIT_CACHE_SIZE));
		memcpy (pTempCache, m_ModelCache, sizeof(MODEL_CACHE) * m_dwCacheSize);
		delete[] m_ModelCache;
		m_ModelCache = pTempCache;
		m_dwCacheSize += INIT_CACHE_SIZE;
		return true;
	}
	return false;
}

// uvolni model (v cache tabulke) s indexom wIdx
//---------------------------------
void CP3DMeshLoader::UnloadCacheItem (DWORD dwIdx)
{
	if (m_ModelCache == NULL) return;

	if (m_ModelCache[dwIdx].filename)
		delete[] m_ModelCache[dwIdx].filename;

	if (m_ModelCache[dwIdx].pMeshObject)		// unload mesh objektu
	{
		STAT(STAT_ALL_VERTS, -(int)((CP3DMesh*)m_ModelCache[dwIdx].pMeshObject)->GetVertexCount());
		D3DXFRAME *pFrameRoot;
		if (m_ModelCache[dwIdx].LoadFlags >= MESH_LOAD_HIERARCHY)
		{
			pFrameRoot = ((CP3DMeshHierarchy*)m_ModelCache[dwIdx].pMeshObject)->m_pFrameRoot;
			((CP3DMeshHierarchy*)m_ModelCache[dwIdx].pMeshObject)->m_dwMeshID = 0xFFFFFFFF;
		}
		else
		{
			pFrameRoot = ((CP3DMesh*)m_ModelCache[dwIdx].pMeshObject)->m_pFrameRoot;
			((CP3DMesh*)m_ModelCache[dwIdx].pMeshObject)->m_dwMeshID = 0xFFFFFFFF;
		}

		if (pFrameRoot)
		{
			CDeallocateHierarchy Dealloc;
			D3DXFrameDestroy (pFrameRoot, &Dealloc);
		}
		delete m_ModelCache[dwIdx].pMeshObject;
	}
	memset (&m_ModelCache[dwIdx], 0, sizeof(MODEL_CACHE));
}

// po volani funkcie budu vsetky (okrem tych vytvorenych s flagom CreateNew) existujuce mesh objekty nefunkcne !
//---------------------------------
void CP3DMeshLoader::UnloadAll ()
{
	m_dwNextFreeID = 0;
	if (m_ModelCache)
	{
		for (WORD i=0; i<m_dwCacheSize; i++)
			UnloadCacheItem (i);
		memset (m_ModelCache, 0, m_dwCacheSize * sizeof(MODEL_CACHE));
	}
	if (m_dwCacheSize > INIT_CACHE_SIZE)
	{
		SAFE_DELETE_ARRAY (m_ModelCache)
		m_dwCacheSize = 0;
		ReallocateCacheBuffer ();
	}
}

//---------------------------------
CP3DMesh* CP3DMeshLoader::DuplicateMesh (CP3DMesh *pMeshOrigin, DWORD dwUseThisPainting, ezxml_t xmlTextures)
{
	CP3DMesh *pMeshRet = NULL;

	if (pMeshOrigin == NULL) return NULL;
	if (pMeshRet = new CP3DMesh (*pMeshOrigin))	// kopirovaci konstruktor
	{
		// kontrola na existujuce textury v danej painting-verzii
		if (xmlTextures && pMeshOrigin->m_pFrameRoot)
			CheckTextures (pMeshOrigin->m_pFrameRoot, dwUseThisPainting, xmlTextures);
		pMeshRet->m_dwPaintingVersion = dwUseThisPainting;
	}
	return pMeshRet;
}

// vyplni tabulku nazvami textur a "ich" 7mt materialov
// vrati pocet vyplnenych poloziek
//---------------------------------
DWORD CP3DMeshLoader::Fetch7mt (const char *szFileName, IN OUT MESH_MATERIAL_ASSOCIATION TableToFill[], DWORD dwTableSize)
{
	DWORD dwNextFreeIndex = 0;
	ID3DXFile *pFile = NULL;
	ID3DXFileEnumObject *pEnum = NULL;
	ID3DXFileData *pMeshData = NULL, *pMeshMaterialListData = NULL;
	ID3DXFileData *pMaterialData = NULL, *pTextureData = NULL;

	if (szFileName == NULL) return 0;
	// Create the enumeration object
	if (FAILED (D3DXFileCreate(&pFile)))
		return 0;
	// Register the standard templates
	if (FAILED (pFile->RegisterTemplates ((LPVOID)D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES)))
		{pFile->Release(); return 0;}

	// nacti data z filesystemu
	D3DXF_FILELOADMEMORY mem; BYTE* bData=NULL; ULONG dwSize=0;
	FSFILE* fp = g_pFS->Load(szFileName, bData, dwSize);
	if ((fp==NULL) || (dwSize<=0)) return 0;
	mem.lpMemory = bData;
	mem.dSize = dwSize;

	// Create the enumeration object
	if (FAILED (pFile->CreateEnumObject ((LPVOID)&mem, D3DXF_FILELOAD_FROMMEMORY, &pEnum)))
		{pFile->Release(); g_pFS->UnLoad(fp, bData); return 0;}

	// plati iba pre global material-list
	SIZE_T	numChildren1, numChildren2;
	char		*szStart, *szEnd;
	int		intLenght;

	if (SUCCEEDED (pEnum->GetChildren (&numChildren1)))
		for (SIZE_T i=0; (dwNextFreeIndex < dwTableSize) && (i < numChildren1); i++)
			if (SUCCEEDED (pEnum->GetChild (i, &pMaterialData)))
			{
				GUID guid;
				if (SUCCEEDED (pMaterialData->GetType (&guid)))
					if (guid == TID_D3DRMMaterial)							// material	{3D82AB4D-62DA-11cf-AB39-0020AF71E433}
					{
						char sz7mtFileName[MAX_PATH];
						SIZE_T szSize = MAX_PATH;
						if (SUCCEEDED (pMaterialData->GetName (sz7mtFileName, &szSize)))
							// test na 7mt-material
							if (szStart = strstr (sz7mtFileName, "---"))
								if ((szEnd = strstr (szStart+3, "---")) && ((intLenght = (int)(szEnd - szStart)) > 3))
								{
									*szEnd = 0;
									for (long i=0; i<intLenght-2; i++)
										sz7mtFileName[i] = *(szStart+3+i);
									strcat (sz7mtFileName, ".7mt");
									SIZE_T dataSize = 0;
									BYTE *ppMaterialData = NULL;
									if (SUCCEEDED (pMaterialData->Lock (&dataSize, (LPCVOID*)&ppMaterialData)))
									{
										if ((ppMaterialData != NULL) && (dataSize == sizeof(MESH_RAW_MATERIAL)))
										{
											// ulozenie nazvu 7mt suboru a hodnoty materialu do tabulky
											memcpy (&TableToFill[dwNextFreeIndex].material, ppMaterialData, sizeof(MESH_RAW_MATERIAL));
											TableToFill[dwNextFreeIndex].sz7mtMaterialName = new char [strlen(sz7mtFileName) + 1];
											strcpy (TableToFill[dwNextFreeIndex].sz7mtMaterialName, sz7mtFileName);

											if (SUCCEEDED (pMaterialData->GetChildren (&numChildren2)))
												for (SIZE_T j=0; j<numChildren2; j++)
													if (SUCCEEDED (pMaterialData->GetChild (j, &pTextureData)))
													{
														if (SUCCEEDED (pTextureData->GetType (&guid)))
															if (guid == TID_D3DRMTextureFilename)		// texture_file_name {A42790E1-7810-11cf-8F52-0040333594A3}
															{
																szSize = 0;
																char *ppString = NULL;
																if (SUCCEEDED (pTextureData->Lock (&szSize, (LPCVOID*)&ppString)))
																{
																	if (ppString)		// ulozenie nazvu textury do tabulky
																	{
																		TableToFill[dwNextFreeIndex].szTextureName = new char [strlen(ppString) + 1];
																		strcpy (TableToFill[dwNextFreeIndex].szTextureName, ppString);
																	}
																	pTextureData->Unlock ();
																	pTextureData->Release ();
																	break;
																}
															}
															pTextureData->Release ();
													}
											dwNextFreeIndex++;
										}
										pMaterialData->Unlock ();
									}
								}
					}
				pMaterialData->Release ();
			}

	// Release used COM objects
	pEnum->Release();
	pFile->Release();
	g_pFS->UnLoad(fp, bData);
	return dwNextFreeIndex;
}

//---------------------------------
bool FindTextureIndexTagByName (ezxml_t xmlTextures, char* szTextureName, DWORD &dwIdx)
{
	ezxml_t pTextureTag, pCoordsTag, pPaintingTag;
	DWORD i;
	char szTagName[16];

	if ((xmlTextures == NULL) || (szTextureName == NULL)) return false;

	i=0;
	strcpy (szTagName, "texture0");
	while (pTextureTag = g_pXML->Child (xmlTextures, szTagName))
	{
		if (pCoordsTag = g_pXML->Child (pTextureTag, "coords0"))
			if (pPaintingTag = g_pXML->Child (pCoordsTag, "painting0"))
				if (pPaintingTag->txt)
				{
					char *pTextureName = strrchr (pPaintingTag->txt, '\\');
					if (pTextureName == NULL)
						pTextureName = pPaintingTag->txt;
					else
						pTextureName++;
					if (stricmp (pTextureName, szTextureName) == 0)
						{dwIdx = i; return true;}
				}
		sprintf (szTagName, "texture%u", ++i);
	}

	return false;
}


//---------------------------------
inline DWORD GetTexCoordSetsCount(DWORD dwFVF)
{
	return ((dwFVF & D3DFVF_TEXCOUNT_MASK)>>D3DFVF_TEXCOUNT_SHIFT);
}

//---------------------------------
DWORD GetTexCoordSetSize(DWORD dwFVF,BYTE Index)
{
	// Check if there are any textures defined
	if ( !(dwFVF & D3DFVF_TEXCOUNT_MASK) )
		return 0;	// Error Code

	const DWORD x_rgdwTexCoordSize[] = { 8, 12, 16, 4};
	return x_rgdwTexCoordSize[(dwFVF >> (16 + Index * 2)) & 0x3];
}

// popis funkcie: http://directx.netfirms.com/crackfvf.htm
// v pripade chyby vrati 0xFFFFFFFF
//---------------------------------
DWORD GetFVFComponentOffset (DWORD dwFVF,DWORD dwFVFComponent,BYTE iTexCoordIndex)
{
	if (dwFVF == 0)
		return 0xFFFFFFFF;	// Error Code

	// If the component isn't part of the vertex definition then return error
	// with one exception, the D3DFVF_TEXTUREFORMAT2 flag
	if (((dwFVFComponent & dwFVF) != dwFVFComponent) && (dwFVFComponent != 0))
		return 0xFFFFFFFF;	// Error Code

	// Internal Macro for checking whether we arrived to
	// the required component, and returning the offset if so
	#define RETURN_IF_MATCH_FVF(FVFCode) if(dwFVFComponent==FVFCode){return oCur;}

	DWORD oCur;
	DWORD dwPosition;
	BYTE FloatSize = sizeof(float);
	BYTE DWORDSize = sizeof(DWORD);

	// Start from Zero. Since this is a general function, it doesn't
	// require a strictly valid FVF. For example, we might recieve an
	// FVF code without D3DFVF_XYZ and process it succesfully.
	oCur = 0;

	// Mask position
	dwPosition = dwFVF & D3DFVF_POSITION_MASK;

	// Position is always at the start
	if (dwFVFComponent != 0)
		RETURN_IF_MATCH_FVF(dwPosition);

	// Increase the offset
	switch (dwPosition)
	{
	case D3DFVF_XYZ:
		oCur+=3*FloatSize;	// X,Y and Z
		break;
	case D3DFVF_XYZRHW:
		oCur+=4*FloatSize;	// X,Y,Z and RHW
		break;
	case D3DFVF_XYZB1:
		oCur+=3*FloatSize+FloatSize;	// X,Y,Z and 1 Blending Weight (Beta)
		break;
	case D3DFVF_XYZB2:
		oCur+=3*FloatSize+2*FloatSize;	// X,Y,Z and 2 Betas
		break;
	case D3DFVF_XYZB3:
		oCur+=3*FloatSize+3*FloatSize;	// X,Y,Z and 3 Betas
		break;
	case D3DFVF_XYZB4:
		oCur+=3*FloatSize+4*FloatSize;	// X,Y,Z and 4 Betas
		break;
	case D3DFVF_XYZB5:
		oCur+=3*FloatSize+5*FloatSize;	// X,Y,Z and 5 Betas
		break;
	}

	// Normals come next
	RETURN_IF_MATCH_FVF(D3DFVF_NORMAL);
	if ( dwFVF & D3DFVF_NORMAL )
		oCur += 3*FloatSize;	// X,Y and Z
	// Point Size
	RETURN_IF_MATCH_FVF(D3DFVF_PSIZE);
	if ( dwFVF & D3DFVF_PSIZE )
		oCur += FloatSize;	// A single float
	// Vertex Diffuse
	RETURN_IF_MATCH_FVF(D3DFVF_DIFFUSE);
	if ( dwFVF & D3DFVF_DIFFUSE )
		oCur += DWORDSize;	// BGRA encoded in one DWORD
	// Vertex Specular
	RETURN_IF_MATCH_FVF(D3DFVF_SPECULAR);
	if ( dwFVF & D3DFVF_SPECULAR )
		oCur += DWORDSize;	// BGRA encoded in one DWORD

	// Finally, Texture Coordinate Sets
	// Eight sets, each one might have 1,2,3 or 4 floats
	BYTE cTexCoords;
	// Get the number of Coordinate Sets
	cTexCoords = (BYTE)GetTexCoordSetsCount(dwFVF);
	if ( cTexCoords == 0 )
	{
		// If we arrived here, then we recieved an invalid
		// (unsupported) FVF component code (e.g. D3DFVF_LASTBETA_UBYTE4)
		return 0xFFFFFFFF;	// Error Code
	}

	// There are Texture Coordinate Sets, we'll loop around
	// the number of sets defined, with each loop, we'll
	// construct several FVFs and compare them to the passed FVF
	// component 'dwFVFComponent'. If one matches, then we return
	// the final offset value, otherwise, we'll just increase the
	// offset by the size of the set at current index.
	for (BYTE i=0;i<cTexCoords;i++)
	{
		// Get the size of this coordinate set
		BYTE Size = (BYTE)GetTexCoordSetSize(dwFVF,i);

		if (Size == 0)	// Invalid Texture Coordinate Set
			return 0xFFFFFFFF;	// Error Code

		if (Size == FloatSize)
		{
			// 1D Texture Coordinate Set
			RETURN_IF_MATCH_FVF((DWORD)D3DFVF_TEXCOORDSIZE1(i));
			oCur += FloatSize;
		}

		if (Size == FloatSize*2)
		{
			// 2D Texture Coordinate Set
			if (i == iTexCoordIndex)
			{
				RETURN_IF_MATCH_FVF((DWORD)D3DFVF_TEXCOORDSIZE2(i));
			}
			oCur += FloatSize*2;
		}

		if (Size == FloatSize*3)
		{
			// 3D Texture Coordinate Set
			RETURN_IF_MATCH_FVF((DWORD)D3DFVF_TEXCOORDSIZE3(i));
			oCur += FloatSize*3;
		}

		if (Size == FloatSize*4)
		{
			// 4D Texture Coordinate Set
			RETURN_IF_MATCH_FVF((DWORD)D3DFVF_TEXCOORDSIZE4(i));
			oCur += FloatSize*4;
		}
	}	// For each coordinate set Loop

	// If we arrived here, then we recieved an invalid
	// (unsupported) FVF component code that contains
	// an invalid Texture-Coordinate-Set definition.
	return 0xFFFFFFFF;	// Error Code
}


//---------------------------------
CP3DMesh* CP3DMeshLoader::LoadMeshFromFile (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, \
												ezxml_t xmlTextures, DWORD dwUseThisPainting)
{
	DWORD i, j, dwNumMaterials;
	LPD3DXMESH pMesh, pTempMesh;
	D3DXMESHCONTAINER_DERIVED *pMeshContainer;
	DWORD dw7mtMaterialsNum = 0;
	MESH_MATERIAL_ASSOCIATION Mesh7mtMaterials[MAX_MESH_MATERIALS];
	CP3DMesh *pMeshOut = NULL;
	LPD3DXBUFFER pD3DXMtrlBuffer = NULL, pAdjacencyBuffer = NULL;

	if (filename == NULL) return NULL;
	pMeshOut = new CP3DMesh ();
	if (pMeshOut == NULL) return NULL;
	pMeshOut->m_pFrameRoot = new D3DXFRAME_DERIVED;
	if (pMeshOut->m_pFrameRoot == NULL) goto quit_hand;
	pMeshContainer = new D3DXMESHCONTAINER_DERIVED;
	if (pMeshContainer == NULL) goto quit_hand;
	pMeshOut->m_pFrameRoot->pMeshContainer = pMeshContainer;

	pMeshOut->m_LoadFlags = MeshLoadFlags;

	//////////////////////////////////////////////////////////////////////////
	// nacitanie X modelu
	FSFILE	*pFile;
	BYTE		*pData;
	DWORD		dwSize;
	HRESULT	hr = E_FAIL;

	if (pFile = g_pFS->Load (filename, pData, dwSize))
	{
		hr = D3DXLoadMeshFromXInMemory ((LPCVOID)pData, dwSize, D3DXMESH_MANAGED, \
										g_pD3DDevice, &pAdjacencyBuffer, &pD3DXMtrlBuffer, NULL, \
										&dwNumMaterials, &pMesh);
		g_pFS->UnLoad (pFile, pData);
	}
	if (FAILED (hr)) goto quit_hand;

	// urcenie poctu textur, koordinatov a painting-verzii
	pMeshContainer->NumMaterials = max (1, dwNumMaterials);
	pMeshContainer->dwNumPaintings = 1;
	pMeshOut->m_dwPaintingVersion = dwUseThisPainting;
	pMeshContainer->dwNumCoords = 1;
	if (xmlTextures)
	{
		const char *szTemp;
		if (szTemp = g_pXML->Attr (xmlTextures, "paint_num"))
			pMeshContainer->dwNumPaintings = (DWORD)atoi (szTemp);
		if (szTemp = g_pXML->Attr (xmlTextures, "coord_num"))
			pMeshContainer->dwNumCoords = (DWORD)atoi (szTemp);
	}
	pMeshContainer->dwNumPaintings = max (1, pMeshContainer->dwNumPaintings);
	pMeshContainer->dwNumCoords = max (1, pMeshContainer->dwNumCoords);
	if (pMeshContainer->dwNumPaintings <= dwUseThisPainting)
		pMeshOut->m_dwPaintingVersion = 0;		// chyba, pozadovana painting-verzia nie je v xml definovana

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
				goto quit_hand;

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
			if (xmlTextures)
			{
				ezxml_t pTextureTag, pCoordsTag;
				const char *szTemp;
				char szCoordsTag[16];
				if (pTextureTag = g_pXML->Child (xmlTextures, "texture0"))
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
			CON(MSG_CON_ERR, "MeshLoader: CalculateMeshTangents() function failed");
	}

	//////////////////////////////////////////////////////////////////////////
	// nacitanie informacii o materialoch a texturach z x suboru
	D3DXMATERIAL* d3dxMaterials;
	d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	pMeshContainer->pMaterials	= new D3DXMATERIAL[pMeshContainer->NumMaterials];
	pMeshContainer->pTextures	= new LPDIRECT3DTEXTURE9 [pMeshContainer->NumMaterials * pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings];
	pMeshContainer->p7MTmaterials = new DWORD [pMeshContainer->NumMaterials];

	if ((pMeshContainer->pMaterials == NULL) || (pMeshContainer->pTextures == NULL) || \
														(pMeshContainer->p7MTmaterials == NULL))
		goto quit_hand;
	memset (pMeshContainer->pMaterials, 0, sizeof(D3DXMATERIAL) * pMeshContainer->NumMaterials);
	memset (pMeshContainer->pTextures, 0, sizeof(LPDIRECT3DTEXTURE9) * pMeshContainer->NumMaterials * pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings);
	memset (pMeshContainer->p7MTmaterials, 0xFF, sizeof(DWORD) * pMeshContainer->NumMaterials);

	// nacitanie informacii o 7mt materialoch
	dw7mtMaterialsNum = Fetch7mt (filename, Mesh7mtMaterials, MAX_MESH_MATERIALS);

	// nacitanie materialov
	if (dwNumMaterials > 0)
	{
		for (i=0; i<dwNumMaterials; i++)
		{
			pMeshContainer->pMaterials[i].MatD3D = d3dxMaterials[i].MatD3D;
			// do x suboru sa neexportuje udaj o ambient zlozke materialov, upravit podla potreby
			pMeshContainer->pMaterials[i].MatD3D.Ambient = D3DXCOLOR (1.0f, 1.0f, 1.0f, 1.0f);
			// vypnut Power ak nie je zadana Specular zlozka
			D3DCOLORVALUE &colorSpec = pMeshContainer->pMaterials[i].MatD3D.Specular;
			if ((colorSpec.r == 0) && (colorSpec.g == 0) && (colorSpec.b == 0))
				pMeshContainer->pMaterials[i].MatD3D.Power = 0.0f;

			// nacitanie 7mt materialu, ak je k danemu povrchu priradeny
			#define MATERIAL_FLOAT_TOLERANCE		0.001f
			for (j=0; j<dw7mtMaterialsNum; j++)
				if ((Mesh7mtMaterials[j].sz7mtMaterialName != NULL) && (!Mesh7mtMaterials[j].bAlreadyUsed))
				{
					// porovnanie textur ak su dostupne
					if ((Mesh7mtMaterials[j].szTextureName != NULL) && (d3dxMaterials[i].pTextureFilename != NULL))
						if (stricmp (Mesh7mtMaterials[j].szTextureName, d3dxMaterials[i].pTextureFilename) != 0)
							continue;
					// porovnanie materialov
					if (fabs (Mesh7mtMaterials[j].material.rDif - d3dxMaterials[i].MatD3D.Diffuse.r) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.gDif - d3dxMaterials[i].MatD3D.Diffuse.g) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.bDif - d3dxMaterials[i].MatD3D.Diffuse.b) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.aDif - d3dxMaterials[i].MatD3D.Diffuse.a) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.rSpec - d3dxMaterials[i].MatD3D.Specular.r) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.gSpec - d3dxMaterials[i].MatD3D.Specular.g) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.bSpec - d3dxMaterials[i].MatD3D.Specular.b) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.rEm - d3dxMaterials[i].MatD3D.Emissive.r) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.gEm - d3dxMaterials[i].MatD3D.Emissive.g) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.bEm - d3dxMaterials[i].MatD3D.Emissive.b) > MATERIAL_FLOAT_TOLERANCE) continue;
					if (fabs (Mesh7mtMaterials[j].material.specPower - d3dxMaterials[i].MatD3D.Power) > MATERIAL_FLOAT_TOLERANCE) continue;

					// materialy su zhodne
					char szBuf[MAX_PATH] = "materials\\models\\";
					strcat (szBuf, Mesh7mtMaterials[j].sz7mtMaterialName);
					// 7mt material loading
					pMeshContainer->p7MTmaterials[i] = g_pMaterialManager->LoadMaterial (szBuf);
					//if ((pMeshContainer->p7MTmaterials[i] == 0xFFFFFFFF) && (g_pConsole != NULL)) ERROR SE JIZ VYPSAL V MATERIAL MANAGERU
					//	CON(MSG_CON_ERR, "MeshLoader: Can't load material %s!", szBuf);
					Mesh7mtMaterials[j].bAlreadyUsed = true;
					break;
				}
			// #######################################################
			// naèítanie textúr
			if ((d3dxMaterials[i].pTextureFilename != NULL) && (d3dxMaterials[i].pTextureFilename[0] != 0))
			{
				char szTag[16];
				ezxml_t	pTextureTag = NULL;
				DWORD dwTextureIdx = 0xFFFFFFFF;

				pMeshContainer->pMaterials[i].pTextureFilename = new char [strlen (d3dxMaterials[i].pTextureFilename) + 1];
				strcpy (pMeshContainer->pMaterials[i].pTextureFilename, d3dxMaterials[i].pTextureFilename);

				if (FindTextureIndexTagByName (xmlTextures, d3dxMaterials[i].pTextureFilename, dwTextureIdx))
				{
					sprintf (szTag, "texture%u", dwTextureIdx);
					pTextureTag = g_pXML->Child (xmlTextures, szTag);
				}

				DWORD dwCoordsXPaintings = pMeshContainer->dwNumCoords * pMeshContainer->dwNumPaintings;
				for (j=0; j < pMeshContainer->dwNumCoords; j++)		// pre jednotlive koordinaty
				{
					char szBuf[MAX_PATH] = "textures\\models\\";
					if (pTextureTag == NULL)		// pouzit texturu z x-suboru
					{
						strcat (szBuf, d3dxMaterials[i].pTextureFilename);
						pMeshContainer->pTextures[ i * dwCoordsXPaintings + j*pMeshContainer->dwNumPaintings + pMeshOut->m_dwPaintingVersion] = g_TextureLoader.CreateTexture (szBuf);
					}
					else
					{
						// ziskanie nazvu textury z xml suboru
						ezxml_t	pCoordsTag, pPaintingTag;
						sprintf (szTag, "coords%u", j);
						if (pCoordsTag = g_pXML->Child (pTextureTag, szTag))
						{
							sprintf (szTag, "painting%u", pMeshOut->m_dwPaintingVersion);
							if (pPaintingTag = g_pXML->Child (pCoordsTag, szTag))
								if (pPaintingTag->txt)
								{
									strcat (szBuf, pPaintingTag->txt);
									pMeshContainer->pTextures[ i * dwCoordsXPaintings + j*pMeshContainer->dwNumPaintings + pMeshOut->m_dwPaintingVersion] = g_TextureLoader.CreateTexture (szBuf);
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
	SAFE_RELEASE (pD3DXMtrlBuffer)

	//////////////////////////////////////////////////////////////////////////
	// optimalizacia meshu
#ifdef MAKE_MESH_OPTIMIZATION
	if (pAdjacencyBuffer)
	{
		LPDWORD pdwAdjacency;
		pdwAdjacency = (LPDWORD)pAdjacencyBuffer->GetBufferPointer();
		pMesh->OptimizeInplace ( \
					D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, \
					pdwAdjacency, NULL, NULL, NULL);
	}
	SAFE_RELEASE (pAdjacencyBuffer)
#endif

	//////////////////////////////////////////////////////////////////////////
	pMeshContainer->MeshData.pMesh = pMesh;
	pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

	return pMeshOut;

quit_hand:
	SAFE_RELEASE (pD3DXMtrlBuffer)
	SAFE_RELEASE (pAdjacencyBuffer)
	if (pMeshOut)
	{
		if (pMeshOut->m_pFrameRoot)
		{
			D3DXMESHCONTAINER_DERIVED *pContainerToUnload;
			pContainerToUnload = (D3DXMESHCONTAINER_DERIVED*)pMeshOut->m_pFrameRoot->pMeshContainer;
			SAFE_DELETE (pContainerToUnload)
		}
		SAFE_DELETE (pMeshOut->m_pFrameRoot)
		delete pMeshOut;
	}
	return NULL;
}

// vyhlada framy, ktore "posobia" na dany meshcontainer a nastavi
// ppBoneTransformMatrices - smerniky na matCombined matice tychto framov
//---------------------------------
void CP3DMeshLoader::SetSkinMatrixPtrs (D3DXFRAME_DERIVED *pFrameRoot, D3DXFRAME_DERIVED *pFrameBase)
{
	DWORD dwNumBones;
	D3DXMESHCONTAINER_DERIVED	*pMeshContainer;

	if (pFrameBase == NULL) pFrameBase = pFrameRoot;
	pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pFrameBase->pMeshContainer;

	while (pMeshContainer && pMeshContainer->pSkinInfo)
	{
		dwNumBones = pMeshContainer->pSkinInfo->GetNumBones();
		for (DWORD i = 0; i < dwNumBones; i++)
		{
			D3DXFRAME_DERIVED *pFrame;
			const char *szBoneName;
			if (szBoneName = pMeshContainer->pSkinInfo->GetBoneName(i))
				if (pFrame = pFrameRoot->Find(szBoneName))		// find matching name in frames
					pMeshContainer->ppBoneTransformMatrices[i] = &pFrame->matCombined;	// match frame to bone
		}
		SAFE_RELEASE (pMeshContainer->pSkinInfo)
		pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainer->pNextMeshContainer;
	}

	// rekurzia pre surodencov
	if (pFrameBase->pFrameSibling)
		SetSkinMatrixPtrs (pFrameRoot, (D3DXFRAME_DERIVED*)pFrameBase->pFrameSibling);
	// rekurzia pre potomkov
	if (pFrameBase->pFrameFirstChild)
		SetSkinMatrixPtrs (pFrameRoot, (D3DXFRAME_DERIVED*)pFrameBase->pFrameFirstChild);
}

//---------------------------------
void CheckTextures (D3DXFRAME_DERIVED *pFrame, DWORD dwUseThisPainting, ezxml_t xmlTextures)
{
	DWORD	i, j;
	D3DXMESHCONTAINER_DERIVED	*pContainer = (D3DXMESHCONTAINER_DERIVED*)pFrame->pMeshContainer;

	if (pContainer)
	{
		DWORD dwCoordsXPaintings = pContainer->dwNumCoords * pContainer->dwNumPaintings;

		if (dwUseThisPainting >= pContainer->dwNumPaintings)	// pozaduje sa neexistujuca paint-verzia
			dwUseThisPainting = 0;

		bool bNeedToLoad = true;
		if (pContainer->pTextures)
		{
			for (i=0; (i < pContainer->NumMaterials) && bNeedToLoad; i++)
				for (j=0; j < pContainer->dwNumCoords; j++)
					if (NULL != pContainer->pTextures [i * dwCoordsXPaintings + j*pContainer->dwNumPaintings + dwUseThisPainting])
						{bNeedToLoad = false; break;}

			// ak dana painting-verzia nie je v tabulke, tak sa naloaduje
			if (bNeedToLoad && pContainer->pMaterials)
			{
				for (i=0; i < pContainer->NumMaterials; i++)
					if (pContainer->pMaterials[i].pTextureFilename)
					{
						char szTag[16];
						ezxml_t	pTextureTag = NULL;
						DWORD dwTextureIdx = 0xFFFFFFFF;

						if (FindTextureIndexTagByName (xmlTextures, pContainer->pMaterials[i].pTextureFilename, dwTextureIdx))
						{
							sprintf (szTag, "texture%u", dwTextureIdx);
							pTextureTag = g_pXML->Child (xmlTextures, szTag);
						}

						for (j=0; j < pContainer->dwNumCoords; j++)		// pre jednotlive koordinaty
						{
							char szBuf[MAX_PATH] = "textures\\models\\";
							if (pTextureTag == NULL)		// pouzit texturu z x-suboru
							{
								strcat (szBuf, pContainer->pMaterials[i].pTextureFilename);
								pContainer->pTextures[i * dwCoordsXPaintings + j*pContainer->dwNumPaintings + dwUseThisPainting] = g_TextureLoader.CreateTexture (szBuf);
							}
							else
							{
								// ziskanie nazvu textury z xml suboru
								ezxml_t	pCoordsTag, pPaintingTag;
								sprintf (szTag, "coords%u", j);
								if (pCoordsTag = g_pXML->Child (pTextureTag, szTag))
								{
									sprintf (szTag, "painting%u", dwUseThisPainting);
									if (pPaintingTag = g_pXML->Child (pCoordsTag, szTag))
										if (pPaintingTag->txt)
										{
											strcat (szBuf, pPaintingTag->txt);
											pContainer->pTextures[i * dwCoordsXPaintings + j*pContainer->dwNumPaintings + dwUseThisPainting] = g_TextureLoader.CreateTexture (szBuf);
										}
								}
							}
						}
					}
			}
		}
	}

	if (pFrame->pFrameSibling != NULL)
		CheckTextures ((D3DXFRAME_DERIVED*)pFrame->pFrameSibling, dwUseThisPainting, xmlTextures);

	if (pFrame->pFrameFirstChild != NULL)
		CheckTextures ((D3DXFRAME_DERIVED*)pFrame->pFrameFirstChild, dwUseThisPainting, xmlTextures);
}

//---------------------------------
CP3DMeshHierarchy* CP3DMeshLoader::DuplicateMeshHierarchy (CP3DMeshHierarchy *pMeshOrigin, DWORD dwUseThisPainting, ezxml_t xmlTextures)
{
	DWORD i;
	CP3DMeshHierarchy *pMeshOut;

	if (pMeshOrigin == NULL) return NULL;
	pMeshOut = new CP3DMeshHierarchy (*pMeshOrigin);		// kopirovaci kontruktor
	if (pMeshOut == NULL) return NULL;
	pMeshOut->m_animController = NULL;
	pMeshOut->m_currentTime = 0.0f;
	pMeshOut->m_dwAnimationSetNum = 0;
	pMeshOut->m_dwPaintingVersion = 0;

	//////////////////////////////////////////////////////////////////////////
	// kontrola na existujuce textury v danej painting-verzii
	if (xmlTextures && pMeshOrigin->m_pFrameRoot)
	{
		CheckTextures (pMeshOrigin->m_pFrameRoot, dwUseThisPainting, xmlTextures);
		pMeshOut->m_dwPaintingVersion = dwUseThisPainting;
	}

	//////////////////////////////////////////////////////////////////////////
	// uprava animation controllera
	if (pMeshOrigin->m_animController)
	{
		// pocet trackov musi byt dvojnasobok dwMaxAnimTracks, aby
		// bolo mozne miesat prechody medzi jednotlivymi animaciami
		if (FAILED (pMeshOrigin->m_animController->CloneAnimationController( \
					pMeshOrigin->m_animController->GetMaxNumAnimationOutputs(), \
					pMeshOrigin->m_animController->GetNumAnimationSets(), \
					MAX_ANIM_TRACKS_NUM * 2, \
					pMeshOrigin->m_animController->GetMaxNumEvents(), \
					&pMeshOut->m_animController)))
			goto quit_hand1;

		for (i=0; i < MAX_ANIM_TRACKS_NUM * 2; i++)
			pMeshOut->m_animController->SetTrackEnable (i, FALSE);	// vypnutie vsetkych trackov
		// FIXME: odstranit
		// debug!!!
		pMeshOut->m_animController->SetTrackEnable (0, TRUE);
		// debug!!!

		// ulozenie poctu animacii v .x subore
		pMeshOut->m_dwAnimationSetNum = pMeshOut->m_animController->GetNumAnimationSets();

		for (i=0; i < MAX_ANIM_TRACKS_NUM; i++)
		{
			pMeshOut->m_TrackTable[i].dwAnimationIndex = 0xffffffff;		// ziadna animacia
			pMeshOut->m_TrackTable[i].bHighOrLow = false;
		}
	}

	return pMeshOut;

quit_hand1:
	SAFE_DELETE (pMeshOut)
	return NULL;
}


// pouzivat opatrne! meni strukturu kostry!
// odstranuje vsetky framy(kosti), ktore nie su potrebne, moze odstranit az polovicu kosti
// vrati pocet odstranenych kosti
//---------------------------------
DWORD CP3DMeshLoader::RemoveUnusedFrames (D3DXFRAME_DERIVED &pFrame, bool bFirstCall)
{
	static DWORD dwDeletedBones;
	static D3DXFRAME_DERIVED *pParentFrame;
	D3DXFRAME_DERIVED *pSibling, *pChild;

	if (bFirstCall)
		{dwDeletedBones = 0; pParentFrame = NULL;}

	pSibling = (D3DXFRAME_DERIVED*)pFrame.pFrameSibling;
	pChild = (D3DXFRAME_DERIVED*)pFrame.pFrameFirstChild;

	if ((pFrame.Name == NULL) && (pFrame.pMeshContainer == NULL))
		if ((pChild == NULL) && (pParentFrame != NULL))
		{
			pParentFrame->pFrameFirstChild = pFrame.pFrameSibling;
			delete &pFrame;
			dwDeletedBones += 1;
		}

	// rekurzia pre surodencov
	if (pSibling)
		RemoveUnusedFrames ((D3DXFRAME_DERIVED&)*pSibling, false);
	// rekurzia pre potomkov
	if (pChild)
	{
		pParentFrame = &pFrame;
		RemoveUnusedFrames ((D3DXFRAME_DERIVED&)*pChild, false);
	}
	return dwDeletedBones;
}

//---------------------------------
CP3DMeshHierarchy* CP3DMeshLoader::LoadMeshHierarchyFromFile (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures, DWORD dwUseThisPainting)
{
	if (filename == NULL) return NULL;

	bool bSkinned = (MeshLoadFlags == MESH_LOAD_SKINNED);
	CP3DMeshHierarchy *pMeshOut = new CP3DMeshHierarchy();
	if (pMeshOut == NULL) return NULL;

	pMeshOut->m_LoadFlags = MeshLoadFlags;
	pMeshOut->m_dwPaintingVersion = dwUseThisPainting;

	// nacitanie informacii o 7mt materialoch
	DWORD dw7mtMaterialsNum = 0;
	MESH_MATERIAL_ASSOCIATION Mesh7mtMaterials[MAX_MESH_MATERIALS];
	dw7mtMaterialsNum = Fetch7mt (filename, Mesh7mtMaterials, MAX_MESH_MATERIALS);

	// loading mesh objektu
	ALLOC_HIERARCHY_INITIALIZERS	AllocInitializers;
	AllocInitializers.bLoadSkinInfo = bSkinned ? TRUE : FALSE;
	AllocInitializers.Mesh7mtMaterials = Mesh7mtMaterials;
	AllocInitializers.dw7mtMaterialsNum = dw7mtMaterialsNum;
	AllocInitializers.xmlTextures = xmlTextures;
	AllocInitializers.dwUseThisPainting = dwUseThisPainting;

	// nacitanie X modelu
	FSFILE	*pFile;
	BYTE	*pData;
	DWORD	dwSize;
	HRESULT	hr = E_FAIL;

	if (pFile = g_pFS->Load (filename, pData, dwSize))
	{
		CAllocateHierarchy CAlloc (AllocInitializers);
		hr = D3DXLoadMeshHierarchyFromXInMemory ((LPCVOID)pData, dwSize, D3DXMESH_MANAGED, \
										g_pD3DDevice, &CAlloc, NULL, (LPD3DXFRAME*)&pMeshOut->m_pFrameRoot, \
										&pMeshOut->m_animController);
		g_pFS->UnLoad (pFile, pData);
	}
	if (FAILED (hr)) goto quit_hand2;

	//////////////////////////////////////////////////////////////////////////
	// WARN: pouzivat opatrne!
	DWORD dwRemovedBones = RemoveUnusedFrames (*pMeshOut->m_pFrameRoot);
	//////////////////////////////////////////////////////////////////////////
	// nastavenie skin-matic
	if (bSkinned && (pMeshOut->m_pFrameRoot != NULL))
		SetSkinMatrixPtrs (pMeshOut->m_pFrameRoot);


//	ConvertToKeyframedAnimations (pMeshOut->m_pFrameRoot, pMeshOut->m_animController);

//	IsolateAnimations (filename, pMeshOut->m_animController);

/*
	//////////////////////////////////////////////////////////////////////////
	// konverzia animacii na keyframed_animacie
	// FIXME: TODO: toto asi nie je spravna konverzia!
	if (pMeshOut->m_animController)
	{
		DWORD dwAnimSetNum = pMeshOut->m_animController->GetNumAnimationSets ();
		for (DWORD i=0; i<dwAnimSetNum; i++)
		{
			LPD3DXANIMATIONSET pAnimSet;
			if (SUCCEEDED (pMeshOut->m_animController->GetAnimationSet (i, &pAnimSet)))
			{
				LPD3DXKEYFRAMEDANIMATIONSET pKeyFramedAnimSet;
				if (SUCCEEDED(pAnimSet->QueryInterface (IID_ID3DXKeyframedAnimationSet, (LPVOID*)&pKeyFramedAnimSet)))
				{
					pMeshOut->m_animController->UnregisterAnimationSet (pAnimSet);
					pMeshOut->m_animController->RegisterAnimationSet (pKeyFramedAnimSet);
//					pKeyFramedAnimSet->Release();
				}
				pAnimSet->Release ();
			}
		}
	}
*/

	return pMeshOut;

quit_hand2:
	SAFE_DELETE (pMeshOut)
	return NULL;
}

// ak bForceToCreateNew = true, tak sa model "nekopiruje" z cache tabulky, ale vytvori sa vzdy novy
//---------------------------------
IP3DBaseInterface* CP3DMeshLoader::MeshLoading (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, \
				ezxml_t xmlTextures, DWORD dwUseThisPainting, bool bForceToCreateNew)
{
	IP3DBaseInterface	*pMeshOut = NULL;

	if (bForceToCreateNew)
	{
		// nahrat novy model
		if (MeshLoadFlags >= MESH_LOAD_HIERARCHY)
		{
			if (pMeshOut = LoadMeshHierarchyFromFile (filename, MeshLoadFlags, xmlTextures, dwUseThisPainting))
			{
				((CP3DMeshHierarchy*)pMeshOut)->m_bUniqueModel = true;
				((CP3DMeshHierarchy*)pMeshOut)->m_dwMeshID = UNIQUE_MODEL_IDENT;
			}
		}
		else
		{
			if (pMeshOut = LoadMeshFromFile (filename, MeshLoadFlags, xmlTextures, dwUseThisPainting))
			{
				((CP3DMesh*)pMeshOut)->m_bUniqueModel = true;
				((CP3DMesh*)pMeshOut)->m_dwMeshID = UNIQUE_MODEL_IDENT;
			}
		}
	}
	// vyhladanie v tabulke uz existujucich modelov
	else if (m_ModelCache)
		for (DWORD i=0; i<m_dwCacheSize; i++)
			if ((m_ModelCache[i].pMeshObject != NULL) && (m_ModelCache[i].filename != NULL))
				if (stricmp (filename, m_ModelCache[i].filename) == 0)
					if (MeshLoadFlags == m_ModelCache[i].LoadFlags)
					{
						if (MeshLoadFlags >= MESH_LOAD_HIERARCHY)
						{
							pMeshOut = DuplicateMeshHierarchy ((CP3DMeshHierarchy*)m_ModelCache[i].pMeshObject, dwUseThisPainting, xmlTextures);
							if (pMeshOut)
								((CP3DMeshHierarchy*)pMeshOut)->m_dwMeshID = i;
						}
						else
						{
							pMeshOut = DuplicateMesh ((CP3DMesh*)m_ModelCache[i].pMeshObject, dwUseThisPainting, xmlTextures);
							if (pMeshOut)
								((CP3DMesh*)pMeshOut)->m_dwMeshID = i;
						}
						break;
					}

	return pMeshOut;
}

//---------------------------------
void CP3DMeshLoader::StartCaching ()
{
	if (pCVr_use_loadcaching.GetBool ())
	{
		m_bInCachingState = true;
		if (m_ModelCache == NULL) return;
		for (DWORD i=0; i<m_dwCacheSize; i++)
			m_ModelCache[i].bInUse = false;
	}
}

//---------------------------------
void CP3DMeshLoader::StopCaching ()
{
	if (pCVr_use_loadcaching.GetBool () || m_bInCachingState)
	{
		m_bInCachingState = false;
		if (m_ModelCache == NULL) return;
		for (DWORD i=0; i<m_dwCacheSize; i++)
			if ((m_ModelCache[i].pMeshObject != NULL) && (m_ModelCache[i].bInUse == false))
				UnloadCacheItem (m_ModelCache[i].dwMeshID);		// uvolnit nepouzivany objekt
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//---------------------------------
bool CP3DMeshLoader::PreCache (const char *szFileName, MESH_LOAD_FLAGS MeshLoadFlags, ezxml_t xmlTextures, DWORD dwUseThisPainting)
{
	DWORD i;
	IP3DBaseInterface	*pMesh;

	if (szFileName == NULL) return false;
	if (m_ModelCache == NULL)
		if (!ReallocateCacheBuffer ())
		{
			CON(MSG_CON_ERR, "MeshLoader: Failed to reallocate cache buffer!");
			return false;
		}

	// kontrola ci sa model uz nenachadza v cache tabulke
	for (i=0; i<m_dwCacheSize; i++)
		if ((m_ModelCache[i].pMeshObject != NULL) && (m_ModelCache[i].filename != NULL))
			if (stricmp (szFileName, m_ModelCache[i].filename) == 0)
				if (MeshLoadFlags == m_ModelCache[i].LoadFlags)
				{
					D3DXFRAME_DERIVED *pTopFrame;
					if (MeshLoadFlags >= MESH_LOAD_HIERARCHY)
						pTopFrame = (D3DXFRAME_DERIVED*)((IP3DMeshHierarchy*)m_ModelCache[i].pMeshObject)->GetTopFrame ();
					else
						pTopFrame = (D3DXFRAME_DERIVED*)((IP3DMesh*)m_ModelCache[i].pMeshObject)->GetTopFrame ();
					CheckTextures (pTopFrame, dwUseThisPainting, xmlTextures);
					if (m_bInCachingState)
						m_ModelCache[i].bInUse = true;
					return true;	// model sa uz nachadza v cache tabulke
				}

	// ak sa model nenachadza v tabulke existujucich modelov tak nahrat novy
	if (MeshLoadFlags >= MESH_LOAD_HIERARCHY)
		pMesh = LoadMeshHierarchyFromFile (szFileName, MeshLoadFlags, xmlTextures, dwUseThisPainting);
	else
		pMesh = LoadMeshFromFile (szFileName, MeshLoadFlags, xmlTextures, dwUseThisPainting);

	if (pMesh == NULL) 
	{
		CON(MSG_CON_ERR, "MeshLoader: Failed to load/cache model %s!", szFileName);
		return false;
	}

	// zaradenie do cache tabulky
jump01:
	for (i=0; i<m_dwCacheSize; i++)
		if (m_ModelCache[i].pMeshObject == NULL)
		{
			m_ModelCache[i].filename = new char [strlen (szFileName) + 1];
			strcpy (m_ModelCache[i].filename, szFileName);
			m_ModelCache[i].dwMeshID = m_dwNextFreeID;
			m_ModelCache[i].LoadFlags = MeshLoadFlags;
			m_ModelCache[i].pMeshObject = pMesh;
			if (m_bInCachingState)
				m_ModelCache[i].bInUse = true;
			m_dwNextFreeID++;
			return true;
		}
		if (i == m_dwCacheSize)			// potreba zvacsenia cache tabulky
			if (ReallocateCacheBuffer ())
				goto jump01;

	CON(MSG_CON_ERR, "MeshLoader: Failed to assign model %s to cache table!", szFileName);
	return false;
}

// po volani funkcie budu neplatne vsetky meshe nahrane zo suboru szFileName
// (okrem tych co boli vytvorene s flagom bForceToCreateNew)
// parameter MeshLoadFlags musi byt rovnaky ako bol pri volani funkcie PreCache()
//---------------------------------
bool CP3DMeshLoader::UnCache (const char *szFileName, MESH_LOAD_FLAGS MeshLoadFlags)
{
	if ((szFileName == NULL) || (m_ModelCache == NULL)) return false;

	for (DWORD i=0; i<m_dwCacheSize; i++)
		if ((m_ModelCache[i].pMeshObject != NULL) && (m_ModelCache[i].LoadFlags == MeshLoadFlags))
			if ((m_ModelCache[i].filename != NULL) && (stricmp (m_ModelCache[i].filename, szFileName) == 0))
			{
				UnloadCacheItem (m_ModelCache[i].dwMeshID);
				return  true;
			}
	return false;
}

//---------------------------------
IP3DMesh* CP3DMeshLoader::LoadMesh (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, \
												ezxml_t xmlTextures, DWORD dwUseThisPainting, bool bForceToCreateNew)
{
	if ((filename == NULL) || (MeshLoadFlags != MESH_LOAD))
		return NULL;

	char szPath[MAX_PATH] = "models\\";
	strcat (szPath, filename);
	if (!bForceToCreateNew)
		if (!PreCache (szPath, MeshLoadFlags, xmlTextures, dwUseThisPainting))
			return NULL;
	return (IP3DMesh*)MeshLoading (szPath, MeshLoadFlags, xmlTextures, dwUseThisPainting, bForceToCreateNew);
}

//---------------------------------
IP3DMeshHierarchy* CP3DMeshLoader::LoadMeshHierarchy (const char *filename, MESH_LOAD_FLAGS MeshLoadFlags, \
							ezxml_t xmlTextures, DWORD dwUseThisPainting, bool bForceToCreateNew)
{
	if ((filename == NULL) || (MeshLoadFlags < MESH_LOAD_HIERARCHY))
		return NULL;

	char szPath[MAX_PATH] = "models\\";
	strcat (szPath, filename);
	if (!bForceToCreateNew)
		if (!PreCache (szPath, MeshLoadFlags, xmlTextures, dwUseThisPainting))
			return NULL;
	return (IP3DMeshHierarchy*)MeshLoading (szPath, MeshLoadFlags, xmlTextures, dwUseThisPainting, bForceToCreateNew);
}

// spravi konverziu beznych animacii na keyframed animacie
//---------------------------------
bool ConvertToKeyframedAnimations (D3DXFRAME_DERIVED *pFrameRoot, LPD3DXANIMATIONCONTROLLER pAnimController)
{
	UINT				dwAnimSetsNum, dwAnimationsNum;
	LPCSTR				szAnimSetName, szAnimName;
	LPD3DXANIMATIONSET	pAnimSet;
	HRESULT				hr;
	LPD3DXKEYFRAMEDANIMATIONSET	pKeyframedAnimSet;
	bool bRet = false;

	if (pAnimController == NULL) return false;

	dwAnimSetsNum = pAnimController->GetNumAnimationSets ();
	for (DWORD i=0; i<dwAnimSetsNum; i++)						// pre vsetky animation_set-y
	{
		if (SUCCEEDED(pAnimController->GetAnimationSet (i, &pAnimSet)))
		{
			if ((szAnimSetName = pAnimSet->GetName()) && (dwAnimationsNum = pAnimSet->GetNumAnimations()))
				if (SUCCEEDED (D3DXCreateKeyframedAnimationSet (szAnimSetName, ANIMATION_SAMPLING_RATE, \
					D3DXPLAY_LOOP, dwAnimationsNum, 0, NULL, &pKeyframedAnimSet)))
				{
					for (DWORD j=0; j<dwAnimationsNum; j++)	// pre vsetky animacie
					{
						double dPeriod = pAnimSet->GetPeriod ();
						UINT dwSamplesNum = UINT(dPeriod * ANIMATION_SAMPLING_RATE);
						if ((dwSamplesNum == 0) && (dPeriod > 0.0)) dwSamplesNum = 1;
						if (dwSamplesNum)
						{
							D3DXKEY_VECTOR3 *vecScale = new D3DXKEY_VECTOR3[dwSamplesNum];
							D3DXKEY_VECTOR3 *vecTranslation = new D3DXKEY_VECTOR3[dwSamplesNum];
							D3DXKEY_QUATERNION *quatRotation = new D3DXKEY_QUATERNION[dwSamplesNum];

							for (DWORD k=0; k<dwSamplesNum; k++)	// 'kopirovat' jednotlive sample v animacii
							{
								double dTimePos = double(k) / double(ANIMATION_SAMPLING_RATE);
								hr = pAnimSet->GetSRT (dTimePos, j, &vecScale[k].Value, &quatRotation[k].Value, &vecTranslation[k].Value);
								vecScale[k].Time = float(dTimePos);
								quatRotation[k].Time = float(dTimePos);
								vecTranslation[k].Time = float(dTimePos);
							}
							pAnimSet->GetAnimationNameByIndex (j, &szAnimName);
							DWORD dwAnimIndex;
							hr = pKeyframedAnimSet->RegisterAnimationSRTKeys (szAnimName, dwSamplesNum, dwSamplesNum, dwSamplesNum, \
											vecScale, quatRotation, vecTranslation, &dwAnimIndex);

							SAFE_DELETE_ARRAY (vecScale)
							SAFE_DELETE_ARRAY (vecTranslation)
							SAFE_DELETE_ARRAY (quatRotation)
							// TODO: pridat RegisterAnimationOutput() ???
							D3DXFRAME_DERIVED *pFrame = pFrameRoot->Find (szAnimName);
							if (pFrame)
								hr = pAnimController->RegisterAnimationOutput (szAnimName, &pFrame->TransformationMatrix, NULL, NULL, NULL);
						}
					}
					// 'vymena' animation setu
					hr = pAnimController->UnregisterAnimationSet (pAnimSet);
					hr = pAnimController->RegisterAnimationSet (pKeyframedAnimSet);
				}
			pAnimSet->Release();
		}
	}

	return bRet;
}


bool TestForIsolatedAnimation (LPD3DXFILEDATA pAnimationKey);

//---------------------------------
bool CP3DMeshLoader::IsolateAnimations (const char *szFile, const LPD3DXANIMATIONCONTROLLER pAnimController)
{
	LPD3DXFILE				pFile = NULL;
	LPD3DXFILEENUMOBJECT	pEnum = NULL;
	SIZE_T					numChilds, numChildsAnimSet, numChildsAnim;
	SIZE_T					childIdxAnimSet, childIdxAnim;
	GUID					TemplateGUID;
	LPD3DXFILEDATA			pAnimationSet, pAnimation, pAnimationKey;

	//////////////////////////////////////////////////////////////////////////
	if (pAnimController == NULL) return false;

	if (FAILED (D3DXFileCreate (&pFile)))
		return false;

	if (FAILED (pFile->RegisterTemplates ((LPVOID)D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES)))
		{SAFE_RELEASE (pFile); return false;}

	//////////////////////////////////////////////////////////////////////////
	FSFILE	*pDiskFile=NULL;
	BYTE	*pData = NULL;
	DWORD	dwSize = 0;

	pDiskFile = g_pFS->Load (szFile, pData, dwSize);
	if (pDiskFile == NULL)
	{
		SAFE_RELEASE (pFile)
		return false;
	}
	D3DXF_FILELOADMEMORY memLoad;
	memLoad.dSize = dwSize;
	memLoad.lpMemory = pData;
	if (FAILED (pFile->CreateEnumObject (&memLoad, D3DXF_FILELOAD_FROMMEMORY, &pEnum)))
		goto quit_hand1;

	//////////////////////////////////////////////////////////////////////////
	pEnum->GetChildren (&numChilds);

	// prechadzaju sa vsetky top level objekty
	for (SIZE_T childIdx = 0; childIdx < numChilds; childIdx++)
		if (SUCCEEDED (pEnum->GetChild (childIdx, &pAnimationSet)))
		{
			if (SUCCEEDED (pAnimationSet->GetType (&TemplateGUID)))
				// vyhladavaju sa vsetky AnimationSet-y
				if (TemplateGUID == TID_D3DRMAnimationSet)
				{
					pAnimationSet->GetChildren (&numChildsAnimSet);

					for (childIdxAnimSet = 0; childIdxAnimSet < numChildsAnimSet; childIdxAnimSet++)
						if (SUCCEEDED (pAnimationSet->GetChild (childIdxAnimSet, &pAnimation)))
						{
							if (SUCCEEDED (pAnimation->GetType (&TemplateGUID)))
								// vyhladavaju sa vsetky objekty Animation
								if (TemplateGUID == TID_D3DRMAnimation)
								{
									pAnimation->GetChildren (&numChildsAnim);

									for (childIdxAnim = 0; childIdxAnim < numChildsAnim; childIdxAnim++)
										if (SUCCEEDED (pAnimation->GetChild (childIdxAnim, &pAnimationKey)))
										{
											if (SUCCEEDED (pAnimationKey->GetType (&TemplateGUID)))
												// vyhladavaju sa vsetky objekty AnimationKey
												if (TemplateGUID == TID_D3DRMAnimationKey)
													if (TestForIsolatedAnimation (pAnimationKey))
													{
														// je treba odstranit dany objekt Animation
														char szAnimationSetName[256] = {0};
														char szAnimationName[256] = {0};
														SIZE_T name_size = 256;
														pAnimationSet->GetName(szAnimationSetName, &name_size);
														name_size = 256;
														pAnimation->GetName (szAnimationName, &name_size);

														if (RemoveIsolatedAnimation (szAnimationSetName, szAnimationName, pAnimController))
 															pAnimation->Release();		// FIXME: ???
													}
											pAnimationKey->Release ();
										}
								}
								pAnimation->Release ();
						}
				}
			pAnimationSet->Release();
		}

	SAFE_RELEASE (pEnum)
	SAFE_RELEASE (pFile)
	g_pFS->UnLoad (pDiskFile, pData);
	return true;

quit_hand1:
	SAFE_RELEASE (pEnum)
	SAFE_RELEASE (pFile)
	g_pFS->UnLoad (pDiskFile, pData);
	return false;
}

struct TIMED_FLOAT_KEY {
	DWORD	dwTime;
	DWORD	dwNumValues;
	float	fValues[16];
};
struct ANIMATION_KEY {
	DWORD			dwKeyType;
	DWORD			dwNumKeys;
	TIMED_FLOAT_KEY	pAnimKeys[];
};

//---------------------------------
bool TestForIsolatedAnimation (LPD3DXFILEDATA pAnimationKey)
{
	ANIMATION_KEY	*pAnimKeyData;
	SIZE_T			dataSize;
	P3DXMatrix		matLast;
	bool			bRetValue = true;

	if (pAnimationKey == NULL) return false;

	if (SUCCEEDED (pAnimationKey->Lock (&dataSize, (LPCVOID*)&pAnimKeyData)))
	{
		if (pAnimKeyData->dwKeyType == 4)		// only type matrix
			for (DWORD i=0; i<pAnimKeyData->dwNumKeys; i++)
			{
				if (pAnimKeyData->pAnimKeys[i].dwNumValues != 16)	// 16 hodnot pre matice
					continue;
				P3DXMatrix matTemp(pAnimKeyData->pAnimKeys[i].fValues);
				if (i > 0)
					if (!matTemp.Compare (matLast, 0.0001f))
						{bRetValue = false; break;}
				matLast = matTemp;
			}
		pAnimationKey->Unlock ();
	}
	return bRetValue;
}

//---------------------------------
bool CP3DMeshLoader::RemoveIsolatedAnimation (const char* szAnimSetName, const char* szAnimName, const LPD3DXANIMATIONCONTROLLER pAnimController)
{
	if (pAnimController == NULL) return false;

	LPD3DXANIMATIONSET pAnimSet;
	LPD3DXKEYFRAMEDANIMATIONSET pKeyFramedAnimSet;
	UINT dwAnimIdx;
	bool bRet = false;	

	if (strnicmp (szAnimName, "Anim-", 5) == 0)
		szAnimName += 5;

	if (SUCCEEDED (pAnimController->GetAnimationSetByName (szAnimSetName, &pAnimSet)))
	{
		if (SUCCEEDED(pAnimSet->QueryInterface (IID_ID3DXKeyframedAnimationSet, (LPVOID*)&pKeyFramedAnimSet)))
		{
/*
			// debug !!!
			UINT dwNumAnimations = pKeyFramedAnimSet->GetNumAnimations ();
			for (UINT i=0; i<dwNumAnimations; i++)
			{
				HRESULT hr = pKeyFramedAnimSet->UnregisterAnimation (i);
			}
			// debug !!!
*/
			if (SUCCEEDED(pKeyFramedAnimSet->GetAnimationIndexByName (szAnimName, &dwAnimIdx)))
				// FIXME: TODO: uvolni iba index 0, inak hadze chybu
				bRet = SUCCEEDED(pKeyFramedAnimSet->UnregisterAnimation (dwAnimIdx));
			pKeyFramedAnimSet->Release();
		}
		pAnimSet->Release ();
	}
	return bRet;
}

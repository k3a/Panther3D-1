//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK, MESS
// Purpose:	Bsp map (modified Quake III *.BSP) class definition
//-----------------------------------------------------------------------------

#include "bspmap.h"
#include "common.h"
#include "HP3DMatrix.h"
#include "si_flags.h"

#include "IP3DPhysEngine.h" // PHYS DEBUG:

// makra pre debug ucely
//#define HACK_VERTEXCOLOR		// nastavi farbu vertexov na RGB(1,1,1)
//#define HACK_LIGHTGRID_COLOR	// GetLightFromPoint() vrati vzdy RGB(1,1,1)
//#define HACK_VISIBILITY		// ClusterVisible() vrati vzdy true

REGISTER_SINGLE_CLASS(CP3DBSPMap, IP3DRENDERER_BSPMAP); // zaregistruj tuto tøídu

// singletons
static IP3DPhysEngine* g_pPhysEngine=NULL;
CP3DBSPMap *g_pBSPMap=NULL;

LPDIRECT3DTEXTURE9  *g_pLastTextureMaps;
int					g_intLastNumTextures;

//cvars
static ConVar *pCVr_dyn_lighting=NULL;
extern ConVar pCVr_use_loadcaching;

// static variables
float sfAmbient=0.0f; // ambient value

void cvLoad(const char* fn)
{
	g_pBSPMap->Load(fn);
}

inline int FindIndex(const epair_s *sEnt, const int numKeyP, const char *key)
{
	for (int i=0; i < numKeyP; i++)
		if (strcmpi(sEnt[i].key, key)==0) return i;
	return -1;
}
char *ValueForKey(const epair_s *sEnt, const int numKeyP, const char *key, char *defaultKey)
{
	int keyI = FindIndex(sEnt, numKeyP, key);
	if (keyI>-1) return sEnt[keyI].value;
	return defaultKey;
}

//////////////////////////////////////////////////////////////////////////
CP3DBSPMap::CP3DBSPMap()
{
	g_pBSPMap = this; // pøekopíruj adresu p3dbspmap do globální promìnné

	g_pConsole->RegisterCVar("map", cvLoad); // register map command
	m_bLoaded = false;
	m_bDeluxelMaps = false;
	numPlanes = 0; numNodes = 0; numLeaves = 0; numLeafFaces = 0; numVerts = 0;
	numFaces = 0; numIndex = 0; numTextures = 0; numLights = 0; numLightGrid = 0; numModels = 0;
	m_fWorldMin[0] = m_fWorldMin[1] = m_fWorldMin[2] = 0;
	m_fWorldMax[0] = m_fWorldMax[1] = m_fWorldMax[2] = 0;
	// spravne ???
	m_lightGridInverseSize[0] = m_lightGridInverseSize[1] = 1 / 64.0f; m_lightGridInverseSize[2] = 1 / 128.0f;
	m_lightGridOrigin[0] = m_lightGridOrigin[1] = m_lightGridOrigin[2] = 0;
	m_lightGridBounds[0] = m_lightGridBounds[1] = m_lightGridBounds[2] = 0;
	m_lightGridCellSize[0] = 64.0f; m_lightGridCellSize[1] = 64.0f; m_lightGridCellSize[2] = 128.0f;
	m_dwDefLMmatID = 0xFFFFFFFF; m_dwDefVertexmatID = 0xFFFFFFFF;
	m_intCurCamCluster = -1;
	memset (&m_sVisData, 0, sizeof(BSP_VISDATA));
	m_pVB = NULL;
	m_pIB = NULL;
	m_pVertexDeclaration = NULL;
	m_pEnts = NULL;
	m_pTextureMaps = NULL;
	m_pLightMaps = NULL;
	m_pPlanes = NULL;
	m_pNodes = NULL;
	m_pLeaves = NULL;
	m_pLeafFaces = NULL;
	m_pLightmap = NULL;
	m_pTextures = NULL;
	m_pMaterialID = NULL;
	m_pMaterials = NULL;
	m_pVerts = NULL;
	m_pFaces = NULL;
	m_pIndices = NULL;
	m_pLightGrid = NULL;
	m_pModels = NULL;
	m_pBody = NULL;
	m_pLoadTexture=NULL;
	m_pLoadBar=NULL;
	m_pLoadBarBg=NULL;
	m_pEntMgr=NULL;
	m_pLoadFnt=NULL;
}

CP3DBSPMap::~CP3DBSPMap()
{
	m_pEntMgr->KillAll();
	SAFE_DELETE(m_pLoadFnt);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);
	SAFE_RELEASE (m_pVertexDeclaration);
	SAFE_DELETE(m_pLoadTexture);
	SAFE_DELETE(m_pLoadBar);
	SAFE_DELETE(m_pLoadBarBg);
	SAFE_DELETE_ARRAY(m_pTextureMaps);
	if (m_pLightMaps)
		for (int i=0; i < numLights; i++) // clear textures
			SAFE_RELEASE (m_pLightMaps[i])
	SAFE_DELETE_ARRAY(m_pLightMaps);
	SAFE_DELETE_ARRAY(m_pPlanes);
	SAFE_DELETE(m_pEnts);
	SAFE_DELETE_ARRAY(m_pLightGrid);
	SAFE_DELETE_ARRAY(m_pModels);
	SAFE_DELETE_ARRAY(m_pNodes);
	SAFE_DELETE_ARRAY(m_pLeaves);
	SAFE_DELETE_ARRAY(m_pLeafFaces);
	SAFE_DELETE_ARRAY(m_sVisData.vecs);
	SAFE_DELETE_ARRAY(m_pLightmap);
	SAFE_DELETE_ARRAY(m_pTextures);
	SAFE_DELETE_ARRAY(m_pMaterials);
	SAFE_DELETE_ARRAY(m_pMaterialID);
	SAFE_DELETE_ARRAY(m_pVerts);
	SAFE_DELETE_ARRAY(m_pFaces);
	SAFE_DELETE_ARRAY(m_pIndices);
	SAFE_DELETE (m_pBody);
	g_pBSPMap = NULL; // vynuluj globální promìnnou
}

void CP3DBSPMap::ParseEntities()
{
	char *entS; // jen pro prehlednost
	entS = m_pEnts->ents;

	char line[MAX_LINE_LNG], *curr; int c;
	int currChr=0; // aktuální znak
	epair_s *ePairs=NULL; // pár
	int currEnt=0; // aktuální intex z ePairs
	DWORD lineNum=0;

	do
    {
		// KONTROLA NA KONEC STRINGU
		if (currChr==strlen(entS)) 
		{
			SAFE_DELETE_ARRAY(ePairs);
			CON(MSG_CON_INFO, "Entities loaded (%u entity lines parsed)", lineNum);
			return; // hotovo, = konec stringu, již by chtìl èíst znak navíc = znak 0
		}

		curr = line;            // 'curr' points to the start of 'line'

		c = entS[currChr];
		currChr++;

		lineNum++;

        // read a line ('c' contains the first character)
        do
        {
			*curr++ = c;
            c = entS[currChr];
			currChr++;
        }
        while (c != '\n' && c != '\r');

        *curr = '\0';

		char tmpName[MAX_LINE_LNG];
		for (unsigned int i=0; i < strlen(line); i++)
		{
			if (line[i]=='{')
			{
				// zaèátek tìla entity
				SAFE_DELETE_ARRAY(ePairs);
				ePairs = new epair_s[MAX_ENTS_PARAMS];
				currEnt=0;
				continue;
			}
			if (line[i]=='}')
			{
				// konec tìla entity = spawn entity
				
				// vytvor data o geometrii pokud jsou
				geometryData_s gdata;
				gdata.numFaces = 0;
				char* model = ValueForKey(ePairs, currEnt, "model", NULL);
				if (model) // je modelem
				{
					if (*model == '*') // je staticky model!!
					{
						int modNum = (int)atof(model+1); // cislo modelu
						
						gdata.numFaces = m_pModels[modNum].n_faces;
						gdata.faces = new geometryDataFace_s[gdata.numFaces];
						// projdi a nastav facy
						for (int f = m_pModels[modNum].face; f<m_pModels[modNum].face+m_pModels[modNum].n_faces;f++)
						{
							gdata.faces[f-m_pModels[modNum].face].numVerts = m_pFaces[f].n_verts;
							gdata.faces[f-m_pModels[modNum].face].verts = new P3DXVector3D[gdata.faces[f-m_pModels[modNum].face].numVerts];
							strcpy(gdata.faces[f-m_pModels[modNum].face].texture, m_pTextures[m_pFaces[f].texture].name);
							gdata.faces[f-m_pModels[modNum].face].surfFlags = m_pTextures[m_pFaces[f].texture].flags;
							gdata.faces[f-m_pModels[modNum].face].contFlags = m_pTextures[m_pFaces[f].texture].contents;
							// projdi a nastav vrcholy
							for (int v=m_pFaces[f].vertex;v<m_pFaces[f].vertex+m_pFaces[f].n_verts;v++)
							{
								gdata.faces[f-m_pModels[modNum].face].verts[v-m_pFaces[f].vertex].x = m_pVerts[v].position[0];
								gdata.faces[f-m_pModels[modNum].face].verts[v-m_pFaces[f].vertex].y = m_pVerts[v].position[1];
								gdata.faces[f-m_pModels[modNum].face].verts[v-m_pFaces[f].vertex].z = m_pVerts[v].position[2];
							}
						}
					}
				}

				m_pEntMgr->Spawn(ePairs, currEnt, gdata);

				// clear memory
				for (int ie=0; ie<currEnt; ie++)
				{
					delete[] ePairs[ie].key;
					delete[] ePairs[ie].value;
				}
				continue;
			}

			if (line[i]==' ' && line[i-1] == '"' && line[i+1]=='"') // mezera mezi názvem a hodnotou " "
			{
				strcpy(tmpName, line);
				tmpName[i-1]=0; // ukonèi øetìzec na místì znaku "
				// &tmpName[1] obsahuje NAZEV KLICE
				ePairs[currEnt].key = new char[strlen(&tmpName[1])+2]; // initializuj promìnnou
				strcpy(ePairs[currEnt].key, &tmpName[1]);

				strcpy(tmpName, line);
				tmpName[strlen(tmpName)-1]=0; // ukonèi øetìzec na místì znaku "
				// &tmpName[i+2] obsahuje HODNOTU KLICE
				ePairs[currEnt].value = new char[strlen(&tmpName[i+2])+2]; // initializuj promìnnou
				strcpy(ePairs[currEnt].value, &tmpName[i+2]);

			/*	if (!strcmpi(ePairs[currEnt].key, "model")) // obsahuje model, mozna staticky s *cislo
				{
					if (*ePairs[currEnt].value == '*') // je model *cislo
					{
						// získej index modelu
						int modNum = (int)atof(ePairs[currEnt].value+1);

						// pøidej min BBox
						currEnt++;
						ePairs[currEnt].key = new char[6]; // initializuj promìnnou
						strcpy(ePairs[currEnt].key, "minBB");
						ePairs[currEnt].value = new char[40]; // initializuj promìnnou, WARN: max. poèet znakù u vektoru
						sprintf(ePairs[currEnt].value, "%f %f %f", m_pModels[modNum].mins[0], 
								m_pModels[modNum].mins[1], m_pModels[modNum].mins[2]);

						// pøidej max BBox
						currEnt++;
						ePairs[currEnt].key = new char[6]; // initializuj promìnnou, WARN: max. poèet znakù u vektoru
						strcpy(ePairs[currEnt].key, "maxBB");
						ePairs[currEnt].value = new char[40]; // initializuj promìnnou, WARN: max. poèet znakù u vektoru
						sprintf(ePairs[currEnt].value, "%f %f %f", m_pModels[modNum].maxs[0], 
							m_pModels[modNum].maxs[1], m_pModels[modNum].maxs[2]);
					}
				}*/

				currEnt++;
				continue;
			}
		}
    } while (true);
	SAFE_DELETE_ARRAY(ePairs);
	CON(MSG_CON_ERR, "BSPMAP: ParseEntities(): Unexpected break while parsing entities! Please inform developers!");
}


bool CP3DBSPMap::Init ()
{
	m_pEntMgr = (IP3DEntityMgr*)I_GetClass(IP3DGAME_ENTITYMGR); // ziskej ent. manager
    pCVr_dyn_lighting = g_pConsole->FindConVar ("r_dyn_lighting");

	// vertex declaration pre bsp vertexy
	D3DVERTEXELEMENT9 bsp_declaration[] = 
	{
		{0, 0,  D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_POSITION,	0},
		{0, 12, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_NORMAL,	0},
		{0, 24, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_NORMAL,	1},
		{0, 36, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	0},
		{0, 44, D3DDECLTYPE_FLOAT2,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TEXCOORD,	1},
		{0, 52, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_TANGENT,	0},
		{0, 64, D3DDECLTYPE_FLOAT3,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_BINORMAL,	0},
		{0, 76, D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT,	D3DDECLUSAGE_COLOR,	0},
		D3DDECL_END()
	};
	if (FAILED (g_pD3DDevice->CreateVertexDeclaration (bsp_declaration, &m_pVertexDeclaration)))
	{
		CON(MSG_ERR_FATAL, "BSPMap: Can't create vertex declaration!");
		return false;
	}

	g_pRenderer = (CP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);
	g_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM); // create frustum class
	g_pPhysEngine = (IP3DPhysEngine*)I_GetClass(IP3DPHYS_PHYSENGINE); // create newton class [single]
	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	m_pLoadFnt = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	m_pLoadTexture = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE); // create tex class
	m_pLoadBar = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE); // create tex class
	m_pLoadBarBg = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE); // create tex class

	if ((g_pRenderer == NULL) || (g_pFrustum == NULL) || (g_pPhysEngine == NULL) || \
		(m_pLoadFnt == NULL) || (g_pMaterialManager == NULL))
	{
		CON(MSG_ERR_FATAL, "BSPMap: Can't obtain common singletons!");
		return false;
	}

	m_pLoadFnt->CreateFont("Arial", 15, false, false);
	m_dwDefLMmatID = g_pMaterialManager->GetLMmaterialID ();
	m_dwDefVertexmatID = g_pMaterialManager->GetVertexLightMaterialID ();
	return true;
}

void CP3DBSPMap::ChangeGamma(byte *pImage, int size, float factor)
{
	// Go through every pixel in the lightmap
	for(int i = 0; i < size / 3; i++, pImage += 3) 
	{
		float scale = 1.0f, temp = 0.0f;
		float r = 0, g = 0, b = 0;

		// extract the current RGB values
		r = (float)pImage[0];
		g = (float)pImage[1];
		b = (float)pImage[2];

		// Multiply the factor by the RGB values, while keeping it to a 255 ratio
		r = r * factor / 255.0f;
		g = g * factor / 255.0f;
		b = b * factor / 255.0f;
		
		// Check if the the values went past the highest value
		if(r > 1.0f && (temp = (1.0f/r)) < scale) scale=temp;
		if(g > 1.0f && (temp = (1.0f/g)) < scale) scale=temp;
		if(b > 1.0f && (temp = (1.0f/b)) < scale) scale=temp;

		// Get the scale for this pixel and multiply it by our pixel values
		scale*=255.0f;		
		r*=scale;	g*=scale;	b*=scale;

		// Assign the new gamma'nized RGB values to our image
		pImage[0] = (byte)r;
		pImage[1] = (byte)g;
		pImage[2] = (byte)b;
	}
}

void CP3DBSPMap::DrawLoadScr(int per, const char* desc=NULL)
{
	// KEX: loading screen by se nemel kreslit sam, ale na pozadani vykresleni stavu enginem
/*	g_pRenderer->BeginScene();
	//fntA15->DrawText(FA_CENTER, txt, 0, g_pEngSet.Height/2 - fntA15->GetLineH()/2, g_pEngSet.Width, g_pEngSet.Height, P3DCOLOR_ARGB(255, 255, 0, 0));
	g_p2D->DrawTexture(m_pLoadTexture, 0, 0, g_pEngSet.Width, g_pEngSet.Height);
	g_p2D->DrawTexture(m_pLoadBarBg, g_pEngSet.Width/5, g_pEngSet.Height - 30, g_pEngSet.Width - g_pEngSet.Width/5, g_pEngSet.Height - 20);
	g_p2D->DrawTexture(m_pLoadBar, g_pEngSet.Width/5, g_pEngSet.Height - 30, g_pEngSet.Width/5 + (int)(g_pEngSet.Width - g_pEngSet.Width/2.5f)/100*per, g_pEngSet.Height - 20);
		if (desc)
		{
			m_pLoadFnt->DrawText(FA_CENTER, desc, 0, g_pEngSet.Height - 50, g_pEngSet.Width, g_pEngSet.Height - 35, P3DCOLOR_XRGB(255, 255, 200));
		}
	g_pRenderer->EndScene();
	g_pRenderer->Present(NULL);*/
}

void CP3DBSPMap::ComputeTangents(BSP_VERTEX_USED *pVertices)
{
	P3DXVector3D *tan1 = new P3DXVector3D[numVerts * 2];
	P3DXVector3D *tan2 = tan1 + numVerts;
	ZeroMemory(tan1, numVerts * sizeof(P3DXVector3D) * 2); // ClearMemory

	for (int i = 0; i < numFaces; i++) // projdi všechny pošky (subsety) scény
	{
		P3DXVector3D v1, v2, v3; // verts
		P3DPoint2D w1, w2, w3; // textcoords
		for (int j=0; j<m_pFaces[i].n_indexes; j++) {	// projdi vsechny trojùhelníhy aktuální plošky
			v1.x = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]].position[0];
			v1.y = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]].position[1];
			v1.z = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]].position[2];
			w1.x = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]].texcoord[0];
			w1.y = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]].texcoord[1];

			v2.x = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]].position[0];
			v2.y = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]].position[1];
			v2.z = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]].position[2];
			w2.x = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]].texcoord[0];
			w2.y = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]].texcoord[1];

			v3.x = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]].position[0];
			v3.y = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]].position[1];
			v3.z = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]].position[2];
			w3.x = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]].texcoord[0];
			w3.y = m_pVerts[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]].texcoord[1];

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

			tan1[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]] += sdir;
			tan1[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]] += sdir;
			tan1[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]] += sdir;

			tan2[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3]] += tdir;
			tan2[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1]] += tdir;
			tan2[m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2]] += tdir;
		}
	}

	for (long a = 0; a < numVerts; a++)
	{
		const P3DXVector3D& n(m_pVerts[a].normal);
		const P3DXVector3D& t(tan1[a]);

		// Gram-Schmidt orthogonalize
		P3DXVector3D tan;
		tan = (t - n * (n*t/*dot*/));
		tan.Normalize();
		pVertices[a].tan[0] = tan[0];
		pVertices[a].tan[1] = tan[1];
		pVertices[a].tan[2] = tan[2];

		// Calculate bitangent (binormal)
		float tw = (    ((n%t/*cross*/) * tan2[a]/*dot*/)      < 0.0F) ? -1.0F : 1.0F;
		//P3DXVector3D bitan(n.x*t.x, n.y*t.y, n.y*t.y);
		P3DXVector3D bitan;
		bitan = (n % tan);

		pVertices[a].bitan[0] = bitan[0]*tw;
		pVertices[a].bitan[1] = bitan[1]*tw;
		pVertices[a].bitan[2] = bitan[2]*tw;
	}

	delete[] tan1;
}

void CP3DBSPMap::Load7mt(char* sz7mtPath, int MaterialIndex)
{
	bool diffuseFound=false, surfaceFound=false;
	strcpy(m_pMaterials[MaterialIndex].diffuse, "textures/NULL");
	strcpy(m_pMaterials[MaterialIndex].surface, "common");

	if (sz7mtPath==NULL || sz7mtPath[0]==0)
	{
		CON(MSG_CON_ERR, "BSPMAP: Undefined material path!");
		return;
	}

	// uprav cestu
	char szPath[MAX_PATH];
	strcpy(szPath, sz7mtPath);
	if (szPath[strlen(szPath)-4]!='.') strcat(szPath, ".7mt"); // pripona

	// otevri 7mt
	BYTE *pData; ULONG uSize;
	FSFILE* fp = g_pFS->Load(szPath, pData, uSize);

	if(!fp)
	{
		//CON(MSG_CON_ERR, "Can't open material %s!", sz7mtPathIn_texPathOut);
		return;
	}

	//parsuj string
	// POZOR - pocita s \r\n - jen pro Windows
	bool inBrackets=false; bool BSPBracketsFound=false;
	char szBuffer[256]; int nBuffer=0;
	char* pszLeft=NULL; int numLeft=0;
	for(ULONG i=0;i<uSize;i++)
	{
		if (pData[i]=='{')
		{
			inBrackets=true;
			BSPBracketsFound=true;
			continue;
		}
		if (pData[i]=='}')
		{
			inBrackets=false;
			break;
		}
		if (inBrackets==false) continue; // nejsme mezi { a }
		// ziskej data do bufferu
		if (pData[i]=='\n') // konec radku - vola se az po \r
		{
			nBuffer=0;
		}
		else if (pData[i]=='\r') // konec radku - prvni ukoncovaci znak \r
		{
			if (!pszLeft) continue; // nemame levou stranu!
			if (!nBuffer) continue; // nemame pravou stranu!
			szBuffer[nBuffer]=0; // ukonci string
			// mame i pravou stranu => zpracuj
			if (!stricmp(pszLeft, "diffuse"))
			{
				if (strlen(szBuffer)>(MAX_BSPPATH-1))
					CON(MSG_CON_ERR, "BSPMAP: Diffuse path too long (>%d)! Material %s.", MAX_BSPPATH-1, sz7mtPath);
				strcpy(m_pMaterials[MaterialIndex].diffuse, szBuffer);
				diffuseFound = true;
				continue; // ok, udelano
			}
			// mame i pravou stranu => zpracuj
			if (!stricmp(pszLeft, "surface"))
			{
				if (strlen(szBuffer)>15)
					CON(MSG_CON_ERR, "BSPMAP: Surface name too long (>15)! Material %s.", sz7mtPath);
				strcpy(m_pMaterials[MaterialIndex].surface, szBuffer);
				surfaceFound = true;
				continue; // ok, udelano
			}
		}
		else if (pData[i]==' ') // mezera = oddelovac
		{
			if (pszLeft) {free(pszLeft); pszLeft=NULL;}
			pszLeft=new char[nBuffer+1];
			strncpy(pszLeft, szBuffer, nBuffer);
			pszLeft[nBuffer]=0;
			numLeft=nBuffer;
			nBuffer=0; // ted chceme ziskat pravou stranu
		}
		else // znak
		{
			szBuffer[nBuffer]=pData[i];
			nBuffer++;
		}
	}

	if (BSPBracketsFound)
	{
		if (!diffuseFound)
			CON(MSG_CON_ERR, "BSPMAP: Diffuse texture undefined! Material %s.", sz7mtPath);
			
		if (!surfaceFound)
		{
			CON(MSG_CON_DEBUG, "BSPMAP: Surface name undefined! Material %s.", sz7mtPath);
			strcpy(m_pMaterials[MaterialIndex].surface, "common");
		}
	}
	else
	{
		CON(MSG_CON_ERR, "BSPMAP: Material %s without {} area!", sz7mtPath);
	}

	// uklidit pamet
	if (pszLeft) {delete[](pszLeft); pszLeft=NULL;}

	// uzavri 7mt
	g_pFS->UnLoad(fp, pData);
}

void CP3DBSPMap::ParseWorldspawnEntity()
{
	char *entS; // jen pro prehlednost
	entS = m_pEnts->ents;

	char line[MAX_LINE_LNG], *curr; int c;
	int currChr=0; // aktuální znak
	epair_s *ePairs=NULL; // pár
	int currEnt=0; // aktuální intex z ePairs

	do
	{
		// KONTROLA NA KONEC STRINGU
		if (currChr==strlen(entS)) 
		{
			SAFE_DELETE_ARRAY(ePairs);
			return; // hotovo, = konec stringu, již by chtìl èíst znak navíc = znak 0
		}

		curr = line;            // 'curr' points to the start of 'line'

		c = entS[currChr];
		currChr++;

		// read a line ('c' contains the first character)
		do
		{
			*curr++ = c;
			c = entS[currChr];
			currChr++;
		}
		while (c != '\n' && c != '\r');

		*curr = '\0';

		char tmpName[MAX_LINE_LNG];
		for (unsigned int i=0; i < strlen(line); i++)
		{
			if (line[i]=='{')
			{
				// zaèátek tìla entity
				SAFE_DELETE_ARRAY(ePairs);
				ePairs = new epair_s[MAX_ENTS_PARAMS];
				currEnt=0;
				continue;
			}
			if (line[i]=='}')
			{
				// konec tìla entity = spawn entity

				bool isWorldspawn=false;

				//ziskej _ambient a lightgrid hodnotu
				for (int i=0; i < currEnt; i++)
				{
					if (strcmpi(ePairs[i].value, "worldspawn")==0)
						isWorldspawn = true; // toto je worldspawn entita, dale neni nutne prochazet entity
						
					if (strcmpi(ePairs[i].key, "_ambient")==0)
						sfAmbient = (float)atof(ePairs[i].value);
					if (strcmpi(ePairs[i].key, "gridsize")==0) 
						sscanf (ePairs[i].value, "%f %f %f", &m_lightGridCellSize[0], &m_lightGridCellSize[1], &m_lightGridCellSize[2]);
					
					// clear memory
					delete[] ePairs[i].key;
					delete[] ePairs[i].value;
				}
				if (isWorldspawn) // toto je worldspawn entita, dale neni nutne prochazet entity
					break;
				else
					continue; // neni worldspawn
			}

			if (line[i]==' ' && line[i-1] == '"' && line[i+1]=='"') // mezera mezi názvem a hodnotou " "
			{
				strcpy(tmpName, line);
				tmpName[i-1]=0; // ukonèi øetìzec na místì znaku "
				// &tmpName[1] obsahuje NAZEV KLICE
				ePairs[currEnt].key = new char[strlen(&tmpName[1])+2]; // initializuj promìnnou
				strcpy(ePairs[currEnt].key, &tmpName[1]);

				strcpy(tmpName, line);
				tmpName[strlen(tmpName)-1]=0; // ukonèi øetìzec na místì znaku "
				// &tmpName[i+2] obsahuje HODNOTU KLICE
				ePairs[currEnt].value = new char[strlen(&tmpName[i+2])+2]; // initializuj promìnnou
				strcpy(ePairs[currEnt].value, &tmpName[i+2]);

				currEnt++;
				continue;
			}
		}
	} while (true);
	SAFE_DELETE_ARRAY(ePairs);
}

void CP3DBSPMap::Build()
{
	int i;
	if (g_pD3DDevice == NULL) return;

	for(i = 0; i < numTextures; i++)
	{
		if ((stricmp(m_pTextures[i].name, "noshader") == 0) || \
			(stricmp(m_pTextures[i].name, "materials/NULL") == 0) || \
			(stricmp(m_pTextures[i].name, "materials/common/caulk") == 0) ||  // FIXME: GDS: Docasne reseni. Polygon ys nodraw flagem totiz uz asi vyradi z bsp kompilator
			(stricmp(m_pTextures[i].name, "textures/NULL") == 0))
		{ m_pTextures[i].flags |= SOD_SURF_NODRAW; m_pTextureMaps[i] = NULL; m_pMaterialID[i]= 0xFFFFFFFF; continue; }

		// Ziskej nazev difuzni textury
		char FName[MAX_PATH] = "";
			// pokud je to textura
			if(m_pTextures[i].name[0]=='t')
			{
				strcat(FName, m_pTextures[i].name);
				m_pMaterialID[i] = 0xFFFFFFFF; // nepouziva material
			}
			else // je to 7mt material
			{
				// nacti material
				strcat(FName, m_pTextures[i].name);
				if (FName[strlen(FName)-4]!='.') strcat(FName, ".7mt"); // dopln priponu
				m_pMaterialID[i] = g_pMaterialManager->LoadMaterial(FName);
				if (m_pMaterialID[i]==0xFFFFFFFF)
				{
					CON(MSG_CON_ERR, "BSPMAP: Can't load material %s!", FName);
				}
				// nastav difuzni texturu, pokud existuje
				Load7mt(FName, i);
				strcpy(FName, m_pMaterials[i].diffuse);
			}
		// Nacti difuzni texturu
		m_pTextureMaps[i] = g_TextureLoader.CreateTexture (FName);
		if (m_pTextureMaps[i] == NULL)
		{
			m_pTextures[i].flags |= SOD_SURF_NODRAW;
			CON(MSG_CON_ERR, "BSPMAP: Can't load texture %s!", FName);
		}

		/* LOADING SCREEN */
		DrawLoadScr(20+55/numTextures*i, "Loading materials and textures..."); // max. 75%, FIXME: Asi nemusi byt zobrazovano pri kazde texture
	}

	// ******************************** CREATE LIGHT MAPS ! ******************************************
	D3DLOCKED_RECT lockedRect;
	unsigned char *pLMSurfaceData;
	LPDIRECT3DSURFACE9	pLMSurface;
	unsigned long dwCharsPerRow;
	
	//Mame v mape deluxelmaps?
	m_bDeluxelMaps=false;
	if (numLights>1)
	{
		for(i=0;i<numFaces;i++)
		{
			if (m_pFaces[i].lm_index == 1)
			{
				CON(MSG_CON_ERR, "Warning: This map doesn't contain deluxelmaps!");
				break; // face ma index 1, mapa nema deluxel mapy
			}
		}
		m_bDeluxelMaps=true; // zadny face nema lm index 1, ale presto jsou dve lm 0 a 1 proto je 1 deluxel
	}
	
	//Loop through each lightmap array
	for (i = 0; i < numLights; i++)
	{
		//ChangeGamma((unsigned char *)m_pLightmap[i].imageBits, LIGHTMAP_SIZE*LIGHTMAP_SIZE*3, 7);

		// Create lightmap texture
		if (SUCCEEDED (D3DXCreateTexture (g_pD3DDevice, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_X8R8G8B8, \
													D3DPOOL_MANAGED, &m_pLightMaps[i])))
			// Grab the top level mipmap (original size) and lock the surface
			if (SUCCEEDED (m_pLightMaps[i]->GetSurfaceLevel (0, &pLMSurface)))
			{
				if (SUCCEEDED (pLMSurface->LockRect (&lockedRect, NULL, 0)))
				{
					pLMSurfaceData = static_cast<unsigned char*>(lockedRect.pBits);
					dwCharsPerRow = lockedRect.Pitch;

					// For each set of 3 values in the array write one RGB pixel on the texture.
					for (unsigned long r = 0; r < LIGHTMAP_SIZE; r++)
					{
						unsigned long dwIndex = r * dwCharsPerRow;
						for (unsigned long c = 0; c < LIGHTMAP_SIZE; c++)
						{
							// pokud mame deluxelmapy a pokud je to deluxelmapa (liche id), prehod osy y a z
							if (m_bDeluxelMaps && (i+1)%2==0)
							{
								pLMSurfaceData[dwIndex + c*4 + 1] = m_pLightmap[i].imageBits[r][c][2];
								pLMSurfaceData[dwIndex + c*4 + 0] = m_pLightmap[i].imageBits[r][c][1];
							}
							else // obycejna lm
							{
								pLMSurfaceData[dwIndex + c*4 + 0] = m_pLightmap[i].imageBits[r][c][2];
								pLMSurfaceData[dwIndex + c*4 + 1] = m_pLightmap[i].imageBits[r][c][1];
							}
							pLMSurfaceData[dwIndex + c*4 + 2] = m_pLightmap[i].imageBits[r][c][0];
							
							pLMSurfaceData[dwIndex + c*4 + 3] = 255;
						}
					}
					pLMSurface->UnlockRect ();
					// Now that we've created our texture, run it through a mipmap filter   
					D3DXFilterTexture (m_pLightMaps[i], 0, 0, P3D_TEXFILTER_LINEAR);
					// Now generate mipmaps
					m_pLightMaps[i]->GenerateMipSubLevels();
				}
				pLMSurface->Release ();
			}
	}

	// KEXIK 27.1.2007: pole lightmap je mozne v tomto case odstranit z pameti
	SAFE_DELETE_ARRAY(m_pLightmap);

	/* LOADING SCREEN */
	DrawLoadScr(78, "Preparing vertices...");

	// **************************** CREATE VERTEX BUFFER **********************
	
	for (i=0; i<numFaces; i++)
	{
		// KEXIK 31.1.2007: Facum bez lightmapy (lm_index < 0) je vyhodne nastavit 0. Jedna se totiz o materialy bez lightmapy
		//                  a to jsou z usporneho duvodu materialy jako skip, mozna nodraw a podobne
		if (m_pFaces[i].lm_index<0)
			m_pFaces[i].lm_index = 0;

		m_pFaces[i].n_indexes /= 3;
		// nekresli vodu
		if (m_pTextures[m_pFaces[i].texture].contents==SOD_CONT_WATER)
			m_pTextures[m_pFaces[i].texture].flags &= SOD_SURF_NODRAW;

		// kontrola na typ facu
		if (m_pFaces[i].type != FACE_PLANAR && m_pFaces[i].type != FACE_TRIANGLE_SOUP)
			CON(MSG_CON_ERR, "BSPMAP: Face %d has an unknown face type %d!", i, m_pFaces[i].type);
	}

	m_fWorldMin[0] = m_fWorldMin[1] = m_fWorldMin[2] = 10000000.0f;
	m_fWorldMax[0] = m_fWorldMax[1] = m_fWorldMax[2] = -10000000.0f;

	if (SUCCEEDED (g_pD3DDevice->CreateVertexBuffer (numVerts * sizeof(BSP_VERTEX_USED), 0, 0, D3DPOOL_MANAGED, &m_pVB, NULL)))
	{
		BSP_VERTEX_USED *pVertices = NULL;
		if (SUCCEEDED (m_pVB->Lock (0, 0, (void**)&pVertices, 0)))
		{
			for(i = 0; i < numVerts; i++)
			{
				// Swap the y and z values, and negate the new z so Y is up
				float f_temp = m_pVerts[i].position[1];
				m_pVerts[i].position[1] = m_pVerts[i].position[2];
				m_pVerts[i].position[2] = f_temp;

				pVertices[i].pos[0] = m_pVerts[i].position[0];
				pVertices[i].pos[1] = m_pVerts[i].position[1];
				pVertices[i].pos[2] = m_pVerts[i].position[2];
				
				// ziskani minimalnich a maximalnich pozic sveta
				for(int p=0;p<3;p++)
					if (m_pVerts[i].position[p]<m_fWorldMin[p]) m_fWorldMin[p] = m_pVerts[i].position[p];
				for(int p=0;p<3;p++)
					if (m_pVerts[i].position[p]>m_fWorldMax[p]) m_fWorldMax[p] = m_pVerts[i].position[p];

				// prohod take normaly vrcholu... wrrrr! To me pekne nastvalo :)
				f_temp = m_pVerts[i].normal[1];
				m_pVerts[i].normal[1] = m_pVerts[i].normal[2];
				m_pVerts[i].normal[2] = f_temp;

				pVertices[i].normal[0] = m_pVerts[i].normal[0];
				pVertices[i].normal[1] = m_pVerts[i].normal[1];
				pVertices[i].normal[2] = m_pVerts[i].normal[2];

				pVertices[i].tc1[0] = m_pVerts[i].texcoord[0];
				pVertices[i].tc1[1] = m_pVerts[i].texcoord[1];

				pVertices[i].tc2[0] = m_pVerts[i].lightmap[0];
				pVertices[i].tc2[1] = m_pVerts[i].lightmap[1];

				//deluxely
				pVertices[i].deluxel[0] = m_pVerts[i].deluxel[0];
				pVertices[i].deluxel[1] = m_pVerts[i].deluxel[2]; // prohod Y za Z
				pVertices[i].deluxel[2] = m_pVerts[i].deluxel[1];

				//barvy
#ifdef HACK_VERTEXCOLOR
				pVertices[i].color = P3DCOLOR_XRGB(255,255,255);
#else
				pVertices[i].color = P3DCOLOR_XRGB(m_pVerts[i].color[0], m_pVerts[i].color[1], m_pVerts[i].color[2]);
#endif

				pVertices[i].tan[0] = pVertices[i].tan[1] = pVertices[i].tan[2]  = 0.0f;
				pVertices[i].bitan[0] = pVertices[i].bitan[1] = pVertices[i].bitan[2]  = 0.0f;
			}

			ComputeTangents(pVertices); // Spocti tangent a bitangent
			m_pVB->Unlock();
		}
	}
	else
	{
		CON(MSG_ERR_FATAL, "BSPMAP: Can't create vertex buffer!");
	}
	/* LOADING SCREEN */
	DrawLoadScr(82, "Initialization...");

	// initializuj CBitsety dynamickych svetiel
	g_pLightManager->SetMapFaceNum (numFaces);

	for(i=0; i<numFaces; i++)
	{
		float fTemp = m_pFaces[i].lm_origin[1];
		m_pFaces[i].lm_origin[2] = m_pFaces[i].lm_origin[1];
		m_pFaces[i].lm_origin[1] = m_pFaces[i].lm_origin[2];

		fTemp = m_pFaces[i].normal[1];
		m_pFaces[i].normal[2] = m_pFaces[i].normal[1];
		m_pFaces[i].normal[1] = m_pFaces[i].normal[2];
	}

	for(i = 0; i < numLeaves; i++)
	{
		// Swap the y and z values, and negate the new z so Y is up
		int f_temp = m_pLeaves[i].mins[1];
		m_pLeaves[i].mins[1] = m_pLeaves[i].mins[2];
		m_pLeaves[i].mins[2] = f_temp;
		f_temp = m_pLeaves[i].maxs[1];
		m_pLeaves[i].maxs[1] = m_pLeaves[i].maxs[2];
		m_pLeaves[i].maxs[2] = f_temp;
	}

/*
	for(i = 0; i < numModels; i++)
	{
		// Swap the y and z values, and negate the new z so Y is up
		int f_temp = m_pModels[i].mins[1];
		m_pModels[i].mins[1] = m_pModels[i].mins[2];
		m_pModels[i].mins[2] = f_temp;
		f_temp = m_pModels[i].maxs[1];
		m_pModels[i].maxs[1] = m_pModels[i].maxs[2];
		m_pModels[i].maxs[2] = f_temp;
	}
*/

	for(i = 0; i < numPlanes; i++)
	{
		// Swap the y and z values, and negate the new z so Y is up
		float f_temp = m_pPlanes[i].normal.y;
		m_pPlanes[i].normal.y = m_pPlanes[i].normal.z;
		m_pPlanes[i].normal.z = f_temp;
	}

	// create index buffer
	if (SUCCEEDED (g_pD3DDevice->CreateIndexBuffer (numIndex * 4, 0, D3DFMT_INDEX32, \
																	D3DPOOL_MANAGED, &m_pIB, NULL)))
	{
		BSP_INDEX *pIndices = NULL;
		if (SUCCEEDED (m_pIB->Lock (0, 0, (void**)&pIndices, 0)))
		{
			memcpy (pIndices, m_pIndices, numIndex * sizeof(BSP_INDEX)); // zkopíruj indexy do nového bufferu
			m_pIB->Unlock();
		}
	}

	/* LOADING SCREEN */
	DrawLoadScr(85, "Creating static geometry for physics...");

	// ******************* CREATE PHYSICS COLLISION
	
	m_pBody = (IP3DPhysBody*)I_GetClass(IP3DPHYS_PHYSBODY);
	PhysTrimeshShapeDesc desc;
	desc.pointStrideBytes = sizeof(P3DXVector3D);
	desc.triangleStrideBytes = sizeof(UINT)*3;
	desc.numTriangles=0;
	desc.numVertices = numVerts;

	// Pridej vrcholy
	P3DXVector3D *verts = new P3DXVector3D[desc.numVertices];
	for (i = 0; i < numVerts; i++)
	{
		verts[i].x = m_pVerts[i].position[0];
		verts[i].y = m_pVerts[i].position[1];
		verts[i].z = m_pVerts[i].position[2];
	}

	// Pridej triangly
	for (i = 0; i < numFaces; i++)
	{
		//TODO: povrchy!!! Playerclip a pod.
		// pøeskoè povrchy, které jsou prùchodné
		if (m_pTextures[m_pFaces[i].texture].flags & SOD_SURF_NOPHYS)
			continue;

		for (int j=0; j<m_pFaces[i].n_indexes; j++) {	// projdi vsechny trojùhelníhy scény
			desc.numTriangles++;
		}
	}
	UINT *indices = new UINT[desc.numTriangles*3];
	UINT currIndex=0;
	for (i = 0; i < numFaces; i++)
	{
		//TODO: povrchy!!! Playerclip a pod.
		// pøeskoè povrchy, které jsou prùchodné
		if (m_pTextures[m_pFaces[i].texture].flags & SOD_SURF_NOPHYS)
			continue;

		for (int j=0; j<m_pFaces[i].n_indexes; j++) {	// projdi vsechny trojuhelníhy dane face
			//A - vrchol trianglu
			indices[currIndex] = m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3];
			currIndex++;
			//B - vrchol trianglu
			indices[currIndex] = m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+1];
			currIndex++;
			//C - vrchol trianglu
			indices[currIndex] = m_pFaces[i].vertex+m_pIndices[m_pFaces[i].index+j*3+2];
			currIndex++;
		}
	}

	desc.points = verts;
	desc.triangles = indices;

	PhysShapeDesc descShape;
	descShape.mass = 10.0f;
	m_pBody->AddShapeTrimesh(descShape, desc);
	m_pBody->CreateBody(false);

	delete[] indices;
	delete[] verts;

	//	m_nBody->SetGameMaterialGroupID (g_pNewtonWorld->GetGameMaterialID (physmat_Default));	// asi nie je potrebne???
	// nastavenie velkosti fyzikalneho sveta
	/*P3DXVector3D pointMin, pointMax;
	pointMin.x = m_fWorldMin[0]; pointMin.y = m_fWorldMin[1]; pointMin.z = m_fWorldMin[2];	// KEX 17.03.07: neprehodit Y a Z - uz prehozeno
	pointMax.x = m_fWorldMax[0]; pointMax.y = m_fWorldMax[1]; pointMax.z = m_fWorldMax[2];	// neprehodit Y a Z*/

	// UPRAV STRUKTURY
	for(i = 0;i<numFaces;i++)
	{
		if (m_pTextures[m_pFaces[i].texture].contents==SOD_CONT_WATER)
		{
			m_pTextures[m_pFaces[i].texture].flags &= SOD_SURF_NODRAW; // nekresli vodu
		}
	}
	
	// WORLDSPAWN ENTITY "PRELOAD"
	ParseWorldspawnEntity();

	// ******************* COMPUTE LIGHTGRID BOUNDS *******************
	// urcenie world bounding boxu
	float m_fMapWorldMin[3], m_fMapWorldMax[3];
	if (numModels > 0)
	{
		m_fMapWorldMin[0] = m_pModels[0].mins[0];
		m_fMapWorldMin[1] = m_pModels[0].mins[1];
		m_fMapWorldMin[2] = m_pModels[0].mins[2];
		m_fMapWorldMax[0] = m_pModels[0].maxs[0];
		m_fMapWorldMax[1] = m_pModels[0].maxs[1];
		m_fMapWorldMax[2] = m_pModels[0].maxs[2];
	}

	float	maxs[3];
	for (int i = 0 ; i < 3 ; i++ )
	{
		m_lightGridOrigin[i] = m_lightGridCellSize[i] * ceil (m_fMapWorldMin[i] / m_lightGridCellSize[i]);
		maxs[i] = m_lightGridCellSize[i] * floor (m_fMapWorldMax[i] / m_lightGridCellSize[i]);
		m_lightGridBounds[i] = (int)((maxs[i] - m_lightGridOrigin[i]) / m_lightGridCellSize[i] + 1);
	}

	if (numLightGrid != m_lightGridBounds[0] * m_lightGridBounds[1] * m_lightGridBounds[2])
		CON (MSG_CON_ERR, "BSPMAP: LightGrid cells count do not match!");
	/*
	// deal with overbright lightgrid bits
	for ( i = 0 ; i < numLightGrid ; i++ ) {
	R_ColorShiftLightingBytes( &w->lightGridData[i*8], &w->lightGridData[i*8] );
	R_ColorShiftLightingBytes( &w->lightGridData[i*8+3], &w->lightGridData[i*8+3] );
	}
	*/

	/* LOADING SCREEN */
	DrawLoadScr(93, "Parsing and loading entities...");

	// PARSE ENTITIES AND CREATE ENTITY INSTANCES
	CON(MSG_CON_INFO, "Parsing entities...");
	g_pMeshLoader->StartCaching ();
	ParseEntities();
	SAFE_DELETE(m_pEnts->ents); // now we can free memory used for entity text data
	g_pMeshLoader->StopCaching ();
	PostUnLoad();

	m_pEntMgr->Finalize();//konec nacitani entit

	/* LOADING SCREEN */
	DrawLoadScr(98, "Finalizing...");

	// optimalizuj a udelej posledni initializace
	for (int i=0;i<numLeaves;i++)
	{
		// pretypovanie na float
		m_pLeaves[i].fmins[0] = (float)m_pLeaves[i].mins[0];
		m_pLeaves[i].fmins[1] = (float)m_pLeaves[i].mins[1];
		m_pLeaves[i].fmins[2] = (float)m_pLeaves[i].mins[2];
		m_pLeaves[i].fmaxs[0] = (float)m_pLeaves[i].maxs[0];
		m_pLeaves[i].fmaxs[1] = (float)m_pLeaves[i].maxs[1];
		m_pLeaves[i].fmaxs[2] = (float)m_pLeaves[i].maxs[2];
	}

	// initializuj CBitsety
	m_DrawnFaces.Resize(numFaces);

	//////////////////////////////////////////////////////////////////////////
	// FIXME: dynamic light debug !!!
/*
	DYN_LIGHT_INFO_USER	dynLight;
	P3DXMatrix	lightMat;
	lightMat.SetIdentityMatrix ();
	lightMat.Translate (400, 100, 200);
	dynLight.fRadius = 400;
	dynLight.pos_rot = lightMat;
	dynLight.szFlicker_str = NULL;
	dynLight.sz_light_texture = "textures\\models\\special\\LightPatern.dds";	// LightCube.dds
	//dynLight.sz_light_texture = NULL;
	g_pLightManager->AddDynamicLight (dynLight);
*/

	// FIXME: debug !!!
	//////////////////////////////////////////////////////////////////////////

	/* LOADING SCREEN */
	DrawLoadScr(100, ":) Done :)");
}

bool CP3DBSPMap::Load(const char* FileName)
{
	if (FileName == NULL) return false; // nenastaveno
	FSFILE *fp = NULL;
	// Check if the .bsp file could be opened
	char TmpFn[MAX_PATH]="maps/";
	strcat(TmpFn, FileName);

	// odstran mezeru z konce
	for (int qi=(int)strlen(TmpFn)-1; qi>0; qi--)
	{
		if (TmpFn[qi]==' ' || TmpFn[qi]==0) 
			TmpFn[qi]=0;
		else
			break;
	}

	if (FileName[strlen(FileName)-4]!='.') strcat(TmpFn, ".bsp");
	if((fp = g_pFS->Open(TmpFn, "rb")) == NULL)
	{
		CON(MSG_CON_ERR, "Can't find map '%s'!", TmpFn);
		return false;
	}

	BSP_HEADER header;
	g_pFS->Read(&header, 1, sizeof(BSP_HEADER), fp);

	// check bsp version
	if (strncmp(header.idNumber, "7BSP", 4) != 0)
	{
		g_pFS->Close(fp);
		CON(MSG_CON_ERR, "Wrong BSP map prefix '%s'! Should be '7BSP'...", header.idNumber);
		return false;
	}
	if (header.version != BSP_HEADER_VERSION)
	{
		g_pFS->Close(fp);
		CON(MSG_CON_ERR, "Wrong BSP map version %d! Should be %d...", header.version, BSP_HEADER_VERSION);
		return false;
	}

	char mapShot[256];
	strncpy(mapShot, FileName, sizeof(mapShot)-1);
	if (FileName[strlen(FileName)-4]=='.') mapShot[strlen(FileName)-4] = 0;
	for (int qi=(int)strlen(mapShot)-1; qi>0; qi--)
	{
		if (mapShot[qi]==' ' || mapShot[qi]==0) 
			mapShot[qi]=0;
		else
			break;
	}
	char temp[256];
	strncpy(temp, mapShot, sizeof(temp)-1);
	sprintf(mapShot, "levelshots\\%s", temp);
	if (!m_pLoadTexture->Load(mapShot, TT_2D)) // nacti pro daný level
		if (!m_pLoadTexture->Load("levelshots\\default", TT_2D)) //nacti klasickou
			CON(MSG_CON_ERR, "Can't load level texture!");
	if (!m_pLoadBar->Load("levelshots\\loadbar", TT_2D))
		CON(MSG_CON_ERR, "Can't loadbar texture!");
	if (!m_pLoadBarBg->Load("levelshots\\loadbarbg", TT_2D))
		CON(MSG_CON_ERR, "Can't loadbarbg texture!");

	/* LOADING SCREEN */
	DrawLoadScr(5, "Clearing memory...");

	UnLoad ();

	/* LOADING SCREEN */
	DrawLoadScr(8, "Loading map...");

	CON(MSG_CON_INFO, "Loading map '%s'...", FileName);

	//allocate the space needed for our data structures 
	numVerts = header.entries[lVertcies].length / sizeof(BSP_VERTEX);
	m_pVerts = new BSP_VERTEX[numVerts];

	numFaces = header.entries[lFaces].length / sizeof(BSP_FACE);
	m_pFaces = new BSP_FACE[numFaces];

	numIndex = header.entries[lIndices].length / sizeof(BSP_INDEX);
	m_pIndices = new BSP_INDEX[numIndex];

	numTextures = header.entries[lTextures].length / sizeof(BSP_TEXTURE);
	m_pTextures = new BSP_TEXTURE [numTextures];
	m_pMaterials = new BSP_MATERIAL [numTextures];
	m_pMaterialID = new DWORD[numTextures];
	m_pTextureMaps = new LPDIRECT3DTEXTURE9[numTextures];

	numLights = header.entries[lLightmaps].length / sizeof(BSP_LIGHTMAP);
	m_pLightmap = new BSP_LIGHTMAP [numLights];
	m_pLightMaps = new LPDIRECT3DTEXTURE9[numLights];
	if (!numLights) 
	{
		CON(MSG_CON_ERR, "Error: BSP Map without lightmaps!");
		return false;
	}

	numPlanes = header.entries[lPlanes].length / sizeof(BSP_PLANE);
	m_pPlanes = new BSP_PLANE[numPlanes]; 

	numNodes = header.entries[lNodes].length / sizeof(BSP_NODE);
	m_pNodes = new BSP_NODE[numNodes];

	numLeaves = header.entries[lLeafs].length / sizeof(BSP_LEAF);
	m_pLeaves = new BSP_LEAF[numLeaves];

	numLeafFaces = header.entries[lLeaffaces].length / sizeof(BSP_LEAFFACES);
	m_pLeafFaces = new BSP_LEAFFACES[numLeafFaces];

	numLightGrid = header.entries[lLightgrid].length / sizeof(BSP_LIGHTGRID);
	if (numLightGrid > 0)
		m_pLightGrid = new BSP_LIGHTGRID[numLightGrid];

	numModels = header.entries[lModels].length / sizeof(BSP_MODEL);
	m_pModels = new BSP_MODEL[numModels];

	g_pFS->Seek(fp, header.entries[lVertcies].offset, SEEK_SET);
	g_pFS->Read(m_pVerts, numVerts, sizeof(BSP_VERTEX), fp);

	g_pFS->Seek(fp, header.entries[lIndices].offset, SEEK_SET);
	g_pFS->Read(m_pIndices, numIndex, sizeof(BSP_INDEX), fp);

	g_pFS->Seek(fp, header.entries[lFaces].offset, SEEK_SET);
	g_pFS->Read(m_pFaces, numFaces, sizeof(BSP_FACE), fp);

	g_pFS->Seek(fp, header.entries[lTextures].offset, SEEK_SET);
	g_pFS->Read(m_pTextures, numTextures, sizeof(BSP_TEXTURE), fp);

	g_pFS->Seek(fp, header.entries[lLightmaps].offset, SEEK_SET);

	g_pFS->Read(m_pLightmap, numLights, sizeof(BSP_LIGHTMAP), fp);

	g_pFS->Seek(fp, header.entries[lPlanes].offset, SEEK_SET);
	g_pFS->Read(m_pPlanes, numPlanes, sizeof(BSP_PLANE), fp);

	g_pFS->Seek(fp, header.entries[lNodes].offset, SEEK_SET);
	g_pFS->Read(m_pNodes, numNodes, sizeof(BSP_NODE), fp);

	g_pFS->Seek(fp, header.entries[lLeafs].offset, SEEK_SET);
	g_pFS->Read(m_pLeaves, numLeaves, sizeof(BSP_LEAF), fp);

	g_pFS->Seek(fp, header.entries[lLeaffaces].offset, SEEK_SET);
	g_pFS->Read(m_pLeafFaces, numLeafFaces, sizeof(BSP_LEAFFACES), fp);

	if (numLightGrid > 0)
	{
		g_pFS->Seek(fp, header.entries[lLightgrid].offset, SEEK_SET);
		g_pFS->Read(m_pLightGrid, numLightGrid, sizeof(BSP_LIGHTGRID), fp);
	}

	g_pFS->Seek(fp, header.entries[lModels].offset, SEEK_SET);
	g_pFS->Read(m_pModels, numModels, sizeof(BSP_MODEL), fp);

	// entity.... (mozna lze zjednodusit, .... ale co... zbytecne)
	g_pFS->Seek(fp, header.entries[lEntities].offset, SEEK_SET);
	m_pEnts = new BSP_ENTITY();
	m_pEnts->ents = new char[header.entries[lEntities].length];
	g_pFS->Read(m_pEnts->ents, 1, header.entries[lEntities].length, fp);

	if(header.entries[lVisdata].length)
	{
		//Read VisData
		g_pFS->Seek(fp, header.entries[lVisdata].offset, SEEK_SET);
		g_pFS->Read(&(m_sVisData.n_vecs), 1, sizeof(int), fp);
		g_pFS->Read(&(m_sVisData.vecSize), 1, sizeof(int), fp);

		int size = (m_sVisData.n_vecs * m_sVisData.vecSize);
		m_sVisData.vecs = new byte[size];

		g_pFS->Read(m_sVisData.vecs, 1, sizeof(byte) * size, fp);
	}
	else
	{
		CON(MSG_CON_ERR, "Map %s doesn't contain visibility data, therefore can't be loaded. Please compile map with -vis cmdline switch!", FileName);
		return false;//m_sVisData.vecs = NULL;
	}

	// Close the file
	g_pFS->Close(fp);

	/* LOADING SCREEN */
	DrawLoadScr(20, "Building scene...");

	// Create renderer data from structs
	Build();

	CON(MSG_CON_INFO, "Map '%s' successfuly loaded!", FileName);
	CON(MSG_CON_INFO, "Planes: %d | Nodes: %d | Leaves: %d | Verts: %d | Faces: %d | Indices: %d | Materials: %d | LMs: %d | LG Cells: %d | Models: %d", \
		numPlanes, numNodes, numLeaves, numVerts, numFaces, numIndex, numTextures, numLights, numLightGrid, numModels);

	m_bLoaded = true;

	return true;
}

void CP3DBSPMap::UnLoad()
{
	if (!m_bLoaded) return;

	g_pMaterialManager->TurnOffShaders ();

	// clear prev. map resources
	CON(MSG_CON_INFO, "Clearing map memory...");

	m_intCurCamCluster = -1;
	m_lightGridCellSize[0] = 64.0f; m_lightGridCellSize[1] = 64.0f; m_lightGridCellSize[2] = 128.0f;

	// destroy phys. body
	SAFE_DELETE (m_pBody)

	SAFE_RELEASE(m_pVB)
	SAFE_RELEASE(m_pIB)

	g_pLastTextureMaps = NULL;
	g_intLastNumTextures = 0;
	if (pCVr_use_loadcaching.GetBool ())
	{
		if (numTextures > 0)
			if (g_pLastTextureMaps = new LPDIRECT3DTEXTURE9[numTextures])
			{
				memcpy (g_pLastTextureMaps, m_pTextureMaps, sizeof(LPDIRECT3DTEXTURE9) * numTextures);
				g_intLastNumTextures = numTextures;
			}
	}
	else
	{
		g_pMeshLoader->UnloadAll ();
		g_pMaterialManager->UnloadAll ();	// FIXME: ???
		// clear textures
		if (m_pTextureMaps)
			for (int i=0; i < numTextures; i++)
				g_TextureLoader.UnloadTexture (m_pTextureMaps[i]);
	}

	g_pAlphaManager->UnloadAll ();
	g_pSoundMan->UnloadAll ();		// FIXME: ???

	SAFE_DELETE_ARRAY (m_pTextureMaps)

	if (m_pLightMaps)
		for (int i=0; i < numLights; i++) // clear textures
			SAFE_RELEASE (m_pLightMaps[i])
	SAFE_DELETE_ARRAY (m_pLightMaps)

	SAFE_DELETE(m_pEnts);
	SAFE_DELETE_ARRAY(m_pModels);
	SAFE_DELETE_ARRAY(m_pLightGrid);
	SAFE_DELETE_ARRAY(m_pPlanes);
	SAFE_DELETE_ARRAY(m_pNodes);
	SAFE_DELETE_ARRAY(m_pLeaves);
	SAFE_DELETE_ARRAY(m_pLeafFaces);
	SAFE_DELETE_ARRAY(m_pLightmap);
	SAFE_DELETE_ARRAY(m_pTextures);
	SAFE_DELETE_ARRAY(m_pMaterials);
	SAFE_DELETE_ARRAY(m_pMaterialID);
	SAFE_DELETE_ARRAY(m_pVerts);
	SAFE_DELETE_ARRAY(m_pFaces);
	SAFE_DELETE_ARRAY(m_pIndices);

	m_pEntMgr->KillAll(); // vymaz pripadne nactene entity

	m_bLoaded = false;
}

void CP3DBSPMap::PostUnLoad()
{
	// uvolnenie bsp_map textur, ktore sa v novej mape nenachadzaju - caching
	if (g_pLastTextureMaps)
	{
		for (int i=0; i < g_intLastNumTextures; i++) // clear textures
			g_TextureLoader.UnloadTexture (g_pLastTextureMaps[i]);
		delete[] g_pLastTextureMaps;
	}
	g_pLastTextureMaps = NULL;
	g_intLastNumTextures = 0;
}

inline void CP3DBSPMap::CameraUpdate (P3DXVector3D &camPos)
{
	if (m_bLoaded) m_intCurCamCluster = FindCluster (camPos);
}

inline int CP3DBSPMap::GetCameraCluster ()
{
	return m_intCurCamCluster;
}

inline int CP3DBSPMap::FindCluster (P3DXVector3D &camPos)
{
	return m_pLeaves[FindLeaf(camPos)].cluster;
}

inline int CP3DBSPMap::FindLeaf(P3DXVector3D& camPos)
{
	int index = 0;

	while (index >= 0) {
		const BSP_NODE&  node  = m_pNodes[index];
		const BSP_PLANE& plane = m_pPlanes[node.plane];

		// Distance from point to a plane
		const double distance = plane.normal.x * camPos.x + 
			plane.normal.y * camPos.y + 
			plane.normal.z * camPos.z - plane.dist;

		if (distance >= 0)
			index = node.children[0];
		else
			index = node.children[1];
	}

	return ~index;
}

inline bool CP3DBSPMap::ClusterVisible (int testCluster)
{
#ifdef HACK_VISIBILITY
	return true;
#endif
	if ((testCluster & 0x80000000) || (m_intCurCamCluster & 0x80000000))
		return false;

	unsigned char visSet = m_sVisData.vecs[(m_intCurCamCluster * m_sVisData.vecSize) + (testCluster >> 3)];
	return (visSet & (1 << (testCluster & 7))) != 0;
}

inline bool CP3DBSPMap::ClusterVisible (int srcCluster, int destCluster)
{
#ifdef HACK_VISIBILITY
	return true;
#endif
	if ((destCluster & 0x80000000) || (srcCluster & 0x80000000))
		return false;

	unsigned char visSet = m_sVisData.vecs[(srcCluster * m_sVisData.vecSize) + (destCluster >> 3)];
	return (visSet & (1 << (destCluster & 7))) != 0;
}


void CP3DBSPMap::SetShaderParameters(DWORD matID, DWORD texID, DWORD lmID, SHADER_DYNLIGHT_INPUT *pShaderDynlightsInput)
{
	Prof(RENDERER_CP3DBSPMap__SetShaderParameters);
	WORD wAllParams, wParamsToSet;

	g_pMaterialManager->QueryParameters (matID, (pShaderDynlightsInput!=NULL), \
		g_paramTable, wAllParams, wParamsToSet);
	for (DWORD i=0; i<wParamsToSet; i++)
	{
		switch (g_paramTable[i].SemanticID)
		{
		case SEM_TextureLayer0:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &m_pTextureMaps[texID];
			break;
		case SEM_TextureLayer1:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &m_pLightMaps[lmID];
			break;
		case SEM_TextureLayer2:
			STAT(STAT_SHADER_TEXTURES, 1);
			//if (!bDeluxelMaps) break; // neobsahuje deluxelmapy!	- KEXIK 31.1.2007: Zatim neni potreba, vsechny mapy mame s deluxely
			g_paramTable[i].pValue = &m_pLightMaps[lmID+1];
			break;
			//////////////////////////////////////////////////////////////////////////
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
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[0];
			break;
		case SEM_DynLightTexture1:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[1];
			break;
		case SEM_DynLightTexture2:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[2];
			break;
		case SEM_DynLightTexture3:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &pShaderDynlightsInput->light_textures[3];
			break;
		}
	}
	g_pMaterialManager->SetParameters (matID, g_paramTable, wAllParams);
}

void CP3DBSPMap::Loop ()
{
	if (!m_bLoaded) return;
	Prof(RENDERER_CP3DBSPMap__Loop);

	m_DrawnFaces.ClearAll();

	// Find Visible Face Data
	int intLeafFace;
	for(int l = numLeaves-1; l >= 0 ; --l)
	{
		if(ClusterVisible(m_pLeaves[l].cluster))
		{
			BSP_LEAF &bsp_leaf = m_pLeaves[l];
			if(g_pFrustum->BoxInFrustum(bsp_leaf.fmins[0], bsp_leaf.fmins[1], bsp_leaf.fmins[2], \
				bsp_leaf.fmaxs[0], bsp_leaf.fmaxs[1], bsp_leaf.fmaxs[2]))
			{ // is in frustum - select it for drawing
				for (int f = bsp_leaf.n_leaffaces-1; f >= 0 ; --f)
				{
					intLeafFace = m_pLeafFaces[bsp_leaf.leafface + f];
					if (!(m_pTextures[m_pFaces[intLeafFace].texture].flags & SOD_SURF_NODRAW))
						if(!m_DrawnFaces.On(intLeafFace))
							m_DrawnFaces.Set(intLeafFace);
				}
			}
		}
	}
}

void CP3DBSPMap::Render()
{
	if (!m_bLoaded) return;
	Prof(RENDERER_CP3DBSPMap__Render);

	P3DXMatrix m;
	m.SetIdentityMatrix ();
	g_pRenderer->SetWorldTransform (m);

	g_pD3DDevice->SetStreamSource (0, m_pVB, 0, sizeof(BSP_VERTEX_USED));
	g_pD3DDevice->SetIndices (m_pIB);
	g_pD3DDevice->SetVertexDeclaration (m_pVertexDeclaration);
	STAT(STAT_SET_RENDER_STATE, 2);
	g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW); // WATER: KEXIK: DEBUG: D3DCULL_NONE
	g_pD3DDevice->SetRenderState (D3DRS_LIGHTING, FALSE);
	g_pRenderer->SetTextureStageState(0, RST_COLORARG1, D3DTA_TEXTURE);

	LPDIRECT3DTEXTURE9 diffTex, LMTex;
	int lastLM=-1,lastTex=-1;
	SHADER_DYNLIGHT_INPUT shader_lights, *pShaderDynlightsInput = NULL;
	shader_lights.light_count = 0;
	bool bUseDynLighting = false;
	if (pCVr_dyn_lighting)
		bUseDynLighting = pCVr_dyn_lighting->GetBool ();

	//////////////////////////////////////////////////////////////////////////
	for (int i=numFaces-1; i>=0; --i)
	{
		if (m_DrawnFaces.On(i) == 0) continue;

		int intTexture = m_pFaces[i].texture;

		if (m_pMaterialID[intTexture]!=0xFFFFFFFF) // pokud je shader material
		{
			if (bUseDynLighting)
				if (g_pLightManager->GetFaceLightInfo (i, shader_lights))
					pShaderDynlightsInput = &shader_lights;
				else
					pShaderDynlightsInput = NULL;

			// default LM shader
			if (m_pMaterialID[intTexture] == m_dwDefLMmatID)
			{
				static LPDIRECT3DTEXTURE9 lastDiff0, lastLM0;
				if (lastDiff0 != m_pTextureMaps[intTexture])
					diffTex = lastDiff0 = m_pTextureMaps[intTexture];
				else
					diffTex = NULL;
				if ((m_pFaces[i].lm_index >= 0) && (lastLM0 != m_pLightMaps[m_pFaces[i].lm_index]))
					LMTex = lastLM0 = m_pLightMaps[m_pFaces[i].lm_index];
				else
					LMTex = NULL;

				if (pShaderDynlightsInput == NULL)		// static lighting
				{
					g_pMaterialManager->Establish_LM_Material (diffTex, LMTex);
					STAT(STAT_DRAW_CALLS, 1);
					STAT(STAT_DRAW_VERTS, m_pFaces[i].n_indexes);
					g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, m_pFaces[i].vertex, \
						0, m_pFaces[i].n_verts, m_pFaces[i].index, m_pFaces[i].n_indexes);
				}
				else												// dynamic lighting
				{
					// aplikuj material
					SetShaderParameters(m_dwDefLMmatID, intTexture, m_pFaces[i].lm_index, pShaderDynlightsInput);
					g_pMaterialManager->SetMaterial(m_dwDefLMmatID, (DWORD)pShaderDynlightsInput->light_count);
					// vykresli plosku
					do{
						STAT(STAT_DRAW_CALLS, 1);
						STAT(STAT_DRAW_VERTS, m_pFaces[i].n_indexes);
						g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, m_pFaces[i].vertex, \
							0, m_pFaces[i].n_verts, m_pFaces[i].index, m_pFaces[i].n_indexes);
					}while(g_pMaterialManager->NextPass(m_dwDefLMmatID));
				}
			}

			// default vertex_light shader
			else if (m_pMaterialID[intTexture] == m_dwDefVertexmatID)
			{
				static LPDIRECT3DTEXTURE9 lastDiff1;
				if (lastDiff1 != m_pTextureMaps[intTexture])
					diffTex = lastDiff1 = m_pTextureMaps[intTexture];
				else
					diffTex = NULL;

				if (pShaderDynlightsInput == NULL)		// static lighting
				{
					g_pMaterialManager->Establish_VertexLight_Material (diffTex);
					STAT(STAT_DRAW_CALLS, 1);
					STAT(STAT_DRAW_VERTS, m_pFaces[i].n_indexes);
					g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, m_pFaces[i].vertex, \
						0, m_pFaces[i].n_verts, m_pFaces[i].index, m_pFaces[i].n_indexes);
				}
				else												// dynamic lighting
				{
					// aplikuj material
					SetShaderParameters(m_dwDefVertexmatID, intTexture, 0, pShaderDynlightsInput);
					g_pMaterialManager->SetMaterial(m_dwDefVertexmatID, (DWORD)pShaderDynlightsInput->light_count);
					// vykresli plosku
					do{
						STAT(STAT_DRAW_CALLS, 1);
						STAT(STAT_DRAW_VERTS, m_pFaces[i].n_indexes);
						g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, m_pFaces[i].vertex, \
							0, m_pFaces[i].n_verts, m_pFaces[i].index, m_pFaces[i].n_indexes);
					}while(g_pMaterialManager->NextPass(m_dwDefVertexmatID));
				}
			}
			// custom shader
			else
			{
				// aplikuj material
				SetShaderParameters(m_pMaterialID[intTexture], intTexture, m_pFaces[i].lm_index, pShaderDynlightsInput);
				g_pMaterialManager->SetMaterial(m_pMaterialID[intTexture], (pShaderDynlightsInput != NULL) ? (DWORD)pShaderDynlightsInput->light_count : 0);
				// vykresli plosku
				do{
					STAT(STAT_DRAW_CALLS, 1);
					STAT(STAT_DRAW_VERTS, m_pFaces[i].n_indexes);
					g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, m_pFaces[i].vertex, \
						0, m_pFaces[i].n_verts, m_pFaces[i].index, m_pFaces[i].n_indexes);
				}while(g_pMaterialManager->NextPass(m_pMaterialID[intTexture]));
			}
		}
		//////////////////////////////////////////////////////////////////////////
		// fixed pipeline - tato moznost by nemala nastat!
		else
		{
			// vypni shadery
			g_pMaterialManager->TurnOffShaders();
			// nastav diffuzni
			if(intTexture && lastTex!=intTexture) 
			{
				g_pRenderer->SetTexture (m_pTextureMaps[intTexture]);
				lastTex=intTexture;
			}
			if (m_pFaces[i].type == FACE_PLANAR)		// lightmap
			{
				// nastav lm
				if(m_pFaces[i].lm_index >= 0 && lastLM!=m_pFaces[i].lm_index) 
				{
					g_pRenderer->SetTexture (m_pLightMaps[m_pFaces[i].lm_index], 1);
					lastLM=m_pFaces[i].lm_index;
				}
				// VYKRESLI BSP PLOSKY - POUZIJ LM
				g_pRenderer->SetTextureStageState(0, RST_COLOROP, D3DTOP_SELECTARG1 );
				g_pRenderer->SetTextureStageState(1, RST_COLORARG1, D3DTA_TEXTURE);
				g_pRenderer->SetTextureStageState(1, RST_COLORARG2, D3DTA_CURRENT);
				g_pRenderer->SetTextureStageState(1, RST_COLOROP, D3DTOP_MODULATE);
			}
			else													// vertexlight
			{
				// VYKRESLI BSP PLOSKY OBEKTU - POUZIJ BARVY VRCHOLU
				g_pRenderer->SetTextureStageState(0, RST_COLOROP, D3DTOP_MODULATE );
				g_pRenderer->SetTextureStageState(0, RST_COLORARG2, D3DTA_DIFFUSE);
				g_pRenderer->SetTextureStageState(1, RST_COLOROP, D3DTOP_DISABLE);
			}

			// vykresli plosku - fixed pipeline
			STAT(STAT_DRAW_CALLS, 1);
			g_pD3DDevice->DrawIndexedPrimitive (D3DPT_TRIANGLELIST, m_pFaces[i].vertex, \
				0, m_pFaces[i].n_verts, m_pFaces[i].index, m_pFaces[i].n_indexes);
		}
	}

	g_pMaterialManager->TurnOffShaders ();
	g_pRenderer->SetTexture (NULL);

}


// pomocne premene
P3DSphere		g_BSphere;
BSPCLUSTER_LIST	*g_pClusterList;
DWORD			g_dwFoundedClusters;

//---------------------------------
inline void CP3DBSPMap::FindSphereColisionLeafs (int nodeIndex)
{
	if (g_dwFoundedClusters == MAX_CLUSTER_LIST_COUNT)
		return;

	if (nodeIndex < 0)		// this is a leaf
	{
		BSP_LEAF &leaf = m_pLeaves[-(nodeIndex + 1)];
		if (leaf.cluster >= 0)
		{
			for (register int i=g_dwFoundedClusters-1; i>=0; --i)
				if (g_pClusterList->intCluster[i] == leaf.cluster)
					return;
			g_pClusterList->intCluster[g_dwFoundedClusters] = leaf.cluster;
			g_dwFoundedClusters++;
		}
		return;
	}

	// this is a node
	const BSP_NODE&  node  = m_pNodes[nodeIndex];
	const BSP_PLANE& plane = m_pPlanes[node.plane];

	// distance from point to a plane
	const float distance = plane.normal.x * g_BSphere.pointOrigin.x + 
		plane.normal.y * g_BSphere.pointOrigin.y + 
		plane.normal.z * g_BSphere.pointOrigin.z - plane.dist;

	// je potrebne parsovat obe strany roviny
	if (abs(distance) < g_BSphere.fRadius)
	{
		FindSphereColisionLeafs (node.children [ (distance >= 0) ? 0 : 1 ] );
		FindSphereColisionLeafs (node.children [ (distance >= 0) ? 1 : 0 ] );
		return;
	}

	if (distance >= 0)
		FindSphereColisionLeafs (node.children[0]);
	else
		FindSphereColisionLeafs (node.children[1]);
}

// vypocita do ktorych clusterov "zasahuje" BSphere
//---------------------------------
void CP3DBSPMap::CalculateIntersectClusters (P3DSphere &BSphere, BSPCLUSTER_LIST &ClusterList)
{
	if ((m_pLeaves == NULL) || (m_pNodes == NULL) || (m_pPlanes == NULL))
		return;
	g_BSphere = BSphere;
	g_pClusterList = &ClusterList;
	for (DWORD i=0; i<MAX_CLUSTER_LIST_COUNT; i++)
		g_pClusterList->intCluster[i] = -1;
	ClusterList.dwNumClusters = g_dwFoundedClusters = 0;

	FindSphereColisionLeafs (0);
	ClusterList.dwNumClusters = g_dwFoundedClusters;
}

// vrati hodnoty osvetlenia pre zadany bod mapy
//---------------------------------
bool CP3DBSPMap::GetLightFromPoint (IN P3DXVector3D &point, OUT BSP_LIGHTCELL_INFO &lightCell)
{
	float	pointToGridOffset[3];
	int		pos[3];
	int		i, j;
	float	frac[3];
	int		gridStep[3];
	BSP_LIGHTGRID	*gridData;
	float	totalFactor;
	float	OutAmbient[3] = {0};
	float	OutDirectional[3] = {0};
	float	OutDirection[3] = {0};

#ifdef HACK_LIGHTGRID_COLOR
	lightCell.Ambient = P3DColorValueRGB (0.5f, 0.5f, 0.5f);
	lightCell.Directional = P3DColorValueRGB (0.6f, 0.6f, 0.6f);
	lightCell.Direction = P3DXVector3D (0,-1.0f,0);
	return true;
#endif

	if (m_pLightGrid == NULL)
	{
		lightCell.Ambient.r = 0.5f; lightCell.Ambient.g = 0.5f; lightCell.Ambient.b = 0.5f;
		lightCell.Directional.r = 0.6f; lightCell.Directional.g = 0.6f; lightCell.Directional.b = 0.6f;
		lightCell.Direction.x = lightCell.Direction.z = 0.0f; lightCell.Direction.y = -1.0f;
		return false;
	}

	memset (&lightCell, 0, sizeof(BSP_LIGHTCELL_INFO));

	pointToGridOffset[0] = point.x - m_lightGridOrigin[0];
	pointToGridOffset[1] = point.z - m_lightGridOrigin[1]; // konverzia spat na right-hand system
	pointToGridOffset[2] = point.y - m_lightGridOrigin[2]; // konverzia spat na right-hand system

	for ( i = 0 ; i < 3 ; i++ )
	{
		float	v = pointToGridOffset[i] * m_lightGridInverseSize[i];
		pos[i] = (int)floor (v);
		frac[i] = v - (float)pos[i];
		if (pos[i] < 0)
			pos[i] = 0;
		else if (pos[i] >= m_lightGridBounds[i] - 1)
			pos[i] = m_lightGridBounds[i] - 1;
	}

	// trilerp the light value
	gridStep[0] = 1;
	gridStep[1] = m_lightGridBounds[0];
	gridStep[2] = m_lightGridBounds[0] * m_lightGridBounds[1];

	gridData = m_pLightGrid + pos[0] + pos[1] * gridStep[1] + pos[2] * gridStep[2];

	totalFactor = 0;
	for (i=0; i<8; i++)
	{
		float			factor;
		BSP_LIGHTGRID	*data;
		float			lat, lng;
		float			normal[3];

		factor = 1.0;
		data = gridData;
		for (j=0; j<3 ; j++)
		{
			if (i & (1<<j)) {
				factor *= frac[j];
				data += gridStep[j];
			} else {
				factor *= (1.0f - frac[j]);
			}
		}

		if ((data->ambient[0] | data->ambient[1] | data->ambient[2]) == 0)
			continue;	// ignore samples in walls

		totalFactor += factor;
		OutAmbient[0] += factor * data->ambient[0];
		OutAmbient[1] += factor * data->ambient[1];
		OutAmbient[2] += factor * data->ambient[2];

		OutDirectional[0] += factor * data->directional[0];
		OutDirectional[1] += factor * data->directional[1];
		OutDirectional[2] += factor * data->directional[2];

		lat = (data->direction[1] * P3D_PI) / 360.0f;
		lng = (data->direction[0] * P3D_PI) / 360.0f;
		// decode X as cos( lat ) * sin( long )
		// decode Y as sin( lat ) * sin( long )
		// decode Z as cos( long )
		normal[0] = sin (lng);
		normal[1] = normal[0] * sin(lat);
		normal[0] *= cos(lat);
		normal[2] = cos (lng);

		OutDirection[0] += factor * normal[0];
		OutDirection[1] += factor * normal[1];
		OutDirection[2] += factor * normal[2];
	}

	if (totalFactor > 0 && totalFactor < 0.99f)
	{
		totalFactor = 1.0f / totalFactor;
		OutAmbient[0] *= totalFactor; OutAmbient[1] *= totalFactor; OutAmbient[2] *= totalFactor;
		OutDirectional[0] *= totalFactor; OutDirectional[1] *= totalFactor; OutDirectional[2] *= totalFactor;
	}

	/*
	// volitelny scaling
	#define AMBIENT_SCALE		1.2f;
	#define DIRECTIONAL_SCALE	1.2f;
	OutAmbient[0] *= AMBIENT_SCALE; OutAmbient[1] *= AMBIENT_SCALE; OutAmbient[2] *= AMBIENT_SCALE;
	OutDirectional[0] *= DIRECTIONAL_SCALE; OutDirectional[1] *= DIRECTIONAL_SCALE; OutDirectional[2] *= DIRECTIONAL_SCALE;
	*/

	// normalizacia
	totalFactor = sqrtf ((OutDirection[0] * OutDirection[0]) + \
		(OutDirection[1] * OutDirection[1]) + \
		(OutDirection[2] * OutDirection[2]));
	if (totalFactor > 0)
	{
		totalFactor = 1.0f / totalFactor;
		OutDirection[0] *= totalFactor;
		OutDirection[1] *= totalFactor;
		OutDirection[2] *= totalFactor;
	}

#define DIVIDER_255	0.0039215f		// (1 / 255)
	lightCell.Ambient.r = OutAmbient[0] * DIVIDER_255;
	lightCell.Ambient.g = OutAmbient[1] * DIVIDER_255;
	lightCell.Ambient.b = OutAmbient[2] * DIVIDER_255;
	lightCell.Directional.r = OutDirectional[0] * DIVIDER_255;
	lightCell.Directional.g = OutDirectional[1] * DIVIDER_255;
	lightCell.Directional.b = OutDirectional[2] * DIVIDER_255;
	lightCell.Direction.x = OutDirection[0];
	lightCell.Direction.y = -OutDirection[2];		// right-handed system legacy
	lightCell.Direction.z = OutDirection[1];		// right-handed system legacy
	return true;
}

//////////////////////////////////////////////////////////////////////////
// pomocne premene
P3DSphere		g_DynLightBSphere;
CBitset				*g_pLitFaces;
int					g_intSrcCluster;
P3DXVector3D		g_TempVector;

//#define MAKE_NORMAL_LIGHT_TEST
//---------------------------------
void CP3DBSPMap::FindDynLightClustersRec (int nodeIndex)
{
	if (nodeIndex < 0)		// this is a leaf
	{
		BSP_LEAF &bsp_leaf = m_pLeaves[-(nodeIndex + 1)];
		if (ClusterVisible (g_intSrcCluster, bsp_leaf.cluster))
			for (int i = bsp_leaf.n_leaffaces-1; i >= 0 ; --i)
			{
				int intLeafFace = m_pLeafFaces[bsp_leaf.leafface + i];
				// vynechanie facov, ktore su natocene smerom "od zdroja svetla" - iba pre bsp facy
				// FIXME: tato cast ale vyluci aj par facov co by mali zostat osvetlene !!! ???
				#ifdef MAKE_NORMAL_LIGHT_TEST
				if (m_pFaces[intLeafFace].type == FACE_PLANAR)
				{
					g_TempVector = g_DynLightBSphere.pointOrigin;
					// pouzitie prveho vertexy facu
					g_TempVector -= *(P3DXVector3D*)&m_pVerts[m_pFaces[intLeafFace].vertex].position[0];
					g_TempVector.Normalize ();
					float fDotProduct = g_TempVector * (*(P3DXVector3D*)&m_pFaces[intLeafFace].normal[0]);
					if (fDotProduct < 0.0f)
						continue;
				}
				#endif
				if(!g_pLitFaces->On(intLeafFace))
					g_pLitFaces->Set(intLeafFace);
			}
			return;
	}

	// this is a node
	const BSP_NODE&  node  = m_pNodes[nodeIndex];
	const BSP_PLANE& plane = m_pPlanes[node.plane];

	// distance from point to a plane
	const float distance = plane.normal.x * g_DynLightBSphere.pointOrigin.x + 
		plane.normal.y * g_DynLightBSphere.pointOrigin.y + 
		plane.normal.z * g_DynLightBSphere.pointOrigin.z - plane.dist;

	// je potrebne parsovat obe strany roviny
	if (abs(distance) < g_DynLightBSphere.fRadius)
	{
		FindDynLightClustersRec (node.children[0]);
		FindDynLightClustersRec (node.children[1]);
		return;
	}

	if (distance >= 0)
		FindDynLightClustersRec (node.children[0]);
	else
		FindDynLightClustersRec (node.children[1]);
}

//---------------------------------
bool CP3DBSPMap::FindDynLightClusters (P3DSphere &lightBS, CBitset &lit_faces)
{
	lit_faces.ClearAll ();
	g_intSrcCluster = FindCluster (lightBS.pointOrigin);
	if (g_intSrcCluster < 0) return false;

	g_DynLightBSphere = lightBS;
	g_pLitFaces = &lit_faces;

	FindDynLightClustersRec (0);
	return true;
}

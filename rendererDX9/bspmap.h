//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Bsp map (modified Quake III *.BSP) class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dbspmap.h"
#include "ip3dtexture.h"
#include "ip3dfrustum.h"
#include "ip3dprimitive.h"
#include "ip3dphysengine.h"
#include "IP3DDraw2D.h"
#include "common.h"
#include <d3dx9math.h>


struct BSP_VERTEX_USED
{
	float pos[3]; // The untransformed position for the vertex
	float normal[3]; // Vertex normal
	float deluxel[3]; // Vertex deluxel
	float tc1[2]; // First texture coordinates
	float tc2[2]; // Second texture coordinates
	float tan[3]; // tangent
	float bitan[3]; // bitangent
	DWORD color; // vert. color
};

#define BSP_HEADER_VERSION	47
#define LIGHTMAP_SIZE		1024
#define MAX_BSPPATH			256
// parsing entit
#define MAX_LINE_LNG 500 // max. delka radku
#define MAX_ENTS_PARAMS 100 // max. pocet parametru entity

// ***************************************************************************
// -------------------------- BSP STRUCTS ------------------------------------
struct BSP_ENTRY // info o lumpech v hlavièce
{
	int offset;
	int length;
};

struct BSP_HEADER // hlavièka
{
	char idNumber[4];
	int version;
	BSP_ENTRY entries[17]; // pot lumpù
};

enum BSP_LUMP // lumpy
{
	lEntities,
	lTextures, // materials
	lPlanes,
	lNodes,
	lLeafs,
	lLeaffaces,
	lLeafbrushes,
	lModels,
	lBrushes,
	lBrushSides,
	lVertcies,
	lIndices,
	lEffects, // fogs
	lFaces, // surfaces
	lLightmaps,
	lLightgrid,
	lVisdata //visiblity
};

struct BSP_ENTITY
{
	char *ents;	
};

struct BSP_TEXTURE //textura / material
{
	char name[MAX_BSPPATH]; // nazev  textury/materialu
	int flags; // surf. flags
	int contents; // cont. flags
};

struct BSP_MATERIAL
{
	char diffuse[MAX_BSPPATH]; // path to diffuse texture with textures/
	char surface[16]; // surface name
};

struct BSP_PLANE
{
	P3DXVector3D normal; // float[3]
	float dist;
};

struct BSP_NODE
{
	int plane;
	int children[2]; // negative numbers are -(leafs+1), not nodes
	int mins[3]; // for frustom culling
	int maxs[3];
};

struct BSP_LEAF
{
	int cluster; // -1 = opaque cluster (do I still store these?)
	int area; // for frustum culling
	union {
		int	mins[3];
		float fmins[3];
	};
	union {
		int	maxs[3];
		float	fmaxs[3];
	};
	int leafface; // firstLeafSurface
	int n_leaffaces;
	int leafbrush;
	int n_leafbrushes;
};

typedef int BSP_LEAFFACES;

typedef int BSP_LEAFBRUSHES;

struct BSP_MODEL
{
	float mins[3];
	float maxs[3];
	int face;
	int n_faces;
	int brush;
	int n_brushes;
};

struct BSP_BRUSH
{
	int brushside;
	int n_brushsides;
	int texture; // the shader that determines the contents flags
};

struct BSP_BRUSHSIDES
{
	int plane; // positive plane side faces out of the leaf
	int texture;
};

struct BSP_VERTEX
{
	float position[3];
	float texcoord[2];
	float lightmap[2];
	float normal[3];
	unsigned char color[4];
	float deluxel[3];
};

typedef int BSP_INDEX;

struct BSP_EFFECT // FOG
{
	char name[MAX_BSPPATH]; // NO NAME BUT SHADER (TEXTURE) NAME
	int brush;
	int visible_side; 	// the brush side that ray tests need to clip against (-1 == none)
};

struct BSP_FACE
{
	int texture;
	int effect;
	int type;  // typ FACE_TYPE
	int vertex;
	int n_verts;
	int index;
	int n_indexes;
	int lm_index;
	int lm_start[2];
	int lm_size[2];
	float lm_origin[3];  // face origin ???
	float lm_vecs[2][3]; // MESS: "face bounding box" alebo "World space lightmap s and t unit vectors" ??? | KEXIK: je to tak? misto tohodle a radku pod tim ma Q3 pouze vec3_t lightmapVecs[3]; a vec3_t jsou 3 floaty!
	float normal[3]; // je to tak?
	int size[2]; // path size
};

enum FACE_TYPE {
	FACE_BAD,
	FACE_PLANAR,
	FACE_PATCH,
	FACE_TRIANGLE_SOUP,
	FACE_FLARE
};

struct BSP_LIGHTMAP
{
    unsigned char imageBits[LIGHTMAP_SIZE][LIGHTMAP_SIZE][3];
};

struct BSP_VISDATA
{
	int n_vecs;
	int vecSize;
	unsigned char *vecs; 
};

struct BSP_LIGHTGRID
{
	unsigned char ambient[3];
	unsigned char directional[3];
	unsigned char direction[2];
};

// ***************************************************************************
// --------------------------- CLASS -----------------------------------------

class CP3DBSPMap : public IP3DBSPMap
{
public:
	CP3DBSPMap();
	~CP3DBSPMap();
	bool Init();
	bool Load(const char* FileName);
	void UnLoad();
	void Loop();
	void Render();
	void CalculateIntersectClusters (P3DSphere &BSphere, BSPCLUSTER_LIST &ClusterList);
	bool GetLightFromPoint (IN P3DXVector3D &point, OUT BSP_LIGHTCELL_INFO &lightCell);
	bool ClusterVisible (int srcCluster, int destCluster);
	bool ClusterVisible (int testCluster);
	int FindCluster(P3DXVector3D &camPos);
	void CameraUpdate (P3DXVector3D &camPos);
	int GetCameraCluster();
	bool FindDynLightClusters (P3DSphere &lightBS, CBitset &lit_faces);
private:
	void Load7mt(char* sz7mtPath, int i); // sz7mtPath je cesta k 7mt, nastavi se dle indexu i BSP_MATERIAL
	 void SetShaderParameters(DWORD matID, DWORD texID, DWORD lmID, SHADER_DYNLIGHT_INPUT *pShaderDynlightsInput);
	void Build();
	void PostUnLoad();
	int FindLeaf (P3DXVector3D& camPos);
	void FindSphereColisionLeafs (int nodeIndex);
	void FindDynLightClustersRec (int nodeIndex);
	void ChangeGamma(byte *pImage, int size, float factor);
	void DrawLoadScr(int per, const char* desc);
	void ParseEntities();
	void ParseWorldspawnEntity(); // nutne pro ziskani dat z worldspawn pred ParseEntities()
	void ComputeTangents(BSP_VERTEX_USED *pVertices);
	// variables
	LPDIRECT3DVERTEXBUFFER9			m_pVB;
	LPDIRECT3DVERTEXDECLARATION9	m_pVertexDeclaration;
	LPDIRECT3DINDEXBUFFER9			m_pIB;
	LPDIRECT3DTEXTURE9				*m_pTextureMaps;
	LPDIRECT3DTEXTURE9				*m_pLightMaps;

	bool m_bLoaded;
	bool m_bDeluxelMaps;

	// poèty struktur
	int numPlanes;
	int numNodes;
	int numLeaves;
	int numLeafFaces;
	int numVerts;
	int numFaces;
	int numIndex;
	int numTextures;
	int numLights;
	int numLightGrid;
	int numModels;

	// world bounding box
	float	m_fWorldMin[3];
	float	m_fWorldMax[3];
	// lightgrid info
	float m_lightGridOrigin[3];
	float m_lightGridInverseSize[3];
	int	m_lightGridBounds[3];		// float ???
	float m_lightGridCellSize[3];
	// default materials
	unsigned long	m_dwDefLMmatID, m_dwDefVertexmatID;
	int	m_intCurCamCluster;

	BSP_PLANE *m_pPlanes;
	BSP_NODE *m_pNodes;
	BSP_LEAF *m_pLeaves;
	BSP_LEAFFACES *m_pLeafFaces;
	BSP_VISDATA m_sVisData;
	BSP_LIGHTMAP *m_pLightmap;
	BSP_TEXTURE *m_pTextures;
	BSP_MATERIAL *m_pMaterials; // pole materialu - ma stejne indexy jako m_pTextures
	DWORD *m_pMaterialID; // pole id materialu - ma stejne indexy jako m_pTextures, pokud je prvek 0, neobsahuje material, ale jen texturu
	BSP_VERTEX *m_pVerts;
	BSP_FACE *m_pFaces;
	BSP_INDEX *m_pIndices;
	BSP_ENTITY *m_pEnts;
	BSP_LIGHTGRID *m_pLightGrid;
	BSP_MODEL *m_pModels;

	CBitset m_DrawnFaces;

	// physics body
	IP3DPhysBody* m_pBody;

	// LOADING SCREEN
	IP3DTexture* m_pLoadTexture;
	IP3DTexture* m_pLoadBar;
	IP3DTexture* m_pLoadBarBg;
	IP3DFont* m_pLoadFnt;
	
	IP3DEntityMgr *m_pEntMgr;
};
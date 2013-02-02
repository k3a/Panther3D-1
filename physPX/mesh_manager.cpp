//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (15.9.2007)
// Purpose:	Cooking and mesh management class
//-----------------------------------------------------------------------------
#include "mesh_manager.h"
#include "common.h"
#include "physengine.h"
#include "NxCooking.h"

void CP3DPMeshManager::Initialize()
{

}

void CP3DPMeshManager::Shutdown()
{
	for (unsigned int i=0; i< m_arrMesh.size(); i++)
	{
		CMESH cm = m_arrMesh[i];
		UINT rc = cm.pMesh->getReferenceCount();
		if (rc>0 && g_pConsole) CON(MSG_CON_ERR, "Phys: Trying to release convex mesh with reference count = %d!", rc);
		g_pSDK->releaseConvexMesh(*cm.pMesh);
	}
	m_arrMesh.clear();
	for (unsigned int i=0; i< m_arrCylinder.size(); i++)
	{
		CMESH cm = m_arrMesh[i];
		UINT rc = cm.pMesh->getReferenceCount();
		if (rc>0 && g_pConsole) CON(MSG_CON_ERR, "Phys: Trying to release convex mesh with reference count = %d!", rc);
		g_pSDK->releaseConvexMesh(*cm.pMesh);
	}
	m_arrMesh.clear();
}

NxConvexMesh* CP3DPMeshManager::LoadPhysMesh(const char *pathIn)
{
	if (pathIn[0]==0) return NULL;

	// make correct path
	char path[MAX_PATH];
	if (!strnicmp(pathIn, "models", 6)&& (pathIn[6]=='/' || pathIn[6]=='\\'))
		strcpy(path, pathIn); // contains models/.... path
	else
	{
		strcpy(path, "models\\");
		strcat(path, pathIn);
	}

	// pokus se vyhledat jiz existujici cmesh
	for (unsigned int i=0; i< m_arrMesh.size(); i++)
	{
		CMESH cm = m_arrMesh[i];
		if (!stricmp(&path[7], cm.path)) return cm.pMesh;
	}

	// ------- READ NEW .PHYS CONVEX MESH
	FSFILE* fp = g_pFS->Open(path, "rb");
	if (!fp)
	{
		CON(MSG_CON_ERR, "Phys: Can't open %s file!", path);
		return NULL;
	}

	DWORD dwLen = fp->nLen;
	char head[8];
	bool bBaked=false;
	DWORD numInd = 0;
	DWORD numVerts = 0;
	UINT  *pIndices;
	FLOAT  *pVertices;

	g_pFS->Read(head, 7, 1, fp);
	head[7]=0;

	// check header
	if (!strncmp(head, "7PHYS", 5))
	{
		// check 7PHYS format version
		if (head[6]!='1')
		{
			CON(MSG_CON_ERR, "Phys: File %s is 7PHYS version %c and cannot be loaded!", path, head[6]);
			return NULL;
		}

		// is already cached/baked for PhysX?
		if (head[5]=='.')
		{
			CON(MSG_CON_INFO, "Phys: 7PHYS %s is not baked. Baking...", path);
			bBaked=false;
		}
		else if (head[5]=='p')
			bBaked=true;
		else
		{
			CON(MSG_CON_INFO, "Phys: 7PHYS %s is not baked for current physics engine. Baking...", path, head[6]);
			bBaked=false;
		}
	}
	else
	{
		CON(MSG_CON_ERR, "Phys: File %s is not valid 7PHYS mesh!", path);
		return NULL;
	}

	// read number of indices and vertices
	g_pFS->Read(&numInd, sizeof(DWORD), 1, fp);
	g_pFS->Read(&numVerts, sizeof(DWORD), 1, fp);

	// ------- BAKE OR LOAD BAKED
	MemoryWriteBuffer buf;

	if (!bBaked) // BAKE
	{
		// read indices
		pIndices = new UINT[numInd];
		pVertices = new FLOAT[numVerts*3];
		//g_pFS->Read(pIndices, sizeof(WORD)*numInd, 1, fp);

		// read verts
		//pVertices = (FLOAT *)malloc(sizeof(float)*numVerts*3);
		//pVertices = new FLOAT[numVerts*3];
		//g_pFS->Read(pVertices, sizeof(FLOAT)*numVerts*3, 1, fp);

		g_pFS->Read(pIndices, numInd*sizeof(UINT), 1, fp);

		for (DWORD i=0; i<numVerts; i++)
		{
			float *tmpFloat = &pVertices[i*3];
			g_pFS->Read(tmpFloat, sizeof(FLOAT)*3, 1, fp);
		}

		// vytvor popis trimeshe
		NxConvexMeshDesc meshDesc;
		meshDesc.numVertices                = numVerts;
		meshDesc.numTriangles               = numInd/3;
		meshDesc.pointStrideBytes           = sizeof(FLOAT)*3;
		meshDesc.triangleStrideBytes        = sizeof(UINT)*3;
		meshDesc.points						= pVertices;
		meshDesc.triangles                  = pIndices;                           
		meshDesc.flags                      = 0;

		if (!g_pCooking->NxCookConvexMesh(meshDesc, buf))
		{
			CON(MSG_CON_ERR, "Phys: Can't cook %s 7PHYS file!", path);
			return NULL;
		}

		// cooked, write new, baked 7PHYS data
		g_pFS->Close(fp); fp = NULL;
		FSFILE* fpOut = g_pFS->Open(path, "wb");
		if (!fpOut)
		{
			CON(MSG_CON_ERR, "Phys: Can't write baked data to %s 7PHYS file!", path);
			return NULL;
		}

		// write header
		g_pFS->Write("7PHYSp1", 7, 1, fpOut); // p = PhysX
		g_pFS->Write(&numInd, sizeof(DWORD), 1, fpOut); // zapis pocet indexu
		g_pFS->Write(&numVerts, sizeof(DWORD), 1, fpOut); // zapis pocet vertexu

		// write data
		g_pFS->Write(pIndices, sizeof(UINT)*numInd, 1, fpOut);
		for (DWORD i=0; i<numVerts; i++)
		{
			float *tmpFloat = &pVertices[i*3];
			g_pFS->Write(tmpFloat, sizeof(float)*3, 1, fpOut);
		}

		// write cooked data
		g_pFS->Write(buf.data, buf.currentSize, 1, fpOut);

		// close output file
		g_pFS->Close(fpOut);

		// clear memory
		delete[] pIndices;
		delete[] pVertices;
	}
	else // ALREADY BAKED FOR PhysX
	{
		// set new read pos
		g_pFS->Seek(fp, sizeof(DWORD)*numInd+sizeof(float)*numVerts*3, SEEK_CUR);

		BYTE *buff; DWORD len = fp->nLen-fp->nRelOffset;
		buff = new BYTE[len];
		g_pFS->Read(buff, 1, len, fp);
		buf.storeBuffer(buff, len);
		delete[] buff;
	}

	// ------- CREATE NEW CONVEX TRIMESH
	MemoryReadBuffer readBuffer(buf.data);
	NxConvexMesh *pTriMesh = g_pSDK->createConvexMesh(readBuffer);
	if (!pTriMesh) 
	{
		CON(MSG_CON_ERR, "Phys: Can't create triangle mesh from 7PHYS %s!", path);
		return NULL;
	}

	// push back new cylinder mesh
	CMESH cm;
	strcpy(cm.path, &path[7]);
	cm.pMesh = pTriMesh;
	m_arrMesh.push_back(cm);

	// ------- FREE MEMORY AND CLOSE FILE
	g_pFS->Close(fp);

	return pTriMesh;
}

NxConvexMesh* CP3DPMeshManager::LoadCylinder(float height, float radius)
{
	// pokus se vyhledat jiz existujici cmesh valce
	for (unsigned int i=0; i< m_arrCylinder.size(); i++)
	{
		CYLINDER cm = m_arrCylinder[i];
		if (cm.height == height && cm.radius == radius) return cm.pMesh;
	}

	// udelej trimesh cylinder
	P3DXVector3D pPoints[20];
	UINT pIndices[96]={0,2,1,0,3,18,0,4,3,0,5,4,0,6,5,0,7,6,0,8,7,0,1,8,1,10,9,1,2,10,18,11,19,18,3,11,3,12,11,3,4,12,4,13,12,4,5,13,5,14,13,5,6,14,6,15,14,6,7,15,7,16,15,7,8,16,8,9,16,8,1,9,17,9,10,17,19,11,17,11,12,17,12,13,17,13,14,17,14,15,17,15,16,17,16,9};

	pPoints[0].x=0.0f;                pPoints[0].y=-0.5f*height;  pPoints[0].z=0.0f;
	pPoints[1].x=radius;              pPoints[1].y=-0.5f*height;  pPoints[1].z=0.0f;
	pPoints[2].x=radius*0.707107f;    pPoints[2].y=-0.5f*height;  pPoints[2].z=radius*0.707107f;
	pPoints[3].x=0.0f;                pPoints[3].y=-0.5f*height;  pPoints[3].z=radius;
	pPoints[4].x=radius*-0.707107f;   pPoints[4].y=-0.5f*height;  pPoints[4].z=radius*0.707107f;
	pPoints[5].x=-radius;             pPoints[5].y=-0.5f*height;  pPoints[5].z=0.0f;
	pPoints[6].x=radius*-0.707107f;   pPoints[6].y=-0.5f*height;  pPoints[6].z=radius*-0.707107f;
	pPoints[7].x=0.0f;                pPoints[7].y=-0.5f*height;  pPoints[7].z=-radius;
	pPoints[8].x=radius*0.707107f;    pPoints[8].y=-0.5f*height;  pPoints[8].z=radius*-0.707107f;
	pPoints[9].x=radius;              pPoints[9].y=0.5f*height;   pPoints[9].z=0.0f;
	pPoints[10].x=radius*0.707107f;   pPoints[10].y=0.5f*height;  pPoints[10].z=radius*0.707107f;
	pPoints[11].x=0.0f;               pPoints[11].y=0.5f*height;  pPoints[11].z=radius;
	pPoints[12].x=radius*-0.707107f;  pPoints[12].y=0.5f*height;  pPoints[12].z=radius*0.707107f;
	pPoints[13].x=-radius;            pPoints[13].y=0.5f*height;  pPoints[13].z=0.0f;
	pPoints[14].x=radius*-0.707107f;  pPoints[14].y=0.5f*height;  pPoints[14].z=radius*-0.707107f;
	pPoints[15].x=0.0f;               pPoints[15].y=0.5f*height;  pPoints[15].z=-radius;
	pPoints[16].x=radius*0.707107f;   pPoints[16].y=0.5f*height;  pPoints[16].z=radius*-0.707107f;
	pPoints[17].x=0.0f;               pPoints[17].y=0.5f*height;  pPoints[17].z=0.0f;
	pPoints[18].x=radius*0.707107f;   pPoints[18].y=-0.5f*height; pPoints[18].z=radius*0.707107f;
	pPoints[19].x=radius*0.707107f;   pPoints[19].y=0.5f*height;  pPoints[19].z=radius*0.707107f;

	// vytvor popis trimeshe
	NxConvexMeshDesc meshDesc;
	meshDesc.numVertices                = 20;
	meshDesc.numTriangles               = 32;
	meshDesc.pointStrideBytes           = sizeof(P3DXVector3D);
	meshDesc.triangleStrideBytes        = sizeof(UINT)*3;
	meshDesc.points						= pPoints;
	meshDesc.triangles                  = pIndices;                           
	meshDesc.flags                      = 0;

	MemoryWriteBuffer buf;
	if (!g_pCooking->NxCookConvexMesh(meshDesc, buf))
	{
		CON(MSG_CON_ERR, "Phys: Can't cook triangle mesh for cylinder shape!");
		return NULL;
	}

	MemoryReadBuffer readBuffer(buf.data);
	NxConvexMesh *pTriMesh = g_pSDK->createConvexMesh(readBuffer);
	if (!pTriMesh) 
	{
		CON(MSG_CON_ERR, "Phys: Can't create triangle mesh for cylinder shape!");
		return NULL;
	}

	// push back new cylinder mesh
	CYLINDER cm;
	cm.height = height;
	cm.radius = radius;
	cm.pMesh = pTriMesh;
	m_arrCylinder.push_back(cm);

	return pTriMesh;
}

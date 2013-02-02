//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Declaration of common variables and functions
//-----------------------------------------------------------------------------
#pragma once

#define NOMINMAX
#define MEMORY_DEBUG 1

#include "NxPhysics.h"
#include "NxCooking.h"
#include "NxControllerManager.h"
#include "stream.h"
#include "mesh_manager.h"

#define PSAFE_RELEASE(obj) if (obj) {obj->release(); obj=NULL;}

extern NxPhysicsSDK* g_pSDK;
extern NxScene* g_pScene;
extern NxCookingInterface *g_pCooking;
extern NxControllerManager *g_pCtrlMgr;
extern CP3DPMeshManager g_meshManager;

// ---------  ENGINE 

#include "IP3DConsole.h"
#include "IP3DMaterialManager.h"
#include "IP3DFont.h"
#include "IP3DEngine.h"
#include "IP3DXML.h"
#include "IP3DRenderer.h"
#include "IP3DFileSystem.h"

extern IP3DConsole			*g_pConsole;
extern IP3DMaterialManager	*g_pMaterialManager;
extern IP3DFont				*g_pFont;	
extern IP3DEngine			*g_pEngine;
extern IP3DXML				*g_pXML;
extern IP3DRenderer			*g_pRenderer;
extern IP3DFileSystem		*g_pFS;

// ------- POMOCNE

extern NxMat34 tempMatrix;
extern P3DXMatrix tempMatrixP3D;
NxMat34& Mat2Nx(const P3DXMatrix &mat);
P3DXMatrix& Nx2Mat(const NxMat34 &mat);


class MyAllocator : public NxUserAllocator 
{
public:
	void * malloc(NxU32 size){return ::malloc(size);};
	void * mallocDEBUG(NxU32 size,const char *fileName, int line){return ::malloc(size);};
	void * realloc(void * memory, NxU32 size){return ::realloc(memory,size);};
	void free(void * memory){::free(memory);};
};

extern MyAllocator g_pAllocator;
extern float g_fTimeDelta;
inline float GetTimeDelta()
{
	return g_fTimeDelta;
}

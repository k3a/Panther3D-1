//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Definition of common variables and functions
//-----------------------------------------------------------------------------
#include "common.h"

NxPhysicsSDK* g_pSDK=NULL;
NxScene* g_pScene=NULL;
NxCookingInterface *g_pCooking=NULL;
NxControllerManager *g_pCtrlMgr=NULL;
CP3DPMeshManager g_meshManager;

// ------- ENGINE

IP3DConsole			*g_pConsole=NULL;
IP3DMaterialManager	*g_pMaterialManager=NULL;
IP3DFont			*g_pFont=NULL;	
IP3DEngine			*g_pEngine=NULL;
IP3DXML				*g_pXML=NULL;
IP3DRenderer		*g_pRenderer=NULL;
IP3DFileSystem		*g_pFS=NULL;

// ------- POMOCNE

NxMat34 tempMatrix;
P3DXMatrix tempMatrixP3D;
NxMat34& Mat2Nx(const P3DXMatrix &mat)
{
	tempMatrix.t = NxVec3(mat.m_posit3.x, mat.m_posit3.y, mat.m_posit3.z);
	tempMatrix.M.setColumn(0, NxVec3(mat.m_front3.x, mat.m_front3.y, mat.m_front3.z));
	tempMatrix.M.setColumn(1, NxVec3(mat.m_up3.x, mat.m_up3.y, mat.m_up3.z));
	tempMatrix.M.setColumn(2, NxVec3(mat.m_right3.x, mat.m_right3.y, mat.m_right3.z));
	/*tempMatrix.setRowMajor44((const NxF32*)mat.m16);*/
	return tempMatrix;
}
P3DXMatrix& Nx2Mat(const NxMat34 &mat)
{
	//tempMatrixP3D.m_posit3.x = mat.t.x; tempMatrixP3D.m_posit3.y = mat.t.y; tempMatrixP3D.m_posit3.z = mat.t.z;
	NxF32 rowMat[16];
	mat.getColumnMajor44(rowMat);
	tempMatrixP3D = P3DXMatrix(rowMat[0], rowMat[1], rowMat[2], rowMat[3], rowMat[4], rowMat[5], rowMat[6], rowMat[7], rowMat[8], rowMat[9], rowMat[10], rowMat[11], rowMat[12], rowMat[13], rowMat[14], rowMat[15]);
	return tempMatrixP3D;
}

MyAllocator g_pAllocator;
float g_fTimeDelta=1.0f;
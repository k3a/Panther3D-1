//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Bsp map interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "IP3DEntityMgr.h"
#include "interface.h"
#include "types.h"


class IP3DBSPMap : public IP3DBaseInterface
{
public:
	virtual bool Load(const char* FileName)=0;
	virtual void Loop ()=0;
	virtual void Render()=0;
	virtual void CalculateIntersectClusters (P3DSphere &BSphere, BSPCLUSTER_LIST &ClusterList)=0;
	virtual bool GetLightFromPoint (IN P3DXVector3D &point, OUT BSP_LIGHTCELL_INFO &lightCell)=0;
	virtual bool ClusterVisible (int srcCluster, int destCluster)=0;
	virtual bool ClusterVisible (int testCluster)=0;
	virtual int FindCluster(P3DXVector3D &camPos)=0;
	virtual void CameraUpdate (P3DXVector3D &camPos)=0;
	virtual int GetCameraCluster()=0;
	virtual bool FindDynLightClusters (P3DSphere &lightBS, CBitset &lit_faces)=0;
};

#define IP3DRENDERER_BSPMAP "P3DBSPMap_3" // nazev ifacu
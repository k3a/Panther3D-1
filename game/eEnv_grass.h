//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (13.10.2006)
// Purpose:	Transparent grass - entity
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eEnv_grass : public eBase
{
public:
	eEnv_grass();
	~eEnv_grass();
	bool Spawn(geometryData_s& gdata);
private:
	P3DVector3D m_vOrigin;
	P3DVector3D m_vRot;
	float m_fRadius;
	int m_nQuadCnt;
	float m_fDist;
	float m_fMinSize;
	float m_fMaxSize;
	char* m_strTexture;
	bool bAnimate;
	bool bBillboarded;
};
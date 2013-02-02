//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Dynamic light with automatization - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"

class eLight_dynamic : public eBase
{
public:
	eLight_dynamic();
	~eLight_dynamic();
	bool Spawn(geometryData_s& gdata);
	void Think(float deltaTime);
private:
	//sf
	bool m_bLightOff;
	bool m_bFlickering;
	bool m_bFlickerSmooth;
	//data
	char *m_pszTexture;
	P3DXVector3D m_vPos;
	P3DXVector3D m_vColor;
	float m_fRange;
	char* m_pszFlickerStr;
	float m_fCycleTime;
	//
	DWORD m_dwLight;
	// GDS HACK KEX FIXME
	float angl;
	int rot;
};
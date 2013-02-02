//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Sprite drawing - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"
#include "ip3ddraw2d.h"

class eEnv_sprite : public eBase
{
public:
	eEnv_sprite();
	~eEnv_sprite();
	bool Spawn(geometryData_s& gdata);
	void PreRender();
	void Render(float deltaTime);
	void PostRender();
	//
	void ColorA(eBase* pCaller, int nValue){m_nAlpha=nValue;}; // barvy
	void ColorR(eBase* pCaller, int nValue){m_vColor.x = (float)nValue;};
	void ColorG(eBase* pCaller, int nValue){m_vColor.z = (float)nValue;};
	void ColorB(eBase* pCaller, int nValue){m_vColor.y = (float)nValue;};
private:
	char *m_pszTexture;
	P3DVector3D m_vPos;
	P3DVector3D m_vCenter;
	P3DVector3D m_vColor;
	P3DVector3D m_vScale;
	int m_nAlpha;
	//
	IP3DTexture *m_pTexture;
	P3DXMatrix m_mTmp;
};
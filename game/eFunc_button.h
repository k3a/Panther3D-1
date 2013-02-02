//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Button - entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "eBase.h"
#include "common.h"
#include "HUD.h"

class eFunc_button : public eBase
{
public:
	eFunc_button();
	~eFunc_button();
	bool Spawn(geometryData_s& gdata);
	void Think(float deltaTime);
	void Render(float deltaTime);
	void Finalize();
private:
	eBase* pPlayer;
	P3DXVector3D m_vPos;
	P3DXVector3D m_vPlayerPos;
	P3DXVector3D *m_vPlayerLookat;
	EntEvent OnPress;
	char* m_szMessage;
	float m_fWait;
	float m_fTime;
	float m_fRadius;
	bool bPressed;
	bool bDrawUseButton;
	// -vnitrni promenne
	unsigned int m_sndOn;
	unsigned int m_sndOff;
};
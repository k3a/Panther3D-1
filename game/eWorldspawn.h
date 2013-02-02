//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	World spawn (skybox, ambient sound, ...) entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "eBase.h"
#include "ip3dfont.h"
#include "hcountdowntimer.h"
#include "ip3dsky.h"
#include "IP3DRenderer.h"

class eWorldspawn : public eBase
{
public:
	eWorldspawn();
	~eWorldspawn();
	bool Spawn(geometryData_s& gdata);
	//void Think(float deltaTime);
	void Render(float deltaTime);
private:
	// --- ÚVODNÍ TEXT ---
	char *m_pSText;
	char* m_pszMusic;
	IP3DFont *m_pFont;
	CCountdownTimer m_Tim;
	// --- SKYBOX ---
	IP3DSky *m_pSky;
	IP3DRenderer *m_pRenderer;
	char *m_pszSkyBox;
	float m_fSkyAngle;
	//
	bool m_bReset;
	//
	bool m_bSoundPlayed;
};
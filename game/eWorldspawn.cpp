//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	World spawn (skybox, ambient sound, start text, ...) entity class
//-----------------------------------------------------------------------------
#include "eWorldspawn.h"

REGISTER_ENTITY(eWorldspawn, "worldspawn",false);

eWorldspawn::eWorldspawn()
{
	ENT_DATA("text", m_pSText, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edText");
	ENT_DATA("music", m_pszMusic, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edMusic");
	ENT_DATA("SkyBox", m_pszSkyBox, TYPE_STRING, NULL, E_KEY_DONT_SAVE, "#entities_edSkybox");
	ENT_DATA("sky_angle", m_fSkyAngle, TYPE_FLOAT, 0.0f, E_KEY_DONT_SAVE, "#entities_edAngle");

	m_bSoundPlayed = false;

	P3DAABB aabb(0, 0, 0, 0, 0, 0);
	SetRelAABB(aabb);

	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
}

eWorldspawn::~eWorldspawn()
{
	// --- ÚVODNÍ TEXT ---
	SAFE_DELETE(m_pFont);
	SAFE_DELETE_ARRAY(m_pSText);

	// --- HUDBA NA POZADÍ ---
	g_pSoundMan->UnloadSound("worldspawn_snd");
	SAFE_DELETE_ARRAY(m_pszMusic);

	// --- SKY ---
	SAFE_DELETE_ARRAY(m_pszSkyBox);
}

bool eWorldspawn::Spawn(geometryData_s& gdata)
{
	// --- SKY ---
	m_pSky = (IP3DSky*)I_GetClass(IP3DRENDERER_SKY);
	// --- RENDERER ---
	m_pRenderer = (IP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);

	// --- ÚVODNÍ TEXT ---
	m_pFont = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	m_pFont->CreateFont("Arial", 32, false, false);
	m_Tim.CreateTimer(5000);
	m_bReset = false;

	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	CVr_width = g_pConsole->FindConVar("r_width");
	CVr_height = g_pConsole->FindConVar("r_height");


	// --- SKY ---
	if(m_pszSkyBox)
	{
		char path[MAX_PATH] = "skyboxes\\";
		strcat(path, m_pszSkyBox);
		m_pSky->CreateSkybox(path, NULL);		// FIXME: NULL - doplnit spravnu cloud texturu
	}
	else
		m_pSky->CreateSkybox("skyboxes\\default.dds", "skyboxes\\clouds.dds", m_fSkyAngle);

	// --- HUDBA NA POZADÍ ---

	if (m_pszMusic)
		g_pSoundMan->LoadSound(m_pszMusic);

	return true;
}

void eWorldspawn::Render(float deltaTime)
{
	if (!m_bSoundPlayed)
	{
		g_pSoundMan->PlaySound (m_pszMusic, true);
		m_bSoundPlayed = true;
	}
	// necháme v render, není tak nároèné na výpoèty...

	if (!m_bReset) // ...aby se zacalo odpocitavat az po prvnim renderu (prechod z menu treba)
	{
		m_Tim.EnableTimer(0, true);
		m_Tim.ResetTimer(0);
		m_bReset = true;
	}

	// --- ÚVODNÍ TEXT ---
	if (m_Tim.GetTimeRemaining(0) == 0 || !m_pSText) 
	{
		SAFE_DELETE_ARRAY(m_pSText);
		m_Tim.EnableTimer(0, false);
	}
	else
	{
		unsigned int col = (unsigned int)(255.0f / 4000.0f * m_Tim.GetTimeRemaining(0)); // 2000 je zde schválnì
		if (col > 255) col = 255;
		if (col < 0) col = 0;

		// vytvor rect pro vypis takoveho textu
		P3DRect2D rect;
		rect.x1 = CVr_width->GetInt()/6;
		rect.x2 = CVr_width->GetInt()-CVr_width->GetInt()/6;
		rect.y1 = 0;
		rect.y2 = 500;
		m_pFont->GetTextSize(m_pSText, &rect, NULL, NULL);

		//ziskej vysku a sirku textu a zarovnej
		int h = rect.y2-rect.y1;
		int w = rect.x2-rect.x1;
		rect.y1 = CVr_height->GetInt()/2-h/2;
		rect.y2 = rect.y1+h;
		rect.x1 = CVr_width->GetInt()/2-w/2;
		rect.x2 = rect.x1+w;

		// vykresli text
		m_pFont->DrawText(FA_CENTER, m_pSText, rect, P3DCOLOR_ARGB(col, 0, 0, 0)); // cerne pozadi
		rect.x1 -=1; rect.y1 -=1; // posun pro 3D efekt
		m_pFont->DrawText(FA_CENTER, m_pSText, rect, P3DCOLOR_ARGB(col, 255, 255, 255));
	}
}
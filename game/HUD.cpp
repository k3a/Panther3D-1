//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose: Game HUD
//-----------------------------------------------------------------------------
#include "HUD.h"

CHUD g_HUD;

CHUD::CHUD()
{
	bLoaded = false;
	hudFnt = NULL;
}

void CHUD::Shutdown()
{
	SAFE_DELETE(m_pTexUse);

	// uvolni font
	//if(!RemoveFontResource("data/font.ttf"))
	//	CON(MSG_CON_ERR, "Can't remove hud font!");
	SAFE_DELETE(hudFnt);
}

bool CHUD::Init()
{
	// pokud jiz hudFnt existuje, byla jiz Init() volana a tak nic...
	if (hudFnt) return true;

	bLoaded = false;

	/// priprav font pro HUD
	//if(!AddFontResource("data/font.ttf"))
	//	CON(MSG_CON_ERR, "Can't load hud font!");

	hudFnt = (IP3DFont*)I_GetClass(IP3DRENDERER_FONT);
	hudFnt->CreateFont("Arial", 20, false, false);
	///
	m_pTexUse = (IP3DTexture*)I_GetClass(IP3DRENDERER_TEXTURE);

	char tmpPath[MAX_PATH];
	strcpy(tmpPath, "hud/use.dds");
	if(!m_pTexUse->Load(tmpPath, TT_2D)) return false; // nepodarilo se vytvorit texturu
	///

	g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);
	if (!g_p2D) return false;

	bLoaded = true;
	return true;
}

void CHUD::Render()
{
	// ZAMEROVAC
	int wid = hudFnt->DrawText(FA_CALCRECT, "+", 0, 0, 0, 0, P3DCOLOR_ARGB(255, 255, 200, 200));
	int hei = hudFnt->GetLineH();

	hudFnt->DrawText(FA_NOCLIP, "+", CVr_width->GetInt()/2-wid/2, CVr_height->GetInt()/2-hei/2, 0, 0, P3DCOLOR_ARGB(255, 200, 200, 70));

	// pomocne promenne, priprava na vykreslovani spritu
	P3DXVector3D pos; P3DXMatrix mat;
	mat.SetIdentityMatrix();
	g_p2D->Sprite_Begin(SPRITE_ALPHABLEND);
	g_p2D->Sprite_SetTransform(mat);

	if (bDrawTexUse)
	{
		// USE
		pos.x = CVr_width->GetFloat()/2-32.0f;
		pos.y = CVr_height->GetFloat()/1.5f;
		pos.z = 0;
		g_p2D->Sprite_Draw(m_pTexUse, NULL, NULL, &pos, P3DCOLOR_XRGB(255, 255, 255));
	}

	// konec vykreslovani spritu
	g_p2D->Sprite_End();

	// resetovat
	bDrawTexUse = false;
}

void CHUD::DrawUseText(const char* str)
{
	hudFnt->DrawText(FA_CENTER, str, 0, (int)(CVr_height->GetFloat()/1.5f+64.0f+3.0f), CVr_width->GetInt(), CVr_height->GetInt(), P3DCOLOR_XRGB(0, 143, 206));
}
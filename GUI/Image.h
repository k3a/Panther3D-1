//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Image (with border)
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

class CImage : public IGuiElement
{
public:
	CImage();
	void RenderLine(float deltaTime);
	void RenderSprite(float deltaTime);
	bool CreateImage(int x, int y, int w, int h, bool bDrawBorder, IP3DTexture* tex, IGuiElement* pParent);
	IP3DTexture* GetTexture()const{m_pTexture;};
	void SetTexture(IP3DTexture* tex){m_pTexture=tex;};
	void Update(float deltaTime);
private:
	bool m_bDrawBorder;
	IP3DTexture* m_pTexture;
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};
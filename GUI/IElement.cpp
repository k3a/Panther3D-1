//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	GUI Element (Component)
//-----------------------------------------------------------------------------
#include "IElement.h"
#include "common.h"

void IGuiElement::RenderChildren(float deltaTime, renderType_e type)
{
	Prof(GUI_IGuiElement__RenderChildren);
	if (m_bHidden) return; // okno neni viditelne, ale nebyla smazana instance
	for(unsigned int i=0; i < m_nChilds; i++)
	{
		// nastav pozice
		m_pChilds[i]->SetParentPosition(this->m_nPosX+this->m_nParentX, this->m_nPosY+this->m_nParentY);

		if(m_pChilds[i]->m_bHidden) continue; // pokud je skryt

		// vykresli CUSTOM
		if (type==RT_CUSTOM && m_pChilds[i]->GetElementFlags() & EF_CUSTOM) m_pChilds[i]->RenderCustom(deltaTime);

		// vykresli LINE
		//g_p2D->Line_Begin();
		if (type==RT_LINE && m_pChilds[i]->GetElementFlags() & EF_LINE) m_pChilds[i]->RenderLine(deltaTime);
		//g_p2D->Line_End();

		// vykresli SPRITE
		//g_p2D->Sprite_Begin(SPRITE_ALPHABLEND);
		if (type==RT_SPRITE && m_pChilds[i]->GetElementFlags() & EF_SPRITE) m_pChilds[i]->RenderSprite(deltaTime);
		//g_p2D->Sprite_End();

		//vykresli hierarchii
		m_pChilds[i]->RenderChildren(deltaTime, type);
	}
}

void IGuiElement::UpdateChildren(float deltaTime)
{
	Prof(GUI_IGuiElement__UpdateChildren);
	if (m_bHidden) return; // okno neni viditelne, ale nebyla smazana instance
	if (!m_pParent && !m_bHasFocus) return; // je to okno a neni aktivni - nema focus

	for(unsigned int i=0; i < m_nChilds; i++)
	{
		if(m_pChilds[i]->m_bHidden) continue; // pokud je skryt

		// aktualizuj dite
		m_pChilds[i]->Update(deltaTime);

		// aktualizuj jeho hierarchii
		m_pChilds[i]->UpdateChildren(deltaTime);
	}
}
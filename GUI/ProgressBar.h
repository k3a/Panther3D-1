//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	ProgressBar |====------|
//-----------------------------------------------------------------------------
#pragma once

#include "common.h"
#include "IElement.h"

class CProgressBar : public IGuiElement
{
public:
	CProgressBar();
	void RenderLine(float deltaTime);
	bool CreateProgressBar(int x, int y, int w, int h, bool drawOnlyPartitions, IGuiElement* pParent);
	float GetValue()const{return m_fValue;};
	void SetValue(float fVal){
		if(fVal<m_fValueMin) fVal=m_fValueMin;
		if(fVal>m_fValueMax) fVal=m_fValueMax;
		m_fValue = fVal;
		m_bChanged = true;
	}; // hodnota bude upravena dle min a max!
	void SetMinMax(float fMin, float fMax){
		m_fValueMin=fMin;
		m_fValueMax=fMax;
		if(m_fValue<m_fValueMin) m_fValue=m_fValueMin;
		if(m_fValue>m_fValueMax) m_fValue=m_fValueMax;
		m_bChanged = true;
	}; // hodnota bude upravena dle zadane min a max!
	void Update(float deltaTime);
	bool OnChange()const{return m_bChanged;};
private:
	float m_fValue;
	float m_fValueMin;
	float m_fValueMax;
	bool m_bChanged;
	//
	bool m_bDrawOnlyPartitions;
	float m_fPartSize; // sirka jedne casti, pokud je to progressbar s castmi
	int m_nMaxParts; // maximalni pocet casti - do celeho progressbaru
	// GUI EDITOR
	int edit_nRelPosX;
	int edit_nRelPosY;
	int edit_mode; // 0-nic, 1-posouvani, 2-zmena valikosti
};
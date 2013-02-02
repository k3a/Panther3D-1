//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2006)
// Purpose:	Particle graph modal dialog for Particle Editor
//-----------------------------------------------------------------------------
#include "common.h"

#define GRAPH_NULL_Y 113.0f
#define GRAPH_MAX_X 381.0f // sirka
#define GRAPH_MAX_Y 225.0f // vyska

#define GRAPH_SLD_MAX 100.0f // maximalni pocet posunu ve sliderech
#define GRAPH_MAX_SLD_SIZE 1000.0f // maximalni sirka a vyska celeho grafu vcetne posunu se slidery

class CP3DGUI_ParticleGraph : public CWindow
{
public:
	CP3DGUI_ParticleGraph();
	void Render();
	void Update();
private:
	void DrawPoint(P3DVector2D *v);
	void DrawHorizontalLine(P3DVector2D *v);

	CImage m_imgGraph;
	
	CButton m_btnOK;
	CButton m_btnCancel;
	
	CTextBox m_txtSinAmp; // amplituda
	CTextBox m_txtSinPer; // perioda - za kolik x se znova udela sin
	CButton m_btnSin;
	CTextBox m_txtRandAmp; // amplituda
	CCheckBox m_chkNotNull; // nenulove hodnoty
	CButton m_btnRand;
	
	CLabel m_lblSldX;
	CScrollBar m_scrX; // posun po ose x
	CLabel m_lblSldY;
	CScrollBarVert m_scrY; // posun po ose y
	
	CButton m_btnScaleXPlus;
	CButton m_btnScaleXMinus;
	CButton m_btnScaleYPlus;
	CButton m_btnScaleYMinus;
	CButton m_btnPosXPlus;
	CButton m_btnPosXMinus;
	CButton m_btnPosYPlus;
	CButton m_btnPosYMinus;
	
	int data[(int)GRAPH_MAX_SLD_SIZE];
	int posledniIndex; // aby bylo mozne delat rychle pohyby pri nastavovani
};
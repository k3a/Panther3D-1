//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2006)
// Purpose:	Particle graph modal dialog for Particle Editor
//-----------------------------------------------------------------------------
#include "particlegraph.h"

//////////////////////////// FIXME: POZOR!!!!!!!!!!!!!!!! ///////////////////
////////////////////////////////////////////////////////////////////////////

// ZMENA VERTIKALNIHO SCROLLBORU, MIN. HODNOTA JE NAHORE!!

//////////////////////////// FIXME: POZOR!!!!!!!!!!!!!!!! ///////////////////
////////////////////////////////////////////////////////////////////////////


CP3DGUI_ParticleGraph::CP3DGUI_ParticleGraph()
{
	CreateWindow("Lifetime t(s) / t(s) : Unnamed Emiter", 
		0, 0, 500, 300, P3DWS_CAPTION | P3DWS_CLOSEBTN);
	CenterScreen();
	
	// Initializace elementu
	INIT_ELEMENT(m_imgGraph);
	INIT_ELEMENT(m_btnOK);
	INIT_ELEMENT(m_btnCancel);
	INIT_ELEMENT(m_txtSinAmp);
	INIT_ELEMENT(m_txtSinPer);
	INIT_ELEMENT(m_btnSin);
	INIT_ELEMENT(m_txtRandAmp);
	INIT_ELEMENT(m_chkNotNull);
	INIT_ELEMENT(m_btnRand);
	INIT_ELEMENT(m_lblSldX);
	INIT_ELEMENT(m_scrX);
	INIT_ELEMENT(m_lblSldY);
	INIT_ELEMENT(m_scrY);
	INIT_ELEMENT(m_btnScaleXPlus);
	INIT_ELEMENT(m_btnScaleXMinus);
	INIT_ELEMENT(m_btnScaleYPlus);
	INIT_ELEMENT(m_btnScaleYMinus);
	INIT_ELEMENT(m_btnPosXPlus);
	INIT_ELEMENT(m_btnPosXMinus);
	INIT_ELEMENT(m_btnPosYPlus);
	INIT_ELEMENT(m_btnPosYMinus);

	m_imgGraph.CreateImage(10, 10, (int)GRAPH_MAX_X+1, (int)GRAPH_MAX_Y+1, true, NULL, this);
	//
	m_btnOK.CreateButton(g_pLocaliz->Translate_const("#GUI_Ok"), 405, 10, 84, 29, this);
	m_btnCancel.CreateButton(g_pLocaliz->Translate_const("#GUI_Cancel"), 405, 52, 84, 29, this);
	//
	m_txtSinAmp.CreateTextBox("10", 405, 107, 39, 24, this);
	m_txtSinPer.CreateTextBox("10", 452, 107, 39, 24, this);
	m_btnSin.CreateButton(g_pLocaliz->Translate_const("#GUI_PEditSin"), 405, 137, 84, 29, this);
	
	m_txtRandAmp.CreateTextBox("10", 405, 176, 53, 24, this);
	m_chkNotNull.CreateCheckBox("", 470, 182, 24, 24, false, this);
	m_btnRand.CreateButton(g_pLocaliz->Translate_const("#GUI_PEditRand"), 405, 206, 84, 29, this);
	//
	m_lblSldX.CreateLabel(g_pLocaliz->Translate_const("#GUI_PEditOffsetX"), 10, 242, 183, 22, false, FA_NOCLIP, this);
	m_scrX.CreateScrollBar(10, 236, 382, 15, this);
	m_scrX.SetMinMax(0.0f, GRAPH_SLD_MAX);
	m_lblSldY.CreateLabel(g_pLocaliz->Translate_const("#GUI_PEditOffsetY"), 206, 242, 183, 22, false, FA_NOCLIP, this);
	m_scrY.CreateScrollBar(392, 10, 15, 226, this);
	m_scrY.SetMinMax(0.0f, GRAPH_SLD_MAX);
	m_scrY.SetValue(GRAPH_SLD_MAX/2.0f);
	//
	m_btnScaleXPlus.CreateButton("S+", 405, 243, 21, 22, this);
	m_btnScaleXMinus.CreateButton("S-", 405, 266, 21, 22, this);
	m_btnScaleYPlus.CreateButton("s+", 426, 243, 21, 22, this);
	m_btnScaleYMinus.CreateButton("s-", 426, 266, 21, 22, this);
	m_btnPosXPlus.CreateButton("P+", 447, 243, 21, 22, this);
	m_btnPosXMinus.CreateButton("P-", 447, 266, 21, 22, this);
	m_btnPosYPlus.CreateButton("p+", 468, 243, 21, 22, this);
	m_btnPosYMinus.CreateButton("p-", 468, 266, 21, 22, this);
	
	
	// FIXME:
	// testovaci data
	int i;
	for(i=0;i<GRAPH_MAX_SLD_SIZE;i++)
	{
		data[i] = (int)(sin((double)i/10.0f)*30.0f);
	}
	
	posledniIndex=-1;
}

// dle posuvniku X a Y vykresli dany bod, je-li v tomto rozsahu
void CP3DGUI_ParticleGraph::DrawPoint(P3DVector2D *v) // vstup musi byt pole o dvou indexech, ale jen prvni index musi byt nastaven
{
	// Omezeni pohledu na ose Y
	if (v[0].y < (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue()) || v[0].y > (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue())+GRAPH_MAX_Y-1.0f)
		return; // pokud je mimo aktualni rozsah dle slideru
	
	// Omezeni pohledu na ose X
	if (v[0].x < (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_X)/GRAPH_SLD_MAX*m_scrX.GetValue() || v[0].x > (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_X)/GRAPH_SLD_MAX*m_scrX.GetValue()+GRAPH_MAX_X-1.0f)
		return; // pokud je mimo aktualni rozsah dle slideru

	v[0].x = (v[0].x - (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_X)/GRAPH_SLD_MAX*m_scrX.GetValue())+m_imgGraph.m_nParentX+m_imgGraph.m_nPosX+1.0f;
	v[0].y = (v[0].y - (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue()))+m_imgGraph.m_nParentY+m_imgGraph.m_nPosY+1.0f;
	v[1].x = v[0].x;
	v[1].y = v[0].y+1.0f;
	
	g_p2D->Line_Draw(v, 2, P3DCOLOR_XRGB(75, 75, 75));
}

// dle posuvniku Y vykresli rotizontalni primku, je-li v tomto rozsahu
void CP3DGUI_ParticleGraph::DrawHorizontalLine(P3DVector2D *v) // vstup musi byt pole o dvou indexech, staci nastavit [0].y
{
	// Omezeni pohledu na ose Y
	if (v[0].y < (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue()) || v[0].y > (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue())+GRAPH_MAX_Y-1.0f)
		return; // pokud je mimo aktualni rozsah dle slideru

	v[0].y = (v[0].y - (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue()))+m_imgGraph.m_nParentY+m_imgGraph.m_nPosY+1.0f;
	v[1].y = v[0].y;

	v[0].x = (float)m_imgGraph.m_nParentX+m_imgGraph.m_nPosX+1; v[1].x = (float)m_imgGraph.m_nParentX+m_imgGraph.m_nPosX+m_imgGraph.m_nWidth-1; // ukonci v levem a pravem rohu grafu
	g_p2D->Line_Draw(v, 2, P3DCOLOR_XRGB(110, 110, 110));
}

void CP3DGUI_ParticleGraph::Render()
{
	g_p2D->Line_Begin();
	//////////////////////////////
	
	P3DVector2D v[2];
	
	// vykresli stred grafu - osu x
	v[0].y = GRAPH_MAX_SLD_SIZE/2.0f;
	v[1].y = GRAPH_MAX_SLD_SIZE/2.0f;
	DrawHorizontalLine(v);
	
	// vykresli body grafu
	for(int i=0;i<GRAPH_MAX_SLD_SIZE;i++)
	{	
		v[0].x = (float)i;
		v[0].y = GRAPH_MAX_SLD_SIZE/2.0f-data[i]; // GRAPH_MAX_SLD_SIZE/2.0f protoze 0 je uprostred celeho grafu
		
		DrawPoint(v);
	}
	
	///////////////////////////////
	g_p2D->Line_End();
}

void CP3DGUI_ParticleGraph::Update()
{
	if (g_bLMouseUpFirstTime) posledniIndex=-1;

	// pridavani bodu
	if(g_bLMouseDown && g_nMouseX > m_imgGraph.m_nParentX+m_imgGraph.m_nPosX && g_nMouseX < m_imgGraph.m_nParentX+m_imgGraph.m_nPosX+m_imgGraph.m_nWidth
	&& g_nMouseY > m_imgGraph.m_nParentY+m_imgGraph.m_nPosY && g_nMouseY < m_imgGraph.m_nParentY+m_imgGraph.m_nPosY+m_imgGraph.m_nHeight)
	{
		// zjisti x dle slideru
		int poziceVGrafuZleva = g_nMouseX - m_imgGraph.m_nParentX - m_imgGraph.m_nPosX-1;
		float selIndex = poziceVGrafuZleva + (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_X)/GRAPH_SLD_MAX*m_scrX.GetValue();
			
		// zjisti y dle slideru
		int poziceVGrafuZhora = g_nMouseY - m_imgGraph.m_nParentY - m_imgGraph.m_nPosY-1;
		float hodnota;
		hodnota = GRAPH_MAX_SLD_SIZE-(poziceVGrafuZhora + (GRAPH_MAX_SLD_SIZE-GRAPH_MAX_Y)/GRAPH_SLD_MAX*(GRAPH_SLD_MAX-m_scrY.GetValue()))-GRAPH_MAX_SLD_SIZE/2.0f;

		// pokud je posledni index a nejaka mazera, dopln mezeru - pro rychle pohyby v grafu
		if (posledniIndex>=0 && abs(posledniIndex-(int)selIndex)>1)
		{
			if (posledniIndex < (int)selIndex) // je-li posledni vlevo
			{
				for(int q=posledniIndex+1;q<(int)selIndex;q++)
				{
					data[q] = (int)(data[posledniIndex]+(hodnota-data[posledniIndex])/abs(posledniIndex-(int)selIndex)*(q-posledniIndex));
				}
			}
			else  // je-li posledni vpravo
			{
				for(int q=(int)selIndex+1;q<posledniIndex;q++)
				{
					data[q] = (int)(data[posledniIndex]+(data[posledniIndex]-hodnota)/abs(posledniIndex-(int)selIndex)*(q-posledniIndex));
				}
			}
		}
		
		data[(int)selIndex] = (int)hodnota;
		
		posledniIndex = (int)selIndex;
	}
}
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Window
//-----------------------------------------------------------------------------
#include "Window.h"

// GUI EDITOR - Ukladani ci otvirani nastaveni okna
static bool bOpening=false;
#include "SelectFile.h"

CWindow::CWindow()
{
	m_nWidth=100;
	m_nHeight=75;
	m_nPosX=10;
	m_nPosY=10;

	relX=-1;
	relY=0;
	
	m_nHeaderHeight = TITLE_HEIGHT;

	m_szCaption = new char[11];
	strcpy(m_szCaption, "P3D Window");

	m_bXDown = false;
	m_bXDownFirstTime = false;
	m_bHidden = true;
	m_bModal = false;

	SetElementFlags(EF_LINE | EF_SPRITE); // kreslime line + text
}

bool CWindow::CreateWindow(const char* szCaption, int x, int y, int w, int h, int nStyle)
{
	// nastav okno
	if(szCaption)
	{
		if(m_szCaption) SAFE_DELETE_ARRAY(m_szCaption); 
		m_szCaption = new char[strlen(szCaption)+1];
		strcpy(m_szCaption, szCaption);
	}
	m_nPosX = x;
	m_nPosY = y;
	m_nWidth = w;
	m_nHeight = h;
	m_nStyle = nStyle;

	//registruj okno
	g_pWindows.push_back(this);

	if (nStyle & P3DWS_HIDDEN)
		m_bHidden = true;
	else
		m_bHidden = false;

	return true;
}

void CWindow::RenderLine(float deltaTime)
{
	if(m_bHidden) return; // bylo ukonceno, ale neuvolneno

	// vykresli pozadi titulku
	bool bIsForeground=false;
	IGuiElement* pForegWin = GetFirstVisibleWindow();
	if (pForegWin && pForegWin == this) bIsForeground = true;
	
	// vykresli titulek okna
	if(m_nStyle & P3DWS_CAPTION) g_p2D->Line_DrawFilledRect(m_nPosX, m_nPosY-TITLE_HEIGHT, m_nWidth, TITLE_HEIGHT, bIsForeground ? g_Scheme.GetColor(COL_TITLE_ACTIVE) : g_Scheme.GetColor(COL_TITLE));

	// vykresli pozadi okna
	g_p2D->Line_DrawFilledRect(m_nPosX, m_nPosY, m_nWidth, m_nHeight, g_Scheme.GetColor(COL_BG));

	int posY=m_nPosY; int hei=m_nHeight;
	if (m_nStyle & P3DWS_CAPTION) { posY-=TITLE_HEIGHT; hei+=TITLE_HEIGHT; }

	// vykresli okraje okna
	g_p2D->Line_Draw3DRect(m_nPosX, posY, m_nWidth, hei, g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_SHADOW));

	// vykresli tlacitka okna
	// tlacitko X
	if (m_nStyle & P3DWS_CLOSEBTN)
	{
		if (m_bXDown)
			g_p2D->Line_Draw3DRect((int)((float)m_nPosX+m_nWidth-3-(TITLE_HEIGHT/1.4f)), (int)((float)m_nPosY-(TITLE_HEIGHT/1.4f)-3), (int)(TITLE_HEIGHT/1.4f), (int)(TITLE_HEIGHT/1.4f), g_Scheme.GetColor(COL_SHADOW), g_Scheme.GetColor(COL_LIGHT));
		else
			g_p2D->Line_Draw3DRect((int)((float)m_nPosX+m_nWidth-3-(TITLE_HEIGHT/1.4f)), (int)((float)m_nPosY-(TITLE_HEIGHT/1.4f)-3), (int)(TITLE_HEIGHT/1.4f), (int)(TITLE_HEIGHT/1.4f), g_Scheme.GetColor(COL_LIGHT), g_Scheme.GetColor(COL_SHADOW));
	}
}

// format souboru nastaveni pozic porvku:
// 7GUI_1
// Nazev prvku|poziceX|poziceY|sirka|vyska
// Nazev dalsiho|... (radek vzdy oddelen pouze \n)
char* MakeChar(OUT char* chr, IN int n)
{
	sprintf(chr, "%d", n);
	return chr;
}
void SaveChildsToFile(IGuiElement* guiEl, FILE* fp)
{
	char tmp[100]; // snad to enni moc (!je to rekurze)
	// zapisem tento
	if (guiEl->m_szName) 
	{
		fputs("\n", fp);
		fputs(guiEl->m_szName, fp);
		fputs("|", fp);
		fputs(MakeChar(tmp, guiEl->m_nPosX), fp);
		fputs("|", fp);
		fputs(MakeChar(tmp, guiEl->m_nPosY), fp);
		fputs("|", fp);
		fputs(MakeChar(tmp, guiEl->m_nWidth), fp);
		fputs("|", fp);
		fputs(MakeChar(tmp, guiEl->m_nHeight), fp);
	}
	
	// zapisem jeho potomky
	for(unsigned int i=0; i < guiEl->m_nChilds; i++)
	{
		SaveChildsToFile(guiEl->m_pChilds[i], fp);
	}
}

void CWindow::RenderSprite(float deltaTime)
{
	if(m_bHidden) return; // bylo ukonceno, ale neuvolneno
	
	// VYKRESLI TEXT

	// vykresli titulek
	if (m_nStyle & P3DWS_CAPTION)
	{
		g_Scheme.GetFontTitle()->DrawText(FA_LEFT, m_szCaption, (int)m_nPosX+3, (int)m_nPosY-TITLE_HEIGHT+3, (int)(m_nPosX+m_nWidth)-3, (int)m_nPosY-3, g_Scheme.GetColor(COL_TITLE_TEXT));

		if (m_nStyle & P3DWS_CLOSEBTN) g_Scheme.GetFontLabel()->DrawText(FA_LEFT, "x", (int)((float)m_nPosX+m_nWidth+2-(TITLE_HEIGHT/1.4f)), (int)((float)m_nPosY-(TITLE_HEIGHT/1.4f)-3), m_nPosX+m_nWidth-6, m_nPosY-3, g_Scheme.GetColor(COL_TITLE_TEXT));
	}
	
	// GUI EDITOR
	if (CVgui_editor.GetBool())
	{
		if (GetFirstVisibleWindow()!=this || this->GetElementFlags() & EF_NOT_EDITABLE) return; // toto umoznime pouze pokud je okno foreground
		// vykresli a zpracuj ukladani a nacitani pozic prvku
		g_Scheme._pLabelFont->DrawText("Load|Save", m_nPosX, m_nPosY, P3DCOLOR_ARGB(180, 200, 255, 200));
		if (g_nMouseY >= m_nPosY && g_nMouseY <= m_nPosY+10)
		{
			if (g_bLMouseDownFirstTime && g_nMouseX >= m_nPosX && g_nMouseX <= m_nPosX + 25)
			{
				// load
				dia_SelectFile->SetPath("gui");
				dia_SelectFile->Show(true);
				bOpening = true;
			}
			else if (g_bLMouseDownFirstTime && g_nMouseX > m_nPosX + 25 && g_nMouseX <= m_nPosX + 51)
			{
				// save
				dia_SelectFile->SetPath("gui");
				dia_SelectFile->Show(true);
				bOpening = true;
				bOpening = false;
			}
		}
		
		// zpracuj dialog otevreni ci ulozni souboru
		if (dia_SelectFile->OnOK())
		{
			// jedna se o ulozeni?
			if (!bOpening)
			{
				char currPath[MAX_PATH];
				if (dia_SelectFile->GetPath(currPath, MAX_PATH))
				{
					// uloz cele nastaveni okna
					FILE* fp=fopen(currPath, "wb");
					if (!fp)
					{	
						CON(MSG_CON_ERR, "GUI Editor: Can't save to %s!", currPath);
						return;
					}
					// hlavicka
					fputs("7GUI_1", fp);
					// projdi prvky okna a podle nazvu je postupne uloz
					SaveChildsToFile(this, fp);
					fclose(fp);
				}
			}
			else // otevreni
			{
				char currPath[MAX_PATH];
				if (dia_SelectFile->GetPath(currPath, MAX_PATH))
				{
					int ind=0;
					for (int ii=0;ii<(int)strlen(currPath);ii++)
						if (!strnicmp(&currPath[ii], "data", 4))
						{
							ind = ii+5;
							break;
						}
					LoadFromFile(&currPath[ind]);
				}
			}
		}
	}
}

void CWindow::Update(float deltaTime)
{
	if(m_bHidden) return; // bylo ukonceno, ale neuvolneno

	if (!(m_nStyle & P3DWS_CAPTION)) return; // nema popisek

	if(m_nStyle & P3DWS_CLOSEBTN)
	{
		//Aktualizuj stav tlacitka X
		if (g_nMouseX >= m_nPosX+m_nWidth-3-(TITLE_HEIGHT/1.4f) && g_nMouseX <= m_nPosX+m_nWidth-3
			&& g_nMouseY >= m_nPosY-(TITLE_HEIGHT/1.4f)-3 && g_nMouseY <= m_nPosY-3) 
		{
			if(g_bLMouseDownFirstTime) m_bXDownFirstTime = true;
			if(m_bXDownFirstTime) m_bXDown = true; else m_bXDown = false;

			if (m_bXDown && g_bLMouseUpFirstTime) m_bHidden=true;// TLACITKO X ZMACKUTO A POVOLENO!
		}
		else
			m_bXDown = false;
	}

	//POSUN OKNO - musi byt posledni v Update()
	if(g_bLMouseDownFirstTime)
	{
		if (g_nMouseX < m_nPosX || g_nMouseX > m_nPosX+m_nWidth-(TITLE_HEIGHT/1.4f)-5
			|| g_nMouseY < m_nPosY - TITLE_HEIGHT || g_nMouseY > m_nPosY || !m_bHasFocus) return;
		relX = g_nMouseX-(int)m_nPosX;
		relY = g_nMouseY-(int)m_nPosY;
	}
	if(g_bLMouseUpFirstTime) {m_bXDownFirstTime = false; relX = -1;}

	if(relX>-1) // posun okno
	{
		m_nPosX = g_nMouseX - relX;
		m_nPosY = g_nMouseY - relY;
	}
}

void CWindow::CenterScreen()
{
	m_nPosX = CVr_width->GetInt()/2-m_nWidth/2;
	m_nPosY = CVr_height->GetInt()/2-m_nHeight/2;
}

void CWindow::Show(bool bForeground/* =true */)
{
	m_bHidden=false; 
	if(bForeground) SetFirstVisibleWindow(this);
}

bool SetElementData(IGuiElement* el, const char* elName, int elPosX, int elPosY, int elWid, int elHei)
{
	// zjisti zda to neni tenhle a jestli jo tak ho nastav
	if (el->m_szName && !stricmp(el->m_szName, elName))
	{
		el->m_nPosX = elPosX;
		el->m_nPosY = elPosY;
		el->m_nWidth = elWid;
		el->m_nHeight = elHei;
		return true;
	}
	
	// tento to neni, zkus potomky
	for(unsigned int i=0; i < el->m_nChilds; i++)
	{
		if (SetElementData(el->m_pChilds[i], elName, elPosX, elPosY, elWid, elHei))
			return true;
	}
	
	return false;
}

bool CWindow::LoadFromFile(const char* guiFile)
{
	DWORD dwSize=0;
	BYTE* pData=NULL;
	
	FSFILE* fp=g_pFS->Load(guiFile, pData, dwSize);
	if (!fp) 
	{
		CON(MSG_CON_ERR, "GUI System: Can't load element arrangement from file %s!", guiFile);
		return false;
	}
	
	// nacti
	int aktBlock=0; // 0-hlavicka, 1-nazev prvku, 2-poziceX, 3-poziceY, 4-sirka, 5-vyska
	char aktElName[50];
	int aktElPosX; int aktElPosY; int aktElWid; int aktElHei;
	int lastValidCharPos=0;
	char tmp[30];
	for(unsigned long i=0;i<dwSize;i++)
	{
		if (aktBlock==0) // hlavicka
		{
			if (pData[i]=='\n')
			{
				// zkontroluj hlavicku
				if (strnicmp((char*)pData, "7GUI_1", 6))
				{
					CON(MSG_CON_ERR, "GUI System: Wrong version of element arrangement (file %s)!", guiFile);
					goto H_FAIL;
				}
				// je to ok, jdem na prvni prvek
				aktBlock = 1;
				lastValidCharPos = i+1;
			}
		}
		else if (aktBlock==1) // nazev prvku
		{
			if (pData[i]=='|')
			{
				strncpy(aktElName, (char*)&pData[lastValidCharPos], i-lastValidCharPos);
				aktElName[i-lastValidCharPos]=0; // zakonc string
				aktBlock = 2; // jdem na poziciX
				lastValidCharPos = i+1;
			}
		}
		else if (aktBlock==2) // poziceX
		{
			if (pData[i]=='|')
			{
				strncpy(tmp, (char*)&pData[lastValidCharPos], i-lastValidCharPos);
				tmp[i-lastValidCharPos]=0;
				aktElPosX = atoi(tmp);
				aktBlock = 3; // jdem na poziciY
				lastValidCharPos = i+1;
			}
		}
		else if (aktBlock==3) // poziceY
		{
			if (pData[i]=='|')
			{
				strncpy(tmp, (char*)&pData[lastValidCharPos], i-lastValidCharPos);
				tmp[i-lastValidCharPos]=0;
				aktElPosY = atoi(tmp);
				aktBlock = 4; // jdem na sirku
				lastValidCharPos = i+1;
			}
		}
		else if (aktBlock==4) // sirka
		{
			if (pData[i]=='|')
			{
				strncpy(tmp, (char*)&pData[lastValidCharPos], i-lastValidCharPos);
				tmp[i-lastValidCharPos]=0;
				aktElWid = atoi(tmp);
				aktBlock = 5; // jdem na vysku
				lastValidCharPos = i+1;
			}
		}
		else if (aktBlock==5) // vyska
		{
			if (pData[i]=='\n')
			{
				strncpy(tmp, (char*)&pData[lastValidCharPos], i-lastValidCharPos);
				tmp[i-lastValidCharPos]=0;
				aktElHei = atoi(tmp);
				// nastavit prvek podle ziskanych dat
				SetElementData(this, aktElName, aktElPosX, aktElPosY, aktElWid, aktElHei);
				aktBlock = 1; // jdem zase na nazev prvku
				lastValidCharPos = i+1;
			}
			else if (i == dwSize-1) // posl. znak
			{
				char tmp[30];
				strncpy(tmp, (char*)&pData[lastValidCharPos], i-lastValidCharPos+1);
				tmp[i-lastValidCharPos+1]=0;
				aktElHei = atoi(tmp);
				// nastavit prvek podle ziskanych dat
				SetElementData(this, aktElName, aktElPosX, aktElPosY, aktElWid, aktElHei);
			}
		}
	}
	return true;
H_FAIL:
	g_pFS->UnLoad(fp, pData);
	return false;
}
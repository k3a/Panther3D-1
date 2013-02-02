//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Console class
//-----------------------------------------------------------------------------

#include "common.h"
#include "console.h"
#include <math.h>
#include "helpers.h"
#include "IP3DMaterialManager.h"

REGISTER_SINGLE_CLASS(CP3DConsole, IP3DENGINE_CONSOLE); // zaregistruj tuto tøídu
static IP3DMaterialManager* g_pMaterialManager=NULL;

bool CP3DConsole::RegisterCVar(const char* VarName, CVarCommand cmdPtr)
{
	if (currC == 10) 
	{
		CON(MSG_CON_ERR, "Maximum number of console command vars exceeded!");
		return false; // maximal vars count
	}

	// zjsiti zda uz dany conVar neni registrovany
	for(int q=0; q < currC; q++)
	{		
		if(!strcmpi(cvar_indexes[q].c_str(), VarName)) // je?
		{
			this->Message(MSG_CON_DEBUG, "Warning: CVar %s already registered! Ignored...", VarName );
			return true;
		}
	}

	// pridej
	cvar_indexes[currC] = string(VarName);
	cvar[currC] = cmdPtr;
	currC++;
	return true;
}

void CP3DConsole::RegisterLastConVar(ConVar* pLastConVar)
{
	if (!pLastConVar) return; // pokud není ukazatel, tak se nic nedìje
	if(!m_pLastConVar) // ulož ukazatel na první
		m_pLastConVar = pLastConVar;
	else // pøidej nového posledního na zaèátek starého
	{
		for (ConVar* cv=m_pLastConVar;;cv = cv->GetNext())
			if(cv->GetNext()==NULL)
			{
				cv->SetNext(pLastConVar);
				break;
			}
	}
}

CP3DConsole::CP3DConsole()
{
	// create debug file
	m_pfDebug = fopen("console.log", "w");

	m_sConText = new string[MAX_CON_LINES];
	m_sHistory = new string[MAX_HISTORY];

	currH = -1;
	shift = 0;
	drawCon = false;

	m_pLastConVar = NULL;

	cvar_indexes = new string[MAX_COMMANDS];
	currC = 0;

	m_pListener = NULL;
}

CP3DConsole::~CP3DConsole()
{
	delete[] m_sConText;
	delete[] m_sHistory;
	delete[] cvar_indexes;
	fclose(m_pfDebug);
}

bool CP3DConsole::IsShown()
{
	return drawCon;
}

void CP3DConsole::GetCmdFromStr(const char* cmdStr, char* cmd)
{
	char buff[512];
	strcpy(buff, cmdStr); // zkopíruj do pomocné promìnné
	for(unsigned int i=0; i < strlen(buff); i++) // projdi string
		if (buff[i]==0 || buff[i]==' ') // pokud je znak 0 nebo mezera
		{
			buff[i]=0; // ukonèi øetìzec
			break;
		}
		strcpy(cmd, buff); // zkopíruj na výstup

}

ConVar* CP3DConsole::FindConVar(const char* pName)
{
	for (ConVar* cv=m_pLastConVar;cv;cv = cv->GetNext())
		if(!strcmpi(cv->GetName(), pName))
		{
			return cv;
		}
		return NULL;
}

void CP3DConsole::Command(const char *cmd)
{
	currH = -1; // historie znovu od zaèátku
	//pøidej do historie
	for (int i=MAX_HISTORY-1; i > 0; i--)
	{
		m_sHistory[i] = m_sHistory[i-1];
	}
	m_sHistory[0] = string(cmd); // pøidat

	//this->Message(MSG_CON_INFO, "", cmd); // new line
	this->Message(MSG_CON_INFO, "=> %s:", cmd);
	// basic (no method pointer) commands

	char buff[512]; // jen pøíkaz
	GetCmdFromStr(cmd, buff);

	if (!strcmp(buff, "help"))
	{
		char szFilter[100];
		if (strlen(cmd)-1 > strlen(buff)) // jsou-li parametry (odeèteme 1 znak = mezera)
			strcpy(szFilter, &cmd[strlen(buff)+1]);
		else
			strcpy(szFilter, "");
		// projdi convars
		for(ConVar* cv=m_pLastConVar;cv;cv = cv->GetNext())
		{
			if (!stristr(cv->GetName(),szFilter)
				&&!stristr(cv->GetDefault(),szFilter)
				&&!stristr(cv->GetString(),szFilter)
				&&!stristr(cv->GetHelpStr(),szFilter)) continue; // to není on

			char pszVal[100];
			if (cv->GetString()) 
				strcpy(pszVal, cv->GetString());
			else
				sprintf(pszVal, "%d", cv->GetInt());

			const char* pszDef = cv->GetDefault();
			char pszHelp[256];
			strcpy(pszHelp, cv->GetHelpStr());

			// formátuj nápovìdu
			if (strlen(pszHelp)>0)
			{
				strcpy(pszHelp, "- ");
				strcat(pszHelp, cv->GetHelpStr());
			}

			// formátuj pøíznaky
			char pszFlags[10];
			int nFlags = cv->GetFlags();
			strcpy(pszFlags, " [");
			if (nFlags & CVAR_ARCHIVE)
				strcat(pszFlags, "A");
			if (nFlags & CVAR_READONLY)
				strcat(pszFlags, "R");
			if (nFlags & CVAR_NOT_STRING)
				strcat(pszFlags, "N");

			if (strlen(pszFlags)>2)
				strcat(pszFlags, "]");
			else
				strcpy(pszFlags, "");

			if (!strcmp(pszVal, pszDef))
				this->Message(MSG_CON_INFO, "%s : %s%s %s", cv->GetName(), pszVal, pszFlags, pszHelp);
			else
				this->Message(MSG_CON_INFO, "%s : %s (def. %s)%s %s", cv->GetName(), pszVal, pszDef, pszFlags, pszHelp);
		}
		// projdi všechny pøíkazy
		for (int i=0; i < currC; i++)
		{
			if (!stristr(cvar_indexes[i].c_str(), szFilter)) continue; // to není on
			this->Message (MSG_CON_INFO, "%s", cvar_indexes[i].c_str());
		}
		// vnitøní pøíkazy
		if (stristr("help", szFilter)) this->Message (MSG_CON_INFO, "help");
		if (stristr("quit", szFilter)) this->Message (MSG_CON_INFO, "quit");
		if (stristr("savehelp", szFilter)) this->Message (MSG_CON_INFO, "savehelp");
	}
	else if (!stricmp(buff, "quit")) 
		g_pEngine->Shutdown();
	else if (!stricmp(buff, "savehelp"))
	{
		FILE* fp = fopen("console_help.txt", "wb");
		strcpy(buff, "{|\r\n| '''Nazev''' || '''Default''' || '''Flags''' || '''Help'''\r\n"); fwrite(buff, strlen(buff), 1, fp);
		for(ConVar* cv=m_pLastConVar;cv;cv = cv->GetNext())
		{
			// formátuj pøíznaky
			char pszFlags[10];
			int nFlags = cv->GetFlags();
			strcpy(pszFlags, " [");
			if (nFlags & CVAR_ARCHIVE)
				strcat(pszFlags, "A");
			if (nFlags & CVAR_READONLY)
				strcat(pszFlags, "R");
			if (nFlags & CVAR_NOT_STRING)
				strcat(pszFlags, "N");

			if (strlen(pszFlags)>2)
				strcat(pszFlags, "]");
			else
				strcpy(pszFlags, "");

			sprintf(buff, "|-\r\n| %s || (def. %s) || %s || %s\r\n", cv->GetName(), cv->GetDefault(), pszFlags, cv->GetHelpStr());
			fwrite(buff, strlen(buff), 1, fp);
		}
		strcpy(buff, "|}\r\n"); fwrite(buff, strlen(buff), 1, fp);
		fclose(fp);
		g_pConsole->Message(MSG_CON_INFO, "List of console variables has been saved to console_help.txt.");
	}
	else
	{
		// zavolej prikaz listeneru
		if (m_pListener && m_pListener->Command(buff)) return;

		for (int i=0; i < currC; i++) // projdi všechny pøíkazy
			if (cvar_indexes[i] == string(buff)) // pokud je nalezen
			{
				if (strlen(cmd) > strlen(buff)) // jsou-li parametry
					cvar[i](&cmd[strlen(buff)+1]); // poslat jen parametry
				else
					cvar[i](NULL); // pøíkaz je bez parametrù
				return;
			}

			// projdi všechny ConVary
			for (ConVar* cv=m_pLastConVar;cv;cv = cv->GetNext())
				if(!strcmpi(cv->GetName(), buff))
				{
					if(cv->GetFlags()&CVAR_READONLY) 
					{
						this->Message(MSG_CON_ERR, "ConVar %s is readonly!", buff);
						return;
					}
					if (strlen(cmd)-1 > strlen(buff)) // jsou-li parametry (odeèteme 1 znak = mezera)
						cv->SetValue(&cmd[strlen(buff)+1]);
					else
					{
						char pszVal[100];
						if (cv->GetString()) 
							strcpy(pszVal, cv->GetString());
						else
							sprintf(pszVal, "%d", cv->GetInt());

						const char* pszDef = cv->GetDefault();
						char pszHelp[256];
						strcpy(pszHelp, cv->GetHelpStr());

						// formátuj nápovìdu
						if (strlen(pszHelp)>0)
						{
							strcpy(pszHelp, "- ");
							strcat(pszHelp, cv->GetHelpStr());
						}

						// formátuj pøíznaky
						char pszFlags[10];
						int nFlags = cv->GetFlags();
						strcpy(pszFlags, " [");
						if (nFlags & CVAR_ARCHIVE)
							strcat(pszFlags, "A");
						if (nFlags & CVAR_READONLY)
							strcat(pszFlags, "R");
						if (nFlags & CVAR_NOT_STRING)
							strcat(pszFlags, "N");

						if (strlen(pszFlags)>2)
							strcat(pszFlags, "]");
						else
							strcpy(pszFlags, "");

						if (!strcmp(pszVal, pszDef))
							this->Message(MSG_CON_INFO, "\"%s\" = \"%s\"%s %s", buff, pszVal, pszFlags, pszHelp);
						else
							this->Message(MSG_CON_INFO, "\"%s\" = \"%s\" (def. %s)%s %s", buff, pszVal, pszDef, pszFlags, pszHelp);
					}

					return;
				}

				this->Message(MSG_CON_INFO, "Unrecognized command or variable '%s'!", cmd);
	}
}

void CP3DConsole::Render()
{
	Prof(ENGINE_CP3DConsole__Render);
	// získej glob. input (zajímavý zpùsob... prostì musí být initializován renderer aby to bylo možné získet, takže až zde)
	if (!g_pInput) g_pInput = (IP3DInput*)I_GetClass(IP3DRENDERER_INPUT);

	bool spatnyZnak=false; // bude true, pokud byla stisknuta klavesa ; pro skryti/zobr. konzole => nedojde k pridani znaku do input bufferu

	int fntH = fntA15->GetLineH(); // zjisti výšku øádku
	int fntAH = g_pEngSet.Height/3/fntH; // zjisti výšku prostoru pro konzoli

	if (g_pInput->IsKeyDownFirstTime(DIK_GRAVE)) {drawCon=!drawCon; spatnyZnak=true; } // zobraz/skryj
	if (g_pInput->IsKeyDownFirstTime(DIK_PGUP) && drawCon) if (shift+3 < MAX_CON_LINES ) shift += 3; else shift = MAX_CON_LINES; // posuò o 3 øádky nahoru
	if (g_pInput->IsKeyDownFirstTime(DIK_PGDN) && drawCon) if (shift > 2) shift -= 3; else shift = 0; // posuò o 3 øádky dolù

	// historie pøíkazù
	if (g_pInput->IsKeyDownFirstTime(DIK_UP) && drawCon) // historie++
	{
		currH++;
		if (currH == -1) currH = 0;
		if (currH < MAX_HISTORY && m_sHistory[currH] != string(""))
		{
			inpBuffer = m_sHistory[currH]; 
		}
		else
			currH--;
	}
	if (g_pInput->IsKeyDownFirstTime(DIK_DOWN) && drawCon) // historie--
	{ 
		currH--;
		if (currH < -2) currH = -2;
		if (currH >= 0 && m_sHistory[currH] != string(""))
		{
			inpBuffer = m_sHistory[currH]; 
		}
		else if (currH == -1)
		{
			inpBuffer.clear();
		}
	}

	if (!drawCon) return; // pokud se má konzole vykreslit

	// získej znaky
	unsigned short kChr = g_pInput->GetKeyChar();
	switch(kChr)
	{
	case 13:
		if (inpBuffer.length()>0) { this->Command(inpBuffer.c_str()); inpBuffer.clear(); } // enter - proveï pøíkaz
		break;
	case 8:
		if (inpBuffer.length()>0) inpBuffer.erase(inpBuffer.length()-1); // backspace - smaž znak
		break;
	default:
		if (kChr>0 && spatnyZnak==false/*znak_pro_zobr._konzoly*/) inpBuffer += (char)kChr; // pøidej znak
		break;
	}

	if (!g_pMaterialManager) g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	g_pMaterialManager->TurnOffShaders(); // nepouzivame shadery

	// vykresli pozadí
	g_p2D->DrawSquare(0, 0, g_pEngSet.Width, (fntAH+3)*fntH, P3DCOLOR_XRGB(75, 75, 75));

	// vykresli konzoli
	int i;
	for(i=0; i <= fntAH; i++)
	{
		if (m_sConText[fntAH-i+shift].length() == 0) continue; // pokud není co kreslit
		if (m_sConText[fntAH-i+shift].c_str()[0] == '!')
		{
			// èervený text
			fntA15->DrawText(FA_NOCLIP, &m_sConText[fntAH-i+shift].c_str()[1], 4, fntH*i+4, 0, 0, P3DCOLOR_XRGB(0, 0, 0));
			fntA15->DrawText(FA_NOCLIP, &m_sConText[fntAH-i+shift].c_str()[1], 3, fntH*i+3, 0, 0, P3DCOLOR_XRGB(200, 125, 125));
		}
		else if (m_sConText[fntAH-i+shift].c_str()[0] == 'D')
		{
			// debug modrý text
			fntA15->DrawText(FA_NOCLIP, &m_sConText[fntAH-i+shift].c_str()[1], 4, fntH*i+4, 0, 0, P3DCOLOR_XRGB(0, 0, 0));
			fntA15->DrawText(FA_NOCLIP, &m_sConText[fntAH-i+shift].c_str()[1], 3, fntH*i+3, 0, 0, P3DCOLOR_XRGB(125, 125, 200));
		}
		else
		{
			// klasický text
			fntA15->DrawText(FA_NOCLIP, &m_sConText[fntAH-i+shift].c_str()[1], 4, fntH*i+4, 0, 0, P3DCOLOR_XRGB(0, 0, 0));
			fntA15->DrawText(FA_NOCLIP, &m_sConText[fntAH-i+shift].c_str()[1], 3, fntH*i+3, 0, 0, P3DCOLOR_XRGB(200, 200, 200));
		}
	}
	// vykresli znak '=>'
	int delkaZ = fntA15->DrawText(FA_NOCLIP, "=> ", 4, fntH*i+4, 0, 0, P3DCOLOR_XRGB(0, 0, 0));
	fntA15->DrawText(FA_NOCLIP, "=> ", 3, fntH*i+3, 0, 0, P3DCOLOR_XRGB(255, 255, 255));

	inpBuffer+='_'; // pøidej _ na konec
	fntA15->DrawText(FA_NOCLIP, inpBuffer.c_str(), delkaZ + 4*2, fntH*i+4, 0, 0, P3DCOLOR_XRGB(0, 0, 0));
	fntA15->DrawText(FA_NOCLIP, inpBuffer.c_str(), delkaZ + 3*2, fntH*i+3, 0, 0, P3DCOLOR_XRGB(255, 255, 255));
	inpBuffer.erase(inpBuffer.length()-1); // odeber _ z konce stringu
	if (shift > 0) fntA15->DrawText(FA_NOCLIP, "^", 3, fntH*(i+1)+3, 0, 0, P3DCOLOR_XRGB(255, 255, 255));

	// vykresli možné texty (tipy)
	int nRadek=0; // od 1 do 6 vcetne
	//char* pszTipy[6];
	char pszTipy[6][100];

	/*for (int q=0;q<6;q++)
	pszTipy[q]=new char[100];*/

	// získej tipy z ConVarù
	for(ConVar* cv=m_pLastConVar;cv;cv=cv->GetNext())
	{
		if(!strnicmp(cv->GetName(), inpBuffer.c_str(), strlen(inpBuffer.c_str()))) // zkontroluj ConVary na delku co je zadana
		{
			if (nRadek==5) { nRadek++; strcpy(pszTipy[5], "..."); goto G_DRAW;}; // ok, už je to hodnì

			char pszVal[100];
			if (cv->GetString()) 
				strcpy(pszVal, cv->GetString());
			else
				sprintf(pszVal, "%d", cv->GetInt());

			sprintf(pszTipy[nRadek], "%s = %s", cv->GetName(), pszVal);
			nRadek++;
		}
	}

	// získej tipy z ConCmd
	for(int q=0; q < currC; q++)
	{		
		if(!strnicmp(cvar_indexes[q].c_str(), inpBuffer.c_str(), strlen(inpBuffer.c_str()))) // zkontroluj ConVary na delku co je zadana
		{
			if (nRadek==5) { nRadek++; strcpy(pszTipy[5], "..."); goto G_DRAW;}; // ok, už je to hodnì
			strcpy(pszTipy[nRadek], cvar_indexes[q].c_str());
			nRadek++;
		}
	}
	// ziskej concmd z vnitrnich prikazu
	if (!strnicmp("quit", inpBuffer.c_str(), strlen(inpBuffer.c_str())))
	{
		if (nRadek==5) { nRadek++; strcpy(pszTipy[5], "..."); goto G_DRAW;}; // ok, už je to hodnì
		strcpy(pszTipy[nRadek], "quit");
		nRadek++;
	}
	if (!strnicmp("help", inpBuffer.c_str(), strlen(inpBuffer.c_str())))
	{
		if (nRadek==5) { nRadek++; strcpy(pszTipy[5], "..."); goto G_DRAW;}; // ok, už je to hodnì
		strcpy(pszTipy[nRadek], "help");
		nRadek++;
	}
	if (!strnicmp("savehelp", inpBuffer.c_str(), strlen(inpBuffer.c_str())))
	{
		if (nRadek==5) { nRadek++; strcpy(pszTipy[5], "..."); goto G_DRAW;}; // ok, už je to hodnì
		strcpy(pszTipy[nRadek], "savehelp");
		nRadek++;
	}

G_DRAW:

	// TODO: seradit dle abecedy a velkych pismen

	if (nRadek == 0) return; // nejsou tipy

	char pszPrvni[100];
	strcpy(pszPrvni,pszTipy[0]);
	// odstran '=' a to za tim
	for (unsigned int z=0;z<strlen(pszPrvni);z++)
	{
		if (pszPrvni[z]=='=')
		{
			pszPrvni[z-1]=0;
			break;
		}
	}

	// dopln
	if ((g_pInput->IsKeyDown(DIK_LCONTROL) || g_pInput->IsKeyDown(DIK_RCONTROL)) && (g_pInput->IsKeyDownFirstTime(DIK_LSHIFT)||g_pInput->IsKeyDownFirstTime(DIK_RSHIFT))) // dopln
	{
		strcat(pszPrvni, " "); //aby se nezobrazoval tip
		inpBuffer = pszPrvni;
		return;
	}

	// help
	strcat(pszTipy[0], " (CTRL+SHIFT)");

	// vykresli tipy
	for(int q=0; q<nRadek;q++)
	{
		fntA15->DrawText(FA_NOCLIP, pszTipy[q], 6, fntH*(i+2+q)+4, 0, 0, P3DCOLOR_XRGB(0, 0, 0));
		fntA15->DrawText(FA_NOCLIP, pszTipy[q], 5, fntH*(i+2+q)+3, 0, 0, P3DCOLOR_XRGB(225, 255, 225));
	}

	/*	for (q=0;q<6;q++)
	delete[] pszTipy[q];*/
}

void CP3DConsole::PrintMSG(const char* buffer, ConsoleMsg_t type, const char* scopeName, const char* targetName, const char* clsName)
{
	int i;

	char InternalBuffer[1024];

	if (targetName && clsName) // vypis pro cast s entitama
		sprintf(InternalBuffer, "Entity %s (class %s): %s", targetName, clsName, buffer);
	else
		strcpy(InternalBuffer, buffer);

	if (scopeName && m_pListener) 
		m_pListener->MessageEx(type, scopeName, InternalBuffer); // extra pro listener

	if(m_pListener) m_pListener->Message(type, InternalBuffer); // klasika

	if (type == MSG_LISTENER) return;

	switch(type)
	{
	case MSG_CON_DEBUG:
#ifdef _DEBUG
		for (i =MAX_CON_LINES-1; i > 0; i--)
		{
			m_sConText[i] = m_sConText[i-1];
		}
		m_sConText[0] = string("D") + string(InternalBuffer); // znak 'D' znamena debug modry text
#else
		return;
#endif
		break;
	case MSG_CON_INFO:
		for (i =MAX_CON_LINES-1; i > 0; i--)
		{
			m_sConText[i] = m_sConText[i-1];
		}
		m_sConText[0] = string("O") + string(InternalBuffer); // znak 'O' znamena klasicky text
		break;
	case MSG_CON_ERR:
		for (i =MAX_CON_LINES-1; i > 0; i--)
		{
			m_sConText[i] = m_sConText[i-1];
		}
		m_sConText[0] = string("!") + string(InternalBuffer); // znak '!' znamena error text
		break;
	case MSG_INFO:
		MessageBox(0, InternalBuffer, "Information!", MB_ICONWARNING | MB_SYSTEMMODAL);
		break;
	case MSG_ERR_FATAL:
		MessageBox(0, InternalBuffer, "Fatal error!", MB_ICONSTOP | MB_SYSTEMMODAL);
		exit(-1);
		break;
	}
}

void CP3DConsole::Message(ConsoleMsg_t type, const char* text, ...)
{
	char buffer[1024];
	try
	{
		va_list argList;
		va_start(argList, text);
		vsprintf(buffer,text,argList);
		va_end(argList);
	}
	catch(...)
	{
		Message(MSG_CON_ERR, "Can't create Message() from va_list! Long message (>1024)?");
		return;
	}

	if (type==MSG_ERR_FATAL || type==MSG_INFO) 
		PrintMSG(buffer, type);

	// projdi text a pri kazdem \n udelej novy radek
	char ln[1024]; char chr; int lnLen=0;
	for (unsigned int i=0; i<strlen(buffer)+1; i++)
	{
		chr = buffer[i];
		if (lnLen>0 && (chr=='\n' || chr==0))
		{
			ln[lnLen] = 0;
			PrintMSG(ln, type);
			lnLen = 0;
		}
		else if (chr == '\r') {}
		else
		{
			ln[lnLen] = chr;
			lnLen++;
		}
	}

	// to debug console
	strcat(buffer, "\r\n");
	OutputDebugString((string("> ")+string(buffer)).c_str()); // do ladící konzole, je to tak ok? Nevadí to vytváøení stringu, je pak uvolnìn?
	// to file
	fprintf(m_pfDebug, buffer); // do souboru
	fflush(m_pfDebug);
}

void CP3DConsole::MessageEx(ConsoleMsg_t type, const char* scopeName, const char* text, ...)
{
	char buffer[1024];
	try
	{
		va_list argList;
		va_start(argList, text);
		vsprintf(buffer,text,argList);
		va_end(argList);
	}
	catch(...)
	{
		Message(MSG_CON_ERR, "Can't create Message() from va_list! Long message (>1024)?");
		return;
	}

	if (type==MSG_ERR_FATAL || type==MSG_INFO)   
		PrintMSG(buffer, type, scopeName);

	// projdi text a pri kazdem \n udelej novy radek
	char ln[1024]; char chr; int lnLen=0;
	for (unsigned int i=0; i<strlen(buffer)+1; i++)
	{
		chr = buffer[i];
		if (lnLen>0 && (chr=='\n' || chr==0))
		{
			ln[lnLen] = 0;
			PrintMSG(ln, type, scopeName);
			lnLen = 0;
		}
		else if (chr == '\r') {}
		else
		{
			ln[lnLen] = chr;
			lnLen++;
		}
	}

	// to debug console
	strcat(buffer, "\r\n");
	OutputDebugString((string("> [")+string(scopeName)+string("] ")+string(buffer)).c_str()); // do ladící konzole, je to tak ok? Nevadí to vytváøení stringu, je pak uvolnìn?
	// to file
	fprintf(m_pfDebug, (string("[")+string(scopeName)+string("] ")+string(buffer)).c_str()); // do souboru
	fflush(m_pfDebug);
}

void CP3DConsole::MessageEntity(ConsoleMsg_t type, const char* scopeName, const char* entityTargetName, const char* entityClsName, const char* text, ...)
{
	char buffer[1024];
	try
	{
		va_list argList;
		va_start(argList, text);
		vsprintf(buffer,text,argList);
		va_end(argList);
	}
	catch(...)
	{
		Message(MSG_CON_ERR, "Can't create Message() from va_list! Long message (>1024)?");
		return;
	}

	if (type==MSG_ERR_FATAL || type==MSG_INFO) 
		PrintMSG(buffer, type, scopeName, entityTargetName, entityClsName);

	// projdi text a pri kazdem \n udelej novy radek
	char ln[1024]; char chr; int lnLen=0;
	for (unsigned int i=0; i<strlen(buffer)+1; i++)
	{
		chr = buffer[i];
		if (lnLen>0 && (chr=='\n' || chr==0))
		{
			ln[lnLen] = 0;
			PrintMSG(ln, type, scopeName, entityTargetName, entityClsName);
			lnLen = 0;
		}
		else if (chr == '\r') {}
		else
		{
			ln[lnLen] = chr;
			lnLen++;
		}

	}

	// to debug console
	strcat(buffer, "\r\n");
	OutputDebugString((string("> [")+string(scopeName)+string("] Entity ")+string(entityTargetName)+string(" (class ")+string(entityClsName)+string("): ")+string(buffer)).c_str()); // do ladící konzole, je to tak ok? Nevadí to vytváøení stringu, je pak uvolnìn?
	// to file
	fprintf(m_pfDebug, (string("[")+string(scopeName)+string("] Entity ")+string(entityTargetName)+string(" (class ")+string(entityClsName)+string("): ")+string(buffer)).c_str()); // do souboru
	fflush(m_pfDebug);
}
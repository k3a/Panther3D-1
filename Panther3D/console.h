//----------- Copyright � 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Console class definition
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dconsole.h"
#include "ip3ddraw2d.h"
#include <string>

using namespace std;

#define MAX_COMMANDS 30 // max. po�et ConCmds, p��kaz� konzole
#define MAX_CON_LINES 255 // max. console text lines
#define MAX_HISTORY 10 // max. number of commands in history (key up and down), in Quake it is 32

class CP3DConsole : public IP3DConsole
{
public:
	CP3DConsole();
	~CP3DConsole();
	void Render();
	void Message(ConsoleMsg_t type, const char* text, ...);
	void MessageEx(ConsoleMsg_t type, const char* scopeName, const char* text, ...);
	void MessageEntity(ConsoleMsg_t type, const char* scopeName, const char* entityTargetName, const char* entityClsName, const char* text, ...);
	void Command(const char *cmd);
	bool IsShown();
	void SetConsoleListener(IP3DConsoleListener *listener){m_pListener = listener;};

	bool RegisterCVar(const char* VarName, CVarCommand cmdPtr);
	void RegisterLastConVar(ConVar* pLastConVar);
	ConVar* FindConVar(const char* pName);
private:
	void GetCmdFromStr(const char* cmdStr, char* cmd); // z�skat p��kaz (z "prikaz param1 param2" z�sk� "prikaz")
	void PrintMSG(const char* buffer, ConsoleMsg_t type, const char* scopeName=NULL, const char* targetName=NULL, const char* clsName=NULL); // vlozi text jednoho radku do konzole

	FILE *m_pfDebug; // soubor s v�stupem konzole

	bool drawCon; // vykreslovat konzoli?

	IP3DConsoleListener *m_pListener;
	string inpBuffer; // vstupn� text konzole - takov� ten InputBox
	string* m_sConText; // ve�ker� text konzole po ��dc�ch
	string* m_sHistory; // historie p��kaz�
	int currH; // aktu�ln� index pole historie
	int shift; // in lines
//
	CVarCommand cvar[MAX_COMMANDS]; // toto sta��.... pro prom�nn� pou��vat ConVary
	string* cvar_indexes;
	int currC; // aktu�ln� index
//
	ConVar* m_pLastConVar;
};
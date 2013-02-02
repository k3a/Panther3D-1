//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Game language string manager
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dlocalization.h"

#define MAX_LNG_LINES 1024
#define MAX_LNG_FILES 15
// struktura pro uchovani jazykovych textu jedne kategorie - jednoto souboru
struct LngFile_s
{
public:
	char szName[20]; // nazev kategorie
	char* pIndexes[MAX_LNG_LINES]; // neprelozene nazvy
	char* pVals[MAX_LNG_LINES];  // prelozene nazvy
	ULONG m_lNumLines; // pocet radku (indexu)
};

class CP3DLocalization : public IP3DLocalization
{
public:
	CP3DLocalization();
	~CP3DLocalization();
	bool Init();
	void Shutdown();
	bool Translate(char* szInOutString); // pokud bude buffer maly, pokusi se ho manager zvetsit
	const char* Translate_const(const char* szInString); // NEPOUZIVAT JAKO UKAZATEL, VZDY JEN ZKOPIROVAT VRACENY STRING!
private:
	char m_szTemp[500];
	LngFile_s m_lngFiles[MAX_LNG_FILES];
	UINT m_nLngFiles;
};
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
#include "Localization.h"
#include "ip3dfilesystem.h"
#include "common.h"

REGISTER_SINGLE_CLASS(CP3DLocalization, IP3DENGINE_LOCALIZATION);

static ConVar CV_lang("cl_lang", "czech", CVAR_ARCHIVE, "Game language prefix");

CP3DLocalization::CP3DLocalization(){m_nLngFiles=0; g_pLocaliz = this;};
CP3DLocalization::~CP3DLocalization(){Shutdown(); g_pLocaliz = NULL;};

bool CP3DLocalization::Init()
{
	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM);

	int nValid;
	HANDLE searchHandle=NULL;
	WIN32_FIND_DATA fileData;

	char pathToSearch[MAX_PATH];
	strcpy(pathToSearch,"localization/");
	strcat(pathToSearch, CV_lang.GetString());
	strcat(pathToSearch, "_*.txt");
	
	searchHandle = g_pFS->FindFirstFile(pathToSearch, &fileData);
	nValid = (searchHandle == INVALID_HANDLE_VALUE) ? 0 : 1;

	m_nLngFiles = 0;

	while (nValid)
	{
		if (m_nLngFiles==MAX_LNG_FILES)
		{
			CON(MSG_CON_ERR, "Localization: Max. number of language files (%d) exceeded!", m_lngFiles);
			return false;
		}
	
		if (strcmpi (fileData.cFileName, ".") != 0 &&
			strcmpi (fileData.cFileName, "..") != 0)
		{
			char txtPath[MAX_PATH];
			strcpy(txtPath, "localization/");
			strcat(txtPath, fileData.cFileName);

			// otevrit soubor pro cteni
			BYTE* pData=NULL; ULONG lSize=0; FSFILE* fp;
			fp = g_pFS->Load(txtPath, pData, lSize);
			
			if (fp)
			{
				// nastav novou strukturu
				m_lngFiles[m_nLngFiles].m_lNumLines=0;
				// zkopiruj nazev kategorie
				strcpy(m_lngFiles[m_nLngFiles].szName, &fileData.cFileName[strlen(CV_lang.GetString())+1]);
				
				if (m_lngFiles[m_nLngFiles].szName[strlen(m_lngFiles[m_nLngFiles].szName)-4]!='.')
					CON(MSG_CON_ERR, "Localization: Bad language file extension (should be .txt)!");

				m_lngFiles[m_nLngFiles].szName[strlen(m_lngFiles[m_nLngFiles].szName)-4]=0;

				char szCurrLine[500]; //WARN:
				ULONG lStr=0;
				for(ULONG l=0;l<lSize;l++)
				{
					if (pData[l]!='\n')
					{
						if (pData[l]=='\r') // konec radku 
						{
							szCurrLine[lStr] = 0; // ukonceni stringu
							lStr=0;
							// zpracovani radku
							for(UINT i=0;i<strlen(szCurrLine);i++)
							{
								if (szCurrLine[i]=='=')
								{
									ULONG lineNum = m_lngFiles[m_nLngFiles].m_lNumLines;

									if (lineNum==MAX_LNG_LINES)
									{
										CON(MSG_CON_ERR, "Localization: Max. number of lines (%d) in %s exceeded!", MAX_LNG_LINES, fileData.cFileName);
										return false;
									}

									// hodnota
									m_lngFiles[m_nLngFiles].pVals[lineNum] = new char[strlen(&szCurrLine[i+1])+1];
									strcpy(m_lngFiles[m_nLngFiles].pVals[lineNum],&szCurrLine[i+1]);
									// nazev (index)
									szCurrLine[i]=0;
									m_lngFiles[m_nLngFiles].pIndexes[lineNum] = new char[strlen(szCurrLine)+1];
									strcpy(m_lngFiles[m_nLngFiles].pIndexes[lineNum],szCurrLine);
									m_lngFiles[m_nLngFiles].m_lNumLines++;
									break;
								}
							}
							szCurrLine[0]=0;
							continue;
						}
						if (pData[l]=='|')  // pøeveï | na nový øádek
						{
							szCurrLine[lStr] = '\r';
							lStr++;
							szCurrLine[lStr] = '\n';
							lStr++;
						}
						else
						{
							szCurrLine[lStr] = pData[l];
							lStr++;
						}
					}
				}
				g_pFS->UnLoad(fp, pData);
				
				// info do konzole
				CON(MSG_CON_INFO, "Localization: String table %s loaded (%d lines).", fileData.cFileName, m_lngFiles[m_nLngFiles].m_lNumLines);
			
				// ok
				m_nLngFiles++;
			}
			else
				CON(MSG_CON_ERR, "Localization: Can't load '%s'!", txtPath);
		}
		nValid = g_pFS->FindNextFile(searchHandle, &fileData);
	}
	g_pFS->FindClose(searchHandle);

	if(!m_nLngFiles)
		CON(MSG_CON_ERR, "Localization: Can't find %s localization files!", CV_lang.GetString());

	return true;
}

void CP3DLocalization::Shutdown()
{
	for(UINT i=0;i<m_nLngFiles;i++)
	{
		for(ULONG l=0;l<m_lngFiles[i].m_lNumLines;l++)
		{
			SAFE_DELETE_ARRAY(m_lngFiles[i].pIndexes[l]);
			SAFE_DELETE_ARRAY(m_lngFiles[i].pVals[l]);
		}
	}
}

bool CP3DLocalization::Translate(char* szInOutString)
{
	if(szInOutString[0]!='#') return false; // nema byt prelozen

	int nStart=0; UINT i=0;
	for(i=1;i<strlen(szInOutString);i++)
	{
		if (szInOutString[i]=='_') {nStart=i+1; break;}
	}

	szInOutString[i]=0;
	INT lngFileNum=-1;
	for(i=0;i<m_nLngFiles;i++)
	{
		if (!stricmp(&szInOutString[1], m_lngFiles[i].szName))
		{
			lngFileNum=i;
			break;
		}
	}

	// kontrola na nalezeni souboru
	if (lngFileNum==-1) return false;

	UINT currChar=0;
	UINT inLen=(UINT)strlen(&szInOutString[nStart]);
	for(ULONG l=0;l<m_lngFiles[lngFileNum].m_lNumLines;l++)
	{
		if (currChar>inLen-1) break; // hledame uz pismeno navic, nikdy nenajdem :) tak prerusit
		for (UINT i=currChar;i<inLen;i++) // projdi tuto hodnotu od posledniho testovaneho znaku
		{
			if(szInOutString[nStart+currChar]==m_lngFiles[lngFileNum].pIndexes[l][currChar]) // pokud tento znak souhlasi
			{
				if (i==inLen-1) // naprosto se shoduje cely string
				{
					SAFE_DELETE_ARRAY(szInOutString);
					szInOutString = new char[strlen(m_lngFiles[lngFileNum].pVals[l])+1];
					strcpy(szInOutString, m_lngFiles[lngFileNum].pVals[l]);
					return true;
				}
				currChar++; // tento znak v hodnote plati
			}
			else
			{
				break; // tato hodnota to neni
			}
		}
	}

	return false;
}

const char* CP3DLocalization::Translate_const(const char* szInString)
{
	if(szInString[0]!='#') { 
		strcpy(m_szTemp,szInString); 
		return m_szTemp; 
	} // nema byt prelozen

	int nStart=0; UINT i=0;
	for(i=1;i<strlen(szInString);i++)
	{
		if (szInString[i]=='_') {nStart=i+1; break;}
	}

	char* pszTmp = new char[strlen(szInString)+1];
	strcpy(pszTmp, szInString);
	pszTmp[i]=0;

	INT lngFileNum=-1;
	for(i=0;i<m_nLngFiles;i++)
	{
		if (!stricmp(&pszTmp[1], m_lngFiles[i].szName))
		{
			lngFileNum=i;
			break;
		}
	}
	delete[] pszTmp;

	// kontrola na nalezeni souboru
	if (lngFileNum==-1) return szInString;

	UINT currChar=0;
	UINT inLen=(UINT)strlen(&szInString[nStart]);
	for(ULONG l=0;l<m_lngFiles[lngFileNum].m_lNumLines;l++)
	{
		if (currChar>inLen-1) break; // hledame uz pismeno navic, nikdy nenajdem :) tak prerusit
		for (UINT i=currChar;i<inLen;i++) // projdi tuto hodnotu od posledniho testovaneho znaku
		{
			if(szInString[nStart+currChar]==m_lngFiles[lngFileNum].pIndexes[l][currChar]) // pokud tento znak souhlasi
			{
				if (i==inLen-1) // naprosto se shoduje cely string
				{
					//strcpy(m_szTemp, m_lngFiles[lngFileNum].pVals[l]);
					//return m_szTemp;
					return m_lngFiles[lngFileNum].pVals[l];
				}
				currChar++; // tento znak v hodnote plati
			}
			else
			{
				break; // tato hodnota to neni
			}
		}
	}

	//strcpy(m_szTemp,szInString); 
	CON(MSG_CON_DEBUG, "Localization: Can't translate %s!", szInString);
	return szInString;
}
//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Update manager - can find new updates and download them
//-----------------------------------------------------------------------------
// TODO: TODO_UP ze solution!!!

#include "updater.h"
#include "engine.h"
#include "common.h"
#include <wininet.h>
#include "crc32.h"

REGISTER_SINGLE_CLASS(P3DUpdateManager, IP3DENGINE_UPDATEMANAGER);

static ConVar CVe_updates("e_updates", "1", CVAR_ARCHIVE | CVAR_NOT_STRING, "Povolit automaticke aktualizace");

vector<char*> P3DUpdateManager::m_pUpdLinks;
unsigned int P3DUpdateManager::m_nUpdates;
// stav
float P3DUpdateManager::m_fPercentDownloaded;
unsigned int P3DUpdateManager::m_nDownloaded;

P3DUpdateManager::~P3DUpdateManager()
{
	for(unsigned int i=0; i<m_pUpdLinks.size(); i++)
		delete[] m_pUpdLinks[i];
}

P3DUpdateManager::P3DUpdateManager()
{
	m_nUpdates=0;
	m_fPercentDownloaded=0;
	m_nDownloaded=0;
}

int P3DUpdateManager::CheckForUpdates(const char* bugServer)
{
	if (!CVe_updates.GetBool()) return false;

	char *query = new char[128];
	char *outResult = new char[1024];

	sprintf(query, "%s/?a=d&v1=%d&v2=%d&v3=%d&v4=%d", bugServer, P3DVER_1, P3DVER_2, P3DVER_3, P3DVER_4);
	//do
	//{
	HINTERNET hInet = InternetOpen("P3DUP", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	HINTERNET hFile = InternetOpenUrl(hInet, query, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hInet || !hFile) return 0;
	DWORD size;
	outResult[0]=0;
	if (!InternetReadFile(hFile, outResult, 1023, &size))
		return 0;
	outResult[size]=0;
	if (!InternetCloseHandle(hFile)) return 0;
	if (!InternetCloseHandle(hInet)) return 0;
	//}while(outResult[0]==0);

	// zjisti pocet updatu
	unsigned int lastPos=0;
	for (unsigned int i=0; i<strlen(outResult); i++)
	{
		if (outResult[i]==';') 
		{
			char* tmpChar = new char[i-lastPos+1];
			strncpy(tmpChar, &outResult[lastPos], i-lastPos);
			tmpChar[i-lastPos]=0;
			m_pUpdLinks.push_back(tmpChar);
			
			lastPos = i+1;
			m_nUpdates++;
		}
	}

	delete[] query;
	delete[] outResult;

	return m_nUpdates;
	return 0;
}

bool P3DUpdateManager::DownloadUpdates()
{
	if (!CVe_updates.GetBool()) return false;

	m_fPercentDownloaded=0;
	HANDLE downThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&DownloadThread, NULL, 0, NULL);

	//WaitForSingleObject(downThread, INFINITE);

	CloseHandle(downThread);
	return true;
}
unsigned long P3DUpdateManager::DownloadThread(void* lpParam)
{
	FILE *fp;
	HINTERNET hOpen, hURL;
	char file[128];
	unsigned long read;
	unsigned long dwByteToRead=0;
	unsigned long dwSizeOfRq=4;
	unsigned long dwByteReaded=0;

	m_nDownloaded=0;

	for (unsigned int ui=0; ui < m_pUpdLinks.size(); ui++)
	{
		hOpen = InternetOpen("P3DUpdater", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0 );
		hURL = InternetOpenUrl( hOpen, m_pUpdLinks[ui], NULL, 0, 0, 0 );

		// zjisti velikost souboru online
		if (!HttpQueryInfo(hURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, (LPVOID)&dwByteToRead, &dwSizeOfRq, NULL))
		{
			dwByteToRead = 0;
		}

		if (dwByteToRead==0) 
		{
			CON(MSG_CON_ERR, "UpdateManager: Update %s have zero length!", m_pUpdLinks[ui]);
			return -1;
		}

		//zjisti nazev souboru z linku
		char* strFName=NULL;
		for(unsigned int il=(unsigned int)strlen(m_pUpdLinks[ui])-1; il > 1; il--)
			if (m_pUpdLinks[ui][il]=='\\' || m_pUpdLinks[ui][il]=='/')
			{
				strFName = &m_pUpdLinks[ui][il+1];
				break;
			}
		if (!strFName)
		{
			CON(MSG_CON_ERR, "UpdateManager: Can't get update filename from link!");
			InternetCloseHandle(hURL);
			InternetCloseHandle(hOpen);
			return -1;
		}

		char newPFName[MAX_PATH]="";
		strcpy(newPFName, "data\\");
		strcat(newPFName, strFName);

		FILE *fpp = fopen(newPFName, "rb");
		if(fpp)
		{
			// uz existuje, tak zkontroluj zda je cely a spravne stazeny
			unsigned long fpCrc32=0; fpos_t fpSize=0; unsigned char* fpData=NULL; unsigned long origCRC=0;
			// otevri a nacti buffer
			fseek(fpp, 0, SEEK_END);
			fgetpos(fpp, &fpSize);
			fseek(fpp, 0, SEEK_SET);
			fpSize -= 4; // delka CRC32 unsigned long na konci
			fpData = new unsigned char[(size_t)fpSize];
			fread(fpData, 1, (size_t)fpSize, fpp);

			fpCrc32 = P3DCRC32(fpData, (int)fpSize);
			if (fpCrc32)
			{
				// precti originalni crc ze souboru
				fseek(fpp, (long)fpSize, SEEK_SET);
				fread(&origCRC, 1, 4, fpp);
			}
			delete[] fpData;

			if (origCRC == fpCrc32)
			{
				fclose(fpp);
				InternetCloseHandle(hURL);
				InternetCloseHandle(hOpen);
				continue; // cely a spravne stazeny
			}
			fclose(fpp);
		}

		fp = fopen(newPFName, "wb");
		if (!fp)
		{
			CON(MSG_CON_ERR, "UpdateManager: Can't open file %s with write mode!", newPFName);
			InternetCloseHandle(hURL);
			InternetCloseHandle(hOpen);
			return -1;
		}
		while(InternetReadFile(hURL, file, sizeof(file), &read) && read != 0)
		{
			fwrite(file, sizeof(char), read, fp);
			//file[read] = '\0';
			dwByteReaded+=read;
			m_fPercentDownloaded = 100.0f/dwByteToRead*dwByteReaded;
			//Sleep(1);
		}
		fclose(fp);

		if (dwByteReaded!=dwByteToRead)
		{
			CON(MSG_CON_DEBUG, "UpdateManager: Update %s wasn't downloaded correctly!", strFName);
		}
		else
			m_nDownloaded++;

		InternetCloseHandle(hURL);
		InternetCloseHandle(hOpen);
	}

	return 0;
}
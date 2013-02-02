//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	filesystem 
//-----------------------------------------------------------------------------
#pragma once
#include "ip3dfilesystem.h"
#include <stdio.h>

#define MAX_ARCHIVES 10
#define MAX_FILE_DIR_NAME 64 // max. délka názvu složky nebo souboru - musi byt shodna s P3DPackem!!!

struct PACK_ENTRY
{
	fpos_t offset;
	fpos_t length;
};

struct PACK_HEADER // hlavicka archivu
{
	char packPrefix[16]; // "P3DPack_1.0 :-C"
	char packKeyTest[4]; // sifra bude OK, pokud bude packKeyTest == ":-)"
	PACK_ENTRY entries[2];
};

enum PACK_LUMP
{
	lDirs, // adresare
	lFiles // popisovace souboru
};

struct FILE_INFO // jeden soubor
{
	char			filName[MAX_FILE_DIR_NAME];
	fpos_t			filSize, // velikost
		filOffset; // offset v 7pk
	unsigned long	filCrc;
	unsigned char	timH,
		timM,
		timS,
		datD,
		datM;
	unsigned int	datY,
		dirID;
};

struct DIRECTORY_INFO // jedna slozka
{
	char			dirName[MAX_FILE_DIR_NAME];
	unsigned int	dirID,
		ParentDirID;
};

struct FS_ARCHIVE
{
	char archiveName[256];

	DIRECTORY_INFO *u7pk_dirs;
	unsigned int u7pk_numdirs;

	FILE_INFO *u7pk_files;
	unsigned int u7pk_numfiles;

	FILE* pArchiveFile;
};

struct FS_FINDINFO // NEMENIT!!!
{
	unsigned int dirID;
	DIRECTORY_INFO* pDi;
	UINT numDirs;
	FILE_INFO* pFi;
	UINT numFiles;
	//
	HANDLE hStdio; // null pokud neni stdio hledani
	char szFileName[MAX_FILE_DIR_NAME];
	char szFilePathName[MAX_PATH];
	unsigned int numMatches; // pocet jiz vracenych vysledku vyhovujicich zadani
};

FS_ARCHIVE* g_pArchives[MAX_ARCHIVES];

class CP3DFileSystem : public IP3DFileSystem
{
public:
	CP3DFileSystem() {for (int i=0; i<MAX_ARCHIVES; i++) g_pArchives[i]=NULL; };
	~CP3DFileSystem() {for (int i=0; i<MAX_ARCHIVES; i++) if(g_pArchives[i]) { fclose(g_pArchives[i]->pArchiveFile);  delete[] g_pArchives[i]->u7pk_dirs; delete[] g_pArchives[i]->u7pk_files; delete g_pArchives[i];} };
	bool Init();
	FSFILE* Open(const char* szPath, const char* szMode, bool bArchiveOnly=false);
	FILE* Open(const char* szPath, bool bArchiveOnly=false); // pouze pro cteni, nutne po pouziti zavolat FS->Close()!
	int Close(FSFILE* file);
	int Close(FILE* file);
	FSFILE* Load (IN const char *szFileName, OUT BYTE* &pData, OUT ULONG &size, bool bNullTerminate=false);
	void UnLoad (FSFILE* pFileToUnload, BYTE *pData = NULL);
	size_t Read(void* data, size_t size, size_t count, FSFILE* fp);
	void Seek(FSFILE *fp, long pos, int type);
	int Eof(FSFILE *fp);
	size_t Write(const void *data, size_t size, size_t count, FSFILE *fp);
	int Flush(FSFILE *fp);
	long Tell(FSFILE *fp);
	int Getc(FSFILE *fp);
	int Putc(int chr, FSFILE *fp);
	int Puts(const char* string, FSFILE* fp);
	unsigned long ReadAll(void* data, FSFILE* fp); // +nastavi pozici na puvodni + vrati velikost nebo 0=ERROR
	int Exists(const char* file); // pouzivat jen tam kde je to nutne, jinak pouzivat podminku !fp. Mozne navratove hodnoty: 0-neexistuje, 1-existuje alespon na disku, 2-existuje jen v archivu
	HANDLE FindFirstFile( IN const char* lpFileName, OUT LPWIN32_FIND_DATAA lpFindFileData );
	BOOL FindNextFile( IN HANDLE hFindFile, OUT LPWIN32_FIND_DATAA lpFindFileData );
	BOOL FindClose( IN OUT HANDLE hFindFile );
private:
	int TryLoadArchive(const char* szFilePath); // vrati index v poli g_pArchives. -1 pokud neni archiv. szFilePath musi obsahovat neco jako materials/neco (z toho se ziska materials.7pk)
};
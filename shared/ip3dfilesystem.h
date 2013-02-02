//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	filesystem iface (engine)
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"
#include <stdio.h>

struct FSFILE // identifikator jednoho FileSystem (FS) souboru (misto FILE), NEMÌNIT PARAMETRY, používat jen
{             // jako "ukazatel" na soubor
	FILE* pSysFile;
	unsigned long nRelOffset; // pozice v tomto "virtualnim" souboru, takze celkova pozice = nOffset+nRelOffset
	unsigned long nOffset; // pozice v SysFILE = zacatek tohoto souboru v souboru FILE, zustava stejna
	unsigned long nLen; // delka tohoto FS souboru
	bool bArchive; // true=soubor z archivu, false=primo z disku
};

#define	FILE_ATTRIBUTE_PACK		0x10000000 // u FindFirstFile jako atribut vysledku v pripade ze je to soubor ze 7pk archivu 

class IP3DFileSystem : public IP3DBaseInterface
{
public:
	virtual FSFILE* Open(const char* szPath, const char* szMode, bool bArchiveOnly=false)=0;
	virtual FILE* Open(const char* szPath, bool bArchiveOnly=false)=0; // pouze pro cteni, nutne po pouziti zavolat FS->Close()!
	virtual int Close(FSFILE* file)=0;
	virtual int Close(FILE* file)=0; // jen pro soubory otevrene pomoci FILE* ->Open()!
	virtual FSFILE* Load (IN const char *szFileName, OUT BYTE* &pData, OUT ULONG &size, bool bNullTerminate=false)=0;
	virtual void UnLoad (FSFILE* pFileToUnload, BYTE *pData = NULL)=0;
	virtual size_t Read(void* data, size_t size, size_t count, FSFILE* fp)=0;
	virtual void Seek(FSFILE *fp, long pos, int type)=0;
	virtual int Eof(FSFILE *fp)=0;
	virtual size_t Write(const void *data, size_t size, size_t count, FSFILE *fp)=0;
	virtual int Flush(FSFILE *fp)=0;
	virtual long Tell(FSFILE *fp)=0;
	virtual int Getc(FSFILE *fp)=0;
	virtual int Putc(int chr, FSFILE *fp)=0;
	virtual int Puts(const char* string, FSFILE* fp)=0;
	virtual unsigned long ReadAll(void* data, FSFILE* fp)=0; // +nastavi pozici na puvodni + vrati velikost nebo 0=ERROR
	virtual int Exists(const char* file)=0; // pouzivat jen tam kde je to nutne, jinak pouzivat podminku !fp. Mozne navratove hodnoty: 0-neexistuje, 1-existuje alespon na disku, 2-existuje jen v archivu
	virtual HANDLE FindFirstFile( IN const char* lpFileName, OUT LPWIN32_FIND_DATAA lpFindFileData )=0;
	virtual BOOL FindNextFile( IN HANDLE hFindFile, OUT LPWIN32_FIND_DATAA lpFindFileData )=0;
	virtual BOOL FindClose( IN OUT HANDLE hFindFile )=0;
};

// MAKRA PRO NACITANI SOUBORU DO PAMETI
// POZOR! Pise se se ; na konci a samo uvnitr vytvori promenne pFP, pData a pSize!!!
// FS_UNLOADFILE zase tyto promenne uvolni
// Jen prvni dva parametry jsou vstupni a musi existovat
// Takze si zadate libovolne nazvy dle potreby
// Typy promennych: FSFILE* pFP, BYTE* pData, ULONG pSize
// Nelze otevirat prazdne soubory!
// Pokud nebude soubor nalezen, nebo bude prazdny, kod mezi LOADFILE A UNLOADFILE se vynecha!!! Je treba s tim pocitat!
#define FS_LOADFILE(pFS, fileNameCharPtr, pFP, pData, ulSize) \
	{FSFILE* pFP = pFS##->Open(fileNameCharPtr, "rb");\
	 if(pFP && pFP##->nLen > 0)\
	 {\
	 	 BYTE* pData = new BYTE[##pFP##->nLen];\
		 ULONG ulSize##; ulSize = pFS##->ReadAll(##pData##, ##pFP##)

#define FS_UNLOADFILE(pFS, fileNameCharPtr, pFP, pData, ulSize) \
		 pFS##->Close(##pFP##);\
		 if (pData) delete[] pData##;\
	 }\
	}

#define IP3DENGINE_FILESYSTEM "P3DFileSystem_3" // nazev ifacu
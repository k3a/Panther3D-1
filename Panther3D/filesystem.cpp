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
#include "filesystem.h"
#include "common.h"
// existence souboru
#include <sys/stat.h>
#include <sys/types.h>

REGISTER_SINGLE_CLASS(CP3DFileSystem, IP3DENGINE_FILESYSTEM); // zaregistruj tuto tøídu

ConVar CVe_homePath("e_home_path", "UNKNOWN_HOME_PATH", CVAR_READONLY);

/* POZNAMKY:
- Potreba kompletne zrychlit?
- Zapis do archivu neni implementovan (poznamky WRITE:) - neni potreba! FS umi zapisovat na disk ;)
*/

/** SIFROVACI ALGORYTMUS **/
BYTE BC71_data1[17]={163,243,7  ,181,30 ,247,224,182,177,185,101,6  ,102,239,3  ,175,98}; // prvni sada
BYTE BC71_data2[18]={42 ,164,41 ,172,32 ,57 ,70 ,8  ,59 ,169,244,63 ,11 ,17 ,113,236,137,120}; // druha sada
BYTE BC71_skip[33]={0,1,0,0,1,1,1,0,1,1,1,0,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,1,0,1,0,0,1}; // kterou pouzit sadu
inline void BC71(BYTE* pData, ULONG lOffset, ULONG lSize, ULONG lFileSize)
{
	for(ULONG l=lOffset;l<lSize;l++) // projdi vsechny byty
	{
		if (BC71_skip[l%33] == 0) // sada 1
		{
			pData[l] ^= BC71_data1[(lFileSize+l)%17];
		}
		else // sada 2
		{
			pData[l] ^= BC71_data2[(lFileSize+l)%18];
		}
	}
}
/** /SIFROVACI ALGORYTMUS/ **/

DIRECTORY_INFO* GetDirectory(const char* szPath, DIRECTORY_INFO* pDi, UINT numDirs) // pozor! vstupni cesta musi obsahovat nazev souboru na konci!
{
	//zjisti posledni slozku v ceste
	char szLastFolder[MAX_PATH];
	strcpy(szLastFolder, szPath);
	int kolik=0; UINT len;
	for(UINT i=(UINT)strlen(szPath)-1;i>=0;i--)
	{
		if (szPath[i]=='/')
		{
			kolik++;
			if (kolik==1) {len = i; continue;} // pokud je to prvni lomitko (pred nazvem souboru) jdi dal - chcem nazev slozky, 2. lomitko
			char szTmp[MAX_PATH];
			strcpy(szTmp, &szLastFolder[i+1]);
			szTmp[len-i-1]=0;
			strcpy(szLastFolder, szTmp);
			break;
		}
	}

	for(UINT i=0;i<numDirs;i++)
	{
		if(!strcmpi(pDi[i].dirName, szLastFolder)) //nalezena posledni slozka - v teto slozce musi byt hledany soubor
		{
			return &pDi[i];
		}
	}
	return NULL;
}

FILE_INFO* GetFile(const char* szPath, int nArchiveID) // pozor! cesta typu data/podslozka/.../cosi
{
	char newPath[256];

	// ziskej promenne
	DIRECTORY_INFO* pDi = g_pArchives[nArchiveID]->u7pk_dirs;
	UINT numDirs = g_pArchives[nArchiveID]->u7pk_numdirs;
	FILE_INFO* pFi = g_pArchives[nArchiveID]->u7pk_files;
	UINT numFiles = g_pArchives[nArchiveID]->u7pk_numfiles;

	// preved opravenou cestu tak aby neobsahovala data/podslozku/
	strcpy(newPath, szPath);
	for(UINT i=6;i<(UINT)strlen(newPath);i++)
	{
		if (newPath[i]=='/')
		{
			// ok... nyni je cesta &newPath[i+1] tak prohledej archiv a najdi ID slozky
			DIRECTORY_INFO* pDir = GetDirectory(newPath, pDi, numDirs); // nemazat pointer!
			if (!pDir) 
			{
				CON(MSG_CON_DEBUG, "FS: Can't extract '%s'!", newPath);
				return NULL;
			}
			// ok... nyni mame slozku. Je potreba najit soubor s danym nazvem nachazejici se ve slozce s id pDir->dirID
			char szFileName[MAX_FILE_DIR_NAME];
			//najdi nazev souboru
			for(UINT ii=(UINT)strlen(newPath)-1;ii>0;ii--)
			{
				if (newPath[ii]=='/')
				{
					strcpy(szFileName, &newPath[ii+1]);
					break;
				}
			}

			for(UINT nFil=0;nFil<numFiles;nFil++)
			{
				if (!strcmpi(pFi[nFil].filName, szFileName) && pFi[nFil].dirID == pDir->dirID)
				{
					return &pFi[nFil];
				}
			}
		}
	}
	return NULL;
}

bool CP3DFileSystem::Init()
{
	char currPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currPath);
	
	// zkontroluj pritomnost /
	char lastChar = currPath[strlen(currPath)-1];
	if (lastChar!='\\' && lastChar!='/') strcat(currPath, "\\");
	
	strcat(currPath, "data"); // pripoj data
	
	// nastav jako home
	CVe_homePath.SetValue(currPath);
	return true;
}

inline void CorrectPath(char* szPath)
{
	// DOPLN SLOZKU DATA
	char szFilePath[256]; bool bData=false;
	if (!strnicmp(szPath, "data", 4)) 
	{
		CON(MSG_CON_ERR, "Path with data! %s", szPath);
		bData = true;
		strcpy(szFilePath, szPath);
	}
	if (szPath[1]!=':' && !bData)
	{
		strcpy(szFilePath, "data/");
		if (szPath[0]=='/' || szPath[0]=='\\')
		{
			strcat(szFilePath, &szPath[1]);
		}
		else
		{
			strcat(szFilePath, szPath);
		}
	}

	// ZPRACUJ CESTU A OPRAV LOMITKA
	int newStart=-1;

	for(unsigned int i=0;i<=strlen(szFilePath);i++)
	{
		if (szFilePath[i]=='\\') // oprav \ na /
			szFilePath[i]='/';

		if (i>4)
			if (!strnicmp(&szFilePath[i-5], "data/", 5))
				newStart = i-5;
	}

	if (newStart>=0) // naslo data/ zkopiruj upravenou rel. cestu
		strcpy(szPath, &szFilePath[newStart]);
	else
		strcpy (szPath, szFilePath);
}

inline void GetArchivePath(char* szPath)
{
	for(unsigned int i=6;i<strlen(szPath);i++) //WARN: hodnota 6 znamena misto za data/
	{
		if (szPath[i]=='/')
		{
			szPath[i]=0;
			strcat(szPath, ".7pk");
		}
	}
}

FSFILE* CP3DFileSystem::Open(const char* szPath, const char* szMode, bool bArchiveOnly/*=false*/)
{
	char szFilePath[256]; // nyni se pracuje s timto "vstupnim" retezcem

	// uprav vstupni parametry
	strcpy(szFilePath, szPath);
	CorrectPath(szFilePath);

	FSFILE* fsf = new FSFILE();

	// ******************** STDIO ********************

	if (!bArchiveOnly)
	{
		fsf->pSysFile = fopen(szFilePath, szMode);
		if(fsf->pSysFile)
		{
			fseek(fsf->pSysFile, 0, SEEK_END); // na konec
			fgetpos(fsf->pSysFile, (fpos_t*)&fsf->nLen); // ziskej pos = délku
			fseek(fsf->pSysFile, 0, SEEK_SET); // na zacatek
			fsf->nOffset = 0; // vynuluj offset
			fsf->nRelOffset = 0; // vynuluj rel. offset
			fsf->bArchive = false; // je z disku
			return fsf;
		}
	}

	// ******************** ARCHIVE ******************** (nenalezeno na disku)
	if (!stricmp(szMode, "r"))
	{
		CON(MSG_CON_ERR, "FS: CP3DFileSystem::Open(%s, \"r\") You can read from the archive only in rb (binary) mode!", szPath);
	}
	
	if (bArchiveOnly && szMode[0]!='r') // zkontroluj read mod
	{
		CON(MSG_CON_ERR, "FS: CP3DFileSystem::Open() Write to an archive not implemented!");
		delete fsf;
		return NULL;
	}

	// pokus se nacist/najit nacteny archiv
	int archiveID = TryLoadArchive(szFilePath);
	if (archiveID<0)
	{
		// soubor nadobro neexistuje
		delete fsf;
		return NULL;
	}

	// nalezeno, ziskej soubor z tohoto archivu
	//ziskej soubor
	FILE_INFO* pFi = GetFile(szFilePath, archiveID);
	if (!pFi){
		delete fsf;
		return NULL;
	}

	fsf->nOffset = (ULONG)pFi->filOffset; // vynuluj offset
	fsf->nLen = (ULONG)pFi->filSize;
	fsf->nRelOffset = 0; // vynuluj rel. offset
	fsf->bArchive = true; // je z archivu
	fsf->pSysFile = g_pArchives[archiveID]->pArchiveFile;
	fseek(g_pArchives[archiveID]->pArchiveFile, fsf->nOffset, SEEK_SET); // na zacatek virtualniho souboru

	return fsf;
}

FILE* CP3DFileSystem::Open(const char* szPath, bool bArchiveOnly)
{
	// nejprve zjisti jestli nejde soubor najit normalne na disku
	// toto bude asi rychlejsi nez vytvareni, kopie a otevreni noveho souboru v tempu
	if (!bArchiveOnly && Exists(szPath) == 1)
	{
		char szFilePath[256]; // nyni se pracuje s timto "vstupnim" retezcem

		// uprav vstupni parametry
		strcpy(szFilePath, szPath);
		CorrectPath(szFilePath);

		FILE* hddfp = fopen(szFilePath, "rb");
		if (!hddfp)
			return NULL;
		else
			return hddfp;
	}

	// ziskej cestu k temp souboru
	char tempFN[100], tempPATH[MAX_PATH];
	GetTempPath(MAX_PATH-1, tempPATH);
	GetTempFileName(tempPATH, "p3d_", 0, tempFN);
	
	BYTE* fileData; ULONG fileLen=0;
	FSFILE *fp = Load(szPath, fileData, fileLen);
	if (fileLen<=0) return NULL;
		// zapis do temp soubou
		FILE* tempFp = fopen(tempFN, "wb");
		if (!tempFp) return NULL;
		fwrite(fileData, fileLen, 1, tempFp);
		fflush(tempFp);
		fseek(tempFp, 0, SEEK_SET);
		tempFp->_tmpfname = new char[strlen(tempFN)+1];
		strcpy(tempFp->_tmpfname, tempFN);
	UnLoad(fp, fileData);
	
	return tempFp;
}

int CP3DFileSystem::Close(FSFILE* file)
{
	if (!file) return NULL;
	int ret = 1;
	if (!file->bArchive) ret = fclose(file->pSysFile);
	delete file; // bylo vytvoreno pomoci new
	file = NULL; //radeji
	return ret;
}

int CP3DFileSystem::Close(FILE* file)
{
	if (!file) return NULL;
	
	char fName[MAX_PATH];
	strcpy(fName, file->_tmpfname);
	
	delete[] file->_tmpfname;
	file->_tmpfname = NULL;
	int ret = fclose(file);
	
	DeleteFile(fName);
	return 1; // je to ok?
}

FSFILE* CP3DFileSystem::Load (IN const char *szFileName, OUT BYTE* &pData, OUT ULONG &size, bool bNullTerminate)
{
	FSFILE	*pFileOut;
	if (szFileName == NULL) return NULL;

	if ((pFileOut = Open (szFileName, "rb")) == NULL)
		return NULL;

	if (pFileOut->nLen == 0)
		goto quit_hand;

	if (bNullTerminate) 
		pData = new BYTE[pFileOut->nLen+1];
	else
		pData = new BYTE[pFileOut->nLen];

	if (pData == NULL)
		goto quit_hand;

	size = ReadAll (pData, pFileOut);

	if (bNullTerminate) 
	{
		size++;
		pData[pFileOut->nLen]=0;
	}
	return pFileOut;

quit_hand:
	Close (pFileOut);
	return NULL;
}

void CP3DFileSystem::UnLoad (FSFILE* pFileToUnload, BYTE *pData)
{
	Close (pFileToUnload);
	if (pData) delete[] pData;
}

size_t CP3DFileSystem::Read(void* data, size_t size, size_t count, FSFILE* fp)
{
	if (count==0 || size==0) return 0;
	// Pokud zadana velikost presahuje veliksot souboru, nastav velikost tak aby byla presne.
	// To proto aby se v pripade archivu necetl dalsi soubor nasledujici za timto!
	if (fp->nRelOffset+(size*count) > fp->nLen) 
	{
		if (count==1)
		{
			size = fp->nLen-fp->nRelOffset;
			count = 1;
		}
		else if (size==1)
		{
			size = 1;
			count = fp->nLen-fp->nRelOffset;
		}
		else
		{
			CON(MSG_CON_ERR, "FS: CP3DFileSystem::Read() File size exceeded for file %s! Trying size=1 count=OffsetToFileLen!", fp->pSysFile);
			size = 1;
			count = fp->nLen-fp->nRelOffset;
		}
	}

	if (fp->bArchive) fseek(fp->pSysFile, fp->nOffset+fp->nRelOffset, SEEK_SET);
	size_t posun = fread(data, size, count, fp->pSysFile);

	if (fp->bArchive) BC71((BYTE*)data, fp->nRelOffset, (unsigned long)posun*size, fp->nLen); //desifruj

	fp->nRelOffset+=(unsigned long)posun*size; // pocet prectenych bytu = pocet prectenych bloku * velikost bloku

	return posun;
}

void CP3DFileSystem::Seek(FSFILE *fp, long pos, int type)
{
	if (type==SEEK_CUR)
		fp->nRelOffset += pos;
	else if(type==SEEK_END)
		fp->nRelOffset = fp->nLen + pos;
	else // SEEK_SET
		fp->nRelOffset = pos;

	if (fp->nRelOffset > fp->nLen)
	{
		CON(MSG_CON_ERR, "FS: CP3DFileSystem::Seek() File size exceeded! Pos=0");
		fp->nRelOffset = 0;
	}

	fseek(fp->pSysFile, fp->nOffset+fp->nRelOffset, SEEK_SET);
}

int CP3DFileSystem::Eof(FSFILE *fp)
{
	if (!fp) return -1;
	if (fp->nRelOffset>=fp->nLen) // FIXME: tohle je EOF?
		return 1;

	return 0;
}

size_t CP3DFileSystem::Write(const void *data, size_t size, size_t count, FSFILE *fp)
{
	if (count==0 || size==0) return 0;
	if (fp->bArchive) return 0; // WRITE:

	size_t posun = fwrite(data, size, count, fp->pSysFile);
	fp->nRelOffset+=(unsigned long)posun*size; // pocet prectenych bytu = pocet prectenych bloku * velikost bloku

	return posun;
}

int CP3DFileSystem::Flush(FSFILE *fp)
{
	if (fp->bArchive) return 0; // WRITE: flush se pouziva jen u zapisu=nepouzivat pro archiv do ktereho se nezapisuje?
	return fflush(fp->pSysFile);
}

long CP3DFileSystem::Tell(FSFILE *fp)
{
	if (fp->nRelOffset > fp->nLen) return -1L;

	return fp->nRelOffset;
}

int CP3DFileSystem::Getc(FSFILE *fp)
{
	if (fp->nRelOffset >= fp->nLen) return -1;

	if (fp->bArchive) fseek(fp->pSysFile, fp->nRelOffset, SEEK_SET);
	int ret = fgetc(fp->pSysFile);
	if (ret!=-1) 
	{
		if (fp->bArchive) BC71((BYTE*)&ret, fp->nRelOffset, 1, fp->nLen); //desifruj
		fp->nRelOffset++; // pokud nebylo EOF nebo neco podobneho
	}

	return ret;
}

int CP3DFileSystem::Putc(int chr, FSFILE *fp)
{
	if (fp->bArchive) return -1; // WRITE:

	int ret = fputc(chr, fp->pSysFile);
	if (ret!=-1) fp->nRelOffset++; // pokud nenastala chyba

	return ret;
}

int CP3DFileSystem::Puts(const char* string, FSFILE* fp)
{
	if (fp->bArchive) return -1; // WRITE:

	long pos = ftell(fp->pSysFile);
	int ret = fputs(string, fp->pSysFile);

	if (ret!=-1) fp->nRelOffset+=(ftell(fp->pSysFile)-pos); // pokud nenastala chyba

	return ret;
}

unsigned long CP3DFileSystem::ReadAll(void* data, FSFILE* fp)
{
	//long oldPos = Tell(fp);
	Seek(fp, 0, SEEK_SET);

	ULONG retVar = (ULONG)Read(data, 1, fp->nLen, fp);
	//((byte*)data)[fp->nLen-1]=0;

	//Seek(fp, oldPos, SEEK_SET);

	return retVar;
}

int CP3DFileSystem::Exists(const char* file)
{
	char szFilePath[256]; // nyni se pracuje s timto "vstupnim" retezcem

	// uprav vstupni parametry
	strcpy(szFilePath, file);
	CorrectPath(szFilePath);

	// zkontroluj klasicky v stdio
	struct stat mystat;
	if (!stat(szFilePath,&mystat)) return 1;

	// zkontroluj existenci v archivech
	FSFILE* fp = NULL;
	if((fp = Open(file, "rb", true)) != NULL)
	{
		Close(fp);
		return 2;
	}
	return 0;
}

int CP3DFileSystem::TryLoadArchive(const char* szFilePath)
{
	char szArchivePath[256];
	strcpy(szArchivePath, szFilePath);
	GetArchivePath(szArchivePath);

	FILE *pSysFile = fopen(szArchivePath, "rb");

	if(pSysFile)
	{
		// pokus se najit jiz nacteny archiv
		int i;
		for (i=0; i<MAX_ARCHIVES; i++) 
			if(g_pArchives[i]) 
				if (!strcmpi(g_pArchives[i]->archiveName, szArchivePath))
				{
					// nalezeno
					fclose(pSysFile); // uz netreba, byl nalezen otevreny
					return i;
				}

				// NENALEZEN JIZ NACTENY, nacti struktury a ziskej soubor

				// zkontroluj archiv
				PACK_HEADER header;
				fread(&header, 1, sizeof(PACK_HEADER), pSysFile);
				if (strcmp(header.packPrefix, "P3DPack_1.1 :-C"))
				{
					CON(MSG_CON_ERR, "FS: (%s) Bad 7pk archive or archive version!", szArchivePath);
					fclose(pSysFile);
					return -1;
				}
				// test desifrace
				BC71((BYTE*)header.packKeyTest, 0, 4, 4); //desifruj
				if (strcmp(header.packKeyTest, ":-)"))
				{
					CON(MSG_CON_ERR, "FS: (%s) Bad 7pk archive key or cypher!", szArchivePath);
					fclose(pSysFile);
					return -1;
				}

				// najdi volne cislo archivu - index pole g_pArchives
				int archiveID=-1;
				for (i=0; i<MAX_ARCHIVES; i++) 
					if(!g_pArchives[i]) 
					{archiveID = i; break;}
					if (archiveID==-1) { CON(MSG_CON_ERR, "FS: MAX_ARCHIVES exceeded!"); return NULL;}

					if (g_pConsole) CON(MSG_CON_INFO, "FS: Archive %s was loaded", szArchivePath);

					g_pArchives[archiveID] = new FS_ARCHIVE();
					strcpy(g_pArchives[i]->archiveName, szArchivePath);
					g_pArchives[i]->pArchiveFile = pSysFile;

					// ok... mame archiv... neni vyhrane, musime v archivu najit soubory a pozice
					// nacti adresare
					g_pArchives[archiveID]->u7pk_numdirs = (UINT)header.entries[lDirs].length / sizeof(DIRECTORY_INFO);
					g_pArchives[archiveID]->u7pk_dirs = new DIRECTORY_INFO[g_pArchives[archiveID]->u7pk_numdirs];
					fseek(pSysFile, (long)header.entries[lDirs].offset, SEEK_SET);
					fread(g_pArchives[archiveID]->u7pk_dirs, g_pArchives[archiveID]->u7pk_numdirs, sizeof(DIRECTORY_INFO), pSysFile);
					// nacti soubory
					g_pArchives[archiveID]->u7pk_numfiles = (UINT)header.entries[lFiles].length / sizeof(FILE_INFO);
					g_pArchives[archiveID]->u7pk_files = new FILE_INFO[g_pArchives[archiveID]->u7pk_numfiles];
					fseek(pSysFile, (long)header.entries[lFiles].offset, SEEK_SET);
					fread(g_pArchives[archiveID]->u7pk_files, g_pArchives[archiveID]->u7pk_numfiles, sizeof(FILE_INFO), pSysFile);

					return archiveID;
	}
	else
	{
		return -1;
	}
}

HANDLE CP3DFileSystem::FindFirstFile( IN const char* lpFileName, OUT LPWIN32_FIND_DATAA lpFindFileData )
{
	char szFilePath[256]; // nyni se pracuje s timto "vstupnim" retezcem
	// uprav vstupni parametry
	strcpy(szFilePath, lpFileName);
	CorrectPath(szFilePath);

	// ******* ARCHIVE ********
	// pokus se nacist/najit nacteny archiv
	int nArchiveID = TryLoadArchive(szFilePath);
	if (nArchiveID<0)
	{
		// soubor v archivu neexistuje, zkus stdio
		goto STDIO;
	}


	// ziskej promenne
	DIRECTORY_INFO* pDi = g_pArchives[nArchiveID]->u7pk_dirs;
	UINT numDirs = g_pArchives[nArchiveID]->u7pk_numdirs;
	FILE_INFO* pFi = g_pArchives[nArchiveID]->u7pk_files;
	UINT numFiles = g_pArchives[nArchiveID]->u7pk_numfiles;

	// preved opravenou cestu tak aby neobsahovala data/podslozku/
	for(UINT i=6;i<(UINT)strlen(szFilePath);i++)
	{
		if (szFilePath[i]=='/')
		{
			// ok... nyni je cesta &newPath[i+1] tak prohledej archiv a najdi ID slozky
			DIRECTORY_INFO* pDir = GetDirectory(szFilePath, pDi, numDirs); // nemazat pointer!
			if (!pDir) 
			{
				return INVALID_HANDLE_VALUE;
			}
			// ok... nyni mame slozku. Je potreba najit soubor s danym nazvem nachazejici se ve slozce s id pDir->dirID
			char szFileName[MAX_FILE_DIR_NAME];
			//najdi nazev souboru
			for(UINT ii=(UINT)strlen(szFilePath)-1;ii>0;ii--)
			{
				if (szFilePath[ii]=='/')
				{
					strcpy(szFileName, &szFilePath[ii+1]);
					break;
				}
			}

			for(UINT nFil=0;nFil<numFiles;nFil++)
			{
				if (pFi[nFil].dirID == pDir->dirID && wildcmp(szFileName, pFi[nFil].filName))
				{
					// vypln handle
					FS_FINDINFO *pHandle = new FS_FINDINFO();
					pHandle->dirID = pDir->dirID;
					pHandle->pDi = pDi;
					pHandle->numDirs = numDirs;
					pHandle->pFi = pFi;
					pHandle->numFiles = numFiles;
					pHandle->numMatches=1;
					pHandle->hStdio = NULL;
					strcpy(pHandle->szFileName, szFileName);

					// FIXME: Nejak zrychlit nebo neco
					strcpy(szFilePath, lpFileName);
					CorrectPath(szFilePath);
					strcpy(pHandle->szFilePathName, szFilePath);

					// vypln lpFindFileData (TODO: zatim jen nazev souboru, je treba vic?)
					strcpy(lpFindFileData->cFileName, pFi[nFil].filName);
					lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_PACK;

					return pHandle;
				}
			}
		}
	}

STDIO:

	// ******* STDIO ********
	// uprav vstupni parametry
	strcpy(szFilePath, lpFileName);
	CorrectPath(szFilePath);

	int nValid;
	HANDLE searchHandle=::FindFirstFile(szFilePath, lpFindFileData);
	nValid = (searchHandle == INVALID_HANDLE_VALUE) ? 0 : 1;
	if (nValid)
	{
		// vypln handle
		FS_FINDINFO *pHandle = new FS_FINDINFO();
		pHandle->hStdio = searchHandle;
		pHandle->numMatches = 1;
		pHandle->szFilePathName[0] = 0;

		return pHandle;
	}

	return INVALID_HANDLE_VALUE;
}


BOOL CP3DFileSystem::FindNextFile( IN HANDLE hFindFile, OUT LPWIN32_FIND_DATAA lpFindFileData )
{
	FS_FINDINFO* pHandle = (FS_FINDINFO*)hFindFile;
	if (!pHandle || pHandle==INVALID_HANDLE_VALUE) return FALSE;

	// ******* STDIO V PRIPADE POKRACOVANI ********
	if (pHandle->hStdio)
	{
		return ::FindNextFile(pHandle->hStdio, lpFindFileData);
	}

	// ******* ARCHIVE ********

	unsigned int numMatches=0;

	for(UINT nFil=0;nFil<pHandle->numFiles;nFil++)
	{
		if (pHandle->pFi[nFil].dirID == pHandle->pDi->dirID && wildcmp(pHandle->szFileName, pHandle->pFi[nFil].filName))
		{
			numMatches++;

			if (numMatches > pHandle->numMatches)
			{
				// vypln handle
				pHandle->numMatches=numMatches;

				// vypln lpFindFileData (TODO: zatim jen nazev souboru, je treba vic?)
				strcpy(lpFindFileData->cFileName, pHandle->pFi[nFil].filName);
				lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_PACK;

				return TRUE;
			}
		}
	}

	// ******* STDIO V PRIPADE KONCE ARCHIVE ********
	pHandle->hStdio = ::FindFirstFile(pHandle->szFilePathName, lpFindFileData);
	if (pHandle->hStdio == INVALID_HANDLE_VALUE) 
		return FALSE; // pokud neni v stdio
	else
		return TRUE; // je stdio
}

BOOL CP3DFileSystem::FindClose(IN OUT HANDLE hFindFile )
{
	if (!hFindFile || hFindFile==INVALID_HANDLE_VALUE) return FALSE;
	FS_FINDINFO* pHandle = (FS_FINDINFO*)hFindFile;
	::FindClose(pHandle->hStdio);
	delete pHandle;
	return TRUE;
}
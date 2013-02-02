//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (31.7.2006,13.1.2006)
// Purpose:	Error handling and reporting functions
//-----------------------------------------------------------------------------
#include "exception.h"
#include "zlib_inc.h"

#include <shellapi.h>
#include <shlobj.h>
#include <stdio.h>
#include <wininet.h>

#pragma warning (disable: 4312)

bool GetErrorModuleInfo(EXCEPTION_POINTERS* pExceptionPointers, OUT char* outStr, DWORD sizeStr, DWORD* relAddress)
{
	BOOL EBP_OK = TRUE;
	DWORD EBP = pExceptionPointers->ContextRecord->Ebp;
	DWORD EIP = pExceptionPointers->ContextRecord->Eip;

	do
	{
		// Check if EBP is a good address
		// I'm expecting a standard stack frame, so
		// EPB must be aligned on a DWORD address
		// and it should be possible to read 8 bytes
		// starting at it (next EBP, caller).
		if( (DWORD)EBP & 3 )
			EBP_OK = FALSE ;
		if( EBP_OK && IsBadReadPtr( (void*)EBP, 8 ) )
			EBP_OK = FALSE ;
		if( EBP_OK )
		{
			BYTE* caller = EIP ? (BYTE*)EIP : *((BYTE**)EBP + 1) ;
			EBP = EIP ? EBP : *(DWORD*)EBP ;
			if( EIP )
				EIP = 0 ; // So it is considered just once
			// Get the instance handle of the module where caller belongs to
			MEMORY_BASIC_INFORMATION mbi ;
			VirtualQuery( caller, &mbi, sizeof( mbi ) ) ;
			// The instance handle is equal to the allocation base in Win32
			HINSTANCE hInstance = (HINSTANCE)mbi.AllocationBase ;
			// If EBP is valid, hInstance is not 0
			if( hInstance == 0 )
				EBP_OK = FALSE ;
			else
			{
				GetModuleFileName(hInstance, outStr, sizeStr);
				*(relAddress) = (DWORD)(caller - (BYTE*)hInstance);
				return true;
			}
		}
		else
			break ; // End of the call chain
	}
	while( TRUE ) ;
	return false;
}

bool HttpQuery(const char* params, char* outResult, int resultStrLen)
{
	char *query = new char[strlen(params)+64];

	sprintf(query, "http://bug.7thsquad.com/?%s", params);
	//do
	//{
		HINTERNET hInet = InternetOpen("P3D", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
		HINTERNET hFile = InternetOpenUrl(hInet, query, NULL, 0, INTERNET_FLAG_RELOAD, 0);
		DWORD size;
		outResult[0]=0;
		InternetReadFile(hFile, outResult, resultStrLen, &size);
		outResult[size]=0;
		InternetCloseHandle(hFile);
		InternetCloseHandle(hInet);
	//}while(outResult[0]==0);
	delete[] query;
	return true;
}

int GetFileInfo(const char *filepath, 
				const char *fileinfo, char *fileinfovalue)
{
	DWORD whandle;
	DWORD rsize;
	int retval;
	unsigned char *buffer=NULL;

	strcpy(fileinfovalue, "n/a");

	rsize=GetFileVersionInfoSize(filepath, &whandle);
	if (rsize>0)
	{
		buffer = new unsigned char[rsize];
		if (GetFileVersionInfo(filepath, 
			whandle, rsize, buffer))
		{
			unsigned short *subBlock;
			unsigned int len = 0;
			if (VerQueryValue(buffer, 
				"\\VarFileInfo\\Translation", (void **)&subBlock, &len))
			{
				char spv[256];
				char *versionInfo;
				len=0;

				sprintf(spv, "\\StringFileInfo\\%04x%04x\\%s", 
					subBlock[0], subBlock[1], fileinfo);
				if (VerQueryValue(buffer, spv, (void *
					*)&versionInfo, &len))
				{
					strncpy(fileinfovalue, versionInfo, len);
					retval=0;
				}
				else
					retval=1;
			}
			else
				retval=2;
		}
		else
			retval=3;
	}
	else
		retval=4;

	if (buffer!=NULL) delete[] buffer;
	return retval;
}

int HandleException(EXCEPTION_POINTERS* pExceptionPointers)
{
  try
  {
	DWORD eCode = pExceptionPointers->ExceptionRecord->ExceptionCode;
	//
	char eName[64]="N/A"; char eDesc[512]="N/A"; char eMsg[448]="N/A";
	#define eDESC(errNameStr, errDescStr) strcpy (eName, errNameStr); strcpy (eDesc, errDescStr); break
	//
	switch(eCode)
	{
		case EXCEPTION_SINGLE_STEP: eDESC("Single step", "A trace trap or other single-instruction mechanism signaled that one instruction has been executed.");
		case EXCEPTION_ACCESS_VIOLATION:// eDESC("Acces violation", "The thread tried to read from or write to a virtual address for which it does not have the appropriate access.");
			strcpy(eName, "Access violation");
			sprintf(eDesc, "The thread tried to %s to a virtual address 0x%X.", pExceptionPointers->ExceptionRecord->ExceptionInformation[0]?"write":"read", pExceptionPointers->ExceptionRecord->ExceptionInformation[1]?pExceptionPointers->ExceptionRecord->ExceptionInformation[1]:0);
			break;
		case EXCEPTION_IN_PAGE_ERROR: eDESC("Page error", "The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.");
		case EXCEPTION_ILLEGAL_INSTRUCTION: eDESC("Illegal instruction", "The thread tried to execute an invalid instruction.");
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: eDESC("Noncontinuable exception", "The thread tried to continue execution after a noncontinuable exception occurred.");
		case EXCEPTION_INVALID_DISPOSITION: eDESC("Invalid disponsition", "An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.");
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: eDESC("Array bounds exceeded", "The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.");
		case EXCEPTION_FLT_DENORMAL_OPERAND: eDESC("Float denormal operand", "One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.");
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: eDESC("Float division by zero", "The thread tried to divide a floating-point value by a floating-point divisor of zero.");
		case EXCEPTION_FLT_INEXACT_RESULT: eDESC("Float inexact result", "The result of a floating-point operation cannot be represented exactly as a decimal fraction.");
		case EXCEPTION_FLT_INVALID_OPERATION: eDESC("Float invalid operation", "Other floating-point exception.");
		case EXCEPTION_FLT_STACK_CHECK: eDESC("Float stack check failed", "The stack overflowed or underflowed as the result of a floating-point operation.");
		case EXCEPTION_FLT_UNDERFLOW: eDESC("Float stack underflow", "The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.");
		case EXCEPTION_INT_DIVIDE_BY_ZERO: eDESC("Integer division by zero", "The thread tried to divide an integer value by an integer divisor of zero.");
		case EXCEPTION_INT_OVERFLOW: eDESC("Integer overflow", "The result of an integer operation caused a carry out of the most significant bit of the result.");
		case EXCEPTION_PRIV_INSTRUCTION: eDESC("Instruction not allowed", "The thread tried to execute an instruction whose operation is not allowed in the current machine mode.");
		case EXCEPTION_STACK_OVERFLOW: eDESC("Stack overflow", "The thread used up its stack.");
		case EXCEPTION_INVALID_HANDLE: eDESC("Invalid Handle", "");
		case EXCEPTION_BREAKPOINT: eDESC("Breakpoint", "A breakpoint was encountered.");
		case EXCEPTION_DATATYPE_MISALIGNMENT: eDESC("Datatype misalignment", "The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.");
		//
		case STATUS_WAIT_0: eDESC("Wait 0", "");
		case STATUS_ABANDONED_WAIT_0: eDESC("Abandoned wait 0", "");
		case STATUS_USER_APC: eDESC("User APC", "");
		case STATUS_TIMEOUT: eDESC("Timeout", "");
		case STATUS_PENDING: eDESC("Pending", "");
		case STATUS_SEGMENT_NOTIFICATION: eDESC("Segment notification", "");
		case STATUS_GUARD_PAGE_VIOLATION: eDESC("Guard page violation", "");
		case STATUS_NO_MEMORY: eDESC("No memory", "");
		case STATUS_CONTROL_C_EXIT: eDESC("Control C Exit", "");
		case STATUS_FLOAT_MULTIPLE_FAULTS: eDESC("Float Multiple Faults", "");
		case STATUS_FLOAT_MULTIPLE_TRAPS: eDESC("Float Multiple Traps", "");
		default:
			strcpy(eName, "Unknown exception");
			sprintf(eDesc, "An unknown exception with code %d has been thrown.", eCode);
			break;
	}
	
	char eModNameOrig[MAX_PATH]="N/A";
	char eModName[64]="N/A";
	DWORD eModAddr = 0;
	if (GetErrorModuleInfo(pExceptionPointers, eModNameOrig, MAX_PATH, &eModAddr))
	{
		for(int si = (int)strlen(eModNameOrig)-1; si > -1; si--)
		{
			if (eModNameOrig[si] == '/' || eModNameOrig[si] == '\\') 
			{
				strcpy(eModName,eModNameOrig+si+1);
				break;
			}
		}
	}

	// sloz zpravu
	int msgRet = IDNO; // standardne ne
	#ifdef USE_MINIDUMP
		sprintf(eMsg, "Unexpected error occured!\r\n\r\nModule: %s\r\nModule address: %#.8x\r\n\r\nException: %s\r\nException description: %s\r\n\r\nPlease click yes to sent us an anonymous error information.\r\nIt doesn't contain any private information and is used to improve our software.", eModName, eModAddr, eName, eDesc, eModName);
		msgRet = MessageBox(NULL, eMsg, "FATAL ERROR!", MB_SYSTEMMODAL | MB_ICONSTOP | MB_YESNO);
	#else
		sprintf(eMsg, "Unexpected error occured!\r\n\r\nModule: %s\r\nModule address: %#.8x\r\n\r\nException: %s\r\nException description: %s", eModName, eModAddr, eName, eDesc, eModName);
		MessageBox(NULL, eMsg, "FATAL ERROR!", MB_SYSTEMMODAL | MB_ICONSTOP);
	#endif

	// otevri konzoli pokud je to debug verze
	#ifdef _DEBUG
		ShellExecute(NULL, "open", "console.log", "", "", 1);
	#endif

	// generuj minidump a posli ho
	#ifdef USE_MINIDUMP
		if (msgRet == IDYES)
		{
			// ziskej verzi modulu
			char ver[32]="";
			char eModVer[16]; int cv=0;
			GetFileInfo(eModNameOrig, "FileVersion", ver);
			// z x, x, x, x udelej x.x.x.x
			for(unsigned int vv=0;vv<strlen(ver);vv++)
			{
				if (ver[vv]==',')
					eModVer[cv] = '.';
				else
					eModVer[cv] = ver[vv];

				if (ver[vv]!=' ') cv++;
			}
			eModVer[cv]=0;

			// vytvor dotaz na server zda jiz toto neexistuje
			char buffer[512]="ERR:Can't connect to server!";
			sprintf(buffer, "a=u&mo=%s&ve=%s&ad=%d&ex=%u", eModName, eModVer, eModAddr, eCode);
			HttpQuery(buffer, buffer, 512);
			if (buffer[0]=='E')
			{
				MessageBox(NULL, &buffer[4], "Error sending report", MB_SYSTEMMODAL | MB_ICONSTOP);
				goto UKLID;
			}
			else if (buffer[4]=='E') // OK::EXI (uz existuje)
			{
				//OK::EXI:N/A --- "Thank you, it's a known bug, but we haven't time to fix it, please wait..."
				//OK::EXI:NOP:<desc> --- "Thank you, it's a fixed bug, please wait for a new update/patch...\r\n\r\nBug description:\r\n%s"
				//OK::EXI:FIX:<updateNumber> -- "Thank you, it's a fixed bug!\r\nA browser window with update/patch will be opened..."
				switch(buffer[9])
				{
				case '/':
					MessageBox(NULL, "Thank you for your report, it's a known bug, but we haven't time to fix it, please wait...", "Server response", MB_SYSTEMMODAL | MB_ICONINFORMATION);
					goto UKLID;
				case 'O':
					char desc[512];
					sprintf(desc, "Thank you for your report, it's a known bug, please wait for a new update/patch...\r\n\r\nBug description:\r\n%s", &buffer[12]);
					MessageBox(NULL, desc, "Server response", MB_SYSTEMMODAL | MB_ICONINFORMATION);
					goto UKLID;
				case 'I':
					char qu[512];
					sprintf(qu, "http://bug.7thsquad.com/?a=g&up=%s", &buffer[12]);
					MessageBox(NULL, "Thank you for your report, it's a fixed bug!\r\nA browser window with update/patch will be opened...", "Server response", MB_SYSTEMMODAL | MB_ICONINFORMATION);
					ShellExecute(NULL, "open", qu, "", "", 1);
					goto UKLID;
				}
			}

			/// /// /// /// NOVA CHYBA => VYGENERUJ MINIDUMP A VYTVOR ZIP /// /// /// ///
			char fName[MAX_PATH]="-";
			FILE *fp; FILE *fpConsole;

			// vybeneruj minidump
			GenerateMinidump(pExceptionPointers, fName);
			if (fName[0]=='-') goto UKLID; // ukonci aplikaci

			// vytvor zip soubor vloz do nej konzoli a minidump
			fp = fopen(fName, "rb");
				if (!fp) goto UKLID; // ukonci aplikaci

			fpConsole = fopen("console.log", "rb");
				if (!fpConsole) goto UKLID; // ukonci aplikaci

			strcat(fName, ".zip");
			zipFile zip = zipOpen(fName, 0);

			zipAddFileToZip(zip, "console.log", fpConsole);
			zipAddFileToZip(zip, "minidump.dmp", fp);

			fclose(fp);
			fclose(fpConsole);
			zipClose(zip, NULL);

			/// /// /// /// UPLOADNI TO NA FTP /// /// /// ///
			char shortFName[128];
			for (int ss=(int)strlen(fName)-1;ss>=0;ss--)
			{
				if (fName[ss]=='/' || fName[ss]=='\\')
				{
					strcpy(shortFName, &fName[ss+1]);
					break;
				}
			}

			HINTERNET hopen = InternetOpen("P3D", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
			//InternetSetStatusCallback(hopen, (INTERNET_STATUS_CALLBACK)InetCallback);
			HINTERNET hcon = InternetConnect(hopen, "ftp.7thsquad.com", INTERNET_DEFAULT_FTP_PORT, "7thsquad_com_bug", "BuggyFTP", INTERNET_SERVICE_FTP, 0, 1);
			//FtpSetCurrentDirectory(hcon, "test");
			if(!FtpPutFile(hcon, fName/*local*/, shortFName/*remote*/, FTP_TRANSFER_TYPE_BINARY, 1))
			{
				MessageBox(0, "Can't upload report pack with error data!", "Error sending report", MB_ICONSTOP | MB_SYSTEMMODAL);
				InternetCloseHandle(hcon);
				InternetCloseHandle(hopen);
				goto UKLID;
			}
			InternetCloseHandle(hcon);
			InternetCloseHandle(hopen);

			/// /// /// /// INFORMUJ O TOM HTTP SERVER /// /// /// ///
			sprintf(buffer, "a=u&mo=%s&ve=%s&ad=%d&ex=%u&fn=%s", eModName, eModVer, eModAddr, eCode, shortFName);
			HttpQuery(buffer, buffer, 512);
			if (buffer[0]=='E')
			{
				MessageBox(NULL, &buffer[4], "Error sending report", MB_SYSTEMMODAL | MB_ICONSTOP);
				goto UKLID;
			}
			else if (buffer[4]=='A') // OK::ADD --- Novy a byl pridan
			{
				MessageBox(NULL, "Thank you for your report, it's a new bug!\r\nIt will be patched as soon as possible...", "Thank you!", MB_SYSTEMMODAL | MB_ICONINFORMATION);
				goto UKLID;
			}

UKLID:

			// smaz zip a minidump
			DeleteFile(fName);
			fName[strlen(fName)-4]=0;
			DeleteFile(fName);
		}
	#endif

	exit(EXIT_FAILURE); // ukonci aplikaci

	return EXCEPTION_EXECUTE_HANDLER;
  }
  catch (...)
  {
	MessageBox(0, "Error generating error report.\r\nThis error can't be send!\r\n\r\nQuitting...", "FATAL ERROR!", MB_SYSTEMMODAL | MB_ICONSTOP);
	exit(-1); // chyba behem error reportu
  }
}

int GenerateMinidump(EXCEPTION_POINTERS* pExceptionPointers, char* dmpFileName)
{
#ifdef USE_MINIDUMP // pouzij minidump
	BOOL bMiniDumpSuccessful;
	char szPath[1024]; 
	char szFileName[MAX_PATH]; 
	char* szAppName = "P3DMinidump";
	DWORD dwBufferSize = MAX_PATH;
	HANDLE hDumpFile;
	SYSTEMTIME stLocalTime;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	GetLocalTime( &stLocalTime );
	GetCurrentDirectory(dwBufferSize, szPath );
	if (szPath[strlen(szPath)-1]=='\\' || szPath[strlen(szPath)-1]=='/') szPath[strlen(szPath)-1] = 0;

	sprintf( szFileName, "%s\\%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", 
		szPath, szAppName, 
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
		GetCurrentProcessId(), GetCurrentThreadId());
	hDumpFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	ExpParam.ThreadId = GetCurrentThreadId();
	ExpParam.ExceptionPointers = pExceptionPointers;
	ExpParam.ClientPointers = TRUE;

	bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
		hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

	if (bMiniDumpSuccessful)
	{
		strcpy(dmpFileName, szFileName);
	}
	else
	{
		strcpy(dmpFileName, "-MINIDUMP-FAILED-TO-CREATE-");
	}
	CloseHandle(hDumpFile);
#endif
	return EXCEPTION_EXECUTE_HANDLER;
}
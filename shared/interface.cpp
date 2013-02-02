//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Naèítání DLL modulù, získávání ukazatelù na tøídy (klas. i singl.).
//			Pouze pro Windows.
// Notes: Je ale dùležité sledovat co je singleton a neukonèovat ho...
//		  Není to ošetøeno, musíte na to dávat pozor, to snad není tak tìžké.
//-----------------------------------------------------------------------------

#include "interface.h"
#include "string"

CClassManager *g_pClsMgr=NULL;
static char* s_szModuleName=NULL;

//-----------------------------------------------------------------------------
// CLASS MANAGER
//----------------------------------------------------------------------------- 

//pomocna funkce na vypis textu
void DbgPrint(WORD objSize, const char* text, ...)
{
#ifdef _DEBUG
	if (!s_szModuleName) return;

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
		return;
	}

	SYSTEMTIME st;
	GetSystemTime(&st);

	char* buffer2=new char[strlen(buffer)+50];
	if (objSize!=0){
		bool bMinus = objSize<0?true:false;
		if (bMinus) objSize = -objSize;
		char memInfo[16];
		if (objSize<1024) sprintf(memInfo, "%dB", objSize);
		else if (objSize<1048576) sprintf(memInfo, "%.2fkB", objSize/1024.0f);
		else if (objSize<1073741824) sprintf(memInfo, "%.2fMB", objSize/1073741824.0f);
		else sprintf(memInfo, "TOO BIG");

		if (bMinus)
			sprintf(buffer2, "[%d:%d:%d] [%s] [-%9s] : %s\r\n", st.wHour, st.wMinute, st.wSecond, s_szModuleName, memInfo, buffer);
		else
			sprintf(buffer2, "[%d:%d:%d] [%s] [ %9s] : %s\r\n", st.wHour, st.wMinute, st.wSecond, s_szModuleName, memInfo, buffer);
	}else{
		sprintf(buffer2, "[%d:%d:%d] [%s] [          ] : %s\r\n", st.wHour, st.wMinute, st.wSecond, s_szModuleName, buffer);
	}


	g_pClsMgr->PrintFile(buffer2, strlen(buffer2));

	delete[] buffer2;
#endif
}

CClassManager::CClassManager()
{
	m_pLastHolder=NULL;
	m_nNumSingletons=0;
	m_nNumModules=0;

	m_clsFactory = NULL;
}

void CClassManager::SetClassFactory(ClassFactoryFn ffn)
{
	m_clsFactory = ffn;
}

bool CClassManager::LoadNewModule(const char* szMouleName)
{
	char dllPath[MAX_PATH];
	sprintf(dllPath, "%s\\%s", m_szDllPath, szMouleName);

	HMODULE hDLL = LoadLibrary(dllPath);
	if (!hDLL) // neni exe ani dll, preskoc na dalsi soubor
	{
		return false;
	}
	ModuleEntrypoint dllEntry = (ModuleEntrypoint)GetProcAddress(hDLL, ENTRYPOINT_NAME_STR);
	if (dllEntry!=NULL) 
	{
		if (m_nNumModules == MAX_MODULES-1)
		{
			MessageBox(0, "Maximum number of project modules exceeded!", "MAX_MODULES",  MB_SYSTEMMODAL);
			return false;
		}

		DbgPrint(0, "LOADING : Loading dll module '%s'", szMouleName);

		m_modules[m_nNumModules] = hDLL; // zkopiruj dll handle

		// nastav tomuto modulu ukazatel na tuto instanci ClsManagera
		dllEntry(this);

		// zkopiruj ukazatel na posledni holder
		if (!m_pLastHolder) 
			m_pLastHolder = dllEntry(NULL);
		else
		{
			ClassHolder *pTmpHold;
			for(pTmpHold=m_pLastHolder; pTmpHold->m_pNext;pTmpHold=pTmpHold->m_pNext) 0;
			pTmpHold->m_pNext = dllEntry(NULL);
		}

		m_nNumModules++;
		return true;
	}
	else
	{
		DbgPrint(0, "LOADING : Wrong entrypoint in module '%s'", szMouleName);
		FreeLibrary(hDLL);
		return false;
	}
}

void CClassManager::PrintFile(const char* data, size_t dataSize)
{
/*	EnterCriticalSection(&s_cs);
	m_debugLines = m_debugLines + data;
	LeaveCriticalSection(&s_cs);*/
}

void CClassManager::Initialize(const char *szDllPath)
{
	m_debugLines = "";
	DbgPrint(0, "INITIALIZE : Loading dll modules from '%s'", szDllPath);

	m_pLastHolder = ClassHolder::g_pLastHolder;

	m_nNumModules=0;

	// PROJDI VSECHNY DLL A ZJISTI KTERE PATRI K PROJEKTU A ULOZ JE
	int nValid;
	HANDLE searchHandle=NULL;
	WIN32_FIND_DATA fileData;

	strcpy(m_szDllPath, szDllPath);

	char pathToSearch[MAX_PATH];
	strcpy(pathToSearch,szDllPath);
	if (pathToSearch[strlen(pathToSearch)-1]!='\\' && pathToSearch[strlen(pathToSearch)-1]!='/') strcat(pathToSearch, "\\");
	strcat(pathToSearch, "*.dll");

	searchHandle = FindFirstFile(pathToSearch, &fileData);
	nValid = (searchHandle == INVALID_HANDLE_VALUE) ? 0 : 1;

	while (nValid)
	{
		if (strcmpi (fileData.cFileName, ".") != 0 &&
			strcmpi (fileData.cFileName, "..") != 0)
		{
			// zjisti zda je to knihovna z projektu
			char dllPath[MAX_PATH];
			strcpy(dllPath, szDllPath);
			if (dllPath[strlen(dllPath)-1]!='\\' && dllPath[strlen(dllPath)-1]!='/') strcat(dllPath, "\\");
			strcat(dllPath, fileData.cFileName);

			HMODULE hDLL = LoadLibrary(dllPath);
			if (!hDLL) // neni exe ani dll, preskoc na dalsi soubor
			{
				nValid = FindNextFile(searchHandle, &fileData);
				continue;
			}
			ModuleEntrypoint dllEntry = (ModuleEntrypoint)GetProcAddress(hDLL, ENTRYPOINT_NAME_STR);
			if (dllEntry!=NULL) 
			{
				if (m_nNumModules == MAX_MODULES-1)
				{
					MessageBox(0, "Maximum number of project modules exceeded!", "MAX_MODULES",  MB_SYSTEMMODAL);
					return;
				}

				DbgPrint(0, "LOADING : Loading dll module '%s'", fileData.cFileName);

				m_modules[m_nNumModules] = hDLL; // zkopiruj dll handle

				// nastav tomuto modulu ukazatel na tuto instanci ClsManagera
				dllEntry(this);

				// zkopiruj ukazatel na posledni holder
				if (!m_pLastHolder) 
					m_pLastHolder = dllEntry(NULL);
				else
				{
					ClassHolder *pTmpHold;
					ClassHolder *pLast;
					pLast = m_pLastHolder;
					for(pTmpHold=m_pLastHolder; pTmpHold->m_pNext && pTmpHold->m_pNext!=m_pLastHolder;pTmpHold=pTmpHold->m_pNext) 0;
					pTmpHold->m_pNext = dllEntry(NULL);
				}

				m_nNumModules++;
			}
			else
				FreeLibrary(hDLL);
		}
		nValid = FindNextFile(searchHandle, &fileData);
	}

	FindClose(searchHandle);

	if (!m_nNumModules)
	{
		MessageBox(0, "Can't load dll modules! Bad path? Modules doesn't exist?", "DLL MODULES", MB_ICONSTOP | MB_SYSTEMMODAL);
		exit(-1);
	}
}

void* CClassManager::GetClass(const char* className)
{
	// ---POKUS SE NAJIT VE VSECH NACTENYCH DLL
	for(ClassHolder *pHold=m_pLastHolder; pHold; pHold=pHold->m_pNext) // projdi všechny tøídy (holdery)
	{
		if(strcmp(pHold->m_clsName, className) == 0) // tøída nalezena
		{
			void* pClass = (void*)pHold->m_clsFn(); //predbezne ziskani ukazatele - nezbytne (stejne ho hledame, je to jedno :))
			if (pHold->m_nID==1 && pHold->m_pClass==NULL) // *singleton* (jeste nevytvoreny)
			{
				DbgPrint(sizeof(pClass), "FIRST INSTANCE : Creating the first instance of singleton '%s'", pHold->m_clsName);

				m_nNumSingletons++; // poradi
				pHold->m_nID=m_nNumSingletons; // nastav poradi
				pHold->m_pClass=pClass;
			}
			else if (pHold->m_nID==1) // vytvoreny singleton
			{
				//DbgPrint("GetClass(): Returning instance of singleton '%s'.\r\n", pHold->m_clsName);
				return pHold->m_pClass;
			}
			else if (!pHold->m_nID) // klasicka trida
			{
				//DbgPrint("GetClass(): Returning instance of class '%s'.\r\n", pHold->m_clsName);
				return pClass;
			}
			if (!pHold->m_pClass) DbgPrint(0, "GET CLASS : Can't get the first instance of singleton '%s'!", pHold->m_clsName);
			return pHold->m_pClass; // nalezeno, vra ukazatel
		}
	}

	// ---POKUS SE NAJIT PRIMO V EXE
	for(ClassHolder *pHold=ClassHolder::g_pLastHolder; pHold; pHold=pHold->m_pNext) // projdi všechny tøídy (holdery) tohodle exe
	{
		if(strcmp(pHold->m_clsName, className) == 0) // tøída nalezena
		{
			void* pClass = (void*)pHold->m_clsFn(); //predbezne ziskani ukazatele - nezbytne (stejne ho hledame, je to jedno :))
			if (pHold->m_nID==1 && pHold->m_pClass==NULL) // *singleton* (jeste nevytvoreny)
			{
				IP3DBaseInterface d;
				DbgPrint(sizeof(pClass), "FIRST INSTANCE : Creating the first instance of singleton '%s'", pHold->m_clsName);

				m_nNumSingletons++; // poradi
				pHold->m_nID=m_nNumSingletons; // nastav poradi
				pHold->m_pClass=pClass;
			}
			else if (pHold->m_nID==1) // vytvoreny singleton
			{
				//DbgPrint("GetClass(): Returning instance of singleton '%s'.\r\n", pHold->m_clsName);
				return pHold->m_pClass;
			}
			else if (!pHold->m_nID) // klasicka trida
			{
				//DbgPrint("GetClass(): Returning instance of class '%s'.\r\n", pHold->m_clsName);
				return pClass;
			}
			if (!pHold->m_pClass) DbgPrint(0, "GET CLASS : Can't get the first instance of singleton '%s'!", pHold->m_clsName);
			return pHold->m_pClass; // nalezeno, vra ukazatel
		}
	}

	// ---POKUS SE NAJIT EXTERNE POMOCI FACTORY FUNKCE (PRO EDITOR)
	// todle poskytuje bud primo game nebo editor (ktery si nacte potrebne exe hry)
	if (m_clsFactory) m_clsFactory(className);

	DbgPrint(0, "GET CLASS : Can't find class '%s'!", className);
	char tmp[512];
	sprintf(tmp, "GetClass(): Can't find class '%s'!", "ERROR!", className);
	MessageBox(0, tmp, "ERROR!", MB_ICONEXCLAMATION);
	return NULL; // nenalezeno 
}

void CClassManager::Shutdown()
{
	DbgPrint(0, "SHUTDOWN : Shutdowning singletons...");

	// ukonci singletony
	for(unsigned int i = m_nNumSingletons; i > 0; i--) // UKONCI SINGLETONY ODZADU
	{
		for(ClassHolder *pHold=m_pLastHolder; pHold; pHold=pHold->m_pNext) // projdi všechny tøídy (holdery)
		{
			if(pHold->m_nID==i) //id vyhovuje aktualnimu poradi
			{
				if (pHold->m_pClass!=NULL) // jeste nebyl ukoncen
				{
					DbgPrint(-(WORD)sizeof(pHold->m_pClass), "SHUTDOWN : Deleting instance of singleton '%s'", pHold->m_clsName);

					delete((IP3DBaseInterface*)pHold->m_pClass); // získej ukazatel na vytvoøený singleton, pøetypuj a ukonèi ho
					pHold->m_pClass=NULL;
				}
				break; // nalezen
			}
		}
	}

	DbgPrint(0, "SHUTDOWN : Checking memory leaks...");
	for(ClassHolder *pHold=m_pLastHolder; pHold; pHold=pHold->m_pNext)
	{
		if(pHold->g_pLastHolder && pHold->m_numInstances>0)
		{
			DbgPrint(0, "Memory leak! Undeleted class '%s'", pHold->m_clsName);
		}
	}


	DbgPrint(0, "SHUTDOWN : Shutdowning dll modules");

	// ukonci dll moduly
	for(UINT i=0;i<m_nNumModules;i++)
	{
		ModuleEntrypoint dllEntry = (ModuleEntrypoint)GetProcAddress(m_modules[i], ENTRYPOINT_NAME_STR);
		dllEntry((CClassManager *)1); // TODO: check return values? not necesary, I think
		FreeLibrary(m_modules[i]);
	}

	DbgPrint(0, "SHUTDOWN : SINGLETONS: TODO: number");
	
	// print debug file
	FILE *fp = fopen("iface.log", "wb");
	fwrite(m_debugLines.c_str(), 1, m_debugLines.size(), fp);
	fclose(fp);
}

//-----------------------------------------------------------------------------
// POMOCNÉ FUNKCE A DEFINICE
//----------------------------------------------------------------------------- 

void* I_GetClass(const char* szClassName)
{
	if (g_pClsMgr) return g_pClsMgr->GetClass(szClassName);
	return NULL;
};

bool I_LoadNewModule(const char* szMouleName)
{
	if (g_pClsMgr) return g_pClsMgr->LoadNewModule(szMouleName);
	return false;
};

void I_Initialize(const char* szDllPath)
{
	if (g_pClsMgr)
	{
		MessageBox(0, "Second call of I_Initialize(...)!", "Unexpected fatal Error!",  MB_SYSTEMMODAL);
		ExitProcess(-1);
	}
	g_pClsMgr = new CClassManager();
	g_pClsMgr->Initialize(szDllPath);
}

void I_RegisterModule(const char* szModuleName)
{
	if (s_szModuleName) return; // second call!
	s_szModuleName = new char[16];
	strncpy(s_szModuleName, szModuleName, 15);
}

void I_RegisterClassFactory(ClassFactoryFn clsFn)
{
	if (!g_pClsMgr)
	{
		MessageBox(0, "You must call I_Initialize() before I_RegisterClassFactory()!", "Unexpected fatal Error!",  MB_SYSTEMMODAL);
		exit(-1);
	}
	g_pClsMgr->SetClassFactory(clsFn);
}

void I_Shutdown()
{
	if (!g_pClsMgr)
	{
		MessageBox(0, "Shutdown() without I_Initialize(...)!", "Unexpected fatal Error!", MB_SYSTEMMODAL);
		ExitProcess(-1);
	}

	DbgPrint(0, "SHUTDOWN : Successful shutdown.");
	if (s_szModuleName) SAFE_DELETE_ARRAY(s_szModuleName);

	g_pClsMgr->Shutdown();
	delete g_pClsMgr;
	g_pClsMgr=NULL;
}

//-----------------------------------------------------------------------------
// DRZAK JEDNE TRIDY (MUZE VYTVARET I VICE INSTANCI DLE TYPU)
//-----------------------------------------------------------------------------

ClassHolder* ClassHolder::g_pLastHolder = NULL;

ClassHolder::ClassHolder(CreateClsFn clsFn, const char* clsName) :
m_clsName(clsName)
{
	m_pNext = g_pLastHolder; // nastav jako dalsi soucasny posledni holder
	m_clsFn = clsFn; // zkopíruj ukazatel na funkci pro vytvoøení tøídy
	g_pLastHolder = this; // posledni holder = tento
	m_nID=0; // zatim nevime zda je to singleton, dovime se to az po prvnim ziskani tridy
	m_pClass=NULL; // zatim nic
	m_numInstances = 0;
}


DLL_EXPORT ClassHolder* ENTRYPOINT_NAME(CClassManager* pClsMgr) // DLL EXPORT: je-li pClsMgr NULL, vrati posledni holder, pokud ne, nastavi dll tento manager
{
	if (pClsMgr==(CClassManager*)1)
	{
		DbgPrint(0, "SHUTDOWN : Successful shutdown. TODO: stats");
		if (s_szModuleName) SAFE_DELETE_ARRAY(s_szModuleName);
		return (ClassHolder *)1;
	}

	if (!pClsMgr)
		return ClassHolder::g_pLastHolder;
	else
		g_pClsMgr = pClsMgr;

	return NULL;
}
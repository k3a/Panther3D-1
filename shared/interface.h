//----------- Copyright � 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Na��t�n� DLL modul�, z�sk�v�n� ukazatel� na t��dy (klas. i singl.).
//			Pouze pro Windows.
// Notes: Je ale d�le�it� sledovat co je singleton a neukon�ovat ho...
//		  Nen� to o�et�eno, mus�te na to d�vat pozor, to snad nen� tak t�k�.
//-----------------------------------------------------------------------------
#pragma once

#include "windows.h"
#include "stdio.h"
#include "symbols.h"
#include <string>
using namespace std;

#define ENTRYPOINT_NAME DllSynapse // nazev entrypointu kazde dll - odviji se od nazvu tymu a projektu
#define ENTRYPOINT_NAME_STR "DllSynapse" // totez jako vyse jen jako retezec

#define MAX_MODULES 30 // maximalni pocet modulu

class ClassHolder;
class CClassManager;

typedef void* (*ClassFactoryFn)(const char* clsName);
typedef void* (*CreateClsFn)();
typedef ClassHolder* (*ModuleEntrypoint)(CClassManager* pClsMgr);

void DbgPrint(WORD objSize, const char* text, ...);

//-----------------------------------------------------------------------------
// CLASS MANAGER
//----------------------------------------------------------------------------- 

class CClassManager
{
public:
	CClassManager();
	void Initialize(const char* szDllPath); // VOLA EXE: najde vsechny knihovny aplikace v zadanem adresari
	void SetClassFactory(ClassFactoryFn fn); // VOLA EXE: nastavi class factory
	bool LoadNewModule(const char* szMouleName); // VOLA EDITOR PRI NACITANI .EXE MODULU HRY
	void Shutdown(); // VOLA EXE: uvolni vsechny singletony a dll moduly
	void* GetClass(const char* className); // vrati instanci tridy podle jejiho identifikatoru
	void PrintFile(const char* data, size_t dataSize);
private:
	ClassHolder* m_pLastHolder; // holdery
	UINT m_nNumSingletons; // pocet jiz nactenych singletonu

	HMODULE m_modules[MAX_MODULES]; // dll moduly
	UINT m_nNumModules; // pocet dll modulu

	char m_szDllPath[MAX_PATH];

	ClassFactoryFn m_clsFactory;
	string m_debugLines;
};

//-----------------------------------------------------------------------------
// POMOCN� FUNKCE A DEFINICE
//----------------------------------------------------------------------------- 

void* I_GetClass(const char* szClassName);
void I_Initialize(const char* szDllPath); // POZOR! VOLA POUZE JEDNOU EXE
void I_RegisterModule(const char* szModuleName);
void I_RegisterClassFactory(ClassFactoryFn clsFn);
bool I_LoadNewModule(const char* szMouleName); // POZOR! VOLA EDITOR PRI NACITANI .EXE MODULU HRY
void I_Shutdown(); // POZOR! VOLA POUZE JEDNOU EXE

#define  DLL_EXPORT   extern "C" __declspec( dllexport )

//-----------------------------------------------------------------------------
// DRZAK JEDNE TRIDY (MUZE VYTVARET I VICE INSTANCI DLE TYPU)
//-----------------------------------------------------------------------------
class ClassHolder
{
public:
	ClassHolder(CreateClsFn clsFn, const char* clsName);

	const char* m_clsName;
	CreateClsFn m_clsFn;
	ClassHolder* m_pNext;
	unsigned int m_nID; // poradi inicializace singletonu (pokud je 0 neni singleton nebo byl ukoncen) - ukoncovat se bude od nejvetsiho k nejmensimu
	void* m_pClass; // pokud je trida singleton, bude zde na ni ulozen ukazatel (pokud bude NULL, jeste nebyla vytvorena instance)
	static ClassHolder* g_pLastHolder;
	DWORD m_numInstances; // actual number of class instances
};

//-----------------------------------------------------------------------------
// Z�KLADN� INTERFACE - V�ECHNY JSOU OD N�J ODVOZENY
//-----------------------------------------------------------------------------
class IP3DBaseInterface
{
public:
	IP3DBaseInterface(){z__Holder=NULL;};
	virtual ~IP3DBaseInterface() {
		if (z__Holder) 
		{
			DbgPrint(-(WORD)sizeof(this), "DELETE : Deleting class '%s'", z__Holder->m_clsName);
			z__Holder->m_numInstances--;
		}
	};
	virtual bool Init() { return true; };
	void z__SetHolder(ClassHolder* pHolder){ z__Holder=pHolder; };
public:
	ClassHolder* z__Holder;
};

extern CClassManager *g_pClsMgr;

//-----------------------------------------------------------------------------
// MAKRA PRO JEDNODU��� PR�CI S IFACE SYST�MEM (REGISTRACE)
//-----------------------------------------------------------------------------
// P�i vytv��en� nov� t��dy v projektu, mus�te vytvo�it jej� interface (abstraktn� t��du) a v cpp souboru
// t��dy zaregistrovat danou t��du. To provedete nap�. takto:
// REGISTER_CLASS(CP3DSky, "P3DSky"); zaregistruje d��ve deklarovanou t��du CP3DSky. Potom bude mo�no z�skat
// neomezen� mno�stv� instanc� t�to t��dy dle n�zvu "CP3DSky". Pokud chcete umo�ni vytv��et jen
// jednu instanci (singleton) n�jak� t��dy pro cel� projekt, pou�ijte REGISTER_SINGLE_CLASS.
// Dule�it� informace:
// - Singletony se samy vytvo�� pouze jednou, jsou-li pot�eba. P�i ukon�en� je pot�eba zavolat I_UnloadSingletons().
// - Klasick� t��dy se vytvo�� a z�sk� se ukazatel. P�i ukon�ov�n� je t�eba takov�to t��dy ukon�it pomoc� delete.

// Zaregistruje t��du a nastav� vytv��ec� funkci na zadanou "createClsFn".
// Pozor! Funkce se mus� sama postarat o vytvo�en� instance t��dy a zavol�n� INIT()!!!
#define REGISTER_CLASS_FN(className, callNameStr, createClsFn) \
	static ClassHolder __g_cls##className##_holder(createClsFn, callNameStr);

// Zaregistruje t��du a vytvo�� pro ni vytv��ec� funkci.
#define REGISTER_CLASS(className, callNameStr) \
	extern ClassHolder __g_cls##className##_holder;\
	static void* __Create##className##Class() {className* pCls=new className;\
	if (pCls->Init()){ __g_cls##className##_holder.m_numInstances++; pCls->z__SetHolder(&__g_cls##className##_holder); return (void*)pCls; } else {delete pCls; return NULL;} }\
	REGISTER_CLASS_FN(className, callNameStr, __Create##className##Class);

// Zaregistruje jedine�nou t��du (singleton) z ukazatele "globalClassVar". T��da se mus� o sebe
// postarat sama (vytvo�it se a ukon�it se). globalClassVar by proto m�la b�t prom�nn� typu
// dan� t��dy. Potom se o ukon�en� postar� syst�m s�m.
// Tak�e se vytvo�� t�eba CMojeTrida g_singl; REGISTER_SINGLE_CLASS_G(-, -, g_singl);
#define REGISTER_SINGLE_CLASS_G(className, callNameStr, globalClassVar) \
	static BYTE __g##className##_bInitState=0; /*stav vol�n� Init(): 0=Create je�t� nebylo vol�no, 1=Init ok, 2=err*/\
	static void* __Create##className##Class() {if(!__g##className##_bInitState)\
	{__g##className##_bInitState = globalClassVar.Init() ? 1 : 2;}\
	return __g##className##_bInitState==1 ? (void*)&globalClassVar : NULL;}\
	static ClassHolder __g_cls##className##_holder(__Create##className##Class, callNameStr);

// Zaregistruje jedine�nou t��du (singleton) a vytvo�� glob�n� ukazatel na tuto t��du.
// Takto vytvo�en� singleton se postar� o vytvo�en� a ukon�en� instance s�m.
#define REGISTER_SINGLE_CLASS(className, callNameStr) \
	static className* __g_##className##_singleton=NULL;\
	extern ClassHolder __g_cls##className##_holderS;\
	static bool __g##className##_bSuccess=false;\
	static void* __Create##className##ClassS() {\
		if (!__g_##className##_singleton) {\
			__g_##className##_singleton = new className;\
			__g##className##_bSuccess = __g_##className##_singleton->Init();\
			__g_cls##className##_holderS.m_nID = 1;\
		}\
		if (__g_##className##_singleton && !__g##className##_bSuccess) {delete __g_##className##_singleton;\
		__g_##className##_singleton=NULL;}\
		return __g##className##_bSuccess ? (void*)__g_##className##_singleton : NULL;}\
	static ClassHolder __g_cls##className##_holderS(__Create##className##ClassS, callNameStr);
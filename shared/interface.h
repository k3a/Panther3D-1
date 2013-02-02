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
// POMOCNÉ FUNKCE A DEFINICE
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
// ZÁKLADNÍ INTERFACE - VŠECHNY JSOU OD NÌJ ODVOZENY
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
// MAKRA PRO JEDNODUŠŠÍ PRÁCI S IFACE SYSTÉMEM (REGISTRACE)
//-----------------------------------------------------------------------------
// Pøi vytváøení nové tøídy v projektu, musíte vytvoøit její interface (abstraktní tøídu) a v cpp souboru
// tøídy zaregistrovat danou tøídu. To provedete napø. takto:
// REGISTER_CLASS(CP3DSky, "P3DSky"); zaregistruje døíve deklarovanou tøídu CP3DSky. Potom bude možno získat
// neomezené množství instancí této tøídy dle názvu "CP3DSky". Pokud chcete umožni vytváøet jen
// jednu instanci (singleton) nìjaké tøídy pro celý projekt, použijte REGISTER_SINGLE_CLASS.
// Duležité informace:
// - Singletony se samy vytvoøí pouze jednou, jsou-li potøeba. Pøi ukonèení je potøeba zavolat I_UnloadSingletons().
// - Klasické tøídy se vytvoøí a získá se ukazatel. Pøi ukonèování je tøeba takovéto tøídy ukonèit pomocí delete.

// Zaregistruje tøídu a nastaví vytváøecí funkci na zadanou "createClsFn".
// Pozor! Funkce se musí sama postarat o vytvoøení instance tøídy a zavolání INIT()!!!
#define REGISTER_CLASS_FN(className, callNameStr, createClsFn) \
	static ClassHolder __g_cls##className##_holder(createClsFn, callNameStr);

// Zaregistruje tøídu a vytvoøí pro ni vytváøecí funkci.
#define REGISTER_CLASS(className, callNameStr) \
	extern ClassHolder __g_cls##className##_holder;\
	static void* __Create##className##Class() {className* pCls=new className;\
	if (pCls->Init()){ __g_cls##className##_holder.m_numInstances++; pCls->z__SetHolder(&__g_cls##className##_holder); return (void*)pCls; } else {delete pCls; return NULL;} }\
	REGISTER_CLASS_FN(className, callNameStr, __Create##className##Class);

// Zaregistruje jedineènou tøídu (singleton) z ukazatele "globalClassVar". Tøída se musí o sebe
// postarat sama (vytvoøit se a ukonèit se). globalClassVar by proto mìla být promìnná typu
// dané tøídy. Potom se o ukonèení postará systém sám.
// Takže se vytvoøí tøeba CMojeTrida g_singl; REGISTER_SINGLE_CLASS_G(-, -, g_singl);
#define REGISTER_SINGLE_CLASS_G(className, callNameStr, globalClassVar) \
	static BYTE __g##className##_bInitState=0; /*stav volání Init(): 0=Create ještì nebylo voláno, 1=Init ok, 2=err*/\
	static void* __Create##className##Class() {if(!__g##className##_bInitState)\
	{__g##className##_bInitState = globalClassVar.Init() ? 1 : 2;}\
	return __g##className##_bInitState==1 ? (void*)&globalClassVar : NULL;}\
	static ClassHolder __g_cls##className##_holder(__Create##className##Class, callNameStr);

// Zaregistruje jedineènou tøídu (singleton) a vytvoøí globání ukazatel na tuto tøídu.
// Takto vytvoøený singleton se postará o vytvoøení a ukonèení instance sám.
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
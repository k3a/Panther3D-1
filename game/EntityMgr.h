//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Entity manager declaration
// Description: Tato tøída slouží pro vytváøení instancí entitových tøíd,
//				napojení na BSPMap a volání daných metod entit.
//-----------------------------------------------------------------------------
#pragma once
#include "IP3DEntityMgr.h"

inline int FindIndex(const epair_s *sEnt, const int numKeyP, const char *key);
// ziskavani klicu z entit, OPTIM: TODO: jak udelat inline??
inline char *ValueForKey(const epair_s *sEnt, const int numKeyP, const char *key, char *defaultKey);
inline float FloatForKey(const epair_s *sEnt, const int numKeyP, const char *key, float defaultKey);
inline void VectorForKey(const epair_s *sEnt, const int numKeyP, const char *key, P3DXVector3D *vecOut );
// *********************************************************************************************************************
// VECI PRO PRIKAZY ENTITAM
#define ECMDLIST_BEGIN 	switch (cmdID){
#define ECMD_BEGIN(_ec_name) \
		case _ec_name##: \
        if (numParams==-1) return true; \
        else if (numParams != g_entCmds[cmdID].numParams) \
		{ \
			CON(MSG_CON_DEBUG, "Entity %s (classname %s): Command %s takes %d parameters (not %d)!", GetTargetName(), GetClassName(), g_entCmds[cmdID].szName, g_entCmds[cmdID].numParams, numParams); \
			return false; \
		}
#define ECMD_END return true;
#define ECMDLIST_END } return false;
// specialni console message s vypisem nazvu entity a tridy
#define ECON(type, text, ...) if(g_pConsole) g_pConsole->MessageEntity(type, __FUNCTION__, GetTargetName(), GetClassName(), text, ##__VA_ARGS__)

// jeden prikaz 
struct ECMD_COMMAND
{
	const char* szName; // nazev prikazu
	int numParams; // pocet parametru
	BYTE type; // TYPE_FLOAT, TYPE_INT, TYPE_STRING
	const char* szHelp; // popis funkce a parametru
};

// enum prikazu pro entity (nutno upravit v .cpp)
enum
{
	_EC_ENABLE=0,
	_EC_DISABLE,
	_EC_SETTEXT,
	_EC_SETCOLOR,
	_EC_SETALPHA,
	_EC_BREAK,
	_EC_TOGGLE,
	_EC_SETVALUE,
	_EC_RESET,
	_EC_ACTION,
	_EC_PRINT,
	_EC_COMMAND,
	_EC_KILL,

	_EC_NUMCMDS
};

// jeden parametr prikazu
struct ECMD_PARAM
{
	union
	{
		int i;
		float f;
		char* c; // nevim prave jak tady s tim :( Je to ukazatel!!!
	};
};

extern ECMD_COMMAND g_entCmds[];


class CP3DEntityMgr : public IP3DEntityMgr
{
public:
	bool Initialize();
	void KillAll();
	bool Spawn(const epair_s *sEnt, const int numEntP, geometryData_s gdata); // vstupni data budou smazana, je nutne je zkopirovat, vola bspmap.cpp pri nacitani
	void Finalize(); // vola bspmap.cpp po nacteni vsech entit, dale pak tento manager po loadu
	void Render(float deltaTime, bool renderModels= true, bool renderEntities=true, bool renderHelpers=false);
	void Think(float deltaTime);
	bool Load(const char* szSaveFile){return false;};
	bool Save(const char* szSaveFile){return false;};
	eBase* FindEntityByTargetname(const char* szTargetName); // najde prvni entitu s timto targetname. Pokud bude potreba hledat vice se stejnym targetname, udelat novou metodu. Zatim ma kazda entita jiny targetname.
	eBase* FindEntityByClassname(const char* szClassName); // vrati prvni z holderu, pokud existuje, dalsi lze ziskat pomoci GetNext() a GetPrevious()
	//
	static bool s_renderModels;
	static bool s_renderEntities;
	static bool s_renderHelpers;
};

// *********************************************************************************************************************
// DRŽÁK JEDNÉ ENTITOVÉ TØÍDY
class eBase;
typedef void* (*CreateEntityFn)();
class EntClassHolder
{
public:
	EntClassHolder(CreateEntityFn fnCreateEntity, const char* szEntityClassName, bool bNeedSort);

	const char* m_szEntityClassName;
	CreateEntityFn m_fnCreateEntity;
	EntClassHolder* m_pNext;
	static EntClassHolder* g_pLastEntityHolder;

	// prace s instancemi entitovych trid tohoto classname
	eBase* m_pFirstEntity;
	eBase* SpawnNewEntity(); // vytvori novou instanci a seradi ji

	bool m_bNeedSort;
};

// REGISTRACNI MAKRO (className, entityClassnameStr, needSort, needThink, needRender)
#define REGISTER_ENTITY(className, entityClassnameStr, needSort) \
	static void* __Create##className##EntityClass() {className* pCls=new className;\
	return (void*)pCls;}\
	static EntClassHolder __g_entCls##className##_holder(__Create##className##EntityClass, entityClassnameStr, needSort)
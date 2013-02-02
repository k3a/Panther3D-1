//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Console interface + ConVars (engine, shared)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

typedef void (*CVarCommand) (const char* cmdl); // pointer-to-method
//typedef void (*CVarCommand) (int argc, char* argv[]); // pointer-to-method

enum ConsoleMsg_t
{
	MSG_CON_DEBUG,
	MSG_CON_INFO,
	MSG_CON_ERR,
	MSG_INFO,
	MSG_ERR,
	MSG_ERR_FATAL,
	MSG_LISTENER // vypise tuhle zpravu pouze v listeneru
};

// Console listener - umoznuje napojit entitu na nejake externi zarizeni
// Aplikace naplni tenhle interface a console bude volat jeho metody
class IP3DConsoleListener
{
public:
	virtual void Message(ConsoleMsg_t type, const char* text)=0;
	virtual void MessageEx(ConsoleMsg_t type, const char* scopeName, const char* text)=0; // rozsireny vypis vcetne funkce ve ktere se to stalo
	virtual bool Command(const char *cmd)=0; // vrati true pokud je to jeho prikaz
};

class ConVar;

#define CON(type, text, ...) if(g_pConsole) g_pConsole->MessageEx(type, __FUNCTION__, text, ##__VA_ARGS__)

class IP3DConsole : public IP3DBaseInterface
{
public:
	virtual void Render()=0;
	virtual void Message(ConsoleMsg_t type, const char* text, ...)=0;
	virtual void MessageEx(ConsoleMsg_t type, const char* scopeName, const char* text, ...)=0;
	virtual void MessageEntity(ConsoleMsg_t type, const char* scopeName, const char* entityTargetName, const char* entityClsName, const char* text, ...)=0;
	virtual void Command(const char *cmd)=0;
	virtual bool IsShown()=0;
	virtual void SetConsoleListener(IP3DConsoleListener *listener)=0; // po destrukci nastaveneho listeneru nutno znova volat s parametrem NULL

	virtual bool RegisterCVar(const char* VarName, CVarCommand cmdPtr)=0;
	virtual void RegisterLastConVar(ConVar* pLastConVar)=0;
	virtual ConVar* FindConVar(const char* pName)=0;
};

#define IP3DENGINE_CONSOLE "P3DConsole_3" // nazev ifacu


#define	CVAR_ARCHIVE 1 // uložit do nastavení
#define	CVAR_READONLY 2 // nemožnost zmìnit CVar z konzole
#define	CVAR_NOT_STRING 4 // nepøekládat na string hodnotu

extern ConVar* g_pLastConVar;

typedef void ( *CVarChangeFn )( ConVar* pCVar/*, const char* pOldValue*/ );

/*
Nápovìda:
Za zmìnu ConVaru je také považováno nastavení SetChangeCallback(), protože je-li ConVar archivován,
dojde v konstruktoru ConVaru k naètení hodnoty. Tato zmìna ale nemùže volat v konstruktoru ukazatel
na funkci. Takže, když je volána SetChangeCallback(), automaticky se po nastavení nastavovaná funkce 
zavolá a je možné provést nastavení. Proto je dobré SetChangeCallback() v místì, kde je možné provézt
zmìnu. (Tomuto volání lze pøedejít nastavení druhého parametru na false)
*/

// FIXME: Je potøeba SetValue pro bool nebo se vpoho pøetypuje na int?
class ConVar 
{
public:
	ConVar(const char* pVarName, const char* pDefault, int flags, const char* pHelp = 0);
	~ConVar();

	void SetChangeCallback(CVarChangeFn pChange, bool bCall = true); // lze nastavit i NULL

	inline float GetFloat() const;
	inline int GetInt() const;
	inline bool GetBool() const {return !!GetInt();}
	inline const char* GetString() const;
	const char*	GetDefault() const;
	const char*	GetHelpStr() const;
	const char* GetName() const;
	ConVar* GetNext() const;
	void SetNext(ConVar* pNext);
	int GetFlags() const;

	void SetValue(float val);
	void SetValue(int val);
	void SetValue(const char* val);

	void Reset();

private:
	// zakl. data
	int				m_nFlags;
	const char*		m_pszName;
	const char*		m_pszHelp;
	const char*		m_pszDefaultValue;
	ConVar*	m_pNext; // ukazatel na další ConVar (NULL pokud není)

	// hodnoty
	char*			m_pszValue; // dynamicky upravované pole
	int				m_nValueLen; // velikost m_pszValue + znak 0
	int				m_nValue;
	float			m_fValue;

	// callback pro zmenu
	CVarChangeFn	m_fnChange;
};

inline float ConVar::GetFloat() const
{
	return m_fValue;
}

inline int ConVar::GetInt() const
{
	return m_nValue;
}

inline const char* ConVar::GetString() const
{
	if ( m_nFlags & CVAR_NOT_STRING )
		return NULL; // ConVar don't have string type!!!

	return m_pszValue;
}
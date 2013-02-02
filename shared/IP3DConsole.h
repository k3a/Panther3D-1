//----------- Copyright � 2005-2007, 7thSquad, all rights reserved ------------
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


#define	CVAR_ARCHIVE 1 // ulo�it do nastaven�
#define	CVAR_READONLY 2 // nemo�nost zm�nit CVar z konzole
#define	CVAR_NOT_STRING 4 // nep�ekl�dat na string hodnotu

extern ConVar* g_pLastConVar;

typedef void ( *CVarChangeFn )( ConVar* pCVar/*, const char* pOldValue*/ );

/*
N�pov�da:
Za zm�nu ConVaru je tak� pova�ov�no nastaven� SetChangeCallback(), proto�e je-li ConVar archivov�n,
dojde v konstruktoru ConVaru k na�ten� hodnoty. Tato zm�na ale nem��e volat v konstruktoru ukazatel
na funkci. Tak�e, kdy� je vol�na SetChangeCallback(), automaticky se po nastaven� nastavovan� funkce 
zavol� a je mo�n� prov�st nastaven�. Proto je dobr� SetChangeCallback() v m�st�, kde je mo�n� prov�zt
zm�nu. (Tomuto vol�n� lze p�edej�t nastaven� druh�ho parametru na false)
*/

// FIXME: Je pot�eba SetValue pro bool nebo se vpoho p�etypuje na int?
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
	ConVar*	m_pNext; // ukazatel na dal�� ConVar (NULL pokud nen�)

	// hodnoty
	char*			m_pszValue; // dynamicky upravovan� pole
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
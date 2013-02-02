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
#include "IP3DConsole.h"
#include "symbols.h"
#include <assert.h>
#include <stdio.h>

static char IniFilePath[MAX_PATH];
static bool haveIniFilePath=false;

ConVar* g_pLastConVar=NULL;

ConVar::ConVar(const char* pVarName, const char* pDefault, int flags, const char* pHelp/*=0*/)
{
	assert( pVarName ); assert( pDefault );  // prázdná zadání - zakázáno

	m_nFlags = flags;
	m_pszHelp = pHelp ? pHelp : "";
	m_pszName = pVarName;
	m_pszDefaultValue = pDefault;

	// vytvoø string hodnotu
	if(m_nFlags & CVAR_ARCHIVE) // naèti uloženou hodnotu
	{
		// získej cestu k ini
		if (!haveIniFilePath)
		{
			haveIniFilePath = true;
			GetCurrentDirectory(MAX_PATH, IniFilePath);
			strcat(IniFilePath, "\\data\\settings.ini");
		}

		// naèti z ini do doèasného bufferu
		char tmpStr[256]; // WARN: max. 256 dlouhý string
		GetPrivateProfileString("ConVars", pVarName, pDefault, tmpStr, 256, IniFilePath);

		m_nValueLen = (int)strlen(tmpStr)+1;
		m_pszValue = new char[m_nValueLen];
		strcpy(m_pszValue, tmpStr); // nastav
	}
	else // ulož default
	{
		m_nValueLen = (int)strlen(pDefault)+1;
		m_pszValue = new char[m_nValueLen];
		strcpy(m_pszValue, pDefault); // nastav
	}

    
	// vytvoø ostatní typy hodnoty
	m_fValue = (float)atof(m_pszValue);
	m_nValue = (int)m_fValue;

	if (m_nFlags&CVAR_NOT_STRING) { delete[] m_pszValue; m_pszValue=NULL; }

	// Zaregistruj tento ConVar
	m_pNext = g_pLastConVar;
	g_pLastConVar = this;
}

ConVar::~ConVar()
{
	if(m_nFlags & CVAR_ARCHIVE) // uložit do ini
	{
		if(!m_pszValue)
		{
			m_pszValue = new char[100];
			sprintf(m_pszValue, "%f", m_fValue); //FIXME: staci float ukladani?
		}
		WritePrivateProfileString("ConVars", m_pszName, m_pszValue, IniFilePath);
	}
	SAFE_DELETE_ARRAY(m_pszValue);
}

void ConVar::SetChangeCallback(CVarChangeFn pChange, bool bCall /*= true*/)
{
	m_fnChange = pChange;
	if (bCall && m_fnChange) m_fnChange(this);
}

const char* ConVar::GetDefault() const
{
	return m_pszDefaultValue;
}

const char* ConVar::GetHelpStr() const
{
	return m_pszHelp;
}

const char* ConVar::GetName() const
{
	return m_pszName;
}

ConVar* ConVar::GetNext() const
{
	return m_pNext;
}

void ConVar::SetNext(ConVar* pNext)
{
	m_pNext = pNext;
}

int ConVar::GetFlags() const
{
	return m_nFlags;
}

void ConVar::SetValue(float val)
{
	m_nValue = (int)val;
	m_fValue = val;

	if (!( m_nFlags & CVAR_NOT_STRING ))
	{
		char tmpVal[ 32 ];
		sprintf(tmpVal, "%f", val);
		SetValue(tmpVal);
	}

	if (m_fnChange) m_fnChange(this); // zavolej callback zmìny
}

void ConVar::SetValue(int val)
{
	m_nValue = val;
	m_fValue = (float)val;

	if (!( m_nFlags & CVAR_NOT_STRING ))
	{
		char tmpVal[ 32 ];
		sprintf(tmpVal, "%d", val);
		SetValue(tmpVal);
	}

	if (m_fnChange) m_fnChange(this); // zavolej callback zmìny
}

void ConVar::SetValue(const char* val)
{
	if (!(m_nFlags & CVAR_NOT_STRING))
	{
		int strLen = (int)strlen(val)+1;

		if (strLen>m_nValueLen) // je-li potøeba, zvìtši buffer hodnoty
		{
			//SAFE_DELETE_ARRAY(m_pszValue); //FIXME: !!!!!! POTØEBA UKONÈIT!!!! Proè se to nepovede jen 1. ConVar?
			m_pszValue = new char[strLen];
			m_nValueLen = strLen;
		}
		strcpy(m_pszValue, val); // nastav
	}

	m_fValue = (float)atof(val);
	m_nValue = (int)m_fValue;

	if (m_fnChange) m_fnChange(this); // zavolej callback zmìny
}

void ConVar::Reset()
{
	SetValue(m_pszDefaultValue);
}
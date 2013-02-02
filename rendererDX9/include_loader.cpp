//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (17.05.2007)
// Purpose:	Include loader for effect framework
//-----------------------------------------------------------------------------

// Microsoft to v tomhle smeru bych rek docela podelal. Nedal tady zadne user data a tudis nemuzu spolu s otevrenymi daty v pameti
// ulozit FSFILE a pak v Close() provest ligalni Unload(). Misto toho jen smazu buffer :(. Trapne ale nenapadlo me zadne reseni.

#include "include_loader.h"
#include "common.h"

static FSFILE *s_pLast=NULL;

HRESULT STDMETHODCALLTYPE CP3DIncludeLoader::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
	// vytvorit cestu
	char openPath[MAX_PATH]="shaders/";
	strcat(openPath, pFileName);

	// otevrit soubor pro cteni
	BYTE* pData=NULL; ULONG lSize=0;
	s_pLast = g_pFS->Load(openPath, pData, lSize);

	*ppData = pData;
	*pBytes = lSize;

	if (lSize && s_pLast)
		return S_OK;
	else
		return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CP3DIncludeLoader::Close(LPCVOID pData)
{
	if (s_pLast)
	{
		g_pFS->UnLoad(s_pLast, (BYTE*)pData);
		return S_OK;
	}
	else
		return S_FALSE;
}
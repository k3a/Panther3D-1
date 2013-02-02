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
#pragma once
#include "D3DX9Effect.h"

class CP3DIncludeLoader : public ID3DXInclude
{
public:
	STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
	STDMETHOD(Close)(LPCVOID pData);
};
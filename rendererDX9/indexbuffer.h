//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Index buffer class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3dindexbuffer.h"
#include "d3dx9.h"

class CP3DIndexBuffer : public IP3DIndexBuffer
{
public:
	CP3DIndexBuffer();
	~CP3DIndexBuffer();
	HRESULT CreateIB(int indNum);
	HRESULT Lock (void** data, unsigned int SizeToLock = 0);
	HRESULT UnLock ();
	void Use();
	void Release();
private:
	LPDIRECT3DINDEXBUFFER9 m_pIB;
};
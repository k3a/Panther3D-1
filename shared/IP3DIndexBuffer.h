//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Index buffer interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"

class IP3DIndexBuffer : public IP3DBaseInterface
{
public:
	virtual long CreateIB(int indNum)=0;
	virtual long Lock (void** data, unsigned int SizeToLock = 0)=0;
	virtual long UnLock ()=0;
	virtual void Use()=0;
	virtual void Release()=0;
};

#define IP3DRENDERER_INDEXBUFFER "P3DIndexBuffer_1" // nazev ifacu
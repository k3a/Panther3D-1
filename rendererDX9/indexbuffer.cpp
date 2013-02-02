//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Index buffer class definition
//-----------------------------------------------------------------------------

#include "indexbuffer.h"
#include "common.h"

REGISTER_CLASS(CP3DIndexBuffer, IP3DRENDERER_INDEXBUFFER); // zaregistruj tuto tøídu

CP3DIndexBuffer::CP3DIndexBuffer()
{
	m_pIB = NULL;
}

CP3DIndexBuffer::~CP3DIndexBuffer()
{
	SAFE_RELEASE(m_pIB);
}

HRESULT CP3DIndexBuffer::CreateIB(int indNum)
{
	return g_pD3DDevice->CreateIndexBuffer (indNum * 4, 0, D3DFMT_INDEX32, \
														D3DPOOL_MANAGED, &m_pIB, NULL);
}

HRESULT CP3DIndexBuffer::Lock (void** data, unsigned int SizeToLock)
{
	Prof(RENDERER_CP3DVertexBuffer__Lock);
	return m_pIB->Lock (0, SizeToLock, data, 0);
}

HRESULT CP3DIndexBuffer::UnLock ()
{
	Prof(RENDERER_CP3DVertexBuffer__UnLock);
	return m_pIB->Unlock();
}

void CP3DIndexBuffer::Use()
{
	g_pD3DDevice->SetIndices(m_pIB);
}

void CP3DIndexBuffer::Release()
{
	SAFE_RELEASE(m_pIB);
}
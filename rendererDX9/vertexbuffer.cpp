//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Vertex buffer class definition
//-----------------------------------------------------------------------------

#include "vertexbuffer.h"
#include "common.h"
#include <fstream>

REGISTER_CLASS(CP3DVertexBuffer, IP3DRENDERER_VERTEXBUFFER); // zaregistruj tuto tøídu

CP3DVertexBuffer::CP3DVertexBuffer()
{
	g_pRenderer = (CP3DRenderer*)I_GetClass(IP3DRENDERER_RENDERER);
	m_vertNum = 0;
	m_SizeInBytes = 0;
	m_pVB = NULL;
	m_pVertexDeclaration = NULL;
}

CP3DVertexBuffer::~CP3DVertexBuffer()
{
	STAT(STAT_ALL_VERTS, -(int)m_vertNum);
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pVertexDeclaration)
}

HRESULT CP3DVertexBuffer::CreateVB(int vertNum, P3DVertexElement *vertDeclaration, UINT nSizeInBytes, bool dynamic, bool writeonly, bool points)
{

	if(FAILED(GenerateVertexDeclaration(vertDeclaration))) return E_FAIL;
	if(FAILED(g_pD3DDevice->CreateVertexBuffer(vertNum * nSizeInBytes, 0, 0, D3DPOOL_MANAGED, &m_pVB, NULL))) return E_FAIL;
	m_SizeInBytes = nSizeInBytes;
	
	m_vertNum = vertNum;

	STAT(STAT_ALL_VERTS, m_vertNum);

	return S_OK;
}

HRESULT CP3DVertexBuffer::Lock (void** data, unsigned int SizeToLock)
{
	Prof(RENDERER_CP3DVertexBuffer__Lock);
	return m_pVB->Lock (0, SizeToLock, data, 0);
}

HRESULT CP3DVertexBuffer::UnLock ()
{
	Prof(RENDERER_CP3DVertexBuffer__UnLock);
	return m_pVB->Unlock();
}

void CP3DVertexBuffer::Use()
{
		g_pD3DDevice->SetStreamSource( 0, m_pVB, 0, m_SizeInBytes );
		g_pD3DDevice->SetVertexDeclaration(m_pVertexDeclaration);
}

void CP3DVertexBuffer::Render(P3DPrimitiveType_t primType, int start, int num)
{
	STAT(STAT_DRAW_CALLS, 1);
	STAT(STAT_DRAW_VERTS, num);
	g_pD3DDevice->DrawPrimitive( (D3DPRIMITIVETYPE)primType, start, num );
}

void CP3DVertexBuffer::RenderIndexed(P3DPrimitiveType_t primType, int baseVertIndex, unsigned int MinIndex, unsigned int NumVert, unsigned int StartIndex, unsigned int PrimitiveCnt)
{
	STAT(STAT_DRAW_CALLS, 1);
	STAT(STAT_DRAW_VERTS, PrimitiveCnt);
	g_pD3DDevice->DrawIndexedPrimitive( (D3DPRIMITIVETYPE)primType, baseVertIndex, MinIndex, NumVert, StartIndex, PrimitiveCnt );
}

void CP3DVertexBuffer::Release()
{
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pVertexDeclaration)
	m_vertNum = 0;
	m_SizeInBytes = 0;
}

HRESULT CP3DVertexBuffer::GenerateVertexDeclaration(P3DVertexElement *vertDeclaration)
{
	// We find out how many Elements has our Vertex Declaration 
	UINT numElements = 0;
	while(vertDeclaration[numElements].vertexType != P3DVD_NONE)
	{
		numElements++;
	}
	if(numElements == 0) return E_FAIL;
	//g_pConsole->Message(MSG_CON_INFO, ")
	CON(MSG_CON_INFO, "numElements je %d", numElements);  

	// Than we create the Vertex Declaration
	D3DVERTEXELEMENT9* dwDecl = new D3DVERTEXELEMENT9[numElements+1];
	WORD offset = 0;
	UINT numVU = 15;	//There are 15 values in P3DVertexUsage enum
	BYTE* pusageIndex = new BYTE[numVU]; 
	for(UINT i=0; i<numVU; i++) pusageIndex[i] = 0;

	for(UINT i=0; i<numElements; i++)
	{
		// Generating Vertex Declaration how D3D wants it
		dwDecl[i].Stream = 0;
		dwDecl[i].Method = D3DDECLMETHOD_DEFAULT;

		switch(vertDeclaration[i].vertexType)
		{
		case P3DVD_NONE:
			{
			dwDecl[i].Type = D3DDECLTYPE_UNUSED;
			dwDecl[i].Offset = offset;
			offset += 0;
			break;
			}

		case P3DVD_FLOAT1:
			{
			dwDecl[i].Type = D3DDECLTYPE_FLOAT1;
			dwDecl[i].Offset = offset;
			offset += 4;
			break;
			}

		case P3DVD_FLOAT2:
			{
			dwDecl[i].Type = D3DDECLTYPE_FLOAT2;
			dwDecl[i].Offset = offset;
			offset += 8;
			break;
			}

		case P3DVD_FLOAT3:
			{
			dwDecl[i].Type = D3DDECLTYPE_FLOAT3;
			dwDecl[i].Offset = offset;
			offset += 12;
			break;
			}

		case P3DVD_FLOAT4:
			{
			dwDecl[i].Type = D3DDECLTYPE_FLOAT4;
			dwDecl[i].Offset = offset;
			offset += 16;
			break;
			}
		}

		switch(vertDeclaration[i].vertexUsage)
		{
		case P3DVU_POSITION:
			dwDecl[i].Usage = D3DDECLUSAGE_POSITION;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_POSITION];
			pusageIndex[P3DVU_POSITION]++;
			break;

		case P3DVU_BLENDWEIGHT:
			dwDecl[i].Usage = D3DDECLUSAGE_BLENDWEIGHT;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_BLENDWEIGHT];
			pusageIndex[P3DVU_BLENDWEIGHT]++;
			break;

		case P3DVU_BLENDINDICES:
			dwDecl[i].Usage = D3DDECLUSAGE_BLENDINDICES;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_BLENDINDICES];
			pusageIndex[P3DVU_BLENDINDICES]++;
			break;

		case P3DVU_NORMAL:
			dwDecl[i].Usage = D3DDECLUSAGE_NORMAL;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_NORMAL];
			pusageIndex[P3DVU_NORMAL]++;
			break;

		case P3DVU_PSIZE:
			dwDecl[i].Usage = D3DDECLUSAGE_PSIZE;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_PSIZE];
			pusageIndex[P3DVU_PSIZE]++;
			break;

		case P3DVU_TEXCOORD:
			dwDecl[i].Usage = D3DDECLUSAGE_TEXCOORD;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_TEXCOORD];
			pusageIndex[P3DVU_TEXCOORD]++;
			break;

		case P3DVU_TANGENT:
			dwDecl[i].Usage = D3DDECLUSAGE_TANGENT;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_TANGENT];
			pusageIndex[P3DVU_TANGENT]++;
			break;

		case P3DVU_BINORMAL:
			dwDecl[i].Usage = D3DDECLUSAGE_BINORMAL;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_BINORMAL];
			pusageIndex[P3DVU_BINORMAL]++;
			break;

		case P3DVU_TESSFACTOR:
			dwDecl[i].Usage = D3DDECLUSAGE_TESSFACTOR;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_TESSFACTOR];
			pusageIndex[P3DVU_TESSFACTOR]++;
			break;

		case P3DVU_POSITIONT:
			dwDecl[i].Usage = D3DDECLUSAGE_POSITIONT;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_POSITIONT];
			pusageIndex[P3DVU_POSITIONT]++;
			break;

		case P3DVU_COLOR:
			dwDecl[i].Usage = D3DDECLUSAGE_COLOR;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_COLOR];
			pusageIndex[P3DVU_COLOR]++;
			break;

		case P3DVU_FOG:
			dwDecl[i].Usage = D3DDECLUSAGE_FOG;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_FOG];
			pusageIndex[P3DVU_FOG]++;
			break;

		case P3DVU_DEPTH:
			dwDecl[i].Usage = D3DDECLUSAGE_DEPTH;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_DEPTH];
			pusageIndex[P3DVU_DEPTH]++;
			break;

		case P3DVU_SAMPLE:
			dwDecl[i].Usage = D3DDECLUSAGE_SAMPLE;
			dwDecl[i].UsageIndex = pusageIndex[P3DVU_SAMPLE];
			pusageIndex[P3DVU_SAMPLE]++;
			break;
		}

	}

	//D3DDECL_END() Last member of an array
	dwDecl[numElements].Stream = 0xFF;
	dwDecl[numElements].Offset = 0;
	dwDecl[numElements].Type = D3DDECLTYPE_UNUSED;
	dwDecl[numElements].Method = 0;
	dwDecl[numElements].Usage = 0;
	dwDecl[numElements].UsageIndex = 0;

	
	std::ofstream fout;
	fout.open("vdgen.txt");
	for(UINT i=0; i<(numElements+1); i++)
	{
		fout << int(dwDecl[i].Method) << "\n";//sedi
		fout << int(dwDecl[i].Offset) << "\n";//sedi
		fout << int(dwDecl[i].Stream) << "\n";//sedi
		fout << int(dwDecl[i].Type) << "\n";//sedi
		fout << int(dwDecl[i].Usage) << "\n";//sedi
		fout << int(dwDecl[i].UsageIndex) << "\n";//sedi
		fout << "-------------\n";
	}

	HRESULT hr;
	hr = g_pD3DDevice->CreateVertexDeclaration(dwDecl, &m_pVertexDeclaration);
	if(!FAILED(hr)) CON(MSG_CON_INFO, "m_pVertexDeclaration vytvoreno");  
	delete[] dwDecl;
	delete[] pusageIndex;
	return hr;
}

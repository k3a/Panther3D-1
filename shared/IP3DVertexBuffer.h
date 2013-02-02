//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Vertex buffer interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include "types.h" 

enum P3DVBType_t
{
	P3DVBTYPE_POS1TEX,				// pozice, tex1
	P3DVBTYPE_POSNORMAL1TEX,		// pozicia, normala, tex1
	P3DVBTYPE_POSRHW1TEX,			// transformovaná pozice, tex1
	P3DVBTYPE_POSRHWDIFFUSE,		// transformovaná pozice, barva
	P3DVBTYPE_FX_POSNORMAL2TEXCOLOR	// pozice, normal, tex1, tex2, color, tangent, binormal = Pro shadery
};

struct P3DPOS1TEX
{
	float x, y, z; // The untransformed position for the vertex
	float tu, tv; // First texture coordinates
};

struct P3DPOSNORMAL1TEX
{
	float x, y, z; // The untransformed position for the vertex
	float nx, ny, nz;	// normal
	float tu, tv;	// First texture coordinates
};

struct P3DPOSRHW1TEX
{
   float x, y, z, rhw; // The transformed position for the vertex
	float tu, tv; // First texture coordinates
};

struct P3DPOSRHWC
{
   float x, y, z, rhw; // The transformed position for the vertex
	unsigned long c; // Color
};

struct P3D_FX_POSNORMAL2TEXCOLOR
{
   float pos[3]; // The untransformed position for the vertex
   float normal[3]; // Vertex normal
	float tc1[2]; // First texture coordinates
	float tc2[2]; // Second texture coordinates
	float tan[3]; // tangent
	float bitan[3]; // bitangent
	DWORD color; // vert. color
};

enum P3DPrimitiveType_t {
//	P3DPT_POINTLIST = 1,
// P3DPT_LINELIST = 2,
// P3DPT_LINESTRIP = 3,
	P3DPT_TRIANGLELIST = 4,
	P3DPT_TRIANGLESTRIP = 5,
//	P3DPT_TRIANGLEFAN = 6,
};


/** Defines Vertex Type for P3DVertexElement struct \see P3DVertexElement */
enum P3DVertexType
{
	P3DVD_NONE = 0,
	P3DVD_FLOAT1,
	P3DVD_FLOAT2,
	P3DVD_FLOAT3,
	P3DVD_FLOAT4
};

#define P3DVD_COLORRGB P3DVD_FLOAT3
#define P3DVD_COLORRGBA P3DVD_FLOAT4

/** Defines Vertex Usage for P3DVertexElement struct \see P3DVertexElement */
enum P3DVertexUsage
{
	P3DVU_NONE = 0,
	P3DVU_POSITION,
    P3DVU_BLENDWEIGHT,
    P3DVU_BLENDINDICES,
    P3DVU_NORMAL,
    P3DVU_PSIZE,
    P3DVU_TEXCOORD,
    P3DVU_TANGENT,
    P3DVU_BINORMAL,
    P3DVU_TESSFACTOR,
    P3DVU_POSITIONT,
    P3DVU_COLOR,
    P3DVU_FOG,
    P3DVU_DEPTH,
    P3DVU_SAMPLE
};

/** Defines Vertex Element which is used for generating the Vertex Declaration \see IP3DVertexBuffer */
struct P3DVertexElement
{
	P3DVertexElement()
	{

	}

	P3DVertexElement(UINT type, UINT usage)
	{
		vertexType = type;
		vertexUsage = usage;
	}

	UINT vertexType;
	UINT vertexUsage;
};

// Set this as last element of the P3DVertexElement array
#define P3DVE_END() P3DVertexElement(P3DVD_NONE, P3DVU_NONE)

//MY TEMPORAL VERTEX STRUCT FOR VERTEX BUFFER TESTING
struct tmpVertPosClr
{
	P3DXPoint3D pos;
	P3DXPoint3D clr;
};

class IP3DVertexBuffer : public IP3DBaseInterface
{
public:
	/** Creates vertex buffer */
	virtual long CreateVB(int vertNum, P3DVertexElement *vertDeclaration, UINT nSizeInBytes, bool dynamic = false, bool writeonly = false, bool points = false)=0;
	/** Locks the Vertex Buffer and obtains pointer on it's memory. */
	virtual long Lock (void** data, unsigned int SizeToLock = 0)=0;
	/** Unlocks Vertex Buffer. */
	virtual long UnLock ()=0;
	/** Renders selected part of Vertex Buffer. */
	virtual void Render(P3DPrimitiveType_t primType, int start, int num)=0;
	/** Renders selected part of Vertex Buffer considering Indices. */
	virtual void RenderIndexed(P3DPrimitiveType_t primType, int baseVertIndex, unsigned int MinIndex, unsigned int NumVert, unsigned int StartIndex, unsigned int PrimitiveCnt)=0;
	/** Tells to engine the definition of vertices in Vertex Buffer. */
	virtual void Use()=0;
	/** Releases this class. */
	virtual void Release()=0;
};

#define IP3DRENDERER_VERTEXBUFFER "P3DVertexBuffer_1" // nazev ifacu
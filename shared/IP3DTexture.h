//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Texture map interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include "types.h"

struct P3DLOCKED_RECT {
    int Pitch;
    void *pBits;
};

enum TexType
{
	TT_NONE,
	TT_2D,
	TT_CUBE,
	TT_VOLUME,
	TT_RT
};


#define SIZE_SCREEN 99999
#define SIZE_SCREEN_HALF 99998
#define SIZE_SCREEN_QUART 99998

enum TexFormat
{
	TF_NONE    = 0,
	TF_I8      = 1,
	TF_IA8     = 2,
	TF_RGB8    = 3,
	TF_RGBA8   = 4,
	TF_I16     = 5,
	TF_IA16    = 6,
	TF_RGB16   = 7,
	TF_RGBA16  = 8,
	TF_R16f    = 9,
	TF_RG16f   = 10,
	TF_RGB16f  = 11,
	TF_RGBA16f = 12,
	TF_R32f    = 13,
	TF_RG32f   = 14,
	TF_RGB32f  = 15,
	TF_RGBA32f = 16,
	TF_I16f    = 17,
	TF_IA16f   = 18,
	TF_I32f    = 19,
	TF_IA32f   = 20,
	TF_UV8     = 21,
	TF_UVWQ8   = 22,
	TF_UV16    = 23,
	TF_UVWQ16  = 24,
	TF_DEPTH16 = 25,
	TF_DEPTH24 = 26,
	TF_RGB332  = 27,
	TF_RGB10A2 = 28,
	TF_UV5L6   = 29,
	TF_UVW10A2 = 30,
	TF_DXT1    = 31,
	TF_DXT3    = 32,
	TF_DXT5    = 33,
	TF_ATI1N   = 34, // newest 3Dc addition, only ATI cards
	TF_ATI2N   = 35, // 3Dc, some nVidia and ATI cards
	TT_NUM_FORMATS = 36
};

#define TF_R8    TF_I8
#define TF_RG8   TF_IA8
#define TF_R16   TF_I16
#define TF_RG16  TF_IA16

struct TexDesc
{
	TexType type;
	TexFormat format;
	UINT mipmaps;
	UINT width;
	UINT height;
};

#define P3D_TEXFILTER_LINEAR (3 << 0)

class IP3DTexture : public IP3DBaseInterface
{
public:
	virtual bool Load(const char* FileName, TexType texType, bool bForceToCreateNew = false)=0;
	virtual void UnLoad ()=0;
	virtual void Use(unsigned long Stage = 0)=0;
	virtual bool Create(unsigned int Width, unsigned int Height,unsigned int MipLevels, TexFormat format, bool renderTarget = false)=0;
	virtual void GetSurfaceData(unsigned int SurfaceLevel = 0, P3DLOCKED_RECT *pRect = NULL)=0;
	virtual void SaveSurfaceData()=0;
	virtual void Filter(unsigned long ulFilter)=0;
	virtual void *GetD3DTexture()=0;
	/** Get IDirect3DSurface9 pointer. Retrieved surface must be released! */
	virtual void *GetD3DSurface(unsigned int nSurfaceLevel)=0;
	virtual void GetDesc(TexDesc &desc)=0;
	// render target feature
	/** Set surface of this texture as render target. Texture must be created as render target! 
	After rendering to MRT, you SHOULD call IP3DRenderer::SetDefaultRenderTarget().
	\param rtIndex Render target index for multiple render targets (MRT) */
	virtual bool SetAsRenderTarget(DWORD rtIndex)=0;
	/** Copies current back buffer content to this render target texture.  
	Texture must be created as render target!*/
	virtual bool CaptureCurrentBackBuffer()=0;
};

#define IP3DRENDERER_TEXTURE "P3DTexture_2" // nazev ifacu
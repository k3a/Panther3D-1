//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Texture map (jpg, tga, ....) class definition
//-----------------------------------------------------------------------------

#include "texture.h"
#include "common.h"

REGISTER_CLASS(CP3DTexture, IP3DRENDERER_TEXTURE); // zaregistruj tuto tøídu

D3DFORMAT g_TexFormat[] = {
	D3DFMT_UNKNOWN,
	D3DFMT_L8,
	D3DFMT_A8L8,
	D3DFMT_X8R8G8B8,
	D3DFMT_A8R8G8B8,

	D3DFMT_L16,
	D3DFMT_G16R16,
	D3DFMT_A16B16G16R16, // RGB16 will be expanded to RGBA16
	D3DFMT_A16B16G16R16,

	D3DFMT_R16F,
	D3DFMT_G16R16F,
	D3DFMT_A16B16G16R16F, // RGB16F will be expanded to RGBA16F
	D3DFMT_A16B16G16R16F,

	D3DFMT_R32F,
	D3DFMT_G32R32F,
	D3DFMT_A32B32G32R32F, // RGB32F will be expanded to RGBA32F
	D3DFMT_A32B32G32R32F,

	D3DFMT_R16F, // I and IA formats uploaded as R and RG
	D3DFMT_G16R16F,
	D3DFMT_R32F,
	D3DFMT_G32R32F,

	D3DFMT_V8U8,
	D3DFMT_Q8W8V8U8,
	D3DFMT_V16U16,
	D3DFMT_Q16W16V16U16,

	(D3DFORMAT) MAKEFOURCC('D', 'F', '1', '6'), // Depth16
	(D3DFORMAT) MAKEFOURCC('D', 'F', '2', '4'), // Depth24

	D3DFMT_R3G3B2,
	D3DFMT_A2B10G10R10,
	D3DFMT_L6V5U5,
	D3DFMT_A2W10V10U10,

	D3DFMT_DXT1,
	D3DFMT_DXT3,
	D3DFMT_DXT5,
	(D3DFORMAT) MAKEFOURCC('A', 'T', 'I', '1'), // ATI1N
	(D3DFORMAT) MAKEFOURCC('A', 'T', 'I', '2'), // ATI2N
};

CP3DTexture::CP3DTexture()
{
	m_pTexture = NULL;
	m_pSurface = NULL;
	m_texDesc.width = 0;
	m_texDesc.height = 0;
	m_texDesc.mipmaps = 0;
	m_texDesc.type = TT_NONE;
}

CP3DTexture::~CP3DTexture()
{
	UnLoad ();
}

bool CP3DTexture::FindExtension(char *strFileName)
{
	char strJPGPath[MAX_PATH] = {0};
	char strTGAPath[MAX_PATH] = {0};
	char strDDSPath[MAX_PATH] = {0};
	char strBMPPath[MAX_PATH] = {0};

	if (strFileName[1]!=':') // pokud se jedná o relativní cestu (Ta by ale ve funálu mìla být relativní!!!)
	{
		// jen zkopíruj cestu, pak se budou pøidávat pøípony
		strcpy(strJPGPath, strFileName);
		strcpy(strTGAPath, strJPGPath);
		strcpy(strDDSPath, strJPGPath);
		strcpy(strBMPPath, strJPGPath);
	}
	
	// pøidej pøípony
	strcat(strJPGPath, ".jpg");
	strcat(strTGAPath, ".tga");
	strcat(strDDSPath, ".dds");
	strcat(strBMPPath, ".bmp");

	if(g_pFS->Exists(strDDSPath))
	{
		strcat(strFileName, ".dds");
		return true;
	}
	if(g_pFS->Exists(strJPGPath))
	{
		strcat(strFileName, ".jpg");
		return true;
	}
	if(g_pFS->Exists(strTGAPath))
	{
		strcat(strFileName, ".tga");
		return true;
	}
	if(g_pFS->Exists(strBMPPath))
	{
		strcat(strFileName, ".bmp");
		return true;
	}
	return false; // not found
}


bool CP3DTexture::Load(const char* FileName, TexType texType, bool bForceToCreateNew)
{
	if ((FileName == NULL) ||  (FileName[0] == 0)) return false;
	// uvolnenie predoslej textury ak existuje
	UnLoad ();

	char strFileName[MAX_PATH];
	strncpy (strFileName, FileName, MAX_PATH-1);
	if(!g_pFS->Exists(strFileName)) // is FileName with extension and exists?
		if (!FindExtension(strFileName))	// Try find texture filename extension
		{
			CON(MSG_CON_ERR, "Texture: Requested texture doesn't exist '%s'!", FileName);
			return false;
		}
	// loading
	switch (texType)
	{
	case TT_2D:
		m_pTexture = g_TextureLoader.CreateTexture (strFileName, bForceToCreateNew); break;
	case TT_CUBE:
		m_pCubeTexture = g_TextureLoader.CreateCubeTexture (strFileName, bForceToCreateNew); break;
	case TT_VOLUME:
		m_pVolumeTexture = g_TextureLoader.CreateVolumeTexture (strFileName, bForceToCreateNew);
	}

	if (m_pTexture == NULL)
		{CON(MSG_CON_ERR, "Can't create texture from file '%s'!", FileName); return false;}

	m_texDesc.type = texType;
	D3DSURFACE_DESC desc;
	m_pTexture->GetLevelDesc(0, &desc);
	m_texDesc.width = desc.Width;
	m_texDesc.height = desc.Height;
	m_texDesc.mipmaps = m_pTexture->GetLevelCount();

	// get format
	D3DFORMAT fmt = desc.Format; int fID=TF_NONE;
	for (int i=0;i<TT_NUM_FORMATS;i++)
	{
		if (g_TexFormat[i]==fmt) 
		{
			fID=i;
			break;
		}
	}
	m_texDesc.format = TexFormat(fID);

	return true;
}

void CP3DTexture::UnLoad ()
{
	SAFE_RELEASE(m_pSurface);
	if (m_pTexture)
		g_TextureLoader.UnloadTexture (m_pTexture);
	m_pTexture = NULL;
	m_texDesc.type = TT_NONE;
}

void CP3DTexture::Use(unsigned long Stage)
{
	g_pRenderer->SetTexture (m_pTexture, Stage);
}

bool CP3DTexture::Create(unsigned int Width, unsigned int Height,unsigned int MipLevels, TexFormat format, bool renderTarget)
{
	if (Width==SIZE_SCREEN) Width = g_pEngSet.Width; 
	if (Height==SIZE_SCREEN) Height = g_pEngSet.Height;

	if (Width==SIZE_SCREEN_HALF) Width = g_pEngSet.Width / 2; 
	if (Height==SIZE_SCREEN_HALF) Height = g_pEngSet.Height / 2;

	if (Width==SIZE_SCREEN_QUART) Width = g_pEngSet.Width / 4; 
	if (Height==SIZE_SCREEN_QUART) Height = g_pEngSet.Height / 4;

	STAT(STAT_TEXTURE_ALL_TEXELS, Width*Height);
	HRESULT hr = D3DXCreateTexture(g_pD3DDevice, Width, Height, MipLevels, renderTarget ? D3DUSAGE_RENDERTARGET : 0, g_TexFormat[format], renderTarget ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED, &m_pTexture);

	if (SUCCEEDED(hr) && m_pTexture) 
		m_texDesc.type = renderTarget ? TT_RT : TT_2D;
	else
		m_texDesc.type = TT_NONE;

	m_texDesc.width = Width;
	m_texDesc.height = Height;
	m_texDesc.mipmaps = MipLevels;
	m_texDesc.format = format;

	m_pTexture->GetSurfaceLevel(0, &m_pSurface);

	return (m_texDesc.type!=TT_NONE);
}

void CP3DTexture::GetSurfaceData(unsigned int SurfaceLevel, P3DLOCKED_RECT *pRect)
{
	// Grab the mipmap level and lock the surface
	if (SUCCEEDED (m_pTexture->GetSurfaceLevel(SurfaceLevel, &m_pSurface)))
		m_pSurface->LockRect((D3DLOCKED_RECT*)pRect, NULL, 0);
}
void CP3DTexture::SaveSurfaceData()
{
	if (m_pSurface) m_pSurface->UnlockRect();
}

void CP3DTexture::Filter(unsigned long ulFilter)
{
	if (m_pTexture) D3DXFilterTexture(m_pTexture, 0, 0, ulFilter);
}

void *CP3DTexture::GetD3DTexture()
{
	return (void*)m_pTexture;
}

void *CP3DTexture::GetD3DSurface(unsigned int SurfaceLevel)
{
	IDirect3DSurface9 *m_pSurf=NULL;
	m_pTexture->GetSurfaceLevel(SurfaceLevel, &m_pSurf);
	return (void*)m_pSurf;
}

void CP3DTexture::GetDesc(TexDesc &desc)
{
	if (!m_pTexture) return;

	desc = m_texDesc;

	if (m_texDesc.width==SIZE_SCREEN) desc.width = g_pEngSet.Width; 
	if (m_texDesc.height==SIZE_SCREEN) desc.height = g_pEngSet.Height;

	if (m_texDesc.width==SIZE_SCREEN_HALF) desc.width = g_pEngSet.Width / 2; 
	if (m_texDesc.height==SIZE_SCREEN_HALF) desc.height = g_pEngSet.Height / 2;

	if (m_texDesc.width==SIZE_SCREEN_QUART) desc.width = g_pEngSet.Width / 4; 
	if (m_texDesc.height==SIZE_SCREEN_QUART) desc.height = g_pEngSet.Height / 4;
}

bool CP3DTexture::SetAsRenderTarget(DWORD rtIndex)
{
	if (SUCCEEDED(g_pD3DDevice->SetRenderTarget(rtIndex, m_pSurface)))
		return true;
	else
		return false;
}

bool CP3DTexture::CaptureCurrentBackBuffer()
{
	if (SUCCEEDED(g_pD3DDevice->StretchRect((IDirect3DSurface9*)g_pRenderer->GetBackBufferSurface(), NULL, m_pSurface, NULL, D3DTEXF_NONE)))
		return true;
	else
		return false;
}
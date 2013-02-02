//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Renderer interface (renderer)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include "ip3dfont.h"
#include "ip3dconsole.h"
#include "ip3dengine.h"
#include "hp3dmatrix.h"
#include "IP3DTexture.h"

enum RenderPrimitiveType_t {
	RPT_POINTLIST				  = 1,
	RPT_LINELIST              = 2,
	RPT_LINESTRIP             = 3,
	RPT_TRIANGLELIST          = 4,
	RPT_TRIANGLESTRIP         = 5,
	RPT_TRIANGLEFAN           = 6
};

enum RenderFVF_t {
	RFVF_XYZ               = 0x002,
	RFVF_XYZRHW            = 0x004,
	RFVF_XYZW              = 0x4002,
	RFVF_NORMAL            = 0x010,
	RFVF_PSIZE             = 0x020,
	RFVF_DIFFUSE           = 0x040,
	RFVF_SPECULAR          = 0x080,
	RFVF_TEX0              = 0x000,
	RFVF_TEX1              = 0x100,
	RFVF_TEX2              = 0x200,
	RFVF_TEX3              = 0x300,
	RFVF_TEX4              = 0x400,
	RFVF_TEX5              = 0x500,
	RFVF_TEX6              = 0x600,
	RFVF_TEX7              = 0x700,
	RFVF_TEX8              = 0x800,
	RFVF_XYZB1             = 0x006,
	RFVF_XYZB2             = 0x008,
	RFVF_XYZB3             = 0x00a,
	RFVF_XYZB4             = 0x00c,
	RFVF_XYZB5             = 0x00e,
	RFVF_POSITION_MASK     = 0x400E,
	RFVF_TEXCOUNT_MASK     = 0xf00,
	RFVF_TEXCOUNT_SHIFT    = 8
};

enum RenderScreenShotFormat {
	RXIFF_BMP = 0,
	RXIFF_JPG = 1,
	RXIFF_TGA = 2,
	RXIFF_PNG = 3,
	RXIFF_DDS = 4,
//	RXIFF_PPM = 5,
//	RXIFF_DIB = 6,
//	RXIFF_HDR = 7,
//	RXIFF_PFM = 8
};

enum RenderCmpFunc_t {
    RCMP_NEVER = 1,
    RCMP_LESS = 2,
    RCMP_EQUAL = 3,
    RCMP_LESSEQUAL = 4,
    RCMP_GREATER = 5,
    RCMP_NOTEQUAL = 6,
    RCMP_GREATEREQUAL = 7,
    RCMP_ALWAYS = 8
};

enum RenderBlendMode_t {
    RBLEND_ZERO = 1,
    RBLEND_ONE = 2,
    RBLEND_SRCCOLOR = 3,
    RBLEND_INVSRCCOLOR = 4,
    RBLEND_SRCALPHA = 5,
    RBLEND_INVSRCALPHA = 6,
    RBLEND_DESTALPHA = 7,
    RBLEND_INVDESTALPHA = 8,
    RBLEND_DESTCOLOR = 9,
    RBLEND_INVDESTCOLOR = 10,
    RBLEND_SRCALPHASAT = 11,
    RBLEND_BOTHSRCALPHA = 12,
    RBLEND_BOTHINVSRCALPHA = 13,
    RBLEND_BLENDFACTOR = 14,
    RBLEND_INVBLENDFACTOR = 15
};

enum RenderFillMode_t {
    RFILL_POINT = 1,
    RFILL_WIREFRAME = 2,
    RFILL_SOLID = 3
};

enum RenderCullMode_t {
    RCULL_NONE = 1,
    RCULL_CW = 2,
    RCULL_CCW = 3
};
enum RenderStageType_t {
    RST_COLOROP = 1,
    RST_COLORARG1 = 2,
    RST_COLORARG2 = 3,
    RST_ALPHAOP = 4,
    RST_ALPHAARG1 = 5,
    RST_ALPHAARG2 = 6,
    RST_BUMPENVMAT00 = 7,
    RST_BUMPENVMAT01 = 8,
    RST_BUMPENVMAT10 = 9,
    RST_BUMPENVMAT11 = 10,
    RST_TEXCOORDINDEX = 11,
    RST_BUMPENVLSCALE = 22,
    RST_BUMPENVLOFFSET = 23,
    RST_TEXTURETRANSFORMFLAGS = 24,
    RST_COLORARG0 = 26,
    RST_ALPHAARG0 = 27,
    RST_RESULTARG = 28,
    RST_CONSTANT = 32
};

enum RenderTransfType_t {
    RTT_VIEW = 2,
    RTT_PROJECTION = 3,
    RTT_TEXTURE0 = 16,
    RTT_TEXTURE1 = 17,
    RTT_TEXTURE2 = 18,
    RTT_TEXTURE3 = 19,
    RTT_TEXTURE4 = 20,
    RTT_TEXTURE5 = 21,
    RTT_TEXTURE6 = 22,
    RTT_TEXTURE7 = 23,
	RTT_WORLD = 256
};

enum RenderTextureAddressMode_t {
	RTAM_WRAP = 1,
	RTAM_MIRROR = 2,
	RTAM_CLAMP = 3,
	RTAM_BORDER = 4,
	RTAM_MIRRORONCE = 5
};

enum RenderLightType_t
{
	RLT_POINT = 1,
	RLT_SPOT = 2,
	RLT_DIRECTIONAL = 3
};

struct P3DLIGHT9 {
	RenderLightType_t Type;
	P3DColorValueRGBA Diffuse;
	P3DColorValueRGBA Specular;
	P3DColorValueRGBA Ambient;
	P3DXVector3D Position;
	P3DXVector3D Direction;
	float Range;
	float Falloff;
	float Attenuation0;
	float Attenuation1;
	float Attenuation2;
	float Theta;
	float Phi;
};


class IP3DRenderer : public IP3DBaseInterface
{
public:
	// ovládání vykreslování
	virtual bool InitRenderer(HWND hWnd)=0;
	virtual void BeginScene(unsigned long clrColor=P3DCOLOR_XRGB(100,100,100), bool bClearTarget=false, bool bClearStencil=false, bool bClearZBuffer=true)=0;
	virtual void EndScene()=0;
	virtual void Present( HWND hWnd )=0;
	virtual EngineSettings_t GetSettings()=0;
	// screenshot
	virtual bool TakeScreenShot(RenderScreenShotFormat Format = RXIFF_JPG)=0;
	virtual IP3DTexture* TakeScreenShotTexture()=0; // aplikace si pak texturu musi uvolnit sama pomoci delete
	// barvy
	virtual void SetAmbientColor( unsigned long color )=0;
	virtual void SetMaterialColor( unsigned long color )=0;
	// matice, transformace, pozice
	virtual void SetCamera (P3DXVector3D &EyePos, P3DXVector3D &ViewVector, P3DXVector3D &UpVector)=0;
	virtual void SetProjection (float fov, float fAspectRatio, float fZnear, float fZfar)=0;
	virtual void SetProjectionOrtho(float fWidth, float fHeight, float fZnear, float fZfar)=0;
	virtual void SetPosition( float pX, float pY, float pZ, float rX, float rY, float rZ )=0;
	virtual void SetWorldTransform (P3DXMatrix &mat)=0;
	virtual void GetTransform( RenderTransfType_t transfType, P3DXMatrix *mat )=0;
	virtual void SetFVF (RenderFVF_t dwNewFVF)=0;
	virtual void SetTexture (void* pTexture, DWORD dwStage = 0)=0;
	virtual void DrawPrimitiveUp (RenderPrimitiveType_t primType, UINT primCount, const void* pVertexData, UINT VertexStride)=0;
	virtual void GetCameraInfo (P3DXVector3D &camPos, float &FOV)=0; // doda ostatnim castem enginu
	// práce s depth bufferem
	virtual void ZEnable( bool bEnable )=0;
	virtual void ZWriteEnable( bool bEnable )=0;
	virtual void DepthFunc( RenderCmpFunc_t depthFunc )=0;
	// alpha blending
	virtual void EnableBlending( bool bEnable )=0;
	virtual void BlendFunc( RenderBlendMode_t srcMode, RenderBlendMode_t dstMode )=0;
	// alpha testing
	virtual void EnableAlphaTest( bool bEnable )=0;
	virtual void AlphaFunc( RenderCmpFunc_t alphaFunc)=0; //, float alphaRef /* [0-1] */ )=0;
	// separated alpha
	// ... TODO: zatím nebyla potøebná
	// výplò objektù (fill mode)
	virtual void SetFillMode( RenderFillMode_t fillMode )=0;
	// oøezávání (culling)
	virtual void SetCullMode( RenderCullMode_t cullMode )=0;
	// osvìtlování
	virtual void EnableLighting( bool bEnable )=0;
	virtual void SetLight (unsigned long index, P3DLIGHT9 &lightProperties)=0;
	virtual void LightEnable (unsigned long index, bool bEnable)=0;
	virtual void EnableSpecular( bool bEnable )=0; // musí být povoleno osvìtlování
	// vertex blending - napø. pro animace
	virtual void EnableVertexBlend( bool bEnable )=0;
	// mlha (fog)
	virtual void EnableFog( bool bEnable )=0;
	// TODO: D3DRS_RANGEFOGENABLE
	virtual void SetFogParams( float start,  float end, float density )=0; // density=[0-1]
	virtual void SetFogColor( unsigned long color )=0;
	// nastavování jednotlivých vrstev textur
	virtual void SetTextureStageState( unsigned long stage, RenderStageType_t stageType, unsigned long value )=0;
	// sampler states
	virtual void SetTextureAddressModeUV (RenderTextureAddressMode_t Value, unsigned long sampler = 0)=0;

	virtual void SetDefaultRenderTarget()=0;

	// nastavi dohodnute def. render states
	virtual void SetDefaultRenderStates ()=0;

	// transformace
	virtual void WorldToScreenSpace (P3DXVector3D &world, bool bCenterView)=0; // transformuje pozice z 3D do 2D podle akt. nastaveni projekce. Pokud je bCenterView true, nastavi view pohled na pozici 0 0 0

	// jine - pomocne
	virtual void* GetD3DDevice()=0;
	virtual void* GetBackBufferSurface()const=0;
	virtual void SaveSceneIntoCubeMap(float fX, float fY, float fZ)=0;
};

#define IP3DRENDERER_RENDERER "P3DRenderer_1" // nazev ifacu
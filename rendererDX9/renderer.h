//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose: Base renderer class declaration
//-----------------------------------------------------------------------------
#pragma once

#include "ip3drenderer.h"
#include "ip3dfont.h"
#include "d3dx9.h"
#include "texture.h"

class CP3DRenderer : public IP3DRenderer
{
public:
	// konstr., destr.
	CP3DRenderer();
	~CP3DRenderer();
	// ovládání vykreslování
	bool InitRenderer(HWND hWnd);
	void BeginScene(unsigned long clrColor=P3DCOLOR_XRGB(100,100,100), bool bClearTarget=false, bool bClearStencil=false, bool bClearZBuffer=true);
	void EndScene();
	void Present( HWND hWnd );
	EngineSettings_t GetSettings();
	// screenshot
	bool TakeScreenShot (RenderScreenShotFormat Format = RXIFF_JPG);
	IP3DTexture* TakeScreenShotTexture(); // aplikace si pak texturu musi uvolnit sama pomoci delete
	// barvy
	void SetAmbientColor( unsigned long color );
	void SetMaterialColor( unsigned long color );
	// matice, transformace, pozice
	void SetCamera (P3DXVector3D &EyePos, P3DXVector3D &ViewVector, P3DXVector3D &UpVector);
	void SetProjection (float fov, float fAspectRatio, float fZnear, float fZfar);
	void SetProjectionOrtho(float fWidth, float fHeight, float fZnear, float fZfar);
	void SetPosition( float pX, float pY, float pZ, float rX, float rY, float rZ );
	void SetWorldTransform (P3DXMatrix &mat);
	void GetTransform( RenderTransfType_t transfType, P3DXMatrix *mat );
	void SetFVF (RenderFVF_t dwNewFVF);
	void SetTexture (void* pTexture, DWORD dwStage = 0);
	void DrawPrimitiveUp (RenderPrimitiveType_t primType, UINT primCount, const void* pVertexData, UINT VertexStride);
	void GetCameraInfo (P3DXVector3D &camPos, float &FOV);
	// práce s depth bufferem
	void ZEnable( bool bEnable );
	void ZWriteEnable( bool bEnable );
	void DepthFunc( RenderCmpFunc_t depthFunc );
	// alpha blending
	void EnableBlending( bool bEnable );
	void BlendFunc( RenderBlendMode_t srcMode, RenderBlendMode_t dstMode );
	// alpha testing
	void EnableAlphaTest( bool bEnable );
	void AlphaFunc( RenderCmpFunc_t alphaFunc); //, float alphaRef /* [0-1] */ )=0;
	// separated alpha
	// ... TODO: zatím nebyla potøebná
	// výplò objektù (fill mode)
	void SetFillMode( RenderFillMode_t fillMode );
	// oøezávání (culling)
	void SetCullMode( RenderCullMode_t cullMode );
	// osvìtlování
	void EnableLighting( bool bEnable );
	void SetLight (unsigned long index, P3DLIGHT9 &lightProperties);
	void LightEnable (unsigned long index, bool bEnable);
	void EnableSpecular( bool bEnable ); // musí být povoleno osvìtlování
	// vertex blending - napø. pro animace
	void EnableVertexBlend( bool bEnable );
	// mlha (fog)
	void EnableFog( bool bEnable );
	// TODO: D3DRS_RANGEFOGENABLE
	void SetFogParams( float start,  float end, float density );
	void SetFogColor( unsigned long color );
	void SetTextureStageState( unsigned long stage, RenderStageType_t stageType, unsigned long value );
	// sampler states
	void SetTextureAddressModeUV (RenderTextureAddressMode_t Value, unsigned long sampler = 0);

	void SetDefaultRenderTarget();

	// nastavi dohodnute def. render states
	void SetDefaultRenderStates ();

	// transformace
	void WorldToScreenSpace (P3DXVector3D &world, bool bCenterView=false);

	// jine - pomocne
	void* GetD3DDevice();
	void* GetBackBufferSurface()const{return m_pBackBufferSurf;};
	void SaveSceneIntoCubeMap(float fX, float fY, float fZ);
private:
	// metody
	static void CV_SetGamma(ConVar* conVar);

	// promenne
	D3DCAPS9 m_caps;
	D3DPRESENT_PARAMETERS	m_pparams;
	void OnLostDevice();
	// posledni funkcni hodnoty - po uspesne zmene rozliseni nutno aktualizovat tyto hodnoty!!!
	int m_nWidth;
	int m_nHeight;
	// device capabilities
	int m_caps_max_anisotr;
	static D3DGAMMARAMP m_origGama;
	// camera vector pro WorldToScreenSpace
	P3DXVector3D m_cameraVector;
	// default render target
	IDirect3DSurface9* m_pBackBufferSurf;
};
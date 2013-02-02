//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Base renderer class definition
//-----------------------------------------------------------------------------
#include "common.h"
#include "renderer.h"
#include "IP3DDraw2D.h"
#include "bspmap.h"
#include "vertexbuffer.h"
#include "effect.h"

CP3DVertexBuffer vb;
CP3DEffect effect;

//#define	USE_PERFHUD			// odkomentovat pre pouzitie s NVPerfHUD

REGISTER_SINGLE_CLASS(CP3DRenderer, IP3DRENDERER_RENDERER); // zaregistruj tuto tøídu

extern CP3DBSPMap *g_pBSPMap;
D3DGAMMARAMP CP3DRenderer::m_origGama;

void CV_wireframe(ConVar* conVar);
ConVar CVr_wireframe("r_wireframe", "0", CVAR_NOT_STRING, "Vykreslit drátový model?");
static ConVar CVr_width("r_width", "800", CVAR_ARCHIVE | CVAR_NOT_STRING, "Sirka okna enginu. Nutno restartovat engine!");
static ConVar CVr_height("r_height", "600", CVAR_ARCHIVE | CVAR_NOT_STRING, "Výška okna enginu. Nutno restartovat engine!");
static ConVar CVr_windowed("r_windowed", "1", CVAR_ARCHIVE | CVAR_NOT_STRING, "Spustit engine v okne? Nutno restartovat engine!");
static ConVar CVr_multisample("r_multisample", "0", CVAR_ARCHIVE | CVAR_NOT_STRING);
static ConVar CVr_adapter("r_adapter", "0", CVAR_ARCHIVE | CVAR_NOT_STRING);
static ConVar CVr_ref("r_ref", "0", CVAR_ARCHIVE | CVAR_NOT_STRING);
static ConVar CVr_vsync("r_vsync", "0", CVAR_ARCHIVE | CVAR_NOT_STRING);
static ConVar CVr_texfilter("r_texfilter", "3", CVAR_ARCHIVE | CVAR_NOT_STRING, "Typ texturoveho filtrovani (1-point 2-linear 3-anisotropic)");
static ConVar CVr_texfilter_aniso("r_texfilter_aniso", "1", CVAR_ARCHIVE | CVAR_NOT_STRING, "Hodnota stupne anizotr. filtrovani (1 vypnuto, obvykle nastavitelne az do 16)");
static ConVar CVr_gamma("r_gamma", "100", CVAR_ARCHIVE | CVAR_NOT_STRING);

CP3DRenderer::CP3DRenderer()
{
}

CP3DRenderer::~CP3DRenderer()
{
	// --- vrat orig. hodnoty
	g_pD3DDevice->SetGammaRamp(0, D3DSGR_CALIBRATE, &m_origGama);

	// ukonèi zdroje
	#ifdef _DEBUG
	g_stats.Shutdown(); // STATS
	#endif

	SAFE_RELEASE(m_pBackBufferSurf);

	SAFE_RELEASE(g_pD3DDevice);
	SAFE_RELEASE(g_pD3D);

	SAFE_DELETE(g_pResMgr);
}

bool CP3DRenderer::InitRenderer(HWND hWnd)
{
	I_RegisterModule("rendererDX9");
	// memory leaks detection
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(152);

	g_pConsole = (IP3DConsole*)I_GetClass(IP3DENGINE_CONSOLE);
	g_pConsole->RegisterLastConVar(g_pLastConVar); // zaregistrovat ConVary tohoto dll projektu - NUTNÉ!

	g_pFS = (IP3DFileSystem*)I_GetClass(IP3DENGINE_FILESYSTEM); //get filesystem
	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
	g_pTimer = (IP3DTimer*)I_GetClass(IP3DENGINE_TIMER);

	// profiler
	Prof_stackPTR = g_pEngine->GetProf_stack();
	Prof_dummyPTR = g_pEngine->GetProf_dummy();
	Prof_StackAppendPTR = g_pEngine->GetProf_StackAppendFn();

	g_pEngSet.Width = CVr_width.GetInt();
	g_pEngSet.Height = CVr_height.GetInt();
	g_pEngSet.Windowed = CVr_windowed.GetBool();

	g_pEngSet.hWnd = hWnd;

	// zkontroluj, zda se shoduji verze .h a DLL
	if (!D3DXCheckVersion(D3D_SDK_VERSION, D3DX_SDK_VERSION))
		CON(MSG_CON_ERR, "Warning: Wrong DirectX DLL versions, please install latest DirectX!");

	// Vytvoø D3D objekt
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		CON(MSG_ERR_FATAL, "Can't create Direct3D object! Please install DirectX 9...");

	// test for depth buffer support - zatial sa nepouziva stencil
	// D3DFMT_D32, D3DFMT_D24X8, D3DFMT_D16
	D3DFORMAT	DepthBufFormat = D3DFMT_D16;
	if (SUCCEEDED (g_pD3D->GetDeviceCaps (D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_caps)))
		if (SUCCEEDED (g_pD3D->CheckDeviceFormat(m_caps.AdapterOrdinal, D3DDEVTYPE_HAL, \
							D3DFMT_X8R8G8B8, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X8)))
			DepthBufFormat = D3DFMT_D24X8;
			
	// test na vs a ps 2.0
	if(m_caps.VertexShaderVersion<D3DVS_VERSION(1,1) || m_caps.PixelShaderVersion<D3DPS_VERSION(2,0))
	{
		CON(MSG_ERR_FATAL, "Pixel shaders 2.0 not supported!");
	}
	
	// test na format backbufferu
	if(FAILED(g_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, CVr_ref.GetBool() ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL, 
										 D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8, g_pEngSet.Windowed)))
	{
		CON(MSG_ERR_FATAL, "Backbuffer format A8R8G8B8 not supported!");
	}

	// ziskej caps do member promennych
	m_caps_max_anisotr = (int)m_caps.MaxAnisotropy;

	// Set up the structure used to create the D3DDevice
	ZeroMemory( &m_pparams, sizeof(m_pparams) );
	m_pparams.Windowed = g_pEngSet.Windowed;
	m_pparams.hDeviceWindow = hWnd;
	m_pparams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_pparams.BackBufferCount = 1;
	m_pparams.BackBufferFormat = D3DFMT_A8R8G8B8;
	m_pparams.EnableAutoDepthStencil = TRUE;
	m_pparams.AutoDepthStencilFormat = DepthBufFormat;
	m_pparams.BackBufferWidth = g_pEngSet.Width;
	m_pparams.BackBufferHeight = g_pEngSet.Height;
	m_pparams.MultiSampleType = CVr_multisample.GetInt() ? D3DMULTISAMPLE_NONMASKABLE : D3DMULTISAMPLE_NONE;
	m_pparams.MultiSampleQuality = CVr_multisample.GetInt();
	m_pparams.PresentationInterval = CVr_vsync.GetBool() ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
	m_pparams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER | D3DPRESENT_LINEAR_CONTENT;

	m_nWidth = g_pEngSet.Width;
	m_nHeight = g_pEngSet.Height;

#ifdef _DEBUG
	CON(MSG_CON_INFO, "= DirectX 9 (D3D%d, D3DX%d) Renderer (%s, %s, DEBUG) initialization =", D3D_SDK_VERSION, D3DX_SDK_VERSION,__DATE__, __TIME__);
#else
	CON(MSG_CON_INFO, "= DirectX 9 (D3D%d, D3DX%d) Renderer (%s, %s) initialization =", D3D_SDK_VERSION, D3DX_SDK_VERSION, __DATE__, __TIME__);
#endif

	// vypis nazev a info o grafarne do konzole
	D3DADAPTER_IDENTIFIER9 ai;
	if(SUCCEEDED(g_pD3D->GetAdapterIdentifier(CVr_adapter.GetInt(), 0, &ai)))
	{
		CON(MSG_CON_INFO, "Renderer: %s", ai.Description);
	}

	// vytvoreni zarizeni
#ifdef USE_PERFHUD
	// debug pre NVPerfHUD
	if (FAILED (g_pD3D->CreateDevice (g_pD3D->GetAdapterCount()-1, D3DDEVTYPE_REF, hWnd, \
	D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_pparams, &g_pD3DDevice)))
#else
	if (FAILED (g_pD3D->CreateDevice( CVr_adapter.GetInt(), CVr_ref.GetBool() ? D3DDEVTYPE_REF : D3DDEVTYPE_HAL, hWnd, \
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_pparams, &g_pD3DDevice)))
#endif
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't create rendering device!\n\rTry restart or reinstall game...\n\rTry reinstall DirectX and graphic drivers...");
		return false;
	}

	g_pResMgr = new CP3DResourceManager;
	g_pResMgr->Initialize();

	g_pFrustum = (IP3DFrustum*)I_GetClass(IP3DRENDERER_FRUSTUM);
	if (g_pFrustum == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain Frustum class!");
		return false;
	}
	g_pMeshLoader = (IP3DMeshLoader*)I_GetClass(IP3DRENDERER_MESHLOADER);
	if (g_pMeshLoader == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain MeshLoader!");
		return false;
	}
	g_pAlphaManager = (IP3DAlphaManager*)I_GetClass(IP3DRENDERER_ALPHAMANAGER);
	if (g_pAlphaManager == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain AlphaManager!");
		return false;
	}

	g_pXML = (IP3DXML*)I_GetClass(IP3DENGINE_XML);
	if (g_pXML == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain XML class!");
		return false;
	}

	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	if (g_pMaterialManager == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain MaterialManager!");
		return false;
	}
	float ViewportSize[2];
	ViewportSize[0] = 1.0f / float(g_pEngSet.Width);
	ViewportSize[1] = 1.0f / float(g_pEngSet.Height);
	g_pMaterialManager->OnViewportSizeChange (ViewportSize);

	g_pPostProcessMgr = (IP3DPostprocessManager*)I_GetClass(IP3DRENDERER_POSTPROCESSMANAGER);
	if (g_pPostProcessMgr == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain PostProcessManager!");
		return false;
	}

	g_pPhysEngine = (IP3DPhysEngine*)I_GetClass(IP3DPHYS_PHYSENGINE);
	if (g_pPhysEngine == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain Physics Engine Class!");
		return false;
	}

	g_pLightManager = (IP3DLightManager*)I_GetClass(IP3DRENDERER_LIGHTMANAGER);
	if (g_pLightManager == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain DynlightManager!");
		return false;
	}

	if ( !g_TextureLoader.Init() ) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't initialize TextureLoader!");
		return false;
	}

	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
	if (g_pSoundMan == NULL) 
	{
		CON(MSG_ERR_FATAL, "Renderer: Can't obtain SoundManager!");
		return false;
	}

	SetDefaultRenderStates ();

	// nastavenie sampler states pre potreby shader modelu 3.0
	g_pD3DDevice->SetSamplerState(4, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(4, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(4, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(5, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(5, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(5, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(6, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(6, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(6, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(7, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(7, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(7, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

	// --- get original parameters
	g_pD3DDevice->GetGammaRamp(0, &m_origGama);

	// get default render target
	if(FAILED(g_pD3DDevice->GetRenderTarget(0, &m_pBackBufferSurf)))

	CON(MSG_CON_INFO, "Renderer: DX9 %d x %d %s initialized!", m_pparams.BackBufferWidth, m_pparams.BackBufferHeight, m_pparams.Windowed ? "windowed" : "fullscreen");
	// nastavit callbacky ConVar-ov
	CVr_wireframe.SetChangeCallback(CV_wireframe);
	CVr_gamma.SetChangeCallback(&CP3DRenderer::CV_SetGamma);

	#ifdef _DEBUG
	g_stats.Init(); // STATS
	#endif

	/////////////////
	tmpVertPosClr myvert[] =
	{
		{P3DXPoint3D(-50,0,0), P3DXPoint3D(1,0,0)},
		{P3DXPoint3D(-50,50,0), P3DXPoint3D(0,1,0)},
		{P3DXPoint3D(50,50,0), P3DXPoint3D(0,0,1)}
	};
	P3DVertexElement ve[3];
	ve[0] = P3DVertexElement(P3DVD_FLOAT3, P3DVU_POSITION);
	ve[1] = P3DVertexElement(P3DVD_FLOAT3, P3DVU_COLOR);
	ve[2] = P3DVE_END();

	vb.CreateVB(3, ve, sizeof(tmpVertPosClr));
	tmpVertPosClr *pv = 0;
	
	vb.Lock((void**)&pv);
		pv[0] = myvert[0];
		pv[1] = myvert[1];
		pv[2] = myvert[2];
	vb.UnLock();
	effect.Create("solid.fx");
	////////////////

	return true;
}

void CV_wireframe(ConVar* conVar)
{
	STAT(STAT_SET_RENDER_STATE, 1);

	if(conVar->GetBool())
		g_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	else
		g_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
}

void CP3DRenderer::CV_SetGamma(ConVar* conVar)
{
	float gamma = conVar->GetFloat();
	int i;
	D3DGAMMARAMP gr;

	/*for(int i=0; i<256; i++)
	{
		adGam = 32768.0f/255.0f*-(i-255.0f)*gam;
		gr.red[i] = (WORD)(m_origGama.red[i]*255.0f+adGam);
		gr.green[i] = (WORD)(m_origGama.green[i]*255.0f+adGam);
		gr.blue[i] = (WORD)(m_origGama.blue[i]*255.0f+adGam);
	}*/

	if ( gamma <= 0.0 )  // 0.0 gamma je cerna 
	{
		for ( i=0; i<256; ++i ) 
			gr.red[i] = gr.green[i] = gr.blue[i] = 0;
	} 
	else		// 1.0 gamma je normalni
		if ( gamma == 1.0 ) 
		{
			for (i=0; i<256; ++i) 
				gr.red[i] = gr.green[i] = gr.blue[i] = (i << 8) | i;
		} 
		else // gamma je urcita
		{ 
			int value;
			gamma = 1.0f/gamma;

			for (i=0; i<256; ++i) 
			{
				value = (int)(pow(i/256.0f, gamma)*65535.0f+0.5f);

				if (value>65535) value = 65535;

				gr.red[i] = gr.green[i] = gr.blue[i] = value;
			}
		}


	g_pD3DDevice->SetGammaRamp(0, D3DSGR_CALIBRATE, &gr);
}

void CP3DRenderer::BeginScene(unsigned long clrColor, bool bClearTarget, bool bClearStencil, bool bClearZBuffer)
{
	#ifdef _DEBUG
	g_stats.BeginNewFrame(); // STATS
	#endif

	if (CVr_wireframe.GetBool()) bClearTarget = true; // když je wireframe vykreslování, vymaž target, protože muže jít videt až na konec scény (delalo by to "bordel")

	bClearTarget = true;		// FIXME: debug !!!

	// Set clear params
	DWORD flags=0;
	if (bClearTarget) flags |= D3DCLEAR_TARGET;
	if (bClearStencil) flags |= D3DCLEAR_STENCIL;
	if (bClearZBuffer) flags |= D3DCLEAR_ZBUFFER;

	if (flags)
		g_pD3DDevice->Clear( 0, NULL, flags, clrColor, 1.0f, 0 );

	// Begin the scene
	g_pD3DDevice->BeginScene();
}

void CP3DRenderer::EndScene()
{
	P3DXMatrix matvp = g_matViewProj;
	P3DXMatrix matw;
	P3DXMatrix matwvp = matw * matvp;
	matw.SetIdentityMatrix();
	SetWorldTransform(matw);
	vb.Use();
	//effect.SetValue("gWVP", &matwvp, sizeof(P3DXMatrix));
	//effect.Begin("SolidTech");
	vb.Render(P3DPT_TRIANGLELIST, 0, 1);
	//effect.End();
	
	
	Prof(RENDERER_CP3DRenderer__EndScene);

/*
	// FIXME: TODO: debug !!!
	// testovat ci je vyhodne odoslat command buffer na spracovanie
	LPDIRECT3DQUERY9 pEventQuery = NULL;
	if (SUCCEEDED (g_pD3DDevice->CreateQuery(D3DQUERYTYPE_EVENT, &pEventQuery)))
	{
		pEventQuery->Issue(D3DISSUE_END);	// add an end marker to the command buffer queue.
		pEventQuery->GetData (NULL, 0, D3DGETDATA_FLUSH);	// empty the command buffer
		pEventQuery->Release ();
	}
	// FIXME: TODO: debug !!!
*/

	#ifdef _DEBUG
	g_stats.EndNewFrame();
	g_stats.DrawStats(); // STATS
	#endif

	g_pResMgr->RenderStats();

	// End the scene
	g_pD3DDevice->EndScene();
	// KEX: Proc?
}

void CP3DRenderer::Present(HWND hWnd)
{
	Prof(RENDERER_CP3DRenderer__Present);

	// Present the backbuffer contents to the display
	if (g_pD3DDevice->Present( NULL, NULL, hWnd, NULL ) == D3DERR_DEVICELOST)
		OnLostDevice();
}

EngineSettings_t CP3DRenderer::GetSettings()
{
	EngineSettings_t set;
	set.Width = m_pparams.BackBufferWidth;
	set.Height = m_pparams.BackBufferHeight;
	set.Windowed = m_pparams.Windowed;
	set.hWnd = m_pparams.hDeviceWindow;
	return set;
}

// obnovenie lost device
// je potrebne uvolnit vsetky zdroje, ktore su D3DPOOL_DEFAULT a znova ich obnovit
//---------------------------------
void CP3DRenderer::OnLostDevice()
{
	HRESULT hr;
	while ((hr = g_pD3DDevice->TestCooperativeLevel ()) == D3DERR_DEVICELOST)
	{
		// spracovanie sprav, napr. pre pripad uzatvorenia okna uzivatelom, bude to fungovat ???
		// KEX: S NULL hwnd asi ne ;) ael asi ani neni potreba vubec
		MSG msg;
		while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
			if (msg.message == WM_QUIT)
			{PostQuitMessage (0); return;}
		}
		Sleep (10);
	}

	if (hr == S_OK)
		return;	// ziadna chyba
	else if (hr == D3DERR_DRIVERINTERNALERROR)
	{
		CON (MSG_ERR_FATAL, "Internal driver error! Can't recover lost device. Try restart game.");
		return;
	}
	else if (hr != D3DERR_DEVICENOTRESET)
		return;		// neznama chyba

	//teraz je mozne obnovit device

	/*
	Nasleduje uvolnenie vsetkych D3DPOOL_DEFAULT (video-memory) zdrojov - textury, VB, IB, atd.
	Okrem D3DPOOL_DEFAULT zdrojov je treba uvolnit aj pripadne vytvorene swap chains,
	render targets, depth stencil surfaces a state blocks.
	Zdroje v D3DPOOL_SYSTEMMEM alebo D3DPOOL_MANAGED nie potrebne obnovovat, pretoze sa nachadzaju
	v system memory, alebo su aspon v nej "zalohovane".
	Taktiez treba obnovit vsetky predosle "device states", shadery nie je treba obnovovat.
	Ak neboli uvolnene vsetky D3DPOOL_DEFAULT zdroje fcia g_pD3DDevice->Reset() bude neuspesna!

	Before calling the IDirect3DDevice9::Reset method for a device, an application should release
	any explicit render targets, depth stencil surfaces, additional swap chains, state blocks,
	and D3DPOOL_DEFAULT resources associated with the device.
	*/

	//uvolnenie zdrojov ...				TODO: pridat !!!
	CON(MSG_ERR_FATAL, "OnLostDevice() not yet implemented, please don't use ALT+TAB!\r\nQuitting...");
	exit(-1);

	//resetování zaøízení
	if (SUCCEEDED (g_pD3DDevice->Reset (&m_pparams)))
	{		
		/*
		nasledujuci kod by mal byt rovnaky ako pri starte programu (vytvorenie zariadenia a zdrojov)
		s tym rozdielom, ze nie je potrebne znovu vytvarat D3DPOOL_SYSTEMMEM a D3DPOOL_MANAGED zdroje
		*/

		//obnovenie zdrojov ...				TODO: pridat !!!
		// pridat pripadnu notifikaciu casti programu ktore to vyzaduju, napr. AlphaManager a pod.
	}

	SetDefaultRenderStates ();
}

// ulozi screnshot do Data\SnapShots
bool CP3DRenderer::TakeScreenShot (RenderScreenShotFormat Format)
{
	if (GetFileAttributes ("data\\snapshots\\") == 0xFFFFFFFF)
		if (!CreateDirectory ("data\\snapshots\\", NULL))
			return false;

	LPDIRECT3DSURFACE9 ScreenShotSurface = NULL;
	if (FAILED(g_pD3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &ScreenShotSurface)))
		return false;

	char szFileName[32];
	for (DWORD i=0; i<100; i++)
	{
		wsprintf (szFileName, "data\\snapshots\\screen%02i.", i);
		switch (Format)
		{
		case RXIFF_BMP:
			strcat (szFileName, "bmp"); break;
		case RXIFF_JPG:
			strcat (szFileName, "jpg"); break;
		case RXIFF_TGA:
			strcat (szFileName, "tga"); break;
		case RXIFF_PNG:
			strcat (szFileName, "png"); break;
		case RXIFF_DDS:
			strcat (szFileName, "dds");
		}
		if (g_pFS->Exists(szFileName))
		{
			if (SUCCEEDED (D3DXSaveSurfaceToFile(szFileName, (D3DXIMAGE_FILEFORMAT)Format, ScreenShotSurface, NULL, NULL)))
			{ScreenShotSurface->Release(); return true;}
			break;
		}
	}
	SAFE_RELEASE (ScreenShotSurface)
		return false;
}

//---------------------------------
IP3DTexture* CP3DRenderer::TakeScreenShotTexture ()
{
	LPDIRECT3DSURFACE9 ScreenShotSurface = NULL;
	if (FAILED(g_pD3DDevice->GetBackBuffer (0, 0, D3DBACKBUFFER_TYPE_MONO, &ScreenShotSurface)))
		return NULL;

	// vytvor texturu
	CP3DTexture* pTex = new CP3DTexture();
	pTex->Create(m_nWidth, m_nHeight, 1, TF_RGB8, false);

	P3DLOCKED_RECT pRect;
	pRect.pBits = NULL;
	// ziskej data
	ScreenShotSurface->LockRect((D3DLOCKED_RECT*)&pRect, NULL, 0);
	if (!pRect.pBits) goto quitHand;

	// ziskej cilova data
	P3DLOCKED_RECT pRectCil;
	pTex->GetSurfaceData(0, &pRectCil);

	// zkopiruj data
	if(pRectCil.Pitch==pRect.Pitch) // pokud se shoduji Pitch, je mozno pouzit rychle memcpy
		memcpy(pRectCil.pBits, pRect.pBits, sizeof(char)*m_nWidth*m_nHeight*4);
	else // pokud se neshoduji Pitch, musi se to udelat timto slozitym zpusobem
	{
		unsigned char* pSurface = static_cast<unsigned char*>(pRect.pBits);
		unsigned char* pTarSurface = static_cast<unsigned char*>(pRectCil.pBits);
		for (unsigned long r = 0; r < (unsigned int)m_nHeight; r++)
		{
			unsigned long dwIndexTar = r * pRectCil.Pitch;
			unsigned long dwIndex = r * pRect.Pitch;
			for (unsigned long c = 0; c < (unsigned int)m_nWidth; c++)
			{
				pTarSurface[dwIndexTar + c*4 + 0] = pSurface[dwIndex + c*4 + 0];
				pTarSurface[dwIndexTar + c*4 + 1] = pSurface[dwIndex + c*4 + 1];
				pTarSurface[dwIndexTar + c*4 + 2] = pSurface[dwIndex + c*4 + 2];
			}
		}
	}

	// uzavri zdroj i cil
	ScreenShotSurface->UnlockRect();
	pTex->SaveSurfaceData();

	SAFE_RELEASE (ScreenShotSurface) // surface uz neni potreba

		return (IP3DTexture*)pTex; // vrat hotovou texturu

quitHand:
	if(pTex) delete pTex;
	return NULL;
}

//---------------------------------
void CP3DRenderer::SetAmbientColor( unsigned long color )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_AMBIENT, color );
}

// --------------------------------
void CP3DRenderer::SetMaterialColor( unsigned long color )
{
	D3DMATERIAL9 mat;
	mat.Emissive.a = (color>>24&0xff)/255.0f; mat.Emissive.r = (color>>16&0xff)/255.0f; mat.Emissive.g = (color>>8&0xff)/255.0f; mat.Emissive.b = (color&0xff)/255.0f;
	mat.Ambient = mat.Emissive;
	mat.Diffuse = mat.Emissive;
	mat.Specular = mat.Emissive;
	g_pD3DDevice->SetMaterial(&mat);
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	g_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
}

//---------------------------------
void CP3DRenderer::SetCamera (P3DXVector3D &EyePos, P3DXVector3D &ViewVector, P3DXVector3D &UpVector)
{
	P3DXVector3D LookAtPos;
	LookAtPos.x = EyePos.x + ViewVector.x;
	LookAtPos.y = EyePos.y + ViewVector.y;
	LookAtPos.z = EyePos.z + ViewVector.z;
	m_cameraVector = ViewVector;
	m_cameraVector.Normalize();

	D3DXMatrixLookAtLH (&g_matView, (D3DXVECTOR3*)&EyePos, (D3DXVECTOR3*)&LookAtPos, (D3DXVECTOR3*)&UpVector);
	g_pD3DDevice->SetTransform (D3DTS_VIEW, &g_matView);
	D3DXMatrixMultiply (&g_matViewProj, &g_matView, &g_matProj);
	// update (shader) global parameters
	g_cameraPosition = EyePos;
	g_pFrustum->CalculateFrustum (g_matView, g_matProj);
	if (g_pBSPMap) g_pBSPMap->CameraUpdate (g_cameraPosition);
	g_pMaterialManager->OnViewMatrixChange (&LookAtPos);
}

void CP3DRenderer::SetProjection (float fov, float fAspectRatio, float fZnear, float fZfar)
{
	D3DXMatrixPerspectiveFovLH (&g_matProj, fov, fAspectRatio, fZnear, fZfar);
	g_pD3DDevice->SetTransform (D3DTS_PROJECTION, &g_matProj);
	D3DXMatrixMultiply (&g_matViewProj, &g_matView, &g_matProj);
	// update (shader) global parameters
	g_currentFOV = fov;
	g_pFrustum->CalculateFrustum (g_matView, g_matProj);
	g_pMaterialManager->OnProjectionMatrixChange ();
}

void CP3DRenderer::SetProjectionOrtho(float fWidth, float fHeight, float fZnear, float fZfar)
{
	D3DXMatrixOrthoLH (&g_matProj, fWidth, fHeight, fZnear, fZfar);
	g_pD3DDevice->SetTransform (D3DTS_PROJECTION, &g_matProj);
	D3DXMatrixMultiply (&g_matViewProj, &g_matView, &g_matProj);
	// update (shader) global parameters
	//g_currentFOV = fov; // doesn't need to zero or sth similar?
	g_pFrustum->CalculateFrustum (g_matView, g_matProj);
	g_pMaterialManager->OnProjectionMatrixChange ();
}

void CP3DRenderer::SetPosition( float pX, float pY, float pZ, float rX, float rY, float rZ )
{
	D3DXMATRIXA16 matOut;
	D3DXMATRIXA16 matP;
	D3DXMATRIXA16 matR;
	// UPD: pozri definiciu D3DXMatrixRotationYawPitchRoll()
	D3DXMatrixRotationYawPitchRoll( &matR, rY, rX, rZ );
	D3DXMatrixTranslation( &matP, pX, pY, pZ );
	D3DXMatrixMultiply ( &matOut, &matR, &matP );

	g_pD3DDevice->SetTransform( D3DTS_WORLD, &matOut ); 
}

void CP3DRenderer::SetWorldTransform (P3DXMatrix &mat)
{
	g_pD3DDevice->SetTransform (D3DTS_WORLD, (D3DMATRIX*)&mat);
}

void CP3DRenderer::GetTransform( RenderTransfType_t transfType, P3DXMatrix *mat )
{
	g_pD3DDevice->GetTransform((D3DTRANSFORMSTATETYPE)transfType, (D3DMATRIX*)mat);
}
void CP3DRenderer::SetFVF (RenderFVF_t dwNewFVF)
{
	g_pD3DDevice->SetFVF (dwNewFVF);
}
void CP3DRenderer::SetTexture (void* pTexture, DWORD dwStage)
{
	static LPDIRECT3DTEXTURE9	pLastTextures[8];
	if (pLastTextures[dwStage] != pTexture)
	{
		g_pD3DDevice->SetTexture (dwStage, (LPDIRECT3DTEXTURE9)pTexture);
		pLastTextures[dwStage] = (LPDIRECT3DTEXTURE9)pTexture;
	}
}
void CP3DRenderer::DrawPrimitiveUp (RenderPrimitiveType_t primType, UINT primCount, const void* pVertexData, UINT VertexStride)
{
	Prof(RENDERER_CP3DRenderer__DrawPrimitiveUp);
	STAT(STAT_DRAW_CALLS, 1);
	STAT(STAT_DRAW_VERTS, primCount);
	g_pD3DDevice->DrawPrimitiveUP ((D3DPRIMITIVETYPE)primType, primCount, pVertexData, VertexStride);
}
void CP3DRenderer::ZEnable( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_ZENABLE, bEnable);
}
void CP3DRenderer::ZWriteEnable( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, bEnable);
}
void CP3DRenderer::DepthFunc( RenderCmpFunc_t depthFunc )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_ZFUNC, depthFunc );
}
void CP3DRenderer::EnableBlending( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, bEnable );
}
void CP3DRenderer::BlendFunc( RenderBlendMode_t srcMode, RenderBlendMode_t dstMode )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_SRCBLEND, srcMode );
	g_pD3DDevice->SetRenderState( D3DRS_DESTBLEND, dstMode );
}
void CP3DRenderer::EnableAlphaTest( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE, bEnable );
}
void CP3DRenderer::AlphaFunc( RenderCmpFunc_t alphaFunc)
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, alphaFunc );
}
void CP3DRenderer::SetFillMode( RenderFillMode_t fillMode )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_FILLMODE, fillMode );
}
void CP3DRenderer::SetCullMode( RenderCullMode_t cullMode )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_CULLMODE, cullMode );
}
void CP3DRenderer::EnableLighting( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_LIGHTING, bEnable );
}
void CP3DRenderer::SetLight (unsigned long index, P3DLIGHT9 &lightProperties)
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetLight (index, (D3DLIGHT9*)&lightProperties);
}
void CP3DRenderer::LightEnable (unsigned long index, bool bEnable)
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->LightEnable (index, (BOOL)bEnable);
}

void CP3DRenderer::EnableSpecular( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, bEnable );
}
void CP3DRenderer::EnableVertexBlend( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_VERTEXBLEND, bEnable ? D3DVBF_3WEIGHTS : D3DVBF_DISABLE ); // TODO: num of WEIGHTS?
}
void CP3DRenderer::EnableFog( bool bEnable )
{
	STAT(STAT_SET_RENDER_STATE, 2);
	//if (m_caps.RasterCaps & D3DPRASTERCAPS_ZFOG)
	g_pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	g_pD3DDevice->SetRenderState( D3DRS_FOGENABLE, bEnable );
}
void CP3DRenderer::SetFogParams( float start,  float end, float density )
{
	STAT(STAT_SET_RENDER_STATE, 3);
	g_pD3DDevice->SetRenderState( D3DRS_FOGSTART, *((DWORD*)(&start)) );
	g_pD3DDevice->SetRenderState( D3DRS_FOGEND, *((DWORD*)(&end)) );
	g_pD3DDevice->SetRenderState( D3DRS_FOGDENSITY, *((DWORD*)(&density)) );
}
void CP3DRenderer::SetFogColor( unsigned long color )
{
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState( D3DRS_FOGCOLOR, color );
}
void CP3DRenderer::SetTextureStageState( unsigned long stage, RenderStageType_t stageType, unsigned long value )
{
	STAT(STAT_SET_TEXTURE_STAGE_STATE, 1);
	g_pD3DDevice->SetTextureStageState(stage, (D3DTEXTURESTAGESTATETYPE)stageType, value);
}
void CP3DRenderer::SetTextureAddressModeUV (RenderTextureAddressMode_t Value, unsigned long sampler)
{
	STAT(STAT_SET_SAMPLER_STATE, 2);
	g_pD3DDevice->SetSamplerState (sampler, D3DSAMP_ADDRESSU, Value);
	g_pD3DDevice->SetSamplerState (sampler, D3DSAMP_ADDRESSV, Value);
}

// nevolat prilis casto, iba raz po skonceni bloku programu ktory tieto parametre zmenil
void CP3DRenderer::SetDefaultRenderStates ()
{
	STAT(STAT_SET_RENDER_STATE, 9);
	// render states
	g_pD3DDevice->SetRenderState (D3DRS_ZENABLE, D3DZB_TRUE);
	g_pD3DDevice->SetRenderState (D3DRS_ZWRITEENABLE, TRUE);
	g_pD3DDevice->SetRenderState (D3DRS_FILLMODE, D3DFILL_SOLID);
	g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW);
	g_pD3DDevice->SetRenderState (D3DRS_ALPHATESTENABLE, FALSE);
	g_pD3DDevice->SetRenderState (D3DRS_ALPHABLENDENABLE, FALSE);
	g_pD3DDevice->SetRenderState (D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pD3DDevice->SetRenderState (D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	g_pD3DDevice->SetRenderState (D3DRS_LIGHTING, FALSE);
	// FIXME: potrebne???
	// Texture mapping
// 	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
// 	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
// 	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
	// FIXME: potrebne???

	// Texture filtering
	int texFilter = CVr_texfilter.GetInt();
	int texAniso = CVr_texfilter_aniso.GetInt();
	if (texFilter<1) texFilter=1;
	if (texFilter>3) texFilter=3;
	if (texAniso>m_caps_max_anisotr) texAniso=m_caps_max_anisotr;

	STAT(STAT_SET_SAMPLER_STATE, 14);
	// diffuse maps
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, texAniso );
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, texFilter );
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, texFilter );
	g_pD3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, texFilter );
	// light maps
	g_pD3DDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	// deluxel maps
	g_pD3DDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
	g_pD3DDevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );
	// normal maps
	g_pD3DDevice->SetSamplerState(3, D3DSAMP_MAXANISOTROPY, texAniso );
	g_pD3DDevice->SetSamplerState(3, D3DSAMP_MAGFILTER, texFilter );
	g_pD3DDevice->SetSamplerState(3, D3DSAMP_MINFILTER, texFilter );
	g_pD3DDevice->SetSamplerState(3, D3DSAMP_MIPFILTER, texFilter );
}

void CP3DRenderer::GetCameraInfo (P3DXVector3D &camPos, float &FOV)
{
	camPos = g_cameraPosition;
	FOV = g_currentFOV;
}

// jine - pomocne
void* CP3DRenderer::GetD3DDevice() { return g_pD3DDevice; }

//--------------------------------------------------------------------------------------
D3DXMATRIX GetCubeMapViewMatrix( DWORD dwFace, D3DXVECTOR3 vEyePt )
{
	//D3DXVECTOR3 vEyePt   = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vLookDir;
	D3DXVECTOR3 vUpDir;

	switch( dwFace )
	{
	case D3DCUBEMAP_FACE_POSITIVE_X:
		vLookDir = D3DXVECTOR3( vEyePt.x+1.0f, vEyePt.y+0.0f, vEyePt.z+0.0f );
		vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_X:
		vLookDir = D3DXVECTOR3(vEyePt.x-1.0f, vEyePt.y+0.0f, vEyePt.z+0.0f );
		vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	case D3DCUBEMAP_FACE_POSITIVE_Y:
		vLookDir = D3DXVECTOR3( vEyePt.x+0.0f, vEyePt.y+1.0f, vEyePt.z+0.0f );
		vUpDir   = D3DXVECTOR3( 0.0f, 0.0f, -1.0f );
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_Y:
		vLookDir = D3DXVECTOR3( vEyePt.x+0.0f, vEyePt.y-1.0f, vEyePt.z+0.0f );
		vUpDir   = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
		break;
	case D3DCUBEMAP_FACE_POSITIVE_Z:
		vLookDir = D3DXVECTOR3( vEyePt.x+0.0f, vEyePt.y+0.0f, vEyePt.z+1.0f );
		vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_Z:
		vLookDir = D3DXVECTOR3( vEyePt.x+0.0f, vEyePt.y+0.0f, vEyePt.z-1.0f );
		vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		break;
	}

	// Set the view transform for this cubemap surface
	D3DXMATRIXA16 mView;
	D3DXMatrixLookAtLH( &mView, &vEyePt, &vLookDir, &vUpDir );
	return mView;
}

//--------------------------------------------------------------------------------------
void CP3DRenderer::SaveSceneIntoCubeMap(float fX, float fY, float fZ)
{
	IDirect3DCubeTexture9* pCubeMap=NULL;
	P3DXVector3D pos;

	// kontrola zda je co renderovat
	if (!g_pBSPMap)
		return;

	// vytvor cubemapu
	g_pD3DDevice->CreateCubeTexture( 256/*=size*/,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&pCubeMap,
		NULL );

	// The projection matrix has a FOV of 90 degrees and asp ratio of 1
	D3DXMATRIXA16 mProj;
	D3DXMatrixPerspectiveFovLH( &mProj, D3DX_PI * 0.5f, 1.0f, 5.00f, 100000.0f );

	D3DXMATRIXA16 mViewDir;
	D3DXMatrixIdentity(&mViewDir);
	mViewDir._41 = fX; mViewDir._42 = fY; mViewDir._43 = fZ;
	pos.x = fX; pos.y = fY; pos.z = fZ;

	LPDIRECT3DSURFACE9 pRTOld = NULL;
	if (FAILED(g_pD3DDevice->GetRenderTarget(0, &pRTOld )))
	{
		CON(MSG_CON_ERR, "CP3DRenderer::RenderSceneIntoCubeMap: Can't get render target!");
		return;
	}



	IDirect3DSurface9*           g_pDepthCube = NULL;
	g_pD3DDevice->CreateDepthStencilSurface( 256,
		256,
		D3DFMT_D16,
		D3DMULTISAMPLE_NONE,
		0,
		TRUE,
		&g_pDepthCube,
		NULL );

	LPDIRECT3DSURFACE9 pDSOld = NULL;
	if( SUCCEEDED( g_pD3DDevice->GetDepthStencilSurface( &pDSOld ) ) )
	{
		// If the device has a depth-stencil buffer, use
		// the depth stencil buffer created for the cube textures.
		g_pD3DDevice->SetDepthStencilSurface( g_pDepthCube );
	}

	g_pD3DDevice->SetTransform(D3DTS_PROJECTION, &mProj); // pro fixed pipe
	g_matProj=mProj; // pro material managera
	D3DXMatrixMultiply (&g_matViewProj, &g_matView, &g_matProj);
	g_pMaterialManager->OnProjectionMatrixChange();

	D3DXMATRIXA16 mWorld;
	D3DXMatrixIdentity(&mWorld);
	g_pD3DDevice->SetTransform(D3DTS_WORLD, &mWorld);

	for( int nFace = 0; nFace < 6; nFace++ )
	{
		LPDIRECT3DSURFACE9 pSurf=NULL;

		pCubeMap->GetCubeMapSurface( (D3DCUBEMAP_FACES)nFace, 0, &pSurf );
		g_pD3DDevice->SetRenderTarget( 0, pSurf );
		SAFE_RELEASE( pSurf );

		g_pD3DDevice->Clear( 0L, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0x000000ff, 1.0f, 0L );

		// Begin the scene
		if( SUCCEEDED( g_pD3DDevice->BeginScene() ) )
		{
			D3DXVECTOR3 vec;
			vec.x = fX; vec.y = fY; vec.z = fZ;
			D3DXMATRIXA16 mView = GetCubeMapViewMatrix( nFace, vec);
			g_pD3DDevice->SetTransform(D3DTS_VIEW, &mView); // pro fixed pipe
			g_matView=mView; // pro material managera
			D3DXMatrixMultiply (&g_matViewProj, &g_matView, &g_matProj);
			g_pMaterialManager->OnViewMatrixChange();
			g_pFrustum->CalculateFrustum (g_matView, g_matProj);

			// Vsechno co je potreba vyrenderovat do cubemapy
			g_pBSPMap->Render();
			//g_pBSPMap->RenderCollisionGeometry();

			// End the scene.
			g_pD3DDevice->EndScene();
		}
	}

	// Restore depth-stencil buffer and render target
	if( pDSOld )
	{
		g_pD3DDevice->SetDepthStencilSurface( pDSOld );
		SAFE_RELEASE( pDSOld );
	}
	
	//DEBUG: Pouze pro pokusne ulozeni na disk, pak nejak vracet texturu

	// Restore render target
	g_pD3DDevice->SetRenderTarget( 0, pRTOld );
	SAFE_RELEASE( pRTOld );

	D3DXSaveTextureToFile("C:\\myCube.dds", D3DXIFF_DDS, pCubeMap, NULL );

	SAFE_RELEASE(pCubeMap);



	SAFE_RELEASE(g_pDepthCube);
}

void CP3DRenderer::WorldToScreenSpace (P3DXVector3D &world, bool bCenterView)
{
	P3DXMatrix tmp;
	tmp.SetIdentityMatrix();
	tmp.m_posit3 = world;
	P3DXMatrix matView = g_matView;
	P3DXMatrix matProj = g_matProj;

	if (bCenterView) 
	{
		P3DXVector3D up(0.0f, 1.0f, 0.0f);
		P3DXVector3D at(0,0,0);
		matView.LookAt(-m_cameraVector * 20, at, up);
	}

	tmp *= matView;
	tmp *= matProj;

	float x=tmp._41/tmp._44;
	float y=tmp._42/tmp._44;
	float z=tmp._43/*/tmp._44*/;

	world.x = (float)g_pEngSet.Width*(1+x)/2.0f;
	world.y = (float)g_pEngSet.Height*(1-y)/2.0f;
	world.z = z;
}

void CP3DRenderer::SetDefaultRenderTarget()
{
	g_pD3DDevice->SetRenderTarget(0, m_pBackBufferSurf);
	for (UINT i=1;i<m_caps.NumSimultaneousRTs;i++)
		g_pD3DDevice->SetRenderTarget(i, NULL);
}
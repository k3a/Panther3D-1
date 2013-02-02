//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (9.12.2006)
// Purpose:	Water Manager(Helper functions)
//-----------------------------------------------------------------------------

// TODO: Dokumentace

// Rendering plan:
// 1. RenderNormalScene
// 2. BeginRefractive()
//    RenderAllWaterPolygonsWithoutAnyShader - for clipmask (NormalSceneBackbuffer,alpha)
// 3. BeginReflective()
//    RenderReflectiveScene
// 4. BeginWaterSurface()
//	  RenderAllWaterPolygonsWithoutAnyShader
// 5. EndWaterSurface()

#include "water_manager.h"
#include "vertexbuffer.h"
#include "ip3ddraw2d.h"

REGISTER_SINGLE_CLASS (CP3DWaterManager, IP3DRENDERER_WATERMANAGER); // zaregistruj tuto tøídu

static ConVar CVr__water("r__water", "0", CVAR_ARCHIVE | CVAR_NOT_STRING, "Debug water surface");

static IP3DDraw2D *s_p2D=NULL;

struct VertexPosNorTex
{
	P3DXPoint3D pos;
	P3DXPoint3D nor;
	P3DXPoint2D tex;
};

CP3DWaterManager::CP3DWaterManager()
{
	m_pTexRefract = NULL;
	m_pSurfRefract = NULL;
	m_pTexReflect = NULL;
	m_pSurfReflect = NULL;
	m_pBackBufferSurf=NULL;
	m_pDepthStencil=NULL;
	m_pWaterDepthStencil=NULL;
}

//---------------------------------
CP3DWaterManager::~CP3DWaterManager()
{
	// refraction
	SAFE_RELEASE (m_pTexRefract);
	SAFE_RELEASE (m_pSurfRefract);

	// reflection
	SAFE_RELEASE (m_pTexReflect);
	SAFE_RELEASE (m_pSurfReflect);
	
	// vsechny depth stencily a back buffer
	SAFE_RELEASE (m_pBackBufferSurf);
	SAFE_RELEASE (m_pDepthStencil);
	SAFE_RELEASE (m_pWaterDepthStencil);
}

//---------------------------------
bool CP3DWaterManager::Init()
{
    // ------- VYTVOR RENDER TARGETS -------

	// REFRACTIVE
	// TODO: OPTIM: FINAL: WATER: Nutno optimalizovat velikost tehle textur!!! Staci asi mensi rozliseni
	if (FAILED (D3DXCreateTexture(g_pD3DDevice, g_pEngSet.Width>>1, g_pEngSet.Height>>1, 1, D3DUSAGE_RENDERTARGET, \
		D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, \
		&m_pTexRefract)))
	{ 
		CON(MSG_CON_ERR, "Water manager: Can't create texture for refraction!");
		return false;
	}

	if (FAILED (m_pTexRefract->GetSurfaceLevel (0, &m_pSurfRefract)))
	{ 
		CON(MSG_CON_ERR, "Water manager: Can't get surface level (1)!");
		return false;
	}

	// REFLECTIVE
	if (FAILED (D3DXCreateTexture(g_pD3DDevice, g_pEngSet.Width>>2, g_pEngSet.Height>>2, 1, D3DUSAGE_RENDERTARGET, \
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, \
		&m_pTexReflect)))
	{ 
		CON(MSG_CON_ERR, "Water manager: Can't create texture for reflection!");
		return false;
	}

	if (FAILED (m_pTexReflect->GetSurfaceLevel (0, &m_pSurfReflect)))
	{ 
		CON(MSG_CON_ERR, "Water manager: Can't get surface level (2)!");
		return false;
	}
	
	// ------- ZISKEJ BACK BUFFER, ORIGINAL DEPTHSTENCIL A VYTVOR NOVY DEPTHSTENCIL -------
	// backbuffer
	if(FAILED(g_pD3DDevice->GetRenderTarget(0, &m_pBackBufferSurf)))
	{
		CON(MSG_CON_ERR, "Water manager: Can't get device backbuffer surface!");
		return false;
	}
	// depthstencil 
	if(FAILED(g_pD3DDevice->GetDepthStencilSurface(&m_pDepthStencil)))
	{
		CON(MSG_CON_ERR, "Water manager: Can't get device depth stencil surface!");
		return false;
	}
	// vytvor depthstencil bez multisamplingu
	if(FAILED(g_pD3DDevice->CreateDepthStencilSurface(g_pEngSet.Width, g_pEngSet.Height, D3DFMT_D24X8, (D3DMULTISAMPLE_TYPE)0, 0, 0, &m_pWaterDepthStencil, 0)))
	{
		CON(MSG_CON_ERR, "Water manager: Can't create depth stencil surface!");
		return false;
	}

	// ------- NACTI MATERIAL PRO KRESLENI VODNI PLOCHY -------
	g_pMaterialManager = (IP3DMaterialManager*)I_GetClass(IP3DRENDERER_MATERIALMANAGER);
	if (g_pMaterialManager == NULL)
	{
		CON(MSG_CON_ERR, "Water manager: g_pMaterialManager == NULL!");
		return false;
	}

	m_dwMatWater = g_pMaterialManager->LoadMaterial("materials/spec_water.7mt");
	if (m_dwMatWater==0xFFFFFFFF)
	{
		CON(MSG_CON_ERR, "Water manager: Can't load spec_water.7mt!");
		return false;
	}
	
	// ------- NACTI MATERIAL PRO KRESLENI CLIPMASKY PRO REFR. Z VODNI PLOCHY -------
	m_dwMatClipmask = g_pMaterialManager->LoadMaterial("materials/spec_water_clip.7mt");
	if (m_dwMatClipmask==0xFFFFFFFF)
	{
		CON(MSG_CON_ERR, "Water manager: Can't load spec_water_clip.7mt!");
		return false;
	}

	s_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);
	if (!s_p2D) return false;

	return true;
}

/*!
* \brief
* Vola se pred kreslenim refractive sceny pro vodu.
* 
* Vola se pred kreslenim reflective sceny pro vodu. Vola se jako uple prvni metoda pri kresleni vody.
*/
void CP3DWaterManager::BeginRefractiveScene()
{
	// <Doslo k vykresleni normaln isceny>
	
	// ----- vykresli polygony vody do masky -----
	// bila v alfa kanalu bude maska (tyto mista se nebudou refractovat)
	STAT(STAT_SET_RENDER_STATE, 3);
	g_pD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);    
	g_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
	
	// Priprava shaderu na kresleni clipmasky
	g_pMaterialManager->SetMaterial(m_dwMatClipmask);
	
	// vykresli (probiha mimo tento manager)
	// FIXME: Pouze docasne pro test vytvorim a vykreslim plochu vody zde!!!
	float a = 500.0f;
	g_pRenderer->SetCullMode(RCULL_CW);

	/*
	CP3DVertexBuffer *vb = new CP3DVertexBuffer();
	vb->CreateVB(P3DVBTYPE_POSNORMAL1TEX, 4);
	P3DPOSNORMAL1TEX *pVertices = NULL;
	if (SUCCEEDED (vb->Lock ((void**)&pVertices, 4 * sizeof(P3DPOSRHWC))))
	{
		// * .
		// . .
		pVertices[0].x = -a;
		pVertices[0].y = 0.0f;
		pVertices[0].z = -a;
		pVertices[0].tu = 0.0f;
		pVertices[0].tv = 0.0f;

		// . *
		// . .
		pVertices[1].x = a;
		pVertices[1].y = 0.0f;
		pVertices[1].z = -a;
		pVertices[1].tu = 1.0f;
		pVertices[1].tv = 0.0f;

		// . .
		// * .
		pVertices[2].x = -a;
		pVertices[2].y = 0.0f;
		pVertices[2].z = a;
		pVertices[2].tu = 0.0f;
		pVertices[2].tv = 1.0f;

		// . .
		// . *
		pVertices[3].x = a;
		pVertices[3].y = 0.0f;
		pVertices[3].z = a;
		pVertices[3].tu = 1.0f;
		pVertices[3].tv = 1.0f;
		vb->UnLock();
	}

	vb->Use();
	vb->Render(P3DPT_TRIANGLESTRIP, 0, 2);
	delete vb;
	*/
}

/*!
 * \brief
 * Vola se pred kreslenim reflective sceny pro vodu.
 * 
 * Vola se po kresleni refractive sceny a pred kreslenim reflective sceny pro vodu.
 */
void CP3DWaterManager::BeginReflectiveScene(P3DXVector3D *reflCameraPos)
{
/*
	// UPD: nie je potrebne
	// Konec shaderu na kresleni clipmasky
	if (g_pMaterialManager->NextPass(m_dwMatClipmask))
	{
		// neni mozne vykreslit dalsi Pass pro clipmasku!
		CON(MSG_CON_ERR, "Water manager: Water clipmask shader with num passes > 1!!!");
	}
*/

	// ----- veci co mame jeste z refractive -----
	// nastav zpet stavy zarizeni
	STAT(STAT_SET_RENDER_STATE, 2);
	g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW); //FIXME: ? ok
	g_pD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x000000F);
	// zkopiruj backbuffer (akt. scenu) vcetne refr. masky v alfa kanalu do refrakcni textury
	g_pD3DDevice->StretchRect(m_pBackBufferSurf, NULL, m_pSurfRefract, NULL, D3DTEXF_NONE);

	// ----- nastav refl. target -----
	if (FAILED (g_pD3DDevice->SetRenderTarget (0, m_pSurfReflect))) return;
	
	// ----- nastav stencil a smaz zbuffer a target -----
	// FIXME: je potrebne zmazat target?
	g_pD3DDevice->SetDepthStencilSurface(m_pWaterDepthStencil);
	g_pD3DDevice->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255, 0, 0, 128), 1.0f, 0);
	
	// ----- nastav spravny clip plane -----
	// stavy zarizeni
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 1);
	
	// potrebne nastaveni
	D3DXMATRIX mWaterWorld;
	D3DXMatrixIdentity(&mWaterWorld); // FIXME: zatim je voda na 0 0 0

	// Reflection plane in local space.
	D3DXPLANE waterPlaneL(0.0f, -1.0f, 0.0f, 0.0f);
	// TODO: mozno tu chyba pouzitie D3DXPlaneNormalize() ???

/*
	// UPD: premenne sa k nicomu nepouzivaju
	// Reflection plane in world space.
	D3DXMATRIX WInvTrans;
	D3DXMatrixInverse(&WInvTrans, 0, &(mWaterWorld));
	D3DXMatrixTranspose(&WInvTrans, &WInvTrans);
	D3DXPLANE waterPlaneW;
	D3DXPlaneTransform(&waterPlaneW, &waterPlaneL, &WInvTrans);
*/
	// Reflection plane in homogeneous clip space.
	D3DXMATRIX WVPInvTrans;
	D3DXMatrixInverse(&WVPInvTrans, 0, &(mWaterWorld*g_matViewProj));
	D3DXMatrixTranspose(&WVPInvTrans, &WVPInvTrans);
	D3DXPLANE waterPlaneH;
	D3DXPlaneTransform(&waterPlaneH, &waterPlaneL, &WVPInvTrans);
	g_pD3DDevice->SetClipPlane(0, (float*)waterPlaneH);
	
	// prehod kameru
	m_matViewOld = g_matView;
	D3DXMATRIX tmp;
	D3DXPLANE pl;
	pl.a = 0.0f;
	pl.b = 1.0f;
	pl.c = 0.0f;
	pl.d = 0.0f; // WARN: voda na y=0
	//D3DXMatrixIdentity(&tmp);		// UPD: nie je potrebne
	D3DXMatrixReflect(&tmp, &pl);
	
	D3DXMatrixMultiply(&g_matView, &tmp, &m_matViewOld);
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &g_matView);
	g_pMaterialManager->OnViewMatrixChange(NULL);
	g_pFrustum->CalculateFrustum (g_matView, g_matProj);

	// UPD: asi nepouzitelna navratova hodnota ?
	if (reflCameraPos) 
	{
		reflCameraPos->x = g_matView._41; reflCameraPos->y = g_matView._42; reflCameraPos->z = g_matView._43;
	}
	//g_pD3DDevice->SetRenderState (D3DRS_CULLMODE, D3DCULL_CCW); // TODO: opacne cullmode kvuli reflekce
}

/*!
* \brief
* Vola se pred samotnym kreslenim vodniho meshe.
* 
* Pred kreslenim vodni plochy je nutno zavolat tuto metodu
*/
void CP3DWaterManager::BeginWaterSurface()
{
	// ----- vrat stavy zarizeni po reflekci -----
	g_pD3DDevice->SetRenderTarget(0, m_pBackBufferSurf);
	g_pD3DDevice->SetDepthStencilSurface(m_pDepthStencil);
	//g_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW); // TODO: Vratit zpet cullmode nastavene kvuli reflekce

	// prohod zpet kameru
	g_matView = m_matViewOld;
	g_pD3DDevice->SetTransform(D3DTS_VIEW, &g_matView);
	g_pMaterialManager->OnViewMatrixChange(NULL);

	// vratit zpet stavy zarizeni
	STAT(STAT_SET_RENDER_STATE, 1);
	g_pD3DDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);

	// Priprava shaderu na kresleni
	SetShaderParameters();
	g_pMaterialManager->SetMaterial(m_dwMatWater);

	// vykresli (probiha mimo tento manager)
	// FIXME: Pouze docasne pro test vytvorim a vykreslim plochu vody zde!!!

	// FIXME: Mess - podla mna bude lepsie ak sa vodna plocha bude kreslit vo water_manageri
	// water_manager by mal spravovat vsetko okolo vodnej plochy, teda aj test viditelnosti
	// ak nebude vodna plocha viditelna potom water_manager spravi okamzity navrat zo vsetkych svojich funkcii
	// ako to vidis ty?

	float a = 500.0f;
	g_pRenderer->SetCullMode(RCULL_CW);

	/*
	CP3DVertexBuffer *vb = new CP3DVertexBuffer();
	vb->CreateVB(P3DVBTYPE_POSNORMAL1TEX, 4);
	P3DPOSNORMAL1TEX *pVertices = NULL;
	if (SUCCEEDED (vb->Lock ((void**)&pVertices, 4 * sizeof(P3DPOSRHWC))))
	{
		// * .
		// . .
		pVertices[0].x = -a;
		pVertices[0].y = 0.0f;
		pVertices[0].z = -a;
		pVertices[0].tu = 0.0f;
		pVertices[0].tv = 0.0f;

		// . *
		// . .
		pVertices[1].x = a;
		pVertices[1].y = 0.0f;
		pVertices[1].z = -a;
		pVertices[1].tu = 1.0f;
		pVertices[1].tv = 0.0f;

		// . .
		// * .
		pVertices[2].x = -a;
		pVertices[2].y = 0.0f;
		pVertices[2].z = a;
		pVertices[2].tu = 0.0f;
		pVertices[2].tv = 1.0f;

		// . .
		// . *
		pVertices[3].x = a;
		pVertices[3].y = 0.0f;
		pVertices[3].z = a;
		pVertices[3].tu = 1.0f;
		pVertices[3].tv = 1.0f;
		vb->UnLock();
	}

	vb->Use();
	vb->Render(P3DPT_TRIANGLESTRIP, 0, 2);
	delete vb;
	*/
}

/*!
 * \brief
 * Ukonci vykreslovani vodni plochy.
 * 
 * Vola se po vykresleni meshe vodni plochy. Ukonci aktualni vodni shader.
 */
void CP3DWaterManager::EndWaterSurface()
{
/*
	// UPD: NextPass() je zbytocny
	// Konec shaderu na kresleni
	if (g_pMaterialManager->NextPass(m_dwMatWater))
	{
		// neni mozne vykreslit dalsi Pass pro vodu!
		CON(MSG_CON_ERR, "Water manager: Water shader with num passes > 1!!!");
	}
*/

	if (CVr__water.GetBool())
	{

		g_pMaterialManager->TurnOffShaders();
		g_pD3DDevice->SetTexture(0, m_pTexRefract);
		s_p2D->DrawTexture(NULL, 10, 10, 310, 310);
		g_pD3DDevice->SetTexture(0, m_pTexReflect);
		s_p2D->DrawTexture(NULL, 315, 10, 615, 310);
	}
}

/*!
 * \brief
 * Nastavi vsechny parametry shaderu.
 * 
 * Nastavi refractive a reflective textury a dalsi parametry pro shader.
 */
void CP3DWaterManager::SetShaderParameters()
{
	WORD wAllParams, wParamsToSet;

	g_pMaterialManager->QueryParameters (m_dwMatWater, false, g_paramTable, wAllParams, wParamsToSet);

	for (DWORD i=0; i<wParamsToSet; i++)
	{
		switch (g_paramTable[i].SemanticID)
		{
		case SEM_TextureLayer0:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &m_pTexRefract;
			break;
		case SEM_TextureLayer1:
			STAT(STAT_SHADER_TEXTURES, 1);
			g_paramTable[i].pValue = &m_pTexReflect;
			break;
		}

		g_pMaterialManager->SetParameters (m_dwMatWater, g_paramTable, wAllParams);
	}
}
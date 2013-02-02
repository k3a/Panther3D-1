//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Class for drawing 2d objects (line, texture, sphere, ...)
//-----------------------------------------------------------------------------

#include "draw2d.h"
#include "vertexbuffer.h"
#include "common.h"

REGISTER_SINGLE_CLASS(CP3DDraw2D, IP3DRENDERER_DRAW2D); // zaregistruj tuto tøídu

CP3DDraw2D::CP3DDraw2D()
{
	g_p2D = this;

	m_pLine = NULL;
	m_pSprite = NULL;
	m_pFont = NULL;

	m_bNeedSetWidth=false;
	m_fLineWidth=1;
}

CP3DDraw2D::~CP3DDraw2D()
{
	SAFE_RELEASE(m_pLine);
	SAFE_RELEASE(m_pSprite);
	SAFE_RELEASE(m_pFont);
}

// Not good code, but how to draw 2D triangles fast in shader-driven engine? :-D
void CP3DDraw2D::DrawTexture(IP3DTexture *pTexture, int x1, int y1, int x2, int y2, float tu1, float tv1, float tu2, float tv2)
{

	struct ScreenVertex
	{
		FLOAT x, y, z, rhw;
		FLOAT u, v;
	};

	DWORD tmpFill;
	STAT(STAT_SET_RENDER_STATE, 3);
	g_pD3DDevice->GetRenderState(D3DRS_FILLMODE, &tmpFill); // získej fill mode
	g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID); // nastav plné fill mode
	//g_pRenderer->SetCullMode(RCULL_CCW);

	ScreenVertex verts[4];
	verts[0].x = (FLOAT)x1-.5f; verts[0].y = (FLOAT)y1-.5f; verts[0].z = 0.0f; verts[0].rhw = 1.0f; verts[0].u = tu1; verts[0].v = tv1;
	verts[1].x = (FLOAT)x2-.5f; verts[1].y = (FLOAT)y1-.5f; verts[1].z = 0.0f; verts[1].rhw = 1.0f; verts[1].u = tu2; verts[1].v = tv1;
	verts[2].x = (FLOAT)x1-.5f; verts[2].y = (FLOAT)y2-.5f; verts[2].z = 0.0f; verts[2].rhw = 1.0f; verts[2].u = tu1; verts[2].v = tv2;
	verts[3].x = (FLOAT)x2-.5f; verts[3].y = (FLOAT)y2-.5f; verts[3].z = 0.0f; verts[3].rhw = 1.0f; verts[3].u = tu2; verts[3].v = tv2;

	// awful, i know :(
	g_pRenderer->SetTextureStageState(0, RST_COLOROP, D3DTOP_SELECTARG1 );
	g_pRenderer->SetTextureStageState(0, RST_COLORARG1, D3DTA_TEXTURE);
	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	g_pD3DDevice->SetSamplerState (0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	g_pD3DDevice->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
	g_pD3DDevice->SetTexture(0, (IDirect3DBaseTexture9 *)pTexture->GetD3DTexture());
	g_pD3DDevice->SetTexture(1, NULL);
	STAT(STAT_DRAW_CALLS, 1);
	STAT(STAT_DRAW_VERTS, 2);
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(ScreenVertex));

	g_pD3DDevice->SetRenderState(D3DRS_FILLMODE, tmpFill);
}

void CP3DDraw2D::DrawSquare(int x1, int y1, int x2, int y2, unsigned long color1)
{
	Line_DrawFilledRect(x1, y1, x2-x1, y2-y1, color1);
}

void CP3DDraw2D::Line_Begin()
{
	if (!m_pLine) D3DXCreateLine(g_pD3DDevice, &m_pLine);
	m_pLine->Begin();
}

void CP3DDraw2D::Line_End()
{
	if (!m_pLine) D3DXCreateLine(g_pD3DDevice, &m_pLine);
	m_pLine->End();
}

void CP3DDraw2D::Line_SetWidth(int w)
{
	m_fLineWidth = w;
	m_bNeedSetWidth = true;
}

void CP3DDraw2D::Line_DrawSimple(int x1, int y1, int x2, int y2, unsigned long color)
{
	P3DVector2D vec[2];
	vec[0].x = (float)x1; vec[0].y = (float)y1;
	vec[1].x = (float)x2; vec[1].y = (float)y2;

	Line_Draw(vec, 2, color);
}

void CP3DDraw2D::Line_Draw(P3DVector2D *points, DWORD numPoints, unsigned long color)
{
	if (!m_pLine) D3DXCreateLine(g_pD3DDevice, &m_pLine);
	if (m_bNeedSetWidth)
	{
		Line_End();
		m_pLine->SetWidth((float)m_fLineWidth);
		Line_Begin();
		m_bNeedSetWidth = false;
	}
	m_pLine->Draw((D3DXVECTOR2*)points, numPoints, color); // vykresli èáru
}

void CP3DDraw2D::Line_DrawFilledRect(int x, int y, int w, int h, unsigned long color)
{
//	Prof(RENDERER_CP3DDraw2D__Line_DrawFilledRect);

	struct ScreenVertex
	{
		FLOAT x, y, z, rhw;
		DWORD color;
	};

	ScreenVertex verts[4];
	verts[0].x = (FLOAT)x; verts[0].y = (FLOAT)y; verts[0].z = 0.0f; verts[0].rhw = 1.0f; verts[0].color = color;
	verts[1].x = (FLOAT)x+w; verts[1].y = (FLOAT)y; verts[1].z = 0.0f; verts[1].rhw = 1.0f; verts[1].color = color;
	verts[2].x = (FLOAT)x; verts[2].y = (FLOAT)y+h; verts[2].z = 0.0f; verts[2].rhw = 1.0f; verts[2].color = color;
	verts[3].x = (FLOAT)x+w; verts[3].y = (FLOAT)y+h; verts[3].z = 0.0f; verts[3].rhw = 1.0f; verts[3].color = color;

	//Line_End();

	g_pD3DDevice->SetFVF(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);
	g_pD3DDevice->SetTexture(0, NULL);
	STAT(STAT_DRAW_CALLS, 1);
	STAT(STAT_DRAW_VERTS, 2);
	g_pD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(ScreenVertex));
}

void CP3DDraw2D::Line_DrawRect(int x, int y, int w, int h, unsigned long color)
{
	P3DVector2D vec[5];
	vec[0].x = (float)x; vec[0].y = (float)y;
	vec[1].x = (float)x; vec[1].y = (float)y+h;
	vec[2].x = (float)x+w; vec[2].y = (float)y+h;
	vec[3].x = (float)x+w; vec[3].y = (float)y;
	vec[4].x = (float)x; vec[4].y = (float)y;

	Line_Draw(vec, 5, color);
}

void CP3DDraw2D::Line_Draw3DRect(int x, int y, int w, int h, unsigned long colorLeftTop, unsigned long colorRightBottom)
{
	P3DVector2D vec[4];
	// Left, Top
	vec[0].x = (float)x+w; vec[0].y = (float)y;
	vec[1].x = (float)x; vec[1].y = (float)y;
	vec[2].x = (float)x; vec[2].y = (float)y+h;
	Line_Draw(vec, 3, colorLeftTop);

	// Right, Bottom
	vec[0].x = (float)x; vec[0].y = (float)y+h;
	vec[1].x = (float)x+w; vec[1].y = (float)y+h;
	vec[2].x = (float)x+w; vec[2].y = (float)y;
	Line_Draw(vec, 3, colorRightBottom);
}

void CP3DDraw2D::Draw3DLine(P3DXVector3D &start, P3DXVector3D &end, unsigned long color)
{
	P3DXMatrix matTransform;
	matTransform.SetIdentityMatrix ();
	g_pRenderer->SetWorldTransform (matTransform);
	g_pRenderer->SetFVF (RFVF_XYZ);
	g_pRenderer->SetTexture (NULL);

	P3DXVector3D pPoints[2];
	pPoints[0]=start; pPoints[1]=end;

	if (color!=0) 
	{
		g_pRenderer->SetMaterialColor(color);
		g_pRenderer->EnableLighting(true);
	}

	g_pRenderer->DrawPrimitiveUp (RPT_LINESTRIP, 1, pPoints, 12);

	if (color!=0) g_pRenderer->EnableLighting(false);
}

void CP3DDraw2D::DrawAABB(P3DAABB &aabb, unsigned long color)
{
	if (color!=0) 
	{
		g_pRenderer->SetMaterialColor(color); // nastav dopredu jen jdenou, pak 0
		g_pRenderer->EnableLighting(true);
	}

	P3DXVector3D start, end;

	/*
				 H_________G MAX     A[min.x,min.y,min.z]
				 /|       /|         B[max.x,min.y,min.z]
				E--------F |         C[max.x,min.y,max.z]
				| |      | |         D[min.x,min.y,max.z]
			    | |D_____|_|C        E[min.x,max.y,min.z]
				|/       |/          F[max.x,max.y,min.z]
				A--------B           G[max.x,max.y,max.z]
			 MIN                     H[min.x,max.y,max,z]

        y z
		|/_x
	*/

	//AB
	start.x = aabb.min.x; start.y = aabb.min.y; start.z = aabb.min.z;
	end.x = aabb.max.x; end.y = aabb.min.y; end.z = aabb.min.z;
	Draw3DLine(start, end, 0);

	//BC
	start.x = aabb.max.x; start.y = aabb.min.y; start.z = aabb.min.z;
	end.x = aabb.max.x; end.y = aabb.min.y; end.z = aabb.max.z;
	Draw3DLine(start, end, 0);

	//CD
	start.x = aabb.max.x; start.y = aabb.min.y; start.z = aabb.max.z;
	end.x = aabb.min.x; end.y = aabb.min.y; end.z = aabb.max.z;
	Draw3DLine(start, end, 0);

	//DA
	start.x = aabb.min.x; start.y = aabb.min.y; start.z = aabb.max.z;
	end.x = aabb.min.x; end.y = aabb.min.y; end.z = aabb.min.z;
	Draw3DLine(start, end, 0);

	//EF
	start.x = aabb.min.x; start.y = aabb.max.y; start.z = aabb.min.z;
	end.x = aabb.max.x; end.y = aabb.max.y; end.z = aabb.min.z;
	Draw3DLine(start, end, 0);

	//FG
	start.x = aabb.max.x; start.y = aabb.max.y; start.z = aabb.min.z;
	end.x = aabb.max.x; end.y = aabb.max.y; end.z = aabb.max.z;
	Draw3DLine(start, end, 0);

	//GH
	start.x = aabb.max.x; start.y = aabb.max.y; start.z = aabb.max.z;
	end.x = aabb.min.x; end.y = aabb.max.y; end.z = aabb.max.z;
	Draw3DLine(start, end, 0);

	//HE
	start.x = aabb.min.x; start.y = aabb.max.y; start.z = aabb.max.z;
	end.x = aabb.min.x; end.y = aabb.max.y; end.z = aabb.min.z;
	Draw3DLine(start, end, 0);

	//AE
	start.x = aabb.min.x; start.y = aabb.min.y; start.z = aabb.min.z;
	end.x = aabb.min.x; end.y = aabb.max.y; end.z = aabb.min.z;
	Draw3DLine(start, end, 0);

	//BF
	start.x = aabb.max.x; start.y = aabb.min.y; start.z = aabb.min.z;
	end.x = aabb.max.x; end.y = aabb.max.y; end.z = aabb.min.z;
	Draw3DLine(start, end, 0);

	//CG
	start.x = aabb.max.x; start.y = aabb.min.y; start.z = aabb.max.z;
	end.x = aabb.max.x; end.y = aabb.max.y; end.z = aabb.max.z;
	Draw3DLine(start, end, 0);

	//DH
	start.x = aabb.min.x; start.y = aabb.min.y; start.z = aabb.max.z;
	end.x = aabb.min.x; end.y = aabb.max.y; end.z = aabb.max.z;
	Draw3DLine(start, end, 0);

	if (color!=0) g_pRenderer->EnableLighting(false);
}

bool CP3DDraw2D::Sprite_Begin(DWORD flags)
{
	if(!m_pSprite) // vytvor sprite objekt
	{
		if(FAILED(D3DXCreateSprite(g_pD3DDevice, &m_pSprite))) return false;
	}

	if(flags & SPRITE_BILLBOARD || flags & SPRITE_SORT_DEPTH_BACKTOFRONT || flags & SPRITE_SORT_DEPTH_FRONTTOBACK)
	{
		m_pSprite->SetWorldViewLH(NULL, &g_matView);
	}

	if(FAILED(m_pSprite->Begin(flags))) return false;
	return true;
}

bool CP3DDraw2D::Sprite_End()
{
	if(FAILED(m_pSprite->End())) return false;
	return true;
}

bool CP3DDraw2D::Sprite_Draw(IP3DTexture *pTexture, CONST RECT *pSrcRect, P3DXVector3D *pCenter, P3DXVector3D *pPosition, DWORD color)
{
	//if(!m_pSprite) // vytvor sprite objekt // FINAL: mozno odstranit test, pokud se vzdy zavola begin
	//{
	//	if(FAILED(D3DXCreateSprite(g_pD3DDevice, &m_pSprite))) return false;
	//}
	STAT(STAT_DRAW_CALLS, 1);
	if(FAILED(m_pSprite->Draw((LPDIRECT3DTEXTURE9)pTexture->GetD3DTexture(), pSrcRect, (D3DXVECTOR3*)pCenter, (D3DXVECTOR3*)pPosition, color))) return false;
	return true;
}

void CP3DDraw2D::Sprite_SetTransform(P3DXMatrix &mat)
{
	m_pSprite->SetTransform((D3DXMATRIX*)&mat);
}

void CP3DDraw2D::DrawText(const char* text, int x, int y, unsigned long color)
{
	if (!m_pFont)
	{

		D3DXCreateFont( g_pD3DDevice,            // D3D device
			15,               // Height
			0,                     // Width
			FW_NORMAL,               // Weight
			1,                     // MipLevels, 0 = autogen mipmaps
			false,                 // Italic
			DEFAULT_CHARSET,       // CharSet
			OUT_DEFAULT_PRECIS,    // OutputPrecision
			DEFAULT_QUALITY,       // Quality
			DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
			"Arial",              // pFaceName
			&m_pFont);              // ppFont
		m_pFont->PreloadCharacters(32, 255); // preload chars to VRAM
	}
	RECT rc;
	SetRect( &rc, x, y, 0, 0 );
	m_pFont->DrawText( NULL, text, -1, &rc, DT_NOCLIP, color);
}
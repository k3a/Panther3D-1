#include "panRender.h"
#include "common.h"

panRender::panRender( wxWindow* parent,  eVT vtype )
:
panRenderGui( parent )
{
	m_fOrthoZoom = 100;
	m_vPos.x = m_vPos.y = m_vPos.z = 0.0f;

	mvtype = vtype;
	notRealtimeCnt=0;

	switch(mvtype)
	{
	case LEFT_TOP: 
		miViewType = Settings::GetInt(LT_VIEW_TYPE);
		miRenderType = Settings::GetInt(LT_RENDER_TYPE);
		mbRealtime = Settings::GetBool(LT_REALTIME);
		break;
	case LEFT_BOTTOM:  
		miViewType = Settings::GetInt(LB_VIEW_TYPE);
		miRenderType = Settings::GetInt(LB_RENDER_TYPE);
		mbRealtime = Settings::GetBool(LB_REALTIME); 
		break;
	case RIGHT_TOP: 
		miViewType = Settings::GetInt(RT_VIEW_TYPE);
		miRenderType = Settings::GetInt(RT_RENDER_TYPE);
		mbRealtime = Settings::GetBool(RT_REALTIME); 
		break;
	default:
		miViewType = Settings::GetInt(RB_VIEW_TYPE);
		miRenderType = Settings::GetInt(RB_RENDER_TYPE);
		mbRealtime = Settings::GetBool(RB_REALTIME); 
		break;
	}

	switch (miViewType)
	{
	case 0: CheckToolbar(wxID_RIGHT, true); break;
	case 1: CheckToolbar(wxID_TOP, true); break;
	case 2: CheckToolbar(wxID_FRONT, true); break;
	case 3: CheckToolbar(wxID_PERSPECT, true); break;
	}
	switch (miRenderType)
	{
	case 0: CheckToolbar(wxID_WIRE, true); break;
	case 1: CheckToolbar(wxID_TEXT, true); break;
	case 2: CheckToolbar(wxID_LIT, true); break;
	}
	if (mbRealtime) CheckToolbar(wxID_REALTIME, true);

	SetActive(false);
}

void panRender::toolMaximizeClicked( wxCommandEvent& event )
{
	if (event.IsChecked())
		g_pFrmMain->SetFullscreenViewport(mvtype);
	else
		g_pFrmMain->SetFourViewportsArrangement(mvtype);
}

void panRender::toolRightClicked( wxCommandEvent& event )
{
	miViewType = 0;
}

void panRender::toolTopClicked( wxCommandEvent& event )
{
	miViewType = 1;
}

void panRender::toolFrontClicked( wxCommandEvent& event )
{
	miViewType = 2;
}

void panRender::toolPerspectClicked( wxCommandEvent& event )
{
	miViewType = 3;
}

void panRender::toolWireClicked( wxCommandEvent& event )
{
	miRenderType = 0;
}

void panRender::toolTextClicked( wxCommandEvent& event )
{
	miRenderType = 1;
}

void panRender::toolLitClicked( wxCommandEvent& event )
{
	miRenderType = 2;
}

void panRender::toolRealtimeClicked( wxCommandEvent& event )
{
	mbRealtime = event.IsChecked();
	if (mbRealtime) 
		b_nRealtimeWindows++;
	else
		b_nRealtimeWindows--;
}

void panRender::toolRenderClicked( wxCommandEvent& event )
{
	// TODO: Implement toolRenderClicked
}

void panRender::OnIdle( wxIdleEvent& event )
{
	return; // DEBUG!!!
	if (b_nRealtimeWindows>0 && !mbRealtime) 
	{
		notRealtimeCnt++;
		if (notRealtimeCnt<300) { event.Skip(); return; }
		notRealtimeCnt=0;
	}

	// INPUT
	if (m_bActive)
	{
		float delta = g_pEngine->GetDeltaTime();
		P3DMOUSED md = g_pInput->GetMouseData();

		// ortho zoom
		m_fOrthoZoom += md.z * delta * 100000;
		if (m_fOrthoZoom<10) m_fOrthoZoom = 10;

		// axes
		switch(miViewType)
		{
		case 0: // right
			//m_vPos
			break;
		case 1: // top
			m_vPos.x -= md.x * delta * 1000;
			m_vPos.z -= md.y * delta * 1000;
			//g_pRenderer->SetProjectionOrtho(m_fOrthoZoom*fAspect, m_fOrthoZoom, 0, DEFAULT_FAR_PLANE);
			//g_pEngine->SetCamera(pntEye, P3DXPoint3D(0, -1, 0), P3DXPoint3D(0, 0, 1));
			break;
		case 2: // front
			//g_pRenderer->SetProjectionOrtho(m_fOrthoZoom*fAspect, m_fOrthoZoom, 0, DEFAULT_FAR_PLANE);
			//g_pEngine->SetCamera(pntEye, P3DXPoint3D(0, 0, 1), P3DXPoint3D(0, 1, 0));
			break;
		case 3: // perspect
			//g_pRenderer->SetProjection(DEFAULT_FIELD_OF_VIEW, fAspect,DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);
			break;
		}

		//char tmp[32];
		//sprintf(tmp, "%f", m_fOrthoZoom);
		//g_pFrmMain->AddConsoleMsg(tmp, tmp);
	}

	// SET PROJECTION AND CAMERA
	wxSize vpSize = this->panViewport->GetSize();
	float fAspect = (float)vpSize.x/(float)vpSize.y;
	P3DXPoint3D pntEye;
	pntEye.x = pntEye.y = pntEye.z = 0;

	switch(miViewType)
	{
	case 0: // right
		g_pRenderer->SetProjectionOrtho(m_fOrthoZoom*fAspect, m_fOrthoZoom, 0, DEFAULT_FAR_PLANE);
		g_pEngine->SetCamera(pntEye, P3DXPoint3D(-1, 0, 0), P3DXPoint3D(0, 1, 0));
		break;
	case 1: // top
		g_pRenderer->SetProjectionOrtho(m_fOrthoZoom*fAspect, m_fOrthoZoom, 0, DEFAULT_FAR_PLANE);
		g_pEngine->SetCamera(m_vPos, P3DXPoint3D(0, -1, 0), P3DXPoint3D(0, 0, 1));
		break;
	case 2: // front
		g_pRenderer->SetProjectionOrtho(m_fOrthoZoom*fAspect, m_fOrthoZoom, 0, DEFAULT_FAR_PLANE);
		g_pEngine->SetCamera(pntEye, P3DXPoint3D(0, 0, 1), P3DXPoint3D(0, 1, 0));
		break;
	case 3: // perspect
		g_pRenderer->SetProjection(DEFAULT_FIELD_OF_VIEW, fAspect,DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);
		break;
	}

	// RENDER SCENE
	eEngineState engState = g_pEngine->RenderScene(GetViewportHWND());
	if (m_bActive) g_pEngine->UpdateScene();
	// ---------------------

	if (mbRealtime) event.RequestMore(true);
}

void panRender::OnMouse( wxMouseEvent& event )
{
	g_pFrmMain->SetActiveViewport(mvtype);
}

void panRender::CheckToolbar(int id, bool checked)
{
	m_toolBar2->ToggleTool(id, checked);
}

void panRender::Destroying()
{
	//DEBUG!!!!
	return;
	switch(mvtype)
	{
	case LEFT_TOP:
		Settings::Set(LT_VIEW_TYPE, miViewType);
		Settings::Set(LT_RENDER_TYPE, miRenderType);
		Settings::Set(LT_REALTIME, mbRealtime);
		break;
	case LEFT_BOTTOM:
		Settings::Set(LB_VIEW_TYPE, miViewType);
		Settings::Set(LB_RENDER_TYPE, miRenderType);
		Settings::Set(LB_REALTIME, mbRealtime);
		break;
	case RIGHT_TOP:
		Settings::Set(RT_VIEW_TYPE, miViewType);
		Settings::Set(RT_RENDER_TYPE, miRenderType);
		Settings::Set(RT_REALTIME, mbRealtime);
		break;
	default:
		Settings::Set(RB_VIEW_TYPE, miViewType);
		Settings::Set(RB_RENDER_TYPE, miRenderType);
		Settings::Set(RB_REALTIME, mbRealtime);
		break;
	}
}
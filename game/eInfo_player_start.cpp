//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Player start entity class
//-----------------------------------------------------------------------------
// Poznamka: g_pCharCtrl je singleton, globalni promenna se ziska v game.cpp
// Prepsan character controller, jinak v kodu hledat //CH

#include "eInfo_player_start.h"

REGISTER_ENTITY(eInfo_player_start, "info_player_start", false);

// mys
static ConVar CVcl_mousexspeed("cl_mousexspeed", "6.0", CVAR_ARCHIVE|CVAR_NOT_STRING);
static ConVar CVcl_mouseyspeed("cl_mouseyspeed", "6.0", CVAR_ARCHIVE|CVAR_NOT_STRING);
// klavesnice
static ConVar CVcl_moveforward("k_moveforward", "17", CVAR_ARCHIVE|CVAR_NOT_STRING);		// W
static ConVar CVcl_movebackward("k_movebackward", "31", CVAR_ARCHIVE|CVAR_NOT_STRING);		// S
static ConVar CVcl_moveleft("k_moveleft", "30", CVAR_ARCHIVE|CVAR_NOT_STRING);				// A
static ConVar CVcl_moveright("k_moveright", "32", CVAR_ARCHIVE|CVAR_NOT_STRING);			// D
static ConVar CVcl_shift("k_shift", "42", CVAR_ARCHIVE|CVAR_NOT_STRING);					// Shift
static ConVar CVcl_jump("k_jump", "57", CVAR_ARCHIVE|CVAR_NOT_STRING);						// Space
static ConVar CVcl_crouch("k_crouch", "46", CVAR_ARCHIVE|CVAR_NOT_STRING);					// C
static ConVar CVcl_use("k_use", "18", CVAR_ARCHIVE|CVAR_NOT_STRING);						// E
static ConVar CVphys_throw_speed("phys_throw_speed", "1", CVAR_ARCHIVE|CVAR_NOT_STRING);

static IP3DPhysCharacter *g_pPhysChar; // FIXME: DEBUG: docasne static, pozdeji global

eInfo_player_start::eInfo_player_start()
{
	//registrace
	ENT_DATA("angle", m_nAngle, TYPE_INT, 0, E_KEY_SAVE, "#entities_edAngle");
	ENT_DATA_VEC("lookat", m_vLookat, 0, 0, 0, E_SAVE, "");

	P3DAABB aabb(-30, -85, -30, 30, 85, 30);
	SetRelAABB(aabb);

	// vynulovat hodnoty

	m_vecDesiredVelocity = P3DXVector3D(0,0,0);
	m_Pos.x = m_Pos.y = m_Pos.z = 0;
	m_CameraPos.x = m_CameraPos.y = m_CameraPos.z = 0;
	m_ViewVec.x = m_ViewVec.y = 0; m_ViewVec.z = 1.0f;
	m_UpVec.x = m_UpVec.z = 0; m_UpVec.y = 1.0f;
	m_fHeadingHor = m_fHeadingVer = 0.0f;
	m_dwPressedKeys = 0;
	m_dwStateFlags = 0;
	m_bStartCrouch = false;
	m_bOnLadder = false;
	m_fClimbTime = 0.0f;

	m_dwCurFilterBufIdx = 0;
	for (register DWORD i=0; i < DEF_FILTER_BUFFER_LEN; i++)
	{
		m_pFilterBuffer[i].fValueHorizontal = 0;
		m_pFilterBuffer[i].fValueVertical = 0;
		m_pFilterBuffer[i].fTime = 0;
		if (i==0)
			m_pFilterBuffer[0].fFactor = DEF_FILTER_WEIGHT_RATIO;
		else
			m_pFilterBuffer[i].fFactor = m_pFilterBuffer[i-1].fFactor * DEF_FILTER_WEIGHT_RATIO;
	}
}

eInfo_player_start::~eInfo_player_start()
{
	g_HUD.Shutdown();
}

bool eInfo_player_start::Spawn(geometryData_s& gdata)
{	
	g_pPhysChar = (IP3DPhysCharacter*)I_GetClass(IP3DPHYS_CHARACTER);

	g_pSoundMan = (IP3DSoundManager*)I_GetClass(IP3DSOUND_SOUNDMANAGER);
	g_pEngine = (IP3DEngine*)I_GetClass(IP3DENGINE_ENGINE);
	g_HUD.Init();

	return true;
}

void eInfo_player_start::Finalize()
{
	GetWorldPos(m_vPos);

	m_vRot.x = 0;
	m_vRot.y = (float)m_nAngle;

	P3DXVector3D vecCamDir(m_vLookat);
	g_pSoundMan->Listener_SetPositionOrientation (m_vPos, vecCamDir - m_vPos);

	g_pPhysChar->CreateCharacter();
	g_pPhysChar->SetPosition(m_vPos);
}

// DEBUG
//P3DXVector3D lastPos;
void eInfo_player_start::Think(float deltaTime)
{
	/*******P3DXVector3D		pUp;

	g_pCharCtrl->Loop(); // aktualizuj character controllera
	g_pCharCtrl->GetViewInfo (m_vPos, m_vLookat, pUp);
	g_pCharCtrl->GetRot(m_vRot.x, m_vRot.y);

	m_nAngle = (int)m_vRot.y;

	// DEBUG
	P3DXVector3D pohyb = m_vPos;
	pohyb -= lastPos;
	//pohyb *= 10.0f;
	P3DXVector3D lok = m_vLookat - m_vPos;
	sPhysChar->Move(pohyb);
	sPhysChar->GetPosition(m_vPos);
	lok += m_vPos;

	g_pEngine->SetCamera(m_vPos, lok//m_vLookat//, &pUp);

	P3DXVector3D vecFront(m_vLookat);
	g_pSoundMan->Listener_SetPositionOrientation(m_vPos, vecFront-m_vPos);

	SetWorldPos(m_vPos);

	//DEBUG
	g_pCharCtrl->GetViewInfo (m_vPos, m_vLookat, pUp);
	lastPos = m_vPos;


	// DEBUG: EDITOR: FIXME:
	P3DXVector3D vec = m_vLookat - m_vPos;
	//vec.Normalize();
	//vec = vec * 100;
	P3DRay ray(m_vPos, vec);
	eBase* pEnt = g_pEntMgr->FindEntityByClassname("func_button");
	if (pEnt->EditorTestSelection(ray)) 
	{
		g_p2D->Line_DrawFilledRect(0, 0, 50 ,50, 0);
		pEnt->EditorSetSelected(true);
	}*/


	float	fSinHor, fSinVer, fCosHor, fCosVer, fTimeDelta;
	float	fMouseHorDelta, fMouseVerDelta;
	float	fBufX = 0, fBufZ = 0;

	if (!g_pInput) return;
	if (g_pConsole->IsShown()) return;

	fTimeDelta = g_pTimer->GetTimeDelta_s();

	// mouse input
	P3DMOUSED md = g_pInput->GetMouseData();
	fMouseHorDelta = (float)md.x;
	fMouseVerDelta = (float)md.y;
	FilterMouse (fMouseHorDelta, fMouseVerDelta, fTimeDelta);

	m_fHeadingHor += fMouseHorDelta;
	m_fHeadingVer += fMouseVerDelta;

	// ohranicenie vertikalnej rotacie
	if (m_fHeadingVer > VIEW_TRESHOLD)
		m_fHeadingVer = (float)VIEW_TRESHOLD;
	else if (m_fHeadingVer < -VIEW_TRESHOLD)
		m_fHeadingVer = (float)-VIEW_TRESHOLD;

	if (m_bOnLadder)
	{ProcessLadder(fTimeDelta); return;}

	// keyboard input
	fSinHor = sinf(m_fHeadingHor);
	fCosHor = cosf(m_fHeadingHor);
	fSinVer = sinf(m_fHeadingVer);
	fCosVer = cosf(m_fHeadingVer);

	// ************************************
	if (g_pInput->IsKeyDown (CVcl_moveforward.GetInt()))		// move straight
	{fBufX = fSinHor * 1.5f; fBufZ = fCosHor * 1.5f;}

	if (g_pInput->IsKeyDown (CVcl_movebackward.GetInt()))		// move back
	{fBufX -= fSinHor; fBufZ -= fCosHor;}

	if (g_pInput->IsKeyDown (CVcl_moveleft.GetInt()))			// move left
	{fBufX -= fCosHor; fBufZ += fSinHor;}

	if (g_pInput->IsKeyDown (CVcl_moveright.GetInt()))			// move right
	{fBufX += fCosHor; fBufZ -= fSinHor;}

	// ************************************

	m_vecDesiredVelocity.x = fBufX;
	m_vecDesiredVelocity.z = fBufZ;
	m_vecDesiredVelocity.Normalize ();		// udava (jednotkovy) vektor pozadovaneho smeru pohybu

	if (g_pInput->IsKeyDownFirstTime (CVcl_crouch.GetInt()))			// crouch
		if ((m_dwStateFlags & PLAYER_STATE_CROUCHING) == 0)
			m_bStartCrouch = !m_bStartCrouch;

	if (g_pInput->IsKeyDownFirstTime (CVcl_jump.GetInt()))				// jump
		if (m_dwStateFlags & PLAYER_STATE_CROUCH)
		{
			if (((m_dwStateFlags & PLAYER_STATE_CROUCHING) != 0) && (m_bStartCrouch == false))
				m_vecDesiredVelocity.y = JUMP_SPEED;
			m_bStartCrouch = false;
		}
		else if ((m_dwStateFlags & PLAYER_STATE_CROUCHING) == 0)
			m_vecDesiredVelocity.y = JUMP_SPEED;

	if (g_pInput->IsKeyDown (CVcl_shift.GetInt()))	// Shift
		m_dwPressedKeys = KEY_SHIFT;
	else
		m_dwPressedKeys = 0;

	// objekt picking
#define	HOLD_DISTANCE	100.0f		// vzdialenost drzaneho objektu od kamery
/*	m_PickingPoint.x = NVALUE_OUT(m_CameraPos.x) + (fSinHor * fCosVer) * HOLD_DISTANCE;	// aktualizacia picking pointu
	m_PickingPoint.y = NVALUE_OUT(m_CameraPos.y) - fSinVer * HOLD_DISTANCE;
	m_PickingPoint.z = NVALUE_OUT(m_CameraPos.z) + (fCosHor * fCosVer) * HOLD_DISTANCE;

	if (g_pInput->IsKeyDownFirstTime(CVcl_use.GetInt()) || m_bDropDown)
	{
		if (m_dwStateFlags & PLAYER_STATE_PICKING)		// pustenie objektu
		{
			DropDownPickedObject();
		}
		else											// zdvihnutie objektu
		{
#define MAX_PICKUP_RAYCAST_LENGTH	200.0f
			P3DXVector3D	pStart;
			P3DXVector3D pEnd;
			float	fRayLengthMultiplier = 1.0f;

			pStart = NPointOut (m_CameraPos);
			pEnd.x = fSinHor * fCosVer;
			pEnd.y = -fSinVer;
			pEnd.z = fCosHor * fCosVer;
			if (m_dwStateFlags & PLAYER_STATE_CROUCH)
				fRayLengthMultiplier = 0.7f;
			else if (m_fHeadingVer < P3DToRadian (60.0f))	// skratenie dlzky pickup raycastu pri pohlade nahor
				fRayLengthMultiplier = ((m_fHeadingVer + VIEW_TRESHOLD) / P3DToRadian(150.0f)) * 0.5f + 0.5f;
			pEnd *= fRayLengthMultiplier * MAX_PICKUP_RAYCAST_LENGTH;
			pEnd += pStart;

			IP3DRigidBody* body = g_pNewtonWorld->GetNearestRayCastBody (pStart, pEnd, 0.1f, OBJECT_MAX_WEIGHT);
			if (body) // bylo nalezeno takové tìleso
				if (body != m_pDontPickThisBody)		// nie je mozne zdvihnut objekt, ktory je priamo pod playerom
				{
					// oznacit ako non-collidable s geometriou playera
					m_PickObjectMatID = body->GetGameMaterialGroupID();
					body->SetGameMaterialGroupID ("npc_noncollide");

					// oznacit ako always-active
					body->SetAutoFreeze (false);
					m_bDropDown = false;
					m_dwStateFlags |= PLAYER_STATE_PICKING;
					g_pNewtonWorld->ActivateBody(body, true);
					m_oldPickFATCallback = body->GetForceAndTorqueCallback(); // uloz stary callback
					body->SetForceAndTorqueCallback (PickUpApplyForceAndTorque);
					m_pPickingBody = body;
				}
		}
	}
	else if (m_dwStateFlags & PLAYER_STATE_PICKING)
	{
		if (g_pInput->IsLeftButtonDownFirstTime ())		// left button click
		{
			// odhodenie drzaneho objektu
			float fObjectWeight = 1.0f;
			if (m_pPickingBody)
				fObjectWeight = max (1.0f, m_pPickingBody->GetMass ());
			if (fObjectWeight > OBJECT_MAX_WEIGHT)
				fObjectWeight = OBJECT_MAX_WEIGHT;

			P3DXVector3D vecObjectVelocity(m_LookAtPoint);
			vecObjectVelocity -= m_CameraPos;
			vecObjectVelocity.Normalize ();
			vecObjectVelocity *= CVphys_throw_speed.GetFloat() * OBJECT_THROW_SPEED / pow (fObjectWeight, 0.65f);
			// nahodna rotacia
			P3DXVector3D vecOmega;
			vecOmega.x = float(rand()) / float(RAND_MAX);
			vecOmega.x *= 10.0f * (1.0f - (fObjectWeight / OBJECT_MAX_WEIGHT));
			vecOmega.y = float(rand()) / float(RAND_MAX);
			vecOmega.y -= 1.0f;
			vecOmega.y *= 15.0f * (1.0f - (fObjectWeight / OBJECT_MAX_WEIGHT));
			vecOmega.z = 0;
			m_pPickingBody->SetOmega (vecOmega);
			DropDownPickedObject(&vecObjectVelocity);
		}
	}*/

	g_pPhysChar->Move(m_vecDesiredVelocity);

	// ziskej pozoco, rotaci a nastav enginu
	g_pPhysChar->GetPosition(m_vPos);

	m_ViewVec.x = fSinHor * fCosVer;
	m_ViewVec.y = -fSinVer;
	m_ViewVec.z = fCosHor * fCosVer;

	P3DXVector3D up(0, 1, 0);
	g_pEngine->SetCamera(m_vPos, m_ViewVec, up);

	g_pSoundMan->Listener_SetPositionOrientation(m_vPos, m_ViewVec);

	SetWorldPos(m_vPos);
}

void eInfo_player_start::Render(float deltaTime)
{
	g_HUD.Render();
}

//---------------------------------
inline void eInfo_player_start::FilterMouse (IN OUT float &fValueHor, IN OUT float &fValueVer, const float fTimeDelta)
{
	float fTimeAccu = fTimeDelta, fDivider = 1.0f;
	float fValueHorBuf, fValueVerBuf;
	DWORD dwIdx = m_dwCurFilterBufIdx, i = 0;

	fValueHorBuf = pow (fabs(fValueHor), 1.3f);		// zmenit konstantu ???
	if (fValueHor < 0.0f)
		fValueHorBuf = -fValueHorBuf;
	fValueHor = fValueHorBuf;
	fValueVerBuf = pow (fabs(fValueVer), 1.3f);
	if (fValueVer < 0.0f)
		fValueVerBuf = -fValueVerBuf;
	fValueVer = fValueVerBuf;

	fValueHorBuf = fValueHor *= CVcl_mousexspeed.GetFloat() * MOUSE_SENSITIVITY;
	fValueVerBuf = fValueVer *= CVcl_mouseyspeed.GetFloat() * MOUSE_SENSITIVITY;

	// vypocet priemeru
	do{
		fTimeAccu += m_pFilterBuffer[dwIdx].fTime;
		if (fTimeAccu > DEF_FILTER_MAX_TIME) break;
		fValueHor += m_pFilterBuffer[dwIdx].fValueHorizontal * m_pFilterBuffer[i].fFactor;
		fValueVer += m_pFilterBuffer[dwIdx].fValueVertical * m_pFilterBuffer[i].fFactor;
		fDivider += m_pFilterBuffer[i].fFactor;
		i++;
		if (++dwIdx >= DEF_FILTER_BUFFER_LEN) dwIdx = 0;
	}while (dwIdx != m_dwCurFilterBufIdx);

	// ulozenie novej hodnoty
	if (m_dwCurFilterBufIdx == 0)
		m_dwCurFilterBufIdx = DEF_FILTER_BUFFER_LEN - 1;
	else
		m_dwCurFilterBufIdx -= 1;
	m_pFilterBuffer[m_dwCurFilterBufIdx].fValueHorizontal = fValueHorBuf;
	m_pFilterBuffer[m_dwCurFilterBufIdx].fValueVertical = fValueVerBuf;
	m_pFilterBuffer[m_dwCurFilterBufIdx].fTime = fTimeDelta;

	fValueHor /= fDivider;
	fValueVer /= fDivider;
}

//---------------------------------
void eInfo_player_start::LadderBegin (P3DXVector3D &pointDesiredCameraPos, float fHorizontalAngle)
{
	if (m_bOnLadder) return;
	m_LadderInfo.dwCamMoveStartTime = g_pTimer->GetTickCount_ms();
	m_LadderInfo.pointStartingCameraPos = m_CameraPos;
	m_LadderInfo.pointDesiredCameraPos = pointDesiredCameraPos;
	m_LadderInfo.fAngleLadderRot = P3DToRadian (fHorizontalAngle);

	float fCutOffPiece;
	// odstranenie nadbytocnych uhlov rotacie
	fCutOffPiece = ((int)(m_fHeadingHor / (2*P3D_PI))) * 2*P3D_PI;
	m_LadderInfo.fAngleStartingCameraHorRot = m_fHeadingHor - fCutOffPiece;
	// odstranenie nadbytocnych uhlov rotacie
	fCutOffPiece = ((int)(m_LadderInfo.fAngleLadderRot / 2*P3D_PI)) * 2*P3D_PI;
	m_LadderInfo.fAngleLadderRot -= fCutOffPiece;

	m_bOnLadder = true;
	m_LadderInfo.playerState = PLAYER_LADDER_APPROACHING;
	/*if (m_dwStateFlags & PLAYER_STATE_PICKING)		// pustenie objektu ak player prave nejaky drzi
		DropDownPickedObject();*/

	g_pPhysChar->SetCollision(false);

	m_fClimbTime = 0.0f;
}

//---------------------------------
void eInfo_player_start::LadderEnd (bool bUpOrDown)
{
	if (!m_bOnLadder) return;

	if (bUpOrDown)
	{
		m_LadderInfo.playerState = PLAYER_LADDER_LEAVING;
		m_LadderInfo.dwCamMoveStartTime = g_pTimer->GetTickCount_ms();
		m_LadderInfo.pointStartingCameraPos = m_CameraPos;
		m_LadderInfo.pointDesiredCameraPos = m_CameraPos;
		m_LadderInfo.pointDesiredCameraPos.x += sinf(m_LadderInfo.fAngleLadderRot) * 0.45f;
		m_LadderInfo.pointDesiredCameraPos.z += cosf(m_LadderInfo.fAngleLadderRot) * 0.45f;
	}
	else
	{
		m_bOnLadder = false;
		g_pPhysChar->SetCollision(true);
		g_pPhysChar->SetPosition(m_CameraPos);
	}
}

//---------------------------------
void eInfo_player_start::ProcessLadder (float fTimeDelta)
{
	float				fTimeDiff, fAngleDiff;
	float				fSignum = 0;
	P3DXVector3D	vecSlide;
	m_UpVec.x = 0; m_UpVec.y = 1.0; m_UpVec.z = 0;

	// on ladder
	if (m_LadderInfo.playerState == PLAYER_ONLADDER)
	{
		if (g_pInput->IsKeyDownFirstTime (CVcl_jump.GetInt()))		// jump
			LadderEnd (false);
		else if (g_pInput->IsKeyDown (CVcl_moveforward.GetInt()))		// forward key
			fSignum = 1.0f;
		else if (g_pInput->IsKeyDown (CVcl_movebackward.GetInt()))	// backward key
			fSignum = -1.0f;

		if (fSignum)
		{
			// kmitavy pohyb kamery
			#define CLIMB_LEAN_DISTANCE	0.05f;
			m_fClimbTime += fTimeDelta * (2.0f * P3D_PI);
			float fSinClimbTime = sinf(m_fClimbTime);
			float fClimbAspect = fabs(fSinClimbTime) + 0.3f;
			if (fClimbAspect > 1.0f) fClimbAspect = 1.0f;
			float fTemp, fClimbLeanX, fClimbLeanZ;
			fTemp = m_LadderInfo.fAngleLadderRot + (P3D_PI * 0.5f);
			fClimbLeanX = sinf(fTemp) * fSinClimbTime * CLIMB_LEAN_DISTANCE;
			fClimbLeanZ = cosf(fTemp) * fSinClimbTime * CLIMB_LEAN_DISTANCE;

			m_Pos.y += LADDER_CLIMB_SPEED * fTimeDelta * fClimbAspect * fSignum;
			m_CameraPos.y = m_Pos.y;
			m_CameraPos.x = m_Pos.x + fClimbLeanX;
			m_CameraPos.z = m_Pos.z + fClimbLeanZ;
		}

		// limit rotacie kamery 45 stupnov na obe strany
		if ((m_fHeadingHor - m_LadderInfo.fAngleLadderRot) > (P3D_PI/2))
			m_fHeadingHor = m_LadderInfo.fAngleLadderRot + (P3D_PI/2);
		else if ((m_fHeadingHor - m_LadderInfo.fAngleLadderRot) < (-P3D_PI/2))
			m_fHeadingHor = m_LadderInfo.fAngleLadderRot - (P3D_PI/2);
	}
	// approaching
	if (m_LadderInfo.playerState == PLAYER_LADDER_APPROACHING)
	{
		fTimeDiff = ((float)(g_pTimer->GetTickCount_ms() -  m_LadderInfo.dwCamMoveStartTime)) * 0.001f;
		if (fTimeDiff >= LADDER_APPROACH_LEAVE_TIME)		// koniec "priblizovania" sa k ladderu
		{
			m_LadderInfo.playerState = PLAYER_ONLADDER;
			m_Pos = m_CameraPos = m_LadderInfo.pointDesiredCameraPos;
			m_fHeadingHor = m_LadderInfo.fAngleLadderRot;
		}
		else														// "priblizovanie" sa k ladderu
		{
			vecSlide = m_LadderInfo.pointDesiredCameraPos;
			vecSlide -= m_LadderInfo.pointStartingCameraPos;
			vecSlide *= fTimeDiff / LADDER_APPROACH_LEAVE_TIME;
			vecSlide += m_LadderInfo.pointStartingCameraPos;
			m_Pos = m_CameraPos = vecSlide;

			fAngleDiff = m_LadderInfo.fAngleLadderRot - m_LadderInfo.fAngleStartingCameraHorRot;
			if (fAngleDiff > P3D_PI)
				fAngleDiff = -2*P3D_PI + fAngleDiff;
			else if (fAngleDiff < -P3D_PI)
				fAngleDiff = 2*P3D_PI + fAngleDiff;
			m_fHeadingHor = m_LadderInfo.fAngleStartingCameraHorRot + (fAngleDiff * fTimeDiff / LADDER_APPROACH_LEAVE_TIME);
		}
	}
	// leaving - spracovava sa iba pri "hornom" vystupe
	else if (m_LadderInfo.playerState == PLAYER_LADDER_LEAVING)
	{
		fTimeDiff = ((float)(g_pTimer->GetTickCount_ms() -  m_LadderInfo.dwCamMoveStartTime)) * 0.001f;
		if (fTimeDiff >= LADDER_APPROACH_LEAVE_TIME)		// koniec "vystupu" z objektu ladder
		{
			m_bOnLadder = false;
			g_pPhysChar->SetCollision(true);
			m_Pos = m_CameraPos = m_LadderInfo.pointDesiredCameraPos;
			g_pPhysChar->SetPosition(m_Pos);
		}
		else
		{
			vecSlide = m_LadderInfo.pointDesiredCameraPos;
			vecSlide -= m_LadderInfo.pointStartingCameraPos;
			vecSlide *= fTimeDiff / LADDER_APPROACH_LEAVE_TIME;
			vecSlide += m_LadderInfo.pointStartingCameraPos;
			m_Pos = m_CameraPos = vecSlide;
		}
	}

	// nastavenie m_ViewVec
	float fCosVer = cosf(m_fHeadingVer);
	m_ViewVec.x = sinf(m_fHeadingHor) * fCosVer;
	m_ViewVec.y = - sinf(m_fHeadingVer);
	m_ViewVec.z = cosf(m_fHeadingHor) * fCosVer;
}
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Player start entity class declaration
//-----------------------------------------------------------------------------
#pragma once
#include "common.h"
#include "eBase.h"
#include "HUD.h"

// player state flags
#define PLAYER_STATE_CROUCHING			0x00000001	// player prave prechadza zo stavu not-crouch na crouch, alebo naopak
#define PLAYER_STATE_CROUCH				0x00000002	// player je v crouch polohe
#define PLAYER_STATE_IN_THE_AIR			0x00000004	// player nie je v kontakte so zemou
#define PLAYER_STATE_SLIDE_DOWN			0x00000008	// player sa klze nadol
#define PLAYER_STATE_PICKING				0x00000010	// player zdviha alebo drzi objekt

#define KEY_SHIFT		0x00000001
#define KEY_CTRL		0x00000002
#define KEY_ALT			0x00000004

// parametre filtra hodnot mysi
#define MOUSE_SENSITIVITY			0.0004f
#define DEF_FILTER_BUFFER_LEN		9		// v skutocnosti o jeden viac - 10
#define DEF_FILTER_WEIGHT_RATIO		0.75f
#define DEF_FILTER_MAX_TIME			0.05f

//
#define VIEW_TRESHOLD (P3D_PI/2.0f - 0.05f)
#define ALWAYS_RUN_SPEED			4.5f		// rychlost pohybu
#define STEP_MODIFIER				1.6f		// "nasobitel" rychlosti pohybu pri stlacenom Shift
#define JUMP_SPEED					4.0f		// nepriamo sa tym nastavuje vyska skoku pri JUMP
#define CROUCH_SPEED				1.7f		// rychlost pohybu v crouch polohe
#define CROUCH_PERIOD				0.3f		// cas, potrebny na zmenu polohy crouch-not_crouch a opacne

// ladders values
#define LADDER_APPROACH_LEAVE_TIME	0.2f		// cas po ktory sa kamera presuva na "priamku laddera" po stlaceni E
#define LADDER_CLIMB_SPEED			1.0f

//------------------------------------------------------------------------------

// mouse filter buffer
//---------------------------------
struct FILTER_BUFFER_ITEM{
	float		fValueHorizontal;			// polozky fValue* a fTime tvoria kruhovy zasobnik
	float		fValueVertical;
	float		fTime;
	float		fFactor;			// nijak sa nevztahuje na polozky fValue* a fTime!
};

// ladder structs
//---------------------------------
enum LADDER_STATE {
	PLAYER_LADDER_APPROACHING,	// player stlacil E a kamera sa prave presuva na priamku ladderu, presun trva LADDER_APPROACH_LEAVE_TIME sec
	PLAYER_ONLADDER,
	PLAYER_LADDER_LEAVING		// podobne ako Approaching, ale plati pre opustenie ladderu
};
struct LADDER_INFO {
	LADDER_STATE	playerState;
	DWORD			dwCamMoveStartTime;	// pouzivane pri posune kamery pri vystupe/zostupe na ladder
	P3DXVector3D		pointStartingCameraPos, pointDesiredCameraPos;	//		-//-
	float			fAngleStartingCameraHorRot, fAngleLadderRot;
};

class eInfo_player_start : public eBase
{
public:
	eInfo_player_start();
	~eInfo_player_start();
	void Render(float deltaTime);
	bool Spawn(geometryData_s& gdata);
	void Finalize();
	void Think(float deltaTime);
private:
	inline void FilterMouse (IN OUT float &fValueHor, IN OUT float &fValueVer, const float fTimeDelta);
	void ProcessLadder (float fTimeDelta);
	void LadderBegin (P3DXVector3D &pointDesiredCameraPos, float fHorizontalAngle);
	void LadderEnd (bool bUpOrDown);
private:
	P3DXVector3D m_vPos, m_vRot, m_vLookat;
	int m_nAngle;
	P3DXVector3D		m_vecDesiredVelocity;
	bool				m_bStartCrouch;
	DWORD				m_dwPressedKeys;
	// 
	// mouse filtering
	FILTER_BUFFER_ITEM	m_pFilterBuffer [DEF_FILTER_BUFFER_LEN];
	DWORD				m_dwCurFilterBufIdx;		// oznacuje aktualny vrchol zasobniku m_pFilterBuffer
	// character
	DWORD				m_dwStateFlags;
	P3DXVector3D			m_Pos, m_CameraPos, m_ViewVec;
	P3DXVector3D			m_UpVec;
	float				m_fHeadingHor, m_fHeadingVer;
	// ladder variables
	bool				m_bOnLadder;
	LADDER_INFO			m_LadderInfo;
	float				m_fClimbTime;
};
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Base entity part: Physics description and interaction
//-----------------------------------------------------------------------------
#pragma once
#include "IP3DPhysEngine.h"
#include "IP3DXML.h"

class eBase; // dopredna deklarace, je rozsirena pomoci include v cpp

// kolizni skupiny - obekty mezi sebou budou kolidovat pokud obsahuji stejny flag
#define COLLISION_PLAYER (1 << 0)
#define COLLISION_VEHICLE (1 << 1)
#define COLLISION_NPC (1 << 2)

/** Type of entity movement */
enum eMoveType
{
	/** Non-physically moved entity  (can be moved directly by SetWorldMatrix */
	MOVE_NONE = 0,
	/** Physically moved entity (phys. body) */
	MOVE_PHYS,
	/** Moved by parent and physically driven - body must be kinematic */
	MOVE_PARENT
};

class eBasePhysObj
{
public:
	void InitBasePhysObj(eBase* thisBase);
	void ShutdownBasePhysObj();
	bool LoadPhysModel(const char* modelXmlPath, bool bStatic=false); // udela fyzikalni model ze specifikovaneho souboru
	bool CreatePhysModel(bool bStatic=false); // vytvori podle toho co je nacteno v renderable casti eBase, vesmes staci
	void SetPhysWorldMatrix(P3DXMatrix &mat){ if(!bLoaded) return; if (m_moveType == MOVE_PARENT) m_pBody->MoveMatrix(mat); else m_pBody->SetMatrix(mat); };
	void SetMoveType(eMoveType mt);
	eMoveType GetMoveType()const{ return m_moveType;};
private:
	bool LoadFromXml (const ezxml_t modelXml, const char* modelpath, bool bStatic);
	void GetXmlTransformMatrix(ezxml_t pXml, P3DXMatrix &pMatrix);
private:
	//
	bool bLoaded;
	eBase* base;
	eMoveType m_moveType;
	//
	IP3DPhysBody *m_pBody;
};
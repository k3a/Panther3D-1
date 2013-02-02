//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Entity manager iface (game) used by engine
//-----------------------------------------------------------------------------
#pragma once
#include "interface.h"
#include "types.h"

struct epair_s
{
	char *key;
	char *value;
};

struct geometryDataFace_s
{
	P3DXVector3D* verts;
	UINT numVerts;
	char texture[64];
	int surfFlags;
	int contFlags;
};

struct geometryData_s
{
	geometryDataFace_s* faces;
	UINT numFaces;
};

class eBase;

class IP3DEntityMgr : public IP3DBaseInterface
{
public:
	// komentare k temto metodam jsou v EntityMgr.h souboru
	virtual bool Initialize()=0;
	virtual void KillAll()=0;
	virtual bool Spawn(const epair_s *sEnt, const int numEntP, geometryData_s gdata)=0; // vstupni data budou smazana, je nutne je zkopirovat, vola bspmap.cpp pri nacitani
	virtual void Finalize()=0; // vola bspmap.cpp po nacteni vsech entit, dale pak tento manager po loadu
	virtual void Render(float deltaTime, bool renderModels= true, bool renderEntities=true, bool renderHelpers=false)=0;
	virtual void Think(float deltaTime)=0;
	virtual bool Load(const char* szSaveFile)=0;
	virtual bool Save(const char* szSaveFile)=0;
	virtual eBase* FindEntityByTargetname(const char* szTargetName)=0; // najde prvni entitu s timto targetname. Pokud bude potreba hledat vice se stejnym targetname, udelat novou metodu. Zatim ma kazda entita jiny targetname.
	virtual eBase* FindEntityByClassname(const char* szClassName)=0; // vrati prvni z holderu, pokud existuje, dalsi lze ziskat pomoci GetNext() a GetPrevious()
};

#define IP3DGAME_ENTITYMGR "P3DEntityMgr_1" // nazev ifacu
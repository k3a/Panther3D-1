//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (7.7.2006)
// Purpose:	Particle manager and particle systems
//-----------------------------------------------------------------------------
#pragma once

// docasne, dokud nejsou ifacy
#include "interface.h"
//
#include "IP3DTexture.h"
#include "IP3DRenderer.h"
#include "IP3DVertexBuffer.h"
#include "IP3DIndexBuffer.h"
/*
struct Particle
{
public:
	P3DXVector3D		position;
	P3DXVector3D		oldPos;
	float			lifeTime; // bude se sama odecitat, jakmile je 0, znamena to mrtvou castici
	
	P3DXVector3D		dirAndVelocity; // nastaví se pri spawnu, ubírá se dle odporu vzduchu a sklání se dle gravitace
	int				energy;
	float			size;
};
*/
/*
// Particle manager - spravuje cele particle systemy, system je mozne nacist ze souboru
class P3DPManager
{
public:
	void UpdateAllSystems(float fDeltaTime);
	void RenderAlLSystems();
	int LoadSystemFromFile(); // vrati ID systemu
	bool RemoveSystem(int ID);
private:
	UINT nLastID;
	P3DPSystem *m_pSystems; // dynamicky alokované pole
};
*/

// Particle system - obsahuje nekolik emitoru. Emitory lze pridavat. Jakmile uz v emitoru
//					 nebude zadna ziva castice a emitor nebude opakujici, bude sam odstranen
#define MAX_EMITTERS 10
class P3DPSystem
{
public:
	P3DPSystem(){ m_numEmitters=0; isAlive=true; };
	~P3DPSystem();
	void UpdateAllEmitters(float fDeltaTime);
	void RenderAllEmitters();
	void AddEmitter(P3DPEmitter* pEmitter);
	bool Create(IP3DTexture* pTex); // ze zadanych parametru pripravi vertex a index buffery, castice a texturu
	P3DXVector3D position; // absolutni pozice systemu ve scene
	bool isAlive; // ma nejake zive emitory?
private:
	P3DPEmitter *m_pEmitters[MAX_EMITTERS]; // WARN: maximalne MAX_EMITTERS emitoru!
	BYTE m_numEmitters;
};

// Particle emitter - obsahuje urcity pocet castic. Castice se mohou ruzne premistovat v zavislosti
//					  jejich vlastnostech. Emitovat se mohou jen mrtve castice. 
//					  Pole ktere se nastavi, se samy v destruktoru uvolni.
class P3DPEmitter
{
friend class P3DPSystem;

public:
	P3DPEmitter();
	~P3DPEmitter()
	void Update(float fDeltaTime);
	void Render();
private:
	Particle *m_pParticles; // pole castic
	IP3DVertexBuffer m_pVerts;
	IP3DIndexBuffer m_pIndexes;
protected:
	P3DPSystem* m_pSystem; // ukazatel na system, ke keteremu patri
public:
	bool isAlive;
	// nastaveni emitoru - grafy maji x rovnu delce emitoru
	int nType; // 0 - ctvercovy typ, 1 - koule
	P3DXVector3D pntMin; // min hodnoty BB nebo x jako polomer koule (pokud je typ koule)
	P3DXVector3D pntMax; // max hodnoty BB
	float fLen; // delka emitoru v sekundach
	bool bRepeat; // opakujici se emitor?
	P3DXVector3D *ppntRelPos; // pole (graf) - relativni pozice emitoru k systemu
	P3DXVector3D *pvDirAndVelocity; // pole (graf) - smer a rychlost castice
	int nDensity; // pocet castic emitovanych za 1 sekundu (pozdeji graf?)
	float *pfLifeTime; // pole (graf) - zivotnost prave emotovane castice
	float *pfSizeMult; // pole (graf) - nasobitel velikosti prave emitovane castice
	int nParticles; // max. pocet castic
	// nastaveni castic - grafy maji x jako lifetime a ten muze byt ruzny dle nastaveni emitoru
	float fAirResistance; // odpor vzduchu, zpomaleni o m/s za kazdou sekundu - odpocitava se rychlost u kazde castice
	IP3DTexture *pTexture; // textura castice
	float *pfSize; // pole (graf) - pomer veliksoti castice
	float *pfRotation; // pole (graf) - rotace castice kolem osy s tim ze osa miri od stredu castice ke kamere
	int *nfTransp; // pole (graf) - hodnota pruhlednosti
	P3DXVector3D *ppntColor; // pole (graf) - x, y, z jako RGB
	bool bStretch; // "roztahnout" castici mezi jeji posledni a aktualni bod? Tvorba jisker
};
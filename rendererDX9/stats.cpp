//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Univetsal project stars counter
//-----------------------------------------------------------------------------
#include "stats.h"
#include "IP3DDraw2D.h"
#include "IP3DConsole.h"
#include "symbols.h"

static ConVar CVdrawstats("r_drawstats", "0", CVAR_NOT_STRING|CVAR_ARCHIVE, "Vykreslit statistiku rendereru. Jen debug.");

// -----------------------------------------------------------------------------

static IP3DDraw2D* s_2D=NULL;
static stat_t m_stats[STAT_NUM_OF_STATS]=
{
	{"STAT_SET_RENDER_STATE",			true,	0},
	{"STAT_SET_TEXTURE_STAGE_STATE",	true,	0},
	{"STAT_SET_SAMPLER_STATE",			true,	0},
	{"STAT_SHADER_SET_VALUE",			true,	0},
	{"STAT_SHADER_SET_TECHNIQUE",		true,	0},
	{"STAT_SHADER_BEGIN_PASS",			true,	0},
	{"STAT_SHADER_DYNLIGHT_PASS",		true,	0},
	{"STAT_SHADER_TEXTURES",			true,	0},
	{"STAT_TEXTURE_ALL_TEXELS",			false,	0},
	{"STAT_DRAW_CALLS",					true,	0},
	{"STAT_DRAW_VERTS",					true,	0},
	{"STAT_ALL_VERTS",					false,	0},
};


void CP3DStats::Init()
{
	s_2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);

	BeginNewFrame(); // radeji ;)
}
void CP3DStats::Shutdown()
{

}

void CP3DStats::BeginNewFrame()
{
	// vynuluj
	for(int i=0;i<STAT_NUM_OF_STATS;i++)
	{
		if (m_stats[i].bReset) m_stats[i].nCurrValue = 0;
	}
}

int x = 0;
void CP3DStats::EndNewFrame()
{
	x++; if (x<10)return; // pozdeji dle FPS ;)

	// zkopci pro vykresleni
	for(int i=0;i<STAT_NUM_OF_STATS;i++)
	{
		m_stats_render[i].nCurrValue = m_stats[i].nCurrValue;
	}

	x=0;
}

void CP3DStats::IncrementStat(int statID, int addValue)
{
	m_stats[statID].nCurrValue += addValue;
}

void CP3DStats::DrawStats()
{
	if (!CVdrawstats.GetBool()) return;

	char line[512];

	for(int i=0;i<STAT_NUM_OF_STATS;i++)
	{
		if (m_stats_render[i].nCurrValue > 1024*1024)
			sprintf(line, "%s: %d M", m_stats[i].szName, m_stats_render[i].nCurrValue/(1024*1024));
		else if (m_stats_render[i].nCurrValue > 1024)
			sprintf(line, "%s: %d k", m_stats[i].szName, m_stats_render[i].nCurrValue/1024);
		else
			sprintf(line, "%s: %d", m_stats[i].szName, m_stats_render[i].nCurrValue);
		
		s_2D->DrawText(line, 13, 13+13*i, P3DCOLOR_XRGB(200, 0, 0));
	}
}
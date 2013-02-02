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
#pragma once

#include "windows.h"

enum
{
	STAT_SET_RENDER_STATE=0,
	STAT_SET_TEXTURE_STAGE_STATE,
	STAT_SET_SAMPLER_STATE,
	STAT_SHADER_SET_VALUE,
	STAT_SHADER_SET_TECHNIQUE,
	STAT_SHADER_BEGIN_PASS,
	STAT_SHADER_DYNLIGHT_PASS,
	STAT_SHADER_TEXTURES,
	STAT_TEXTURE_ALL_TEXELS,
	STAT_DRAW_CALLS,
	STAT_DRAW_VERTS,
	STAT_ALL_VERTS,
	STAT_NUM_OF_STATS
};

struct stat_t
{
	const char* szName;
	bool bReset; // resetovat kazdy novy snimek?
	int nCurrValue;
};

class CP3DStats
{
public:
	void Init();
	void Shutdown();
	void BeginNewFrame();
	void EndNewFrame();
	void IncrementStat(int statID, int addValue);
	void DrawStats();
private:
	stat_t m_stats_render[STAT_NUM_OF_STATS]; // zpomalene vysledky
};

#ifdef _DEBUG
	#define STAT(statID, addValue) g_stats.IncrementStat(statID, addValue)
#else
	#define STAT(statID, addValue)
#endif
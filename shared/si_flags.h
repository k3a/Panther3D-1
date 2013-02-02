//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (25.7.2006)
// Purpose:	Shared surface info flags
//-----------------------------------------------------------------------------

/* -------------------------------------------------------------------------------

content and surface flags

------------------------------------------------------------------------------- */

/* game flags */
#define SOD_CONT_SOLID				1			/* an eye is never valid in a solid */
#define SOD_CONT_LAVA					8
#define SOD_CONT_SLIME				16
#define SOD_CONT_WATER				32
#define SOD_CONT_FOG					64

#define SOD_CONT_AREAPORTAL			0x8000

#define SOD_CONT_PLAYERCLIP			0x10000
#define SOD_CONT_MONSTERCLIP			0x20000
#define SOD_CONT_TELEPORTER			0x40000
#define SOD_CONT_JUMPPAD				0x80000
#define SOD_CONT_CLUSTERPORTAL		0x100000
#define SOD_CONT_DONOTENTER			0x200000
#define SOD_CONT_BOTCLIP				0x400000

#define SOD_CONT_ORIGIN				0x1000000	/* removed before bsping an entity */

#define SOD_CONT_BODY					0x2000000	/* should never be on a brush, only in game */
#define SOD_CONT_CORPSE				0x4000000
#define SOD_CONT_DETAIL				0x8000000	/* brushes not used for the bsp */
#define SOD_CONT_STRUCTURAL			0x10000000	/* brushes used for the bsp */
#define SOD_CONT_TRANSLUCENT			0x20000000	/* don't consume surface fragments inside */
#define SOD_CONT_TRIGGER				0x40000000
#define SOD_CONT_NODROP				0x80000000	/* don't leave bodies or items (death fog, lava) */

#define SOD_SURF_NODAMAGE				0x1			/* never give falling damage */
#define SOD_SURF_SLICK				0x2			/* effects game physics */
#define SOD_SURF_SKY					0x4			/* lighting from environment map */
#define SOD_SURF_LADDER				0x8
#define SOD_SURF_NOIMPACT				0x10		/* don't make missile explosions */
#define SOD_SURF_NOMARKS				0x20		/* don't leave missile marks */
#define SOD_SURF_FLESH				0x40		/* make flesh sounds and effects */
#define SOD_SURF_NODRAW				0x80		/* don't generate a drawsurface at all */
#define SOD_SURF_HINT					0x100		/* make a primary bsp splitter */
#define SOD_SURF_SKIP					0x200		/* completely ignore, allowing non-closed brushes */
#define SOD_SURF_NOLIGHTMAP			0x400		/* surface doesn't need a lightmap */
#define SOD_SURF_POINTLIGHT			0x800		/* generate lighting info at vertexes */
#define SOD_SURF_METALSTEPS			0x1000		/* clanking footsteps */
#define SOD_SURF_NOSTEPS				0x2000		/* no footstep sounds */
#define SOD_SURF_NONSOLID				0x4000		/* don't collide against curves with this set */
#define SOD_SURF_LIGHTFILTER			0x8000		/* act as a light filter during q3map -light */
#define SOD_SURF_ALPHASHADOW			0x10000		/* do per-pixel light shadow casting in q3map */
#define SOD_SURF_NODLIGHT				0x20000		/* don't dlight even if solid (solid lava, skies) */
#define SOD_SURF_DUST					0x40000		/* leave a dust trail when walking on this surface */
// sod
#define SOD_SURF_NOPHYS					0x80000     /* neni fyzikalni face */
#define SOD_SURF_ENTITYFACE				0x100000    /* specialni face patrici k solid entite - napr. obrazovka plocheho monitoru pro entitu render-to-texture kamery */

/* ydnar flags */
#define SOD_SURF_VERTEXLIT			(SOD_SURF_POINTLIGHT | SOD_SURF_NOLIGHTMAP)
//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose: Debug physics renderer
//-----------------------------------------------------------------------------
#include "debug_renderable.h"
#include "IP3DDraw2D.h"

static IP3DDraw2D* g_p2D=NULL;

ConVar CVphys_dbg_stats("phys_dbg_stats", "0", CVAR_NOT_STRING | CVAR_ARCHIVE, "Show physics engine stats");

void RenderDebug(const NxDebugRenderable& data)
{
	if (!g_p2D) g_p2D = (IP3DDraw2D*)I_GetClass(IP3DRENDERER_DRAW2D);

	// Render points
	{
		/*NxU32 NbPoints = data.getNbPoints();
		const NxDebugPoint* Points = data.getPoints();
		glBegin(GL_POINTS);

		while(NbPoints--)
		{
			setupColor(Points->color);
			glVertex3fv(&Points->p.x);
			Points++;
		}
		glEnd();*/
	}

	// Render lines
	{
		NxU32 NbLines = data.getNbLines();
		const NxDebugLine* Lines = data.getLines();

		while(NbLines--)
		{
			g_p2D->Draw3DLine(P3DXPoint3D(&Lines->p0.x), P3DXPoint3D(&Lines->p1.x), Lines->color);
			Lines++;
		}
	}

	// Render triangles
	{
		NxU32 NbTris = data.getNbTriangles();
		const NxDebugTriangle* Triangles = data.getTriangles();

		struct DbgVertex
		{
			FLOAT x, y, z;
			DWORD color;
		} verts[3];

		while(NbTris--)
		{
			verts[0].x = Triangles->p0.x; verts[0].y = Triangles->p0.y; verts[0].z = Triangles->p0.z; verts[0].color = Triangles->color;
			verts[1].x = Triangles->p1.x; verts[1].y = Triangles->p1.y; verts[1].z = Triangles->p1.z; verts[1].color = Triangles->color;
			verts[2].x = Triangles->p2.x; verts[2].y = Triangles->p2.y; verts[2].z = Triangles->p2.z; verts[2].color = Triangles->color;
			
			g_pRenderer->SetFVF( (RenderFVF_t)(RFVF_XYZ | RFVF_DIFFUSE));
			g_pRenderer->DrawPrimitiveUp(RPT_TRIANGLELIST, 1, verts, sizeof(DbgVertex));
			Triangles++;
		}
	}   

	//renderuj statistiku
	if (CVphys_dbg_stats.GetBool())
	{
		NxSceneStats stats;
		char statStr[256]; int currY=10;

		g_pScene->getStats(stats);

		sprintf(statStr, "-- Physics Engine Stats --"); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "Actors: %d (%d)", stats.numActors, stats.maxActors); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "DynamicActors: %d (%d)", stats.numDynamicActors, stats.maxDynamicActors); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "StaticShapes: %d (%d)", stats.numStaticShapes, stats.maxStaticShapes); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "DynamicShapes: %d (%d)", stats.numDynamicShapes, stats.maxDynamicShapes); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "Joints: %d (%d)", stats.numJoints, stats.maxJoints); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "ActiveActors: %d (%d)", stats.numDynamicActorsInAwakeGroups,  stats.maxDynamicActorsInAwakeGroups); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "SolverBodies: %d (%d)", stats.numSolverBodies , stats.maxSolverBodies); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "AxisConstraints", stats.numAxisConstraints, stats.maxAxisConstraints); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "Contacts: %d (%d)", stats.numContacts, stats.maxContacts); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
		sprintf(statStr, "Pairs: %d (%d)", stats.numPairs, stats.maxPairs); g_p2D->DrawText(statStr, 10, currY, P3DCOLOR_XRGB(255, 0, 0)); currY += 12;
	}
	/*
	const NxSceneStats2 *stats =  g_pScene->getStats2();
	char statStr[256]; int currY=10; int currX=10;
	if (stats)
	{
		for(UINT i=0; i<stats->numStats; i++)
		{
			sprintf(statStr, "%s: %d (%d)", stats->stats[i].name, stats->stats[i].curValue, stats->stats[i].maxValue);
			g_p2D->DrawText(statStr, currX, currY, P3DCOLOR_XRGB(255, 0, 0));
			currY += 12;
			if (currY>600)
			{
				currY = 10;
				currX+=200;
			}
		}
	}*/
}

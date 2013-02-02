#pragma once

/** Unique setting id. DON'T DELETE!!! ONLY ADD NEW! */
enum
{
	// from frmMain
	WIN_POS_X=0,
	WIN_POS_Y,
	WIN_SIZE_X,
	WIN_SIZE_Y,
	WIN_MAXIMIZED,
	WIN_LRSASH,
	WIN_TBSASH,
	MNU_TABLEFT,
	DONT_SWITCH, // don't switch to properties on entity creation
	LAST_FULLSCR_VIEWPORT,

	// viewports (panRender.cpp)
	LT_VIEW_TYPE,
	LT_RENDER_TYPE,
	LT_REALTIME,
	//-
	RT_VIEW_TYPE,
	RT_RENDER_TYPE,
	RT_REALTIME,
	//-
	LB_VIEW_TYPE,
	LB_RENDER_TYPE,
	LB_REALTIME,
	//-
	RB_VIEW_TYPE,
	RB_RENDER_TYPE,
	RB_REALTIME,

	// from frmMain
	// toolbar
	T_TOOL,
	T_LOCAL,
	T_PHYS_IN_EDITOR,
	T_ORBIT,
	T_GRID,
	T_GRIDSIZE,
	T_FAR_PLANE,
	T_CAMERA_SPEED,


	NUM_SETTINGS // DON'T DELETE, THIS MUST BE LAST
};

/** Class for editor settings, works only with numbers!!! */
class Settings
{
public:
	/** only int number */
	static char* GetString(int id);
	static float GetFloat(int id);
	static int GetInt(int id);
	static long GetLong(int id);
	static bool GetBool(int id);

	/** only int number */
	static void Set(int id, char* data);
	static void Set(int id, float data);
	static void Set(int id, int data);
	static void Set(int id, long data);
	static void Set(int id, bool data);

	static bool Load(const char* filename);
	static bool Save(const char* filename);
private:
	static char msTmp[32];
	static float mfData[NUM_SETTINGS];
};

#include "settings.h"
#include <windows.h>
#include <stdio.h>

char Settings::msTmp[32];
float Settings::mfData[NUM_SETTINGS];

char* Settings::GetString(int id)
{
	if (id>=NUM_SETTINGS) { msTmp[0]=0; return msTmp; };
	sprintf(msTmp, "%d", mfData[id]);
	return msTmp;
}

float Settings::GetFloat(int id)
{
	if (id>=NUM_SETTINGS) return 0;
	return mfData[id];
}

int Settings::GetInt(int id)
{
	if (id>=NUM_SETTINGS) return 0;
	return (int)mfData[id];
}

long Settings::GetLong(int id)
{
	if (id>=NUM_SETTINGS) return 0;
	return (long)mfData[id];
}

bool Settings::GetBool(int id)
{
	if (id>=NUM_SETTINGS) return false;
	return !!mfData[id];
}





void Settings::Set(int id, char* data)
{
	if (id>=NUM_SETTINGS) return;
	mfData[id] = (float)atoi(data);
}
void Settings::Set(int id, float data)
{
	if (id>=NUM_SETTINGS) return;
	mfData[id] = data;

}
void Settings::Set(int id, int data)
{
	if (id>=NUM_SETTINGS) return;
	mfData[id] = (float)data;

}
void Settings::Set(int id, long data)
{
	if (id>=NUM_SETTINGS) return;
	mfData[id] = (float)data;

}
void Settings::Set(int id, bool data)
{
	if (id>=NUM_SETTINGS) return;
	mfData[id] = float(data);
}






bool Settings::Load(const char* filename)
{
	// try to load as much as you can
	FILE* fp = fopen(filename, "rb");
	if (!fp) return false;

	fseek(fp, 0, SEEK_END);
	long len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (len > sizeof(mfData)) len = sizeof(mfData);

	fread(mfData, 1, len, fp);
	fclose(fp);
	return true;
}

bool Settings::Save(const char* filename)
{
	// save all settings
	FILE* fp = fopen(filename, "wb");
	if (!fp) return false;
	fwrite(mfData, 1, sizeof(mfData), fp);
	fclose(fp);
	return true;
}

//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK (11.7.2007)
// Purpose:	Physics body class
//-----------------------------------------------------------------------------
#pragma once

typedef struct CPUINFO_TYP {
	bool bSSE;        // Streaming SIMD Extensions
	bool bSSE2;       // STreaming SIMD Extensions 2
	bool b3DNOW;      // 3DNow! (vendor independant)
	bool b3DNOWEX;    // 3DNow! (AMD specific extensions)
	bool bMMX;        // MMX support
	bool bMMXEX;      // MMX (AMD specific extensions)
	bool bEXT;        // extended features available
	char vendor[13];  // vendor name
	char name[48];    // cpu name
	int nCores;       // pocet jader
} CPUINFO;

typedef struct cpuid_args_s {
	unsigned long eax;
	unsigned long ebx;
	unsigned long ecx;
	unsigned long edx;
} CPUID_ARGS;

///////////////////////// GLOBALS //////////////////////

extern bool g_bSSE;
extern CPUINFO g_bCPUInfo;
void P3DInitCPU(); // vola se samo pri inicializaci globalnych promennych
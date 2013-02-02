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
#include "cpu.h"
#include "interface.h"
#include <intrin.h>

class CPUInfoLoader
{
public:
	CPUInfoLoader(){ P3DInitCPU(); };
} g__cpu__info;

// globalni promenne
bool g_bSSE = false;
CPUINFO g_bCPUInfo;

// dopredne deklarace
CPUINFO GetCPUInfo();
bool    OSSupportsSSE();
void    GetCPUName(char*, int, const char*);
int GetCoresPerPackage();

// zakladni funkce pro ziskani informaci o CPU a nastaveni globalni g_bSSE 
// (=urcuje zda je mozne pouzit SSE instrukce na danem OS a procesoru)
void P3DInitCPU() {
	CPUINFO info = GetCPUInfo();
	bool    bOS  = OSSupportsSSE();

	if (info.bSSE && bOS) g_bSSE= true;
	else g_bSSE = false;
	info.bSSE = g_bSSE;

	// zjisti pocet jader
	if (!strncmp(info.vendor, "GenuineIntel", 12)) 
		info.nCores = GetCoresPerPackage();

	g_bCPUInfo = info;
}

// Funkce na ziskani informaci o CPU
CPUINFO GetCPUInfo() {
	CPUINFO info;
	char *pStr = info.vendor;
	int n=1;
	int *pn = &n;

	// set all values to 0 (false)
	memset(&info, 0, sizeof(CPUINFO));

	// 1: See if we can get CPUID and vendor 's name then
	//    check for SSE and MMX Support (vendor independant)
	__try {
		_asm {
			mov  eax, 0          // eax=0 => CPUID returns vendor name
				CPUID                // perform CPUID function

				mov  esi,     pStr
				mov  [esi],   ebx    // first 4 chars
				mov  [esi+4], edx    // next for chars
				mov  [esi+8], ecx    // last 4 chars

				mov  eax, 1          // EAX=1 => CPUID returns feature bits
				CPUID                // perform CPUID (puts feature info to EDX)

				test edx, 04000000h  // test bit 26 for SSE2
				jz   _NOSSE2         // if test failed jump
				mov  [info.bSSE2], 1 // set to true

_NOSSE2: test edx, 02000000h  // test bit 25 for SSE
		 jz   _NOSSE          // if test failed jump
		 mov  [info.bSSE], 1  // set to true

_NOSSE:  test edx, 00800000h  // test bit 23 for MMX
		 jz   _EXIT1          // if test failed jump
		 mov  [info.bMMX], 1  // set to true
_EXIT1:  // nothing to do anymore
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
			return info;         // cpu inactive
		return info;            // unexpected exception occurred
	}

	// 2: See if we can get extended info (vendor independant)
	_asm {
		mov  eax, 80000000h     // EAX=0x8 => CPUID returns extended features
			CPUID
			cmp  eax, 80000000h     // must be greater than 0x80
			jbe  _EXIT2             // below or equal 0x80 then jump away
			mov [info.bEXT], 1      // set to true

			mov  eax, 80000001h     // => CPUID will copy ext. feat. bits to EDX
			CPUID
			test edx, 80000000h     // 0x8 indicates 3DNow!support
			jz   _EXIT2             // if failed jump away
			mov  [info.b3DNOW], 1   // set to true
_EXIT2:
	}

	// 3: Get vendor specific stuff
	//    INTEL: CPU id
	//    AMD:   CPU id, 3dnow_ex, mmx_ex
	if ( (strncmp(info.vendor, "GenuineIntel", 12)==0) && info.bEXT) {       // INTEL
		_asm {

			mov  eax, 1             // => CPUID will copy ext. feat. info
				CPUID                   //    to EDX and brand id to EBX
				mov  esi,   pn          // get brand id which is only supported
				mov  [esi], ebx         // by processors > PIII/Celeron
		}
		int m=0;
		memcpy(&m, pn, sizeof(char)); // id only needs lowest 8 bits
		n = m;
	}
	else if ( (strncmp(info.vendor, "AuthenticAMD", 12)==0) && info.bEXT) {  // AMD

		_asm {
			mov  eax, 1             // => CPUID will copy ext. feat. info
				CPUID                   //    to EDX and brand id to EAX
				mov  esi,   pn          // get cpu type
				mov  [esi], eax

				mov  eax, 0x80000001    // => CPUID will copy ext. feat. bits
				CPUID                   //    to EDX and cpu type to EAX

				test edx, 0x40000000    // 0x4 indicates AMD extended 3DNow!
				jz   _AMD1              // if failed jump away
				mov  [info.b3DNOWEX], 1 // set to true
_AMD1:   test edx, 0x00400000    // 0x004 indicates AMD extended MMX
		 jz   _AMD2              // if fail jump away
		 mov  [info.bMMXEX], 1   // set to true
_AMD2:
		}
	}

	else {
		if (info.bEXT)
			; /* UNKNOWN VENDOR */
		else
			; /* lack of extended features */
	}

	info.vendor[13] = '\0';                // end of string
	info.name[0] = 0;
	GetCPUName(info.name, n, info.vendor); // select cpu name

	return info;
}

// kontrola zda OS podporuje SSE
bool OSSupportsSSE() {
	// try SSE instruction and look for crash
	__try
	{
		_asm xorps xmm0, xmm0
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
			return false;  // sse not supported by os
		return false;     // unknown exception occured
	}

	return true;
}

// Get CPU name - neni moc duveryhodne
void GetCPUName(char *chName, int n, const char *vendor) {
	int nExIds=0; int CPUInfo[4] = {-1};
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	// Get the information associated with each extended ID.
	for (int i=0x80000000; i<=nExIds; ++i)
	{
		__cpuid(CPUInfo, i);

		// Interpret CPU brand string
		if  (i == 0x80000002)
			memcpy(chName, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000003)
			memcpy(chName + 16, CPUInfo, sizeof(CPUInfo));
		else if  (i == 0x80000004)
			memcpy(chName + 32, CPUInfo, sizeof(CPUInfo));
	}

	if (chName[0]) return;

	// Intel
	if (strncmp(vendor, "GenuineIntel", 12)==0) {
		switch (n) {
		 case 0:
			 sprintf(chName, "Unknown Old"); break;
		 case 1:
			 sprintf(chName, "Celeron"); break;
		 case 2:
			 sprintf(chName, "Pentium III"); break;
		 case 3:
			 sprintf(chName, "Pentium III Xeon"); break;
		 case 4:
			 sprintf(chName, "Pentium III"); break;
		 case 6:
			 sprintf(chName, "Pentium III-M"); break;
		 case 7:
			 sprintf(chName, "Pentium Celeron"); break;
		 case 8:
			 sprintf(chName, "Pentium IV (Genuine)"); break;
		 case 9:
			 sprintf(chName, "Pentium IV"); break;
		 case 10:
			 sprintf(chName, "Pentium Celeron"); break;
		 case 11:
			 sprintf(chName, "Pentium Xeon / Xeon-MP"); break;
		 case 12:
			 sprintf(chName, "Pentium Xeon-MP"); break;
		 case 14:
			 sprintf(chName, "Pentium IV-M / Xeon"); break;
		 case 15:
			 sprintf(chName, "Pentium Celeron"); break;
		 default: 
			 sprintf(chName, "Unknown"); break;
		}
	}
	// AMD
	else if (strncmp(vendor, "AuthenticAMD", 12)==0) {
		switch (n) {
		 case 1660:
			 sprintf(chName, "Athlon / Duron (Model-7)"); break;
		 case 1644:
			 sprintf(chName, "Athlon / Duron (Model-6)"); break;
		 case 1596:
			 sprintf(chName, "Athlon / Duron (Model-3)"); break;
		 case 1612:
			 sprintf(chName, "Athlon (Model-4)"); break;
		 case 1580:
			 sprintf(chName, "Athlon (Model-2)"); break;
		 case 1564:
			 sprintf(chName, "Athlon (Model-1)"); break;
		 case 1463:
			 sprintf(chName, "K6-III (Model-9)"); break;
		 case 1420:
			 sprintf(chName, "K6-2 (Model-8)"); break;
		 case 1404:
			 sprintf(chName, "K6 (Model-7)"); break;
		 case 1388:
			 sprintf(chName, "K6 (Model-6)"); break;
		 case 1340:
			 sprintf(chName, "K5 (Model-3)"); break;
		 case 1324:
			 sprintf(chName, "K5 (Model-2)"); break;
		 case 1308:
			 sprintf(chName, "K5 (Model-1)"); break;
		 case 1292:
			 sprintf(chName, "K5 (Model-0)"); break;
		 default:
			 sprintf(chName, "Unknown"); break;
		}
	}
	return;
}

void cpuid32(CPUID_ARGS* p) {
	__asm {
		mov	edi, p
			mov eax, [edi].eax
			mov ecx, [edi].ecx // for functions such as eax=4
			cpuid
			mov [edi].eax, eax
			mov [edi].ebx, ebx
			mov [edi].ecx, ecx
			mov [edi].edx, edx
	}
}

int GetCoresPerPackage()
{
	int nCaches=0; // Is explicit cache info available?
	int coresPerPackage=1; // Assume 1 core per package if info not available 
	DWORD t;
	int cacheIndex;
	CPUID_ARGS ca;

	ca.eax = 0;
	cpuid32(&ca);
	t = ca.eax;
	if ((t > 3) && (t < 0x80000000)) { 
		for (cacheIndex=0; ; cacheIndex++) {
			ca.eax = 4;
			ca.ecx = cacheIndex;
			cpuid32(&ca);
			t = ca.eax;
			if ((t & 0x1F) == 0)
				break;
			nCaches++;
		}
	}

	if (nCaches > 0) {
		ca.eax = 4;
		ca.ecx = 0; // first explicit cache
		cpuid32(&ca);
		coresPerPackage = ((ca.eax >> 26) & 0x3F) + 1; // 31:26
	}
	return coresPerPackage;
}
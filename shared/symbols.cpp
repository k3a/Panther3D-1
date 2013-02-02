//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	MESS
// Purpose:	Shared symbols
//-----------------------------------------------------------------------------
/*
MÌsto pro umisùov·nÌ sdÌlen˝ch funkci
*/
#include "symbols.h"

int wildcmp(const char *pszWildcard, const char *pszString) {
	// Written by Jack Handy - jakkhandy@hotmail.com
	const char *wild = pszWildcard;
	const char *string = pszString;
	const char *cp = 0, *mp = 0;

	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?')) {
			return 0;
		}
		wild++;
		string++;
	}

	while (*string) {
		if (*wild == '*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string+1;
		} else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		} else {
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*') {
		wild++;
	}
	return !*wild;
}
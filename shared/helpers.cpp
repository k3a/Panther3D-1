//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------ 
// 
// This contents is the property of 7thSquad team. 
// The contents may be used and/or copied only with the written permission of 
// 7thSquad team, or in accordance with the terms and conditions stipulated in 
// the agreement/contract under which the contents have been supplied. 
// 
// Author:     KEXIK 
// Purpose:     Shared helper functions cpp 
//----------------------------------------------------------------------------- 
#include "helpers.h" 
#include <string.h>

const char *stristr( const char *s, const char *find) 
{ 
	if(!s | !find) return (const char*)0;

	register char c, sc; 
	register size_t len; 

	if ((c = *find++) != 0) 
	{ 
		if (c >= 'a' && c <= 'z') 
		{ 
			c -= ('a' - 'A'); 
		} 
		len = strlen(find); 
		do 
		{ 
			do 
			{ 
				if ((sc = *s++) == 0) 
					return NULL; 
				if (sc >= 'a' && sc <= 'z') 
				{ 
					sc -= ('a' - 'A'); 
				} 
			} while (sc != c); 
		} while (strnicmp(s, find, len) != 0); 
		s--; 
	} 
	return s; 
}

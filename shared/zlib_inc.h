//------------ Copyright © 2005-2007 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	Zlib + helpers
//-----------------------------------------------------------------------------
#pragma once

#include <stdio.h>
#include "zlib/zlib.h"
#include "zlib/zip.h"

#define BUFLEN 16384

// GZIP
bool gz_compress(FILE *in, gzFile out);
bool gz_uncompress(gzFile in, FILE *out);

//ZIP
bool zipAddFileToZip(zipFile zip, const char* newFilePath, FILE* fp);
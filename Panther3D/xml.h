//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	XML Reader/writer, class based on ezXML
//-----------------------------------------------------------------------------

/* ezxml.h
 *
 * Copyright 2004, 2005 Aaron Voisine <aaron@voisine.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "IP3DXML.h"

#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>

#define EZXML_BUFSIZE 1024 // size of internal memory buffers
#define EZXML_NAMEM   0x80 // name is malloced
#define EZXML_TXTM    0x40 // txt is malloced
#define EZXML_DUP     0x20 // attribute name and value are strduped
#define EZXML_WS   "\t\r\n "  // whitespace
#define EZXML_ERRL 128        // maximum error string length

typedef struct ezxml_root *ezxml_root_t;
struct ezxml_root {       // additional data for the root tag
    struct ezxml xml;     // is a super-struct built on top of ezxml struct
    ezxml_t cur;          // current xml tree insertion point
    char *m;              // original xml string
    size_t len;           // length of allocated memory for mmap, -1 for malloc
    char *u;              // UTF-8 conversion of string if original was UTF-16
    char *s;              // start of work area
    char *e;              // end of work area
    char **ent;           // general entities (ampersand sequences)
    char ***attr;         // default attributes
    char ***pi;           // processing instructions
    short standalone;     // non-zero if <?xml standalone="yes"?>
    char err[EZXML_ERRL]; // error string
};

class CP3DXML : public IP3DXML
{
public:
	bool Init();

	ezxml_t Parse_str(char *s, size_t len);

	ezxml_t Parse_file(const char *fn);

	ezxml_t Child(ezxml_t xml, const char *name);

	ezxml_t Next(ezxml_t xml);

	ezxml_t Idx(ezxml_t xml, int idx);

	#define Name(xml) ((xml) ? xml->name : NULL)

	#define Txt(xml) ((xml) ? xml->txt : "")

	const char *Attr(ezxml_t xml, const char *attr);

	ezxml_t Get(ezxml_t xml, ...);

	char *ToXml(ezxml_t xml);

	const char **Pi(ezxml_t xml, const char *target);

	void Free(ezxml_t xml);

	const char *Error(ezxml_t xml);

	ezxml_t New(const char *name);
	#define New_d(name) SetFlag(ezxml_new(strdup(name)), EZXML_NAMEM)

	ezxml_t AddChild(ezxml_t xml, const char *name, size_t off);
	#define AddChild_d(xml, name, off) SetFlag(AddChild(xml, strdup(name), off), EZXML_NAMEM)

	ezxml_t SetTxt(ezxml_t xml, const char *txt);
	#define SetTxt_d(xml, txt) SetFlag(SetTxt(xml, strdup(txt)), EZXML_TXTM)

	void SetAttr(ezxml_t xml, const char *name, const char *value);
	#define SetAttr_d(xml, name, value) SetAttr(SetFlag(xml, EZXML_DUP), strdup(name), strdup(value))

	ezxml_t SetFlag(ezxml_t xml, short flag);

	void Remove(ezxml_t xml);

private:
	ezxml_t VGet(ezxml_t xml, va_list ap);
	ezxml_t Err(ezxml_root_t root, char *s, const char *err, ...);
	char* Decode(char *s, char **ent, char t);
	void OpenTag(ezxml_root_t root, char *name, char **attr);
	void CharContent(ezxml_root_t root, char *s, size_t len, char t);
	ezxml_t CloseTag(ezxml_root_t root, char *name, char *s);
	int EntOk(char *name, char *s, char **ent);
	void ProcInst(ezxml_root_t root, char *s, size_t len);
	short Internal_dtd(ezxml_root_t root, char *s, size_t len);
	char* Str2utf8(char **s, size_t *len);
	void FreeAttr(char **attr);
	char* AmpEncode(const char *s, size_t len, char **dst, size_t *dlen, size_t *max, short a);
	char* ToXml_r(ezxml_t xml, char **s, size_t *len, size_t *max, size_t start, char ***attr);
};
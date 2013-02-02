//----------- Copyright © 2005-2007, 7thSquad, all rights reserved ------------
//
// This contents is the property of 7thSquad team.
// The contents may be used and/or copied only with the written permission of
// 7thSquad team, or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Author:	KEXIK
// Purpose:	XML Reader/writer iface (engine)
//-----------------------------------------------------------------------------
#pragma once

#include "interface.h"
#include <stdio.h>

typedef struct ezxml *ezxml_t;
struct ezxml {
    char *name;      // tag name
    char **attr;     // tag attributes { name, value, name, value, ... NULL }
    char *txt;       // tag character content, empty string if none
    size_t off;      // tag offset from start of parent tag character content
    ezxml_t next;    // next tag with same name in this section at this depth
    ezxml_t sibling; // next tag with different name in same section and depth
    ezxml_t ordered; // next tag, same section and depth, in original order
    ezxml_t child;   // head of sub tag list, NULL if none
    ezxml_t parent;  // parent tag, NULL if current tag is root tag
    short flags;     // additional information
};

class IP3DXML : public IP3DBaseInterface
{
public:
	// Given a string of xml data and its length, parses it and creates an ezxml
	// structure. For efficiency, modifies the data by adding null terminators
	// and decoding ampersand sequences. If you don't want this, copy the data and
	// pass in the copy. Returns NULL on failure.
	virtual ezxml_t Parse_str(char *s, size_t len)=0;

	// Wrapper for ezxml_parse_str() that accepts a file path. Reads the entire
	// stream into memory and then parses it.
	virtual ezxml_t Parse_file(const char *fn)=0;

	// returns the first child tag (one level deeper) with the given name or NULL
	// if not found
	virtual ezxml_t Child(ezxml_t xml, const char *name)=0;

	// returns the next tag of the same name in the same section and depth or NULL
	// if not found
	//virtual #define Next(xml) ((xml) ? xml->next : NULL)
	virtual ezxml_t Next(ezxml_t xml)=0;

	// Returns the Nth tag with the same name in the same section at the same depth
	// or NULL if not found. An index of 0 returns the tag given.
	virtual ezxml_t Idx(ezxml_t xml, int idx)=0;

	// returns the name of the given tag
	//#define Name(xml) ((xml) ? xml->name : NULL)

	// returns the given tag's character content or empty string if none
	//#define Txt(xml) ((xml) ? xml->txt : "")

	// returns the value of the requested tag attribute, or NULL if not found
	virtual const char *Attr(ezxml_t xml, const char *attr)=0;

	// Traverses the ezxml sturcture to retrieve a specific subtag. Takes a
	// variable length list of tag names and indexes. The argument list must be
	// terminated by either an index of -1 or an empty string tag name. Example: 
	// title = ezxml_get(library, "shelf", 0, "book", 2, "title", -1);
	// This retrieves the title of the 3rd book on the 1st shelf of library.
	// Returns NULL if not found.
	virtual ezxml_t Get(ezxml_t xml, ...)=0;

	// Converts an ezxml structure back to xml. Returns a string of xml data that
	// must be freed.
	virtual char *ToXml(ezxml_t xml)=0;

	// returns a NULL terminated array of processing instructions for the given
	// target
	virtual const char **Pi(ezxml_t xml, const char *target)=0;

	// frees the memory allocated for an ezxml structure
	virtual void Free(ezxml_t xml)=0;
	    
	// returns parser error message or empty string if none
	virtual const char *Error(ezxml_t xml)=0;

	// returns a new empty ezxml structure with the given root tag name
	virtual ezxml_t New(const char *name)=0;

	// wrapper for ezxml_new() that strdup()s name
	//#define New_d(name) SetFlag(ezxml_new(strdup(name)), EZXML_NAMEM)

	// Adds a child tag. off is the offset of the child tag relative to the start
	// of the parent tag's character content. Returns the child tag.
	virtual ezxml_t AddChild(ezxml_t xml, const char *name, size_t off)=0;

	// wrapper for ezxml_add_child() that strdup()s name
//	#define AddChild_d(xml, name, off) SetFlag(AddChild(xml, strdup(name), off), EZXML_NAMEM)

	// sets the character content for the given tag and returns the tag
	virtual ezxml_t SetTxt(ezxml_t xml, const char *txt)=0;

	// wrapper for ezxml_set_txt() that strdup()s txt
	//#define SetTxt_d(xml, txt) SetFlag(SetTxt(xml, strdup(txt)), EZXML_TXTM)

	// Sets the given tag attribute or adds a new attribute if not found. A value
	// of NULL will remove the specified attribute.
	virtual void SetAttr(ezxml_t xml, const char *name, const char *value)=0;

	// Wrapper for ezxml_set_attr() that strdup()s name/value. Value cannot be NULL
	//#define SetAttr_d(xml, name, value) SetAttr(SetFlag(xml, EZXML_DUP), strdup(name), strdup(value))

	// sets a flag for the given tag and returns the tag
	virtual ezxml_t SetFlag(ezxml_t xml, short flag)=0;

	// removes a tag along with all its subtags
	virtual void Remove(ezxml_t xml)=0;
};

#define IP3DENGINE_XML "P3DXML_2" // nazev ifacu
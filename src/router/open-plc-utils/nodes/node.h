/*====================================================================*
 *
 *   node.h - document node definitions and declarations;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef NODE_HEADER
#define NODE_HEADER

/*====================================================================*
 *   system headers;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdint.h>

/*====================================================================*
 *   custom headers;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define ISO_CHARSET "iso-8859-1"
#define ISO_CONTENT "text/html"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define W3C_STD "-//W3C//DTD XHTML 1.0 Strict//EN"
#define W3C_DTD "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define XML_VERSION "1.0"
#define XML_NAMESPACE "http://www.w3.org/2001/XMLSchema-instance"
#define XML_CHARSET ISO_CHARSET
#define XML_LANGUAGE "en-us"
#define XML_SCHEMA "http://www.w3.org/2001/XMLSchema"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define NODE_ELEM '<'
#define NODE_SGML '!'
#define NODE_INST '?'
#define NODE_ATTR ' '
#define NODE_VALU '='
#define NODE_ETAG '/'
#define NODE_DATA '>'

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define DATA_SCHEMA "piboffset.xsd"
#define DATA_NAME "name"
#define DATA_TEXT "text"
#define DATA_OBJECT "pib"
#define DATA_MEMBER "object"
#define DATA_OFFSET "offset"
#define DATA_LENGTH "length"
#define DATA_STRUCT "array"
#define DATA_STRING "dataString"
#define DATA_MEMORY "dataHex"
#define DATA_HUGE "dataHuge"
#define DATA_LONG "dataLong"
#define DATA_WORD "dataWord"
#define DATA_BYTE "dataByte"

/*====================================================================*
 *   variables;
 *--------------------------------------------------------------------*/

typedef struct node

{
	struct node * above;
	struct node * prior;
	struct node * after;
	struct node * below;
	unsigned line;
	unsigned type;
	char * text;
}

NODE;

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

char const * xmlselect (NODE const *, char const * element, char const * attribute);
signed xmlread (NODE *, char const * filename);
signed xmlscan (NODE *);
signed xmledit (NODE const *, void * memory, size_t extent);
NODE * xmlopen (char const * filename);
NODE * xmlnode (NODE *);
NODE const * xmlelement (NODE const *, char const * name);
NODE const * xmlattribute (NODE const *, char const * name);
NODE const * xmlvalue (NODE const *);
NODE const * xmldata (NODE const *);
void xmlschema ();
void xmltree (NODE const *);
void xmlfree (NODE *);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif


/*====================================================================*
 *
 *   void xmlschema ();
 *
 *   node.h
 *
 *   print an XML schema on stdout that is compatible with function
 *   xmledit; the schema should be used to validate files before any
 *   attempt is make to parse them;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLSCHEMA_SOURCE
#define XMLSCHEMA_SOURCE

#include "../nodes/node.h"
#include "../tools/format.h"

void xmlschema ()

{
	unsigned margin = 0;
	output (margin, "<?xml version='%s' encoding='%s'?>", XML_VERSION, XML_CHARSET);
	output (margin++, "<xs:schema xmlns:xs='%s' elementFormDefault='%s'>", XML_SCHEMA, "qualified");
	output (margin++, "<xs:element name='%s'>", DATA_OBJECT);
	output (margin++, "<xs:complexType>");

#if 0

	output (margin, "<xs:attribute name='%s' use='optional' type='xs:NCName'/>", DATA_NAME);
	output (margin, "<xs:attribute name='%s' use='optional' type='xs:string'/>", DATA_TEXT);

#endif

	output (margin++, "<xs:sequence>");
	output (margin, "<xs:element maxOccurs='unbounded' ref='%s'/>", DATA_MEMBER);
	output (margin--, "</xs:sequence>");
	output (margin--, "</xs:complexType>");
	output (margin--, "</xs:element>");
	output (margin++, "<xs:element name='%s'>", DATA_MEMBER);
	output (margin++, "<xs:complexType>");
	output (margin, "<xs:attribute name='%s' use='required' type='xs:NCName'/>", DATA_NAME);

#if 0

	output (margin, "<xs:attribute name='%s' use='optional' type='xs:string'/>", DATA_TEXT);

#endif

	output (margin++, "<xs:sequence>");
	output (margin++, "<xs:sequence>");
	output (margin, "<xs:element ref='%s'/>", DATA_OFFSET);
	output (margin, "<xs:element ref='%s'/>", DATA_LENGTH);
	output (margin--, "</xs:sequence>");
	output (margin++, "<xs:choice>");
	output (margin, "<xs:element ref='%s'/>", DATA_STRUCT);
	output (margin, "<xs:element ref='%s'/>", DATA_STRING);
	output (margin, "<xs:element ref='%s'/>", DATA_HUGE);
	output (margin, "<xs:element ref='%s'/>", DATA_LONG);
	output (margin, "<xs:element ref='%s'/>", DATA_WORD);
	output (margin, "<xs:element ref='%s'/>", DATA_BYTE);
	output (margin--, "</xs:choice>");
	output (margin--, "</xs:sequence>");
	output (margin--, "</xs:complexType>");
	output (margin--, "</xs:element>");
	output (margin, "<xs:element name='%s' type='xs:positiveInteger'/>", DATA_LENGTH);
	output (margin, "<xs:element name='%s' type='xs:hexBinary'/>", DATA_OFFSET);
	output (margin++, "<xs:element name='%s'>", DATA_STRUCT);
	output (margin++, "<xs:complexType>");

#if 0

	output (margin, "<xs:attribute name='%s' use='optional' type='xs:NCName'/>", DATA_NAME);
	output (margin, "<xs:attribute name='%s' use='optional' type='xs:string'/>", DATA_TEXT);

#endif

	output (margin++, "<xs:choice maxOccurs='unbounded'>");
	output (margin, "<xs:element ref='%s'/>", DATA_BYTE);
	output (margin, "<xs:element ref='%s'/>", DATA_MEMORY);
	output (margin--, "</xs:choice>");
	output (margin--, "</xs:complexType>");
	output (margin--, "</xs:element>");
	output (margin, "<xs:element name='%s' type='xs:string'/>", DATA_STRING);
	output (margin, "<xs:element name='%s' type='xs:hexBinary'/>", DATA_MEMORY);
	output (margin, "<xs:element name='%s' type='xs:unsignedLong'/>", DATA_HUGE);
	output (margin, "<xs:element name='%s' type='xs:unsignedInt'/>", DATA_LONG);
	output (margin, "<xs:element name='%s' type='xs:unsignedShort'/>", DATA_WORD);
	output (margin, "<xs:element name='%s' type='xs:unsignedByte'/>", DATA_BYTE);
	output (margin--, "</xs:schema>");
	return;
}


#endif


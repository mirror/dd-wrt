/*====================================================================*
 *
 *   char const * xmlselect (struct node const * node, char const * element, char const * attribute);
 *
 *   node.h
 *
 *   search node for the named element and attribute and return the
 *   attribute value;
 *
 *   if the attribute argument is null or nil then return the element
 *   content string;
 *
 *   if the element does not exist or the attribute does not exist
 *   for that element or the attribute has no value then an empty
 *   string is returned;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLSELECT_SOURCE
#define XMLSELECT_SOURCE

#include <string.h>

#include "../nodes/node.h"

char const * xmlselect (struct node const * node, char const * element, char const * attribute)

{
	node = xmlelement (node, element);
	if ((attribute) && (* attribute))
	{
		node = xmlattribute (node, attribute);
		node = xmlvalue (node);
	}
	else
	{
		node = xmldata (node);
	}
	return (node? node->text: "");
}


#endif


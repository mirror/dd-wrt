/*====================================================================*
 *
 *   struct node * xmlattribute (struct node const * node, char const * name);
 *
 *   node.h
 *
 *   search an element node for the named attribute node; return the
 *   attribute node address;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLATTRIBUTE_SOURCE
#define XMLATTRIBUTE_SOURCE

#include <string.h>

#include "../nodes/node.h"

struct node const * xmlattribute (struct node const * node, char const * name)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		if (node->type == NODE_ATTR)
		{
			if (!strcmp (node->text, name))
			{
				break;
			}
		}
		node=node->after;
	}
	return (node);
}


#endif


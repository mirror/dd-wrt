/*====================================================================*
 *
 *   struct node const * xmldata (struct node const * node);
 *
 *   node.h
 *
 *   search an element node for the next data node and return the
 *   data node address;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLDATA_SOURCE
#define XMLDATA_SOURCE

#include <string.h>

#include "../nodes/node.h"

struct node const * xmldata (struct node const * node)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		if (node->type == NODE_DATA)
		{
			break;
		}
		node=node->after;
	}
	return (node);
}


#endif


/*====================================================================*
 *
 *   NODE * xmlfree (NODE * node);
 *
 *   node.h
 *
 *   recursively free child nodes; minimize recursion by following
 *   node->after at each level;
 *
 *   the caller must free the root node; this allows that the root
 *   to be statically declared, if desired;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLFREE_SOURCE
#define XMLFREE_SOURCE

#include <stdlib.h>
#include <memory.h>

#include "../nodes/node.h"

void xmlfree (NODE * node)

{
	if (node)
	{
		node = node->below;
	}
	while (node)
	{
		NODE * temp = node;
		if (node->above)
		{
			node->above->below = (NODE *)(0);
		}
		if (node->prior)
		{
			node->prior->after = (NODE *)(0);
		}
		xmlfree (node);
		node = node->after;
		free (temp);
	}
	return;
}


#endif


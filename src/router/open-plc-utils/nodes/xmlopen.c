/*====================================================================*
 *
 *   NODE * xmlopen (char const * filename);
 *
 *   node.h
 *
 *   open an XML file and return the parse tree root;
 *
 *   the entire file is read into a buffer associated with the text
 *   member in the root node; the buffer is then split into strings
 *   referenced by child nodes, forming a hierarchial string vector;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef XMLOPEN_SOURCE
#define XMLOPEN_SOURCE

#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>

#include "../nodes/node.h"
#include "../tools/memory.h"
#include "../tools/files.h"
#include "../tools/error.h"

NODE * xmlopen (char const * filename)

{
	ssize_t length;
	NODE * node = NEW (NODE);
	signed fd = open (filename, O_BINARY|O_RDONLY);
	if (fd == -1)
	{
		error (1, errno, FILE_CANTOPEN, filename);
	}
	length = lseek (fd, 0, SEEK_END);
	if (length == -1)
	{
		error (1, errno, FILE_CANTSEEK, filename);
	}
	if (lseek (fd, 0, SEEK_SET) == -1)
	{
		error (1, errno, FILE_CANTHOME, filename);
	}
	memset (node, 0, sizeof (NODE));
	node->text = STR (length);
	if (read (fd, node->text, length) < length)
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	node->text [length] = (char)(0);
	close (fd);
	xmlscan (node);
	return (node);
}


#endif


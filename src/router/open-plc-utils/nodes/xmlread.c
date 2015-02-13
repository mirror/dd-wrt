/*====================================================================*
 *
 *   NODE * xmlread (NODE * node, char const * filename);
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

#ifndef XMLREAD_SOURCE
#define XMLREAD_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>

#include "../tools/error.h"
#include "../tools/files.h"
#include "../nodes/node.h"

signed xmlread (NODE * node, char const * filename)

{
	struct stat stat;
	signed fd;
	memset (node, 0, sizeof (NODE));
	if (lstat (filename, &stat))
	{
		error (1, errno, FILE_CANTSTAT, filename);
	}
	if (!(node->text = malloc (stat.st_size + 1)))
	{
		error (1, errno, FILE_CANTLOAD, filename);
	}
	if ((fd = open (filename, O_BINARY|O_RDONLY)) == -1)
	{
		error (1, errno, FILE_CANTOPEN, filename);
	}
	if (read (fd, node->text, stat.st_size) != stat.st_size)
	{
		error (1, errno, FILE_CANTREAD, filename);
	}
	node->text [stat.st_size] = (char)(0);
	close (fd);
	return (0);
}


#endif


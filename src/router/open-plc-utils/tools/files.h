/*====================================================================*
 *
 *   files.h - custom filesystem definitions and declarations;
 *
 *   this file is a subset of the original that includes only those
 *   definitions and declaration needed for toolkit programs;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef FILES_HEADER
#define FILES_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *   path and file extenders;
 *--------------------------------------------------------------------*/

#ifdef WIN32
#define PATH_C_EXTENDER '\\'
#define FILE_C_EXTENDER '.'
#else
#define PATH_C_EXTENDER '/'
#define FILE_C_EXTENDER '.'
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define FILE_CANTSTAT "%s can't stat %s", __func__
#define FILE_CANTOPEN "%s can't open %s", __func__
#define FILE_CANTHOME "%s can't home %s", __func__
#define FILE_CANTSIZE "%s can't size %s", __func__
#define FILE_CANTSEEK "%s can't seek %s", __func__
#define FILE_CANTREAD "%s can't read %s", __func__
#define FILE_CANTLOAD "%s can't load %s", __func__
#define FILE_CANTEDIT "%s can't edit %s", __func__
#define FILE_CANTSAVE "%s can't save %s", __func__
#define FILE_CANTSPAN "%s won't span %s", __func__
#define FILE_WONTREAD "%s won't read %s", __func__

/*====================================================================*
 *   file create and access modes;
 *--------------------------------------------------------------------*/

#ifdef WIN32
#define FILE_FILEMODE S_IREAD|S_IWRITE
#else
#define FILE_FILEMODE S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define O_BINARY 0
#endif

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

FILE *efopen (char const *filename, char const *openmode);
FILE *efreopen (char const *filename, char const *openmode, FILE *fp);
bool checkfilename (char const * string);
char const * filepart (char const * string);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif


/*====================================================================*
 *
 *   putoptv.h - putopt related definitions and declarations;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef PUTOPTV_HEADER
#define PUTOPTV_HEADER

/*====================================================================*
 *   constant definitions;
 *--------------------------------------------------------------------*/

#define PUTOPTV_I_OPTIONS 0
#define PUTOPTV_I_COMMAND 1
#define PUTOPTV_I_PROGRAM 2
#define PUTOPTV_I_DETAILS 3

#define PUTOPTV_S_FILTER "file [file] [...] or [< stdin][> stdout]"
#define PUTOPTV_S_FUNNEL "file [file] [...] [> stdout]"
#define PUTOPTV_S_SEARCH "findspec [findspec] [...] [> stdout]"
#define PUTOPTV_S_DIVINE "[> stdout]"

/*====================================================================*
 *   function declarations;
 *--------------------------------------------------------------------*/

void putoptv (char const *help []);

/*====================================================================*
 *   end definitions;
 *--------------------------------------------------------------------*/

#endif


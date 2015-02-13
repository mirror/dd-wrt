/*====================================================================*
 *
 *   main.c - template program;
 *
 *.
 *:
 *;
 *
 *--------------------------------------------------------------------*/


/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/error.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/error.c"
#endif

/*====================================================================*
 *
 *   void function (void);
 *
 *
 *--------------------------------------------------------------------*/

void function (void)

{
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"",
		"",
		"Template Program",
		(char const *) (0)
	};
	signed c;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (!argc)
	{
		function ();
	}
	while ((argc) && (* argv))
	{
		function ();
		argc--;
		argv++;
	}
	exit (0);
}


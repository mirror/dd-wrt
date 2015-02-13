/*====================================================================*
 *
 *   int getoptv (int argc, char const * argv[], char const * optv[]);
 *
 *   getoptv.h
 *
 *   this is a posix compliant getopt() function that supports no
 *   extensions; see the posix website for specification details;
 *
 *   <http://www.opengroup.org/onlinepubs/007904975/functions/getopt.html>
 *
 *   we implemented this function to ensure that linux and windows
 *   consoles act the same; microsoft c++ would not compile the
 *   debian version of getopt and so, after trying to fix things,
 *   we decided to start fresh; the debian version is too complex;
 *
 *   this function conforms to posix standard; it does not support
 *   gnu style extensions like "--option" for arguments or "ab::c"
 *   for operands; if you don't know what that means then you won't
 *   care, either; you should avoid such extentions, anyway;
 *
 *   the posix standard says that command options and operands must
 *   precede other arguments; this version of getopt allows options
 *   and operands to appear anywhere and makes non-compliant argv[]
 *   compliant in the process;
 *
 *   we define characters instead of coding them so that microsoft
 *   folks can use '/' instead of '-' and still preserve the posix
 *   behaviour;
 *
 *   we declare optarg as "char const *" so that the target cannot
 *   be changed by the application; this is not POSIX compliant so
 *   it will conflict with other getopt variants; getopt.h is often
 *   included with unistd.h which is a common file;
 *
 *   systems.
 *
 *   this version calls virtually no functions and should compile
 *   on any posix system;
 *
 *   you may include getoptv.h or declare these variables:
 *
 *    extern char const *optarg;
 *    extern int optopt;
 *    extern int optind;
 *    extern int opterr;
 *
 *   you may cut and paste this c language code segment to get you
 *   started; you must insert your own code and case breaks;
 *
 *    signed c;
 *    optind = 1;
 *    opterr = 1;
 *
 *    while ((c = getoptv(argc, argv, * optv)) != -1)
 *    {
 *       switch(c)
 *       {
 *          case 'a': // optopt is 'a'; optarg is NULL;
 *          case 'b': // optopt is 'b'; optarg is operand;
 *          case ':': // optopt is option; optarg is NULL; missing operand;
 *          case '?': // optopt is option; optarg is NULL; illegal option;
 *           default: // optopt is option: optarg is NULL; illegal option;
 *       }
 *    }
 *
 *    after options and operands are processed, optind points to
 *    the next argv [] string; loop until optind equals argc or
 *    argv[optind] is NULL; we check both but either will do;
 *
 *    while ((optind < argc) && (argv [optind]))
 *    {
 *       // do stuff to argv[optind++].
 *    }
 *
 *   alternately, and even better, the following works just fine:
 *
 *    argc -= optind;
 *    argv += optind;
 *    while ((argc) && (* argv))
 *    {
 *       // do stuff to * argv.
 *
 *    }
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef GETOPTV_SOURCE
#define GETOPTV_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/error.h"

char const * program_name = "program";
char * optarg = (char *) (0);
signed optopt = (char) (0);
signed optind = 1;
signed opterr = 1;
signed optmin = 0;
signed getoptv (int argc, char const * argv [], char const * optv [])

{
	static char const * string;
	static char const * option;
	static signed count;
	signed index;
	if ((optind == 0) || (optind == 1))
	{
		for (program_name = string = * argv; * string; string++)
		{
			if ((* string == '/') || (* string == '\\'))
			{
				program_name = string + 1;
			}
		}
		string = (char *) (0);
		if (argc == optmin)
		{
			putoptv (optv);
			exit (0);
		}
		count = optind = 1;
	}
	while ((count < argc) || (string))
	{
		if (string)
		{
			if (*string)
			{
				optarg = (char *) (0);
				optopt = *string++;
				for (option = * optv; * option; option++)
				{
					if (optopt == GETOPTV_C_OPERAND)
					{
						continue;
					}
					if (*option == GETOPTV_C_OPERAND)
					{
						continue;
					}
					if (*option == optopt)
					{
						option++;
						if (*option != GETOPTV_C_OPERAND)
						{
							return (optopt);
						}
						if (*string)
						{
							optarg = (char *) (string);
							string = (char *) (0);
							return (optopt);
						}
						if (count < argc)
						{
							optarg = (char *)(argv [count]);
							for (index = count++; index > optind; index--)
							{
								argv [index] = argv [index - 1];
							}
							argv [optind++] = optarg;
							return (optopt);
						}
						if (opterr)
						{
							error (1, 0, "option '%c' needs an operand.", optopt);
						}
						if (** optv == GETOPTV_C_OPERAND)
						{
							return (GETOPTV_C_OPERAND);
						}
						return (GETOPTV_C_ILLEGAL);
					}
				}
				if (opterr)
				{
					error (1, 0, "option '%c' has no meaning.", optopt);
				}
				return (GETOPTV_C_ILLEGAL);
			}
			else
			{
				string = (char *) (0);
			}
		}
		if (count < argc)
		{
			string = argv [count];
			if (*string == GETOPTV_C_OPTION)
			{
				for (index = count; index > optind; index--)
				{
					argv [index] = argv [index - 1];
				}
				argv [optind++] = string++;
				if (*string == GETOPTV_C_VERSION)
				{
					version ();
					exit (0);
				}
				if (*string == GETOPTV_C_SUMMARY)
				{
					putoptv (optv);
					exit (0);
				}
				if (*string == GETOPTV_C_OPTION)
				{
					string++;
					if (!strcmp (string, ""))
					{
						optarg = (char *) (0);
						optopt = (char) (0);
						return (-1);
					}
					if (!strcmp (string, "version"))
					{
						version ();
						exit (0);
					}
					if (!strcmp (string, "help"))
					{
						putoptv (optv);
						exit (0);
					}
					optarg = (char *)(string);
					optopt = GETOPTV_C_OPTION;
					return (-1);
				}
			}
			else
			{
				string = (char *) (0);
			}
			count++;
		}
	}
	optarg = (char *) (0);
	optopt = (char) (0);
	return (-1);
}


#endif


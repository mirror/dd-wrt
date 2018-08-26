/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

static void print (char *symbol, int is_defined)
{
   if (is_defined)
     fprintf (stdout, "%s is defined\n", symbol);
   else
     fprintf (stdout, "%s is NOT defined\n", symbol);
}

int main (int argc, char **argv)
{
   print ("__MSDOS__",
#ifdef __MSDOS__
	  1
#else
	  0
#endif
	  );

   print ("IBMPC_SYSTEM",
#ifdef IBMPC_SYSTEM
	  1
#else
	  0
#endif
	  );

   print ("REAL_UNIX_SYSTEM",
#ifdef REAL_UNIX_SYSTEM
	  1
#else
	  0
#endif
	  );

   print ("__os2__",
#ifdef __os2__
	  1
#else
	  0
#endif
	  );

   print ("__WIN32__",
#ifdef __WIN32__
	  1
#else
	  0
#endif
	  );

   print ("__unix__",
#ifdef __unix__
	  1
#else
	  0
#endif
	  );

   print ("__GO32__",
#ifdef __GO32__
	  1
#else
	  0
#endif
	  );

   print ("__DJGPP__",
#ifdef __DJGPP__
	  1
#else
	  0
#endif
	  );

   print ("__MSDOS_16BIT__",
#ifdef __MSDOS_16BIT__
	  1
#else
	  0
#endif
	  );

   return 0;
}

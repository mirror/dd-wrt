/* This routine illustrates the keypad interface.  To implement
 * detection of a single escape character, allow for timeout.
 */

#include <stdio.h>
#include <slang.h>

#include "demolib.c"

#define TIMEOUT 2		       /* 2/10 of a second */

static int getch (void)
{
   int ch;
   
   while (0 == SLang_input_pending (1000))
     continue;
   
   ch = SLang_getkey ();
 
   if (ch == 033)		       /* escape */
     {
	if (0 == SLang_input_pending (TIMEOUT))
	  return 033;
     }
   
   SLang_ungetkey (ch);
   
   return SLkp_getkey ();
}


int main (int argc, char **argv)
{
   int ch;

   (void) argc; (void) argv;

   if (-1 == demolib_init_terminal (1, 0))
     return 1;

   fprintf (stderr, "This program illustrates the slkeypad facility.\n");
   fprintf (stderr, "Press any key ('q' quits).\n");
   while ('q' != (ch = getch ()))
     {
	fprintf (stderr, "Keysym: %d\r\n", ch);
     }
   
   demolib_exit (0);
   
   return 0;
}

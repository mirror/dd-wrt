#include <stdio.h>
#include <slang.h>
#include <stdlib.h>

static void usage (void)
{
   fprintf (stderr, "Usage: mkmake [DEF1 [DEF2 ...]]\n");
   exit (1);
}

int main (int argc, char **argv)
{
   char buf[1024];
   int i;
   SLprep_Type *pt;

   if (isatty (0))
     usage ();

   if (NULL == (pt = SLprep_new ()))
     return 1;

   if ((-1 == SLprep_set_prefix (pt, "!"))
       || (-1 == SLprep_set_comment (pt, "#", ""))
       || (-1 == SLprep_set_flags (pt, SLPREP_BLANK_LINES_OK | SLPREP_COMMENT_LINES_OK)))
     {
	SLprep_delete (pt);
	return 1;
     }

   for (i = 1; i < argc; i++)
     SLdefine_for_ifdef (argv[i]);

   while (NULL != fgets (buf, sizeof (buf) - 1, stdin))
     {
	if (SLprep_line_ok (buf, pt))
	  {
	     fputs (buf, stdout);
	  }
     }

   SLprep_delete (pt);
   return 0;
}


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <slang.h>

static void issue_instructions (void)
{
   (void) fputs ("Enter any text at the prompts.\n", stdout);
   (void) fputs ("Entering \"quit\" will exit the example\n", stdout);
   (void) fputs ("An EOF (^D) will also exit the example\n", stdout);
   (void) fputs ("\n", stdout);
}

static int example_1 (void)
{
   SLrline_Type *rl;
   unsigned int width = 80;

   issue_instructions ();

   SLang_init_tty (-1, 0, 1);
   rl = SLrline_open (width, SL_RLINE_BLINK_MATCH);

   while (1)
     {
	char *line;
	unsigned int len;

	line = SLrline_read_line (rl, "prompt>", &len);
	if (line == NULL)
	  break;
	if (0 == strcmp (line, "quit"))
	  {
	     SLfree (line);
	     break;
	  }
	(void) fprintf (stdout, "\nRead %d bytes: %s\n", strlen(line), line);
	SLfree (line);
     }
   SLrline_close (rl);
   SLang_reset_tty ();
   return 0;
}

static int example_2 (void)
{
   SLrline_Type *rl;
   unsigned int width = 80;

   issue_instructions ();

   SLang_init_tty (-1, 0, 1);
   SLang_set_abort_signal (NULL);

   rl = SLrline_open (width, SL_RLINE_BLINK_MATCH);
   if (rl == NULL)
     return -1;

   while (1)
     {
	char *line;
	unsigned int len;

	line = SLrline_read_line (rl, "prompt>", &len);
	if (line == NULL)
	  {
	     int err = SLang_get_error ();
	     if (err == SL_UserBreak_Error)
	       {
		  (void) fprintf (stderr, "*Interrupted*\n");
		  SLang_set_error (0);
		  SLKeyBoard_Quit = 0;
		  continue;
	       }
	     if (err == 0)
	       break;		       /* EOF */
	     fprintf (stderr, "Error Occurred: %s\n", SLerr_strerror (err));
	     break;
	  }
	if (0 == strcmp (line, "quit"))
	  {
	     SLfree (line);
	     break;
	  }
	(void) fprintf (stdout, "\nRead %d bytes: %s\n", strlen(line), line);
	SLfree (line);
     }
   SLrline_close (rl);
   SLang_reset_tty ();
   return 0;
}

static int example_3 (void)
{
   SLrline_Type *rl;
   unsigned int width = 80;

   if ((-1 == SLang_init_all ())
       || (-1 == SLang_init_array_extra ())
       || (-1 == SLang_init_import ()))
     return -1;

   (void) SLpath_set_load_path ("../slsh/lib");

   if (-1 == SLrline_init ("demo/rline", NULL, NULL))
     return -1;

   issue_instructions ();

   SLang_init_tty (-1, 0, 1);
   SLang_set_abort_signal (NULL);

   rl = SLrline_open2 ("rline", width, SL_RLINE_BLINK_MATCH);
   if (rl == NULL)
     return -1;

   while (1)
     {
	char *line;
	unsigned int len;

	line = SLrline_read_line (rl, "prompt>", &len);
	if (line == NULL)
	  {
	     int err = SLang_get_error ();
	     if (err == SL_UserBreak_Error)
	       {
		  (void) fprintf (stderr, "*Interrupted*\n");
		  SLang_set_error (0);
		  SLKeyBoard_Quit = 0;
		  continue;
	       }
	     if (err == 0)
	       break;		       /* EOF */
	     fprintf (stderr, "Error Occurred: %s\n", SLerr_strerror (err));
	     break;
	  }
	if (0 == strcmp (line, "quit"))
	  {
	     SLfree (line);
	     break;
	  }
	(void) fprintf (stdout, "\nRead %d bytes: %s\n", strlen(line), line);
	if (-1 == SLrline_save_line (rl))
	  break;

	SLfree (line);
     }
   SLrline_close (rl);
   SLang_reset_tty ();
   return 0;
}

int main (int argc, char **argv)
{
   int n = 1;

   if (argc == 2)
     n = atoi (argv[1]);

   switch (n)
     {
      case 1:
	(void) example_1 ();
	break;
      case 2:
	(void) example_2 ();
	break;
      case 3:
	(void) example_3 ();
	break;

      default:
	(void) fprintf (stderr, "Invalid/Unsupported example number: %d\n", n);
	return 1;
     }
   (void) fprintf (stdout, "\nExample %d done\n", n);
   return 0;
}

/* This program has a simple purpose: strip the documentation from
 * one or more slang .sl files or .c files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct
{
   char *lang;
   char *doc_start_string;
   char *doc_end_string;
   char *doc_prefix_string;
}
Doc_File_Type;

Doc_File_Type Doc_Files [] =
{
     {   "sl",
	"%!%+",
	"%!%-",
	"%"
     },
     {
	"c",
	"/*%+",
	" *%-",
	" *"
     },
     {
	NULL, NULL, NULL, NULL
     }
};

static Doc_File_Type *get_doc_type (char *lang)
{
   Doc_File_Type *dt;

   dt = Doc_Files;

   while (dt->lang != NULL)
     {
	if (0 == strcmp (dt->lang, lang))
	  return dt;

	dt++;
     }
   return NULL;
}

static char *Tm_Comment_String = "#c";

static int doc_strip (char *file, FILE *in, FILE *out, Doc_File_Type *df)
{
   char line[1024];
   char ch_start, ch_end, ch_prefix;
   unsigned int start_len, end_len, prefix_len;
   int level;
   char *start, *end, *prefix;
   unsigned int linenum;

   if (df == NULL)
     return -1;

   start = df->doc_start_string;
   end = df->doc_end_string;
   prefix = df->doc_prefix_string;

   ch_start = *start;
   start_len = strlen (start);
   ch_end = *end;
   end_len = strlen (end);
   ch_prefix = *prefix;
   prefix_len = strlen (prefix);

   linenum = 0;
   level = 0;
   while (NULL != fgets (line, sizeof (line), in))
     {
	linenum++;
	if (level == 0)
	  {
	     if ((*line == ch_start)
		 && (0 == strncmp (line, start, start_len)))
	       {
		  level = 1;
		  fprintf (out, "%s __LINE__: %u\n", Tm_Comment_String, linenum);
	       }
	     continue;
	  }

	if ((*line == ch_end)
	    && (0 == strncmp (line, end, end_len)))
	  {
	     fputs ("\\done\n", out);
	     level = 0;
	     continue;
	  }

	if ((*line == ch_prefix)
	    && (0 == strncmp (line, prefix, prefix_len)))
	  fputs (line + prefix_len, out);
	else
	  {
	     fprintf (stderr, "%s:%u: expecting %s\n", file, linenum, prefix);
	     fputs ("\\done\n", out);
	     return -1;
	     /* fputs (line, out); */
	  }
     }
   return 0;
}

static char *guess_language (char *file)
{
   unsigned int len;
   char *f;

   len = strlen (file);
   f = file + len;

   while ((f > file) && (*f != '.'))
     f--;

   if (*f == '.') f++;
   if (0 == strcmp (f, "c")) return "c";
   if (0 == strcmp (f, "sl")) return "sl";

   return "sl";			       /* default */
}

static void usage (char *pgm)
{
   fprintf (stderr, "Usage: %s [-c] [-sl] file.... > docfile\n", pgm);
}

int main (int argc, char **argv)
{
   FILE *fpin;
   FILE *fpout;
   char *pgm;
   int i;
   char *language;

   pgm = "sl2tm";

   fpout = stdout;

   language = NULL;

   for (i = 1; i < argc; i++)
     {
	if (*argv[i] != '-')
	  break;

	if (0 == strcmp (argv[i], "-c"))
	  language = "c";
	else if (0 == strcmp (argv[i], "-sl"))
	  language = "sl";
	else
	  {
	     usage (pgm);
	     return 1;
	  }
     }

   if (i >= argc)
     {
	if (isatty (0))
	  {
	     usage (pgm);
	     return 1;
	  }

	if (language == NULL) language = "sl";
	(void) doc_strip ("<stdin>", stdin, fpout, get_doc_type (language));
	return 0;
     }

   while (i < argc)
     {
	char *file;
	char *lang;
	Doc_File_Type *dt;

	file = argv[i];
	i++;

	lang = language;

	if ((lang == NULL)
	    && (NULL == (lang = guess_language (file))))
	  lang = "sl";

	if (NULL == (dt = get_doc_type (lang)))
	  continue;

	if (NULL == (fpin = fopen (file, "r")))
	  {
	     fprintf (stderr, "Unable to open %s -- skipping it.\n", file);
	     continue;
	  }

	fprintf (stdout, "%s __FILE__: %s\n", Tm_Comment_String, file);
	(void) doc_strip (file, fpin, fpout, dt);
	fclose (fpin);
     }

   if (fpout != stdout) fclose (fpout);
   return 0;
}

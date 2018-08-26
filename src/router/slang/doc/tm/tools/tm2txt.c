#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/* This program is a quick hack to turn the run-time library .tm files into
 * a decent looking ascii text file.  The currently available SGML-tools are
 * not up to my standards for doing that.
 */

static int Run_Silent;
static int Top_Level;

typedef struct
{
   char *name;
   char *value;
}
Macro_String_Type;

static Macro_String_Type Macro_Strings [] =
{
   {"slang", "S-Lang"},
   {"jed", "jed"},
   {"NULL", "NULL"},
   {"-1", "-1"},
   {"0", "0"},
   {"1", "1"},
   {"2", "2"},
   {"3", "3"},
   {"4", "4"},
   {"5", "5"},
   {"6", "6"},
   {"7", "7"},
   {"8", "8"},
   {"9", "9"},
   {NULL, NULL}
};

typedef struct Section_Type
{
   char *section_name;
   int (*format_fun) (struct Section_Type *);
   unsigned int flags;
}
Section_Type;

static int format_function (Section_Type *);
static int format_synopsis (Section_Type *);
static int format_usage (Section_Type *);
static int format_description (Section_Type *);
static int format_example (Section_Type *);
static int format_notes (Section_Type *);
static int format_see_also (Section_Type *);
static int format_done (Section_Type *);

static Section_Type Sections [] =
{
     {
	"function",
	format_function,
	0
     },
     {
	"synopsis",
	format_synopsis,
	0
     },
     {
	"usage",
	format_usage,
	0
     },
     {
	"description",
	format_description,
	0
     },
     {
	"example",
	format_example,
	0
     },
     {
	"notes",
	format_notes,
	0
     },
     {
	"seealso",
	format_see_also,
	0
     },
     {
	"done",
	format_done,
	0
     },
     {
	"variable",
	format_function,
	0
     },
     {
	"datatype",
	format_function,
	0
     },
     {
	NULL, NULL, 0
     }
};

static FILE *Input_File_Ptr;
static FILE *Output_File_Ptr;

#define MAX_BUF_LEN	1024
static char Input_Buffer [MAX_BUF_LEN];
static unsigned int Line_Number;
static int Input_Buffer_Pushed;
static int Line_Type;
static char *This_Filename;

#define END_OF_FILE 1
#define SECTION_LINE 2
#define VERBATUM_LINE 3

static char Source_File[MAX_BUF_LEN];

static int set_source_file (char *s)
{
   strncpy (Source_File, s, MAX_BUF_LEN);
   Source_File[MAX_BUF_LEN-1] = 0;
   This_Filename = Source_File;
   return 0;
}

static int set_source_linenum (char *s)
{
   unsigned int n;

   if (1 == sscanf (s, "%u", &n))
     Line_Number = n;

   return 0;
}

static int unget_input (char *buf)
{
   if (buf != NULL)
     {
	char *inp = Input_Buffer;
	while (*buf != 0)
	  {
	     *inp++ = *buf++;
	  }
	*inp = 0;
     }
   Input_Buffer_Pushed++;
   return 0;
}

static int begin_verbatum (void);
static int end_verbatum (void);
static int indent (unsigned int);

static int verbatum_mode (void)
{
   begin_verbatum ();
   while (NULL != fgets (Input_Buffer, MAX_BUF_LEN, Input_File_Ptr))
     {
	Line_Number++;

	if (Input_Buffer[0] == '#')
	  {
	     if ((Input_Buffer[1] != 'v')
		 || (Input_Buffer[2] != '-'))
	       {
		  fprintf (stderr, "%s:%u:Expecting verbatum end\n", This_Filename, Line_Number);
		  return -1;
	       }
	     break;
	  }

	indent (3);
	fputs (Input_Buffer, stdout);
     }
   end_verbatum ();
   return 0;
}

static int get_next_line (void)
{
   unsigned int len;

   while (1)
     {
	if (Input_Buffer_Pushed == 0)
	  {
	     if (NULL == fgets (Input_Buffer, MAX_BUF_LEN, Input_File_Ptr))
	       {
		  Line_Type = END_OF_FILE;
		  return -1;
	       }
	     Line_Number++;
	  }

	Input_Buffer_Pushed = 0;
	len = strlen (Input_Buffer);
	if (len && (Input_Buffer[len - 1] == '\n'))
	  Input_Buffer [len - 1] = 0;

	switch (*Input_Buffer)
	  {
	   case ';':
	   case '%':
	     break;

	   case '#':
	     if (Input_Buffer[1] == 'v')
	       {
		  if (Input_Buffer[2] == '+')
		    {
		       if (-1 == verbatum_mode ())
			 return -1;
		    }
		  else
		    {
		       fprintf (stderr, "%s:%u:Expecting verbatum start\n", This_Filename, Line_Number);
		       return -1;
		    }
		  break;
	       }
             if (Input_Buffer[1] == 'c')
	       {
		  if (0 == strncmp (Input_Buffer, "#c __FILE__: ", 13))
		    {
		       set_source_file (Input_Buffer + 13);
		       break;
		    }
		  if (0 == strncmp (Input_Buffer, "#c __LINE__: ", 13))
		    {
		       set_source_linenum (Input_Buffer + 13);
		       break;
		    }
		  break;
	       }

	     break;

	   case '\\':
	     Line_Type = SECTION_LINE;
	     return 1;

	   default:
	     Line_Type = 0;
	     return 0;
	  }
     }
}

static Section_Type *get_section (char *buf)
{
   char *name;
   Section_Type *sec;
   int has_colon;

   if (*buf == '\\') buf++;

   name = buf;
   has_colon = 0;
   while (*buf != 0)
     {
	if ((*buf == '\n')
	    || (*buf == ' ')
	    || (*buf == '\t'))
	  {
	     *buf = 0;
	     break;
	  }
	if (*buf == '{')
	  {
	     has_colon = 1;
	     *buf = 0;
	     break;
	  }
	buf++;
     }

   sec = Sections;
   while (1)
     {
	if (sec->section_name == NULL)
	  {
	     if (Run_Silent == 0)
	       fprintf (stderr, "%s:%u:Unknown section '%s'\n", This_Filename, Line_Number, name);
	     return NULL;
	  }

	if (0 == strcmp (sec->section_name, name))
	  break;

	sec++;
     }

   if (has_colon)
     {
	unget_input (buf + 1);
     }

   return sec;
}

static int process_file (FILE *fp)
{
   Section_Type *sec;

   Input_File_Ptr = fp;
   Output_File_Ptr = stdout;
   Line_Number = 0;
   Top_Level = 1;
   Line_Type = 0;

   while (1)
     {
	while ((Line_Type != SECTION_LINE)
	       && (Line_Type != END_OF_FILE))
	  get_next_line ();

	if (Line_Type == END_OF_FILE)
	  break;

	if (NULL == (sec = get_section (Input_Buffer)))
	  {
	     if (Run_Silent == 0)
	       fprintf (stderr, "%s:%u:Error ignored.\n", This_Filename, Line_Number);
	     get_next_line ();
	     continue;
	  }

	if (sec->format_fun == NULL)
	  {
	     get_next_line ();
	     continue;
	  }

	if (-1 == (*sec->format_fun)(sec))
	  {
	     fprintf (stderr, "%s:%u:Fatal error\n", This_Filename, Line_Number);
	     return -1;
	  }
     }
   return 0;
}

static void usage (void)
{
   char *pgm = "tm2txt";

   fprintf (stderr, "%s usage:\n", pgm);
   fprintf (stderr, "%s [--help] [--quiet] [files...]\n", pgm);
}

int main (int argc, char **argv)
{
   if ((argc > 1)
       && ((0 == strcmp (argv[1], "--help")) || (0 == strcmp (argv[1], "-h"))))
     {
	usage ();
	return 1;
     }

   if ((argc > 1) && (0 == strcmp (argv[1], "--quiet")))
     {
	Run_Silent = 1;
	argc--;
	argv++;
     }

   if ((argc == 1) && isatty (fileno(stdin)))
     {
	usage ();
	return 1;
     }

   if (argc == 1)
     {
	This_Filename = "<stdin>";
	process_file (stdin);
     }
   else
     {
	int i;

	for (i = 1; i < argc; i++)
	  {
	     char *file = argv[i];
	     FILE *fp;

	     if (NULL == (fp = fopen (file, "r")))
	       {
		  fprintf (stderr, "Unable to open %s, skipping it.\n", file);
		  continue;
	       }
	     This_Filename = file;

	     if (-1 == process_file (fp))
	       {
		  fprintf (stderr, "Fatal error encountered processing %s\n",
			   file);
		  fclose (fp);
		  return 1;
	       }

	     fclose (fp);
	  }
     }

   return 0;
}

static int write_boldface (char *s)
{
   fprintf (Output_File_Ptr, "%s", s);
   return 0;
}

#if 0
static int write_tt (char *s)
{
   fprintf (Output_File_Ptr, "`%s'", s);
   return 0;
}
#endif
static int newline (void)
{
   fputs ("\n", Output_File_Ptr);
   return 0;
}

static int write_section_name (char *s)
{
   newline ();
   fputs (" ", Output_File_Ptr);
   write_boldface (s);
   newline ();
   return 0;
}

static char *write_verbatum_output (char *buf)
{
   while (*buf && (*buf != '}'))
     {
	if (*buf == '\\')
	  {
	     buf++;
	     if (*buf == 0)
	       break;
	  }

	putc (*buf, Output_File_Ptr);
	buf++;
     }
   if (*buf == '}') buf++;
   return buf;
}

static char *write_macro (char *s)
{
   char *s1;
   char ch;
   Macro_String_Type *m;

   s1 = s;
   while ((ch = *s1) != 0)
     {
	if ((0 == isalnum (ch))
	    && (ch != '-')
	    && (ch != '_'))
	  break;

	s1++;
     }
   *s1 = 0;

   m = Macro_Strings;
   while (m->name != NULL)
     {
	if (0 == strcmp (m->name, s))
	  {
	     fputs (m->value, Output_File_Ptr);
	     *s1 = ch;
	     return s1;
	  }
	m++;
     }
   fprintf (Output_File_Ptr, "\\%s", s);
   if (Run_Silent == 0)
     fprintf (stderr, "%s:%u:%s not defined\n", This_Filename, Line_Number, s);
   *s1 = ch;
   return s1;
}

static int write_with_escape (char *buf)
{
   char ch;

   while (1)
     {
	ch = *buf++;
	switch (ch)
	  {
	   case 0:
	     return 0;

	   case '\\':
	     if (*buf == '\\')
	       {
		  putc (*buf, Output_File_Ptr);
		  buf++;
		  break;
	       }

	     if ((0 == strncmp ("var{", buf, 4))
		 || (0 == strncmp ("par{", buf, 4))
		 || (0 == strncmp ("fun{", buf, 4)))
	       {
		  putc ('`', Output_File_Ptr);
		  buf = write_verbatum_output (buf + 4);
		  putc ('\'', Output_File_Ptr);
		  break;
	       }

	     if (0 == strncmp ("url{", buf, 4))
	       {
		  putc ('<', Output_File_Ptr);
		  buf = write_verbatum_output (buf + 4);
		  putc ('>', Output_File_Ptr);
		  break;
	       }

	     if ((0 == strncmp ("exmp{", buf, 5))
		 || (0 == strncmp ("ifun{", buf, 5))
		 || (0 == strncmp ("cfun{", buf, 5))
		 || (0 == strncmp ("sfun{", buf, 5))
		 || (0 == strncmp ("ivar{", buf, 5))
		 || (0 == strncmp ("cvar{", buf, 5))
		 || (0 == strncmp ("svar{", buf, 5)))
	       {
		  putc ('`', Output_File_Ptr);
		  buf = write_verbatum_output (buf + 5);
		  putc ('\'', Output_File_Ptr);
		  break;
	       }

	     if (0 == strncmp ("em{", buf, 3))
	       {
		  putc ('_', Output_File_Ptr);
		  buf = write_verbatum_output (buf + 3);
		  putc ('_', Output_File_Ptr);
		  break;
	       }

	     buf = write_macro (buf);
	     break;

	   default:
	     putc (ch, Output_File_Ptr);
	     break;

	  }
     }
}

static int indent (unsigned int n)
{
   while (n)
     {
	putc (' ', Output_File_Ptr);
	n--;
     }

   return 0;
}

static int write_line (void)
{
   char *s = Input_Buffer;
   unsigned int min_indent = 3;

   while ((*s == ' ') && min_indent)
     {
	s++;
	min_indent--;
     }

   indent (min_indent);
   write_with_escape (Input_Buffer);
   newline ();
   return 0;
}

static int format_function (Section_Type *sec)
{
   (void) sec;
   if (Top_Level == 0)
     {
	fprintf (stderr, "%s:%u:\\function or \\variable not at top-level\n",
		 This_Filename, Line_Number);
	fprintf (stderr, "  Input line: %s\n", Input_Buffer);
	return -1;
     }
   Top_Level = 0;

   write_verbatum_output (Input_Buffer);
   newline ();
   get_next_line ();
   return 0;
}

static int format_usage (Section_Type *sec)
{
   (void) sec;
   write_section_name ("USAGE");
   indent (3);
   write_verbatum_output (Input_Buffer);
   newline ();

   get_next_line ();
   return 0;
}

static int format_description (Section_Type *sec)
{
   (void) sec;
   write_section_name ("DESCRIPTION");

   while (0 == get_next_line ())
     {
	write_line ();
     }
   return 0;

}

static int format_example (Section_Type *sec)
{
   (void) sec;
   write_section_name ("EXAMPLE");

   while (0 == get_next_line ())
     {
	write_line ();
     }
   return 0;
}

static int format_notes (Section_Type *sec)
{
   (void) sec;
   write_section_name ("NOTES");

   while (0 == get_next_line ())
     {
	write_line ();
     }
   return 0;
}

static int format_see_also (Section_Type *sec)
{
   (void) sec;
   write_section_name ("SEE ALSO");
   indent (3);
   write_verbatum_output (Input_Buffer);
   newline ();
   get_next_line ();
   return 0;
}

int format_synopsis (Section_Type *sec)
{
   (void) sec;
   write_section_name ("SYNOPSIS");
   indent (3);
   write_verbatum_output (Input_Buffer);
   newline ();
   get_next_line ();
   return 0;
}

int format_done (Section_Type *sec)
{
   (void) sec;
   if (Top_Level)
     {
	fprintf (stderr, "%s:%u:\\done seen at top-level\n",
		 This_Filename, Line_Number);
	return -1;
     }

   fputs ("--------------------------------------------------------------\n",
	  Output_File_Ptr);
   newline ();
   while (0 == get_next_line ())
     ;
   Top_Level = 1;
   return 0;
}

static int begin_verbatum (void)
{
   newline ();
   return 0;
}

static int end_verbatum (void)
{
   newline ();
   return 0;
}

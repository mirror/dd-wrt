#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static char *Make_Intrinsic_Forms [] =
{
   "0",
   "I",
   "S",
   "II",
   "SS",
   "SI",
   "IS",
   "III",
   "IIS",
   "ISI",
   "ISS",
   "SII",
   "SIS",
   "SSI",
   "SSS",
   "IIII",
   "SSSS",
   NULL
};

static char *output_start (FILE *fp)
{
   char **form;

   form = Make_Intrinsic_Forms;

   while (*form != NULL)
     {
	char *r = "0ISD";
	char *f;

	while (*r != 0)
	  {
	     switch (*r)
	       {
		case '0':
		  fprintf (fp, "static void (*V_F");
		  break;

		case 'I':
		  fprintf (fp, "static int (*I_F");
		  break;

		case 'S':
		  fprintf (fp, "static char *(*S_F");
		  break;

		case 'D':
		  fprintf (fp, "static double (*D_F");
		  break;
	       }

	     f = *form;
	     fprintf (fp, "%s)(", f);
	     while (*f != 0)
	       {
		  if (f != *form) fputc (',', fp);
		  switch (*f)
		    {
		     case 'I':
		       fputs ("int*", fp);
		       break;

		     case 'S':
		       fputs ("char*", fp);
		       break;

		     case '0':
		       fputs ("void", fp);
		       break;
		    }
		  f++;
	       }
	     fputs (");\n", fp);

	     r++;
	  }
	form++;
     }

   fputs ("\n", fp);

   fprintf (fp, "static void chkproto_not_used_fun (void)\n{\n");

   return 0;
}

static void output_finish (FILE *fp)
{
   fputs ("\n}\n", fp);
}

static char *skip_whitespace (char *p)
{
   while ((*p == ' ') || (*p == '\t') || (*p == '\n'))
     p++;

   return p;
}

static int do_make_intrinsic (unsigned int linenum, char *p)
{
   char *e;
   char *name;
   int ret_type;

   e = p;
   while (*e && (*e != ' ') && (*e != '\t') && (*e != '('))
     e++;

   if (*e == 0)
     {
	return -1;
     }
   *e++ = 0;

   /* We expect form: IIS("name", function_name, RETURN_TYPE) */
   while (*e && (*e != ',')) e++;
   if (*e == 0)
     {
	return -1;
     }
   e = skip_whitespace (e + 1);
   name = e;
   if (*name == 0)
     {
	return -1;
     }
   while (*e && (*e != ',')) e++;
   if (*e == 0)
     {
	return -1;
     }
   *e = 0;

   e = skip_whitespace (e + 1);
   if (0 == strncmp (e, "SLANG_", 6))
     e += 6;

   if (0 == strncmp (e, "VOID_TYPE", 8))
     ret_type = 'V';
   else if (0 == strncmp (e, "STRING_TYPE", 11))
     ret_type = 'S';
   else if (0 == strncmp (e, "INT_TYPE", 8))
     ret_type = 'I';
   else if (0 == strncmp (e, "DOUBLE_TYPE", 8))
     ret_type = 'D';
   else
     {
	fprintf (stderr, "return type on line %u is not supported\n", linenum);
	return -1;
     }

   fprintf (stdout, "  %c_F%s = %s;\n", ret_type, p, name);
   return 0;
}

int main (int argc, char **argv)
{
   char line [1024];
   FILE *fp;
   unsigned int linenum;

   if (isatty (0)
       || (argc > 1))
     {
	fprintf (stderr, "Usage: %s < infile > outfile\n", argv[0]);
	exit (1);
     }

   output_start (stdout);

   fp = stdin;

   linenum = 0;
   while (NULL != fgets (line, sizeof (line), fp))
     {
	char *p = skip_whitespace (line);

	linenum++;

	if (*p != 'M') continue;

	if (0 != strncmp (p, "MAKE_INTRINSIC", 14))
	  continue;

	if (p[14] != '_')
	  {
	     fprintf (stderr, "Warning: line %u is old-fashioned\n", linenum);
	     continue;
	  }

	p += 15;

	switch (*p)
	  {
	   case 'I':
	   case 'S':
	   case '0':
	     do_make_intrinsic (linenum, p);
	     break;

	   default:
	     fprintf (stderr, "Warning: Unable to handle MAKE_INTRINSIC form on line %u\n",
		      linenum);
	     break;
	  }
     }

   output_finish (stdout);
   return 0;
}


#define SLANG_UNTIC
char *SLang_Untic_Terminfo_File;
#include "sltermin.c"

static void usage (void)
{
   fprintf (stderr, "Usage: untic [[--terminfo filename] | [term]]\n");
   exit (1);
}

int main (int argc, char **argv)
{
   SLterminfo_Type *t;
   Tgetstr_Map_Type *map = Tgetstr_Map;
   unsigned char *str;
   char *term;

   term = getenv ("TERM");
   if (argc > 1)
     {
	if (!strcmp ("--help", argv[1])) usage ();
	if (argc == 2)
	  term = argv[1];
	else if ((argc == 3) && !strcmp(argv[1], "--terminfo"))
	  {
	     SLang_Untic_Terminfo_File = argv[2];
	  }
	else usage ();
     }
   else if (term == NULL) return -1;

   SLtt_Try_Termcap = 0;
   t = _pSLtt_tigetent (term);
   if (t == NULL) return -1;

   puts (t->terminal_names);
   while (*map->name != 0)
     {
	str = (unsigned char *) SLtt_tigetstr (map->name, (char **) &t);
	if (str == NULL)
	  {
	     map++;
	     continue;
	     /* str = (unsigned char *) "NULL"; */
	  }

	fprintf (stdout, "\t%s=", map->name);
	while (*str)
	  {
	     if ((int) (*str & 0x7F) < ' ')
	       {
		  putc ('^', stdout);
		  *str += '@';
	       }
	     putc (*str, stdout);
	     str++;
	  }
	if (map->comment != NULL)
	  fprintf (stdout, "\t\t%s", map->comment);
	putc ('\n', stdout);
	map++;
     }

   map = Tgetflag_Map;
   while (*map->name != 0)
     {
	if (_pSLtt_tigetflag (t, map->name) > 0)
	  {
	     fprintf (stdout, "\t%s\t\t%s\n",
		      map->name,
		      ((map->comment == NULL) ? "" : map->comment));
	  }
	map++;
     }
   map = Tgetnum_Map;
   while (*map->name != 0)
     {
	int val;
	if ((val = SLtt_tigetnum (map->name, (char **) &t)) >= 0)
	  {
	     fprintf (stdout, "\t%s#%d\t\t%s\n",
		      map->name, val,
		      ((map->comment == NULL) ? "" : map->comment));
	  }
	map++;
     }

   return 0;
}


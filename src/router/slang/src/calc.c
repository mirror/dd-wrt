/* This is a simple demo program for the S-Lang interpreter. */

#include "config.h"
#include <stdio.h>
#include <math.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#define BENCHMARK_TESTING	0

#if BENCHMARK_TESTING
# include <sys/time.h>
# include <sys/resource.h>
# include <unistd.h>
#endif

#include "slang.h"

static int open_readline (void);
static void close_readline (void);

static void help (void)
{
   puts("ALL statements MUST be terminated with a ';' character, e.g., quit();\n");
   puts("Available functions:");
   puts("  cos, sin, tan, atan, acos, asin, exp, log, sqrt, fabs, log10, pow, PI, E");
   puts("\nas well as other intrinsic S-Lang functions.");
   puts("See S-Lang language documentation for further details.\n");
   SLang_run_hooks ("calc_help", 0);
}

/* The following three functions will be callable from the interpreter */
static void quit_calc (void)
{
   close_readline ();
   exit (1);
}

static void exit_calc (int *status)
{
   close_readline ();
   exit (*status);
}

/* Now here is a table that provides the link between the above functions and
   the S-Lang interpreter */
static SLang_Intrin_Fun_Type Calc_Intrinsics [] =
{
   MAKE_INTRINSIC("quit", quit_calc, SLANG_VOID_TYPE, 0),
   MAKE_INTRINSIC_I("exit", exit_calc, SLANG_VOID_TYPE),
   MAKE_INTRINSIC("help", help, SLANG_VOID_TYPE, 0),
   SLANG_END_INTRIN_FUN_TABLE
};

typedef struct
{
   int i_value;
   char *s_value;
   double d_value;
}
My_Struct_Type;

static My_Struct_Type My_Struct =
{
   -41,
   NULL,
   7.18
};

static My_Struct_Type *My_Struct_Ptr = &My_Struct;

static SLang_IStruct_Field_Type My_Struct_Field_Table [] =
{
   MAKE_ISTRUCT_FIELD(My_Struct_Type, i_value, "i", SLANG_INT_TYPE, 0),
   MAKE_ISTRUCT_FIELD(My_Struct_Type, s_value, "s", SLANG_STRING_TYPE, 0),
   MAKE_ISTRUCT_FIELD(My_Struct_Type, d_value, "d", SLANG_DOUBLE_TYPE, 0),
   SLANG_END_ISTRUCT_TABLE
};

static int add_my_struct_type (void)
{
   return SLadd_istruct_table (My_Struct_Field_Table,
			       (VOID_STAR) &My_Struct_Ptr,
			       "MyS");
}

static int take_input (void);

int main (int argc, char **argv)
{
   if ((-1 == SLang_init_all ())
       /* || (-1 == SLang_init_import ()) / * dynamic linking */
       || (-1 == add_my_struct_type ())
       || (-1 == SLadd_intrin_fun_table (Calc_Intrinsics, NULL)))
     {
	fprintf(stderr, "Unable to initialize S-Lang.\n");
	exit (1);
     }

   SLang_Traceback = 1;

   if (argc == 1)
     SLang_load_file("calc.sl");

   while (--argc && !SLang_get_error ())
     {
	argv++;
	SLang_load_file (*argv);
     }

   fputs("Type 'help();' for help and a list of available functions.\n", stdout);
   fputs("All statements must end with a ';' character, e.g, 2*7+3;\n", stdout);
   fputs("\nType `quit;' to exit this program.\n", stdout);

   if (-1 == open_readline ())
     return 1;

   while (1)
     {
	if (SLang_get_error ())
	  {
	     SLang_restart (1);
	     /* SLang_set_error (0); */
	  }
	SLKeyBoard_Quit = 0;
	take_input ();
     }
}

/* For a detailed explanation of all of this, see slang/demo/useropen.c */

static SLrline_Type *Calc_RLI;
SLang_Load_Type *Readline_Load_Object;

static char *read_using_readline (SLang_Load_Type *x)
{
   static char *input_hook = "calc_take_input_hook";
   char *prompt, *line;

   if (x->parse_level == 0)
     {
	if ((input_hook != NULL)
	    && (-1 == SLang_run_hooks (input_hook, 0)))
	  {
	     input_hook = NULL;
	     return NULL;
	  }
	prompt = "Calc> ";
     }
   else
     prompt = "      ";

   if (NULL == (line = SLrline_read_line (Calc_RLI, prompt, NULL)))
     return SLmake_string ("quit();");

   putc ('\n', stdout);  fflush (stdout);

   SLrline_save_line (Calc_RLI);
   return line;
}

static int open_readline (void)
{
   if (SLang_init_tty (-1, 0, 1))
     {
	fprintf(stderr, "Unable to initialize tty.\n");
	return -1;
     }
   SLang_set_abort_signal (NULL);

   if (Calc_RLI == NULL)
     {
	Calc_RLI = SLrline_open (80, SL_RLINE_BLINK_MATCH);
	if (Calc_RLI == NULL)
	  return -1;
     }

   if (NULL == (Readline_Load_Object = SLallocate_load_type ("<stdin>")))
     {
	close_readline ();
	return -1;
     }

   Readline_Load_Object->read = read_using_readline;
   return 0;
}

static void close_readline (void)
{
   if (Readline_Load_Object != NULL)
     {
	SLdeallocate_load_type (Readline_Load_Object);
	Readline_Load_Object = NULL;
     }
   SLrline_close (Calc_RLI);
   Calc_RLI = NULL;
   SLang_reset_tty ();
}

static int take_input (void)
{
   return SLang_load_object (Readline_Load_Object);
}

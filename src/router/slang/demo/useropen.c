/* This demo indicates how to read and parse a S-Lang file by bypassing the
 * built-in routines.
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <slang.h>

/* Suppose that you want to read input using a read line package
 * such as one provided by S-Lang.  For generality, lets assume that this
 * function is called 'readline' and it is prototyped as:
 *   int readline (char *prompt, char *buffer);
 * where it returns the number of characters read and -1 if end of file.  The
 * first parameter is a prompt and the second represents the buffer where the
 * characters are to placed.  Also assume that this routine requires that the
 * function 'init_readline' be called first before it can be used and
 * 'reset_readline' must be called after using it.
 *
 * The goal here is to get S-Lang to call the readline function.
 */

/* For the purposes of this demo, we will use just fgets */
#define MAX_BUF_LEN	256
static int readline (char *prompt, char *buf)
{
   fputs (prompt, stdout);  fflush (stdout);
   if (NULL == fgets (buf, MAX_BUF_LEN, stdin)) return -1;
   return (int) strlen (buf);
}

static int init_readline (void)
{
   puts ("Initializing readline."); fflush (stdout);
   return 0;
}

static void reset_readline (void)
{
   puts ("Resetting readline."); fflush (stdout);
}

/* Now lets define the function that S-Lang will use to actually read the data.
 * It calls readline.  S-Lang will call this function and the function must
 * return a pointer to the buffer containg the characters of the line or NULL
 * upon end of file.  In many ways, it is like fgets except that it is passed
 * a pointer to SLang_Load_Type in stead of FILE.
 */

typedef struct
{
   char buf[MAX_BUF_LEN];
   char *prompt;
}
Our_Client_Data_Type;

static char *read_using_readline (SLang_Load_Type *x)
{
   Our_Client_Data_Type *client_data;

   client_data = (Our_Client_Data_Type *) x->client_data;
   if (-1 == readline (client_data->prompt, client_data->buf))
     return NULL;

   return client_data->buf;
}

/* Now, we all of this is tied together in this routine which will be called
 * from main below.
 */

static int read_input (void)
{
   SLang_Load_Type *x;
   Our_Client_Data_Type client_data;

   if (NULL == (x = SLallocate_load_type ("<readline>")))
     return -1;

   client_data.prompt = "Demo> ";

   x->client_data = (VOID_STAR) &client_data;
   x->read = read_using_readline; /* function to call to perform the read */
   SLang_load_object (x);
   return 0;
}

/* Now here is are some intrinsic functions */

int main (int argc, char **argv)
{
   /* usual stuff */

   (void) argc; (void) argv;

   if ((-1 == SLang_init_slang ())    /* basic interpreter functions */
       || (-1 == SLang_init_slmath ()) 	       /* sin, cos, etc... */
#ifdef unix
       || (-1 == SLang_init_slunix ())	       /* unix system calls */
#endif
       || (-1 == SLang_init_slfile ()))	       /* file i/o */
     {
	fprintf(stderr, "Unable to initialize S-Lang.\n");
	return 1;
     }

   init_readline ();

   read_input ();

   reset_readline ();
   return SLang_get_error ();
}


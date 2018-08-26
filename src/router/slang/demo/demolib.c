/* These routines are used by several of the demos. */
#include "config.h"
#include <stdio.h>
#include <signal.h>

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <slang.h>

static void demolib_exit (int sig)
{
   SLang_reset_tty ();
   SLsmg_reset_smg ();

   if (sig)
     {
	fprintf (stderr, "Exiting on signal %d\n", sig);
	exit (1);
     }
   exit (sig);
}

#ifdef SIGTSTP
static void sigtstp_handler (int sig)
{
   demolib_exit (sig);
}
#endif

#ifdef SIGINT
static void sigint_handler (int sig)
{
   demolib_exit (sig);
}
#endif

static void init_signals (void)
{
#ifdef SIGTSTP
   SLsignal (SIGTSTP, sigtstp_handler);
#endif
#ifdef SIGINT
   SLsignal (SIGINT, sigint_handler);
#endif
}

static void exit_error_hook (char *fmt, va_list ap)
{
   SLang_reset_tty ();
   SLsmg_reset_smg ();

   vfprintf (stderr, fmt, ap);
   fputc ('\n', stderr);
   exit (1);
}

static int demolib_init_terminal (int tty, int smg)
{
   SLang_Exit_Error_Hook = exit_error_hook;

   /* It is wise to block the occurrence of display  related signals while we are
    * initializing.
    */
   SLsig_block_signals ();

   SLtt_get_terminfo ();

   /* SLkp_init assumes that SLtt_get_terminfo has been called. */
   if (tty && (-1 == SLkp_init ()))
     {
	SLsig_unblock_signals ();
	return -1;
     }

   init_signals ();

   if (tty) SLang_init_tty (-1, 0, 1);
#ifdef REAL_UNIX_SYSTEM
   if (tty) SLtty_set_suspend_state (1);
#endif
   if (smg)
     {
	(void) SLutf8_enable (-1);

	if (-1 == SLsmg_init_smg ())
	  {
	     SLsig_unblock_signals ();
	     return -1;
	  }
     }

   SLsig_unblock_signals ();

   return 0;
}


/* This is simply a process that segfaults */
#include <config.h>
#include <stdlib.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#include "disable-crash-handling.h"

int
main (int argc, char **argv)
{
  _dbus_disable_crash_handling ();

#ifdef HAVE_RAISE
  raise (SIGSEGV);
#else
  {
    volatile char *p = NULL;
    *p = 'a';
  }
#endif

  return 0;
}

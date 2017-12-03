#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/errno.h>
#include "pppoe.h"

#define control_exit() (void) 0

extern  void
printErr(char const *str);

/**********************************************************************
*%FUNCTION: fatalSys
*%ARGUMENTS:
* str -- error message
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Prints a message plus the errno value to stderr and syslog and exits.
***********************************************************************/
void
fatalSys(char const *str)
{
    char buf[SMALLBUF];
    snprintf(buf, SMALLBUF, "%s: %s", str, strerror(errno));
    printErr(buf);
    control_exit();
    exit(EXIT_FAILURE);
}

/**********************************************************************
*%FUNCTION: sysErr
*%ARGUMENTS:
* str -- error message
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Prints a message plus the errno value to syslog.
***********************************************************************/
void
sysErr(char const *str)
{
    char buf[1024];
    sprintf(buf, "%.256s: %.256s", str, strerror(errno));
    printErr(buf);
}

/**********************************************************************
*%FUNCTION: rp_fatal
*%ARGUMENTS:
* str -- error message
*%RETURNS:
* Nothing
*%DESCRIPTION:
* Prints a message to stderr and syslog and exits.
***********************************************************************/
void
rp_fatal(char const *str)
{
    printErr(str);
    control_exit();
    exit(EXIT_FAILURE);
}

#ifdef HAVE_PPPOERELAY
extern int pppoerelay_main(int argc, char *argv[]);
#endif
#ifdef HAVE_PPPOESERVER
extern int pppoeserver_main(int argc, char *argv[]);
#endif


int main(int argc,char *argv[])
{
  char *base = strrchr (argv[0], '/');      
  base = base ? base + 1 : argv[0];
#ifdef HAVE_PPPOERELAY
  if (strstr (base, "pppoe-relay"))
    return pppoerelay_main(argc,argv);
#endif
#ifdef HAVE_PPPOESERVER
  if (strstr (base, "pppoe-server"))
    return pppoeserver_main(argc,argv);
#endif
return -1;
}


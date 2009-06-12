#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef _POSIX_SOURCE
#include <signal.h>
#endif

#if defined(HAVE_SYSLOG)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/syslog.h>
#ifndef _PATH_LOG
#define _PATH_LOG "/dev/log"
#endif
#endif

long __guard[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void __guard_setup (void)
{
  int fd;
  if (__guard[0]!=0) return;
  fd = open ("/dev/urandom", 0);
  if (fd != -1) {
    ssize_t size = read (fd, (char*)&__guard, sizeof(__guard));
    close (fd) ;
    if (size == sizeof(__guard)) return;
  }
  /* If a random generator can't be used, the protector switches the guard
     to the "terminator canary" */
  ((char*)__guard)[0] = 0; ((char*)__guard)[1] = 0;
  ((char*)__guard)[2] = '\n'; ((char*)__guard)[3] = 255;
}

void __stack_smash_handler (char func[], int damaged)
{
#if defined (__GNU_LIBRARY__)
  extern char * __progname;
#endif
  const char message[] = ": stack smashing attack in function ";
  int bufsz = 512, len;
  char buf[bufsz];
#if defined(HAVE_SYSLOG)
  int LogFile;
  struct sockaddr_un SyslogAddr;  /* AF_UNIX address of local logger */
#endif
#ifdef _POSIX_SOURCE
  {
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGABRT);  /* Block all signal handlers */
    sigprocmask(SIG_BLOCK, &mask, NULL); /* except SIGABRT */
  }
#endif

  strcpy(buf, "<2>"); len=3;    /* send LOG_CRIT */
#if defined (__GNU_LIBRARY__)
  strncat(buf, __progname, bufsz-len-1); len = strlen(buf);
#endif
  if (bufsz>len) {strncat(buf, message, bufsz-len-1); len = strlen(buf);}
  if (bufsz>len) {strncat(buf, func, bufsz-len-1); len = strlen(buf);}
  /* print error message */
  write (STDERR_FILENO, buf+3, len-3);
#if defined(HAVE_SYSLOG)
  if ((LogFile = socket(AF_UNIX, SOCK_DGRAM, 0)) != -1) {
                                                                                                                     
    /*                                                                                                               
     * Send "found" message to the "/dev/log" path
     */
    SyslogAddr.sun_family = AF_UNIX;
    (void)strncpy(SyslogAddr.sun_path, _PATH_LOG,
          sizeof(SyslogAddr.sun_path) - 1);
    SyslogAddr.sun_path[sizeof(SyslogAddr.sun_path) - 1] = '\0';
    sendto(LogFile, buf, len, 0, (struct sockaddr *)&SyslogAddr,
       sizeof(SyslogAddr));
  }
#endif

#ifdef _POSIX_SOURCE
  { /* Make sure the default handler is associated with SIGABRT */
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigfillset(&sa.sa_mask);    /* Block all signals */
    sa.sa_flags = 0;
    sa.sa_handler = SIG_DFL;
    sigaction(SIGABRT, &sa, NULL);
    (void)kill(getpid(), SIGABRT);
  }
#endif
  _exit(127);
}


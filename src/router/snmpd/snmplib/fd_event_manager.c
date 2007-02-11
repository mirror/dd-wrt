/* UNIT: File Descriptor (FD) Event Manager                              */
#include <net-snmp/net-snmp-config.h>
#ifdef HAVE_SYS_SELECT
#include <sys/select.h>
#endif
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/fd_event_manager.h>
#include <net-snmp/library/snmp_logging.h>
int     external_readfd[NUM_EXTERNAL_FDS],   external_readfdlen   = 0;
int     external_writefd[NUM_EXTERNAL_FDS],  external_writefdlen  = 0;
int     external_exceptfd[NUM_EXTERNAL_FDS], external_exceptfdlen = 0;
void  (*external_readfdfunc[NUM_EXTERNAL_FDS]) (int, void *);
void  (*external_writefdfunc[NUM_EXTERNAL_FDS]) (int, void *);
void  (*external_exceptfdfunc[NUM_EXTERNAL_FDS]) (int, void *);
void   *external_readfd_data[NUM_EXTERNAL_FDS];
void   *external_writefd_data[NUM_EXTERNAL_FDS];
void   *external_exceptfd_data[NUM_EXTERNAL_FDS];

static int external_fd_unregistered;

/*
 * Register a given fd for read events.  Call callback when events
 * are received.
 */
int
register_readfd(int fd, void (*func) (int, void *), void *data)
{
    if (external_readfdlen < NUM_EXTERNAL_FDS) {
        external_readfd[external_readfdlen] = fd;
        external_readfdfunc[external_readfdlen] = func;
        external_readfd_data[external_readfdlen] = data;
        external_readfdlen++;
        DEBUGMSGTL(("fd_event_manager:register_readfd", "registered fd %d\n", fd));
        return FD_REGISTERED_OK;
    } else {
        snmp_log(LOG_CRIT, "register_readfd: too many file descriptors\n");
        return FD_REGISTRATION_FAILED;
    }
}

/*
 * Register a given fd for write events.  Call callback when events
 * are received.
 */
int
register_writefd(int fd, void (*func) (int, void *), void *data)
{
    if (external_writefdlen < NUM_EXTERNAL_FDS) {
        external_writefd[external_writefdlen] = fd;
        external_writefdfunc[external_writefdlen] = func;
        external_writefd_data[external_writefdlen] = data;
        external_writefdlen++;
        DEBUGMSGTL(("fd_event_manager:register_writefd", "registered fd %d\n", fd));
        return FD_REGISTERED_OK;
    } else {
        snmp_log(LOG_CRIT,
                 "register_writefd: too many file descriptors\n");
        return FD_REGISTRATION_FAILED;
    }
}

/*
 * Register a given fd for exception events.  Call callback when events
 * are received.
 */
int
register_exceptfd(int fd, void (*func) (int, void *), void *data)
{
    if (external_exceptfdlen < NUM_EXTERNAL_FDS) {
        external_exceptfd[external_exceptfdlen] = fd;
        external_exceptfdfunc[external_exceptfdlen] = func;
        external_exceptfd_data[external_exceptfdlen] = data;
        external_exceptfdlen++;
        DEBUGMSGTL(("fd_event_manager:register_exceptfd", "registered fd %d\n", fd));
        return FD_REGISTERED_OK;
    } else {
        snmp_log(LOG_CRIT,
                 "register_exceptfd: too many file descriptors\n");
        return FD_REGISTRATION_FAILED;
    }
}

/*
 * Unregister a given fd for read events.
 */ 
int
unregister_readfd(int fd)
{
    int             i, j;

    for (i = 0; i < external_readfdlen; i++) {
        if (external_readfd[i] == fd) {
            external_readfdlen--;
            for (j = i; j < external_readfdlen; j++) {
                external_readfd[j] = external_readfd[j + 1];
                external_readfdfunc[j] = external_readfdfunc[j + 1];
                external_readfd_data[j] = external_readfd_data[j + 1];
            }
            DEBUGMSGTL(("fd_event_manager:unregister_readfd", "unregistered fd %d\n", fd));
            external_fd_unregistered = 1;
            return FD_UNREGISTERED_OK;
        }
    }
    return FD_NO_SUCH_REGISTRATION;
}

/*
 * Unregister a given fd for read events.
 */ 
int
unregister_writefd(int fd)
{
    int             i, j;

    for (i = 0; i < external_writefdlen; i++) {
        if (external_writefd[i] == fd) {
            external_writefdlen--;
            for (j = i; j < external_writefdlen; j++) {
                external_writefd[j] = external_writefd[j + 1];
                external_writefdfunc[j] = external_writefdfunc[j + 1];
                external_writefd_data[j] = external_writefd_data[j + 1];
            }
            DEBUGMSGTL(("fd_event_manager:unregister_writefd", "unregistered fd %d\n", fd));
            external_fd_unregistered = 1;
            return FD_UNREGISTERED_OK;
        }
    }
    return FD_NO_SUCH_REGISTRATION;
}

/*
 * Unregister a given fd for exception events.
 */
int
unregister_exceptfd(int fd)
{
    int             i, j;

    for (i = 0; i < external_exceptfdlen; i++) {
        if (external_exceptfd[i] == fd) {
            external_exceptfdlen--;
            for (j = i; j < external_exceptfdlen; j++) {
                external_exceptfd[j] = external_exceptfd[j + 1];
                external_exceptfdfunc[j] = external_exceptfdfunc[j + 1];
                external_exceptfd_data[j] = external_exceptfd_data[j + 1];
            }
            DEBUGMSGTL(("fd_event_manager:unregister_exceptfd", "unregistered fd %d\n",
                        fd));
            external_fd_unregistered = 1;
            return FD_UNREGISTERED_OK;
        }
    }
    return FD_NO_SUCH_REGISTRATION;
}

/* 
 * NET-SNMP External Event Info 
 */
void netsnmp_external_event_info(int *numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds)
{
  int i;

  external_fd_unregistered = 0;

  for (i = 0; i < external_readfdlen; i++) {
    FD_SET(external_readfd[i], readfds);
    if (external_readfd[i] >= *numfds)
      *numfds = external_readfd[i] + 1;
  }
  for (i = 0; i < external_writefdlen; i++) {
    FD_SET(external_writefd[i], writefds);
    if (external_writefd[i] >= *numfds)
      *numfds = external_writefd[i] + 1;
  }
  for (i = 0; i < external_exceptfdlen; i++) {
    FD_SET(external_exceptfd[i], exceptfds);
    if (external_exceptfd[i] >= *numfds)
      *numfds = external_exceptfd[i] + 1;
  }
}

/* 
 * NET-SNMP Dispatch External Events 
 */
void netsnmp_dispatch_external_events(int *count, fd_set *readfds, fd_set *writefds, fd_set *exceptfds)
{
  int i;
  for (i = 0;
       *count && (i < external_readfdlen) && !external_fd_unregistered; i++) {
      if (FD_ISSET(external_readfd[i], readfds)) {
          DEBUGMSGTL(("fd_event_manager:netsnmp_dispatch_external_events", 
                     "readfd[%d] = %d\n", i, external_readfd[i]));
          external_readfdfunc[i] (external_readfd[i],
                                  external_readfd_data[i]);
          FD_CLR(external_readfd[i], readfds);
          *count--;
      }
  }
  for (i = 0;
       *count && (i < external_writefdlen) && !external_fd_unregistered; i++) {
      if (FD_ISSET(external_writefd[i], writefds)) {
          DEBUGMSGTL(("fd_event_manager:netsnmp_dispatch_external_events", 
                     "writefd[%d] = %d\n", i, external_writefd[i]));
          external_writefdfunc[i] (external_writefd[i],
                                   external_writefd_data[i]);
          FD_CLR(external_writefd[i], writefds);
          *count--;
      }
  }
  for (i = 0;
       *count && (i < external_exceptfdlen) && !external_fd_unregistered; i++) {
      if (FD_ISSET(external_exceptfd[i], exceptfds)) {
          DEBUGMSGTL(("fd_event_manager:netsnmp_dispatch_external_events", 
                     "exceptfd[%d] = %d\n", i, external_exceptfd[i]));
          external_exceptfdfunc[i] (external_exceptfd[i],
                                    external_exceptfd_data[i]);
          FD_CLR(external_exceptfd[i], exceptfds);
          *count--;
      }
  }
}

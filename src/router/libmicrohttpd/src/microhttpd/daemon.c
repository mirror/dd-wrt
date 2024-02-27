/*
  This file is part of libmicrohttpd
  Copyright (C) 2007-2018 Daniel Pittman and Christian Grothoff
  Copyright (C) 2015-2024 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

/**
 * @file microhttpd/daemon.c
 * @brief  A minimal-HTTP server library
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */
#include "platform.h"
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#include "mhd_threads.h"
#endif
#include "internal.h"
#include "response.h"
#include "connection.h"
#include "memorypool.h"
#include "mhd_limits.h"
#include "autoinit_funcs.h"
#include "mhd_mono_clock.h"
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#include "mhd_locks.h"
#endif
#include "mhd_sockets.h"
#include "mhd_itc.h"
#include "mhd_compat.h"
#include "mhd_send.h"
#include "mhd_align.h"
#include "mhd_str.h"

#ifdef MHD_USE_SYS_TSEARCH
#include <search.h>
#else  /* ! MHD_USE_SYS_TSEARCH */
#include "tsearch.h"
#endif /* ! MHD_USE_SYS_TSEARCH */

#ifdef HTTPS_SUPPORT
#include "connection_https.h"
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
#endif /* HTTPS_SUPPORT */

#if defined(_WIN32) && ! defined(__CYGWIN__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#ifdef MHD_USE_POSIX_THREADS
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */
#endif /* MHD_USE_POSIX_THREADS */

/**
 * Default connection limit.
 */
#ifdef MHD_POSIX_SOCKETS
#define MHD_MAX_CONNECTIONS_DEFAULT (FD_SETSIZE - 3 - 1 - MHD_ITC_NUM_FDS_)
#else
#define MHD_MAX_CONNECTIONS_DEFAULT (FD_SETSIZE - 2)
#endif

/**
 * Default memory allowed per connection.
 */
#define MHD_POOL_SIZE_DEFAULT (32 * 1024)


/* Forward declarations. */


/**
 * Global initialisation function.
 */
void
MHD_init (void);

/**
 * Global deinitialisation function.
 */
void
MHD_fini (void);

/**
 * Close all connections for the daemon.
 * Must only be called when MHD_Daemon::shutdown was set to true.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon daemon to close down
 */
static void
close_all_connections (struct MHD_Daemon *daemon);

#ifdef EPOLL_SUPPORT

/**
 * Do epoll()-based processing.
 *
 * @param daemon daemon to run poll loop for
 * @param millisec the maximum time in milliseconds to wait for events,
 *                 set to '0' for non-blocking processing,
 *                 set to '-1' to wait indefinitely.
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static enum MHD_Result
MHD_epoll (struct MHD_Daemon *daemon,
           int32_t millisec);

#endif /* EPOLL_SUPPORT */

#ifdef _AUTOINIT_FUNCS_ARE_SUPPORTED
/**
 * Do nothing - global initialisation is
 * performed by library constructor.
 */
#define MHD_check_global_init_() (void) 0
#else  /* ! _AUTOINIT_FUNCS_ARE_SUPPORTED */
/**
 * Track global initialisation
 */
volatile int global_init_count = 0;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#ifdef MHD_MUTEX_STATIC_DEFN_INIT_
/**
 * Global initialisation mutex
 */
MHD_MUTEX_STATIC_DEFN_INIT_ (global_init_mutex_);
#endif /* MHD_MUTEX_STATIC_DEFN_INIT_ */
#endif


/**
 * Check whether global initialisation was performed
 * and call initialiser if necessary.
 */
void
MHD_check_global_init_ (void)
{
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#ifdef MHD_MUTEX_STATIC_DEFN_INIT_
  MHD_mutex_lock_chk_ (&global_init_mutex_);
#endif /* MHD_MUTEX_STATIC_DEFN_INIT_ */
#endif
  if (0 == global_init_count++)
    MHD_init ();
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#ifdef MHD_MUTEX_STATIC_DEFN_INIT_
  MHD_mutex_unlock_chk_ (&global_init_mutex_);
#endif /* MHD_MUTEX_STATIC_DEFN_INIT_ */
#endif
}


#endif /* ! _AUTOINIT_FUNCS_ARE_SUPPORTED */

#ifdef HAVE_MESSAGES
/**
 * Default logger function
 */
static void
MHD_default_logger_ (void *cls,
                     const char *fm,
                     va_list ap)
{
  vfprintf ((FILE *) cls, fm, ap);
#ifdef _DEBUG
  fflush ((FILE *) cls);
#endif /* _DEBUG */
}


#endif /* HAVE_MESSAGES */


/**
 * Free the memory allocated by MHD.
 *
 * If any MHD function explicitly mentions that returned pointer must be
 * freed by this function, then no other method must be used to free
 * the memory.
 *
 * @param ptr the pointer to free.
 * @sa #MHD_digest_auth_get_username(), #MHD_basic_auth_get_username_password3()
 * @sa #MHD_basic_auth_get_username_password()
 * @note Available since #MHD_VERSION 0x00095600
 * @ingroup specialized
 */
_MHD_EXTERN void
MHD_free (void *ptr)
{
  free (ptr);
}


/**
 * Maintain connection count for single address.
 */
struct MHD_IPCount
{
  /**
   * Address family. AF_INET or AF_INET6 for now.
   */
  int family;

  /**
   * Actual address.
   */
  union
  {
    /**
     * IPv4 address.
     */
    struct in_addr ipv4;
#ifdef HAVE_INET6
    /**
     * IPv6 address.
     */
    struct in6_addr ipv6;
#endif
  } addr;

  /**
   * Counter.
   */
  unsigned int count;
};


/**
 * Lock shared structure for IP connection counts and connection DLLs.
 *
 * @param daemon handle to daemon where lock is
 */
static void
MHD_ip_count_lock (struct MHD_Daemon *daemon)
{
  mhd_assert (NULL == daemon->master);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->per_ip_connection_mutex);
#else
  (void) daemon;
#endif
}


/**
 * Unlock shared structure for IP connection counts and connection DLLs.
 *
 * @param daemon handle to daemon where lock is
 */
static void
MHD_ip_count_unlock (struct MHD_Daemon *daemon)
{
  mhd_assert (NULL == daemon->master);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->per_ip_connection_mutex);
#else
  (void) daemon;
#endif
}


/**
 * Tree comparison function for IP addresses (supplied to tsearch() family).
 * We compare everything in the struct up through the beginning of the
 * 'count' field.
 *
 * @param a1 first address to compare
 * @param a2 second address to compare
 * @return -1, 0 or 1 depending on result of compare
 */
static int
MHD_ip_addr_compare (const void *a1,
                     const void *a2)
{
  return memcmp (a1,
                 a2,
                 offsetof (struct MHD_IPCount,
                           count));
}


/**
 * Parse address and initialize @a key using the address.
 *
 * @param addr address to parse
 * @param addrlen number of bytes in @a addr
 * @param key where to store the parsed address
 * @return #MHD_YES on success and #MHD_NO otherwise (e.g., invalid address type)
 */
static enum MHD_Result
MHD_ip_addr_to_key (const struct sockaddr_storage *addr,
                    socklen_t addrlen,
                    struct MHD_IPCount *key)
{
  memset (key,
          0,
          sizeof(*key));

  /* IPv4 addresses */
  if (sizeof (struct sockaddr_in) <= (size_t) addrlen)
  {
    if (AF_INET == addr->ss_family)
    {
      key->family = AF_INET;
      memcpy (&key->addr.ipv4,
              &((const struct sockaddr_in *) addr)->sin_addr,
              sizeof(((const struct sockaddr_in *) NULL)->sin_addr));
      return MHD_YES;
    }
  }

#ifdef HAVE_INET6
  if (sizeof (struct sockaddr_in6) <= (size_t) addrlen)
  {
    /* IPv6 addresses */
    if (AF_INET6 == addr->ss_family)
    {
      key->family = AF_INET6;
      memcpy (&key->addr.ipv6,
              &((const struct sockaddr_in6 *) addr)->sin6_addr,
              sizeof(((const struct sockaddr_in6 *) NULL)->sin6_addr));
      return MHD_YES;
    }
  }
#endif

  /* Some other address */
  return MHD_NO;
}


/**
 * Check if IP address is over its limit in terms of the number
 * of allowed concurrent connections.  If the IP is still allowed,
 * increments the connection counter.
 *
 * @param daemon handle to daemon where connection counts are tracked
 * @param addr address to add (or increment counter)
 * @param addrlen number of bytes in @a addr
 * @return Return #MHD_YES if IP below limit, #MHD_NO if IP has surpassed limit.
 *   Also returns #MHD_NO if fails to allocate memory.
 */
static enum MHD_Result
MHD_ip_limit_add (struct MHD_Daemon *daemon,
                  const struct sockaddr_storage *addr,
                  socklen_t addrlen)
{
  struct MHD_IPCount *newkeyp;
  struct MHD_IPCount *keyp;
  struct MHD_IPCount **nodep;
  enum MHD_Result result;

  daemon = MHD_get_master (daemon);
  /* Ignore if no connection limit assigned */
  if (0 == daemon->per_ip_connection_limit)
    return MHD_YES;

  newkeyp = (struct MHD_IPCount *) malloc (sizeof(struct MHD_IPCount));
  if (NULL == newkeyp)
    return MHD_NO;

  /* Initialize key */
  if (MHD_NO == MHD_ip_addr_to_key (addr,
                                    addrlen,
                                    newkeyp))
  {
    free (newkeyp);
    return MHD_YES; /* Allow unhandled address types through */
  }

  MHD_ip_count_lock (daemon);

  /* Search for the IP address */
  nodep = (struct MHD_IPCount **) tsearch (newkeyp,
                                           &daemon->per_ip_connection_count,
                                           &MHD_ip_addr_compare);
  if (NULL == nodep)
  {
    MHD_ip_count_unlock (daemon);
    free (newkeyp);
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to add IP connection count node.\n"));
#endif
    return MHD_NO;
  }
  keyp = *nodep;
  /* Test if there is room for another connection; if so,
   * increment count */
  result = (keyp->count < daemon->per_ip_connection_limit) ? MHD_YES : MHD_NO;
  if (MHD_NO != result)
    ++keyp->count;
  MHD_ip_count_unlock (daemon);

  /* If we got an existing node back, free the one we created */
  if (keyp != newkeyp)
    free (newkeyp);

  return result;
}


/**
 * Decrement connection count for IP address, removing from table
 * count reaches 0.
 *
 * @param daemon handle to daemon where connection counts are tracked
 * @param addr address to remove (or decrement counter)
 * @param addrlen number of bytes in @a addr
 */
static void
MHD_ip_limit_del (struct MHD_Daemon *daemon,
                  const struct sockaddr_storage *addr,
                  socklen_t addrlen)
{
  struct MHD_IPCount search_key;
  struct MHD_IPCount *found_key;
  void **nodep;

  daemon = MHD_get_master (daemon);
  /* Ignore if no connection limit assigned */
  if (0 == daemon->per_ip_connection_limit)
    return;
  /* Initialize search key */
  if (MHD_NO == MHD_ip_addr_to_key (addr,
                                    addrlen,
                                    &search_key))
    return;

  MHD_ip_count_lock (daemon);

  /* Search for the IP address */
  if (NULL == (nodep = tfind (&search_key,
                              &daemon->per_ip_connection_count,
                              &MHD_ip_addr_compare)))
  {
    /* Something's wrong if we couldn't find an IP address
     * that was previously added */
    MHD_PANIC (_ ("Failed to find previously-added IP address.\n"));
  }
  found_key = (struct MHD_IPCount *) *nodep;
  /* Validate existing count for IP address */
  if (0 == found_key->count)
  {
    MHD_PANIC (_ ("Previously-added IP address had counter of zero.\n"));
  }
  /* Remove the node entirely if count reduces to 0 */
  if (0 == --found_key->count)
  {
    tdelete (found_key,
             &daemon->per_ip_connection_count,
             &MHD_ip_addr_compare);
    MHD_ip_count_unlock (daemon);
    free (found_key);
  }
  else
    MHD_ip_count_unlock (daemon);
}


#ifdef HTTPS_SUPPORT
/**
 * Read and setup our certificate and key.
 *
 * @param daemon handle to daemon to initialize
 * @return 0 on success
 */
static int
MHD_init_daemon_certificate (struct MHD_Daemon *daemon)
{
  gnutls_datum_t key;
  gnutls_datum_t cert;
  int ret;

#if GNUTLS_VERSION_MAJOR >= 3
  if (NULL != daemon->cert_callback)
  {
    gnutls_certificate_set_retrieve_function2 (daemon->x509_cred,
                                               daemon->cert_callback);
  }
#endif
#if GNUTLS_VERSION_NUMBER >= 0x030603
  else if (NULL != daemon->cert_callback2)
  {
    gnutls_certificate_set_retrieve_function3 (daemon->x509_cred,
                                               daemon->cert_callback2);
  }
#endif

  if (NULL != daemon->https_mem_trust)
  {
    size_t paramlen;
    paramlen = strlen (daemon->https_mem_trust);
    if (UINT_MAX < paramlen)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Too long trust certificate.\n"));
#endif
      return -1;
    }
    cert.data = (unsigned char *) _MHD_DROP_CONST (daemon->https_mem_trust);
    cert.size = (unsigned int) paramlen;
    if (gnutls_certificate_set_x509_trust_mem (daemon->x509_cred,
                                               &cert,
                                               GNUTLS_X509_FMT_PEM) < 0)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Bad trust certificate format.\n"));
#endif
      return -1;
    }
  }

  if (daemon->have_dhparams)
  {
    gnutls_certificate_set_dh_params (daemon->x509_cred,
                                      daemon->https_mem_dhparams);
  }
  /* certificate & key loaded from memory */
  if ( (NULL != daemon->https_mem_cert) &&
       (NULL != daemon->https_mem_key) )
  {
    size_t param1len;
    size_t param2len;

    param1len = strlen (daemon->https_mem_key);
    param2len = strlen (daemon->https_mem_cert);
    if ( (UINT_MAX < param1len) ||
         (UINT_MAX < param2len) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Too long key or certificate.\n"));
#endif
      return -1;
    }
    key.data = (unsigned char *) _MHD_DROP_CONST (daemon->https_mem_key);
    key.size = (unsigned int) param1len;
    cert.data = (unsigned char *) _MHD_DROP_CONST (daemon->https_mem_cert);
    cert.size = (unsigned int) param2len;

    if (NULL != daemon->https_key_password)
    {
#if GNUTLS_VERSION_NUMBER >= 0x030111
      ret = gnutls_certificate_set_x509_key_mem2 (daemon->x509_cred,
                                                  &cert,
                                                  &key,
                                                  GNUTLS_X509_FMT_PEM,
                                                  daemon->https_key_password,
                                                  0);
#else
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to setup x509 certificate/key: pre 3.X.X version " \
                   "of GnuTLS does not support setting key password.\n"));
#endif
      return -1;
#endif
    }
    else
      ret = gnutls_certificate_set_x509_key_mem (daemon->x509_cred,
                                                 &cert,
                                                 &key,
                                                 GNUTLS_X509_FMT_PEM);
#ifdef HAVE_MESSAGES
    if (0 != ret)
      MHD_DLOG (daemon,
                _ ("GnuTLS failed to setup x509 certificate/key: %s\n"),
                gnutls_strerror (ret));
#endif
    return ret;
  }
#if GNUTLS_VERSION_MAJOR >= 3
  if (NULL != daemon->cert_callback)
    return 0;
#endif
#if GNUTLS_VERSION_NUMBER >= 0x030603
  else if (NULL != daemon->cert_callback2)
    return 0;
#endif
#ifdef HAVE_MESSAGES
  MHD_DLOG (daemon,
            _ ("You need to specify a certificate and key location.\n"));
#endif
  return -1;
}


/**
 * Initialize security aspects of the HTTPS daemon
 *
 * @param daemon handle to daemon to initialize
 * @return 0 on success
 */
static int
MHD_TLS_init (struct MHD_Daemon *daemon)
{
  switch (daemon->cred_type)
  {
  case GNUTLS_CRD_CERTIFICATE:
    if (0 !=
        gnutls_certificate_allocate_credentials (&daemon->x509_cred))
      return GNUTLS_E_MEMORY_ERROR;
    return MHD_init_daemon_certificate (daemon);
  case GNUTLS_CRD_PSK:
    if (0 !=
        gnutls_psk_allocate_server_credentials (&daemon->psk_cred))
      return GNUTLS_E_MEMORY_ERROR;
    return 0;
  case GNUTLS_CRD_ANON:
  case GNUTLS_CRD_SRP:
  case GNUTLS_CRD_IA:
  default:
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Error: invalid credentials type %d specified.\n"),
              daemon->cred_type);
#endif
    return -1;
  }
}


#endif /* HTTPS_SUPPORT */


#undef MHD_get_fdset

/**
 * Obtain the `select()` sets for this daemon.
 * Daemon's FDs will be added to fd_sets. To get only
 * daemon FDs in fd_sets, call FD_ZERO for each fd_set
 * before calling this function. FD_SETSIZE is assumed
 * to be platform's default.
 *
 * This function should be called only when MHD is configured to
 * use "external" sockets polling with 'select()' or with 'epoll'.
 * In the latter case, it will only add the single 'epoll' file
 * descriptor used by MHD to the sets.
 * It's necessary to use #MHD_get_timeout() to get maximum timeout
 * value for `select()`. Usage of `select()` with indefinite timeout
 * (or timeout larger than returned by #MHD_get_timeout()) will
 * violate MHD API and may results in pending unprocessed data.
 *
 * This function must be called only for daemon started
 * without #MHD_USE_INTERNAL_POLLING_THREAD flag.
 *
 * @param daemon daemon to get sets from
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @param max_fd increased to largest FD added (if larger
 *               than existing value); can be NULL
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call or any FD didn't
 *         fit fd_set.
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_get_fdset (struct MHD_Daemon *daemon,
               fd_set *read_fd_set,
               fd_set *write_fd_set,
               fd_set *except_fd_set,
               MHD_socket *max_fd)
{
  return MHD_get_fdset2 (daemon,
                         read_fd_set,
                         write_fd_set,
                         except_fd_set,
                         max_fd,
#ifdef HAS_FD_SETSIZE_OVERRIDABLE
                         daemon->fdset_size_set_by_app ?
                         ((unsigned int) daemon->fdset_size) :
                         ((unsigned int) _MHD_SYS_DEFAULT_FD_SETSIZE)
#else  /* ! HAS_FD_SETSIZE_OVERRIDABLE */
                         ((unsigned int) _MHD_SYS_DEFAULT_FD_SETSIZE)
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */
                         );
}


#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
/**
 * Obtain the select() file descriptor sets for the
 * given @a urh.
 *
 * @param urh upgrade handle to wait for
 * @param[out] rs read set to initialize
 * @param[out] ws write set to initialize
 * @param[out] es except set to initialize
 * @param[out] max_fd maximum FD to update
 * @param fd_setsize value of FD_SETSIZE
 * @return true on success, false on error
 */
static bool
urh_to_fdset (struct MHD_UpgradeResponseHandle *urh,
              fd_set *rs,
              fd_set *ws,
              fd_set *es,
              MHD_socket *max_fd,
              int fd_setsize)
{
  const MHD_socket conn_sckt = urh->connection->socket_fd;
  const MHD_socket mhd_sckt = urh->mhd.socket;
  bool res = true;

#ifndef HAS_FD_SETSIZE_OVERRIDABLE
  (void) fd_setsize;  /* Mute compiler warning */
  fd_setsize = (int) FD_SETSIZE; /* Help compiler to optimise */
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

  /* Do not add to 'es' only if socket is closed
   * or not used anymore. */
  if (MHD_INVALID_SOCKET != conn_sckt)
  {
    if ( (urh->in_buffer_used < urh->in_buffer_size) &&
         (! MHD_add_to_fd_set_ (conn_sckt,
                                rs,
                                max_fd,
                                fd_setsize)) )
      res = false;
    if ( (0 != urh->out_buffer_used) &&
         (! MHD_add_to_fd_set_ (conn_sckt,
                                ws,
                                max_fd,
                                fd_setsize)) )
      res = false;
    /* Do not monitor again for errors if error was detected before as
     * error state is remembered. */
    if ((0 == (urh->app.celi & MHD_EPOLL_STATE_ERROR)) &&
        ((0 != urh->in_buffer_size) ||
         (0 != urh->out_buffer_size) ||
         (0 != urh->out_buffer_used))
        && (NULL != es))
      (void) MHD_add_to_fd_set_ (conn_sckt,
                                 es,
                                 max_fd,
                                 fd_setsize);
  }
  if (MHD_INVALID_SOCKET != mhd_sckt)
  {
    if ( (urh->out_buffer_used < urh->out_buffer_size) &&
         (! MHD_add_to_fd_set_ (mhd_sckt,
                                rs,
                                max_fd,
                                fd_setsize)) )
      res = false;
    if ( (0 != urh->in_buffer_used) &&
         (! MHD_add_to_fd_set_ (mhd_sckt,
                                ws,
                                max_fd,
                                fd_setsize)) )
      res = false;
    /* Do not monitor again for errors if error was detected before as
     * error state is remembered. */
    if ((0 == (urh->mhd.celi & MHD_EPOLL_STATE_ERROR)) &&
        ((0 != urh->out_buffer_size) ||
         (0 != urh->in_buffer_size) ||
         (0 != urh->in_buffer_used))
        && (NULL != es))
      MHD_add_to_fd_set_ (mhd_sckt,
                          es,
                          max_fd,
                          fd_setsize);
  }

  return res;
}


/**
 * Update the @a urh based on the ready FDs in
 * the @a rs, @a ws, and @a es.
 *
 * @param urh upgrade handle to update
 * @param rs read result from select()
 * @param ws write result from select()
 * @param es except result from select()
 * @param fd_setsize value of FD_SETSIZE used when fd_sets were created
 */
static void
urh_from_fdset (struct MHD_UpgradeResponseHandle *urh,
                const fd_set *rs,
                const fd_set *ws,
                const fd_set *es,
                int fd_setsize)
{
  const MHD_socket conn_sckt = urh->connection->socket_fd;
  const MHD_socket mhd_sckt = urh->mhd.socket;

  /* Reset read/write ready, preserve error state. */
  urh->app.celi &= (~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY)
                    & ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY));
  urh->mhd.celi &= (~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY)
                    & ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY));

  mhd_assert (urh->connection->sk_nonblck);

#ifndef HAS_FD_SETSIZE_OVERRIDABLE
  (void) fd_setsize; /* Mute compiler warning */
  mhd_assert (((int) FD_SETSIZE) <= fd_setsize);
  fd_setsize = FD_SETSIZE; /* Help compiler to optimise */
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

  if (MHD_INVALID_SOCKET != conn_sckt)
  {
    if (MHD_SCKT_FD_FITS_FDSET_SETSIZE_ (conn_sckt, NULL, fd_setsize))
    {
      if (FD_ISSET (conn_sckt, (fd_set *) _MHD_DROP_CONST (rs)))
        urh->app.celi |= MHD_EPOLL_STATE_READ_READY;
      if (FD_ISSET (conn_sckt, (fd_set *) _MHD_DROP_CONST (ws)))
        urh->app.celi |= MHD_EPOLL_STATE_WRITE_READY;
      if ((NULL != es) &&
          FD_ISSET (conn_sckt, (fd_set *) _MHD_DROP_CONST (es)))
        urh->app.celi |= MHD_EPOLL_STATE_ERROR;
    }
    else
    { /* Cannot check readiness. Force ready state is safe as socket is non-blocking */
      urh->app.celi |= MHD_EPOLL_STATE_READ_READY;
      urh->app.celi |= MHD_EPOLL_STATE_WRITE_READY;
    }
  }
  if ((MHD_INVALID_SOCKET != mhd_sckt))
  {
    if (MHD_SCKT_FD_FITS_FDSET_SETSIZE_ (mhd_sckt, NULL, fd_setsize))
    {
      if (FD_ISSET (mhd_sckt, (fd_set *) _MHD_DROP_CONST (rs)))
        urh->mhd.celi |= MHD_EPOLL_STATE_READ_READY;
      if (FD_ISSET (mhd_sckt, (fd_set *) _MHD_DROP_CONST (ws)))
        urh->mhd.celi |= MHD_EPOLL_STATE_WRITE_READY;
      if ((NULL != es) &&
          FD_ISSET (mhd_sckt, (fd_set *) _MHD_DROP_CONST (es)))
        urh->mhd.celi |= MHD_EPOLL_STATE_ERROR;
    }
    else
    { /* Cannot check readiness. Force ready state is safe as socket is non-blocking */
      urh->mhd.celi |= MHD_EPOLL_STATE_READ_READY;
      urh->mhd.celi |= MHD_EPOLL_STATE_WRITE_READY;
    }
  }
}


#ifdef HAVE_POLL

/**
 * Set required 'event' members in 'pollfd' elements,
 * assuming that @a p[0].fd is MHD side of socketpair
 * and @a p[1].fd is TLS connected socket.
 *
 * @param urh upgrade handle to watch for
 * @param p pollfd array to update
 */
static void
urh_update_pollfd (struct MHD_UpgradeResponseHandle *urh,
                   struct pollfd p[2])
{
  p[0].events = 0;
  p[1].events = 0;

  if (urh->in_buffer_used < urh->in_buffer_size)
    p[0].events |= POLLIN;
  if (0 != urh->out_buffer_used)
    p[0].events |= POLLOUT;

  /* Do not monitor again for errors if error was detected before as
   * error state is remembered. */
  if ((0 == (urh->app.celi & MHD_EPOLL_STATE_ERROR)) &&
      ((0 != urh->in_buffer_size) ||
       (0 != urh->out_buffer_size) ||
       (0 != urh->out_buffer_used)))
    p[0].events |= MHD_POLL_EVENTS_ERR_DISC;

  if (urh->out_buffer_used < urh->out_buffer_size)
    p[1].events |= POLLIN;
  if (0 != urh->in_buffer_used)
    p[1].events |= POLLOUT;

  /* Do not monitor again for errors if error was detected before as
   * error state is remembered. */
  if ((0 == (urh->mhd.celi & MHD_EPOLL_STATE_ERROR)) &&
      ((0 != urh->out_buffer_size) ||
       (0 != urh->in_buffer_size) ||
       (0 != urh->in_buffer_used)))
    p[1].events |= MHD_POLL_EVENTS_ERR_DISC;
}


/**
 * Set @a p to watch for @a urh.
 *
 * @param urh upgrade handle to watch for
 * @param p pollfd array to set
 */
static void
urh_to_pollfd (struct MHD_UpgradeResponseHandle *urh,
               struct pollfd p[2])
{
  p[0].fd = urh->connection->socket_fd;
  p[1].fd = urh->mhd.socket;
  urh_update_pollfd (urh,
                     p);
}


/**
 * Update ready state in @a urh based on pollfd.
 * @param urh upgrade handle to update
 * @param p 'poll()' processed pollfd.
 */
static void
urh_from_pollfd (struct MHD_UpgradeResponseHandle *urh,
                 struct pollfd p[2])
{
  /* Reset read/write ready, preserve error state. */
  urh->app.celi &= (~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY)
                    & ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY));
  urh->mhd.celi &= (~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY)
                    & ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY));

  if (0 != (p[0].revents & POLLIN))
    urh->app.celi |= MHD_EPOLL_STATE_READ_READY;
  if (0 != (p[0].revents & POLLOUT))
    urh->app.celi |= MHD_EPOLL_STATE_WRITE_READY;
  if (0 != (p[0].revents & POLLHUP))
    urh->app.celi |= MHD_EPOLL_STATE_READ_READY | MHD_EPOLL_STATE_WRITE_READY;
  if (0 != (p[0].revents & MHD_POLL_REVENTS_ERRROR))
    urh->app.celi |= MHD_EPOLL_STATE_ERROR;
  if (0 != (p[1].revents & POLLIN))
    urh->mhd.celi |= MHD_EPOLL_STATE_READ_READY;
  if (0 != (p[1].revents & POLLOUT))
    urh->mhd.celi |= MHD_EPOLL_STATE_WRITE_READY;
  if (0 != (p[1].revents & POLLHUP))
    urh->mhd.celi |= MHD_EPOLL_STATE_ERROR;
  if (0 != (p[1].revents & MHD_POLL_REVENTS_ERRROR))
    urh->mhd.celi |= MHD_EPOLL_STATE_READ_READY | MHD_EPOLL_STATE_WRITE_READY;
}


#endif /* HAVE_POLL */
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */


/**
 * Internal version of #MHD_get_fdset2().
 *
 * @param daemon daemon to get sets from
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @param max_fd increased to largest FD added (if larger
 *               than existing value); can be NULL
 * @param fd_setsize value of FD_SETSIZE
 * @return #MHD_YES on success, #MHD_NO if any FD didn't
 *         fit fd_set.
 * @ingroup event
 */
static enum MHD_Result
internal_get_fdset2 (struct MHD_Daemon *daemon,
                     fd_set *read_fd_set,
                     fd_set *write_fd_set,
                     fd_set *except_fd_set,
                     MHD_socket *max_fd,
                     int fd_setsize)
{
  struct MHD_Connection *pos;
  struct MHD_Connection *posn;
  enum MHD_Result result = MHD_YES;
  MHD_socket ls;
  bool itc_added;

#ifndef HAS_FD_SETSIZE_OVERRIDABLE
  (void) fd_setsize;  /* Mute compiler warning */
  fd_setsize = (int) FD_SETSIZE; /* Help compiler to optimise */
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

  if (daemon->shutdown)
    return MHD_YES;

  /* The order of FDs added is important for W32 sockets as W32 fd_set has
     limits for number of added FDs instead of the limit for the higher
     FD value. */

  /* Add ITC FD first. The daemon must be able to respond on application
     commands issued in other threads. */
  itc_added = false;
  if (MHD_ITC_IS_VALID_ (daemon->itc))
  {
    itc_added = MHD_add_to_fd_set_ (MHD_itc_r_fd_ (daemon->itc),
                                    read_fd_set,
                                    max_fd,
                                    fd_setsize);
    if (! itc_added)
      result = MHD_NO;
  }

  ls = daemon->was_quiesced ? MHD_INVALID_SOCKET : daemon->listen_fd;
  if (! itc_added &&
      (MHD_INVALID_SOCKET != ls))
  {
    /* Add listen FD if ITC was not added. Listen FD could be used to signal
       the daemon shutdown. */
    if (MHD_add_to_fd_set_ (ls,
                            read_fd_set,
                            max_fd,
                            fd_setsize))
      ls = MHD_INVALID_SOCKET;   /* Already added */
    else
      result = MHD_NO;
  }

  /* Add all sockets to 'except_fd_set' as well to watch for
   * out-of-band data. However, ignore errors if INFO_READ
   * or INFO_WRITE sockets will not fit 'except_fd_set'. */
  /* Start from oldest connections. Make sense for W32 FDSETs. */
  for (pos = daemon->connections_tail; NULL != pos; pos = posn)
  {
    posn = pos->prev;

    switch (pos->event_loop_info)
    {
    case MHD_EVENT_LOOP_INFO_READ:
    case MHD_EVENT_LOOP_INFO_PROCESS_READ:
      if (! MHD_add_to_fd_set_ (pos->socket_fd,
                                read_fd_set,
                                max_fd,
                                fd_setsize))
        result = MHD_NO;
#ifdef MHD_POSIX_SOCKETS
      if (NULL != except_fd_set)
        (void) MHD_add_to_fd_set_ (pos->socket_fd,
                                   except_fd_set,
                                   max_fd,
                                   fd_setsize);
#endif /* MHD_POSIX_SOCKETS */
      break;
    case MHD_EVENT_LOOP_INFO_WRITE:
      if (! MHD_add_to_fd_set_ (pos->socket_fd,
                                write_fd_set,
                                max_fd,
                                fd_setsize))
        result = MHD_NO;
#ifdef MHD_POSIX_SOCKETS
      if (NULL != except_fd_set)
        (void) MHD_add_to_fd_set_ (pos->socket_fd,
                                   except_fd_set,
                                   max_fd,
                                   fd_setsize);
#endif /* MHD_POSIX_SOCKETS */
      break;
    case MHD_EVENT_LOOP_INFO_PROCESS:
      if ( (NULL == except_fd_set) ||
           ! MHD_add_to_fd_set_ (pos->socket_fd,
                                 except_fd_set,
                                 max_fd,
                                 fd_setsize))
        result = MHD_NO;
      break;
    case MHD_EVENT_LOOP_INFO_CLEANUP:
      /* this should never happen */
      break;
    }
  }
#ifdef MHD_WINSOCK_SOCKETS
  /* W32 use limited array for fd_set so add INFO_READ/INFO_WRITE sockets
   * only after INFO_BLOCK sockets to ensure that INFO_BLOCK sockets will
   * not be pushed out. */
  if (NULL != except_fd_set)
  {
    for (pos = daemon->connections_tail; NULL != pos; pos = posn)
    {
      posn = pos->prev;
      MHD_add_to_fd_set_ (pos->socket_fd,
                          except_fd_set,
                          max_fd,
                          fd_setsize);
    }
  }
#endif /* MHD_WINSOCK_SOCKETS */
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  if (1)
  {
    struct MHD_UpgradeResponseHandle *urh;

    for (urh = daemon->urh_tail; NULL != urh; urh = urh->prev)
    {
      if (MHD_NO ==
          urh_to_fdset (urh,
                        read_fd_set,
                        write_fd_set,
                        except_fd_set,
                        max_fd,
                        fd_setsize))
        result = MHD_NO;
    }
  }
#endif

  if (MHD_INVALID_SOCKET != ls)
  {
    /* The listen socket is present and hasn't been added */
    if ((daemon->connections < daemon->connection_limit) &&
        ! daemon->at_limit)
    {
      if (! MHD_add_to_fd_set_ (ls,
                                read_fd_set,
                                max_fd,
                                fd_setsize))
        result = MHD_NO;
    }
  }

#if _MHD_DEBUG_CONNECT
#ifdef HAVE_MESSAGES
  if (NULL != max_fd)
    MHD_DLOG (daemon,
              _ ("Maximum socket in select set: %d\n"),
              *max_fd);
#endif
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  return result;
}


/**
 * Obtain the `select()` sets for this daemon.
 * Daemon's FDs will be added to fd_sets. To get only
 * daemon FDs in fd_sets, call FD_ZERO for each fd_set
 * before calling this function.
 *
 * Passing custom FD_SETSIZE as @a fd_setsize allow usage of
 * larger/smaller than platform's default fd_sets.
 *
 * This function should be called only when MHD is configured to
 * use "external" sockets polling with 'select()' or with 'epoll'.
 * In the latter case, it will only add the single 'epoll' file
 * descriptor used by MHD to the sets.
 * It's necessary to use #MHD_get_timeout() to get maximum timeout
 * value for `select()`. Usage of `select()` with indefinite timeout
 * (or timeout larger than returned by #MHD_get_timeout()) will
 * violate MHD API and may results in pending unprocessed data.
 *
 * This function must be called only for daemon started
 * without #MHD_USE_INTERNAL_POLLING_THREAD flag.
 *
 * @param daemon daemon to get sets from
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @param max_fd increased to largest FD added (if larger
 *               than existing value); can be NULL
 * @param fd_setsize value of FD_SETSIZE
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call or any FD didn't
 *         fit fd_set.
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_get_fdset2 (struct MHD_Daemon *daemon,
                fd_set *read_fd_set,
                fd_set *write_fd_set,
                fd_set *except_fd_set,
                MHD_socket *max_fd,
                unsigned int fd_setsize)
{
  if ( (NULL == daemon) ||
       (NULL == read_fd_set) ||
       (NULL == write_fd_set) ||
       MHD_D_IS_USING_THREADS_ (daemon) ||
       MHD_D_IS_USING_POLL_ (daemon))
    return MHD_NO;

#ifdef HAVE_MESSAGES
  if (NULL == except_fd_set)
  {
    MHD_DLOG (daemon,
              _ ("MHD_get_fdset2() called with except_fd_set "
                 "set to NULL. Such behavior is unsupported.\n"));
  }
#endif

#ifdef HAS_FD_SETSIZE_OVERRIDABLE
  if (0 == fd_setsize)
    return MHD_NO;
  else if (((unsigned int) INT_MAX) < fd_setsize)
    fd_setsize = (unsigned int) INT_MAX;
#ifdef HAVE_MESSAGES
  else if (daemon->fdset_size > ((int) fd_setsize))
  {
    if (daemon->fdset_size_set_by_app)
    {
      MHD_DLOG (daemon,
                _ ("%s() called with fd_setsize (%u) " \
                   "less than value set by MHD_OPTION_APP_FD_SETSIZE (%d). " \
                   "Some socket FDs may be not processed. " \
                   "Use MHD_OPTION_APP_FD_SETSIZE with the correct value.\n"),
                "MHD_get_fdset2", fd_setsize, daemon->fdset_size);
    }
    else
    {
      MHD_DLOG (daemon,
                _ ("%s() called with fd_setsize (%u) " \
                   "less than FD_SETSIZE used by MHD (%d). " \
                   "Some socket FDs may be not processed. " \
                   "Consider using MHD_OPTION_APP_FD_SETSIZE option.\n"),
                "MHD_get_fdset2", fd_setsize, daemon->fdset_size);
    }
  }
#endif /* HAVE_MESSAGES */
#else  /* ! HAS_FD_SETSIZE_OVERRIDABLE */
  if (((unsigned int) FD_SETSIZE) > fd_setsize)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("%s() called with fd_setsize (%u) " \
                 "less than fixed FD_SETSIZE value (%d) used on the " \
                 "platform.\n"),
              "MHD_get_fdset2", fd_setsize, (int) FD_SETSIZE);
#endif /* HAVE_MESSAGES */
    return MHD_NO;
  }
  fd_setsize = (int) FD_SETSIZE; /* Help compiler to optimise */
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon))
  {
    if (daemon->shutdown)
      return MHD_YES;

    /* we're in epoll mode, use the epoll FD as a stand-in for
       the entire event set */

    return MHD_add_to_fd_set_ (daemon->epoll_fd,
                               read_fd_set,
                               max_fd,
                               (int) fd_setsize) ? MHD_YES : MHD_NO;
  }
#endif

  return internal_get_fdset2 (daemon,
                              read_fd_set,
                              write_fd_set,
                              except_fd_set,
                              max_fd,
                              (int) fd_setsize);
}


/**
 * Call the handlers for a connection in the appropriate order based
 * on the readiness as detected by the event loop.
 *
 * @param con connection to handle
 * @param read_ready set if the socket is ready for reading
 * @param write_ready set if the socket is ready for writing
 * @param force_close set if a hard error was detected on the socket;
 *        if this information is not available, simply pass #MHD_NO
 * @return #MHD_YES to continue normally,
 *         #MHD_NO if a serious error was encountered and the
 *         connection is to be closed.
 */
static enum MHD_Result
call_handlers (struct MHD_Connection *con,
               bool read_ready,
               bool write_ready,
               bool force_close)
{
  enum MHD_Result ret;
  bool states_info_processed = false;
  /* Fast track flag */
  bool on_fasttrack = (con->state == MHD_CONNECTION_INIT);
  ret = MHD_YES;

  mhd_assert ((0 == (con->daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_valid_ID_ (con->tid)));
  mhd_assert ((0 != (con->daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (! MHD_thread_handle_ID_is_valid_ID_ (con->tid)));
  mhd_assert ((0 == (con->daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_current_thread_ (con->tid)));

#ifdef HTTPS_SUPPORT
  if (con->tls_read_ready)
    read_ready = true;
#endif /* HTTPS_SUPPORT */
  if ( (0 != (MHD_EVENT_LOOP_INFO_READ & con->event_loop_info)) &&
       (read_ready || (force_close && con->sk_nonblck)) )
  {
    MHD_connection_handle_read (con, force_close);
    mhd_assert (! force_close || MHD_CONNECTION_CLOSED == con->state);
    ret = MHD_connection_handle_idle (con);
    if (force_close)
      return ret;
    states_info_processed = true;
  }
  if (! force_close)
  {
    /* No need to check value of 'ret' here as closed connection
     * cannot be in MHD_EVENT_LOOP_INFO_WRITE state. */
    if ( (MHD_EVENT_LOOP_INFO_WRITE == con->event_loop_info) &&
         write_ready)
    {
      MHD_connection_handle_write (con);
      ret = MHD_connection_handle_idle (con);
      states_info_processed = true;
    }
  }
  else
  {
    MHD_connection_close_ (con,
                           MHD_REQUEST_TERMINATED_WITH_ERROR);
    return MHD_connection_handle_idle (con);
  }

  if (! states_info_processed)
  {   /* Connection is not read or write ready, but external conditions
       * may be changed and need to be processed. */
    ret = MHD_connection_handle_idle (con);
  }
  /* Fast track for fast connections. */
  /* If full request was read by single read_handler() invocation
     and headers were completely prepared by single MHD_connection_handle_idle()
     then try not to wait for next sockets polling and send response
     immediately.
     As writeability of socket was not checked and it may have
     some data pending in system buffers, use this optimization
     only for non-blocking sockets. */
  /* No need to check 'ret' as connection is always in
   * MHD_CONNECTION_CLOSED state if 'ret' is equal 'MHD_NO'. */
  else if (on_fasttrack && con->sk_nonblck)
  {
    if (MHD_CONNECTION_HEADERS_SENDING == con->state)
    {
      MHD_connection_handle_write (con);
      /* Always call 'MHD_connection_handle_idle()' after each read/write. */
      ret = MHD_connection_handle_idle (con);
    }
    /* If all headers were sent by single write_handler() and
     * response body is prepared by single MHD_connection_handle_idle()
     * call - continue. */
    if ((MHD_CONNECTION_NORMAL_BODY_READY == con->state) ||
        (MHD_CONNECTION_CHUNKED_BODY_READY == con->state))
    {
      MHD_connection_handle_write (con);
      ret = MHD_connection_handle_idle (con);
    }
  }

  /* All connection's data and states are processed for this turn.
   * If connection already has more data to be processed - use
   * zero timeout for next select()/poll(). */
  /* Thread-per-connection do not need global zero timeout as
   * connections are processed individually. */
  /* Note: no need to check for read buffer availability for
   * TLS read-ready connection in 'read info' state as connection
   * without space in read buffer will be marked as 'info block'. */
  if ( (! con->daemon->data_already_pending) &&
       (! MHD_D_IS_USING_THREAD_PER_CONN_ (con->daemon)) )
  {
    if (0 != (MHD_EVENT_LOOP_INFO_PROCESS & con->event_loop_info))
      con->daemon->data_already_pending = true;
#ifdef HTTPS_SUPPORT
    else if ( (con->tls_read_ready) &&
              (0 != (MHD_EVENT_LOOP_INFO_READ & con->event_loop_info)) )
      con->daemon->data_already_pending = true;
#endif /* HTTPS_SUPPORT */
  }
  return ret;
}


#ifdef UPGRADE_SUPPORT
/**
 * Finally cleanup upgrade-related resources. It should
 * be called when TLS buffers have been drained and
 * application signaled MHD by #MHD_UPGRADE_ACTION_CLOSE.
 *
 * @param connection handle to the upgraded connection to clean
 */
static void
cleanup_upgraded_connection (struct MHD_Connection *connection)
{
  struct MHD_UpgradeResponseHandle *urh = connection->urh;

  if (NULL == urh)
    return;
#ifdef HTTPS_SUPPORT
  /* Signal remote client the end of TLS connection by
   * gracefully closing TLS session. */
  if (0 != (connection->daemon->options & MHD_USE_TLS))
    gnutls_bye (connection->tls_session,
                GNUTLS_SHUT_WR);

  if (MHD_INVALID_SOCKET != urh->mhd.socket)
    MHD_socket_close_chk_ (urh->mhd.socket);

  if (MHD_INVALID_SOCKET != urh->app.socket)
    MHD_socket_close_chk_ (urh->app.socket);
#endif /* HTTPS_SUPPORT */
  connection->urh = NULL;
  free (urh);
}


#endif /* UPGRADE_SUPPORT */


#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
/**
 * Performs bi-directional forwarding on upgraded HTTPS connections
 * based on the readiness state stored in the @a urh handle.
 * @remark To be called only from thread that processes
 * connection's recv(), send() and response.
 *
 * @param urh handle to process
 */
static void
process_urh (struct MHD_UpgradeResponseHandle *urh)
{
  /* Help compiler to optimize:
   * pointers to 'connection' and 'daemon' are not changed
   * during this processing, so no need to chain dereference
   * each time. */
  struct MHD_Connection *const connection = urh->connection;
  struct MHD_Daemon *const daemon = connection->daemon;
  /* Prevent data races: use same value of 'was_closed' throughout
   * this function. If 'was_closed' changed externally in the middle
   * of processing - it will be processed on next iteration. */
  bool was_closed;

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (connection->tid) );
#endif /* MHD_USE_THREADS */

  mhd_assert (0 != (daemon->options & MHD_USE_TLS));

  if (daemon->shutdown)
  {
    /* Daemon shutting down, application will not receive any more data. */
#ifdef HAVE_MESSAGES
    if (! urh->was_closed)
    {
      MHD_DLOG (daemon,
                _ ("Initiated daemon shutdown while \"upgraded\" " \
                   "connection was not closed.\n"));
    }
#endif
    urh->was_closed = true;
  }
  was_closed = urh->was_closed;
  if (was_closed)
  {
    /* Application was closed connections: no more data
     * can be forwarded to application socket. */
    if (0 < urh->in_buffer_used)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to forward to application %" PRIu64 \
                   " bytes of data received from remote side: " \
                   "application closed data forwarding.\n"),
                (uint64_t) urh->in_buffer_used);
#endif

    }
    /* Discard any data received form remote. */
    urh->in_buffer_used = 0;
    /* Do not try to push data to application. */
    urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
    /* Reading from remote client is not required anymore. */
    urh->in_buffer_size = 0;
    urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
    connection->tls_read_ready = false;
  }

  /* On some platforms (W32, possibly Darwin) failed send() (send() will
   * always fail after remote disconnect was detected) may discard data in
   * system buffers received by system but not yet read by recv().  So, before
   * trying send() on any socket, recv() must be performed at first otherwise
   * last part of incoming data may be lost.  If disconnect or error was
   * detected - try to read from socket to dry data possibly pending is system
   * buffers. */

  /*
   * handle reading from remote TLS client
   */
  if (((0 != ((MHD_EPOLL_STATE_ERROR | MHD_EPOLL_STATE_READ_READY)
              & urh->app.celi)) ||
       (connection->tls_read_ready)) &&
      (urh->in_buffer_used < urh->in_buffer_size))
  {
    ssize_t res;
    size_t buf_size;

    buf_size = urh->in_buffer_size - urh->in_buffer_used;
    if (buf_size > SSIZE_MAX)
      buf_size = SSIZE_MAX;

    res = gnutls_record_recv (connection->tls_session,
                              &urh->in_buffer[urh->in_buffer_used],
                              buf_size);
    if (0 >= res)
    {
      connection->tls_read_ready = false;
      if (GNUTLS_E_INTERRUPTED != res)
      {
        urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
        if ((GNUTLS_E_AGAIN != res) ||
            (0 != (MHD_EPOLL_STATE_ERROR & urh->app.celi)))
        {
          /* TLS unrecoverable error has been detected,
             socket error was detected and all data has been read,
             or socket was disconnected/shut down. */
          /* Stop trying to read from this TLS socket. */
          urh->in_buffer_size = 0;
        }
      }
    }
    else   /* 0 < res */
    {
      urh->in_buffer_used += (size_t) res;
      connection->tls_read_ready =
        (0 < gnutls_record_check_pending (connection->tls_session));
    }
  }

  /*
   * handle reading from application
   */
  /* If application signalled MHD about socket closure then
   * check for any pending data even if socket is not marked
   * as 'ready' (signal may arrive after poll()/select()).
   * Socketpair for forwarding is always in non-blocking mode
   * so no risk that recv() will block the thread. */
  if (((0 != ((MHD_EPOLL_STATE_ERROR | MHD_EPOLL_STATE_READ_READY)
              & urh->mhd.celi))
       || was_closed) /* Force last reading from app if app has closed the connection */
      && (urh->out_buffer_used < urh->out_buffer_size))
  {
    ssize_t res;
    size_t buf_size;

    buf_size = urh->out_buffer_size - urh->out_buffer_used;
    if (buf_size > MHD_SCKT_SEND_MAX_SIZE_)
      buf_size = MHD_SCKT_SEND_MAX_SIZE_;

    res = MHD_recv_ (urh->mhd.socket,
                     &urh->out_buffer[urh->out_buffer_used],
                     buf_size);
    if (0 >= res)
    {
      const int err = MHD_socket_get_error_ ();
      if ((0 == res) ||
          ((! MHD_SCKT_ERR_IS_EINTR_ (err)) &&
           (! MHD_SCKT_ERR_IS_LOW_RESOURCES_ (err))))
      {
        urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
        if ((0 == res) ||
            (was_closed) ||
            (0 != (MHD_EPOLL_STATE_ERROR & urh->mhd.celi)) ||
            (! MHD_SCKT_ERR_IS_EAGAIN_ (err)))
        {
          /* Socket disconnect/shutdown was detected;
           * Application signalled about closure of 'upgraded' socket and
           * all data has been read from application;
           * or persistent / unrecoverable error. */
          /* Do not try to pull more data from application. */
          urh->out_buffer_size = 0;
        }
      }
    }
    else   /* 0 < res */
    {
      urh->out_buffer_used += (size_t) res;
      if (buf_size > (size_t) res)
        urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
    }
  }

  /*
   * handle writing to remote HTTPS client
   */
  if ( (0 != (MHD_EPOLL_STATE_WRITE_READY & urh->app.celi)) &&
       (urh->out_buffer_used > 0) )
  {
    ssize_t res;
    size_t data_size;

    data_size = urh->out_buffer_used;
    if (data_size > SSIZE_MAX)
      data_size = SSIZE_MAX;

    res = gnutls_record_send (connection->tls_session,
                              urh->out_buffer,
                              data_size);
    if (0 >= res)
    {
      if (GNUTLS_E_INTERRUPTED != res)
      {
        urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
        if (GNUTLS_E_AGAIN != res)
        {
          /* TLS connection shut down or
           * persistent / unrecoverable error. */
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Failed to forward to remote client %" PRIu64 \
                       " bytes of data received from application: %s\n"),
                    (uint64_t) urh->out_buffer_used,
                    gnutls_strerror ((int) res));
#endif
          /* Discard any data unsent to remote. */
          urh->out_buffer_used = 0;
          /* Do not try to pull more data from application. */
          urh->out_buffer_size = 0;
          urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
        }
      }
    }
    else   /* 0 < res */
    {
      const size_t next_out_buffer_used = urh->out_buffer_used - (size_t) res;
      if (0 != next_out_buffer_used)
      {
        memmove (urh->out_buffer,
                 &urh->out_buffer[res],
                 next_out_buffer_used);
      }
      urh->out_buffer_used = next_out_buffer_used;
    }
    if ( (0 == urh->out_buffer_used) &&
         (0 != (MHD_EPOLL_STATE_ERROR & urh->app.celi)) )
    {
      /* Unrecoverable error on socket was detected and all
       * pending data was sent to remote. */
      /* Do not try to send to remote anymore. */
      urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
      /* Do not try to pull more data from application. */
      urh->out_buffer_size = 0;
      urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
    }
  }

  /*
   * handle writing to application
   */
  if ( (0 != (MHD_EPOLL_STATE_WRITE_READY & urh->mhd.celi)) &&
       (urh->in_buffer_used > 0) )
  {
    ssize_t res;
    size_t data_size;

    data_size = urh->in_buffer_used;
    if (data_size > MHD_SCKT_SEND_MAX_SIZE_)
      data_size = MHD_SCKT_SEND_MAX_SIZE_;

    res = MHD_send_ (urh->mhd.socket,
                     urh->in_buffer,
                     data_size);
    if (0 >= res)
    {
      const int err = MHD_socket_get_error_ ();
      if ( (! MHD_SCKT_ERR_IS_EINTR_ (err)) &&
           (! MHD_SCKT_ERR_IS_LOW_RESOURCES_ (err)) )
      {
        urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
        if (! MHD_SCKT_ERR_IS_EAGAIN_ (err))
        {
          /* Socketpair connection shut down or
           * persistent / unrecoverable error. */
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Failed to forward to application %" PRIu64 \
                       " bytes of data received from remote side: %s\n"),
                    (uint64_t) urh->in_buffer_used,
                    MHD_socket_strerr_ (err));
#endif
          /* Discard any data received from remote. */
          urh->in_buffer_used = 0;
          /* Reading from remote client is not required anymore. */
          urh->in_buffer_size = 0;
          urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
          connection->tls_read_ready = false;
        }
      }
    }
    else   /* 0 < res */
    {
      const size_t next_in_buffer_used = urh->in_buffer_used - (size_t) res;
      if (0 != next_in_buffer_used)
      {
        memmove (urh->in_buffer,
                 &urh->in_buffer[res],
                 next_in_buffer_used);
        if (data_size > (size_t) res)
          urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
      }
      urh->in_buffer_used = next_in_buffer_used;
    }
    if ( (0 == urh->in_buffer_used) &&
         (0 != (MHD_EPOLL_STATE_ERROR & urh->mhd.celi)) )
    {
      /* Do not try to push data to application. */
      urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
      /* Reading from remote client is not required anymore. */
      urh->in_buffer_size = 0;
      urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
      connection->tls_read_ready = false;
    }
  }

  /* Check whether data is present in TLS buffers
   * and incoming forward buffer have some space. */
  if ( (connection->tls_read_ready) &&
       (urh->in_buffer_used < urh->in_buffer_size) &&
       (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon)) )
    daemon->data_already_pending = true;

  if ( (daemon->shutdown) &&
       ( (0 != urh->out_buffer_size) ||
         (0 != urh->out_buffer_used) ) )
  {
    /* Daemon shutting down, discard any remaining forward data. */
#ifdef HAVE_MESSAGES
    if (0 < urh->out_buffer_used)
      MHD_DLOG (daemon,
                _ ("Failed to forward to remote client %" PRIu64 \
                   " bytes of data received from application: daemon shut down.\n"),
                (uint64_t) urh->out_buffer_used);
#endif
    /* Discard any data unsent to remote. */
    urh->out_buffer_used = 0;
    /* Do not try to sent to remote anymore. */
    urh->app.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_WRITE_READY);
    /* Do not try to pull more data from application. */
    urh->out_buffer_size = 0;
    urh->mhd.celi &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
  }

  if (! was_closed && urh->was_closed)
    daemon->data_already_pending = true; /* Force processing again */
}


#endif /* HTTPS_SUPPORT  && UPGRADE_SUPPORT */

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#ifdef UPGRADE_SUPPORT
/**
 * Main function of the thread that handles an individual connection
 * after it was "upgraded" when #MHD_USE_THREAD_PER_CONNECTION is set.
 * @remark To be called only from thread that process
 * connection's recv(), send() and response.
 *
 * @param con the connection this thread will handle
 */
static void
thread_main_connection_upgrade (struct MHD_Connection *con)
{
#ifdef HTTPS_SUPPORT
  struct MHD_UpgradeResponseHandle *urh = con->urh;
  struct MHD_Daemon *daemon = con->daemon;

  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (con->tid) );
  /* Here, we need to bi-directionally forward
     until the application tells us that it is done
     with the socket; */
  if ( (0 != (daemon->options & MHD_USE_TLS)) &&
       MHD_D_IS_USING_SELECT_ (daemon))
  {
    while ( (0 != urh->in_buffer_size) ||
            (0 != urh->out_buffer_size) ||
            (0 != urh->in_buffer_used) ||
            (0 != urh->out_buffer_used) )
    {
      /* use select */
      fd_set rs;
      fd_set ws;
      fd_set es;
      MHD_socket max_fd;
      int num_ready;
      bool result;

      FD_ZERO (&rs);
      FD_ZERO (&ws);
      FD_ZERO (&es);
      max_fd = MHD_INVALID_SOCKET;
      result = urh_to_fdset (urh,
                             &rs,
                             &ws,
                             &es,
                             &max_fd,
                             FD_SETSIZE);
      if (! result)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (con->daemon,
                  _ ("Error preparing select.\n"));
#endif
        break;
      }
      /* FIXME: does this check really needed? */
      if (MHD_INVALID_SOCKET != max_fd)
      {
        struct timeval *tvp;
        struct timeval tv;
        if (((con->tls_read_ready) &&
             (urh->in_buffer_used < urh->in_buffer_size)) ||
            (daemon->shutdown))
        {         /* No need to wait if incoming data is already pending in TLS buffers. */
          tv.tv_sec = 0;
          tv.tv_usec = 0;
          tvp = &tv;
        }
        else
          tvp = NULL;
        num_ready = MHD_SYS_select_ (max_fd + 1,
                                     &rs,
                                     &ws,
                                     &es,
                                     tvp);
      }
      else
        num_ready = 0;
      if (num_ready < 0)
      {
        const int err = MHD_socket_get_error_ ();

        if (MHD_SCKT_ERR_IS_EINTR_ (err))
          continue;
#ifdef HAVE_MESSAGES
        MHD_DLOG (con->daemon,
                  _ ("Error during select (%d): `%s'\n"),
                  err,
                  MHD_socket_strerr_ (err));
#endif
        break;
      }
      urh_from_fdset (urh,
                      &rs,
                      &ws,
                      &es,
                      (int) FD_SETSIZE);
      process_urh (urh);
    }
  }
#ifdef HAVE_POLL
  else if (0 != (daemon->options & MHD_USE_TLS))
  {
    /* use poll() */
    struct pollfd p[2];
    memset (p,
            0,
            sizeof (p));
    p[0].fd = urh->connection->socket_fd;
    p[1].fd = urh->mhd.socket;

    while ( (0 != urh->in_buffer_size) ||
            (0 != urh->out_buffer_size) ||
            (0 != urh->in_buffer_used) ||
            (0 != urh->out_buffer_used) )
    {
      int timeout;

      urh_update_pollfd (urh, p);

      if (((con->tls_read_ready) &&
           (urh->in_buffer_used < urh->in_buffer_size)) ||
          (daemon->shutdown))
        timeout = 0;     /* No need to wait if incoming data is already pending in TLS buffers. */
      else
        timeout = -1;

      if (MHD_sys_poll_ (p,
                         2,
                         timeout) < 0)
      {
        const int err = MHD_socket_get_error_ ();

        if (MHD_SCKT_ERR_IS_EINTR_ (err))
          continue;
#ifdef HAVE_MESSAGES
        MHD_DLOG (con->daemon,
                  _ ("Error during poll: `%s'\n"),
                  MHD_socket_strerr_ (err));
#endif
        break;
      }
      urh_from_pollfd (urh,
                       p);
      process_urh (urh);
    }
  }
  /* end POLL */
#endif
  /* end HTTPS */
#endif /* HTTPS_SUPPORT */
  /* TLS forwarding was finished. Cleanup socketpair. */
  MHD_connection_finish_forward_ (con);
  /* Do not set 'urh->clean_ready' yet as 'urh' will be used
   * in connection thread for a little while. */
}


#endif /* UPGRADE_SUPPORT */


/**
 * Get maximum wait period for the connection (the amount of time left before
 * connection time out)
 * @param c the connection to check
 * @return the maximum number of millisecond before the connection must be
 *         processed again.
 */
static uint64_t
connection_get_wait (struct MHD_Connection *c)
{
  const uint64_t now = MHD_monotonic_msec_counter ();
  const uint64_t since_actv = now - c->last_activity;
  const uint64_t timeout = c->connection_timeout_ms;
  uint64_t mseconds_left;

  mhd_assert (0 != timeout);
  /* Keep the next lines in sync with #connection_check_timedout() to avoid
   * undesired side-effects like busy-waiting. */
  if (timeout < since_actv)
  {
    if (UINT64_MAX / 2 < since_actv)
    {
      const uint64_t jump_back = c->last_activity - now;
      /* Very unlikely that it is more than quarter-million years pause.
       * More likely that system clock jumps back. */
      if (5000 >= jump_back)
      { /* Jump back is less than 5 seconds, try to recover. */
        return 100; /* Set wait time to 0.1 seconds */
      }
      /* Too large jump back */
    }
    return 0; /* Connection has timed out */
  }
  else if (since_actv == timeout)
  {
    /* Exact match for timeout and time from last activity.
     * Maybe this is just a precise match or this happens because the timer
     * resolution is too low.
     * Set wait time to 0.1 seconds to avoid busy-waiting with low
     * timer resolution as connection is not timed-out yet. */
    return 100;
  }
  mseconds_left = timeout - since_actv;

  return mseconds_left;
}


/**
 * Main function of the thread that handles an individual
 * connection when #MHD_USE_THREAD_PER_CONNECTION is set.
 *
 * @param data the `struct MHD_Connection` this thread will handle
 * @return always 0
 */
static MHD_THRD_RTRN_TYPE_ MHD_THRD_CALL_SPEC_
thread_main_handle_connection (void *data)
{
  struct MHD_Connection *con = data;
  struct MHD_Daemon *daemon = con->daemon;
  int num_ready;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket maxsock;
#ifdef WINDOWS
#ifdef HAVE_POLL
  unsigned int extra_slot;
#endif /* HAVE_POLL */
#define EXTRA_SLOTS 1
#else  /* !WINDOWS */
#define EXTRA_SLOTS 0
#endif /* !WINDOWS */
#ifdef HAVE_POLL
  struct pollfd p[1 + EXTRA_SLOTS];
#endif
#undef EXTRA_SLOTS
#ifdef HAVE_POLL
  const bool use_poll = MHD_D_IS_USING_POLL_ (daemon);
#else  /* ! HAVE_POLL */
  const bool use_poll = 0;
#endif /* ! HAVE_POLL */
  bool was_suspended = false;
  MHD_thread_handle_ID_set_current_thread_ID_ (&(con->tid));

  while ( (! daemon->shutdown) &&
          (MHD_CONNECTION_CLOSED != con->state) )
  {
    bool use_zero_timeout;
#ifdef UPGRADE_SUPPORT
    struct MHD_UpgradeResponseHandle *const urh = con->urh;
#else  /* ! UPGRADE_SUPPORT */
    static const void *const urh = NULL;
#endif /* ! UPGRADE_SUPPORT */

    if ( (con->suspended) &&
         (NULL == urh) )
    {
      /* Connection was suspended, wait for resume. */
      was_suspended = true;
      if (! use_poll)
      {
        FD_ZERO (&rs);
        if (! MHD_add_to_fd_set_ (MHD_itc_r_fd_ (daemon->itc),
                                  &rs,
                                  NULL,
                                  FD_SETSIZE))
        {
  #ifdef HAVE_MESSAGES
          MHD_DLOG (con->daemon,
                    _ ("Failed to add FD to fd_set.\n"));
  #endif
          goto exit;
        }
        if (0 > MHD_SYS_select_ (MHD_itc_r_fd_ (daemon->itc) + 1,
                                 &rs,
                                 NULL,
                                 NULL,
                                 NULL))
        {
          const int err = MHD_socket_get_error_ ();

          if (MHD_SCKT_ERR_IS_EINTR_ (err))
            continue;
#ifdef HAVE_MESSAGES
          MHD_DLOG (con->daemon,
                    _ ("Error during select (%d): `%s'\n"),
                    err,
                    MHD_socket_strerr_ (err));
#endif
          break;
        }
      }
#ifdef HAVE_POLL
      else     /* use_poll */
      {
        p[0].events = POLLIN;
        p[0].fd = MHD_itc_r_fd_ (daemon->itc);
        p[0].revents = 0;
        if (0 > MHD_sys_poll_ (p,
                               1,
                               -1))
        {
          if (MHD_SCKT_LAST_ERR_IS_ (MHD_SCKT_EINTR_))
            continue;
#ifdef HAVE_MESSAGES
          MHD_DLOG (con->daemon,
                    _ ("Error during poll: `%s'\n"),
                    MHD_socket_last_strerr_ ());
#endif
          break;
        }
      }
#endif /* HAVE_POLL */
      MHD_itc_clear_ (daemon->itc);
      continue; /* Check again for resume. */
    }           /* End of "suspended" branch. */

    if (was_suspended)
    {
      MHD_update_last_activity_ (con);     /* Reset timeout timer. */
      /* Process response queued during suspend and update states. */
      MHD_connection_handle_idle (con);
      was_suspended = false;
    }

    use_zero_timeout =
      (0 != (MHD_EVENT_LOOP_INFO_PROCESS & con->event_loop_info)
#ifdef HTTPS_SUPPORT
       || ( (con->tls_read_ready) &&
            (0 != (MHD_EVENT_LOOP_INFO_READ & con->event_loop_info)) )
#endif /* HTTPS_SUPPORT */
      );
    if (! use_poll)
    {
      /* use select */
      bool err_state = false;
      struct timeval tv;
      struct timeval *tvp;
      if (use_zero_timeout)
      {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        tvp = &tv;
      }
      else if (con->connection_timeout_ms > 0)
      {
        const uint64_t mseconds_left = connection_get_wait (con);
#if (SIZEOF_UINT64_T - 2) >= SIZEOF_STRUCT_TIMEVAL_TV_SEC
        if (mseconds_left / 1000 > TIMEVAL_TV_SEC_MAX)
          tv.tv_sec = TIMEVAL_TV_SEC_MAX;
        else
#endif /* (SIZEOF_UINT64_T - 2) >= SIZEOF_STRUCT_TIMEVAL_TV_SEC */
        tv.tv_sec = (_MHD_TIMEVAL_TV_SEC_TYPE) mseconds_left / 1000;

        tv.tv_usec = ((uint16_t) (mseconds_left % 1000)) * ((int32_t) 1000);
        tvp = &tv;
      }
      else
        tvp = NULL;

      FD_ZERO (&rs);
      FD_ZERO (&ws);
      FD_ZERO (&es);
      maxsock = MHD_INVALID_SOCKET;
      switch (con->event_loop_info)
      {
      case MHD_EVENT_LOOP_INFO_READ:
      case MHD_EVENT_LOOP_INFO_PROCESS_READ:
        if (! MHD_add_to_fd_set_ (con->socket_fd,
                                  &rs,
                                  &maxsock,
                                  FD_SETSIZE))
          err_state = true;
        break;
      case MHD_EVENT_LOOP_INFO_WRITE:
        if (! MHD_add_to_fd_set_ (con->socket_fd,
                                  &ws,
                                  &maxsock,
                                  FD_SETSIZE))
          err_state = true;
        break;
      case MHD_EVENT_LOOP_INFO_PROCESS:
        if (! MHD_add_to_fd_set_ (con->socket_fd,
                                  &es,
                                  &maxsock,
                                  FD_SETSIZE))
          err_state = true;
        break;
      case MHD_EVENT_LOOP_INFO_CLEANUP:
        /* how did we get here!? */
        goto exit;
      }
#ifdef WINDOWS
      if (MHD_ITC_IS_VALID_ (daemon->itc) )
      {
        if (! MHD_add_to_fd_set_ (MHD_itc_r_fd_ (daemon->itc),
                                  &rs,
                                  &maxsock,
                                  FD_SETSIZE))
          err_state = 1;
      }
#endif
      if (err_state)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (con->daemon,
                  _ ("Failed to add FD to fd_set.\n"));
#endif
        goto exit;
      }

      num_ready = MHD_SYS_select_ (maxsock + 1,
                                   &rs,
                                   &ws,
                                   &es,
                                   tvp);
      if (num_ready < 0)
      {
        const int err = MHD_socket_get_error_ ();

        if (MHD_SCKT_ERR_IS_EINTR_ (err))
          continue;
#ifdef HAVE_MESSAGES
        MHD_DLOG (con->daemon,
                  _ ("Error during select (%d): `%s'\n"),
                  err,
                  MHD_socket_strerr_ (err));
#endif
        break;
      }
#ifdef WINDOWS
      /* Clear ITC before other processing so additional
       * signals will trigger select() again */
      if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
           (FD_ISSET (MHD_itc_r_fd_ (daemon->itc),
                      &rs)) )
        MHD_itc_clear_ (daemon->itc);
#endif
      if (MHD_NO ==
          call_handlers (con,
                         FD_ISSET (con->socket_fd,
                                   &rs),
                         FD_ISSET (con->socket_fd,
                                   &ws),
                         FD_ISSET (con->socket_fd,
                                   &es)) )
        goto exit;
    }
#ifdef HAVE_POLL
    else
    {
      int timeout_val;
      /* use poll */
      if (use_zero_timeout)
        timeout_val = 0;
      else if (con->connection_timeout_ms > 0)
      {
        const uint64_t mseconds_left = connection_get_wait (con);
#if SIZEOF_UINT64_T >= SIZEOF_INT
        if (mseconds_left >= INT_MAX)
          timeout_val = INT_MAX;
        else
#endif /* SIZEOF_UINT64_T >= SIZEOF_INT */
        timeout_val = (int) mseconds_left;
      }
      else
        timeout_val = -1;
      memset (&p,
              0,
              sizeof (p));
      p[0].fd = con->socket_fd;
      switch (con->event_loop_info)
      {
      case MHD_EVENT_LOOP_INFO_READ:
      case MHD_EVENT_LOOP_INFO_PROCESS_READ:
        p[0].events |= POLLIN | MHD_POLL_EVENTS_ERR_DISC;
        break;
      case MHD_EVENT_LOOP_INFO_WRITE:
        p[0].events |= POLLOUT | MHD_POLL_EVENTS_ERR_DISC;
        break;
      case MHD_EVENT_LOOP_INFO_PROCESS:
        p[0].events |= MHD_POLL_EVENTS_ERR_DISC;
        break;
      case MHD_EVENT_LOOP_INFO_CLEANUP:
        /* how did we get here!? */
        goto exit;
      }
#ifdef WINDOWS
      extra_slot = 0;
      if (MHD_ITC_IS_VALID_ (daemon->itc))
      {
        p[1].events |= POLLIN;
        p[1].fd = MHD_itc_r_fd_ (daemon->itc);
        p[1].revents = 0;
        extra_slot = 1;
      }
#endif
      if (MHD_sys_poll_ (p,
#ifdef WINDOWS
                         1 + extra_slot,
#else
                         1,
#endif
                         timeout_val) < 0)
      {
        if (MHD_SCKT_LAST_ERR_IS_ (MHD_SCKT_EINTR_))
          continue;
#ifdef HAVE_MESSAGES
        MHD_DLOG (con->daemon,
                  _ ("Error during poll: `%s'\n"),
                  MHD_socket_last_strerr_ ());
#endif
        break;
      }
#ifdef WINDOWS
      /* Clear ITC before other processing so additional
       * signals will trigger poll() again */
      if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
           (0 != (p[1].revents & (POLLERR | POLLHUP | POLLIN))) )
        MHD_itc_clear_ (daemon->itc);
#endif
      if (MHD_NO ==
          call_handlers (con,
                         (0 != (p[0].revents & POLLIN)),
                         (0 != (p[0].revents & POLLOUT)),
                         (0 != (p[0].revents & MHD_POLL_REVENTS_ERR_DISC)) ))
        goto exit;
    }
#endif
#ifdef UPGRADE_SUPPORT
    if (MHD_CONNECTION_UPGRADE == con->state)
    {
      /* Normal HTTP processing is finished,
       * notify application. */
      if ( (NULL != daemon->notify_completed) &&
           (con->rq.client_aware) )
        daemon->notify_completed (daemon->notify_completed_cls,
                                  con,
                                  &con->rq.client_context,
                                  MHD_REQUEST_TERMINATED_COMPLETED_OK);
      con->rq.client_aware = false;

      thread_main_connection_upgrade (con);
      /* MHD_connection_finish_forward_() was called by thread_main_connection_upgrade(). */

      /* "Upgraded" data will not be used in this thread from this point. */
      con->urh->clean_ready = true;
      /* If 'urh->was_closed' set to true, connection will be
       * moved immediately to cleanup list. Otherwise connection
       * will stay in suspended list until 'urh' will be marked
       * with 'was_closed' by application. */
      MHD_resume_connection (con);

      /* skip usual clean up  */
      return (MHD_THRD_RTRN_TYPE_) 0;
    }
#endif /* UPGRADE_SUPPORT */
  }
#if _MHD_DEBUG_CLOSE
#ifdef HAVE_MESSAGES
  MHD_DLOG (con->daemon,
            _ ("Processing thread terminating. Closing connection.\n"));
#endif
#endif
  if (MHD_CONNECTION_CLOSED != con->state)
    MHD_connection_close_ (con,
                           (daemon->shutdown) ?
                           MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN :
                           MHD_REQUEST_TERMINATED_WITH_ERROR);
  MHD_connection_handle_idle (con);
exit:
  if (NULL != con->rp.response)
  {
    MHD_destroy_response (con->rp.response);
    con->rp.response = NULL;
  }

  if (MHD_INVALID_SOCKET != con->socket_fd)
  {
    shutdown (con->socket_fd,
              SHUT_WR);
    /* 'socket_fd' can be used in other thread to signal shutdown.
     * To avoid data races, do not close socket here. Daemon will
     * use more connections only after cleanup anyway. */
  }
  if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
       (! MHD_itc_activate_ (daemon->itc, "t")) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to signal thread termination via inter-thread " \
                 "communication channel.\n"));
#endif
  }
  return (MHD_THRD_RTRN_TYPE_) 0;
}


#endif


/**
 * Free resources associated with all closed connections.
 * (destroy responses, free buffers, etc.).  All closed
 * connections are kept in the "cleanup" doubly-linked list.
 *
 * @param daemon daemon to clean up
 */
static void
MHD_cleanup_connections (struct MHD_Daemon *daemon);

#if defined(HTTPS_SUPPORT)
#if defined(MHD_SEND_SPIPE_SUPPRESS_NEEDED) && \
  defined(MHD_SEND_SPIPE_SUPPRESS_POSSIBLE) && \
  ! defined(MHD_socket_nosignal_) && \
  (GNUTLS_VERSION_NUMBER + 0 < 0x030402) && defined(MSG_NOSIGNAL)
/**
 * Older version of GnuTLS do not support suppressing of SIGPIPE signal.
 * Use push function replacement with suppressing SIGPIPE signal where necessary
 * and if possible.
 */
#define MHD_TLSLIB_NEED_PUSH_FUNC 1
#endif /* MHD_SEND_SPIPE_SUPPRESS_NEEDED &&
          MHD_SEND_SPIPE_SUPPRESS_POSSIBLE &&
          ! MHD_socket_nosignal_ && (GNUTLS_VERSION_NUMBER+0 < 0x030402) &&
          MSG_NOSIGNAL */

#ifdef MHD_TLSLIB_NEED_PUSH_FUNC
/**
 * Data push function replacement with suppressing SIGPIPE signal
 * for TLS library.
 */
static ssize_t
MHD_tls_push_func_ (gnutls_transport_ptr_t trnsp,
                    const void *data,
                    size_t data_size)
{
#if (MHD_SCKT_SEND_MAX_SIZE_ < SSIZE_MAX) || (0 == SSIZE_MAX)
  if (data_size > MHD_SCKT_SEND_MAX_SIZE_)
    data_size = MHD_SCKT_SEND_MAX_SIZE_;
#endif /* (MHD_SCKT_SEND_MAX_SIZE_ < SSIZE_MAX) || (0 == SSIZE_MAX) */
  return MHD_send_ ((MHD_socket) (intptr_t) (trnsp), data, data_size);
}


#endif /* MHD_TLSLIB_DONT_SUPPRESS_SIGPIPE */


/**
 * Function called by GNUtls to obtain the PSK for a given session.
 *
 * @param session the session to lookup PSK for
 * @param username username to lookup PSK for
 * @param[out] key where to write PSK
 * @return 0 on success, -1 on error
 */
static int
psk_gnutls_adapter (gnutls_session_t session,
                    const char *username,
                    gnutls_datum_t *key)
{
  struct MHD_Connection *connection;
  struct MHD_Daemon *daemon;
#if GNUTLS_VERSION_MAJOR >= 3
  void *app_psk;
  size_t app_psk_size;
#endif /* GNUTLS_VERSION_MAJOR >= 3 */

  connection = gnutls_session_get_ptr (session);
  if (NULL == connection)
  {
#ifdef HAVE_MESSAGES
    /* Cannot use our logger, we don't even have "daemon" */
    MHD_PANIC (_ ("Internal server error. This should be impossible.\n"));
#endif
    return -1;
  }
  daemon = connection->daemon;
#if GNUTLS_VERSION_MAJOR >= 3
  if (NULL == daemon->cred_callback)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("PSK not supported by this server.\n"));
#endif
    return -1;
  }
  if (0 != daemon->cred_callback (daemon->cred_callback_cls,
                                  connection,
                                  username,
                                  &app_psk,
                                  &app_psk_size))
    return -1;
  if (NULL == (key->data = gnutls_malloc (app_psk_size)))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("PSK authentication failed: gnutls_malloc failed to " \
                 "allocate memory.\n"));
#endif
    free (app_psk);
    return -1;
  }
  if (UINT_MAX < app_psk_size)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("PSK authentication failed: PSK too long.\n"));
#endif
    free (app_psk);
    return -1;
  }
  key->size = (unsigned int) app_psk_size;
  memcpy (key->data,
          app_psk,
          app_psk_size);
  free (app_psk);
  return 0;
#else
  (void) username; (void) key; /* Mute compiler warning */
#ifdef HAVE_MESSAGES
  MHD_DLOG (daemon,
            _ ("PSK not supported by this server.\n"));
#endif
  return -1;
#endif
}


#endif /* HTTPS_SUPPORT */


/**
 * Do basic preparation work on the new incoming connection.
 *
 * This function do all preparation that is possible outside main daemon
 * thread.
 * @remark Could be called from any thread.
 *
 * @param daemon daemon that manages the connection
 * @param client_socket socket to manage (MHD will expect
 *        to receive an HTTP request from this socket next).
 * @param addr IP address of the client
 * @param addrlen number of bytes in @a addr
 * @param external_add indicate that socket has been added externally
 * @param non_blck indicate that socket in non-blocking mode
 * @param sk_spipe_supprs indicate that the @a client_socket has
 *                         set SIGPIPE suppression
 * @param sk_is_nonip _MHD_YES if this is not a TCP/IP socket
 * @return pointer to the connection on success, NULL if this daemon could
 *        not handle the connection (i.e. malloc failed, etc).
 *        The socket will be closed in case of error; 'errno' is
 *        set to indicate further details about the error.
 */
static struct MHD_Connection *
new_connection_prepare_ (struct MHD_Daemon *daemon,
                         MHD_socket client_socket,
                         const struct sockaddr_storage *addr,
                         socklen_t addrlen,
                         bool external_add,
                         bool non_blck,
                         bool sk_spipe_supprs,
                         enum MHD_tristate sk_is_nonip)
{
  struct MHD_Connection *connection;
  int eno = 0;

#ifdef HAVE_MESSAGES
#if _MHD_DEBUG_CONNECT
  MHD_DLOG (daemon,
            _ ("Accepted connection on socket %d.\n"),
            client_socket);
#endif
#endif
  if ( (daemon->connections == daemon->connection_limit) ||
       (MHD_NO == MHD_ip_limit_add (daemon,
                                    addr,
                                    addrlen)) )
  {
    /* above connection limit - reject */
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Server reached connection limit. " \
                 "Closing inbound connection.\n"));
#endif
    MHD_socket_close_chk_ (client_socket);
#if defined(ENFILE) && (ENFILE + 0 != 0)
    errno = ENFILE;
#endif
    return NULL;
  }

  /* apply connection acceptance policy if present */
  if ( (NULL != daemon->apc) &&
       (MHD_NO == daemon->apc (daemon->apc_cls,
                               (const struct sockaddr *) addr,
                               addrlen)) )
  {
#if _MHD_DEBUG_CLOSE
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Connection rejected by application. Closing connection.\n"));
#endif
#endif
    MHD_socket_close_chk_ (client_socket);
    MHD_ip_limit_del (daemon,
                      addr,
                      addrlen);
#if defined(EACCESS) && (EACCESS + 0 != 0)
    errno = EACCESS;
#endif
    return NULL;
  }

  if (NULL == (connection = MHD_calloc_ (1, sizeof (struct MHD_Connection))))
  {
    eno = errno;
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Error allocating memory: %s\n"),
              MHD_strerror_ (errno));
#endif
    MHD_socket_close_chk_ (client_socket);
    MHD_ip_limit_del (daemon,
                      addr,
                      addrlen);
    errno = eno;
    return NULL;
  }

  if (! external_add)
  {
    connection->sk_corked = _MHD_OFF;
    connection->sk_nodelay = _MHD_OFF;
  }
  else
  {
    connection->sk_corked = _MHD_UNKNOWN;
    connection->sk_nodelay = _MHD_UNKNOWN;
  }

  if (0 < addrlen)
  {
    if (NULL == (connection->addr = malloc ((size_t) addrlen)))
    {
      eno = errno;
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Error allocating memory: %s\n"),
                MHD_strerror_ (errno));
#endif
      MHD_socket_close_chk_ (client_socket);
      MHD_ip_limit_del (daemon,
                        addr,
                        addrlen);
      free (connection);
      errno = eno;
      return NULL;
    }
    memcpy (connection->addr,
            addr,
            (size_t) addrlen);
  }
  else
    connection->addr = NULL;
  connection->addr_len = addrlen;
  connection->socket_fd = client_socket;
  connection->sk_nonblck = non_blck;
  connection->is_nonip = sk_is_nonip;
  connection->sk_spipe_suppress = sk_spipe_supprs;
#ifdef MHD_USE_THREADS
  MHD_thread_handle_ID_set_invalid_ (&connection->tid);
#endif /* MHD_USE_THREADS */
  connection->daemon = daemon;
  connection->connection_timeout_ms = daemon->connection_timeout_ms;
  connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
  if (0 != connection->connection_timeout_ms)
    connection->last_activity = MHD_monotonic_msec_counter ();

  if (0 == (daemon->options & MHD_USE_TLS))
  {
    /* set default connection handlers  */
    MHD_set_http_callbacks_ (connection);
  }
  else
  {
#ifdef HTTPS_SUPPORT
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030500)
    gnutls_init_flags_t
#else
    unsigned int
#endif
    flags;

    flags = GNUTLS_SERVER;
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030402)
    flags |= GNUTLS_NO_SIGNAL;
#endif /* GNUTLS_VERSION_NUMBER >= 0x030402 */
#if GNUTLS_VERSION_MAJOR >= 3
    flags |= GNUTLS_NONBLOCK;
#endif /* GNUTLS_VERSION_MAJOR >= 3*/
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030603)
    if (0 != (daemon->options & MHD_USE_POST_HANDSHAKE_AUTH_SUPPORT))
      flags |= GNUTLS_POST_HANDSHAKE_AUTH;
#endif
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030605)
    if (0 != (daemon->options & MHD_USE_INSECURE_TLS_EARLY_DATA))
      flags |= GNUTLS_ENABLE_EARLY_DATA;
#endif
    connection->tls_state = MHD_TLS_CONN_INIT;
    MHD_set_https_callbacks (connection);
    if ((GNUTLS_E_SUCCESS != gnutls_init (&connection->tls_session, flags)) ||
        (GNUTLS_E_SUCCESS != gnutls_priority_set (connection->tls_session,
                                                  daemon->priority_cache)))
    {
      if (NULL != connection->tls_session)
        gnutls_deinit (connection->tls_session);
      MHD_socket_close_chk_ (client_socket);
      MHD_ip_limit_del (daemon,
                        addr,
                        addrlen);
      if (NULL != connection->addr)
        free (connection->addr);
      free (connection);
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to initialise TLS session.\n"));
#endif
#if defined(EPROTO) && (EPROTO + 0 != 0)
      errno = EPROTO;
#endif
      return NULL;
    }
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030200)
    if (! daemon->disable_alpn)
    {
      static const char prt1[] = "http/1.1"; /* Registered code for HTTP/1.1 */
      static const char prt2[] = "http/1.0"; /* Registered code for HTTP/1.0 */
      static const gnutls_datum_t prts[2] =
      { {_MHD_DROP_CONST (prt1), MHD_STATICSTR_LEN_ (prt1)},
        {_MHD_DROP_CONST (prt2), MHD_STATICSTR_LEN_ (prt2)} };

      if (GNUTLS_E_SUCCESS !=
          gnutls_alpn_set_protocols (connection->tls_session,
                                     prts,
                                     sizeof(prts) / sizeof(prts[0]),
                                     0 /* | GNUTLS_ALPN_SERVER_PRECEDENCE */))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Failed to set ALPN protocols.\n"));
#else  /* ! HAVE_MESSAGES */
        (void) 0; /* Mute compiler warning */
#endif /* ! HAVE_MESSAGES */
      }
    }
#endif /* GNUTLS_VERSION_NUMBER >= 0x030200 */
    gnutls_session_set_ptr (connection->tls_session,
                            connection);
    switch (daemon->cred_type)
    {
    /* set needed credentials for certificate authentication. */
    case GNUTLS_CRD_CERTIFICATE:
      gnutls_credentials_set (connection->tls_session,
                              GNUTLS_CRD_CERTIFICATE,
                              daemon->x509_cred);
      break;
    case GNUTLS_CRD_PSK:
      gnutls_credentials_set (connection->tls_session,
                              GNUTLS_CRD_PSK,
                              daemon->psk_cred);
      gnutls_psk_set_server_credentials_function (daemon->psk_cred,
                                                  &psk_gnutls_adapter);
      break;
    case GNUTLS_CRD_ANON:
    case GNUTLS_CRD_SRP:
    case GNUTLS_CRD_IA:
    default:
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to setup TLS credentials: " \
                   "unknown credential type %d.\n"),
                daemon->cred_type);
#endif
      gnutls_deinit (connection->tls_session);
      MHD_socket_close_chk_ (client_socket);
      MHD_ip_limit_del (daemon,
                        addr,
                        addrlen);
      if (NULL != connection->addr)
        free (connection->addr);
      free (connection);
      MHD_PANIC (_ ("Unknown credential type.\n"));
#if defined(EINVAL) && (EINVAL + 0 != 0)
      errno = EINVAL;
#endif
      return NULL;
    }
#if (GNUTLS_VERSION_NUMBER + 0 >= 0x030109) && ! defined(_WIN64)
    gnutls_transport_set_int (connection->tls_session,
                              (int) (client_socket));
#else  /* GnuTLS before 3.1.9 or Win x64 */
    gnutls_transport_set_ptr (connection->tls_session,
                              (gnutls_transport_ptr_t) \
                              (intptr_t) client_socket);
#endif /* GnuTLS before 3.1.9 or Win x64 */
#ifdef MHD_TLSLIB_NEED_PUSH_FUNC
    gnutls_transport_set_push_function (connection->tls_session,
                                        MHD_tls_push_func_);
#endif /* MHD_TLSLIB_NEED_PUSH_FUNC */
    if (daemon->https_mem_trust)
      gnutls_certificate_server_set_request (connection->tls_session,
                                             GNUTLS_CERT_REQUEST);
#else  /* ! HTTPS_SUPPORT */
    MHD_socket_close_chk_ (client_socket);
    MHD_ip_limit_del (daemon,
                      addr,
                      addrlen);
    free (connection->addr);
    free (connection);
    MHD_PANIC (_ ("TLS connection on non-TLS daemon.\n"));
#if 0
    /* Unreachable code */
    eno = EINVAL;
    return NULL;
#endif
#endif /* ! HTTPS_SUPPORT */
  }

  return connection;
}


#ifdef MHD_USE_THREADS
/**
 * Close prepared, but not yet processed connection.
 * @param daemon     the daemon
 * @param connection the connection to close
 */
static void
new_connection_close_ (struct MHD_Daemon *daemon,
                       struct MHD_Connection *connection)
{
  mhd_assert (connection->daemon == daemon);
  mhd_assert (! connection->in_cleanup);
  mhd_assert (NULL == connection->next);
  mhd_assert (NULL == connection->nextX);
#ifdef EPOLL_SUPPORT
  mhd_assert (NULL == connection->nextE);
#endif /* EPOLL_SUPPORT */

#ifdef HTTPS_SUPPORT
  if (NULL != connection->tls_session)
  {
    mhd_assert (0 != (daemon->options & MHD_USE_TLS));
    gnutls_deinit (connection->tls_session);
  }
#endif /* HTTPS_SUPPORT */
  MHD_socket_close_chk_ (connection->socket_fd);
  MHD_ip_limit_del (daemon,
                    connection->addr,
                    connection->addr_len);
  if (NULL != connection->addr)
    free (connection->addr);
  free (connection);
}


#endif /* MHD_USE_THREADS */


/**
 * Finally insert the new connection to the list of connections
 * served by the daemon and start processing.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon daemon that manages the connection
 * @param connection the newly created connection
 * @return #MHD_YES on success, #MHD_NO on error
 */
static enum MHD_Result
new_connection_process_ (struct MHD_Daemon *daemon,
                         struct MHD_Connection *connection)
{
  int eno = 0;

  mhd_assert (connection->daemon == daemon);

#ifdef MHD_USE_THREADS
  /* Function manipulate connection and timeout DL-lists,
   * must be called only within daemon thread. */
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */

  /* Allocate memory pool in the processing thread so
   * intensively used memory area is allocated in "good"
   * (for the thread) memory region. It is important with
   * NUMA and/or complex cache hierarchy. */
  connection->pool = MHD_pool_create (daemon->pool_size);
  if (NULL == connection->pool)
  { /* 'pool' creation failed */
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Error allocating memory: %s\n"),
              MHD_strerror_ (errno));
#endif
#if defined(ENOMEM) && (ENOMEM + 0 != 0)
    eno = ENOMEM;
#endif
    (void) 0; /* Mute possible compiler warning */
  }
  else
  { /* 'pool' creation succeed */
    MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
    /* Firm check under lock. */
    if (daemon->connections >= daemon->connection_limit)
    { /* Connections limit */
      MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Server reached connection limit. "
                   "Closing inbound connection.\n"));
#endif
#if defined(ENFILE) && (ENFILE + 0 != 0)
      eno = ENFILE;
#endif
      (void) 0; /* Mute possible compiler warning */
    }
    else
    { /* Have space for new connection */
      daemon->connections++;
      DLL_insert (daemon->connections_head,
                  daemon->connections_tail,
                  connection);
      if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
      {
        XDLL_insert (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     connection);
      }
      MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);

      MHD_connection_set_initial_state_ (connection);

      if (NULL != daemon->notify_connection)
        daemon->notify_connection (daemon->notify_connection_cls,
                                   connection,
                                   &connection->socket_context,
                                   MHD_CONNECTION_NOTIFY_STARTED);
#ifdef MHD_USE_THREADS
      if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
      {
        mhd_assert (! MHD_D_IS_USING_EPOLL_ (daemon));
        if (! MHD_create_named_thread_ (&connection->tid,
                                        "MHD-connection",
                                        daemon->thread_stack_size,
                                        &thread_main_handle_connection,
                                        connection))
        {
          eno = errno;
#ifdef HAVE_MESSAGES
#ifdef EAGAIN
          if (EAGAIN == eno)
            MHD_DLOG (daemon,
                      _ ("Failed to create a new thread because it would "
                         "have exceeded the system limit on the number of "
                         "threads or no system resources available.\n"));
          else
#endif /* EAGAIN */
          MHD_DLOG (daemon,
                    _ ("Failed to create a thread: %s\n"),
                    MHD_strerror_ (eno));
#endif /* HAVE_MESSAGES */
        }
        else               /* New thread has been created successfully */
          return MHD_YES;  /* *** Function success exit point *** */
      }
      else
#else  /* ! MHD_USE_THREADS */
      if (1)
#endif /* ! MHD_USE_THREADS */
      { /* No 'thread-per-connection' */
#ifdef MHD_USE_THREADS
        connection->tid = daemon->tid;
#endif /* MHD_USE_THREADS */
#ifdef EPOLL_SUPPORT
        if (MHD_D_IS_USING_EPOLL_ (daemon))
        {
          if (0 == (daemon->options & MHD_USE_TURBO))
          {
            struct epoll_event event;

            event.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET | EPOLLRDHUP;
            event.data.ptr = connection;
            if (0 != epoll_ctl (daemon->epoll_fd,
                                EPOLL_CTL_ADD,
                                connection->socket_fd,
                                &event))
            {
              eno = errno;
#ifdef HAVE_MESSAGES
              MHD_DLOG (daemon,
                        _ ("Call to epoll_ctl failed: %s\n"),
                        MHD_socket_last_strerr_ ());
#endif
            }
            else
            { /* 'socket_fd' has been added to 'epool' */
              connection->epoll_state |= MHD_EPOLL_STATE_IN_EPOLL_SET;

              return MHD_YES;  /* *** Function success exit point *** */
            }
          }
          else
          {
            connection->epoll_state |= MHD_EPOLL_STATE_READ_READY
                                       | MHD_EPOLL_STATE_WRITE_READY
                                       | MHD_EPOLL_STATE_IN_EREADY_EDLL;
            EDLL_insert (daemon->eready_head,
                         daemon->eready_tail,
                         connection);

            return MHD_YES;  /* *** Function success exit point *** */
          }
        }
        else /* No 'epoll' */
#endif /* EPOLL_SUPPORT */
        return MHD_YES;    /* *** Function success exit point *** */
      }

      /* ** Below is a cleanup path ** */
      if (NULL != daemon->notify_connection)
        daemon->notify_connection (daemon->notify_connection_cls,
                                   connection,
                                   &connection->socket_context,
                                   MHD_CONNECTION_NOTIFY_CLOSED);
      MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
      if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
      {
        XDLL_remove (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     connection);
      }
      DLL_remove (daemon->connections_head,
                  daemon->connections_tail,
                  connection);
      daemon->connections--;
      MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
    }
    MHD_pool_destroy (connection->pool);
  }
  /* Free resources allocated before the call of this functions */
#ifdef HTTPS_SUPPORT
  if (NULL != connection->tls_session)
    gnutls_deinit (connection->tls_session);
#endif /* HTTPS_SUPPORT */
  MHD_ip_limit_del (daemon,
                    connection->addr,
                    connection->addr_len);
  if (NULL != connection->addr)
    free (connection->addr);
  MHD_socket_close_chk_ (connection->socket_fd);
  free (connection);
  if (0 != eno)
    errno = eno;
#ifdef EINVAL
  else
    errno = EINVAL;
#endif /* EINVAL */
  return MHD_NO;  /* *** Function failure exit point *** */
}


/**
 * Add another client connection to the set of connections
 * managed by MHD.  This API is usually not needed (since
 * MHD will accept inbound connections on the server socket).
 * Use this API in special cases, for example if your HTTP
 * server is behind NAT and needs to connect out to the
 * HTTP client.
 *
 * The given client socket will be managed (and closed!) by MHD after
 * this call and must no longer be used directly by the application
 * afterwards.
 *
 * @param daemon daemon that manages the connection
 * @param client_socket socket to manage (MHD will expect
 *        to receive an HTTP request from this socket next).
 * @param addr IP address of the client
 * @param addrlen number of bytes in @a addr
 * @param external_add perform additional operations needed due
 *        to the application calling us directly
 * @param non_blck indicate that socket in non-blocking mode
 * @param sk_spipe_supprs indicate that the @a client_socket has
 *                         set SIGPIPE suppression
 * @param sk_is_nonip _MHD_YES if this is not a TCP/IP socket
 * @return #MHD_YES on success, #MHD_NO if this daemon could
 *        not handle the connection (i.e. malloc failed, etc).
 *        The socket will be closed in any case; 'errno' is
 *        set to indicate further details about the error.
 */
static enum MHD_Result
internal_add_connection (struct MHD_Daemon *daemon,
                         MHD_socket client_socket,
                         const struct sockaddr_storage *addr,
                         socklen_t addrlen,
                         bool external_add,
                         bool non_blck,
                         bool sk_spipe_supprs,
                         enum MHD_tristate sk_is_nonip)
{
  struct MHD_Connection *connection;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /* Direct add to master daemon could never happen. */
  mhd_assert (NULL == daemon->worker_pool);
#endif

  if (MHD_D_IS_USING_SELECT_ (daemon) &&
      (! MHD_D_DOES_SCKT_FIT_FDSET_ (client_socket, daemon)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("New connection socket descriptor (%d) is not less " \
                 "than FD_SETSIZE (%d).\n"),
              (int) client_socket,
              (int) MHD_D_GET_FD_SETSIZE_ (daemon));
#endif
    MHD_socket_close_chk_ (client_socket);
#if defined(ENFILE) && (ENFILE + 0 != 0)
    errno = ENFILE;
#endif
    return MHD_NO;
  }

  if (MHD_D_IS_USING_EPOLL_ (daemon) &&
      (! non_blck) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Epoll mode supports only non-blocking sockets\n"));
#endif
    MHD_socket_close_chk_ (client_socket);
#if defined(EINVAL) && (EINVAL + 0 != 0)
    errno = EINVAL;
#endif
    return MHD_NO;
  }

  connection = new_connection_prepare_ (daemon,
                                        client_socket,
                                        addr, addrlen,
                                        external_add,
                                        non_blck,
                                        sk_spipe_supprs,
                                        sk_is_nonip);
  if (NULL == connection)
    return MHD_NO;

  if ((external_add) &&
      MHD_D_IS_THREAD_SAFE_ (daemon))
  {
    /* Connection is added externally and MHD is thread safe mode. */
    MHD_mutex_lock_chk_ (&daemon->new_connections_mutex);
    DLL_insert (daemon->new_connections_head,
                daemon->new_connections_tail,
                connection);
    daemon->have_new = true;
    MHD_mutex_unlock_chk_ (&daemon->new_connections_mutex);

    /* The rest of connection processing must be handled in
     * the daemon thread. */
    if ((MHD_ITC_IS_VALID_ (daemon->itc)) &&
        (! MHD_itc_activate_ (daemon->itc, "n")))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to signal new connection via inter-thread " \
                   "communication channel.\n"));
#endif
    }
    return MHD_YES;
  }

  return new_connection_process_ (daemon, connection);
}


static void
new_connections_list_process_ (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *local_head;
  struct MHD_Connection *local_tail;
  mhd_assert (daemon->have_new);
  mhd_assert (MHD_D_IS_THREAD_SAFE_ (daemon));

  /* Detach DL-list of new connections from the daemon for
   * following local processing. */
  MHD_mutex_lock_chk_ (&daemon->new_connections_mutex);
  mhd_assert (NULL != daemon->new_connections_head);
  local_head = daemon->new_connections_head;
  local_tail = daemon->new_connections_tail;
  daemon->new_connections_head = NULL;
  daemon->new_connections_tail = NULL;
  daemon->have_new = false;
  MHD_mutex_unlock_chk_ (&daemon->new_connections_mutex);
  (void) local_head; /* Mute compiler warning */

  /* Process new connections in FIFO order. */
  do
  {
    struct MHD_Connection *c;   /**< Currently processed connection */

    c = local_tail;
    DLL_remove (local_head,
                local_tail,
                c);
    mhd_assert (daemon == c->daemon);
    if (MHD_NO == new_connection_process_ (daemon, c))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to start serving new connection.\n"));
#endif
      (void) 0;
    }
  } while (NULL != local_tail);

}


/**
 * Internal version of ::MHD_suspend_connection().
 *
 * @remark In thread-per-connection mode: can be called from any thread,
 * in any other mode: to be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param connection the connection to suspend
 */
void
internal_suspend_connection_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  mhd_assert (NULL == daemon->worker_pool);

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if (connection->resuming)
  {
    /* suspending again while we didn't even complete resuming yet */
    connection->resuming = false;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
    return;
  }
  if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
  {
    if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
      XDLL_remove (daemon->normal_timeout_head,
                   daemon->normal_timeout_tail,
                   connection);
    else
      XDLL_remove (daemon->manual_timeout_head,
                   daemon->manual_timeout_tail,
                   connection);
  }
  DLL_remove (daemon->connections_head,
              daemon->connections_tail,
              connection);
  mhd_assert (! connection->suspended);
  DLL_insert (daemon->suspended_connections_head,
              daemon->suspended_connections_tail,
              connection);
  connection->suspended = true;
#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon))
  {
    if (0 != (connection->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
    {
      EDLL_remove (daemon->eready_head,
                   daemon->eready_tail,
                   connection);
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_IN_EREADY_EDLL);
    }
    if (0 != (connection->epoll_state & MHD_EPOLL_STATE_IN_EPOLL_SET))
    {
      if (0 != epoll_ctl (daemon->epoll_fd,
                          EPOLL_CTL_DEL,
                          connection->socket_fd,
                          NULL))
        MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_IN_EPOLL_SET);
    }
    connection->epoll_state |= MHD_EPOLL_STATE_SUSPENDED;
  }
#endif
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
}


/**
 * Suspend handling of network data for a given connection.
 * This can be used to dequeue a connection from MHD's event loop
 * (not applicable to thread-per-connection!) for a while.
 *
 * If you use this API in conjunction with an "internal" socket polling,
 * you must set the option #MHD_USE_ITC to ensure that a resumed
 * connection is immediately processed by MHD.
 *
 * Suspended connections continue to count against the total number of
 * connections allowed (per daemon, as well as per IP, if such limits
 * are set).  Suspended connections will NOT time out; timeouts will
 * restart when the connection handling is resumed.  While a
 * connection is suspended, MHD will not detect disconnects by the
 * client.
 *
 * The only safe way to call this function is to call it from the
 * #MHD_AccessHandlerCallback or #MHD_ContentReaderCallback.
 *
 * Finally, it is an API violation to call #MHD_stop_daemon while
 * having suspended connections (this will at least create memory and
 * socket leaks or lead to undefined behavior).  You must explicitly
 * resume all connections before stopping the daemon.
 *
 * @param connection the connection to suspend
 *
 * @sa #MHD_AccessHandlerCallback
 */
_MHD_EXTERN void
MHD_suspend_connection (struct MHD_Connection *connection)
{
  struct MHD_Daemon *const daemon = connection->daemon;

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
#endif /* MHD_USE_THREADS */

  if (0 == (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME))
    MHD_PANIC (_ ("Cannot suspend connections without " \
                  "enabling MHD_ALLOW_SUSPEND_RESUME!\n"));
#ifdef UPGRADE_SUPPORT
  if (NULL != connection->urh)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Error: connection scheduled for \"upgrade\" cannot " \
                 "be suspended.\n"));
#endif /* HAVE_MESSAGES */
    return;
  }
#endif /* UPGRADE_SUPPORT */
  internal_suspend_connection_ (connection);
}


/**
 * Resume handling of network data for suspended connection.  It is
 * safe to resume a suspended connection at any time.  Calling this
 * function on a connection that was not previously suspended will
 * result in undefined behavior.
 *
 * If you are using this function in "external" sockets polling mode, you must
 * make sure to run #MHD_run() and #MHD_get_timeout() afterwards (before
 * again calling #MHD_get_fdset()), as otherwise the change may not be
 * reflected in the set returned by #MHD_get_fdset() and you may end up
 * with a connection that is stuck until the next network activity.
 *
 * @param connection the connection to resume
 */
_MHD_EXTERN void
MHD_resume_connection (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
#if defined(MHD_USE_THREADS)
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */

  if (0 == (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME))
    MHD_PANIC (_ ("Cannot resume connections without enabling " \
                  "MHD_ALLOW_SUSPEND_RESUME!\n"));
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  connection->resuming = true;
  daemon->resuming = true;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
       (! MHD_itc_activate_ (daemon->itc, "r")) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to signal resume via inter-thread " \
                 "communication channel.\n"));
#endif
  }
}


#ifdef UPGRADE_SUPPORT
/**
 * Mark upgraded connection as closed by application.
 *
 * The @a connection pointer must not be used after call of this function
 * as it may be freed in other thread immediately.
 * @param connection the upgraded connection to mark as closed by application
 */
void
MHD_upgraded_connection_mark_app_closed_ (struct MHD_Connection *connection)
{
  /* Cache 'daemon' here to avoid data races */
  struct MHD_Daemon *const daemon = connection->daemon;
#if defined(MHD_USE_THREADS)
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */
  mhd_assert (NULL != connection->urh);
  mhd_assert (0 != (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME));

  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
  connection->urh->was_closed = true;
  connection->resuming = true;
  daemon->resuming = true;
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
  if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
       (! MHD_itc_activate_ (daemon->itc, "r")) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to signal resume via " \
                 "inter-thread communication channel.\n"));
#endif
  }
}


#endif /* UPGRADE_SUPPORT */

/**
 * Run through the suspended connections and move any that are no
 * longer suspended back to the active state.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon daemon context
 * @return #MHD_YES if a connection was actually resumed
 */
static enum MHD_Result
resume_suspended_connections (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *pos;
  struct MHD_Connection *prev = NULL;
  enum MHD_Result ret;
  const bool used_thr_p_c = (0 != (daemon->options
                                   & MHD_USE_THREAD_PER_CONNECTION));
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  mhd_assert (NULL == daemon->worker_pool);
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
#endif

  ret = MHD_NO;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif

  if (daemon->resuming)
  {
    prev = daemon->suspended_connections_tail;
    /* During shutdown check for resuming is forced. */
    mhd_assert ((NULL != prev) || (daemon->shutdown) || \
                (0 != (daemon->options & MHD_ALLOW_UPGRADE)));
  }

  daemon->resuming = false;

  while (NULL != (pos = prev))
  {
#ifdef UPGRADE_SUPPORT
    struct MHD_UpgradeResponseHandle *const urh = pos->urh;
#else  /* ! UPGRADE_SUPPORT */
    static const void *const urh = NULL;
#endif /* ! UPGRADE_SUPPORT */
    prev = pos->prev;
    if ( (! pos->resuming)
#ifdef UPGRADE_SUPPORT
         || ( (NULL != urh) &&
              ( (! urh->was_closed) ||
                (! urh->clean_ready) ) )
#endif /* UPGRADE_SUPPORT */
         )
      continue;
    ret = MHD_YES;
    mhd_assert (pos->suspended);
    DLL_remove (daemon->suspended_connections_head,
                daemon->suspended_connections_tail,
                pos);
    pos->suspended = false;
    if (NULL == urh)
    {
      DLL_insert (daemon->connections_head,
                  daemon->connections_tail,
                  pos);
      if (! used_thr_p_c)
      {
        /* Reset timeout timer on resume. */
        if (0 != pos->connection_timeout_ms)
          pos->last_activity = MHD_monotonic_msec_counter ();

        if (pos->connection_timeout_ms == daemon->connection_timeout_ms)
          XDLL_insert (daemon->normal_timeout_head,
                       daemon->normal_timeout_tail,
                       pos);
        else
          XDLL_insert (daemon->manual_timeout_head,
                       daemon->manual_timeout_tail,
                       pos);
      }
#ifdef EPOLL_SUPPORT
      if (MHD_D_IS_USING_EPOLL_ (daemon))
      {
        if (0 != (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
          MHD_PANIC ("Resumed connection was already in EREADY set.\n");
        /* we always mark resumed connections as ready, as we
           might have missed the edge poll event during suspension */
        EDLL_insert (daemon->eready_head,
                     daemon->eready_tail,
                     pos);
        pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL   \
                            | MHD_EPOLL_STATE_READ_READY
                            | MHD_EPOLL_STATE_WRITE_READY;
        pos->epoll_state &= ~((enum MHD_EpollState) MHD_EPOLL_STATE_SUSPENDED);
      }
#endif
    }
#ifdef UPGRADE_SUPPORT
    else
    {
      /* Data forwarding was finished (for TLS connections) AND
       * application was closed upgraded connection.
       * Insert connection into cleanup list. */

      if ( (NULL != daemon->notify_completed) &&
           (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon)) &&
           (pos->rq.client_aware) )
      {
        daemon->notify_completed (daemon->notify_completed_cls,
                                  pos,
                                  &pos->rq.client_context,
                                  MHD_REQUEST_TERMINATED_COMPLETED_OK);
        pos->rq.client_aware = false;
      }
      DLL_insert (daemon->cleanup_head,
                  daemon->cleanup_tail,
                  pos);
      daemon->data_already_pending = true;
    }
#endif /* UPGRADE_SUPPORT */
    pos->resuming = false;
  }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if ( (used_thr_p_c) &&
       (MHD_NO != ret) )
  {   /* Wake up suspended connections. */
    if (! MHD_itc_activate_ (daemon->itc,
                             "w"))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to signal resume of connection via " \
                   "inter-thread communication channel.\n"));
#endif
    }
  }
  return ret;
}


/**
 * Add another client connection to the set of connections managed by
 * MHD.  This API is usually not needed (since MHD will accept inbound
 * connections on the server socket).  Use this API in special cases,
 * for example if your HTTP server is behind NAT and needs to connect
 * out to the HTTP client, or if you are building a proxy.
 *
 * If you use this API in conjunction with a internal select or a
 * thread pool, you must set the option
 * #MHD_USE_ITC to ensure that the freshly added
 * connection is immediately processed by MHD.
 *
 * The given client socket will be managed (and closed!) by MHD after
 * this call and must no longer be used directly by the application
 * afterwards.
 *
 * @param daemon daemon that manages the connection
 * @param client_socket socket to manage (MHD will expect
 *        to receive an HTTP request from this socket next).
 * @param addr IP address of the client
 * @param addrlen number of bytes in @a addr
 * @return #MHD_YES on success, #MHD_NO if this daemon could
 *        not handle the connection (i.e. malloc() failed, etc).
 *        The socket will be closed in any case; `errno` is
 *        set to indicate further details about the error.
 * @ingroup specialized
 */
_MHD_EXTERN enum MHD_Result
MHD_add_connection (struct MHD_Daemon *daemon,
                    MHD_socket client_socket,
                    const struct sockaddr *addr,
                    socklen_t addrlen)
{
  bool sk_nonbl;
  bool sk_spipe_supprs;
  struct sockaddr_storage addrstorage;

  /* TODO: fix atomic value reading */
  if ((! MHD_D_IS_THREAD_SAFE_ (daemon)) &&
      (daemon->connection_limit <= daemon->connections))
    MHD_cleanup_connections (daemon);

#ifdef HAVE_MESSAGES
  if (MHD_D_IS_USING_THREADS_ (daemon) &&
      (0 == (daemon->options & MHD_USE_ITC)))
  {
    MHD_DLOG (daemon,
              _ ("MHD_add_connection() has been called for daemon started"
                 " without MHD_USE_ITC flag.\nDaemon will not process newly"
                 " added connection until any activity occurs in already"
                 " added sockets.\n"));
  }
#endif /* HAVE_MESSAGES */
  if (0 != addrlen)
  {
    if (AF_INET == addr->sa_family)
    {
      if (sizeof(struct sockaddr_in) > (size_t) addrlen)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("MHD_add_connection() has been called with "
                     "incorrect 'addrlen' value.\n"));
#endif /* HAVE_MESSAGES */
        return MHD_NO;
      }
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      if ((0 != addr->sa_len) &&
          (sizeof(struct sockaddr_in) > (size_t) addr->sa_len) )
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("MHD_add_connection() has been called with " \
                     "non-zero value of 'sa_len' member of " \
                     "'struct sockaddr' which does not match 'sa_family'.\n"));
#endif /* HAVE_MESSAGES */
        return MHD_NO;
      }
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
    }
#ifdef HAVE_INET6
    if (AF_INET6 == addr->sa_family)
    {
      if (sizeof(struct sockaddr_in6) > (size_t) addrlen)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("MHD_add_connection() has been called with "
                     "incorrect 'addrlen' value.\n"));
#endif /* HAVE_MESSAGES */
        return MHD_NO;
      }
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      if ((0 != addr->sa_len) &&
          (sizeof(struct sockaddr_in6) > (size_t) addr->sa_len) )
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("MHD_add_connection() has been called with " \
                     "non-zero value of 'sa_len' member of " \
                     "'struct sockaddr' which does not match 'sa_family'.\n"));
#endif /* HAVE_MESSAGES */
        return MHD_NO;
      }
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
    }
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
    if ((0 != addr->sa_len) &&
        (addrlen > addr->sa_len))
      addrlen = (socklen_t) addr->sa_len;   /* Use safest value */
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
#endif /* HAVE_INET6 */
  }

  if (! MHD_socket_nonblocking_ (client_socket))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to set nonblocking mode on new client socket: %s\n"),
              MHD_socket_last_strerr_ ());
#endif
    sk_nonbl = false;
  }
  else
    sk_nonbl = true;

#ifndef MHD_WINSOCK_SOCKETS
  sk_spipe_supprs = false;
#else  /* MHD_WINSOCK_SOCKETS */
  sk_spipe_supprs = true; /* Nothing to suppress on W32 */
#endif /* MHD_WINSOCK_SOCKETS */
#if defined(MHD_socket_nosignal_)
  if (! sk_spipe_supprs)
    sk_spipe_supprs = MHD_socket_nosignal_ (client_socket);
  if (! sk_spipe_supprs)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to suppress SIGPIPE on new client socket: %s\n"),
              MHD_socket_last_strerr_ ());
#else  /* ! HAVE_MESSAGES */
    (void) 0; /* Mute compiler warning */
#endif /* ! HAVE_MESSAGES */
#ifndef MSG_NOSIGNAL
    /* Application expects that SIGPIPE will be suppressed,
     * but suppression failed and SIGPIPE cannot be suppressed with send(). */
    if (! daemon->sigpipe_blocked)
    {
      int err = MHD_socket_get_error_ ();
      MHD_socket_close_ (client_socket);
      MHD_socket_fset_error_ (err);
      return MHD_NO;
    }
#endif /* MSG_NOSIGNAL */
  }
#endif /* MHD_socket_nosignal_ */

  if ( (0 != (daemon->options & MHD_USE_TURBO)) &&
       (! MHD_socket_noninheritable_ (client_socket)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to set noninheritable mode on new client socket.\n"));
#endif
  }

  /* Copy to sockaddr_storage structure to avoid alignment problems */
  if (0 < addrlen)
    memcpy (&addrstorage, addr, (size_t) addrlen);
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN
  addrstorage.ss_len = addrlen; /* Force set the right length */
#endif /* HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN */

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (NULL != daemon->worker_pool)
  {
    unsigned int i;
    /* have a pool, try to find a pool with capacity; we use the
       socket as the initial offset into the pool for load
       balancing */
    for (i = 0; i < daemon->worker_pool_size; ++i)
    {
      struct MHD_Daemon *const worker =
        &daemon->worker_pool[(i + (unsigned int) client_socket)
                             % daemon->worker_pool_size];
      if (worker->connections < worker->connection_limit)
        return internal_add_connection (worker,
                                        client_socket,
                                        &addrstorage,
                                        addrlen,
                                        true,
                                        sk_nonbl,
                                        sk_spipe_supprs,
                                        _MHD_UNKNOWN);
    }
    /* all pools are at their connection limit, must refuse */
    MHD_socket_close_chk_ (client_socket);
#if defined(ENFILE) && (ENFILE + 0 != 0)
    errno = ENFILE;
#endif
    return MHD_NO;
  }
#endif /* MHD_USE_POSIX_THREADS || MHD_USE_W32_THREADS */

  return internal_add_connection (daemon,
                                  client_socket,
                                  &addrstorage,
                                  addrlen,
                                  true,
                                  sk_nonbl,
                                  sk_spipe_supprs,
                                  _MHD_UNKNOWN);
}


/**
 * Accept an incoming connection and create the MHD_Connection object for
 * it.  This function also enforces policy by way of checking with the
 * accept policy callback.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon handle with the listen socket
 * @return #MHD_YES on success (connections denied by policy or due
 *         to 'out of memory' and similar errors) are still considered
 *         successful as far as #MHD_accept_connection() is concerned);
 *         a return code of #MHD_NO only refers to the actual
 *         accept() system call.
 */
static enum MHD_Result
MHD_accept_connection (struct MHD_Daemon *daemon)
{
  struct sockaddr_storage addrstorage;
  socklen_t addrlen;
  MHD_socket s;
  MHD_socket fd;
  bool sk_nonbl;
  bool sk_spipe_supprs;
  bool sk_cloexec;
  enum MHD_tristate sk_non_ip;
#if defined(_DEBUG) && defined (USE_ACCEPT4)
  const bool use_accept4 = ! daemon->avoid_accept4;
#elif defined (USE_ACCEPT4)
  static const bool use_accept4 = true;
#else  /* ! USE_ACCEPT4 && ! _DEBUG */
  static const bool use_accept4 = false;
#endif /* ! USE_ACCEPT4 && ! _DEBUG */

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */

  if ( (MHD_INVALID_SOCKET == (fd = daemon->listen_fd)) ||
       (daemon->was_quiesced) )
    return MHD_NO;

  addrlen = (socklen_t) sizeof (addrstorage);
  memset (&addrstorage,
          0,
          (size_t) addrlen);
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN
  addrstorage.ss_len = addrlen;
#endif /* HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN */

  /* Initialise with default values to avoid compiler warnings */
  sk_nonbl = false;
  sk_spipe_supprs = false;
  sk_cloexec = false;
  s = MHD_INVALID_SOCKET;

#ifdef USE_ACCEPT4
  if (use_accept4 &&
      (MHD_INVALID_SOCKET !=
       (s = accept4 (fd,
                     (struct sockaddr *) &addrstorage,
                     &addrlen,
                     SOCK_CLOEXEC_OR_ZERO | SOCK_NONBLOCK_OR_ZERO
                     | SOCK_NOSIGPIPE_OR_ZERO))))
  {
    sk_nonbl = (SOCK_NONBLOCK_OR_ZERO != 0);
#ifndef MHD_WINSOCK_SOCKETS
    sk_spipe_supprs = (SOCK_NOSIGPIPE_OR_ZERO != 0);
#else  /* MHD_WINSOCK_SOCKETS */
    sk_spipe_supprs = true; /* Nothing to suppress on W32 */
#endif /* MHD_WINSOCK_SOCKETS */
    sk_cloexec = (SOCK_CLOEXEC_OR_ZERO != 0);
  }
#endif /* USE_ACCEPT4 */
#if defined(_DEBUG) || ! defined(USE_ACCEPT4)
  if (! use_accept4 &&
      (MHD_INVALID_SOCKET !=
       (s = accept (fd,
                    (struct sockaddr *) &addrstorage,
                    &addrlen))))
  {
#ifdef MHD_ACCEPT_INHERIT_NONBLOCK
    sk_nonbl = daemon->listen_nonblk;
#else  /* ! MHD_ACCEPT_INHERIT_NONBLOCK */
    sk_nonbl = false;
#endif /* ! MHD_ACCEPT_INHERIT_NONBLOCK */
#ifndef MHD_WINSOCK_SOCKETS
    sk_spipe_supprs = false;
#else  /* MHD_WINSOCK_SOCKETS */
    sk_spipe_supprs = true; /* Nothing to suppress on W32 */
#endif /* MHD_WINSOCK_SOCKETS */
    sk_cloexec = false;
  }
#endif /* _DEBUG || !USE_ACCEPT4 */

  if (MHD_INVALID_SOCKET == s)
  {
    const int err = MHD_socket_get_error_ ();

    /* This could be a common occurrence with multiple worker threads */
    if (MHD_SCKT_ERR_IS_ (err,
                          MHD_SCKT_EINVAL_))
      return MHD_NO;   /* can happen during shutdown */
    if (MHD_SCKT_ERR_IS_DISCNN_BEFORE_ACCEPT_ (err))
      return MHD_NO;   /* do not print error if client just disconnected early */
#ifdef HAVE_MESSAGES
    if (! MHD_SCKT_ERR_IS_EAGAIN_ (err) )
      MHD_DLOG (daemon,
                _ ("Error accepting connection: %s\n"),
                MHD_socket_strerr_ (err));
#endif
    if (MHD_SCKT_ERR_IS_LOW_RESOURCES_ (err) )
    {
      /* system/process out of resources */
      if (0 == daemon->connections)
      {
#ifdef HAVE_MESSAGES
        /* Not setting 'at_limit' flag, as there is no way it
           would ever be cleared.  Instead trying to produce
           bit fat ugly warning. */
        MHD_DLOG (daemon,
                  _ ("Hit process or system resource limit at FIRST " \
                     "connection. This is really bad as there is no sane " \
                     "way to proceed. Will try busy waiting for system " \
                     "resources to become magically available.\n"));
#endif
      }
      else
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
        daemon->at_limit = true;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Hit process or system resource limit at %u " \
                     "connections, temporarily suspending accept(). " \
                     "Consider setting a lower MHD_OPTION_CONNECTION_LIMIT.\n"),
                  (unsigned int) daemon->connections);
#endif
      }
    }
    return MHD_NO;
  }

  sk_non_ip = daemon->listen_is_unix;
  if (0 >= addrlen)
  {
#ifdef HAVE_MESSAGES
    if (_MHD_NO != daemon->listen_is_unix)
      MHD_DLOG (daemon,
                _ ("Accepted socket has zero-length address. "
                   "Processing the new socket as a socket with " \
                   "unknown type.\n"));
#endif
    addrlen = 0;
    sk_non_ip = _MHD_YES; /* IP-type addresses have non-zero length */
  }
  if (((socklen_t) sizeof (addrstorage)) < addrlen)
  {
    /* Should not happen as 'sockaddr_storage' must be large enough to
     * store any address supported by the system. */
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Accepted socket address is larger than expected by " \
                 "system headers. Processing the new socket as a socket with " \
                 "unknown type.\n"));
#endif
    addrlen = 0;
    sk_non_ip = _MHD_YES; /* IP-type addresses must fit */
  }

  if (! sk_nonbl && ! MHD_socket_nonblocking_ (s))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to set nonblocking mode on incoming connection " \
                 "socket: %s\n"),
              MHD_socket_last_strerr_ ());
#else  /* ! HAVE_MESSAGES */
    (void) 0; /* Mute compiler warning */
#endif /* ! HAVE_MESSAGES */
  }
  else
    sk_nonbl = true;

  if (! sk_cloexec && ! MHD_socket_noninheritable_ (s))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to set noninheritable mode on incoming connection " \
                 "socket.\n"));
#else  /* ! HAVE_MESSAGES */
    (void) 0; /* Mute compiler warning */
#endif /* ! HAVE_MESSAGES */
  }

#if defined(MHD_socket_nosignal_)
  if (! sk_spipe_supprs && ! MHD_socket_nosignal_ (s))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to suppress SIGPIPE on incoming connection " \
                 "socket: %s\n"),
              MHD_socket_last_strerr_ ());
#else  /* ! HAVE_MESSAGES */
    (void) 0; /* Mute compiler warning */
#endif /* ! HAVE_MESSAGES */
#ifndef MSG_NOSIGNAL
    /* Application expects that SIGPIPE will be suppressed,
     * but suppression failed and SIGPIPE cannot be suppressed with send(). */
    if (! daemon->sigpipe_blocked)
    {
      (void) MHD_socket_close_ (s);
      return MHD_NO;
    }
#endif /* MSG_NOSIGNAL */
  }
  else
    sk_spipe_supprs = true;
#endif /* MHD_socket_nosignal_ */
#ifdef HAVE_MESSAGES
#if _MHD_DEBUG_CONNECT
  MHD_DLOG (daemon,
            _ ("Accepted connection on socket %d\n"),
            s);
#endif
#endif
  (void) internal_add_connection (daemon,
                                  s,
                                  &addrstorage,
                                  addrlen,
                                  false,
                                  sk_nonbl,
                                  sk_spipe_supprs,
                                  sk_non_ip);
  return MHD_YES;
}


/**
 * Free resources associated with all closed connections.
 * (destroy responses, free buffers, etc.).  All closed
 * connections are kept in the "cleanup" doubly-linked list.
 * @remark To be called only from thread that
 * process daemon's select()/poll()/etc.
 *
 * @param daemon daemon to clean up
 */
static void
MHD_cleanup_connections (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *pos;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
  mhd_assert (NULL == daemon->worker_pool);

  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  while (NULL != (pos = daemon->cleanup_tail))
  {
    DLL_remove (daemon->cleanup_head,
                daemon->cleanup_tail,
                pos);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
    if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) &&
        (! pos->thread_joined) &&
        (! MHD_thread_handle_ID_join_thread_ (pos->tid)) )
      MHD_PANIC (_ ("Failed to join a thread.\n"));
#endif
#ifdef UPGRADE_SUPPORT
    cleanup_upgraded_connection (pos);
#endif /* UPGRADE_SUPPORT */
    MHD_pool_destroy (pos->pool);
#ifdef HTTPS_SUPPORT
    if (NULL != pos->tls_session)
      gnutls_deinit (pos->tls_session);
#endif /* HTTPS_SUPPORT */

    /* clean up the connection */
    if (NULL != daemon->notify_connection)
      daemon->notify_connection (daemon->notify_connection_cls,
                                 pos,
                                 &pos->socket_context,
                                 MHD_CONNECTION_NOTIFY_CLOSED);
    MHD_ip_limit_del (daemon,
                      pos->addr,
                      pos->addr_len);
#ifdef EPOLL_SUPPORT
    if (MHD_D_IS_USING_EPOLL_ (daemon))
    {
      if (0 != (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
      {
        EDLL_remove (daemon->eready_head,
                     daemon->eready_tail,
                     pos);
        pos->epoll_state &=
          ~((enum MHD_EpollState) MHD_EPOLL_STATE_IN_EREADY_EDLL);
      }
      if ( (-1 != daemon->epoll_fd) &&
           (0 != (pos->epoll_state & MHD_EPOLL_STATE_IN_EPOLL_SET)) )
      {
        /* epoll documentation suggests that closing a FD
           automatically removes it from the epoll set; however,
           this is not true as if we fail to do manually remove it,
           we are still seeing an event for this fd in epoll,
           causing grief (use-after-free...) --- at least on my
           system. */
        if (0 != epoll_ctl (daemon->epoll_fd,
                            EPOLL_CTL_DEL,
                            pos->socket_fd,
                            NULL))
          MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
        pos->epoll_state &=
          ~((enum MHD_EpollState)
            MHD_EPOLL_STATE_IN_EPOLL_SET);
      }
    }
#endif
    if (NULL != pos->rp.response)
    {
      MHD_destroy_response (pos->rp.response);
      pos->rp.response = NULL;
    }
    if (MHD_INVALID_SOCKET != pos->socket_fd)
      MHD_socket_close_chk_ (pos->socket_fd);
    if (NULL != pos->addr)
      free (pos->addr);
    free (pos);

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
    daemon->connections--;
    daemon->at_limit = false;
  }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
}


/**
 * Obtain timeout value for polling function for this daemon.
 *
 * This function set value to the amount of milliseconds for which polling
 * function (`select()`, `poll()` or epoll) should at most block, not the
 * timeout value set for connections.
 *
 * Any "external" sockets polling function must be called with the timeout
 * value provided by this function. Smaller timeout values can be used for
 * polling function if it is required for any reason, but using larger
 * timeout value or no timeout (indefinite timeout) when this function
 * return #MHD_YES will break MHD processing logic and result in "hung"
 * connections with data pending in network buffers and other problems.
 *
 * It is important to always use this function (or #MHD_get_timeout64(),
 * #MHD_get_timeout64s(), #MHD_get_timeout_i() functions) when "external"
 * polling is used.
 * If this function returns #MHD_YES then #MHD_run() (or #MHD_run_from_select())
 * must be called right after return from polling function, regardless of
 * the states of MHD FDs.
 *
 * In practice, if #MHD_YES is returned then #MHD_run() (or
 * #MHD_run_from_select()) must be called not later than @a timeout
 * millisecond even if no activity is detected on sockets by sockets
 * polling function.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon daemon to query for timeout
 * @param[out] timeout set to the timeout (in milliseconds)
 * @return #MHD_YES on success, #MHD_NO if timeouts are
 *         not used and no data processing is pending.
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_get_timeout (struct MHD_Daemon *daemon,
                 MHD_UNSIGNED_LONG_LONG *timeout)
{
  uint64_t t64;
  if (MHD_NO == MHD_get_timeout64 (daemon, &t64))
    return MHD_NO;

#if SIZEOF_UINT64_T > SIZEOF_UNSIGNED_LONG_LONG
  if (ULLONG_MAX <= t64)
    *timeout = ULLONG_MAX;
  else
#endif /* SIZEOF_UINT64_T > SIZEOF_UNSIGNED_LONG_LONG */
  *timeout = (MHD_UNSIGNED_LONG_LONG) t64;
  return MHD_YES;
}


/**
 * Obtain timeout value for external polling function for this daemon.
 *
 * This function set value to the amount of milliseconds for which polling
 * function (`select()`, `poll()` or epoll) should at most block, not the
 * timeout value set for connections.
 *
 * Any "external" sockets polling function must be called with the timeout
 * value provided by this function. Smaller timeout values can be used for
 * polling function if it is required for any reason, but using larger
 * timeout value or no timeout (indefinite timeout) when this function
 * return #MHD_YES will break MHD processing logic and result in "hung"
 * connections with data pending in network buffers and other problems.
 *
 * It is important to always use this function (or #MHD_get_timeout(),
 * #MHD_get_timeout64s(), #MHD_get_timeout_i() functions) when "external"
 * polling is used.
 * If this function returns #MHD_YES then #MHD_run() (or #MHD_run_from_select())
 * must be called right after return from polling function, regardless of
 * the states of MHD FDs.
 *
 * In practice, if #MHD_YES is returned then #MHD_run() (or
 * #MHD_run_from_select()) must be called not later than @a timeout
 * millisecond even if no activity is detected on sockets by sockets
 * polling function.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon daemon to query for timeout
 * @param[out] timeout64 the pointer to the variable to be set to the
 *                  timeout (in milliseconds)
 * @return #MHD_YES if timeout value has been set,
 *         #MHD_NO if timeouts are not used and no data processing is pending.
 * @note Available since #MHD_VERSION 0x00097701
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_get_timeout64 (struct MHD_Daemon *daemon,
                   uint64_t *timeout64)
{
  uint64_t earliest_deadline;
  struct MHD_Connection *pos;
  struct MHD_Connection *earliest_tmot_conn; /**< the connection with earliest timeout */

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
#endif /* MHD_USE_THREADS */

  if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Illegal call to MHD_get_timeout.\n"));
#endif
    return MHD_NO;
  }
  if (daemon->data_already_pending
      || (NULL != daemon->cleanup_head)
      || daemon->resuming
      || daemon->have_new
      || daemon->shutdown)
  {
    /* Some data or connection statuses already waiting to be processed. */
    *timeout64 = 0;
    return MHD_YES;
  }
#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon) &&
      ((NULL != daemon->eready_head)
#if defined(UPGRADE_SUPPORT) && defined(HTTPS_SUPPORT)
       || (NULL != daemon->eready_urh_head)
#endif /* UPGRADE_SUPPORT && HTTPS_SUPPORT */
      ) )
  {
    /* Some connection(s) already have some data pending. */
    *timeout64 = 0;
    return MHD_YES;
  }
#endif /* EPOLL_SUPPORT */

  earliest_tmot_conn = NULL;
  earliest_deadline = 0; /* mute compiler warning */
  /* normal timeouts are sorted, so we only need to look at the 'tail' (oldest) */
  pos = daemon->normal_timeout_tail;
  if ( (NULL != pos) &&
       (0 != pos->connection_timeout_ms) )
  {
    earliest_tmot_conn = pos;
    earliest_deadline = pos->last_activity + pos->connection_timeout_ms;
  }

  for (pos = daemon->manual_timeout_tail; NULL != pos; pos = pos->prevX)
  {
    if (0 != pos->connection_timeout_ms)
    {
      if ( (NULL == earliest_tmot_conn) ||
           (earliest_deadline - pos->last_activity >
            pos->connection_timeout_ms) )
      {
        earliest_tmot_conn = pos;
        earliest_deadline = pos->last_activity + pos->connection_timeout_ms;
      }
    }
  }

  if (NULL != earliest_tmot_conn)
  {
    *timeout64 = connection_get_wait (earliest_tmot_conn);
    return MHD_YES;
  }
  return MHD_NO;
}


#if defined(HAVE_POLL) || defined(EPOLL_SUPPORT)
/**
 * Obtain timeout value for external polling function for this daemon.
 *
 * This function set value to the amount of milliseconds for which polling
 * function (`select()`, `poll()` or epoll) should at most block, not the
 * timeout value set for connections.
 *
 * Any "external" sockets polling function must be called with the timeout
 * value provided by this function (if returned value is non-negative).
 * Smaller timeout values can be used for polling function if it is required
 * for any reason, but using larger timeout value or no timeout (indefinite
 * timeout) when this function returns non-negative value will break MHD
 * processing logic and result in "hung" connections with data pending in
 * network buffers and other problems.
 *
 * It is important to always use this function (or #MHD_get_timeout(),
 * #MHD_get_timeout64(), #MHD_get_timeout_i() functions) when "external"
 * polling is used.
 * If this function returns non-negative value then #MHD_run() (or
 * #MHD_run_from_select()) must be called right after return from polling
 * function, regardless of the states of MHD FDs.
 *
 * In practice, if zero or positive value is returned then #MHD_run() (or
 * #MHD_run_from_select()) must be called not later than returned amount of
 * millisecond even if no activity is detected on sockets by sockets
 * polling function.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon the daemon to query for timeout
 * @return -1 if connections' timeouts are not set and no data processing
 *         is pending, so external polling function may wait for sockets
 *         activity for indefinite amount of time,
 *         otherwise returned value is the the maximum amount of millisecond
 *         that external polling function must wait for the activity of FDs.
 * @note Available since #MHD_VERSION 0x00097701
 * @ingroup event
 */
_MHD_EXTERN int64_t
MHD_get_timeout64s (struct MHD_Daemon *daemon)
{
  uint64_t utimeout;
  if (MHD_NO == MHD_get_timeout64 (daemon, &utimeout))
    return -1;
  if (INT64_MAX < utimeout)
    return INT64_MAX;

  return (int64_t) utimeout;
}


/**
 * Obtain timeout value for external polling function for this daemon.
 *
 * This function set value to the amount of milliseconds for which polling
 * function (`select()`, `poll()` or epoll) should at most block, not the
 * timeout value set for connections.
 *
 * Any "external" sockets polling function must be called with the timeout
 * value provided by this function (if returned value is non-negative).
 * Smaller timeout values can be used for polling function if it is required
 * for any reason, but using larger timeout value or no timeout (indefinite
 * timeout) when this function returns non-negative value will break MHD
 * processing logic and result in "hung" connections with data pending in
 * network buffers and other problems.
 *
 * It is important to always use this function (or #MHD_get_timeout(),
 * #MHD_get_timeout64(), #MHD_get_timeout64s() functions) when "external"
 * polling is used.
 * If this function returns non-negative value then #MHD_run() (or
 * #MHD_run_from_select()) must be called right after return from polling
 * function, regardless of the states of MHD FDs.
 *
 * In practice, if zero or positive value is returned then #MHD_run() (or
 * #MHD_run_from_select()) must be called not later than returned amount of
 * millisecond even if no activity is detected on sockets by sockets
 * polling function.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon the daemon to query for timeout
 * @return -1 if connections' timeouts are not set and no data processing
 *         is pending, so external polling function may wait for sockets
 *         activity for indefinite amount of time,
 *         otherwise returned value is the the maximum amount of millisecond
 *         (capped at INT_MAX) that external polling function must wait
 *         for the activity of FDs.
 * @note Available since #MHD_VERSION 0x00097701
 * @ingroup event
 */
_MHD_EXTERN int
MHD_get_timeout_i (struct MHD_Daemon *daemon)
{
#if SIZEOF_INT >= SIZEOF_INT64_T
  return MHD_get_timeout64s (daemon);
#else  /* SIZEOF_INT < SIZEOF_INT64_T */
  const int64_t to64 = MHD_get_timeout64s (daemon);
  if (INT_MAX >= to64)
    return (int) to64;
  return INT_MAX;
#endif /* SIZEOF_INT < SIZEOF_INT64_T */
}


/**
 * Obtain timeout value for polling function for this daemon.
 * @remark To be called only from the thread that processes
 * daemon's select()/poll()/etc.
 *
 * @param daemon the daemon to query for timeout
 * @param max_timeout the maximum return value (in milliseconds),
 *                    ignored if set to '-1'
 * @return timeout value in milliseconds or -1 if no timeout is expected.
 */
static int64_t
get_timeout_millisec_ (struct MHD_Daemon *daemon,
                       int32_t max_timeout)
{
  uint64_t d_timeout;
  mhd_assert (0 <= max_timeout || -1 == max_timeout);
  if (0 == max_timeout)
    return 0;

  if (MHD_NO == MHD_get_timeout64 (daemon, &d_timeout))
    return max_timeout;

  if ((0 < max_timeout) && ((uint64_t) max_timeout < d_timeout))
    return max_timeout;

  if (INT64_MAX <= d_timeout)
    return INT64_MAX;

  return (int64_t) d_timeout;
}


/**
 * Obtain timeout value for polling function for this daemon.
 * @remark To be called only from the thread that processes
 * daemon's select()/poll()/etc.
 *
 * @param daemon the daemon to query for timeout
 * @param max_timeout the maximum return value (in milliseconds),
 *                    ignored if set to '-1'
 * @return timeout value in milliseconds, capped to INT_MAX, or
 *         -1 if no timeout is expected.
 */
static int
get_timeout_millisec_int (struct MHD_Daemon *daemon,
                          int32_t max_timeout)
{
  int64_t res;

  res = get_timeout_millisec_ (daemon, max_timeout);
#if SIZEOF_INT < SIZEOF_INT64_T
  if (INT_MAX <= res)
    return INT_MAX;
#endif /* SIZEOF_INT < SIZEOF_INT64_T */
  return (int) res;
}


#endif /* HAVE_POLL || EPOLL_SUPPORT */

/**
 * Internal version of #MHD_run_from_select().
 *
 * @param daemon daemon to run select loop for
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @param fd_setsize value of FD_SETSIZE used when fd_sets were created
 * @return #MHD_NO on serious errors, #MHD_YES on success
 * @ingroup event
 */
static enum MHD_Result
internal_run_from_select (struct MHD_Daemon *daemon,
                          const fd_set *read_fd_set,
                          const fd_set *write_fd_set,
                          const fd_set *except_fd_set,
                          int fd_setsize)
{
  MHD_socket ds;
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  struct MHD_UpgradeResponseHandle *urh;
  struct MHD_UpgradeResponseHandle *urhn;
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */

  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_valid_ID_ (daemon->tid)));
  mhd_assert ((0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (! MHD_thread_handle_ID_is_valid_ID_ (daemon->tid)));
  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_current_thread_ (daemon->tid)));

  mhd_assert (0 < fd_setsize);
  (void) fd_setsize; /* Mute compiler warning */
#ifndef HAS_FD_SETSIZE_OVERRIDABLE
  (void) fd_setsize; /* Mute compiler warning */
  mhd_assert (((int) FD_SETSIZE) <= fd_setsize);
  fd_setsize = FD_SETSIZE; /* Help compiler to optimise */
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

  /* Clear ITC to avoid spinning select */
  /* Do it before any other processing so new signals
     will trigger select again and will be processed */
  if (MHD_ITC_IS_VALID_ (daemon->itc))
  { /* Have ITC */
    bool need_to_clear_itc = true; /* ITC is always non-blocking, it is safe to clear even if ITC not activated */
    if (MHD_SCKT_FD_FITS_FDSET_SETSIZE_ (MHD_itc_r_fd_ (daemon->itc),
                                         NULL, fd_setsize))
      need_to_clear_itc = FD_ISSET (MHD_itc_r_fd_ (daemon->itc), \
                                    (fd_set *) _MHD_DROP_CONST (read_fd_set)); /* Skip clearing, if not needed */
    if (need_to_clear_itc)
      MHD_itc_clear_ (daemon->itc);
  }

  /* Reset. New value will be set when connections are processed. */
  /* Note: no-op for thread-per-connection as it is always false in that mode. */
  daemon->data_already_pending = false;

  /* Process externally added connection if any */
  if (daemon->have_new)
    new_connections_list_process_ (daemon);

  /* select connection thread handling type */
  ds = daemon->listen_fd;
  if ( (MHD_INVALID_SOCKET != ds) &&
       (! daemon->was_quiesced) )
  {
    bool need_to_accept;
    if (MHD_SCKT_FD_FITS_FDSET_SETSIZE_ (ds, NULL, fd_setsize))
      need_to_accept = FD_ISSET (ds,
                                 (fd_set *) _MHD_DROP_CONST (read_fd_set));
    else                                       /* Cannot check whether new connection are pending */
      need_to_accept = daemon->listen_nonblk;  /* Try to accept if non-blocking */

    if (need_to_accept)
      (void) MHD_accept_connection (daemon);
  }

  if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
  {
    /* do not have a thread per connection, process all connections now */
    struct MHD_Connection *pos;
    for (pos = daemon->connections_tail; NULL != pos; pos = pos->prev)
    {
      MHD_socket cs;
      bool r_ready;
      bool w_ready;
      bool has_err;

      cs = pos->socket_fd;
      if (MHD_INVALID_SOCKET == cs)
        continue;

      if (MHD_SCKT_FD_FITS_FDSET_SETSIZE_ (cs, NULL, fd_setsize))
      {
        r_ready = FD_ISSET (cs,
                            (fd_set *) _MHD_DROP_CONST (read_fd_set));
        w_ready = FD_ISSET (cs,
                            (fd_set *) _MHD_DROP_CONST (write_fd_set));
        has_err = (NULL != except_fd_set) &&
                  FD_ISSET (cs,
                            (fd_set *) _MHD_DROP_CONST (except_fd_set));
      }
      else
      { /* Cannot check the real readiness */
        r_ready = pos->sk_nonblck;
        w_ready = r_ready;
        has_err = false;
      }
      call_handlers (pos,
                     r_ready,
                     w_ready,
                     has_err);
    }
  }

#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  /* handle upgraded HTTPS connections */
  for (urh = daemon->urh_tail; NULL != urh; urh = urhn)
  {
    urhn = urh->prev;
    /* update urh state based on select() output */
    urh_from_fdset (urh,
                    read_fd_set,
                    write_fd_set,
                    except_fd_set,
                    fd_setsize);
    /* call generic forwarding function for passing data */
    process_urh (urh);
    /* Finished forwarding? */
    if ( (0 == urh->in_buffer_size) &&
         (0 == urh->out_buffer_size) &&
         (0 == urh->in_buffer_used) &&
         (0 == urh->out_buffer_used) )
    {
      MHD_connection_finish_forward_ (urh->connection);
      urh->clean_ready = true;
      /* Resuming will move connection to cleanup list. */
      MHD_resume_connection (urh->connection);
    }
  }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  MHD_cleanup_connections (daemon);
  return MHD_YES;
}


#undef MHD_run_from_select

/**
 * Run webserver operations. This method should be called by clients
 * in combination with #MHD_get_fdset and #MHD_get_timeout() if the
 * client-controlled select method is used.
 * This function specifies FD_SETSIZE used when provided fd_sets were
 * created. It is important on platforms where FD_SETSIZE can be
 * overridden.
 *
 * You can use this function instead of #MHD_run if you called
 * 'select()' on the result from #MHD_get_fdset2().  File descriptors in
 * the sets that are not controlled by MHD will be ignored.  Calling
 * this function instead of #MHD_run() is more efficient as MHD will
 * not have to call 'select()' again to determine which operations are
 * ready.
 *
 * If #MHD_get_timeout() returned #MHD_YES, than this function must be
 * called right after 'select()' returns regardless of detected activity
 * on the daemon's FDs.
 *
 * This function cannot be used with daemon started with
 * #MHD_USE_INTERNAL_POLLING_THREAD flag.
 *
 * @param daemon the daemon to run select loop for
 * @param read_fd_set the read set
 * @param write_fd_set the write set
 * @param except_fd_set the except set
 * @param fd_setsize the value of FD_SETSIZE
 * @return #MHD_NO on serious errors, #MHD_YES on success
 * @sa #MHD_get_fdset2(), #MHD_OPTION_APP_FD_SETSIZE
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_run_from_select2 (struct MHD_Daemon *daemon,
                      const fd_set *read_fd_set,
                      const fd_set *write_fd_set,
                      const fd_set *except_fd_set,
                      unsigned int fd_setsize)
{
  if (MHD_D_IS_USING_POLL_ (daemon) ||
      MHD_D_IS_USING_THREADS_ (daemon))
    return MHD_NO;
  if ((NULL == read_fd_set) || (NULL == write_fd_set))
    return MHD_NO;
#ifdef HAVE_MESSAGES
  if (NULL == except_fd_set)
  {
    MHD_DLOG (daemon,
              _ ("MHD_run_from_select() called with except_fd_set "
                 "set to NULL. Such behavior is deprecated.\n"));
  }
#endif /* HAVE_MESSAGES */

#ifdef HAS_FD_SETSIZE_OVERRIDABLE
  if (0 == fd_setsize)
    return MHD_NO;
  else if (((unsigned int) INT_MAX) < fd_setsize)
    fd_setsize = (unsigned int) INT_MAX;
#ifdef HAVE_MESSAGES
  else if (daemon->fdset_size > ((int) fd_setsize))
  {
    if (daemon->fdset_size_set_by_app)
    {
      MHD_DLOG (daemon,
                _ ("%s() called with fd_setsize (%u) " \
                   "less than value set by MHD_OPTION_APP_FD_SETSIZE (%d). " \
                   "Some socket FDs may be not processed. " \
                   "Use MHD_OPTION_APP_FD_SETSIZE with the correct value.\n"),
                "MHD_run_from_select2", fd_setsize, daemon->fdset_size);
    }
    else
    {
      MHD_DLOG (daemon,
                _ ("%s() called with fd_setsize (%u) " \
                   "less than FD_SETSIZE used by MHD (%d). " \
                   "Some socket FDs may be not processed. " \
                   "Consider using MHD_OPTION_APP_FD_SETSIZE option.\n"),
                "MHD_run_from_select2", fd_setsize, daemon->fdset_size);
    }
  }
#endif /* HAVE_MESSAGES */
#else  /* ! HAS_FD_SETSIZE_OVERRIDABLE */
  if (((unsigned int) FD_SETSIZE) > fd_setsize)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("%s() called with fd_setsize (%u) " \
                 "less than fixed FD_SETSIZE value (%d) used on the " \
                 "platform.\n"),
              "MHD_run_from_select2", fd_setsize, (int) FD_SETSIZE);
#endif /* HAVE_MESSAGES */
    return MHD_NO;
  }
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

  if (MHD_D_IS_USING_EPOLL_ (daemon))
  {
#ifdef EPOLL_SUPPORT
    enum MHD_Result ret = MHD_epoll (daemon,
                                     0);

    MHD_cleanup_connections (daemon);
    return ret;
#else  /* ! EPOLL_SUPPORT */
    return MHD_NO;
#endif /* ! EPOLL_SUPPORT */
  }

  /* Resuming external connections when using an extern mainloop  */
  if (0 != (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME))
    resume_suspended_connections (daemon);

  return internal_run_from_select (daemon,
                                   read_fd_set,
                                   write_fd_set,
                                   except_fd_set,
                                   (int) fd_setsize);
}


/**
 * Run webserver operations. This method should be called by clients
 * in combination with #MHD_get_fdset and #MHD_get_timeout() if the
 * client-controlled select method is used.
 *
 * You can use this function instead of #MHD_run if you called
 * `select()` on the result from #MHD_get_fdset.  File descriptors in
 * the sets that are not controlled by MHD will be ignored.  Calling
 * this function instead of #MHD_run is more efficient as MHD will
 * not have to call `select()` again to determine which operations are
 * ready.
 *
 * If #MHD_get_timeout() returned #MHD_YES, than this function must be
 * called right after `select()` returns regardless of detected activity
 * on the daemon's FDs.
 *
 * This function cannot be used with daemon started with
 * #MHD_USE_INTERNAL_POLLING_THREAD flag.
 *
 * @param daemon daemon to run select loop for
 * @param read_fd_set read set
 * @param write_fd_set write set
 * @param except_fd_set except set
 * @return #MHD_NO on serious errors, #MHD_YES on success
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_run_from_select (struct MHD_Daemon *daemon,
                     const fd_set *read_fd_set,
                     const fd_set *write_fd_set,
                     const fd_set *except_fd_set)
{
  return MHD_run_from_select2 (daemon,
                               read_fd_set,
                               write_fd_set,
                               except_fd_set,
#ifdef HAS_FD_SETSIZE_OVERRIDABLE
                               daemon->fdset_size_set_by_app ?
                               ((unsigned int) daemon->fdset_size) :
                               ((unsigned int) _MHD_SYS_DEFAULT_FD_SETSIZE)
#else  /* ! HAS_FD_SETSIZE_OVERRIDABLE */
                               ((unsigned int) _MHD_SYS_DEFAULT_FD_SETSIZE)
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */
                               );
}


/**
 * Main internal select() call.  Will compute select sets, call select()
 * and then #internal_run_from_select with the result.
 *
 * @param daemon daemon to run select() loop for
 * @param millisec the maximum time in milliseconds to wait for events,
 *                 set to '0' for non-blocking processing,
 *                 set to '-1' to wait indefinitely.
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static enum MHD_Result
MHD_select (struct MHD_Daemon *daemon,
            int32_t millisec)
{
  int num_ready;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket maxsock;
  struct timeval timeout;
  struct timeval *tv;
  int err_state;
  MHD_socket ls;

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  if (daemon->shutdown)
    return MHD_NO;
  FD_ZERO (&rs);
  FD_ZERO (&ws);
  FD_ZERO (&es);
  maxsock = MHD_INVALID_SOCKET;
  err_state = MHD_NO;
  if ( (0 != (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME)) &&
       (MHD_NO != resume_suspended_connections (daemon)) &&
       (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon)) )
    millisec = 0;

  if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
  {
    /* single-threaded, go over everything */
    if (MHD_NO ==
        internal_get_fdset2 (daemon,
                             &rs,
                             &ws,
                             &es,
                             &maxsock,
                             (int) FD_SETSIZE))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Could not obtain daemon fdsets.\n"));
#endif
      err_state = MHD_YES;
    }
  }
  else
  {
    bool itc_added;
    /* accept only, have one thread per connection */
    itc_added = false;
    if (MHD_ITC_IS_VALID_ (daemon->itc))
    {
      itc_added = MHD_add_to_fd_set_ (MHD_itc_r_fd_ (daemon->itc),
                                      &rs,
                                      &maxsock,
                                      (int) FD_SETSIZE);
      if (! itc_added)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon, _ ("Could not add control inter-thread " \
                             "communication channel FD to fdset.\n"));
#endif
        err_state = MHD_YES;
      }
    }
    if ( (MHD_INVALID_SOCKET != (ls = daemon->listen_fd)) &&
         (! daemon->was_quiesced) )
    {
      /* Stop listening if we are at the configured connection limit */
      /* If we're at the connection limit, no point in really
         accepting new connections; however, make sure we do not miss
         the shutdown OR the termination of an existing connection; so
         only do this optimisation if we have a signaling ITC in
         place. */
      if (! itc_added ||
          ((daemon->connections < daemon->connection_limit) &&
           ! daemon->at_limit))
      {
        if (! MHD_add_to_fd_set_ (ls,
                                  &rs,
                                  &maxsock,
                                  (int) FD_SETSIZE))
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Could not add listen socket to fdset.\n"));
#endif
          err_state = MHD_YES;
        }
      }
    }
  }

  if (MHD_NO != err_state)
    millisec = 0;
  if (0 == millisec)
  {
    timeout.tv_usec = 0;
    timeout.tv_sec = 0;
    tv = &timeout;
  }
  else
  {
    uint64_t mhd_tmo;
    uint64_t select_tmo;

    if ( (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon)) &&
         (MHD_NO != MHD_get_timeout64 (daemon, &mhd_tmo)) )
    {
      if ( (0 < millisec) &&
           (mhd_tmo > (uint64_t) millisec) )
        select_tmo = (uint64_t) millisec;
      else
        select_tmo = mhd_tmo;
      tv = &timeout; /* have timeout value */
    }
    else if (0 < millisec)
    {
      select_tmo = (uint64_t) millisec;
      tv = &timeout; /* have timeout value */
    }
    else
    {
      select_tmo = 0; /* Not actually used, silent compiler warning */
      tv = NULL;
    }

    if (NULL != tv)
    { /* have timeout value */
#if (SIZEOF_UINT64_T - 2) >= SIZEOF_STRUCT_TIMEVAL_TV_SEC
      if (select_tmo / 1000 > TIMEVAL_TV_SEC_MAX)
        timeout.tv_sec = TIMEVAL_TV_SEC_MAX;
      else
#endif /* (SIZEOF_UINT64_T - 2) >= SIZEOF_STRUCT_TIMEVAL_TV_SEC */
      timeout.tv_sec = (_MHD_TIMEVAL_TV_SEC_TYPE) (select_tmo / 1000);

      timeout.tv_usec = ((uint16_t) (select_tmo % 1000)) * ((int32_t) 1000);
    }
  }
  num_ready = MHD_SYS_select_ (maxsock + 1,
                               &rs,
                               &ws,
                               &es,
                               tv);
  if (daemon->shutdown)
    return MHD_NO;
  if (num_ready < 0)
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return (MHD_NO == err_state) ? MHD_YES : MHD_NO;
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("select failed: %s\n"),
              MHD_socket_strerr_ (err));
#endif
    return MHD_NO;
  }
  if (MHD_NO != internal_run_from_select (daemon,
                                          &rs,
                                          &ws,
                                          &es,
                                          (int) FD_SETSIZE))
    return (MHD_NO == err_state) ? MHD_YES : MHD_NO;
  return MHD_NO;
}


#ifdef HAVE_POLL
/**
 * Process all of our connections and possibly the server
 * socket using poll().
 *
 * @param daemon daemon to run poll loop for
 * @param millisec the maximum time in milliseconds to wait for events,
 *                 set to '0' for non-blocking processing,
 *                 set to '-1' to wait indefinitely.
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static enum MHD_Result
MHD_poll_all (struct MHD_Daemon *daemon,
              int32_t millisec)
{
  unsigned int num_connections;
  struct MHD_Connection *pos;
  struct MHD_Connection *prev;
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  struct MHD_UpgradeResponseHandle *urh;
  struct MHD_UpgradeResponseHandle *urhn;
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */

  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_valid_ID_ (daemon->tid)));
  mhd_assert ((0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (! MHD_thread_handle_ID_is_valid_ID_ (daemon->tid)));
  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_current_thread_ (daemon->tid)));

  if ( (0 != (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME)) &&
       (MHD_NO != resume_suspended_connections (daemon)) )
    millisec = 0;

  /* count number of connections and thus determine poll set size */
  num_connections = 0;
  for (pos = daemon->connections_head; NULL != pos; pos = pos->next)
    num_connections++;
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  for (urh = daemon->urh_head; NULL != urh; urh = urh->next)
    num_connections += 2;
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  {
    unsigned int i;
    int timeout;
    unsigned int poll_server;
    int poll_listen;
    int poll_itc_idx;
    struct pollfd *p;
    MHD_socket ls;

    p = MHD_calloc_ ((2 + (size_t) num_connections),
                     sizeof (struct pollfd));
    if (NULL == p)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Error allocating memory: %s\n"),
                MHD_strerror_ (errno));
#endif
      return MHD_NO;
    }
    poll_server = 0;
    poll_listen = -1;
    if ( (MHD_INVALID_SOCKET != (ls = daemon->listen_fd)) &&
         (! daemon->was_quiesced) &&
         (daemon->connections < daemon->connection_limit) &&
         (! daemon->at_limit) )
    {
      /* only listen if we are not at the connection limit */
      p[poll_server].fd = ls;
      p[poll_server].events = POLLIN;
      p[poll_server].revents = 0;
      poll_listen = (int) poll_server;
      poll_server++;
    }
    poll_itc_idx = -1;
    if (MHD_ITC_IS_VALID_ (daemon->itc))
    {
      p[poll_server].fd = MHD_itc_r_fd_ (daemon->itc);
      p[poll_server].events = POLLIN;
      p[poll_server].revents = 0;
      poll_itc_idx = (int) poll_server;
      poll_server++;
    }

    timeout = get_timeout_millisec_int (daemon, millisec);

    i = 0;
    for (pos = daemon->connections_tail; NULL != pos; pos = pos->prev)
    {
      p[poll_server + i].fd = pos->socket_fd;
      switch (pos->event_loop_info)
      {
      case MHD_EVENT_LOOP_INFO_READ:
      case MHD_EVENT_LOOP_INFO_PROCESS_READ:
        p[poll_server + i].events |= POLLIN | MHD_POLL_EVENTS_ERR_DISC;
        break;
      case MHD_EVENT_LOOP_INFO_WRITE:
        p[poll_server + i].events |= POLLOUT | MHD_POLL_EVENTS_ERR_DISC;
        break;
      case MHD_EVENT_LOOP_INFO_PROCESS:
        p[poll_server + i].events |=  MHD_POLL_EVENTS_ERR_DISC;
        break;
      case MHD_EVENT_LOOP_INFO_CLEANUP:
        timeout = 0; /* clean up "pos" immediately */
        break;
      }
      i++;
    }
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
    for (urh = daemon->urh_tail; NULL != urh; urh = urh->prev)
    {
      urh_to_pollfd (urh, &(p[poll_server + i]));
      i += 2;
    }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
    if (0 == poll_server + num_connections)
    {
      free (p);
      return MHD_YES;
    }
    if (MHD_sys_poll_ (p,
                       poll_server + num_connections,
                       timeout) < 0)
    {
      const int err = MHD_socket_get_error_ ();
      if (MHD_SCKT_ERR_IS_EINTR_ (err))
      {
        free (p);
        return MHD_YES;
      }
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("poll failed: %s\n"),
                MHD_socket_strerr_ (err));
#endif
      free (p);
      return MHD_NO;
    }

    /* handle ITC FD */
    /* do it before any other processing so
       new signals will be processed in next loop */
    if ( (-1 != poll_itc_idx) &&
         (0 != (p[poll_itc_idx].revents & POLLIN)) )
      MHD_itc_clear_ (daemon->itc);

    /* handle shutdown */
    if (daemon->shutdown)
    {
      free (p);
      return MHD_NO;
    }

    /* Process externally added connection if any */
    if (daemon->have_new)
      new_connections_list_process_ (daemon);

    /* handle 'listen' FD */
    if ( (-1 != poll_listen) &&
         (0 != (p[poll_listen].revents & POLLIN)) )
      (void) MHD_accept_connection (daemon);

    /* Reset. New value will be set when connections are processed. */
    daemon->data_already_pending = false;

    i = 0;
    prev = daemon->connections_tail;
    while (NULL != (pos = prev))
    {
      prev = pos->prev;
      /* first, sanity checks */
      if (i >= num_connections)
        break;     /* connection list changed somehow, retry later ... */
      if (p[poll_server + i].fd != pos->socket_fd)
        continue;  /* fd mismatch, something else happened, retry later ... */
      call_handlers (pos,
                     0 != (p[poll_server + i].revents & POLLIN),
                     0 != (p[poll_server + i].revents & POLLOUT),
                     0 != (p[poll_server + i].revents
                           & MHD_POLL_REVENTS_ERR_DISC));
      i++;
    }
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
    for (urh = daemon->urh_tail; NULL != urh; urh = urhn)
    {
      if (i >= num_connections)
        break;   /* connection list changed somehow, retry later ... */

      /* Get next connection here as connection can be removed
       * from 'daemon->urh_head' list. */
      urhn = urh->prev;
      /* Check for fd mismatch. FIXME: required for safety? */
      if ((p[poll_server + i].fd != urh->connection->socket_fd) ||
          (p[poll_server + i + 1].fd != urh->mhd.socket))
        break;
      urh_from_pollfd (urh,
                       &p[poll_server + i]);
      i += 2;
      process_urh (urh);
      /* Finished forwarding? */
      if ( (0 == urh->in_buffer_size) &&
           (0 == urh->out_buffer_size) &&
           (0 == urh->in_buffer_used) &&
           (0 == urh->out_buffer_used) )
      {
        /* MHD_connection_finish_forward_() will remove connection from
         * 'daemon->urh_head' list. */
        MHD_connection_finish_forward_ (urh->connection);
        urh->clean_ready = true;
        /* If 'urh->was_closed' already was set to true, connection will be
         * moved immediately to cleanup list. Otherwise connection
         * will stay in suspended list until 'urh' will be marked
         * with 'was_closed' by application. */
        MHD_resume_connection (urh->connection);
      }
    }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */

    free (p);
  }
  return MHD_YES;
}


/**
 * Process only the listen socket using poll().
 *
 * @param daemon daemon to run poll loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static enum MHD_Result
MHD_poll_listen_socket (struct MHD_Daemon *daemon,
                        int may_block)
{
  struct pollfd p[2];
  int timeout;
  unsigned int poll_count;
  int poll_listen;
  int poll_itc_idx;
  MHD_socket ls;

  mhd_assert (MHD_thread_handle_ID_is_valid_ID_ (daemon->tid));
  mhd_assert (MHD_thread_handle_ID_is_current_thread_ (daemon->tid));

  memset (&p,
          0,
          sizeof (p));
  poll_count = 0;
  poll_listen = -1;
  poll_itc_idx = -1;
  if ( (MHD_INVALID_SOCKET != (ls = daemon->listen_fd)) &&
       (! daemon->was_quiesced) )

  {
    p[poll_count].fd = ls;
    p[poll_count].events = POLLIN;
    p[poll_count].revents = 0;
    poll_listen = (int) poll_count;
    poll_count++;
  }
  if (MHD_ITC_IS_VALID_ (daemon->itc))
  {
    p[poll_count].fd = MHD_itc_r_fd_ (daemon->itc);
    p[poll_count].events = POLLIN;
    p[poll_count].revents = 0;
    poll_itc_idx = (int) poll_count;
    poll_count++;
  }

  if (0 != (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME))
    (void) resume_suspended_connections (daemon);

  if (MHD_NO == may_block)
    timeout = 0;
  else
    timeout = -1;
  if (0 == poll_count)
    return MHD_YES;
  if (MHD_sys_poll_ (p,
                     poll_count,
                     timeout) < 0)
  {
    const int err = MHD_socket_get_error_ ();

    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return MHD_YES;
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("poll failed: %s\n"),
              MHD_socket_strerr_ (err));
#endif
    return MHD_NO;
  }
  if ( (0 <= poll_itc_idx) &&
       (0 != (p[poll_itc_idx].revents & POLLIN)) )
    MHD_itc_clear_ (daemon->itc);

  /* handle shutdown */
  if (daemon->shutdown)
    return MHD_NO;

  /* Process externally added connection if any */
  if (daemon->have_new)
    new_connections_list_process_ (daemon);

  if ( (0 <= poll_listen) &&
       (0 != (p[poll_listen].revents & POLLIN)) )
    (void) MHD_accept_connection (daemon);
  return MHD_YES;
}


#endif

#ifdef HAVE_POLL

/**
 * Do poll()-based processing.
 *
 * @param daemon daemon to run poll()-loop for
 * @param may_block #MHD_YES if blocking, #MHD_NO if non-blocking
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static enum MHD_Result
MHD_poll (struct MHD_Daemon *daemon,
          int may_block)
{
  if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    return MHD_poll_all (daemon,
                         may_block ? -1 : 0);
  return MHD_poll_listen_socket (daemon,
                                 may_block);
}


#endif /* HAVE_POLL */


#ifdef EPOLL_SUPPORT

/**
 * How many events to we process at most per epoll() call?  Trade-off
 * between required stack-size and number of system calls we have to
 * make; 128 should be way enough to avoid more than one system call
 * for most scenarios, and still be moderate in stack size
 * consumption.  Embedded systems might want to choose a smaller value
 * --- but why use epoll() on such a system in the first place?
 */
#define MAX_EVENTS 128


#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)

/**
 * Checks whether @a urh has some data to process.
 *
 * @param urh upgrade handler to analyse
 * @return 'true' if @a urh has some data to process,
 *         'false' otherwise
 */
static bool
is_urh_ready (struct MHD_UpgradeResponseHandle *const urh)
{
  const struct MHD_Connection *const connection = urh->connection;

  if ( (0 == urh->in_buffer_size) &&
       (0 == urh->out_buffer_size) &&
       (0 == urh->in_buffer_used) &&
       (0 == urh->out_buffer_used) )
    return false;
  if (connection->daemon->shutdown)
    return true;
  if ( ( (0 != ((MHD_EPOLL_STATE_READ_READY | MHD_EPOLL_STATE_ERROR)
                & urh->app.celi)) ||
         (connection->tls_read_ready) ) &&
       (urh->in_buffer_used < urh->in_buffer_size) )
    return true;
  if ( ( (0 != ((MHD_EPOLL_STATE_READ_READY | MHD_EPOLL_STATE_ERROR)
                & urh->mhd.celi)) ||
         urh->was_closed) &&
       (urh->out_buffer_used < urh->out_buffer_size) )
    return true;
  if ( (0 != (MHD_EPOLL_STATE_WRITE_READY & urh->app.celi)) &&
       (urh->out_buffer_used > 0) )
    return true;
  if ( (0 != (MHD_EPOLL_STATE_WRITE_READY & urh->mhd.celi)) &&
       (urh->in_buffer_used > 0) )
    return true;
  return false;
}


/**
 * Do epoll()-based processing for TLS connections that have been
 * upgraded.  This requires a separate epoll() invocation as we
 * cannot use the `struct MHD_Connection` data structures for
 * the `union epoll_data` in this case.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 */
static enum MHD_Result
run_epoll_for_upgrade (struct MHD_Daemon *daemon)
{
  struct epoll_event events[MAX_EVENTS];
  int num_events;
  struct MHD_UpgradeResponseHandle *pos;
  struct MHD_UpgradeResponseHandle *prev;

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
#endif /* MHD_USE_THREADS */

  num_events = MAX_EVENTS;
  while (0 != num_events)
  {
    unsigned int i;
    /* update event masks */
    num_events = epoll_wait (daemon->epoll_upgrade_fd,
                             events,
                             MAX_EVENTS,
                             0);
    if (-1 == num_events)
    {
      const int err = MHD_socket_get_error_ ();

      if (MHD_SCKT_ERR_IS_EINTR_ (err))
        return MHD_YES;
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Call to epoll_wait failed: %s\n"),
                MHD_socket_strerr_ (err));
#endif
      return MHD_NO;
    }
    for (i = 0; i < (unsigned int) num_events; i++)
    {
      struct UpgradeEpollHandle *const ueh = events[i].data.ptr;
      struct MHD_UpgradeResponseHandle *const urh = ueh->urh;
      bool new_err_state = false;

      if (urh->clean_ready)
        continue;

      /* Update ueh state based on what is ready according to epoll() */
      if (0 != (events[i].events & EPOLLIN))
      {
        ueh->celi |= MHD_EPOLL_STATE_READ_READY;
      }
      if (0 != (events[i].events & EPOLLOUT))
      {
        ueh->celi |= MHD_EPOLL_STATE_WRITE_READY;
      }
      if (0 != (events[i].events & EPOLLHUP))
      {
        ueh->celi |= MHD_EPOLL_STATE_READ_READY | MHD_EPOLL_STATE_WRITE_READY;
      }

      if ( (0 == (ueh->celi & MHD_EPOLL_STATE_ERROR)) &&
           (0 != (events[i].events & (EPOLLERR | EPOLLPRI))) )
      {
        /* Process new error state only one time and avoid continuously
         * marking this connection as 'ready'. */
        ueh->celi |= MHD_EPOLL_STATE_ERROR;
        new_err_state = true;
      }
      if (! urh->in_eready_list)
      {
        if (new_err_state ||
            is_urh_ready (urh))
        {
          EDLL_insert (daemon->eready_urh_head,
                       daemon->eready_urh_tail,
                       urh);
          urh->in_eready_list = true;
        }
      }
    }
  }
  prev = daemon->eready_urh_tail;
  while (NULL != (pos = prev))
  {
    prev = pos->prevE;
    process_urh (pos);
    if (! is_urh_ready (pos))
    {
      EDLL_remove (daemon->eready_urh_head,
                   daemon->eready_urh_tail,
                   pos);
      pos->in_eready_list = false;
    }
    /* Finished forwarding? */
    if ( (0 == pos->in_buffer_size) &&
         (0 == pos->out_buffer_size) &&
         (0 == pos->in_buffer_used) &&
         (0 == pos->out_buffer_used) )
    {
      MHD_connection_finish_forward_ (pos->connection);
      pos->clean_ready = true;
      /* If 'pos->was_closed' already was set to true, connection
       * will be moved immediately to cleanup list. Otherwise
       * connection will stay in suspended list until 'pos' will
       * be marked with 'was_closed' by application. */
      MHD_resume_connection (pos->connection);
    }
  }

  return MHD_YES;
}


#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */


/**
 * Pointer-marker to distinguish ITC slot in epoll sets.
 */
static const char *const epoll_itc_marker = "itc_marker";


/**
 * Do epoll()-based processing.
 *
 * @param daemon daemon to run poll loop for
 * @param millisec the maximum time in milliseconds to wait for events,
 *                 set to '0' for non-blocking processing,
 *                 set to '-1' to wait indefinitely.
 * @return #MHD_NO on serious errors, #MHD_YES on success
 */
static enum MHD_Result
MHD_epoll (struct MHD_Daemon *daemon,
           int32_t millisec)
{
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  static const char *const upgrade_marker = "upgrade_ptr";
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  struct MHD_Connection *pos;
  struct MHD_Connection *prev;
  struct epoll_event events[MAX_EVENTS];
  struct epoll_event event;
  int timeout_ms;
  int num_events;
  unsigned int i;
  MHD_socket ls;
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  bool run_upgraded = false;
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  bool need_to_accept;

  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_valid_ID_ (daemon->tid)));
  mhd_assert ((0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (! MHD_thread_handle_ID_is_valid_ID_ (daemon->tid)));
  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (MHD_thread_handle_ID_is_current_thread_ (daemon->tid)));

  if (-1 == daemon->epoll_fd)
    return MHD_NO; /* we're down! */
  if (daemon->shutdown)
    return MHD_NO;
  if ( (MHD_INVALID_SOCKET != (ls = daemon->listen_fd)) &&
       (! daemon->was_quiesced) &&
       (daemon->connections < daemon->connection_limit) &&
       (! daemon->listen_socket_in_epoll) &&
       (! daemon->at_limit) )
  {
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.ptr = daemon;
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_ADD,
                        ls,
                        &event))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Call to epoll_ctl failed: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
    daemon->listen_socket_in_epoll = true;
  }
  if ( (daemon->was_quiesced) &&
       (daemon->listen_socket_in_epoll) )
  {
    if ( (0 != epoll_ctl (daemon->epoll_fd,
                          EPOLL_CTL_DEL,
                          ls,
                          NULL)) &&
         (ENOENT != errno) )   /* ENOENT can happen due to race with
                                  #MHD_quiesce_daemon() */
      MHD_PANIC ("Failed to remove listen FD from epoll set.\n");
    daemon->listen_socket_in_epoll = false;
  }

#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  if ( ( (! daemon->upgrade_fd_in_epoll) &&
         (-1 != daemon->epoll_upgrade_fd) ) )
  {
    event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    event.data.ptr = _MHD_DROP_CONST (upgrade_marker);
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_ADD,
                        daemon->epoll_upgrade_fd,
                        &event))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Call to epoll_ctl failed: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
    daemon->upgrade_fd_in_epoll = true;
  }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  if ( (daemon->listen_socket_in_epoll) &&
       ( (daemon->connections == daemon->connection_limit) ||
         (daemon->at_limit) ||
         (daemon->was_quiesced) ) )
  {
    /* we're at the connection limit, disable listen socket
 for event loop for now */
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_DEL,
                        ls,
                        NULL))
      MHD_PANIC (_ ("Failed to remove listen FD from epoll set.\n"));
    daemon->listen_socket_in_epoll = false;
  }

  if ( (0 != (daemon->options & MHD_TEST_ALLOW_SUSPEND_RESUME)) &&
       (MHD_NO != resume_suspended_connections (daemon)) )
    millisec = 0;

  timeout_ms = get_timeout_millisec_int (daemon, millisec);

  /* Reset. New value will be set when connections are processed. */
  /* Note: Used mostly for uniformity here as same situation is
   * signaled in epoll mode by non-empty eready DLL. */
  daemon->data_already_pending = false;

  need_to_accept = false;
  /* drain 'epoll' event queue; need to iterate as we get at most
     MAX_EVENTS in one system call here; in practice this should
     pretty much mean only one round, but better an extra loop here
     than unfair behavior... */
  num_events = MAX_EVENTS;
  while (MAX_EVENTS == num_events)
  {
    /* update event masks */
    num_events = epoll_wait (daemon->epoll_fd,
                             events,
                             MAX_EVENTS,
                             timeout_ms);
    if (-1 == num_events)
    {
      const int err = MHD_socket_get_error_ ();
      if (MHD_SCKT_ERR_IS_EINTR_ (err))
        return MHD_YES;
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Call to epoll_wait failed: %s\n"),
                MHD_socket_strerr_ (err));
#endif
      return MHD_NO;
    }
    for (i = 0; i < (unsigned int) num_events; i++)
    {
      /* First, check for the values of `ptr` that would indicate
         that this event is not about a normal connection. */
      if (NULL == events[i].data.ptr)
        continue;     /* shutdown signal! */
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
      if (upgrade_marker == events[i].data.ptr)
      {
        /* activity on an upgraded connection, we process
           those in a separate epoll() */
        run_upgraded = true;
        continue;
      }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
      if (epoll_itc_marker == events[i].data.ptr)
      {
        /* It's OK to clear ITC here as all external
           conditions will be processed later. */
        MHD_itc_clear_ (daemon->itc);
        continue;
      }
      if (daemon == events[i].data.ptr)
      {
        /* Check for error conditions on listen socket. */
        /* FIXME: Initiate MHD_quiesce_daemon() to prevent busy waiting? */
        if (0 == (events[i].events & (EPOLLERR | EPOLLHUP)))
          need_to_accept = true;
        continue;
      }
      /* this is an event relating to a 'normal' connection,
         remember the event and if appropriate mark the
         connection as 'eready'. */
      pos = events[i].data.ptr;
      /* normal processing: update read/write data */
      if (0 != (events[i].events & (EPOLLPRI | EPOLLERR | EPOLLHUP)))
      {
        pos->epoll_state |= MHD_EPOLL_STATE_ERROR;
        if (0 == (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL))
        {
          EDLL_insert (daemon->eready_head,
                       daemon->eready_tail,
                       pos);
          pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
        }
      }
      else
      {
        if (0 != (events[i].events & EPOLLIN))
        {
          pos->epoll_state |= MHD_EPOLL_STATE_READ_READY;
          if ( ( (0 != (MHD_EVENT_LOOP_INFO_READ & pos->event_loop_info)) ||
                 (pos->read_buffer_size > pos->read_buffer_offset) ) &&
               (0 == (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL) ) )
          {
            EDLL_insert (daemon->eready_head,
                         daemon->eready_tail,
                         pos);
            pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
          }
        }
        if (0 != (events[i].events & EPOLLOUT))
        {
          pos->epoll_state |= MHD_EPOLL_STATE_WRITE_READY;
          if ( (MHD_EVENT_LOOP_INFO_WRITE == pos->event_loop_info) &&
               (0 == (pos->epoll_state & MHD_EPOLL_STATE_IN_EREADY_EDLL) ) )
          {
            EDLL_insert (daemon->eready_head,
                         daemon->eready_tail,
                         pos);
            pos->epoll_state |= MHD_EPOLL_STATE_IN_EREADY_EDLL;
          }
        }
      }
    }
  }

  /* Process externally added connection if any */
  if (daemon->have_new)
    new_connections_list_process_ (daemon);

  if (need_to_accept)
  {
    unsigned int series_length = 0;

    /* Run 'accept' until it fails or daemon at limit of connections.
     * Do not accept more then 10 connections at once. The rest will
     * be accepted on next turn (level trigger is used for listen
     * socket). */
    while ( (MHD_NO != MHD_accept_connection (daemon)) &&
            (series_length < 10) &&
            (daemon->connections < daemon->connection_limit) &&
            (! daemon->at_limit) )
      series_length++;
  }

  /* Handle timed-out connections; we need to do this here
     as the epoll mechanism won't call the 'MHD_connection_handle_idle()' on everything,
     as the other event loops do.  As timeouts do not get an explicit
     event, we need to find those connections that might have timed out
     here.

     Connections with custom timeouts must all be looked at, as we
     do not bother to sort that (presumably very short) list. */
  prev = daemon->manual_timeout_tail;
  while (NULL != (pos = prev))
  {
    prev = pos->prevX;
    MHD_connection_handle_idle (pos);
  }
  /* Connections with the default timeout are sorted by prepending
     them to the head of the list whenever we touch the connection;
     thus it suffices to iterate from the tail until the first
     connection is NOT timed out */
  prev = daemon->normal_timeout_tail;
  while (NULL != (pos = prev))
  {
    prev = pos->prevX;
    MHD_connection_handle_idle (pos);
    if (MHD_CONNECTION_CLOSED != pos->state)
      break; /* sorted by timeout, no need to visit the rest! */
  }

#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  if (run_upgraded || (NULL != daemon->eready_urh_head))
    run_epoll_for_upgrade (daemon);
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */

  /* process events for connections */
  prev = daemon->eready_tail;
  while (NULL != (pos = prev))
  {
    prev = pos->prevE;
    call_handlers (pos,
                   0 != (pos->epoll_state & MHD_EPOLL_STATE_READ_READY),
                   0 != (pos->epoll_state & MHD_EPOLL_STATE_WRITE_READY),
                   0 != (pos->epoll_state & MHD_EPOLL_STATE_ERROR));
    if (MHD_EPOLL_STATE_IN_EREADY_EDLL ==
        (pos->epoll_state & (MHD_EPOLL_STATE_SUSPENDED
                             | MHD_EPOLL_STATE_IN_EREADY_EDLL)))
    {
      if ( ((MHD_EVENT_LOOP_INFO_READ == pos->event_loop_info) &&
            (0 == (pos->epoll_state & MHD_EPOLL_STATE_READ_READY)) ) ||
           ((MHD_EVENT_LOOP_INFO_WRITE == pos->event_loop_info) &&
            (0 == (pos->epoll_state & MHD_EPOLL_STATE_WRITE_READY)) ) ||
           (MHD_EVENT_LOOP_INFO_CLEANUP == pos->event_loop_info) )
      {
        EDLL_remove (daemon->eready_head,
                     daemon->eready_tail,
                     pos);
        pos->epoll_state &=
          ~((enum MHD_EpollState) MHD_EPOLL_STATE_IN_EREADY_EDLL);
      }
    }
  }

  return MHD_YES;
}


#endif


/**
 * Run webserver operations (without blocking unless in client callbacks).
 *
 * This method should be called by clients in combination with
 * #MHD_get_fdset() (or #MHD_get_daemon_info() with MHD_DAEMON_INFO_EPOLL_FD
 * if epoll is used) and #MHD_get_timeout() if the client-controlled
 * connection polling method is used (i.e. daemon was started without
 * #MHD_USE_INTERNAL_POLLING_THREAD flag).
 *
 * This function is a convenience method, which is useful if the
 * fd_sets from #MHD_get_fdset were not directly passed to `select()`;
 * with this function, MHD will internally do the appropriate `select()`
 * call itself again.  While it is acceptable to call #MHD_run (if
 * #MHD_USE_INTERNAL_POLLING_THREAD is not set) at any moment, you should
 * call #MHD_run_from_select() if performance is important (as it saves an
 * expensive call to `select()`).
 *
 * If #MHD_get_timeout() returned #MHD_YES, than this function must be called
 * right after polling function returns regardless of detected activity on
 * the daemon's FDs.
 *
 * @param daemon daemon to run
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call.
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_run (struct MHD_Daemon *daemon)
{
  if ( (daemon->shutdown) ||
       MHD_D_IS_USING_THREADS_ (daemon) )
    return MHD_NO;

  (void) MHD_run_wait (daemon, 0);
  return MHD_YES;
}


/**
 * Run webserver operation with possible blocking.
 *
 * This function does the following: waits for any network event not more than
 * specified number of milliseconds, processes all incoming and outgoing data,
 * processes new connections, processes any timed-out connection, and does
 * other things required to run webserver.
 * Once all connections are processed, function returns.
 *
 * This function is useful for quick and simple (lazy) webserver implementation
 * if application needs to run a single thread only and does not have any other
 * network activity.
 *
 * This function calls MHD_get_timeout() internally and use returned value as
 * maximum wait time if it less than value of @a millisec parameter.
 *
 * It is expected that the "external" socket polling function is not used in
 * conjunction with this function unless the @a millisec is set to zero.
 *
 * @param daemon the daemon to run
 * @param millisec the maximum time in milliseconds to wait for network and
 *                 other events. Note: there is no guarantee that function
 *                 blocks for the specified amount of time. The real processing
 *                 time can be shorter (if some data or connection timeout
 *                 comes earlier) or longer (if data processing requires more
 *                 time, especially in user callbacks).
 *                 If set to '0' then function does not block and processes
 *                 only already available data (if any).
 *                 If set to '-1' then function waits for events
 *                 indefinitely (blocks until next network activity or
 *                 connection timeout).
 * @return #MHD_YES on success, #MHD_NO if this
 *         daemon was not started with the right
 *         options for this call or some serious
 *         unrecoverable error occurs.
 * @note Available since #MHD_VERSION 0x00097206
 * @ingroup event
 */
_MHD_EXTERN enum MHD_Result
MHD_run_wait (struct MHD_Daemon *daemon,
              int32_t millisec)
{
  enum MHD_Result res;
  if ( (daemon->shutdown) ||
       MHD_D_IS_USING_THREADS_ (daemon) )
    return MHD_NO;

  mhd_assert (! MHD_thread_handle_ID_is_valid_handle_ (daemon->tid));

  if (0 > millisec)
    millisec = -1;
#ifdef HAVE_POLL
  if (MHD_D_IS_USING_POLL_ (daemon))
  {
    res = MHD_poll_all (daemon, millisec);
    MHD_cleanup_connections (daemon);
  }
  else
#endif /* HAVE_POLL */
#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon))
  {
    res = MHD_epoll (daemon, millisec);
    MHD_cleanup_connections (daemon);
  }
  else
#endif
  if (1)
  {
    mhd_assert (MHD_D_IS_USING_SELECT_ (daemon));
#ifdef HAS_FD_SETSIZE_OVERRIDABLE
#ifdef HAVE_MESSAGES
    if (daemon->fdset_size_set_by_app
        && (((int) FD_SETSIZE) < daemon->fdset_size))
    {
      MHD_DLOG (daemon,
                _ ("MHD_run()/MHD_run_wait() called for daemon started with " \
                   "MHD_OPTION_APP_FD_SETSIZE option (%d). " \
                   "The library was compiled with smaller FD_SETSIZE (%d). " \
                   "Some socket FDs may be not processed. " \
                   "Use MHD_run_from_select2() instead of MHD_run() or " \
                   "do not use MHD_OPTION_APP_FD_SETSIZE option.\n"),
                daemon->fdset_size, (int) FD_SETSIZE);
    }
#endif /* HAVE_MESSAGES */
#endif /* HAS_FD_SETSIZE_OVERRIDABLE */

    res = MHD_select (daemon, millisec);
    /* MHD_select does MHD_cleanup_connections already */
  }
  return res;
}


/**
 * Close the given connection, remove it from all of its
 * DLLs and move it into the cleanup queue.
 * @remark To be called only from thread that
 * process daemon's select()/poll()/etc.
 *
 * @param pos connection to move to cleanup
 */
static void
close_connection (struct MHD_Connection *pos)
{
  struct MHD_Daemon *daemon = pos->daemon;

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */

  if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
  {
    MHD_connection_mark_closed_ (pos);
    return;   /* must let thread to do the rest */
  }
  MHD_connection_close_ (pos,
                         MHD_REQUEST_TERMINATED_DAEMON_SHUTDOWN);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  mhd_assert (! pos->suspended);
  mhd_assert (! pos->resuming);
  if (pos->connection_timeout_ms == daemon->connection_timeout_ms)
    XDLL_remove (daemon->normal_timeout_head,
                 daemon->normal_timeout_tail,
                 pos);
  else
    XDLL_remove (daemon->manual_timeout_head,
                 daemon->manual_timeout_tail,
                 pos);
  DLL_remove (daemon->connections_head,
              daemon->connections_tail,
              pos);
  DLL_insert (daemon->cleanup_head,
              daemon->cleanup_tail,
              pos);
  daemon->data_already_pending = true;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
}


#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
/**
 * Thread that runs the polling loop until the daemon
 * is explicitly shut down.
 *
 * @param cls `struct MHD_Deamon` to run select loop in a thread for
 * @return always 0 (on shutdown)
 */
static MHD_THRD_RTRN_TYPE_ MHD_THRD_CALL_SPEC_
MHD_polling_thread (void *cls)
{
  struct MHD_Daemon *daemon = cls;
#ifdef HAVE_PTHREAD_SIGMASK
  sigset_t s_mask;
  int err;
#endif /* HAVE_PTHREAD_SIGMASK */

  MHD_thread_handle_ID_set_current_thread_ID_ (&(daemon->tid));
#ifdef HAVE_PTHREAD_SIGMASK
  if ((0 == sigemptyset (&s_mask)) &&
      (0 == sigaddset (&s_mask, SIGPIPE)))
  {
    err = pthread_sigmask (SIG_BLOCK, &s_mask, NULL);
  }
  else
    err = errno;
  if (0 == err)
    daemon->sigpipe_blocked = true;
#ifdef HAVE_MESSAGES
  else
    MHD_DLOG (daemon,
              _ ("Failed to block SIGPIPE on daemon thread: %s\n"),
              MHD_strerror_ (errno));
#endif /* HAVE_MESSAGES */
#endif /* HAVE_PTHREAD_SIGMASK */
  while (! daemon->shutdown)
  {
#ifdef HAVE_POLL
    if (MHD_D_IS_USING_POLL_ (daemon))
      MHD_poll (daemon, MHD_YES);
    else
#endif /* HAVE_POLL */
#ifdef EPOLL_SUPPORT
    if (MHD_D_IS_USING_EPOLL_ (daemon))
      MHD_epoll (daemon, -1);
    else
#endif
    MHD_select (daemon, -1);
    MHD_cleanup_connections (daemon);
  }

  /* Resume any pending for resume connections, join
   * all connection's threads (if any) and finally cleanup
   * everything. */
  if (0 != (MHD_TEST_ALLOW_SUSPEND_RESUME & daemon->options))
    resume_suspended_connections (daemon);
  close_all_connections (daemon);

  return (MHD_THRD_RTRN_TYPE_) 0;
}


#endif


/**
 * Process escape sequences ('%HH') Updates val in place; the
 * result cannot be larger than the input.
 * The result must also still be 0-terminated.
 *
 * @param cls closure (use NULL)
 * @param connection handle to connection, not used
 * @param val value to unescape (modified in the process)
 * @return length of the resulting val (strlen(val) maybe
 *  shorter afterwards due to elimination of escape sequences)
 */
static size_t
unescape_wrapper (void *cls,
                  struct MHD_Connection *connection,
                  char *val)
{
  bool broken;
  size_t res;
  (void) cls; /* Mute compiler warning. */

  /* TODO: add individual parameter */
  if (0 <= connection->daemon->client_discipline)
    return MHD_str_pct_decode_in_place_strict_ (val);

  res = MHD_str_pct_decode_in_place_lenient_ (val, &broken);
#ifdef HAVE_MESSAGES
  if (broken)
  {
    MHD_DLOG (connection->daemon,
              _ ("The URL encoding is broken.\n"));
  }
#endif /* HAVE_MESSAGES */
  return res;
}


/**
 * Start a webserver on the given port.  Variadic version of
 * #MHD_start_daemon_va.
 *
 * @param flags combination of `enum MHD_FLAG` values
 * @param port port to bind to (in host byte order),
 *        use '0' to bind to random free port,
 *        ignored if #MHD_OPTION_SOCK_ADDR or
 *        #MHD_OPTION_LISTEN_SOCKET is provided
 *        or #MHD_USE_NO_LISTEN_SOCKET is specified
 * @param apc callback to call to check which clients
 *        will be allowed to connect; you can pass NULL
 *        in which case connections from any IP will be
 *        accepted
 * @param apc_cls extra argument to @a apc
 * @param dh handler called for all requests (repeatedly)
 * @param dh_cls extra argument to @a dh
 * @return NULL on error, handle to daemon on success
 * @ingroup event
 */
_MHD_EXTERN struct MHD_Daemon *
MHD_start_daemon (unsigned int flags,
                  uint16_t port,
                  MHD_AcceptPolicyCallback apc,
                  void *apc_cls,
                  MHD_AccessHandlerCallback dh,
                  void *dh_cls,
                  ...)
{
  struct MHD_Daemon *daemon;
  va_list ap;

  va_start (ap,
            dh_cls);
  daemon = MHD_start_daemon_va (flags,
                                port,
                                apc,
                                apc_cls,
                                dh,
                                dh_cls,
                                ap);
  va_end (ap);
  return daemon;
}


/**
 * Stop accepting connections from the listening socket.  Allows
 * clients to continue processing, but stops accepting new
 * connections.  Note that the caller is responsible for closing the
 * returned socket; however, if MHD is run using threads (anything but
 * external select mode), socket will be removed from existing threads
 * with some delay and it must not be closed while it's in use. To make
 * sure that the socket is not used anymore, call #MHD_stop_daemon.
 *
 * Note that some thread modes require the caller to have passed
 * #MHD_USE_ITC when using this API.  If this daemon is
 * in one of those modes and this option was not given to
 * #MHD_start_daemon, this function will return #MHD_INVALID_SOCKET.
 *
 * @param daemon daemon to stop accepting new connections for
 * @return old listen socket on success, #MHD_INVALID_SOCKET if
 *         the daemon was already not listening anymore
 * @ingroup specialized
 */
_MHD_EXTERN MHD_socket
MHD_quiesce_daemon (struct MHD_Daemon *daemon)
{
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  unsigned int i;
#endif
  MHD_socket ret;

  ret = daemon->listen_fd;
  if ((MHD_INVALID_SOCKET == ret)
      || daemon->was_quiesced)
    return MHD_INVALID_SOCKET;
  if ( (0 == (daemon->options & (MHD_USE_ITC))) &&
       MHD_D_IS_USING_THREADS_ (daemon) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Using MHD_quiesce_daemon in this mode " \
                 "requires MHD_USE_ITC.\n"));
#endif
    return MHD_INVALID_SOCKET;
  }

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (NULL != daemon->worker_pool)
    for (i = 0; i < daemon->worker_pool_size; i++)
    {
      daemon->worker_pool[i].was_quiesced = true;
#ifdef EPOLL_SUPPORT
      if (MHD_D_IS_USING_EPOLL_ (daemon) &&
          (-1 != daemon->worker_pool[i].epoll_fd) &&
          (daemon->worker_pool[i].listen_socket_in_epoll) )
      {
        if (0 != epoll_ctl (daemon->worker_pool[i].epoll_fd,
                            EPOLL_CTL_DEL,
                            ret,
                            NULL))
          MHD_PANIC (_ ("Failed to remove listen FD from epoll set.\n"));
        daemon->worker_pool[i].listen_socket_in_epoll = false;
      }
      else
#endif
      if (MHD_ITC_IS_VALID_ (daemon->worker_pool[i].itc))
      {
        if (! MHD_itc_activate_ (daemon->worker_pool[i].itc, "q"))
          MHD_PANIC (_ ("Failed to signal quiesce via inter-thread " \
                        "communication channel.\n"));
      }
    }
#endif
  daemon->was_quiesced = true;
#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon) &&
      (-1 != daemon->epoll_fd) &&
      (daemon->listen_socket_in_epoll) )
  {
    if ( (0 != epoll_ctl (daemon->epoll_fd,
                          EPOLL_CTL_DEL,
                          ret,
                          NULL)) &&
         (ENOENT != errno) )   /* ENOENT can happen due to race with
                                  #MHD_epoll() */
      MHD_PANIC ("Failed to remove listen FD from epoll set.\n");
    daemon->listen_socket_in_epoll = false;
  }
#endif
  if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
       (! MHD_itc_activate_ (daemon->itc, "q")) )
    MHD_PANIC (_ ("failed to signal quiesce via inter-thread " \
                  "communication channel.\n"));
  return ret;
}


/**
 * Temporal location of the application-provided parameters/options.
 * Used when options are decoded from #MHD_start_deamon() parameters, but
 * not yet processed/applied.
 */
struct MHD_InterimParams_
{
  /**
   * The total number of all user options used.
   *
   * Contains number only of meaningful options, i.e. #MHD_OPTION_END and
   * #MHD_OPTION_ARRAY themselves are not counted, while options inside
   * #MHD_OPTION_ARRAY are counted.
   */
  size_t num_opts;
  /**
   * Set to 'true' if @a fdset_size is set by application.
   */
  bool fdset_size_set;
  /**
   * The value for #MHD_OPTION_APP_FD_SETSIZE set by application.
   */
  int fdset_size;
  /**
   * Set to 'true' if @a listen_fd is set by application.
   */
  bool listen_fd_set;
  /**
   * Application-provided listen socket.
   */
  MHD_socket listen_fd;
  /**
   * Set to 'true' if @a server_addr is set by application.
   */
  bool pserver_addr_set;
  /**
   * Application-provided struct sockaddr to bind server to.
   */
  const struct sockaddr *pserver_addr;
  /**
   * Set to 'true' if @a server_addr_len is set by application.
   */
  bool server_addr_len_set;
  /**
   * Applicaiton-provided the size of the memory pointed by @a server_addr.
   */
  socklen_t server_addr_len;
};

/**
 * Signature of the MHD custom logger function.
 *
 * @param cls closure
 * @param format format string
 * @param va arguments to the format string (fprintf-style)
 */
typedef void
(*VfprintfFunctionPointerType)(void *cls,
                               const char *format,
                               va_list va);


/**
 * Parse a list of options given as varargs.
 *
 * @param daemon the daemon to initialize
 * @param servaddr where to store the server's listen address
 * @param params the interim parameters to be assigned to
 * @param ap the options
 * @return #MHD_YES on success, #MHD_NO on error
 */
static enum MHD_Result
parse_options_va (struct MHD_Daemon *daemon,
                  struct MHD_InterimParams_ *params,
                  va_list ap);


/**
 * Parse a list of options given as varargs.
 *
 * @param daemon the daemon to initialize
 * @param servaddr where to store the server's listen address
 * @param params the interim parameters to be assigned to
 * @param ... the options
 * @return #MHD_YES on success, #MHD_NO on error
 */
static enum MHD_Result
parse_options (struct MHD_Daemon *daemon,
               struct MHD_InterimParams_ *params,
               ...)
{
  va_list ap;
  enum MHD_Result ret;

  va_start (ap, params);
  ret = parse_options_va (daemon,
                          params,
                          ap);
  va_end (ap);
  return ret;
}


#ifdef HTTPS_SUPPORT
/**
 * Type of GnuTLS priorities base string
 */
enum MHD_TlsPrioritiesBaseType
{
  MHD_TLS_PRIO_BASE_LIBMHD = 0, /**< @c "@LIBMICROHTTPD" */
  MHD_TLS_PRIO_BASE_SYSTEM = 1, /**< @c "@SYSTEM" */
#if GNUTLS_VERSION_NUMBER >= 0x030300
  MHD_TLS_PRIO_BASE_DEFAULT,    /**< Default priorities string */
#endif /* GNUTLS_VERSION_NUMBER >= 0x030300 */
  MHD_TLS_PRIO_BASE_NORMAL      /**< @c "NORMAL */
};

static const struct _MHD_cstr_w_len MHD_TlsBasePriotities[] = {
  _MHD_S_STR_W_LEN ("@LIBMICROHTTPD"),
  _MHD_S_STR_W_LEN ("@SYSTEM"),
#if GNUTLS_VERSION_NUMBER >= 0x030300
  {NULL, 0},
#endif /* GNUTLS_VERSION_NUMBER >= 0x030300 */
  _MHD_S_STR_W_LEN ("NORMAL")
};

/**
 * Initialise TLS priorities with default settings
 * @param daemon the daemon to initialise TLS priorities
 * @return true on success, false on error
 */
static bool
daemon_tls_priorities_init_default (struct MHD_Daemon *daemon)
{
  unsigned int p;
  int res;

  mhd_assert (0 != (((unsigned int) daemon->options) & MHD_USE_TLS));
  mhd_assert (NULL == daemon->priority_cache);
  mhd_assert (MHD_TLS_PRIO_BASE_NORMAL + 1 == \
              sizeof(MHD_TlsBasePriotities) / sizeof(MHD_TlsBasePriotities[0]));

  res = GNUTLS_E_SUCCESS; /* Mute compiler warning */

  for (p = 0;
       p < sizeof(MHD_TlsBasePriotities) / sizeof(MHD_TlsBasePriotities[0]);
       ++p)
  {
    res = gnutls_priority_init (&daemon->priority_cache,
                                MHD_TlsBasePriotities[p].str, NULL);
    if (GNUTLS_E_SUCCESS == res)
    {
#ifdef _DEBUG
#ifdef HAVE_MESSAGES
      switch ((enum MHD_TlsPrioritiesBaseType) p)
      {
      case MHD_TLS_PRIO_BASE_LIBMHD:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "@LIBMICROHTTPD application-specific system-wide " \
                     "configuration.\n") );
        break;
      case MHD_TLS_PRIO_BASE_SYSTEM:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "@SYSTEM system-wide configuration.\n") );
        break;
#if GNUTLS_VERSION_NUMBER >= 0x030300
      case MHD_TLS_PRIO_BASE_DEFAULT:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "GnuTLS default configuration.\n") );
        break;
#endif /* GNUTLS_VERSION_NUMBER >= 0x030300 */
      case MHD_TLS_PRIO_BASE_NORMAL:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "NORMAL configuration.\n") );
        break;
      default:
        mhd_assert (0);
      }
#endif /* HAVE_MESSAGES */
#endif /* _DEBUG */
      return true;
    }
  }
#ifdef HAVE_MESSAGES
  MHD_DLOG (daemon,
            _ ("Failed to set GnuTLS priorities. Last error: %s\n"),
            gnutls_strerror (res));
#endif /* HAVE_MESSAGES */
  return false;
}


/**
 * The inner helper function for #daemon_tls_priorities_init_app().
 * @param daemon the daemon to use
 * @param prio   the appication-specified appendix for default priorities
 * @param prio_len the length of @a prio
 * @param buf    the temporal buffer for string manipulations
 * @param buf_size the size of the @a buf
 * @return true on success, false on error
 */
static bool
daemon_tls_priorities_init_append_inner_ (struct MHD_Daemon *daemon,
                                          const char *prio,
                                          size_t prio_len,
                                          char *buf,
                                          const size_t buf_size)
{
  unsigned int p;
  int res;
  const char *err_pos;

  (void) buf_size; /* Mute compiler warning for non-Debug builds */
  mhd_assert (0 != (((unsigned int) daemon->options) & MHD_USE_TLS));
  mhd_assert (NULL == daemon->priority_cache);
  mhd_assert (MHD_TLS_PRIO_BASE_NORMAL + 1 == \
              sizeof(MHD_TlsBasePriotities) / sizeof(MHD_TlsBasePriotities[0]));

  res = GNUTLS_E_SUCCESS; /* Mute compiler warning */

  for (p = 0;
       p < sizeof(MHD_TlsBasePriotities) / sizeof(MHD_TlsBasePriotities[0]);
       ++p)
  {

#if GNUTLS_VERSION_NUMBER >= 0x030300
#if GNUTLS_VERSION_NUMBER >= 0x030603
    if (NULL == MHD_TlsBasePriotities[p].str)
      res = gnutls_priority_init2 (&daemon->priority_cache, prio, &err_pos,
                                   GNUTLS_PRIORITY_INIT_DEF_APPEND);
    else
#else /* 0x030300 <= GNUTLS_VERSION_NUMBER
         && GNUTLS_VERSION_NUMBER < 0x030603 */
    if (NULL == MHD_TlsBasePriotities[p].str)
      continue; /* Skip the value, no way to append priorities to the default string */
    else
#endif /* GNUTLS_VERSION_NUMBER < 0x030603 */
#endif /* GNUTLS_VERSION_NUMBER >= 0x030300 */
    if (1)
    {
      size_t buf_pos;

      mhd_assert (NULL != MHD_TlsBasePriotities[p].str);
      buf_pos = 0;
      memcpy (buf + buf_pos, MHD_TlsBasePriotities[p].str,
              MHD_TlsBasePriotities[p].len);
      buf_pos += MHD_TlsBasePriotities[p].len;
      buf[buf_pos++] = ':';
      memcpy (buf + buf_pos, prio, prio_len + 1);
#ifdef _DEBUG
      buf_pos += prio_len + 1;
      mhd_assert (buf_size >= buf_pos);
#endif /* _DEBUG */
      res = gnutls_priority_init (&daemon->priority_cache, buf, &err_pos);
    }
    if (GNUTLS_E_SUCCESS == res)
    {
#ifdef _DEBUG
#ifdef HAVE_MESSAGES
      switch ((enum MHD_TlsPrioritiesBaseType) p)
      {
      case MHD_TLS_PRIO_BASE_LIBMHD:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "priorities specified by application appended to " \
                     "@LIBMICROHTTPD application-specific system-wide " \
                     "configuration.\n") );
        break;
      case MHD_TLS_PRIO_BASE_SYSTEM:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "priorities specified by application appended to " \
                     "@SYSTEM system-wide configuration.\n") );
        break;
#if GNUTLS_VERSION_NUMBER >= 0x030300
      case MHD_TLS_PRIO_BASE_DEFAULT:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "priorities specified by application appended to " \
                     "GnuTLS default configuration.\n") );
        break;
#endif /* GNUTLS_VERSION_NUMBER >= 0x030300 */
      case MHD_TLS_PRIO_BASE_NORMAL:
        MHD_DLOG (daemon,
                  _ ("GnuTLS priorities have been initialised with " \
                     "priorities specified by application appended to " \
                     "NORMAL configuration.\n") );
        break;
      default:
        mhd_assert (0);
      }
#endif /* HAVE_MESSAGES */
#endif /* _DEBUG */
      return true;
    }
  }
#ifdef HAVE_MESSAGES
  MHD_DLOG (daemon,
            _ ("Failed to set GnuTLS priorities. Last error: %s. " \
               "The problematic part starts at: %s\n"),
            gnutls_strerror (res), err_pos);
#endif /* HAVE_MESSAGES */
  return false;
}


#define LOCAL_BUFF_SIZE 128

/**
 * Initialise TLS priorities with default settings with application-specified
 * appended string.
 * @param daemon the daemon to initialise TLS priorities
 * @param prio the application specified priorities to be appended to
 *             the GnuTLS standard priorities string
 * @return true on success, false on error
 */
static bool
daemon_tls_priorities_init_append (struct MHD_Daemon *daemon, const char *prio)
{
  static const size_t longest_base_prio = MHD_STATICSTR_LEN_ ("@LIBMICROHTTPD");
  bool ret;
  size_t prio_len;
  size_t buf_size_needed;

  if (NULL == prio)
    return daemon_tls_priorities_init_default (daemon);

  if (':' == prio[0])
    ++prio;

  prio_len = strlen (prio);

  buf_size_needed = longest_base_prio + 1 + prio_len + 1;

  if (LOCAL_BUFF_SIZE >= buf_size_needed)
  {
    char local_buffer[LOCAL_BUFF_SIZE];
    ret = daemon_tls_priorities_init_append_inner_ (daemon, prio, prio_len,
                                                    local_buffer,
                                                    LOCAL_BUFF_SIZE);
  }
  else
  {
    char *allocated_buffer;
    allocated_buffer = (char *) malloc (buf_size_needed);
    if (NULL == allocated_buffer)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Error allocating memory: %s\n"),
                MHD_strerror_ (errno));
#endif
      return false;
    }
    ret = daemon_tls_priorities_init_append_inner_ (daemon, prio, prio_len,
                                                    allocated_buffer,
                                                    buf_size_needed);
    free (allocated_buffer);
  }
  return ret;
}


#endif /* HTTPS_SUPPORT */


/**
 * Parse a list of options given as varargs.
 *
 * @param[in,out] daemon the daemon to initialize
 * @param[out] params the interim parameters to be assigned to
 * @param ap the options
 * @return #MHD_YES on success, #MHD_NO on error
 */
static enum MHD_Result
parse_options_va (struct MHD_Daemon *daemon,
                  struct MHD_InterimParams_ *params,
                  va_list ap)
{
  enum MHD_OPTION opt;
  struct MHD_OptionItem *oa;
  unsigned int i;
  unsigned int uv;
#ifdef HTTPS_SUPPORT
  const char *pstr;
#if GNUTLS_VERSION_MAJOR >= 3
  gnutls_certificate_retrieve_function2 * pgcrf;
#endif
#if GNUTLS_VERSION_NUMBER >= 0x030603
  gnutls_certificate_retrieve_function3 * pgcrf2;
#endif
#endif /* HTTPS_SUPPORT */

  while (MHD_OPTION_END != (opt = (enum MHD_OPTION) va_arg (ap, int)))
  {
    /* Increase counter at start, so resulting value is number of
     * processed options, including any failed ones. */
    params->num_opts++;
    switch (opt)
    {
    case MHD_OPTION_CONNECTION_MEMORY_LIMIT:
      if (1)
      {
        size_t val;

        val = va_arg (ap,
                      size_t);
        if (0 != val)
        {
          daemon->pool_size = val;
          if (64 > daemon->pool_size)
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("Warning: specified " \
                         "MHD_OPTION_CONNECTION_MEMORY_LIMIT " \
                         "value is too small and rounded up to 64.\n"));
#endif /* HAVE_MESSAGES */
            daemon->pool_size = 64;
          }
          if (daemon->pool_size / 4 < daemon->pool_increment)
            daemon->pool_increment = daemon->pool_size / 4;
        }
      }
      break;
    case MHD_OPTION_CONNECTION_MEMORY_INCREMENT:
      if (1)
      {
        size_t val;

        val = va_arg (ap,
                      size_t);

        if (0 != val)
        {
          daemon->pool_increment = val;
          if (daemon->pool_size / 4 < daemon->pool_increment)
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("Warning: specified " \
                         "MHD_OPTION_CONNECTION_MEMORY_INCREMENT value is " \
                         "too large and rounded down to 1/4 of " \
                         "MHD_OPTION_CONNECTION_MEMORY_LIMIT.\n"));
#endif /* HAVE_MESSAGES */
            daemon->pool_increment = daemon->pool_size / 4;
          }
        }
      }
      break;
    case MHD_OPTION_CONNECTION_LIMIT:
      daemon->connection_limit = va_arg (ap,
                                         unsigned int);
      break;
    case MHD_OPTION_CONNECTION_TIMEOUT:
      uv = va_arg (ap,
                   unsigned int);
#if (SIZEOF_UINT64_T - 2) <= SIZEOF_UNSIGNED_INT
      if ((UINT64_MAX / 4000 - 1) < uv)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("The specified connection timeout (%u) is too large. " \
                     "Maximum allowed value (%" PRIu64 ") will be used " \
                     "instead.\n"),
                  uv,
                  (UINT64_MAX / 4000 - 1));
#endif
        uv = UINT64_MAX / 4000 - 1;
      }
#endif /* (SIZEOF_UINT64_T - 2) <= SIZEOF_UNSIGNED_INT */
      daemon->connection_timeout_ms = ((uint64_t) uv) * 1000;
      break;
    case MHD_OPTION_NOTIFY_COMPLETED:
      daemon->notify_completed = va_arg (ap,
                                         MHD_RequestCompletedCallback);
      daemon->notify_completed_cls = va_arg (ap,
                                             void *);
      break;
    case MHD_OPTION_NOTIFY_CONNECTION:
      daemon->notify_connection = va_arg (ap,
                                          MHD_NotifyConnectionCallback);
      daemon->notify_connection_cls = va_arg (ap,
                                              void *);
      break;
    case MHD_OPTION_PER_IP_CONNECTION_LIMIT:
      daemon->per_ip_connection_limit = va_arg (ap,
                                                unsigned int);
      break;
    case MHD_OPTION_SOCK_ADDR_LEN:
      params->server_addr_len = va_arg (ap,
                                        socklen_t);
      params->server_addr_len_set = true;
      params->pserver_addr = va_arg (ap,
                                     const struct sockaddr *);
      params->pserver_addr_set = true;
      break;
    case MHD_OPTION_SOCK_ADDR:
      params->server_addr_len_set = false;
      params->pserver_addr = va_arg (ap,
                                     const struct sockaddr *);
      params->pserver_addr_set = true;
      break;
    case MHD_OPTION_URI_LOG_CALLBACK:
      daemon->uri_log_callback = va_arg (ap,
                                         LogCallback);
      daemon->uri_log_callback_cls = va_arg (ap,
                                             void *);
      break;
    case MHD_OPTION_SERVER_INSANITY:
      daemon->insanity_level = (enum MHD_DisableSanityCheck)
                               va_arg (ap,
                                       unsigned int);
      break;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    case MHD_OPTION_THREAD_POOL_SIZE:
      daemon->worker_pool_size = va_arg (ap,
                                         unsigned int);
      if (0 == daemon->worker_pool_size)
      {
        (void) 0; /* MHD_OPTION_THREAD_POOL_SIZE ignored, do nothing */
      }
      else if (1 == daemon->worker_pool_size)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Warning: value \"1\", specified as the thread pool " \
                     "size, is ignored. Thread pool is not used.\n"));
#endif
        daemon->worker_pool_size = 0;
      }
#if SIZEOF_UNSIGNED_INT >= (SIZEOF_SIZE_T - 2)
      /* Next comparison could be always false on some platforms and whole branch will
       * be optimized out on these platforms. On others it will be compiled into real
       * check. */
      else if (daemon->worker_pool_size >=
               (SIZE_MAX / sizeof (struct MHD_Daemon)))            /* Compiler may warn on some platforms, ignore warning. */
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Specified thread pool size (%u) too big.\n"),
                  daemon->worker_pool_size);
#endif
        return MHD_NO;
      }
#endif /* SIZEOF_UNSIGNED_INT >= (SIZEOF_SIZE_T - 2) */
      else
      {
        if (! MHD_D_IS_USING_THREADS_ (daemon))
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("MHD_OPTION_THREAD_POOL_SIZE option is specified but "
                       "MHD_USE_INTERNAL_POLLING_THREAD flag is not specified.\n"));
#endif
          return MHD_NO;
        }
        if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Both MHD_OPTION_THREAD_POOL_SIZE option and "
                       "MHD_USE_THREAD_PER_CONNECTION flag are specified.\n"));
#endif
          return MHD_NO;
        }
      }
      break;
#endif
#ifdef HTTPS_SUPPORT
    case MHD_OPTION_HTTPS_MEM_KEY:
      pstr = va_arg (ap,
                     const char *);
      if (0 != (daemon->options & MHD_USE_TLS))
        daemon->https_mem_key = pstr;
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif
      break;
    case MHD_OPTION_HTTPS_KEY_PASSWORD:
      pstr = va_arg (ap,
                     const char *);
      if (0 != (daemon->options & MHD_USE_TLS))
        daemon->https_key_password = pstr;
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif
      break;
    case MHD_OPTION_HTTPS_MEM_CERT:
      pstr = va_arg (ap,
                     const char *);
      if (0 != (daemon->options & MHD_USE_TLS))
        daemon->https_mem_cert = pstr;
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif
      break;
    case MHD_OPTION_HTTPS_MEM_TRUST:
      pstr = va_arg (ap,
                     const char *);
      if (0 != (daemon->options & MHD_USE_TLS))
        daemon->https_mem_trust = pstr;
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif
      break;
    case MHD_OPTION_HTTPS_CRED_TYPE:
      daemon->cred_type = (gnutls_credentials_type_t) va_arg (ap,
                                                              int);
      break;
    case MHD_OPTION_HTTPS_MEM_DHPARAMS:
      pstr = va_arg (ap,
                     const char *);
      if (0 != (daemon->options & MHD_USE_TLS))
      {
        gnutls_datum_t dhpar;
        size_t pstr_len;

        if (gnutls_dh_params_init (&daemon->https_mem_dhparams) < 0)
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Error initializing DH parameters.\n"));
#endif
          return MHD_NO;
        }
        dhpar.data = (unsigned char *) _MHD_DROP_CONST (pstr);
        pstr_len = strlen (pstr);
        if (UINT_MAX < pstr_len)
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Diffie-Hellman parameters string too long.\n"));
#endif
          return MHD_NO;
        }
        dhpar.size = (unsigned int) pstr_len;
        if (gnutls_dh_params_import_pkcs3 (daemon->https_mem_dhparams,
                                           &dhpar,
                                           GNUTLS_X509_FMT_PEM) < 0)
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Bad Diffie-Hellman parameters format.\n"));
#endif
          gnutls_dh_params_deinit (daemon->https_mem_dhparams);
          return MHD_NO;
        }
        daemon->have_dhparams = true;
      }
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif
      break;
    case MHD_OPTION_HTTPS_PRIORITIES:
    case MHD_OPTION_HTTPS_PRIORITIES_APPEND:
      pstr = va_arg (ap,
                     const char *);
      if (0 != (daemon->options & MHD_USE_TLS))
      {
        if (NULL != daemon->priority_cache)
          gnutls_priority_deinit (daemon->priority_cache);

        if (MHD_OPTION_HTTPS_PRIORITIES == opt)
        {
          int init_res;
          const char *err_pos;
          init_res = gnutls_priority_init (&daemon->priority_cache,
                                           pstr,
                                           &err_pos);
          if (GNUTLS_E_SUCCESS != init_res)
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("Setting priorities to '%s' failed: %s " \
                         "The problematic part starts at: %s\n"),
                      pstr,
                      gnutls_strerror (init_res),
                      err_pos);
#endif
            daemon->priority_cache = NULL;
            return MHD_NO;
          }
        }
        else
        {
          /* The cache has been deinited */
          daemon->priority_cache = NULL;
          if (! daemon_tls_priorities_init_append (daemon, pstr))
            return MHD_NO;
        }
      }
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif
      break;
    case MHD_OPTION_HTTPS_CERT_CALLBACK:
#if GNUTLS_VERSION_MAJOR < 3
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("MHD_OPTION_HTTPS_CERT_CALLBACK requires building " \
                   "MHD with GnuTLS >= 3.0.\n"));
#endif
      return MHD_NO;
#else
      pgcrf = va_arg (ap,
                      gnutls_certificate_retrieve_function2 *);
      if (0 != (daemon->options & MHD_USE_TLS))
        daemon->cert_callback = pgcrf;
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif /*  HAVE_MESSAGES */
      break;
#endif
    case MHD_OPTION_HTTPS_CERT_CALLBACK2:
#if GNUTLS_VERSION_NUMBER < 0x030603
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("MHD_OPTION_HTTPS_CERT_CALLBACK2 requires building " \
                   "MHD with GnuTLS >= 3.6.3.\n"));
#endif
      return MHD_NO;
#else
      pgcrf2 = va_arg (ap,
                       gnutls_certificate_retrieve_function3 *);
      if (0 != (daemon->options & MHD_USE_TLS))
        daemon->cert_callback2 = pgcrf2;
#ifdef HAVE_MESSAGES
      else
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD but " \
                     "MHD_USE_TLS not set.\n"),
                  opt);
#endif /* HAVE_MESSAGES */
      break;
#endif
#endif /* HTTPS_SUPPORT */
#ifdef DAUTH_SUPPORT
    case MHD_OPTION_DIGEST_AUTH_RANDOM:
    case MHD_OPTION_DIGEST_AUTH_RANDOM_COPY:
      daemon->digest_auth_rand_size = va_arg (ap,
                                              size_t);
      daemon->digest_auth_random = va_arg (ap,
                                           const char *);
      if (MHD_OPTION_DIGEST_AUTH_RANDOM_COPY == opt)
        /* Set to some non-NULL value just to indicate that copy is required. */
        daemon->digest_auth_random_copy = daemon;
      else
        daemon->digest_auth_random_copy = NULL;
      break;
    case MHD_OPTION_NONCE_NC_SIZE:
      daemon->nonce_nc_size = va_arg (ap,
                                      unsigned int);
      break;
    case MHD_OPTION_DIGEST_AUTH_NONCE_BIND_TYPE:
      daemon->dauth_bind_type = va_arg (ap,
                                        unsigned int);
      if (0 != (daemon->dauth_bind_type & MHD_DAUTH_BIND_NONCE_URI_PARAMS))
        daemon->dauth_bind_type |= MHD_DAUTH_BIND_NONCE_URI;
      break;
    case MHD_OPTION_DIGEST_AUTH_DEFAULT_NONCE_TIMEOUT:
      if (1)
      {
        unsigned int val;
        val = va_arg (ap,
                      unsigned int);
        if (0 != val)
          daemon->dauth_def_nonce_timeout = val;
      }
      break;
    case MHD_OPTION_DIGEST_AUTH_DEFAULT_MAX_NC:
      if (1)
      {
        uint32_t val;
        val = va_arg (ap,
                      uint32_t);
        if (0 != val)
          daemon->dauth_def_max_nc = val;
      }
      break;
#else  /* ! DAUTH_SUPPORT */
    case MHD_OPTION_DIGEST_AUTH_RANDOM:
    case MHD_OPTION_DIGEST_AUTH_RANDOM_COPY:
    case MHD_OPTION_NONCE_NC_SIZE:
    case MHD_OPTION_DIGEST_AUTH_NONCE_BIND_TYPE:
    case MHD_OPTION_DIGEST_AUTH_DEFAULT_NONCE_TIMEOUT:
    case MHD_OPTION_DIGEST_AUTH_DEFAULT_MAX_NC:
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Digest Auth is disabled for this build " \
                   "of GNU libmicrohttpd.\n"));
#endif /* HAVE_MESSAGES */
      return MHD_NO;
#endif /* ! DAUTH_SUPPORT */
    case MHD_OPTION_LISTEN_SOCKET:
      params->listen_fd = va_arg (ap,
                                  MHD_socket);
      params->listen_fd_set = true;
      break;
    case MHD_OPTION_EXTERNAL_LOGGER:
#ifdef HAVE_MESSAGES
      daemon->custom_error_log = va_arg (ap,
                                         VfprintfFunctionPointerType);
      daemon->custom_error_log_cls = va_arg (ap,
                                             void *);
      if (1 != params->num_opts)
        MHD_DLOG (daemon,
                  _ ("MHD_OPTION_EXTERNAL_LOGGER is not the first option "
                     "specified for the daemon. Some messages may be "
                     "printed by the standard MHD logger.\n"));

#else
      (void) va_arg (ap,
                     VfprintfFunctionPointerType);
      (void) va_arg (ap,
                     void *);
#endif
      break;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    case MHD_OPTION_THREAD_STACK_SIZE:
      daemon->thread_stack_size = va_arg (ap,
                                          size_t);
      break;
#endif
    case MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE:
#ifdef TCP_FASTOPEN
      daemon->fastopen_queue_size = va_arg (ap,
                                            unsigned int);
      break;
#else  /* ! TCP_FASTOPEN */
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("TCP fastopen is not supported on this platform.\n"));
#endif /* HAVE_MESSAGES */
      return MHD_NO;
#endif /* ! TCP_FASTOPEN */
    case MHD_OPTION_LISTENING_ADDRESS_REUSE:
      daemon->listening_address_reuse = va_arg (ap,
                                                unsigned int) ? 1 : -1;
      break;
    case MHD_OPTION_LISTEN_BACKLOG_SIZE:
      daemon->listen_backlog_size = va_arg (ap,
                                            unsigned int);
      break;
    case MHD_OPTION_STRICT_FOR_CLIENT:
      daemon->client_discipline = va_arg (ap, int); /* Temporal assignment */
      /* Map to correct value */
      if (-1 >= daemon->client_discipline)
        daemon->client_discipline = -3;
      else if (1 <= daemon->client_discipline)
        daemon->client_discipline = 1;
#ifdef HAVE_MESSAGES
      if ( (0 != (daemon->options & MHD_USE_PEDANTIC_CHECKS)) &&
           (1 != daemon->client_discipline) )
      {
        MHD_DLOG (daemon,
                  _ ("Flag MHD_USE_PEDANTIC_CHECKS is ignored because "
                     "another behaviour is specified by "
                     "MHD_OPTION_STRICT_CLIENT.\n"));
      }
#endif /* HAVE_MESSAGES */
      break;
    case MHD_OPTION_CLIENT_DISCIPLINE_LVL:
      daemon->client_discipline = va_arg (ap, int);
#ifdef HAVE_MESSAGES
      if ( (0 != (daemon->options & MHD_USE_PEDANTIC_CHECKS)) &&
           (1 != daemon->client_discipline) )
      {
        MHD_DLOG (daemon,
                  _ ("Flag MHD_USE_PEDANTIC_CHECKS is ignored because "
                     "another behaviour is specified by "
                     "MHD_OPTION_CLIENT_DISCIPLINE_LVL.\n"));
      }
#endif /* HAVE_MESSAGES */
      break;
    case MHD_OPTION_ARRAY:
      params->num_opts--; /* Do not count MHD_OPTION_ARRAY */
      oa = va_arg (ap, struct MHD_OptionItem *);
      i = 0;
      while (MHD_OPTION_END != (opt = oa[i].option))
      {
        switch (opt)
        {
        /* all options taking 'size_t' */
        case MHD_OPTION_CONNECTION_MEMORY_LIMIT:
        case MHD_OPTION_CONNECTION_MEMORY_INCREMENT:
        case MHD_OPTION_THREAD_STACK_SIZE:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (size_t) oa[i].value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* all options taking 'unsigned int' */
        case MHD_OPTION_NONCE_NC_SIZE:
        case MHD_OPTION_CONNECTION_LIMIT:
        case MHD_OPTION_CONNECTION_TIMEOUT:
        case MHD_OPTION_PER_IP_CONNECTION_LIMIT:
        case MHD_OPTION_THREAD_POOL_SIZE:
        case MHD_OPTION_TCP_FASTOPEN_QUEUE_SIZE:
        case MHD_OPTION_LISTENING_ADDRESS_REUSE:
        case MHD_OPTION_LISTEN_BACKLOG_SIZE:
        case MHD_OPTION_SERVER_INSANITY:
        case MHD_OPTION_DIGEST_AUTH_NONCE_BIND_TYPE:
        case MHD_OPTION_DIGEST_AUTH_DEFAULT_NONCE_TIMEOUT:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (unsigned int) oa[i].value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* all options taking 'enum' */
        case MHD_OPTION_HTTPS_CRED_TYPE:
#ifdef HTTPS_SUPPORT
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (gnutls_credentials_type_t) oa[i].value,
                                       MHD_OPTION_END))
#endif /* HTTPS_SUPPORT */
          return MHD_NO;
          break;
        /* all options taking 'MHD_socket' */
        case MHD_OPTION_LISTEN_SOCKET:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (MHD_socket) oa[i].value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* all options taking 'int' */
        case MHD_OPTION_STRICT_FOR_CLIENT:
        case MHD_OPTION_CLIENT_DISCIPLINE_LVL:
        case MHD_OPTION_SIGPIPE_HANDLED_BY_APP:
        case MHD_OPTION_TLS_NO_ALPN:
        case MHD_OPTION_APP_FD_SETSIZE:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (int) oa[i].value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* all options taking 'uint32_t' */
        case MHD_OPTION_DIGEST_AUTH_DEFAULT_MAX_NC:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (uint32_t) oa[i].value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* all options taking one pointer */
        case MHD_OPTION_SOCK_ADDR:
        case MHD_OPTION_HTTPS_MEM_KEY:
        case MHD_OPTION_HTTPS_KEY_PASSWORD:
        case MHD_OPTION_HTTPS_MEM_CERT:
        case MHD_OPTION_HTTPS_MEM_TRUST:
        case MHD_OPTION_HTTPS_MEM_DHPARAMS:
        case MHD_OPTION_HTTPS_PRIORITIES:
        case MHD_OPTION_HTTPS_PRIORITIES_APPEND:
        case MHD_OPTION_ARRAY:
        case MHD_OPTION_HTTPS_CERT_CALLBACK:
        case MHD_OPTION_HTTPS_CERT_CALLBACK2:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       oa[i].ptr_value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* all options taking two pointers */
        case MHD_OPTION_NOTIFY_COMPLETED:
        case MHD_OPTION_NOTIFY_CONNECTION:
        case MHD_OPTION_URI_LOG_CALLBACK:
        case MHD_OPTION_EXTERNAL_LOGGER:
        case MHD_OPTION_UNESCAPE_CALLBACK:
        case MHD_OPTION_GNUTLS_PSK_CRED_HANDLER:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (void *) oa[i].value,
                                       oa[i].ptr_value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* options taking size_t-number followed by pointer */
        case MHD_OPTION_DIGEST_AUTH_RANDOM:
        case MHD_OPTION_DIGEST_AUTH_RANDOM_COPY:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (size_t) oa[i].value,
                                       oa[i].ptr_value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        /* options taking socklen_t-number followed by pointer */
        case MHD_OPTION_SOCK_ADDR_LEN:
          if (MHD_NO == parse_options (daemon,
                                       params,
                                       opt,
                                       (socklen_t) oa[i].value,
                                       oa[i].ptr_value,
                                       MHD_OPTION_END))
            return MHD_NO;
          break;
        case MHD_OPTION_END: /* Not possible */
        default:
          return MHD_NO;
        }
        i++;
      }
      break;
    case MHD_OPTION_UNESCAPE_CALLBACK:
      daemon->unescape_callback = va_arg (ap,
                                          UnescapeCallback);
      daemon->unescape_callback_cls = va_arg (ap,
                                              void *);
      break;
#ifdef HTTPS_SUPPORT
    case MHD_OPTION_GNUTLS_PSK_CRED_HANDLER:
#if GNUTLS_VERSION_MAJOR >= 3
      daemon->cred_callback = va_arg (ap,
                                      MHD_PskServerCredentialsCallback);
      daemon->cred_callback_cls = va_arg (ap,
                                          void *);
      break;
#else
      MHD_DLOG (daemon,
                _ ("MHD HTTPS option %d passed to MHD compiled " \
                   "without GNUtls >= 3.\n"),
                opt);
      return MHD_NO;
#endif
#endif /* HTTPS_SUPPORT */
    case MHD_OPTION_SIGPIPE_HANDLED_BY_APP:
      if (! MHD_D_IS_USING_THREADS_ (daemon))
        daemon->sigpipe_blocked = ( (va_arg (ap,
                                             int)) != 0);
      else
      {
        (void) va_arg (ap,
                       int);
      }
      break;
    case MHD_OPTION_TLS_NO_ALPN:
#ifdef HTTPS_SUPPORT
      daemon->disable_alpn = (va_arg (ap,
                                      int) != 0);
#else  /* ! HTTPS_SUPPORT */
      (void) va_arg (ap, int);
#endif /* ! HTTPS_SUPPORT */
#ifdef HAVE_MESSAGES
      if (0 == (daemon->options & MHD_USE_TLS))
        MHD_DLOG (daemon,
                  _ ("MHD HTTPS option %d passed to MHD " \
                     "but MHD_USE_TLS not set.\n"),
                  (int) opt);
#endif /* HAVE_MESSAGES */
      break;
    case MHD_OPTION_APP_FD_SETSIZE:
      params->fdset_size_set = true;
      params->fdset_size = va_arg (ap,
                                   int);
      break;
#ifndef HTTPS_SUPPORT
    case MHD_OPTION_HTTPS_MEM_KEY:
    case MHD_OPTION_HTTPS_MEM_CERT:
    case MHD_OPTION_HTTPS_CRED_TYPE:
    case MHD_OPTION_HTTPS_PRIORITIES:
    case MHD_OPTION_HTTPS_PRIORITIES_APPEND:
    case MHD_OPTION_HTTPS_MEM_TRUST:
    case MHD_OPTION_HTTPS_CERT_CALLBACK:
    case MHD_OPTION_HTTPS_MEM_DHPARAMS:
    case MHD_OPTION_HTTPS_KEY_PASSWORD:
    case MHD_OPTION_GNUTLS_PSK_CRED_HANDLER:
    case MHD_OPTION_HTTPS_CERT_CALLBACK2:
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("MHD HTTPS option %d passed to MHD "
                   "compiled without HTTPS support.\n"),
                opt);
#endif
      return MHD_NO;
#endif /* HTTPS_SUPPORT */
    case MHD_OPTION_END: /* Not possible */
    default:
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Invalid option %d! (Did you terminate "
                   "the list with MHD_OPTION_END?).\n"),
                opt);
#endif
      return MHD_NO;
    }
  }
  return MHD_YES;
}


#ifdef EPOLL_SUPPORT
static int
setup_epoll_fd (struct MHD_Daemon *daemon)
{
  int fd;

#ifndef HAVE_MESSAGES
  (void) daemon; /* Mute compiler warning. */
#endif /* ! HAVE_MESSAGES */

#ifdef USE_EPOLL_CREATE1
  fd = epoll_create1 (EPOLL_CLOEXEC);
#else  /* ! USE_EPOLL_CREATE1 */
  fd = epoll_create (MAX_EVENTS);
#endif /* ! USE_EPOLL_CREATE1 */
  if (MHD_INVALID_SOCKET == fd)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Call to epoll_create1 failed: %s\n"),
              MHD_socket_last_strerr_ ());
#endif
    return MHD_INVALID_SOCKET;
  }
#if ! defined(USE_EPOLL_CREATE1)
  if (! MHD_socket_noninheritable_ (fd))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to set noninheritable mode on epoll FD.\n"));
#endif
  }
#endif /* ! USE_EPOLL_CREATE1 */
  return fd;
}


/**
 * Setup epoll() FD for the daemon and initialize it to listen
 * on the listen FD.
 * @remark To be called only from MHD_start_daemon_va()
 *
 * @param daemon daemon to initialize for epoll()
 * @return #MHD_YES on success, #MHD_NO on failure
 */
static enum MHD_Result
setup_epoll_to_listen (struct MHD_Daemon *daemon)
{
  struct epoll_event event;
  MHD_socket ls;

  mhd_assert (MHD_D_IS_USING_EPOLL_ (daemon));
  mhd_assert (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION));
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               (MHD_INVALID_SOCKET != (ls = daemon->listen_fd)) || \
               MHD_ITC_IS_VALID_ (daemon->itc) );
  daemon->epoll_fd = setup_epoll_fd (daemon);
  if (! MHD_D_IS_USING_THREADS_ (daemon)
      && (0 != (daemon->options & MHD_USE_AUTO)))
  {
    /* Application requested "MHD_USE_AUTO", probably MHD_get_fdset() will be
       used.
       Make sure that epoll FD is suitable for fd_set.
       Actually, MHD_get_fdset() is allowed for MHD_USE_EPOLL direct,
       but most probably direct requirement for MHD_USE_EPOLL means that
       epoll FD will be used directly. This logic is fuzzy, but better
       than nothing with current MHD API. */
    if (! MHD_D_DOES_SCKT_FIT_FDSET_ (daemon->epoll_fd, daemon))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("The epoll FD is too large to be used with fd_set.\n"));
#endif /* HAVE_MESSAGES */
      return MHD_NO;
    }
  }
  if (-1 == daemon->epoll_fd)
    return MHD_NO;
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  if (0 != (MHD_ALLOW_UPGRADE & daemon->options))
  {
    daemon->epoll_upgrade_fd = setup_epoll_fd (daemon);
    if (MHD_INVALID_SOCKET == daemon->epoll_upgrade_fd)
      return MHD_NO;
  }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  if ( (MHD_INVALID_SOCKET != (ls = daemon->listen_fd)) &&
       (! daemon->was_quiesced) )
  {
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.ptr = daemon;
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_ADD,
                        ls,
                        &event))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Call to epoll_ctl failed: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
    daemon->listen_socket_in_epoll = true;
  }

  if (MHD_ITC_IS_VALID_ (daemon->itc))
  {
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.ptr = _MHD_DROP_CONST (epoll_itc_marker);
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_ADD,
                        MHD_itc_r_fd_ (daemon->itc),
                        &event))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Call to epoll_ctl failed: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      return MHD_NO;
    }
  }
  return MHD_YES;
}


#endif


/**
 * Apply interim parameters
 * @param[in,out] d the daemon to use
 * @param[out] ppsockaddr the pointer to store the pointer to 'struct sockaddr'
 *                        if provided by application
 * @param[out] psockaddr_len the size memory area pointed by 'struct sockaddr'
 *                           if provided by application
 * @param[in,out] params the interim parameters to process
 * @return true in case of success,
 *         false in case of critical error (the daemon must be closed).
 */
static bool
process_interim_params (struct MHD_Daemon *d,
                        const struct sockaddr **ppsockaddr,
                        socklen_t *psockaddr_len,
                        struct MHD_InterimParams_ *params)
{
  if (params->fdset_size_set)
  {
    if (0 >= params->fdset_size)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("MHD_OPTION_APP_FD_SETSIZE value (%d) is not positive.\n"),
                params->fdset_size);
#endif /* HAVE_MESSAGES */
      return false;
    }
    if (MHD_D_IS_USING_THREADS_ (d))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("MHD_OPTION_APP_FD_SETSIZE is ignored for daemon started " \
                   "with MHD_USE_INTERNAL_POLLING_THREAD.\n"));
#endif /* HAVE_MESSAGES */
      (void) 0;
    }
    else if (MHD_D_IS_USING_POLL_ (d))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("MHD_OPTION_APP_FD_SETSIZE is ignored for daemon started " \
                   "with MHD_USE_POLL.\n"));
#endif /* HAVE_MESSAGES */
      (void) 0;
    }
    else
    { /* The daemon without internal threads, external sockets polling */
#ifndef HAS_FD_SETSIZE_OVERRIDABLE
      if (((int) FD_SETSIZE) != params->fdset_size)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (d,
                  _ ("MHD_OPTION_APP_FD_SETSIZE value (%d) does not match " \
                     "the platform FD_SETSIZE value (%d) and this platform " \
                     "does not support overriding of FD_SETSIZE.\n"),
                  params->fdset_size, (int) FD_SETSIZE);
#endif /* HAVE_MESSAGES */
        return false;
      }
#else  /* HAS_FD_SETSIZE_OVERRIDABLE */
      d->fdset_size = params->fdset_size;
      d->fdset_size_set_by_app = true;
#endif /* HAS_FD_SETSIZE_OVERRIDABLE */
    }
  }

  if (params->listen_fd_set)
  {
    if (MHD_INVALID_SOCKET == params->listen_fd)
    {
      (void) 0; /* Use MHD-created socket */
    }
#ifdef HAS_SIGNED_SOCKET
    else if (0 > params->listen_fd)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("The value provided for MHD_OPTION_LISTEN_SOCKET " \
                   "is invalid.\n"));
#endif /* HAVE_MESSAGES */
      return false;
    }
#endif /* HAS_SIGNED_SOCKET */
    else if (0 != (d->options & MHD_USE_NO_LISTEN_SOCKET))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("MHD_OPTION_LISTEN_SOCKET specified for daemon "
                   "with MHD_USE_NO_LISTEN_SOCKET flag set.\n"));
#endif /* HAVE_MESSAGES */
      (void) MHD_socket_close_ (params->listen_fd);
      return false;
    }
    else
    {
      d->listen_fd = params->listen_fd;
      d->listen_is_unix = _MHD_UNKNOWN;
#ifdef MHD_USE_GETSOCKNAME
      d->port = 0;  /* Force use of autodetection */
#endif /* MHD_USE_GETSOCKNAME */
    }
  }

  mhd_assert (! params->server_addr_len_set || params->pserver_addr_set);
  if (params->pserver_addr_set)
  {
    if (NULL == params->pserver_addr)
    {
      /* The size must be zero if set */
      if (params->server_addr_len_set && (0 != params->server_addr_len))
        return false;
      /* Ignore parameter if it is NULL */
    }
    else if (MHD_INVALID_SOCKET != d->listen_fd)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("MHD_OPTION_LISTEN_SOCKET cannot be used together with " \
                   "MHD_OPTION_SOCK_ADDR_LEN or MHD_OPTION_SOCK_ADDR.\n"));
#endif /* HAVE_MESSAGES */
      return false;
    }
    else if (0 != (d->options & MHD_USE_NO_LISTEN_SOCKET))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (d,
                _ ("MHD_OPTION_SOCK_ADDR_LEN or MHD_OPTION_SOCK_ADDR " \
                   "specified for daemon with MHD_USE_NO_LISTEN_SOCKET " \
                   "flag set.\n"));
#endif /* HAVE_MESSAGES */
      if (MHD_INVALID_SOCKET != d->listen_fd)
      {
        (void) MHD_socket_close_ (params->listen_fd);
        params->listen_fd = MHD_INVALID_SOCKET;
      }
      return false;
    }
    else
    {
      *ppsockaddr = params->pserver_addr;
      if (params->server_addr_len_set)
      {
        /* The size must be non-zero if set */
        if (0 == params->server_addr_len)
          return false;
        *psockaddr_len = params->server_addr_len;
      }
      else
        *psockaddr_len = 0;
    }
  }
  return true;
}


/**
 * Start a webserver on the given port.
 *
 * @param flags combination of `enum MHD_FLAG` values
 * @param port port to bind to (in host byte order),
 *        use '0' to bind to random free port,
 *        ignored if #MHD_OPTION_SOCK_ADDR or
 *        #MHD_OPTION_LISTEN_SOCKET is provided
 *        or #MHD_USE_NO_LISTEN_SOCKET is specified
 * @param apc callback to call to check which clients
 *        will be allowed to connect; you can pass NULL
 *        in which case connections from any IP will be
 *        accepted
 * @param apc_cls extra argument to @a apc
 * @param dh handler called for all requests (repeatedly)
 * @param dh_cls extra argument to @a dh
 * @param ap list of options (type-value pairs,
 *        terminated with #MHD_OPTION_END).
 * @return NULL on error, handle to daemon on success
 * @ingroup event
 */
_MHD_EXTERN struct MHD_Daemon *
MHD_start_daemon_va (unsigned int flags,
                     uint16_t port,
                     MHD_AcceptPolicyCallback apc,
                     void *apc_cls,
                     MHD_AccessHandlerCallback dh,
                     void *dh_cls,
                     va_list ap)
{
  const MHD_SCKT_OPT_BOOL_ on = 1;
  struct MHD_Daemon *daemon;
  MHD_socket listen_fd = MHD_INVALID_SOCKET;
  const struct sockaddr *pservaddr = NULL;
  socklen_t addrlen;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  unsigned int i;
#endif
  enum MHD_FLAG eflags; /* same type as in MHD_Daemon */
  enum MHD_FLAG *pflags;
  struct MHD_InterimParams_ *interim_params;

  MHD_check_global_init_ ();
  eflags = (enum MHD_FLAG) flags;
  pflags = &eflags;

  if (0 != (*pflags & MHD_USE_THREAD_PER_CONNECTION))
    *pflags |= MHD_USE_INTERNAL_POLLING_THREAD; /* Force enable, log warning later if needed */

#ifndef HAVE_INET6
  if (0 != (*pflags & MHD_USE_IPv6))
    return NULL;
#endif
#ifndef HAVE_POLL
  if (0 != (*pflags & MHD_USE_POLL))
    return NULL;
#endif
#ifndef EPOLL_SUPPORT
  if (0 != (*pflags & MHD_USE_EPOLL))
    return NULL;
#endif /* ! EPOLL_SUPPORT */
#ifndef HTTPS_SUPPORT
  if (0 != (*pflags & MHD_USE_TLS))
    return NULL;
#endif /* ! HTTPS_SUPPORT */
#ifndef TCP_FASTOPEN
  if (0 != (*pflags & MHD_USE_TCP_FASTOPEN))
    return NULL;
#endif
  if (0 != (*pflags & MHD_ALLOW_UPGRADE))
  {
#ifdef UPGRADE_SUPPORT
    *pflags |= MHD_ALLOW_SUSPEND_RESUME;
#else  /* ! UPGRADE_SUPPORT */
    return NULL;
#endif /* ! UPGRADE_SUPPORT */
  }
#ifdef MHD_USE_THREADS
  if ((MHD_USE_NO_THREAD_SAFETY | MHD_USE_INTERNAL_POLLING_THREAD) ==
      ((MHD_USE_NO_THREAD_SAFETY | MHD_USE_INTERNAL_POLLING_THREAD)
       & *pflags))
    return NULL; /* Cannot be thread-unsafe with multiple threads */
#else  /* ! MHD_USE_THREADS */
  if (0 != (*pflags & MHD_USE_INTERNAL_POLLING_THREAD))
    return NULL;
#endif /* ! MHD_USE_THREADS */

  if (NULL == dh)
    return NULL;

  /* Check for invalid combinations of flags. */
  if ((0 != (*pflags & MHD_USE_POLL)) && (0 != (*pflags & MHD_USE_EPOLL)))
    return NULL;
  if ((0 != (*pflags & MHD_USE_EPOLL)) &&
      (0 != (*pflags & MHD_USE_THREAD_PER_CONNECTION)))
    return NULL;
  if ((0 != (*pflags & MHD_USE_POLL)) &&
      (0 == (*pflags & (MHD_USE_INTERNAL_POLLING_THREAD
                        | MHD_USE_THREAD_PER_CONNECTION))))
    return NULL;
  if ((0 != (*pflags & MHD_USE_AUTO)) &&
      (0 != (*pflags & (MHD_USE_POLL | MHD_USE_EPOLL))))
    return NULL;

  if (0 != (*pflags & MHD_USE_AUTO))
  {
#if defined(EPOLL_SUPPORT) && defined(HAVE_POLL)
    if (0 != (*pflags & MHD_USE_THREAD_PER_CONNECTION))
      *pflags |= MHD_USE_POLL;
    else
      *pflags |= MHD_USE_EPOLL; /* Including "external select" mode */
#elif defined(HAVE_POLL)
    if (0 != (*pflags & MHD_USE_INTERNAL_POLLING_THREAD))
      *pflags |= MHD_USE_POLL; /* Including thread-per-connection */
#elif defined(EPOLL_SUPPORT)
    if (0 == (*pflags & MHD_USE_THREAD_PER_CONNECTION))
      *pflags |= MHD_USE_EPOLL; /* Including "external select" mode */
#else
    /* No choice: use select() for any mode - do not modify flags */
#endif
  }

  if (0 != (*pflags & MHD_USE_NO_THREAD_SAFETY))
    *pflags = (*pflags & ~((enum MHD_FLAG) MHD_USE_ITC)); /* useless in single-threaded environment */
  else if (0 != (*pflags & MHD_USE_INTERNAL_POLLING_THREAD))
  {
#ifdef HAVE_LISTEN_SHUTDOWN
    if (0 != (*pflags & MHD_USE_NO_LISTEN_SOCKET))
#endif
    *pflags |= MHD_USE_ITC;       /* yes, must use ITC to signal thread */
  }

  if (NULL == (daemon = MHD_calloc_ (1, sizeof (struct MHD_Daemon))))
    return NULL;
  interim_params = (struct MHD_InterimParams_ *) \
                   MHD_calloc_ (1, sizeof (struct MHD_InterimParams_));
  if (NULL == interim_params)
  {
    int err_num = errno;
    free (daemon);
    errno = err_num;
    return NULL;
  }
#ifdef EPOLL_SUPPORT
  daemon->epoll_fd = -1;
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  daemon->epoll_upgrade_fd = -1;
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
#endif
  /* try to open listen socket */
#ifdef HTTPS_SUPPORT
  daemon->priority_cache = NULL;
#endif /* HTTPS_SUPPORT */
  daemon->listen_fd = MHD_INVALID_SOCKET;
  daemon->listen_is_unix = _MHD_NO;
  daemon->listening_address_reuse = 0;
  daemon->options = *pflags;
  pflags = &daemon->options;
  daemon->client_discipline = (0 != (*pflags & MHD_USE_PEDANTIC_CHECKS)) ?
                              1 : 0;
  daemon->port = port;
  daemon->apc = apc;
  daemon->apc_cls = apc_cls;
  daemon->default_handler = dh;
  daemon->default_handler_cls = dh_cls;
  daemon->connections = 0;
  daemon->connection_limit = MHD_MAX_CONNECTIONS_DEFAULT;
  daemon->pool_size = MHD_POOL_SIZE_DEFAULT;
  daemon->pool_increment = MHD_BUF_INC_SIZE;
  daemon->unescape_callback = &unescape_wrapper;
  daemon->connection_timeout_ms = 0;       /* no timeout */
  MHD_itc_set_invalid_ (daemon->itc);
#ifdef MHD_USE_THREADS
  MHD_thread_handle_ID_set_invalid_ (&daemon->tid);
#endif /* MHD_USE_THREADS */
#ifdef SOMAXCONN
  daemon->listen_backlog_size = SOMAXCONN;
#else  /* !SOMAXCONN */
  daemon->listen_backlog_size = 511; /* should be safe value */
#endif /* !SOMAXCONN */
#ifdef HAVE_MESSAGES
  daemon->custom_error_log = &MHD_default_logger_;
  daemon->custom_error_log_cls = stderr;
#endif
#ifndef MHD_WINSOCK_SOCKETS
  daemon->sigpipe_blocked = false;
#else  /* MHD_WINSOCK_SOCKETS */
  /* There is no SIGPIPE on W32, nothing to block. */
  daemon->sigpipe_blocked = true;
#endif /* _WIN32 && ! __CYGWIN__ */
#if defined(_DEBUG) && defined(HAVE_ACCEPT4)
  daemon->avoid_accept4 = false;
#endif /* _DEBUG */
#ifdef HAS_FD_SETSIZE_OVERRIDABLE
  daemon->fdset_size = (int) FD_SETSIZE;
  daemon->fdset_size_set_by_app = false;
#endif /* HAS_FD_SETSIZE_OVERRIDABLE */

#ifdef DAUTH_SUPPORT
  daemon->digest_auth_rand_size = 0;
  daemon->digest_auth_random = NULL;
  daemon->nonce_nc_size = 4; /* tiny */
  daemon->dauth_def_nonce_timeout = MHD_DAUTH_DEF_TIMEOUT_;
  daemon->dauth_def_max_nc = MHD_DAUTH_DEF_MAX_NC_;
#endif
#ifdef HTTPS_SUPPORT
  if (0 != (*pflags & MHD_USE_TLS))
  {
    daemon->cred_type = GNUTLS_CRD_CERTIFICATE;
  }
#endif /* HTTPS_SUPPORT */

  interim_params->num_opts = 0;
  interim_params->fdset_size_set = false;
  interim_params->fdset_size = 0;
  interim_params->listen_fd_set = false;
  interim_params->listen_fd = MHD_INVALID_SOCKET;
  interim_params->pserver_addr_set = false;
  interim_params->pserver_addr = NULL;
  interim_params->server_addr_len_set = false;
  interim_params->server_addr_len = 0;

  if (MHD_NO == parse_options_va (daemon,
                                  interim_params,
                                  ap))
  {
#ifdef HTTPS_SUPPORT
    if ( (0 != (*pflags & MHD_USE_TLS)) &&
         (NULL != daemon->priority_cache) )
      gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
    free (interim_params);
    free (daemon);
    return NULL;
  }
  if (! process_interim_params (daemon,
                                &pservaddr,
                                &addrlen,
                                interim_params))
  {
    free (interim_params);
    free (daemon);
    return NULL;
  }
  free (interim_params);
  interim_params = NULL;
#ifdef HTTPS_SUPPORT
  if ((0 != (*pflags & MHD_USE_TLS))
      && (NULL == daemon->priority_cache)
      && ! daemon_tls_priorities_init_default (daemon))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to initialise GnuTLS priorities.\n"));
#endif /* HAVE_MESSAGES */
    free (daemon);
    return NULL;
  }
#endif /* HTTPS_SUPPORT */

#ifdef HAVE_MESSAGES
  if ( (0 != (flags & MHD_USE_THREAD_PER_CONNECTION)) &&
       (0 == (flags & MHD_USE_INTERNAL_POLLING_THREAD)) )
  {
    MHD_DLOG (daemon,
              _ ("Warning: MHD_USE_THREAD_PER_CONNECTION must be used " \
                 "only with MHD_USE_INTERNAL_POLLING_THREAD. " \
                 "Flag MHD_USE_INTERNAL_POLLING_THREAD was added. " \
                 "Consider setting MHD_USE_INTERNAL_POLLING_THREAD " \
                 "explicitly.\n"));
  }
#endif

  if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon)
      && ((NULL != daemon->notify_completed)
          || (NULL != daemon->notify_connection)) )
    *pflags |= MHD_USE_ITC; /* requires ITC */

#ifdef _DEBUG
#ifdef HAVE_MESSAGES
  MHD_DLOG (daemon,
            _ ("Using debug build of libmicrohttpd.\n") );
#endif /* HAVE_MESSAGES */
#endif /* _DEBUG */

  if ( (0 != (*pflags & MHD_USE_ITC))
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
       && (0 == daemon->worker_pool_size)
#endif
       )
  {
    if (! MHD_itc_init_ (daemon->itc))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to create inter-thread communication channel: %s\n"),
                MHD_itc_last_strerror_ ());
#endif
#ifdef HTTPS_SUPPORT
      if (NULL != daemon->priority_cache)
        gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
      free (daemon);
      return NULL;
    }
    if (MHD_D_IS_USING_SELECT_ (daemon) &&
        (! MHD_D_DOES_SCKT_FIT_FDSET_ (MHD_itc_r_fd_ (daemon->itc), daemon)) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("file descriptor for inter-thread communication " \
                   "channel exceeds maximum value.\n"));
#endif
      MHD_itc_destroy_chk_ (daemon->itc);
#ifdef HTTPS_SUPPORT
      if (NULL != daemon->priority_cache)
        gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
      free (daemon);
      return NULL;
    }
  }

#ifdef DAUTH_SUPPORT
  if (NULL != daemon->digest_auth_random_copy)
  {
    mhd_assert (daemon == daemon->digest_auth_random_copy);
    daemon->digest_auth_random_copy = malloc (daemon->digest_auth_rand_size);
    if (NULL == daemon->digest_auth_random_copy)
    {
#ifdef HTTPS_SUPPORT
      if (0 != (*pflags & MHD_USE_TLS))
        gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
      free (daemon);
      return NULL;
    }
    memcpy (daemon->digest_auth_random_copy,
            daemon->digest_auth_random,
            daemon->digest_auth_rand_size);
    daemon->digest_auth_random = daemon->digest_auth_random_copy;
  }
  if (daemon->nonce_nc_size > 0)
  {
    if ( ( (size_t) (daemon->nonce_nc_size * sizeof (struct MHD_NonceNc)))
         / sizeof(struct MHD_NonceNc) != daemon->nonce_nc_size)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Specified value for NC_SIZE too large.\n"));
#endif
#ifdef HTTPS_SUPPORT
      if (0 != (*pflags & MHD_USE_TLS))
        gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
      free (daemon->digest_auth_random_copy);
      free (daemon);
      return NULL;
    }
    daemon->nnc = MHD_calloc_ (daemon->nonce_nc_size,
                               sizeof (struct MHD_NonceNc));
    if (NULL == daemon->nnc)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to allocate memory for nonce-nc map: %s\n"),
                MHD_strerror_ (errno));
#endif
#ifdef HTTPS_SUPPORT
      if (0 != (*pflags & MHD_USE_TLS))
        gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
      free (daemon->digest_auth_random_copy);
      free (daemon);
      return NULL;
    }
  }

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (! MHD_mutex_init_ (&daemon->nnc_lock))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("MHD failed to initialize nonce-nc mutex.\n"));
#endif
#ifdef HTTPS_SUPPORT
    if (0 != (*pflags & MHD_USE_TLS))
      gnutls_priority_deinit (daemon->priority_cache);
#endif /* HTTPS_SUPPORT */
    free (daemon->digest_auth_random_copy);
    free (daemon->nnc);
    free (daemon);
    return NULL;
  }
#endif
#endif

  /* Thread polling currently works only with internal select thread mode */
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if ( (! MHD_D_IS_USING_THREADS_ (daemon)) &&
       (daemon->worker_pool_size > 0) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("MHD thread polling only works with " \
                 "MHD_USE_INTERNAL_POLLING_THREAD.\n"));
#endif
    goto free_and_fail;
  }
#endif

  if ( (MHD_INVALID_SOCKET == daemon->listen_fd) &&
       (0 == (*pflags & MHD_USE_NO_LISTEN_SOCKET)) )
  {
    /* try to open listen socket */
    struct sockaddr_in servaddr4;
#ifdef HAVE_INET6
    struct sockaddr_in6 servaddr6;
    const bool use_ipv6 = (0 != (*pflags & MHD_USE_IPv6));
#else  /* ! HAVE_INET6 */
    const bool use_ipv6 = false;
#endif /* ! HAVE_INET6 */
    int domain;

    if (NULL != pservaddr)
    {
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
      const socklen_t sa_len = pservaddr->sa_len;
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
#ifdef HAVE_INET6
      if (use_ipv6 && (AF_INET6 != pservaddr->sa_family))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("MHD_USE_IPv6 is enabled, but 'struct sockaddr *' " \
                     "specified for MHD_OPTION_SOCK_ADDR_LEN or " \
                     "MHD_OPTION_SOCK_ADDR is not IPv6 address.\n"));
#endif /* HAVE_MESSAGES */
        goto free_and_fail;
      }
#endif /* HAVE_INET6 */
      switch (pservaddr->sa_family)
      {
      case AF_INET:
        if (1)
        {
          struct sockaddr_in sa4;
          uint16_t sa4_port;
          if ((0 != addrlen)
              && (((socklen_t) sizeof(sa4)) > addrlen))
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("The size specified for MHD_OPTION_SOCK_ADDR_LEN " \
                         "option is wrong.\n"));
#endif /* HAVE_MESSAGES */
            goto free_and_fail;
          }
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
          if (0 != sa_len)
          {
            if (((socklen_t) sizeof(sa4)) > sa_len)
            {
#ifdef HAVE_MESSAGES
              MHD_DLOG (daemon,
                        _ ("The value of 'struct sockaddr.sa_len' provided " \
                           "via MHD_OPTION_SOCK_ADDR_LEN option is not zero " \
                           "and does not match 'sa_family' value of the " \
                           "same structure.\n"));
#endif /* HAVE_MESSAGES */
              goto free_and_fail;
            }
            if ((0 == addrlen) || (sa_len < addrlen))
              addrlen = sa_len; /* Use smaller value for safety */
          }
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
          if (0 == addrlen)
            addrlen = sizeof(sa4);
          memcpy (&sa4, pservaddr, sizeof(sa4));  /* Required due to stronger alignment */
          sa4_port = (uint16_t) ntohs (sa4.sin_port);
#ifndef MHD_USE_GETSOCKNAME
          if (0 != sa4_port)
#endif /* ! MHD_USE_GETSOCKNAME */
          daemon->port = sa4_port;
          domain = PF_INET;
        }
        break;
#ifdef HAVE_INET6
      case AF_INET6:
        if (1)
        {
          struct sockaddr_in6 sa6;
          uint16_t sa6_port;
          if ((0 != addrlen)
              && (((socklen_t) sizeof(sa6)) > addrlen))
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("The size specified for MHD_OPTION_SOCK_ADDR_LEN " \
                         "option is wrong.\n"));
#endif /* HAVE_MESSAGES */
            goto free_and_fail;
          }
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
          if (0 != sa_len)
          {
            if (((socklen_t) sizeof(sa6)) > sa_len)
            {
#ifdef HAVE_MESSAGES
              MHD_DLOG (daemon,
                        _ ("The value of 'struct sockaddr.sa_len' provided " \
                           "via MHD_OPTION_SOCK_ADDR_LEN option is not zero " \
                           "and does not match 'sa_family' value of the " \
                           "same structure.\n"));
#endif /* HAVE_MESSAGES */
              goto free_and_fail;
            }
            if ((0 == addrlen) || (sa_len < addrlen))
              addrlen = sa_len; /* Use smaller value for safety */
          }
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
          if (0 == addrlen)
            addrlen = sizeof(sa6);
          memcpy (&sa6, pservaddr, sizeof(sa6));  /* Required due to stronger alignment */
          sa6_port = (uint16_t) ntohs (sa6.sin6_port);
#ifndef MHD_USE_GETSOCKNAME
          if (0 != sa6_port)
#endif /* ! MHD_USE_GETSOCKNAME */
          daemon->port = sa6_port;
          domain = PF_INET6;
          *pflags |= ((enum MHD_FLAG) MHD_USE_IPv6);
        }
        break;
#endif /* HAVE_INET6 */
#ifdef AF_UNIX
      case AF_UNIX:
#endif /* AF_UNIX */
      default:
#ifdef HAVE_STRUCT_SOCKADDR_SA_LEN
        if (0 == addrlen)
          addrlen = sa_len;
        else if ((0 != sa_len) && (sa_len < addrlen))
          addrlen = sa_len; /* Use smaller value for safety */
#endif /* HAVE_STRUCT_SOCKADDR_SA_LEN */
        if (0 >= addrlen)
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("The 'sa_family' of the 'struct sockaddr' provided " \
                       "via MHD_OPTION_SOCK_ADDR option is not supported.\n"));
#endif /* HAVE_MESSAGES */
          goto free_and_fail;
        }
#ifdef AF_UNIX
        if (AF_UNIX == pservaddr->sa_family)
        {
          daemon->port = 0;     /* special value for UNIX domain sockets */
          daemon->listen_is_unix = _MHD_YES;
#ifdef PF_UNIX
          domain = PF_UNIX;
#else /* ! PF_UNIX */
          domain = AF_UNIX;
#endif /* ! PF_UNIX */
        }
        else /* combined with the next 'if' */
#endif /* AF_UNIX */
        if (1)
        {
          daemon->port = 0;     /* ugh */
          daemon->listen_is_unix = _MHD_UNKNOWN;
          /* Assumed the same values for AF_* and PF_* */
          domain = pservaddr->sa_family;
        }
        break;
      }
    }
    else
    {
      if (! use_ipv6)
      {
        memset (&servaddr4,
                0,
                sizeof (struct sockaddr_in));
        servaddr4.sin_family = AF_INET;
        servaddr4.sin_port = htons (port);
        if (0 != INADDR_ANY)
          servaddr4.sin_addr.s_addr = htonl (INADDR_ANY);
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
        servaddr4.sin_len = sizeof (struct sockaddr_in);
#endif
        pservaddr = (struct sockaddr *) &servaddr4;
        addrlen = (socklen_t) sizeof(servaddr4);
        daemon->listen_is_unix = _MHD_NO;
        domain = PF_INET;
      }
#ifdef HAVE_INET6
      else
      {
#ifdef IN6ADDR_ANY_INIT
        static const struct in6_addr static_in6any = IN6ADDR_ANY_INIT;
#endif
        memset (&servaddr6,
                0,
                sizeof (struct sockaddr_in6));
        servaddr6.sin6_family = AF_INET6;
        servaddr6.sin6_port = htons (port);
#ifdef IN6ADDR_ANY_INIT
        servaddr6.sin6_addr = static_in6any;
#endif
#ifdef HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN
        servaddr6.sin6_len = sizeof (struct sockaddr_in6);
#endif
        pservaddr = (struct sockaddr *) &servaddr6;
        addrlen = (socklen_t) sizeof (servaddr6);
        daemon->listen_is_unix = _MHD_NO;
        domain = PF_INET6;
      }
#endif /* HAVE_INET6 */
    }

    listen_fd = MHD_socket_create_listen_ (domain);
    if (MHD_INVALID_SOCKET == listen_fd)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to create socket for listening: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      goto free_and_fail;
    }
    if (MHD_D_IS_USING_SELECT_ (daemon) &&
        (! MHD_D_DOES_SCKT_FIT_FDSET_ (listen_fd, daemon)) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Listen socket descriptor (%d) is not " \
                   "less than daemon FD_SETSIZE value (%d).\n"),
                (int) listen_fd,
                (int) MHD_D_GET_FD_SETSIZE_ (daemon));
#endif
      MHD_socket_close_chk_ (listen_fd);
      listen_fd = MHD_INVALID_SOCKET;
      goto free_and_fail;
    }

    /* Apply the socket options according to listening_address_reuse. */
    if (0 == daemon->listening_address_reuse)
    {
#ifndef MHD_WINSOCK_SOCKETS
      /* No user requirement, use "traditional" default SO_REUSEADDR
       * on non-W32 platforms, and do not fail if it doesn't work.
       * Don't use it on W32, because on W32 it will allow multiple
       * bind to the same address:port, like SO_REUSEPORT on others. */
      if (0 > setsockopt (listen_fd,
                          SOL_SOCKET,
                          SO_REUSEADDR,
                          (const void *) &on, sizeof (on)))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("setsockopt failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
      }
#endif /* ! MHD_WINSOCK_SOCKETS */
    }
    else if (daemon->listening_address_reuse > 0)
    {
      /* User requested to allow reusing listening address:port. */
#ifndef MHD_WINSOCK_SOCKETS
      /* Use SO_REUSEADDR on non-W32 platforms, and do not fail if
       * it doesn't work. */
      if (0 > setsockopt (listen_fd,
                          SOL_SOCKET,
                          SO_REUSEADDR,
                          (const void *) &on, sizeof (on)))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("setsockopt failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
      }
#endif /* ! MHD_WINSOCK_SOCKETS */
      /* Use SO_REUSEADDR on Windows and SO_REUSEPORT on most platforms.
       * Fail if SO_REUSEPORT is not defined or setsockopt fails.
       */
      /* SO_REUSEADDR on W32 has the same semantics
         as SO_REUSEPORT on BSD/Linux */
#if defined(MHD_WINSOCK_SOCKETS) || defined(SO_REUSEPORT)
      if (0 > setsockopt (listen_fd,
                          SOL_SOCKET,
#ifndef MHD_WINSOCK_SOCKETS
                          SO_REUSEPORT,
#else  /* MHD_WINSOCK_SOCKETS */
                          SO_REUSEADDR,
#endif /* MHD_WINSOCK_SOCKETS */
                          (const void *) &on,
                          sizeof (on)))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("setsockopt failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
        goto free_and_fail;
      }
#else  /* !MHD_WINSOCK_SOCKETS && !SO_REUSEPORT */
      /* we're supposed to allow address:port re-use, but
         on this platform we cannot; fail hard */
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Cannot allow listening address reuse: " \
                   "SO_REUSEPORT not defined.\n"));
#endif
      goto free_and_fail;
#endif /* !MHD_WINSOCK_SOCKETS && !SO_REUSEPORT */
    }
    else   /* if (daemon->listening_address_reuse < 0) */
    {
      /* User requested to disallow reusing listening address:port.
       * Do nothing except for Windows where SO_EXCLUSIVEADDRUSE
       * is used and Solaris with SO_EXCLBIND.
       * Fail if MHD was compiled for W32 without SO_EXCLUSIVEADDRUSE
       * or setsockopt fails.
       */
#if (defined(MHD_WINSOCK_SOCKETS) && defined(SO_EXCLUSIVEADDRUSE)) || \
      (defined(__sun) && defined(SO_EXCLBIND))
      if (0 > setsockopt (listen_fd,
                          SOL_SOCKET,
#ifdef SO_EXCLUSIVEADDRUSE
                          SO_EXCLUSIVEADDRUSE,
#else  /* SO_EXCLBIND */
                          SO_EXCLBIND,
#endif /* SO_EXCLBIND */
                          (const void *) &on,
                          sizeof (on)))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("setsockopt failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
        goto free_and_fail;
      }
#elif defined(MHD_WINSOCK_SOCKETS) /* SO_EXCLUSIVEADDRUSE not defined on W32? */
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Cannot disallow listening address reuse: " \
                   "SO_EXCLUSIVEADDRUSE not defined.\n"));
#endif
      goto free_and_fail;
#endif /* MHD_WINSOCK_SOCKETS */
    }

    /* check for user supplied sockaddr */
    daemon->listen_fd = listen_fd;

    if (0 != (*pflags & MHD_USE_IPv6))
    {
#ifdef IPPROTO_IPV6
#ifdef IPV6_V6ONLY
      /* Note: "IPV6_V6ONLY" is declared by Windows Vista ff., see "IPPROTO_IPV6 Socket Options"
         (http://msdn.microsoft.com/en-us/library/ms738574%28v=VS.85%29.aspx);
         and may also be missing on older POSIX systems; good luck if you have any of those,
         your IPv6 socket may then also bind against IPv4 anyway... */
      const MHD_SCKT_OPT_BOOL_ v6_only =
        (MHD_USE_DUAL_STACK != (*pflags & MHD_USE_DUAL_STACK));
      if (0 > setsockopt (listen_fd,
                          IPPROTO_IPV6, IPV6_V6ONLY,
                          (const void *) &v6_only,
                          sizeof (v6_only)))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("setsockopt failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
      }
#endif
#endif
    }
    if (0 != bind (listen_fd, pservaddr, addrlen))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to bind to port %u: %s\n"),
                (unsigned int) port,
                MHD_socket_last_strerr_ ());
#endif
      MHD_socket_close_chk_ (listen_fd);
      listen_fd = MHD_INVALID_SOCKET;
      goto free_and_fail;
    }
#ifdef TCP_FASTOPEN
    if (0 != (*pflags & MHD_USE_TCP_FASTOPEN))
    {
      if (0 == daemon->fastopen_queue_size)
        daemon->fastopen_queue_size = MHD_TCP_FASTOPEN_QUEUE_SIZE_DEFAULT;
      if (0 != setsockopt (listen_fd,
                           IPPROTO_TCP,
                           TCP_FASTOPEN,
                           (const void *) &daemon->fastopen_queue_size,
                           sizeof (daemon->fastopen_queue_size)))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("setsockopt failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
      }
    }
#endif
    if (0 != listen (listen_fd,
                     (int) daemon->listen_backlog_size))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to listen for connections: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      MHD_socket_close_chk_ (listen_fd);
      listen_fd = MHD_INVALID_SOCKET;
      goto free_and_fail;
    }
  }
  else
  {
    if (MHD_D_IS_USING_SELECT_ (daemon) &&
        (! MHD_D_DOES_SCKT_FIT_FDSET_ (daemon->listen_fd, daemon)) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Listen socket descriptor (%d) is not " \
                   "less than daemon FD_SETSIZE value (%d).\n"),
                (int) daemon->listen_fd,
                (int) MHD_D_GET_FD_SETSIZE_ (daemon));
#endif
      goto free_and_fail;
    }
    else
    {
#if defined(SOL_SOCKET) && (defined(SO_DOMAIN) || defined(SO_PROTOCOL_INFOW))
      int af;
      int opt_name;
      void *poptval;
      socklen_t optval_size;
#ifdef SO_DOMAIN
      opt_name = SO_DOMAIN;
      poptval = &af;
      optval_size = (socklen_t) sizeof (af);
#else  /* SO_PROTOCOL_INFOW */
      WSAPROTOCOL_INFOW prot_info;
      opt_name = SO_PROTOCOL_INFOW;
      poptval = &prot_info;
      optval_size = (socklen_t) sizeof (prot_info);
#endif /* SO_PROTOCOL_INFOW */

      if (0 == getsockopt (daemon->listen_fd,
                           SOL_SOCKET,
                           opt_name,
                           poptval,
                           &optval_size))
      {
#ifndef SO_DOMAIN
        af = prot_info.iAddressFamily;
#endif /* SO_DOMAIN */
        switch (af)
        {
        case AF_INET:
          daemon->listen_is_unix = _MHD_NO;
          break;
#ifdef HAVE_INET6
        case AF_INET6:
          *pflags |= MHD_USE_IPv6;
          daemon->listen_is_unix = _MHD_NO;
          break;
#endif /* HAVE_INET6 */
#ifdef AF_UNIX
        case AF_UNIX:
          daemon->port = 0;     /* special value for UNIX domain sockets */
          daemon->listen_is_unix = _MHD_YES;
          break;
#endif /* AF_UNIX */
        default:
          daemon->port = 0;     /* ugh */
          daemon->listen_is_unix = _MHD_UNKNOWN;
          break;
        }
      }
      else
#endif /* SOL_SOCKET && (SO_DOMAIN || SO_PROTOCOL_INFOW)) */
      daemon->listen_is_unix = _MHD_UNKNOWN;
    }

    listen_fd = daemon->listen_fd;
#ifdef MHD_USE_GETSOCKNAME
    daemon->port = 0;  /* Force use of autodetection */
#endif /* MHD_USE_GETSOCKNAME */
  }

#ifdef MHD_USE_GETSOCKNAME
  if ( (0 == daemon->port) &&
       (0 == (*pflags & MHD_USE_NO_LISTEN_SOCKET)) &&
       (_MHD_YES != daemon->listen_is_unix) )
  {   /* Get port number. */
    struct sockaddr_storage bindaddr;

    memset (&bindaddr,
            0,
            sizeof (struct sockaddr_storage));
    addrlen = sizeof (struct sockaddr_storage);
#ifdef HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN
    bindaddr.ss_len = (socklen_t) addrlen;
#endif
    if (0 != getsockname (listen_fd,
                          (struct sockaddr *) &bindaddr,
                          &addrlen))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to get listen port number: %s\n"),
                MHD_socket_last_strerr_ ());
#endif /* HAVE_MESSAGES */
    }
#ifdef MHD_POSIX_SOCKETS
    else if (sizeof (bindaddr) < addrlen)
    {
      /* should be impossible with `struct sockaddr_storage` */
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to get listen port number " \
                   "(`struct sockaddr_storage` too small!?).\n"));
#endif /* HAVE_MESSAGES */
    }
#ifndef __linux__
    else if (0 == addrlen)
    {
      /* Many non-Linux-based platforms return zero addrlen
       * for AF_UNIX sockets */
      daemon->port = 0;     /* special value for UNIX domain sockets */
      if (_MHD_UNKNOWN == daemon->listen_is_unix)
        daemon->listen_is_unix = _MHD_YES;
    }
#endif /* __linux__ */
#endif /* MHD_POSIX_SOCKETS */
    else
    {
      switch (bindaddr.ss_family)
      {
      case AF_INET:
        {
          struct sockaddr_in *s4 = (struct sockaddr_in *) &bindaddr;

          daemon->port = ntohs (s4->sin_port);
          daemon->listen_is_unix = _MHD_NO;
          break;
        }
#ifdef HAVE_INET6
      case AF_INET6:
        {
          struct sockaddr_in6 *s6 = (struct sockaddr_in6 *) &bindaddr;

          daemon->port = ntohs (s6->sin6_port);
          daemon->listen_is_unix = _MHD_NO;
          mhd_assert (0 != (*pflags & MHD_USE_IPv6));
          break;
        }
#endif /* HAVE_INET6 */
#ifdef AF_UNIX
      case AF_UNIX:
        daemon->port = 0;     /* special value for UNIX domain sockets */
        daemon->listen_is_unix = _MHD_YES;
        break;
#endif
      default:
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Listen socket has unknown address family!\n"));
#endif
        daemon->port = 0;     /* ugh */
        daemon->listen_is_unix = _MHD_UNKNOWN;
        break;
      }
    }
  }
#endif /* MHD_USE_GETSOCKNAME */

  if (MHD_INVALID_SOCKET != listen_fd)
  {
    mhd_assert (0 == (*pflags & MHD_USE_NO_LISTEN_SOCKET));
    if (! MHD_socket_nonblocking_ (listen_fd))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to set nonblocking mode on listening socket: %s\n"),
                MHD_socket_last_strerr_ ());
#endif
      if (MHD_D_IS_USING_EPOLL_ (daemon)
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
          || (daemon->worker_pool_size > 0)
#endif
          )
      {
        /* Accept must be non-blocking. Multiple children may wake up
         * to handle a new connection, but only one will win the race.
         * The others must immediately return. */
        MHD_socket_close_chk_ (listen_fd);
        listen_fd = MHD_INVALID_SOCKET;
        goto free_and_fail;
      }
      daemon->listen_nonblk = false;
    }
    else
      daemon->listen_nonblk = true;
  }
  else
  {
    mhd_assert (0 != (*pflags & MHD_USE_NO_LISTEN_SOCKET));
    daemon->listen_nonblk = false; /* Actually listen socket does not exist */
  }

#ifdef EPOLL_SUPPORT
  if (MHD_D_IS_USING_EPOLL_ (daemon)
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      && (0 == daemon->worker_pool_size)
#endif
      )
  {
    if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Combining MHD_USE_THREAD_PER_CONNECTION and " \
                   "MHD_USE_EPOLL is not supported.\n"));
#endif
      goto free_and_fail;
    }
    if (MHD_NO == setup_epoll_to_listen (daemon))
      goto free_and_fail;
  }
#endif /* EPOLL_SUPPORT */

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (! MHD_mutex_init_ (&daemon->per_ip_connection_mutex))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("MHD failed to initialize IP connection limit mutex.\n"));
#endif
    if (MHD_INVALID_SOCKET != listen_fd)
      MHD_socket_close_chk_ (listen_fd);
    goto free_and_fail;
  }
#endif

#ifdef HTTPS_SUPPORT
  /* initialize HTTPS daemon certificate aspects & send / recv functions */
  if ( (0 != (*pflags & MHD_USE_TLS)) &&
       (0 != MHD_TLS_init (daemon)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Failed to initialize TLS support.\n"));
#endif
    if (MHD_INVALID_SOCKET != listen_fd)
      MHD_socket_close_chk_ (listen_fd);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
#endif
    goto free_and_fail;
  }
#endif /* HTTPS_SUPPORT */
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /* Start threads if requested by parameters */
  if (MHD_D_IS_USING_THREADS_ (daemon))
  {
    /* Internal thread (or threads) is used.
     * Make sure that MHD will be able to communicate with threads. */
    /* If using a thread pool ITC will be initialised later
     * for each individual worker thread. */
#ifdef HAVE_LISTEN_SHUTDOWN
    mhd_assert ((1 < daemon->worker_pool_size) || \
                (MHD_ITC_IS_VALID_ (daemon->itc)) || \
                (MHD_INVALID_SOCKET != daemon->listen_fd));
#else  /* ! HAVE_LISTEN_SHUTDOWN */
    mhd_assert ((1 < daemon->worker_pool_size) || \
                (MHD_ITC_IS_VALID_ (daemon->itc)));
#endif /* ! HAVE_LISTEN_SHUTDOWN */
    if (0 == daemon->worker_pool_size)
    {
      if (! MHD_mutex_init_ (&daemon->cleanup_connection_mutex))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Failed to initialise internal lists mutex.\n"));
#endif
        MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
        if (MHD_INVALID_SOCKET != listen_fd)
          MHD_socket_close_chk_ (listen_fd);
        goto free_and_fail;
      }
      if (! MHD_mutex_init_ (&daemon->new_connections_mutex))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Failed to initialise mutex.\n"));
#endif
        MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
        MHD_mutex_destroy_chk_ (&daemon->cleanup_connection_mutex);
        if (MHD_INVALID_SOCKET != listen_fd)
          MHD_socket_close_chk_ (listen_fd);
        goto free_and_fail;
      }
      if (! MHD_create_named_thread_ (&daemon->tid,
                                      MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) ?
                                      "MHD-listen" : "MHD-single",
                                      daemon->thread_stack_size,
                                      &MHD_polling_thread,
                                      daemon) )
      {
#ifdef HAVE_MESSAGES
#ifdef EAGAIN
        if (EAGAIN == errno)
          MHD_DLOG (daemon,
                    _ ("Failed to create a new thread because it would have " \
                       "exceeded the system limit on the number of threads or " \
                       "no system resources available.\n"));
        else
#endif /* EAGAIN */
        MHD_DLOG (daemon,
                  _ ("Failed to create listen thread: %s\n"),
                  MHD_strerror_ (errno));
#endif /* HAVE_MESSAGES */
        MHD_mutex_destroy_chk_ (&daemon->new_connections_mutex);
        MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
        MHD_mutex_destroy_chk_ (&daemon->cleanup_connection_mutex);
        if (MHD_INVALID_SOCKET != listen_fd)
          MHD_socket_close_chk_ (listen_fd);
        goto free_and_fail;
      }
    }
    else   /* 0 < daemon->worker_pool_size */
    {
      /* Coarse-grained count of connections per thread (note error
       * due to integer division). Also keep track of how many
       * connections are leftover after an equal split. */
      unsigned int conns_per_thread = daemon->connection_limit
                                      / daemon->worker_pool_size;
      unsigned int leftover_conns = daemon->connection_limit
                                    % daemon->worker_pool_size;

      mhd_assert (2 <= daemon->worker_pool_size);
      i = 0;     /* we need this in case fcntl or malloc fails */

      /* Allocate memory for pooled objects */
      daemon->worker_pool = malloc (sizeof (struct MHD_Daemon)
                                    * daemon->worker_pool_size);
      if (NULL == daemon->worker_pool)
        goto thread_failed;

      /* Start the workers in the pool */
      for (i = 0; i < daemon->worker_pool_size; ++i)
      {
        /* Create copy of the Daemon object for each worker */
        struct MHD_Daemon *d = &daemon->worker_pool[i];

        memcpy (d, daemon, sizeof (struct MHD_Daemon));
        /* Adjust polling params for worker daemons; note that memcpy()
           has already copied MHD_USE_INTERNAL_POLLING_THREAD thread mode into
           the worker threads. */
        d->master = daemon;
        d->worker_pool_size = 0;
        d->worker_pool = NULL;
        if (! MHD_mutex_init_ (&d->cleanup_connection_mutex))
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Failed to initialise internal lists mutex.\n"));
#endif
          goto thread_failed;
        }
        if (! MHD_mutex_init_ (&d->new_connections_mutex))
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (daemon,
                    _ ("Failed to initialise mutex.\n"));
#endif
          MHD_mutex_destroy_chk_ (&d->cleanup_connection_mutex);
          goto thread_failed;
        }
        if (0 != (*pflags & MHD_USE_ITC))
        {
          if (! MHD_itc_init_ (d->itc))
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("Failed to create worker inter-thread " \
                         "communication channel: %s\n"),
                      MHD_itc_last_strerror_ () );
#endif
            MHD_mutex_destroy_chk_ (&d->new_connections_mutex);
            MHD_mutex_destroy_chk_ (&d->cleanup_connection_mutex);
            goto thread_failed;
          }
          if (MHD_D_IS_USING_SELECT_ (d) &&
              (! MHD_D_DOES_SCKT_FIT_FDSET_ (MHD_itc_r_fd_ (d->itc), daemon)) )
          {
#ifdef HAVE_MESSAGES
            MHD_DLOG (daemon,
                      _ ("File descriptor for worker inter-thread " \
                         "communication channel exceeds maximum value.\n"));
#endif
            MHD_itc_destroy_chk_ (d->itc);
            MHD_mutex_destroy_chk_ (&d->new_connections_mutex);
            MHD_mutex_destroy_chk_ (&d->cleanup_connection_mutex);
            goto thread_failed;
          }
        }
        else
          MHD_itc_set_invalid_ (d->itc);

#ifdef HAVE_LISTEN_SHUTDOWN
        mhd_assert ((MHD_ITC_IS_VALID_ (d->itc)) || \
                    (MHD_INVALID_SOCKET != d->listen_fd));
#else  /* ! HAVE_LISTEN_SHUTDOWN */
        mhd_assert (MHD_ITC_IS_VALID_ (d->itc));
#endif /* ! HAVE_LISTEN_SHUTDOWN */

        /* Divide available connections evenly amongst the threads.
         * Thread indexes in [0, leftover_conns) each get one of the
         * leftover connections. */
        d->connection_limit = conns_per_thread;
        if (i < leftover_conns)
          ++d->connection_limit;
#ifdef EPOLL_SUPPORT
        if (MHD_D_IS_USING_EPOLL_ (d) &&
            (MHD_NO == setup_epoll_to_listen (d)) )
        {
          if (MHD_ITC_IS_VALID_ (d->itc))
            MHD_itc_destroy_chk_ (d->itc);
          MHD_mutex_destroy_chk_ (&d->new_connections_mutex);
          MHD_mutex_destroy_chk_ (&d->cleanup_connection_mutex);
          goto thread_failed;
        }
#endif
        /* Some members must be used only in master daemon */
#if defined(MHD_USE_THREADS)
        memset (&d->per_ip_connection_mutex, 0x7F,
                sizeof(d->per_ip_connection_mutex));
#endif /* MHD_USE_THREADS */
#ifdef DAUTH_SUPPORT
        d->nnc = NULL;
        d->nonce_nc_size = 0;
        d->digest_auth_random_copy = NULL;
#if defined(MHD_USE_THREADS)
        memset (&d->nnc_lock, 0x7F, sizeof(d->nnc_lock));
#endif /* MHD_USE_THREADS */
#endif /* DAUTH_SUPPORT */

        /* Spawn the worker thread */
        if (! MHD_create_named_thread_ (&d->tid,
                                        "MHD-worker",
                                        daemon->thread_stack_size,
                                        &MHD_polling_thread,
                                        d))
        {
#ifdef HAVE_MESSAGES
#ifdef EAGAIN
          if (EAGAIN == errno)
            MHD_DLOG (daemon,
                      _ ("Failed to create a new pool thread because it would " \
                         "have exceeded the system limit on the number of " \
                         "threads or no system resources available.\n"));
          else
#endif /* EAGAIN */
          MHD_DLOG (daemon,
                    _ ("Failed to create pool thread: %s\n"),
                    MHD_strerror_ (errno));
#endif
          /* Free memory for this worker; cleanup below handles
           * all previously-created workers. */
          MHD_mutex_destroy_chk_ (&d->cleanup_connection_mutex);
          if (MHD_ITC_IS_VALID_ (d->itc))
            MHD_itc_destroy_chk_ (d->itc);
          MHD_mutex_destroy_chk_ (&d->new_connections_mutex);
          MHD_mutex_destroy_chk_ (&d->cleanup_connection_mutex);
          goto thread_failed;
        }
      }
    }
  }
  else
  { /* Daemon without internal threads */
    if (! MHD_mutex_init_ (&daemon->cleanup_connection_mutex))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to initialise internal lists mutex.\n"));
#endif
      MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
      if (MHD_INVALID_SOCKET != listen_fd)
        MHD_socket_close_chk_ (listen_fd);
      goto free_and_fail;
    }
    if (! MHD_mutex_init_ (&daemon->new_connections_mutex))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to initialise mutex.\n"));
#endif
      MHD_mutex_destroy_chk_ (&daemon->cleanup_connection_mutex);
      MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
      if (MHD_INVALID_SOCKET != listen_fd)
        MHD_socket_close_chk_ (listen_fd);
      goto free_and_fail;
    }
  }
#endif
#ifdef HTTPS_SUPPORT
  /* API promises to never use the password after initialization,
     so we additionally NULL it here to not deref a dangling pointer. */
  daemon->https_key_password = NULL;
#endif /* HTTPS_SUPPORT */

  return daemon;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
thread_failed:
  /* If no worker threads created, then shut down normally. Calling
     MHD_stop_daemon (as we do below) doesn't work here since it
     assumes a 0-sized thread pool means we had been in the default
     MHD_USE_INTERNAL_POLLING_THREAD mode. */
  if (0 == i)
  {
    if (MHD_INVALID_SOCKET != listen_fd)
      MHD_socket_close_chk_ (listen_fd);
    listen_fd = MHD_INVALID_SOCKET;
    MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
    if (NULL != daemon->worker_pool)
      free (daemon->worker_pool);
    goto free_and_fail;
  }

  /* Shutdown worker threads we've already created. Pretend
     as though we had fully initialized our daemon, but
     with a smaller number of threads than had been
     requested. */
  daemon->worker_pool_size = i;
  MHD_stop_daemon (daemon);
  return NULL;
#endif

free_and_fail:
  /* clean up basic memory state in 'daemon' and return NULL to
     indicate failure */
#ifdef EPOLL_SUPPORT
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  if (daemon->upgrade_fd_in_epoll)
  {
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_DEL,
                        daemon->epoll_upgrade_fd,
                        NULL))
      MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
    daemon->upgrade_fd_in_epoll = false;
  }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
  if (-1 != daemon->epoll_fd)
    close (daemon->epoll_fd);
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  if (-1 != daemon->epoll_upgrade_fd)
    close (daemon->epoll_upgrade_fd);
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
#endif /* EPOLL_SUPPORT */
#ifdef DAUTH_SUPPORT
  free (daemon->digest_auth_random_copy);
  free (daemon->nnc);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_destroy_chk_ (&daemon->nnc_lock);
#endif
#endif
#ifdef HTTPS_SUPPORT
  if (0 != (*pflags & MHD_USE_TLS))
  {
    gnutls_priority_deinit (daemon->priority_cache);
    if (daemon->x509_cred)
      gnutls_certificate_free_credentials (daemon->x509_cred);
    if (daemon->psk_cred)
      gnutls_psk_free_server_credentials (daemon->psk_cred);
  }
#endif /* HTTPS_SUPPORT */
  if (MHD_ITC_IS_VALID_ (daemon->itc))
    MHD_itc_destroy_chk_ (daemon->itc);
  if (MHD_INVALID_SOCKET != listen_fd)
    (void) MHD_socket_close_ (listen_fd);
  if ((MHD_INVALID_SOCKET != daemon->listen_fd) &&
      (listen_fd != daemon->listen_fd))
    (void) MHD_socket_close_ (daemon->listen_fd);
  free (daemon);
  return NULL;
}


/**
 * Close all connections for the daemon.
 * Must only be called when MHD_Daemon::shutdown was set to true.
 * @remark To be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param daemon daemon to close down
 */
static void
close_all_connections (struct MHD_Daemon *daemon)
{
  struct MHD_Connection *pos;
  const bool used_thr_p_c = MHD_D_IS_USING_THREAD_PER_CONN_ (daemon);
#ifdef UPGRADE_SUPPORT
  const bool upg_allowed = (0 != (daemon->options & MHD_ALLOW_UPGRADE));
#endif /* UPGRADE_SUPPORT */
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  struct MHD_UpgradeResponseHandle *urh;
  struct MHD_UpgradeResponseHandle *urhn;
  const bool used_tls = (0 != (daemon->options & MHD_USE_TLS));
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) || \
               MHD_thread_handle_ID_is_current_thread_ (daemon->tid) );
  mhd_assert (NULL == daemon->worker_pool);
#endif /* MHD_USE_THREADS */
  mhd_assert (daemon->shutdown);

#ifdef MHD_USE_THREADS
/* Remove externally added new connections that are
   * not processed by the daemon thread. */
  MHD_mutex_lock_chk_ (&daemon->new_connections_mutex);
  while (NULL != (pos = daemon->new_connections_tail))
  {
    mhd_assert (MHD_D_IS_USING_THREADS_ (daemon));
    DLL_remove (daemon->new_connections_head,
                daemon->new_connections_tail,
                pos);
    new_connection_close_ (daemon, pos);
  }
  MHD_mutex_unlock_chk_ (&daemon->new_connections_mutex);
#endif /* MHD_USE_THREADS */

#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
  /* give upgraded HTTPS connections a chance to finish */
  /* 'daemon->urh_head' is not used in thread-per-connection mode. */
  for (urh = daemon->urh_tail; NULL != urh; urh = urhn)
  {
    mhd_assert (! used_thr_p_c);
    urhn = urh->prev;
    /* call generic forwarding function for passing data
       with chance to detect that application is done. */
    process_urh (urh);
    MHD_connection_finish_forward_ (urh->connection);
    urh->clean_ready = true;
    /* Resuming will move connection to cleanup list. */
    MHD_resume_connection (urh->connection);
  }
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */

  /* Give suspended connections a chance to resume to avoid
     running into the check for there not being any suspended
     connections left in case of a tight race with a recently
     resumed connection. */
  if (0 != (MHD_TEST_ALLOW_SUSPEND_RESUME & daemon->options))
  {
    daemon->resuming = true;   /* Force check for pending resume. */
    resume_suspended_connections (daemon);
  }
  /* first, make sure all threads are aware of shutdown; need to
     traverse DLLs in peace... */
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
#ifdef UPGRADE_SUPPORT
  if (upg_allowed)
  {
    struct MHD_Connection *susp;

    susp = daemon->suspended_connections_tail;
    while (NULL != susp)
    {
      if (NULL == susp->urh)     /* "Upgraded" connection? */
        MHD_PANIC (_ ("MHD_stop_daemon() called while we have " \
                      "suspended connections.\n"));
#ifdef HTTPS_SUPPORT
      else if (used_tls &&
               used_thr_p_c &&
               (! susp->urh->clean_ready) )
        shutdown (susp->urh->app.socket,
                  SHUT_RDWR);     /* Wake thread by shutdown of app socket. */
#endif /* HTTPS_SUPPORT */
      else
      {
#ifdef HAVE_MESSAGES
        if (! susp->urh->was_closed)
          MHD_DLOG (daemon,
                    _ ("Initiated daemon shutdown while \"upgraded\" " \
                       "connection was not closed.\n"));
#endif
        susp->urh->was_closed = true;
        /* If thread-per-connection is used, connection's thread
         * may still processing "upgrade" (exiting). */
        if (! used_thr_p_c)
          MHD_connection_finish_forward_ (susp);
        /* Do not use MHD_resume_connection() as mutex is
         * already locked. */
        susp->resuming = true;
        daemon->resuming = true;
      }
      susp = susp->prev;
    }
  }
  else /* This 'else' is combined with next 'if' */
#endif /* UPGRADE_SUPPORT */
  if (NULL != daemon->suspended_connections_head)
    MHD_PANIC (_ ("MHD_stop_daemon() called while we have " \
                  "suspended connections.\n"));
#if defined(UPGRADE_SUPPORT) && defined(HTTPS_SUPPORT)
#ifdef MHD_USE_THREADS
  if (upg_allowed && used_tls && used_thr_p_c)
  {
    /* "Upgraded" threads may be running in parallel. Connection will not be
     * moved to the "cleanup list" until connection's thread finishes.
     * We must ensure that all "upgraded" connections are finished otherwise
     * connection may stay in "suspended" list and will not be cleaned. */
    for (pos = daemon->suspended_connections_tail; NULL != pos; pos = pos->prev)
    {
      /* Any connection found here is "upgraded" connection, normal suspended
       * connections are already removed from this list. */
      mhd_assert (NULL != pos->urh);
      if (! pos->thread_joined)
      {
        /* While "cleanup" list is not manipulated by "upgraded"
         * connection, "cleanup" mutex is required for call of
         * MHD_resume_connection() during finishing of "upgraded"
         * thread. */
        MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
        if (! MHD_thread_handle_ID_join_thread_ (pos->tid))
          MHD_PANIC (_ ("Failed to join a thread.\n"));
        pos->thread_joined = true;
        MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
      }
    }
  }
#endif /* MHD_USE_THREADS */
#endif
  for (pos = daemon->connections_tail; NULL != pos; pos = pos->prev)
  {
    shutdown (pos->socket_fd,
              SHUT_RDWR);
#ifdef MHD_WINSOCK_SOCKETS
    if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) &&
        (MHD_ITC_IS_VALID_ (daemon->itc)) &&
        (! MHD_itc_activate_ (daemon->itc, "e")) )
      MHD_PANIC (_ ("Failed to signal shutdown via inter-thread " \
                    "communication channel.\n"));
#endif
  }

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  /* now, collect per-connection threads */
  if (used_thr_p_c)
  {
    pos = daemon->connections_tail;
    while (NULL != pos)
    {
      if (! pos->thread_joined)
      {
        MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
        if (! MHD_thread_handle_ID_join_thread_ (pos->tid))
          MHD_PANIC (_ ("Failed to join a thread.\n"));
        MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
        pos->thread_joined = true;
        /* The thread may have concurrently modified the DLL,
           need to restart from the beginning */
        pos = daemon->connections_tail;
        continue;
      }
      pos = pos->prev;
    }
  }
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif

#ifdef UPGRADE_SUPPORT
  /* Finished threads with "upgraded" connections need to be moved
   * to cleanup list by resume_suspended_connections(). */
  /* "Upgraded" connections that were not closed explicitly by
   * application should be moved to cleanup list too. */
  if (upg_allowed)
  {
    daemon->resuming = true;   /* Force check for pending resume. */
    resume_suspended_connections (daemon);
  }
#endif /* UPGRADE_SUPPORT */

  mhd_assert (NULL == daemon->suspended_connections_head);
  /* now that we're alone, move everyone to cleanup */
  while (NULL != (pos = daemon->connections_tail))
  {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    if (MHD_D_IS_USING_THREAD_PER_CONN_ (daemon) &&
        (! pos->thread_joined) )
      MHD_PANIC (_ ("Failed to join a thread.\n"));
#endif
    close_connection (pos);
  }
  MHD_cleanup_connections (daemon);
}


/**
 * Shutdown an HTTP daemon.
 *
 * @param daemon daemon to stop
 * @ingroup event
 */
_MHD_EXTERN void
MHD_stop_daemon (struct MHD_Daemon *daemon)
{
  MHD_socket fd;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  unsigned int i;
#endif

  if (NULL == daemon)
    return;
  if ( (daemon->shutdown) && (NULL == daemon->master) )
    MHD_PANIC (_ ("MHD_stop_daemon() was called twice."));

  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (NULL != daemon->worker_pool) || \
              (MHD_thread_handle_ID_is_valid_handle_ (daemon->tid)));
  mhd_assert (((0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) &&
               (NULL == daemon->worker_pool)) || \
              (! MHD_thread_handle_ID_is_valid_handle_ (daemon->tid)));

  /* Slave daemons must be stopped by master daemon. */
  mhd_assert ( (NULL == daemon->master) || (daemon->shutdown) );

  daemon->shutdown = true;
  if (daemon->was_quiesced)
    fd = MHD_INVALID_SOCKET; /* Do not use FD if daemon was quiesced */
  else
    fd = daemon->listen_fd;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (NULL != daemon->worker_pool)
  {   /* Master daemon with worker pool. */
    mhd_assert (1 < daemon->worker_pool_size);
    mhd_assert (MHD_D_IS_USING_THREADS_ (daemon));

    /* Let workers shutdown in parallel. */
    for (i = 0; i < daemon->worker_pool_size; ++i)
    {
      daemon->worker_pool[i].shutdown = true;
      if (MHD_ITC_IS_VALID_ (daemon->worker_pool[i].itc))
      {
        if (! MHD_itc_activate_ (daemon->worker_pool[i].itc,
                                 "e"))
          MHD_PANIC (_ ("Failed to signal shutdown via inter-thread " \
                        "communication channel.\n"));
      }
      else
        mhd_assert (MHD_INVALID_SOCKET != fd);
    }
#ifdef HAVE_LISTEN_SHUTDOWN
    if (MHD_INVALID_SOCKET != fd)
    {
      (void) shutdown (fd,
                       SHUT_RDWR);
    }
#endif /* HAVE_LISTEN_SHUTDOWN */
    for (i = 0; i < daemon->worker_pool_size; ++i)
    {
      MHD_stop_daemon (&daemon->worker_pool[i]);
    }
    free (daemon->worker_pool);
    mhd_assert (MHD_ITC_IS_INVALID_ (daemon->itc));
#ifdef EPOLL_SUPPORT
    mhd_assert (-1 == daemon->epoll_fd);
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
    mhd_assert (-1 == daemon->epoll_upgrade_fd);
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
#endif /* EPOLL_SUPPORT */
  }
  else
#endif
  {   /* Worker daemon or single daemon. */
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    if (MHD_D_IS_USING_THREADS_ (daemon))
    {     /* Worker daemon or single daemon with internal thread(s). */
      mhd_assert (0 == daemon->worker_pool_size);
      /* Separate thread(s) is used for polling sockets. */
      if (MHD_ITC_IS_VALID_ (daemon->itc))
      {
        if (! MHD_itc_activate_ (daemon->itc,
                                 "e"))
          MHD_PANIC (_ ("Failed to signal shutdown via inter-thread " \
                        "communication channel.\n"));
      }
      else
      {
#ifdef HAVE_LISTEN_SHUTDOWN
        if (MHD_INVALID_SOCKET != fd)
        {
          if (NULL == daemon->master)
            (void) shutdown (fd,
                             SHUT_RDWR);
        }
        else
#endif /* HAVE_LISTEN_SHUTDOWN */
        mhd_assert (false); /* Should never happen */
      }

      if (! MHD_thread_handle_ID_join_thread_ (daemon->tid))
      {
        MHD_PANIC (_ ("Failed to join a thread.\n"));
      }
      /* close_all_connections() was called in daemon thread. */
    }
    else
#endif
    {
      /* No internal threads are used for polling sockets. */
      close_all_connections (daemon);
    }
    mhd_assert (NULL == daemon->connections_head);
    mhd_assert (NULL == daemon->cleanup_head);
    mhd_assert (NULL == daemon->suspended_connections_head);
    mhd_assert (NULL == daemon->new_connections_head);
#if defined(UPGRADE_SUPPORT) && defined(HTTPS_SUPPORT)
    mhd_assert (NULL == daemon->urh_head);
#endif /* UPGRADE_SUPPORT && HTTPS_SUPPORT */

    if (MHD_ITC_IS_VALID_ (daemon->itc))
      MHD_itc_destroy_chk_ (daemon->itc);

#ifdef EPOLL_SUPPORT
    if (MHD_D_IS_USING_EPOLL_ (daemon) &&
        (-1 != daemon->epoll_fd) )
      MHD_socket_close_chk_ (daemon->epoll_fd);
#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
    if (MHD_D_IS_USING_EPOLL_ (daemon) &&
        (-1 != daemon->epoll_upgrade_fd) )
      MHD_socket_close_chk_ (daemon->epoll_upgrade_fd);
#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT */
#endif /* EPOLL_SUPPORT */

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_destroy_chk_ (&daemon->cleanup_connection_mutex);
    MHD_mutex_destroy_chk_ (&daemon->new_connections_mutex);
#endif
  }

  if (NULL == daemon->master)
  {   /* Cleanup that should be done only one time in master/single daemon.
       * Do not perform this cleanup in worker daemons. */

    if (MHD_INVALID_SOCKET != fd)
      MHD_socket_close_chk_ (fd);

    /* TLS clean up */
#ifdef HTTPS_SUPPORT
    if (daemon->have_dhparams)
    {
      gnutls_dh_params_deinit (daemon->https_mem_dhparams);
      daemon->have_dhparams = false;
    }
    if (0 != (daemon->options & MHD_USE_TLS))
    {
      gnutls_priority_deinit (daemon->priority_cache);
      if (daemon->x509_cred)
        gnutls_certificate_free_credentials (daemon->x509_cred);
      if (daemon->psk_cred)
        gnutls_psk_free_server_credentials (daemon->psk_cred);
    }
#endif /* HTTPS_SUPPORT */

#ifdef DAUTH_SUPPORT
    free (daemon->digest_auth_random_copy);
    free (daemon->nnc);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_destroy_chk_ (&daemon->nnc_lock);
#endif
#endif
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_destroy_chk_ (&daemon->per_ip_connection_mutex);
#endif
    free (daemon);
  }
}


/**
 * Obtain information about the given daemon.
 * The returned pointer is invalidated with the next call of this function or
 * when the daemon is stopped.
 *
 * @param daemon what daemon to get information about
 * @param info_type what information is desired?
 * @param ... depends on @a info_type
 * @return NULL if this information is not available
 *         (or if the @a info_type is unknown)
 * @ingroup specialized
 */
_MHD_EXTERN const union MHD_DaemonInfo *
MHD_get_daemon_info (struct MHD_Daemon *daemon,
                     enum MHD_DaemonInfoType info_type,
                     ...)
{
  if (NULL == daemon)
    return NULL;

  mhd_assert ((0 == (daemon->options & MHD_USE_SELECT_INTERNALLY)) || \
              (NULL != daemon->worker_pool) || \
              (MHD_thread_handle_ID_is_valid_handle_ (daemon->tid)));
  mhd_assert (((0 != (daemon->options & MHD_USE_SELECT_INTERNALLY)) &&
               (NULL == daemon->worker_pool)) || \
              (! MHD_thread_handle_ID_is_valid_handle_ (daemon->tid)));

  switch (info_type)
  {
  case MHD_DAEMON_INFO_KEY_SIZE:
    return NULL;   /* no longer supported */
  case MHD_DAEMON_INFO_MAC_KEY_SIZE:
    return NULL;   /* no longer supported */
  case MHD_DAEMON_INFO_LISTEN_FD:
    daemon->daemon_info_dummy_listen_fd.listen_fd = daemon->listen_fd;
    return &daemon->daemon_info_dummy_listen_fd;
  case MHD_DAEMON_INFO_EPOLL_FD:
#ifdef EPOLL_SUPPORT
    daemon->daemon_info_dummy_epoll_fd.epoll_fd = daemon->epoll_fd;
    return &daemon->daemon_info_dummy_epoll_fd;
#else  /* ! EPOLL_SUPPORT */
    return NULL;
#endif /* ! EPOLL_SUPPORT */
  case MHD_DAEMON_INFO_CURRENT_CONNECTIONS:
    if (! MHD_D_IS_THREAD_SAFE_ (daemon))
      MHD_cleanup_connections (daemon);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    else if (daemon->worker_pool)
    {
      unsigned int i;
      /* Collect the connection information stored in the workers. */
      daemon->connections = 0;
      for (i = 0; i < daemon->worker_pool_size; i++)
      {
        /* FIXME: next line is thread-safe only if read is atomic. */
        daemon->connections += daemon->worker_pool[i].connections;
      }
    }
#endif
    daemon->daemon_info_dummy_num_connections.num_connections
      = daemon->connections;
    return &daemon->daemon_info_dummy_num_connections;
  case MHD_DAEMON_INFO_FLAGS:
    daemon->daemon_info_dummy_flags.flags = daemon->options;
    return &daemon->daemon_info_dummy_flags;
  case MHD_DAEMON_INFO_BIND_PORT:
    daemon->daemon_info_dummy_port.port = daemon->port;
    return &daemon->daemon_info_dummy_port;
  default:
    return NULL;
  }
}


/**
 * Obtain the version of this library
 *
 * @return static version string, e.g. "0.9.9"
 * @ingroup specialized
 */
_MHD_EXTERN const char *
MHD_get_version (void)
{
#ifdef PACKAGE_VERSION
  return PACKAGE_VERSION;
#else  /* !PACKAGE_VERSION */
  static char ver[12] = "\0\0\0\0\0\0\0\0\0\0\0";
  if (0 == ver[0])
  {
    int res = MHD_snprintf_ (ver,
                             sizeof(ver),
                             "%x.%x.%x",
                             (int) (((uint32_t) MHD_VERSION >> 24) & 0xFF),
                             (int) (((uint32_t) MHD_VERSION >> 16) & 0xFF),
                             (int) (((uint32_t) MHD_VERSION >> 8) & 0xFF));
    if ((0 >= res) || (sizeof(ver) <= res))
      return "0.0.0"; /* Can't return real version */
  }
  return ver;
#endif /* !PACKAGE_VERSION */
}


/**
 * Obtain the version of this library as a binary value.
 *
 * @return version binary value, e.g. "0x00090900" (#MHD_VERSION of
 *         compiled MHD binary)
 * @note Available since #MHD_VERSION 0x00097601
 * @ingroup specialized
 */
_MHD_EXTERN uint32_t
MHD_get_version_bin (void)
{
  return (uint32_t) MHD_VERSION;
}


/**
 * Get information about supported MHD features.
 * Indicate that MHD was compiled with or without support for
 * particular feature. Some features require additional support
 * by kernel. Kernel support is not checked by this function.
 *
 * @param feature type of requested information
 * @return #MHD_YES if feature is supported by MHD, #MHD_NO if
 * feature is not supported or feature is unknown.
 * @ingroup specialized
 */
_MHD_EXTERN enum MHD_Result
MHD_is_feature_supported (enum MHD_FEATURE feature)
{
  switch (feature)
  {
  case MHD_FEATURE_MESSAGES:
#ifdef HAVE_MESSAGES
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_TLS:
#ifdef HTTPS_SUPPORT
    return MHD_YES;
#else  /* ! HTTPS_SUPPORT */
    return MHD_NO;
#endif  /* ! HTTPS_SUPPORT */
  case MHD_FEATURE_HTTPS_CERT_CALLBACK:
#if defined(HTTPS_SUPPORT) && GNUTLS_VERSION_MAJOR >= 3
    return MHD_YES;
#else  /* !HTTPS_SUPPORT || GNUTLS_VERSION_MAJOR < 3 */
    return MHD_NO;
#endif /* !HTTPS_SUPPORT || GNUTLS_VERSION_MAJOR < 3 */
  case MHD_FEATURE_HTTPS_CERT_CALLBACK2:
#if defined(HTTPS_SUPPORT) && GNUTLS_VERSION_NUMBER >= 0x030603
    return MHD_YES;
#else  /* !HTTPS_SUPPORT || GNUTLS_VERSION_NUMBER < 0x030603 */
    return MHD_NO;
#endif /* !HTTPS_SUPPORT || GNUTLS_VERSION_NUMBER < 0x030603 */
  case MHD_FEATURE_IPv6:
#ifdef HAVE_INET6
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_IPv6_ONLY:
#if defined(IPPROTO_IPV6) && defined(IPV6_V6ONLY)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_POLL:
#ifdef HAVE_POLL
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_EPOLL:
#ifdef EPOLL_SUPPORT
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_SHUTDOWN_LISTEN_SOCKET:
#ifdef HAVE_LISTEN_SHUTDOWN
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_SOCKETPAIR:
#ifdef _MHD_ITC_SOCKETPAIR
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_TCP_FASTOPEN:
#ifdef TCP_FASTOPEN
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_BASIC_AUTH:
#ifdef BAUTH_SUPPORT
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH:
#ifdef DAUTH_SUPPORT
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_POSTPROCESSOR:
#ifdef HAVE_POSTPROCESSOR
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_HTTPS_KEY_PASSWORD:
#if defined(HTTPS_SUPPORT) && GNUTLS_VERSION_NUMBER >= 0x030111
    return MHD_YES;
#else  /* !HTTPS_SUPPORT || GNUTLS_VERSION_NUMBER < 0x030111 */
    return MHD_NO;
#endif /* !HTTPS_SUPPORT || GNUTLS_VERSION_NUMBER < 0x030111 */
  case MHD_FEATURE_LARGE_FILE:
#if defined(HAVE_PREAD64) || defined(_WIN32)
    return MHD_YES;
#elif defined(HAVE_PREAD)
    return (sizeof(uint64_t) > sizeof(off_t)) ? MHD_NO : MHD_YES;
#elif defined(HAVE_LSEEK64)
    return MHD_YES;
#else
    return (sizeof(uint64_t) > sizeof(off_t)) ? MHD_NO : MHD_YES;
#endif
  case MHD_FEATURE_THREAD_NAMES:
#if defined(MHD_USE_THREAD_NAME_)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_UPGRADE:
#if defined(UPGRADE_SUPPORT)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_RESPONSES_SHARED_FD:
#if defined(HAVE_PREAD64) || defined(HAVE_PREAD) || defined(_WIN32)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_AUTODETECT_BIND_PORT:
#ifdef MHD_USE_GETSOCKNAME
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_AUTOSUPPRESS_SIGPIPE:
#if defined(MHD_SEND_SPIPE_SUPPRESS_POSSIBLE) || \
    ! defined(MHD_SEND_SPIPE_SUPPRESS_NEEDED)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_SENDFILE:
#ifdef _MHD_HAVE_SENDFILE
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_THREADS:
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_HTTPS_COOKIE_PARSING:
#if defined(COOKIE_SUPPORT)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_RFC2069:
#ifdef DAUTH_SUPPORT
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_MD5:
#if defined(DAUTH_SUPPORT) && defined(MHD_MD5_SUPPORT)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_SHA256:
#if defined(DAUTH_SUPPORT) && defined(MHD_SHA256_SUPPORT)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_SHA512_256:
#if defined(DAUTH_SUPPORT) && defined(MHD_SHA512_256_SUPPORT)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_AUTH_INT:
#ifdef DAUTH_SUPPORT
    return MHD_NO;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_ALGO_SESSION:
#ifdef DAUTH_SUPPORT
    return MHD_NO;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DIGEST_AUTH_USERHASH:
#ifdef DAUTH_SUPPORT
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_EXTERN_HASH:
#if defined(MHD_MD5_TLSLIB) || defined(MHD_SHA256_TLSLIB)
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_DEBUG_BUILD:
#ifdef _DEBUG
    return MHD_YES;
#else
    return MHD_NO;
#endif
  case MHD_FEATURE_FLEXIBLE_FD_SETSIZE:
#ifdef HAS_FD_SETSIZE_OVERRIDABLE
    return MHD_YES;
#else  /* ! HAS_FD_SETSIZE_OVERRIDABLE */
    return MHD_NO;
#endif /* ! HAS_FD_SETSIZE_OVERRIDABLE */

  default:
    break;
  }
  return MHD_NO;
}


#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#if defined(HTTPS_SUPPORT) && GCRYPT_VERSION_NUMBER < 0x010600
#if defined(MHD_USE_POSIX_THREADS)
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#elif defined(MHD_W32_MUTEX_)

static int
gcry_w32_mutex_init (void **ppmtx)
{
  *ppmtx = malloc (sizeof (MHD_mutex_));

  if (NULL == *ppmtx)
    return ENOMEM;
  if (! MHD_mutex_init_ ((MHD_mutex_ *) *ppmtx))
  {
    free (*ppmtx);
    *ppmtx = NULL;
    return EPERM;
  }

  return 0;
}


static int
gcry_w32_mutex_destroy (void **ppmtx)
{
  int res = (MHD_mutex_destroy_ ((MHD_mutex_ *) *ppmtx)) ? 0 : EINVAL;
  free (*ppmtx);
  return res;
}


static int
gcry_w32_mutex_lock (void **ppmtx)
{
  return MHD_mutex_lock_ ((MHD_mutex_ *) *ppmtx) ? 0 : EINVAL;
}


static int
gcry_w32_mutex_unlock (void **ppmtx)
{
  return MHD_mutex_unlock_ ((MHD_mutex_ *) *ppmtx) ? 0 : EINVAL;
}


static struct gcry_thread_cbs gcry_threads_w32 = {
  (GCRY_THREAD_OPTION_USER | (GCRY_THREAD_OPTION_VERSION << 8)),
  NULL, gcry_w32_mutex_init, gcry_w32_mutex_destroy,
  gcry_w32_mutex_lock, gcry_w32_mutex_unlock,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

#endif /* defined(MHD_W32_MUTEX_) */
#endif /* HTTPS_SUPPORT && GCRYPT_VERSION_NUMBER < 0x010600 */
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */

/**
 * Initialize do setup work.
 */
void
MHD_init (void)
{
#if defined(MHD_WINSOCK_SOCKETS)
  WSADATA wsd;
#endif /* MHD_WINSOCK_SOCKETS */

  MHD_set_panic_func (NULL, NULL);

#if defined(MHD_WINSOCK_SOCKETS)
  if (0 != WSAStartup (MAKEWORD (2, 2), &wsd))
    MHD_PANIC (_ ("Failed to initialize winsock.\n"));
  if ((2 != LOBYTE (wsd.wVersion)) && (2 != HIBYTE (wsd.wVersion)))
    MHD_PANIC (_ ("Winsock version 2.2 is not available.\n"));
#endif /* MHD_WINSOCK_SOCKETS */
#ifdef HTTPS_SUPPORT
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#if GCRYPT_VERSION_NUMBER < 0x010600
#if GNUTLS_VERSION_NUMBER <= 0x020b00
#if defined(MHD_USE_POSIX_THREADS)
  if (0 != gcry_control (GCRYCTL_SET_THREAD_CBS,
                         &gcry_threads_pthread))
    MHD_PANIC (_ ("Failed to initialise multithreading in libgcrypt.\n"));
#elif defined(MHD_W32_MUTEX_)
  if (0 != gcry_control (GCRYCTL_SET_THREAD_CBS,
                         &gcry_threads_w32))
    MHD_PANIC (_ ("Failed to initialise multithreading in libgcrypt.\n"));
#endif /* defined(MHD_W32_MUTEX_) */
#endif /* GNUTLS_VERSION_NUMBER <= 0x020b00 */
  gcry_check_version (NULL);
#else
  if (NULL == gcry_check_version ("1.6.0"))
    MHD_PANIC (_ ("libgcrypt is too old. MHD was compiled for " \
                  "libgcrypt 1.6.0 or newer.\n"));
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
  gnutls_global_init ();
#endif /* HTTPS_SUPPORT */
  MHD_monotonic_sec_counter_init ();
  MHD_send_init_static_vars_ ();
  MHD_init_mem_pools_ ();
  /* Check whether sizes were correctly detected by configure */
#ifdef _DEBUG
  if (1)
  {
    struct timeval tv;
    mhd_assert (sizeof(tv.tv_sec) == SIZEOF_STRUCT_TIMEVAL_TV_SEC);
  }
#endif /* _DEBUG */
  mhd_assert (sizeof(uint64_t) == SIZEOF_UINT64_T);
}


void
MHD_fini (void)
{
#ifdef HTTPS_SUPPORT
  gnutls_global_deinit ();
#endif /* HTTPS_SUPPORT */
#if defined(MHD_WINSOCK_SOCKETS)
  WSACleanup ();
#endif /* MHD_WINSOCK_SOCKETS */
  MHD_monotonic_sec_counter_finish ();
}


#ifdef _AUTOINIT_FUNCS_ARE_SUPPORTED
_SET_INIT_AND_DEINIT_FUNCS (MHD_init, MHD_fini);
#endif /* _AUTOINIT_FUNCS_ARE_SUPPORTED */

/* end of daemon.c */

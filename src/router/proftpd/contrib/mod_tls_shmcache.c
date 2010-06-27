/*
 * ProFTPD: mod_tls_shmcache -- a module which provides a shared SSL session
 *                              cache using SysV shared memory
 *
 * Copyright (c) 2009 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * This is mod_tls_shmcache, contrib software for proftpd 1.3.x and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 *  --- DO NOT DELETE BELOW THIS LINE ----
 *  $Id: mod_tls_shmcache.c,v 1.7 2009/12/18 17:40:13 castaglia Exp $
 *  $Libraries: -lssl -lcrypto$
 */

#include "conf.h"
#include "privs.h"
#include "mod_tls.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#ifdef HAVE_MLOCK
# include <sys/mman.h>
#endif

#define MOD_TLS_SHMCACHE_VERSION		"mod_tls_shmcache/0.1"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030301
# error "ProFTPD 1.3.3rc1 or later required"
#endif

module tls_shmcache_module;

#define TLS_SHMCACHE_PROJ_ID	247

/* Assume a maximum SSL session (serialized) length of 10K.  Note that this
 * is different from the SSL_MAX_SSL_SESSION_ID_LENGTH provided by OpenSSL.
 * There is no limit imposed on the length of the ASN1 description of the
 * SSL session data.
 */
#ifndef TLS_MAX_SSL_SESSION_SIZE
# define TLS_MAX_SSL_SESSION_SIZE	1024 * 10
#endif

/* The default number of SSL sessions cached in OpenSSL's internal cache
 * is SSL_SESSION_CACHE_MAX_SIZE_DEFAULT, which is defined as 1024*20.
 * This is NOT a size in _bytes_, but is a size in _counts_ of sessions.
 *
 * Thus the default size of our shm segment should also, in theory, be
 * able to hold the same number of sessions.
 *
 * The recommended default size for Apache's mod_ssl shm segment is 512000
 * bytes (500KB).
 */

struct shmcache_entry {
  time_t expires;
  unsigned int sess_id_len;
  unsigned char sess_id[SSL_MAX_SSL_SESSION_ID_LENGTH];
  unsigned int sess_datalen;
  unsigned char sess_data[TLS_MAX_SSL_SESSION_SIZE];
};

/* The difference between shmcache_entry and shmcache_large_entry is that the
 * buffers in the latter are dynamically allocated from the heap, not
 * allocated out of the shm segment.  The large_entry struct is used for
 * storing sessions which don't fit into the normal entry struct; this also
 * means that these large entries are NOT shared across processes.
 */
struct shmcache_large_entry {
  time_t expires;
  unsigned int sess_id_len;
  unsigned char *sess_id;
  unsigned int sess_datalen;
  unsigned char *sess_data;
};

/* The number of entries in the list is determined at run-time, based on
 * the maximum desired size of the shared memory segment.
 */
struct shmcache_data {

  /* Cache metadata. */
  unsigned int nhits;
  unsigned int nmisses;

  unsigned int nstored;
  unsigned int ndeleted;
  unsigned int nexpired;
  unsigned int nerrors;

  /* This tracks the number of sessions that could not be added because
   * they exceeded TLS_MAX_SSL_SESSION_SIZE.
   */
  unsigned int nexceeded;
  unsigned int exceeded_maxsz;

  /* Track the timestamp of the next session to expire in the cache; used
   * as an optimization when flushing the cache of expired sessions.
   */
  time_t next_expiring;

  /* These listlen/listsz track the number of entries in the cache and total
   * entries possible, and thus can be used for determining the fullness of
   * the cache.
   */
  unsigned int sd_listlen, sd_listsz;

  /* It is important that this field be the last in the struct! */
  struct shmcache_entry *sd_entries;
};

static tls_sess_cache_t shmcache;

static struct shmcache_data *shmcache_data = NULL;
static size_t shmcache_datasz = 0;
static int shmcache_shmid = -1;
static pr_fh_t *shmcache_fh = NULL;

static array_header *shmcache_sess_list = NULL;

static const char *trace_channel = "tls_shmcache";

static int shmcache_close(tls_sess_cache_t *);

static const char *shmcache_get_crypto_errors(void) {
  unsigned int count = 0;
  unsigned long e = ERR_get_error();
  BIO *bio = NULL;
  char *data = NULL;
  long datalen;
  const char *str = "(unknown)";

  /* Use ERR_print_errors() and a memory BIO to build up a string with
   * all of the error messages from the error queue.
   */

  if (e)
    bio = BIO_new(BIO_s_mem());

  while (e) {
    pr_signals_handle();
    BIO_printf(bio, "\n  (%u) %s", ++count, ERR_error_string(e, NULL));
    e = ERR_get_error();
  }

  datalen = BIO_get_mem_data(bio, &data);
  if (data) {
    data[datalen] = '\0';
    str = pstrdup(permanent_pool, data);
  }

  if (bio)
    BIO_free(bio);

  return str;
}

static const char *shmcache_get_lock_desc(int lock_type) {
  const char *lock_desc;

  switch (lock_type) {
    case F_RDLCK:
      lock_desc = "read-lock";
      break;

    case F_WRLCK:
      lock_desc = "write-lock";
      break;

    case F_UNLCK:
      lock_desc = "unlock";
      break;

    default:
      lock_desc = "[unknown]";
  }

  return lock_desc;
}

/* XXX There is anecdotal (and real) evidence that using SysV semaphores
 * is faster than fcntl(2)/flock(3).  However, semaphores are not cleaned up
 * if the process dies tragically.  Could possibly deal with this in an
 * exit event handler, though.  Something to keep in mind.
 */
static int shmcache_lock_shm(int lock_type) {
  const char *lock_desc;
  int fd;
  struct flock lock;
  unsigned int nattempts = 1;

  lock.l_type = lock_type;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;

  fd = PR_FH_FD(shmcache_fh);
  lock_desc = shmcache_get_lock_desc(lock_type);

  pr_trace_msg(trace_channel, 9, "attempting to %s shmcache fd %d", lock_desc,
    fd);

  while (fcntl(fd, F_SETLK, &lock) < 0) {
    int xerrno = errno;

    if (xerrno == EINTR) {
      pr_signals_handle();
      continue;
    }

    pr_trace_msg(trace_channel, 3, "%s of shmcache fd %d failed: %s",
      lock_desc, fd, strerror(xerrno));
    if (xerrno == EACCES) {
      struct flock locker;

      /* Get the PID of the process blocking this lock. */
      if (fcntl(fd, F_GETLK, &locker) == 0) {
        pr_trace_msg(trace_channel, 3, "process ID %lu has blocking %s on "
          "shmcache fd %d", (unsigned long) locker.l_pid,
          shmcache_get_lock_desc(locker.l_type), fd);
      }

      /* Treat this as an interrupted call, call pr_signals_handle() (which
       * will delay for a few msecs because of EINTR), and try again.
       * After 10 attempts, give up altogether.
       */

      nattempts++;
      if (nattempts <= 10) {
        errno = EINTR;

        pr_signals_handle();
        continue;
      }

      errno = xerrno;
      return -1;
    }

    errno = xerrno;
    return -1;
  }

  pr_trace_msg(trace_channel, 9, "%s of shmcache fd %d succeeded", lock_desc,
    fd);
  return 0;
}

/* Use a hash function to hash the given lookup key to a slot in the
 * sd_entries list.  This hash, module the number of entries, is the initial
 * iteration start point.  This will hopefully avoid having to do many linear
 * scans for the add/get/delete operations.
 *
 * Use Perl's hashing algorithm.
 */
static unsigned int shmcache_hash(unsigned char *sess_id,
    unsigned int sess_id_len) {
  unsigned int i = 0;
  size_t sz = sess_id_len;

  while (sz--) {
    const unsigned char *k = sess_id;
    unsigned int c = *k;
    k++;

    /* Always handle signals in potentially long-running while loops. */
    pr_signals_handle();

    i = (i * 33) + c;
  }

  return i;
}

static struct shmcache_data *shmcache_get_shm(pr_fh_t *fh,
    size_t requested_size) {
  int rem, shmid, xerrno = 0;
  int shm_existed = FALSE;
  struct shmcache_data *data = NULL;
  size_t shm_size;
  unsigned int shm_sess_max = 0;
  key_t key;

  /* Calculate the size to allocate.  First, calculate the maximum number
   * of sessions we can cache, given the configured size.  Then
   * calculate the shm segment size to allocate to hold that number of
   * sessions.  Round the segment size up to the nearest SHMLBA boundary.
   */
  shm_sess_max = (requested_size - sizeof(struct shmcache_data)) /
    (sizeof(struct shmcache_entry));
  shm_size = sizeof(struct shmcache_data) +
    (shm_sess_max * sizeof(struct shmcache_entry));

  rem = shm_size % SHMLBA;
  if (rem != 0) {
    shm_size = (shm_size - rem + SHMLBA);
    pr_log_debug(DEBUG9, MOD_TLS_SHMCACHE_VERSION
      ": rounded requested size up to %lu bytes", (unsigned long) shm_size);
  }

  key = ftok(fh->fh_path, TLS_SHMCACHE_PROJ_ID);
  if (key == (key_t) -1) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": unable to get key for path '%s': %s", fh->fh_path, strerror(errno));
    return NULL;
  }

  /* Try first using IPC_CREAT|IPC_EXCL, to check if there is an existing
   * shm for this key.  If so, use a flags value of zero.
   *
   * We use root privs for this, to make sure that the shm can only be
   * access by a process with root privs.  This is equivalent to having
   * a root-owned file in the filesystem.  We need to protect the sensitive
   * session data (which contains master keys and such) from prying eyes.
   */

  PRIVS_ROOT
  shmid = shmget(key, shm_size, IPC_CREAT|IPC_EXCL|0600);
  xerrno = errno;
  PRIVS_RELINQUISH

  if (shmid < 0) {
    if (xerrno == EEXIST) {
      shm_existed = TRUE;

      PRIVS_ROOT
      shmid = shmget(key, 0, 0);
      xerrno = errno;
      PRIVS_RELINQUISH

      if (shmid < 0) {
        pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
          ": unable to get shm for existing key: %s", strerror(xerrno));
        errno = xerrno;
        return NULL;
      }

    } else {
      errno = xerrno;
      return NULL;
    }
  }

  /* Attach to the shm. */
  pr_log_debug(DEBUG10, MOD_TLS_SHMCACHE_VERSION
    ": attempting to attach to shm ID %d", shmid);

  PRIVS_ROOT
  data = (struct shmcache_data *) shmat(shmid, NULL, 0);
  xerrno = errno;
  PRIVS_RELINQUISH

  if (data == NULL) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": unable to attach to shm ID %d: %s", shmid, strerror(xerrno));
    errno = xerrno;
    return NULL;
  }

  if (shm_existed) {
    struct shmid_ds ds;
    int res;

    /* If we already have a shmid, check for size differences; the admin
     * may have configured a larger/smaller cache size.  Use shmctl(IP_STAT)
     * to determine the existing segment size.
     */

    PRIVS_ROOT
    res = shmctl(shmid, IPC_STAT, &ds);
    xerrno = errno;
    PRIVS_RELINQUISH

    if (res == 0) {
      pr_log_debug(DEBUG10, MOD_TLS_SHMCACHE_VERSION
        ": existing shm size: %u bytes", (unsigned int) ds.shm_segsz);

      if (ds.shm_segsz != shm_size) {
        if (ds.shm_segsz > shm_size) {
          pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
            ": requested shm size (%lu bytes) is smaller than existing shm "
            "size, migrating to smaller shm (may result in loss of cache data)",
            (unsigned long) shm_size);

        } else if (ds.shm_segsz < shm_size) {
          pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
            ": requested shm size (%lu bytes) is larger than existing shm "
            "size, migrating to larger shm", (unsigned long) shm_size);
        }

        /* XXX In future versions, we could probably handle the migration
         * of the existing cache data to a different size shm segment.
         *
         * If the shm size was increased:
         *   Simply iterate through existing valid cached sessions and
         *   load them into the new shm segment; the hash index will change
         *   because of the change in shm size, hence the reloading.
         *
         * If the shm size was decreased:
         *   Need to first sort existing valid cached sessions by expiration
         *   timestamp, in _decreasing_ order; this sorted order is the order
         *   in which they will be loaded into the new shm.  This guarantees
         *   that the sessions _most_ likely to expire will be added later,
         *   when the possibility of a cache fill in the smaller shm is higher.
         *   Those older sessions, once the cache is full, would be lost.
         *
         * For now, though, we complain about this, and tell the admin to
         * manually remove shm.
         */

        pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
          ": remove existing shmcache using 'ftpdctl tls sesscache remove' "
          "before using new size");

        shmcache_close(NULL);

        errno = EINVAL;
        return NULL;
      }

    } else {
      pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
        ": unable to stat shm ID %d: %s", shmid, strerror(xerrno));
      errno = xerrno;
    }

  } else {
    /* Make sure the memory is initialized. */
    if (shmcache_lock_shm(F_WRLCK) < 0) {
      pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
        ": error write-locking shmcache: %s", strerror(errno));
    }

    memset(data, 0, shm_size);

    if (shmcache_lock_shm(F_UNLCK) < 0) {
      pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
        ": error unlocking shmcache: %s", strerror(errno));
    }
  }

  shmcache_datasz = shm_size;

  shmcache_shmid = shmid;
  pr_log_debug(DEBUG9, MOD_TLS_SHMCACHE_VERSION
    ": using shm ID %d for shmcache path '%s'", shmcache_shmid,
    fh->fh_path);

  data->sd_entries = (struct shmcache_entry *) (data + sizeof(struct shmcache_data));
  data->sd_listsz = shm_sess_max;

  return data;
}

/* Scan the entire list, clearing out expired sessions.  Logs the number
 * of sessions that expired and updates the header stat.
 *
 * NOTE: Callers are assumed to handle the locking of the shm before/after
 * calling this function!
 */
static unsigned int shmcache_flush(void) {
  register unsigned int i;
  unsigned int flushed = 0;
  time_t now, next_expiring = 0;

  now = time(NULL);

  /* We always scan the in-memory large session entry list. */
  if (shmcache_sess_list != NULL) {
    struct shmcache_large_entry *entries;

    entries = shmcache_sess_list->elts;
    for (i = 0; i < shmcache_sess_list->nelts; i++) {
      struct shmcache_large_entry *entry;

      entry = &(entries[i]);

      if (entry->expires > now) {
        /* This entry has expired; clear its slot. */
        entry->expires = 0;
        pr_memscrub(entry->sess_data, entry->sess_datalen);
      }
    }
  }

  /* If now is earlier than the earliest expiring session in the cache,
   * then a scan will be pointless.
   */
  if (now < shmcache_data->next_expiring) {
    unsigned int secs;

    secs = shmcache_data->next_expiring - now;
    tls_log("shmcache: no expired sessions to flush; %u secs to next "
      "expiration", secs);
    return 0;
  }

  tls_log("shmcache: flushing cache of expired sessions");

  for (i = 0; i < shmcache_data->sd_listsz; i++) {
    struct shmcache_entry *entry;

    entry = &(shmcache_data->sd_entries[i]);
    if (entry->expires > 0) {
      if (entry->expires > now) {
        if (entry->expires < next_expiring) {
          next_expiring = entry->expires;
        }

      } else {
        /* This entry has expired; clear its slot. */
        entry->expires = 0;
        pr_memscrub(entry->sess_data, entry->sess_datalen);

        /* Don't forget to update the stats. */
        shmcache_data->nexpired++;

        if (shmcache_data->sd_listlen > 0) {
          shmcache_data->sd_listlen--;
        }

        flushed++;
      }
    }

    shmcache_data->next_expiring = next_expiring;
  }

  tls_log("shmcache: flushed %u expired %s from cache", flushed,
    flushed != 1 ? "sessions" : "session");
  return flushed;
}

/* Cache implementation callbacks.
 */

static int shmcache_open(tls_sess_cache_t *cache, char *info, long timeout) {
  int fd;
  char *ptr;
  size_t requested_size;

  pr_trace_msg(trace_channel, 9, "opening shmcache cache %p", cache);

  /* The info string must be formatted like:
   *
   *  /file=%s[&size=%u]
   *
   * where the optional size is in bytes.  There is a minimum size; if the
   * configured size is less than the minimum, it's an error.  The default
   * size (when no size is explicitly configured) is, of course, larger than
   * the minimum size.
   */

  if (strncmp(info, "/file=", 6) != 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
      ": badly formatted info '%s', unable to open shmcache", info);
    errno = EINVAL;
    return -1;
  }

  info += 6;

  /* Check for the optional size parameter. */
  ptr = strchr(info, '&');
  if (ptr != NULL) {
    if (strncmp(ptr + 1, "size=", 5) == 0) {
      char *tmp = NULL;
      long size; 

      size = strtol(ptr + 6, &tmp, 10);
      if (tmp && *tmp) {
        pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
          ": badly formatted size parameter '%s', ignoring", ptr + 1);

        /* Default size of 1.5M.  That should hold around 100 sessions. */
        requested_size = 1538 * 1024;

      } else {
        size_t min_size;

        /* The bare minimum size MUST be able to hold at least one session. */
        min_size = sizeof(struct shmcache_data) +
          sizeof(struct shmcache_entry);

        if (size < min_size) {
          pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
            ": requested size (%lu bytes) smaller than minimum size "
            "(%lu bytes), ignoring", (unsigned long) size,
            (unsigned long) min_size);
        
          /* Default size of 1.5M.  That should hold around 100 sessions. */
          requested_size = 1538 * 1024;

        } else {
          requested_size = size;
        }
      }

    } else {
      pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
        ": badly formatted size parameter '%s', ignoring", ptr + 1);

      /* Default size of 1.5M.  That should hold around 100 sessions. */
      requested_size = 1538 * 1024;
    }

    *ptr = '\0';

  } else {
    /* Default size of 1.5M.  That should hold around 100 sessions. */
    requested_size = 1538 * 1024;
  }

  /* We could change the cache_mode flags here, based on the given
   * info, if needs be.
   */

  if (pr_fs_valid_path(info) < 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
      ": file '%s' not an absolute path", info);

    errno = EINVAL;
    return -1;
  }

  /* If shmcache_fh is not null, then we are a restarted server.  And if
   * the 'info' path does not match that previous fh, then the admin
   * has changed the configuration.
   *
   * For now, we complain about this, and tell the admin to manually remove
   * the old file/shm.
   */
  if (shmcache_fh != NULL &&
      strcmp(shmcache_fh->fh_path, info) != 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
      ": file '%s' does not match previously configured file '%s'",
      info, shmcache_fh->fh_path);
    pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
      ": remove existing shmcache using 'ftpdctl tls sesscache remove' "
      "before using new file");

    errno = EINVAL;
    return -1;
  }

  PRIVS_ROOT
  shmcache_fh = pr_fsio_open(info, O_RDWR|O_CREAT);
  PRIVS_RELINQUISH

  if (shmcache_fh == NULL) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": error: unable to open file '%s': %s", info, strerror(errno));

    errno = EINVAL;
    return -1;
  }

  /* Make sure that we don't inadvertently get one of the Big Three file
   * descriptors (stdin/stdout/stderr), as can happen especially if the
   * server has restarted.
   */
  fd = PR_FH_FD(shmcache_fh);
  if (fd <= STDERR_FILENO) {
    int res;

    res = pr_fs_get_usable_fd(fd);
    if (res < 0) { 
      pr_log_debug(DEBUG0,
        "warning: unable to find good fd for shmcache fd %d: %s",
        fd, strerror(errno));
 
    } else {
      close(fd);
      PR_FH_FD(shmcache_fh) = res;
    }
  }

  pr_log_debug(DEBUG10, MOD_TLS_SHMCACHE_VERSION
    ": requested shmcache file: %s (fd %d)", shmcache_fh->fh_path,
    PR_FH_FD(shmcache_fh));
  pr_log_debug(DEBUG10, MOD_TLS_SHMCACHE_VERSION
    ": requested shmcache size: %lu bytes", (unsigned long) requested_size);

  shmcache_data = shmcache_get_shm(shmcache_fh, requested_size);
  if (shmcache_data == NULL) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": unable to allocate shm: %s", strerror(errno));
    pr_fsio_close(shmcache_fh);
    shmcache_fh = NULL;

    errno = EINVAL;
    return -1;
  }

  cache->cache_pool = make_sub_pool(session.pool);
  pr_pool_tag(cache->cache_pool, MOD_TLS_SHMCACHE_VERSION);

  cache->cache_timeout = timeout;

  return 0;
}

static int shmcache_close(tls_sess_cache_t *cache) {
  pr_trace_msg(trace_channel, 9, "closing shmcache cache %p", cache);

  if (cache != NULL &&
      cache->cache_pool != NULL) {
    destroy_pool(cache->cache_pool);

    if (shmcache_sess_list != NULL) {
      register unsigned int i;
      struct shmcache_large_entry *entries;

      entries = shmcache_sess_list->elts;
      for (i = 0; i < shmcache_sess_list->nelts; i++) {
        struct shmcache_large_entry *entry;

        entry = &(entries[i]);
        if (entry->expires > 0) {
          pr_memscrub(entry->sess_data, entry->sess_datalen);
        }
      }

      shmcache_sess_list = NULL;
    }
  }

  if (shmcache_shmid >= 0) {
    int res, xerrno = 0;

    PRIVS_ROOT
#if !defined(_POSIX_SOURCE)
    res = shmdt((char *) shmcache_data);
#else
    res = shmdt((const char *) shmcache_data);
#endif
    xerrno = errno;
    PRIVS_RELINQUISH

    if (res < 0) {
      pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
        ": error detaching shm ID %d: %s", shmcache_shmid, strerror(xerrno));
    }

    shmcache_data = NULL;
  }

  pr_fsio_close(shmcache_fh);
  shmcache_fh = NULL;
  return 0;
}

static int shmcache_add_large_sess(tls_sess_cache_t *cache,
    unsigned char *sess_id, unsigned int sess_id_len, time_t expires,
    SSL_SESSION *sess, int sess_len) {
  struct shmcache_large_entry *entry;

  if (sess_len > TLS_MAX_SSL_SESSION_SIZE) {
    /* We may get sessions to add to the list which do not exceed the max
     * size, but instead are here because we couldn't get the lock on the
     * shmcache.  Don't track these in the 'exceeded' stats'.
     */

    if (shmcache_lock_shm(F_WRLCK) == 0) {
      shmcache_data->nexceeded++;
      if (sess_len > shmcache_data->exceeded_maxsz) {
        shmcache_data->exceeded_maxsz = sess_len;
      }

      if (shmcache_lock_shm(F_UNLCK) < 0) {
        tls_log("shmcache: error unlocking shmcache: %s",
        strerror(errno));
      }

    } else {
      tls_log("shmcache: error write-locking shmcache: %s",
        strerror(errno));
    }
  }

  if (shmcache_sess_list != NULL) {
    register unsigned int i;
    struct shmcache_large_entry *entries;
    time_t now;

    /* Look for any expired sessions in the list to overwrite/reuse. */
    entries = shmcache_sess_list->elts;
    now = time(NULL);
    for (i = 0; i < shmcache_sess_list->nelts; i++) {
      entry = &(entries[i]);

      if (entry->expires > now) {
        /* This entry has expired; clear and reuse its slot. */
        entry->expires = 0;
        pr_memscrub(entry->sess_data, entry->sess_datalen);

        break;
      }
    }

  } else {
    shmcache_sess_list = make_array(cache->cache_pool, 1,
      sizeof(struct shmcache_large_entry));
    entry = push_array(shmcache_sess_list);
  }

  entry->expires = expires;
  entry->sess_id_len = sess_id_len;
  entry->sess_id = palloc(cache->cache_pool, sess_id_len);
  memcpy(entry->sess_id, sess_id, sess_id_len);
  entry->sess_datalen = sess_len;
  entry->sess_data = palloc(cache->cache_pool, sess_len);
  i2d_SSL_SESSION(sess, &(entry->sess_data));

  return 0;
}

static int shmcache_add(tls_sess_cache_t *cache, unsigned char *sess_id,
    unsigned int sess_id_len, time_t expires, SSL_SESSION *sess) {
  register unsigned int i;
  unsigned int h, idx, last;
  int found_slot = FALSE, need_lock = TRUE, res = 0, sess_len;

  pr_trace_msg(trace_channel, 9, "adding session to shmcache cache %p", cache);

  /* First we need to find out how much space is needed for the serialized
   * session data.  There is no known maximum size for SSL session data;
   * this module is currently designed to allow only up to a certain size.
   */
  sess_len = i2d_SSL_SESSION(sess, NULL);
  if (sess_len > TLS_MAX_SSL_SESSION_SIZE) {
    tls_log("shmcache: length of serialized SSL session data (%d) exceeds "
      "maximum size (%u), unable to add to shared shmcache, adding to list",
      sess_len, TLS_MAX_SSL_SESSION_SIZE);

    /* Instead of rejecting the add here, we add the session to a "large
     * session" list.  Thus the large session would still be cached per process
     * and will not be lost.
     *
     * XXX We should also track how often this happens, and possibly trigger
     * a shmcache resize (using a larger record size, vs larger cache size)
     * so that we can cache these large records in the shm segment.
     */

    return shmcache_add_large_sess(cache, sess_id, sess_id_len, expires,
      sess, sess_len);
  }

  if (shmcache_data->sd_listlen == shmcache_data->sd_listsz) {
    /* It appears that the cache is full.  Try flushing any expired
     * sessions.
     */

    if (shmcache_lock_shm(F_WRLCK) == 0) {
      if (shmcache_flush() > 0) {
        /* If we made room, then do NOT release the lock; we keep the lock
         * so that we can add the session.
         */
        need_lock = FALSE;

      } else {
        /* Release the lock, and use the "large session" list fallback. */
        if (shmcache_lock_shm(F_UNLCK) < 0) {
          tls_log("shmcache: error unlocking shmcache: %s", strerror(errno));
        }

        return shmcache_add_large_sess(cache, sess_id, sess_id_len, expires,
          sess, sess_len);
      }

    } else {
      tls_log("shmcache: unable to flush shm cache: error write-locking "
        "shmcache: %s", strerror(errno));

      /* Add this session to the "large session" list instead as a fallback. */
      return shmcache_add_large_sess(cache, sess_id, sess_id_len, expires,
        sess, sess_len);
    }
  }

  /* Hash the key, start looking for an open slot. */
  h = shmcache_hash(sess_id, sess_id_len);
  idx = h % shmcache_data->sd_listsz;

  if (need_lock) {
    if (shmcache_lock_shm(F_WRLCK) < 0) {
      tls_log("shmcache: unable to add session to shm cache: error "
        "write-locking shmcache: %s", strerror(errno));

      /* Add this session to the "large session" list instead as a fallback. */
      return shmcache_add_large_sess(cache, sess_id, sess_id_len, expires,
        sess, sess_len);
    }
  }

  i = idx;
  last = idx > 0 ? (idx - 1) : 0;

  do {
    struct shmcache_entry *entry;

    pr_signals_handle();

    /* Look for the first open slot (i.e. expires == 0). */
    entry = &(shmcache_data->sd_entries[i]);
    if (entry->expires == 0) {
      unsigned char *ptr;

      entry->expires = expires;
      entry->sess_id_len = sess_id_len;
      memcpy(entry->sess_id, sess_id, sess_id_len);
      entry->sess_datalen = sess_len;

      ptr = entry->sess_data;
      i2d_SSL_SESSION(sess, &ptr);

      shmcache_data->sd_listlen++;
      shmcache_data->nstored++;

      if (shmcache_data->next_expiring > 0) {
        if (expires < shmcache_data->next_expiring) {
          shmcache_data->next_expiring = expires;
        }

      } else {
        shmcache_data->next_expiring = expires;
      }

      found_slot = TRUE;
      break;
    }

    if (i < shmcache_data->sd_listsz) {
      i++;

    } else {
      i = 0;
    }

  } while (i != last);

  /* There is a race condition possible between the open slots check
   * above and the scan through the slots.  So if we didn't actually find
   * an open slot at this point, add it to the "large session" list.
   */
  if (!found_slot) {
    res = shmcache_add_large_sess(cache, sess_id, sess_id_len, expires, sess,
      sess_len);
  }

  if (need_lock) {
    if (shmcache_lock_shm(F_UNLCK) < 0) {
      tls_log("shmcache: error unlocking shmcache: %s", strerror(errno));
    }
  }

  return res;
}

static SSL_SESSION *shmcache_get(tls_sess_cache_t *cache,
    unsigned char *sess_id, unsigned int sess_id_len) {
  unsigned int h, idx;
  SSL_SESSION *sess = NULL;

  pr_trace_msg(trace_channel, 9, "getting session from shmcache cache %p",
    cache); 

  /* Look for the requested session in the "large session" list first. */
  if (shmcache_sess_list != NULL) {
    register unsigned int i;
    struct shmcache_large_entry *entries;

    entries = shmcache_sess_list->elts;
    for (i = 0; i < shmcache_sess_list->nelts; i++) {
      struct shmcache_large_entry *entry;

      entry = &(entries[i]);
      if (entry->expires > 0 &&
          entry->sess_id_len == sess_id_len &&
          memcmp(entry->sess_id, sess_id, entry->sess_id_len) == 0) {
        time_t now;

        now = time(NULL);
        if (entry->expires <= now) {
          TLS_D2I_SSL_SESSION_CONST unsigned char *ptr;

          ptr = entry->sess_data;
          sess = d2i_SSL_SESSION(NULL, &ptr, entry->sess_datalen);
          if (sess == NULL) {
            tls_log("shmcache: error retrieving session from cache: %s",
              shmcache_get_crypto_errors());

          } else {
            break;
          }
        }
      }
    }
  }

  if (sess) {
    return sess;
  }

  h = shmcache_hash(sess_id, sess_id_len);
  idx = h % shmcache_data->sd_listsz;

  if (shmcache_lock_shm(F_WRLCK) == 0) {
    register unsigned int i;
    unsigned int last;

    i = idx;
    last = idx > 0 ? (idx -1) : 0;

    do {
      struct shmcache_entry *entry;

      pr_signals_handle();

      entry = &(shmcache_data->sd_entries[i]);
      if (entry->expires > 0 &&
          entry->sess_id_len == sess_id_len &&
          memcmp(entry->sess_id, sess_id, entry->sess_id_len) == 0) {
        time_t now;

        /* Don't forget to update the stats. */
        now = time(NULL);

        if (entry->expires > now) {
          TLS_D2I_SSL_SESSION_CONST unsigned char *ptr;

          ptr = entry->sess_data;
          sess = d2i_SSL_SESSION(NULL, &ptr, entry->sess_datalen);
          if (sess != NULL) {
            shmcache_data->nhits++;

          } else {
            tls_log("shmcache: error retrieving session from cache: %s",
              shmcache_get_crypto_errors());
            shmcache_data->nerrors++;
          }
        }

        break;
      }

      if (i < shmcache_data->sd_listsz) {
        i++;

      } else {
        i = 0;
      }

    } while (i != last);

    if (sess == NULL) {
      shmcache_data->nmisses++;
      errno = ENOENT;
    }

    if (shmcache_lock_shm(F_UNLCK) < 0) {
      tls_log("shmcache: error unlocking shmcache: %s", strerror(errno));
    }

  } else {
    tls_log("shmcache: unable to retrieve session from cache: error "
      "write-locking shmcache: %s", strerror(errno));

    errno = EPERM;
  }

  return sess;
}

static int shmcache_delete(tls_sess_cache_t *cache,
    unsigned char *sess_id, unsigned int sess_id_len) {
  unsigned int h, idx;
  int res;

  pr_trace_msg(trace_channel, 9, "removing session from shmcache cache %p",
    cache);

  /* Look for the requested session in the "large session" list first. */
  if (shmcache_sess_list != NULL) {
    register unsigned int i;
    struct shmcache_large_entry *entries;

    entries = shmcache_sess_list->elts;
    for (i = 0; i < shmcache_sess_list->nelts; i++) {
      struct shmcache_large_entry *entry;

      entry = &(entries[i]);
      if (entry->sess_id_len == sess_id_len &&
          memcmp(entry->sess_id, sess_id, entry->sess_id_len) == 0) {

        pr_memscrub(entry->sess_data, entry->sess_datalen);
        entry->expires = 0;
        return 0;
      }
    }
  }

  h = shmcache_hash(sess_id, sess_id_len);
  idx = h % shmcache_data->sd_listsz;

  if (shmcache_lock_shm(F_WRLCK) == 0) {
    register unsigned int i;
    unsigned int last;

    i = idx;
    last = idx > 0 ? (idx - 1) : 0;

    do {
      struct shmcache_entry *entry;

      pr_signals_handle();

      entry = &(shmcache_data->sd_entries[i]);
      if (entry->sess_id_len == sess_id_len &&
          memcmp(entry->sess_id, sess_id, entry->sess_id_len) == 0) {
        time_t now;

        pr_memscrub(entry->sess_data, entry->sess_datalen);

        if (shmcache_data->sd_listlen > 0) {
          shmcache_data->sd_listlen--;
        }

        /* Don't forget to update the stats. */
        now = time(NULL);
        if (entry->expires > now) {
          shmcache_data->ndeleted++;

        } else {
          shmcache_data->nexpired++;
        }

        entry->expires = 0;
        break;
      }

      if (i < shmcache_data->sd_listsz) {
        i++;

      } else {
        i = 0;
      }

    } while (i != last);

    if (shmcache_lock_shm(F_UNLCK) < 0) {
      tls_log("shmcache: error unlocking shmcache: %s", strerror(errno));
    }

    res = 0;

  } else {
    tls_log("shmcache: unable to delete session from cache: error "
      "write-locking shmcache: %s", strerror(errno));

    errno = EPERM;
    res = -1;
  }

  return res;
}

static int shmcache_clear(tls_sess_cache_t *cache) {
  register unsigned int i;
  int res;

  pr_trace_msg(trace_channel, 9, "clearing shmcache cache %p", cache); 

  if (shmcache_shmid < 0) {
    errno = EINVAL;
    return -1;
  }

  if (shmcache_sess_list != NULL) {
    struct shmcache_large_entry *entries;
    
    entries = shmcache_sess_list->elts;
    for (i = 0; i < shmcache_sess_list->nelts; i++) {
      struct shmcache_large_entry *entry;

      entry = &(entries[i]);
      entry->expires = 0;
      pr_memscrub(entry->sess_data, entry->sess_datalen);
    }
  }

  if (shmcache_lock_shm(F_WRLCK) < 0) {
    tls_log("shmcache: unable to clear cache: error write-locking shmcache: %s",
      strerror(errno));
    return -1;
  }

  for (i = 0; i < shmcache_data->sd_listsz; i++) {
    struct shmcache_entry *entry;

    entry = &(shmcache_data->sd_entries[i]);

    entry->expires = 0;
    pr_memscrub(entry->sess_data, entry->sess_datalen);
  }

  res = shmcache_data->sd_listlen; 
  shmcache_data->sd_listlen = 0;

  if (shmcache_lock_shm(F_UNLCK) < 0) {
    tls_log("shmcache: error unlocking shmcache: %s", strerror(errno));
  }

  return res;
}

static int shmcache_remove(tls_sess_cache_t *cache) {
  int res;
  struct shmid_ds ds;
  const char *cache_file;

  if (shmcache_fh == NULL) {
    return 0;
  }

  pr_trace_msg(trace_channel, 9, "removing shmcache cache %p", cache); 

  cache_file = shmcache_fh->fh_path;
  (void) shmcache_close(cache);

  if (shmcache_shmid < 0) {
    errno = EINVAL;
    return -1;
  }

  pr_log_debug(DEBUG9, MOD_TLS_SHMCACHE_VERSION
    ": attempting to remove shm ID %d", shmcache_shmid);

  PRIVS_ROOT
  res = shmctl(shmcache_shmid, IPC_RMID, &ds);
  PRIVS_RELINQUISH

  if (res < 0) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": error removing shm ID %d: %s", shmcache_shmid, strerror(errno));

  } else {
    pr_log_debug(DEBUG9, MOD_TLS_SHMCACHE_VERSION
      ": removed shm ID %d", shmcache_shmid);
    shmcache_shmid = -1;
  }

  /* Don't forget to remove the on-disk file as well. */
  unlink(cache_file);

  return res;
}

static int shmcache_status(tls_sess_cache_t *cache,
    void (*statusf)(void *, const char *, ...), void *arg, int flags) {
  int res, xerrno = 0;
  struct shmid_ds ds;
  pool *tmp_pool;

  pr_trace_msg(trace_channel, 9, "checking shmcache cache %p", cache); 

  if (shmcache_lock_shm(F_RDLCK) < 0) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": error read-locking shmcache: %s", strerror(errno));
    return -1;
  }

  tmp_pool = make_sub_pool(permanent_pool);

  statusf(arg, "%s", "Shared memory (shm) SSL session cache provided by "
    MOD_TLS_SHMCACHE_VERSION);
  statusf(arg, "%s", "");
  statusf(arg, "Shared memory segment ID: %d", shmcache_shmid);

  PRIVS_ROOT
  res = shmctl(shmcache_shmid, IPC_STAT, &ds);
  xerrno = errno;
  PRIVS_RELINQUISH

  if (res == 0) {
    statusf(arg, "Shared memory segment size: %u bytes",
      (unsigned int) ds.shm_segsz);
    statusf(arg, "Shared memory cache created on: %s",
      pr_strtime(ds.shm_ctime));
    statusf(arg, "Shared memory attach count: %u",
      (unsigned int) ds.shm_nattch);

  } else {
    statusf(arg, "Unable to stat shared memory segment ID %d: %s",
      shmcache_shmid, strerror(xerrno));
  } 

  statusf(arg, "%s", "");
  statusf(arg, "Max session cache size: %u", shmcache_data->sd_listsz);
  statusf(arg, "Current session cache size: %u", shmcache_data->sd_listlen);
  statusf(arg, "%s", "");
  statusf(arg, "Cache lifetime hits: %u", shmcache_data->nhits);
  statusf(arg, "Cache lifetime misses: %u", shmcache_data->nmisses);
  statusf(arg, "%s", "");
  statusf(arg, "Cache lifetime sessions stored: %u", shmcache_data->nstored);
  statusf(arg, "Cache lifetime sessions deleted: %u", shmcache_data->ndeleted);
  statusf(arg, "Cache lifetime sessions expired: %u", shmcache_data->nexpired);
  statusf(arg, "%s", "");
  statusf(arg, "Cache lifetime errors handling sessions in cache: %u",
    shmcache_data->nerrors);
  statusf(arg, "Cache lifetime sessions exceeding max entry size: %u",
    shmcache_data->nexceeded);
  if (shmcache_data->nexceeded > 0) {
    statusf(arg, "  Largest session exceeding max entry size: %u",
      shmcache_data->exceeded_maxsz);
  }

  if (flags & TLS_SESS_CACHE_STATUS_FL_SHOW_SESSIONS) {
    register unsigned int i;

    statusf(arg, "%s", "");
    statusf(arg, "%s", "Cached sessions:");

    if (shmcache_data->sd_listlen == 0) {
      statusf(arg, "%s", "  (none)");
    }

    /* We _could_ use SSL_SESSION_print(), which is what the sess_id
     * command-line tool does.  The problem is that SSL_SESSION_print() shows
     * too much (particularly, it shows the master secret).  And
     * SSL_SESSION_print() does not support a flags argument to use for
     * specifying which bits of the session we want to print.
     *
     * Instead, we get to do the more dangerous (compatibility-wise) approach
     * of rolling our own printing function.
     */

    for (i = 0; i < shmcache_data->sd_listsz; i++) {
      struct shmcache_entry *entry;

      pr_signals_handle();

      entry = &(shmcache_data->sd_entries[i]);
      if (entry->expires > 0) {
        SSL_SESSION *sess;
        TLS_D2I_SSL_SESSION_CONST unsigned char *ptr;
        time_t ts;

        ptr = entry->sess_data;
        sess = d2i_SSL_SESSION(NULL, &ptr, entry->sess_datalen); 
        if (sess == NULL) {
          pr_log_pri(PR_LOG_INFO, MOD_TLS_SHMCACHE_VERSION
            ": error retrieving session from cache: %s",
            shmcache_get_crypto_errors());
          continue;
        }

        statusf(arg, "%s", "  -----BEGIN SSL SESSION PARAMETERS-----");

        /* XXX Directly accessing these fields cannot be a Good Thing. */
        if (sess->session_id_length > 0) {
          register unsigned int j;
          char *sess_id_str;

          sess_id_str = pcalloc(tmp_pool, (sess->session_id_length * 2) + 1);

          for (j = 0; j < sess->session_id_length; j++) {
            sprintf((char *) &(sess_id_str[j*2]), "%02X", sess->session_id[j]);
          }

          statusf(arg, "    Session ID: %s", sess_id_str);
        }

        if (sess->sid_ctx_length > 0) {
          register unsigned int j;
          char *sid_ctx_str;

          sid_ctx_str = pcalloc(tmp_pool, (sess->sid_ctx_length * 2) + 1);

          for (j = 0; j < sess->sid_ctx_length; j++) {
            sprintf((char *) &(sid_ctx_str[j*2]), "%02X", sess->sid_ctx[j]);
          }

          statusf(arg, "    Session ID Context: %s", sid_ctx_str);
        }

        switch (sess->ssl_version) {
          case SSL3_VERSION:
            statusf(arg, "    Protocol: %s", "SSLv3");
            break;

          case TLS1_VERSION:
            statusf(arg, "    Protocol: %s", "TLSv1");
            break;

          default:
            statusf(arg, "    Protocol: %s", "unknown");
        }

        ts = SSL_SESSION_get_time(sess);
        statusf(arg, "    Started: %s", pr_strtime(ts));
        ts = entry->expires;
        statusf(arg, "    Expires: %s (%u secs)", pr_strtime(ts),
          SSL_SESSION_get_timeout(sess));

        SSL_SESSION_free(sess);
        statusf(arg, "%s", "  -----END SSL SESSION PARAMETERS-----");
        statusf(arg, "%s", "");
      }
    }
  }

  if (shmcache_lock_shm(F_UNLCK) < 0) {
    pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
      ": error unlocking shmcache: %s", strerror(errno));
  }

  destroy_pool(tmp_pool);
  return 0;
}

/* Event Handlers
 */

/* Daemon PID */
extern pid_t mpid;

static void shmcache_shutdown_ev(const void *event_data, void *user_data) {
  if (mpid == getpid() &&
      ServerType == SERVER_STANDALONE) {

    /* Remove external session caches on shutdown; the security policy/config
     * may have changed, e.g. becoming more strict, and allow clients to
     * resumed cached sessions from a more relaxed security config is not a 
     * Good Thing at all.
     */
    shmcache_remove(NULL);
  }
}

#if defined(PR_SHARED_MODULE)
static void shmcache_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_tls_shmcache.c", (const char *) event_data) == 0) {
    pr_event_unregister(&tls_shmcache_module, NULL, NULL);
    tls_sess_cache_unregister("shm");

    /* This clears our cache by detaching and destroying the shared memory
     * segment.
     */
    shmcache_remove(NULL);
  }
}
#endif /* !PR_SHARED_MODULE */

static void shmcache_restart_ev(const void *event_data, void *user_data) {
  /* Clear external session caches on shutdown; the security policy/config
   * may have changed, e.g. becoming more strict, and allow clients to
   * resumed cached sessions from a more relaxed security config is not a 
   * Good Thing at all.
   */
  shmcache_clear(NULL);
}

/* Initialization functions
 */

static int tls_shmcache_init(void) {
  pr_event_register(&tls_shmcache_module, "core.exit", shmcache_shutdown_ev,
    NULL);
#if defined(PR_SHARED_MODULE)
  pr_event_register(&tls_shmcache_module, "core.module-unload",
    shmcache_mod_unload_ev, NULL);
#endif /* !PR_SHARED_MODULE */
  pr_event_register(&tls_shmcache_module, "core.restart", shmcache_restart_ev,
    NULL);

  /* Prepare our cache handler. */
  memset(&shmcache, 0, sizeof(shmcache));
  shmcache.open = shmcache_open;
  shmcache.close = shmcache_close;
  shmcache.add = shmcache_add;
  shmcache.get = shmcache_get;
  shmcache.delete = shmcache_delete;
  shmcache.clear = shmcache_clear;
  shmcache.remove = shmcache_remove;
  shmcache.status = shmcache_status;

#ifdef SSL_SESS_CACHE_NO_INTERNAL
  /* Take a chance, and inform OpenSSL that it does not need to use its own
   * internal session caching; using the external session cache (i.e. us)
   * will be enough.
   *
   * Using NO_INTERNAL is equivalent to NO_INTERNAL_LOOKUP|NO_INTERNAL_STORE.
   */
  shmcache.cache_mode = SSL_SESS_CACHE_NO_INTERNAL;
#endif

  /* Register ourselves with mod_tls. */
  if (tls_sess_cache_register("shm", &shmcache) < 0) {
    pr_log_pri(PR_LOG_NOTICE, MOD_TLS_SHMCACHE_VERSION
      ": notice: error registering 'shm' SSL session cache: %s",
      strerror(errno));
    return -1;
  }

  return 0;
}

static int tls_shmcache_sess_init(void) {
  pr_event_unregister(&tls_shmcache_module, "core.exit", shmcache_shutdown_ev);

#ifdef HAVE_MLOCK
  if (shmcache_data != NULL) {
    int res, xerrno = 0;

    /* Make sure the memory is pinned in RAM where possible.
     *
     * Since this is a session process, we do not need to worry about
     * explicitly unlocking the locked memory; that will happen automatically
     * when the session process exits.
     */
    PRIVS_ROOT
    res = mlock(shmcache_data, shmcache_datasz);
    xerrno = errno;
    PRIVS_RELINQUISH

    if (res < 0) {
      pr_log_debug(DEBUG1, MOD_TLS_SHMCACHE_VERSION
        ": error locking 'shm' cache (%lu bytes) into memory: %s",
        (unsigned long) shmcache_datasz, strerror(xerrno));

    } else {
      pr_log_debug(DEBUG5, MOD_TLS_SHMCACHE_VERSION
        ": 'shm' cache locked into memory (%lu bytes)",
        (unsigned long) shmcache_datasz);
    }
  }
#endif

  return 0;
}

/* Module API tables
 */

module tls_shmcache_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "tls_shmcache",

  /* Module configuration handler table */
  NULL,

  /* Module command handler table */
  NULL,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  tls_shmcache_init,

  /* Session initialization function */
  tls_shmcache_sess_init,

  /* Module version */
  MOD_TLS_SHMCACHE_VERSION
};

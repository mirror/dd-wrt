/*
 * ProFTPD: mod_ban -- a module implementing ban lists using the Controls API
 *
 * Copyright (c) 2004-2010 TJ Saunders
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
 * This is mod_ban, contrib software for proftpd 1.2.x/1.3.x.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 *
 * $Id: mod_ban.c,v 1.37 2010/02/22 17:01:30 castaglia Exp $
 */

#include "conf.h"
#include "privs.h"
#include "mod_ctrls.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#define MOD_BAN_VERSION			"mod_ban/0.5.5"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030101
# error "ProFTPD 1.3.1rc1 or later required"
#endif

#ifndef PR_USE_CTRLS
# error "Controls support required (use --enable-ctrls)"
#endif

#define BAN_PROJ_ID		76
#define BAN_TIMER_INTERVAL	60

#ifndef HAVE_FLOCK
# define LOCK_SH        1
# define LOCK_EX        2
# define LOCK_UN        8
# define LOCK_NB        4
#endif /* HAVE_FLOCK */

/* Maximum length of user name/reason/event strings.
 */
#ifndef BAN_STRING_MAXSZ
# define BAN_STRING_MAXSZ	128
#endif

#ifndef BAN_LIST_MAXSZ
# define BAN_LIST_MAXSZ		512
#endif

#ifndef BAN_EVENT_LIST_MAXSZ
# define BAN_EVENT_LIST_MAXSZ	512
#endif

/* From src/main.c */
extern pid_t mpid;

module ban_module;
static ctrls_acttab_t ban_acttab[];

/* Pool for this module's use */
static pool *ban_pool = NULL;

struct ban_entry {
  unsigned int be_type;
  char be_name[BAN_STRING_MAXSZ];
  char be_reason[BAN_STRING_MAXSZ];
  char be_mesg[BAN_STRING_MAXSZ];
  time_t be_expires;
  unsigned int be_sid;
};

#define BAN_TYPE_CLASS		1
#define BAN_TYPE_HOST		2
#define BAN_TYPE_USER		3

struct ban_list {
  struct ban_entry bl_entries[BAN_LIST_MAXSZ];
  unsigned int bl_listlen;
  unsigned int bl_next_slot;
};

struct ban_event_entry {
  unsigned int bee_type;
  char bee_src[BAN_STRING_MAXSZ];
  unsigned int bee_count_max;
  unsigned int bee_count_curr;
  time_t bee_start;
  time_t bee_window;
  time_t bee_expires;
  char bee_mesg[BAN_STRING_MAXSZ];
  unsigned int bee_sid;
};

#define BAN_EV_TYPE_ANON_REJECT_PASSWORDS	1
#define BAN_EV_TYPE_MAX_CLIENTS_PER_CLASS	2
#define BAN_EV_TYPE_MAX_CLIENTS_PER_HOST	3
#define BAN_EV_TYPE_MAX_CLIENTS_PER_USER	4
#define BAN_EV_TYPE_MAX_HOSTS_PER_USER		5
#define BAN_EV_TYPE_MAX_LOGIN_ATTEMPTS		6
#define BAN_EV_TYPE_TIMEOUT_IDLE		7
#define BAN_EV_TYPE_TIMEOUT_NO_TRANSFER		8
#define BAN_EV_TYPE_MAX_CONN_PER_HOST		9
#define BAN_EV_TYPE_CLIENT_CONNECT_RATE		10
#define BAN_EV_TYPE_TIMEOUT_LOGIN		11

struct ban_event_list {
  struct ban_event_entry bel_entries[BAN_EVENT_LIST_MAXSZ];
  unsigned int bel_listlen;
  unsigned int bel_next_slot;
};

struct ban_data {
  struct ban_list bans;
  struct ban_event_list events;
};

static struct ban_data *ban_lists = NULL;
static int ban_engine = -1;
static int ban_logfd = -1;
static char *ban_log = NULL;
static char *ban_mesg = NULL;
static int ban_shmid = -1;
static char *ban_table = NULL;
static pr_fh_t *ban_tabfh = NULL;
static int ban_timerno = -1;

static int ban_lock_shm(int);

static void ban_anonrejectpasswords_ev(const void *, void *);
static void ban_clientconnectrate_ev(const void *, void *);
static void ban_maxclientsperclass_ev(const void *, void *);
static void ban_maxclientsperhost_ev(const void *, void *);
static void ban_maxclientsperuser_ev(const void *, void *);
static void ban_maxconnperhost_ev(const void *, void *);
static void ban_maxhostsperuser_ev(const void *, void *);
static void ban_maxloginattempts_ev(const void *, void *);
static void ban_timeoutidle_ev(const void *, void *);
static void ban_timeoutlogin_ev(const void *, void *);
static void ban_timeoutnoxfer_ev(const void *, void *);

static struct ban_data *ban_get_shm(pr_fh_t *tabfh) {
  int shmid;
  int shm_existed = FALSE;
  struct ban_data *data = NULL;
  key_t key;

  /* If we already have a shmid, no need to do anything. */
  if (ban_shmid >= 0) {
    errno = EEXIST;
    return NULL;
  }

  /* Get a key for this path. */
  key = ftok(tabfh->fh_path, BAN_PROJ_ID);
  if (key == (key_t) -1) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "unable to get key for '%s': %s", tabfh->fh_path, strerror(errno));
    return NULL;
  }

  /* Try first using IPC_CREAT|IPC_EXCL, to check if there is an existing
   * shm for this key.  If there is, try again, using a flag of zero.
   */

  shmid = shmget(key, sizeof(struct ban_data), IPC_CREAT|IPC_EXCL|0666);
  if (shmid < 0) {

    if (errno == EEXIST) {
      shm_existed = TRUE;

      shmid = shmget(key, 0, 0);

    } else {
      return NULL;
    }
  }

  /* Attach to the shm. */
  data = (struct ban_data *) shmat(shmid, NULL, 0);
  if (data == NULL) {
    int xerrno = errno;
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "unable to attach to shm: %s", strerror(xerrno));

    errno = xerrno;
    return NULL;
  }

  if (!shm_existed) {

    /* Make sure the memory is initialized. */
    if (ban_lock_shm(LOCK_EX) < 0) {
      (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
        "error write-locking shm: %s", strerror(errno));
    }

    memset(data, '\0', sizeof(struct ban_data));

    if (ban_lock_shm(LOCK_UN) < 0) {
      (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
        "error unlocking shm: %s", strerror(errno));
    }
  }

  ban_shmid = shmid;
  (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
    "obtained shmid %d for BanTable '%s'", ban_shmid, tabfh->fh_path);

  return data;
}

static int ban_lock_shm(int flags) {
  static unsigned int ban_nlocks = 0;

#ifndef HAVE_FLOCK
  int lock_flag;
  struct flock lock;
#endif /* HAVE_FLOCK */

  if (ban_nlocks &&
      ((flags & LOCK_SH) || (flags & LOCK_EX))) {
    ban_nlocks++;
    return 0;
  }

  if (ban_nlocks == 0 &&
      (flags & LOCK_UN)) {
    return 0;
  }

#ifdef HAVE_FLOCK
  while (flock(ban_tabfh->fh_fd, flags) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return -1;
  }

  if ((flags & LOCK_SH) ||
      (flags & LOCK_EX)) {
    ban_nlocks++;

  } else if (flags & LOCK_UN) {
    ban_nlocks--;
  }

  return 0;
#else
  lock_flag = F_SETLKW;

  lock.l_whence = 0;
  lock.l_start = lock.l_len = 0;

  if (flags & LOCK_SH) {
    lock.l_type = F_RDLCK;

  } else if (flags & LOCK_EX) {
    lock.l_type = F_WRLCK;

  } else if (flags & LOCK_UN) {
    lock.l_type= F_UNLCK;

  } else {
    errno = EINVAL;
    return -1;
  }

  if (flags & LOCK_NB)
    lock_flag = F_SETLK;

  while (fcntl(ban_tabfh->fh_fd, lock_flag, &lock) < 0) {
    if (errno == EINTR) {
      pr_signals_handle();
      continue;
    }

    return -1;
  }

  if ((flags & LOCK_SH) ||
      (flags & LOCK_EX)) {
    ban_nlocks++;

  } else if (flags & LOCK_UN) {
    ban_nlocks--;
  }

  return 0;
#endif /* HAVE_FLOCK */
}

static int ban_disconnect_class(const char *class) {
  pr_scoreboard_entry_t *score = NULL;
  unsigned char kicked_class = FALSE;
  unsigned int nclients = 0;
  pid_t session_pid;

  if (!class) {
    errno = EINVAL;
    return -1;
  }

  /* Iterate through the scoreboard, and send a SIGTERM to each
   * PID whose class matches the given class.  Make sure that we exclude
   * our own PID from that list; our own termination is handled elsewhere.
   */

  if (pr_rewind_scoreboard() < 0 &&
      errno != EINVAL) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error rewinding scoreboard: %s", strerror(errno));
  }

  session_pid = getpid();

  while ((score = pr_scoreboard_entry_read()) != NULL) {
    pr_signals_handle();

    if (score->sce_pid != session_pid &&
        strcmp(class, score->sce_class) == 0) {
      int res = 0;

      PRIVS_ROOT
      res = pr_scoreboard_entry_kill(score, SIGTERM);
      PRIVS_RELINQUISH

      if (res == 0) {
        kicked_class = TRUE;
        nclients++;

      } else {
        (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
          "error disconnecting class '%s' [process %lu]: %s", class,
            (unsigned long) score->sce_pid, strerror(errno));
      }
    }
  }

  if (pr_restore_scoreboard() < 0 &&
      errno != EINVAL) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error restoring scoreboard: %s", strerror(errno));
  }

  if (kicked_class) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "disconnected %u %s from class '%s'", nclients,
      nclients != 1 ? "clients" : "client", class);
    return 0;
  }

  errno = ENOENT;
  return -1;
}

static int ban_disconnect_host(const char *host) {
  pr_scoreboard_entry_t *score = NULL;
  unsigned char kicked_host = FALSE;
  unsigned int nclients = 0;
  pid_t session_pid;

  if (!host) {
    errno = EINVAL;
    return -1;
  }

  /* Iterate through the scoreboard, and send a SIGTERM to each
   * PID whose address matches the given host.  Make sure that we exclude
   * our own PID from that list; our own termination is handled elsewhere.
   */

  if (pr_rewind_scoreboard() < 0 &&
      errno != EINVAL) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error rewinding scoreboard: %s", strerror(errno));
  }

  session_pid = getpid();

  while ((score = pr_scoreboard_entry_read()) != NULL) {
    pr_signals_handle();

    if (score->sce_pid != session_pid &&
        strcmp(host, score->sce_client_addr) == 0) {
      int res = 0;

      PRIVS_ROOT
      res = pr_scoreboard_entry_kill(score, SIGTERM);
      PRIVS_RELINQUISH

      if (res == 0) {
        kicked_host = TRUE;
        nclients++;

      } else {
        (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
          "error disconnecting host '%s' [process %lu]: %s", host,
            (unsigned long) score->sce_pid, strerror(errno));
      }
    }
  }

  if (pr_restore_scoreboard() < 0 &&
      errno != EINVAL) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error restoring scoreboard: %s", strerror(errno));
  }

  if (kicked_host) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "disconnected %u %s from host '%s'", nclients,
      nclients != 1 ? "clients" : "client", host);
    return 0;
  }

  errno = ENOENT;
  return -1;
}

static int ban_disconnect_user(const char *user) {
  pr_scoreboard_entry_t *score = NULL;
  unsigned char kicked_user = FALSE;
  unsigned int nclients = 0;
  pid_t session_pid;

  if (!user) {
    errno = EINVAL;
    return -1;
  }

  /* Iterate through the scoreboard, and send a SIGTERM to each
   * PID whose name matches the given user name.  Make sure that we exclude
   * our own PID from that list; our own termination is handled elsewhere.
   */

  if (pr_rewind_scoreboard() < 0 &&
      errno != EINVAL) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error rewinding scoreboard: %s", strerror(errno));
  }

  session_pid = getpid();

  while ((score = pr_scoreboard_entry_read()) != NULL) {
    pr_signals_handle();

    if (score->sce_pid != session_pid &&
        strcmp(user, score->sce_user) == 0) {
      int res = 0;

      PRIVS_ROOT
      res = pr_scoreboard_entry_kill(score, SIGTERM);
      PRIVS_RELINQUISH

      if (res == 0) {
        kicked_user = TRUE;
        nclients++;

      } else {
        (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
          "error disconnecting user '%s' [process %lu]: %s", user,
            (unsigned long) score->sce_pid, strerror(errno));
      }
    }
  }

  if (pr_restore_scoreboard() < 0 &&
      errno != EINVAL) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error restoring scoreboard: %s", strerror(errno));
  }

  if (kicked_user) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "disconnected %u %s from user '%s'", nclients,
      nclients != 1 ? "clients" : "client", user);
    return 0;
  }

  errno = ENOENT;
  return -1;
}

/* Parse a string formatted as "hh:mm:ss" into a time_t. */
static time_t ban_parse_timestr(const char *str) {
  unsigned int hours, mins, secs;

  if (sscanf(str, "%2u:%2u:%2u", &hours, &mins, &secs) != 3) {
    errno = EINVAL;
    return -1;
  }

  return (hours * 60 * 60) + (mins * 60) + secs;
}

/* Send a configured rule-specific message (from the BanOnEvent configuration)
 * or, if there isn't a rule-specific message, the BanMessage to the client.
 */
static void ban_send_mesg(pool *p, const char *user, const char *rule_mesg) {
  char *mesg = NULL;

  if (rule_mesg) {
    mesg = pstrdup(p, rule_mesg);

  } else if (ban_mesg) {
    mesg = pstrdup(p, ban_mesg);
  }

  if (mesg) {
    mesg = pstrdup(p, mesg);

    if (strstr(mesg, "%c")) {
      const char *class;

      class = session.class ? session.class->cls_name : "(none)";
      mesg = sreplace(p, mesg, "%c", class, NULL);
    }

    if (strstr(mesg, "%a")) {
      const char *remote_ip;

      remote_ip = pr_netaddr_get_ipstr(session.c->remote_addr);
      mesg = sreplace(p, mesg, "%a", remote_ip, NULL);
    }

    if (strstr(mesg, "%u"))
      mesg = sreplace(p, mesg, "%u", user, NULL);

    pr_response_send_async(R_530, "%s", mesg);
  }

  return;
}

/* List manipulation routines
 */

/* Add an entry to the ban list. */
static int ban_list_add(unsigned int type, unsigned int sid,
    const char *name, const char *reason, time_t lasts, const char *rule_mesg) {
  unsigned int old_slot;
  int seen = FALSE;

  if (!ban_lists) {
    errno = EPERM;
    return -1;
  }

  old_slot = ban_lists->bans.bl_next_slot;

  /* Find an open slot in the list for this new entry. */
  while (TRUE) {
    struct ban_entry *be;

    pr_signals_handle();

    if (ban_lists->bans.bl_next_slot == BAN_LIST_MAXSZ)
      ban_lists->bans.bl_next_slot = 0;

    be = &(ban_lists->bans.bl_entries[ban_lists->bans.bl_next_slot]);
    if (be->be_type == 0) {
      be->be_type = type;
      be->be_sid = sid;

      sstrncpy(be->be_name, name, sizeof(be->be_name));
      sstrncpy(be->be_reason, reason, sizeof(be->be_reason));
      be->be_expires = lasts ? time(NULL) + lasts : 0;

      memset(be->be_mesg, '\0', sizeof(be->be_mesg));
      if (rule_mesg) {
        sstrncpy(be->be_mesg, rule_mesg, sizeof(be->be_mesg));
      }

      switch (type) {
        case BAN_TYPE_USER:
          pr_event_generate("mod_ban.ban-user",
            ban_lists->bans.bl_entries[ban_lists->bans.bl_next_slot].be_name);
          ban_disconnect_user(name);
          break;

        case BAN_TYPE_HOST:
          pr_event_generate("mod_ban.ban-host",
            ban_lists->bans.bl_entries[ban_lists->bans.bl_next_slot].be_name);
          ban_disconnect_host(name);
          break;

        case BAN_TYPE_CLASS:
          pr_event_generate("mod_ban.ban-class",
            ban_lists->bans.bl_entries[ban_lists->bans.bl_next_slot].be_name);
          ban_disconnect_class(name);
          break;
      }

      ban_lists->bans.bl_next_slot++;
      ban_lists->bans.bl_listlen++;
      break;
      
    } else {
      pr_signals_handle(); 

      if (ban_lists->bans.bl_next_slot == old_slot &&
          seen == TRUE) {

        /* This happens when we've scanned the entire list, found no
         * empty slot, and have returned back to the slot at which we
         * started.
         */
        (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
          "maximum number of ban slots (%u) already in use", BAN_LIST_MAXSZ);

        errno = ENOSPC;
        return -1;
      }

      ban_lists->bans.bl_next_slot++;
      seen = TRUE;
    }
  }

  return 0;
}

/* Check if a ban of the specified type, for the given server ID and name,
 * is present in the ban list.
 *
 * If the caller provides a `mesg' pointer, then if a ban exists, that
 * pointer will point to any custom client-displayable message.
 */
static int ban_list_exists(unsigned int type, unsigned int sid,
    const char *name, char **mesg) {

  if (!ban_lists) {
    errno = EPERM;
    return -1;
  }

  if (ban_lists->bans.bl_listlen) {
    register unsigned int i = 0;

    for (i = 0; i < BAN_LIST_MAXSZ; i++) {
      pr_signals_handle();

      if (ban_lists->bans.bl_entries[i].be_type == type &&
          (ban_lists->bans.bl_entries[i].be_sid == 0 ||
           ban_lists->bans.bl_entries[i].be_sid == sid) &&
          strcmp(ban_lists->bans.bl_entries[i].be_name, name) == 0) {

        if (mesg != NULL &&
            strlen(ban_lists->bans.bl_entries[i].be_mesg) > 0) {
          *mesg = ban_lists->bans.bl_entries[i].be_mesg;
        }

        return 0;
      }
    }
  }

  errno = ENOENT;
  return -1;
}

static int ban_list_remove(unsigned int type, const char *name) {

  if (!ban_lists) {
    errno = EPERM;
    return -1;
  }

  if (ban_lists->bans.bl_listlen) {
    register unsigned int i = 0;

    for (i = 0; i < BAN_LIST_MAXSZ; i++) {
      pr_signals_handle();

      if (ban_lists->bans.bl_entries[i].be_type == type &&
          (name ? strcmp(ban_lists->bans.bl_entries[i].be_name, name) == 0 :
           TRUE)) {

        switch (type) {
          case BAN_TYPE_USER:
            pr_event_generate("mod_ban.permit-user",
              ban_lists->bans.bl_entries[i].be_name);
            break;

          case BAN_TYPE_HOST:
            pr_event_generate("mod_ban.permit-host",
              ban_lists->bans.bl_entries[i].be_name);
            break;

          case BAN_TYPE_CLASS:
            pr_event_generate("mod_ban.permit-class",
              ban_lists->bans.bl_entries[i].be_name);
            break;
        }

        memset(&(ban_lists->bans.bl_entries[i]), '\0',
          sizeof(struct ban_entry)); 

        ban_lists->bans.bl_listlen--;

        if (name)
          return 0;
      }
    }
  }

  if (!name)
    return 0;

  errno = ENOENT;
  return -1;
}

/* Remove all expired bans from the list. */
static void ban_list_expire(void) {
  time_t now = time(NULL);
  register unsigned int i = 0;

  if (!ban_lists || ban_lists->bans.bl_listlen == 0)
    return;

  for (i = 0; i < BAN_LIST_MAXSZ; i++) {
    pr_signals_handle();

    if (ban_lists->bans.bl_entries[i].be_type &&
        ban_lists->bans.bl_entries[i].be_expires &&
        !(ban_lists->bans.bl_entries[i].be_expires > now)) {

      (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
        "ban for %s '%s' has expired (%lu seconds ago)",
        ban_lists->bans.bl_entries[i].be_type == BAN_TYPE_USER ? "user" : 
        ban_lists->bans.bl_entries[i].be_type == BAN_TYPE_HOST ? "host" :
          "class",
        ban_lists->bans.bl_entries[i].be_name,
        (unsigned long) now - ban_lists->bans.bl_entries[i].be_expires);

      ban_list_remove(ban_lists->bans.bl_entries[i].be_type,
        ban_lists->bans.bl_entries[i].be_name);
    }
  }
}

static const char *ban_event_entry_typestr(unsigned int type) {
  switch (type) {
    case BAN_EV_TYPE_ANON_REJECT_PASSWORDS:
      return "AnonRejectPasswords";

    case BAN_EV_TYPE_MAX_CLIENTS_PER_CLASS:
      return "MaxClientsPerClass";

    case BAN_EV_TYPE_MAX_CLIENTS_PER_HOST:
      return "MaxClientsPerHost";

    case BAN_EV_TYPE_MAX_CLIENTS_PER_USER:
      return "MaxClientsPerUser";

    case BAN_EV_TYPE_MAX_HOSTS_PER_USER:
      return "MaxHostsPerUser";

    case BAN_EV_TYPE_MAX_LOGIN_ATTEMPTS:
      return "MaxLoginAttempts";

    case BAN_EV_TYPE_TIMEOUT_IDLE:
      return "TimeoutIdle";

    case BAN_EV_TYPE_TIMEOUT_LOGIN:
      return "TimeoutLogin";

    case BAN_EV_TYPE_TIMEOUT_NO_TRANSFER:
      return "TimeoutNoTransfer";

    case BAN_EV_TYPE_MAX_CONN_PER_HOST:
      return "MaxConnectionsPerHost";

    case BAN_EV_TYPE_CLIENT_CONNECT_RATE:
      return "ClientConnectRate";
  }

  return NULL;
}

/* Add an entry to the ban event list. */
static int ban_event_list_add(unsigned int type, unsigned int sid,
    const char *src, unsigned int max, time_t window, time_t expires) {
  unsigned int old_slot;
  int seen = FALSE;

  if (!ban_lists) {
    errno = EPERM;
    return -1;
  }

  old_slot = ban_lists->events.bel_next_slot;

  /* Find an open slot in the list for this new entry. */
  while (TRUE) {
    struct ban_event_entry *bee;

    pr_signals_handle();

    if (ban_lists->events.bel_next_slot == BAN_EVENT_LIST_MAXSZ)
      ban_lists->events.bel_next_slot = 0;

    bee = &(ban_lists->events.bel_entries[ban_lists->events.bel_next_slot]);

    if (bee->bee_type == 0) {
      bee->bee_type = type;
      bee->bee_sid = sid;

      sstrncpy(bee->bee_src, src, sizeof(bee->bee_src));
      bee->bee_count_max = max;
      time(&bee->bee_start);
      bee->bee_window = window;
      bee->bee_expires = expires;

      ban_lists->events.bel_next_slot++;
      ban_lists->events.bel_listlen++;
      break;

    } else {
      pr_signals_handle();

      if (ban_lists->events.bel_next_slot == old_slot &&
          seen == TRUE) {

        /* This happens when we've scanned the entire list, found no
         * empty slot, and have returned back to the slot at which we
         * started.
         */
        (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
          "maximum number of ban event slots (%u) already in use",
          BAN_EVENT_LIST_MAXSZ);

        errno = ENOSPC;
        return -1;
      }

      ban_lists->events.bel_next_slot++;
      seen = TRUE;
    }
  }

  return 0;
}

static struct ban_event_entry *ban_event_list_get(unsigned int type,
    unsigned int sid, const char *src) {

  if (!ban_lists)
    return NULL;

  if (ban_lists->events.bel_listlen) {
    register unsigned int i = 0;

    for (i = 0; i < BAN_EVENT_LIST_MAXSZ; i++) {
      pr_signals_handle();

      if (ban_lists->events.bel_entries[i].bee_type == type &&
          ban_lists->events.bel_entries[i].bee_sid == sid &&
          strcmp(ban_lists->events.bel_entries[i].bee_src, src) == 0) {
        return &(ban_lists->events.bel_entries[i]);
      }
    }
  }

  return NULL;
}

static int ban_event_list_remove(unsigned int type, unsigned int sid,
    const char *src) {

  if (!ban_lists) {
    errno = EPERM;
    return -1;
  }

  if (ban_lists->events.bel_listlen) {
    register unsigned int i = 0;

    for (i = 0; i < BAN_EVENT_LIST_MAXSZ; i++) {
      pr_signals_handle();

      if (ban_lists->events.bel_entries[i].bee_type == type &&
          ban_lists->events.bel_entries[i].bee_sid == sid &&
          (src ? strcmp(ban_lists->events.bel_entries[i].bee_src, src) == 0 :
           TRUE)) {
        memset(&(ban_lists->events.bel_entries[i]), 0,
          sizeof(struct ban_event_entry));

        ban_lists->events.bel_listlen--;

        if (src)
          return 0;
      }
    }
  }

  if (!src)
    return 0;

  errno = ENOENT;
  return -1;
}

static void ban_event_list_expire(void) {
  register unsigned int i = 0;
  time_t now = time(NULL);

  if (!ban_lists ||
      ban_lists->events.bel_listlen == 0)
    return;

  for (i = 0; i < BAN_EVENT_LIST_MAXSZ; i++) {
    time_t bee_end = ban_lists->events.bel_entries[i].bee_start +
      ban_lists->events.bel_entries[i].bee_window;

    pr_signals_handle();

    if (ban_lists->events.bel_entries[i].bee_type &&
        ban_lists->events.bel_entries[i].bee_expires &&
        !(bee_end > now)) {
      (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
        "ban event %s entry '%s' has expired (%lu seconds ago)",
        ban_event_entry_typestr(ban_lists->events.bel_entries[i].bee_type),
        ban_lists->events.bel_entries[i].bee_src,
        (unsigned long) now - bee_end);

      ban_event_list_remove(ban_lists->events.bel_entries[i].bee_type,
        ban_lists->events.bel_entries[i].bee_sid,
        ban_lists->events.bel_entries[i].bee_src);
    }
  }
}

/* Controls handlers
 */

static int ban_handle_ban(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i = 0;

  /* Check the ban ACL */
  if (!pr_ctrls_check_acl(ctrl, ban_acttab, "ban")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (!reqargv) {
    pr_ctrls_add_response(ctrl, "missing arguments");
    return -1;
  }

  if (ban_engine != TRUE) {
    pr_ctrls_add_response(ctrl, MOD_BAN_VERSION " not enabled");
    return -1;
  }

  /* Make sure the lists are up-to-date. */
  ban_list_expire();
  ban_event_list_expire();

  /* Handle 'ban user' requests */
  if (strcmp(reqargv[0], "user") == 0) {

    if (reqargc < 2) {
      pr_ctrls_add_response(ctrl, "missing arguments");
      return -1;
    }

    if (ban_lock_shm(LOCK_EX) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    /* Add each given user name to the list */
    for (i = 1; i < reqargc; i++) {
     
      /* Check for duplicates. */
      if (ban_list_exists(BAN_TYPE_USER, 0, reqargv[i], NULL) < 0) {

        if (ban_lists->bans.bl_listlen < BAN_LIST_MAXSZ) {
          const char *reason = pstrcat(ctrl->ctrls_tmp_pool, "requested by '",
            ctrl->ctrls_cl->cl_user, "' on ", pr_strtime(time(NULL)), NULL);

          ban_list_add(BAN_TYPE_USER, 0, reqargv[i], reason, 0, NULL);
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "added '%s' to banned users list", reqargv[i]);
          pr_ctrls_add_response(ctrl, "user %s banned", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "maximum list size reached, unable to "
            "ban user '%s'", reqargv[i]);

      } else
        pr_ctrls_add_response(ctrl, "user %s already banned", reqargv[i]);
    }

    ban_lock_shm(LOCK_UN);

  /* Handle 'ban host' requests */
  } else if (strcmp(reqargv[0], "host") == 0) {

    if (reqargc < 2) {
      pr_ctrls_add_response(ctrl, "missing arguments");
      return -1;
    }

    if (ban_lock_shm(LOCK_EX) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    /* Add each site to the list */
    for (i = 1; i < reqargc; i++) {

      /* XXX handle multiple addresses */
      pr_netaddr_t *site = pr_netaddr_get_addr(ctrl->ctrls_tmp_pool,
        reqargv[i], NULL);

      if (!site) {
        pr_ctrls_add_response(ctrl, "ban: unknown host '%s'", reqargv[i]);
        continue;
      }
 
      /* Check for duplicates. */
      if (ban_list_exists(BAN_TYPE_HOST, 0, pr_netaddr_get_ipstr(site),
          NULL) < 0) {

        if (ban_lists->bans.bl_listlen < BAN_LIST_MAXSZ) {
          ban_list_add(BAN_TYPE_HOST, 0, pr_netaddr_get_ipstr(site),
            pstrcat(ctrl->ctrls_tmp_pool, "requested by '",
              ctrl->ctrls_cl->cl_user, "' on ",
              pr_strtime(time(NULL)), NULL), 0, NULL);
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "added '%s' to banned hosts list", reqargv[i]);
          pr_ctrls_add_response(ctrl, "host %s banned", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "maximum list size reached, unable to "
            "ban host '%s'", reqargv[i]);

      } else
        pr_ctrls_add_response(ctrl, "host %s already banned", reqargv[i]);
    }

    ban_lock_shm(LOCK_UN);

  /* Handle 'ban class' requests */
  } else if (strcmp(reqargv[0], "class") == 0) {

    if (reqargc < 2) {
      pr_ctrls_add_response(ctrl, "missing arguments");
      return -1;
    }

    if (ban_lock_shm(LOCK_EX) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    /* Add each given class name to the list */
    for (i = 1; i < reqargc; i++) {

      /* Check for duplicates. */
      if (ban_list_exists(BAN_TYPE_CLASS, 0, reqargv[i], NULL) < 0) {

        if (ban_lists->bans.bl_listlen < BAN_LIST_MAXSZ) {
          const char *reason = pstrcat(ctrl->ctrls_tmp_pool, "requested by '",
            ctrl->ctrls_cl->cl_user, "' on ", pr_strtime(time(NULL)), NULL);

          ban_list_add(BAN_TYPE_CLASS, 0, reqargv[i], reason, 0, NULL);
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "added '%s' to banned classes list", reqargv[i]);
          pr_ctrls_add_response(ctrl, "class %s banned", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "maximum list size reached, unable to "
            "ban class '%s'", reqargv[i]);

      } else
        pr_ctrls_add_response(ctrl, "class %s already banned", reqargv[i]);
    }

    ban_lock_shm(LOCK_UN);

  /* Handle 'ban info' requests */
  } else if (strcmp(reqargv[0], "info") == 0) {
    int optc;
    int verbose = FALSE, show_events = FALSE, have_bans = FALSE;
    const char *opts = "ev";

    /* Check for options. */

#if defined(FREEBSD4) || defined(FREEBSD5) || \
    defined(FREEBSD6) || defined(FREEBSD7) || \
    defined(DARWIN7) || defined(DARWIN8) || defined(DARWIN9)
    optreset = 1;
    opterr = 1;
    optind = 1;

#elif defined(SOLARIS2)
    opterr = 0;
    optind = 1;

#else
    opterr = 0;
    optind = 0;
#endif /* !FreeBSD, !Mac OSX and !Solaris2 */

    if (pr_env_get(permanent_pool, "POSIXLY_CORRECT") == NULL) {
      pr_env_set(permanent_pool, "POSIXLY_CORRECT", "1");
    }

    while ((optc = getopt(reqargc, reqargv, opts)) != -1) {
      switch (optc) {
        case 'e':
          show_events = TRUE;
          break;

        case 'v':
          verbose = TRUE;
          break;

        case '?':
          pr_ctrls_add_response(ctrl, "unsupported parameter: '%s'",
            reqargv[1]);
          return -1;
      }
    }

    if (ban_lock_shm(LOCK_SH) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION, "showing ban lists");

    if (ban_lists->bans.bl_listlen) {
      int have_user = FALSE, have_host = FALSE, have_class = FALSE;

      for (i = 0; i < BAN_LIST_MAXSZ; i++) {
        if (ban_lists->bans.bl_entries[i].be_type == BAN_TYPE_USER) {
          have_bans = TRUE;

          if (!have_user) {
            pr_ctrls_add_response(ctrl, "Banned Users:");
            have_user = TRUE;
          }

          pr_ctrls_add_response(ctrl, "  %s",
            ban_lists->bans.bl_entries[i].be_name);

          if (verbose) {
            pr_ctrls_add_response(ctrl, "    Reason: %s",
              ban_lists->bans.bl_entries[i].be_reason);

            if (ban_lists->bans.bl_entries[i].be_expires) {
              time_t now = time(NULL);
              time_t then = ban_lists->bans.bl_entries[i].be_expires;

              pr_ctrls_add_response(ctrl, "    Expires: %s (in %lu seconds)",
                pr_strtime(then), (unsigned long) (then - now));

            } else
              pr_ctrls_add_response(ctrl, "    Expires: never");
          }
        }
      }

      for (i = 0; i < BAN_LIST_MAXSZ; i++) {
        if (ban_lists->bans.bl_entries[i].be_type == BAN_TYPE_HOST) {
          have_bans = TRUE;

          if (!have_host) {
            if (have_user)
              pr_ctrls_add_response(ctrl, "%s", "");
   
            pr_ctrls_add_response(ctrl, "Banned Hosts:");
            have_host = TRUE;
          }

          pr_ctrls_add_response(ctrl, "  %s",
            ban_lists->bans.bl_entries[i].be_name);

          if (verbose) {
            pr_ctrls_add_response(ctrl, "    Reason: %s",
              ban_lists->bans.bl_entries[i].be_reason);

            if (ban_lists->bans.bl_entries[i].be_expires) {
              time_t now = time(NULL);
              time_t then = ban_lists->bans.bl_entries[i].be_expires;

              pr_ctrls_add_response(ctrl, "    Expires: %s (in %lu seconds)",
                pr_strtime(then), (unsigned long) (then - now));

            } else
              pr_ctrls_add_response(ctrl, "    Expires: never");
          }
        }
      }

      for (i = 0; i < BAN_LIST_MAXSZ; i++) {
        if (ban_lists->bans.bl_entries[i].be_type == BAN_TYPE_CLASS) {
          have_bans = TRUE;

          if (!have_class) {
            if (have_host)
              pr_ctrls_add_response(ctrl, "%s", "");

            pr_ctrls_add_response(ctrl, "Banned Classes:");
            have_class = TRUE;
          }

          pr_ctrls_add_response(ctrl, "  %s",
            ban_lists->bans.bl_entries[i].be_name);

          if (verbose) {

            pr_ctrls_add_response(ctrl, "    Reason: %s",
              ban_lists->bans.bl_entries[i].be_reason);

            if (ban_lists->bans.bl_entries[i].be_expires) {
              time_t now = time(NULL);
              time_t then = ban_lists->bans.bl_entries[i].be_expires;

              pr_ctrls_add_response(ctrl, "    Expires: %s (in %lu seconds)",
                pr_strtime(then), (unsigned long) (then - now));

            } else
              pr_ctrls_add_response(ctrl, "    Expires: never");
          }
        }
      }

    } else
      pr_ctrls_add_response(ctrl, "No bans");

/* XXX need a way to clear the event list, too, I think...? */

    if (show_events) {
      pr_ctrls_add_response(ctrl, "%s", "");

      if (ban_lists->events.bel_listlen) {
        int have_banner = FALSE;
        time_t now = time(NULL);

        for (i = 0; i < BAN_EVENT_LIST_MAXSZ; i++) {
          int type = ban_lists->events.bel_entries[i].bee_type;

          switch (type) {
            case BAN_EV_TYPE_ANON_REJECT_PASSWORDS:
            case BAN_EV_TYPE_MAX_CLIENTS_PER_CLASS:
            case BAN_EV_TYPE_MAX_CLIENTS_PER_HOST:
            case BAN_EV_TYPE_MAX_CLIENTS_PER_USER:
            case BAN_EV_TYPE_MAX_HOSTS_PER_USER:
            case BAN_EV_TYPE_MAX_LOGIN_ATTEMPTS:
            case BAN_EV_TYPE_TIMEOUT_IDLE:
            case BAN_EV_TYPE_TIMEOUT_LOGIN:
            case BAN_EV_TYPE_TIMEOUT_NO_TRANSFER:
            case BAN_EV_TYPE_MAX_CONN_PER_HOST:
            case BAN_EV_TYPE_CLIENT_CONNECT_RATE:
              if (!have_banner) {
                pr_ctrls_add_response(ctrl, "Ban Events:");
                have_banner = TRUE;
              }

              pr_ctrls_add_response(ctrl, "  Event: %s",
                ban_event_entry_typestr(type));
              pr_ctrls_add_response(ctrl, "  Source: %s",
                ban_lists->events.bel_entries[i].bee_src);
              pr_ctrls_add_response(ctrl, "    Occurrences: %u/%u",
                ban_lists->events.bel_entries[i].bee_count_curr,
                ban_lists->events.bel_entries[i].bee_count_max);
              pr_ctrls_add_response(ctrl, "    Entry Expires: %lu seconds",
                (unsigned long) ban_lists->events.bel_entries[i].bee_start +
                  ban_lists->events.bel_entries[i].bee_window - now);

              break;
          }
        }

      } else
        pr_ctrls_add_response(ctrl, "No ban events");
    }

    ban_lock_shm(LOCK_UN);

  } else {
    pr_ctrls_add_response(ctrl, "unknown ban type requested: '%s'",
      reqargv[0]);
    return -1;
  }

  return 0;
}

static int ban_handle_permit(pr_ctrls_t *ctrl, int reqargc,
    char **reqargv) {
  register unsigned int i = 0;

  /* Check the permit ACL */
  if (!pr_ctrls_check_acl(ctrl, ban_acttab, "permit")) {

    /* Access denied */
    pr_ctrls_add_response(ctrl, "access denied");
    return -1;
  }

  /* Sanity check */
  if (reqargc < 2 || reqargv == NULL) {
    pr_ctrls_add_response(ctrl, "missing arguments");
    return -1;
  }

  if (ban_engine != TRUE) {
    pr_ctrls_add_response(ctrl, MOD_BAN_VERSION " not enabled");
    return -1;
  }

  /* Handle 'permit user' requests */
  if (strcmp(reqargv[0], "user") == 0) {

    if (ban_lists->bans.bl_listlen == 0) {
      pr_ctrls_add_response(ctrl, "permit request unnecessary");
      pr_ctrls_add_response(ctrl, "no users are banned");
      return 0;
    }

    if (ban_lock_shm(LOCK_EX) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    if (strcmp(reqargv[1], "*") == 0) {

      /* Clear the list by permitting all users. */
      ban_list_remove(BAN_TYPE_USER, NULL);
      pr_ctrls_add_response(ctrl, "all users permitted");

    } else {

      /* Permit each given user name. */
      for (i = 1; i < reqargc; i++) {
        if (ban_list_remove(BAN_TYPE_USER, reqargv[i]) == 0) {
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "removed '%s' from ban list", reqargv[i]);
          pr_ctrls_add_response(ctrl, "user '%s' permitted", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "user '%s' not banned", reqargv[i]);
      }
    }

    ban_lock_shm(LOCK_UN);

  /* Handle 'permit host' requests */
  } else if (strcmp(reqargv[0], "host") == 0) {

    if (ban_lists->bans.bl_listlen == 0) {
      pr_ctrls_add_response(ctrl, "permit request unnecessary");
      pr_ctrls_add_response(ctrl, "no hosts are banned");
      return 0;
    }

    if (ban_lock_shm(LOCK_EX) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    if (strcmp(reqargv[1], "*") == 0) {

      /* Clear the list by permitting all hosts. */
      ban_list_remove(BAN_TYPE_HOST, NULL);
      pr_ctrls_add_response(ctrl, "all hosts permitted");

    } else {

      for (i = 1; i < reqargc; i++) {

        /* XXX handle multiple addresses */
        pr_netaddr_t *site = pr_netaddr_get_addr(ctrl->ctrls_tmp_pool,
          reqargv[i], NULL);

        if (site) {
          if (ban_list_remove(BAN_TYPE_HOST, pr_netaddr_get_ipstr(site)) == 0) {
            (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
              "removed '%s' from banned hosts list", reqargv[i]);
            pr_ctrls_add_response(ctrl, "host '%s' permitted", reqargv[i]);

          } else
            pr_ctrls_add_response(ctrl, "host '%s' not banned", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "unable to resolve '%s' to an IP address",
            reqargv[i]);
      }
    }

    ban_lock_shm(LOCK_UN);

  /* Handle 'permit class' requests */
  } else if (strcmp(reqargv[0], "class") == 0) {

    if (ban_lists->bans.bl_listlen == 0) {
      pr_ctrls_add_response(ctrl, "permit request unnecessary");
      pr_ctrls_add_response(ctrl, "no classes are banned");
      return 0;
    }

    if (ban_lock_shm(LOCK_EX) < 0) {
      pr_ctrls_add_response(ctrl, "error locking shm: %s", strerror(errno));
      return -1;
    }

    if (strcmp(reqargv[1], "*") == 0) {

      /* Clear the list by permitting all classes. */
      ban_list_remove(BAN_TYPE_CLASS, NULL);
      pr_ctrls_add_response(ctrl, "all classes permitted");

    } else {

      /* Permit each given class name. */
      for (i = 1; i < reqargc; i++) {
        if (ban_list_remove(BAN_TYPE_CLASS, reqargv[i]) == 0) {
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "removed '%s' from banned classes list", reqargv[i]);
          pr_ctrls_add_response(ctrl, "class '%s' permitted", reqargv[i]);

        } else
          pr_ctrls_add_response(ctrl, "class '%s' not banned", reqargv[i]);
      }
    }

    ban_lock_shm(LOCK_UN);
 
  } else {
    pr_ctrls_add_response(ctrl, "unknown ban type requested: '%s'",
      reqargv[0]);
    return -1;
  }

  return 0;
}

/* Command handlers
 */

MODRET ban_pre_pass(cmd_rec *cmd) {
  char *user, *rule_mesg = NULL;

  if (ban_engine != TRUE)
    return PR_DECLINED(cmd);

  user = pr_table_get(session.notes, "mod_auth.orig-user", NULL);

  if (!user)
    return PR_DECLINED(cmd);

  /* Make sure the list is up-to-date. */
  ban_list_expire();

  /* Check banned user list */
  if (ban_list_exists(BAN_TYPE_USER, main_server->sid, user, &rule_mesg) == 0) {
    pr_log_pri(PR_LOG_INFO, MOD_BAN_VERSION
      ": Login denied: user '%s' banned", user);
    ban_send_mesg(cmd->tmp_pool, user, rule_mesg);
    return PR_ERROR_MSG(cmd, R_530, _("Login incorrect."));
  }

  return PR_DECLINED(cmd);
}

/* Configuration handlers
 */

/* usage: BanControlsACLs actions|all allow|deny user|group list */
MODRET set_banctrlsacls(cmd_rec *cmd) {
  char *bad_action = NULL, **actions = NULL;

  CHECK_ARGS(cmd, 4);
  CHECK_CONF(cmd, CONF_ROOT);

  /* We can cheat here, and use the ctrls_parse_acl() routine to
   * separate the given string...
   */
  actions = ctrls_parse_acl(cmd->tmp_pool, cmd->argv[1]);

  /* Check the second parameter to make sure it is "allow" or "deny" */
  if (strcmp(cmd->argv[2], "allow") != 0 &&
      strcmp(cmd->argv[2], "deny") != 0)
    CONF_ERROR(cmd, "second parameter must be 'allow' or 'deny'");

  /* Check the third parameter to make sure it is "user" or "group" */
  if (strcmp(cmd->argv[3], "user") != 0 &&
      strcmp(cmd->argv[3], "group") != 0)
    CONF_ERROR(cmd, "third parameter must be 'user' or 'group'");

  bad_action = pr_ctrls_set_module_acls(ban_acttab, ban_pool, actions,
    cmd->argv[2], cmd->argv[3], cmd->argv[4]);
  if (bad_action != NULL)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown action: '",
      bad_action, "'", NULL));

  return PR_HANDLED(cmd);
}

/* usage: BanEngine on|off */
MODRET set_banengine(cmd_rec *cmd) {
  int bool = -1, ctxt_type;
  config_rec *c;

  CHECK_ARGS(cmd, 1);

  bool = get_boolean(cmd, 1);
  if (bool == -1)
    CONF_ERROR(cmd, "expected Boolean parameter");

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  ctxt_type = (cmd->config && cmd->config->config_type != CONF_PARAM ?
     cmd->config->config_type : cmd->server->config_type ?
     cmd->server->config_type : CONF_ROOT);

  if (ctxt_type == CONF_ROOT) {
    /* If ban_engine has not been initialized yet, and this is the
     * "server config" section, we can do it here.  And even if the
     * previously initialized value is 0 ("BanEngine off"), if the
     * current value is 1 ("BanEngine on"), use it.  This can haappen,
     * for example, when there are multiple BanEngine directives in the
     * config, in <IfClass> sections, for whitelisting.
     */

    if (ban_engine == -1) {
      ban_engine = bool;
    }

    if (bool == TRUE) {
      ban_engine = bool;
    }
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = palloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = bool;

  return PR_HANDLED(cmd);
}

/* usage: BanLog path|"none" */
MODRET set_banlog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (strcasecmp(cmd->argv[1], "none") != 0 &&
      pr_fs_valid_path(cmd->argv[1]) < 0) {
    CONF_ERROR(cmd, "must be an absolute path");
  }

  ban_log = pstrdup(ban_pool, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: BanMessage mesg */
MODRET set_banmessage(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  ban_mesg = pstrdup(ban_pool, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: BanOnEvent event freq duration [mesg] */
MODRET set_banonevent(cmd_rec *cmd) {
  struct ban_event_entry *bee;
  int n;
  char *tmp;

  CHECK_ARGS(cmd, 3);
  CHECK_CONF(cmd, CONF_ROOT);

  bee = pcalloc(ban_pool, sizeof(struct ban_event_entry));

  tmp = strchr(cmd->argv[2], '/');
  if (!tmp)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "badly formatted freq parameter: '",
      cmd->argv[2], "'", NULL));

  /* The frequency string is formatted as "N/hh:mm:ss", where N is the count
   * to be reached within the given time interval.
   */

  *tmp = '\0';

  n = atoi(cmd->argv[2]);
  if (n < 1)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "freq occurrences must be greater than 0", NULL));
  bee->bee_count_max = n;

  bee->bee_window = ban_parse_timestr(tmp+1);
  if (bee->bee_window == (time_t) -1)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "badly formatted freq parameter: '", cmd->argv[2], "'", NULL));
  if (bee->bee_window == 0)
    CONF_ERROR(cmd, "freq parameter cannot be '00:00:00'");

  /* The duration is the next parameter. */
  bee->bee_expires = ban_parse_timestr(cmd->argv[3]);
  if (bee->bee_expires == (time_t) -1)
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool,
      "badly formatted duration parameter: '", cmd->argv[2], "'", NULL));
  if (bee->bee_expires == 0)
    CONF_ERROR(cmd, "duration parameter cannot be '00:00:00'");

  /* If present, the next parameter is a custom ban message. */
  if (cmd->argc == 5) {
    sstrncpy(bee->bee_mesg, cmd->argv[4], sizeof(bee->bee_mesg));
  }

  if (strcasecmp(cmd->argv[1], "AnonRejectPasswords") == 0) {
    bee->bee_type = BAN_EV_TYPE_ANON_REJECT_PASSWORDS;
    pr_event_register(&ban_module, "mod_auth.anon-reject-passwords",
      ban_anonrejectpasswords_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "ClientConnectRate") == 0) {
    bee->bee_type = BAN_EV_TYPE_CLIENT_CONNECT_RATE;
    pr_event_register(&ban_module, "mod_ban.client-connect-rate",
      ban_clientconnectrate_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "MaxClientsPerClass") == 0) {
    bee->bee_type = BAN_EV_TYPE_MAX_CLIENTS_PER_CLASS;
    pr_event_register(&ban_module, "mod_auth.max-clients-per-class",
      ban_maxclientsperclass_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "MaxClientsPerHost") == 0) {
    bee->bee_type = BAN_EV_TYPE_MAX_CLIENTS_PER_HOST;
    pr_event_register(&ban_module, "mod_auth.max-clients-per-host",
      ban_maxclientsperhost_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "MaxClientsPerUser") == 0) {
    bee->bee_type = BAN_EV_TYPE_MAX_CLIENTS_PER_USER;
    pr_event_register(&ban_module, "mod_auth.max-clients-per-user",
      ban_maxclientsperuser_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "MaxConnectionsPerHost") == 0) {
    bee->bee_type = BAN_EV_TYPE_MAX_CONN_PER_HOST;
    pr_event_register(&ban_module, "mod_auth.max-connections-per-host",
      ban_maxconnperhost_ev, bee);
  
  } else if (strcasecmp(cmd->argv[1], "MaxHostsPerUser") == 0) {
    bee->bee_type = BAN_EV_TYPE_MAX_HOSTS_PER_USER;
    pr_event_register(&ban_module, "mod_auth.max-hosts-per-user",
      ban_maxhostsperuser_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "MaxLoginAttempts") == 0) {
    bee->bee_type = BAN_EV_TYPE_MAX_LOGIN_ATTEMPTS;
    pr_event_register(&ban_module, "mod_auth.max-login-attempts",
      ban_maxloginattempts_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "TimeoutIdle") == 0) {
    bee->bee_type = BAN_EV_TYPE_TIMEOUT_IDLE;
    pr_event_register(&ban_module, "core.timeout-idle",
      ban_timeoutidle_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "TimeoutLogin") == 0) {
    bee->bee_type = BAN_EV_TYPE_TIMEOUT_LOGIN;
    pr_event_register(&ban_module, "core.timeout-login",
      ban_timeoutlogin_ev, bee);

  } else if (strcasecmp(cmd->argv[1], "TimeoutNoTransfer") == 0) {
    bee->bee_type = BAN_EV_TYPE_TIMEOUT_NO_TRANSFER;
    pr_event_register(&ban_module, "core.timeout-no-transfer",
      ban_timeoutnoxfer_ev, bee);

  } else {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "unknown ", cmd->argv[0], " name: '",
      cmd->argv[1], "'", NULL));
  }

  return PR_HANDLED(cmd);
}

/* usage: BanTable path */
MODRET set_bantable(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT);

  if (pr_fs_valid_path(cmd->argv[1]) < 0)
    CONF_ERROR(cmd, "must be an absolute path");

  ban_table = pstrdup(ban_pool, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* Timer handlers
 */

static int ban_timer_cb(CALLBACK_FRAME) {
  ban_list_expire();
  ban_event_list_expire();
  return 1;
}

/* Event handlers
 */

static void ban_exit_ev(const void *event_data, void *user_data) {

  /* Remove the shm from the system.  We can only do this reliably
   * when the standalone daemon process exits; if it's an inetd process,
   * there many be other proftpd processes still running.
   */

  if (getpid() == mpid &&
      ServerType == SERVER_STANDALONE &&
      ban_shmid >= 0) {
    struct shmid_ds ds;
    int res;

#if !defined(_POSIX_SOURCE)
    res = shmdt((char *) ban_lists);
#else
    res = shmdt((const void *) ban_lists);
#endif

    if (res < 0) {
      pr_log_debug(DEBUG1, MOD_BAN_VERSION ": error detaching shm: %s",
        strerror(errno));
    }

    memset(&ds, 0, sizeof(ds));

    PRIVS_ROOT
    res = shmctl(ban_shmid, IPC_RMID, &ds);
    PRIVS_RELINQUISH

    if (res < 0) {
      pr_log_debug(DEBUG1, MOD_BAN_VERSION ": error removing shmid %d: %s",
        ban_shmid, strerror(errno));
    }
  }
}

/* A helper function, to factor many of the BanOnEvent event handling
 * code into a single location.
 */
static void ban_handle_event(unsigned int ev_type, int ban_type,
    const char *src, struct ban_event_entry *tmpl) {
  config_rec *c;
  int end_session = FALSE;
  struct ban_event_entry *bee = NULL;
  const char *event = ban_event_entry_typestr(ev_type);
  pool *tmp_pool = NULL;

  /* Check to see if the BanEngine directive is set to 'off'.  We need
   * to do this here since events can happen before the POST_CMD PASS
   * handling that mod_ban does.
   */
  c = find_config(main_server->conf, CONF_PARAM, "BanEngine", FALSE);
  if (c) {
    int use_bans = *((int *) c->argv[0]);

    if (!use_bans)
      return;
  }

  if (ban_lock_shm(LOCK_EX) < 0) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "error write-locking shm: %s", strerror(errno));
    return;
  }

  tmp_pool = make_sub_pool(ban_pool);

  ban_event_list_expire();

  bee = ban_event_list_get(ev_type, main_server->sid, src);

  if (!bee &&
      tmpl->bee_count_max > 0) {
    /* Add a new entry. */
    if (ban_event_list_add(ev_type, main_server->sid, src, tmpl->bee_count_max,
        tmpl->bee_window, tmpl->bee_expires) < 0) {
      (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
        "error adding ban event for %s: %s", event, strerror(errno));

    } else {
      (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
        "added ban event for %s", event);
    }

    bee = ban_event_list_get(ev_type, main_server->sid, src);
  }

  if (bee) {
    /* Update the entry. */
    if (bee->bee_count_curr < bee->bee_count_max) {
      bee->bee_count_curr++;
    }

    if (bee->bee_count_curr >= bee->bee_count_max) {
      int res;

      /* Threshold has been reached, add an entry to the ban list.
       * Check for an existing entry first, though.
       */

      res = ban_list_exists(ban_type, main_server->sid, src, NULL);
      if (res < 0) {
        const char *reason = pstrcat(tmp_pool, event, " autoban at ",
          pr_strtime(time(NULL)), NULL);

        ban_list_expire();

        if (ban_list_add(ban_type, main_server->sid, src, reason,
            tmpl->bee_expires, tmpl->bee_mesg) < 0) {
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "error adding %s-triggered autoban for %s '%s': %s", event,
            ban_type == BAN_TYPE_USER ? "user" :
              ban_type == BAN_TYPE_HOST ? "host" : "class", src,
            strerror(errno));

        } else {
          (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
            "added %s-triggered autoban for %s '%s'", event,
              ban_type == BAN_TYPE_USER ? "user" :
                ban_type == BAN_TYPE_HOST ? "host" : "class", src);
        }

        end_session = TRUE;

      } else
        (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
          "updated count for %s event entry: %u curr, %u max", event,
          bee->bee_count_curr, bee->bee_count_max);
    }
  }

  ban_lock_shm(LOCK_UN);

  if (end_session) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "%s autoban threshold reached, ending session", event);
    pr_log_debug(DEBUG3, MOD_BAN_VERSION
      ": autoban threshold reached, ending session");
    ban_send_mesg(tmp_pool, ban_type == BAN_TYPE_USER ? src : "(none)", NULL);
    end_login(0);
  }

  return;
}

static void ban_anonrejectpasswords_ev(const void *event_data,
    void *user_data) {

  /* For this event, event_data is the client. */
  conn_t *c = (conn_t *) event_data;
  const char *ipstr;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ipstr = pr_netaddr_get_ipstr(c->remote_addr);
  ban_handle_event(BAN_EV_TYPE_ANON_REJECT_PASSWORDS, BAN_TYPE_HOST,
    ipstr, tmpl);
}

static void ban_clientconnectrate_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the client. */
  conn_t *c = (conn_t *) event_data;
  const char *ipstr;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ipstr = pr_netaddr_get_ipstr(c->remote_addr);
  ban_handle_event(BAN_EV_TYPE_CLIENT_CONNECT_RATE, BAN_TYPE_HOST, ipstr, tmpl);
}

static void ban_maxclientsperclass_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the class name. */
  char *class = (char *) event_data;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  if (class) {
    ban_handle_event(BAN_EV_TYPE_MAX_CLIENTS_PER_CLASS, BAN_TYPE_CLASS,
      class, tmpl);
  }
}

static void ban_maxclientsperhost_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the client. */
  conn_t *c = (conn_t *) event_data;
  const char *ipstr;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ipstr = pr_netaddr_get_ipstr(c->remote_addr);
  ban_handle_event(BAN_EV_TYPE_MAX_CLIENTS_PER_HOST, BAN_TYPE_HOST,
    ipstr, tmpl);
}

static void ban_maxclientsperuser_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the user name. */
  char *user = (char *) event_data;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ban_handle_event(BAN_EV_TYPE_MAX_CLIENTS_PER_USER, BAN_TYPE_USER,
    user, tmpl);
}

static void ban_maxconnperhost_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the client. */
  conn_t *c = (conn_t *) event_data;
  const char *ipstr;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ipstr = pr_netaddr_get_ipstr(c->remote_addr);
  ban_handle_event(BAN_EV_TYPE_MAX_CONN_PER_HOST, BAN_TYPE_HOST,
    ipstr, tmpl);
}

static void ban_maxhostsperuser_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the user name. */
  char *user = (char *) event_data;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ban_handle_event(BAN_EV_TYPE_MAX_HOSTS_PER_USER, BAN_TYPE_USER,
    user, tmpl);
}

static void ban_maxloginattempts_ev(const void *event_data, void *user_data) {

  /* For this event, event_data is the client. */
  conn_t *c = (conn_t *) event_data;
  const char *ipstr;

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ipstr = pr_netaddr_get_ipstr(c->remote_addr);
  ban_handle_event(BAN_EV_TYPE_MAX_LOGIN_ATTEMPTS, BAN_TYPE_HOST, ipstr,
    tmpl);
}

#if defined(PR_SHARED_MODULE)
static void ban_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_ban.c", (const char *) event_data) == 0) {
    register unsigned int i;

    for (i = 0; ban_acttab[i].act_action; i++) {
      (void) pr_ctrls_unregister(&ban_module, ban_acttab[i].act_action);
    }

    if (ban_timerno > 0) {
      (void) pr_timer_remove(ban_timerno, &ban_module);
      ban_timerno = -1;
    }

    pr_event_unregister(&ban_module, NULL, NULL);

    if (ban_pool) {
      destroy_pool(ban_pool);
      ban_pool = NULL;
    }

    if (ban_tabfh) {
      (void) pr_fsio_close(ban_tabfh);
      ban_tabfh = NULL;
    }

    if (ban_logfd > 0) {
      (void) close(ban_logfd);
      ban_logfd = -1;
    }

    ban_engine = -1;
  }
}
#endif /* PR_SHARED_MODULE */

static void ban_postparse_ev(const void *event_data, void *user_data) {
  struct ban_data *lists;

  if (ban_engine != TRUE)
    return;

  /* Open the BanLog. */
  if (ban_log &&
      strcasecmp(ban_log, "none") != 0) {
    int res;

    PRIVS_ROOT
    res = pr_log_openfile(ban_log, &ban_logfd, 0660);
    PRIVS_RELINQUISH

    switch (res) {
      case 0:
        break;

      case -1:
        pr_log_debug(DEBUG1, MOD_BAN_VERSION ": unable to open BanLog '%s': %s",
          ban_log, strerror(errno));
        break;

      case PR_LOG_SYMLINK:
        pr_log_debug(DEBUG1, MOD_BAN_VERSION ": unable to open BanLog '%s': %s",
          ban_log, "is a symlink");
        break;

      case PR_LOG_WRITABLE_DIR:
        pr_log_debug(DEBUG1, MOD_BAN_VERSION ": unable to open BanLog '%s': %s",
          ban_log, "parent directory is world-writable");
        break;
    } 
  }

  /* Make sure the BanTable exists. */
  if (!ban_table) {
    pr_log_pri(PR_LOG_NOTICE, MOD_BAN_VERSION
      ": missing required BanTable configuration");
    end_login(1);
  }

  PRIVS_ROOT
  ban_tabfh = pr_fsio_open(ban_table, O_RDWR|O_CREAT); 
  PRIVS_RELINQUISH

  if (!ban_tabfh) {
    pr_log_pri(PR_LOG_NOTICE, MOD_BAN_VERSION
      ": unable to open BanTable '%s': %s", ban_table, strerror(errno));
    end_login(1);
  }

  /* Get the shm for storing all of our ban info. */
  lists = ban_get_shm(ban_tabfh);
  if (lists == NULL &&
      errno != EEXIST) {
    pr_log_pri(PR_LOG_NOTICE, MOD_BAN_VERSION
      ": unable to get shared memory for BanTable '%s': %s", ban_table,
      strerror(errno));
    end_login(1);
  }

  if (lists)
    ban_lists = lists;

  ban_timerno = pr_timer_add(BAN_TIMER_INTERVAL, -1, &ban_module, ban_timer_cb,
    "ban list expiry");
  return;
}

static void ban_restart_ev(const void *event_data, void *user_data) {
  register unsigned int i;

  if (ban_pool) {
    destroy_pool(ban_pool);
    ban_pool = NULL;
  }

  ban_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ban_pool, MOD_BAN_VERSION);

  /* Register the control handlers */
  for (i = 0; ban_acttab[i].act_action; i++) {

    /* Allocate and initialize the ACL for this control. */
    ban_acttab[i].act_acl = pcalloc(ban_pool, sizeof(ctrls_acl_t));
    pr_ctrls_init_acl(ban_acttab[i].act_acl);
  }

  /* Unregister any BanOnEvent event handlers */
  pr_event_unregister(&ban_module, "core.timeout-idle", NULL);
  pr_event_unregister(&ban_module, "core.timeout-login", NULL);
  pr_event_unregister(&ban_module, "core.timeout-no-transfer", NULL);
  pr_event_unregister(&ban_module, "mod_auth.anon-reject-passwords", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-clients-per-class", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-clients-per-host", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-clients-per-user", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-connections-per-host", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-hosts-per-user", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-login-attempts", NULL);
  pr_event_unregister(&ban_module, "mod_auth.max-users-per-host", NULL);
  pr_event_unregister(&ban_module, "mod_ban.client-connect-rate", NULL);

  /* "Bounce" the log file descriptor */
  close(ban_logfd);
  ban_logfd = -1;

  if (ban_log &&
      strcasecmp(ban_log, "none") != 0) {
    int res;

    PRIVS_ROOT
    res = pr_log_openfile(ban_log, &ban_logfd, 0660);
    PRIVS_RELINQUISH

    switch (res) {
      case 0:
        break;

      case -1:
        pr_log_debug(DEBUG1, MOD_BAN_VERSION ": unable to open BanLog '%s': %s",
          ban_log, strerror(errno));
        break;

      case PR_LOG_SYMLINK:
        pr_log_debug(DEBUG1, MOD_BAN_VERSION ": unable to open BanLog '%s': %s",
          ban_log, "is a symlink");
        break;

      case PR_LOG_WRITABLE_DIR:
        pr_log_debug(DEBUG1, MOD_BAN_VERSION ": unable to open BanLog '%s': %s",
          ban_log, "parent directory is world-writable");
        break;
    }
  }

  if (ban_timerno > 0) {
    (void) pr_timer_remove(ban_timerno, &ban_module);
    ban_timerno = -1;
  }

  return;
}

static void ban_timeoutidle_ev(const void *event_data, void *user_data) {
  const char *ipstr = pr_netaddr_get_ipstr(session.c->remote_addr);

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ban_handle_event(BAN_EV_TYPE_TIMEOUT_IDLE, BAN_TYPE_HOST, ipstr, tmpl);
}

static void ban_timeoutlogin_ev(const void *event_data, void *user_data) {
  const char *ipstr = pr_netaddr_get_ipstr(session.c->remote_addr);

  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;

  if (ban_engine != TRUE)
    return;

  ban_handle_event(BAN_EV_TYPE_TIMEOUT_LOGIN, BAN_TYPE_HOST, ipstr, tmpl);
}

static void ban_timeoutnoxfer_ev(const void *event_data, void *user_data) {
  const char *ipstr = pr_netaddr_get_ipstr(session.c->remote_addr);
  
  /* user_data is a template of the ban event entry. */
  struct ban_event_entry *tmpl = user_data;
  
  if (ban_engine != TRUE)
    return;
  
  ban_handle_event(BAN_EV_TYPE_TIMEOUT_NO_TRANSFER, BAN_TYPE_HOST, ipstr, tmpl);
}

/* Initialization routines
 */

static int ban_init(void) {
  register unsigned int i = 0;

  /* Allocate the pool for this module's use. */
  ban_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(ban_pool, MOD_BAN_VERSION);

  /* Register the control handlers */
  for (i = 0; ban_acttab[i].act_action; i++) {

    /* Allocate and initialize the ACL for this control. */
    ban_acttab[i].act_acl = pcalloc(ban_pool, sizeof(ctrls_acl_t));
    pr_ctrls_init_acl(ban_acttab[i].act_acl);

    if (pr_ctrls_register(&ban_module, ban_acttab[i].act_action,
        ban_acttab[i].act_desc, ban_acttab[i].act_cb) < 0)
     pr_log_pri(PR_LOG_INFO, MOD_BAN_VERSION
        ": error registering '%s' control: %s",
        ban_acttab[i].act_action, strerror(errno));
  }

  pr_event_register(&ban_module, "core.exit", ban_exit_ev, NULL);
#if defined(PR_SHARED_MODULE)
  pr_event_register(&ban_module, "core.module-unload", ban_mod_unload_ev,
    NULL);
#endif /* PR_SHARED_MODULE */
  pr_event_register(&ban_module, "core.postparse", ban_postparse_ev, NULL);
  pr_event_register(&ban_module, "core.restart", ban_restart_ev, NULL);

  return 0;
}

static int ban_sess_init(void) {
  config_rec *c;
  pool *tmp_pool;
  const char *class, *remote_ip;
  char *rule_mesg = NULL;

  if (ban_engine != TRUE)
    return 0;

  /* Check to see if the BanEngine directive is set to 'off'. */
  c = find_config(main_server->conf, CONF_PARAM, "BanEngine", FALSE);
  if (c) {
    int use_bans = *((int *) c->argv[0]);

    if (!use_bans) {
      ban_engine = FALSE;
      return 0;
    }
  }

  tmp_pool = make_sub_pool(ban_pool);

  class = session.class ? session.class->cls_name : "";
  remote_ip = pr_netaddr_get_ipstr(session.c->remote_addr);

  /* Make sure the list is up-to-date. */
  ban_list_expire();

  /* Check banned host list */
  if (ban_list_exists(BAN_TYPE_HOST, main_server->sid, remote_ip,
      &rule_mesg) == 0) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "login from host '%s' denied due to host ban", remote_ip);
    pr_log_pri(PR_LOG_INFO, MOD_BAN_VERSION
      ": Login denied: host '%s' banned", remote_ip);

    ban_send_mesg(tmp_pool, "(none)", rule_mesg);
    destroy_pool(tmp_pool);

    errno = EACCES;
    return -1;
  }

  /* Check banned class list */
  if (ban_list_exists(BAN_TYPE_CLASS, main_server->sid, class,
      &rule_mesg) == 0) {
    (void) pr_log_writefile(ban_logfd, MOD_BAN_VERSION,
      "login from class '%s' denied due to class ban", class);
    pr_log_pri(PR_LOG_INFO, MOD_BAN_VERSION
      ": Login denied: class '%s' banned", class);

    ban_send_mesg(tmp_pool, "(none)", rule_mesg); 
    destroy_pool(tmp_pool);

    errno = EACCES;
    return -1;
  }

  pr_event_generate("mod_ban.client-connect-rate", session.c);

  pr_event_unregister(&ban_module, "core.exit", ban_exit_ev);
  pr_event_unregister(&ban_module, "core.restart", ban_restart_ev);

  return 0;
}

/* Controls table
 */

static ctrls_acttab_t ban_acttab[] = {
  { "ban",	"ban a class, host, or user from using the daemon",	NULL,
     ban_handle_ban },
  { "permit",	"allow a banned class, host or user to use the daemon",	NULL,
    ban_handle_permit },
  { NULL, NULL, NULL, NULL }
};

/* Module API tables
 */

static conftable ban_conftab[] = {
  { "BanControlsACLs",		set_banctrlsacls,	NULL },
  { "BanEngine",		set_banengine,		NULL },
  { "BanLog",			set_banlog,		NULL },
  { "BanMessage",		set_banmessage,		NULL },
  { "BanOnEvent",		set_banonevent,		NULL },
  { "BanTable",			set_bantable,		NULL },
  { NULL }
};

static cmdtable ban_cmdtab[] = {
  { PRE_CMD,	C_PASS,	G_NONE,	ban_pre_pass,	FALSE,	FALSE },
  { 0, NULL }
};

module ban_module = {
  NULL, NULL,

  /* Module API version 2.0 */
  0x20,

  /* Module name */
  "ban",

  /* Module configuration handler table */
  ban_conftab,

  /* Module command handler table */
  ban_cmdtab,

  /* Module authentication handler table */
  NULL,

  /* Module initialization function */
  ban_init,

  /* Session initialization function */
  ban_sess_init,

  /* Module version */
  MOD_BAN_VERSION
};

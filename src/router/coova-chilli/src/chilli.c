/* -*- mode: c; c-basic-offset: 2 -*- */
/*
 * Copyright (C) 2007-2013 David Bird (Coova Technologies) <support@coova.com>
 * Copyright (C) 2003-2005 Mondru AB.,
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sys/sysinfo.h>
#include "chilli.h"
#include "bstrlib.h"
#ifdef ENABLE_MODULES
#include "chilli_module.h"
#endif

struct tun_t *tun;                /* TUN instance            */
struct ippool_t *ippool;          /* Pool of IP addresses */
struct radius_t *radius;          /* Radius client instance */
struct dhcp_t *dhcp = NULL;       /* DHCP instance */
struct redir_t *redir = NULL;     /* Redir instance */

#ifdef ENABLE_MULTIROUTE
#include "rtmon.h"
struct rtmon_t _rtmon;
#endif

static int connections=0;
struct app_conn_t *firstfreeconn=0; /* First free in linked list */
struct app_conn_t *lastfreeconn=0;  /* Last free in linked list */
struct app_conn_t *firstusedconn=0; /* First used in linked list */
struct app_conn_t *lastusedconn=0;  /* Last used in linked list */
struct app_conn_t admin_session;

struct timespec mainclock;
time_t checktime;
time_t rereadtime;

static int *p_keep_going = 0;
static int *p_reload_config = 0;
/*static int do_timeouts = 1;*/
static int do_interval = 0;

#ifdef ENABLE_UAMANYIP
/* some IPv4LL/APIPA(rfc 3927) specific stuff for uamanyip */
struct in_addr ipv4ll_ip;
struct in_addr ipv4ll_mask;
#endif
#ifdef ENABLE_SSDP
#define SSDP_MCAST_ADDR ("239.255.255.250")
#define SSDP_PORT 1900
struct in_addr ssdp;
#endif

static int acct_req(acct_type type,
		    struct app_conn_t *conn,
		    uint8_t status_type);

static pid_t chilli_pid = 0;

#ifdef ENABLE_CHILLIPROXY
static pid_t proxy_pid = 0;
#endif

#ifdef ENABLE_CHILLIRADSEC
static pid_t radsec_pid = 0;
#endif

#ifdef ENABLE_CHILLIREDIR
static pid_t redir_pid = 0;
#endif

typedef struct child {
  pid_t pid;
  uint8_t type;
  time_t started;
  char *name;
  struct child *next;
} CHILD;

static unsigned long child_count_tot = 0;
static int child_count = 0;
CHILD * children = 0;

CHILD *child_create(uint8_t type, pid_t pid, char *name) {
  CHILD *node;
  if (!(node=malloc(sizeof(CHILD)))) return 0;
  node->started = mainclock_now();
  node->type = type;
  node->pid = pid;
  node->name = name;
  node->next = 0;
  return node;
}

CHILD *child_insert_head(CHILD *list, uint8_t type, pid_t pid, char *name) {
  CHILD *newnode = child_create(type, pid, name);
  if (!newnode) return 0;
  newnode->next = list;
  return newnode;
}

int child_add_pid(uint8_t type, pid_t pid, char *name) {
  if (!children) {
    /* Create the list head, the main process */
    children = child_create(CHILLI_PROC, getpid(), "[chilli]");
    if (!children) return -1;
  }
  children->next = child_insert_head(children->next, type, pid, name);
  if (!children->next) return -1;
  return 0;
}

int child_remove_pid(pid_t pid) {
  /* Will never be the head "children"-
     which is the main process pid */
  CHILD *list, *node;
  if (children) {
    list = children;
    while (list->next && list->next->pid != pid)
      list = list->next;

    if (list->next) {
      node = list->next;
      list->next = node->next;
#if(_debug_)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Freed child process %d [%s]", __FUNCTION__, __LINE__, node->pid, node->name);
#endif
      free(node);
      return 0;
    }
  }
  return -1;
}

#if defined(ENABLE_CHILLIREDIR) || defined(ENABLE_CHILLIPROXY) || defined(ENABLE_CHILLIRADSEC)
static pid_t launch_daemon(char *name, char *path) {
  pid_t cpid = getpid();
  pid_t p = chilli_fork(CHILLI_PROC_DAEMON, name);

  if (p < 0) {

    syslog(LOG_ERR, "%s: fork failed", strerror(errno));

  } else if (p == 0) {

    char *newargs[16];
    char file[128];
    int i=0;

    chilli_binconfig(file, sizeof(file), cpid);

    newargs[i++] = name;
    newargs[i++] = "-b";
    newargs[i++] = file;
    newargs[i++] = NULL;

    if (execv(path, newargs) != 0) {
      syslog(LOG_ERR, "%s: execl() did not return 0!", strerror(errno));
      exit(0);
    }

  } else {

    return p;

  }

  return 0;
}
#endif

#ifdef ENABLE_CHILLIPROXY
static void launch_chilliproxy(void) {
  proxy_pid = launch_daemon("[chilli_proxy]", SBINDIR "/chilli_proxy");
}
#endif

#ifdef ENABLE_CHILLIREDIR
static void launch_chilliredir(void) {
  redir_pid = launch_daemon("[chilli_redir]", SBINDIR "/chilli_redir");
}
#endif

#ifdef ENABLE_CHILLIRADSEC
static void launch_chilliradsec(void) {
  radsec_pid = launch_daemon("[chilli_radsec]", SBINDIR "/chilli_radsec");
}
#endif

static int proc_status(char *name, pid_t pid) {
  char buffer[128];
  char * line = 0;
  size_t len = 0;
  int ret = 0;
  ssize_t read;
  FILE* fp;

  snprintf(buffer, sizeof(buffer), "/proc/%i/status", pid);
  fp = fopen(buffer, "r");
  if (!fp) return -1;

  while ((read = getline(&line, &len, fp)) != -1) {
    if (!memcmp(line, name, strlen(name))) {
      int i;
      if (sscanf(line+strlen(name)+1, "%d %s", &i, buffer) == 2) {
	ret = i;
	if (buffer[0] == 'm') ret *= 1000;
	else if (buffer[0] == 'g') ret *= 1000000;
      }
    }
  }

  if (line)
    free(line);

  fclose(fp);
  return ret;
}

static int proc_countfds(pid_t pid) {
  char buffer[128];
  int ret = 0;
  DIR *dir;

  struct dirent * d = 0;

  snprintf(buffer, sizeof(buffer), "/proc/%i/fd", pid);
  dir = opendir(buffer);
  if (!dir) return -1;

  while ((d = readdir(dir)))
    if (d->d_name[0] != '.')
      ret++;

  closedir(dir);
  return ret;
}

void child_print(bstring s) {
  time_t now = mainclock_now();
  CHILD *node = children;
  char line[256];

  snprintf(line, sizeof(line), "Children %d Max %d Total %ld\n",
                child_count, _options.childmax, child_count_tot);

  bcatcstr(s, line);

  while (node) {
    char *n = "";
    switch (node->type) {
      case CHILLI_PROC:        n = "Main";     break;
      case CHILLI_PROC_DAEMON: n = "Daemon";   break;
      case CHILLI_PROC_REDIR:  n = "Redirect"; break;
      case CHILLI_PROC_SCRIPT: n = "Script";   break;
    }
    snprintf(line, sizeof(line)-1,
		  "PID %8d %-8s %-20s up %d [Vm Size: %d RSS: %d FDs: %d]\n",
		  node->pid, n, node->name,
		  (int)(now - node->started),
		  proc_status("VmSize:", node->pid),
		  proc_status("VmRSS:", node->pid),
		  proc_countfds(node->pid));
    bcatcstr(s, line);
    node = node->next;
  }
}

void child_killall(int sig) {
  CHILD *node = children;
  while (node) {
    kill(node->pid, sig);
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): pid %d killed %d", __FUNCTION__, __LINE__, getpid(), node->pid);
    node = node->next;
  }
}

pid_t chilli_fork(uint8_t type, char *name) {
  pid_t pid;

  if (child_count == _options.childmax) {
    return -1;
  }

  pid = fork();

  if (pid > 0) {
    if (child_add_pid(type, pid, name) == 0) {
      ++child_count;
      ++child_count_tot;
    }
  }

  return pid;
}

static void _sigchld(int signum) {
  pid_t pid;
  int stat;
  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): child %d terminated", __FUNCTION__, __LINE__, pid);
#endif
#ifdef ENABLE_CHILLIRADSEC
    if (radsec_pid > 0 && radsec_pid == pid) {
      syslog(LOG_ERR, "Having to re-launch chilli_radsec... PID %d exited", pid);
      launch_chilliradsec();
    }
#endif
#ifdef ENABLE_CHILLIPROXY
    if (proxy_pid > 0 && proxy_pid == pid) {
      syslog(LOG_ERR, "Having to re-launch chilli_proxy... PID %d exited", pid);
      launch_chilliproxy();
    }
#endif
#ifdef ENABLE_CHILLIREDIR
    if (redir_pid > 0 && redir_pid == pid) {
      syslog(LOG_ERR, "Having to re-launch chilli_redir... PID %d exited", pid);
      launch_chilliredir();
    }
#endif
    if (child_remove_pid(pid) == 0)
      child_count--;
  }
}

static void _sigterm(int signum) {
  syslog(LOG_DEBUG, "%s(%d): SIGTERM: shutdown", __FUNCTION__, __LINE__);
  if (p_keep_going)
    *p_keep_going = 0;
}

static void _sigvoid(int signum) {
#if(_debug_)
  syslog(LOG_DEBUG, "%s(%d): received %d signal", __FUNCTION__, __LINE__, signum);
#endif
}

static void _sigusr1(int signum) {
  syslog(LOG_DEBUG, "%s(%d): SIGUSR1: reloading configuration", __FUNCTION__, __LINE__);

  if (p_reload_config)
    *p_reload_config = 1;

#ifdef ENABLE_CHILLIREDIR
  if (redir_pid)
    kill(redir_pid, SIGUSR1);
#endif

#ifdef ENABLE_CHILLIPROXY
  if (proxy_pid)
    kill(proxy_pid, SIGUSR1);
#endif

#ifdef ENABLE_CHILLIRADSEC
  if (radsec_pid)
    kill(radsec_pid, SIGUSR1);
#endif

}

static void _sighup(int signum) {
  syslog(LOG_DEBUG, "%s(%d): SIGHUP: rereading configuration", __FUNCTION__, __LINE__);

  do_interval = 1;
}

int chilli_handle_signal(void *ctx, int fd) {
  int signo = selfpipe_read();
#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): caught %d via selfpipe", __FUNCTION__, __LINE__,  signo);
#endif
  switch (signo) {
    case SIGCHLD: _sigchld(signo); break;
    case SIGPIPE: _sigvoid(signo); break;
    case SIGHUP:  _sighup(signo);  break;
    case SIGUSR1: _sigusr1(signo); break;
    case SIGTERM:
    case SIGINT:  _sigterm(signo); break;
    default: return signo;
  }
  return 0;
}

void chilli_signals(int *with_term, int *with_hup) {
  selfpipe_trap(SIGCHLD);
  selfpipe_trap(SIGPIPE);

  if (with_hup) {
    p_reload_config = with_hup;
    selfpipe_trap(SIGHUP);
    selfpipe_trap(SIGUSR1);
  }

  if (with_term) {
    p_keep_going = with_term;
    selfpipe_trap(SIGTERM);
    selfpipe_trap(SIGINT);
  }
}

int chilli_binconfig(char *file, size_t flen, pid_t pid) {
  char *env=NULL;
  if (pid == 0) {
    char * bc = _options.binconfig;
    if (bc) {
      snprintf(file, flen, "%s", bc);
      return 0;
    } else {
      pid = chilli_pid;
      if (pid == 0)
	pid = getpid();
    }
  }
  // hack the _opt programm does not use --statedir....
  env=getenv("CHILLISTATEDIR");
  if (env)
  	snprintf(file, flen, "%s/chilli.%d.cfg.bin",env, pid);
  else if (_options.statedir)
  	snprintf(file, flen, "%s/chilli.%d.cfg.bin",_options.statedir, pid);
  else
	snprintf(file, flen, DEFSTATEDIR "/chilli.%d.cfg.bin", pid);
  return 0;
}

int chilli_appconn_run(int (*cb)(struct app_conn_t *, void *), void *d) {
  struct app_conn_t *appconn = firstusedconn;
  while (appconn) {
    if (cb(appconn, d))
      return 1;
    appconn = appconn->next;
  }
  return 0;
}

#ifdef HAVE_LIBRT
static struct timespec startup_real;
static struct timespec startup_mono;
#endif

static time_t start_tick = 0;

time_t mainclock_towall(time_t t) {
#ifdef HAVE_LIBRT
  if (startup_real.tv_sec)
    return startup_real.tv_sec + (t - start_tick);
#endif
  return mainclock.tv_sec;
}

time_t mainclock_wall(void) {
  return mainclock_towall(mainclock.tv_sec);
}

time_t mainclock_tick(void) {
#ifdef HAVE_LIBRT
  struct timespec ts;
#if defined(CLOCK_MONOTONIC)
  clockid_t cid = CLOCK_MONOTONIC;
#else
  clockid_t cid = CLOCK_REALTIME;
#endif
  int res = clock_gettime(cid, &ts);
  if (res == -1 && errno == EINVAL) {
    cid = CLOCK_REALTIME;
    res = clock_gettime(cid, &ts);
  }
  if (res == -1) {
    syslog(LOG_ERR, "%s: clock_gettime()", strerror(errno));
    /* drop through to old time() */
  } else {
    mainclock.tv_sec = ts.tv_sec;
    mainclock.tv_nsec = ts.tv_nsec;
    return mainclock.tv_sec;
  }
#endif
  if (time(&mainclock.tv_sec) == (time_t)-1) {
    syslog(LOG_ERR, "%s: time()", strerror(errno));
  }
  return mainclock.tv_sec;
}

time_t mainclock_now(void) {
  return mainclock.tv_sec;
}

time_t mainclock_rt(void) {
  time_t rt = 0;
#ifdef HAVE_LIBRT
  struct timespec ts;
  clockid_t cid = CLOCK_REALTIME;
  if (clock_gettime(cid, &ts) < 0) {
    syslog(LOG_ERR, "%s: clock_gettime()", strerror(errno));
    /* drop through to old time() */
  } else {
    rt = ts.tv_sec;
    return rt;
  }
#endif
  if (time(&rt) == (time_t)-1) {
    syslog(LOG_ERR, "%s: time()", strerror(errno));
  }
  return rt;
}

int mainclock_rtdiff(time_t past) {
  time_t rt = mainclock_rt();
  return (int) difftime(rt, past);
}

int mainclock_diff(time_t past) {
  return (int) (mainclock.tv_sec - past);
}

uint32_t mainclock_diffu(time_t past) {
  int i = mainclock_diff(past);
  if (i > 0) return (uint32_t) i;
  return 0;
}

void mainclock_tsdiff(struct timespec *out,
		      struct timespec *start,
		      struct timespec *end) {
  if ((end->tv_nsec - start->tv_nsec) < 0) {
    out->tv_sec = end->tv_sec - start->tv_sec-1;
    out->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
  } else {
    out->tv_sec = end->tv_sec - start->tv_sec;
    out->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
}

double mainclock_diffd(struct timespec * past) {
  double d = 0;
  struct timespec tsd;
  mainclock_tsdiff(&tsd, past, &mainclock);
  d  = tsd.tv_nsec;
  d /= 1000000000.0;
  d += tsd.tv_sec;
  return d;
}

uint8_t* chilli_called_station(struct session_state *state) {
#ifdef ENABLE_LOCATION
  if (_options.location_copy_called && state->redir.calledlen) {
    return state->redir.called;
  }
#endif
  return dhcp_nexthop(dhcp);
}

static void set_sessionid(struct app_conn_t *appconn, char full) {
  appconn->rt = (int) mainclock_rt();

  snprintf(appconn->s_state.sessionid,
		sizeof(appconn->s_state.sessionid),
		"%.8lld%.8x", (long long int)appconn->rt, appconn->unit);

  appconn->s_state.redir.classlen = 0;
  appconn->s_state.redir.statelen = 0;

#ifdef ENABLE_SESSIONID
  if (full) {
    uint8_t * his = appconn->hismac;
    uint8_t * called = dhcp_nexthop(dhcp);
    snprintf(appconn->s_state.chilli_sessionid,
		  sizeof(appconn->s_state.chilli_sessionid),
		  "SES-"
		  "%.2X%.2X%.2X%.2X%.2X%.2X-"
		  "%.2X%.2X%.2X%.2X%.2X%.2X-"
		  "%.8lx%.8x",
		  MAC_ARG(his), MAC_ARG(called),
		  appconn->rt, appconn->unit);
  }
#endif
}

/* Used to write process ID to file. Assume someone else will delete */
static void log_pid(char *pidfile) {
  FILE *file;
  mode_t oldmask;

  oldmask = umask(022);
  file = fopen(pidfile, "w");
  umask(oldmask);
  if(!file) return;
  fprintf(file, "%d\n", getpid());
  fclose(file);
}

#ifdef ENABLE_LEAKYBUCKET
static inline void leaky_bucket_init(struct app_conn_t *conn) {

  if (_options.bwbucketupsize) {
    conn->s_state.bucketupsize = _options.bwbucketupsize;
  } else {
#ifdef BUCKET_SIZE
    conn->s_state.bucketupsize = BUCKET_SIZE;
#else

    conn->s_state.bucketupsize =
        conn->s_params.bandwidthmaxup / 8 * BUCKET_TIME;

    if (conn->s_state.bucketupsize < BUCKET_SIZE_MIN)
      conn->s_state.bucketupsize = BUCKET_SIZE_MIN;
#endif
  }

  if (_options.bwbucketdnsize) {
    conn->s_state.bucketdownsize = _options.bwbucketdnsize;
  } else {
#ifdef BUCKET_SIZE
    conn->s_state.bucketdownsize = BUCKET_SIZE;
#else

    conn->s_state.bucketdownsize =
        conn->s_params.bandwidthmaxdown / 8 * BUCKET_TIME;

    if (conn->s_state.bucketdownsize < BUCKET_SIZE_MIN)
      conn->s_state.bucketdownsize = BUCKET_SIZE_MIN;
#endif
  }

  if (_options.bwbucketminsize > 0) {
    if (conn->s_state.bucketupsize < _options.bwbucketminsize)
      conn->s_state.bucketupsize = _options.bwbucketminsize;
    if (conn->s_state.bucketdownsize < _options.bwbucketminsize)
      conn->s_state.bucketdownsize = _options.bwbucketminsize;
  }
}

/* Perform leaky bucket on up- and downlink traffic */
static inline int
leaky_bucket(struct app_conn_t *conn,
	     uint64_t octetsup, uint64_t octetsdown) {
  int result = 0;
  uint64_t upbytes=0, dnbytes=0;
  long double timediff;

  timediff = mainclock_diffd(&conn->s_state.last_bw_time);

  if (conn->s_params.bandwidthmaxup) {
    upbytes = (uint64_t) ((timediff * conn->s_params.bandwidthmaxup) / 8);

    if (!conn->s_state.bucketupsize) {
      leaky_bucket_init(conn);
    }

    if (conn->s_state.bucketup > upbytes) {
      conn->s_state.bucketup -= upbytes;
    }
    else {
      conn->s_state.bucketup = 0;
    }

    if ((conn->s_state.bucketup + octetsup) >
	conn->s_state.bucketupsize) {
      if (_options.debug)
	syslog(LOG_DEBUG, "%s(%d): Leaky bucket dropping upload overflow from "MAC_FMT, __FUNCTION__, __LINE__,
               MAC_ARG(conn->hismac));
      result = -1;
    }
    else {
      conn->s_state.bucketup += octetsup;
    }
  }

  if (conn->s_params.bandwidthmaxdown) {
    dnbytes = (uint64_t) ((timediff * conn->s_params.bandwidthmaxdown) / 8);

    if (!conn->s_state.bucketdownsize) {
      leaky_bucket_init(conn);
    }

    if (conn->s_state.bucketdown > dnbytes) {
      conn->s_state.bucketdown -= dnbytes;
    }
    else {
      conn->s_state.bucketdown = 0;
    }

    if ((conn->s_state.bucketdown + octetsdown) >
	conn->s_state.bucketdownsize) {
      if (_options.debug)
	syslog(LOG_DEBUG, "%s(%d): Leaky bucket dropping download overflow to "MAC_FMT, __FUNCTION__, __LINE__,
               MAC_ARG(conn->hismac));
      result = -1;
    }
    else {
      conn->s_state.bucketdown += octetsdown;
    }
  }

#if(0)
  if (_options.debug &&
      (conn->s_params.bandwidthmaxup || conn->s_params.bandwidthmaxdown))
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Leaky bucket: bucketup: %lld/%lld, " __FUNCTION__, __LINE__,
             "bucketdown: %lld/%lld, up: %lld/(%lld), down: %lld/(%lld)",
             conn->s_state.bucketup, conn->s_state.bucketupsize,
             conn->s_state.bucketdown, conn->s_state.bucketdownsize,
             octetsup, upbytes, octetsdown, dnbytes);
#endif

  conn->s_state.last_bw_time.tv_sec = mainclock.tv_sec;
  conn->s_state.last_bw_time.tv_nsec = mainclock.tv_nsec;

  return result;
}
#endif

void set_env(char *name, char type, void *value, int len) {
  char *v=0;
  char s[1024];

  memset(s,0,sizeof(s));

  switch(type) {

    case VAL_IN_ADDR:
      strlcpy(s, inet_ntoa(*(struct in_addr *)value), sizeof(s));
      v = s;
      break;

    case VAL_MAC_ADDR:
      {
        uint8_t * mac = (uint8_t*)value;
        snprintf(s, sizeof(s), MAC_FMT, MAC_ARG(mac));
        v = s;
      }
      break;

    case VAL_ULONG:
      snprintf(s, sizeof(s), "%ld", (long int)*(uint32_t *)value);
      v = s;
      break;

    case VAL_ULONG64:
      snprintf(s, sizeof(s), "%ld", (long int)*(uint64_t *)value);
      v = s;
      break;

    case VAL_USHORT:
      snprintf(s, sizeof(s), "%d", (int)(*(uint16_t *)value));
      v = s;
      break;

    case VAL_STRING:
      if (len > 0) {
        if (len > sizeof(s) - 1)
          len = sizeof(s) - 1;
        memcpy(s, (char*)value, len);
        s[len]=0;
        v = s;
      } else {
        v = (char*)value;
      }
      break;
  }

  if (name != NULL && v != NULL) {
    if (setenv(name, v, 1) != 0) {
      syslog(LOG_ERR, "%s: setenv(%s, %s, 1) did not return 0!", strerror(errno), name, v);
    }
  }
}

int runscript(struct app_conn_t *appconn, char* script,
	      char *loc, char *oloc) {
  int status;
  uint32_t sessiontime;

  if ((status = chilli_fork(CHILLI_PROC_SCRIPT, script)) < 0) {
    syslog(LOG_ERR, "%s: forking %s", strerror(errno), script);
    return 0;
  }

  if (status > 0) { /* Parent */
    return 0;
  }

#ifdef ENABLE_LAYER3
  if (_options.layer3)
    set_env("LAYER3", VAL_STRING, "1", 0);
#endif
  set_env("DEV", VAL_STRING, tun(tun, 0).devname, 0);
  set_env("NET", VAL_IN_ADDR, &appconn->net, 0);
  set_env("MASK", VAL_IN_ADDR, &appconn->mask, 0);
  set_env("ADDR", VAL_IN_ADDR, &appconn->ourip, 0);
  set_env("USER_NAME", VAL_STRING, appconn->s_state.redir.username, 0);
  set_env("NAS_IP_ADDRESS", VAL_IN_ADDR,&_options.radiuslisten, 0);
  set_env("SERVICE_TYPE", VAL_STRING, "1", 0);
  set_env("FRAMED_IP_ADDRESS", VAL_IN_ADDR, &appconn->hisip, 0);
  set_env("FILTER_ID", VAL_STRING, appconn->s_params.filteridbuf, 0);
  set_env("STATE", VAL_STRING, appconn->s_state.redir.statebuf, appconn->s_state.redir.statelen);
  set_env("CLASS", VAL_STRING, appconn->s_state.redir.classbuf, appconn->s_state.redir.classlen);
  set_env("CUI", VAL_STRING, appconn->s_state.redir.cuibuf, appconn->s_state.redir.cuilen);
  set_env("SESSION_TIMEOUT", VAL_ULONG64, &appconn->s_params.sessiontimeout, 0);
  set_env("IDLE_TIMEOUT", VAL_ULONG, &appconn->s_params.idletimeout, 0);
  set_env("CALLING_STATION_ID", VAL_MAC_ADDR, appconn->hismac, 0);
  set_env("CALLED_STATION_ID", VAL_MAC_ADDR, chilli_called_station(&appconn->s_state), 0);
  set_env("NAS_ID", VAL_STRING, _options.radiusnasid, 0);
  set_env("NAS_PORT_TYPE", VAL_STRING, "19", 0);
  set_env("ACCT_SESSION_ID", VAL_STRING, appconn->s_state.sessionid, 0);
  set_env("ACCT_INTERIM_INTERVAL", VAL_USHORT, &appconn->s_params.interim_interval, 0);
  set_env("WISPR_LOCATION_ID", VAL_STRING, _options.radiuslocationid, 0);
  set_env("WISPR_LOCATION_NAME", VAL_STRING, _options.radiuslocationname, 0);
  set_env("WISPR_BANDWIDTH_MAX_UP", VAL_ULONG, &appconn->s_params.bandwidthmaxup, 0);
  set_env("WISPR_BANDWIDTH_MAX_DOWN", VAL_ULONG, &appconn->s_params.bandwidthmaxdown, 0);
  /*set_env("WISPR-SESSION_TERMINATE_TIME", VAL_USHORT, &appconn->sessionterminatetime, 0);*/
  set_env("COOVACHILLI_MAX_INPUT_OCTETS", VAL_ULONG64, &appconn->s_params.maxinputoctets, 0);
  set_env("COOVACHILLI_MAX_OUTPUT_OCTETS", VAL_ULONG64, &appconn->s_params.maxoutputoctets, 0);
  set_env("COOVACHILLI_MAX_TOTAL_OCTETS", VAL_ULONG64, &appconn->s_params.maxtotaloctets, 0);
  set_env("INPUT_OCTETS", VAL_ULONG64, &appconn->s_state.input_octets, 0);
  set_env("OUTPUT_OCTETS", VAL_ULONG64, &appconn->s_state.output_octets, 0);
  set_env("INPUT_PACKETS", VAL_ULONG64, &appconn->s_state.input_packets, 0);
  set_env("OUTPUT_PACKETS", VAL_ULONG64, &appconn->s_state.output_packets, 0);
  sessiontime = mainclock_diffu(appconn->s_state.start_time);
  set_env("SESSION_TIME", VAL_ULONG, &sessiontime, 0);
  sessiontime = mainclock_diffu(appconn->s_state.last_up_time);
  set_env("IDLE_TIME", VAL_ULONG, &sessiontime, 0);

  if (loc) {
    set_env("LOCATION", VAL_STRING, loc, 0);
  }
  if (oloc) {
    set_env("OLD_LOCATION", VAL_STRING, oloc, 0);
  }

  if (appconn->s_state.terminate_cause)
    set_env("TERMINATE_CAUSE", VAL_ULONG,
	    &appconn->s_state.terminate_cause, 0);

  if (execl(
#ifdef ENABLE_CHILLISCRIPT
          SBINDIR "/chilli_script", SBINDIR "/chilli_script", _options.binconfig,
#else
          script,
#endif
          script, (char *) 0) != 0) {
    syslog(LOG_ERR, "%s: exec %s failed", strerror(errno), script);
  }

  exit(0);
}

/***********************************************************
 *
 * Functions handling uplink protocol authentication.
 * Called in response to radius access request response.
 *
 ***********************************************************/

static int newip(struct ippoolm_t **ipm, struct in_addr *hisip, uint8_t *hismac) {

#ifdef ENABLE_UAMANYIP
  struct in_addr tmpip;

  if (_options.autostatip && hismac) {
    if (!hisip) hisip = &tmpip;
    hisip->s_addr = htonl((_options.autostatip % 255) * 0x1000000 +
			  hismac[3] * 0x10000 +
			  hismac[4] * 0x100 +
			  hismac[5]);
  }
#endif

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): newip %s", __FUNCTION__, __LINE__,  inet_ntoa(*hisip));

  if (ippool_newip(ippool, ipm, hisip, 1)) {
    if (ippool_newip(ippool, ipm, hisip, 0)) {
      syslog(LOG_ERR, "Failed to allocate either static or dynamic IP address");
      return -1;
    }
  }

  return 0;
}


/*
 * A few functions to manage connections
 */

static int initconn(void) {
  checktime = rereadtime = mainclock.tv_sec;
  return 0;
}

int chilli_new_conn(struct app_conn_t **conn) {
  int n;

  if (!firstfreeconn) {

    if (connections == _options.max_clients) {
      syslog(LOG_ERR, "reached max connections %d!", _options.max_clients);
      return -1;
    }

    n = ++connections;

    if (!(*conn = calloc(1, sizeof(struct app_conn_t)))) {
      syslog(LOG_ERR, "Out of memory!");
      connections--;
      return -1;
    }

  } else {

    *conn = firstfreeconn;
    n = (*conn)->unit;

    /* Remove from link of free */
    if (firstfreeconn->next) {
      firstfreeconn->next->prev = NULL;
      firstfreeconn = firstfreeconn->next;
    }
    else { /* Took the last one */
      firstfreeconn = NULL;
      lastfreeconn = NULL;
    }

    /* Initialise structures */
    memset(*conn, 0, sizeof(struct app_conn_t));
  }

  /* Initalise connection with default options */
  session_param_defaults(&(*conn)->s_params);

  /* Insert into link of used */
  if (firstusedconn) {
    firstusedconn->prev = *conn;
    (*conn)->next = firstusedconn;
  }
  else { /* First insert */
    lastusedconn = *conn;
  }

  firstusedconn = *conn;

  (*conn)->inuse = 1;
  (*conn)->unit = n;

  return 0; /* Success */
}

int static freeconn(struct app_conn_t *conn) {
  int n = conn->unit;

#ifdef ENABLE_GARDENACCOUNTING
  if (_options.uamgardendata) {
    acct_req(ACCT_GARDEN, conn, RADIUS_STATUS_TYPE_STOP);
  }
#endif

#ifdef WITH_PATRICIA
  if (conn->ptree)
    patricia_destroy (conn->ptree, free);
#endif

#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
  /*remove from location list (if we have a location/are in list) !!??*/
  if (conn->loc_search_node!=NULL) location_close_conn(conn,1);
#endif

  /* Remove from link of used */
  if ((conn->next) && (conn->prev)) {
    conn->next->prev = conn->prev;
    conn->prev->next = conn->next;
  }
  else if (conn->next) { /* && prev == 0 */
    conn->next->prev = NULL;
    firstusedconn = conn->next;
  }
  else if (conn->prev) { /* && next == 0 */
    conn->prev->next = NULL;
    lastusedconn = conn->prev;
  }
  else { /* if ((next == 0) && (prev == 0)) */
    firstusedconn = NULL;
    lastusedconn = NULL;
  }

  /* Initialise structures */
  memset(conn, 0, sizeof(struct app_conn_t));
  conn->unit = n;

  /* Insert into link of free */
  if (firstfreeconn) {
    firstfreeconn->prev = conn;
  }
  else { /* First insert */
    lastfreeconn = conn;
  }

  conn->next = firstfreeconn;
  firstfreeconn = conn;

  return 0;
}

int chilli_getconn(struct app_conn_t **conn, uint32_t ip,
		   uint32_t nasip, uint32_t nasport) {

  struct app_conn_t *appconn = firstusedconn;

  while (appconn) {

    if (!appconn->inuse) {
      syslog(LOG_ERR, "Connection with inuse == 0!");
    }

    if (ip && appconn->hisip.s_addr == ip) {
      *conn = appconn;
      return 0;
    }

    if (nasip && nasport &&
	(appconn->nasip == nasip) && (appconn->nasport == nasport)) {
      *conn = appconn;
      return 0;
    }

    appconn = appconn->next;
  }

  return -1; /* Not found */
}

static int dnprot_terminate(struct app_conn_t *appconn) {
  appconn->s_state.authenticated = 0;
#ifdef ENABLE_SESSIONSTATE
  appconn->s_state.session_state = 0;
#endif
  if (appconn->s_params.url[0] &&
      appconn->s_params.flags & UAM_CLEAR_URL) {
    appconn->s_params.flags &= ~UAM_CLEAR_URL;
    appconn->s_params.url[0] = 0;
  }
#ifdef ENABLE_LEAKYBUCKET
  appconn->s_state.bucketup = 0;
  appconn->s_state.bucketdown = 0;
  appconn->s_state.bucketupsize = 0;
  appconn->s_state.bucketdownsize = 0;
#endif
#ifdef HAVE_NETFILTER_COOVA
  if (_options.kname) {
    kmod_coova_update(appconn);
  }
#endif
#ifdef ENABLE_LAYER3
  if (!_options.layer3)
#endif
    switch (appconn->dnprot) {
      case DNPROT_WPA:
#ifdef ENABLE_EAPOL
      case DNPROT_EAPOL:
#endif
        if (appconn->dnlink)
          ((struct dhcp_conn_t*) appconn->dnlink)->authstate = DHCP_AUTH_NONE;
        break;
      case DNPROT_MAC:
      case DNPROT_UAM:
      case DNPROT_DHCP_NONE:
      case DNPROT_NULL:
        if (appconn->dnlink)
          ((struct dhcp_conn_t*) appconn->dnlink)->authstate = DHCP_AUTH_DNAT;
        break;
#ifdef ENABLE_LAYER3
      case DNPROT_LAYER3:
        break;
#endif
      default:
        syslog(LOG_ERR, "Unknown downlink protocol");
        break;
    }
  return 0;
}



/* Check for:
 * - Session-Timeout
 * - Idle-Timeout
 * - Interim-Interim accounting
 * - Reread configuration file and DNS entries
 */

void session_interval(struct app_conn_t *conn) {
  uint32_t sessiontime;
  uint32_t idletime;
  uint32_t interimtime;

  sessiontime = mainclock_diffu(conn->s_state.start_time);
  idletime    = mainclock_diffu(conn->s_state.last_up_time);
  interimtime = mainclock_diffu(conn->s_state.interim_time);

  if (conn->s_state.authenticated == 1) {
    if ((conn->s_params.sessiontimeout) &&
	(sessiontime > conn->s_params.sessiontimeout)) {
#ifdef ENABLE_SESSIONSTATE
      conn->s_state.session_state =
          RADIUS_VALUE_COOVACHILLI_SESSION_TIMEOUT_REACHED;
#endif
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT);
    }
    else if ((conn->s_params.sessionterminatetime) &&
	     (mainclock_rtdiff(conn->s_params.sessionterminatetime) > 0)) {
#ifdef ENABLE_SESSIONSTATE
      conn->s_state.session_state =
          RADIUS_VALUE_COOVACHILLI_SESSION_LOGOUT_TIME_REACHED;
#endif
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT);
    }
    else if ((conn->s_params.idletimeout) &&
	     (idletime > conn->s_params.idletimeout)) {
#ifdef ENABLE_SESSIONSTATE
      conn->s_state.session_state =
          RADIUS_VALUE_COOVACHILLI_SESSION_IDLE_TIMEOUT_REACHED;
#endif
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT);
    }
    else if ((conn->s_params.maxinputoctets) &&
	     (conn->s_state.input_octets > conn->s_params.maxinputoctets)) {
#ifdef ENABLE_SESSIONSTATE
      conn->s_state.session_state =
          RADIUS_VALUE_COOVACHILLI_SESSION_IN_DATALIMIT_REACHED;
#endif
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT);
    }
    else if ((conn->s_params.maxoutputoctets) &&
	     (conn->s_state.output_octets > conn->s_params.maxoutputoctets)) {
#ifdef ENABLE_SESSIONSTATE
      conn->s_state.session_state =
          RADIUS_VALUE_COOVACHILLI_SESSION_OUT_DATALIMIT_REACHED;
#endif
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT);
    }
    else if ((conn->s_params.maxtotaloctets) &&
	     ((conn->s_state.input_octets + conn->s_state.output_octets) >
	      conn->s_params.maxtotaloctets)) {
#ifdef ENABLE_SESSIONSTATE
      conn->s_state.session_state =
          RADIUS_VALUE_COOVACHILLI_SESSION_TOTAL_DATALIMIT_REACHED;
#endif
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT);
    }
    else if ((conn->s_params.interim_interval) &&
	     (interimtime >= conn->s_params.interim_interval)) {

#ifdef ENABLE_MODULES
      { int i;
	for (i=0; i < MAX_MODULES; i++) {
	  if (!_options.modules[i].name[0]) break;
	  if (_options.modules[i].ctx) {
	    struct chilli_module *m =
                (struct chilli_module *)_options.modules[i].ctx;
	    if (m->session_update)
              m->session_update(conn);
	  }
	}
      }
#endif

      acct_req(ACCT_USER, conn, RADIUS_STATUS_TYPE_INTERIM_UPDATE);
    }
  }
#ifdef ENABLE_GARDENACCOUNTING
  interimtime = mainclock_diffu(conn->s_state.garden_interim_time);
  if (_options.uamgardendata &&
      _options.definteriminterval &&
      interimtime >= _options.definteriminterval) {
    acct_req(ACCT_GARDEN, conn, RADIUS_STATUS_TYPE_INTERIM_UPDATE);
  }
#endif
}

static int checkconn(void) {
  struct app_conn_t *conn;
  struct dhcp_conn_t* dhcpconn;
  uint32_t checkdiff;
  uint32_t rereaddiff;

#ifdef HAVE_NETFILTER_COOVA
  if (_options.kname) {
    kmod_coova_sync();
  }
#endif

  checkdiff = mainclock_diffu(checktime);

  if (checkdiff < CHECK_INTERVAL)
    return 0;

  checktime = mainclock.tv_sec;

  if (admin_session.s_state.authenticated) {
    session_interval(&admin_session);
  }

  for (conn = firstusedconn; conn; conn=conn->next) {
    if (conn->inuse != 0) {
      if (
#ifdef ENABLE_LAYER3
              !_options.layer3 &&
#endif
              !(dhcpconn = (struct dhcp_conn_t *)conn->dnlink)) {
	syslog(LOG_WARNING, "No downlink protocol");
	continue;
      }
      session_interval(conn);
    }
  }

  /* Reread configuration file and recheck DNS */
  if (_options.interval) {
    rereaddiff = mainclock_diffu(rereadtime);
    if (rereaddiff >= _options.interval) {
      rereadtime = mainclock.tv_sec;
      do_interval = 1;
    }
  }

  return 0;
}

void chilli_freeconn(void) {
  struct app_conn_t *conn, *c;
  struct dhcp_conn_t *d = NULL;

  for (conn = firstusedconn; conn; ) {
    c = conn;
    conn = conn->next;
    d = (struct dhcp_conn_t *)c->dnlink;
    if (d) d->peer = NULL;
    free(c);
  }

  for (conn = firstfreeconn; conn; ) {
    c = conn;
    conn = conn->next;
    free(c);
  }
}

/* Kill all connections and send Radius Acct Stop */
int static killconn(void) {
  struct app_conn_t *conn;

  for (conn = firstusedconn; conn; conn = conn->next) {
    if ((conn->inuse != 0) && (conn->s_state.authenticated == 1)) {
      terminate_appconn(conn, RADIUS_TERMINATE_CAUSE_NAS_REBOOT);
    }
  }

  if (admin_session.s_state.authenticated) {
    admin_session.s_state.terminate_cause = RADIUS_TERMINATE_CAUSE_NAS_REBOOT;
    acct_req(ACCT_USER, &admin_session, RADIUS_STATUS_TYPE_STOP);
  }

#ifdef ENABLE_ACCOUNTING_ONOFF
  acct_req(ACCT_USER, &admin_session, RADIUS_STATUS_TYPE_ACCOUNTING_OFF);
#endif

  chilli_freeconn();
  return 0;
}

/* Compare a MAC address to the addresses given in the macallowed option */
int static maccmp(unsigned char *mac) {
  int i;

  for (i=0; i<_options.macoklen; i++)
    if (!memcmp(mac, _options.macok[i], PKT_ETH_ALEN))
      return 0;

  return -1;
}

int chilli_req_attrs(struct radius_t *radius,
		     struct radius_packet_t *pack,
		     acct_type type,
		     uint32_t service_type,
		     uint8_t status_type,
		     uint32_t port,
		     uint8_t *hismac,
		     struct in_addr *hisip,
		     struct session_state *state) {
  char *sessionid = state->sessionid;
  char mac[MACSTRLEN+1];

  switch(pack->code) {
    case RADIUS_CODE_ACCESS_REQUEST:

      if (_options.radiusoriginalurl) {
        if (state->redir.userurl[0])
          radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
                         RADIUS_VENDOR_COOVACHILLI,
                         RADIUS_ATTR_COOVACHILLI_ORIGINALURL,
                         0, (uint8_t *) state->redir.userurl,
                         strlen(state->redir.userurl));

#ifdef ENABLE_USERAGENT
        if (state->redir.useragent[0])
          radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
                         RADIUS_VENDOR_COOVACHILLI,
                         RADIUS_ATTR_COOVACHILLI_USER_AGENT,
                         0, (uint8_t *) state->redir.useragent,
                         strlen(state->redir.useragent));
#endif

#ifdef ENABLE_ACCEPTLANGUAGE
        if (state->redir.acceptlanguage[0])
          radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
                         RADIUS_VENDOR_COOVACHILLI,
                         RADIUS_ATTR_COOVACHILLI_ACCEPT_LANGUAGE,
                         0, (uint8_t *) state->redir.acceptlanguage,
                         strlen(state->redir.acceptlanguage));
#endif
      }

      break;
    default:
      break;
  }

  if (service_type)
    radius_addattr(radius, pack, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		   service_type, NULL, 0);

  if (status_type)
    radius_addattr(radius, pack, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0,
		   status_type, NULL, 0);

#ifdef ENABLE_IEEE8021Q
  if (_options.ieee8021q && state->tag8021q) {
    radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI, RADIUS_ATTR_COOVACHILLI_VLAN_ID,
		   (uint32_t)ntohs(state->tag8021q & PKT_8021Q_MASK_VID), 0, 0);
  }
#endif

#ifdef ENABLE_GARDENACCOUNTING
  switch(type) {
    case ACCT_GARDEN:
      sessionid = state->garden_sessionid;
      break;
    case ACCT_USER:
    default:
      break;
  }
#endif

  if (*sessionid) {
    radius_addattr(radius, pack, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
		   (uint8_t *)sessionid, strlen(sessionid));
  }

#ifdef ENABLE_SESSIONID
  if (state->chilli_sessionid[0]) {
    radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI,
		   RADIUS_ATTR_COOVACHILLI_SESSION_ID,
		   0, (uint8_t *) state->chilli_sessionid,
		   strlen(state->chilli_sessionid));
  }
#endif

#ifdef ENABLE_APSESSIONID
  if (state->ap_sessionid[0]) {
    radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI,
		   RADIUS_ATTR_COOVACHILLI_AP_SESSION_ID,
		   0, (uint8_t *) state->ap_sessionid,
		   strlen(state->ap_sessionid));
  }
#endif

  switch(type) {
#ifdef ENABLE_GARDENACCOUNTING
    case ACCT_GARDEN:
      break;
#endif
    case ACCT_USER:
      if (state->redir.classlen) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): RADIUS Request + Class(%zu)", __FUNCTION__, __LINE__,  state->redir.classlen);
        radius_addattr(radius, pack,
                       RADIUS_ATTR_CLASS, 0, 0, 0,
                       state->redir.classbuf,
                       state->redir.classlen);
      }

      if (state->redir.cuilen > 1) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): RADIUS Request + CUI(%zu)", __FUNCTION__, __LINE__, state->redir.cuilen);
        radius_addattr(radius, pack,
                       RADIUS_ATTR_CHARGEABLE_USER_IDENTITY, 0, 0, 0,
                       state->redir.cuibuf,
                       state->redir.cuilen);
      }

      if (state->redir.statelen) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): RADIUS Request + State(%d)", __FUNCTION__, __LINE__, state->redir.statelen);
        radius_addattr(radius, pack,
                       RADIUS_ATTR_STATE, 0, 0, 0,
                       state->redir.statebuf,
                       state->redir.statelen);
      }
      break;
  }

  if (hisip && hisip->s_addr) {
    radius_addattr(radius, pack, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
		   ntohl(hisip->s_addr), NULL, 0);
  }

  radius_addattr(radius, pack, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
		 _options.radiusnasporttype, NULL, 0);

  if (port) {
    char portid[16+1];
    snprintf(portid, sizeof(portid), "%.8d", port);

    radius_addattr(radius, pack, RADIUS_ATTR_NAS_PORT, 0, 0,
		   port, NULL, 0);

    radius_addattr(radius, pack, RADIUS_ATTR_NAS_PORT_ID, 0, 0, 0,
		   (uint8_t *) portid, strlen(portid));
  }

  /* Include his MAC address */
  if (hismac) {
    snprintf(mac, sizeof(mac), MAC_FMT, MAC_ARG(hismac));

    radius_addattr(radius, pack, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0,
		   (uint8_t*) mac, MACSTRLEN);
  }

  radius_addcalledstation(radius, pack, state);

  radius_addnasip(radius, pack);

#ifdef ENABLE_PROXYVSA
  radius_addvsa(pack, &state->redir);
#endif

#ifdef ENABLE_LOCATION
  if (state->location[0]) {
    radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI,
		   RADIUS_ATTR_COOVACHILLI_LOCATION,
		   0, (uint8_t *) state->location,
		   strlen(state->location));
    if (state->location_changes) {
      radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		     RADIUS_VENDOR_COOVACHILLI,
		     RADIUS_ATTR_COOVACHILLI_LOCATION_CHANGE_COUNT,
		     (uint32_t) state->location_changes, 0, 0);
    }
  }
#endif

  /* Include NAS-Identifier if given in configuration options */
  if (_options.radiusnasid) {
    radius_addattr(radius, pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		   (uint8_t*) _options.radiusnasid, strlen(_options.radiusnasid));
  }

  if (_options.radiuslocationid) {
    radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_ID, 0,
		   (uint8_t*) _options.radiuslocationid,
		   strlen(_options.radiuslocationid));
  }

  if (_options.radiuslocationname) {
    radius_addattr(radius, pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_NAME, 0,
		   (uint8_t*) _options.radiuslocationname,
		   strlen(_options.radiuslocationname));
  }

#ifdef ENABLE_MODULES
  { int i;
    for (i=0; i < MAX_MODULES; i++) {
      if (!_options.modules[i].name[0]) break;
      if (_options.modules[i].ctx) {
	struct chilli_module *m =
            (struct chilli_module *)_options.modules[i].ctx;
	if (m->radius_handler) {
	  int res = m->radius_handler(radius, 0, pack, 0);
	  switch (res) {
            case CHILLI_RADIUS_OK:
              break;
            default:
              return 0;
	  }
	}
      }
    }
  }
#endif

  return 0;
}

int chilli_auth_radius(struct radius_t *radius) {
  struct radius_packet_t radius_pack;
  int ret = -1;

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return ret;
  }

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 (uint8_t *)_options.adminuser, strlen(_options.adminuser));

  if (_options.adminpasswd)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		   (uint8_t *)_options.adminpasswd, strlen(_options.adminpasswd));

  chilli_req_attrs(radius, &radius_pack,
		   ACCT_USER,
		   RADIUS_SERVICE_TYPE_ADMIN_USER, 0,
		   0, 0, 0,
		   &admin_session.s_state);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  return radius_req(radius, &radius_pack, &admin_session);
}

int static auth_radius(struct app_conn_t *appconn,
		       char *username, char *password,
		       uint8_t *dhcp_pkt, size_t dhcp_len) {
  struct dhcp_conn_t *dhcpconn = (struct dhcp_conn_t *)appconn->dnlink;
  struct radius_packet_t radius_pack;
  char mac[MACSTRLEN+1];

  uint32_t service_type = RADIUS_SERVICE_TYPE_LOGIN;

  if (!radius) return -1;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Starting radius authentication", __FUNCTION__, __LINE__);

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  /* Include his MAC address */
  snprintf(mac, sizeof(mac), MAC_FMT, MAC_ARG(dhcpconn->hismac));

  if (!username) {

    service_type = RADIUS_SERVICE_TYPE_FRAMED;

    strlcpy(appconn->s_state.redir.username, mac, USERNAMESIZE);

    if (_options.macsuffix) {
      size_t ulen = strlen(appconn->s_state.redir.username);
      strlcpy(appconn->s_state.redir.username + ulen,
              _options.macsuffix, USERNAMESIZE - ulen);
    }

    username = appconn->s_state.redir.username;

  } else {

    strlcpy(appconn->s_state.redir.username, username, USERNAMESIZE);

  }

  if (!password) {
    password = _options.macpasswd;
    if (!password) {
      password = appconn->s_state.redir.username;
    }
  }

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 (uint8_t *) username, strlen(username));

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		 (uint8_t *) password, strlen(password));

  appconn->authtype = PAP_PASSWORD;

#ifdef ENABLE_DHCPRADIUS
  if (_options.dhcpradius && dhcp_pkt) {
    struct dhcp_tag_t *tag = 0;
    struct pkt_udphdr_t *udph = pkt_udphdr(dhcp_pkt);
    struct dhcp_packet_t *dhcppkt = pkt_dhcppkt(dhcp_pkt);

#define maptag(OPT,VSA) tag=0;                                          \
    if (!dhcp_gettag(dhcppkt, ntohs(udph->len)-PKT_UDP_HLEN, &tag, OPT)) { \
      radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,	\
                     RADIUS_VENDOR_COOVACHILLI, VSA, 0,                 \
                     (uint8_t *) tag->v, tag->l); }
    /*
     *  Mapping of DHCP options to RADIUS Vendor Specific Attributes
     */
    maptag( DHCP_OPTION_PARAMETER_REQUEST_LIST,
	    RADIUS_ATTR_COOVACHILLI_DHCP_PARAMETER_REQUEST_LIST );
    maptag( DHCP_OPTION_VENDOR_CLASS_IDENTIFIER,
	    RADIUS_ATTR_COOVACHILLI_DHCP_VENDOR_CLASS_ID );
    maptag( DHCP_OPTION_CLIENT_IDENTIFIER,
	    RADIUS_ATTR_COOVACHILLI_DHCP_CLIENT_ID );
    maptag( DHCP_OPTION_CLIENT_FQDN,
	    RADIUS_ATTR_COOVACHILLI_DHCP_CLIENT_FQDN );
    maptag( DHCP_OPTION_HOSTNAME,
	    RADIUS_ATTR_COOVACHILLI_DHCP_HOSTNAME );
#undef maptag
  }
#endif

  chilli_req_attrs(radius, &radius_pack,
		   ACCT_USER, service_type, 0,
		   appconn->unit, appconn->hismac,
		   &appconn->hisip, &appconn->s_state);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  return radius_req(radius, &radius_pack, appconn);
}


#ifdef ENABLE_RADPROXY
/*********************************************************
 * radius proxy functions
 * Used to send a response to a received radius request
 *********************************************************/

/* Reply with an access reject */
int static radius_access_reject(struct app_conn_t *conn) {
  struct radius_packet_t radius_pack;

  conn->radiuswait = 0;

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REJECT)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  radius_pack.id = conn->radiusid;
  radius_resp(radius, &radius_pack, &conn->radiuspeer, conn->authenticator);
  return 0;
}

/* Reply with an access challenge */
int static radius_access_challenge(struct app_conn_t *conn) {
  struct radius_packet_t radius_pack;
  size_t offset = 0;
  size_t eaplen = 0;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Sending RADIUS AccessChallenge to client", __FUNCTION__, __LINE__);

  conn->radiuswait = 0;

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_CHALLENGE)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  radius_pack.id = conn->radiusid;

  /* Include EAP */
  do {
    if ((conn->challen - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = conn->challen - offset;

    if (radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		       conn->chal + offset, eaplen)) {
      syslog(LOG_ERR, "radius_default_pack() failed");
      return -1;
    }
    offset += eaplen;
  }
  while (offset < conn->challen);

  if (conn->s_state.redir.statelen) {
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_STATE, 0, 0, 0,
		   conn->s_state.redir.statebuf,
		   conn->s_state.redir.statelen);
  }

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  radius_resp(radius, &radius_pack, &conn->radiuspeer, conn->authenticator);
  return 0;
}

/* Send off an access accept */

int static radius_access_accept(struct app_conn_t *conn) {
  struct radius_packet_t radius_pack;
  size_t offset = 0;
  size_t eaplen = 0;

  uint8_t mppekey[RADIUS_ATTR_VLEN];
  size_t mppelen;

  conn->radiuswait = 0;

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_ACCEPT)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  radius_pack.id = conn->radiusid;

  /* Include EAP (if present) */
  offset = 0;
  while (offset < conn->challen) {
    if ((conn->challen - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = conn->challen - offset;

    radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		   conn->chal + offset, eaplen);

    offset += eaplen;
  }

  if (conn->sendlen) {
    radius_keyencode(radius, mppekey, RADIUS_ATTR_VLEN,
		     &mppelen, conn->sendkey,
		     conn->sendlen, conn->authenticator,
		     radius->proxysecret, radius->proxysecretlen);

    radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_MS, RADIUS_ATTR_MS_MPPE_SEND_KEY, 0,
		   (uint8_t *)mppekey, mppelen);
  }

  if (conn->recvlen) {
    radius_keyencode(radius, mppekey, RADIUS_ATTR_VLEN,
		     &mppelen, conn->recvkey,
		     conn->recvlen, conn->authenticator,
		     radius->proxysecret, radius->proxysecretlen);

    radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_MS, RADIUS_ATTR_MS_MPPE_RECV_KEY, 0,
		   (uint8_t *)mppekey, mppelen);
  }

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  radius_resp(radius, &radius_pack, &conn->radiuspeer, conn->authenticator);
  return 0;
}
#endif

/*********************************************************
 * radius accounting functions
 * Used to send accounting request to radius server
 *********************************************************/

static int acct_req(acct_type type,
		    struct app_conn_t *conn, uint8_t status_type)
{
  struct radius_packet_t radius_pack;
  uint32_t service_type = 0;
  uint32_t timediff;
#ifdef ENABLE_GARDENACCOUNTING
  char incl_garden = 0;
#endif

  switch(status_type) {
    case RADIUS_STATUS_TYPE_START:
    case RADIUS_STATUS_TYPE_ACCOUNTING_ON:
      conn->s_state.start_time = mainclock.tv_sec;
      conn->s_state.interim_time = mainclock.tv_sec;
      conn->s_state.last_time = mainclock.tv_sec;
      conn->s_state.last_up_time = mainclock.tv_sec;
      switch(type) {
#ifdef ENABLE_GARDENACCOUNTING
        case ACCT_GARDEN:
          snprintf(conn->s_state.garden_sessionid,
                        sizeof(conn->s_state.garden_sessionid),
                        "UAM-%s-%.8x%.8x", inet_ntoa(conn->hisip),
                        (int) mainclock_rt(), conn->unit);
          conn->s_state.garden_start_time = mainclock.tv_sec;
          conn->s_state.garden_interim_time = mainclock.tv_sec;
          conn->s_state.garden_input_octets = 0;
          conn->s_state.garden_output_octets = 0;
          conn->s_state.other_input_octets = 0;
          conn->s_state.other_output_octets = 0;
          break;
#endif
        case ACCT_USER:
          conn->s_state.input_packets = 0;
          conn->s_state.output_packets = 0;
          conn->s_state.input_octets = 0;
          conn->s_state.output_octets = 0;
          break;
      }
      break;

    case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
      conn->s_state.interim_time = mainclock.tv_sec;
      /* drop through */

    case RADIUS_STATUS_TYPE_STOP:
      switch(type) {
#ifdef ENABLE_GARDENACCOUNTING
        case ACCT_GARDEN:
          conn->s_state.garden_interim_time = mainclock.tv_sec;
          if (!conn->s_state.garden_interim_time)
            return 0;
#endif
        case ACCT_USER:
          break;
      }
      break;
  }

  /*
   *  Return if there is no RADIUS accounting for this session.
   */
  if (conn->s_params.flags & NO_ACCOUNTING)
    return 0;

  /*
   *  Build and send RADIUS Accounting.
   */
  if (radius_default_pack(radius, &radius_pack,
			  RADIUS_CODE_ACCOUNTING_REQUEST)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  if (RADIUS_STATUS_TYPE_ACCOUNTING_ON  != status_type &&
      RADIUS_STATUS_TYPE_ACCOUNTING_OFF != status_type) {

    switch(type) {
#ifdef ENABLE_GARDENACCOUNTING
      case ACCT_GARDEN:
        break;
#endif
      case ACCT_USER:
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
                       (uint8_t*) conn->s_state.redir.username,
                       strlen(conn->s_state.redir.username));
        break;
    }

    if (conn->is_adminsession) {

      service_type = RADIUS_SERVICE_TYPE_ADMIN_USER;
#ifdef ENABLE_GARDENACCOUNTING
      incl_garden = 1;
#endif

#if defined(HAVE_SYS_SYSINFO_H) && defined(HAVE_SYSINFO)
      {
	struct sysinfo the_info;

	if (sysinfo(&the_info)) {

	  syslog(LOG_ERR, "%s: sysinfo()", strerror(errno));

	} else {
	  float shiftfloat;
	  float fav[3];
	  char b[128];

	  shiftfloat = (float) (1<<SI_LOAD_SHIFT);

	  fav[0]=((float)the_info.loads[0])/shiftfloat;
	  fav[1]=((float)the_info.loads[1])/shiftfloat;
	  fav[2]=((float)the_info.loads[2])/shiftfloat;

	  radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
			 RADIUS_VENDOR_COOVACHILLI, RADIUS_ATTR_COOVACHILLI_SYS_UPTIME,
			 (uint32_t) the_info.uptime, NULL, 0);

	  snprintf(b, sizeof(b), "%f %f %f",fav[0],fav[1],fav[2]);

	  radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
			 RADIUS_VENDOR_COOVACHILLI, RADIUS_ATTR_COOVACHILLI_SYS_LOADAVG,
			 0, (uint8_t *) b, strlen(b));

	  snprintf(b, sizeof(b), "%ld %ld %ld %ld",
                        the_info.totalram,
                        the_info.freeram,
                        the_info.sharedram,
                        the_info.bufferram);

	  radius_addattr(radius, &radius_pack,
			 RADIUS_ATTR_VENDOR_SPECIFIC,
			 RADIUS_VENDOR_COOVACHILLI,
			 RADIUS_ATTR_COOVACHILLI_SYS_MEMORY,
			 0, (uint8_t *) b, strlen(b));
	}
      }
#endif
    }
  }

  if (status_type == RADIUS_STATUS_TYPE_STOP ||
      status_type == RADIUS_STATUS_TYPE_INTERIM_UPDATE) {

    switch(type) {
#ifdef ENABLE_GARDENACCOUNTING
      case ACCT_GARDEN:
        incl_garden = 1;
        if (!conn->s_state.garden_start_time) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): session hasn't started yet", __FUNCTION__, __LINE__);
          return 0;
        }
        timediff = mainclock_diffu(conn->s_state.garden_start_time);

        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_SESSION_TIME, 0, 0,
                       timediff, NULL, 0);
        break;
#endif
      case ACCT_USER:
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_INPUT_OCTETS, 0, 0,
                       (uint32_t) conn->s_state.input_octets, NULL, 0);
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_OUTPUT_OCTETS, 0, 0,
                       (uint32_t) conn->s_state.output_octets, NULL, 0);

        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS,
                       0, 0, (uint32_t) (conn->s_state.input_octets >> 32), NULL, 0);
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS,
                       0, 0, (uint32_t) (conn->s_state.output_octets >> 32), NULL, 0);

        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_INPUT_PACKETS, 0, 0,
                       conn->s_state.input_packets, NULL, 0);
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_OUTPUT_PACKETS, 0, 0,
                       conn->s_state.output_packets, NULL, 0);

        timediff = mainclock_diffu(conn->s_state.start_time);

        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_SESSION_TIME, 0, 0,
                       timediff, NULL, 0);
        break;
    }

#ifdef ENABLE_GARDENACCOUNTING
    if (incl_garden && _options.uamgardendata) {
      radius_addattr(radius, &radius_pack,
		     RADIUS_ATTR_VENDOR_SPECIFIC,
		     RADIUS_VENDOR_COOVACHILLI,
		     RADIUS_ATTR_COOVACHILLI_GARDEN_INPUT_OCTETS,
		     (uint32_t) conn->s_state.garden_input_octets, NULL, 0);
      radius_addattr(radius, &radius_pack,
		     RADIUS_ATTR_VENDOR_SPECIFIC,
		     RADIUS_VENDOR_COOVACHILLI,
		     RADIUS_ATTR_COOVACHILLI_GARDEN_OUTPUT_OCTETS,
		     (uint32_t) conn->s_state.garden_output_octets, NULL, 0);

      radius_addattr(radius, &radius_pack,
		     RADIUS_ATTR_VENDOR_SPECIFIC,
		     RADIUS_VENDOR_COOVACHILLI,
		     RADIUS_ATTR_COOVACHILLI_GARDEN_INPUT_GIGAWORDS,
		     (uint32_t) (conn->s_state.garden_input_octets >> 32), NULL, 0);
      radius_addattr(radius, &radius_pack,
		     RADIUS_ATTR_VENDOR_SPECIFIC,
		     RADIUS_VENDOR_COOVACHILLI,
		     RADIUS_ATTR_COOVACHILLI_GARDEN_OUTPUT_GIGAWORDS,
		     (uint32_t) (conn->s_state.garden_output_octets >> 32), NULL, 0);

      if (_options.uamotherdata) {
	radius_addattr(radius, &radius_pack,
		       RADIUS_ATTR_VENDOR_SPECIFIC,
		       RADIUS_VENDOR_COOVACHILLI,
		       RADIUS_ATTR_COOVACHILLI_OTHER_INPUT_OCTETS,
		       (uint32_t) conn->s_state.other_input_octets, NULL, 0);
	radius_addattr(radius, &radius_pack,
		       RADIUS_ATTR_VENDOR_SPECIFIC,
		       RADIUS_VENDOR_COOVACHILLI,
		       RADIUS_ATTR_COOVACHILLI_OTHER_OUTPUT_OCTETS,
		       (uint32_t) conn->s_state.other_output_octets, NULL, 0);

	radius_addattr(radius, &radius_pack,
		       RADIUS_ATTR_VENDOR_SPECIFIC,
		       RADIUS_VENDOR_COOVACHILLI,
		       RADIUS_ATTR_COOVACHILLI_OTHER_INPUT_GIGAWORDS,
		       (uint32_t) (conn->s_state.other_input_octets >> 32), NULL, 0);
	radius_addattr(radius, &radius_pack,
		       RADIUS_ATTR_VENDOR_SPECIFIC,
		       RADIUS_VENDOR_COOVACHILLI,
		       RADIUS_ATTR_COOVACHILLI_OTHER_OUTPUT_GIGAWORDS,
		       (uint32_t) (conn->s_state.other_output_octets >> 32), NULL, 0);
      }
    }
#endif
  }

  if (status_type == RADIUS_STATUS_TYPE_STOP ||
      status_type == RADIUS_STATUS_TYPE_ACCOUNTING_OFF) {

    switch(type) {
#ifdef ENABLE_GARDENACCOUNTING
      case ACCT_GARDEN:
        break;
#endif
      case ACCT_USER:
        radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_TERMINATE_CAUSE,
                       0, 0, conn->s_state.terminate_cause, NULL, 0);
        break;
    }
  }

#ifdef ENABLE_SESSIONSTATE
  if (conn->s_state.session_state) {
    radius_addattr(radius, &radius_pack,
		   RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_COOVACHILLI,
		   RADIUS_ATTR_COOVACHILLI_SESSION_STATE,
		   conn->s_state.session_state, 0, 0);
  }
#endif

  chilli_req_attrs(radius, &radius_pack,
		   type, service_type, status_type,
		   conn->unit, conn->hismac,
		   &conn->hisip,
		   &conn->s_state);

  radius_req(radius, &radius_pack, conn);

  return 0;
}

#ifdef ENABLE_UAMANYIP
/**
 * Assigns an ip from the dynamic pool, for SNAT'ing anyip connections.
 * If anyip is on and the clients address is outside of our network,
 * we need to SNAT to an ip of our network.
 */
int chilli_assign_snat(struct app_conn_t *appconn, int force) {
  struct ippoolm_t *newipm;

  if (!_options.uamanyip) return 0;
  if (!_options.uamnatanyip) return 0;
  if (appconn->natip.s_addr && !force) return 0;

  /* check if excluded from anyip */
  if (_options.uamnatanyipex_addr.s_addr &&
      (appconn->hisip.s_addr & _options.uamnatanyipex_mask.s_addr) ==
      _options.uamnatanyipex_addr.s_addr) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Excluding ip %s from SNAT becuase it is in uamnatanyipex", __FUNCTION__, __LINE__,
             inet_ntoa(appconn->hisip));
    return 0;
  }

  if ((appconn->hisip.s_addr & _options.mask.s_addr) == _options.net.s_addr)
    return 0;

  if (_options.debug) {
    syslog(LOG_DEBUG, "%s(%d): Request SNAT ip for client ip: %s", __FUNCTION__, __LINE__,
           inet_ntoa(appconn->hisip));
    syslog(LOG_DEBUG, "%s(%d): SNAT mask: %s", __FUNCTION__, __LINE__, inet_ntoa(appconn->mask));
    syslog(LOG_DEBUG, "%s(%d): SNAT ourip: %s", __FUNCTION__, __LINE__, inet_ntoa(appconn->ourip));
  }

  if (ippool_newip(ippool, &newipm, &appconn->natip, 0)) {
    syslog(LOG_ERR, "Failed to allocate SNAT IP address");
    /*
     *  Clean up the static pool listing too, it's misconfigured now.
     */
    if (appconn->dnlink) {
      dhcp_freeconn((struct dhcp_conn_t *)appconn->dnlink, 0);
    }
    return -1;
  }

  appconn->natip.s_addr = newipm->addr.s_addr;
  newipm->peer = appconn;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): SNAT IP %s assigned", __FUNCTION__, __LINE__, inet_ntoa(appconn->natip));

  return 0;
}
#endif


/***********************************************************
 *
 * Functions handling downlink protocol authentication.
 * Called in response to radius access request response.
 *
 ***********************************************************/

int dnprot_reject(struct app_conn_t *appconn) {
  struct dhcp_conn_t* dhcpconn = NULL;
  /*struct ippoolm_t *ipm;*/

  if (appconn->is_adminsession) return 0;

  switch (appconn->dnprot) {

#ifdef ENABLE_EAPOL
    case DNPROT_EAPOL:
      if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
        syslog(LOG_ERR, "No downlink protocol");
        return 0;
      }

      dhcp_sendEAPreject(dhcpconn, NULL, 0);
      return 0;
#endif

    case DNPROT_UAM:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Rejecting UAM", __FUNCTION__, __LINE__);
      return 0;

#ifdef ENABLE_RADPROXY
    case DNPROT_WPA:
      return radius_access_reject(appconn);
#endif

    case DNPROT_MAC:
      /* remove the username since we're not logged in */
      if (!appconn->s_state.authenticated)
        strlcpy(appconn->s_state.redir.username, "-", USERNAMESIZE);

      if (!(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink)) {
        syslog(LOG_ERR, "No downlink protocol");
        return 0;
      }

      if (_options.macauthdeny) {
        dhcpconn->authstate = DHCP_AUTH_DROP;
        appconn->dnprot = DNPROT_NULL;
      }
      else {
        dhcpconn->authstate = DHCP_AUTH_DNAT;
        appconn->dnprot = DNPROT_UAM;
      }

      return 0;

    case DNPROT_NULL:
#ifdef ENABLE_LAYER3
    case DNPROT_LAYER3:
#endif
      return 0;

    default:
      syslog(LOG_ERR, "Unknown downlink protocol");
      return 0;
  }
}

#if defined(ENABLE_RADPROXY) || defined(ENABLE_EAPOL)
int static dnprot_challenge(struct app_conn_t *appconn) {

  switch (appconn->dnprot) {

#ifdef ENABLE_EAPOL
    case DNPROT_EAPOL:
      {
        struct dhcp_conn_t* dhcpconn = NULL;
        if (!(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink)) {
          syslog(LOG_ERR, "No downlink protocol");
          return 0;
        }

        dhcp_sendEAP(dhcpconn, appconn->chal, appconn->challen);
      }
      break;
#endif

    case DNPROT_NULL:
    case DNPROT_UAM:
    case DNPROT_MAC:
#ifdef ENABLE_LAYER3
    case DNPROT_LAYER3:
#endif
      break;

#ifdef ENABLE_RADPROXY
    case DNPROT_WPA:
      radius_access_challenge(appconn);
      break;
#endif

    default:
      syslog(LOG_ERR, "Unknown downlink protocol");
  }

  return 0;
}
#endif

int dnprot_accept(struct app_conn_t *appconn) {
  struct dhcp_conn_t* dhcpconn = NULL;

  if (appconn->is_adminsession) return 0;

  if (!appconn->hisip.s_addr) {
    syslog(LOG_ERR, "IP address not allocated");
    return 0;
  }

  switch (appconn->dnprot) {

#ifdef ENABLE_EAPOL
    case DNPROT_EAPOL:
      if (!(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink)) {
        syslog(LOG_ERR, "No downlink protocol");
        return 0;
      }

      dhcp_set_addrs(dhcpconn,
                     &appconn->hisip, &appconn->hismask,
                     &appconn->ourip, &appconn->mask,
                     &appconn->dns1, &appconn->dns2);

      /* This is the one and only place eapol authentication is accepted */

      dhcpconn->authstate = DHCP_AUTH_PASS;

      /* Tell client it was successful */
      dhcp_sendEAP(dhcpconn, appconn->chal, appconn->challen);

      syslog(LOG_WARNING, "Do not know how to set encryption keys on this platform!");
      break;
#endif

    case DNPROT_UAM:
      if (!(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink)) {
        syslog(LOG_ERR, "No downlink protocol");
        return 0;
      }

      dhcp_set_addrs(dhcpconn,
                     &appconn->hisip, &appconn->hismask,
                     &appconn->ourip, &appconn->mask,
                     &appconn->dns1, &appconn->dns2);

      /* This is the one and only place UAM authentication is accepted */
      dhcpconn->authstate = DHCP_AUTH_PASS;
      appconn->s_params.flags &= ~REQUIRE_UAM_AUTH;
      break;

#ifdef ENABLE_RADPROXY
    case DNPROT_WPA:
      if (!(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink)) {
        syslog(LOG_ERR, "No downlink protocol");
        return 0;
      }

      dhcp_set_addrs(dhcpconn,
                     &appconn->hisip, &appconn->hismask,
                     &appconn->ourip, &appconn->mask,
                     &appconn->dns1, &appconn->dns2);

      /* This is the one and only place WPA authentication is accepted */
      if (appconn->s_params.flags & REQUIRE_UAM_AUTH) {
        appconn->dnprot = DNPROT_DHCP_NONE;
        dhcpconn->authstate = DHCP_AUTH_NONE;
      }
      else {
        dhcpconn->authstate = DHCP_AUTH_PASS;
      }

      /* Tell access point it was successful */
      radius_access_accept(appconn);

      break;
#endif

    case DNPROT_MAC:
      if (!(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink)) {
        syslog(LOG_ERR, "No downlink protocol");
        return 0;
      }

      dhcp_set_addrs(dhcpconn,
                     &appconn->hisip, &appconn->hismask,
                     &appconn->ourip, &appconn->mask,
                     &appconn->dns1, &appconn->dns2);

      dhcpconn->authstate = DHCP_AUTH_PASS;
      break;

    case DNPROT_NULL:
    case DNPROT_DHCP_NONE:
      return 0;

#ifdef ENABLE_LAYER3
    case DNPROT_LAYER3:
      break;
#endif

    default:
      syslog(LOG_ERR, "Unknown downlink protocol");
      return 0;
  }

  if ((dhcpconn && appconn->s_params.flags & REQUIRE_UAM_SPLASH) ||
      (dhcpconn && appconn->s_params.flags & REQUIRE_UAM_AUTH))
    dhcpconn->authstate = DHCP_AUTH_SPLASH;

  if (!(appconn->s_params.flags & REQUIRE_UAM_AUTH)) {
    /* This is the one and only place state is switched to authenticated */
    appconn->s_state.authenticated = 1;

#ifdef ENABLE_SESSIONSTATE
    appconn->s_state.session_state =
        RADIUS_VALUE_COOVACHILLI_SESSION_AUTH;
#endif

#ifdef HAVE_NETFILTER_COOVA
    if (_options.kname) {
      kmod_coova_update(appconn);
    }
#endif

#ifdef ENABLE_MODULES
    { int i;
      for (i=0; i < MAX_MODULES; i++) {
	if (!_options.modules[i].name[0]) break;
	if (_options.modules[i].ctx) {
	  struct chilli_module *m =
              (struct chilli_module *)_options.modules[i].ctx;
	  if (m->session_start)
	    m->session_start(appconn);
	}
      }
    }
#endif

    /* if (!(appconn->s_params.flags & IS_UAM_REAUTH))*/
    acct_req(ACCT_USER, appconn, RADIUS_STATUS_TYPE_START);

    /* Run connection up script */
    if (_options.conup && !(appconn->s_params.flags & NO_SCRIPT)) {
 if (_options.debug)
     syslog(LOG_DEBUG, "%s(%d): Calling connection up script: %s\n", __FUNCTION__, __LINE__, _options.conup);
      runscript(appconn, _options.conup, 0, 0);
    }
  }

  appconn->s_params.flags &= ~IS_UAM_REAUTH;

#ifdef ENABLE_STATFILE
  if (_options.statusfilesave)
    printstatus();
#endif

  return 0;
}

#ifdef ENABLE_SSDP
static int fwd_ssdp(struct in_addr *dst,
		    struct pkt_ipphdr_t *iph,
		    struct pkt_udphdr_t *udph,
		    struct pkt_buffer *pb,
		    int ethhdr) {
  struct pkt_ethhdr_t *ethh = pkt_ethhdr(pkt_buffer_head(pb));

  if (udph && dst->s_addr == ssdp.s_addr) {

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): src="MAC_FMT" "
             "dst="MAC_FMT" prot=%.4x", __FUNCTION__, __LINE__,
             MAC_ARG(ethh->src),
             MAC_ARG(ethh->dst),
             ntohs(ethh->prot));

    /* TODO: also check that the source is from this machine in case we
     * are forwarding packets that we dont want to. */

    if (_options.debug) {
      char *bufr = (char *)(((void *)udph) + sizeof(struct pkt_udphdr_t));
      struct in_addr src;

      src.s_addr = iph->saddr;

      syslog(LOG_DEBUG, "%s(%d): ssdp multicast from %s\n%.*s", __FUNCTION__, __LINE__, inet_ntoa(src),
             ntohs(udph->len), bufr);
    }

    /* This sends to a unicast MAC address but a multicast IP address.
     */
    struct dhcp_conn_t *conn = dhcp->firstusedconn;
    while (conn) {
      if (conn->inuse && conn->authstate == DHCP_AUTH_PASS) {
	/*
	if (_options.debug)
  syslog(LOG_DEBUG, "sending to %s.", inet_ntoa(conn->hisip ));
	*/
	dhcp_data_req(conn, pb, ethhdr);
      }
      conn = conn->next;
    }

    return 1; /* match */
  }

  return 0; /* no match */
}
#endif

#ifdef ENABLE_LAYER3
static int fwd_layer3(struct app_conn_t *appconn,
		      struct in_addr *dst,
		      struct pkt_udphdr_t *udph,
		      struct pkt_buffer *pb,
		      int ethhdr) {
  if (udph && udph->src == htons(DHCP_BOOTPS)) {
    struct dhcp_packet_t *pdhcp =
        (struct dhcp_packet_t *)(((void *)udph) + PKT_UDP_HLEN);

    if (pdhcp && pdhcp->op == DHCP_BOOTREPLY &&
	pdhcp->options[0] == 0x63 &&
	pdhcp->options[1] == 0x82 &&
	pdhcp->options[2] == 0x53 &&
	pdhcp->options[3] == 0x63) {

      if (!appconn) {
	struct in_addr src;
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Detecting layer3 IP assignment", __FUNCTION__, __LINE__);

	src.s_addr = pdhcp->yiaddr;
	appconn = chilli_connect_layer3(&src, 0);
	if (!appconn) {
	  syslog(LOG_ERR, "could not allocate for %s", inet_ntoa(src));
	  return 1;
	}
      }

#ifdef ENABLE_TAP
      if (_options.usetap) {
	struct pkt_ethhdr_t *ethh = pkt_ethhdr(pkt_buffer_head(pb));

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): forwarding layer3 dhcp-broadcast: %s",__FUNCTION__, __LINE__, inet_ntoa(*dst));

	dhcp_send(dhcp, -1, ethh->dst,
		  pkt_buffer_head(pb),
		  pkt_buffer_length(pb));
      } else
#endif
      {
	struct pkt_ethhdr_t *ethh;
	uint8_t packet[PKT_MAX_LEN];
	size_t length = pkt_buffer_length(pb);
	size_t hdrlen = PKT_ETH_HLEN;

	memset(packet, 0, hdrlen);

	ethh = pkt_ethhdr(packet);

	uint8_t *dstmac = pdhcp->chaddr;

	memcpy(packet + hdrlen,
	       pkt_buffer_head(pb),
	       pkt_buffer_length(pb));
	length += hdrlen;

	copy_mac6(ethh->dst, dstmac);
	copy_mac6(ethh->src, dhcp_nexthop(dhcp));
	ethh->prot = htons(PKT_ETH_PROTO_IP);

	dhcp_send(dhcp, -1, dstmac, packet, length);
      }
      return 1;
    }
  }
  return 0;
}
#endif


/*
 * Tun callbacks
 *
 * Called from the tun_decaps function. This method is passed either
 * a Ethernet frame or an IP packet.
 */

int cb_tun_ind(struct tun_t *tun, struct pkt_buffer *pb, int idx) {
  struct in_addr dst;
  struct ippoolm_t *ipm;
  struct app_conn_t *appconn = 0;
  struct pkt_udphdr_t *udph = 0;
  struct pkt_ipphdr_t *ipph;

  uint8_t *pack = pkt_buffer_head(pb);
  size_t len = pkt_buffer_length(pb);

  int ethhdr = (tun(tun, idx).flags & NET_ETHHDR) != 0;
  size_t ip_len = len;

#ifdef ENABLE_TAP
  if (idx) ethhdr = 0;

  if (ethhdr) {
    struct pkt_ethhdr_t *ethh = 0;
    /*
     *   Will never be 802.1Q
     */
    uint16_t prot;

    ethh = pkt_ethhdr(pack);
    prot = ntohs(ethh->prot);

    ipph = (struct pkt_ipphdr_t *)((char *)pack + PKT_ETH_HLEN);
    ip_len -= PKT_ETH_HLEN;

    switch (prot) {
      case PKT_ETH_PROTO_IPv6:
        return 0;

      case PKT_ETH_PROTO_IP:
        break;

      case PKT_ETH_PROTO_ARP:
        {
          /*
           *  Send arp reply
           */
          uint8_t packet[PKT_BUFFER];

          struct pkt_ethhdr_t *p_ethh = pkt_ethhdr(pack);
          struct arp_packet_t *p_arp = pkt_arppkt(pack);
          struct pkt_ethhdr_t *packet_ethh = pkt_ethhdr(packet);
          struct arp_packet_t *packet_arp =
              ((struct arp_packet_t *)(((uint8_t*)(packet)) + PKT_ETH_HLEN));

          size_t length = PKT_ETH_HLEN + sizeof(struct arp_packet_t);

          struct in_addr reqaddr;

          /*
           *   Get local copy of the target address to resolve
           */
          memcpy(&reqaddr.s_addr, p_arp->tpa, PKT_IP_ALEN);

          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): arp: ifidx=%d src="MAC_FMT" "
                   "dst="MAC_FMT" "
                   "prot=%.4x (asking for %s)", __FUNCTION__, __LINE__,
                   tun(tun,idx).ifindex,
                   MAC_ARG(ethh->src),
                   MAC_ARG(ethh->dst),
                   ntohs(ethh->prot),
                   inet_ntoa(reqaddr));

          /*
           *  Lookup request address, see if we control it.
           */
          if (ippool_getip(ippool, &ipm, &reqaddr)) {
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): ARP for unknown IP %s", __FUNCTION__, __LINE__, inet_ntoa(reqaddr));
            return 0;
          }

          if ((appconn  = (struct app_conn_t *)ipm->peer) == NULL ||
              (appconn->dnlink) == NULL) {
            syslog(LOG_ERR, "No peer protocol defined for ARP request");
            return 0;
          }

          /* Get packet default values */
          memset(&packet, 0, sizeof(packet));

          /* ARP Payload */
          packet_arp->hrd = htons(DHCP_HTYPE_ETH);
          packet_arp->pro = htons(PKT_ETH_PROTO_IP);
          packet_arp->hln = PKT_ETH_ALEN;
          packet_arp->pln = PKT_IP_ALEN;
          packet_arp->op  = htons(DHCP_ARP_REPLY);

          /* Source address */
          /*memcpy(packet_arp->sha, appconn->hismac, PKT_ETH_ALEN);*/
          memcpy(packet_arp->sha, dhcp->rawif[0].hwaddr, PKT_ETH_ALEN);

#ifdef ENABLE_UAMANYIP
          /*
           * ARP replies need to tell the NATed ip address,
           * when client is an anyip client.
           */
          if (_options.uamanyip && appconn->natip.s_addr) {
            memcpy(packet_arp->spa, &appconn->natip.s_addr, PKT_IP_ALEN);
            if (_options.debug) {
              char ip[56];
              char snatip[56];
              strlcpy(ip, inet_ntoa(appconn->hisip), sizeof(ip));
              strlcpy(snatip, inet_ntoa(appconn->natip), sizeof(snatip));
              syslog(LOG_DEBUG, "%s(%d): SNAT anyip in ARP response from %s to %s", __FUNCTION__, __LINE__,
                     ip, snatip);
            }
          } else
#endif
            memcpy(packet_arp->spa, &appconn->hisip.s_addr, PKT_IP_ALEN);

          /* Target address */
          memcpy(packet_arp->tha, p_arp->sha, PKT_ETH_ALEN);
          memcpy(packet_arp->tpa, p_arp->spa, PKT_IP_ALEN);

          /* Ethernet header */
          memcpy(packet_ethh->dst, p_ethh->src, PKT_ETH_ALEN);
          /*memcpy(packet_ethh->src, appconn->hismac, PKT_ETH_ALEN);*/
          memcpy(packet_ethh->src, dhcp->rawif[0].hwaddr, PKT_ETH_ALEN);

          packet_ethh->prot = htons(PKT_ETH_PROTO_ARP);

          if (_options.debug) {
            syslog(LOG_DEBUG, "%s(%d): arp-reply: src="MAC_FMT" "
                   "dst="MAC_FMT, __FUNCTION__, __LINE__,
                   MAC_ARG(packet_ethh->src),
                   MAC_ARG(packet_ethh->dst));

            memcpy(&reqaddr.s_addr, packet_arp->spa, PKT_IP_ALEN);
            syslog(LOG_DEBUG, "%s(%d): arp-reply: source sha="MAC_FMT" spa=%s", __FUNCTION__, __LINE__,
                   MAC_ARG(packet_arp->sha),
                   inet_ntoa(reqaddr));

            memcpy(&reqaddr.s_addr, packet_arp->tpa, PKT_IP_ALEN);
            syslog(LOG_DEBUG, "%s(%d): arp-reply: target tha="MAC_FMT" tpa=%s", __FUNCTION__, __LINE__,
                   MAC_ARG(packet_arp->tha),
                   inet_ntoa(reqaddr));
          }

          return tun_write(tun, (uint8_t *)&packet, length, idx);
        }
      default:
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): unhandled protocol %x", __FUNCTION__, __LINE__, prot);
        return 0;
    }

  } else
#endif
  {
    ipph = (struct pkt_ipphdr_t *)pack;
  }

  size_t hlen = (ipph->version_ihl & 0x0f) << 2;
  if (ntohs(ipph->tot_len) > ip_len || hlen > ip_len) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): invalid IP packet %d / %zu", __FUNCTION__, __LINE__,
             ntohs(ipph->tot_len),
             len);
    return 0;
  }

  /*
   *  Filter out unsupported / unhandled protocols,
   *  and check some basic length sanity.
   */
  switch(ipph->protocol) {
    case PKT_IP_PROTO_GRE:
    case PKT_IP_PROTO_TCP:
    case PKT_IP_PROTO_ICMP:
    case PKT_IP_PROTO_ESP:
    case PKT_IP_PROTO_AH:
      break;
    case PKT_IP_PROTO_UDP:
      {
        /*
         * Only the first IP fragment has the UDP header.
         */
        if (iphdr_offset((struct pkt_iphdr_t*)ipph) == 0) {
          udph = (struct pkt_udphdr_t *)(((void *)ipph) + hlen);
        }
        if (udph && !iphdr_more_frag((struct pkt_iphdr_t*)ipph) && (ntohs(udph->len) > ip_len)) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): invalid UDP packet %d / %d / %zu", __FUNCTION__, __LINE__,
                   ntohs(ipph->tot_len),
                   udph ? ntohs(udph->len) : -1, ip_len);
          return 0;
        }
      }
      break;
    default:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): dropping unhandled packet: %x", __FUNCTION__, __LINE__, ipph->protocol);
      return 0;
  }

  dst.s_addr = ipph->daddr;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): sending to : %s", __FUNCTION__, __LINE__, inet_ntoa(dst));
#endif

  if (ippool_getip(ippool, &ipm, &dst)) {

    /*
     *  TODO: If within statip range, allow the packet through (?)
     */

#ifdef ENABLE_SSDP
    if (fwd_ssdp(&dst, ipph, udph, pb, ethhdr)) return 0;
#endif

#ifdef ENABLE_LAYER3
    if (_options.layer3)
      if (fwd_layer3(0, &dst, udph, pb, ethhdr))
	return 0;
#endif

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): dropping packet with unknown destination: %s", __FUNCTION__, __LINE__, inet_ntoa(dst));

    return 0;
  }

  appconn = (struct app_conn_t *)ipm->peer;

#ifdef ENABLE_LEAKYBUCKET
  if (_options.scalewin && appconn && appconn->s_state.bucketdownsize) {
    uint16_t win = appconn->s_state.bucketdownsize -
        appconn->s_state.bucketdown;
    //log_dbg("window scaling to %d", win);
    pkt_shape_tcpwin((struct pkt_iphdr_t *)ipph, win);
  }
#endif

#ifdef ENABLE_LAYER3
  if (_options.layer3 && appconn && !appconn->dnlink)
    if (fwd_layer3(appconn, &dst, udph, pb, ethhdr))
      return 0;
#endif

  if (appconn == NULL || appconn->dnlink == NULL) {
    syslog(LOG_ERR, "No %s protocol defined for %s",
           appconn ? "dnlink" : "peer", inet_ntoa(dst));
    return 0;
  }

#ifdef ENABLE_UAMANYIP
  /**
   * connection needs to be NAT'ed, since client is an anyip client
   * outside of our network.
   * So, let's NAT the SNAT ip back to it's client ip.
   */
  if (_options.uamanyip && appconn->natip.s_addr) {
#if(_debug_ > 1)
    if (_options.debug) {
      char ip[56];
      char snatip[56];
      strlcpy(ip, inet_ntoa(appconn->hisip), sizeof(ip));
      strlcpy(snatip, inet_ntoa(appconn->natip), sizeof(snatip));
      syslog(LOG_DEBUG, "%s(%d): SNAT anyip replace %s back to %s; snat was: %s", __FUNCTION__, __LINE__,
             inet_ntoa(dst), ip, snatip);
    }
#endif
    ipph->daddr = appconn->hisip.s_addr;
    if (chksum((struct pkt_iphdr_t *) ipph) < 0)
      return 0;
  }
#endif

  /* If the ip src is uamlisten and psrc is uamport
     we won't call leaky_bucket */
  if ( ! (ipph->saddr  == _options.uamlisten.s_addr &&
	  (ipph->sport == htons(_options.uamport)
#ifdef ENABLE_UAMUIPORT
	   || ipph->sport == htons(_options.uamuiport)
#endif
	   ))) {
    if (chilli_acct_tosub(appconn, ipph))
      return 0;
  }

  switch (appconn->dnprot) {
    case DNPROT_NULL:
    case DNPROT_DHCP_NONE:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Dropping...", __FUNCTION__, __LINE__);
      break;

    case DNPROT_UAM:
    case DNPROT_WPA:
    case DNPROT_MAC:
#ifdef ENABLE_EAPOL
    case DNPROT_EAPOL:
#endif
#ifdef ENABLE_LAYER3
    case DNPROT_LAYER3:
#endif
      dhcp_data_req((struct dhcp_conn_t *)appconn->dnlink, pb, ethhdr);
      break;

    default:
      syslog(LOG_ERR, "Unknown downlink protocol: %d", appconn->dnprot);
      break;
  }

  return 0;
}

/*********************************************************
 *
 * Redir callbacks
 *
 *********************************************************/

int cb_redir_getstate(struct redir_t *redir,
		      struct sockaddr_in *address,
		      struct sockaddr_in *baddress,
		      struct redir_conn_t *conn) {
  struct in_addr *addr = &address->sin_addr;
  struct ippoolm_t *ipm;
  struct app_conn_t *appconn;
  struct dhcp_conn_t *dhcpconn;
  uint8_t flags = 0;

#if defined(HAVE_NETFILTER_QUEUE) || defined(HAVE_NETFILTER_COOVA)
  if (_options.uamlisten.s_addr != _options.dhcplisten.s_addr) {
    addr->s_addr  = addr->s_addr & ~(_options.mask.s_addr);
    addr->s_addr |= _options.dhcplisten.s_addr & _options.mask.s_addr;
  }
#endif

  if (ippool_getip(ippool, &ipm, addr)) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): did not find %s", __FUNCTION__, __LINE__, inet_ntoa(*addr));
    return -1;
  }

  if ( (appconn  = (struct app_conn_t *)ipm->peer)        == NULL ||
       (dhcpconn = (struct dhcp_conn_t *)appconn->dnlink) == NULL ) {
    syslog(LOG_WARNING, "No peer protocol defined app-null=%d", appconn == 0);
    return -1;
  }

  conn->nasip = _options.radiuslisten;
  conn->nasport = appconn->unit;
  memcpy(conn->hismac, dhcpconn->hismac, PKT_ETH_ALEN);
  conn->ourip = appconn->ourip;
  conn->hisip = appconn->hisip;

#ifdef HAVE_SSL
  /*
   *  Determine if the connection is SSL or not.
   */
  {
    int n;
    for (n=0; n < DHCP_DNAT_MAX; n++) {
      /*
       *  First, search the dnat list to see if we are tracking the port.
       */
      /*syslog("%d(%d) == %d",ntohs(dhcpconn->dnat[n].src_port),ntohs(dhcpconn->dnat[n].dst_port),ntohs(address->sin_port));*/
      if (dhcpconn->dnat[n].src_port == address->sin_port) {
	if (dhcpconn->dnat[n].dst_port == htons(DHCP_HTTPS)
#ifdef ENABLE_UAMUIPORT
	    || (_options.uamuissl && dhcpconn->dnat[n].dst_port == htons(_options.uamuiport))
#endif
	    ) {
#if(_debug_)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): redir connection is SSL", __FUNCTION__, __LINE__);
#endif
	  flags |= USING_SSL;
	}
	break;
      }
    }
#ifdef ENABLE_UAMUIPORT
    /*
     *  If not in dnat, if uamuissl is enabled, and this is indeed that
     *  port, then we also know it is SSL (directly to https://uamlisten:uamuiport).
     */
    if (n == DHCP_DNAT_MAX && _options.uamuissl &&
	ntohs(baddress->sin_port) == _options.uamuiport) {
#if(_debug_)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): redir connection is SSL", __FUNCTION__, __LINE__);
#endif
      flags |= USING_SSL;
    }
#endif
  }
#endif

  conn->flags = flags;

  memcpy(&conn->s_params, &appconn->s_params, sizeof(appconn->s_params));
  memcpy(&conn->s_state,  &appconn->s_state,  sizeof(appconn->s_state));

  /* reset state */
  appconn->uamexit = 0;

  return conn->s_state.authenticated == 1;
}

#ifdef ENABLE_LOCATION
int
chilli_learn_location(uint8_t *loc, int loclen,
		      struct app_conn_t *appconn, char force) {
  char loc_buff[MAX_LOCATION_LENGTH];
  char prev_loc_buff[MAX_LOCATION_LENGTH];
  int prev_loc_len=0;

  char has_new_location = 0;
  char restart_accounting = 0;

  if (loclen >= MAX_LOCATION_LENGTH) {
    syslog(LOG_ERR, "Location too long %d", loclen);
    return 0;
  }

  memcpy(loc_buff, (char *)loc, loclen);
  loc_buff[loclen]=0;

  strlcpy(prev_loc_buff, appconn->s_state.location, sizeof(prev_loc_buff));
  prev_loc_len = strlen(prev_loc_buff);

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Learned location : [%.*s]", __FUNCTION__, __LINE__, loclen, loc);

  if (prev_loc_len == 0 ||
      prev_loc_len != loclen ||
      memcmp(prev_loc_buff, loc, prev_loc_len)) {

    if (force || !strcmp(appconn->s_state.pending_location,
			 loc_buff)) {

      has_new_location = 1;
      appconn->s_state.location_changes++;
      appconn->s_state.pending_location[0]=0;

      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Learned new-location : %d [%.*s] old %d [%s]", __FUNCTION__, __LINE__,
               loclen, loclen, loc,
               prev_loc_len, prev_loc_buff);

#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
      location_add_conn(appconn, loc_buff);
#endif

      if (_options.locationupdate) {
	runscript(appconn, _options.locationupdate,
		  loc_buff, prev_loc_buff);
      }

      if (_options.location_stop_start) {
	restart_accounting = 1;
      }
    } else {
      strlcpy(appconn->s_state.pending_location, loc_buff,
	sizeof(appconn->s_state.pending_location));
    }
  }

  if (appconn->s_state.authenticated == 1 &&
      has_new_location && restart_accounting) {
    /* make sure not already over a limit ... */
    session_interval(appconn);

#ifdef ENABLE_SESSIONSTATE
    appconn->s_state.session_state =
        RADIUS_VALUE_COOVACHILLI_SESSION_LOCATION_CHANGE;
#endif

    if (appconn->s_state.authenticated == 1)
      acct_req(ACCT_USER, appconn, RADIUS_STATUS_TYPE_STOP);
  }

#ifdef ENABLE_GARDENACCOUNTING
  if (_options.uamgardendata &&
      has_new_location && restart_accounting &&
      appconn->hisip.s_addr) {

#ifdef ENABLE_SESSIONSTATE
    appconn->s_state.session_state =
        RADIUS_VALUE_COOVACHILLI_SESSION_LOCATION_CHANGE;
#endif

    acct_req(ACCT_GARDEN, appconn, RADIUS_STATUS_TYPE_STOP);
  }
#endif

  if (has_new_location) {
    memcpy(appconn->s_state.location, (char *) loc, loclen);
    appconn->s_state.location[loclen] = 0;
  }

#ifdef ENABLE_GARDENACCOUNTING
  if (_options.uamgardendata &&
      has_new_location && restart_accounting &&
      appconn->hisip.s_addr) {
    acct_req(ACCT_GARDEN, appconn, RADIUS_STATUS_TYPE_START);
  }
#endif

  if (appconn->s_state.authenticated == 1 &&
      has_new_location && restart_accounting) {

    /* adjust session parameters to be zeroed out */

    set_sessionid(appconn, 0);

    if (appconn->s_params.sessiontimeout) {
      uint32_t sessiontime = mainclock_diffu(appconn->s_state.start_time);
      if (sessiontime > appconn->s_params.sessiontimeout)
	/* (should be the case, having done the session_interval() above) */
	appconn->s_params.sessiontimeout -= sessiontime;
    }

    if (appconn->s_params.maxinputoctets) {
      if (appconn->s_params.maxinputoctets > appconn->s_state.input_octets)
	appconn->s_params.maxinputoctets -= appconn->s_state.input_octets;
    }

    if (appconn->s_params.maxoutputoctets) {
      if (appconn->s_params.maxoutputoctets > appconn->s_state.output_octets)
	appconn->s_params.maxoutputoctets -= appconn->s_state.output_octets;
    }

    if (appconn->s_params.maxtotaloctets) {
      uint64_t total = appconn->s_state.input_octets +
          appconn->s_state.output_octets;
      if (appconn->s_params.maxtotaloctets > total)
	appconn->s_params.maxtotaloctets -= total;
    }

    appconn->s_state.start_time = mainclock.tv_sec;
    appconn->s_state.interim_time = mainclock.tv_sec;
    appconn->s_state.last_time = mainclock.tv_sec;
    appconn->s_state.last_up_time = mainclock.tv_sec;
    appconn->s_state.input_packets = 0;
    appconn->s_state.input_octets = 0;
    appconn->s_state.output_packets = 0;
    appconn->s_state.output_octets = 0;

    acct_req(ACCT_USER, appconn, RADIUS_STATUS_TYPE_START);
  }
  else if (_options.location_immediate_update && has_new_location) {

#ifdef ENABLE_SESSIONSTATE
    int old_state = appconn->s_state.session_state;

    appconn->s_state.session_state =
        RADIUS_VALUE_COOVACHILLI_SESSION_LOCATION_CHANGE;
#endif

    if (appconn->s_state.authenticated == 1)
      acct_req(ACCT_USER, appconn, RADIUS_STATUS_TYPE_INTERIM_UPDATE);

#ifdef ENABLE_GARDENACCOUNTING
    if (_options.uamgardendata)
      acct_req(ACCT_GARDEN, appconn, RADIUS_STATUS_TYPE_INTERIM_UPDATE);
#endif

#ifdef ENABLE_SESSIONSTATE
    appconn->s_state.session_state = old_state;
#endif

  }

  return 0;
}
#endif

#ifdef ENABLE_RADPROXY

#if defined(ENABLE_LOCATION) || defined(ENABLE_PROXYVSA)
static int
chilli_proxy_radlocation(struct radius_packet_t *pack,
			 struct app_conn_t *appconn, char force) {
  struct radius_attr_t *attr = 0;
#ifdef ENABLE_PROXYVSA
  uint8_t * vsa = appconn->s_state.redir.vsa;
  int instance=0;
#endif

  if (_options.location_copy_called) {
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0)) {
      if (attr->l - 2 < sizeof(appconn->s_state.redir.called) - 1) {
	appconn->s_state.redir.calledlen = attr->l - 2;
	memcpy(appconn->s_state.redir.called, attr->v.t,
	       appconn->s_state.redir.calledlen);
	appconn->s_state.redir.called[appconn->s_state.redir.calledlen]=0;
      }
    }
  }

#ifdef ENABLE_PROXYVSA
  do {
    attr=NULL;
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC, 0, 0,
			instance++)) {

      if ((appconn->s_state.redir.vsalen + (size_t) attr->l) >
	  RADIUS_PROXYVSA) {
	syslog(LOG_WARNING, "VSAs too long");
	return -1;
      }

      memcpy(vsa + appconn->s_state.redir.vsalen,
	     (void *)attr, (size_t) attr->l);
      appconn->s_state.redir.vsalen += (size_t) attr->l;

#if(_debug_)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Remembering VSA", __FUNCTION__, __LINE__);
#endif
    }
  } while (attr);
#endif

#ifdef ENABLE_LOCATION
  if (_options.proxy_loc[0].attr) {
    int i;

    attr = 0;

    for (i=0; i < PROXYVSA_ATTR_CNT; i++) {

      if (!_options.proxy_loc[i].attr_vsa &&
	  !_options.proxy_loc[i].attr)
	break;

      if (!_options.proxy_loc[i].attr_vsa) {
	/*
	 *  We have a loc_attr, but it isn't a VSA (so not included above)
	 */

#if(_debug_)
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): looking for attr %d", __FUNCTION__, __LINE__, _options.proxy_loc[i].attr);
#endif

	if (radius_getattr(pack, &attr, _options.proxy_loc[i].attr,
			   0, 0, 0)) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): didn't find attr %d", __FUNCTION__, __LINE__, _options.proxy_loc[i].attr);
	  attr = 0;
	}
      } else {
	/*
	 *  We have a loc_attr and VSA number (so it is included above).
	 */

#if(_debug_)
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): looking for attr %d/%d", __FUNCTION__, __LINE__, _options.proxy_loc[i].attr_vsa,
                 _options.proxy_loc[i].attr);
#endif

	if (radius_getattr(pack, &attr,
			   RADIUS_ATTR_VENDOR_SPECIFIC,
			   _options.proxy_loc[i].attr_vsa,
			   _options.proxy_loc[i].attr, 0)) {
#if(_debug_)
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): didn't find attr %d/%d", __FUNCTION__, __LINE__, _options.proxy_loc[i].attr_vsa,
                   _options.proxy_loc[i].attr);
#endif
	  attr = 0;
	}
      }
    }

    if (attr && attr->l > 2) {
      chilli_learn_location(attr->v.t, attr->l - 2, appconn, force);
    }
  }
#endif

  return 0;
}
#endif

/* Handle an accounting request */
int accounting_request(struct radius_packet_t *pack,
		       struct sockaddr_in *peer) {
  struct radius_attr_t *attr = NULL;
  struct radius_attr_t *hismacattr = NULL;
  struct radius_attr_t *nasipattr = NULL;
  struct radius_attr_t *nasportattr = NULL;
  struct radius_packet_t radius_pack;
  struct app_conn_t *appconn = NULL;
  struct dhcp_conn_t *dhcpconn = NULL;
  uint8_t hismac[PKT_ETH_ALEN];
  char macstr[RADIUS_ATTR_VLEN];
  size_t macstrlen;
  unsigned int temp[PKT_ETH_ALEN];
  uint32_t nasip = 0;
  uint32_t nasport = 0;
  int status_type;
  int i;

  if (radius_default_pack(radius, &radius_pack,
			  RADIUS_CODE_ACCOUNTING_RESPONSE)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  /* We will only respond, not proxy */
  radius_pack.id = pack->id;

  /* Status type */
  if (radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0, 0)) {
    syslog(LOG_ERR, "Status type is missing from radius request");
    radius_resp(radius, &radius_pack, peer, pack->authenticator);
    return 0;
  }

  status_type = ntohl(attr->v.i);

  if (RADIUS_STATUS_TYPE_ACCOUNTING_ON  != status_type &&
      RADIUS_STATUS_TYPE_ACCOUNTING_OFF != status_type) {

    /* NAS IP */
    if (!radius_getattr(pack, &nasipattr,
			RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 0)) {
      if ((nasipattr->l-2) != sizeof(appconn->nasip)) {
	syslog(LOG_ERR, "Wrong length of NAS IP address");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }
      nasip = nasipattr->v.i;
    }

    /* NAS PORT */
    if (!radius_getattr(pack, &nasportattr,
			RADIUS_ATTR_NAS_PORT, 0, 0, 0)) {
      if ((nasportattr->l-2) != sizeof(appconn->nasport)) {
	syslog(LOG_ERR, "Wrong length of NAS port");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }
      nasport = nasportattr->v.i;
    }

    /* Calling Station ID (MAC Address) */
    if (!radius_getattr(pack, &hismacattr,
			RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Calling Station ID is: %.*s", __FUNCTION__, __LINE__,
               hismacattr->l-2, hismacattr->v.t);

      if ((macstrlen = (size_t)hismacattr->l-2) >= (RADIUS_ATTR_VLEN-1)) {
	syslog(LOG_ERR, "Wrong length of called station ID");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }

      memcpy(macstr, hismacattr->v.t, macstrlen);
      macstr[macstrlen] = 0;

      /* Replace anything but hex with space */
      for (i=0; i<macstrlen; i++)
	if (!isxdigit((int) macstr[i]))
	  macstr[i] = 0x20;

      if (sscanf (macstr, "%2x %2x %2x %2x %2x %2x",
		  &temp[0], &temp[1], &temp[2],
		  &temp[3], &temp[4], &temp[5]) != 6) {
	syslog(LOG_ERR, "Failed to convert Calling Station ID to MAC Address");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }

      for (i = 0; i < PKT_ETH_ALEN; i++)
	hismac[i] = temp[i];
    }

    if (hismacattr) {
      /*
       * Look for mac address.
       * If not found allocate new..
       */
#ifdef ENABLE_LAYER3
      if (_options.layer3) {
	syslog(LOG_ERR, "Not supported in layer3 mode");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      } else {
#endif
	if (dhcp_hashget(dhcp, &dhcpconn, hismac)) {
	  if (dhcp_newconn(dhcp, &dhcpconn, hismac)) {
	    syslog(LOG_ERR, "Out of connections");
	    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
	  }
	}
	if (!(dhcpconn->peer)) {
	  syslog(LOG_ERR, "No peer protocol defined");
	  return radius_resp(radius, &radius_pack, peer, pack->authenticator);
	}
	appconn = (struct app_conn_t *)dhcpconn->peer;
#ifdef ENABLE_LAYER3
      }
#endif
    }
    else if (nasipattr && nasportattr) { /* Look for NAS IP / Port */
      if (chilli_getconn(&appconn, 0, nasip, nasport)) {
	syslog(LOG_ERR, "Unknown connection");
	radius_resp(radius, &radius_pack, peer, pack->authenticator);
	return 0;
      }
    }
    else {
      syslog(LOG_ERR, "Calling Station ID or NAS IP/Port is missing from radius request");
      radius_resp(radius, &radius_pack, peer, pack->authenticator);
      return 0;
    }

    if (!appconn) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): No application context for RADIUS proxy", __FUNCTION__, __LINE__);
      return 0;
    }

    /* Silently ignore radius request if allready processing one */
    if (appconn->radiuswait) {
      if (appconn->radiuswait == 2) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Giving up on previous packet.. not dropping this one", __FUNCTION__, __LINE__);
	appconn->radiuswait = 0;
      } else {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): Dropping RADIUS while waiting", __FUNCTION__, __LINE__);
	appconn->radiuswait++;
	return 0;
      }
    }

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Handing RADIUS accounting proxy packet", __FUNCTION__, __LINE__);

    dhcpconn = (struct dhcp_conn_t*) appconn->dnlink;

#ifdef ENABLE_APSESSIONID
    if (!radius_getattr(pack, &attr,
			RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0)) {
      int len = attr->l-2;
      if (len >= sizeof(appconn->s_state.ap_sessionid))
	len = sizeof(appconn->s_state.ap_sessionid) - 1;
      memcpy(appconn->s_state.ap_sessionid, attr->v.t, len);
      appconn->s_state.ap_sessionid[len]=0;
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): AP Acct-Session-ID is: %s", __FUNCTION__, __LINE__,
               appconn->s_state.ap_sessionid);
    }
#endif

#if defined(ENABLE_LOCATION) || defined(ENABLE_PROXYVSA)
    switch (status_type) {
      case RADIUS_STATUS_TYPE_START:
        if (chilli_proxy_radlocation(pack, appconn, 1))
          return radius_resp(radius, &radius_pack, peer, pack->authenticator);
        break;
      case RADIUS_STATUS_TYPE_INTERIM_UPDATE:
      case RADIUS_STATUS_TYPE_STOP:
        if (chilli_proxy_radlocation(pack, appconn, 0))
          return radius_resp(radius, &radius_pack, peer, pack->authenticator);
        break;
    }
#endif

    /* -- This needs to be optional --
       case RADIUS_STATUS_TYPE_STOP:
       if (!dhcpconn) {
       syslog(LOG_ERR,"No downlink protocol defined for RADIUS proxy client");
       return 0;
       }
       dhcp_freeconn(dhcpconn, RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
       break;
    */
  }

  if (_options.proxyonacct) {
    /*
     *  Drop the in-coming packet through to the chilli
     *  RADIUS queue.
     */
    struct radius_packet_t acct_pack;
    if (!radius_default_pack(radius, &acct_pack,
			     RADIUS_CODE_ACCOUNTING_REQUEST)) {
      size_t len = ntohs(pack->length) - RADIUS_HDRSIZE;
      memcpy(acct_pack.payload, pack->payload, len);
      acct_pack.length = htons(len + RADIUS_HDRSIZE);
      radius_req(radius, &acct_pack, appconn);
    }
  }

  radius_resp(radius, &radius_pack, peer, pack->authenticator);
  return 0;
}

int access_request(struct radius_packet_t *pack,
		   struct sockaddr_in *peer) {
  struct radius_packet_t radius_pack;

  struct ippoolm_t *ipm = NULL;

  struct radius_attr_t *hisipattr = NULL;
  struct radius_attr_t *nasipattr = NULL;
  struct radius_attr_t *nasportattr = NULL;
  struct radius_attr_t *hismacattr = NULL;
  struct radius_attr_t *uidattr = NULL;
  struct radius_attr_t *pwdattr = NULL;
  struct radius_attr_t *eapattr = NULL;

  struct in_addr hisip;
  char pwd[RADIUS_ATTR_VLEN];
  size_t pwdlen;
  uint8_t hismac[PKT_ETH_ALEN];
  char macstr[RADIUS_ATTR_VLEN];
  size_t macstrlen;
  unsigned int temp[PKT_ETH_ALEN];
  int i;

  struct app_conn_t *appconn = NULL;
  struct dhcp_conn_t *dhcpconn = NULL;

  uint8_t resp[MAX_EAP_LEN];     /* EAP response */
  size_t resplen;                /* Length of EAP response */

  size_t offset = 0;
  size_t eaplen = 0;
  int instance = 0;
  uint8_t qid;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): RADIUS Access-Request received", __FUNCTION__, __LINE__);

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REJECT)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  /* result of qnext */
  qid = radius_pack.id;

  /* keep in case of reject */
  radius_pack.id = pack->id;

  /*
   *  User is identified by either IP address OR MAC address
   */

  /* Framed IP address (Conditional) */
  if (!radius_getattr(pack, &hisipattr,
		      RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
    if ((hisipattr->l-2) != sizeof(hisip.s_addr)) {
      syslog(LOG_ERR, "Wrong length of framed IP address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    hisip.s_addr = hisipattr->v.i;
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Framed IP address is: %s", __FUNCTION__, __LINE__, inet_ntoa(hisip));
  }

  /* Calling Station ID: MAC Address (Conditional) */
  if (!radius_getattr(pack, &hismacattr,
		      RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Calling Station ID is: %.*s", __FUNCTION__, __LINE__, hismacattr->l-2, hismacattr->v.t);

    if ((macstrlen = (size_t)hismacattr->l-2) >= (RADIUS_ATTR_VLEN-1)) {
      syslog(LOG_ERR, "Wrong length of calling station ID");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }

    memcpy(macstr, hismacattr->v.t, macstrlen);
    macstr[macstrlen] = 0;

    /* Replace anything but hex with space */
    for (i=0; i<macstrlen; i++)
      if (!isxdigit((int) macstr[i]))
	macstr[i] = 0x20;

    if (sscanf (macstr, "%2x %2x %2x %2x %2x %2x",
		&temp[0], &temp[1], &temp[2],
		&temp[3], &temp[4], &temp[5]) != 6) {
      syslog(LOG_ERR, "Failed to convert Calling Station ID to MAC Address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }

    for (i = 0; i < PKT_ETH_ALEN; i++)
      hismac[i] = temp[i];
  }

  /* Framed IP address or MAC Address must be given in request */
  if ((!hisipattr) && (!hismacattr)) {
    syslog(LOG_ERR, "Framed IP address or Calling Station ID is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* Username (Mandatory) */
  if (radius_getattr(pack, &uidattr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
    syslog(LOG_ERR, "User-Name is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  if (hisipattr) { /* Find user based on IP address */
    if (ippool_getip(ippool, &ipm, &hisip)) {
      syslog(LOG_ERR, "RADIUS-Request: IP Address not found");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }

    if ((appconn  = (struct app_conn_t *)ipm->peer)        == NULL ||
	(dhcpconn = (struct dhcp_conn_t *)appconn->dnlink) == NULL) {
      syslog(LOG_ERR, "RADIUS-Request: No peer protocol defined");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
  }
  else if (hismacattr) {
    /*
     * Look for mac address.
     * If not found allocate new..
     */
#ifdef ENABLE_LAYER3
    if (_options.layer3) {
      syslog(LOG_ERR, "Not supported in layer3 mode");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    } else {
#endif
      if (dhcp_hashget(dhcp, &dhcpconn, hismac)) {
	if (dhcp_newconn(dhcp, &dhcpconn, hismac)) {
	  syslog(LOG_ERR, "Out of connections");
	  return radius_resp(radius, &radius_pack, peer, pack->authenticator);
	}
      }
      if (!(dhcpconn->peer)) {
	syslog(LOG_ERR, "No peer protocol defined");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }
      appconn = (struct app_conn_t *)dhcpconn->peer;
#ifdef ENABLE_LAYER3
    }
#endif
  }
  else {
    syslog(LOG_ERR, "Framed-IP-Address or Calling-Station-ID required in RADIUS request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* Silently ignore radius request if already processing one */
  if (appconn->radiuswait) {
    if (appconn->radiuswait == 2) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Giving up on previous packet.. not dropping this one", __FUNCTION__, __LINE__);
      appconn->radiuswait = 0;
    } else {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Dropping RADIUS while waiting", __FUNCTION__, __LINE__);
      appconn->radiuswait++;
      return 0;
    }
  }

  dhcpconn->lasttime = mainclock_now();

  /* Password */
  if (!radius_getattr(pack, &pwdattr, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0)) {
    if (radius_pwdecode(radius, (uint8_t*) pwd, RADIUS_ATTR_VLEN, &pwdlen,
			pwdattr->v.t, pwdattr->l-2, pack->authenticator,
			radius->proxysecret,
			radius->proxysecretlen)) {
      syslog(LOG_ERR, "radius_pwdecode() failed");
      return -1;
    }
#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Password is: %s", __FUNCTION__, __LINE__, pwd);
#endif
  }

  /* Get EAP message */
  resplen = 0;
  do {
    eapattr=NULL;
    if (!radius_getattr(pack, &eapattr, RADIUS_ATTR_EAP_MESSAGE, 0, 0,
			instance++)) {
      if ((resplen + (size_t)eapattr->l-2) > MAX_EAP_LEN) {
	syslog(LOG_INFO, "EAP message too long %zu %d", resplen, (int)eapattr->l-2);
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }
      memcpy(resp + resplen, eapattr->v.t, (size_t)eapattr->l-2);
      resplen += (size_t)eapattr->l-2;
    }
  } while (eapattr);

  if (resplen) {
    appconn->dnprot = DNPROT_WPA;
  }

#if defined(ENABLE_LOCATION) || defined(ENABLE_PROXYVSA)
  if (chilli_proxy_radlocation(pack, appconn, 1))
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
#endif

  /* Passwd or EAP must be given in request */
  if ((!pwdattr) && (!resplen)) {
    syslog(LOG_ERR, "Password or EAP message is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

#ifdef ENABLE_RADPROXY
  if (_options.proxymacaccept && !resplen) {
    syslog(LOG_INFO, "Accepting MAC login");
    radius_pack.code = RADIUS_CODE_ACCESS_ACCEPT;
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }
#endif

  /* CoovaChilli Notes:
     Dublicate logins should be allowed as it might be the terminal
     moving from one access point to another. It is however
     unacceptable to login with another username on top of an allready
     existing connection

     TODO: New username should be allowed, but should result in
     a accounting stop message for the old connection.
     this does however pose a denial of service attack possibility

     If allready logged in send back accept message with username
     TODO ? Should this be a reject: Dont login twice ?
  */

  /* Terminate previous session if trying to login with another username */
  if ((appconn->s_state.authenticated == 1) &&
      ((strlen(appconn->s_state.redir.username) != uidattr->l-2) ||
       (memcmp(appconn->s_state.redir.username, uidattr->v.t, uidattr->l-2)))) {
    terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
    if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REJECT)) {
      syslog(LOG_ERR, "radius_default_pack() failed");
      return -1;
    }
  }

  /* NAS IP */
  if (!radius_getattr(pack, &nasipattr, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 0)) {
    if ((nasipattr->l-2) != sizeof(appconn->nasip)) {
      syslog(LOG_ERR, "Wrong length of NAS IP address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn->nasip = nasipattr->v.i;
  }

  /* NAS PORT */
  if (!radius_getattr(pack, &nasportattr, RADIUS_ATTR_NAS_PORT, 0, 0, 0)) {
    if ((nasportattr->l-2) != sizeof(appconn->nasport)) {
      syslog(LOG_ERR, "Wrong length of NAS port");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn->nasport = nasportattr->v.i;
  }

  /* Store parameters for later use */
  if (uidattr->l-2 < USERNAMESIZE) {
    memcpy(appconn->s_state.redir.username,
	   (char *)uidattr->v.t, uidattr->l-2);
    appconn->s_state.redir.username[uidattr->l-2]=0;
  }


  appconn->radiuswait = 1;
  appconn->radiusid = pack->id;

  if (pwdattr)
    appconn->authtype = PAP_PASSWORD;
  else
    appconn->authtype = EAP_MESSAGE;

  memcpy(&appconn->radiuspeer, peer, sizeof(*peer));
  memcpy(appconn->authenticator, pack->authenticator, RADIUS_AUTHLEN);
  memcpy(appconn->hismac, dhcpconn->hismac, PKT_ETH_ALEN);

  /* Build up radius request */
  radius_pack.code = RADIUS_CODE_ACCESS_REQUEST;

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 uidattr->v.t, uidattr->l - 2);

  if (appconn->s_state.redir.statelen) {
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_STATE, 0, 0, 0,
		   appconn->s_state.redir.statebuf,
		   appconn->s_state.redir.statelen);
  }

  if (pwdattr)
    radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		   (uint8_t*) pwd, pwdlen);

  /* Include EAP (if present) */
  offset = 0;
  while (offset < resplen) {

    if ((resplen - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = resplen - offset;

    radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		   resp + offset, eaplen);

    offset += eaplen;
  }

  if (resplen) {
    if (_options.wpaguests)
      radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		     RADIUS_VENDOR_COOVACHILLI, RADIUS_ATTR_COOVACHILLI_CONFIG,
		     0, (uint8_t*)"allow-wpa-guests", 16);
  }

  chilli_req_attrs(radius, &radius_pack,
		   ACCT_USER,
		   _options.framedservice ? RADIUS_SERVICE_TYPE_FRAMED :
		   RADIUS_SERVICE_TYPE_LOGIN, 0,
		   appconn->unit, appconn->hismac,
		   &appconn->hisip, &appconn->s_state);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  /* restore request id to that of queue */
  radius_pack.id = qid;

  return radius_req(radius, &radius_pack, appconn);
}

/*********************************************************
 *
 * radius proxy callback functions (request from radius server)
 *
 *********************************************************/

/* Radius callback when radius request has been received */
int cb_radius_ind(struct radius_t *rp, struct radius_packet_t *pack,
		  struct sockaddr_in *peer) {

  if (rp != radius) {
    syslog(LOG_ERR, "Radius callback from unknown instance");
    return 0;
  }

  switch (pack->code) {
    case RADIUS_CODE_ACCOUNTING_REQUEST:
      return accounting_request(pack, peer);
    case RADIUS_CODE_ACCESS_REQUEST:
      return access_request(pack, peer);
    default:
      syslog(LOG_ERR, "Unsupported radius request received: %d", pack->code);
      return 0;
  }
}
#endif

static int
session_disconnect(struct app_conn_t *appconn,
		   struct dhcp_conn_t *dhcpconn,
		   int term_cause) {

#ifdef ENABLE_MODULES
  { int i;
    for (i=0; i < MAX_MODULES; i++) {
      if (!_options.modules[i].name[0]) break;
      if (_options.modules[i].ctx) {
        struct chilli_module *m =
	    (struct chilli_module *)_options.modules[i].ctx;
        if (m->dhcp_disconnect)
          m->dhcp_disconnect(appconn, dhcpconn);
      }
    }
  }
#endif

  terminate_appconn(appconn,
		    term_cause ? term_cause :
		    appconn->s_state.terminate_cause ?
		    appconn->s_state.terminate_cause :
		    RADIUS_TERMINATE_CAUSE_LOST_CARRIER);

  if (appconn->uplink) {
    struct ippoolm_t *member = (struct ippoolm_t *) appconn->uplink;

#ifdef ENABLE_UAMANYIP
    if (_options.uamanyip) {
      if (!appconn->natip.s_addr) {
	if (member->in_use && member->is_static) {
	  struct in_addr mask;
	  int res;
	  mask.s_addr = 0xffffffff;
	  res = net_del_route(&member->addr, &appconn->ourip, &mask);
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): Removing route: %s %d", __FUNCTION__, __LINE__, inet_ntoa(member->addr), res);
	}
      } else {
	struct ippoolm_t *natipm;
	if (ippool_getip(ippool, &natipm, &appconn->natip) == 0) {
	  if (ippool_freeip(ippool, natipm)) {
	    syslog(LOG_ERR, "ippool_freeip(%s) failed for nat ip!",
                   inet_ntoa(appconn->natip));
	  }
	}
      }
    }
#endif

    if (member->in_use && (!dhcpconn || !dhcpconn->is_reserved)) {
      if (ippool_freeip(ippool, member)) {
	syslog(LOG_ERR, "ippool_freeip(%s) failed!",
               inet_ntoa(member->addr));
      }
    }

#if defined(ENABLE_TAP) && defined(SIOCDARP)
    if (_options.usetap) {
      /*
       *    USETAP ARP
       */
      int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      if (sockfd > 0) {
	struct arpreq req;

	memset(&req, 0, sizeof(req));

	SET_SA_FAMILY(req.arp_pa, AF_INET);
	((struct sockaddr_in *) &req.arp_pa)->sin_addr.s_addr =
            appconn->hisip.s_addr;
	req.arp_flags = ATF_PERM | ATF_PUBL;

	strlcpy(req.arp_dev, tuntap(tun).devname, sizeof(req.arp_dev));

	if (ioctl(sockfd, SIOCDARP, &req) < 0) {
	  perror("ioctrl()");
	}

	safe_close(sockfd);
      }
    }
#endif
  }

  if (_options.macdown) {
    syslog(LOG_DEBUG, "%s(%d): Calling MAC down script: %s", __FUNCTION__, __LINE__, _options.macdown);
    runscript(appconn, _options.macdown, 0, 0);
  }

  if (!dhcpconn || !dhcpconn->is_reserved) {
    freeconn(appconn);
    if (dhcpconn)
      dhcpconn->peer = 0;
  }

#ifdef ENABLE_BINSTATFILE
  if (_options.statusfilesave)
    printstatus();
#endif

  return 0;
}

static int
upprot_getip(struct app_conn_t *appconn,
	     struct in_addr *hisip,
	     struct in_addr *hismask) {
  struct ippoolm_t *ipm = 0;

  struct dhcp_conn_t *dhcpconn = (struct dhcp_conn_t *)appconn->dnlink;

#if(_debug_ > 1)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): UPPROT - GETIP", __FUNCTION__, __LINE__);
#endif

  /* If IP address is already allocated: Fill it in */
  /* This should only happen for UAM */
  if (appconn->uplink) {
    ipm = (struct ippoolm_t *)appconn->uplink;
  }

  if (ipm == 0) {
    /* Allocate static or dynamic IP address */

    if (newip(&ipm, hisip, dhcpconn ? dhcpconn->hismac : 0))
      return dnprot_reject(appconn);

    appconn->hisip.s_addr = ipm->addr.s_addr;

    if (hismask && hismask->s_addr)
      appconn->hismask.s_addr = hismask->s_addr;
    else
      appconn->hismask.s_addr = _options.mask.s_addr;

    /* TODO: Too many "listen" and "ourip" addresses! */
    if (!appconn->ourip.s_addr)
      appconn->ourip.s_addr = _options.dhcplisten.s_addr;

    appconn->uplink = ipm;
    ipm->peer = appconn;

#ifdef ENABLE_GARDENACCOUNTING
    if (_options.uamgardendata) {
      acct_req(ACCT_GARDEN, appconn, RADIUS_STATUS_TYPE_START);
    }
#endif
  }

#ifdef ENABLE_UAMANYIP
  if (chilli_assign_snat(appconn, 0) != 0) {
    return -1;
  }
#endif

  return dnprot_accept(appconn);
}

void session_param_defaults(struct session_params *params) {

  if (_options.defsessiontimeout && !params->sessiontimeout)
    params->sessiontimeout = _options.defsessiontimeout;

  if (_options.defidletimeout && !params->idletimeout)
    params->idletimeout = _options.defidletimeout;

  if (_options.defbandwidthmaxdown && !params->bandwidthmaxdown)
    params->bandwidthmaxdown = _options.defbandwidthmaxdown;

  if (_options.defbandwidthmaxup && !params->bandwidthmaxup)
    params->bandwidthmaxup = _options.defbandwidthmaxup;

  if (_options.definteriminterval && !params->interim_interval)
    params->interim_interval = _options.definteriminterval;
}

void
config_radius_session(struct session_params *params,
		      struct radius_packet_t *pack,
		      struct app_conn_t *appconn,
		      int reconfig) {
  struct radius_attr_t *attr = NULL;
  size_t offset = 0;
  /*int is_splash = 0;*/
  int seen;

  /* Session timeout */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_SESSION_TIMEOUT, 0, 0, 0))
    params->sessiontimeout = ntohl(attr->v.i);
  else if (!reconfig)
    params->sessiontimeout = 0;

  /* Idle timeout */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_IDLE_TIMEOUT, 0, 0, 0))
    params->idletimeout = ntohl(attr->v.i);
  else if (!reconfig)
    params->idletimeout = 0;

  /* Filter ID */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_FILTER_ID, 0, 0, 0)) {
    params->filteridlen = attr->l-2;
    memcpy(params->filteridbuf, attr->v.t, attr->l-2);
    params->filteridbuf[attr->l-2] = 0;
  }
  else if (!reconfig) {
    params->filteridlen = 0;
    params->filteridbuf[0] = 0;
  }

  /* Interim interval */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_ACCT_INTERIM_INTERVAL, 0, 0, 0)) {
    params->interim_interval = ntohl(attr->v.i);
    if (params->interim_interval < 60) {
      syslog(LOG_ERR, "Received too small radius Acct-Interim-Interval: %d; resettings to default.",
             params->interim_interval);
      params->interim_interval = 0;
    }
  }
  else if (!reconfig)
    params->interim_interval = 0;

  /* Bandwidth up */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP, 0))
    params->bandwidthmaxup = ntohl(attr->v.i);
  else if (!reconfig)
    params->bandwidthmaxup = 0;

  /* Bandwidth down */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN, 0))
    params->bandwidthmaxdown = ntohl(attr->v.i);
  else if (!reconfig)
    params->bandwidthmaxdown = 0;

#ifdef RADIUS_ATTR_COOVACHILLI_BANDWIDTH_MAX_UP
  /* Bandwidth up */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_BANDWIDTH_MAX_UP, 0))
    params->bandwidthmaxup = ntohl(attr->v.i) * 1000;
#endif

#ifdef RADIUS_ATTR_COOVACHILLI_BANDWIDTH_MAX_DOWN
  /* Bandwidth down */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_BANDWIDTH_MAX_DOWN, 0))
    params->bandwidthmaxdown = ntohl(attr->v.i) * 1000;
#endif

  /* Max input octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_MAX_INPUT_OCTETS, 0))
    params->maxinputoctets = ntohl(attr->v.i);
  else if (!reconfig)
    params->maxinputoctets = 0;

  /* Max output octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_MAX_OUTPUT_OCTETS, 0))
    params->maxoutputoctets = ntohl(attr->v.i);
  else if (!reconfig)
    params->maxoutputoctets = 0;

  /* Max total octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_MAX_TOTAL_OCTETS, 0))
    params->maxtotaloctets = ntohl(attr->v.i);
  else if (!reconfig)
    params->maxtotaloctets = 0;


  /* Max input gigawords */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_MAX_INPUT_GIGAWORDS, 0))
    params->maxinputoctets |= ((uint64_t)ntohl(attr->v.i) & 0xffffffff) << 32;

  /* Max output gigawords */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_MAX_OUTPUT_GIGAWORDS, 0))
    params->maxoutputoctets |= ((uint64_t)ntohl(attr->v.i) & 0xffffffff) << 32;

  /* Max total octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_MAX_TOTAL_GIGAWORDS, 0))
    params->maxtotaloctets |= ((uint64_t)ntohl(attr->v.i) & 0xffffffff) << 32;

  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_REQUIRE_UAM, 0)) {
    memcpy(params->url, attr->v.t, attr->l-2);
    params->url[attr->l-2] = 0;
  }

#ifdef ENABLE_REDIRINJECT
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_COOVACHILLI,
		      RADIUS_ATTR_COOVACHILLI_INJECT_URL, 0)) {
    memcpy(params->url, attr->v.t, attr->l-2);
    params->url[attr->l-2] = 0;
    params->flags |= UAM_INJECT_URL | REQUIRE_UAM_AUTH;
  }
#endif

#ifdef EX_CONFIG_RADIUS_SESSION
#include EX_CONFIG_RADIUS_SESSION
#endif

#ifdef ENABLE_MULTIROUTE
  if (tun) {
    /* Route Index, look-up by interface name */
    if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			RADIUS_VENDOR_COOVACHILLI,
			RADIUS_ATTR_COOVACHILLI_ROUTE_TO_INTERFACE, 0)) {
      char name[256];
      memcpy(name, attr->v.t, attr->l-2);
      name[attr->l-2] = 0;
      params->routeidx = tun_name2idx(tun, name);
    }
    else if (!reconfig) {
      params->routeidx = tun->routeidx;
    }
  }
#endif

#ifdef ENABLE_COOVACHILLICONFIG
  {
    /*
     *  Looking for CoovaChilli-Config attributes with
     *  special messages.
     */
    const char *adminreset = "admin-reset";
    const char *uamauth = "require-uam-auth";
    const char *splash = "splash";
    const char *logout = "logout";

#ifdef ENABLE_SESSGARDEN
    const char *uamallowed = "uamallowed=";
    const int uamallowed_len = strlen(uamallowed);
#endif

    while (!radius_getnextattr(pack, &attr,
			       RADIUS_ATTR_VENDOR_SPECIFIC,
			       RADIUS_VENDOR_COOVACHILLI,
			       RADIUS_ATTR_COOVACHILLI_CONFIG,
			       0, &offset)) {
      size_t len = (size_t) attr->l - 2;
      char *val = (char *) attr->v.t;

      if (len == strlen(uamauth) && !memcmp(val, uamauth, len)) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): received require-uam-auth", __FUNCTION__, __LINE__);
	params->flags |= REQUIRE_UAM_AUTH;
      }
      else if (len == strlen(splash) && !memcmp(val, splash, len)) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): received splash response", __FUNCTION__, __LINE__);
	params->flags |= REQUIRE_UAM_SPLASH;
	/*is_splash = 1;*/
      }
#ifdef ENABLE_SESSGARDEN
      else if (len > uamallowed_len && len < 255 &&
	       !memcmp(val, uamallowed, uamallowed_len)) {
	char name[256];

	/* copy and null-terminate */
	len -= uamallowed_len;
	val += uamallowed_len;
	memcpy(name, val, len);
	name[len]=0;

	if (len == 5 && !memcmp(name,"reset",5)) {
	  params->pass_through_count = 0;
#ifdef HAVE_PATRICIA
	  if (appconn && appconn->ptree) {
	    patricia_destroy (appconn->ptree, free);
	    appconn->ptree = NULL;
	  }
#endif
	} else {

#ifdef HAVE_PATRICIA
	  if (appconn && appconn->ptree == NULL)
	    appconn->ptree = patricia_new (32);
#endif

	  pass_throughs_from_string(params->pass_throughs,
				    SESSION_PASS_THROUGH_MAX,
				    &params->pass_through_count,
				    name, 0, 0
#ifdef HAVE_PATRICIA
				    , appconn ? appconn->ptree : 0
#endif
				    );
	}
      }
#endif
      else if (appconn && len >= strlen(logout) &&
	       !memcmp(val, logout, strlen(logout))) {
	if (appconn)
	  terminate_appconn(appconn,
			    RADIUS_TERMINATE_CAUSE_USER_REQUEST);
      } else if (appconn && len >= strlen(adminreset) &&
		 !memcmp(val, adminreset, strlen(adminreset))) {
	dhcp_release_mac(dhcp, appconn->hismac,
			 RADIUS_TERMINATE_CAUSE_ADMIN_RESET);
      }
    }
  }
#endif

  seen = 0;
  offset = 0;
  while (!radius_getnextattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
			     RADIUS_VENDOR_WISPR,
			     RADIUS_ATTR_WISPR_REDIRECTION_URL,
			     0, &offset)) {
    ssize_t clen, nlen = (ssize_t) attr->l - 2;
    char *url = (char*) attr->v.t;

    if (seen == 0) { params->url[0]=0; seen=1; }
    clen = strlen((char*)params->url);

    if (clen + nlen > sizeof(params->url) - 1)
      nlen = sizeof(params->url) - clen - 1;

    if (nlen > 0) {
      memcpy(params->url + clen, url, nlen);
      params->url[clen + nlen] = 0;
      params->flags |= UAM_CLEAR_URL;
    }

    /*if (!is_splash) {
      params->flags |= REQUIRE_REDIRECT;
      }*/
  }

  /* Session-Terminate-Time */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_SESSION_TERMINATE_TIME, 0)) {
    char attrs[RADIUS_ATTR_VLEN + 1];
    struct tm stt;
    int tzhour, tzmin;
    char *tz;
    int result;

    memcpy(attrs, attr->v.t, attr->l-2);
    attrs[attr->l-2] = 0;

    memset(&stt, 0, sizeof(stt));

    result = sscanf(attrs, "%d-%d-%dT%d:%d:%d %d:%d",
		    &stt.tm_year, &stt.tm_mon, &stt.tm_mday,
		    &stt.tm_hour, &stt.tm_min, &stt.tm_sec,
		    &tzhour, &tzmin);

    if (result == 8) { /* Timezone */
      /* tzhour and tzmin is hours and minutes east of GMT */
      /* timezone is defined as seconds west of GMT. Excludes DST */
      stt.tm_year -= 1900;
      stt.tm_mon  -= 1;
      stt.tm_hour -= tzhour; /* Adjust for timezone */
      stt.tm_min  -= tzmin;  /* Adjust for timezone */
      /*      stt.tm_hour += daylight;*/
      /*stt.tm_min  -= (timezone / 60);*/
      tz = getenv("TZ");
      setenv("TZ", "", 1); /* Set environment to UTC */
      tzset();
      params->sessionterminatetime = mktime(&stt);
      if (tz) setenv("TZ", tz, 1);
      else    unsetenv("TZ");
      tzset();
    }
    else if (result >= 6) { /* Local time */
      tzset();
      stt.tm_year -= 1900;
      stt.tm_mon  -= 1;
      stt.tm_isdst = -1; /*daylight;*/
      params->sessionterminatetime = mktime(&stt);
    }
    else {
      params->sessionterminatetime = 0;
      syslog(LOG_WARNING, "Invalid WISPr-Session-Terminate-Time received: %s", attrs);
    }
  }
  else if (!reconfig)
    params->sessionterminatetime = 0;

  session_param_defaults(params);
}

static int chilliauth_cb(struct radius_t *radius,
			 struct radius_packet_t *pack,
			 struct radius_packet_t *pack_req,
			 void *cbp) {

  struct radius_attr_t *attr = NULL;
  size_t offset = 0;

  if (!pack) {
    syslog(LOG_ERR, "Radius request timed out");
    return 0;
  }

  if ((pack->code != RADIUS_CODE_ACCESS_REJECT) &&
      (pack->code != RADIUS_CODE_ACCESS_CHALLENGE) &&
      (pack->code != RADIUS_CODE_ACCESS_ACCEPT)) {
    syslog(LOG_ERR, "Unknown radius access reply code %d", pack->code);
    return 0;
  }

  /* ACCESS-ACCEPT */
  if (pack->code != RADIUS_CODE_ACCESS_ACCEPT) {
    syslog(LOG_ERR, "Administrative-User Login Failed");
    return 0;
  }

  if (_options.adminupdatefile) {

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): looking to replace: %s", __FUNCTION__, __LINE__, _options.adminupdatefile);

    if (!radius_getnextattr(pack, &attr,
			    RADIUS_ATTR_VENDOR_SPECIFIC,
			    RADIUS_VENDOR_COOVACHILLI,
			    RADIUS_ATTR_COOVACHILLI_CONFIG,
			    0, &offset)) {

      char template[] = "/tmp/hs.conf.XXXXXX";
      char * hs_conf = _options.adminupdatefile;

      /*
       *  We have configurations in the administrative-user session.
       *  Save to a temporary file.
       */

      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): using template temp file: %s", __FUNCTION__, __LINE__, template);

      int fd = mkstemp(template);
      if (fd > 0) {
        do {
          if (safe_write(fd, attr->v.t, attr->l - 2) < 0 || safe_write(fd, "\n", 1) < 0) {
            syslog(LOG_ERR, "%s: adminupdatefile", strerror(errno));
            break;
           }
         }
         while (!radius_getnextattr(pack, &attr,
                  RADIUS_ATTR_VENDOR_SPECIFIC,
                  RADIUS_VENDOR_COOVACHILLI,
                  RADIUS_ATTR_COOVACHILLI_CONFIG,
                  0, &offset));

      /*
       *  Check to see if this file is different from the chilli/hs.conf
       */
      {
	int oldfd = open(hs_conf, O_RDONLY);

	if (fd > 0) {
     lseek(fd, SEEK_SET, 0);
	  int differ = (oldfd > 0) ? 0 : 1;
	  char b1[100], b2[100];
	  ssize_t r1, r2;

	  if (!differ) {
	    do {
	      r1 = safe_read(fd, b1, sizeof(b1));
	      r2 = safe_read(oldfd, b2, sizeof(b2));

	      if (r1 != r2 || strncmp(b1, b2, r1))
		differ = 1;
	    }
	    while (!differ && r1 > 0 && r2 > 0);
	  }

	  if (oldfd) {
		  safe_close(oldfd);
		  oldfd=0;
	  }

	  if (differ) {
            if (_options.debug)
              syslog(LOG_DEBUG, "%s(%d): Writing out new hs.conf file with administraive-user settings", __FUNCTION__, __LINE__);

	    oldfd = open(hs_conf, O_RDWR | O_TRUNC | O_CREAT, 0644);

	    if (fd > 0 && oldfd > 0) {
        lseek(fd, SEEK_SET, 0);

	      while ((r1 = safe_read(fd, b1, sizeof(b1))) > 0 &&
		     safe_write(oldfd, b1, r1) > 0);

	      safe_close(oldfd); oldfd=0;
	      do_interval = 1;
	    }
	  }
	}
	if (oldfd > 0) safe_close(oldfd);
      }

      /* unlink(hs_temp); */
		  safe_close(fd);
		}
    }
  }

  if (!admin_session.s_state.authenticated) {
    admin_session.s_state.authenticated = 1;
    acct_req(ACCT_USER, &admin_session, RADIUS_STATUS_TYPE_START);
  }

  /* reset these values to zero */
  admin_session.s_params.idletimeout = 0;
  admin_session.s_params.sessionterminatetime = 0;

  /* should instead honor this with a re-auth (see interval) */
  admin_session.s_params.sessiontimeout = 0;

  return 0;
}

int cb_radius_acct_conf(struct radius_t *radius,
			struct radius_packet_t *pack,
			struct radius_packet_t *pack_req, void *cbp) {
  struct app_conn_t *appconn = (struct app_conn_t*) cbp;

  if (!appconn) {
    syslog(LOG_ERR,"No peer protocol defined");
    return 0;
  }

  if (!pack) /* Timeout */
    return 0;

  config_radius_session(&appconn->s_params, pack, appconn, 1);
  return 0;
}

/*********************************************************
 * radius callback functions (response from radius server)
 *********************************************************/
/* Radius callback when access accept/reject/challenge has been received */
int cb_radius_auth_conf(struct radius_t *radius,
			struct radius_packet_t *pack,
			struct radius_packet_t *pack_req, void *cbp) {
  struct radius_attr_t *hisipattr = NULL;
  struct radius_attr_t *lmntattr = NULL;
  struct radius_attr_t *sendattr = NULL;
  struct radius_attr_t *recvattr = NULL;
  struct radius_attr_t *succattr = NULL;

  struct radius_attr_t *stateattr = NULL;
  struct radius_attr_t *classattr = NULL;
  struct radius_attr_t *uidattr = NULL;

#ifdef ENABLE_RADPROXY
  int instance = 0;
  struct radius_attr_t *policyattr = NULL;
  struct radius_attr_t *typesattr = NULL;
  struct radius_attr_t *eapattr = NULL;
#endif

  int force_ip = 0;
  struct in_addr hisip;
  struct in_addr hismask;

  struct app_conn_t *appconn = (struct app_conn_t*) cbp;

  struct dhcp_conn_t *dhcpconn = NULL;

  if (!appconn) {
    syslog(LOG_ERR,"No peer protocol defined");
    return 0;
  }

  dhcpconn = (struct dhcp_conn_t *)appconn->dnlink;

  /* Initialise */
  appconn->s_state.redir.statelen = 0;
  hisip.s_addr = hismask.s_addr = 0;

#ifdef ENABLE_RADPROXY
  appconn->challen  = 0;
  appconn->sendlen  = 0;
  appconn->recvlen  = 0;
  appconn->lmntlen  = 0;
#endif

  if (!pack) { /* Timeout */
    syslog(LOG_ERR, "RADIUS request id=%d timed out for session %s",
           pack_req ? pack_req->id : -1,
           appconn->s_state.sessionid);
    if (_options.noradallow) {
      session_param_defaults(&appconn->s_params);
      return upprot_getip(appconn, &appconn->reqip, 0);
    }
    return dnprot_reject(appconn);
  }

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Received RADIUS response id=%d", __FUNCTION__, __LINE__, pack->id);
#endif


  /* Framed IP address (Optional) */
  if (!radius_getattr(pack, &hisipattr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)
#ifdef ENABLE_DHCPRADIUS
      || !radius_getattr(pack, &hisipattr,
			 RADIUS_ATTR_VENDOR_SPECIFIC,
			 RADIUS_VENDOR_COOVACHILLI,
			 RADIUS_ATTR_COOVACHILLI_DHCP_IP_ADDRESS, 0)
#endif
      ) {
    if ((hisipattr->l-2) != sizeof(struct in_addr)) {
      syslog(LOG_ERR, "Wrong length of framed IP address");
      return dnprot_reject(appconn);
    }
    force_ip = 1;
    hisip.s_addr = hisipattr->v.i;

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Framed IP address set to: %s", __FUNCTION__, __LINE__, inet_ntoa(hisip));

    if (!radius_getattr(pack, &hisipattr, RADIUS_ATTR_FRAMED_IP_NETMASK, 0, 0, 0)
#ifdef ENABLE_DHCPRADIUS
	|| !radius_getattr(pack, &hisipattr,
			   RADIUS_ATTR_VENDOR_SPECIFIC,
			   RADIUS_VENDOR_COOVACHILLI,
			   RADIUS_ATTR_COOVACHILLI_DHCP_IP_NETMASK, 0)
#endif
	) {
      if ((hisipattr->l-2) != sizeof(struct in_addr)) {
	syslog(LOG_ERR, "Wrong length of framed IP netmask");
	return dnprot_reject(appconn);
      }
      hismask.s_addr = hisipattr->v.i;

      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Framed IP netmask set to: %s", __FUNCTION__, __LINE__, inet_ntoa(hismask));
    }
  }
  else {
    hisip.s_addr = appconn->reqip.s_addr;
  }

  if (force_ip) {
    if (appconn->uplink) {
      struct ippoolm_t *ipm = (struct ippoolm_t *)appconn->uplink;

      if (hisip.s_addr) {
	/*
	 *  Force the assigment of an IP address.
	 */
	if (ipm->addr.s_addr != hisip.s_addr) {
	  uint8_t hwaddr[sizeof(dhcpconn->hismac)];
	  memcpy(hwaddr, dhcpconn->hismac, sizeof(hwaddr));

          if (_options.debug) {
            syslog(LOG_DEBUG, "%s(%d): Old ip address freed %s", __FUNCTION__, __LINE__, inet_ntoa(ipm->addr));
            syslog(LOG_DEBUG, "%s(%d): Resetting ip address to %s", __FUNCTION__, __LINE__, inet_ntoa(hisip));
          }

	  dhcp_freeconn(dhcpconn, 0);
	  dhcp_newconn(dhcp, &dhcpconn, hwaddr);

	  appconn->dnprot = DNPROT_MAC;
	  appconn->authtype = PAP_PASSWORD;
	  dhcpconn->authstate = DHCP_AUTH_DNAT;

	  ipm = 0;
	}
      }
    }
  }

#ifdef ENABLE_DHCPRADIUS
  if (_options.dhcpradius) {
    struct radius_attr_t *attr = NULL;
    if (dhcpconn) {
      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_SERVER_NAME, 0)) {
	memcpy(dhcpconn->dhcp_opts.sname, attr->v.t, attr->l-2);
      }

      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_FILENAME, 0)) {
	memcpy(dhcpconn->dhcp_opts.file, attr->v.t, attr->l-2);
      }

      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_OPTION, 0)) {
	memcpy(dhcpconn->dhcp_opts.options, attr->v.t,
	       dhcpconn->dhcp_opts.option_length = attr->l-2);
      }

      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_DNS1, 0)) {
	if ((attr->l-2) == sizeof(struct in_addr)) {
	  appconn->dns1.s_addr = attr->v.i;
	  dhcpconn->dns1.s_addr = attr->v.i;
	}
      }

      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_DNS2, 0)) {
	if ((attr->l-2) == sizeof(struct in_addr)) {
	  appconn->dns2.s_addr = attr->v.i;
	  dhcpconn->dns2.s_addr = attr->v.i;
	}
      }

      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_GATEWAY, 0)) {
	if ((attr->l-2) == sizeof(struct in_addr)) {
	  appconn->ourip.s_addr = attr->v.i;
	  dhcpconn->ourip.s_addr = attr->v.i;
	}
      }

      if (!radius_getattr(pack, &attr,
			  RADIUS_ATTR_VENDOR_SPECIFIC,
			  RADIUS_VENDOR_COOVACHILLI,
			  RADIUS_ATTR_COOVACHILLI_DHCP_DOMAIN, 0)) {
	if (attr->l-2 < DHCP_DOMAIN_LEN) {
	  strncpy(dhcpconn->domain, (char *)attr->v.t, attr->l-2);
	  dhcpconn->domain[attr->l-2]=0;
	}
      }
    }
  }
#endif

  /* ACCESS-REJECT */
  if (pack->code == RADIUS_CODE_ACCESS_REJECT) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Received RADIUS Access-Reject", __FUNCTION__, __LINE__);
    config_radius_session(&appconn->s_params, pack, appconn, 0); /*XXX*/
    return dnprot_reject(appconn);
  }

  /* Get State */
  if (!radius_getattr(pack, &stateattr, RADIUS_ATTR_STATE, 0, 0, 0)) {
    appconn->s_state.redir.statelen = stateattr->l-2;
    memcpy(appconn->s_state.redir.statebuf, stateattr->v.t, stateattr->l-2);
  }

#ifdef ENABLE_RADPROXY
  /* ACCESS-CHALLENGE */
  if (pack->code == RADIUS_CODE_ACCESS_CHALLENGE) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Received RADIUS Access-Challenge", __FUNCTION__, __LINE__);

    /* Get EAP message */
    appconn->challen = 0;
    do {
      eapattr=NULL;
      if (!radius_getattr(pack, &eapattr, RADIUS_ATTR_EAP_MESSAGE, 0, 0, instance++)) {
	if ((appconn->challen + eapattr->l-2) > MAX_EAP_LEN) {
	  syslog(LOG_INFO, "EAP message too long %zu %d",
                 appconn->challen, (int) eapattr->l-2);
	  return dnprot_reject(appconn);
	}
	memcpy(appconn->chal+appconn->challen, eapattr->v.t, eapattr->l-2);
	appconn->challen += eapattr->l-2;
      }
    } while (eapattr);

    if (!appconn->challen) {
      syslog(LOG_INFO, "No EAP message found");
      return dnprot_reject(appconn);
    }

    return dnprot_challenge(appconn);
  }
#endif

  /* ACCESS-ACCEPT */
  if (pack->code != RADIUS_CODE_ACCESS_ACCEPT) {
    syslog(LOG_ERR, "Unknown RADIUS code");
    return dnprot_reject(appconn);
  }

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Received RADIUS Access-Accept", __FUNCTION__, __LINE__);
#endif

  if (!radius_getattr(pack, &uidattr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
    if (uidattr->l-2 < USERNAMESIZE) {
      memcpy(appconn->s_state.redir.username,
        (char *)uidattr->v.t, uidattr->l-2);
        appconn->s_state.redir.username[uidattr->l-2]=0;
    }
#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Received User-Name override from RADIUS Access-Accept: %s", __FUNCTION__, __LINE__, appconn->s_state.redir.username);
#endif
  }

  /* Class */
  if (!radius_getattr(pack, &classattr, RADIUS_ATTR_CLASS, 0, 0, 0)) {
    appconn->s_state.redir.classlen = classattr->l-2;
    memcpy(appconn->s_state.redir.classbuf, classattr->v.t, classattr->l-2);
    /*syslog("!!!! CLASSLEN = %d !!!!", appconn->s_state.redir.classlen);*/
  }
  else {
    /*syslog("!!!! RESET CLASSLEN !!!!");*/
    appconn->s_state.redir.classlen = 0;
  }

  config_radius_session(&appconn->s_params, pack, appconn, 0);

  if (appconn->is_adminsession) {
    /* for the admin session */
    return chilliauth_cb(radius, pack, pack_req, cbp);
  }

  if (appconn->s_params.sessionterminatetime) {
    if (mainclock_rtdiff(appconn->s_params.sessionterminatetime) > 0) {
      syslog(LOG_WARNING, "WISPr-Session-Terminate-Time in the past received, rejecting");
      return dnprot_reject(appconn);
    }
  }

#ifdef ENABLE_RADPROXY
  /* EAP Message */
  appconn->challen = 0;
  do {
    eapattr=NULL;
    if (!radius_getattr(pack, &eapattr, RADIUS_ATTR_EAP_MESSAGE, 0, 0,
			instance++)) {
      if ((appconn->challen + eapattr->l-2) > MAX_EAP_LEN) {
	syslog(LOG_INFO, "EAP message too long %zu %d",
               appconn->challen, (int) eapattr->l-2);
	return dnprot_reject(appconn);
      }
      memcpy(appconn->chal + appconn->challen,
	     eapattr->v.t, eapattr->l-2);

      appconn->challen += eapattr->l-2;
    }
  } while (eapattr);

  /* Get sendkey */
  if (!radius_getattr(pack, &sendattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_MPPE_SEND_KEY, 0)) {
    if (radius_keydecode(radius, appconn->sendkey, RADIUS_ATTR_VLEN, &appconn->sendlen,
			 (uint8_t *)&sendattr->v.t, sendattr->l-2,
			 pack_req->authenticator,
			 radius->secret, radius->secretlen)) {
      syslog(LOG_ERR, "radius_keydecode() failed!");
      return dnprot_reject(appconn);
    }
  }

  /* Get recvkey */
  if (!radius_getattr(pack, &recvattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_MPPE_RECV_KEY, 0)) {
    if (radius_keydecode(radius, appconn->recvkey, RADIUS_ATTR_VLEN, &appconn->recvlen,
			 (uint8_t *)&recvattr->v.t, recvattr->l-2,
			 pack_req->authenticator,
			 radius->secret, radius->secretlen) ) {
      syslog(LOG_ERR, "radius_keydecode() failed!");
      return dnprot_reject(appconn);
    }
  }

  /* Get LMNT keys */
  if (!radius_getattr(pack, &lmntattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_CHAP_MPPE_KEYS, 0)) {

    /* TODO: Check length of vendor attributes */
    if (radius_pwdecode(radius, appconn->lmntkeys, RADIUS_MPPEKEYSSIZE,
			&appconn->lmntlen, (uint8_t *)&lmntattr->v.t,
			lmntattr->l-2, pack_req->authenticator,
			radius->secret, radius->secretlen)) {
      syslog(LOG_ERR, "radius_pwdecode() failed");
      return dnprot_reject(appconn);
    }
  }

  /* Get encryption policy */
  if (!radius_getattr(pack, &policyattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_MPPE_ENCRYPTION_POLICY, 0)) {
    appconn->policy = ntohl(policyattr->v.i);
  }

  /* Get encryption types */
  if (!radius_getattr(pack, &typesattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_MPPE_ENCRYPTION_TYPES, 0)) {
    appconn->types = ntohl(typesattr->v.i);
  }

  /* Get MS_Chap_v2 SUCCESS */
  if (!radius_getattr(pack, &succattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_CHAP2_SUCCESS, 0)) {
    if ((succattr->l-5) != MS2SUCCSIZE) {
      syslog(LOG_ERR, "Wrong length of MS-CHAP2 success: %d", succattr->l-5);
      return dnprot_reject(appconn);
    }
    memcpy(appconn->ms2succ, ((void*)&succattr->v.t)+3, MS2SUCCSIZE);
  }
#endif

  switch(appconn->authtype) {

    case PAP_PASSWORD:
      break;

#ifdef ENABLE_RADPROXY
    case EAP_MESSAGE:
      if (!appconn->challen) {
        syslog(LOG_INFO, "No EAP message found");
        return dnprot_reject(appconn);
      }
      break;
#endif

    case CHAP_DIGEST_MD5:
      break;

    case CHAP_MICROSOFT:
      if (!lmntattr) {
        syslog(LOG_INFO, "No MPPE keys found");
        return dnprot_reject(appconn);
      }
      if (!succattr) {
        syslog(LOG_ERR, "No MS-CHAP2 success found");
        return dnprot_reject(appconn);
      }
      break;

    case CHAP_MICROSOFT_V2:
      if (!sendattr) {
        syslog(LOG_INFO, "No MPPE sendkey found");
        return dnprot_reject(appconn);
      }

      if (!recvattr) {
        syslog(LOG_INFO, "No MPPE recvkey found");
        return dnprot_reject(appconn);
      }

      break;

    default:
      syslog(LOG_ERR, "Unknown authtype");
      return dnprot_reject(appconn);
  }

  return upprot_getip(appconn, &hisip, &hismask);
}

#ifdef ENABLE_COA
/* Radius callback when coa or disconnect request has been received */
int cb_radius_coa_ind(struct radius_t *radius, struct radius_packet_t *pack,
		      struct sockaddr_in *peer) {
  struct app_conn_t *appconn;
  struct radius_attr_t *uattr = NULL;
  struct radius_attr_t *sattr = NULL;
  struct radius_packet_t radius_pack;
  int authorize = 0;
  int found = 0;
  int iscoa = 0;

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Received coa or disconnect request\n", __FUNCTION__, __LINE__);
#endif

  if (pack->code != RADIUS_CODE_DISCONNECT_REQUEST &&
      pack->code != RADIUS_CODE_COA_REQUEST) {
    syslog(LOG_ERR, "Radius packet not supported: %d,\n", pack->code);
    return -1;
  }

  iscoa = pack->code == RADIUS_CODE_COA_REQUEST;

  /* Get username */
  if (radius_getattr(pack, &uattr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
    syslog(LOG_WARNING, "Username must be included in disconnect request");
    return -1;
  }

  if (!radius_getattr(pack, &sattr, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0))
    if (_options.debug) {
      syslog(LOG_DEBUG, "%s(%d): Session-id present in disconnect. Only disconnecting that session\n", __FUNCTION__, __LINE__);

      syslog(LOG_DEBUG, "%s(%d): Looking for session [username=%.*s,sessionid=%.*s]", __FUNCTION__, __LINE__,
             uattr->l-2, uattr->v.t, sattr ? sattr->l-2 : 3,
             sattr ? (char*)sattr->v.t : "all");
    }

  for (appconn = firstusedconn; appconn; appconn = appconn->next) {

    if (!appconn->inuse) { syslog(LOG_ERR, "Connection with inuse == 0!"); }

    if (
            (strlen(appconn->s_state.redir.username) == uattr->l-2 &&
             !memcmp(appconn->s_state.redir.username, uattr->v.t, uattr->l-2)) &&
            (!sattr ||
             (strlen(appconn->s_state.sessionid) == sattr->l-2 &&
              !strncasecmp(appconn->s_state.sessionid, (char*)sattr->v.t, sattr->l-2)))) {

#if(_debug_)
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Found session %s", __FUNCTION__, __LINE__, appconn->s_state.sessionid);
#endif

      if (iscoa) {
	struct radius_attr_t *attr = NULL;

	/* Session state */
	if (!radius_getattr(pack, &attr,
			    RADIUS_ATTR_VENDOR_SPECIFIC,
			    RADIUS_VENDOR_COOVACHILLI,
			    RADIUS_ATTR_COOVACHILLI_SESSION_STATE, 0)) {
	  uint32_t v = ntohl(attr->v.i);
	  switch (v) {
            case RADIUS_VALUE_COOVACHILLI_SESSION_AUTH:
              if (!appconn->s_state.authenticated)
                authorize = 1;
              break;
            case RADIUS_VALUE_COOVACHILLI_SESSION_NOAUTH:
              if (appconn->s_state.authenticated)
                terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
              break;
	  }
	}
      } else {
	terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_ADMIN_RESET);
      }

      config_radius_session(&appconn->s_params, pack, appconn, 0);

      if (authorize)
	dnprot_accept(appconn);

      found = 1;
    }
  }

  if (found) {
    if (radius_default_pack(radius, &radius_pack,
			    iscoa ? RADIUS_CODE_COA_ACK : RADIUS_CODE_DISCONNECT_ACK)) {
      syslog(LOG_ERR, "radius_default_pack() failed");
      return -1;
    }
  }
  else {
    if (radius_default_pack(radius, &radius_pack,
			    iscoa ? RADIUS_CODE_COA_NAK : RADIUS_CODE_DISCONNECT_NAK)) {
      syslog(LOG_ERR, "radius_default_pack() failed");
      return -1;
    }
  }

  radius_pack.id = pack->id;
  (void) radius_coaresp(radius, &radius_pack, peer, pack->authenticator);

  return 0;
}
#endif

/***********************************************************
 *
 * dhcp callback functions
 *
 ***********************************************************/

/* DHCP callback for allocating new IP address */
/* In the case of WPA it is allready allocated,
 * for UAM address is allocated before authentication */
int cb_dhcp_request(struct dhcp_conn_t *conn, struct in_addr *addr,
		    uint8_t *dhcp_pkt, size_t dhcp_len) {
  struct app_conn_t *appconn = conn->peer;
  struct ippoolm_t *ipm = 0;
  char domacauth = (char) _options.macauth;
  char allocate = 1;

#if(_debug_)
  syslog(LOG_DEBUG, "%s(%d): DHCP request for IP address %s", __FUNCTION__, __LINE__,
         addr ? inet_ntoa(*addr) : "n/a");
#endif

  if (!appconn) {
    syslog(LOG_ERR, "Peer protocol not defined");
    return -1;
  }

#ifdef ENABLE_UAMANYIP
  /* if uamanyip is on we have to filter out which ip's are allowed */
  if (_options.uamanyip && addr && addr->s_addr) {

    if (addr->s_addr == _options.uamlisten.s_addr) {
      return -1;
    }

    if (_options.uamanyipex_addr.s_addr &&
	(addr->s_addr & _options.uamanyipex_mask.s_addr) ==
	_options.uamanyipex_addr.s_addr) {
      return -1;
    }

    if ((addr->s_addr & ipv4ll_mask.s_addr) == ipv4ll_ip.s_addr) {
      /* clients with an IPv4LL ip normally have no default gw assigned, rendering uamanyip useless
	 They must rather get a proper dynamic ip via dhcp */
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): IPv4LL/APIPA address requested, ignoring %s", __FUNCTION__, __LINE__,
               inet_ntoa(*addr));
      return -1;
    }
  }
#endif

  /* Save for MAC auth later */
  appconn->reqip.s_addr = addr ? addr->s_addr : 0;

  if (appconn->uplink) {

    /*
     *  IP Address is already known and allocated.
     */
    ipm = (struct ippoolm_t*) appconn->uplink;

  } else {

    if ( ! conn->is_reserved) {

      if ((_options.macoklen) &&
	  (appconn->dnprot == DNPROT_DHCP_NONE) &&
	  !maccmp(conn->hismac)) {

	/*
	 *  When using macallowed option, and hismac matches.
	 */
	appconn->dnprot = DNPROT_MAC;

	if (_options.macallowlocal) {
	  char mac[MACSTRLEN+1];

	  snprintf(mac, sizeof(mac), MAC_FMT, MAC_ARG(conn->hismac));

	  strlcpy(appconn->s_state.redir.username, mac, USERNAMESIZE);

	  if (_options.macsuffix) {
	    size_t ulen = strlen(appconn->s_state.redir.username);
	    strlcpy(appconn->s_state.redir.username + ulen,
                    _options.macsuffix, USERNAMESIZE - ulen);
	  }

	  /*
	   *  Local MAC allowed list, authenticate without RADIUS.
	   */
	  upprot_getip(appconn, &appconn->reqip, 0);

	  syslog(LOG_INFO, "Granted MAC=%s with IP=%s access without radius auth",
                 mac, inet_ntoa(appconn->hisip));

	  ipm = (struct ippoolm_t*) appconn->uplink;
	  domacauth = 0;

	} else {
	  /*
	   *  Otherwise, authenticate with RADIUS.
	   */
	  auth_radius(appconn, 0, 0, dhcp_pkt, dhcp_len);

	  allocate = !_options.strictmacauth;
	  domacauth = 0;
	}

      } else if ((_options.macauth) &&
		 (appconn->dnprot == DNPROT_DHCP_NONE)) {

	/*
	 *  Using macauth option to authenticate via RADIUS.
	 */
	appconn->dnprot = DNPROT_MAC;

	auth_radius(appconn, 0, 0, dhcp_pkt, dhcp_len);

	allocate = !_options.strictmacauth;
	domacauth = 0;
      }
    }
  }

  if (!ipm) {

    if (!allocate)
      return -1;

    if (appconn->dnprot != DNPROT_DHCP_NONE && appconn->hisip.s_addr) {
      syslog(LOG_WARNING, "Requested IP address when already allocated (hisip %s)",
             inet_ntoa(appconn->hisip));
      appconn->reqip.s_addr = appconn->hisip.s_addr;
    }

    /* Allocate dynamic IP address */
    /* XXX  if (ippool_newip(ippool, &ipm, &appconn->reqip, 0)) {*/
    if (newip(&ipm, &appconn->reqip, conn->hismac)) {
      syslog(LOG_ERR, "Failed allocate dynamic IP address");
      return -1;
    }

    appconn->hisip.s_addr = ipm->addr.s_addr;
    appconn->hismask.s_addr = _options.mask.s_addr;

    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Client MAC="MAC_FMT" assigned IP %s" , __FUNCTION__, __LINE__,
             MAC_ARG(conn->hismac), inet_ntoa(appconn->hisip));

#ifdef ENABLE_MODULES
    { int i;
      for (i=0; i < MAX_MODULES; i++) {
	if (!_options.modules[i].name[0]) break;
	if (_options.modules[i].ctx) {
	  struct chilli_module *m =
              (struct chilli_module *)_options.modules[i].ctx;
	  if (m->dhcp_connect)
	    m->dhcp_connect(appconn, conn);
	}
      }
    }
#endif

    /* TODO: Too many "listen" and "our" addresses hanging around */
    if (!appconn->ourip.s_addr)
      appconn->ourip.s_addr = _options.dhcplisten.s_addr;

    appconn->uplink = ipm;
    ipm->peer = appconn;

#ifdef ENABLE_GARDENACCOUNTING
    if (_options.uamgardendata) {
      acct_req(ACCT_GARDEN, appconn, RADIUS_STATUS_TYPE_START);
    }
#endif

#ifdef ENABLE_UAMANYIP
    if (chilli_assign_snat(appconn, 0) != 0) {
      return -1;
    }
#endif
  }

  if (ipm) {
    dhcp_set_addrs(conn,
		   &ipm->addr, &_options.mask,
		   &appconn->ourip, &appconn->mask,
		   &_options.dns1, &_options.dns2);
  }

  if (!appconn->s_state.authenticated) {

    if (domacauth) {
      auth_radius(appconn, 0, 0, dhcp_pkt, dhcp_len);
    }

    /* if not already authenticated, ensure DNAT authstate */
#ifdef ENABLE_LAYER3
    if (!_options.layer3)
#endif
      conn->authstate = DHCP_AUTH_DNAT;
  }

  /* If IP was requested before authentication it was UAM */
  if (appconn->dnprot == DNPROT_DHCP_NONE)
    appconn->dnprot = DNPROT_UAM;

  if (_options.dhcpnotidle)
    appconn->s_state.last_up_time = mainclock.tv_sec;

  return 0;
}


int chilli_connect(struct app_conn_t **appconn, struct dhcp_conn_t *conn) {
  struct app_conn_t *aconn;

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): New Chilli Connection", __FUNCTION__, __LINE__);
#endif

  /* Allocate new application connection */
  if (chilli_new_conn(appconn)) {
    syslog(LOG_ERR, "Failed to allocate connection");
    return -1;
  }

  aconn = *appconn;
  aconn->dnlink =  conn;
  aconn->dnprot =  DNPROT_DHCP_NONE;

  aconn->net.s_addr = _options.net.s_addr;
  aconn->mask.s_addr = _options.mask.s_addr;
  aconn->dns1.s_addr = _options.dns1.s_addr;
  aconn->dns2.s_addr = _options.dns2.s_addr;

  if (conn) {
    memcpy(aconn->hismac, conn->hismac, PKT_ETH_ALEN);
  }

  set_sessionid(aconn, 1);

  app_conn_set_idx(aconn, conn);

#ifdef ENABLE_BINSTATFILE
  if (_options.statusfilesave)
    printstatus();
#endif

  return 0;
}

/* DHCP callback for establishing new connection */
int cb_dhcp_connect(struct dhcp_conn_t *conn) {
  struct app_conn_t *appconn;

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): New DHCP request from MAC="MAC_FMT, __FUNCTION__, __LINE__,
           MAC_ARG(conn->hismac));

  if (chilli_connect(&appconn, conn))
    return 0;

  conn->peer = appconn;

  conn->authstate = DHCP_AUTH_NONE; /* TODO: Not yet authenticated */

  if (_options.macup) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Calling MAC up script: %s", __FUNCTION__, __LINE__, _options.macup);
    runscript(appconn, _options.macup, 0, 0);
  }

  return 0;
}

#ifdef ENABLE_LAYER3
struct app_conn_t * chilli_connect_layer3(struct in_addr *src, struct dhcp_conn_t *conn) {
  struct app_conn_t *appconn = 0;
  struct ippoolm_t *ipm = 0;

  if (ippool_getip(ippool, &ipm, src)) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): New Layer3 %s", __FUNCTION__, __LINE__, inet_ntoa(*src));
    if (ippool_newip(ippool, &ipm, src, 1)) {
      if (ippool_newip(ippool, &ipm, src, 0)) {
	syslog(LOG_ERR, "Failed to allocate either static or dynamic IP address");
	return 0;
      }
    }
  }

  if (!ipm) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): unknown ip", __FUNCTION__, __LINE__);
    return 0;
  }

  if ((appconn = (struct app_conn_t *)ipm->peer) == NULL) {
    if (chilli_getconn(&appconn, src->s_addr, 0, 0)) {
      if (chilli_connect(&appconn, conn)) {
	syslog(LOG_ERR, "chilli_connect()");
	return 0;
      }
    }
  }

  appconn->s_state.last_up_time = mainclock_now();
  appconn->hisip.s_addr = src->s_addr;
  appconn->hismask.s_addr = _options.mask.s_addr;
  appconn->dnprot = DNPROT_LAYER3;
  appconn->uplink = ipm;
  ipm->peer = appconn;
  return appconn;
}
#endif

#ifdef ENABLE_CHILLIQUERY
static char *state2name(int authstate) {
  switch(authstate) {
    case DHCP_AUTH_NONE:   return "none";
    case DHCP_AUTH_DROP:   return "drop";
    case DHCP_AUTH_PASS:   return "pass";
    case DHCP_AUTH_DNAT:   return "dnat";
    case DHCP_AUTH_SPLASH: return "splash";
#ifdef ENABLE_LAYER3
    case DHCP_AUTH_ROUTER: return "layer2";
#endif
    default:               return "unknown";
  }
}

int chilli_getinfo(struct app_conn_t *appconn, bstring b, int fmt) {
  uint32_t sessiontime = 0;
  uint32_t idletime = 0;

  if (appconn->s_state.authenticated) {
    sessiontime = mainclock_diffu(appconn->s_state.start_time);
    idletime    = mainclock_diffu(appconn->s_state.last_up_time);
  }

  switch(fmt) {
#ifdef ENABLE_JSON
    case LIST_JSON_FMT:
      if (appconn->s_state.authenticated)
        session_json_fmt(&appconn->s_state, &appconn->s_params, b, 0);
      break;
#endif
    default:
      {
        bstring tmp = bfromcstr("");

        /* adding: session-id auth-state user-name */
        bassignformat(tmp, " %.*s %d %.*s",
                      appconn->s_state.sessionid[0] ? strlen(appconn->s_state.sessionid) : 1,
                      appconn->s_state.sessionid[0] ? appconn->s_state.sessionid : "-",
                      appconn->s_state.authenticated,
                      appconn->s_state.redir.username[0] ? strlen(appconn->s_state.redir.username) : 1,
                      appconn->s_state.redir.username[0] ? appconn->s_state.redir.username : "-");
        bconcat(b, tmp);

        /* adding: session-time/session-timeout idle-time/idle-timeout */
        bassignformat(tmp, " %d/%d %d/%d",
                      sessiontime, (int)appconn->s_params.sessiontimeout,
                      idletime, (int)appconn->s_params.idletimeout);
        bconcat(b, tmp);

        /* adding: input-octets/max-input-octets */
#ifdef ENABLE_GARDENACCOUNTING
        if (_options.uamgardendata && _options.uamotherdata)
          bassignformat(tmp, " %lld/%lld/%lld/%lld",
                        appconn->s_state.input_octets,
                        appconn->s_params.maxinputoctets,
                        appconn->s_state.garden_input_octets,
                        appconn->s_state.other_input_octets);
        else if (_options.uamgardendata)
          bassignformat(tmp, " %lld/%lld/%lld",
                        appconn->s_state.input_octets,
                        appconn->s_params.maxinputoctets,
                        appconn->s_state.garden_input_octets);
        else
#endif
          bassignformat(tmp, " %lld/%lld",
                        appconn->s_state.input_octets,
                        appconn->s_params.maxinputoctets);
        bconcat(b, tmp);

        /* adding: output-octets/max-output-octets */
#ifdef ENABLE_GARDENACCOUNTING
        if (_options.uamgardendata && _options.uamotherdata)
          bassignformat(tmp, " %lld/%lld/%lld/%lld",
                        appconn->s_state.output_octets,
                        appconn->s_params.maxoutputoctets,
                        appconn->s_state.garden_output_octets,
                        appconn->s_state.other_output_octets);
        else if (_options.uamgardendata)
          bassignformat(tmp, " %lld/%lld/%lld",
                        appconn->s_state.output_octets,
                        appconn->s_params.maxoutputoctets,
                        appconn->s_state.garden_output_octets);
        else
#endif
          bassignformat(tmp, " %lld/%lld",
                        appconn->s_state.output_octets,
                        appconn->s_params.maxoutputoctets);
        bconcat(b, tmp);

        /* adding: max-total-octets option-swapoctets */
        bassignformat(tmp, " %lld %d",
                      appconn->s_params.maxtotaloctets, _options.swapoctets);
        bconcat(b, tmp);

#ifdef ENABLE_LEAKYBUCKET
        /* adding: max-bandwidth-up max-bandwidth-down */
        if (appconn->s_state.bucketupsize) {
          bassignformat(tmp, " %d%%/%lld",
                        (int) (appconn->s_state.bucketup * 100 /
                               appconn->s_state.bucketupsize),
                        appconn->s_params.bandwidthmaxup);
          bconcat(b, tmp);
        } else
#endif
          bcatcstr(b, " 0/0");

#ifdef ENABLE_LEAKYBUCKET
        if (appconn->s_state.bucketdownsize) {
          bassignformat(tmp, " %d%%/%lld ",
                        (int) (appconn->s_state.bucketdown * 100 /
                               appconn->s_state.bucketdownsize),
                        appconn->s_params.bandwidthmaxdown);
          bconcat(b, tmp);
        } else
#endif
          bcatcstr(b, " 0/0 ");

        /* adding: original url */
        if (appconn->s_state.redir.userurl[0])
          bcatcstr(b, appconn->s_state.redir.userurl);
        else
          bcatcstr(b, "-");

#ifdef ENABLE_IEEE8021Q
        /* adding: vlan, if one */
        if (_options.ieee8021q && appconn->s_state.tag8021q) {
          bassignformat(tmp, " vlan=%d",
                        (int)ntohs(appconn->s_state.tag8021q &
                                   PKT_8021Q_MASK_VID));
          bconcat(b, tmp);
        } else {
#endif
#ifdef ENABLE_MULTILAN
          if (app_conn_idx(appconn)) {
            bassignformat(tmp, " vlan=%s",
                          _options.moreif[app_conn_idx(appconn)-1].vlan ?
                          _options.moreif[app_conn_idx(appconn)-1].vlan :
                          _options.moreif[app_conn_idx(appconn)-1].dhcpif);
          } else {
            bassignformat(tmp, " vlan=%s", _options.vlan);
          }
          bconcat(b, tmp);
#endif
#ifdef ENABLE_IEEE8021Q
        }
#endif

#ifdef ENABLE_LOCATION
        if (appconn->s_state.location[0]) {
          bstring tmp2 = bfromcstr("");
          bcatcstr(b, " loc=");
          bassigncstr(tmp, appconn->s_state.location);
          redir_urlencode(tmp, tmp2);
          bconcat(b, tmp2);
          bdestroy(tmp2);
        }
#endif

        bdestroy(tmp);
      }
  }
  return 0;
}

void chilli_print(bstring s, int listfmt,
		  struct app_conn_t *appconn,
		  struct dhcp_conn_t *conn) {

  if (!appconn && conn)
    appconn = (struct app_conn_t *)conn->peer;

  if (
#ifdef ENABLE_LAYER3
          !_options.layer3 &&
#endif
          (!appconn || !appconn->inuse)) {
#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Can not print info about unused chilli connection", __FUNCTION__, __LINE__);
#endif
    return;
  } else if (conn && !conn->inuse) {
#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Can not print info about unused dhcp connection", __FUNCTION__, __LINE__);
#endif
    return;
  } else {
    bstring b = bfromcstr("");
    bstring tmp = bfromcstr("");

    switch(listfmt) {
#ifdef ENABLE_JSON
      case LIST_JSON_FMT:
        if ((conn && conn != dhcp->firstusedconn) ||
            (appconn && appconn != firstusedconn))
          bcatcstr(b, ",");

        bcatcstr(b, "{");

        if (appconn) {
          bcatcstr(b, "\"nasPort\":");
          bassignformat(tmp, "%d", appconn->unit);
          bconcat(b, tmp);
          bcatcstr(b, ",\"clientState\":");
          bassignformat(tmp, "%d", appconn->s_state.authenticated);
          bconcat(b, tmp);
          bcatcstr(b, ",\"ipAddress\":\"");
          bcatcstr(b, inet_ntoa(appconn->hisip));
          bcatcstr(b, "\"");
        }

        if (conn) {
          if (appconn) bcatcstr(b, ",");
          bcatcstr(b, "\"macAddress\":\"");
          bassignformat(tmp, MAC_FMT, MAC_ARG(conn->hismac));
          bconcat(b, tmp);
          bcatcstr(b, "\",\"dhcpState\":\"");
          bcatcstr(b, state2name(conn->authstate));
          bcatcstr(b, "\"");
        }

        if (appconn)
          chilli_getinfo(appconn, b, listfmt);

        bcatcstr(b, "}");
        break;
#endif

      default:
        if (conn && !appconn)
          bassignformat(b, MAC_FMT" %s", MAC_ARG(conn->hismac),
                        state2name(conn->authstate));
        else if (conn)
          bassignformat(b, MAC_FMT" %s %s", MAC_ARG(conn->hismac),
                        inet_ntoa(conn->hisip), state2name(conn->authstate));
        else
          bassignformat(b, "%s", inet_ntoa(appconn->hisip));

        switch(listfmt) {
          case LIST_LONG_FMT:
            if (appconn)
              chilli_getinfo(appconn, b, listfmt);
            break;
          case LIST_SHORT_FMT:
            if (conn) {
              bassignformat(tmp, " %d/%d",
                            mainclock_diff(conn->lasttime),
                            dhcp->lease);
              bconcat(b, tmp);
            }
            break;
        }

        bcatcstr(b, "\n");
        break;
    }

    bconcat(s, b);

    bdestroy(b);
    bdestroy(tmp);
  }
}
#endif

static void
clear_appconn(struct app_conn_t *appconn) {
  appconn->s_params.bandwidthmaxup =
      appconn->s_params.bandwidthmaxdown =
      appconn->s_params.maxinputoctets =
      appconn->s_params.maxoutputoctets =
      appconn->s_params.maxtotaloctets =
      appconn->s_params.sessiontimeout = 0;
  appconn->s_params.idletimeout = 0;
  appconn->s_params.interim_interval = 0;
  appconn->s_params.sessionterminatetime = 0;
}

int terminate_appconn(struct app_conn_t *appconn, int terminate_cause) {

  if (appconn->s_state.authenticated == 1) {

    dnprot_terminate(appconn);

    appconn->s_state.terminate_cause = terminate_cause;

#ifdef ENABLE_MODULES
    { int i;
      for (i=0; i < MAX_MODULES; i++) {
	if (!_options.modules[i].name[0]) break;
	if (_options.modules[i].ctx) {
	  struct chilli_module *m =
              (struct chilli_module *)_options.modules[i].ctx;
	  if (m->session_stop)
	    m->session_stop(appconn);
	}
      }
    }
#endif

    if (_options.condown && !(appconn->s_params.flags & NO_SCRIPT)) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Calling connection down script: %s\n", __FUNCTION__, __LINE__, _options.condown);
      runscript(appconn, _options.condown, 0, 0);
    }

    acct_req(ACCT_USER, appconn, RADIUS_STATUS_TYPE_STOP);

    clear_appconn(appconn);
    set_sessionid(appconn, 0);

#ifdef ENABLE_STATFILE
    if (_options.statusfilesave)
      printstatus();
#endif
  }

  return 0;
}

/* Callback when a dhcp connection is deleted */
int cb_dhcp_disconnect(struct dhcp_conn_t *conn, int term_cause) {
  struct app_conn_t *appconn;

  syslog(LOG_INFO, "DHCP Released MAC="MAC_FMT" IP=%s",
         MAC_ARG(conn->hismac), inet_ntoa(conn->hisip));

  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): DHCP connection removed", __FUNCTION__, __LINE__);

  if (!conn->peer) {
    /* No appconn allocated. Stop here */
#ifdef ENABLE_BINSTATFILE
    if (_options.statusfilesave)
      printstatus();
#endif
    return 0;
  }

  appconn = (struct app_conn_t*) conn->peer;

  return session_disconnect(appconn, conn, term_cause);
}

/* Callback for receiving messages from dhcp */
int cb_dhcp_data_ind(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {
  struct app_conn_t *appconn = dhcp_get_appconn_pkt(conn, pkt_iphdr(pack), 0);
  struct pkt_ipphdr_t *ipph = pkt_ipphdr(pack);

  /*if (_options.debug)
    syslog(LOG_DEBUG, "cb_dhcp_data_ind. Packet received. DHCP authstate: %d\n",
    conn->authstate);*/

#ifdef ENABLE_LEAKYBUCKET
  if (_options.scalewin && appconn && appconn->s_state.bucketup) {
    uint16_t win = appconn->s_state.bucketupsize -
        appconn->s_state.bucketup;
    //log_dbg("window scaling to %d", win);
    pkt_shape_tcpwin((struct pkt_iphdr_t *)ipph, win);
  }
#endif

  if (!appconn) {
#ifdef ENABLE_LAYER3
    if (_options.layer3) {
      struct ippoolm_t *ipm = 0;
      struct in_addr addr;

      addr.s_addr = ipph->saddr;

      if (!addr.s_addr) {
	return tun_encaps(tun, pack, len, 0);
      }

      if (ippool_getip(ippool, &ipm, &addr)) {
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): unknown IP address: %s", __FUNCTION__, __LINE__, inet_ntoa(addr));
	return -1;
      }

      appconn = ipm->peer;
    }
    if (!appconn)
#endif
    {
      syslog(LOG_ERR, "No peer protocol defined");
      return -1;
    }
  }

  switch (appconn->dnprot) {
    case DNPROT_NULL:
    case DNPROT_DHCP_NONE:
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): NULL: %d", __FUNCTION__, __LINE__, appconn->dnprot);
      return -1;

    case DNPROT_UAM:
    case DNPROT_WPA:
    case DNPROT_MAC:
#ifdef ENABLE_EAPOL
    case DNPROT_EAPOL:
#endif
#ifdef ENABLE_LAYER3
    case DNPROT_LAYER3:
#endif
      break;

    default:
      syslog(LOG_ERR, "Unknown downlink protocol: %d", appconn->dnprot);
      break;
  }

#ifdef ENABLE_UAMANYIP
  /**
   * packet is coming from an anyip client, therefore SNAT address
   * has been assigned from dynamic pool. So, let's do the SNAT here.
   */
  if (_options.uamanyip && appconn->natip.s_addr) {
#if(_debug_ > 1)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): SNAT to: %s", __FUNCTION__, __LINE__, inet_ntoa(appconn->natip));
#endif
    ipph->saddr = appconn->natip.s_addr;
    if (chksum((struct pkt_iphdr_t *) ipph) < 0)
      return 0;
  }
#endif

  /*
   * If the ip dst is uamlisten and pdst is uamport we won't call leaky_bucket,
   * and we always send these packets through to the tun/tap interface (index 0)
   */
  if (ipph->daddr  == _options.uamlisten.s_addr &&
      (ipph->dport == htons(_options.uamport)
#ifdef ENABLE_UAMUIPORT
       || ipph->dport == htons(_options.uamuiport)
#endif
       )) {
    return tun_encaps(tun, pack, len, 0);
  }

  if (chilli_acct_fromsub(appconn, ipph))
    return 0;

  return tun_encaps(tun, pack, len, appconn->s_params.routeidx);
}

int chilli_acct_fromsub(struct app_conn_t *appconn,
			struct pkt_ipphdr_t *ipph) {
  int len = ntohs(ipph->tot_len);
#ifdef ENABLE_GARDENACCOUNTING
  char checked_garden = 0;
#endif
  char is_garden = 0;
  char is_auth;
  char do_acct;
#ifdef ENABLE_LEAKYBUCKET
  char do_bw;
#endif

  is_auth = appconn->s_state.authenticated == 1;

  if (is_auth) {

    do_acct = is_auth;
#ifdef ENABLE_LEAKYBUCKET
    do_bw = is_auth;
#endif

#ifdef ENABLE_AUTHEDALLOWED
    if (dhcp_garden_check_auth(dhcp, 0, appconn, ipph, 1)) {
      /*
       *  Change to garden accounting.
       */
      if (_options.nousergardendata)
	is_garden = 1;
#ifdef ENABLE_LEAKYBUCKET
      do_bw = 0;
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): !!!! Skipping leaky bucket because of authedallowed", __FUNCTION__, __LINE__);
#endif
    }
#endif

    if (_options.uamauthedallowed
#ifdef ENABLE_LEAKYBUCKET
        && do_bw
#endif
        ) {
#ifdef ENABLE_GARDENACCOUNTING
      checked_garden = 1;
#endif
      if (dhcp_garden_check(dhcp, 0, appconn, ipph, 1)) {
	/*
	 *  Garden accounting taken care of in dhcp_garden_check()
	 */
	do_acct = 0;
#ifdef ENABLE_LEAKYBUCKET
	do_bw = 0;
        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): !!!! Skipping leaky bucket because of uamauthedallowed", __FUNCTION__, __LINE__);
#endif
      }
    }

#ifdef ENABLE_GARDENACCOUNTING
    if (_options.nousergardendata) {
      if (!checked_garden) {
	checked_garden = 1;
	if (dhcp_garden_check(dhcp, 0, appconn, ipph, 1)) {
	  do_acct = 0;
	}
      }
    }
#endif

#ifdef ENABLE_LEAKYBUCKET
#ifndef COUNT_UPLINK_DROP
    if (do_bw) {
      if (leaky_bucket(appconn, len, 0)) return 1;
    }
#endif
#endif

    if (do_acct) {

      if (is_garden) {
#ifdef ENABLE_GARDENACCOUNTING
	if (_options.swapoctets) {
	  appconn->s_state.garden_input_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.garden_input_octets += len;
	  }
	} else {
	  appconn->s_state.garden_output_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.garden_output_octets += len;
	  }
	}
#endif
      } else {
	if (_options.swapoctets) {
	  appconn->s_state.input_packets++;
	  appconn->s_state.input_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.input_packets++;
	    admin_session.s_state.input_octets += len;
	  }
	} else {
	  appconn->s_state.output_packets++;
	  appconn->s_state.output_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.output_packets++;
	    admin_session.s_state.output_octets += len;
	  }
	}
      }
    }

#ifdef ENABLE_LEAKYBUCKET
#ifdef COUNT_UPLINK_DROP
    if (do_bw) {
      if (leaky_bucket(appconn, len, 0)) return 1;
    }
#endif
#endif
  }

  appconn->s_state.last_time = mainclock.tv_sec;
  appconn->s_state.last_up_time = mainclock.tv_sec;

  return 0;
}

int chilli_acct_tosub(struct app_conn_t *appconn,
		      struct pkt_ipphdr_t *ipph) {
  int len = ntohs(ipph->tot_len);
#ifdef ENABLE_GARDENACCOUNTING
  char checked_garden = 0;
#endif
  char is_garden = 0;
  char is_auth;
  char do_acct;
#ifdef ENABLE_LEAKYBUCKET
  char do_bw;
#endif

  is_auth = appconn->s_state.authenticated == 1;

  if (is_auth) {

    do_acct = is_auth;
#ifdef ENABLE_LEAKYBUCKET
    do_bw = is_auth;
#endif

#ifdef ENABLE_AUTHEDALLOWED
    if (dhcp_garden_check_auth(dhcp, 0, appconn, ipph, 0)) {
      /*
       *  Change to garden accounting.
       */
      if (_options.nousergardendata)
	is_garden = 1;
#ifdef ENABLE_LEAKYBUCKET
      do_bw = 0;
#endif
    }
#endif

    if (_options.uamauthedallowed
#ifdef ENABLE_LEAKYBUCKET
        && do_bw
#endif
        ) {
#ifdef ENABLE_GARDENACCOUNTING
      checked_garden = 1;
#endif
      if (dhcp_garden_check(dhcp, 0, appconn, ipph, 0)) {
	/*
	 *  Garden accounting taken care of in dhcp_garden_check()
	 */
	do_acct = 0;
#ifdef ENABLE_LEAKYBUCKET
	do_bw = 0;
#endif
      }
    }

#ifdef ENABLE_GARDENACCOUNTING
    if (_options.nousergardendata) {
      if (!checked_garden) {
	checked_garden = 1;
	if (dhcp_garden_check(dhcp, 0, appconn, ipph, 0)) {
	  do_acct = 0;
	}
      }
    }
#endif

#ifdef ENABLE_LEAKYBUCKET
#ifndef COUNT_DOWNLINK_DROP
    if (do_bw) {
      if (leaky_bucket(appconn, 0, len)) return 1;
    }
#endif
#endif

    if (do_acct) {

      if (is_garden) {
#ifdef ENABLE_GARDENACCOUNTING
	if (_options.swapoctets) {
	  appconn->s_state.garden_output_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.garden_output_octets += len;
	  }
	} else {
	  appconn->s_state.garden_input_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.garden_input_octets += len;
	  }
	}
#endif
      } else {
	if (_options.swapoctets) {
	  appconn->s_state.output_packets++;
	  appconn->s_state.output_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.output_packets++;
	    admin_session.s_state.output_octets += len;
	  }
	} else {
	  appconn->s_state.input_packets++;
	  appconn->s_state.input_octets += len;
	  if (admin_session.s_state.authenticated) {
	    admin_session.s_state.input_packets++;
	    admin_session.s_state.input_octets += len;
	  }
	}
      }
    }

#ifdef ENABLE_LEAKYBUCKET
#ifdef COUNT_DOWNLINK_DROP
    if (do_bw) {
      if (leaky_bucket(appconn, 0, len)) return 1;
    }
#endif
#endif
  }

  appconn->s_state.last_time = mainclock.tv_sec;

  return 0;
}

#ifdef ENABLE_EAPOL
/* Callback for receiving messages from eapol */
int cb_dhcp_eap_ind(struct dhcp_conn_t *conn, uint8_t *pack, size_t len) {
  struct eap_packet_t *eap = (struct eap_packet_t *)pack;
  struct app_conn_t *appconn = conn->peer;
  struct radius_packet_t radius_pack;
  size_t offset;

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): EAP Packet received", __FUNCTION__, __LINE__);
#endif

  /* If this is the first EAPOL authentication request */
  if ((appconn->dnprot == DNPROT_DHCP_NONE) ||
      (appconn->dnprot == DNPROT_EAPOL)) {
    if ((eap->code == 2) && /* Response */
	(eap->type == 1) && /* Identity */
	(len > 5) &&        /* Must be at least 5 octets */
	((len - 5) < REDIR_USERNAMESIZE-1 )) {
      memcpy(appconn->s_state.redir.username, eap->payload, len - 5);
      appconn->s_state.redir.username[len - 5] = 0;
      appconn->dnprot = DNPROT_EAPOL;
      appconn->authtype = EAP_MESSAGE;
    }
    else if (appconn->dnprot == DNPROT_DHCP_NONE) {
      syslog(LOG_ERR, "Initial EAP response was not a valid identity response!");
      return 0;
    }
  }

  /* Return if not EAPOL */
  if (appconn->dnprot != DNPROT_EAPOL) {
    syslog(LOG_WARNING, "Received EAP message, processing for authentication");
    appconn->dnprot = DNPROT_EAPOL;
    return 0;
  }

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST)) {
    syslog(LOG_ERR, "radius_default_pack() failed");
    return -1;
  }

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
                 (uint8_t*) appconn->s_state.redir.username,
                 strlen(appconn->s_state.redir.username));

  /* Include EAP (if present) */
  offset = 0;
  while (offset < len) {
    size_t eaplen;

    if ((len - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = len - offset;

    radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		   pack + offset, eaplen);

    offset += eaplen;
  }

  chilli_req_attrs(radius, &radius_pack,
		   ACCT_USER,
		   _options.framedservice ? RADIUS_SERVICE_TYPE_FRAMED :
		   RADIUS_SERVICE_TYPE_LOGIN, 0,
		   appconn->unit, appconn->hismac,
		   &appconn->hisip, &appconn->s_state);

  radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR,
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  return radius_req(radius, &radius_pack, appconn);
}
#endif

/***********************************************************
 *
 *   uam message handling functions
 *
 ***********************************************************/

int static uam_msg(struct redir_msg_t *msg) {

  struct ippoolm_t *ipm;
  struct app_conn_t *appconn = NULL;
  struct dhcp_conn_t* dhcpconn;

#if defined(HAVE_NETFILTER_QUEUE) || defined(HAVE_NETFILTER_COOVA)
  if (_options.uamlisten.s_addr != _options.dhcplisten.s_addr) {
    msg->mdata.address.sin_addr.s_addr  = msg->mdata.address.sin_addr.s_addr & ~(_options.mask.s_addr);
    msg->mdata.address.sin_addr.s_addr |= _options.dhcplisten.s_addr & _options.mask.s_addr;
  }
#endif

  if (ippool_getip(ippool, &ipm, &msg->mdata.address.sin_addr)) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): UAM login with unknown IP address: %s", __FUNCTION__, __LINE__, inet_ntoa(msg->mdata.address.sin_addr));
    return 0;
  }

  if ((appconn  = (struct app_conn_t *)ipm->peer)        == NULL ||
      (dhcpconn = (struct dhcp_conn_t *)appconn->dnlink) == NULL) {
    syslog(LOG_ERR, "No peer protocol defined");
    return 0;
  }

  if (appconn->s_state.authenticated == 0 || msg->mtype == REDIR_LOGOUT) {
    /* Ensure that the session is not already authenticated before changing session */
    if (msg->mdata.opt & REDIR_MSG_OPT_REDIR)
      memcpy(&appconn->s_state.redir, &msg->mdata.redir, sizeof(msg->mdata.redir));

    if (msg->mdata.opt & REDIR_MSG_OPT_PARAMS)
      memcpy(&appconn->s_params, &msg->mdata.params, sizeof(msg->mdata.params));

    if (msg->mdata.opt & REDIR_MSG_NSESSIONID)
      set_sessionid(appconn, 0);
  }

  switch(msg->mtype) {

    case REDIR_LOGIN:
      if (appconn->uamabort) {
        syslog(LOG_INFO, "UAM login from username=%s IP=%s was aborted!",
	       msg->mdata.redir.username, inet_ntoa(appconn->hisip));
        appconn->uamabort = 0;
        return 0;
      }

      syslog(LOG_INFO, "Successful UAM login from username=%s IP=%s",
	     msg->mdata.redir.username, inet_ntoa(appconn->hisip));

      /* Initialise */
      appconn->s_params.routeidx = tun->routeidx;
      appconn->s_state.redir.statelen = 0;

#ifdef ENABLE_RADPROXY
      appconn->challen  = 0;
      appconn->sendlen  = 0;
      appconn->recvlen  = 0;
      appconn->lmntlen  = 0;
#endif

      memcpy(appconn->hismac, dhcpconn->hismac, PKT_ETH_ALEN);

#ifdef ENABLE_LEAKYBUCKET
      leaky_bucket_init(appconn);
#endif

      return upprot_getip(appconn, 0, 0);

    case REDIR_LOGOUT:

      syslog(LOG_INFO, "Received UAM logoff from username=%s IP=%s",
	     appconn->s_state.redir.username, inet_ntoa(appconn->hisip));

      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Received logoff from UAM", __FUNCTION__, __LINE__);

      if (appconn->s_state.authenticated == 1) {
#ifdef ENABLE_SESSIONSTATE
        appconn->s_state.session_state =
            RADIUS_VALUE_COOVACHILLI_SESSION_USER_LOGOUT_URL;
#endif
        terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_USER_REQUEST);
        appconn->s_params.sessiontimeout = 0;
        appconn->s_params.idletimeout = 0;
      }

      appconn->uamabort = 0;
      appconn->s_state.uamtime = mainclock.tv_sec;

#ifdef ENABLE_LAYER3
      if (!_options.layer3)
#endif
        dhcpconn->authstate = DHCP_AUTH_DNAT;

      break;

    case REDIR_ABORT:

      syslog(LOG_INFO, "Received UAM abort from IP=%s", inet_ntoa(appconn->hisip));

      appconn->uamabort = 1; /* Next login will be aborted */
      appconn->s_state.uamtime = 0;  /* Force generation of new challenge */

#ifdef ENABLE_LAYER3
      if (!_options.layer3)
#endif
        dhcpconn->authstate = DHCP_AUTH_DNAT;

      terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_USER_REQUEST);

      break;

    case REDIR_CHALLENGE:
      appconn->s_state.uamtime = mainclock.tv_sec;
      appconn->uamabort = 0;
      break;

    case REDIR_MACREAUTH:
      auth_radius(appconn, 0, 0, 0, 0);
      break;

    case REDIR_ALREADY:
    case REDIR_NOTYET:
      break;
  }

  return 0;
}

#if defined(ENABLE_CHILLIQUERY) || defined(ENABLE_CLUSTER)
static struct app_conn_t * find_app_conn(struct cmdsock_request *req,
                                         int *has_criteria) {
  struct app_conn_t *appconn = 0;
  struct dhcp_conn_t *dhcpconn = 0;

  if (req->ip.s_addr) {
    appconn = dhcp_get_appconn_ip(0, &req->ip);
    if (has_criteria)
      *has_criteria = 1;
  } else {
#ifdef ENABLE_LAYER3
    if (!_options.layer3)
#endif
      if (req->mac[0]||req->mac[1]||req->mac[2]||
	  req->mac[3]||req->mac[4]||req->mac[5]) {
	dhcp_hashget(dhcp, &dhcpconn, req->mac);
	if (has_criteria)
	  *has_criteria = 1;
      }
  }

  if (!appconn && dhcpconn
#ifdef ENABLE_LAYER3
      && !_options.layer3
#endif
      )
    appconn = (struct app_conn_t *) dhcpconn->peer;

  if (!appconn && req->d.sess.sessionid[0] != 0) {
    struct app_conn_t *aconn = firstusedconn;
    if (has_criteria)
      *has_criteria = 1;
    while (aconn) {
      if (!strcmp(aconn->s_state.sessionid, req->d.sess.sessionid)) {
	appconn = aconn;
	break;
      }
      aconn = aconn->next;
    }
  }

  if (appconn && !appconn->inuse) {
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): appconn not in use!", __FUNCTION__, __LINE__);
    return 0;
  }

  return appconn;
}

int chilli_cmd(struct cmdsock_request *req, bstring s, int sock) {

#ifdef HAVE_NETFILTER_COOVA
  if (_options.kname) {
    kmod_coova_sync();
  }
#endif

  switch(req->type) {

#ifdef ENABLE_INSPECT
    case CMDSOCK_INSPECT:
      {
        struct app_conn_t *appconn = 0;
        struct dhcp_conn_t *dhcpconn = 0;
        uint8_t z[PKT_ETH_ALEN];
        memset(z, 0, PKT_ETH_ALEN);

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): looking to inspect ip=%s/mac="MAC_FMT, __FUNCTION__, __LINE__,
                 inet_ntoa(req->ip), MAC_ARG(req->mac));

        if (req->ip.s_addr)
          appconn = dhcp_get_appconn_ip(0, &req->ip);
        else
#ifdef ENABLE_LAYER3
          if (!_options.layer3)
#endif
            dhcp_hashget(dhcp, &dhcpconn, req->mac);

        if (!appconn && !dhcpconn) {

          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): not found", __FUNCTION__, __LINE__);

        } else {

          bstring tmp = bfromcstr("");

          uint32_t sessiontime = 0;
          uint32_t idletime = 0;

          char *down="down";
          char *up="up";

          char *in_label=down;
          char *out_label=up;

          if (!appconn)
            appconn = (struct app_conn_t *) dhcpconn->peer;

          if (!dhcpconn
#ifdef ENABLE_LAYER3
              && !_options.layer3
#endif
              )
            dhcpconn = (struct dhcp_conn_t *) appconn->dnlink;

          if (_options.swapoctets) {
            in_label=up;
            out_label=down;
          }

          if (appconn->s_state.authenticated) {
            sessiontime = mainclock_diffu(appconn->s_state.start_time);
            idletime    = mainclock_diffu(appconn->s_state.last_up_time);
          }

          bassignformat(tmp,
                        "MAC:   "MAC_FMT"   IP:  %s\n"
                        "---------------------------------------------------\n",
                        MAC_ARG(appconn->hismac),
                        inet_ntoa(appconn->hisip));
          bconcat(s, tmp);

#ifdef ENABLE_IEEE8021Q
          /* adding: vlan, if one */
          if (_options.ieee8021q && appconn->s_state.tag8021q) {
            bassignformat(tmp, "%20s: %d\n",
                          "vlan",
                          (int)ntohs(appconn->s_state.tag8021q &
                                     PKT_8021Q_MASK_VID));
            bconcat(s, tmp);
          } else {
#endif
#ifdef ENABLE_MULTILAN
            if (app_conn_idx(appconn)) {
              bassignformat(tmp,
                            "%20s: %s\n"
                            "%20s: %s\n",
                            "dhcpif",
                            _options.moreif[app_conn_idx(appconn)-1].dhcpif,
                            "vlan",
                            _options.moreif[app_conn_idx(appconn)-1].vlan ?
                            _options.moreif[app_conn_idx(appconn)-1].vlan :
                            _options.moreif[app_conn_idx(appconn)-1].dhcpif);
            } else {
              bassignformat(tmp,
                            "%20s: %s\n"
                            "%20s: %s\n",
                            "dhcpif",
                            _options.dhcpif,
                            "vlan",
                            _options.vlan);
            }
            bconcat(s, tmp);
#endif
#ifdef ENABLE_IEEE8021Q
          }
#endif

          bassignformat(tmp,
                        "%20s:%s authenticated\n",
                        "status",
                        appconn->s_state.authenticated ?
                        "" : " not");
          bconcat(s, tmp);

          {
            char buffer[128];
            redir_chartohex(appconn->s_state.redir.uamchal,
                            buffer, REDIR_MD5LEN);
            bassignformat(tmp,
                          "%20s: %s\n",
                          "challenge",
                          buffer);
            bconcat(s, tmp);
          }

          bassignformat(tmp,
                        "%20s: %.*s\n",
                        "session id",
                        appconn->s_state.sessionid[0] ?
                        strlen(appconn->s_state.sessionid) : 1,
                        appconn->s_state.sessionid[0] ?
                        appconn->s_state.sessionid : "-");
          bconcat(s, tmp);

#ifdef ENABLE_SESSIONID
          bassignformat(tmp,
                        "%20s: %s\n",
                        "chilli session id",
                        appconn->s_state.chilli_sessionid);
          bconcat(s, tmp);
#endif
#ifdef ENABLE_GARDENACCOUNTING
          if (appconn->s_state.garden_sessionid[0]) {
            bassignformat(tmp,
                          "%20s: %s\n",
                          "garden session id",
                          appconn->s_state.garden_sessionid);
            bconcat(s, tmp);
          }
#endif
#ifdef ENABLE_APSESSIONID
          if (appconn->s_state.ap_sessionid[0]) {
            bassignformat(tmp,
                          "%20s: %s\n",
                          "ap session id",
                          appconn->s_state.ap_sessionid);
            bconcat(s, tmp);
          }
#endif

#ifdef ENABLE_LOCATION
          if (appconn->s_state.location[0]) {
            bassignformat(tmp,
                          "%20s: %s\n",
                          "location",
                          appconn->s_state.location);
            bconcat(s, tmp);
          }
          if (appconn->s_state.pending_location[0]) {
            bassignformat(tmp,
                          "%20s: %s\n",
                          "pending location",
                          appconn->s_state.pending_location);
            bconcat(s, tmp);
          }
          if (appconn->s_state.location_changes) {
            bassignformat(tmp,
                          "%20s: %d\n",
                          "location changes",
                          appconn->s_state.location_changes);
            bconcat(s, tmp);
          }
#endif

          bassignformat(tmp,
                        "%20s: %.*s\n",
                        "user name",
                        appconn->s_state.redir.username[0] ?
                        strlen(appconn->s_state.redir.username) : 1,
                        appconn->s_state.redir.username[0] ?
                        appconn->s_state.redir.username : "-");
          bconcat(s, tmp);

          bassignformat(tmp,
                        "%20s: %d sec\n"
                        "%20s: %d sec\n"
                        "%20s: %d sec\n"
                        "%20s: %d sec\n",
                        "session time",
                        sessiontime,
                        "session timeout",
                        (int)appconn->s_params.sessiontimeout,
                        "idle time",
                        idletime,
                        "idle timeout",
                        (int)appconn->s_params.idletimeout);
          bconcat(s, tmp);

          bassignformat(tmp,
                        "%20s: in=%s, out=%s\n",
                        "octets in/out",
                        in_label, out_label);
          bconcat(s, tmp);

          if (appconn->s_params.maxtotaloctets) {
            bassignformat(tmp,
                          "%20s: %lld\n",
                          "max total octets",
                          appconn->s_params.maxtotaloctets);
            bconcat(s, tmp);
          }

          bassignformat(tmp,
                        "%20s: %lld\n",
                        "octets in",
                        appconn->s_state.input_octets);
          bconcat(s, tmp);

          if (appconn->s_params.maxinputoctets) {
            bassignformat(tmp,
                          "%20s: %lld\n",
                          "max octets in",
                          appconn->s_params.maxinputoctets);
            bconcat(s, tmp);
          }

#ifdef ENABLE_GARDENACCOUNTING
          if (_options.uamgardendata) {
            bassignformat(tmp,
                          "%20s: %lld\n",
                          "garden octets in",
                          appconn->s_state.garden_input_octets);
            bconcat(s, tmp);
            if (_options.uamotherdata) {
              bassignformat(tmp,
                            "%20s: %lld\n",
                            "other octets in",
                            appconn->s_state.other_input_octets);
              bconcat(s, tmp);
            }
          }
#endif

          bassignformat(tmp,
                        "%20s: %lld\n",
                        "octets out",
                        appconn->s_state.output_octets);
          bconcat(s, tmp);

          if (appconn->s_params.maxoutputoctets) {
            bassignformat(tmp,
                          "%20s: %lld\n",
                          "max octets out",
                          appconn->s_params.maxoutputoctets);
            bconcat(s, tmp);
          }

#ifdef ENABLE_GARDENACCOUNTING
          if (_options.uamgardendata) {
            bassignformat(tmp,
                          "%20s: %lld\n",
                          "garden octets out",
                          appconn->s_state.garden_output_octets);
            bconcat(s, tmp);
            if (_options.uamotherdata) {
              bassignformat(tmp,
                            "%20s: %lld\n",
                            "other octets out",
                            appconn->s_state.other_output_octets);
              bconcat(s, tmp);
            }
          }
#endif

          bassignformat(tmp,
                        "%20s: %lld (%d%%)\n"
                        "%20s: %lld (%d%%)\n",
                        "max b/w up",
                        appconn->s_params.bandwidthmaxup,
                        appconn->s_state.bucketupsize ?
                        (int) (appconn->s_state.bucketup * 100
                               / appconn->s_state.bucketupsize) : 0,
                        "max b/w down",
                        appconn->s_params.bandwidthmaxdown,
                        appconn->s_state.bucketdownsize ?
                        (int) (appconn->s_state.bucketdown * 100
                               / appconn->s_state.bucketdownsize) : 0);
          bconcat(s, tmp);

          bassignformat(tmp,
                        "%20s: %d sec ago\n",
                        "last sent time",
                        appconn->s_state.last_up_time ?
                        mainclock_now()-appconn->s_state.last_up_time:0);
          bconcat(s, tmp);

          if (dhcpconn) {
            bassignformat(tmp,
                          "%20s: %d sec ago\n",
                          "last seen",
                          dhcpconn->lasttime ?
                          mainclock_now()-dhcpconn->lasttime:0);
            bconcat(s, tmp);
          }

          bassignformat(tmp,
                        "%20s: %s\n",
                        "user url",
                        appconn->s_state.redir.userurl);
          bconcat(s, tmp);

#ifdef ENABLE_USERAGENT
          if (appconn->s_state.redir.useragent[0]) {
            bassignformat(tmp,
                          "%20s: %s\n",
                          "user agent",
                          appconn->s_state.redir.useragent);
            bconcat(s, tmp);
          }
#endif

#ifdef ENABLE_ACCEPTLANGUAGE
          if (appconn->s_state.redir.acceptlanguage[0]) {
            bassignformat(tmp,
                          "%20s: %s\n",
                          "accept language",
                          appconn->s_state.redir.acceptlanguage);
            bconcat(s, tmp);
          }
#endif

          bassignformat(tmp,
                        "%20s: %s\n",
                        "url param",
                        appconn->s_params.url);
          bconcat(s, tmp);

          bassignformat(tmp,
                        "%20s:",
                        "flags");
          if (appconn->s_params.flags & REQUIRE_UAM_AUTH)
            bcatcstr(tmp, " require-uam-auth");
          if (appconn->s_params.flags & REQUIRE_UAM_SPLASH)
            bcatcstr(tmp, " uam-splash");
          if (appconn->s_params.flags & REQUIRE_REDIRECT)
            bcatcstr(tmp, " require-redirect");
          if (appconn->s_params.flags & NO_ACCOUNTING)
            bcatcstr(tmp, " no-accounting");
          if (appconn->s_params.flags & NO_SCRIPT)
            bcatcstr(tmp, " no-script");
          if (appconn->s_params.flags & UAM_INJECT_URL)
            bcatcstr(tmp, " inject");
          if (appconn->s_params.flags & UAM_CLEAR_URL)
            bcatcstr(tmp, " clear-url");
          bcatcstr(tmp, "\n");
          bconcat(s, tmp);

#ifdef ENABLE_PROXYVSA
          bassignformat(tmp, "%20s: %d\n",
                        "vsa length",
                        (int)appconn->s_state.redir.vsalen);
          bconcat(s, tmp);
#endif

#ifdef ENABLE_SESSGARDEN
          if (appconn->s_params.pass_through_count > 0) {
            char mask[32];
            pass_through *pt;
            int i;

            bassignformat(tmp,
                          "%20s: %d\n",
                          "garden entries",
                          appconn->s_params.pass_through_count);
            bconcat(s, tmp);

            for (i = 0; i < appconn->s_params.pass_through_count; i++) {
              pt = &appconn->s_params.pass_throughs[i];

              strlcpy(mask, inet_ntoa(pt->mask), sizeof(mask));

              bassignformat(tmp,
                            "%20s: host=%-16s mask=%-16s proto=%-3d port=%-3d"
#ifdef ENABLE_GARDENEXT
                            " expiry=%-3d"
#endif
                            "\n", "",
                            inet_ntoa(pt->host), mask,
                            pt->proto, pt->port
#ifdef ENABLE_GARDENEXT
                            , pt->expiry ? pt->expiry - mainclock_now() : 0
#endif
                            );
              bconcat(s, tmp);
            }
          }
#endif

          bdestroy(tmp);
        }
      }
      break;
#endif

    case CMDSOCK_ADD_GARDEN:
    case CMDSOCK_REM_GARDEN:
      {
        char remove = (req->type == CMDSOCK_REM_GARDEN);
#ifdef ENABLE_SESSGARDEN
        uint8_t z[PKT_ETH_ALEN];
        memset(z, 0, PKT_ETH_ALEN);

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): looking to %s to garden ip=%s/sessionid=%s", __FUNCTION__, __LINE__,
                 remove ? "remove" : "add", inet_ntoa(req->ip), req->d.sess.sessionid);

        if (req->ip.s_addr || memcmp(req->mac, z, PKT_ETH_ALEN)) {
          struct app_conn_t *appconn = firstusedconn;

          while (appconn) {
            if (appconn->inuse &&
                ( (req->ip.s_addr != 0 && appconn->hisip.s_addr == req->ip.s_addr) ||
                  (!memcmp(appconn->hismac, req->mac, PKT_ETH_ALEN))
                  ) ) {

              if (_options.debug)
                syslog(LOG_DEBUG, "%s(%d): remote %s garden for session %s", __FUNCTION__, __LINE__,
                       remove ? "rem" : "add", appconn->s_state.sessionid);

#ifdef HAVE_PATRICIA
              if (appconn->ptree == NULL)
                appconn->ptree = patricia_new (32);
#endif

              pass_throughs_from_string(appconn->s_params.pass_throughs,
                                        SESSION_PASS_THROUGH_MAX,
                                        &appconn->s_params.pass_through_count,
                                        req->d.data, 1, remove
#ifdef HAVE_PATRICIA
                                        , appconn->ptree
#endif
                                        );
              break;
            }
            appconn = appconn->next;
          }
        } else {
#endif
          pass_throughs_from_string(dhcp->pass_throughs,
                                    MAX_PASS_THROUGHS,
                                    &dhcp->num_pass_throughs,
                                    req->d.data, 1, remove
#ifdef HAVE_PATRICIA
                                    , dhcp->ptree_dyn
#endif
                                    );
#ifdef ENABLE_SESSGARDEN
        }
#endif
      }
      break;

    case CMDSOCK_LOGOUT:
      {
        struct app_conn_t *appconn = find_app_conn(req, 0);

        if (_options.debug)
          syslog(LOG_DEBUG, "%s(%d): looking to logout session %s", __FUNCTION__, __LINE__,
                 inet_ntoa(req->ip));

        if (appconn) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): found %s %s", __FUNCTION__, __LINE__,
                   inet_ntoa(appconn->hisip), appconn->s_state.sessionid);

          terminate_appconn(appconn, RADIUS_TERMINATE_CAUSE_ADMIN_RESET);
        }
      }
      break;

    case CMDSOCK_LIST_IPPOOL:
      ippool_print(sock, ippool);
      break;

    case CMDSOCK_LIST_GARDEN:
      garden_print(sock);
      break;

    case CMDSOCK_LIST_RADQUEUE:
      radius_printqueue(sock, radius);
      break;

    case CMDSOCK_LIST:
      {
        int listfmt = (req->options & CMDSOCK_OPT_JSON) ?
            LIST_JSON_FMT : LIST_LONG_FMT;

        struct app_conn_t *appconn=0;
        struct dhcp_conn_t *dhcpconn=0;

        int crt = 0;

#ifdef ENABLE_JSON
        if (listfmt == LIST_JSON_FMT) {
          bcatcstr(s, "{ \"sessions\":[");
        }
#endif

        appconn = find_app_conn(req, &crt);
        if (appconn) {
#ifdef ENABLE_LAYER3
          if (!_options.layer3)
#endif
            dhcpconn = (struct dhcp_conn_t *)appconn->dnlink;

          chilli_print(s, listfmt, appconn, dhcpconn);

        } else if (!crt) {
#ifdef ENABLE_LAYER3
          if (_options.layer3) {
            for (appconn = firstusedconn; appconn;
                 appconn = appconn->next) {
              chilli_print(s, listfmt, appconn, 0);
            }
          } else {
#endif
            if (dhcp) {
              dhcpconn = dhcp->firstusedconn;
              while (dhcpconn) {
                chilli_print(s, listfmt, 0, dhcpconn);
                dhcpconn = dhcpconn->next;
              }
            }
#ifdef ENABLE_LAYER3
          }
#endif
        }

#ifdef ENABLE_JSON
        if (listfmt == LIST_JSON_FMT) {
          bcatcstr(s, "]}");
        }
#endif
      }
      break;

    case CMDSOCK_DHCP_LIST:
      if (dhcp) {
        int listfmt = req->options & CMDSOCK_OPT_JSON ?
            LIST_JSON_FMT : LIST_SHORT_FMT;

        struct dhcp_conn_t *conn;

#ifdef ENABLE_JSON
        if (listfmt == LIST_JSON_FMT) {
          bcatcstr(s, "{ \"sessions\":[");
        }
#endif
        conn = dhcp->firstusedconn;
        while (conn) {
          chilli_print(s, listfmt, 0, conn);
          conn = conn->next;
        }
#ifdef ENABLE_JSON
        if (listfmt == LIST_JSON_FMT) {
          bcatcstr(s, "]}");
        }
#endif
      }
      break;

    case CMDSOCK_DHCP_DROP:
      if (dhcp)
        dhcp_block_mac(dhcp, req->mac);
      break;

    case CMDSOCK_DHCP_RELEASE:
      if (dhcp)
        dhcp_release_mac(dhcp, req->mac,
                         RADIUS_TERMINATE_CAUSE_ADMIN_RESET);
      break;

#ifdef ENABLE_MULTIROUTE
    case CMDSOCK_ROUTE_SET:
    case CMDSOCK_ROUTE_GW:
      {
        if (req->type == CMDSOCK_ROUTE_GW) {
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): setting route for idx %d", __FUNCTION__, __LINE__, req->d.sess.params.routeidx);
          copy_mac6(tun(tun, req->d.sess.params.routeidx).gwaddr, req->mac);
        } else {
          struct dhcp_conn_t *conn = dhcp->firstusedconn;
          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): looking to alter session %s", __FUNCTION__, __LINE__, inet_ntoa(req->ip));
          while (conn && conn->inuse) {
            if (conn->peer) {
              struct app_conn_t * appconn = (struct app_conn_t*)conn->peer;
              if (!memcmp(appconn->hismac, req->mac, 6)) {
                if (_options.debug)
                  syslog(LOG_DEBUG, "%s(%d): routeidx %s %d", __FUNCTION__, __LINE__,
                         appconn->s_state.sessionid,
                         req->d.sess.params.routeidx);
                appconn->s_params.routeidx = req->d.sess.params.routeidx;
                break;
              }
            }
            conn = conn->next;
          }
        }
      }
      /* drop through */
    case CMDSOCK_ROUTE:
      {
        int i;
        bstring b = bfromcstr("routes:\n");
        int err = 0;
        if (safe_write(sock, b->data, b->slen) == b->slen) {
          for (i=0; !err && i<tun->_interface_count; i++) {
            char gw[56];

            strlcpy(gw, inet_ntoa(tun->_interfaces[i].gateway), sizeof(gw));

            bassignformat(b, "idx: %d dev: %s %s "MAC_FMT" "
                          "%s "MAC_FMT"%s\n",
                          i, tun->_interfaces[i].devname,
                          inet_ntoa(tun->_interfaces[i].address),
                          MAC_ARG(tun->_interfaces[i].hwaddr),
                          gw,
                          MAC_ARG(tun->_interfaces[i].gwaddr),
                          i == 0 ? " (tun/tap)":"");

            if (safe_write(sock, b->data, b->slen) < 0)
              err = 1;
          }

          if (!err) {
            struct dhcp_conn_t *conn = dhcp->firstusedconn;
            bassignformat(b, "subscribers:\n");
            if (safe_write(sock, b->data, b->slen) == b->slen) {
              while (conn) {
                struct app_conn_t *appconn = (struct app_conn_t *)conn->peer;

                bassignformat(b, "mac: "MAC_FMT" -> idx: %d\n",
                              MAC_ARG(appconn->hismac),
                              appconn->s_params.routeidx);

                if (safe_write(sock, b->data, b->slen) < 0)
                  break;

                conn = conn->next;
              }
            }
          }
        }
        bdestroy(b);
      }
      rtmon_print_ifaces(&_rtmon, sock);
      rtmon_print_routes(&_rtmon, sock);
      break;
#endif

    case CMDSOCK_LOGIN:
    case CMDSOCK_UPDATE:
    case CMDSOCK_AUTHORIZE:
      if (dhcp) {
        struct app_conn_t *appconn = find_app_conn(req, 0);
        if (appconn) {
          char *uname = req->d.sess.username;

          if (_options.debug)
            syslog(LOG_DEBUG, "%s(%d): remotely authorized session %s", __FUNCTION__, __LINE__,
                   appconn->s_state.sessionid);

          memcpy(&appconn->s_params, &req->d.sess.params,
                 sizeof(req->d.sess.params));

          if (uname[0])
            strlcpy(appconn->s_state.redir.username,
                    uname, USERNAMESIZE);

          session_param_defaults(&appconn->s_params);

#ifdef ENABLE_LEAKYBUCKET
          leaky_bucket_init(appconn);
#endif

          switch(req->type) {
            case CMDSOCK_LOGIN:
              auth_radius(appconn, uname, req->d.sess.password, 0, 0);
              break;
            case CMDSOCK_AUTHORIZE:
              dnprot_accept(appconn);
              break;
            case CMDSOCK_UPDATE:
              break;
          }
        }
      }
      break;


#if defined(ENABLE_LOCATION) && defined(HAVE_AVL)
    case CMDSOCK_LISTLOC:
    case CMDSOCK_LISTLOCSUM:
      location_printlist(s, req->d.sess.location,
			 (req->options & CMDSOCK_OPT_JSON),
			 (req->type == CMDSOCK_LISTLOC));
      break;
#endif

    case CMDSOCK_RELOAD:
      _sigusr1(SIGUSR1);
      break;

#ifdef ENABLE_STATFILE
    case CMDSOCK_STATUSFILE:
      printstatus();
      break;
#endif

#ifdef ENABLE_CLUSTER
    case CMDSOCK_PEERS:
      print_peers(s);
      break;

    case CMDSOCK_PEER_SET:
      get_chilli_peer(-1)->state = PEER_STATE_ACTIVE;
      dhcp_peer_update(1);
      break;
#endif

    case CMDSOCK_PROCS:
      child_print(s);
      break;

    default:
      {
        char unknown = 1;
#ifdef ENABLE_MODULES
        int i;
        for (i=0; i < MAX_MODULES; i++) {
          if (!_options.modules[i].name[0]) break;
          if (_options.modules[i].ctx) {
            struct chilli_module *m =
                (struct chilli_module *)_options.modules[i].ctx;
            if (m->cmdsock_handler) {
              switch (m->cmdsock_handler(req, s, sock)) {
                case CHILLI_CMDSOCK_OK:
                  unknown = 0;
                  break;
              }
            }
          }
        }
#endif
        if (unknown) {
          syslog(LOG_ERR, "unknown cmdsock command");
          safe_close(sock);
          return -1;
        }
      }
  }

  return 0;
}
#endif

#ifdef ENABLE_CHILLIQUERY
static int cmdsock_accept(void *nullData, int sock) {
  struct sockaddr_un remote;
  struct cmdsock_request req;

  bstring s = 0;
  socklen_t len;
  int csock;
  int rval = 0;

#if(_debug_)
  if (_options.debug)
    syslog(LOG_DEBUG, "%s(%d): Processing cmdsock request...", __FUNCTION__, __LINE__);
#endif

  len = sizeof(remote);
  if ((csock = safe_accept(sock, (struct sockaddr *)&remote, &len)) == -1) {
    syslog(LOG_ERR, "%s: cmdsock_accept()/accept()", strerror(errno));
    return -1;
  }

  if (safe_read(csock, &req, sizeof(req)) != sizeof(req)) {
    syslog(LOG_ERR, "%s: cmdsock_accept()/read()", strerror(errno));
    safe_close(csock);
    return -1;
  }

  s = bfromcstr("");
  if (s == NULL) {
    syslog(LOG_ERR, "bfromstr(): memory allocation error");
    safe_close(csock);
    return -1;
  }

  rval = chilli_cmd(&req, s, csock);

  if (net_write(csock, s->data, s->slen) < 0)
    syslog(LOG_ERR, "%s: write()", strerror(errno));

  bdestroy(s);
  shutdown(csock, 2);
  safe_close(csock);

  return rval;
}
#endif

#if XXX_IO_DAEMON
int chilli_io(int fd_ctrl_r, int fd_ctrl_w, int fd_pkt_r, int fd_pkt_w) {
  int maxfd = 0;
  fd_set fds;
  int status;

  while (1) {
    fd_zero(&fds);

    fd_set(fd_ctrl_r, &fds);
    fd_set(fd_ctrl_w, &fds);
    fd_set(fd_pkt_r, &fds);
    fd_set(fd_pkt_w, &fds);

    if  ((status = select(maxfd + 1, &fds, NULL, NULL, NULL)) == -1) {
      if (EINTR != errno) {
	syslog(LOG_ERR, "select() returned -1!");
      }
    }

    if (status > 0) {
      if (fd_isset(fd_ctrl_r, &fds)) {
      }
      if (fd_isset(fd_ctrl_w, &fds)) {
      }
      if (fd_isset(fd_pkt_r, &fds)) {
      }
      if (fd_isset(fd_pkt_w, &fds)) {
      }
    } else {
      syslog(LOG_ERR, "%s: problem in select", strerror(errno));
      break;
    }
  }

  exit(1);
}
#endif

#ifdef USING_IPC_UNIX
int static redir_msg(struct redir_t *this) {
  struct redir_msg_t msg;
  struct sockaddr_un remote;
  socklen_t len = sizeof(remote);
  int socket = safe_accept(this->msgfd, (struct sockaddr *)&remote, &len);
  if (socket > 0) {
    int msgresult = safe_read(socket, &msg, sizeof(msg));
    if (msgresult == sizeof(msg)) {
      if (msg.mtype == REDIR_MSG_STATUS_TYPE) {
	struct redir_conn_t conn;
	memset(&conn, 0, sizeof(conn));
	if (cb_redir_getstate(redir,
			      &msg.mdata.address,
			      &msg.mdata.baddress,
			      &conn) != -1) {
	  if (safe_write(socket, &conn, sizeof(conn)) < 0) {
	    syslog(LOG_ERR, "%s: redir_msg writing", strerror(errno));
	  }
	}
      } else {
	uam_msg(&msg);
      }
    } else if (msgresult == -1) {
      syslog(LOG_ERR, "%s: redir_msg read", strerror(errno));
    } else {
      syslog(LOG_ERR, "invalid size %d", msgresult);
    }
    safe_close(socket);
  }
  return 0;
}
#endif

#ifdef ENABLE_MULTIROUTE
int chilli_getconn_byroute(struct app_conn_t **conn, int idx) {

  struct app_conn_t *appconn = firstusedconn;

  while (appconn) {

    if (appconn->s_params.routeidx == idx) {
      *conn = appconn;
      return 0;
    }

    appconn = appconn->next;
  }

  return 1;
}

static int rtmon_proc_route(struct rtmon_t *rtmon,
			    struct rtmon_iface *iface,
			    struct rtmon_route *route) {
  int i;
  for (i=0; i < tun->_interface_count; i++) {
    if (tun->_interfaces[i].ifindex == route->if_index) {
      memcpy(tun->_interfaces[i].gwaddr, route->gwaddr, sizeof(tun->_interfaces[i].gwaddr));
      tun->_interfaces[i].gateway.s_addr = route->gateway.s_addr;
    }
  }

  return 0;
}

static int rtmon_accept(struct rtmon_t *rtmon, int idx) {
  if (rtmon_read_event(rtmon))
    syslog(LOG_ERR, "%s: error reading netlink message", strerror(errno));
  return 0;
}
#endif

static inline void macauth_reserved(void) {
  struct dhcp_conn_t *conn = dhcp->firstusedconn;
  struct app_conn_t *appconn;

  while (conn) {
    if (conn->is_reserved && conn->peer) {
      appconn = (struct app_conn_t *)conn->peer;
      if (!appconn->s_state.authenticated) {
	auth_radius((struct app_conn_t *)conn->peer, 0, 0, 0, 0);
      }
    }
    conn = conn->next;
  }
}

#ifdef ENABLE_LAYER3
static int session_timeout(void) {
  struct app_conn_t *conn = firstusedconn;

  while (conn) {
    struct app_conn_t *check_conn = conn;
    conn = conn->next;
    if (mainclock_diff(check_conn->s_state.last_up_time) >
	_options.lease + _options.leaseplus) {
      if (_options.debug)
        syslog(LOG_DEBUG, "%s(%d): Session timeout: Removing connection", __FUNCTION__, __LINE__);
      session_disconnect(check_conn, 0, RADIUS_TERMINATE_CAUSE_LOST_CARRIER);
    }
  }

  return 0;
}
#endif

int chilli_main(int argc, char **argv) {
  select_ctx sctx;
  int status;

  /*  struct itimerval itval; */
  int lastSecond = 0;

#ifdef ENABLE_CHILLIQUERY
  int cmdsock = -1;
#endif

  pid_t cpid = getpid();

#ifdef USING_IPC_MSG
  struct redir_msg_t msg;
  int msgresult;
#endif

#if XXX_IO_DAEMON
  pid_t chilli_fork = 0;
  int is_slave = 0;

  int ctrl_main_to_io[2];  /* 0/1 read/write - control messages from main -> io */
  int ctrl_io_to_main[2];  /* 0/1 read/write - control messages from io -> main */
  int pkt_main_to_io[2];
  int pkt_io_to_main[2];
#endif

  int i;

  int keep_going = 1;
  int reload_config = 0;

  int syslog_options = LOG_PID;
  int syslog_debug_options = 0;
  char *syslog_ident = NULL;

#ifdef LOG_PERROR
  syslog_debug_options = LOG_PERROR;
#endif

  syslog_ident = basename(argv[0]);

  /* Start out also logging to stderr until we load options. */
  //openlog(syslog_ident, syslog_options|syslog_debug_options, LOG_DAEMON);

  options_init();

  /* Process options given in configuration file and command line */
  if (process_options(argc, argv, 0))
    exit(1);

  /* foreground                                                   */
  /* If flag not given run as a daemon                            */
  if (!_options.foreground) {

#if defined (__FreeBSD__)  || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
    if (fork() > 0) {
      exit(0);
#else
      if (daemon(1, 1)) {
        syslog(LOG_ERR, "daemon() failed!");
#endif
      }
      else {

        /*
         *  We switched PID when we forked.
         *  To keep things nice and tity, lets move the
         *  binary configuration file to the new directory.
         *
         *  TODO: This process isn't ideal. But, the goal remains
         *  that we don't need cmdline.o in the running chilli. We may
         *  want to move away from gengetopt as it isn't exactly the most
         *  flexible or light-weight.
         */

        mode_t process_mask = umask(0077);
        char file[128];
        char file2[128];
        int ok;

        pid_t new_pid = getpid();

        bstring bt = bfromcstr("");

        /*
         * Format the filename of the current (cpid) and new binconfig files.
         */
        chilli_binconfig(file, sizeof(file), cpid);
        chilli_binconfig(file2, sizeof(file2), new_pid);

        /*
         * Reset the binconfig option and save current setttings.
         */
        _options.binconfig = file2;
        ok = options_save(file2, bt);

        if (!ok) {
          syslog(LOG_ERR, "%s: could not save configuration options! [%s]", strerror(errno), file2);
          exit(1);
        }

        /*
         * Reset binconfig (since file2 is a local variable)
         */
        _options.binconfig = 0;
        umask(process_mask);

        bdestroy(bt);

        if (!options_binload(file2)) {
          syslog(LOG_ERR, "%s: could not reload configuration! [%s]", strerror(errno), file2);
          exit(1);
        }
      }
    }

#ifdef LOG_NFACILITIES
//    if (_options.logfacility < 0 || _options.logfacility > LOG_NFACILITIES)
//      _options.logfacility= LOG_FAC(LOG_DAEMON);
#endif

    //closelog();

    //if (_options.debug)
    //  syslog_options |= syslog_debug_options;
    //openlog(syslog_ident, syslog_options, (_options.logfacility<<3));
    //if (!_options.debug)
    //  setlogmask(LOG_UPTO(_options.loglevel));

    chilli_signals(&keep_going, &reload_config);

#if XXX_IO_DAEMON
    pipe(ctrl_main_to_io);
    pipe(ctrl_io_to_main);
    pipe(pkt_main_to_io);
    pipe(pkt_io_to_main);

    chilli_fork = chilli_fork(CHILLI_PROC_DAEMON, "[chilli-io]");
    is_slave = chilli_fork > 0;
    if (chilli_fork < 0) perror("fork()");
    if (chilli_fork == 0)
      /* kick off io daemon */
      return chilli_io(ctrl_main_to_io[0],
                       ctrl_io_to_main[1],
                       pkt_main_to_io[0],
                       pkt_io_to_main[1]);
#endif

    chilli_pid = getpid();

    /* This has to be done after we have our final pid */
    log_pid((_options.pidfile && *_options.pidfile) ? _options.pidfile : DEFPIDFILE);

#ifdef ENABLE_UAMANYIP
    /* setup IPv4LL/APIPA network ip and mask for uamanyip exception */
    inet_aton("169.254.0.0", &ipv4ll_ip);
    inet_aton("255.255.0.0", &ipv4ll_mask);
#endif
#ifdef ENABLE_SSDP
    ssdp.s_addr = inet_addr(SSDP_MCAST_ADDR);
#endif

    syslog(LOG_INFO, "CoovaChilli %s. "
           "Copyright 2002-2005 Mondru AB. Licensed under GPL. "
           "Copyright 2006-2012 David Bird (Coova Technologies). "
           "Licensed under GPL. "
           "See http://coova.github.io/ for details.", VERSION);

    memset(&sctx, 0, sizeof(sctx));

#ifdef HAVE_LIBRT
    memset(&startup_real, 0, sizeof(startup_real));
    memset(&startup_mono, 0, sizeof(startup_mono));
    if (clock_gettime(CLOCK_REALTIME, &startup_real) < 0) {
      syslog(LOG_ERR, "%s: getting startup (realtime) time", strerror(errno));
    }
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): clock realtime sec %lld nsec %ld", __FUNCTION__, __LINE__,
	(long long int)startup_real.tv_sec, startup_real.tv_nsec);
#ifdef CLOCK_MONOTONIC
    if (clock_gettime(CLOCK_MONOTONIC, &startup_mono) < 0) {
      syslog(LOG_ERR, "%s: getting startup (monotonic) time", strerror(errno));
    }
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): clock monotonic sec %lld nsec %ld", __FUNCTION__, __LINE__,
	(long long int)startup_mono.tv_sec, startup_mono.tv_nsec);
#endif
#endif

    start_tick = mainclock_tick();

    /* Create a tunnel interface */
    if (tun_new(&tun)) {
      syslog(LOG_ERR, "Failed to create tun");
      exit(1);
    }

    tun_setaddr(tun,
                &_options.uamlisten,
                &_options.uamlisten,
                &_options.mask);

    tun_set_cb_ind(tun, cb_tun_ind);

    if (_options.ipup)
      tun_runscript(tun, _options.ipup, 0);

    /* Allocate ippool for dynamic IP address allocation */
    if (ippool_new(&ippool,
                   _options.dynip,
                   _options.dhcpstart,
                   _options.dhcpend,
                   _options.statip,
                   _options.allowdyn,
                   _options.allowstat)) {
      syslog(LOG_ERR, "Failed to allocate IP pool!");
      exit(1);
    }

    /* Create an instance of dhcp */
    if (dhcp_new(&dhcp,
                 _options.max_clients,
                 _options.dhcphashsize,
                 _options.dhcpif,
                 _options.dhcpusemac,
                 _options.dhcpmac, 1,
                 &_options.dhcplisten, _options.lease, 1,
                 &_options.uamlisten, _options.uamport,
                 _options.noc2c)) {
      syslog(LOG_ERR, "Failed to create dhcp listener on %s", _options.dhcpif);
      exit(1);
    }

    dhcp_set_cb_request(dhcp, cb_dhcp_request);
    dhcp_set_cb_connect(dhcp, cb_dhcp_connect);
    dhcp_set_cb_disconnect(dhcp, cb_dhcp_disconnect);
    dhcp_set_cb_data_ind(dhcp, cb_dhcp_data_ind);
#ifdef ENABLE_EAPOL
    dhcp_set_cb_eap_ind(dhcp, cb_dhcp_eap_ind);
#endif

    if (dhcp_set(dhcp,
                 _options.ethers,
                 (_options.debug & DEBUG_DHCP))) {
      syslog(LOG_ERR, "Failed to set DHCP parameters");
      exit(1);
    }

    /* Create an instance of radius */
    if (radius_new(&radius,
                   &_options.radiuslisten,
                   _options.coaport,
                   _options.coanoipcheck, 1) ||
        radius_init_q(radius, _options.radiusqsize)) {
      syslog(LOG_ERR, "Failed to create radius");
      return -1;
    }

    radius_set(radius, dhcp ? dhcp->rawif[0].hwaddr : 0,
               (_options.debug & DEBUG_RADIUS));

    radius_set_cb_auth_conf(radius, cb_radius_auth_conf);
#ifdef ENABLE_COA
    radius_set_cb_coa_ind(radius, cb_radius_coa_ind);
#endif
#ifdef ENABLE_RADPROXY
    radius_set_cb_ind(radius, cb_radius_ind);
#endif

    if (_options.acct_update)
      radius_set_cb_acct_conf(radius, cb_radius_acct_conf);

    /* Initialise connections */
    initconn();

    /* Create an instance of redir */
    if (redir_new(&redir, &_options.uamlisten, _options.uamport,
#ifdef ENABLE_UAMUIPORT
                  _options.uamuiport
#else
                  0
#endif
                  )) {
      syslog(LOG_ERR, "Failed to create redir");
      return -1;
    }

    if (!_options.redir && redir_listen(redir)) {
      syslog(LOG_ERR, "Failed to create redir listen");
      return -1;
    }

    if (redir_ipc(redir)) {
      syslog(LOG_ERR, "Failed to create redir IPC");
      return -1;
    }

    redir_set(redir, dhcp->rawif[0].hwaddr, (_options.debug));

    /* not really needed for chilliredir */
    redir_set_cb_getstate(redir, cb_redir_getstate);

#ifdef ENABLE_CHILLIQUERY
    if (_options.cmdsocket) {
      cmdsock = cmdsock_init();
    } else {
      cmdsock = cmdsock_port_init();
    }
    if (cmdsock < 0) {
      syslog(LOG_ERR, "%s: Failed to initialize chilli query socket", strerror(errno));
      return -1;
    }
#endif

    if (_options.radsec) {
#ifdef ENABLE_CHILLIRADSEC
      launch_chilliradsec();
#else
      syslog(LOG_ERR, "Feature is not supported; use --enable-chilliradsec");
      _options.radsec = 0;
#endif
    } else if (_options.uamaaaurl) {
#ifdef ENABLE_CHILLIPROXY
      launch_chilliproxy();
#else
      syslog(LOG_ERR, "Feature is not supported; use --enable-chilliproxy");
#endif
    }

    if (_options.redir) {
#ifdef ENABLE_CHILLIREDIR
      launch_chilliredir();
#else
      syslog(LOG_ERR, "Feature is not supported; use --enable-chilliredir");
      _options.redir = 0;
#endif
    }

#if(_debug_)
    if (_options.debug)
      syslog(LOG_DEBUG, "%s(%d): Waiting for client request...", __FUNCTION__, __LINE__);
#endif

    /*
     * Administrative-User session
     */
    memset(&admin_session, 0, sizeof(admin_session));

#ifdef ENABLE_BINSTATFILE
    if (loadstatus() != 0) /* Only indicate a fresh start-up if we didn't load keepalive sessions */
#endif
    {
#ifdef ENABLE_ACCOUNTING_ONOFF
      acct_req(ACCT_USER, &admin_session, RADIUS_STATUS_TYPE_ACCOUNTING_ON);
#endif
#ifdef HAVE_NETFILTER_COOVA
      if (_options.kname) {
        kmod_coova_clear();
      }
#endif
    }

    if (_options.ethers && *_options.ethers && _options.macauth)
      macauth_reserved();

    if (_options.adminuser) {
      admin_session.is_adminsession = 1;
      strlcpy(admin_session.s_state.redir.username,
              _options.adminuser,
              sizeof(admin_session.s_state.redir.username));
      set_sessionid(&admin_session, 0);
      chilli_auth_radius(radius);
    }

#ifdef ENABLE_UAMDOMAINFILE
    garden_load_domainfile();
#endif

#ifdef HAVE_PATRICIA
    garden_patricia_reload();
#endif

#ifdef ENABLE_LOCATION
    location_init();
#endif

    /******************************************************************/
    /* Main select loop                                               */
    /******************************************************************/

    if (_options.gid && setgid(_options.gid)) {
      syslog(LOG_ERR, "%d setgid(%d) failed while running with gid = %d",
             errno, _options.gid, getgid());
    }

    if (_options.uid && setuid(_options.uid)) {
      syslog(LOG_ERR, "%d setuid(%d) failed while running with uid = %d",
             errno, _options.uid, getuid());
    }

    if (net_select_init(&sctx))
      syslog(LOG_ERR, "%s: select init", strerror(errno));

#ifdef ENABLE_MULTIROUTE
    tun->sctx = &sctx;
    for (i=0; i < tun->_interface_count; i++)
      net_select_reg(&sctx,
                     (tun)->_interfaces[i].fd,
                     SELECT_READ, (select_callback) tun_decaps,
                     tun, i);
#else
    net_select_reg(&sctx,
                   (tun)->_tuntap.fd,
                   SELECT_READ, (select_callback) tun_decaps,
                   tun, 0);
#endif

    net_select_reg(&sctx, selfpipe_init(),
                   SELECT_READ, (select_callback)chilli_handle_signal,
                   0, 0);

    net_select_reg(&sctx, radius->fd, SELECT_READ,
                   (select_callback)radius_decaps, radius, 0);

#ifdef ENABLE_RADPROXY
    if (radius->proxyfd)
      net_select_reg(&sctx, radius->proxyfd, SELECT_READ,
                     (select_callback)radius_proxy_ind, radius, 0);
#endif

#if defined(__linux__)
    net_select_reg(&sctx, dhcp->relayfd, SELECT_READ,
                   (select_callback)dhcp_relay_decaps, dhcp, 0);

    for (i=0; i < MAX_RAWIF && dhcp->rawif[i].fd > 0; i++) {
      net_select_reg(&sctx, dhcp->rawif[i].fd, SELECT_READ,
                     (select_callback)dhcp_decaps, dhcp, i);

      dhcp->rawif[i].sctx = &sctx;
    }

#ifdef HAVE_NETFILTER_QUEUE
    if (dhcp->qif_in.fd && dhcp->qif_out.fd) {
      net_select_reg(&sctx, dhcp->qif_in.fd, SELECT_READ,
                     (select_callback)dhcp_decaps, dhcp, 1);

      net_select_reg(&sctx, dhcp->qif_out.fd, SELECT_READ,
                     (select_callback)dhcp_decaps, dhcp, 2);
    }
#endif

#elif defined (__FreeBSD__)  || defined (__APPLE__) || defined (__OpenBSD__) || defined (__NetBSD__)
    for (i=0; i < MAX_RAWIF && dhcp->rawif[i].fd > 0; i++) {
      net_select_reg(&sctx, dhcp->rawif[i].fd, SELECT_READ,
                     (select_callback)dhcp_receive, dhcp, i);
    }
#endif

#ifdef USING_IPC_UNIX
    net_select_reg(&sctx, redir->msgfd, SELECT_READ,
                   (select_callback)redir_msg, redir, 0);
#endif

    if (!_options.redir) {
      net_select_reg(&sctx, redir->fd[0], SELECT_READ,
                     (select_callback)redir_accept, redir, 0);
      net_select_reg(&sctx, redir->fd[1], SELECT_READ,
                     (select_callback)redir_accept, redir, 1);
    }

#ifdef ENABLE_MULTIROUTE
    if (!rtmon_init(&_rtmon, rtmon_proc_route)) {
      net_select_reg(&sctx, _rtmon.fd, SELECT_READ,
                     (select_callback)rtmon_accept, &_rtmon, 0);
    }
#endif

#ifdef ENABLE_CHILLIQUERY
    net_select_reg(&sctx, cmdsock, SELECT_READ,
                   (select_callback)cmdsock_accept, 0, cmdsock);
#endif

    mainclock_tick();
    while (keep_going) {

      if (reload_config) {

        reload_options(argc, argv);

        reload_config = 0;

        /* Reinit DHCP parameters */
        if (dhcp) {
          dhcp_set(dhcp,
                   _options.ethers,
                   (_options.debug & DEBUG_DHCP));
        }

        /* Reinit RADIUS parameters */
        radius_set(radius, dhcp->rawif[0].hwaddr,
                   (_options.debug & DEBUG_RADIUS));

        /* Reinit Redir parameters */
        redir_set(redir, dhcp->rawif[0].hwaddr, _options.debug);

#ifdef HAVE_PATRICIA
        garden_patricia_reload();
#endif

#ifdef ENABLE_UAMDOMAINFILE
        garden_load_domainfile();
#endif
      }

      if (do_interval) {
        reprocess_options(argc, argv);

        do_interval = 0;

        if (_options.adminuser)
          chilli_auth_radius(radius);
      }

      if (lastSecond != mainclock.tv_sec) {
        /*
         *  Every second, more or less
         */
        radius_timeout(radius);

        if (dhcp)
          dhcp_timeout(dhcp);

#ifdef ENABLE_LAYER3
        if (_options.layer3)
          session_timeout();
#endif

        checkconn();
        lastSecond = mainclock.tv_sec;

#ifdef ENABLE_CLUSTER
        dhcp_peer_update(0);
#endif
      }

      if (net_select_prepare(&sctx))
        syslog(LOG_ERR, "%s: select prepare", strerror(errno));

      status = net_select(&sctx);

      mainclock_tick();

#ifdef USING_IPC_MSG
      if ((msgresult =
           TEMP_FAILURE_RETRY(msgrcv(redir->msgid, (void *)&msg, sizeof(msg.mdata), 0, IPC_NOWAIT)))  == -1) {
        if ((errno != EAGAIN) && (errno != ENOMSG))
          syslog(LOG_ERR, "%s: msgrcv() failed!", strerror(errno));
      }

      if (msgresult > 0)
        uam_msg(&msg);
#endif

      if (status > 0) {

        net_run_selected(&sctx, status);

      }

#ifdef USING_MMAP

      net_run(&dhcp->rawif[0]);

#ifdef ENABLE_MULTIROUTE
      if (tun) {
        for (i=0; i < (tun)->_interface_count; i++) {
          net_run(&(tun)->_interfaces[i]);
        }
      }
#endif

#endif

    } /* while(keep_going) */

    syslog(LOG_INFO, "CoovaChilli shutting down");

    if (_options.seskeepalive) {
#ifdef ENABLE_BINSTATFILE
      if (printstatus() != 0)
        syslog(LOG_ERR, "%s: could not save status file", strerror(errno));
#else
      syslog(LOG_WARNING, "Not stopping sessions! seskeepalive should be used with compile option --enable-binstatusfile");
#endif
    } else {
      killconn();
#ifdef ENABLE_STATFILE
      if (printstatus() != 0)
        syslog(LOG_ERR, "%s: could not save status file", strerror(errno));
#endif
    }

    child_killall(SIGTERM);

    if (_options.ipdown)
      tun_runscript(tun, _options.ipdown, 1);

    if (redir)
      redir_free(redir);

    if (radius)
      radius_free(radius);

    if (dhcp)
      dhcp_free(dhcp);

    if (tun)
      tun_free(tun);

    if (ippool)
      ippool_free(ippool);

    /*
     *  Terminate not-so-nicely
     */

#ifdef ENABLE_CHILLIQUERY
    cmdsock_shutdown();
#endif

#ifdef ENABLE_CHILLIREDIR
    if (redir_pid > 0) {
      kill(redir_pid, SIGTERM);
    }
#endif
#ifdef ENABLE_CHILLIPROXY
    if (proxy_pid > 0) {
      kill(proxy_pid, SIGTERM);
    }
#endif
#ifdef ENABLE_CHILLIRADSEC
    if (radsec_pid > 0) {
      kill(radsec_pid, SIGTERM);
    }
#endif

#ifdef ENABLE_UAMDOMAINFILE
    garden_free_domainfile();
#endif

    selfpipe_finish();

    /* child_killall(SIGKILL);*/

    options_cleanup();

    return 0;
  }

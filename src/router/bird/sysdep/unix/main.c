/*
 *	BIRD Internet Routing Daemon -- Unix Entry Point
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#undef LOCAL_DEBUG

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <libgen.h>

#include "nest/bird.h"
#include "lib/lists.h"
#include "lib/resource.h"
#include "lib/socket.h"
#include "lib/event.h"
#include "lib/string.h"
#include "nest/route.h"
#include "nest/protocol.h"
#include "nest/iface.h"
#include "nest/cli.h"
#include "nest/locks.h"
#include "conf/conf.h"
#include "filter/filter.h"

#include "unix.h"
#include "krt.h"

/*
 *	Debugging
 */

#ifdef DEBUGGING
static int debug_flag = 1;
#else
static int debug_flag = 0;
#endif

void
async_dump(void)
{
  debug("INTERNAL STATE DUMP\n\n");

  rdump(&root_pool);
  sk_dump_all();
  tm_dump_all();
  if_dump_all();
  neigh_dump_all();
  rta_dump_all();
  rt_dump_all();
  protos_dump_all();

  debug("\n");
}

/*
 *	Dropping privileges
 */

#ifdef CONFIG_RESTRICTED_PRIVILEGES
#include "lib/syspriv.h"
#else

static inline void
drop_uid(uid_t uid)
{
  die("Cannot change user on this platform");
}

#endif

static inline void
drop_gid(gid_t gid)
{
  if (setgid(gid) < 0)
    die("setgid: %m");
}

/*
 *	Reading the Configuration
 */

#ifdef PATH_IPROUTE_DIR

static inline void
add_num_const(char *name, int val)
{
  struct symbol *s = cf_find_symbol(name);
  s->class = SYM_CONSTANT | T_INT;
  s->def = cfg_allocz(sizeof(struct f_val));
  SYM_TYPE(s) = T_INT;
  SYM_VAL(s).i = val;
}

/* the code of read_iproute_table() is based on
   rtnl_tab_initialize() from iproute2 package */
static void
read_iproute_table(char *file, char *prefix, int max)
{
  char buf[512], namebuf[512];
  char *name;
  int val;
  FILE *fp;

  strcpy(namebuf, prefix);
  name = namebuf + strlen(prefix);

  fp = fopen(file, "r");
  if (!fp)
    return;

  while (fgets(buf, sizeof(buf), fp))
  {
    char *p = buf;

    while (*p == ' ' || *p == '\t')
      p++;

    if (*p == '#' || *p == '\n' || *p == 0)
      continue;
   
    if (sscanf(p, "0x%x %s\n", &val, name) != 2 &&
	sscanf(p, "0x%x %s #", &val, name) != 2 &&
	sscanf(p, "%d %s\n", &val, name) != 2 &&
	sscanf(p, "%d %s #", &val, name) != 2)
      continue;

    if (val < 0 || val > max)
      continue;

    for(p = name; *p; p++)
      if ((*p < 'a' || *p > 'z') && (*p < '0' || *p > '9') && (*p != '_'))
	*p = '_';

    add_num_const(namebuf, val);
  }

  fclose(fp);
}

#endif // PATH_IPROUTE_DIR


static char *config_name = PATH_CONFIG_FILE;

static int
cf_read(byte *dest, unsigned int len, int fd)
{
  int l = read(fd, dest, len);
  if (l < 0)
    cf_error("Read error");
  return l;
}

void
sysdep_preconfig(struct config *c)
{
  init_list(&c->logfiles);

#ifdef PATH_IPROUTE_DIR
  read_iproute_table(PATH_IPROUTE_DIR "/rt_protos", "ipp_", 256);
  read_iproute_table(PATH_IPROUTE_DIR "/rt_realms", "ipr_", 256);
  read_iproute_table(PATH_IPROUTE_DIR "/rt_scopes", "ips_", 256);
  read_iproute_table(PATH_IPROUTE_DIR "/rt_tables", "ipt_", 256);
#endif
}

int
sysdep_commit(struct config *new, struct config *old UNUSED)
{
  log_switch(debug_flag, &new->logfiles, new->syslog_name);
  return 0;
}

static int
unix_read_config(struct config **cp, char *name)
{
  struct config *conf = config_alloc(name);
  int ret;

  *cp = conf;
  conf->file_fd = open(name, O_RDONLY);
  if (conf->file_fd < 0)
    return 0;
  cf_read_hook = cf_read;
  ret = config_parse(conf);
  close(conf->file_fd);
  return ret;
}

static struct config *
read_config(void)
{
  struct config *conf;

  if (!unix_read_config(&conf, config_name))
    {
      if (conf->err_msg)
	die("%s, line %d: %s", conf->err_file_name, conf->err_lino, conf->err_msg);
      else
	die("Unable to open configuration file %s: %m", config_name);
    }

  return conf;
}

void
async_config(void)
{
  struct config *conf;

  log(L_INFO "Reconfiguration requested by SIGHUP");
  if (!unix_read_config(&conf, config_name))
    {
      if (conf->err_msg)
	log(L_ERR "%s, line %d: %s", conf->err_file_name, conf->err_lino, conf->err_msg);
      else
	log(L_ERR "Unable to open configuration file %s: %m", config_name);
      config_free(conf);
    }
  else
    config_commit(conf, RECONFIG_HARD, 0);
}

static struct config *
cmd_read_config(char *name)
{
  struct config *conf;

  if (!name)
    name = config_name;

  cli_msg(-2, "Reading configuration from %s", name);
  if (!unix_read_config(&conf, name))
    {
      if (conf->err_msg)
	cli_msg(8002, "%s, line %d: %s", conf->err_file_name, conf->err_lino, conf->err_msg);
      else
	cli_msg(8002, "%s: %m", name);
      config_free(conf);
      conf = NULL;
    }

  return conf;
}

void
cmd_check_config(char *name)
{
  struct config *conf = cmd_read_config(name);
  if (!conf)
    return;

  cli_msg(20, "Configuration OK");
  config_free(conf);
}

static void
cmd_reconfig_msg(int r)
{
  switch (r)
    {
    case CONF_DONE:	cli_msg( 3, "Reconfigured"); break;
    case CONF_PROGRESS: cli_msg( 4, "Reconfiguration in progress"); break;
    case CONF_QUEUED:	cli_msg( 5, "Reconfiguration already in progress, queueing new config"); break;
    case CONF_UNQUEUED:	cli_msg(17, "Reconfiguration already in progress, removing queued config"); break;
    case CONF_CONFIRM:	cli_msg(18, "Reconfiguration confirmed"); break;
    case CONF_SHUTDOWN:	cli_msg( 6, "Reconfiguration ignored, shutting down"); break;
    case CONF_NOTHING:	cli_msg(19, "Nothing to do"); break;
    default:		break;
    }
}

/* Hack for scheduled undo notification */
cli *cmd_reconfig_stored_cli;

void
cmd_reconfig_undo_notify(void)
{
  if (cmd_reconfig_stored_cli)
    {
      cli *c = cmd_reconfig_stored_cli;
      cli_printf(c, CLI_ASYNC_CODE, "Config timeout expired, starting undo");
      cli_write_trigger(c);
    }
}

void
cmd_reconfig(char *name, int type, int timeout)
{
  if (cli_access_restricted())
    return;

  struct config *conf = cmd_read_config(name);
  if (!conf)
    return;

  int r = config_commit(conf, type, timeout);

  if ((r >= 0) && (timeout > 0))
    {
      cmd_reconfig_stored_cli = this_cli;
      cli_msg(-22, "Undo scheduled in %d s", timeout);
    }

  cmd_reconfig_msg(r);
}

void
cmd_reconfig_confirm(void)
{
  if (cli_access_restricted())
    return;

  int r = config_confirm();
  cmd_reconfig_msg(r);
}

void
cmd_reconfig_undo(void)
{
  if (cli_access_restricted())
    return;

  cli_msg(-21, "Undo requested");

  int r = config_undo();
  cmd_reconfig_msg(r);
}

/*
 *	Command-Line Interface
 */

static sock *cli_sk;
static char *path_control_socket = PATH_CONTROL_SOCKET;


static void
cli_write(cli *c)
{
  sock *s = c->priv;

  while (c->tx_pos)
    {
      struct cli_out *o = c->tx_pos;

      int len = o->wpos - o->outpos;
      s->tbuf = o->outpos;
      o->outpos = o->wpos;

      if (sk_send(s, len) <= 0)
	return;

      c->tx_pos = o->next;
    }

  /* Everything is written */
  s->tbuf = NULL;
  cli_written(c);
}

void
cli_write_trigger(cli *c)
{
  sock *s = c->priv;

  if (s->tbuf == NULL)
    cli_write(c);
}

static void
cli_tx(sock *s)
{
  cli_write(s->data);
}

int
cli_get_command(cli *c)
{
  sock *s = c->priv;
  byte *t = c->rx_aux ? : s->rbuf;
  byte *tend = s->rpos;
  byte *d = c->rx_pos;
  byte *dend = c->rx_buf + CLI_RX_BUF_SIZE - 2;

  while (t < tend)
    {
      if (*t == '\r')
	t++;
      else if (*t == '\n')
	{
	  t++;
	  c->rx_pos = c->rx_buf;
	  c->rx_aux = t;
	  *d = 0;
	  return (d < dend) ? 1 : -1;
	}
      else if (d < dend)
	*d++ = *t++;
    }
  c->rx_aux = s->rpos = s->rbuf;
  c->rx_pos = d;
  return 0;
}

static int
cli_rx(sock *s, int size UNUSED)
{
  cli_kick(s->data);
  return 0;
}

static void
cli_err(sock *s, int err)
{
  if (config->cli_debug)
    {
      if (err)
	log(L_INFO "CLI connection dropped: %s", strerror(err));
      else
	log(L_INFO "CLI connection closed");
    }
  cli_free(s->data);
}

static int
cli_connect(sock *s, int size UNUSED)
{
  cli *c;

  if (config->cli_debug)
    log(L_INFO "CLI connect");
  s->rx_hook = cli_rx;
  s->tx_hook = cli_tx;
  s->err_hook = cli_err;
  s->data = c = cli_new(s);
  s->pool = c->pool;		/* We need to have all the socket buffers allocated in the cli pool */
  c->rx_pos = c->rx_buf;
  c->rx_aux = NULL;
  rmove(s, c->pool);
  return 1;
}

static void
cli_init_unix(uid_t use_uid, gid_t use_gid)
{
  sock *s;

  cli_init();
  s = cli_sk = sk_new(cli_pool);
  s->type = SK_UNIX_PASSIVE;
  s->rx_hook = cli_connect;
  s->rbsize = 1024;

  /* Return value intentionally ignored */
  unlink(path_control_socket);

  if (sk_open_unix(s, path_control_socket) < 0)
    die("Cannot create control socket %s: %m", path_control_socket);

  if (use_uid || use_gid)
    if (chown(path_control_socket, use_uid, use_gid) < 0)
      die("chown: %m");

  if (chmod(path_control_socket, 0660) < 0)
    die("chmod: %m");
}

/*
 *	PID file
 */

static char *pid_file;
static int pid_fd;

static inline void
open_pid_file(void)
{
  if (!pid_file)
    return;

  pid_fd = open(pid_file, O_WRONLY|O_CREAT, 0664);
  if (pid_fd < 0)
    die("Cannot create PID file %s: %m", pid_file);
}

static inline void
write_pid_file(void)
{
  int pl, rv;
  char ps[24];

  if (!pid_file)
    return;

  /* We don't use PID file for uniqueness, so no need for locking */

  pl = bsnprintf(ps, sizeof(ps), "%ld\n", (long) getpid());
  if (pl < 0)
    bug("PID buffer too small");

  rv = ftruncate(pid_fd, 0);
  if (rv < 0)
    die("fruncate: %m");
    
  rv = write(pid_fd, ps, pl);
  if(rv < 0)
    die("write: %m");

  close(pid_fd);
}

static inline void
unlink_pid_file(void)
{
  if (pid_file)
    unlink(pid_file);
}


/*
 *	Shutdown
 */

void
cmd_shutdown(void)
{
  if (cli_access_restricted())
    return;

  cli_msg(7, "Shutdown requested");
  order_shutdown();
}

void
async_shutdown(void)
{
  DBG("Shutting down...\n");
  order_shutdown();
}

void
sysdep_shutdown_done(void)
{
  unlink_pid_file();
  unlink(path_control_socket);
  log_msg(L_FATAL "Shutdown completed");
  exit(0);
}

/*
 *	Signals
 */

static void
handle_sighup(int sig UNUSED)
{
  DBG("Caught SIGHUP...\n");
  async_config_flag = 1;
}

static void
handle_sigusr(int sig UNUSED)
{
  DBG("Caught SIGUSR...\n");
  async_dump_flag = 1;
}

static void
handle_sigterm(int sig UNUSED)
{
  DBG("Caught SIGTERM...\n");
  async_shutdown_flag = 1;
}

static void
signal_init(void)
{
  struct sigaction sa;

  bzero(&sa, sizeof(sa));
  sa.sa_handler = handle_sigusr;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGUSR1, &sa, NULL);
  sa.sa_handler = handle_sighup;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGHUP, &sa, NULL);
  sa.sa_handler = handle_sigterm;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGTERM, &sa, NULL);
  signal(SIGPIPE, SIG_IGN);
}

/*
 *	Parsing of command-line arguments
 */

static char *opt_list = "c:dD:ps:P:u:g:fR";
static int parse_and_exit;
char *bird_name;
static char *use_user;
static char *use_group;
static int run_in_foreground = 0;

static void
usage(void)
{
  fprintf(stderr, "Usage: %s [-c <config-file>] [-d] [-D <debug-file>] [-p] [-s <control-socket>] [-P <pid-file>] [-u <user>] [-g <group>] [-f] [-R]\n", bird_name);
  exit(1);
}

static inline char *
get_bird_name(char *s, char *def)
{
  char *t;
  if (!s)
    return def;
  t = strrchr(s, '/');
  if (!t)
    return s;
  if (!t[1])
    return def;
  return t+1;
}

static inline uid_t
get_uid(const char *s)
{
  struct passwd *pw;
  char *endptr;
  long int rv;

  if (!s)
    return 0;

  errno = 0;
  rv = strtol(s, &endptr, 10);

  if (!errno && !*endptr)
    return rv;

  pw = getpwnam(s);
  if (!pw)
    die("Cannot find user '%s'", s);

  return pw->pw_uid;
}

static inline gid_t
get_gid(const char *s)
{
  struct group *gr;
  char *endptr;
  long int rv;

  if (!s)
    return 0;
  
  errno = 0;
  rv = strtol(s, &endptr, 10);

  if (!errno && !*endptr)
    return rv;

  gr = getgrnam(s);
  if (!gr)
    die("Cannot find group '%s'", s);

  return gr->gr_gid;
}

static void
parse_args(int argc, char **argv)
{
  int c;

  bird_name = get_bird_name(argv[0], "bird");
  if (argc == 2)
    {
      if (!strcmp(argv[1], "--version"))
	{
	  fprintf(stderr, "BIRD version " BIRD_VERSION "\n");
	  exit(0);
	}
      if (!strcmp(argv[1], "--help"))
	usage();
    }
  while ((c = getopt(argc, argv, opt_list)) >= 0)
    switch (c)
      {
      case 'c':
	config_name = optarg;
	break;
      case 'd':
	debug_flag |= 1;
	break;
      case 'D':
	log_init_debug(optarg);
	debug_flag |= 2;
	break;
      case 'p':
	parse_and_exit = 1;
	break;
      case 's':
	path_control_socket = optarg;
	break;
      case 'P':
	pid_file = optarg;
	break;
      case 'u':
	use_user = optarg;
	break;
      case 'g':
	use_group = optarg;
	break;
      case 'f':
	run_in_foreground = 1;
	break;
      case 'R':
	graceful_restart_recovery();
	break;
      default:
	usage();
      }
  if (optind < argc)
    usage();
}

/*
 *	Hic Est main()
 */

int
main(int argc, char **argv)
{
#ifdef HAVE_LIBDMALLOC
  if (!getenv("DMALLOC_OPTIONS"))
    dmalloc_debug(0x2f03d00);
#endif

  parse_args(argc, argv);
  if (debug_flag == 1)
    log_init_debug("");
  log_switch(debug_flag, NULL, NULL);

  resource_init();
  olock_init();
  io_init();
  rt_init();
  if_init();
  roa_init();
  config_init();

  uid_t use_uid = get_uid(use_user);
  gid_t use_gid = get_gid(use_group);

  if (!parse_and_exit)
  {
    test_old_bird(path_control_socket);
    cli_init_unix(use_uid, use_gid);
  }

  if (use_gid)
    drop_gid(use_gid);

  if (use_uid)
    drop_uid(use_uid);

  if (!parse_and_exit)
    open_pid_file();

  protos_build();
  proto_build(&proto_unix_kernel);
  proto_build(&proto_unix_iface);

  struct config *conf = read_config();

  if (parse_and_exit)
    exit(0);

  if (!(debug_flag||run_in_foreground))
    {
      pid_t pid = fork();
      if (pid < 0)
	die("fork: %m");
      if (pid)
	return 0;
      setsid();
      close(0);
      if (open("/dev/null", O_RDWR) < 0)
	die("Cannot open /dev/null: %m");
      dup2(0, 1);
      dup2(0, 2);
    }

  main_thread_init();

  write_pid_file();

  signal_init();

  config_commit(conf, RECONFIG_HARD, 0);

  graceful_restart_init();

#ifdef LOCAL_DEBUG
  async_dump_flag = 1;
#endif

  log(L_INFO "Started");
  DBG("Entering I/O loop.\n");

  io_loop();
  bug("I/O loop died");
}

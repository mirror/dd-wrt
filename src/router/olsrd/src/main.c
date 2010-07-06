/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>

#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "scheduler.h"
#include "parser.h"
#include "generate_msg.h"
#include "plugin_loader.h"
#include "apm.h"
#include "net_os.h"
#include "build_msg.h"
#include "net_olsr.h"
#include "mid_set.h"
#include "mpr_selector_set.h"
#include "gateway.h"
#include "olsr_niit.h"

#ifdef LINUX_NETLINK_ROUTING
#include <linux/types.h>
#include <linux/rtnetlink.h>
#include "kernel_routes.h"

#endif

#ifdef WIN32
#include <winbase.h>
#define close(x) closesocket(x)
int __stdcall SignalHandler(unsigned long signo) __attribute__ ((noreturn));
void ListInterfaces(void);
void DisableIcmpRedirects(void);
bool olsr_win32_end_request = false;
bool olsr_win32_end_flag = false;
#else
static void olsr_shutdown(int) __attribute__ ((noreturn));
#endif

#if defined linux || __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
#define DEFAULT_LOCKFILE_PREFIX "/var/run/olsrd"
#elif defined WIN32
#define DEFAULT_LOCKFILE_PREFIX "C:\\olsrd"
#else
#define DEFAULT_LOCKFILE_PREFIX "olsrd"
#endif

/*
 * Local function prototypes
 */
void olsr_reconfigure(int) __attribute__ ((noreturn));

static void print_usage(bool error);

static int set_default_ifcnfs(struct olsr_if *, struct if_config_options *);

static int olsr_process_arguments(int, char *[], struct olsrd_config *,
    struct if_config_options *);

#ifndef WIN32
static char **olsr_argv;
#endif

static char
    copyright_string[] __attribute__ ((unused)) =
        "The olsr.org Optimized Link-State Routing daemon(olsrd) Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org) All rights reserved.";

/* Data for OLSR locking */
#ifndef WIN32
static int lock_fd = 0;
#endif
static char lock_file_name[FILENAME_MAX];
struct olsr_cookie_info *def_timer_ci = NULL;

/*
 * Creates a zero-length locking file and use fcntl to
 * place an exclusive lock over it. The lock will be
 * automatically erased when the olsrd process ends,
 * so it will even work well with a SIGKILL.
 *
 * Additionally the lock can be killed by removing the
 * locking file.
 */
static void olsr_create_lock_file(void) {
#ifdef WIN32
  HANDLE lck = CreateEvent(NULL, TRUE, FALSE, lock_file_name);
  if (NULL == lck || ERROR_ALREADY_EXISTS == GetLastError()) {
    if (NULL == lck) {
      fprintf(stderr,
          "Error, cannot create OLSR lock '%s'.\n",
          lock_file_name);
    } else {
      CloseHandle(lck);
      fprintf(stderr,
          "Error, cannot aquire OLSR lock '%s'.\n"
          "Another OLSR instance might be running.\n",
          lock_file_name);
    }
    olsr_exit("", EXIT_FAILURE);
  }
#else
  struct flock lck;

  /* create file for lock */
  lock_fd = open(lock_file_name, O_WRONLY | O_CREAT, S_IRWXU);
  if (lock_fd == 0) {
    close(lock_fd);
    fprintf(stderr,
        "Error, cannot create OLSR lock '%s'.\n",
        lock_file_name);
    olsr_exit("", EXIT_FAILURE);
  }

  /* create exclusive lock for the whole file */
  lck.l_type = F_WRLCK;
  lck.l_whence = SEEK_SET;
  lck.l_start = 0;
  lck.l_len = 0;
  lck.l_pid = 0;

  if (fcntl(lock_fd, F_SETLK, &lck) == -1) {
    close(lock_fd);
    fprintf(stderr,
        "Error, cannot aquire OLSR lock '%s'.\n"
        "Another OLSR instance might be running.\n",
        lock_file_name);
    olsr_exit("", EXIT_FAILURE);
  }
#endif
  return;
}

/**
 * loads a config file
 * @return <0 if load failed, 0 otherwise
 */
static int
olsrmain_load_config(char *file) {
  struct stat statbuf;

  if (stat(file, &statbuf) < 0) {
    fprintf(stderr, "Could not find specified config file %s!\n%s\n\n",
        file, strerror(errno));
    return -1;
  }

  if (olsrd_parse_cnf(file) < 0) {
    fprintf(stderr, "Error while reading config file %s!\n", file);
    return -1;
  }
  return 0;
}

/**
 * Main entrypoint
 */

int main(int argc, char *argv[]) {
  struct if_config_options *default_ifcnf;
  char conf_file_name[FILENAME_MAX];
  struct ipaddr_str buf;
  bool loadedConfig = false;
  int i;

#ifdef LINUX_NETLINK_ROUTING
  struct interface *ifn;
#endif

#ifdef WIN32
  WSADATA WsaData;
  size_t len;
#endif

  /* paranoia checks */
  assert(sizeof(uint8_t) == 1);
  assert(sizeof(uint16_t) == 2);
  assert(sizeof(uint32_t) == 4);
  assert(sizeof(int8_t) == 1);
  assert(sizeof(int16_t) == 2);
  assert(sizeof(int32_t) == 4);

  printf("\n *** %s ***\n Build date: %s on %s\n http://www.olsr.org\n\n",
      olsrd_version, build_date, build_host);

  if (argc == 2) {
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "/?") == 0) {
      print_usage(false);
      exit(0);
    }
    if (strcmp(argv[1], "-v") == 0) {
      exit(0);
    }
  }

  debug_handle = stdout;
#ifndef WIN32
  olsr_argv = argv;
#endif
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

#ifndef WIN32
  /* Check if user is root */
  if (geteuid()) {
    fprintf(stderr, "You must be root(uid = 0) to run olsrd!\nExiting\n\n");
    exit(EXIT_FAILURE);
  }
#else
  DisableIcmpRedirects();

  if (WSAStartup(0x0202, &WsaData)) {
    fprintf(stderr, "Could not initialize WinSock.\n");
    olsr_exit(__func__, EXIT_FAILURE);
  }
#endif

  /* Open syslog */
  olsr_openlog("olsrd");

  /* Using PID as random seed */
  srandom(getpid());

  /* Init widely used statics */
  memset(&all_zero, 0, sizeof(union olsr_ip_addr));

  /*
   * Set configfile name and
   * check if a configfile name was given as parameter
   */
#ifdef WIN32
#ifndef WINCE
  GetWindowsDirectory(conf_file_name, FILENAME_MAX - 11);
#else
  conf_file_name[0] = 0;
#endif

  len = strlen(conf_file_name);

  if (len == 0 || conf_file_name[len - 1] != '\\')
  conf_file_name[len++] = '\\';

  strscpy(conf_file_name + len, "olsrd.conf", sizeof(conf_file_name) - len);
#else
  strscpy(conf_file_name, OLSRD_GLOBAL_CONF_FILE, sizeof(conf_file_name));
#endif

  olsr_cnf = olsrd_get_default_cnf();
  for (i=1; i < argc-1;) {
    if (strcmp(argv[i], "-f") == 0) {
      loadedConfig = true;

      if (olsrmain_load_config(argv[i+1]) < 0) {
        exit(EXIT_FAILURE);
      }

      if (i+2 < argc) {
        memmove(&argv[i], &argv[i+2], sizeof(*argv) * (argc-i-1));
      }
      argc -= 2;
    }
    else {
      i++;
    }
  }

  /*
   * set up configuration prior to processing commandline options
   */
  if (!loadedConfig && olsrmain_load_config(conf_file_name) == 0) {
    loadedConfig = true;
  }

  if (!loadedConfig) {
    olsrd_free_cnf(olsr_cnf);
    olsr_cnf = olsrd_get_default_cnf();
  }

  default_ifcnf = get_default_if_config();
  if (default_ifcnf == NULL) {
    fprintf(stderr, "No default ifconfig found!\n");
    exit(EXIT_FAILURE);
  }

  /* Initialize timers */
  olsr_init_timers();

  /*
   * Process olsrd options.
   */
  if (olsr_process_arguments(argc, argv, olsr_cnf, default_ifcnf) < 0) {
    print_usage(true);
    olsr_exit(__func__, EXIT_FAILURE);
  }

  /*
   * Set configuration for command-line specified interfaces
   */
  set_default_ifcnfs(olsr_cnf->interfaces, default_ifcnf);

  /* free the default ifcnf */
  free(default_ifcnf);

  /* Sanity check configuration */
  if (olsrd_sanity_check_cnf(olsr_cnf) < 0) {
    fprintf(stderr, "Bad configuration!\n");
    olsr_exit(__func__, EXIT_FAILURE);
  }

  /*
   * Establish file lock to prevent multiple instances
   */
  if (olsr_cnf->lock_file) {
    strscpy(lock_file_name, olsr_cnf->lock_file, sizeof(lock_file_name));
  } else {
    size_t l;
#ifdef DEFAULT_LOCKFILE_PREFIX
    strscpy(lock_file_name, DEFAULT_LOCKFILE_PREFIX, sizeof(lock_file_name));
#else
    strscpy(lock_file_name, conf_file_name, sizeof(lock_file_name));
#endif
    l = strlen(lock_file_name);
    snprintf(&lock_file_name[l], sizeof(lock_file_name) - l, "-ipv%d.lock",
        olsr_cnf->ip_version == AF_INET ? 4 : 6);
  }

  /*
   * Print configuration
   */
  if (olsr_cnf->debug_level > 1) {
    olsrd_print_cnf(olsr_cnf);
  }

  def_timer_ci = olsr_alloc_cookie("Default Timer Cookie", OLSR_COOKIE_TYPE_TIMER);

  /*
   * socket for ioctl calls
   */
  olsr_cnf->ioctl_s = socket(olsr_cnf->ip_version, SOCK_DGRAM, 0);
  if (olsr_cnf->ioctl_s < 0) {
#ifndef WIN32
    olsr_syslog(OLSR_LOG_ERR, "ioctl socket: %m");
#endif
    olsr_exit(__func__, 0);
  }
#ifdef LINUX_NETLINK_ROUTING
  olsr_cnf->rtnl_s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
  if (olsr_cnf->rtnl_s < 0) {
    olsr_syslog(OLSR_LOG_ERR, "rtnetlink socket: %m");
    olsr_exit(__func__, 0);
  }
  fcntl(olsr_cnf->rtnl_s, F_SETFL, O_NONBLOCK);

  if ((olsr_cnf->rt_monitor_socket = rtnetlink_register_socket(RTMGRP_LINK)) < 0) {
    olsr_syslog(OLSR_LOG_ERR, "rtmonitor socket: %m");
    olsr_exit(__func__, 0);
  }
#endif

  /*
   * create routing socket
   */
#if defined __FreeBSD__ || __FreeBSD_kernel__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  olsr_cnf->rts = socket(PF_ROUTE, SOCK_RAW, 0);
  if (olsr_cnf->rts < 0) {
    olsr_syslog(OLSR_LOG_ERR, "routing socket: %m");
    olsr_exit(__func__, 0);
  }
#endif

#ifdef LINUX_NETLINK_ROUTING
  /* initialize gateway system */
  if (olsr_cnf->smart_gw_active) {
    if (olsr_init_gateways()) {
      olsr_exit("Cannot initialize gateway tunnels", 1);
    }
  }

  /* initialize niit if index */
  if (olsr_cnf->use_niit) {
    olsr_init_niit();
  }
#endif

  /* Init empty TC timer */
  set_empty_tc_timer(GET_TIMESTAMP(0));

  /* enable ip forwarding on host */
  /* Disable redirects globally */
#ifndef WIN32
  net_os_set_global_ifoptions();
#endif

  /* Initialize parser */
  olsr_init_parser();

  /* Initialize route-exporter */
  olsr_init_export_route();

  /* Initialize message sequencnumber */
  init_msg_seqno();

  /* Initialize dynamic willingness calculation */
  olsr_init_willingness();

  /*
   *Set up willingness/APM
   */
  if (olsr_cnf->willingness_auto) {
    if (apm_init() < 0) {
      OLSR_PRINTF(1, "Could not read APM info - setting default willingness(%d)\n", WILL_DEFAULT);

      olsr_syslog(OLSR_LOG_ERR,
          "Could not read APM info - setting default willingness(%d)\n",
          WILL_DEFAULT);

      olsr_cnf->willingness_auto = 0;
      olsr_cnf->willingness = WILL_DEFAULT;
    } else {
      olsr_cnf->willingness = olsr_calculate_willingness();

      OLSR_PRINTF(1, "Willingness set to %d - next update in %.1f secs\n", olsr_cnf->willingness, olsr_cnf->will_int);
    }
  }

  /* Initialize net */
  init_net();

  /* Initializing networkinterfaces */
  if (!olsr_init_interfacedb()) {
    if (olsr_cnf->allow_no_interfaces) {
      fprintf(
          stderr,
          "No interfaces detected! This might be intentional, but it also might mean that your configuration is fubar.\nI will continue after 5 seconds...\n");
      olsr_startup_sleep(5);
    } else {
      fprintf(stderr, "No interfaces detected!\nBailing out!\n");
      olsr_exit(__func__, EXIT_FAILURE);
    }
  }

  olsr_do_startup_sleep();

  /* Print heartbeat to stdout */

#if !defined WINCE
  if (olsr_cnf->debug_level > 0 && isatty(STDOUT_FILENO)) {
    olsr_start_timer(STDOUT_PULSE_INT, 0, OLSR_TIMER_PERIODIC,
        &generate_stdout_pulse, NULL, 0);
  }
#endif

  /* Initialize the IPC socket */

  if (olsr_cnf->ipc_connections > 0) {
    ipc_init();
  }
  /* Initialisation of different tables to be used. */
  olsr_init_tables();

  /* daemon mode */
#ifndef WIN32
  if (olsr_cnf->debug_level == 0 && !olsr_cnf->no_fork) {
    printf("%s detaching from the current process...\n", olsrd_version);
    if (daemon(0, 0) < 0) {
      printf("daemon(3) failed: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }
#endif

  /*
   * Create locking file for olsrd, will be cleared after olsrd exits
   */
  olsr_create_lock_file();

  /* Load plugins */
  olsr_load_plugins();

  OLSR_PRINTF(1, "Main address: %s\n\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));

#ifdef LINUX_NETLINK_ROUTING
  /* create policy routing priorities if necessary */
  if (DEF_RT_NONE != olsr_cnf->rt_table_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version,
        olsr_cnf->rt_table, olsr_cnf->rt_table_pri, NULL, true);
  }
  if (DEF_RT_NONE != olsr_cnf->rt_table_tunnel_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version,
        olsr_cnf->rt_table_tunnel, olsr_cnf->rt_table_tunnel_pri, NULL, true);
  }
  if (DEF_RT_NONE != olsr_cnf->rt_table_default_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version,
        olsr_cnf->rt_table_default, olsr_cnf->rt_table_default_pri, NULL, true);
  }

  /* OLSR sockets */
  if (DEF_RT_NONE != olsr_cnf->rt_table_defaultolsr_pri) {
    for (ifn = ifnet; ifn; ifn = ifn->int_next) {
      olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table_default,
          olsr_cnf->rt_table_defaultolsr_pri, ifn->int_name, true);
    }
  }

  /* trigger gateway selection */
  if (olsr_cnf->smart_gw_active) {
    olsr_trigger_inetgw_startup();
  }

  /* trigger niit static route setup */
  if (olsr_cnf->use_niit) {
    olsr_setup_niit_routes();
  }

  /* create lo:olsr interface */
  if (olsr_cnf->use_src_ip_routes) {
    olsr_os_localhost_if(&olsr_cnf->main_addr, true);
  }
#endif

  /* Start syslog entry */
  olsr_syslog(OLSR_LOG_INFO, "%s successfully started", olsrd_version);

  /*
   *signal-handlers
   */

  /* ctrl-C and friends */
#ifdef WIN32
#ifndef WINCE
  SetConsoleCtrlHandler(SignalHandler, true);
#endif
#else
  signal(SIGHUP, olsr_reconfigure);
  signal(SIGINT, olsr_shutdown);
  signal(SIGQUIT, olsr_shutdown);
  signal(SIGILL, olsr_shutdown);
  signal(SIGABRT, olsr_shutdown);
  //  signal(SIGSEGV, olsr_shutdown);
  signal(SIGTERM, olsr_shutdown);
  signal(SIGPIPE, SIG_IGN);
  // Ignoring SIGUSR1 and SIGUSR1 by default to be able to use them in plugins
  signal(SIGUSR1, SIG_IGN);
  signal(SIGUSR2, SIG_IGN);
#endif

  link_changes = false;

  /* Starting scheduler */
  olsr_scheduler();

  /* Like we're ever going to reach this ;-) */
  return 1;
} /* main */

/**
 * Reconfigure olsrd. Currently kind of a hack...
 *
 *@param signal the signal that triggered this callback
 */
#ifndef WIN32
void olsr_reconfigure(int signo __attribute__ ((unused))) {
  /* if we are started with -nofork, we do not weant to go into the
   * background here. So we can simply stop on -HUP
   */
  olsr_syslog(OLSR_LOG_INFO, "sot: olsr_reconfigure()\n");
  if (!olsr_cnf->no_fork) {
    if (!fork()) {
      int i;
      sigset_t sigs;
      /* New process */
      sleep(3);
      sigemptyset(&sigs);
      sigaddset(&sigs, SIGHUP);
      sigprocmask(SIG_UNBLOCK, &sigs, NULL);
      for (i = sysconf(_SC_OPEN_MAX); --i > STDERR_FILENO;) {
        close(i);
      }
      printf("Restarting %s\n", olsr_argv[0]);
      olsr_syslog(OLSR_LOG_INFO, "Restarting %s\n", olsr_argv[0]);
      execv(olsr_argv[0], olsr_argv);
      olsr_syslog(OLSR_LOG_ERR, "execv(%s) fails: %s!\n", olsr_argv[0],
          strerror(errno));
    } else {
      olsr_syslog(OLSR_LOG_INFO, "RECONFIGURING!\n");
    }
  }
  olsr_shutdown(0);
}
#endif

static void olsr_shutdown_messages(void) {
  struct interface *ifn;

  /* send TC reset */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    /* clean output buffer */
    net_output(ifn);

    /* send 'I'm gone' messages */
    if (olsr_cnf->lq_level > 0) {
      olsr_output_lq_tc(ifn);
      olsr_output_lq_hello(ifn);
    }
    else {
      generate_tc(ifn);
      generate_hello(ifn);
    }
    net_output(ifn);
  }
}

/**
 *Function called at shutdown. Signal handler
 *
 * @param signal the signal that triggered this call
 */
#ifdef WIN32
int __stdcall
SignalHandler(unsigned long signo)
#else
static void olsr_shutdown(int signo __attribute__ ((unused)))
#endif
{
  struct interface *ifn;
  int exit_value;

  OLSR_PRINTF(1, "Received signal %d - shutting down\n", (int)signo);

#ifdef WIN32
  OLSR_PRINTF(1, "Waiting for the scheduler to stop.\n");

  olsr_win32_end_request = TRUE;

  while (!olsr_win32_end_flag)
  Sleep(100);

  OLSR_PRINTF(1, "Scheduler stopped.\n");
#endif

  /* clear all links and send empty hellos/tcs */
  olsr_reset_all_links();

  /* deactivate fisheye and immediate TCs */
  olsr_cnf->lq_fish = 0;
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    ifn->immediate_send_tc = false;
  }
  increase_local_ansn();

  /* send first shutdown message burst */
  olsr_shutdown_messages();

  /* delete all routes */
  olsr_delete_all_kernel_routes();

  /* send second shutdown message burst */
  olsr_shutdown_messages();

  /* now try to cleanup the rest of the mess */
  olsr_delete_all_tc_entries();

  olsr_delete_all_mid_entries();

#ifdef LINUX_NETLINK_ROUTING
  /* trigger gateway selection */
  if (olsr_cnf->smart_gw_active) {
    olsr_cleanup_gateways();
  }

  /* trigger niit static route cleanup */
  if (olsr_cnf->use_niit) {
    olsr_cleanup_niit_routes();
  }

  /* cleanup lo:olsr interface */
  if (olsr_cnf->use_src_ip_routes) {
    olsr_os_localhost_if(&olsr_cnf->main_addr, false);
  }
#endif

  olsr_destroy_parser();

  OLSR_PRINTF(1, "Closing sockets...\n");

  /* front-end IPC socket */
  if (olsr_cnf->ipc_connections > 0) {
    shutdown_ipc();
  }

  /* OLSR sockets */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    close(ifn->olsr_socket);
    close(ifn->send_socket);

#ifdef LINUX_NETLINK_ROUTING
    if (DEF_RT_NONE != olsr_cnf->rt_table_defaultolsr_pri) {
      olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table_default,
          olsr_cnf->rt_table_defaultolsr_pri, ifn->int_name, false);
    }
#endif
  }

  /* Closing plug-ins */
  olsr_close_plugins();

  /* Reset network settings */
  net_os_restore_ifoptions();

  /* ioctl socket */
  close(olsr_cnf->ioctl_s);

#ifdef LINUX_NETLINK_ROUTING
  if (DEF_RT_NONE != olsr_cnf->rt_table_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version,
        olsr_cnf->rt_table, olsr_cnf->rt_table_pri, NULL, false);
  }
  if (DEF_RT_NONE != olsr_cnf->rt_table_tunnel_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version,
        olsr_cnf->rt_table_tunnel, olsr_cnf->rt_table_tunnel_pri, NULL, false);
  }
  if (DEF_RT_NONE != olsr_cnf->rt_table_default_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version,
        olsr_cnf->rt_table_default, olsr_cnf->rt_table_default_pri, NULL, false);
  }
  close(olsr_cnf->rtnl_s);
  close (olsr_cnf->rt_monitor_socket);
#endif

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  /* routing socket */
  close(olsr_cnf->rts);
#endif

  /* Free cookies and memory pools attached. */
  OLSR_PRINTF(0, "Free all memory...\n");
  olsr_delete_all_cookies();

  olsr_syslog(OLSR_LOG_INFO, "%s stopped", olsrd_version);

  OLSR_PRINTF(1, "\n <<<< %s - terminating >>>>\n           http://www.olsr.org\n", olsrd_version);

  exit_value = olsr_cnf->exit_value;
  olsrd_free_cnf(olsr_cnf);

  exit(exit_value);
}

/**
 * Print the command line usage
 */
static void print_usage(bool error) {
  fprintf(
      stderr,
        "%s"
        "usage: olsrd [-f <configfile>] [ -i interface1 interface2 ... ]\n"
        "  [-d <debug_level>] [-ipv6] [-multi <IPv6 multicast address>]\n"
        "  [-lql <LQ level>] [-lqw <LQ winsize>] [-lqnt <nat threshold>]\n"
        "  [-bcast <broadcastaddr>] [-ipc] [-dispin] [-dispout] [-delgw]\n"
        "  [-hint <hello interval (secs)>] [-tcint <tc interval (secs)>]\n"
        "  [-midint <mid interval (secs)>] [-hnaint <hna interval (secs)>]\n"
        "  [-T <Polling Rate (secs)>] [-nofork] [-hemu <ip_address>]\n"
        "  [-lql <LQ level>] [-lqa <LQ aging factor>]\n",
        error ? "An error occured somwhere between your keyboard and your chair!\n" : "");
}

/**
 * Sets the provided configuration on all unconfigured
 * interfaces
 *
 * @param ifs a linked list of interfaces to check and possible update
 * @param cnf the default configuration to set on unconfigured interfaces
 */
int set_default_ifcnfs(struct olsr_if *ifs, struct if_config_options *cnf) {
  int changes = 0;

  while (ifs) {
    if (ifs->cnf == NULL) {
      ifs->cnf = olsr_malloc(sizeof(struct if_config_options),
          "Set default config");
      *ifs->cnf = *cnf;
      changes++;
    }
    ifs = ifs->next;
  }
  return changes;
}

#define NEXT_ARG do { argv++;argc--; } while (0)
#define CHECK_ARGC do { if(!argc) { \
      if((argc - 1) == 1){ \
      fprintf(stderr, "Error parsing command line options!\n"); \
      } else { \
      argv--; \
      fprintf(stderr, "You must provide a parameter when using the %s switch!\n", *argv); \
     } \
     olsr_exit(__func__, EXIT_FAILURE); \
     } } while (0)

/**
 * Process command line arguments passed to olsrd
 *
 */
static int olsr_process_arguments(int argc, char *argv[],
    struct olsrd_config *cnf, struct if_config_options *ifcnf) {
  while (argc > 1) {
    NEXT_ARG;
#ifdef WIN32
    /*
     *Interface list
     */
    if (strcmp(*argv, "-int") == 0) {
      ListInterfaces();
      exit(0);
    }
#endif

    /*
     *Configfilename
     */
    if (strcmp(*argv, "-f") == 0) {
      fprintf(stderr, "Configfilename must ALWAYS be first argument!\n\n");
      olsr_exit(__func__, EXIT_FAILURE);
    }

    /*
     *Use IP version 6
     */
    if (strcmp(*argv, "-ipv6") == 0) {
      cnf->ip_version = AF_INET6;
      continue;
    }

    /*
     *Broadcast address
     */
    if (strcmp(*argv, "-bcast") == 0) {
      struct in_addr in;
      NEXT_ARG;
      CHECK_ARGC;

      if (inet_aton(*argv, &in) == 0) {
        printf("Invalid broadcast address! %s\nSkipping it!\n", *argv);
        continue;
      }
      memcpy(&ifcnf->ipv4_multicast.v4, &in.s_addr, sizeof(ifcnf->ipv4_multicast.v4));
      continue;
    }

    /*
     * Set LQ level
     */
    if (strcmp(*argv, "-lql") == 0) {
      int tmp_lq_level;
      NEXT_ARG;
      CHECK_ARGC;

      /* Sanity checking is done later */
      sscanf(*argv, "%d", &tmp_lq_level);
      olsr_cnf->lq_level = tmp_lq_level;
      continue;
    }

    /*
     * Set LQ winsize
     */
    if (strcmp(*argv, "-lqa") == 0) {
      float tmp_lq_aging;
      NEXT_ARG;
      CHECK_ARGC;

      sscanf(*argv, "%f", &tmp_lq_aging);

      if (tmp_lq_aging < MIN_LQ_AGING || tmp_lq_aging > MAX_LQ_AGING) {
        printf("LQ aging factor %f not allowed. Range [%f-%f]\n", tmp_lq_aging,
            MIN_LQ_AGING, MAX_LQ_AGING);
        olsr_exit(__func__, EXIT_FAILURE);
      }
      olsr_cnf->lq_aging = tmp_lq_aging;
      continue;
    }

    /*
     * Set NAT threshold
     */
    if (strcmp(*argv, "-lqnt") == 0) {
      float tmp_lq_nat_thresh;
      NEXT_ARG;
      CHECK_ARGC;

      sscanf(*argv, "%f", &tmp_lq_nat_thresh);

      if (tmp_lq_nat_thresh < 0.1 || tmp_lq_nat_thresh > 1.0) {
        printf("NAT threshold %f not allowed. Range [%f-%f]\n",
            tmp_lq_nat_thresh, 0.1, 1.0);
        olsr_exit(__func__, EXIT_FAILURE);
      }
      olsr_cnf->lq_nat_thresh = tmp_lq_nat_thresh;
      continue;
    }

    /*
     * Enable additional debugging information to be logged.
     */
    if (strcmp(*argv, "-d") == 0) {
      NEXT_ARG;
      CHECK_ARGC;

      sscanf(*argv, "%d", &cnf->debug_level);
      continue;
    }

    /*
     * Interfaces to be used by olsrd.
     */
    if (strcmp(*argv, "-i") == 0) {
      NEXT_ARG;
      CHECK_ARGC;

      if (*argv[0] == '-') {
        fprintf(stderr, "You must provide an interface label!\n");
        olsr_exit(__func__, EXIT_FAILURE);
      }
      printf("Queuing if %s\n", *argv);
      olsr_create_olsrif(*argv, false);

      while ((argc - 1) && (argv[1][0] != '-')) {
        NEXT_ARG;
        printf("Queuing if %s\n", *argv);
        olsr_create_olsrif(*argv, false);
      }

      continue;
    }
    /*
     * Set the hello interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-hint") == 0) {
      NEXT_ARG;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->hello_params.emission_interval);
      ifcnf->hello_params.validity_time = ifcnf->hello_params.emission_interval
          * 3;
      continue;
    }

    /*
     * Set the HNA interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-hnaint") == 0) {
      NEXT_ARG;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->hna_params.emission_interval);
      ifcnf->hna_params.validity_time = ifcnf->hna_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the MID interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-midint") == 0) {
      NEXT_ARG;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->mid_params.emission_interval);
      ifcnf->mid_params.validity_time = ifcnf->mid_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the tc interval to be used by olsrd.
     *
     */
    if (strcmp(*argv, "-tcint") == 0) {
      NEXT_ARG;
      CHECK_ARGC;
      sscanf(*argv, "%f", &ifcnf->tc_params.emission_interval);
      ifcnf->tc_params.validity_time = ifcnf->tc_params.emission_interval * 3;
      continue;
    }

    /*
     * Set the polling interval to be used by olsrd.
     */
    if (strcmp(*argv, "-T") == 0) {
      NEXT_ARG;
      CHECK_ARGC;
      sscanf(*argv, "%f", &cnf->pollrate);
      continue;
    }

    /*
     * Should we display the contents of packages beeing sent?
     */
    if (strcmp(*argv, "-dispin") == 0) {
      parser_set_disp_pack_in(true);
      continue;
    }

    /*
     * Should we display the contents of incoming packages?
     */
    if (strcmp(*argv, "-dispout") == 0) {
      net_set_disp_pack_out(true);
      continue;
    }

    /*
     * Should we set up and send on a IPC socket for the front-end?
     */
    if (strcmp(*argv, "-ipc") == 0) {
      cnf->ipc_connections = 1;
      continue;
    }

    /*
     * IPv6 multicast addr
     */
    if (strcmp(*argv, "-multi") == 0) {
      struct in6_addr in6;
      NEXT_ARG;
      CHECK_ARGC;
      if (inet_pton(AF_INET6, *argv, &in6) <= 0) {
        fprintf(stderr, "Failed converting IP address %s\n", *argv);
        exit(EXIT_FAILURE);
      }

      memcpy(&ifcnf->ipv6_multicast, &in6, sizeof(struct in6_addr));

      continue;
    }

    /*
     * Host emulation
     */
    if (strcmp(*argv, "-hemu") == 0) {
      struct in_addr in;
      struct olsr_if *ifa;

      NEXT_ARG;
      CHECK_ARGC;
      if (inet_pton(AF_INET, *argv, &in) <= 0) {
        fprintf(stderr, "Failed converting IP address %s\n", *argv);
        exit(EXIT_FAILURE);
      }
      /* Add hemu interface */

      ifa = olsr_create_olsrif("hcif01", true);

      if (!ifa)
        continue;

      ifa->cnf = get_default_if_config();
      ifa->host_emul = true;
      memcpy(&ifa->hemu_ip, &in, sizeof(union olsr_ip_addr));
      cnf->host_emul = true;

      continue;
    }

    /*
     * Delete possible default GWs
     */
    if (strcmp(*argv, "-delgw") == 0) {
      olsr_cnf->del_gws = true;
      continue;
    }

    if (strcmp(*argv, "-nofork") == 0) {
      cnf->no_fork = true;
      continue;
    }

    return -1;
  }
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

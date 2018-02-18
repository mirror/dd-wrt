/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
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

#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>

#include "cfgparser/olsrd_conf.h"
#include "cfgparser/olsrd_conf_checksum.h"
#include "ipcalc.h"
#include "defs.h"
#include "builddata.h"
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
#include "olsr_random.h"
#include "pid_file.h"
#include "lock_file.h"
#include "cli.h"

#if defined(__GLIBC__) && defined(__linux__) && !defined(__ANDROID__) && !defined(__UCLIBC__)
  #define OLSR_HAVE_EXECINFO_H
#endif

#ifdef OLSR_HAVE_EXECINFO_H
  #include <execinfo.h>
#endif /* OLSR_HAVE_EXECINFO_H */

#ifdef __linux__
#include <linux/types.h>
#include <linux/rtnetlink.h>
#include "kernel_routes.h"

#endif /* __linux__ */

#ifdef _WIN32
#include <process.h>
#include <winbase.h>
#define olsr_shutdown(x) SignalHandler(x)
#define close(x) closesocket(x)
int __stdcall SignalHandler(unsigned long signo) __attribute__ ((noreturn));
void DisableIcmpRedirects(void);
#endif /* _WIN32 */

struct timer_entry * heartBeatTimer = NULL;

static char **olsr_argv = NULL;

struct olsr_cookie_info *def_timer_ci = NULL;

static void printStacktrace(const char * message) {
#ifdef OLSR_HAVE_EXECINFO_H
  void *bt_array[64];
  size_t bt_size;
  size_t bt_index=0;
  char ** bt_syms;

  bt_size = backtrace(bt_array, 64);
  bt_syms = backtrace_symbols(bt_array, bt_size);

  olsr_syslog(OLSR_LOG_ERR, "%s", message);
  while (bt_index < bt_size) {
    olsr_syslog(OLSR_LOG_ERR, "%s", bt_syms[bt_index++]);
  }
#else
  olsr_syslog(OLSR_LOG_ERR, "%s (logging a stack trace is not supported on this platform)", message);
#endif /* OLSR_HAVE_EXECINFO_H */
}

#ifndef _WIN32
/**
 * Reconfigure olsrd. Currently kind of a hack...
 *
 *@param signo the signal that triggered this callback
 */
__attribute__ ((noreturn))
static void olsr_reconfigure(int signo __attribute__ ((unused))) {
#ifndef _WIN32
  int errNr = errno;
#endif
  /* if we are started with -nofork, we do not want to go into the
   * background here. So we can simply stop on -HUP
   */
  olsr_syslog(OLSR_LOG_INFO, "sot: olsr_reconfigure()\n");
  if (!olsr_cnf->no_fork) {
    if (!fork()) {
      int i;
      sigset_t sigs;

      /* New process, wait a bit to let the old process exit */
      sleep(3);
      sigemptyset(&sigs);
      sigaddset(&sigs, SIGHUP);
      sigprocmask(SIG_UNBLOCK, &sigs, NULL);
      for (i = sysconf(_SC_OPEN_MAX); --i > STDERR_FILENO;) {
        close(i);
      }
      printf("Restarting %s", olsr_argv[0]);
      olsr_syslog(OLSR_LOG_INFO, "Restarting %s", olsr_argv[0]);
      execv(olsr_argv[0], olsr_argv);
      olsr_syslog(OLSR_LOG_ERR, "execv(%s) failed: %s", olsr_argv[0], strerror(errno));
    } else {
      olsr_syslog(OLSR_LOG_INFO, "RECONFIGURING");
    }
  }
#ifndef _WIN32
  errno = errNr;
#endif
  olsr_exit(NULL, EXIT_SUCCESS);
}
#endif /* _WIN32 */

static void olsr_shutdown_messages(void) {
  struct interface_olsr *ifn;

  /* send TC reset */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    /* clean output buffer */
    net_output(ifn);

    /* send 'I'm gone' messages */
    if (olsr_cnf->lq_level > 0) {
      olsr_output_lq_tc(ifn);
      olsr_output_lq_hello(ifn);
    } else {
      generate_tc(ifn);
      generate_hello(ifn);
    }
    net_output(ifn);
  }
}

/**
 *Function called at shutdown. Signal handler
 *
 * @param signo the signal that triggered this call
 */
__attribute__ ((noreturn))
#ifdef _WIN32
int __stdcall
SignalHandler(unsigned long signo)
#else /* _WIN32 */
static void olsr_shutdown(int signo __attribute__ ((unused)))
#endif /* _WIN32 */
{
  static volatile bool inShutdown = false;
#ifndef _WIN32
  int errNr = errno;
#endif
  struct interface_olsr *ifn;
  int exit_value;

#ifdef __linux__
  OLSR_PRINTF(1, "Received signal %s - shutting down\n", strsignal(signo));
#else
  OLSR_PRINTF(1, "Received signal %d - shutting down\n", (int)signo);
#endif

  if (inShutdown) {
    /* already in shutdown, do not allow nested shutdown */
    printStacktrace("nested shutdown: exiting immediately");
    exit(olsr_cnf->exit_value);
  }
  inShutdown = true;

  /* instruct the scheduler to stop */
  olsr_scheduler_stop();

#ifdef __linux__
  if (olsr_cnf->smart_gw_active) {
    olsr_shutdown_gateways();
  }
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

#ifdef __linux__
  if (olsr_cnf->smart_gw_active) {
    olsr_cleanup_gateways();
  }
#endif

#ifdef __linux__
  /* trigger niit static route cleanup */
  if (olsr_cnf->use_niit) {
    olsr_cleanup_niit_routes();
  }

  /* cleanup lo:olsr interface */
  if (olsr_cnf->use_src_ip_routes) {
    olsr_os_localhost_if(&olsr_cnf->main_addr, false);
  }
#endif /* __linux__ */

  olsr_destroy_parser();

  olsrd_cfgfile_cleanup();

  OLSR_PRINTF(1, "Closing sockets...\n");

  /* front-end IPC socket */
  if (olsr_cnf->ipc_connections > 0) {
    shutdown_ipc();
  }

  /* OLSR sockets */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    close(ifn->olsr_socket);
    close(ifn->send_socket);

#ifdef __linux__
    if (DEF_RT_NONE != olsr_cnf->rt_table_defaultolsr_pri) {
      olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table_default,
          olsr_cnf->rt_table_defaultolsr_pri, ifn->int_name, false);
    }
#endif /* __linux__ */
  }

  /* stop and cleanup any remaining timers */
  olsr_flush_timers();

  /* Closing plug-ins */
  olsr_close_plugins();

  /* Reset network settings */
  net_os_restore_ifoptions();

  /* ioctl socket */
  close(olsr_cnf->ioctl_s);

#ifdef __linux__
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
#endif /* __linux__ */

#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__
  /* routing socket */
  close(olsr_cnf->rts);
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__ */

  /* remove the lock file */
  olsr_remove_lock_file();

  /* stop heartbeat that is showing on stdout */
#if !defined WINCE
  if (heartBeatTimer) {
    olsr_stop_timer(heartBeatTimer);
    heartBeatTimer = NULL;
  }
#endif /* !defined WINCE */

  /* Free cookies and memory pools attached. */
  OLSR_PRINTF(0, "Free all memory...\n");
  olsr_delete_all_cookies();

  olsr_syslog(OLSR_LOG_INFO, "%s stopped", olsrd_version);

  OLSR_PRINTF(1, "\n <<<< %s - terminating >>>>\n           http://www.olsr.org\n", olsrd_version);

  exit_value = olsr_cnf->exit_value;
  olsrd_free_cnf(&olsr_cnf);

  /* close the log */
  olsr_closelog();

#ifndef _WIN32
  errno = errNr;
#endif
  exit(exit_value);
}

#if defined(__linux__) && !defined(__ANDROID__)
__attribute__((noreturn))
static void olsr_segv_handler(int sig) {
  static bool in_segv = false;

  if (!in_segv) {
    in_segv = true;
    printStacktrace("crash");
    olsr_shutdown(sig);

    /* safety net */
    exit(123);
  }

  printStacktrace("nested crash: exiting immediately");
  exit(olsr_cnf->exit_value);
}
#endif /* defined(__linux__) && !defined(__ANDROID__) */

/**
 * Sets the provided configuration on all unconfigured
 * interfaces
 *
 * @param ifs a linked list of interfaces to check and possible update
 * @param cnf the default configuration to set on unconfigured interfaces
 */
static int set_default_ifcnfs(struct olsr_if *ifs, struct if_config_options *cnf) {
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

/**
 * Main entrypoint
 */

static int argc_saved = 0;
char ** argv_saved = NULL;

void get_argc_argv(int *argc, char ***argv) {
  if (argc) {
    *argc = argc_saved;
  }
  if (argv) {
    *argv = argv_saved;
  }
}

int main(int argc, char *argv[]) {
  int argcLocal = argc;

  /* save argc and argv */
  {
    size_t i;

    argc_saved = argc;
    argv_saved = malloc(sizeof(char*) * (argc + 1));
    for (i = 0; i <= (size_t) argc; i++) {
      argv_saved[i] = argv[i];
    }
  }

  /* Open syslog */
  olsr_openlog("olsrd");

  olsrd_config_checksum_init();
  olsrd_config_checksum_add_cli(argc, argv);
  olsrd_config_checksum_add(OLSRD_CONFIG_START, OLSRD_CONFIG_START_LEN);

  /*
   * Initialisation
   */

  /* setup debug printf destination */
  debug_handle = stdout;

  /* set stdout and stderr to unbuffered output */
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  /* setup random seed */
  srandom(time(NULL));

  /* Init widely used statics */
  memset(&all_zero, 0, sizeof(union olsr_ip_addr));

  /* store the arguments for restart */
  olsr_argv = argv;

  /*
   * Start
   */

  print_version();

  if (argcLocal == 2) {
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "/?")) {
      /* help */
      print_usage(false);
      olsr_exit(NULL, EXIT_SUCCESS);
    }
    if (!strcmp(argv[1], "-v")) {
      /* version */
      olsr_exit(NULL, EXIT_SUCCESS);
    }
  }

  /* Check root Privileges */
#ifndef _WIN32
  if (geteuid()) {
    olsr_exit("You must be root (uid = 0) to run olsrd", EXIT_FAILURE);
  }
#else /* _WIN32 */
  DisableIcmpRedirects();

  {
    WSADATA WsaData;
    if (WSAStartup(0x0202, &WsaData)) {
      char buf2[1024];
      snprintf(buf2, sizeof(buf2), "%s: Could not initialise WinSock", __func__);
      olsr_exit(buf2, EXIT_FAILURE);
    }
  }
#endif /* _WIN32 */

  /* load the configuration */
  if (!loadConfig(&argcLocal, argv)) {
    olsr_exit(NULL, EXIT_FAILURE);
  }

  /* Process CLI Arguments */
  {
    char * error = NULL;
    int exitCode = EXIT_FAILURE;

    /* get the default interface config */
    struct if_config_options *default_ifcnf = get_default_if_config();
    if (!default_ifcnf) {
      olsr_exit("No default ifconfig found", EXIT_FAILURE);
    }

    /* Process olsrd options */
    exitCode = olsr_process_arguments(argcLocal, argv, olsr_cnf, default_ifcnf, &error);
    if (exitCode != 0) {
      free(default_ifcnf);
      olsr_exit(error, EXIT_FAILURE);
    }

    /* Set configuration for command-line specified interfaces */
    set_default_ifcnfs(olsr_cnf->interfaces, default_ifcnf);

    /* free the default ifcnf */
    free(default_ifcnf);
  }

  olsrd_config_checksum_add(OLSRD_CONFIG_END, OLSRD_CONFIG_END_LEN);
  olsrd_config_checksum_final();

  /* Sanity check configuration */
  if (olsrd_sanity_check_cnf(olsr_cnf) < 0) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "%s: Bad configuration", __func__);
    olsr_exit(buf2, EXIT_FAILURE);
  }

  /* Setup derived configuration */
  set_derived_cnf(olsr_cnf);

  olsrd_cfgfile_init();

  /* Print configuration */
  if (olsr_cnf->debug_level > 1) {
    olsrd_print_cnf(olsr_cnf);
  }

  /* configuration loaded and sane */

  /* initialise timers */
  olsr_init_timers();

  def_timer_ci = olsr_alloc_cookie("Default Timer Cookie", OLSR_COOKIE_TYPE_TIMER);

  /* create a socket for ioctl calls */
  olsr_cnf->ioctl_s = socket(olsr_cnf->ip_version, SOCK_DGRAM, 0);
  if (olsr_cnf->ioctl_s < 0) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "ioctl socket: %s", strerror(errno));
    olsr_exit(buf2, EXIT_FAILURE);
  }

  /* create a socket for netlink calls */
#ifdef __linux__
  olsr_cnf->rtnl_s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
  if (olsr_cnf->rtnl_s < 0) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "rtnetlink socket: %s", strerror(errno));
    olsr_exit(buf2, EXIT_FAILURE);
  }

  if (fcntl(olsr_cnf->rtnl_s, F_SETFL, O_NONBLOCK)) {
    olsr_syslog(OLSR_LOG_INFO, "rtnetlink could not be set to nonblocking");
  }

  if ((olsr_cnf->rt_monitor_socket = rtnetlink_register_socket(RTMGRP_LINK)) < 0) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "rtmonitor socket: %s", strerror(errno));
    olsr_exit(buf2, EXIT_FAILURE);
  }
#endif /* __linux__ */

  /* create routing socket */
#if defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__
  olsr_cnf->rts = socket(PF_ROUTE, SOCK_RAW, 0);
  if (olsr_cnf->rts < 0) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "routing socket: %s", strerror(errno));
    olsr_exit(buf2, EXIT_FAILURE);
  }
#endif /* defined __FreeBSD__ || defined __FreeBSD_kernel__ || defined __APPLE__ || defined __NetBSD__ || defined __OpenBSD__ */

  /* initialise gateway system */
#ifdef __linux__
  if (olsr_cnf->smart_gw_active && olsr_init_gateways()) {
    olsr_exit("Cannot initialise gateway tunnels", EXIT_FAILURE);
  }
#endif /* __linux__ */

  /* initialise niit if index */
#ifdef __linux__
  if (olsr_cnf->use_niit) {
    olsr_init_niit();
  }
#endif /* __linux__ */

  /* initialise empty TC timer */
  set_empty_tc_timer(GET_TIMESTAMP(0));

  /* enable ip forwarding on host and disable redirects globally (not for WIN32) */
  net_os_set_global_ifoptions();

  /* initialise parser */
  olsr_init_parser();

  /* initialise route exporter */
  olsr_init_export_route();

  /* initialise message sequence number */
  init_msg_seqno();

  /* initialise dynamic willingness calculation */
  olsr_init_willingness();

  /* Set up willingness/APM */
  if (olsr_cnf->willingness_auto) {
    if (apm_init() < 0) {
      OLSR_PRINTF(1, "Could not read APM info - setting default willingness(%d)\n", WILL_DEFAULT);

      olsr_syslog(OLSR_LOG_ERR, "Could not read APM info - setting default willingness(%d)\n",
      WILL_DEFAULT);

      olsr_cnf->willingness_auto = 0;
      olsr_cnf->willingness = WILL_DEFAULT;
    } else {
      olsr_cnf->willingness = olsr_calculate_willingness();

      OLSR_PRINTF(1, "Willingness set to %d - next update in %.1f secs\n", olsr_cnf->willingness, (double )olsr_cnf->will_int);
    }
  }

  /* initialise net */
  init_net();

  /* initialise network interfaces */
  if (!olsr_init_interfacedb()) {
    if (olsr_cnf->allow_no_interfaces) {
      fprintf( stderr, "No interfaces detected! This might be intentional, but it also might mean"
          " that your configuration is fubar.\nI will continue after 5 seconds...\n");
      olsr_startup_sleep(5);
    } else {
      char buf2[1024];
      snprintf(buf2, sizeof(buf2), "%s: No interfaces detected", __func__);
      olsr_exit(buf2, EXIT_FAILURE);
    }
  }

  /* initialise the IPC socket */
  if ((olsr_cnf->ipc_connections > 0) && ipc_init()) {
    olsr_exit("ipc_init failure", EXIT_FAILURE);
  }

  /* Initialisation of different tables to be used. */
  olsr_init_tables();

#ifdef __linux__
  /* startup gateway system */
  if (olsr_cnf->smart_gw_active && olsr_startup_gateways()) {
    olsr_exit("Cannot startup gateway tunnels", EXIT_FAILURE);
  }
#endif /* __linux__ */

  olsr_do_startup_sleep();

  /* start heartbeat that is showing on stdout */
#if !defined WINCE
  if ((olsr_cnf->debug_level > 0) && isatty(STDOUT_FILENO)) {
    heartBeatTimer = olsr_start_timer(STDOUT_PULSE_INT, 0, OLSR_TIMER_PERIODIC, &generate_stdout_pulse, NULL, 0);
  }
#endif /* !defined WINCE */

  /* daemon mode */
#ifndef _WIN32
  if ((olsr_cnf->debug_level == 0) && !olsr_cnf->no_fork) {
    printf("%s detaching from the current process...\n", olsrd_version);
    if (daemon(0, 0) < 0) {
      char buf2[1024];
      snprintf(buf2, sizeof(buf2), "daemon(3) failed: %s", strerror(errno));
      olsr_exit(buf2, EXIT_FAILURE);
    }
  }
#endif /* _WIN32 */

  if (!writePidFile()) {
    olsr_exit(NULL, EXIT_FAILURE);
  }

  /* Create locking file for olsrd, will be cleared after olsrd exits */
  if (!olsr_create_lock_file()) {
    char buf2[1024];
    snprintf(buf2, sizeof(buf2), "Error, cannot create OLSR lock file '%s'", olsr_cnf->lock_file);
    olsr_exit(buf2, EXIT_FAILURE);
  }

  /* Load plugins */
  olsr_load_plugins();

  /* print the main address */
  {
    struct ipaddr_str buf;
    OLSR_PRINTF(1, "Main address: %s\n\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));
  }

  /* create policy routing rules with priorities if necessary */
#ifdef __linux__
  if (DEF_RT_NONE != olsr_cnf->rt_table_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table, olsr_cnf->rt_table_pri, NULL, true);
  }

  if (DEF_RT_NONE != olsr_cnf->rt_table_tunnel_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table_tunnel, olsr_cnf->rt_table_tunnel_pri, NULL, true);
  }

  if (DEF_RT_NONE != olsr_cnf->rt_table_default_pri) {
    olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table_default, olsr_cnf->rt_table_default_pri, NULL, true);
  }

  /* rule to default table on all olsrd interfaces */
  if (DEF_RT_NONE != olsr_cnf->rt_table_defaultolsr_pri) {
    struct interface_olsr *ifn;
    for (ifn = ifnet; ifn; ifn = ifn->int_next) {
      olsr_os_policy_rule(olsr_cnf->ip_version, olsr_cnf->rt_table_default, olsr_cnf->rt_table_defaultolsr_pri, ifn->int_name, true);
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
#endif /* __linux__ */

  /* Start syslog entry */
  olsr_syslog(OLSR_LOG_INFO, "%s successfully started", olsrd_version);

  /* setup signal-handlers */
#ifdef _WIN32
#ifndef WINCE
  SetConsoleCtrlHandler(SignalHandler, true);
#endif /* WINCE */
#else /* _WIN32 */
  signal(SIGHUP, olsr_reconfigure);
  signal(SIGINT, olsr_shutdown);
  signal(SIGQUIT, olsr_shutdown);
  signal(SIGILL, olsr_shutdown);
  signal(SIGABRT, olsr_shutdown);
#if defined(__linux__) && !defined(__ANDROID__)
  signal(SIGSEGV, olsr_segv_handler);
#endif  /* defined(__linux__) && !defined(__ANDROID__) */
  signal(SIGTERM, olsr_shutdown);
  signal(SIGPIPE, SIG_IGN);
  // Ignoring SIGUSR1 and SIGUSR1 by default to be able to use them in plugins
  signal(SIGUSR1, SIG_IGN);
  signal(SIGUSR2, SIG_IGN);
#endif /* _WIN32 */

  /* Starting scheduler */
  olsr_scheduler();

  /* We'll only get here when olsr_shutdown has stopped the scheduler */
  sleep(30);

  exit(EXIT_FAILURE);
} /* main */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */


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

#include "ipcalc.h"
#include "defs.h"
#include "olsr.h"
#include "log.h"
#include "scheduler.h"
#include "parser.h"
#include "generate_msg.h"
#include "plugin_loader.h"
#include "socket_parser.h"
#include "apm.h"
#include "net_os.h"
#include "build_msg.h"
#include "net_olsr.h"

#if LINUX_POLICY_ROUTING
#include <linux/types.h>
#include <linux/rtnetlink.h>
#include <fcntl.h>
#include "kernel_routes.h"
#endif

#ifdef WIN32
#define close(x) closesocket(x)
int __stdcall SignalHandler(unsigned long signal) __attribute__ ((noreturn));
void ListInterfaces(void);
void DisableIcmpRedirects(void);
bool olsr_win32_end_request = false;
bool olsr_win32_end_flag = false;
#else
static void olsr_shutdown(int) __attribute__ ((noreturn));
#endif

/*
 * Local function prototypes
 */
void olsr_reconfigure(int) __attribute__ ((noreturn));

static void print_usage(void);

static int set_default_ifcnfs(struct olsr_if *, struct if_config_options *);

static int olsr_process_arguments(int, char *[], struct olsrd_config *, struct if_config_options *);

#ifndef WIN32
static char **olsr_argv;
#endif

static char copyright_string[] __attribute__ ((unused)) =
  "The olsr.org Optimized Link-State Routing daemon(olsrd) Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org) All rights reserved.";

/**
 * Main entrypoint
 */

int
main(int argc, char *argv[])
{
  struct if_config_options *default_ifcnf;
  char conf_file_name[FILENAME_MAX];
  struct ipaddr_str buf;
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

  /* Grab initial timestamp */
  now_times = olsr_times();
  if ((clock_t)-1 == now_times) {
    const char * const err_msg = strerror(errno);
    olsr_syslog(OLSR_LOG_ERR, "Error in times(): %s, sleeping for a second", err_msg);
    OLSR_PRINTF(1, "Error in times(): %s, sleeping for a second", err_msg);
    sleep(1);
    now_times = olsr_times();
    if ((clock_t)-1 == now_times) {
      olsr_syslog(OLSR_LOG_ERR, "Shutting down because times() does not work");
      fprintf(stderr, "Shutting down because times() does not work\n");
      exit(EXIT_FAILURE);
    }
  }

  printf("\n *** %s ***\n Build date: %s on %s\n http://www.olsr.org\n\n", olsrd_version, build_date, build_host);

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

  if ((argc > 1) && (strcmp(argv[1], "-f") == 0)) {
    struct stat statbuf;

    argv++;
    argc--;
    if (argc == 1) {
      fprintf(stderr, "You must provide a filename when using the -f switch!\n");
      exit(EXIT_FAILURE);
    }

    if (stat(argv[1], &statbuf) < 0) {
      fprintf(stderr, "Could not find specified config file %s!\n%s\n\n", argv[1], strerror(errno));
      exit(EXIT_FAILURE);
    }

    strscpy(conf_file_name, argv[1], sizeof(conf_file_name));
    argv++;
    argc--;

  }

  /*
   * set up configuration prior to processing commandline options
   */
  if (NULL == (olsr_cnf = olsrd_parse_cnf(conf_file_name))) {
    printf("Using default config values(no configfile)\n");
    olsr_cnf = olsrd_get_default_cnf();
  }

  default_ifcnf = get_default_if_config();
  if (default_ifcnf == NULL) {
    fprintf(stderr, "No default ifconfig found!\n");
    exit(EXIT_FAILURE);
  }

  /* Initialize tick resolution */
#ifndef WIN32
  olsr_cnf->system_tick_divider = 1000 / sysconf(_SC_CLK_TCK);
#else
  olsr_cnf->system_tick_divider = 1;
#endif

  /* Initialize timers */
  olsr_init_timers();

  /*
   * Process olsrd options.
   */
  if (olsr_process_arguments(argc, argv, olsr_cnf, default_ifcnf) < 0) {
    print_usage();
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
   * Print configuration
   */
  if (olsr_cnf->debug_level > 1) {
    olsrd_print_cnf(olsr_cnf);
  }
#ifndef WIN32
  /* Disable redirects globally */
  disable_redirects_global(olsr_cnf->ip_version);
#endif

  /*
   * socket for ioctl calls
   */
  olsr_cnf->ioctl_s = socket(olsr_cnf->ip_version, SOCK_DGRAM, 0);
  if (olsr_cnf->ioctl_s < 0) {
    olsr_syslog(OLSR_LOG_ERR, "ioctl socket: %m");
    olsr_exit(__func__, 0);
  }
#if LINUX_POLICY_ROUTING
  olsr_cnf->rtnl_s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
  if (olsr_cnf->rtnl_s < 0) {
    olsr_syslog(OLSR_LOG_ERR, "rtnetlink socket: %m");
    olsr_exit(__func__, 0);
  }
  fcntl(olsr_cnf->rtnl_s, F_SETFL, O_NONBLOCK);
#endif

/*
 * create routing socket
 */
#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  olsr_cnf->rts = socket(PF_ROUTE, SOCK_RAW, 0);
  if (olsr_cnf->rts < 0) {
    olsr_syslog(OLSR_LOG_ERR, "routing socket: %m");
    olsr_exit(__func__, 0);
  }
#endif

  /* Init empty TC timer */
  set_empty_tc_timer(GET_TIMESTAMP(0));

  /*
   *enable ip forwarding on host
   */
  enable_ip_forwarding(olsr_cnf->ip_version);

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

      olsr_syslog(OLSR_LOG_ERR, "Could not read APM info - setting default willingness(%d)\n", WILL_DEFAULT);

      olsr_cnf->willingness_auto = 0;
      olsr_cnf->willingness = WILL_DEFAULT;
    } else {
      olsr_cnf->willingness = olsr_calculate_willingness();

      OLSR_PRINTF(1, "Willingness set to %d - next update in %.1f secs\n", olsr_cnf->willingness, olsr_cnf->will_int);
    }
  }

  /* Initialize net */
  init_net();

#if LINUX_POLICY_ROUTING
  /* Create rule for RtTable to resolve route insertion problems*/
  if ( ( olsr_cnf->rttable < 253) & ( olsr_cnf->rttable > 0 ) ) {
    olsr_netlink_rule(olsr_cnf->ip_version, olsr_cnf->rttable, RTM_NEWRULE);
  }
#endif

  /* Initializing networkinterfaces */
  if (!ifinit()) {
    if (olsr_cnf->allow_no_interfaces) {
      fprintf(stderr,
              "No interfaces detected! This might be intentional, but it also might mean that your configuration is fubar.\nI will continue after 5 seconds...\n");
      sleep(5);
    } else {
      fprintf(stderr, "No interfaces detected!\nBailing out!\n");
      olsr_exit(__func__, EXIT_FAILURE);
    }
  }

  /* Print heartbeat to stdout */

#if !defined WINCE
  if (olsr_cnf->debug_level > 0 && isatty(STDOUT_FILENO)) {
    olsr_start_timer(STDOUT_PULSE_INT, 0, OLSR_TIMER_PERIODIC, &generate_stdout_pulse, NULL, 0);
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

  /* Load plugins */
  olsr_load_plugins();

  OLSR_PRINTF(1, "Main address: %s\n\n", olsr_ip_to_string(&buf, &olsr_cnf->main_addr));

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
#endif

  link_changes = false;

  /* Starting scheduler */
  olsr_scheduler();

  /* Like we're ever going to reach this ;-) */
  return 1;
}                               /* main */

/**
 * Reconfigure olsrd. Currently kind of a hack...
 *
 *@param signal the signal that triggered this callback
 */
#ifndef WIN32
void
olsr_reconfigure(int signal __attribute__ ((unused)))
{
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
      for (i = sysconf(_SC_OPEN_MAX); --i > STDERR_FILENO; ) {
        close(i);
      }
      printf("Restarting %s\n", olsr_argv[0]);
      olsr_syslog(OLSR_LOG_INFO, "Restarting %s\n", olsr_argv[0]);
      execv(olsr_argv[0], olsr_argv);
      olsr_syslog(OLSR_LOG_ERR, "execv(%s) fails: %s!\n", olsr_argv[0], strerror(errno));
    }
    else {
      olsr_syslog(OLSR_LOG_INFO, "RECONFIGURING!\n");
    }
  }
  olsr_shutdown(0);
}
#endif

/**
 *Function called at shutdown. Signal handler
 *
 * @param signal the signal that triggered this call
 */
#ifdef WIN32
int __stdcall
SignalHandler(unsigned long signal)
#else
static void
olsr_shutdown(int signal __attribute__ ((unused)))
#endif
{
  struct interface *ifn;

  OLSR_PRINTF(1, "Received signal %d - shutting down\n", (int)signal);

#ifdef WIN32
  OLSR_PRINTF(1, "Waiting for the scheduler to stop.\n");

  olsr_win32_end_request = TRUE;

  while (!olsr_win32_end_flag)
    Sleep(100);

  OLSR_PRINTF(1, "Scheduler stopped.\n");
#endif

  olsr_delete_all_kernel_routes();

  OLSR_PRINTF(1, "Closing sockets...\n");

  /* front-end IPC socket */
  if (olsr_cnf->ipc_connections > 0) {
    shutdown_ipc();
  }

  /* OLSR sockets */
  for (ifn = ifnet; ifn; ifn = ifn->int_next)
    close(ifn->olsr_socket);

  /* Closing plug-ins */
  olsr_close_plugins();

  /* Reset network settings */
  restore_settings(olsr_cnf->ip_version);

  /* ioctl socket */
  close(olsr_cnf->ioctl_s);

#if LINUX_POLICY_ROUTING
  /* RtTable (linux only!!) */
  if ( ( olsr_cnf->rttable < 253) & ( olsr_cnf->rttable > 0 ) ) {
    olsr_netlink_rule(olsr_cnf->ip_version, olsr_cnf->rttable, RTM_DELRULE);
  }

  close(olsr_cnf->rtnl_s);
#endif

#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  /* routing socket */
  close(olsr_cnf->rts);
#endif

  /* Free cookies and memory pools attached. */
  olsr_delete_all_cookies();

  olsr_syslog(OLSR_LOG_INFO, "%s stopped", olsrd_version);

  OLSR_PRINTF(1, "\n <<<< %s - terminating >>>>\n           http://www.olsr.org\n", olsrd_version);

  exit(olsr_cnf->exit_value);
}

/**
 * Print the command line usage
 */
static void
print_usage(void)
{

  fprintf(stderr,
          "An error occured somwhere between your keyboard and your chair!\n"
          "usage: olsrd [-f <configfile>] [ -i interface1 interface2 ... ]\n"
          "  [-d <debug_level>] [-ipv6] [-multi <IPv6 multicast address>]\n"
          "  [-lql <LQ level>] [-lqw <LQ winsize>] [-lqnt <nat threshold>]\n"
          "  [-bcast <broadcastaddr>] [-ipc] [-dispin] [-dispout] [-delgw]\n"
          "  [-hint <hello interval (secs)>] [-tcint <tc interval (secs)>]\n"
          "  [-midint <mid interval (secs)>] [-hnaint <hna interval (secs)>]\n"
          "  [-T <Polling Rate (secs)>] [-nofork] [-hemu <ip_address>]\n" "  [-lql <LQ level>] [-lqa <LQ aging factor>]\n");
}

/**
 * Sets the provided configuration on all unconfigured
 * interfaces
 *
 * @param ifs a linked list of interfaces to check and possible update
 * @param cnf the default configuration to set on unconfigured interfaces
 */
int
set_default_ifcnfs(struct olsr_if *ifs, struct if_config_options *cnf)
{
  int changes = 0;

  while (ifs) {
    if (ifs->cnf == NULL) {
      ifs->cnf = olsr_malloc(sizeof(struct if_config_options), "Set default config");
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
static int
olsr_process_arguments(int argc, char *argv[], struct olsrd_config *cnf, struct if_config_options *ifcnf)
{
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
      memcpy(&ifcnf->ipv4_broadcast.v4, &in.s_addr, sizeof(uint32_t));
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
        printf("LQ aging factor %f not allowed. Range [%f-%f]\n", tmp_lq_aging, MIN_LQ_AGING, MAX_LQ_AGING);
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
        printf("NAT threshold %f not allowed. Range [%f-%f]\n", tmp_lq_nat_thresh, 0.1, 1.0);
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
      queue_if(*argv, false);

      while ((argc - 1) && (argv[1][0] != '-')) {
        NEXT_ARG;
        printf("Queuing if %s\n", *argv);
        queue_if(*argv, false);
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
      ifcnf->hello_params.validity_time = ifcnf->hello_params.emission_interval * 3;
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

      memcpy(&ifcnf->ipv6_multi_glbl, &in6, sizeof(struct in6_addr));

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

      ifa = queue_if("hcif01", true);

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
 * A wrapper around times(2). Note, that this function has
 * some portability problems, e.g. different operating systems
 * and linux kernel versions may return values counted from
 * an arbitrary point in time (mostly uptime, some count from
 * the epoch. The linux man page therefore recommends not to
 * use this function. On the other hand, this function has
 * proved it's functions but some olsrd implementations does
 * error handling in a clumsy way - thus inducing bugs...
 * 
 * Analysis of times() in different OSes:
 * Linux: 
 *   times() returns the number of clock ticks that have 
 *   elapsed since an arbitrary point in the past.  The return 
 *   value may overflow the possible range of type clock_t.   On
 *     error, (clock_t) -1 is returned, and errno is set appropriately.
 * 
 * BSDs:  
 *  The times() function returns the value of time in CLK_TCK's 
 *  of a second since 0 hours, 0 minutes, 0 seconds, January 1, 
 *  1970, Coordinated Universal Time.
 * 
 * Values for clock_t in different OSes:
 *  OSX ............ unsigned long
 *  linux .......... long int (signed !) 
 *  win32 cygwin.... unsigned long
 *  openBSD ........ int
 * 
 * We therefore need to be very very careful how to (portably)
 * handle overflows!!
 * This current commit does not solve the problem yet. 
 * it merely documents the problems with times()
 * 
 */
clock_t
olsr_times(void)
{
  struct tms tms_buf;
  return times(&tms_buf);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */

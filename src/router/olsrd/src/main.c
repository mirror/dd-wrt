
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: main.c,v 1.83 2005/09/29 05:53:34 kattemat Exp $
 */

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

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

#ifdef WIN32
#define close(x) closesocket(x)
int __stdcall SignalHandler(unsigned long signal);
void ListInterfaces(void);
void DisableIcmpRedirects(void);
olsr_bool olsr_win32_end_request = OLSR_FALSE;
olsr_bool olsr_win32_end_flag = OLSR_FALSE;
#else
static void
olsr_shutdown(int);
#endif

/*
 * Local function prototypes
 */
void
olsr_reconfigure(int);

static void
print_usage(void);

static void
set_default_values(void);

static int
set_default_ifcnfs(struct olsr_if *, struct if_config_options *);

static int
olsr_process_arguments(int, char *[], 
		       struct olsrd_config *, 
		       struct if_config_options *);

static char **olsr_argv;

static char copyright_string[] = "The olsr.org Optimized Link-State Routing daemon(olsrd) Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org) All rights reserved.";

/**
 * Main entrypoint
 */

int
main(int argc, char *argv[])
{
  struct if_config_options *default_ifcnf;
  char conf_file_name[FILENAME_MAX];
  struct tms tms_buf;

  debug_handle = stdout;
  olsr_argv = argv;

#ifdef WIN32
  WSADATA WsaData;
  int len;
#endif

  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

#ifndef WIN32
  /* Initialize tick resolution */
  system_tick_divider = 1000/sysconf(_SC_CLK_TCK);

  /* Check if user is root */
  if(getuid() || getgid())
    {
      fprintf(stderr, "You must be root(uid = 0) to run olsrd!\nExiting\n\n");
      exit(EXIT_FAILURE);
    }
#else
  system_tick_divider = 1;

  DisableIcmpRedirects();

  if (WSAStartup(0x0202, &WsaData))
    {
      fprintf(stderr, "Could not initialize WinSock.\n");
      olsr_exit(__func__, EXIT_FAILURE);
    }
#endif

  /* Grab initial timestamp */
  now_times = times(&tms_buf);

  /* Open syslog */
  olsr_openlog("olsrd");

  /* Set default values */
  set_default_values();
 
  /* Get initial timestep */
  nowtm = NULL;
  while (nowtm == NULL)
    {
      nowtm = localtime((time_t *)&now.tv_sec);
    }
    
  /* The port to use for OLSR traffic */
  olsr_udp_port = htons(OLSRPORT);
    
  printf("\n *** %s ***\n Build date: %s\n http://www.olsr.org\n\n", 
	 SOFTWARE_VERSION, 
	 __DATE__);
    
  /* Using PID as random seed */
  srandom(getpid());


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
  
  strcpy(conf_file_name + len, "olsrd.conf");
#else
  strncpy(conf_file_name, OLSRD_GLOBAL_CONF_FILE, FILENAME_MAX);
#endif

  if ((argc > 1) && (strcmp(argv[1], "-f") == 0)) 
    {
      struct stat statbuf;

      argv++; argc--;
      if(argc == 1)
	{
	  fprintf(stderr, "You must provide a filename when using the -f switch!\n");
	  exit(EXIT_FAILURE);
	}

      if (stat(argv[1], &statbuf) < 0)
	{
	  fprintf(stderr, "Could not find specified config file %s!\n%s\n\n", argv[1], strerror(errno));
	  exit(EXIT_FAILURE);
	}
		 
      strncpy(conf_file_name, argv[1], FILENAME_MAX);
      argv++; argc--;

    }

  /*
   * set up configuration prior to processing commandline options
   */
  if((olsr_cnf = olsrd_parse_cnf(conf_file_name)) == NULL)
    {
      printf("Using default config values(no configfile)\n");
      olsr_cnf = olsrd_get_default_cnf();
    }
  if((default_ifcnf = get_default_if_config()) == NULL)
    {
      fprintf(stderr, "No default ifconfig found!\n");
      exit(EXIT_FAILURE);
    }

  /*
   * Process olsrd options.
   */
  if(olsr_process_arguments(argc, argv, olsr_cnf, default_ifcnf) < 0)
    {
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
  if(olsrd_sanity_check_cnf(olsr_cnf) < 0)
    {
      fprintf(stderr, "Bad configuration!\n");
      olsr_exit(__func__, EXIT_FAILURE);      
    }

  /*
   * Print configuration 
   */
  if(olsr_cnf->debug_level > 1)
    olsrd_print_cnf(olsr_cnf);

#ifndef WIN32
  /* Disable redirects globally */
  disable_redirects_global(olsr_cnf->ip_version);
#endif

  /*
   *socket for icotl calls
   */
  if ((ioctl_s = socket(olsr_cnf->ip_version, SOCK_DGRAM, 0)) < 0) 

    {
      olsr_syslog(OLSR_LOG_ERR, "ioctl socket: %m");
      olsr_exit(__func__, 0);
    }

#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  if ((rts = socket(PF_ROUTE, SOCK_RAW, 0)) < 0)
    {
      olsr_syslog(OLSR_LOG_ERR, "routing socket: %m");
      olsr_exit(__func__, 0);
    }
#endif

  /*
   *enable ip forwarding on host
   */
  enable_ip_forwarding(olsr_cnf->ip_version);

  /* Initialize parser */
  olsr_init_parser();

  /* Initialize message sequencnumber */
  init_msg_seqno();

  /* Initialize dynamic willingness calculation */
  olsr_init_willingness();

  /*
   *Set up willingness/APM
   */
  if(olsr_cnf->willingness_auto)
    {
      if(apm_init() < 0)
	{
	  OLSR_PRINTF(1, "Could not read APM info - setting default willingness(%d)\n", WILL_DEFAULT)

	  olsr_syslog(OLSR_LOG_ERR, "Could not read APM info - setting default willingness(%d)\n", WILL_DEFAULT);

	  olsr_cnf->willingness_auto = 0;
	  olsr_cnf->willingness = WILL_DEFAULT;
	}
      else
	{
	  olsr_cnf->willingness = olsr_calculate_willingness();

	  OLSR_PRINTF(1, "Willingness set to %d - next update in %.1f secs\n", olsr_cnf->willingness, will_int)
	}
    }

  /* Set ipsize and minimum packetsize */
  if(olsr_cnf->ip_version == AF_INET6)
    {
      OLSR_PRINTF(1, "Using IP version 6\n")
      ipsize = sizeof(struct in6_addr);
      minsize = (int)sizeof(olsr_u8_t) * 7; /* Minimum packetsize IPv6 */
    }
  else
    {
      OLSR_PRINTF(1, "Using IP version 4\n")
      ipsize = sizeof(olsr_u32_t);
      minsize = (int)sizeof(olsr_u8_t) * 4; /* Minimum packetsize IPv4 */
    }

  /* Initialize net */
  init_net();

  /* Initializing networkinterfaces */
  if(!ifinit())
    {
      if(olsr_cnf->allow_no_interfaces)
	{
	  fprintf(stderr, "No interfaces detected! This might be intentional, but it also might mean that your configuration is fubar.\nI will continue after 5 seconds...\n");
	  sleep(5);
	}
      else
	{
	  fprintf(stderr, "No interfaces detected!\nBailing out!\n");
	  olsr_exit(__func__, EXIT_FAILURE);
	}
    }

  /* Print heartbeat to stdout */

#if !defined WINCE
  if(olsr_cnf->debug_level > 0 && isatty(STDOUT_FILENO))
    olsr_register_scheduler_event(&generate_stdout_pulse, NULL, STDOUT_PULSE_INT, 0, NULL);
#endif
  
  gettimeofday(&now, NULL);


  /* Initialize the IPC socket */

  if(olsr_cnf->open_ipc)
      ipc_init();

  /* Initialisation of different tables to be used.*/
  olsr_init_tables();

  /* daemon mode */
#ifndef WIN32
  if((olsr_cnf->debug_level == 0) && (!olsr_cnf->no_fork))
    {
      printf("%s detaching from the current process...\n", SOFTWARE_VERSION);
      if(daemon(0, 0) < 0)
	{
	  printf("daemon(3) failed: %s\n", strerror(errno));
	  exit(EXIT_FAILURE);
	}
    }
#endif

  /* Load plugins */
  olsr_load_plugins();

  OLSR_PRINTF(1, "Main address: %s\n\n", olsr_ip_to_string(&main_addr))

  /* Start syslog entry */
  olsr_syslog(OLSR_LOG_INFO, "%s successfully started", SOFTWARE_VERSION);

  /*
   *signal-handlers
   */

  /* ctrl-C and friends */
#ifdef WIN32
#ifndef WINCE
  SetConsoleCtrlHandler(SignalHandler, OLSR_TRUE);
#endif
#else
  signal(SIGHUP, olsr_reconfigure);  
  signal(SIGINT, olsr_shutdown);  
  signal(SIGTERM, olsr_shutdown);  
#endif

  /* Register socket poll event */
  olsr_register_timeout_function(&poll_sockets);

  /* Starting scheduler */
  scheduler();

  /* Stop the compiler from complaining */
  strlen(copyright_string);

  /* Like we're ever going to reach this ;-) */
  return 1;

} /* main */



/**
 * Reconfigure olsrd. Currently kind of a hack...
 *
 *@param signal the signal that triggered this callback
 */
#ifndef WIN32
void
olsr_reconfigure(int signal)
{
  if(!fork())
    {
      /* New process */
      sleep(3);
      printf("Restarting %s\n", olsr_argv[0]);
      execv(olsr_argv[0], olsr_argv);
    }
  olsr_shutdown(0);

  printf("RECONFIGURING!\n");
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
olsr_shutdown(int signal)
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

  OLSR_PRINTF(1, "Closing sockets...\n")

  /* front-end IPC socket */
  if(olsr_cnf->open_ipc)
    shutdown_ipc();

  /* OLSR sockets */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) 
    close(ifn->olsr_socket);

  /* Closing plug-ins */
  olsr_close_plugins();

  /* Reset network settings */
  restore_settings(olsr_cnf->ip_version);

  /* ioctl socket */
  close(ioctl_s);

#if defined __FreeBSD__ || defined __MacOSX__ || defined __NetBSD__ || defined __OpenBSD__
  /* routing socket */
  close(rts);
#endif

  olsr_syslog(OLSR_LOG_INFO, "%s stopped", SOFTWARE_VERSION);

  OLSR_PRINTF(1, "\n <<<< %s - terminating >>>>\n           http://www.olsr.org\n", SOFTWARE_VERSION)

  exit(exit_value);
}





/**
 *Sets the default values of variables at startup
 *
 */
static void
set_default_values()
{
  exit_value = EXIT_SUCCESS; 
  /* If the application exits by signal it is concidered success,
   * if not, exit_value is set by the function calling olsr_exit.
   */

  will_int = 10 * HELLO_INTERVAL; /* Willingness update interval */

  /* Initialize empty TC timer */
  send_empty_tc = GET_TIMESTAMP(0);
}



/**
 * Print the command line usage
 */
static void
print_usage()
{

  fprintf(stderr, "An error occured somwhere between your keyboard and your chair!\n"); 
  fprintf(stderr, "usage: olsrd [-f <configfile>] [ -i interface1 interface2 ... ]\n");
  fprintf(stderr, "  [-d <debug_level>] [-ipv6] [-multi <IPv6 multicast address>]\n"); 
  fprintf(stderr, "  [-bcast <broadcastaddr>] [-ipc] [-dispin] [-dispout] [-delgw]\n");
  fprintf(stderr, "  [-hint <hello interval (secs)>] [-tcint <tc interval (secs)>]\n");
  fprintf(stderr, "  [-midint <mid interval (secs)>] [-hnaint <hna interval (secs)>]\n");
  fprintf(stderr, "  [-T <Polling Rate (secs)>] [-nofork] [-hemu <ip_address>] \n"); 

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

  while(ifs)
    {
      if(ifs->cnf == NULL)
	{
	  ifs->cnf = olsr_malloc(sizeof(struct if_config_options), "Set default config");
	  *ifs->cnf = *cnf;
	  changes++;
	}
      ifs = ifs->next;
    }
  return changes;
}


#define NEXT_ARG argv++;argc--
#define CHECK_ARGC if(!argc) { \
      if((argc - 1) == 1){ \
      fprintf(stderr, "Error parsing command line options!\n"); \
      olsr_exit(__func__, EXIT_FAILURE); \
      } else { \
      argv--; \
      fprintf(stderr, "You must provide a parameter when using the %s switch!\n", *argv); \
      olsr_exit(__func__, EXIT_FAILURE); \
     } \
     }

/**
 * Process command line arguments passed to olsrd
 *
 */
static int
olsr_process_arguments(int argc, char *argv[], 
		       struct olsrd_config *cnf, 
		       struct if_config_options *ifcnf)
{
  while (argc > 1)
    {
      NEXT_ARG;
#ifdef WIN32
      /*
       *Interface list
       */
      if (strcmp(*argv, "-int") == 0)
        {
          ListInterfaces();
          exit(0);
        }
#endif

      /*
       *Configfilename
       */
      if(strcmp(*argv, "-f") == 0) 
	{
	  fprintf(stderr, "Configfilename must ALWAYS be first argument!\n\n");
	  olsr_exit(__func__, EXIT_FAILURE);
	}

      /*
       *Use IP version 6
       */
      if(strcmp(*argv, "-ipv6") == 0) 
	{
	  cnf->ip_version = AF_INET6;
	  continue;
	}

      /*
       *Broadcast address
       */
      if(strcmp(*argv, "-bcast") == 0) 
	{
	  struct in_addr in;
	  NEXT_ARG;
          CHECK_ARGC;

	  if (inet_aton(*argv, &in) == 0)
	    {
	      printf("Invalid broadcast address! %s\nSkipping it!\n", *argv);
	      continue;
	    }
	  memcpy(&ifcnf->ipv4_broadcast.v4, &in.s_addr, sizeof(olsr_u32_t));  
	  continue;
	}
      
      /*
       * Enable additional debugging information to be logged.
       */
      if (strcmp(*argv, "-d") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;

	  sscanf(*argv,"%d", &cnf->debug_level);
	  continue;
	}

		
      /*
       * Interfaces to be used by olsrd.
       */
      if (strcmp(*argv, "-i") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;

	  if(*argv[0] == '-')
	    {
	      fprintf(stderr, "You must provide an interface label!\n");
	      olsr_exit(__func__, EXIT_FAILURE);
	    }
	  printf("Queuing if %s\n", *argv);
	  queue_if(*argv, OLSR_FALSE);

	  while((argc - 1) && (argv[1][0] != '-'))
	    {
	      NEXT_ARG;
	      printf("Queuing if %s\n", *argv);
	      queue_if(*argv, OLSR_FALSE);
	    }

	  continue;
	}
      /*
       * Set the hello interval to be used by olsrd.
       * 
       */
      if (strcmp(*argv, "-hint") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;
	  sscanf(*argv,"%f", &ifcnf->hello_params.emission_interval);
          ifcnf->hello_params.validity_time = ifcnf->hello_params.emission_interval * 3;
	  continue;
	}

      /*
       * Set the HNA interval to be used by olsrd.
       * 
       */
      if (strcmp(*argv, "-hnaint") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;
	  sscanf(*argv,"%f", &ifcnf->hna_params.emission_interval);
          ifcnf->hna_params.validity_time = ifcnf->hna_params.emission_interval * 3;
	  continue;
	}

      /*
       * Set the MID interval to be used by olsrd.
       * 
       */
      if (strcmp(*argv, "-midint") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;
	  sscanf(*argv,"%f", &ifcnf->mid_params.emission_interval);
          ifcnf->mid_params.validity_time = ifcnf->mid_params.emission_interval * 3;
	  continue;
	}

      /*
       * Set the tc interval to be used by olsrd.
       * 
       */
      if (strcmp(*argv, "-tcint") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;
	  sscanf(*argv,"%f", &ifcnf->tc_params.emission_interval);
          ifcnf->tc_params.validity_time = ifcnf->tc_params.emission_interval * 3;
	  continue;
	}

      /*
       * Set the polling interval to be used by olsrd.
       */
      if (strcmp(*argv, "-T") == 0) 
	{
	  NEXT_ARG;
          CHECK_ARGC;
	  sscanf(*argv,"%f",&cnf->pollrate);
	  continue;
	}


      /*
       * Should we display the contents of packages beeing sent?
       */
      if (strcmp(*argv, "-dispin") == 0) 
	{
	  disp_pack_in = OLSR_TRUE;
	  continue;
	}

      /*
       * Should we display the contents of incoming packages?
       */
      if (strcmp(*argv, "-dispout") == 0) 
	{
	  disp_pack_out = OLSR_TRUE;
	  continue;
	}


      /*
       * Should we set up and send on a IPC socket for the front-end?
       */
      if (strcmp(*argv, "-ipc") == 0) 
	{
	  cnf->ipc_connections = 1;
	  cnf->open_ipc = OLSR_TRUE;
	  continue;
	}

      /*
       * IPv6 multicast addr
       */
      if (strcmp(*argv, "-multi") == 0) 
	{
	  struct in6_addr in6;
	  NEXT_ARG;
          CHECK_ARGC;
	  if(inet_pton(AF_INET6, *argv, &in6) < 0)
	    {
	      fprintf(stderr, "Failed converting IP address %s\n", *argv);
	      exit(EXIT_FAILURE);
	    }

	  memcpy(&ifcnf->ipv6_multi_glbl, &in6, sizeof(struct in6_addr));

	  continue;
	}

      /*
       * Host emulation
       */
      if (strcmp(*argv, "-hemu") == 0) 
	{
	  struct in_addr in;
	  struct olsr_if *ifa;
      
	  NEXT_ARG;
          CHECK_ARGC;
	  if(inet_pton(AF_INET, *argv, &in) < 0)
	    {
	      fprintf(stderr, "Failed converting IP address %s\n", *argv);
	      exit(EXIT_FAILURE);
	    }
	  /* Add hemu interface */

	  ifa = queue_if("hcif01", OLSR_TRUE);

	  if(!ifa)
	    continue;

	  ifa->cnf = get_default_if_config();
	  ifa->host_emul = OLSR_TRUE;
	  memcpy(&ifa->hemu_ip, &in, sizeof(union olsr_ip_addr));
	  cnf->host_emul = OLSR_TRUE;

	  continue;
	}


      /*
       * Delete possible default GWs
       */
      if (strcmp(*argv, "-delgw") == 0) 
	{
	  del_gws = OLSR_TRUE;
	  continue;
	}


      if (strcmp(*argv, "-nofork") == 0) 
	{
	  cnf->no_fork = OLSR_TRUE;
	  continue;
	}

      return -1;
    }
  return 0;
}

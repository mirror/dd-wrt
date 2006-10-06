#   define __set_errno(val) ((errno) = (val))

#include "rflow.h"
#include "cfgvar.h"
#include "servers.h"
#include "opt.h"
#include "pidfile.h"
#include "storage.h"

#include <sys/syscall.h>
#include <sys/resource.h>
_syscall3(int, setpriority, __priority_which_t, which, id_t, who, int, prio);

#define IPCAD_VERSION_STRING "123"
#define IPCAD_COPYRIGHT "fcff"

void terminate_threads();

void
sigalarm(int z) {
	/* Issue cancellable call */
	//fprintf(stderr, "Thread %ld alarmed.\n", (long)pthread_self());
	signal(z, sigalarm);
}

void
set_display_now(int z) {
	display_now = 1;
	signal(z, set_display_now);
}

void
sigquit(int z) {
	signoff_now = 1;
	signal(z, sigquit);
}

double self_started;

static int netflow_destination (char* spec)
{
  char *s;
  struct in_addr ip;
  int hport;

  if ((s = strchr (spec, ':')) == NULL)
    rflow_usage ();

  *s++ = '\0';
  hport = atoi (s);
  if (hport == 0 || inet_pton (AF_INET, spec, &ip)<0)
    rflow_usage ();

  if(add_server(netflow_exporter, "NetFlow destination", &ip, hport))
    return -1;
  
  fprintf(stderr, "Configured NetFlow destination at %s:%d\n",
    spec, hport);
  return 0;
}

int
rflow_main(int ac, char **av) {
	struct timeval tv;
	char *config_file = CONFIG_FILE;
	int disable_servers = 0;
	int pidfile_fd = -1;
	int devnull_fd = -1;
	int c;
	unsigned i;

	gettimeofday(&tv, NULL);
	self_started = tv.tv_sec + (double)tv.tv_usec / 1000000;

	/*
	 * Set defaults.
	 */
	conf->netflow_version = 5;
	conf->netflow_timeout_active = 30 * 60;	/* Seconds */
	conf->netflow_timeout_inactive = 15;	/* Seconds */
	conf->netflow_packet_interval = 1;	/* packets of packets */
	for(i = 0; i < sizeof(agr_portmap)/sizeof(agr_portmap[0]); i++)
		agr_portmap[i] = i;	/* Default is 1:1 mapping. */


	while((c = getopt(ac, av, "dhc:SvF:i:N:A:I:V:")) != -1)
	switch(c) {
		case 'N':
			if (! optarg) rflow_usage();
			conf->netflow_version = atoi (optarg);
			if (conf->netflow_version != 1 && conf->netflow_version != 5)
				rflow_usage ();
			break;
		case 'A':
			if (! optarg) rflow_usage();
			if ((conf->netflow_timeout_active = atoi (optarg)) <= 0) rflow_usage ();
			break;
		case 'I':
			if (! optarg) rflow_usage();
			if ((conf->netflow_timeout_inactive = atoi (optarg)) <= 0) rflow_usage ();
			break;
		case 'V':
			if (! optarg) rflow_usage();
			if ((conf->netflow_packet_interval = atoi (optarg)) <= 0) rflow_usage ();
			break;
		case 'c':
			config_file = optarg;
			if(!config_file)
				rflow_usage();
			break;
    case 'F':
      if (! optarg) 
        rflow_usage ();

      if (netflow_destination (optarg))
      {
			  fprintf(stderr, "Failed to install NetFlow destination\n");
			  exit(EX_OSERR);
      }
      break;

    case 'i':
      if (! optarg) 
        rflow_usage ();

      if (cfg_add_iface(optarg, /*IFLAG_PROMISC|*/IFLAG_LARGE_CAP, 0) == NULL)
      {
			  fprintf(stderr, "Failed to open %s\n", optarg);
			  exit(EX_OSERR);
      }
      break;

		case 'd':
			daemon_mode = 1;
			break;
		case 'h':
			rflow_usage();
			break;
		case 'S':
			disable_servers = 1;
			break;
		case 'v':
			fprintf(stderr,
				IPCAD_VERSION_STRING
				IPCAD_COPYRIGHT "\n");
			exit(EX_USAGE);
		default:
			rflow_usage();
	}

	if(init_pthread_options()) {
		fprintf(stderr, "Can't initialize thread options.\n");
		exit(EX_OSERR);
	}

	/*
	 * Open /dev/null for daemonizing.
	 */
	devnull_fd = open(_PATH_DEVNULL, O_RDWR, 0);
	if(devnull_fd == -1) {
		fprintf(stderr, "Can't open " _PATH_DEVNULL ": %s\n",
			strerror(errno));
		exit(EX_OSERR);
	}

#if 0
	/*
	 * Read specified or default configuration file.
	 */
	if(cfgread(config_file))
		exit(EX_NOINPUT);
#endif

	/******************************/
	/* Process configuration data */
	/******************************/

	/*
	 * Simple checks
	 */
	if(conf->packet_sources_head == NULL) {
		fprintf(stderr, "No interfaces initialized.\n");
		exit(EX_NOINPUT);
	}

	/*
	 * Simple deeds.
	 */
	/* Pre-open certain files before doing chroot(). */
	if(ifst_preopen())
		exit(EX_OSERR);

	/* PID file */
	if(conf->pidfile) {
		pidfile_fd = make_pid_file(conf->pidfile);
		if(pidfile_fd == -1) {
			fprintf(stderr,
				"Can't initialize pid file %s: %s\n",
				conf->pidfile, strerror(errno));
			exit(EX_DATAERR);
		}
	}

#if AFLOW
	time(&active_storage.create_time);
#endif
	time(&netflow_storage.create_time);

	/* Daemon mode should be entered BEFORE threads created. */
	if(daemon_mode) {
		fflush(NULL);

		switch(fork()) {
		case -1:
			perror("ipcad");
			exit(EX_OSERR);
		case 0:
			break;
		default:
			_exit(0);
		}

		if(setsid() == -1) {
			perror("setsid() failed");
			exit(EX_OSERR);
		}

		fprintf(stderr, "Daemonized.\n");
		fflush(stderr); /* As a remainder */

		(void)dup2(devnull_fd, 0);
		(void)dup2(devnull_fd, 1);
		(void)dup2(devnull_fd, 2);
		if(devnull_fd > 2)
			close(devnull_fd);

		/* Update pid value */
		if(pidfile_fd != -1) {
			(void)write_pid_file(pidfile_fd, getpid(), NULL);
			/* Ignore virtually impossible errors */
		}
	}


	/***********************************/
	/* Set appropriate signal handlers */
	/***********************************/

	/*
	 * Ignore signals, by default
	 */
	for(c = 1; c < 32; c++) {
		switch(c) {
		case SIGCHLD:
			/*
			 * Setting SIG_IGN breaks threading support
			 * in Linux 2.4.20-20.9
			 */
		case SIGSEGV:
		case SIGBUS:
			/*
			 * No use to continue after SIGSEGV/SIGBUS.
			 * Program must not generate SIGSEGV/SIGBUS
			 * in the first place.
			 */
			continue;
		}
		signal(c, SIG_IGN);
	}

	signal(SIGALRM, sigalarm);
	signal(SIGINT, set_display_now);
	signal(SIGHUP, set_display_now);
	signal(SIGQUIT, sigquit);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, sigquit);
	signal(SIGTTIN, SIG_IGN);

	siginterrupt(SIGALRM, 1);

	/*******************************************/
	/* Start servers to serve clients requests */
	/*******************************************/

	if(disable_servers == 0) {
		if( start_servers() != 0 ) {
			fprintf(stderr,
				"Failed to start one or more servers.\n");
			exit(EX_OSERR);
		}
	}


#ifdef	HAVE_SETPRIORITY
	/*
	 * Set nice, but safe priority.
	 * Required to process high loads without
	 * significant packet loss.
	 */
	setpriority(PRIO_PROCESS, 0, -15);
#endif

	/*
	 * The main working loop.
	 */
	process_packet_sources(conf->packet_sources_head);

	/* Termination */
	terminate_threads();

	/* Leave the pid file gracefully */
	if(pidfile_fd != -1) {
		(void)write_pid_file(pidfile_fd, 0, "finished\n");
		close(pidfile_fd);
	}

	fprintf(stderr, "Quit.\n");

	return 0;
}

void
terminate_threads() {
	packet_source_t *ps;

	end_servers();

	printf("Signalling interface threads");
	for(ps = conf->packet_sources_head; ps; ps = ps->next) {
		if(!ps->thid)
			continue;

		printf("."); fflush(stdout);
		if(pthread_cancel(ps->thid)) {
			printf("%s processing thread is already gone.\n",
				ps->ifName);
			ps->thid = 0;
			continue;
		}

		/* Notify the process */
		pthread_kill(ps->thid, SIGALRM);
	}

	printf("\nWaiting for interface processing threads to terminate...\n");

	for(ps = conf->packet_sources_head; ps; ps = ps->next) {
		if(!ps->thid)
			continue;

		if(pthread_join(ps->thid, NULL)) {
			if(errno == EINVAL)
				printf("Thread processing %s "
					"is not a joinable thread.\n",
					ps->ifName);
			if(errno == ESRCH)
				printf("No thread running "
					"for processing %s\n",
					ps->ifName);
			if(errno == EDEADLK)	/* Impossible state */
				printf("Deadlock avoided "
					"for thread processing %s\n",
					ps->ifName);
		} else {
			printf("Thread processing %s terminated.\n",
				ps->ifName);
		}
	}
}

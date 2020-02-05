/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) John G. Hasler  2009
 * Copyright (C) Miroslav Lichvar  2012-2018
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  The main program
  */

#include "config.h"

#include "sysincl.h"

#include "main.h"
#include "sched.h"
#include "local.h"
#include "sys.h"
#include "ntp_io.h"
#include "ntp_signd.h"
#include "ntp_sources.h"
#include "ntp_core.h"
#include "sources.h"
#include "sourcestats.h"
#include "reference.h"
#include "logging.h"
#include "conf.h"
#include "cmdmon.h"
#include "keys.h"
#include "manual.h"
#include "rtc.h"
#include "refclock.h"
#include "clientlog.h"
#include "nameserv.h"
#include "privops.h"
#include "smooth.h"
#include "tempcomp.h"
#include "util.h"

#define printf(...) fprintf(stderr, __VA_ARGS__);

/* ================================================== */

/* Set when the initialisation chain has been completed.  Prevents finalisation
 * chain being run if a fatal error happened early. */

static int initialised = 0;

static int exit_status = 0;

static int reload = 0;

static REF_Mode ref_mode = REF_ModeNormal;

/* ================================================== */

static void
do_platform_checks(void)
{
  /* Require at least 32-bit integers, two's complement representation and
     the usual implementation of conversion of unsigned integers */
  assert(sizeof (int) >= 4);
  assert(-1 == ~0);
  assert((int32_t)4294967295U == (int32_t)-1);
}

/* ================================================== */

static void
delete_pidfile(void)
{
  const char *pidfile = CNF_GetPidFile();

  if (!pidfile[0])
    return;

  /* Don't care if this fails, there's not a lot we can do */
  unlink(pidfile);
}

/* ================================================== */

void
MAI_CleanupAndExit(void)
{
  if (!initialised) exit(exit_status);
  
  if (CNF_GetDumpDir()[0] != '\0') {
    SRC_DumpSources();
  }

  /* Don't update clock when removing sources */
  REF_SetMode(REF_ModeIgnore);

  SMT_Finalise();
  TMC_Finalise();
  MNL_Finalise();
  CLG_Finalise();
  NSD_Finalise();
  NSR_Finalise();
  SST_Finalise();
  NCR_Finalise();
  NIO_Finalise();
  CAM_Finalise();
  KEY_Finalise();
  RCL_Finalise();
  SRC_Finalise();
  REF_Finalise();
  RTC_Finalise();
  SYS_Finalise();
  SCH_Finalise();
  LCL_Finalise();
  PRV_Finalise();

  delete_pidfile();
  
  CNF_Finalise();
  HSH_Finalise();
  LOG_Finalise();

  exit(exit_status);
}

/* ================================================== */

static void
signal_cleanup(int x)
{
  if (!initialised) exit(0);
  SCH_QuitProgram();
}

/* ================================================== */

static void
quit_timeout(void *arg)
{
  /* Return with non-zero status if the clock is not synchronised */
  exit_status = REF_GetOurStratum() >= NTP_MAX_STRATUM;
  SCH_QuitProgram();
}

/* ================================================== */

static void
ntp_source_resolving_end(void)
{
  NSR_SetSourceResolvingEndHandler(NULL);

  if (reload) {
    /* Note, we want reload to come well after the initialisation from
       the real time clock - this gives us a fighting chance that the
       system-clock scale for the reloaded samples still has a
       semblence of validity about it. */
    SRC_ReloadSources();
  }

  SRC_RemoveDumpFiles();
  RTC_StartMeasurements();
  RCL_StartRefclocks();
  NSR_StartSources();
  NSR_AutoStartSources();

  /* Special modes can end only when sources update their reachability.
     Give up immediatelly if there are no active sources. */
  if (ref_mode != REF_ModeNormal && !SRC_ActiveSources()) {
    REF_SetUnsynchronised();
  }
}

/* ================================================== */

static void
post_init_ntp_hook(void *anything)
{
  if (ref_mode == REF_ModeInitStepSlew) {
    /* Remove the initstepslew sources and set normal mode */
    NSR_RemoveAllSources();
    ref_mode = REF_ModeNormal;
    REF_SetMode(ref_mode);
  }

  /* Close the pipe to the foreground process so it can exit */
  LOG_CloseParentFd();

  CNF_AddSources();
  CNF_AddBroadcasts();

  NSR_SetSourceResolvingEndHandler(ntp_source_resolving_end);
  NSR_ResolveSources();
}

/* ================================================== */

static void
reference_mode_end(int result)
{
  switch (ref_mode) {
    case REF_ModeNormal:
    case REF_ModeUpdateOnce:
    case REF_ModePrintOnce:
      exit_status = !result;
      SCH_QuitProgram();
      break;
    case REF_ModeInitStepSlew:
      /* Switch to the normal mode, the delay is used to prevent polling
         interval shorter than the burst interval if some configured servers
         were used also for initstepslew */
      SCH_AddTimeoutByDelay(2.0, post_init_ntp_hook, NULL);
      break;
    default:
      assert(0);
  }
}

/* ================================================== */

static void
post_init_rtc_hook(void *anything)
{
  if (CNF_GetInitSources() > 0) {
    CNF_AddInitSources();
    NSR_StartSources();
    assert(REF_GetMode() != REF_ModeNormal);
    /* Wait for mode end notification */
  } else {
    (post_init_ntp_hook)(NULL);
  }
}

/* ================================================== */

static void
check_pidfile(void)
{
  const char *pidfile = CNF_GetPidFile();
  FILE *in;
  int pid, count;
  
  in = fopen(pidfile, "r");
  if (!in)
    return;

  count = fscanf(in, "%d", &pid);
  fclose(in);
  
  if (count != 1)
    return;

  if (getsid(pid) < 0)
    return;

  LOG_FATAL("Another chronyd may already be running (pid=%d), check %s",
            pid, pidfile);
}

/* ================================================== */

static void
write_pidfile(void)
{
  const char *pidfile = CNF_GetPidFile();
  FILE *out;

  if (!pidfile[0])
    return;

  out = fopen(pidfile, "w");
  if (!out) {
    LOG_FATAL("Could not open %s : %s", pidfile, strerror(errno));
  } else {
    fprintf(out, "%d\n", (int)getpid());
    fclose(out);
  }
}

/* ================================================== */

#define DEV_NULL "/dev/null"

static void
go_daemon(void)
{
  int pid, fd, pipefd[2];

  /* Create pipe which will the daemon use to notify the grandparent
     when it's initialised or send an error message */
  if (pipe(pipefd)) {
    LOG_FATAL("pipe() failed : %s", strerror(errno));
  }

  /* Does this preserve existing signal handlers? */
  pid = fork();

  if (pid < 0) {
    LOG_FATAL("fork() failed : %s", strerror(errno));
  } else if (pid > 0) {
    /* In the 'grandparent' */
    char message[1024];
    int r;

    close(pipefd[1]);
    r = read(pipefd[0], message, sizeof (message));
    if (r) {
      if (r > 0) {
        /* Print the error message from the child */
        message[sizeof (message) - 1] = '\0';
        fprintf(stderr, "%s\n", message);
      }
      exit(1);
    } else
      exit(0);
  } else {
    close(pipefd[0]);

    setsid();

    /* Do 2nd fork, as-per recommended practice for launching daemons. */
    pid = fork();

    if (pid < 0) {
      LOG_FATAL("fork() failed : %s", strerror(errno));
    } else if (pid > 0) {
      exit(0); /* In the 'parent' */
    } else {
      /* In the child we want to leave running as the daemon */

      /* Change current directory to / */
      if (chdir("/") < 0) {
        LOG_FATAL("chdir() failed : %s", strerror(errno));
      }

      /* Don't keep stdin/out/err from before. But don't close
         the parent pipe yet. */
      for (fd=0; fd<1024; fd++) {
        if (fd != pipefd[1])
          close(fd);
      }

      LOG_SetParentFd(pipefd[1]);

      /* Open /dev/null as new stdin/out/err */
      errno = 0;
      if (open(DEV_NULL, O_RDONLY) != STDIN_FILENO ||
          open(DEV_NULL, O_WRONLY) != STDOUT_FILENO ||
          open(DEV_NULL, O_RDWR) != STDERR_FILENO)
        LOG_FATAL("Could not open %s : %s", DEV_NULL, strerror(errno));
    }
  }
}

/* ================================================== */

static void
print_help(const char *progname)
{
      printf("Usage: %s [-4|-6] [-n|-d] [-q|-Q] [-r] [-R] [-s] [-t TIMEOUT] [-f FILE|COMMAND...]\n",
             progname);
}

/* ================================================== */

static void
print_version(void)
{
  printf("chronyd (chrony) version %s (%s)\n", CHRONY_VERSION, CHRONYD_FEATURES);
}

/* ================================================== */

static int
parse_int_arg(const char *arg)
{
  int i;

  if (sscanf(arg, "%d", &i) != 1)
    LOG_FATAL("Invalid argument %s", arg);
  return i;
}

/* ================================================== */

int main
(int argc, char **argv)
{
  const char *conf_file = DEFAULT_CONF_FILE;
  const char *progname = argv[0];
  char *user = NULL, *log_file = NULL;
  struct passwd *pw;
  int opt, debug = 0, nofork = 0, address_family = IPADDR_UNSPEC;
  int do_init_rtc = 0, restarted = 0, client_only = 0, timeout = 0;
  int scfilter_level = 0, lock_memory = 0, sched_priority = 0;
  int clock_control = 1, system_log = 1;
  int config_args = 0;

  do_platform_checks();

  LOG_Initialise();

  /* Parse (undocumented) long command-line options */
  for (optind = 1; optind < argc; optind++) {
    if (!strcmp("--help", argv[optind])) {
      print_help(progname);
      return 0;
    } else if (!strcmp("--version", argv[optind])) {
      print_version();
      return 0;
    }
  }

  optind = 1;

  /* Parse short command-line options */
  while ((opt = getopt(argc, argv, "46df:F:hl:mnP:qQrRst:u:vx")) != -1) {
    switch (opt) {
      case '4':
      case '6':
        address_family = opt == '4' ? IPADDR_INET4 : IPADDR_INET6;
        break;
      case 'd':
        debug++;
        nofork = 1;
        system_log = 0;
        break;
      case 'f':
        conf_file = optarg;
        break;
      case 'F':
        scfilter_level = parse_int_arg(optarg);
        break;
      case 'l':
        log_file = optarg;
        break;
      case 'm':
        lock_memory = 1;
        break;
      case 'n':
        nofork = 1;
        break;
      case 'P':
        sched_priority = parse_int_arg(optarg);
        break;
      case 'q':
        ref_mode = REF_ModeUpdateOnce;
        nofork = 1;
        client_only = 0;
        system_log = 0;
        break;
      case 'Q':
        ref_mode = REF_ModePrintOnce;
        nofork = 1;
        client_only = 1;
        clock_control = 0;
        system_log = 0;
        break;
      case 'r':
        reload = 1;
        break;
      case 'R':
        restarted = 1;
        break;
      case 's':
        do_init_rtc = 1;
        break;
      case 't':
        timeout = parse_int_arg(optarg);
        break;
      case 'u':
        user = optarg;
        break;
      case 'v':
        print_version();
        return 0;
      case 'x':
        clock_control = 0;
        break;
      default:
        print_help(progname);
        return opt != 'h';
    }
  }

  if (getuid() && !client_only)
    LOG_FATAL("Not superuser");

  /* Turn into a daemon */
  if (!nofork) {
    go_daemon();
  }

  if (log_file) {
    LOG_OpenFileLog(log_file);
  } else if (system_log) {
    LOG_OpenSystemLog();
  }
  
  LOG_SetDebugLevel(debug);
  
  LOG(LOGS_INFO, "chronyd version %s starting (%s)", CHRONY_VERSION, CHRONYD_FEATURES);

  DNS_SetAddressFamily(address_family);

  CNF_Initialise(restarted, client_only);

  /* Parse the config file or the remaining command line arguments */
  config_args = argc - optind;
  if (!config_args) {
    CNF_ReadFile(conf_file);
  } else {
    for (; optind < argc; optind++)
      CNF_ParseLine(NULL, config_args + optind - argc + 1, argv[optind]);
  }

  /* Check whether another chronyd may already be running */
  check_pidfile();

  if (!user)
    user = CNF_GetUser();

  pw = getpwnam(user);
  if (!pw)
    LOG_FATAL("Could not get user/group ID of %s", user);

  /* Create directories for sockets, log files, and dump files */
  CNF_CreateDirs(pw->pw_uid, pw->pw_gid);

  /* Write our pidfile to prevent other instances from running */
  write_pidfile();

  PRV_Initialise();
  LCL_Initialise();
  SCH_Initialise();
  SYS_Initialise(clock_control);
  RTC_Initialise(do_init_rtc);
  SRC_Initialise();
  RCL_Initialise();
  KEY_Initialise();

  /* Open privileged ports before dropping root */
  CAM_Initialise(address_family);
  NIO_Initialise(address_family);
  NCR_Initialise();
  CNF_SetupAccessRestrictions();

  /* Command-line switch must have priority */
  if (!sched_priority) {
    sched_priority = CNF_GetSchedPriority();
  }
  if (sched_priority) {
    SYS_SetScheduler(sched_priority);
  }

  if (lock_memory || CNF_GetLockMemory()) {
    SYS_LockMemory();
  }

  /* Drop root privileges if the specified user has a non-zero UID */
  if (!geteuid() && (pw->pw_uid || pw->pw_gid))
    SYS_DropRoot(pw->pw_uid, pw->pw_gid);

  REF_Initialise();
  SST_Initialise();
  NSR_Initialise();
  NSD_Initialise();
  CLG_Initialise();
  MNL_Initialise();
  TMC_Initialise();
  SMT_Initialise();

  /* From now on, it is safe to do finalisation on exit */
  initialised = 1;

  UTI_SetQuitSignalsHandler(signal_cleanup, 1);

  CAM_OpenUnixSocket();

  if (scfilter_level)
    SYS_EnableSystemCallFilter(scfilter_level);

  if (ref_mode == REF_ModeNormal && CNF_GetInitSources() > 0) {
    ref_mode = REF_ModeInitStepSlew;
  }

  REF_SetModeEndHandler(reference_mode_end);
  REF_SetMode(ref_mode);

  if (timeout > 0)
    SCH_AddTimeoutByDelay(timeout, quit_timeout, NULL);

  if (do_init_rtc) {
    RTC_TimeInit(post_init_rtc_hook, NULL);
  } else {
    post_init_rtc_hook(NULL);
  }

  /* The program normally runs under control of the main loop in
     the scheduler. */
  SCH_MainLoop();

  LOG(LOGS_INFO, "chronyd exiting");

  MAI_CleanupAndExit();

  return 0;
}

/* ================================================== */

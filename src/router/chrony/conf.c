/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2009-2017, 2020, 2024-2025
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

  Module that reads and processes the configuration file.
  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "conf.h"
#include "ntp_sources.h"
#include "ntp_core.h"
#include "nts_ke.h"
#include "refclock.h"
#include "cmdmon.h"
#include "socket.h"
#include "srcparams.h"
#include "logging.h"
#include "nameserv.h"
#include "memory.h"
#include "cmdparse.h"
#include "util.h"

/* ================================================== */

#define MAX_LINE_LENGTH 2048
#define MAX_CONF_DIRS 10
#define MAX_INCLUDE_LEVEL 10

#define SSCANF_IN_RANGE(s, f, x, n, min, max) \
  (sscanf((s), (f), (x), (n)) == 1 && *(x) >= (min) && *(x) <= (max))

/* ================================================== */
/* Forward prototypes */

static void parse_string(char *line, char **result);
static void parse_int(char *line, int *result, int min, int max);
static void parse_double(char *line, double *result);
static void parse_null(char *line, int *result);
static void parse_ints(char *line, ARR_Instance array, int min, int max);

static void parse_allow_deny(char *line, ARR_Instance restrictions, int allow);
static void parse_authselectmode(char *);
static void parse_bindacqaddress(char *);
static void parse_bindaddress(char *);
static void parse_bindcmdaddress(char *);
static void parse_broadcast(char *);
static void parse_clientloglimit(char *);
static void parse_confdir(char *);
static void parse_driftfile(char *);
static void parse_fallbackdrift(char *);
static void parse_hwtimestamp(char *);
static void parse_include(char *);
static void parse_initstepslew(char *);
static void parse_leapsecmode(char *);
static void parse_local(char *);
static void parse_log(char *);
static void parse_mailonchange(char *);
static void parse_makestep(char *);
static void parse_maxchange(char *);
static void parse_ntsserver(char *, ARR_Instance files);
static void parse_ntstrustedcerts(char *);
static void parse_open_commands(char *line);
static void parse_pidfile(char *line);
static void parse_ratelimit(char *line, int *enabled, int *interval,
                            int *burst, int *leak, int *kod);
static void parse_refclock(char *);
static void parse_smoothtime(char *);
static void parse_source(char *line, char *type, int fatal);
static void parse_sourcedir(char *);
static void parse_tempcomp(char *);

/* ================================================== */
/* Configuration variables */

static int print_config = 0;
static int restarted = 0;
static char *rtc_device;
static int acquisition_port = -1;
static int ntp_port = NTP_PORT;
static char *keys_file = NULL;
static char *drift_file = NULL;
static int drift_file_interval = 3600;
static char *rtc_file = NULL;
static double max_update_skew = 1000.0;
static double correction_time_ratio = 3.0;
static double max_clock_error = 1.0; /* in ppm */
static double max_drift = 500000.0; /* in ppm */
static double max_slew_rate = 1e6 / 12.0; /* in ppm */
static double clock_precision = 0.0; /* in seconds */

static SRC_AuthSelectMode authselect_mode = SRC_AUTHSELECT_MIX;
static double max_distance = 3.0;
static double max_jitter = 1.0;
static double reselect_distance = 1e-4;
static double stratum_weight = 1e-3;
static double combine_limit = 3.0;

static int cmd_port = DEFAULT_CANDM_PORT;

static int raw_measurements = 0;
static int do_log_measurements = 0;
static int do_log_selection = 0;
static int do_log_statistics = 0;
static int do_log_tracking = 0;
static int do_log_rtc = 0;
static int do_log_refclocks = 0;
static int do_log_tempcomp = 0;
static int log_banner = 32;
static char *logdir = NULL;
static char *dumpdir = NULL;

static int enable_local=0;
static int local_stratum;
static int local_orphan;
static double local_distance;
static double local_activate;
static double local_wait_synced;
static double local_wait_unsynced;

/* Threshold (in seconds) - if absolute value of initial error is less
   than this, slew instead of stepping */
static double init_slew_threshold;
/* Array of IPAddr */
static ARR_Instance init_sources;

static int enable_manual=0;

/* Flag set if the RTC runs UTC (default is it runs local time
   incl. daylight saving). */
static int rtc_on_utc = 0;

/* Filename used to read the hwclock(8) LOCAL/UTC setting */
static char *hwclock_file;

/* Flag set if the RTC should be automatically synchronised by kernel */
static int rtc_sync = 0;

/* Limit and threshold for clock stepping */
static int make_step_limit = 0;
static double make_step_threshold = 0.0;

/* Threshold for automatic RTC trimming */
static double rtc_autotrim_threshold = 0.0;

/* Minimum number of selectables sources required to update the clock */
static int min_sources = 1;

/* Number of updates before offset checking, number of ignored updates
   before exiting and the maximum allowed offset */
static int max_offset_delay = -1;
static int max_offset_ignore;
static double max_offset;

/* Maximum and minimum number of samples per source */
static int max_samples = 0; /* no limit */
static int min_samples = 6;

/* Threshold for a time adjustment to be logged to syslog */
static double log_change_threshold = 1.0;

static char *mail_user_on_change = NULL;
static double mail_change_threshold = 0.0;

/* Flag indicating that we don't want to log clients, e.g. to save
   memory */
static int no_client_log = 0;

/* Limit memory allocated for the clients log */
static unsigned long client_log_limit = 524288;

/* Minimum and maximum fallback drift intervals */
static int fb_drift_min = 0;
static int fb_drift_max = 0;

/* IP addresses for binding the NTP server sockets to.  UNSPEC family means
   INADDR_ANY will be used */
static IPAddr bind_address4, bind_address6;

/* IP addresses for binding the NTP client sockets to.  UNSPEC family means
   INADDR_ANY will be used */
static IPAddr bind_acq_address4, bind_acq_address6;

/* IP addresses for binding the command socket to.  UNSPEC family means
   the loopback address will be used */
static IPAddr bind_cmd_address4, bind_cmd_address6;

/* Interface names to bind the NTP server, NTP client, and command socket */
static char *bind_ntp_iface = NULL;
static char *bind_acq_iface = NULL;
static char *bind_cmd_iface = NULL;

/* Path to the Unix domain command socket. */
static char *bind_cmd_path = NULL;

/* Differentiated Services Code Point (DSCP) in transmitted NTP packets */
static int ntp_dscp = 0;

/* Path to Samba (ntp_signd) socket. */
static char *ntp_signd_socket = NULL;

/* Filename to use for storing pid of running chronyd, to prevent multiple
 * chronyds being started. */
static char *pidfile = NULL;

/* Rate limiting parameters */
static int ntp_ratelimit_enabled = 0;
static int ntp_ratelimit_interval = 3;
static int ntp_ratelimit_burst = 8;
static int ntp_ratelimit_leak = 2;
static int ntp_ratelimit_kod = 0;
static int nts_ratelimit_enabled = 0;
static int nts_ratelimit_interval = 6;
static int nts_ratelimit_burst = 8;
static int nts_ratelimit_leak = 2;
static int cmd_ratelimit_enabled = 0;
static int cmd_ratelimit_interval = -4;
static int cmd_ratelimit_burst = 8;
static int cmd_ratelimit_leak = 2;

/* Smoothing constants */
static double smooth_max_freq = 0.0; /* in ppm */
static double smooth_max_wander = 0.0; /* in ppm/s */
static int smooth_leap_only = 0;

/* Temperature sensor, update interval and compensation coefficients */
static char *tempcomp_sensor_file = NULL;
static char *tempcomp_point_file = NULL;
static double tempcomp_interval;
static double tempcomp_T0, tempcomp_k0, tempcomp_k1, tempcomp_k2;

static int sched_priority = 0;
static int lock_memory = 0;

/* Leap second handling mode */
static REF_LeapMode leapsec_mode = REF_LeapModeSystem;

/* Name of a system timezone containing leap seconds occuring at midnight */
static char *leapsec_tz = NULL;

/* File name of leap seconds list, usually /usr/share/zoneinfo/leap-seconds.list */
static char *leapsec_list = NULL;

/* Name of the user to which will be dropped root privileges. */
static char *user;

/* Address refresh interval */
static int refresh = 1209600; /* 2 weeks */

#define DEFAULT_NTS_AEADS "30 15"

/* NTS server and client configuration */
static ARR_Instance nts_aeads; /* array of int */
static char *nts_dump_dir = NULL;
static char *nts_ntp_server = NULL;
static ARR_Instance nts_server_cert_files; /* array of (char *) */
static ARR_Instance nts_server_key_files; /* array of (char *) */
static int nts_server_port = NKE_PORT;
static int nts_server_processes = 1;
static int nts_server_connections = 100;
static int nts_refresh = 2419200; /* 4 weeks */
static int nts_rotate = 604800; /* 1 week */
static ARR_Instance nts_trusted_certs_paths; /* array of (char *) */
static ARR_Instance nts_trusted_certs_ids; /* array of uint32_t */

/* Number of clock updates needed to enable certificate time checks */
static int no_cert_time_check = 0;

/* Flag disabling use of system trusted certificates */
static int no_system_cert = 0;

/* Array of CNF_HwTsInterface */
static ARR_Instance hwts_interfaces;

/* Timeout for resuming reading from sockets waiting for HW TX timestamp */
static double hwts_timeout = 0.001;

/* PTP event port (disabled by default) */
static int ptp_port = 0;
/* PTP domain number of NTP-over-PTP messages */
static int ptp_domain = 123;

typedef struct {
  NTP_Source_Type type;
  int pool;
  CPS_NTP_Source params;
  NSR_Status status;
  uint32_t conf_id;
} NTP_Source;

/* Array of NTP_Source */
static ARR_Instance ntp_sources;
/* Array of (char *) */
static ARR_Instance ntp_source_dirs;
/* Flag indicating ntp_sources is used for sourcedirs after config load */
static int conf_ntp_sources_added = 0;

/* Array of RefclockParameters */
static ARR_Instance refclock_sources;

typedef struct _AllowDeny {
  IPAddr ip;
  int subnet_bits;
  int all; /* 1 to override existing more specific defns */
  int allow; /* 0 for deny, 1 for allow */
} AllowDeny;

/* Arrays of AllowDeny */
static ARR_Instance ntp_restrictions;
static ARR_Instance cmd_restrictions;

#define DEFAULT_OPEN_COMMANDS "activity manual rtcdata smoothing sourcename sources sourcestats tracking"

/* Array of int specifying commands allowed from network */
static ARR_Instance open_commands;

typedef struct {
  NTP_Remote_Address addr;
  int interval;
} NTP_Broadcast_Destination;

/* Array of NTP_Broadcast_Destination */
static ARR_Instance broadcasts;

/* ================================================== */

/* The line number in the configuration file being processed */
static int line_number;
static const char *processed_file;
static const char *processed_command;

static int include_level = 0;

/* ================================================== */

static void
command_parse_error(void)
{
    LOG_FATAL("Could not parse %s directive at line %d%s%s",
        processed_command, line_number, processed_file ? " in file " : "",
        processed_file ? processed_file : "");
}

/* ================================================== */

FORMAT_ATTRIBUTE_PRINTF(1, 2)
static void
other_parse_error(const char *format, ...)
{
  char buf[256];
  va_list ap;

  va_start(ap, format);
  vsnprintf(buf, sizeof (buf), format, ap);
  va_end(ap);

  LOG_FATAL("%s at line %d%s%s",
            buf, line_number, processed_file ? " in file " : "",
            processed_file ? processed_file : "");
}

/* ================================================== */

static int
get_number_of_args(char *line)
{
  int num = 0;

  /* The line is normalized, between arguments is just one space */
  if (*line == ' ')
    line++;
  if (*line)
    num++;
  for (; *line; line++) {
    if (*line == ' ')
      num++;
  }

  return num;
}

/* ================================================== */

static void
check_number_of_args(char *line, int num)
{
  num -= get_number_of_args(line);

  if (num) {
    LOG_FATAL("%s arguments for %s directive at line %d%s%s",
        num > 0 ? "Missing" : "Too many",
        processed_command, line_number, processed_file ? " in file " : "",
        processed_file ? processed_file : "");
  }
}

/* ================================================== */

void
CNF_Initialise(int r, int client_only)
{
  char buf[128];

  restarted = r;

  hwts_interfaces = ARR_CreateInstance(sizeof (CNF_HwTsInterface));

  init_sources = ARR_CreateInstance(sizeof (IPAddr));
  ntp_sources = ARR_CreateInstance(sizeof (NTP_Source));
  ntp_source_dirs = ARR_CreateInstance(sizeof (char *));
  refclock_sources = ARR_CreateInstance(sizeof (RefclockParameters));
  broadcasts = ARR_CreateInstance(sizeof (NTP_Broadcast_Destination));

  ntp_restrictions = ARR_CreateInstance(sizeof (AllowDeny));
  cmd_restrictions = ARR_CreateInstance(sizeof (AllowDeny));

  open_commands = ARR_CreateInstance(sizeof (int));
  snprintf(buf, sizeof (buf), DEFAULT_OPEN_COMMANDS);
  parse_open_commands(buf);

  nts_aeads = ARR_CreateInstance(sizeof (int));
  snprintf(buf, sizeof (buf), DEFAULT_NTS_AEADS);
  parse_ints(buf, nts_aeads, 0, INT_MAX);
  nts_server_cert_files = ARR_CreateInstance(sizeof (char *));
  nts_server_key_files = ARR_CreateInstance(sizeof (char *));
  nts_trusted_certs_paths = ARR_CreateInstance(sizeof (char *));
  nts_trusted_certs_ids = ARR_CreateInstance(sizeof (uint32_t));

  rtc_device = Strdup(DEFAULT_RTC_DEVICE);
  hwclock_file = Strdup(DEFAULT_HWCLOCK_FILE);
  user = Strdup(DEFAULT_USER);

  if (client_only) {
    cmd_port = ntp_port = 0;
  } else {
    bind_cmd_path = Strdup(DEFAULT_COMMAND_SOCKET);
    pidfile = Strdup(DEFAULT_PID_FILE);
  }

  SCK_GetAnyLocalIPAddress(IPADDR_INET4, &bind_address4);
  SCK_GetAnyLocalIPAddress(IPADDR_INET6, &bind_address6);
  SCK_GetAnyLocalIPAddress(IPADDR_INET4, &bind_acq_address4);
  SCK_GetAnyLocalIPAddress(IPADDR_INET6, &bind_acq_address6);
  SCK_GetLoopbackIPAddress(IPADDR_INET4, &bind_cmd_address4);
  SCK_GetLoopbackIPAddress(IPADDR_INET6, &bind_cmd_address6);
}

/* ================================================== */

void
CNF_Finalise(void)
{
  unsigned int i;

  for (i = 0; i < ARR_GetSize(hwts_interfaces); i++)
    Free(((CNF_HwTsInterface *)ARR_GetElement(hwts_interfaces, i))->name);
  ARR_DestroyInstance(hwts_interfaces);

  for (i = 0; i < ARR_GetSize(ntp_sources); i++)
    Free(((NTP_Source *)ARR_GetElement(ntp_sources, i))->params.name);
  for (i = 0; i < ARR_GetSize(ntp_source_dirs); i++)
    Free(*(char **)ARR_GetElement(ntp_source_dirs, i));
  for (i = 0; i < ARR_GetSize(refclock_sources); i++) {
    Free(((RefclockParameters *)ARR_GetElement(refclock_sources, i))->driver_name);
    Free(((RefclockParameters *)ARR_GetElement(refclock_sources, i))->driver_parameter);
  }
  for (i = 0; i < ARR_GetSize(nts_server_cert_files); i++)
    Free(*(char **)ARR_GetElement(nts_server_cert_files, i));
  for (i = 0; i < ARR_GetSize(nts_server_key_files); i++)
    Free(*(char **)ARR_GetElement(nts_server_key_files, i));
  for (i = 0; i < ARR_GetSize(nts_trusted_certs_paths); i++)
    Free(*(char **)ARR_GetElement(nts_trusted_certs_paths, i));

  ARR_DestroyInstance(init_sources);
  ARR_DestroyInstance(ntp_sources);
  ARR_DestroyInstance(ntp_source_dirs);
  ARR_DestroyInstance(refclock_sources);
  ARR_DestroyInstance(broadcasts);

  ARR_DestroyInstance(open_commands);

  ARR_DestroyInstance(ntp_restrictions);
  ARR_DestroyInstance(cmd_restrictions);

  ARR_DestroyInstance(nts_aeads);
  ARR_DestroyInstance(nts_server_cert_files);
  ARR_DestroyInstance(nts_server_key_files);
  ARR_DestroyInstance(nts_trusted_certs_paths);
  ARR_DestroyInstance(nts_trusted_certs_ids);

  Free(drift_file);
  Free(dumpdir);
  Free(hwclock_file);
  Free(keys_file);
  Free(leapsec_tz);
  Free(leapsec_list);
  Free(logdir);
  Free(bind_ntp_iface);
  Free(bind_acq_iface);
  Free(bind_cmd_iface);
  Free(bind_cmd_path);
  Free(ntp_signd_socket);
  Free(pidfile);
  Free(rtc_device);
  Free(rtc_file);
  Free(user);
  Free(mail_user_on_change);
  Free(tempcomp_sensor_file);
  Free(tempcomp_point_file);
  Free(nts_dump_dir);
  Free(nts_ntp_server);
}

/* ================================================== */

void
CNF_EnablePrint(void)
{
  print_config = 1;
}

/* ================================================== */

/* Read the configuration file */
void
CNF_ReadFile(const char *filename)
{
  FILE *in;
  char line[MAX_LINE_LENGTH + 1];
  int i;

  include_level++;
  if (include_level > MAX_INCLUDE_LEVEL)
    LOG_FATAL("Maximum include level reached");

  in = UTI_OpenFile(NULL, filename, NULL, 'R', 0);

  for (i = 1; fgets(line, sizeof(line), in); i++) {
    CNF_ParseLine(filename, i, line);
  }

  fclose(in);

  include_level--;
}

/* ================================================== */

/* Parse one configuration line */
void
CNF_ParseLine(const char *filename, int number, char *line)
{
  char *p, *command;

  /* Set global variables used in error messages */
  processed_file = filename;
  line_number = number;

  /* Detect truncated line */
  if (strlen(line) >= MAX_LINE_LENGTH)
    other_parse_error("String too long");

  /* Remove extra white-space and comments */
  CPS_NormalizeLine(line);

  /* Skip blank lines */
  if (!*line) {
    processed_file = NULL;
    return;
  }

  /* We have a real line, now try to match commands */
  processed_command = command = line;
  p = CPS_SplitWord(line);

  if (print_config && strcasecmp(command, "include") && strcasecmp(command, "confdir"))
    printf("%s%s%s\n", command, p[0] != '\0' ? " " : "", p);

  if (!strcasecmp(command, "acquisitionport")) {
    parse_int(p, &acquisition_port, 0, 65535);
  } else if (!strcasecmp(command, "allow")) {
    parse_allow_deny(p, ntp_restrictions, 1);
  } else if (!strcasecmp(command, "authselectmode")) {
    parse_authselectmode(p);
  } else if (!strcasecmp(command, "bindacqaddress")) {
    parse_bindacqaddress(p);
  } else if (!strcasecmp(command, "bindacqdevice")) {
    parse_string(p, &bind_acq_iface);
  } else if (!strcasecmp(command, "bindaddress")) {
    parse_bindaddress(p);
  } else if (!strcasecmp(command, "bindcmdaddress")) {
    parse_bindcmdaddress(p);
  } else if (!strcasecmp(command, "bindcmddevice")) {
    parse_string(p, &bind_cmd_iface);
  } else if (!strcasecmp(command, "binddevice")) {
    parse_string(p, &bind_ntp_iface);
  } else if (!strcasecmp(command, "broadcast")) {
    parse_broadcast(p);
  } else if (!strcasecmp(command, "clientloglimit")) {
    parse_clientloglimit(p);
  } else if (!strcasecmp(command, "clockprecision")) {
    parse_double(p, &clock_precision);
  } else if (!strcasecmp(command, "cmdallow")) {
    parse_allow_deny(p, cmd_restrictions, 1);
  } else if (!strcasecmp(command, "cmddeny")) {
    parse_allow_deny(p, cmd_restrictions, 0);
  } else if (!strcasecmp(command, "cmdport")) {
    parse_int(p, &cmd_port, 0, 65535);
  } else if (!strcasecmp(command, "cmdratelimit")) {
    parse_ratelimit(p, &cmd_ratelimit_enabled, &cmd_ratelimit_interval,
                    &cmd_ratelimit_burst, &cmd_ratelimit_leak, NULL);
  } else if (!strcasecmp(command, "combinelimit")) {
    parse_double(p, &combine_limit);
  } else if (!strcasecmp(command, "confdir")) {
    parse_confdir(p);
  } else if (!strcasecmp(command, "corrtimeratio")) {
    parse_double(p, &correction_time_ratio);
  } else if (!strcasecmp(command, "deny")) {
    parse_allow_deny(p, ntp_restrictions, 0);
  } else if (!strcasecmp(command, "driftfile")) {
    parse_driftfile(p);
  } else if (!strcasecmp(command, "dscp")) {
    parse_int(p, &ntp_dscp, 0, 63);
  } else if (!strcasecmp(command, "dumpdir")) {
    parse_string(p, &dumpdir);
  } else if (!strcasecmp(command, "dumponexit")) {
    /* Silently ignored */
  } else if (!strcasecmp(command, "fallbackdrift")) {
    parse_fallbackdrift(p);
  } else if (!strcasecmp(command, "hwclockfile")) {
    parse_string(p, &hwclock_file);
  } else if (!strcasecmp(command, "hwtimestamp")) {
    parse_hwtimestamp(p);
  } else if (!strcasecmp(command, "hwtstimeout")) {
    parse_double(p, &hwts_timeout);
  } else if (!strcasecmp(command, "include")) {
    parse_include(p);
  } else if (!strcasecmp(command, "initstepslew")) {
    parse_initstepslew(p);
  } else if (!strcasecmp(command, "keyfile")) {
    parse_string(p, &keys_file);
  } else if (!strcasecmp(command, "leapsecmode")) {
    parse_leapsecmode(p);
  } else if (!strcasecmp(command, "leapsectz")) {
    parse_string(p, &leapsec_tz);
  } else if (!strcasecmp(command, "leapseclist")) {
    parse_string(p, &leapsec_list);
  } else if (!strcasecmp(command, "local")) {
    parse_local(p);
  } else if (!strcasecmp(command, "lock_all")) {
    parse_null(p, &lock_memory);
  } else if (!strcasecmp(command, "log")) {
    parse_log(p);
  } else if (!strcasecmp(command, "logbanner")) {
    parse_int(p, &log_banner, 0, INT_MAX);
  } else if (!strcasecmp(command, "logchange")) {
    parse_double(p, &log_change_threshold);
  } else if (!strcasecmp(command, "logdir")) {
    parse_string(p, &logdir);
  } else if (!strcasecmp(command, "mailonchange")) {
    parse_mailonchange(p);
  } else if (!strcasecmp(command, "makestep")) {
    parse_makestep(p);
  } else if (!strcasecmp(command, "manual")) {
    parse_null(p, &enable_manual);
  } else if (!strcasecmp(command, "maxchange")) {
    parse_maxchange(p);
  } else if (!strcasecmp(command, "maxclockerror")) {
    parse_double(p, &max_clock_error);
  } else if (!strcasecmp(command, "maxdistance")) {
    parse_double(p, &max_distance);
  } else if (!strcasecmp(command, "maxdrift")) {
    parse_double(p, &max_drift);
  } else if (!strcasecmp(command, "maxjitter")) {
    parse_double(p, &max_jitter);
  } else if (!strcasecmp(command, "maxntsconnections")) {
    parse_int(p, &nts_server_connections, 1, INT_MAX);
  } else if (!strcasecmp(command, "maxsamples")) {
    parse_int(p, &max_samples, 0, INT_MAX);
  } else if (!strcasecmp(command, "maxslewrate")) {
    parse_double(p, &max_slew_rate);
  } else if (!strcasecmp(command, "maxupdateskew")) {
    parse_double(p, &max_update_skew);
  } else if (!strcasecmp(command, "minsamples")) {
    parse_int(p, &min_samples, 0, INT_MAX);
  } else if (!strcasecmp(command, "minsources")) {
    parse_int(p, &min_sources, 1, INT_MAX);
  } else if (!strcasecmp(command, "nocerttimecheck")) {
    parse_int(p, &no_cert_time_check, 0, INT_MAX);
  } else if (!strcasecmp(command, "noclientlog")) {
    parse_null(p, &no_client_log);
  } else if (!strcasecmp(command, "nosystemcert")) {
    parse_null(p, &no_system_cert);
  } else if (!strcasecmp(command, "ntpsigndsocket")) {
    parse_string(p, &ntp_signd_socket);
  } else if (!strcasecmp(command, "ntsaeads")) {
    parse_ints(p, nts_aeads, 0, INT_MAX);
  } else if (!strcasecmp(command, "ntsratelimit")) {
    parse_ratelimit(p, &nts_ratelimit_enabled, &nts_ratelimit_interval,
                    &nts_ratelimit_burst, &nts_ratelimit_leak, NULL);
  } else if (!strcasecmp(command, "ntscachedir") ||
             !strcasecmp(command, "ntsdumpdir")) {
    parse_string(p, &nts_dump_dir);
  } else if (!strcasecmp(command, "ntsntpserver")) {
    parse_string(p, &nts_ntp_server);
  } else if (!strcasecmp(command, "ntsport")) {
    parse_int(p, &nts_server_port, 0, 65535);
  } else if (!strcasecmp(command, "ntsprocesses")) {
    parse_int(p, &nts_server_processes, 0, 1000);
  } else if (!strcasecmp(command, "ntsrefresh")) {
    parse_int(p, &nts_refresh, 0, INT_MAX);
  } else if (!strcasecmp(command, "ntsrotate")) {
    parse_int(p, &nts_rotate, 0, INT_MAX);
  } else if (!strcasecmp(command, "ntsservercert")) {
    parse_ntsserver(p, nts_server_cert_files);
  } else if (!strcasecmp(command, "ntsserverkey")) {
    parse_ntsserver(p, nts_server_key_files);
  } else if (!strcasecmp(command, "ntstrustedcerts")) {
    parse_ntstrustedcerts(p);
  } else if (!strcasecmp(command, "opencommands")) {
    parse_open_commands(p);
  } else if (!strcasecmp(command, "peer")) {
    parse_source(p, command, 1);
  } else if (!strcasecmp(command, "pidfile")) {
    parse_pidfile(p);
  } else if (!strcasecmp(command, "pool")) {
    parse_source(p, command, 1);
  } else if (!strcasecmp(command, "port")) {
    parse_int(p, &ntp_port, 0, 65535);
  } else if (!strcasecmp(command, "ptpdomain")) {
    parse_int(p, &ptp_domain, 0, 255);
  } else if (!strcasecmp(command, "ptpport")) {
    parse_int(p, &ptp_port, 0, 65535);
  } else if (!strcasecmp(command, "ratelimit")) {
    parse_ratelimit(p, &ntp_ratelimit_enabled, &ntp_ratelimit_interval,
                    &ntp_ratelimit_burst, &ntp_ratelimit_leak, &ntp_ratelimit_kod);
  } else if (!strcasecmp(command, "refclock")) {
    parse_refclock(p);
  } else if (!strcasecmp(command, "refresh")) {
    parse_int(p, &refresh, 0, INT_MAX);
  } else if (!strcasecmp(command, "reselectdist")) {
    parse_double(p, &reselect_distance);
  } else if (!strcasecmp(command, "rtcautotrim")) {
    parse_double(p, &rtc_autotrim_threshold);
  } else if (!strcasecmp(command, "rtcdevice")) {
    parse_string(p, &rtc_device);
  } else if (!strcasecmp(command, "rtcfile")) {
    parse_string(p, &rtc_file);
  } else if (!strcasecmp(command, "rtconutc")) {
    parse_null(p, &rtc_on_utc);
  } else if (!strcasecmp(command, "rtcsync")) {
    parse_null(p, &rtc_sync);
  } else if (!strcasecmp(command, "sched_priority")) {
    parse_int(p, &sched_priority, 0, 100);
  } else if (!strcasecmp(command, "server")) {
    parse_source(p, command, 1);
  } else if (!strcasecmp(command, "smoothtime")) {
    parse_smoothtime(p);
  } else if (!strcasecmp(command, "sourcedir")) {
    parse_sourcedir(p);
  } else if (!strcasecmp(command, "stratumweight")) {
    parse_double(p, &stratum_weight);
  } else if (!strcasecmp(command, "tempcomp")) {
    parse_tempcomp(p);
  } else if (!strcasecmp(command, "user")) {
    parse_string(p, &user);
  } else if (!strcasecmp(command, "commandkey") ||
             !strcasecmp(command, "generatecommandkey") ||
             !strcasecmp(command, "linux_freq_scale") ||
             !strcasecmp(command, "linux_hz")) {
    LOG(LOGS_WARN, "%s directive is no longer supported", command);
  } else {
    other_parse_error("Invalid directive %s", command);
  }

  processed_file = processed_command = NULL;
}

/* ================================================== */

static void
parse_string(char *line, char **result)
{
  check_number_of_args(line, 1);
  Free(*result);
  *result = Strdup(line);
}

/* ================================================== */

static void
parse_int(char *line, int *result, int min, int max)
{
  char *end;
  long r;

  check_number_of_args(line, 1);

  errno = 0;
  r = strtol(line, &end, 10);
  if (errno != 0 || *end != '\0')
    command_parse_error();
  if (r < min || r > max)
    other_parse_error("Invalid value %ld in %s directive (min %d, max %d)",
                      r, processed_command, min, max);
  *result = r;
}

/* ================================================== */

static void
parse_double(char *line, double *result)
{
  check_number_of_args(line, 1);
  if (sscanf(line, "%lf", result) != 1) {
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_null(char *line, int *result)
{
  check_number_of_args(line, 0);
  *result = 1;
}

/* ================================================== */

static void
parse_ints(char *line, ARR_Instance array, int min, int max)
{
  char *s;
  int v;

  ARR_SetSize(array, 0);

  while (*line) {
    s = line;
    line = CPS_SplitWord(line);
    parse_int(s, &v, min, max);
    ARR_AppendElement(array, &v);
  }
}

/* ================================================== */

static void
parse_source(char *line, char *type, int fatal)
{
  CPS_Status status;
  NTP_Source source;

  if (strcasecmp(type, "peer") == 0) {
    source.type = NTP_PEER;
    source.pool = 0;
  } else if (strcasecmp(type, "pool") == 0) {
    source.type = NTP_SERVER;
    source.pool = 1;
  } else if (strcasecmp(type, "server") == 0) {
    source.type = NTP_SERVER;
    source.pool = 0;
  } else {
    if (fatal)
      command_parse_error();
    return;
  }

  /* Avoid comparing uninitialized data in compare_sources() */
  memset(&source.params, 0, sizeof (source.params));

  status = CPS_ParseNTPSourceAdd(line, &source.params);
  if (status != CPS_Success) {
    if (fatal) {
      other_parse_error("Invalid %s %s directive",
                        status == CPS_InvalidOption ? "option in" :
                        status == CPS_InvalidValue ? "value in" : "syntax for", type);
    }
    return;
  }

  source.params.name = Strdup(source.params.name);
  source.status = NSR_NoSuchSource;
  source.conf_id = 0;

  ARR_AppendElement(ntp_sources, &source);
}

/* ================================================== */

static void
parse_sourcedir(char *line)
{
  char *s;

  s = Strdup(line);
  ARR_AppendElement(ntp_source_dirs, &s);
}

/* ================================================== */

static void
parse_ratelimit(char *line, int *enabled, int *interval, int *burst, int *leak, int *kod)
{
  int n, val;
  char *opt;

  *enabled = 1;

  while (*line) {
    opt = line;
    line = CPS_SplitWord(line);
    if (!SSCANF_IN_RANGE(line, "%d%n", &val, &n, -32, 32)) {
      command_parse_error();
      return;
    }
    line += n;
    if (!strcasecmp(opt, "interval"))
      *interval = val;
    else if (!strcasecmp(opt, "burst"))
      *burst = val;
    else if (!strcasecmp(opt, "leak"))
      *leak = val;
    else if (!strcasecmp(opt, "kod") && kod)
      *kod = val;
    else
      command_parse_error();
  }
}

/* ================================================== */

static void
parse_refclock(char *line)
{
  int n, poll, dpoll, filter_length, pps_rate, min_samples, max_samples, sel_options;
  int local, max_lock_age, pps_forced, sel_option, stratum, tai;
  uint32_t ref_id, lock_ref_id;
  double offset, delay, precision, max_dispersion, pulse_width;
  char *p, *cmd, *name, *param;
  RefclockParameters *refclock;

  poll = 4;
  dpoll = 0;
  filter_length = 64;
  local = 0;
  pps_forced = 0;
  pps_rate = 0;
  min_samples = SRC_DEFAULT_MINSAMPLES;
  max_samples = SRC_DEFAULT_MAXSAMPLES;
  sel_options = 0;
  offset = 0.0;
  delay = 1e-9;
  precision = 0.0;
  max_dispersion = 0.0;
  pulse_width = 0.0;
  ref_id = 0;
  max_lock_age = 2;
  lock_ref_id = 0;
  stratum = 0;
  tai = 0;

  if (!*line) {
    command_parse_error();
    return;
  }

  p = line;
  line = CPS_SplitWord(line);

  if (!*line) {
    command_parse_error();
    return;
  }

  name = Strdup(p);

  p = line;
  line = CPS_SplitWord(line);
  param = Strdup(p);

  for (cmd = line; *cmd; line += n, cmd = line) {
    line = CPS_SplitWord(line);

    if (!strcasecmp(cmd, "refid")) {
      if ((n = CPS_ParseRefid(line, &ref_id)) == 0)
        break;
    } else if (!strcasecmp(cmd, "lock")) {
      if ((n = CPS_ParseRefid(line, &lock_ref_id)) == 0)
        break;
    } else if (!strcasecmp(cmd, "poll")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &poll, &n, -32, 32))
        break;
    } else if (!strcasecmp(cmd, "dpoll")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &dpoll, &n, -32, 32))
        break;
    } else if (!strcasecmp(cmd, "filter")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &filter_length, &n, 0, INT_MAX))
        break;
    } else if (!strcasecmp(cmd, "local")) {
      n = 0;
      local = 1;
    } else if (!strcasecmp(cmd, "rate")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &pps_rate, &n, 1, INT_MAX))
        break;
    } else if (!strcasecmp(cmd, "minsamples")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &min_samples, &n, 0, INT_MAX))
        break;
    } else if (!strcasecmp(cmd, "maxlockage")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &max_lock_age, &n, 0, INT_MAX))
        break;
    } else if (!strcasecmp(cmd, "maxsamples")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &max_samples, &n, 0, INT_MAX))
        break;
    } else if (!strcasecmp(cmd, "offset")) {
      if (sscanf(line, "%lf%n", &offset, &n) != 1)
        break;
    } else if (!strcasecmp(cmd, "delay")) {
      if (sscanf(line, "%lf%n", &delay, &n) != 1)
        break;
    } else if (!strcasecmp(cmd, "pps")) {
      n = 0;
      pps_forced = 1;
    } else if (!strcasecmp(cmd, "precision")) {
      if (sscanf(line, "%lf%n", &precision, &n) != 1)
        break;
    } else if (!strcasecmp(cmd, "maxdispersion")) {
      if (sscanf(line, "%lf%n", &max_dispersion, &n) != 1)
        break;
    } else if (!strcasecmp(cmd, "stratum")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &stratum, &n, 0, NTP_MAX_STRATUM - 1))
        break;
    } else if (!strcasecmp(cmd, "tai")) {
      n = 0;
      tai = 1;
    } else if (!strcasecmp(cmd, "width")) {
      if (sscanf(line, "%lf%n", &pulse_width, &n) != 1)
        break;
    } else if ((sel_option = CPS_GetSelectOption(cmd)) != 0) {
      n = 0;
      sel_options |= sel_option;
    } else {
      other_parse_error("Invalid %s %s directive", "option in", processed_command);
      return;
    }
  }

  if (*cmd) {
    other_parse_error("Invalid %s %s directive", "value in", processed_command);
    return;
  }

  refclock = (RefclockParameters *)ARR_GetNewElement(refclock_sources);
  refclock->driver_name = name;
  refclock->driver_parameter = param;
  refclock->driver_poll = dpoll;
  refclock->poll = poll;
  refclock->filter_length = filter_length;
  refclock->local = local;
  refclock->pps_forced = pps_forced;
  refclock->pps_rate = pps_rate;
  refclock->min_samples = min_samples;
  refclock->max_samples = max_samples;
  refclock->sel_options = sel_options;
  refclock->stratum = stratum;
  refclock->tai = tai;
  refclock->offset = offset;
  refclock->delay = delay;
  refclock->precision = precision;
  refclock->max_dispersion = max_dispersion;
  refclock->pulse_width = pulse_width;
  refclock->ref_id = ref_id;
  refclock->max_lock_age = max_lock_age;
  refclock->lock_ref_id = lock_ref_id;
}

/* ================================================== */

static void
parse_log(char *line)
{
  char *log_name;
  do {
    log_name = line;
    line = CPS_SplitWord(line);
    if (*log_name) {
      if (!strcmp(log_name, "rawmeasurements")) {
        do_log_measurements = 1;
        raw_measurements = 1;
      } else if (!strcmp(log_name, "measurements")) {
        do_log_measurements = 1;
      } else if (!strcmp(log_name, "selection")) {
        do_log_selection = 1;
      } else if (!strcmp(log_name, "statistics")) {
        do_log_statistics = 1;
      } else if (!strcmp(log_name, "tracking")) {
        do_log_tracking = 1;
      } else if (!strcmp(log_name, "rtc")) {
        do_log_rtc = 1;
      } else if (!strcmp(log_name, "refclocks")) {
        do_log_refclocks = 1;
      } else if (!strcmp(log_name, "tempcomp")) {
        do_log_tempcomp = 1;
      } else {
        other_parse_error("Invalid log parameter");
        break;
      }
    } else {
      break;
    }
  } while (1);
}

/* ================================================== */

static void
parse_local(char *line)
{
  CPS_Status status;

  status = CPS_ParseLocal(line, &local_stratum, &local_orphan, &local_distance,
                          &local_activate, &local_wait_synced, &local_wait_unsynced);
  if (status != CPS_Success) {
    other_parse_error("Invalid %s %s directive",
                      status == CPS_InvalidOption ? "option in" : "value in",
                      processed_command);
  }

  enable_local = 1;
}

/* ================================================== */

static void
parse_initstepslew(char *line)
{
  char *p, *hostname;
  IPAddr ip_addr;

  /* Ignore the line if chronyd was started with -R. */
  if (restarted) {
    return;
  }

  ARR_SetSize(init_sources, 0);
  p = CPS_SplitWord(line);

  if (sscanf(line, "%lf", &init_slew_threshold) != 1) {
    command_parse_error();
    return;
  }

  while (*p) {
    hostname = p;
    p = CPS_SplitWord(p);
    if (*hostname) {
      if (DNS_Name2IPAddress(hostname, &ip_addr, 1) == DNS_Success) {
        ARR_AppendElement(init_sources, &ip_addr);
      } else {
        LOG(LOGS_WARN, "Could not resolve address of initstepslew server %s", hostname);
      }
    }
  }
}

/* ================================================== */

static void
parse_leapsecmode(char *line)
{
  if (!strcasecmp(line, "system"))
    leapsec_mode = REF_LeapModeSystem;
  else if (!strcasecmp(line, "slew"))
    leapsec_mode = REF_LeapModeSlew;
  else if (!strcasecmp(line, "step"))
    leapsec_mode = REF_LeapModeStep;
  else if (!strcasecmp(line, "ignore"))
    leapsec_mode = REF_LeapModeIgnore;
  else
    command_parse_error();
}

/* ================================================== */

static void
parse_clientloglimit(char *line)
{
  check_number_of_args(line, 1);
  if (sscanf(line, "%lu", &client_log_limit) != 1) {
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_fallbackdrift(char *line)
{
  check_number_of_args(line, 2);
  if (sscanf(line, "%d %d", &fb_drift_min, &fb_drift_max) != 2) {
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_makestep(char *line)
{
  check_number_of_args(line, 2);
  if (sscanf(line, "%lf %d", &make_step_threshold, &make_step_limit) != 2) {
    make_step_limit = 0;
    command_parse_error();
  }

  /* Disable limited makestep if chronyd was started with -R. */
  if (restarted && make_step_limit > 0) {
    make_step_limit = 0;
  }
}

/* ================================================== */

static void
parse_maxchange(char *line)
{
  check_number_of_args(line, 3);
  if (sscanf(line, "%lf %d %d", &max_offset, &max_offset_delay, &max_offset_ignore) != 3) {
    max_offset_delay = -1;
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_mailonchange(char *line)
{
  char *address;
  check_number_of_args(line, 2);
  address = line;
  line = CPS_SplitWord(line);
  Free(mail_user_on_change);
  if (sscanf(line, "%lf", &mail_change_threshold) == 1) {
    mail_user_on_change = Strdup(address);
  } else {
    mail_user_on_change = NULL;
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_ntsserver(char *line, ARR_Instance files)
{
  char *file = NULL;

  parse_string(line, &file);
  ARR_AppendElement(files, &file);
}

/* ================================================== */

static void
parse_ntstrustedcerts(char *line)
{
  uint32_t id;
  char *path;

  if (get_number_of_args(line) == 2) {
    path = CPS_SplitWord(line);
    if (sscanf(line, "%"SCNu32, &id) != 1)
      command_parse_error();
  } else {
    check_number_of_args(line, 1);
    path = line;
    id = 0;
  }

  path = Strdup(path);

  ARR_AppendElement(nts_trusted_certs_paths, &path);
  ARR_AppendElement(nts_trusted_certs_ids, &id);
}

/* ================================================== */

static void
add_open_command(int command)
{
  int i;

  /* Avoid duplicates */
  for (i = 0; i < ARR_GetSize(open_commands); i++) {
    if (*(int *)ARR_GetElement(open_commands, i) == command)
      return;
  }

  ARR_AppendElement(open_commands, &command);
}

/* ================================================== */

static void
parse_open_commands(char *line)
{
  char *s;

  ARR_SetSize(open_commands, 0);

  while (*line) {
    s = line;
    line = CPS_SplitWord(line);

    if (strcasecmp(s, "activity") == 0) {
      add_open_command(REQ_ACTIVITY);
    } else if (strcasecmp(s, "authdata") == 0) {
      add_open_command(REQ_N_SOURCES);
      add_open_command(REQ_AUTH_DATA);
    } else if (strcasecmp(s, "clients") == 0) {
      add_open_command(REQ_CLIENT_ACCESSES_BY_INDEX3);
    } else if (strcasecmp(s, "manual") == 0) {
      add_open_command(REQ_MANUAL_LIST);
    } else if (strcasecmp(s, "ntpdata") == 0) {
      add_open_command(REQ_N_SOURCES);
      add_open_command(REQ_NTP_DATA);
    } else if (strcasecmp(s, "rtcdata") == 0) {
      add_open_command(REQ_RTCREPORT);
    } else if (strcasecmp(s, "selectdata") == 0) {
      add_open_command(REQ_N_SOURCES);
      add_open_command(REQ_SELECT_DATA);
    } else if (strcasecmp(s, "serverstats") == 0) {
      add_open_command(REQ_SERVER_STATS);
    } else if (strcasecmp(s, "smoothing") == 0) {
      add_open_command(REQ_SMOOTHING);
    } else if (strcasecmp(s, "sourcename") == 0) {
      add_open_command(REQ_NTP_SOURCE_NAME);
    } else if (strcasecmp(s, "sources") == 0) {
      add_open_command(REQ_N_SOURCES);
      add_open_command(REQ_SOURCE_DATA);
    } else if (strcasecmp(s, "sourcestats") == 0) {
      add_open_command(REQ_N_SOURCES);
      add_open_command(REQ_SOURCESTATS);
    } else if (strcasecmp(s, "tracking") == 0) {
      add_open_command(REQ_TRACKING);
    } else {
      command_parse_error();
    }
  }
}

/* ================================================== */

static void
parse_allow_deny(char *line, ARR_Instance restrictions, int allow)
{
  int all, subnet_bits;
  AllowDeny *node;
  IPAddr ip;

  if (!CPS_ParseAllowDeny(line, &all, &ip, &subnet_bits))
    command_parse_error();

  node = ARR_GetNewElement(restrictions);
  node->allow = allow;
  node->all = all;
  node->ip = ip;
  node->subnet_bits = subnet_bits;
}
  
/* ================================================== */

static void
parse_authselectmode(char *line)
{
  if (!strcasecmp(line, "require"))
    authselect_mode = SRC_AUTHSELECT_REQUIRE;
  else if (!strcasecmp(line, "prefer"))
    authselect_mode = SRC_AUTHSELECT_PREFER;
  else if (!strcasecmp(line, "mix"))
    authselect_mode = SRC_AUTHSELECT_MIX;
  else if (!strcasecmp(line, "ignore"))
    authselect_mode = SRC_AUTHSELECT_IGNORE;
  else
    command_parse_error();
}

/* ================================================== */

static void
parse_bindacqaddress(char *line)
{
  IPAddr ip;

  check_number_of_args(line, 1);
  if (UTI_StringToIP(line, &ip)) {
    if (ip.family == IPADDR_INET4)
      bind_acq_address4 = ip;
    else if (ip.family == IPADDR_INET6)
      bind_acq_address6 = ip;
  } else {
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_bindaddress(char *line)
{
  IPAddr ip;

  check_number_of_args(line, 1);
  if (UTI_StringToIP(line, &ip)) {
    if (ip.family == IPADDR_INET4)
      bind_address4 = ip;
    else if (ip.family == IPADDR_INET6)
      bind_address6 = ip;
  } else {
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_bindcmdaddress(char *line)
{
  IPAddr ip;

  check_number_of_args(line, 1);

  /* Address starting with / is for the Unix domain socket */
  if (line[0] == '/') {
    parse_string(line, &bind_cmd_path);
    /* / disables the socket */
    if (strcmp(bind_cmd_path, "/") == 0) {
      Free(bind_cmd_path);
      bind_cmd_path = NULL;
    }
  } else if (UTI_StringToIP(line, &ip)) {
    if (ip.family == IPADDR_INET4)
      bind_cmd_address4 = ip;
    else if (ip.family == IPADDR_INET6)
      bind_cmd_address6 = ip;
  } else {
    command_parse_error();
  }
}

/* ================================================== */

static void
parse_broadcast(char *line)
{
  /* Syntax : broadcast <interval> <broadcast-IP-addr> [<port>] */
  NTP_Broadcast_Destination *destination;
  int port;
  int interval;
  char *p;
  IPAddr ip;
  
  p = line;
  line = CPS_SplitWord(line);

  if (sscanf(p, "%d", &interval) != 1) {
    command_parse_error();
    return;
  }

  p = line;
  line = CPS_SplitWord(line);

  if (!UTI_StringToIP(p, &ip)) {
    command_parse_error();
    return;
  }

  p = line;
  line = CPS_SplitWord(line);

  if (*p) {
    if (sscanf(p, "%d", &port) != 1 || *line) {
      command_parse_error();
      return;
    }
  } else {
    /* default port */
    port = NTP_PORT;
  }

  destination = (NTP_Broadcast_Destination *)ARR_GetNewElement(broadcasts);
  destination->addr.ip_addr = ip;
  destination->addr.port = port;
  destination->interval = interval;
}

/* ================================================== */

static void
parse_smoothtime(char *line)
{
  if (get_number_of_args(line) != 3)
    check_number_of_args(line, 2);

  if (sscanf(line, "%lf %lf", &smooth_max_freq, &smooth_max_wander) != 2) {
    smooth_max_freq = 0.0;
    command_parse_error();
  }

  line = CPS_SplitWord(CPS_SplitWord(line));
  smooth_leap_only = 0;

  if (*line) {
    if (!strcasecmp(line, "leaponly"))
      smooth_leap_only = 1;
    else
      command_parse_error();
  }
}

/* ================================================== */
static void
parse_tempcomp(char *line)
{
  char *p;
  int point_form;

  point_form = get_number_of_args(line) == 3;

  if (!point_form)
    check_number_of_args(line, 6);

  p = line;
  line = CPS_SplitWord(line);

  if (!*p) {
    command_parse_error();
    return;
  }

  Free(tempcomp_point_file);

  if (point_form) {
    if (sscanf(line, "%lf", &tempcomp_interval) != 1) {
      command_parse_error();
      return;
    }
    tempcomp_point_file = Strdup(CPS_SplitWord(line));
  } else {
    if (sscanf(line, "%lf %lf %lf %lf %lf", &tempcomp_interval,
               &tempcomp_T0, &tempcomp_k0, &tempcomp_k1, &tempcomp_k2) != 5) {
      command_parse_error();
      return;
    }
    tempcomp_point_file = NULL;
  }

  Free(tempcomp_sensor_file);
  tempcomp_sensor_file = Strdup(p);
}

/* ================================================== */

static void
parse_hwtimestamp(char *line)
{
  CNF_HwTsInterface *iface;
  int n, maxpoll_set = 0;
  char *p, filter[5];

  if (!*line) {
    command_parse_error();
    return;
  }

  p = line;
  line = CPS_SplitWord(line);

  iface = ARR_GetNewElement(hwts_interfaces);
  iface->name = Strdup(p);
  iface->minpoll = 0;
  iface->min_samples = 2;
  iface->max_samples = 16;
  iface->nocrossts = 0;
  iface->rxfilter = CNF_HWTS_RXFILTER_ANY;
  iface->precision = 100.0e-9;
  iface->tx_comp = 0.0;
  iface->rx_comp = 0.0;

  for (p = line; *p; line += n, p = line) {
    line = CPS_SplitWord(line);

    if (!strcasecmp(p, "maxsamples")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &iface->max_samples, &n, 0, INT_MAX))
        break;
    } else if (!strcasecmp(p, "minpoll")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &iface->minpoll, &n, -32, 32))
        break;
    } else if (!strcasecmp(p, "maxpoll")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &iface->maxpoll, &n, -32, 32))
        break;
      maxpoll_set = 1;
    } else if (!strcasecmp(p, "minsamples")) {
      if (!SSCANF_IN_RANGE(line, "%d%n", &iface->min_samples, &n, 0, INT_MAX))
        break;
    } else if (!strcasecmp(p, "precision")) {
      if (sscanf(line, "%lf%n", &iface->precision, &n) != 1)
        break;
    } else if (!strcasecmp(p, "rxcomp")) {
      if (sscanf(line, "%lf%n", &iface->rx_comp, &n) != 1)
        break;
    } else if (!strcasecmp(p, "txcomp")) {
      if (sscanf(line, "%lf%n", &iface->tx_comp, &n) != 1)
        break;
    } else if (!strcasecmp(p, "rxfilter")) {
      if (sscanf(line, "%4s%n", filter, &n) != 1)
        break;
      if (!strcasecmp(filter, "none"))
        iface->rxfilter = CNF_HWTS_RXFILTER_NONE;
      else if (!strcasecmp(filter, "ntp"))
        iface->rxfilter = CNF_HWTS_RXFILTER_NTP;
      else if (!strcasecmp(filter, "ptp"))
        iface->rxfilter = CNF_HWTS_RXFILTER_PTP;
      else if (!strcasecmp(filter, "all"))
        iface->rxfilter = CNF_HWTS_RXFILTER_ALL;
      else
        break;
    } else if (!strcasecmp(p, "nocrossts")) {
      n = 0;
      iface->nocrossts = 1;
    } else {
      break;
    }
  }

  if (*p)
    command_parse_error();

  if (!maxpoll_set)
    iface->maxpoll = iface->minpoll + 1;
}

/* ================================================== */

static void
parse_pidfile(char *line)
{
  parse_string(line, &pidfile);

  /* / disables the PID file handling */
  if (strcmp(pidfile, "/") == 0) {
    Free(pidfile);
    pidfile = NULL;
  }
}

/* ================================================== */

static const char *
get_basename(const char *path)
{
  const char *b = strrchr(path, '/');
  return b ? b + 1 : path;
}

/* ================================================== */

static int
compare_basenames(const void *a, const void *b)
{
  return strcmp(get_basename(*(const char * const *)a),
                get_basename(*(const char * const *)b));
}

/* ================================================== */

static int
search_dirs(char *line, const char *suffix, void (*file_handler)(const char *path))
{
  char *dirs[MAX_CONF_DIRS], buf[MAX_LINE_LENGTH], *path;
  size_t i, j, k, locations, n_dirs;
  glob_t gl;

  n_dirs = UTI_SplitString(line, dirs, MAX_CONF_DIRS);
  if (n_dirs < 1 || n_dirs > MAX_CONF_DIRS)
    return 0;

  /* Get the paths of all config files in the specified directories */
  for (i = 0; i < n_dirs; i++) {
    if (snprintf(buf, sizeof (buf), "%s/*%s", dirs[i], suffix) >= sizeof (buf))
      assert(0);
    if (glob(buf, GLOB_NOSORT | (i > 0 ? GLOB_APPEND : 0), NULL, &gl) != 0)
      ;
  }

  if (gl.gl_pathc > 0) {
    /* Sort the paths by filenames */
    qsort(gl.gl_pathv, gl.gl_pathc, sizeof (gl.gl_pathv[0]), compare_basenames);

    for (i = 0; i < gl.gl_pathc; i += locations) {
      /* Count directories containing files with this name */
      for (j = i + 1, locations = 1; j < gl.gl_pathc; j++, locations++) {
        if (compare_basenames(&gl.gl_pathv[i], &gl.gl_pathv[j]) != 0)
          break;
      }

      /* Read the first file of this name in the order of the directive */
      for (j = 0; j < n_dirs; j++) {
        for (k = 0; k < locations; k++) {
          path = gl.gl_pathv[i + k];
          if (strncmp(path, dirs[j], strlen(dirs[j])) == 0 &&
              strlen(dirs[j]) + 1 + strlen(get_basename(path)) == strlen(path)) {
            file_handler(path);
            break;
          }
        }
        if (k < locations)
          break;
      }
    }
  }

  globfree(&gl);

  return 1;
}

/* ================================================== */

static void
parse_confdir(char *line)
{
  if (!search_dirs(line, ".conf", CNF_ReadFile))
    command_parse_error();
}

/* ================================================== */

static void
parse_driftfile(char *line)
{
  char *path, *opt, *val;

  path = line;
  opt = CPS_SplitWord(path);
  val = CPS_SplitWord(opt);

  if (*path == '\0' ||
      (*opt != '\0' && (strcasecmp(opt, "interval") != 0 ||
                        sscanf(val, "%d", &drift_file_interval) != 1 ||
                        *CPS_SplitWord(val) != '\0'))) {
    command_parse_error();
    return;
  }

  Free(drift_file);
  drift_file = Strdup(path);
}

/* ================================================== */

static void
parse_include(char *line)
{
  glob_t gl;
  size_t i;
  int r;

  check_number_of_args(line, 1);

  if ((r = glob(line,
#ifdef GLOB_NOMAGIC
                GLOB_NOMAGIC |
#endif
                GLOB_ERR, NULL, &gl)) != 0) {
    if (r != GLOB_NOMATCH)
      LOG_FATAL("Could not search for files matching %s", line);

    DEBUG_LOG("glob of %s failed", line);
    return;
  }

  for (i = 0; i < gl.gl_pathc; i++)
    CNF_ReadFile(gl.gl_pathv[i]);

  globfree(&gl);
}

/* ================================================== */

static void
load_source_file(const char *filename)
{
  char line[MAX_LINE_LENGTH + 1];
  FILE *f;

  f = UTI_OpenFile(NULL, filename, NULL, 'r', 0);
  if (!f)
    return;

  while (fgets(line, sizeof (line), f)) {
    /* Require lines to be terminated */
    if (line[0] == '\0' || line[strlen(line) - 1] != '\n')
      break;

    CPS_NormalizeLine(line);
    if (line[0] == '\0')
      continue;

    parse_source(CPS_SplitWord(line), line, 0);
  }

  fclose(f);
}

/* ================================================== */

static int
compare_sources(const void *a, const void *b)
{
  const NTP_Source *sa = a, *sb = b;
  int d;

  if (!sa->params.name)
    return -1;
  if (!sb->params.name)
    return 1;
  if ((d = strcmp(sa->params.name, sb->params.name)) != 0)
    return d;
  if ((d = (int)sa->type - (int)sb->type) != 0)
    return d;
  if ((d = (int)sa->pool - (int)sb->pool) != 0)
    return d;
  if ((d = (int)sa->params.family - (int)sb->params.family) != 0)
    return d;
  if ((d = (int)sa->params.port - (int)sb->params.port) != 0)
    return d;
  return memcmp(&sa->params.params, &sb->params.params, sizeof (sa->params.params));
}

/* ================================================== */

static void
reload_source_dirs(void)
{
  NTP_Source *prev_sources, *new_sources, *source;
  unsigned int i, j, prev_size, new_size, unresolved;
  char buf[MAX_LINE_LENGTH];
  int d, pass, was_added;
  NSR_Status s;

  /* Ignore reload command before adding configured sources */
  if (!conf_ntp_sources_added)
    return;

  prev_size = ARR_GetSize(ntp_sources);

  /* Save the current sources */
  prev_sources = MallocArray(NTP_Source, prev_size);
  memcpy(prev_sources, ARR_GetElements(ntp_sources), prev_size * sizeof (prev_sources[0]));

  /* Load the sources again */
  ARR_SetSize(ntp_sources, 0);
  for (i = 0; i < ARR_GetSize(ntp_source_dirs); i++) {
    if (snprintf(buf, sizeof (buf), "%s",
                 *(char **)ARR_GetElement(ntp_source_dirs, i)) >= sizeof (buf))
      assert(0);
    search_dirs(buf, ".sources", load_source_file);
  }

  /* Add new and remove existing sources according to the new configuration.
     Avoid removing and adding the same source again to keep its state. */

  new_size = ARR_GetSize(ntp_sources);
  new_sources = ARR_GetElements(ntp_sources);
  unresolved = 0;

  LOG_SetContext(LOGC_SourceFile);

  qsort(new_sources, new_size, sizeof (new_sources[0]), compare_sources);

  for (pass = 0; pass < 2; pass++) {
    for (i = j = 0; i < prev_size || j < new_size; i += d <= 0, j += d >= 0) {
      if (i < prev_size && j < new_size)
        d = compare_sources(&prev_sources[i], &new_sources[j]);
      else
        d = i < prev_size ? -1 : 1;

      was_added = d <= 0 && (prev_sources[i].status == NSR_Success ||
                             prev_sources[i].status == NSR_UnresolvedName);

      /* Remove missing sources before adding others to avoid conflicts */
      if (pass == 0 && d < 0 && was_added) {
        NSR_RemoveSourcesById(prev_sources[i].conf_id);
      }

      /* Add new sources and sources that could not be added before */
      if (pass == 1 && (d > 0 || (d == 0 && !was_added))) {
        source = &new_sources[j];
        s = NSR_AddSourceByName(source->params.name, source->params.family, source->params.port,
                                source->pool, source->type, &source->params.params,
                                &source->conf_id);
        source->status = s;

        if (s == NSR_UnresolvedName) {
          unresolved++;
        } else if (s != NSR_Success && (d > 0 || s != prev_sources[i].status)) {
          LOG(LOGS_ERR, "Could not add source %s : %s",
              source->params.name, NSR_StatusToString(s));
        }
      }

      /* Keep unchanged sources */
      if (pass == 1 && d == 0) {
        new_sources[j].status = prev_sources[i].status;
        new_sources[j].conf_id = prev_sources[i].conf_id;
      }
    }
  }

  LOG_UnsetContext(LOGC_SourceFile);

  for (i = 0; i < prev_size; i++)
    Free(prev_sources[i].params.name);
  Free(prev_sources);

  if (unresolved > 0)
    NSR_ResolveSources();
}

/* ================================================== */

void
CNF_CreateDirs(uid_t uid, gid_t gid)
{
  char *dir;

  /* Create a directory for the Unix domain command socket */
  if (bind_cmd_path) {
    dir = UTI_PathToDir(bind_cmd_path);
    UTI_CreateDirAndParents(dir, 0770, uid, gid);

    /* Check the permissions and owner/group in case the directory already
       existed.  It MUST NOT be accessible by others as permissions on Unix
       domain sockets are ignored on some systems (e.g. Solaris). */
    if (!UTI_CheckDirPermissions(dir, 0770, uid, gid)) {
      LOG(LOGS_WARN, "Disabled command socket %s", bind_cmd_path);
      Free(bind_cmd_path);
      bind_cmd_path = NULL;
    }

    Free(dir);
  }

  if (logdir)
    UTI_CreateDirAndParents(logdir, 0750, uid, gid);
  if (dumpdir)
    UTI_CreateDirAndParents(dumpdir, 0750, uid, gid);
  if (nts_dump_dir)
    UTI_CreateDirAndParents(nts_dump_dir, 0750, uid, gid);
}

/* ================================================== */

void
CNF_CheckReadOnlyAccess(void)
{
  unsigned int i;

  if (keys_file)
    UTI_CheckReadOnlyAccess(keys_file);
  for (i = 0; i < ARR_GetSize(nts_server_key_files); i++)
    UTI_CheckReadOnlyAccess(*(char **)ARR_GetElement(nts_server_key_files, i));
}

/* ================================================== */

void
CNF_AddInitSources(void)
{
  CPS_NTP_Source cps_source;
  NTP_Remote_Address ntp_addr;
  char dummy_hostname[2] = "H";
  unsigned int i;

  for (i = 0; i < ARR_GetSize(init_sources); i++) {
    /* Get the default NTP params */
    CPS_ParseNTPSourceAdd(dummy_hostname, &cps_source);

    /* Add the address as a server specified with the iburst option */
    ntp_addr.ip_addr = *(IPAddr *)ARR_GetElement(init_sources, i);
    ntp_addr.port = cps_source.port;
    cps_source.params.iburst = 1;

    if (NSR_AddSource(&ntp_addr, NTP_SERVER, &cps_source.params, NULL) != NSR_Success)
      LOG(LOGS_ERR, "Could not add source %s", UTI_IPToString(&ntp_addr.ip_addr));
  }

  ARR_SetSize(init_sources, 0);
}

/* ================================================== */

void
CNF_AddSources(void)
{
  NTP_Source *source;
  unsigned int i;
  NSR_Status s;

  for (i = 0; i < ARR_GetSize(ntp_sources); i++) {
    source = (NTP_Source *)ARR_GetElement(ntp_sources, i);

    s = NSR_AddSourceByName(source->params.name, source->params.family, source->params.port,
                            source->pool, source->type, &source->params.params, NULL);
    if (s != NSR_Success && s != NSR_UnresolvedName)
      LOG(LOGS_ERR, "Could not add source %s", source->params.name);

    Free(source->params.name);
  }

  /* The arrays will be used for sourcedir (re)loading */
  ARR_SetSize(ntp_sources, 0);
  conf_ntp_sources_added = 1;

  reload_source_dirs();
}

/* ================================================== */

void
CNF_AddRefclocks(void)
{
  RefclockParameters *refclock;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(refclock_sources); i++) {
    refclock = ARR_GetElement(refclock_sources, i);
    RCL_AddRefclock(refclock);
    Free(refclock->driver_name);
    Free(refclock->driver_parameter);
  }

  ARR_SetSize(refclock_sources, 0);
}

/* ================================================== */

void
CNF_AddBroadcasts(void)
{
  unsigned int i;
  NTP_Broadcast_Destination *destination;

  for (i = 0; i < ARR_GetSize(broadcasts); i++) {
    destination = (NTP_Broadcast_Destination *)ARR_GetElement(broadcasts, i);
    NCR_AddBroadcastDestination(&destination->addr, destination->interval);
  }

  ARR_SetSize(broadcasts, 0);
}

/* ================================================== */

void
CNF_ReloadSources(void)
{
  reload_source_dirs();
}

/* ================================================== */

int
CNF_GetNTPPort(void)
{
  return ntp_port;
}

/* ================================================== */

int
CNF_GetAcquisitionPort(void)
{
  return acquisition_port;
}

/* ================================================== */

char *
CNF_GetDriftFile(int *interval)
{
  *interval = drift_file_interval;
  return drift_file;
}

/* ================================================== */

int
CNF_GetLogBanner(void)
{
  return log_banner;
}

/* ================================================== */

char *
CNF_GetLogDir(void)
{
  return logdir;
}

/* ================================================== */

char *
CNF_GetDumpDir(void)
{
  return dumpdir;
}

/* ================================================== */

int
CNF_GetLogMeasurements(int *raw)
{
  *raw = raw_measurements;
  return do_log_measurements;
}

/* ================================================== */

int
CNF_GetLogSelection(void)
{
  return do_log_selection;
}

/* ================================================== */

int
CNF_GetLogStatistics(void)
{
  return do_log_statistics;
}

/* ================================================== */

int
CNF_GetLogTracking(void)
{
  return do_log_tracking;
}

/* ================================================== */

int
CNF_GetLogRtc(void)
{
  return do_log_rtc;
}

/* ================================================== */

int
CNF_GetLogRefclocks(void)
{
  return do_log_refclocks;
}

/* ================================================== */

int
CNF_GetLogTempComp(void)
{
  return do_log_tempcomp;
}

/* ================================================== */

char *
CNF_GetKeysFile(void)
{
  return keys_file;
}

/* ================================================== */

double
CNF_GetRtcAutotrim(void)
{
  return rtc_autotrim_threshold;
}

/* ================================================== */

char *
CNF_GetRtcFile(void)
{
  return rtc_file;
}

/* ================================================== */

char *
CNF_GetRtcDevice(void)
{
  return rtc_device;
}

/* ================================================== */

double
CNF_GetMaxUpdateSkew(void)
{
  return max_update_skew;
}

/* ================================================== */

double
CNF_GetMaxDrift(void)
{
  return max_drift;
}

/* ================================================== */

double
CNF_GetMaxClockError(void)
{
  return max_clock_error;
}

/* ================================================== */

double
CNF_GetCorrectionTimeRatio(void)
{
  return correction_time_ratio;
}

/* ================================================== */

SRC_AuthSelectMode
CNF_GetAuthSelectMode(void)
{
  return authselect_mode;
}

/* ================================================== */

double
CNF_GetMaxSlewRate(void)
{
  return max_slew_rate;
}

/* ================================================== */

double
CNF_GetClockPrecision(void)
{
  return clock_precision;
}

/* ================================================== */

double
CNF_GetMaxDistance(void)
{
  return max_distance;
}

/* ================================================== */

double
CNF_GetMaxJitter(void)
{
  return max_jitter;
}

/* ================================================== */

double
CNF_GetReselectDistance(void)
{
  return reselect_distance;
}

/* ================================================== */

double
CNF_GetStratumWeight(void)
{
  return stratum_weight;
}

/* ================================================== */

double
CNF_GetCombineLimit(void)
{
  return combine_limit;
}

/* ================================================== */

int
CNF_GetManualEnabled(void)
{
  return enable_manual;
}

/* ================================================== */

ARR_Instance
CNF_GetOpenCommands(void)
{
  return open_commands;
}

/* ================================================== */

int
CNF_GetCommandPort(void) {
  return cmd_port;
}

/* ================================================== */

int
CNF_AllowLocalReference(int *stratum, int *orphan, double *distance, double *activate,
                        double *wait_synced, double *wait_unsynced)
{
  if (enable_local) {
    *stratum = local_stratum;
    *orphan = local_orphan;
    *distance = local_distance;
    *activate = local_activate;
    *wait_synced = local_wait_synced;
    *wait_unsynced = local_wait_unsynced;
    return 1;
  } else {
    return 0;
  }
}

/* ================================================== */

int
CNF_GetRtcOnUtc(void)
{
  return rtc_on_utc;
}

/* ================================================== */

int
CNF_GetRtcSync(void)
{
  return rtc_sync;
}

/* ================================================== */

void
CNF_GetMakeStep(int *limit, double *threshold)
{
  *limit = make_step_limit;
  *threshold = make_step_threshold;
}

/* ================================================== */

void
CNF_GetMaxChange(int *delay, int *ignore, double *offset)
{
  *delay = max_offset_delay;
  *ignore = max_offset_ignore;
  *offset = max_offset;
}

/* ================================================== */

double
CNF_GetLogChange(void)
{
  return log_change_threshold;
}

/* ================================================== */

void
CNF_GetMailOnChange(int *enabled, double *threshold, char **user)
{
  if (mail_user_on_change) {
    *enabled = 1;
    *threshold = mail_change_threshold;
    *user = mail_user_on_change;
  } else {
    *enabled = 0;
    *threshold = 0.0;
    *user = NULL;
  }
}  

/* ================================================== */

void
CNF_SetupAccessRestrictions(void)
{
  AllowDeny *node;
  int status;
  unsigned int i;

  for (i = 0; i < ARR_GetSize(ntp_restrictions); i++) {
    node = ARR_GetElement(ntp_restrictions, i);
    status = NCR_AddAccessRestriction(&node->ip, node->subnet_bits, node->allow, node->all);
    if (!status) {
      LOG_FATAL("Bad subnet in %s/%d", UTI_IPToString(&node->ip), node->subnet_bits);
    }
  }

  for (i = 0; i < ARR_GetSize(cmd_restrictions); i++) {
    node = ARR_GetElement(cmd_restrictions, i);
    status = CAM_AddAccessRestriction(&node->ip, node->subnet_bits, node->allow, node->all);
    if (!status) {
      LOG_FATAL("Bad subnet in %s/%d", UTI_IPToString(&node->ip), node->subnet_bits);
    }
  }

  ARR_SetSize(ntp_restrictions, 0);
  ARR_SetSize(cmd_restrictions, 0);
}

/* ================================================== */

int
CNF_GetNoClientLog(void)
{
  return no_client_log;
}

/* ================================================== */

unsigned long
CNF_GetClientLogLimit(void)
{
  return client_log_limit;
}

/* ================================================== */

void
CNF_GetFallbackDrifts(int *min, int *max)
{
  *min = fb_drift_min;
  *max = fb_drift_max;
}

/* ================================================== */

void
CNF_GetBindAddress(int family, IPAddr *addr)
{
  if (family == IPADDR_INET4)
    *addr = bind_address4;
  else if (family == IPADDR_INET6)
    *addr = bind_address6;
  else
    addr->family = IPADDR_UNSPEC;
}

/* ================================================== */

void
CNF_GetBindAcquisitionAddress(int family, IPAddr *addr)
{
  if (family == IPADDR_INET4)
    *addr = bind_acq_address4;
  else if (family == IPADDR_INET6)
    *addr = bind_acq_address6;
  else
    addr->family = IPADDR_UNSPEC;
}

/* ================================================== */

char *
CNF_GetBindNtpInterface(void)
{
  return bind_ntp_iface;
}

/* ================================================== */

char *
CNF_GetBindAcquisitionInterface(void)
{
  return bind_acq_iface;
}

/* ================================================== */

char *
CNF_GetBindCommandInterface(void)
{
  return bind_cmd_iface;
}

/* ================================================== */

char *
CNF_GetBindCommandPath(void)
{
  return bind_cmd_path;
}

/* ================================================== */

void
CNF_GetBindCommandAddress(int family, IPAddr *addr)
{
  if (family == IPADDR_INET4)
    *addr = bind_cmd_address4;
  else if (family == IPADDR_INET6)
    *addr = bind_cmd_address6;
  else
    addr->family = IPADDR_UNSPEC;
}

/* ================================================== */

int
CNF_GetNtpDscp(void)
{
  return ntp_dscp;
}

/* ================================================== */

char *
CNF_GetNtpSigndSocket(void)
{
  return ntp_signd_socket;
}

/* ================================================== */

char *
CNF_GetPidFile(void)
{
  return pidfile;
}

/* ================================================== */

REF_LeapMode
CNF_GetLeapSecMode(void)
{
  return leapsec_mode;
}

/* ================================================== */

char *
CNF_GetLeapSecTimezone(void)
{
  return leapsec_tz;
}

/* ================================================== */

char *
CNF_GetLeapSecList(void)
{
  return leapsec_list;
}

/* ================================================== */

int
CNF_GetSchedPriority(void)
{
  return sched_priority;
}

/* ================================================== */

int
CNF_GetLockMemory(void)
{
  return lock_memory;
}

/* ================================================== */

int CNF_GetNTPRateLimit(int *interval, int *burst, int *leak, int *kod)
{
  *interval = ntp_ratelimit_interval;
  *burst = ntp_ratelimit_burst;
  *leak = ntp_ratelimit_leak;
  *kod = ntp_ratelimit_kod;
  return ntp_ratelimit_enabled;
}

/* ================================================== */

int CNF_GetNtsRateLimit(int *interval, int *burst, int *leak)
{
  *interval = nts_ratelimit_interval;
  *burst = nts_ratelimit_burst;
  *leak = nts_ratelimit_leak;
  return nts_ratelimit_enabled;
}

/* ================================================== */

int CNF_GetCommandRateLimit(int *interval, int *burst, int *leak)
{
  *interval = cmd_ratelimit_interval;
  *burst = cmd_ratelimit_burst;
  *leak = cmd_ratelimit_leak;
  return cmd_ratelimit_enabled;
}

/* ================================================== */

void
CNF_GetSmooth(double *max_freq, double *max_wander, int *leap_only)
{
  *max_freq = smooth_max_freq;
  *max_wander = smooth_max_wander;
  *leap_only = smooth_leap_only;
}

/* ================================================== */

void
CNF_GetTempComp(char **file, double *interval, char **point_file, double *T0, double *k0, double *k1, double *k2)
{
  *file = tempcomp_sensor_file;
  *point_file = tempcomp_point_file;
  *interval = tempcomp_interval;
  *T0 = tempcomp_T0;
  *k0 = tempcomp_k0;
  *k1 = tempcomp_k1;
  *k2 = tempcomp_k2;
}

/* ================================================== */

char *
CNF_GetUser(void)
{
  return user;
}

/* ================================================== */

int
CNF_GetMaxSamples(void)
{
  return max_samples;
}

/* ================================================== */

int
CNF_GetMinSamples(void)
{
  return min_samples;
}

/* ================================================== */

int
CNF_GetMinSources(void)
{
  return min_sources;
}

/* ================================================== */

char *
CNF_GetHwclockFile(void)
{
  return hwclock_file;
}

/* ================================================== */

int
CNF_GetInitSources(void)
{
  return ARR_GetSize(init_sources);
}

/* ================================================== */

double
CNF_GetInitStepThreshold(void)
{
  return init_slew_threshold;
}

/* ================================================== */

int
CNF_GetHwTsInterface(unsigned int index, CNF_HwTsInterface **iface)
{
  if (index >= ARR_GetSize(hwts_interfaces))
    return 0;

  *iface = (CNF_HwTsInterface *)ARR_GetElement(hwts_interfaces, index);
  return 1;
}

/* ================================================== */

double
CNF_GetHwTsTimeout(void)
{
  return hwts_timeout;
}

/* ================================================== */

int
CNF_GetPtpPort(void)
{
  return ptp_port;
}

/* ================================================== */

int
CNF_GetPtpDomain(void)
{
  return ptp_domain;
}

/* ================================================== */

int
CNF_GetRefresh(void)
{
  return refresh;
}

/* ================================================== */

ARR_Instance
CNF_GetNtsAeads(void)
{
  return nts_aeads;
}

/* ================================================== */

char *
CNF_GetNtsDumpDir(void)
{
  return nts_dump_dir;
}

/* ================================================== */

char *
CNF_GetNtsNtpServer(void)
{
  return nts_ntp_server;
}

/* ================================================== */

int
CNF_GetNtsServerCertAndKeyFiles(const char ***certs, const char ***keys)
{
  *certs = ARR_GetElements(nts_server_cert_files);
  *keys = ARR_GetElements(nts_server_key_files);

  if (ARR_GetSize(nts_server_cert_files) != ARR_GetSize(nts_server_key_files))
    LOG_FATAL("Uneven number of NTS certs and keys");

  return ARR_GetSize(nts_server_cert_files);
}

/* ================================================== */

int
CNF_GetNtsServerPort(void)
{
  return nts_server_port;
}

/* ================================================== */

int
CNF_GetNtsServerProcesses(void)
{
  return nts_server_processes;
}

/* ================================================== */

int
CNF_GetNtsServerConnections(void)
{
  return nts_server_connections;
}

/* ================================================== */

int
CNF_GetNtsRefresh(void)
{
  return nts_refresh;
}

/* ================================================== */

int
CNF_GetNtsRotate(void)
{
  return nts_rotate;
}

/* ================================================== */

int
CNF_GetNtsTrustedCertsPaths(const char ***paths, uint32_t **ids)
{
  *paths = ARR_GetElements(nts_trusted_certs_paths);
  *ids = ARR_GetElements(nts_trusted_certs_ids);

  BRIEF_ASSERT(ARR_GetSize(nts_trusted_certs_paths) == ARR_GetSize(nts_trusted_certs_ids));

  return ARR_GetSize(nts_trusted_certs_paths);
}

/* ================================================== */

int
CNF_GetNoSystemCert(void)
{
  return no_system_cert;
}

/* ================================================== */

int
CNF_GetNoCertTimeCheck(void)
{
  return no_cert_time_check;
}

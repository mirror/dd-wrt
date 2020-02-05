/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) John G. Hasler  2009
 * Copyright (C) Miroslav Lichvar  2009-2012, 2014-2018
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

  This is the module specific to the Linux operating system.

  */

#include "config.h"

#include "sysincl.h"

#include <sys/utsname.h>

#if defined(FEAT_PHC) || defined(HAVE_LINUX_TIMESTAMPING)
#include <linux/ptp_clock.h>
#endif

#ifdef FEAT_SCFILTER
#include <sys/prctl.h>
#include <seccomp.h>
#include <termios.h>
#ifdef FEAT_PPS
#include <linux/pps.h>
#endif
#ifdef FEAT_RTC
#include <linux/rtc.h>
#endif
#ifdef HAVE_LINUX_TIMESTAMPING
#include <linux/sockios.h>
#endif
#endif

#ifdef FEAT_PRIVDROP
#include <sys/prctl.h>
#include <sys/capability.h>
#endif

#include "sys_linux.h"
#include "sys_timex.h"
#include "conf.h"
#include "local.h"
#include "logging.h"
#include "privops.h"
#include "util.h"

/* Frequency scale to convert from ppm to the timex freq */
#define FREQ_SCALE (double)(1 << 16)

/* Definitions used if missed in the system headers */
#ifndef ADJ_SETOFFSET
#define ADJ_SETOFFSET           0x0100  /* add 'time' to current time */
#endif
#ifndef ADJ_NANO
#define ADJ_NANO                0x2000  /* select nanosecond resolution */
#endif

/* This is the uncompensated system tick value */
static int nominal_tick;

/* Current tick value */
static int current_delta_tick;

/* The maximum amount by which 'tick' can be biased away from 'nominal_tick'
   (sys_adjtimex() in the kernel bounds this to 10%) */
static int max_tick_bias;

/* The kernel USER_HZ constant */
static int hz;
static double dhz; /* And dbl prec version of same for arithmetic */

/* Flag indicating whether adjtimex() can step the clock */
static int have_setoffset;

/* The assumed rate at which the effective frequency and tick values are
   updated in the kernel */
static int tick_update_hz;

/* ================================================== */

inline static long
our_round(double x)
{
  long y;

  if (x > 0.0)
    y = x + 0.5;
  else
    y = x - 0.5;

  return y;
}

/* ================================================== */
/* Positive means currently fast of true time, i.e. jump backwards */

static int
apply_step_offset(double offset)
{
  struct timex txc;

  txc.modes = ADJ_SETOFFSET | ADJ_NANO;
  txc.time.tv_sec = -offset;
  txc.time.tv_usec = 1.0e9 * (-offset - txc.time.tv_sec);
  if (txc.time.tv_usec < 0) {
    txc.time.tv_sec--;
    txc.time.tv_usec += 1000000000;
  }

  if (SYS_Timex_Adjust(&txc, 1) < 0)
    return 0;

  return 1;
}

/* ================================================== */
/* This call sets the Linux kernel frequency to a given value in parts
   per million relative to the nominal running frequency.  Nominal is taken to
   be tick=10000, freq=0 (for a USER_HZ==100 system, other values otherwise).
   The convention is that this is called with a positive argument if the local
   clock runs fast when uncompensated.  */

static double
set_frequency(double freq_ppm)
{
  struct timex txc;
  long required_tick;
  double required_freq;
  int required_delta_tick;

  required_delta_tick = our_round(freq_ppm / dhz);

  /* Older kernels (pre-2.6.18) don't apply the frequency offset exactly as
     set by adjtimex() and a scaling constant (that depends on the internal
     kernel HZ constant) would be needed to compensate for the error. Because
     chronyd is closed loop it doesn't matter much if we don't scale the
     required frequency, but we want to prevent thrashing between two states
     when the system's frequency error is close to a multiple of USER_HZ.  With
     USER_HZ <= 250, the maximum frequency adjustment of 500 ppm overlaps at
     least two ticks and we can stick to the current tick if it's next to the
     required tick. */
  if (hz <= 250 && (required_delta_tick + 1 == current_delta_tick ||
                    required_delta_tick - 1 == current_delta_tick)) {
    required_delta_tick = current_delta_tick;
  }

  required_freq = -(freq_ppm - dhz * required_delta_tick);
  required_tick = nominal_tick - required_delta_tick;

  txc.modes = ADJ_TICK | ADJ_FREQUENCY;
  txc.freq = required_freq * FREQ_SCALE;
  txc.tick = required_tick;

  SYS_Timex_Adjust(&txc, 0);

  current_delta_tick = required_delta_tick;

  return dhz * current_delta_tick - txc.freq / FREQ_SCALE;
}

/* ================================================== */
/* Read the ppm frequency from the kernel */

static double
read_frequency(void)
{
  struct timex txc;

  txc.modes = 0;

  SYS_Timex_Adjust(&txc, 0);

  current_delta_tick = nominal_tick - txc.tick;

  return dhz * current_delta_tick - txc.freq / FREQ_SCALE;
}

/* ================================================== */

/* Estimate the value of USER_HZ given the value of txc.tick that chronyd finds when
 * it starts.  The only credible values are 100 (Linux/x86) or powers of 2.
 * Also, the bounds checking inside the kernel's adjtimex system call enforces
 * a +/- 10% movement of tick away from the nominal value 1e6/USER_HZ. */

static int
guess_hz(void)
{
  struct timex txc;
  int i, tick, tick_lo, tick_hi, ihz;
  double tick_nominal;

  txc.modes = 0;
  SYS_Timex_Adjust(&txc, 0);
  tick = txc.tick;

  /* Pick off the hz=100 case first */
  if (tick >= 9000 && tick <= 11000) {
    return 100;
  }

  for (i=4; i<16; i++) { /* surely 16 .. 32768 is a wide enough range? */
    ihz = 1 << i;
    tick_nominal = 1.0e6 / (double) ihz;
    tick_lo = (int)(0.5 + tick_nominal*2.0/3.0);
    tick_hi = (int)(0.5 + tick_nominal*4.0/3.0);
    
    if (tick_lo < tick && tick <= tick_hi) {
      return ihz;
    }
  }

  /* oh dear.  doomed. */
  LOG_FATAL("Can't determine hz from tick %d", tick);

  return 0;
}

/* ================================================== */

static int
get_hz(void)
{
#ifdef _SC_CLK_TCK
  int hz;

  if ((hz = sysconf(_SC_CLK_TCK)) < 1)
    return 0;

  return hz;
#else
  return 0;
#endif
}

/* ================================================== */

static int
kernelvercmp(int major1, int minor1, int patch1,
    int major2, int minor2, int patch2)
{
  if (major1 != major2)
    return major1 - major2;
  if (minor1 != minor2)
    return minor1 - minor2;
  return patch1 - patch2;
}

/* ================================================== */

static void
get_kernel_version(int *major, int *minor, int *patch)
{
  struct utsname uts;

  if (uname(&uts) < 0)
    LOG_FATAL("uname() failed");

  *patch = 0;
  if (sscanf(uts.release, "%d.%d.%d", major, minor, patch) < 2)
    LOG_FATAL("Could not parse kernel version");
}

/* ================================================== */

/* Compute the scaling to use on any frequency we set, according to
   the vintage of the Linux kernel being used. */

static void
get_version_specific_details(void)
{
  int major, minor, patch;
  
  hz = get_hz();

  if (!hz)
    hz = guess_hz();

  dhz = (double) hz;
  nominal_tick = (1000000L + (hz/2))/hz; /* Mirror declaration in kernel */
  max_tick_bias = nominal_tick / 10;

  /* In modern kernels the frequency of the clock is updated immediately in the
     adjtimex() system call.  Assume a maximum delay of 10 microseconds. */
  tick_update_hz = 100000;

  get_kernel_version(&major, &minor, &patch);
  DEBUG_LOG("Linux kernel major=%d minor=%d patch=%d", major, minor, patch);

  if (kernelvercmp(major, minor, patch, 2, 2, 0) < 0) {
    LOG_FATAL("Kernel version not supported, sorry.");
  }

  if (kernelvercmp(major, minor, patch, 2, 6, 27) >= 0 &&
      kernelvercmp(major, minor, patch, 2, 6, 33) < 0) {
    /* In tickless kernels before 2.6.33 the frequency is updated in
       a half-second interval */
    tick_update_hz = 2;
  } else if (kernelvercmp(major, minor, patch, 4, 19, 0) < 0) {
    /* In kernels before 4.19 the frequency is updated only on internal ticks
       (CONFIG_HZ).  As their rate cannot be reliably detected from the user
       space, and it may not even be constant (CONFIG_NO_HZ - aka tickless),
       assume the lowest commonly used constant rate */
    tick_update_hz = 100;
  }

  /* ADJ_SETOFFSET support */
  if (kernelvercmp(major, minor, patch, 2, 6, 39) < 0) {
    have_setoffset = 0;
  } else {
    have_setoffset = 1;
  }

  DEBUG_LOG("hz=%d nominal_tick=%d max_tick_bias=%d tick_update_hz=%d",
            hz, nominal_tick, max_tick_bias, tick_update_hz);
}

/* ================================================== */

static void
reset_adjtime_offset(void)
{
  struct timex txc;

  /* Reset adjtime() offset */
  txc.modes = ADJ_OFFSET_SINGLESHOT;
  txc.offset = 0;

  SYS_Timex_Adjust(&txc, 0);
}

/* ================================================== */

static int
test_step_offset(void)
{
  struct timex txc;

  /* Zero maxerror and check it's reset to a maximum after ADJ_SETOFFSET.
     This seems to be the only way how to verify that the kernel really
     supports the ADJ_SETOFFSET mode as it doesn't return an error on unknown
     mode. */

  txc.modes = MOD_MAXERROR;
  txc.maxerror = 0;

  if (SYS_Timex_Adjust(&txc, 1) < 0 || txc.maxerror != 0)
    return 0;

  txc.modes = ADJ_SETOFFSET | ADJ_NANO;
  txc.time.tv_sec = 0;
  txc.time.tv_usec = 0;

  if (SYS_Timex_Adjust(&txc, 1) < 0 || txc.maxerror < 100000)
    return 0;

  return 1;
}

/* ================================================== */

static void
report_time_adjust_blockers(void)
{
#if defined(FEAT_PRIVDROP) && defined(CAP_IS_SUPPORTED)
  if (CAP_IS_SUPPORTED(CAP_SYS_TIME) && cap_get_bound(CAP_SYS_TIME))
    return;
  LOG(LOGS_WARN, "CAP_SYS_TIME not present");
#endif
}

/* ================================================== */
/* Initialisation code for this module */

void
SYS_Linux_Initialise(void)
{
  get_version_specific_details();

  report_time_adjust_blockers();

  reset_adjtime_offset();

  if (have_setoffset && !test_step_offset()) {
    LOG(LOGS_INFO, "adjtimex() doesn't support ADJ_SETOFFSET");
    have_setoffset = 0;
  }

  SYS_Timex_InitialiseWithFunctions(1.0e6 * max_tick_bias / nominal_tick,
                                    1.0 / tick_update_hz,
                                    read_frequency, set_frequency,
                                    have_setoffset ? apply_step_offset : NULL,
                                    0.0, 0.0, NULL, NULL);
}

/* ================================================== */
/* Finalisation code for this module */

void
SYS_Linux_Finalise(void)
{
  SYS_Timex_Finalise();
}

/* ================================================== */

#ifdef FEAT_PRIVDROP
void
SYS_Linux_DropRoot(uid_t uid, gid_t gid, int clock_control)
{
  char cap_text[256];
  cap_t cap;

  if (prctl(PR_SET_KEEPCAPS, 1)) {
    LOG_FATAL("prctl() failed");
  }
  
  UTI_DropRoot(uid, gid);

  /* Keep CAP_NET_BIND_SERVICE only if a server NTP port can be opened
     and keep CAP_SYS_TIME only if the clock control is enabled */
  if (snprintf(cap_text, sizeof (cap_text), "%s %s",
               CNF_GetNTPPort() ? "cap_net_bind_service=ep" : "",
               clock_control ? "cap_sys_time=ep" : "") >= sizeof (cap_text))
    assert(0);

  if ((cap = cap_from_text(cap_text)) == NULL) {
    LOG_FATAL("cap_from_text() failed");
  }

  if (cap_set_proc(cap)) {
    LOG_FATAL("cap_set_proc() failed");
  }

  cap_free(cap);
}
#endif

/* ================================================== */

#ifdef FEAT_SCFILTER
static
void check_seccomp_applicability(void)
{
  int mail_enabled;
  double mail_threshold;
  char *mail_user;

  CNF_GetMailOnChange(&mail_enabled, &mail_threshold, &mail_user);
  if (mail_enabled)
    LOG_FATAL("mailonchange directive cannot be used with -F enabled");
}

/* ================================================== */

void
SYS_Linux_EnableSystemCallFilter(int level)
{
  const int syscalls[] = {
    /* Clock */
    SCMP_SYS(adjtimex), SCMP_SYS(clock_gettime), SCMP_SYS(gettimeofday),
    SCMP_SYS(settimeofday), SCMP_SYS(time),
    /* Process */
    SCMP_SYS(clone), SCMP_SYS(exit), SCMP_SYS(exit_group), SCMP_SYS(getpid),
    SCMP_SYS(getrlimit), SCMP_SYS(rt_sigaction), SCMP_SYS(rt_sigreturn),
    SCMP_SYS(rt_sigprocmask), SCMP_SYS(set_tid_address), SCMP_SYS(sigreturn),
    SCMP_SYS(wait4), SCMP_SYS(waitpid),
    /* Memory */
    SCMP_SYS(brk), SCMP_SYS(madvise), SCMP_SYS(mmap), SCMP_SYS(mmap2),
    SCMP_SYS(mprotect), SCMP_SYS(mremap), SCMP_SYS(munmap), SCMP_SYS(shmdt),
    /* Filesystem */
    SCMP_SYS(_llseek), SCMP_SYS(access), SCMP_SYS(chmod), SCMP_SYS(chown),
    SCMP_SYS(chown32), SCMP_SYS(faccessat), SCMP_SYS(fchmodat), SCMP_SYS(fchownat),
    SCMP_SYS(fstat), SCMP_SYS(fstat64), SCMP_SYS(getdents), SCMP_SYS(getdents64),
    SCMP_SYS(lseek), SCMP_SYS(newfstatat), SCMP_SYS(rename), SCMP_SYS(renameat),
    SCMP_SYS(stat), SCMP_SYS(stat64), SCMP_SYS(statfs), SCMP_SYS(statfs64),
    SCMP_SYS(unlink), SCMP_SYS(unlinkat),
    /* Socket */
    SCMP_SYS(bind), SCMP_SYS(connect), SCMP_SYS(getsockname), SCMP_SYS(getsockopt),
    SCMP_SYS(recv), SCMP_SYS(recvfrom), SCMP_SYS(recvmmsg), SCMP_SYS(recvmsg),
    SCMP_SYS(send), SCMP_SYS(sendmmsg), SCMP_SYS(sendmsg), SCMP_SYS(sendto),
    /* TODO: check socketcall arguments */
    SCMP_SYS(socketcall),
    /* General I/O */
    SCMP_SYS(_newselect), SCMP_SYS(close), SCMP_SYS(open), SCMP_SYS(openat), SCMP_SYS(pipe),
    SCMP_SYS(pipe2), SCMP_SYS(poll), SCMP_SYS(ppoll), SCMP_SYS(pselect6), SCMP_SYS(read),
    SCMP_SYS(futex), SCMP_SYS(select), SCMP_SYS(set_robust_list), SCMP_SYS(write),
    /* Miscellaneous */
    SCMP_SYS(getrandom), SCMP_SYS(sysinfo), SCMP_SYS(uname),
  };

  const int socket_domains[] = {
    AF_NETLINK, AF_UNIX, AF_INET,
#ifdef FEAT_IPV6
    AF_INET6,
#endif
  };

  const static int socket_options[][2] = {
    { SOL_IP, IP_PKTINFO }, { SOL_IP, IP_FREEBIND },
#ifdef FEAT_IPV6
    { SOL_IPV6, IPV6_V6ONLY }, { SOL_IPV6, IPV6_RECVPKTINFO },
#endif
    { SOL_SOCKET, SO_BROADCAST }, { SOL_SOCKET, SO_REUSEADDR },
    { SOL_SOCKET, SO_TIMESTAMP }, { SOL_SOCKET, SO_TIMESTAMPNS },
#ifdef HAVE_LINUX_TIMESTAMPING
    { SOL_SOCKET, SO_SELECT_ERR_QUEUE }, { SOL_SOCKET, SO_TIMESTAMPING },
#endif
  };

  const static int fcntls[] = { F_GETFD, F_SETFD, F_SETFL };

  const static unsigned long ioctls[] = {
    FIONREAD, TCGETS,
#if defined(FEAT_PHC) || defined(HAVE_LINUX_TIMESTAMPING)
    PTP_EXTTS_REQUEST, PTP_SYS_OFFSET,
#ifdef PTP_PIN_SETFUNC
    PTP_PIN_SETFUNC,
#endif
#ifdef PTP_SYS_OFFSET_EXTENDED
    PTP_SYS_OFFSET_EXTENDED,
#endif
#ifdef PTP_SYS_OFFSET_PRECISE
    PTP_SYS_OFFSET_PRECISE,
#endif
#endif
#ifdef FEAT_PPS
    PPS_FETCH,
#endif
#ifdef FEAT_RTC
    RTC_RD_TIME, RTC_SET_TIME, RTC_UIE_ON, RTC_UIE_OFF,
#endif
#ifdef HAVE_LINUX_TIMESTAMPING
    SIOCETHTOOL,
#endif
  };

  scmp_filter_ctx *ctx;
  int i;

  /* Check if the chronyd configuration is supported */
  check_seccomp_applicability();

  /* Start the helper process, which will run without any seccomp filter.  It
     will be used for getaddrinfo(), for which it's difficult to maintain a
     list of required system calls (with glibc it depends on what NSS modules
     are installed and enabled on the system). */
  PRV_StartHelper();

  ctx = seccomp_init(level > 0 ? SCMP_ACT_KILL : SCMP_ACT_TRAP);
  if (ctx == NULL)
      LOG_FATAL("Failed to initialize seccomp");

  /* Add system calls that are always allowed */
  for (i = 0; i < (sizeof (syscalls) / sizeof (*syscalls)); i++) {
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscalls[i], 0) < 0)
      goto add_failed;
  }

  /* Allow sockets to be created only in selected domains */
  for (i = 0; i < sizeof (socket_domains) / sizeof (*socket_domains); i++) {
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 1,
                         SCMP_A0(SCMP_CMP_EQ, socket_domains[i])) < 0)
      goto add_failed;
  }

  /* Allow setting only selected sockets options */
  for (i = 0; i < sizeof (socket_options) / sizeof (*socket_options); i++) {
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setsockopt), 3,
                         SCMP_A1(SCMP_CMP_EQ, socket_options[i][0]),
                         SCMP_A2(SCMP_CMP_EQ, socket_options[i][1]),
                         SCMP_A4(SCMP_CMP_LE, sizeof (int))) < 0)
      goto add_failed;
  }

  /* Allow only selected fcntl calls */
  for (i = 0; i < sizeof (fcntls) / sizeof (*fcntls); i++) {
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 1,
                         SCMP_A1(SCMP_CMP_EQ, fcntls[i])) < 0 ||
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fcntl64), 1,
                         SCMP_A1(SCMP_CMP_EQ, fcntls[i])) < 0)
      goto add_failed;
  }

  /* Allow only selected ioctls */
  for (i = 0; i < sizeof (ioctls) / sizeof (*ioctls); i++) {
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 1,
                         SCMP_A1(SCMP_CMP_EQ, ioctls[i])) < 0)
      goto add_failed;
  }

  if (seccomp_load(ctx) < 0)
    LOG_FATAL("Failed to load seccomp rules");

  LOG(LOGS_INFO, "Loaded seccomp filter");
  seccomp_release(ctx);
  return;

add_failed:
  LOG_FATAL("Failed to add seccomp rules");
}
#endif

/* ================================================== */

int
SYS_Linux_CheckKernelVersion(int req_major, int req_minor)
{
  int major, minor, patch;

  get_kernel_version(&major, &minor, &patch);

  return kernelvercmp(req_major, req_minor, 0, major, minor, patch) <= 0;
}

/* ================================================== */

#if defined(FEAT_PHC) || defined(HAVE_LINUX_TIMESTAMPING)

#define PHC_READINGS 10

static int
process_phc_readings(struct timespec ts[][3], int n, double precision,
                     struct timespec *phc_ts, struct timespec *sys_ts, double *err)
{
  double min_delay = 0.0, delays[PTP_MAX_SAMPLES], phc_sum, sys_sum, sys_prec;
  int i, combined;

  if (n > PTP_MAX_SAMPLES)
    return 0;

  for (i = 0; i < n; i++) {
    delays[i] = UTI_DiffTimespecsToDouble(&ts[i][2], &ts[i][0]);

    if (delays[i] < 0.0) {
      /* Step in the middle of a PHC reading? */
      DEBUG_LOG("Bad PTP_SYS_OFFSET sample delay=%e", delays[i]);
      return 0;
    }

    if (!i || delays[i] < min_delay)
      min_delay = delays[i];
  }

  sys_prec = LCL_GetSysPrecisionAsQuantum();

  /* Combine best readings */
  for (i = combined = 0, phc_sum = sys_sum = 0.0; i < n; i++) {
    if (delays[i] > min_delay + MAX(sys_prec, precision))
      continue;

    phc_sum += UTI_DiffTimespecsToDouble(&ts[i][1], &ts[0][1]);
    sys_sum += UTI_DiffTimespecsToDouble(&ts[i][0], &ts[0][0]) + delays[i] / 2.0;
    combined++;
  }

  assert(combined);

  UTI_AddDoubleToTimespec(&ts[0][1], phc_sum / combined, phc_ts);
  UTI_AddDoubleToTimespec(&ts[0][0], sys_sum / combined, sys_ts);
  *err = MAX(min_delay / 2.0, precision);

  return 1;
}

/* ================================================== */

static int
get_phc_sample(int phc_fd, double precision, struct timespec *phc_ts,
               struct timespec *sys_ts, double *err)
{
  struct timespec ts[PHC_READINGS][3];
  struct ptp_sys_offset sys_off;
  int i;

  /* Silence valgrind */
  memset(&sys_off, 0, sizeof (sys_off));

  sys_off.n_samples = PHC_READINGS;

  if (ioctl(phc_fd, PTP_SYS_OFFSET, &sys_off)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_SYS_OFFSET", strerror(errno));
    return 0;
  }

  for (i = 0; i < PHC_READINGS; i++) {
    ts[i][0].tv_sec = sys_off.ts[i * 2].sec;
    ts[i][0].tv_nsec = sys_off.ts[i * 2].nsec;
    ts[i][1].tv_sec = sys_off.ts[i * 2 + 1].sec;
    ts[i][1].tv_nsec = sys_off.ts[i * 2 + 1].nsec;
    ts[i][2].tv_sec = sys_off.ts[i * 2 + 2].sec;
    ts[i][2].tv_nsec = sys_off.ts[i * 2 + 2].nsec;
  }

  return process_phc_readings(ts, PHC_READINGS, precision, phc_ts, sys_ts, err);
}

/* ================================================== */

static int
get_extended_phc_sample(int phc_fd, double precision, struct timespec *phc_ts,
                        struct timespec *sys_ts, double *err)
{
#ifdef PTP_SYS_OFFSET_EXTENDED
  struct timespec ts[PHC_READINGS][3];
  struct ptp_sys_offset_extended sys_off;
  int i;

  /* Silence valgrind */
  memset(&sys_off, 0, sizeof (sys_off));

  sys_off.n_samples = PHC_READINGS;

  if (ioctl(phc_fd, PTP_SYS_OFFSET_EXTENDED, &sys_off)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_SYS_OFFSET_EXTENDED", strerror(errno));
    return 0;
  }

  for (i = 0; i < PHC_READINGS; i++) {
    ts[i][0].tv_sec = sys_off.ts[i][0].sec;
    ts[i][0].tv_nsec = sys_off.ts[i][0].nsec;
    ts[i][1].tv_sec = sys_off.ts[i][1].sec;
    ts[i][1].tv_nsec = sys_off.ts[i][1].nsec;
    ts[i][2].tv_sec = sys_off.ts[i][2].sec;
    ts[i][2].tv_nsec = sys_off.ts[i][2].nsec;
  }

  return process_phc_readings(ts, PHC_READINGS, precision, phc_ts, sys_ts, err);
#else
  return 0;
#endif
}

/* ================================================== */

static int
get_precise_phc_sample(int phc_fd, double precision, struct timespec *phc_ts,
		       struct timespec *sys_ts, double *err)
{
#ifdef PTP_SYS_OFFSET_PRECISE
  struct ptp_sys_offset_precise sys_off;

  /* Silence valgrind */
  memset(&sys_off, 0, sizeof (sys_off));

  if (ioctl(phc_fd, PTP_SYS_OFFSET_PRECISE, &sys_off)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_SYS_OFFSET_PRECISE",
              strerror(errno));
    return 0;
  }

  phc_ts->tv_sec = sys_off.device.sec;
  phc_ts->tv_nsec = sys_off.device.nsec;
  sys_ts->tv_sec = sys_off.sys_realtime.sec;
  sys_ts->tv_nsec = sys_off.sys_realtime.nsec;
  *err = MAX(LCL_GetSysPrecisionAsQuantum(), precision);

  return 1;
#else
  return 0;
#endif
}

/* ================================================== */

int
SYS_Linux_OpenPHC(const char *path, int phc_index)
{
  struct ptp_clock_caps caps;
  char phc_path[64];
  int phc_fd;

  if (!path) {
    if (snprintf(phc_path, sizeof (phc_path), "/dev/ptp%d", phc_index) >= sizeof (phc_path))
      return -1;
    path = phc_path;
  }

  phc_fd = open(path, O_RDONLY);
  if (phc_fd < 0) {
    LOG(LOGS_ERR, "Could not open %s : %s", path, strerror(errno));
    return -1;
  }

  /* Make sure it is a PHC */
  if (ioctl(phc_fd, PTP_CLOCK_GETCAPS, &caps)) {
    LOG(LOGS_ERR, "ioctl(%s) failed : %s", "PTP_CLOCK_GETCAPS", strerror(errno));
    close(phc_fd);
    return -1;
  }

  UTI_FdSetCloexec(phc_fd);

  return phc_fd;
}

/* ================================================== */

int
SYS_Linux_GetPHCSample(int fd, int nocrossts, double precision, int *reading_mode,
                       struct timespec *phc_ts, struct timespec *sys_ts, double *err)
{
  if ((*reading_mode == 2 || !*reading_mode) && !nocrossts &&
      get_precise_phc_sample(fd, precision, phc_ts, sys_ts, err)) {
    *reading_mode = 2;
    return 1;
  } else if ((*reading_mode == 3 || !*reading_mode) &&
      get_extended_phc_sample(fd, precision, phc_ts, sys_ts, err)) {
    *reading_mode = 3;
    return 1;
  } else if ((*reading_mode == 1 || !*reading_mode) &&
      get_phc_sample(fd, precision, phc_ts, sys_ts, err)) {
    *reading_mode = 1;
    return 1;
  }
  return 0;
}

/* ================================================== */

int
SYS_Linux_SetPHCExtTimestamping(int fd, int pin, int channel,
                                int rising, int falling, int enable)
{
  struct ptp_extts_request extts_req;
#ifdef PTP_PIN_SETFUNC
  struct ptp_pin_desc pin_desc;

  memset(&pin_desc, 0, sizeof (pin_desc));
  pin_desc.index = pin;
  pin_desc.func = enable ? PTP_PF_EXTTS : PTP_PF_NONE;
  pin_desc.chan = channel;

  if (ioctl(fd, PTP_PIN_SETFUNC, &pin_desc)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_PIN_SETFUNC", strerror(errno));
    return 0;
  }
#else
  DEBUG_LOG("Missing PTP_PIN_SETFUNC");
  return 0;
#endif

  memset(&extts_req, 0, sizeof (extts_req));
  extts_req.index = channel;
  extts_req.flags = (enable ? PTP_ENABLE_FEATURE : 0) |
                    (rising ? PTP_RISING_EDGE : 0) |
                    (falling ? PTP_FALLING_EDGE : 0);

  if (ioctl(fd, PTP_EXTTS_REQUEST, &extts_req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_EXTTS_REQUEST", strerror(errno));
    return 0;
  }

  return 1;
}

/* ================================================== */

int
SYS_Linux_ReadPHCExtTimestamp(int fd, struct timespec *phc_ts, int *channel)
{
  struct ptp_extts_event extts_event;

  if (read(fd, &extts_event, sizeof (extts_event)) != sizeof (extts_event)) {
    DEBUG_LOG("Could not read PHC extts event");
    return 0;
  }

  phc_ts->tv_sec = extts_event.t.sec;
  phc_ts->tv_nsec = extts_event.t.nsec;
  *channel = extts_event.index;

  return 1;
}

#endif

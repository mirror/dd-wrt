/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) John G. Hasler  2009
 * Copyright (C) Miroslav Lichvar  2009-2012, 2014-2018
 * Copyright (C) Shachar Raindel  2025
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
#include <poll.h>
#endif

#ifdef HAVE_LINUX_TIMESTAMPING
#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <net/if.h>
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
#include "socket.h"
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

  required_delta_tick = round(freq_ppm / dhz);

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
SYS_Linux_DropRoot(uid_t uid, gid_t gid, SYS_ProcessContext context, int clock_control)
{
  char cap_text[256];
  cap_t cap;

  if (prctl(PR_SET_KEEPCAPS, 1)) {
    LOG_FATAL("prctl() failed");
  }
  
  UTI_DropRoot(uid, gid);

  /* Keep CAP_NET_BIND_SERVICE if the NTP server sockets may need to be bound
     to a privileged port.
     Keep CAP_NET_RAW if an NTP socket may need to be bound to a device on
     kernels before 5.7.
     Keep CAP_SYS_TIME if the clock control is enabled. */
  if (snprintf(cap_text, sizeof (cap_text), "%s %s %s",
               (CNF_GetNTPPort() > 0 && CNF_GetNTPPort() < 1024) ?
                 "cap_net_bind_service=ep" : "",
               (CNF_GetBindNtpInterface() || CNF_GetBindAcquisitionInterface()) &&
                 !SYS_Linux_CheckKernelVersion(5, 7) ? "cap_net_raw=ep" : "",
               clock_control ? "cap_sys_time=ep" : "") >= sizeof (cap_text))
    assert(0);

  /* Helpers don't need any capabilities */
  if (context != SYS_MAIN_PROCESS)
    cap_text[0] = '\0';

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
SYS_Linux_EnableSystemCallFilter(int level, SYS_ProcessContext context)
{
  const int allowed[] = {
    /* Clock */
    SCMP_SYS(adjtimex),
    SCMP_SYS(clock_adjtime),
#ifdef __NR_clock_adjtime64
    SCMP_SYS(clock_adjtime64),
#endif
    SCMP_SYS(clock_gettime),
#ifdef __NR_clock_gettime64
    SCMP_SYS(clock_gettime64),
#endif
    SCMP_SYS(gettimeofday),
    SCMP_SYS(settimeofday),
    SCMP_SYS(time),

    /* Process */
    SCMP_SYS(clone),
#ifdef __NR_clone3
    SCMP_SYS(clone3),
#endif
    SCMP_SYS(exit),
    SCMP_SYS(exit_group),
    SCMP_SYS(getpid),
    SCMP_SYS(getrlimit),
    SCMP_SYS(getuid),
    SCMP_SYS(getuid32),
#ifdef __NR_membarrier
    SCMP_SYS(membarrier),
#endif
#ifdef __NR_rseq
    SCMP_SYS(rseq),
#endif
    SCMP_SYS(rt_sigaction),
    SCMP_SYS(rt_sigreturn),
    SCMP_SYS(rt_sigprocmask),
    SCMP_SYS(set_tid_address),
    SCMP_SYS(sigreturn),
    SCMP_SYS(wait4),
    SCMP_SYS(waitpid),

    /* Memory */
    SCMP_SYS(brk),
    SCMP_SYS(madvise),
    SCMP_SYS(mmap),
    SCMP_SYS(mmap2),
    SCMP_SYS(mprotect),
    SCMP_SYS(mremap),
    SCMP_SYS(munmap),
    SCMP_SYS(shmdt),

    /* Filesystem */
    SCMP_SYS(_llseek),
    SCMP_SYS(access),
    SCMP_SYS(chmod),
    SCMP_SYS(chown),
    SCMP_SYS(chown32),
    SCMP_SYS(faccessat),
    SCMP_SYS(fchmodat),
    SCMP_SYS(fchownat),
    SCMP_SYS(fstat),
    SCMP_SYS(fstat64),
    SCMP_SYS(fstatat64),
    SCMP_SYS(getdents),
    SCMP_SYS(getdents64),
    SCMP_SYS(lseek),
    SCMP_SYS(lstat),
    SCMP_SYS(lstat64),
    SCMP_SYS(newfstatat),
    SCMP_SYS(readlink),
    SCMP_SYS(readlinkat),
    SCMP_SYS(rename),
    SCMP_SYS(renameat),
#ifdef __NR_renameat2
    SCMP_SYS(renameat2),
#endif
    SCMP_SYS(stat),
    SCMP_SYS(stat64),
    SCMP_SYS(statfs),
    SCMP_SYS(statfs64),
#ifdef __NR_statx
    SCMP_SYS(statx),
#endif
    SCMP_SYS(unlink),
    SCMP_SYS(unlinkat),

    /* Socket */
    SCMP_SYS(accept),
    SCMP_SYS(bind),
    SCMP_SYS(connect),
    SCMP_SYS(getsockname),
    SCMP_SYS(getsockopt),
    SCMP_SYS(recv),
    SCMP_SYS(recvfrom),
    SCMP_SYS(recvmmsg),
#ifdef __NR_recvmmsg_time64
    SCMP_SYS(recvmmsg_time64),
#endif
    SCMP_SYS(recvmsg),
    SCMP_SYS(send),
    SCMP_SYS(sendmmsg),
    SCMP_SYS(sendmsg),
    SCMP_SYS(sendto),
    SCMP_SYS(shutdown),
    /* TODO: check socketcall arguments */
    SCMP_SYS(socketcall),

    /* General I/O */
    SCMP_SYS(_newselect),
    SCMP_SYS(close),
    SCMP_SYS(open),
    SCMP_SYS(openat),
    SCMP_SYS(pipe),
    SCMP_SYS(pipe2),
    SCMP_SYS(poll),
    SCMP_SYS(ppoll),
#ifdef __NR_ppoll_time64
    SCMP_SYS(ppoll_time64),
#endif
    SCMP_SYS(pread64),
    SCMP_SYS(pselect6),
#ifdef __NR_pselect6_time64
    SCMP_SYS(pselect6_time64),
#endif
    SCMP_SYS(read),
    SCMP_SYS(futex),
#ifdef __NR_futex_time64
    SCMP_SYS(futex_time64),
#endif
    SCMP_SYS(select),
    SCMP_SYS(set_robust_list),
    SCMP_SYS(write),
    SCMP_SYS(writev),

    /* Miscellaneous */
    SCMP_SYS(getrandom),
    SCMP_SYS(sysinfo),
    SCMP_SYS(uname),
  };

  const int denied_any[] = {
    SCMP_SYS(execve),
#ifdef __NR_execveat
    SCMP_SYS(execveat),
#endif
    SCMP_SYS(fork),
    SCMP_SYS(ptrace),
    SCMP_SYS(vfork),
  };

  const int denied_ntske[] = {
    SCMP_SYS(ioctl),
    SCMP_SYS(setsockopt),
    SCMP_SYS(socket),
  };

  const int socket_domains[] = {
    AF_NETLINK, AF_UNIX, AF_INET,
#ifdef FEAT_IPV6
    AF_INET6,
#endif
  };

  const static int socket_options[][2] = {
    { SOL_IP, IP_PKTINFO }, { SOL_IP, IP_FREEBIND }, { SOL_IP, IP_TOS },
#ifdef FEAT_IPV6
    { SOL_IPV6, IPV6_V6ONLY }, { SOL_IPV6, IPV6_RECVPKTINFO },
#ifdef IPV6_TCLASS
    { SOL_IPV6, IPV6_TCLASS },
#endif
#endif
#ifdef SO_BINDTODEVICE
    { SOL_SOCKET, SO_BINDTODEVICE },
#endif
    { SOL_SOCKET, SO_BROADCAST }, { SOL_SOCKET, SO_REUSEADDR },
#ifdef SO_REUSEPORT
    { SOL_SOCKET, SO_REUSEPORT },
#endif
    { SOL_SOCKET, SO_TIMESTAMP }, { SOL_SOCKET, SO_TIMESTAMPNS },
#ifdef HAVE_LINUX_TIMESTAMPING
    { SOL_SOCKET, SO_SELECT_ERR_QUEUE }, { SOL_SOCKET, SO_TIMESTAMPING },
#endif
  };

  const static int fcntls[] = { F_GETFD, F_SETFD, F_GETFL, F_SETFL };

  const static unsigned long ioctls[] = {
    FIONREAD, TCGETS, TIOCGWINSZ,
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

  unsigned int default_action, deny_action;
  scmp_filter_ctx *ctx;
  int i;

  /* Sign of the level determines the deny action (kill or SIGSYS).
     At level 1, selected syscalls are allowed, others are denied.
     At level 2, selected syscalls are denied, others are allowed. */

  deny_action = level > 0 ? SCMP_ACT_KILL : SCMP_ACT_TRAP;
  if (level < 0)
    level = -level;

  switch (level) {
    case 1:
      default_action = deny_action;
      break;
    case 2:
      default_action = SCMP_ACT_ALLOW;
      break;
    default:
      LOG_FATAL("Unsupported filter level");
  }

  if (context == SYS_MAIN_PROCESS) {
    /* Check if the chronyd configuration is supported */
    check_seccomp_applicability();

    /* At level 1, start a helper process which will not have a seccomp filter.
       It will be used for getaddrinfo(), for which it is difficult to maintain
       a list of required system calls (with glibc it depends on what NSS
       modules are installed and enabled on the system). */
    if (default_action != SCMP_ACT_ALLOW)
      PRV_StartHelper();
  }

  ctx = seccomp_init(default_action);
  if (ctx == NULL)
      LOG_FATAL("Failed to initialize seccomp");

  if (default_action != SCMP_ACT_ALLOW) {
    for (i = 0; i < sizeof (allowed) / sizeof (*allowed); i++) {
      if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, allowed[i], 0) < 0)
        goto add_failed;
    }
  } else {
    for (i = 0; i < sizeof (denied_any) / sizeof (*denied_any); i++) {
      if (seccomp_rule_add(ctx, deny_action, denied_any[i], 0) < 0)
        goto add_failed;
    }

    if (context == SYS_NTSKE_HELPER) {
      for (i = 0; i < sizeof (denied_ntske) / sizeof (*denied_ntske); i++) {
        if (seccomp_rule_add(ctx, deny_action, denied_ntske[i], 0) < 0)
          goto add_failed;
      }
    }
  }

  if (default_action != SCMP_ACT_ALLOW && context == SYS_MAIN_PROCESS) {
    /* Allow opening sockets in selected domains */
    for (i = 0; i < sizeof (socket_domains) / sizeof (*socket_domains); i++) {
      if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(socket), 1,
                           SCMP_A0(SCMP_CMP_EQ, socket_domains[i])) < 0)
        goto add_failed;
    }

    /* Allow selected socket options */
    for (i = 0; i < sizeof (socket_options) / sizeof (*socket_options); i++) {
      if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setsockopt), 2,
                           SCMP_A1(SCMP_CMP_EQ, socket_options[i][0]),
                           SCMP_A2(SCMP_CMP_EQ, socket_options[i][1])))
        goto add_failed;
    }

    /* Allow selected fcntl calls */
    for (i = 0; i < sizeof (fcntls) / sizeof (*fcntls); i++) {
      if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 1,
                           SCMP_A1(SCMP_CMP_EQ, fcntls[i])) < 0 ||
          seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fcntl64), 1,
                           SCMP_A1(SCMP_CMP_EQ, fcntls[i])) < 0)
        goto add_failed;
    }

    /* Allow selected ioctls */
    for (i = 0; i < sizeof (ioctls) / sizeof (*ioctls); i++) {
      if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 1,
                           SCMP_A1(SCMP_CMP_EQ, ioctls[i])) < 0)
        goto add_failed;
    }
  }

  if (seccomp_load(ctx) < 0)
    LOG_FATAL("Failed to load seccomp rules");

  LOG(context == SYS_MAIN_PROCESS ? LOGS_INFO : LOGS_DEBUG,
      "Loaded seccomp filter (level %d)", level);
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

static int
get_phc_readings(int phc_fd, int max_samples, struct timespec ts[][3])
{
  struct ptp_sys_offset sys_off;
  int i;

  max_samples = CLAMP(0, max_samples, PTP_MAX_SAMPLES);

  /* Silence valgrind */
  memset(&sys_off, 0, sizeof (sys_off));

  sys_off.n_samples = max_samples;

  if (ioctl(phc_fd, PTP_SYS_OFFSET, &sys_off)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_SYS_OFFSET", strerror(errno));
    return 0;
  }

  for (i = 0; i < max_samples; i++) {
    ts[i][0].tv_sec = sys_off.ts[i * 2].sec;
    ts[i][0].tv_nsec = sys_off.ts[i * 2].nsec;
    ts[i][1].tv_sec = sys_off.ts[i * 2 + 1].sec;
    ts[i][1].tv_nsec = sys_off.ts[i * 2 + 1].nsec;
    ts[i][2].tv_sec = sys_off.ts[i * 2 + 2].sec;
    ts[i][2].tv_nsec = sys_off.ts[i * 2 + 2].nsec;
  }

  return max_samples;
}

/* ================================================== */

static int
get_extended_phc_readings(int phc_fd, int max_samples, struct timespec ts[][3])
{
#ifdef PTP_SYS_OFFSET_EXTENDED
  struct ptp_sys_offset_extended sys_off;
  int i;

  max_samples = CLAMP(0, max_samples, PTP_MAX_SAMPLES);

  /* Silence valgrind */
  memset(&sys_off, 0, sizeof (sys_off));

  sys_off.n_samples = max_samples;

  if (ioctl(phc_fd, PTP_SYS_OFFSET_EXTENDED, &sys_off)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_SYS_OFFSET_EXTENDED", strerror(errno));
    return 0;
  }

  for (i = 0; i < max_samples; i++) {
    ts[i][0].tv_sec = sys_off.ts[i][0].sec;
    ts[i][0].tv_nsec = sys_off.ts[i][0].nsec;
    ts[i][1].tv_sec = sys_off.ts[i][1].sec;
    ts[i][1].tv_nsec = sys_off.ts[i][1].nsec;
    ts[i][2].tv_sec = sys_off.ts[i][2].sec;
    ts[i][2].tv_nsec = sys_off.ts[i][2].nsec;
  }

  return max_samples;
#else
  return 0;
#endif
}

/* ================================================== */

static int
get_precise_phc_readings(int phc_fd, int max_samples, struct timespec ts[][3])
{
#ifdef PTP_SYS_OFFSET_PRECISE
  struct ptp_sys_offset_precise sys_off;

  if (max_samples < 1)
    return 0;

  /* Silence valgrind */
  memset(&sys_off, 0, sizeof (sys_off));

  if (ioctl(phc_fd, PTP_SYS_OFFSET_PRECISE, &sys_off)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_SYS_OFFSET_PRECISE",
              strerror(errno));
    return 0;
  }

  ts[0][0].tv_sec = sys_off.sys_realtime.sec;
  ts[0][0].tv_nsec = sys_off.sys_realtime.nsec;
  ts[0][1].tv_sec = sys_off.device.sec;
  ts[0][1].tv_nsec = sys_off.device.nsec;
  ts[0][2] = ts[0][0];

  return 1;
#else
  return 0;
#endif
}

/* ================================================== */

/* Make sure an FD is a PHC.  Return the FD if it is, or close the FD
   and return -1 if it is not. */

static int
verify_fd_is_phc(int phc_fd)
{
  struct ptp_clock_caps caps;

  if (ioctl(phc_fd, PTP_CLOCK_GETCAPS, &caps)) {
    LOG(LOGS_ERR, "ioctl(%s) failed : %s", "PTP_CLOCK_GETCAPS", strerror(errno));
    close(phc_fd);
    return -1;
  }

  return phc_fd;
}

/* ================================================== */

static int
open_phc_by_iface_name(const char *iface)
{
#ifdef HAVE_LINUX_TIMESTAMPING
  struct ethtool_ts_info ts_info;
  char phc_device[PATH_MAX];
  struct ifreq req;
  int sock_fd;

  sock_fd = SCK_OpenUdpSocket(NULL, NULL, NULL, 0);
  if (sock_fd < 0)
    return -1;

  memset(&req, 0, sizeof (req));
  memset(&ts_info, 0, sizeof (ts_info));

  if (snprintf(req.ifr_name, sizeof (req.ifr_name), "%s", iface) >=
      sizeof (req.ifr_name)) {
    SCK_CloseSocket(sock_fd);
    return -1;
  }

  ts_info.cmd = ETHTOOL_GET_TS_INFO;
  req.ifr_data = (char *)&ts_info;

  if (ioctl(sock_fd, SIOCETHTOOL, &req)) {
    DEBUG_LOG("ioctl(%s) failed : %s", "SIOCETHTOOL", strerror(errno));
    SCK_CloseSocket(sock_fd);
    return -1;
  }

  /* Simplify failure paths by closing the socket as early as possible */
  SCK_CloseSocket(sock_fd);
  sock_fd = -1;

  if (ts_info.phc_index < 0) {
    DEBUG_LOG("PHC missing on %s", req.ifr_name);
    return -1;
  }

  if (snprintf(phc_device, sizeof (phc_device),
               "/dev/ptp%d", ts_info.phc_index) >= sizeof (phc_device))
    return -1;

  return open(phc_device, O_RDONLY);
#else
  return -1;
#endif
}

/* ================================================== */

int
SYS_Linux_OpenPHC(const char *device)
{
  int phc_fd = -1;

  if (device[0] == '/') {
    phc_fd = open(device, O_RDONLY);
    if (phc_fd >= 0)
      phc_fd = verify_fd_is_phc(phc_fd);
  }

  if (phc_fd < 0) {
    phc_fd = open_phc_by_iface_name(device);
    if (phc_fd < 0) {
      LOG(LOGS_ERR, "Could not open PHC of iface %s : %s",
          device, strerror(errno));
      return -1;
    }
    phc_fd = verify_fd_is_phc(phc_fd);
  }

  if (phc_fd >= 0)
    UTI_FdSetCloexec(phc_fd);

  return phc_fd;
}

/* ================================================== */

int
SYS_Linux_GetPHCReadings(int fd, int nocrossts, int *reading_mode, int max_readings,
                         struct timespec tss[][3])
{
  int r = 0;

  if ((*reading_mode == 2 || *reading_mode == 0) && !nocrossts &&
      (r = get_precise_phc_readings(fd, max_readings, tss)) > 0) {
    *reading_mode = 2;
  } else if ((*reading_mode == 3 || *reading_mode == 0) &&
             (r = get_extended_phc_readings(fd, max_readings, tss)) > 0) {
    *reading_mode = 3;
  } else if ((*reading_mode == 1 || *reading_mode == 0) &&
             (r = get_phc_readings(fd, max_readings, tss)) > 0) {
    *reading_mode = 1;
  }

  return r;
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

  if (pin >= 0 && ioctl(fd, PTP_PIN_SETFUNC, &pin_desc)) {
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

#if defined(PTP_MASK_CLEAR_ALL) && defined(PTP_MASK_EN_SINGLE)
  /* Disable events from other channels on this descriptor */
  if (ioctl(fd, PTP_MASK_CLEAR_ALL))
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_MASK_CLEAR_ALL", strerror(errno));
  else if (ioctl(fd, PTP_MASK_EN_SINGLE, &channel))
    DEBUG_LOG("ioctl(%s) failed : %s", "PTP_MASK_EN_SINGLE", strerror(errno));
#endif

  return 1;
}

/* ================================================== */

int
SYS_Linux_ReadPHCExtTimestamp(int fd, struct timespec *phc_ts, int *channel)
{
  struct ptp_extts_event extts_event;
  struct pollfd pfd;

  /* Make sure the read will not block in case we have multiple
     descriptors of the same PHC (O_NONBLOCK does not work) */
  pfd.fd = fd;
  pfd.events = POLLIN;
  if (poll(&pfd, 1, 0) != 1 || pfd.revents != POLLIN) {
    DEBUG_LOG("Missing PHC extts event");
    return 0;
  }

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

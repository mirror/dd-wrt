/*
 * Copyright (C) 2011-2014 Chuck Coffing <clc@alum.mit.edu>
 * Copyright (C) 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com> (aarch64, mips64, powerpc, x86_64 changes, specific code for dd-wrt)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <signal.h>
#if defined(HAVE_SYSLOG) && !defined(HAVE_MICRO)

//#ifdef __ANDROID__
//#define AIRBAG_NO_BACKTRACE
//#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <asm/byteorder.h>
#include <arpa/inet.h> /* for htonl */
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <syslog.h>
#ifdef TEST
#define dd_syslog(a, args...) \
	do {                  \
	} while (0)
#else
#include <utils.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ucontext.h>
#include <unistd.h>
#ifdef __linux__
#include <sys/prctl.h>
#include <sys/syscall.h>
#endif
#include <pthread.h>

#if defined(__cplusplus)
#include <cxxabi.h>
/*
 * Theoretically might want to disable this, because __cxa_demangle calls malloc/free, which could
 * deadlock or crash when called from signal inside malloc/free.  But pre-malloc a large buffer
 * ahead of time, and that shouldn't actually happen.
 */
#define USE_GCC_DEMANGLE
#define AIRBAG_EXPORT extern "C"
#else
#define AIRBAG_EXPORT
#endif

#if defined(__GNUC__) && !defined(__clang__)
#include <unwind.h>
#define USE_GCC_UNWIND
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 0 /* Supported starting in Linux 2.6.23 */
#endif

#ifndef PR_SET_NAME
#define PR_SET_NAME 15 /* Set process name */
#define PR_GET_NAME 16 /* Get process name */
#endif

#ifndef SI_TKILL
#define SI_TKILL (-6)
#endif

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
static uint32_t busy, pending;
#endif

#if defined(USE_GCC_DEMANGLE)
static char *s_demangleBuf;
static size_t s_demangleBufLen;
#endif
sighandler_t delegate;
#define ALT_STACK_SIZE (MINSIGSTKSZ + 256 * sizeof(void *)) /* or let it all hang out: SIGSTKSZ */
static void *s_altStackSpace;

static const char comment[] = "# ";
static const char section[] = "=== ";
static const char unknown[] = "\?\?\?";
static const char termBt[] = "terminating backtrace";

#define MAX_SIGNALS 32
static const char *sigNames[MAX_SIGNALS] = {
	/*[0        ] = */ NULL,
	/*[SIGHUP   ] = */ "HUP",
	/*[SIGINT   ] = */ "INT",
	/*[SIGQUIT  ] = */ "QUIT",
	/*[SIGILL   ] = */ "ILL",
	/*[SIGTRAP  ] = */ "TRAP",
	/*[SIGABRT  ] = */ "ABRT",
#if defined(__mips__)
	/*[SIGEMT   ] = */ "EMT",
#else
	/*[SIGBUS   ] = */ "BUS",
#endif
	/*[SIGFPE   ] = */ "FPE",
	/*[SIGKILL  ] = */ "KILL",
#if defined(__mips__)
	/*[SIGBUS   ] = */ "BUS",
#else
	/*[SIGUSR1  ] = */ "USR1",
#endif
	/*[SIGSEGV  ] = */ "SEGV",
#if defined(__mips__)
	/*[SIGSYS  ] =  */ "USR2",
#else
	/*[SIGUSR2  ] = */ "USR2",
#endif
	/*[SIGPIPE  ] = */ "PIPE",
	/*[SIGALRM  ] = */ "ALRM",
	/*[SIGTERM  ] = */ "TERM",
#if defined(__mips__)
	/*[SIGUSR1  ] = */ "SIGUSR1",
	/*[SIGUSR2  ] = */ "SIGUSR2",
	/*[SIGCHLD  ] = */ "CHLD",
	/*[SIGPWR  ] = */ "PWR",
	/*[SIGWINCH  ] = */ "WINCH",
	/*[SIGURG  ] = */ "URG",
	/*[SIGIO  ] = */ "IO",
	/*[SIGSTOP   ] = */ "STOP",
	/*[SIGTSTP  ] = */ "TSTP",
	/*[SIGCONT  ] = */ "CONT",
	/*[SIGTTIN] = */ "TTIN",
	/*[SIGTTOU  ] = */ "TTOU",
	/*[SIGVTALRM ] = */ "VTALRM",
	/*[SIGPROF    ] = */ "PROF",
	/*[SIGXCPU   ] = */ "XCPU",
	/*[SIGXFSZ   ] = */ "XFSZ"
#else
	/*[SIGSTKFLT] = */ "STKFLT",
	/*[SIGCHLD  ] = */ "CHLD",
	/*[SIGCONT  ] = */ "CONT",
	/*[SIGSTOP  ] = */ "STOP",
	/*[SIGTSTP  ] = */ "TSTP",
	/*[SIGTTIN  ] = */ "TTIN",
	/*[SIGTTOU  ] = */ "TTOU",
	/*[SIGURG   ] = */ "URG",
	/*[SIGXCPU  ] = */ "XCPU",
	/*[SIGXFSZ  ] = */ "XFSZ",
	/*[SIGVTALRM] = */ "VTALRM",
	/*[SIGPROF  ] = */ "PROF",
	/*[SIGWINCH ] = */ "WINCH",
	/*[SIGIO    ] = */ "IO",
	/*[SIGPWR   ] = */ "PWR",
	/*[SIGSYS   ] = */ "SYS"
#endif
};

#ifdef __LP64__
#if defined(__aarch64__) || defined(__mips__) || defined(__powerpc__)
#define INSTLEN unsigned int
#define INSTLENFMT "08"
#else
#define INSTLEN unsigned long
#define INSTLENFMT "016"
#endif
#define FMTBIT "016"
#else
#define FMTBIT "08"
#define INSTLEN unsigned int
#define INSTLENFMT "08"
#endif
/*
 * Do not use strsignal; it is not async signal safe.
 */
static const char *_strsignal(int sigNum)
{
	return sigNum < 1 || sigNum >= MAX_SIGNALS ? unknown : sigNames[sigNum];
}

#if defined(__x86_64__)
#include "sysdeps/generic_backtrace.c"
#define NMCTXREGS NGREG
#define MCTXREG(uc, i) (uc->uc_mcontext.gregs[i])
#define MCTX_PC(uc) MCTXREG(uc, 16)
static const char *mctxRegNames[NMCTXREGS] = { "R8",  "R9",  "R10",    "R11", "R12",	"R13",	   "R14", "R15",
					       "RDI", "RSI", "RBP",    "RBX", "RDX",	"RAX",	   "RCX", "RSP",
					       "RIP", "EFL", "CSGSFS", "ERR", "TRAPNO", "OLDMASK", "CR2" };

#define FMTLEN "07"
#elif defined(__i386__)
#include "sysdeps/x86_backtrace.c"
#define NMCTXREGS NGREG
#define MCTXREG(uc, i) (uc->uc_mcontext.gregs[i])
#define MCTX_PC(uc) MCTXREG(uc, 14)
static const char *mctxRegNames[NMCTXREGS] = { "GS",  "FS",  "ES",     "DS",  "EDI", "ESI", "EBP", "ESP",  "EBX", "EDX",
					       "ECX", "EAX", "TRAPNO", "ERR", "EIP", "CS",  "EFL", "UESP", "SS" };

#define FMTLEN "06"
#elif defined(__aarch64__)
#include "sysdeps/generic_backtrace.c"
#define NMCTXREGS 31
#define MCTXREG(uc, i) (uc->uc_mcontext.regs[i])
#define MCTX_PC(uc) (uc->uc_mcontext.pc)
static const char *mctxRegNames[NMCTXREGS] = { "X0",  "X1",  "X2",  "X3",  "X4",  "X5",	 "X6",	"X7",  "X8",  "X9",  "X10",
					       "X11", "X12", "X13", "X14", "X15", "X16", "X17", "X18", "X19", "X20", "X21",
					       "X22", "X23", "X24", "X25", "X26", "X27", "X28", "X29", "X30" };

#define FMTLEN "03"
#elif defined(__arm__)
#include "sysdeps/arm_backtrace.c"
#define NMCTXREGS 21
#define MCTXREG(uc, i) (((unsigned long *)(&uc->uc_mcontext))[i])
#define MCTX_PC(uc) MCTXREG(uc, 18)
static const char *mctxRegNames[NMCTXREGS] = { "TRAPNO", "ERRCODE", "OLDMASK", "R0", "R1", "R2",   "R3",
					       "R4",	 "R5",	    "R6",      "R7", "R8", "R9",   "R10",
					       "FP",	 "IP",	    "SP",      "LR", "PC", "CPSR", "FAULTADDR" };

#define FMTLEN "09"
static const int gregOffset = 3;
#elif defined(__mips__)
#include "sysdeps/generic_backtrace.c"
#define NMCTXREGS NGREG
#ifdef CTX_EPC /* Pre-2007 uclibc */
#define MCTXREG(uc, i) (uc->uc_mcontext.gpregs[i])
#define MCTX_PC(uc) (uc->uc_mcontext.gpregs[35])
#else
#define MCTXREG(uc, i) (uc->uc_mcontext.gregs[i])
#define MCTX_PC(uc) (uc->uc_mcontext.pc)
#endif
static const char *mctxRegNames[NMCTXREGS] = { "ZERO", "AT", "V0", "V1", "A0", "A1", "A2", "A3",
#if _MIPS_SIM == _ABIO32
					       "T0",   "T1", "T2", "T3",
#else
					       "A4",   "A5", "A6", "A7",
#endif
					       "T4",   "T5", "T6", "T7", "S0", "S1", "S2", "S3", "S4", "S5",
					       "S6",   "S7", "T8", "T9", "K0", "K1", "GP", "SP", "FP", "RA" };

#define FMTLEN "04"
#elif defined(__powerpc__)
#include "sysdeps/generic_backtrace.c"
#define NMCTXREGS 45
#ifdef __UCLIBC__
#define MCTX_PC(uc) (uc->uc_mcontext.uc_regs->gregs[32])
#define MCTXREG(uc, i) (uc->uc_mcontext.uc_regs->gregs[i])
#else
#define MCTX_PC(uc) (uc->uc_mcontext.gregs[32])
#define MCTXREG(uc, i) (uc->uc_mcontext.gregs[i])
#endif

static const char *mctxRegNames[NMCTXREGS] = { "GPR0",	"GPR1",	 "GPR2",    "GPR3",   "GPR4",  "GPR5",	"GPR6",	 "GPR7",
					       "GPR8",	"GPR9",	 "GPR10",   "GPR11",  "GPR12", "GPR13", "GPR14", "GPR15",
					       "GPR16", "GPR17", "GPR18",   "GPR19",  "GPR20", "GPR21", "GPR22", "GPR23",
					       "GPR24", "GPR25", "GPR26",   "GPR27",  "GPR28", "GPR29", "GPR30", "GPR31",
					       "NIP",	"MSR",	 "ORIG_R3", "CTR",    "LNK",   "XER",	"CCR",
#ifndef __powerpc64__
					       "MQ",
#else
					       "SOFTE",
#endif
					       "TRAP",	"DAR",	 "DSISR",   "RESULT", "DSCR" };

#define FMTLEN "07"
#endif

#if 0
#if defined(__MACH__)
#if __DARWIN_UNIX03
#if defined(__i386__)
pnt = (void *)uc->uc_mcontext->__ss.__eip;
#elif defined(__arm__)
/* don't see mcontext in iphone headers... */
#else
/* pnt = (void*) uc->uc_mcontext->__ss.__srr0; */
#endif
#else
#if defined(__i386__)
pnt = (void *)uc->uc_mcontext->ss.eip;
#elif defined(__arm__)
/* don't see mcontext in iphone headers... */
#else
pnt = (void *)uc->uc_mcontext->ss.srr0;
#endif
#endif
#elif defined(__FreeBSD__)
#if defined(__i386__)
pnt = (void *)uc->uc_mcontext.mc_eip;
#elif defined(__x86_64__)
pnt = (void *)uc->uc_mcontext.mc_rip;
#endif
#elif (defined(__ppc__)) || (defined(__powerpc__))
pnt = (void *)uc->uc_mcontext.regs->nip;
#elif defined(__i386__)
pnt = (void *)uc->uc_mcontext.gregs[REG_EIP];
#elif defined(__x86_64__)
pnt = (void *)uc->uc_mcontext.gregs[REG_RIP];
#elif defined(__mips__)
#ifdef CTX_EPC /* Pre-2007 uclibc */
pnt = (void *)uc->uc_mcontext.gpregs[CTX_EPC];
#else
pnt = (void *)uc->uc_mcontext.pc;
#endif
#endif
#endif

static uint8_t load8(const void *_p, uint8_t *v)
{
	static int fds[2] = { -1, -1 };
	uint8_t b;
	int r;
	const uint8_t *p = (const uint8_t *)_p;

	if (fds[0] == -1) {
		r = pipe(fds);
		(void)r; /* even on failure, degrades gracefully if memory is readable */
	}

	if (v)
		*v = 0;
	errno = 0;
	while ((r = write(fds[1], p, 1)) < 1 && errno == EINTR) {
		;
	}
	if (r == 1) {
		while ((r = read(fds[0], v ? v : &b, 1)) < 1 && errno == EINTR) {
			;
		}
		if (r == 1)
			return 0;
	}
	if (errno == EFAULT)
		return 0xff;
	if (v)
		*v = *p; /* Risk it... */
	return 0;
}

static uint32_t load32(const void *_p, INSTLEN *_v)
{
	int i;
	INSTLEN r = 0;
	INSTLEN v = 0;
	const uint8_t *p = (const uint8_t *)_p;
	for (i = 0; i < sizeof(INSTLEN); ++i) {
		uint8_t b;
		r <<= 8;
		v <<= 8;
		r |= load8(p + i, &b);
		v |= b;
	}
	if (sizeof(INSTLEN) == 8) {
		v = __cpu_to_be64(v);
		r = __cpu_to_be64(r);
	} else {
		v = htonl(v);
		r = htonl(r);
	}
	if (_v)
		*_v = v;
	return r;
}

static uint32_t loadlong(const void *_p, unsigned long *_v)
{
	int i;
	INSTLEN r = 0;
	INSTLEN v = 0;
	const uint8_t *p = (const uint8_t *)_p;
	for (i = 0; i < sizeof(INSTLEN); ++i) {
		uint8_t b;
		r <<= 8;
		v <<= 8;
		r |= load8(p + i, &b);
		v |= b;
	}
	if (sizeof(*_v) == 8) {
		v = __cpu_to_be64(v);
		r = __cpu_to_be64(r);
	} else {
		v = htonl(v);
		r = htonl(r);
	}
	if (_v)
		*_v = v;
	return r;
}

/* some loaned from musl libc, modified to fix crash problem in signal context */
static const struct {
	short sun_family;
	char sun_path[9];
} log_addr = { AF_UNIX, "/dev/log" };

static int log_fd = -1;
static int log_facility = LOG_USER;
static int log_opt;
static char log_ident[32];

static int is_lost_conn(int e)
{
	return e == ECONNREFUSED || e == ECONNRESET || e == ENOTCONN || e == EPIPE;
}

#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 02000000
#define SOCK_NONBLOCK 04000
#endif
static int postindex = -1;
static char *postinfo[33];
static void slog(int priority, const char *message)
{
	char *buf;
	int errno_save = errno;
	int pid;
	int l;
	int hlen;
	int fd;
	if (log_fd < 0) {
		log_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
		if (log_fd >= 0)
			connect(log_fd, (void *)&log_addr, sizeof log_addr);
	}
	if (!(priority & LOG_FACMASK))
		priority |= log_facility;
	pid = (log_opt & LOG_PID) ? getpid() : 0;
	l = 0;
	l = asprintf(&buf, "<%d>%n%s%s%.0d%s: %s", priority, &hlen, log_ident, "[" + !pid, pid, "]" + !pid, message);
	errno = errno_save;
	if (l >= 0) {
		if (send(log_fd, buf, l, 0) < 0 &&
		    (!is_lost_conn(errno) || connect(log_fd, (void *)&log_addr, sizeof log_addr) < 0 ||
		     send(log_fd, buf, l, 0) < 0) &&
		    (log_opt & LOG_CONS)) {
			fd = open("/dev/console", O_WRONLY | O_NOCTTY | O_CLOEXEC);
			if (fd >= 0) {
				dprintf(fd, "%.*s", l - hlen, buf + hlen);
				close(fd);
			}
		}
		if (log_opt & LOG_PERROR)
			dprintf(2, "%.*s", l - hlen, buf + hlen);
	}
	if (buf)
		free(buf);
}

static int airbag_printf(char *fmt, ...)
{
	static char *ss_buffer = NULL;
	static char temp[128];
	;
	va_list args;
	va_start(args, fmt);
	int size = vsnprintf(temp, 128, fmt, args);
	va_end(args);

	if (!size || size < 0)
		return 0;
	if (!ss_buffer) {
		ss_buffer = strdup(temp);
	} else {
		ss_buffer = realloc(ss_buffer, strlen(ss_buffer) + strlen(temp) + 1);
		strcat(ss_buffer, temp);
	}
	if (strchr(ss_buffer, '\n')) {
		slog(LOG_ERR, ss_buffer);
		fprintf(stderr, "%s", ss_buffer);
		free(ss_buffer);
		ss_buffer = NULL;
	}
	return size;
}

static const char *demangle(const char *mangled)
{
	if (!mangled)
		return unknown;
#if defined(USE_GCC_DEMANGLE)
	int status;
	char *newBuf = abi::__cxa_demangle(mangled, s_demangleBuf, &s_demangleBufLen, &status);
	if (newBuf) {
		s_demangleBuf = newBuf;
	}
	if (status == 0)
		return s_demangleBuf;
#endif
	return mangled;
}

static void _airbag_symbol(void *pc, const char *sname, void *saddr)
{
	int printed = 0;

#if !defined(AIRBAG_NO_DLADDR)
	Dl_info info;
	if (dladdr(pc, &info)) {
		unsigned long offset;
		if (info.dli_sname && info.dli_saddr) {
			sname = info.dli_sname;
			saddr = info.dli_saddr;
		} else if (!sname || !saddr) { /* dladdr and heuristic both failed; offset from start of so */
			sname = "";
			saddr = info.dli_fbase;
		}
		offset = (ptrdiff_t)pc - (ptrdiff_t)saddr;
		airbag_printf("%s[0x%" FMTBIT "lx](%s+0x%" FMTBIT "lx)[0x%" FMTBIT "lx]", info.dli_fname,
			      (unsigned long)info.dli_fbase, demangle(sname), offset, (unsigned long)pc);
		printed = 1;
	}
#endif
	if (!printed) {
		airbag_printf("%s(+0)[0x%" FMTBIT "lx]", unknown, (unsigned long)pc);
	}
}

static void airbag_symbol(void *pc)
{
	_airbag_symbol(pc, 0, 0);
}

#if defined(USE_GCC_UNWIND) && !defined(__mips__) && !defined(__arm__)

/* Dummy version in case libgcc_s does not contain the real code.  */
static _Unwind_Word airbag_dummy_getcfa(struct _Unwind_Context *ctx __attribute__((unused)))
{
	return 0;
}

typedef _Unwind_Ptr (*Unwind_GetIP_T)(struct _Unwind_Context *);
typedef _Unwind_Reason_Code (*Unwind_Backtrace_T)(_Unwind_Trace_Fn, void *);
static Unwind_GetIP_T _unwind_GetIP;
static _Unwind_Word (*_unwind_GetCFA)(struct _Unwind_Context *);
static _Unwind_Reason_Code airbag_backtrace_helper(struct _Unwind_Context *ctx, void *a)
{
	struct trace_arg *arg = (struct trace_arg *)a;

	/*  We are first called with address in the __backtrace function. Skip it. */
	if (arg->cnt != -1) {
		arg->array[arg->cnt] = (void *)_unwind_GetIP(ctx);
		/* Check whether we make any progress.  */
		_Unwind_Word cfa = _unwind_GetCFA(ctx);

		if (arg->cnt > 0 && arg->array[arg->cnt - 1] == arg->array[arg->cnt] && cfa == arg->cfa)
			return _URC_END_OF_STACK;
		arg->cfa = cfa;
	}
	if (++arg->cnt >= arg->size)
		return _URC_END_OF_STACK;
	return _URC_NO_REASON;
}
#endif

#ifdef __arm__
/*
 * Search for function name embedded via gcc's -mpoke-function-name.
 * addr should point near the top of the function.
 * If found, populates fname and returns pointer to start of function.
 */
static void *getPokedFnName(uint32_t addr, char *fname)
{
	unsigned int i;
	void *faddr = 0;

	addr &= ~3;
	/* GCC man page suggests len is at "pc - 12", but prologue can vary, so scan back */
	for (i = 0; i < 16; ++i) {
		INSTLEN len;
		addr -= 4;
		if (load32((void *)addr, &len) == 0 && (len & 0xffffff00) == 0xff000000) {
			uint32_t offset;
			len &= 0xff;
			faddr = (void *)(addr + 4);
			addr -= len;
			for (offset = 0; offset < len; ++offset) {
				uint8_t c;
				if (load8((void *)(addr + offset), &c))
					break;
				fname[offset] = c;
			}
			fname[offset] = 0;
			airbag_printf("%sFound poked function name: %s[0x%" FMTBIT "lx]\n", comment, fname, faddr);
			break;
		}
	}
	return faddr;
}
#endif

static int airbag_walkstack(void **buffer, int *repeat, int size, ucontext_t *uc)
{
	(void)uc;

	memset(repeat, 0, sizeof(int) * size);
#if defined(__mips__)
	/* Algorithm derived from:
	 * http://elinux.org/images/6/68/ELC2008_-_Back-tracing_in_MIPS-based_Linux_Systems.pdf
	 */
	unsigned long *addr, *pc, *ra, *sp;
	unsigned long raOffset, stackSize;

#ifdef CTX_EPC /* Pre-2007 uclibc */
	pc = (unsigned long *)uc->uc_mcontext.gpregs[35];
	ra = (unsigned long *)uc->uc_mcontext.gpregs[31];
	sp = (unsigned long *)uc->uc_mcontext.gpregs[29];
#else
	pc = (unsigned long *)uc->uc_mcontext.pc;
	ra = (unsigned long *)uc->uc_mcontext.gregs[31];
	sp = (unsigned long *)uc->uc_mcontext.gregs[29];
#endif
	int depth = 0;
	buffer[depth++] = pc;
	if (size == 1)
		return depth;

	/* Scanning to find the size of the current stack frame */
	raOffset = stackSize = 0;
	for (addr = pc; !raOffset || !stackSize; --addr) {
		INSTLEN v;
		if (load32(addr, &v)) {
			airbag_printf("%sText at 0x%" FMTBIT "lx is not mapped; trying prior frame pointer.\n", comment, addr);
#ifdef CTX_EPC /* Pre-2007 uclibc */
			uc->uc_mcontext.gpregs[35] = (unsigned long)ra;
#else
			uc->uc_mcontext.pc = (unsigned long)ra;
#endif
			goto backward;
		}
		switch (v & 0xffff0000) {
		case 0x27bd0000: /* addiu   sp,sp,??? */
			stackSize = abs((short)(v & 0xffff));
			airbag_printf("%s[0x%" FMTBIT "lx]: stack size %lu\n", comment, addr, stackSize);
			break;
		case 0x67bd0000: /* daddiu   sp,sp,??? */
			stackSize = abs((short)(v & 0xffff));
			airbag_printf("%s[0x%" FMTBIT "lx]: stack size %lu\n", comment, addr, stackSize);
			break;
		case 0xafbf0000: /* sw      ra,???(sp) */
			raOffset = (v & 0xffff);
			airbag_printf("%s[0x%" FMTBIT "lx]: ra offset %lu\n", comment, addr, raOffset);
			break;
		case 0xffbf0000: /* ld     ra,???(sp) */
			raOffset = (v & 0xffff);
			airbag_printf("%s[0x%" FMTBIT "lx]: ra offset %lu\n", comment, addr, raOffset);
			break;
		case 0x3c1c0000: /* lui     gp,??? */
			goto out;
		default:
			break;
		}
	}
out:
	if (raOffset) {
		unsigned long *newRa;
		if (loadlong((unsigned long *)((unsigned long)sp + raOffset), (unsigned long *)&newRa))
			airbag_printf("%sText at RA <- SP[raOffset] 0x%" FMTBIT "lx[0x%" FMTBIT
				      "lx] is not mapped; assuming blown stack.\n",
				      comment, sp, raOffset);
		else
			ra = newRa;
	}
	if (stackSize)
		sp = (unsigned long *)((unsigned long)sp + stackSize);

backward:
	while (depth < size && ra) {
		if (buffer[depth - 1] == ra)
			repeat[depth - 1]++;
		else
			buffer[depth++] = ra;
		raOffset = stackSize = 0;
		for (addr = ra; !raOffset || !stackSize; --addr) {
			INSTLEN v;
			if (load32(addr, &v)) {
				airbag_printf("%sText at 0x%" FMTBIT "lx is not mapped; %s.\n", comment, addr, termBt);
				return depth;
			}
			switch (v & 0xffff0000) {
			case 0x27bd0000: /* addiu   sp,sp,??? */
				stackSize = abs((short)(v & 0xffff));
				airbag_printf("%s[0x%" FMTBIT "lx]: stack size %u\n", comment, addr, stackSize);
				break;
			case 0x67bd0000: /* daddiu   sp,sp,??? */
				stackSize = abs((short)(v & 0xffff));
				airbag_printf("%s[0x%" FMTBIT "lx]: stack size %lu\n", comment, addr, stackSize);
				break;
			case 0xafbf0000: /* sw      ra,???(sp) */
				raOffset = (v & 0xffff);
				airbag_printf("%s[0x%" FMTBIT "lx]: ra offset %lu\n", comment, addr, raOffset);
				break;
			case 0xffbf0000: /* ld     ra,???(sp) */
				raOffset = (v & 0xffff);
				airbag_printf("%s[0x%" FMTBIT "lx]: ra offset %lu\n", comment, addr, raOffset);
				break;
			case 0x3c1c0000: /* lui     gp,??? */
				return depth + 1;
			default:
				break;
			}
		}
		if (loadlong((INSTLEN *)((unsigned long)sp + raOffset), (unsigned long *)&ra)) {
			airbag_printf("%sText at RA <- SP[raOffset] 0x%" FMTBIT "lx[0x%" FMTBIT "lx] is not mapped; %s.\n", comment,
				      sp, raOffset, termBt);
			break;
		}
		sp = (unsigned long *)((unsigned long)sp + stackSize);
	}
	return depth;
#elif defined(__arm__)
	uint32_t pc = MCTX_PC(uc);
	uint32_t fp = MCTXREG(uc, 14);
	uint32_t lr = MCTXREG(uc, 17);
	int depth = 0;
	char fname[257];

	if (pc & 3 || load32((void *)pc, NULL)) {
		airbag_printf("%sCalled through bad function pointer; assuming PC <- LR.\n", comment);
		pc = MCTX_PC(uc) = lr;
	}
	buffer[depth] = (void *)pc;

	/* Heuristic for gcc-generated code:
	 *  - Know PC, FP for current frame.
	 *  - Scan backwards from PC to find the push of prior FP.  This is the function's prologue.
	 *  - Sometimes there's a prior push instruction to account for.
	 *  - Load registers from start of frame based on the push instruction(s).
	 *  - Leaf functions might not push LR.
	 * TODO: what if lr&3 or priorPc&3
	 * TODO: return heuristic fname faddr?
	 */
	while (++depth < size) {
		/*
		 * CondOp-PUSWLRn--Register-list---
		 * 1110100???101101????????????????
		 * Unconditional, op is "load/store multiple", "W" bit because SP is updated,
		 * not "L" bit because is store, to SP
		 */
		const uint32_t stmBits = 0xe82d0000;
		const uint32_t stmMask = 0xfe3f0000;
		int found = 0;
		int i;

		airbag_printf("%sSearching frame %u (FP=0x%" FMTBIT "lx, PC=0x%" FMTBIT "lx)\n", comment, depth - 1, fp, pc);

		for (i = 0; i < 8192 && !found; ++i) {
			INSTLEN instr, instr2;
			if (load32((void *)(pc - i * 4), &instr2)) {
				airbag_printf("%sInstruction at 0x%" FMTBIT "lx is not mapped; %s.\n", comment, pc - i * 4, termBt);
				return depth;
			}
			if ((instr2 & (stmMask | (1 << 11))) == (stmBits | (1 << 11))) {
				void *faddr = 0;
				uint32_t priorPc =
					lr; /* If LR was pushed, will find and use that.  For now assume leaf function. */
				uint32_t priorFp = 0;
				found = 1;
				i++;
				if (load32((void *)(pc - i * 4), &instr) == 0 && (instr & stmMask) == stmBits) {
					int pushes, dir, pre, regNum;
checkStm:
					pushes = 0;
					dir = (instr & (1 << 23)) ? 1 : -1; /* U bit: increment or decrement? */
					pre = (instr & (1 << 24)) ? 1 : 0; /* P bit: pre  TODO */
					airbag_printf("%sPC-%02x[0x%" FMTBIT "lx]: 0x%" FMTBIT "lx stm%s%s sp!\n", comment, i * 4,
						      pc - i * 4, instr, pre == 1 ? "f" : "e", dir == 1 ? "a" : "d");
					for (regNum = 15; regNum >= 0; --regNum) {
						if (instr & (1 << regNum)) {
							INSTLEN reg;
							if (load32((void *)(fp + pushes * 4 * dir), &reg)) {
								airbag_printf("%sStack at 0x%" FMTBIT "lx is not mapped; %s.\n",
									      comment, fp + pushes * 4 * dir, termBt);
								return depth;
							}
							airbag_printf("%sFP%s%02x[0x%" FMTBIT "lx]: 0x%" FMTBIT "lx {%s}\n",
								      comment, dir == 1 ? "+" : "-", pushes * 4,
								      fp + pushes * 4 * dir, reg,
								      mctxRegNames[gregOffset + regNum]);
							pushes++;
							if (regNum == 11)
								priorFp = reg;
							else if (regNum == 14)
								priorPc = reg;
							else if (regNum == 15) {
								/* When built with -mpoke-function-name, PC is also pushed in the
								 * function prologue so that [FP] points near the top of the function
								 * and the poked name can be found. */
								faddr = getPokedFnName(reg, fname);
							}
						}
					}
				}
				if (instr2) {
					i--;
					instr = instr2;
					instr2 = 0;
					goto checkStm;
				}
				airbag_printf("%s%s ", comment, depth == 1 ? "Crashed at" : "Called from");
				_airbag_symbol((void *)pc, fname, faddr);
				airbag_printf("\n");
				pc = priorPc;
				fp = priorFp;
			}
		}

		if (!found) {
			airbag_printf("%sFailed to find prior stack frame; %s.\n", comment, termBt);
			break;
		}
		if (buffer[depth - 1] == (void *)pc)
			repeat[depth - 1]++;
		else
			buffer[depth] = (void *)pc;
	}
	return depth;
#elif defined(USE_GCC_UNWIND)
	/* Not preferred, because doesn't handle blown stack, etc. */
	static void *handle;
	if (!handle)
		handle = dlopen("libgcc_s.so.1", RTLD_LAZY);

	if (handle) {
		static Unwind_Backtrace_T _unwind_Backtrace;
		if (!_unwind_Backtrace)
			_unwind_Backtrace = (Unwind_Backtrace_T)dlsym(handle, "_Unwind_Backtrace");
		if (!_unwind_GetIP)
			_unwind_GetIP = (Unwind_GetIP_T)dlsym(handle, "_Unwind_GetIP");
		if (!_unwind_GetCFA)
	    		_unwind_GetCFA = (dlsym(libgcc_handle, "_Unwind_GetCFA") ?: airbag_dummy_getcfa);
		if (_unwind_Backtrace && _unwind_GetIP) {
#if defined(__i386__)
			struct trace_arg arg = { .array = buffer, .size = size, .cnt = -1 };
#elif defined(__arm__)
			struct trace_arg arg = { .array = buffer, .size = size, .cnt = -1 };
#else
			struct trace_arg arg = { .array = buffer, .cfa = 0, .size = size, .cnt = -1 };
#endif
			//			struct trace_arg arg = { buffer, -1, size, uc };
			if (load8((void *)(MCTX_PC(uc)), NULL)) {
				airbag_printf("%sText at 0x%" FMTBIT "lx is not mapped; trying prior frame pointer.\n", comment,
					      MCTX_PC(uc));
#if defined(__mips__)
				MCTX_PC(uc) = MCTXREG(uc, 31); /* RA */
#elif defined(__arm__)
				MCTX_PC(uc) = MCTXREG(uc, 17); /* LR */
#elif defined(__aarch64__)
				MCTX_PC(uc) = MCTXREG(uc, 30); /* LR */
#elif defined(__powerpc__)
				MCTX_PC(uc) = MCTXREG(uc, 37); /* LINK */
#elif defined(__i386__)
				/* TODO heuristic for -fomit-frame-pointer? */
				uint8_t *fp = (uint8_t *)MCTXREG(uc, 6) + 4;
				uint32_t eip;
				if (load32((void *)fp, &eip)) {
					airbag_printf("%sText at 0x%" FMTBIT "lx is not mapped; cannot get backtrace.\n", comment,
						      fp);
					size = 0;
				} else {
					MCTX_PC(uc) = eip;
				}
#elif defined(__x86_64__)
				/* TODO heuristic for -fomit-frame-pointer? */
				MCTX_PC(uc) = MCTXREG(uc, 5);
#else
				size = 0;
#endif
			}

			if (size >= 1) {
				/* TODO: setjmp, catch SIGSEGV to longjmp back here, to more gracefully handle
				 * corrupted stack. */
				_unwind_Backtrace(airbag_backtrace_helper, &arg);
				if (arg.cnt > 1 && arg.array[arg.cnt - 1] == NULL)
					--arg.cnt;
			}
			return arg.cnt != -1 ? arg.cnt : 0;
		}
	}
	return 0;
#elif !defined(AIRBAG_NO_BACKTRACE)
	/*
	 * Not preferred, because no way to explicitly start at failing PC, doesn't handle
	 * bad PC, doesn't handle blown stack, etc.
	 */
	int res = backtrace(buffer, size);
	backtrace_release();
	return ret;
#else
	return 0;
#endif
}

static void printWhere(void *pc)
{
#if !defined(AIRBAG_NO_DLADDR)
	Dl_info info;
	if (dladdr(pc, &info)) {
		airbag_printf(" in %s\n", demangle(info.dli_sname));
		return;
	}
#endif
	airbag_printf(" at 0x%" FMTBIT "lx\n", pc);
}

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
static uint64_t getNow()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000LL + tv.tv_usec;
}
#endif

struct fault {
	int fault;
	char *message;
};

struct faults {
	int code;
	int errorsize;
	struct fault *errors;
};
#ifndef BUS_MCEERR_AR
#define BUS_MCEERR_AR 4
#endif
#ifndef BUS_MCEERR_AO
#define BUS_MCEERR_AO 5
#endif
static struct fault sigbus_fault[] = {
	{ BUS_ADRALN, "invalid address alignment" },
	{ BUS_ADRERR, "nonexistent physical address" },
	{ BUS_OBJERR, "object-specific hardware error" },
	{ BUS_MCEERR_AR, "hardware memory error consumed on a machine check: action required" },
	{ BUS_MCEERR_AO, "hardware memory error detected in process but not consumed: action optional" },
};

#ifndef __FPE_DECOVF
#define __FPE_DECOVF 9 /* decimal overflow */
#endif
#ifndef __FPE_DECDIV
#define __FPE_DECDIV 10 /* decimal division by zero */
#endif
#ifndef __FPE_DECERR
#define __FPE_DECERR 11 /* packed decimal error */
#endif
#ifndef __FPE_INVASC
#define __FPE_INVASC 12 /* invalid ASCII digit */
#endif
#ifndef __FPE_INVDEC
#define __FPE_INVDEC 13 /* invalid decimal digit */
#endif
#ifndef FPE_FLTUNK
#define FPE_FLTUNK 14 /* undiagnosed floating-point exception */
#endif
#ifndef FPE_CONDTRAP
#define FPE_CONDTRAP 15 /* trap on condition */
#endif

static struct fault sigfpe_fault[] = { { FPE_INTDIV, "integer divide by zero" },
				       { FPE_INTOVF, "integer overflow" },
				       { FPE_FLTDIV, "floating-point divide by zero" },
				       { FPE_FLTOVF, "floating-point overflow" },
				       { FPE_FLTUND, "floating-point underflow" },
				       { FPE_FLTRES, "floating-point inexact result" },
				       { FPE_FLTINV, "floating-point invalid operation" },
				       { FPE_FLTSUB, "subscript out of range" },
				       { __FPE_DECOVF, "decimal overflow" },
				       { __FPE_DECDIV, "decimal division by zero" },
				       { __FPE_DECERR, "packed decimal error" },
				       { __FPE_INVASC, "invalid ASCII digit" },
				       { __FPE_INVDEC, "invalid decimal digit" },
				       { FPE_FLTUNK, "undiagnosed floating-point exception" },
				       { FPE_CONDTRAP, "trap on condition" } };

#ifndef ILL_BADIADDR
#define ILL_BADIADDR 9 /* unimplemented instruction address */
#endif
#ifndef __ILL_BREAK
#define __ILL_BREAK 10 /* illegal break */
#endif
#ifndef __ILL_BNDMOD
#define __ILL_BNDMOD 11 /* bundle-update (modification) in progress */
#endif

static struct fault sigill_fault[] = {
	{ ILL_ILLOPC, "illegal opcode" },
	{ ILL_ILLOPN, "illegal operand" },
	{ ILL_ILLADR, "illegal addressing mode" },
	{ ILL_ILLTRP, "illegal trap" },
	{ ILL_PRVOPC, "privileged opcode" },
	{ ILL_PRVREG, "privileged register" },
	{ ILL_COPROC, "coprocessor error" },
	{ ILL_BADSTK, "stack error" },
	{ ILL_BADIADDR, "unimplemented instruction address" },
	{ __ILL_BREAK, "illegal break" },
	{ __ILL_BNDMOD, "bundle-update (modification) in progress" },
};

#ifndef SEGV_BNDERR
#define SEGV_BNDERR 3 /* failed address bound checks */
#endif
#ifndef SEGV_PKUERR
#define SEGV_PKUERR 4 /* failed protection key checks */
#endif
#ifndef SEGV_ACCADI
#define SEGV_ACCADI 5 /* ADI not enabled for mapped object */
#endif
#ifndef SEGV_ADIDERR
#define SEGV_ADIDERR 6 /* Disrupting MCD error */
#endif
#ifndef SEGV_ADIPERR
#define SEGV_ADIPERR 7 /* Precise MCD exception */
#endif
static struct fault sigsegv_fault[] = {
	{ SEGV_MAPERR, "address not mapped to object" },
	{ SEGV_ACCERR, "invalid permissions for mapped object" },
	{ SEGV_BNDERR, "failed address bound checks" },
	{ SEGV_PKUERR, "failed protection key checks" },
	{ SEGV_ACCADI, "ADI not enabled for mapped object" },
	{ SEGV_ADIDERR, "Disrupting MCD error" },
	{ SEGV_ADIPERR, "Precise MCD exception" },
};

#ifndef TRAP_BRKPT
#define TRAP_BRKPT 1 /* process breakpoint */
#endif
#ifndef TRAP_TRACE
#define TRAP_TRACE 2 /* process trace trap */
#endif
#ifndef TRAP_BRANCH
#define TRAP_BRANCH 3 /* process taken branch trap */
#endif
#ifndef TRAP_HWBKPT
#define TRAP_HWBKPT 4 /* hardware breakpoint/watchpoint */
#endif
#ifndef TRAP_UNK
#define TRAP_UNK 5 /* undiagnosed trap */
#endif

static struct fault sigtrap_fault[] = {
	{ TRAP_BRKPT, "process breakpoint" },
	{ TRAP_TRACE, "process trace trap" },
	{ TRAP_BRANCH, "process taken branch trap" },
	{ TRAP_HWBKPT, "hardware breakpoint/watchpoint" },
	{ TRAP_UNK, "undiagnosed trap" },
};

static struct faults signals[] = {
	{ SIGBUS, sizeof(sigbus_fault) / sizeof(struct fault), sigbus_fault },
	{ SIGFPE, sizeof(sigfpe_fault) / sizeof(struct fault), sigfpe_fault },
	{ SIGILL, sizeof(sigill_fault) / sizeof(struct fault), sigill_fault },
	{ SIGSEGV, sizeof(sigsegv_fault) / sizeof(struct fault), sigsegv_fault },
	{ SIGTRAP, sizeof(sigtrap_fault) / sizeof(struct fault), sigtrap_fault },
};

static void sigHandler(int sigNum, siginfo_t *si, void *ucontext)
{
	ucontext_t *uc = (ucontext_t *)ucontext;
	const uint8_t *pc = (uint8_t *)MCTX_PC(uc);
	int i, a;

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
	__sync_fetch_and_add(&pending, 1);
	if (__sync_val_compare_and_swap(&busy, 0, 1) != 0) {
		uint64_t now, start = getNow();
		while (1) {
			usleep(1000);
			if (__sync_val_compare_and_swap(&busy, 0, 1) == 0)
				break;
			now = getNow();
			if (now < start || now > start + 1000000) {
				/* Timeout; perhaps another thread is recursively crashing or stuck
				 * in airbag_user_callback.  Shut it down now. */
				_exit(EXIT_FAILURE);
			}
		}
	}
#endif

	airbag_printf("Caught SIG%s (%u)", _strsignal(sigNum), sigNum);
	if (si->si_code == SI_USER) {
		airbag_printf(" sent by user %u from process %u\n", si->si_uid, si->si_pid);
	} else if (si->si_code == SI_TKILL) {
		airbag_printf(" sent by tkill\n");
	} else if (si->si_code == SI_KERNEL) {
		airbag_printf(" sent by kernel"); /* rare; well-behaved kernel gives us a real code, handled below */
		printWhere((void *)pc);
	} else {
		printWhere((void *)pc);
		const char *faultReason = 0;
		switch (sigNum) {
		case SIGABRT:
			break;
		case SIGINT:
			break;
		case SIGQUIT:
			break;
		case SIGXCPU:
			break;
		case SIGXFSZ:
			break;
		case SIGSYS:
			break;
#ifdef SIGEMT
		case SIGEMT:
			break;
#endif
		default:
			for (i = 0; i < sizeof(signals) / sizeof(struct faults); i++) {
				struct faults s = signals[i];
				if (s.code == sigNum) {
					struct fault *f = s.errors;
					for (a = 0; a < s.errorsize; a++) {
						if (f[a].fault == si->si_code)
							faultReason = f[a].message;
					}
				}
			}
			break;
		}

		if (faultReason) {
			airbag_printf("Fault at memory location 0x%" FMTBIT "lx", si->si_addr);
			airbag_printf(" due to %s (%x).\n", faultReason, si->si_code);
		}
	}
	for (i = postindex + 1; i < 32; i++) {
		if (postinfo[i])
			airbag_printf("Postinfo: %s\n", postinfo[i]);
	}
	for (i = 0; i < postindex + 1; i++) {
		if (postinfo[i])
			airbag_printf("Postinfo: %s\n", postinfo[i]);
	}
#ifdef __linux__
	{
		char name[17];
		prctl(PR_GET_NAME, (unsigned long)name, 0, 0, 0);
		name[sizeof(name) - 1] = 0;
#ifdef SYS_gettid
		unsigned int thread = syscall(SYS_gettid);
		airbag_printf("Thread %u: %s\n", thread, name);
		char line[256];
		char pid[64];
		sprintf(pid, "/proc/%u/maps", thread);
		FILE *fp = fopen(pid, "rb");
		if (fp) {
			airbag_printf("maps:\n");
			while (!feof(fp) && fgets(line, sizeof(line) - 1, fp)) {
				airbag_printf("%s", line);
			}
			fclose(fp);
		}
#else
		airbag_printf("Thread: %s\n", name);
#endif
	}
#endif

#if 0
	/*
	 * Usually unset and unused on Linux.  Note that strerror it not guaranteed to
	 * be async-signal safe (it deals with the locale) so hit the array directly.
	 * And yet the array is deprecated.  Bugger.
	 */
	if (si->si_errno)
		airbag_printf("Errno %u: %s.\n", si->si_errno, sys_errlist[si->si_errno]);
#endif

	airbag_printf("%sContext:\n", section);
	int width = 0;
	for (i = 0; i < NMCTXREGS; ++i) {
		if (!mctxRegNames[i]) /* Can trim junk per-arch by NULL-ing name. */
			continue;
		if (i) {
			if (width > 70) {
				airbag_printf("\n");
				width = 0;
			} else
				width += airbag_printf(" ");
		}
		width += airbag_printf("%" FMTLEN "s:%" FMTBIT "lx", mctxRegNames[i], MCTXREG(uc, i));
	}
	airbag_printf("\n");

	{
#if defined(__x86_64__) || defined(__i386__)
		const int size = 128;
#else
		const int size = 64;
#endif
		void *buffer[size];
		int repeat[size];
		airbag_printf("%sBacktrace:\n", section);
		int nptrs = airbag_walkstack(buffer, repeat, size, uc);
		for (i = 0; i < nptrs; ++i) {
			airbag_symbol(buffer[i]);
			if (i < size && repeat[i])
				airbag_printf(" (called %u times)", repeat[i] + 1);
			airbag_printf("\n");
		}
		/* Reload PC; walkstack may have discovered better state. */
		pc = (uint8_t *)MCTX_PC(uc);
	}

	width = 0;
	ptrdiff_t bytes = 128;
#if defined(__x86_64__) || defined(__i386__)
	const uint8_t *startPc = pc;
	if (startPc < (uint8_t *)(bytes / 2))
		startPc = 0;
	else
		startPc = pc - bytes / 2;
	const uint8_t *endPc = startPc + bytes;
	const uint8_t *addr;
#else
	pc = (uint8_t *)(((unsigned long)pc) & ~3);
	const INSTLEN *startPc = (INSTLEN *)pc;
	if (startPc < (INSTLEN *)(bytes / 2))
		startPc = 0;
	else
		startPc = (INSTLEN *)(pc - bytes / 2);
	const INSTLEN *endPc = (INSTLEN *)((uint8_t *)startPc + bytes);
	const INSTLEN *addr;
#endif
	airbag_printf("%sCode:\n", section);
	for (addr = startPc; addr < endPc; ++addr) {
		if (width > 70) {
			airbag_printf("\n");
			width = 0;
		}
		if (width == 0) {
			airbag_printf("%" FMTBIT "lx: ", addr);
		}
		width += airbag_printf((const uint8_t *)addr == pc ? ">" : " ");
#if defined(__x86_64__) || defined(__i386__)
		uint8_t b;
		uint8_t invalid = load8(addr, &b);
		if (invalid)
			airbag_printf("??");
		else
			airbag_printf("%02x", b);
		width += 2;
#else
		INSTLEN w;
		INSTLEN invalid = load32(addr, &w);
		int bitlen = sizeof(INSTLEN) - 1;
		for (i = bitlen; i >= 0; --i) {
			unsigned long shift = i * 8;
			if ((invalid >> shift) & 0xff) {
				airbag_printf("??");
			} else {
				airbag_printf("%02x", (w >> shift) & 0xff);
			}
		}
		width += 8;
#endif
	}
	airbag_printf("\n");

	/* Do not use abort(): Would re-raise SIGABRT. */
	/* Do not use exit(): Would run atexit handlers. */
	/* For threads: pthread_exit is not async-signal-safe. */
	/* Only option is to (optionally) wait and then _exit. */

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
	__sync_fetch_and_add(&busy, -1);
	if (__sync_fetch_and_add(&pending, -1) > 1) {
		uint64_t now, start = getNow();
		do {
			usleep(1000);
			now = getNow();
			if (now < start || now > start + 1000000)
				break;
		} while (__sync_fetch_and_add(&pending, 0) > 0);
	}
#endif
	if (delegate)
		delegate(sigNum);
	else
		_exit(EXIT_FAILURE);
}

static int initCrashHandlers()
{
#if defined(USE_GCC_DEMANGLE)
	if (!s_demangleBuf) {
		s_demangleBufLen = 512;
		s_demangleBuf = (char *)malloc(s_demangleBufLen);
		if (!s_demangleBuf)
			return -1;
	}
#endif

	if (!s_altStackSpace) {
		stack_t altStack;
		s_altStackSpace = (void *)malloc(ALT_STACK_SIZE);
		if (!s_altStackSpace)
			return -1;
		altStack.ss_sp = s_altStackSpace;
		altStack.ss_flags = 0;
		altStack.ss_size = ALT_STACK_SIZE;
		if (sigaltstack(&altStack, NULL) != 0) {
			free(s_altStackSpace);
			s_altStackSpace = 0;
			return -1;
		}
	}

	sigset_t mysigset;
	sigemptyset(&mysigset);
	sigaddset(&mysigset, SIGABRT);
	sigaddset(&mysigset, SIGBUS);
	sigaddset(&mysigset, SIGILL);
	sigaddset(&mysigset, SIGSEGV);
	sigaddset(&mysigset, SIGFPE);

	struct sigaction sa;
	sa.sa_sigaction = sigHandler;
	sa.sa_mask = mysigset;
	sa.sa_flags = SA_SIGINFO | SA_ONSTACK;

	sigaction(SIGABRT, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
	sigaction(SIGILL, &sa, 0);
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGFPE, &sa, 0);

	return 0;
}

static void deinitCrashHandlers()
{
	struct sigaction sa;
	sigset_t mysigset;

	sigemptyset(&mysigset);

	sa.sa_handler = SIG_DFL;
	sa.sa_mask = mysigset;
	sa.sa_flags = 0;
	delegate = NULL;
	sigaction(SIGABRT, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
	sigaction(SIGILL, &sa, 0);
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGFPE, &sa, 0);
	sigaction(SIGQUIT, &sa, 0);
	sigaction(SIGTRAP, &sa, 0);
	sigaction(SIGXCPU, &sa, 0);
	sigaction(SIGXFSZ, &sa, 0);
	sigaction(SIGSYS, &sa, 0);
#ifdef SIGEMT
	sigaction(SIGEMT, &sa, 0);
#endif
#if defined(USE_GCC_DEMANGLE)
	if (s_demangleBuf) {
		free(s_demangleBuf);
		s_demangleBuf = 0;
	}
#endif
	if (s_altStackSpace) {
		stack_t altStack;
		altStack.ss_sp = 0;
		altStack.ss_flags = SS_DISABLE;
		altStack.ss_size = 0;
		sigaltstack(&altStack, NULL);
		free(s_altStackSpace);
		s_altStackSpace = 0;
	}
}
static pthread_mutex_t postinfo_mutex;

AIRBAG_EXPORT int airbag_init(void)
{
	pthread_mutex_init(&postinfo_mutex, NULL);
	int i;
	for (i = 0; i < 33; i++)
		postinfo[i] = NULL;
	return initCrashHandlers();
}

int airbag_init_delegate(sighandler_t handler)
{
	delegate = handler;
	return 0;
}

AIRBAG_EXPORT int airbag_name_thread(const char *name)
{
#if defined(__linux__)
	prctl(PR_SET_NAME, (unsigned long)name);
	return 0;
#elif defined(__FreeBSD__) && !defined(AIRBAG_NO_PTHREAD)
	pthread_set_name_np(pthread_self(), name);
	return 0;
#else
	(void)name;
	errno = ENOTSUP;
	return -1;
#endif
}

AIRBAG_EXPORT void airbag_deinit()
{
	deinitCrashHandlers();
}

void airbag_setpostinfo(const char *string)
{
	pthread_mutex_lock(&postinfo_mutex);
	postindex++;
	postindex = postindex % 32;
	if (postinfo[postindex]) {
		free(postinfo[postindex]);
	}
	postinfo[postindex] = strdup(string);
	pthread_mutex_unlock(&postinfo_mutex);
}
#endif

#ifdef TEST
int main(int argc, char *argv[])
{
	airbag_init();
	unsigned *p = 666;
	p[0] = 1;
}

#endif

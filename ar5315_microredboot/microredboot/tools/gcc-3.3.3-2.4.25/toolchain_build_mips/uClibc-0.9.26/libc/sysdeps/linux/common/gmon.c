/*-
 * Copyright (c) 1983, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <features.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/gmon.h>
#include <sys/gmon_out.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>

#ifdef __UCLIBC_PROFILING__

/*  Head of basic-block list or NULL. */
struct __bb *__bb_head;

struct gmonparam _gmonparam = { state: GMON_PROF_OFF };

/*
 * See profil(2) where this is described:
 */
static int	s_scale;
#define		SCALE_1_TO_1	0x10000L

#define ERR(s) write (STDERR_FILENO, s, sizeof (s) - 1)

void moncontrol __P ((int mode));
static void write_hist __P ((int fd));
static void write_call_graph __P ((int fd));
static void write_bb_counts __P ((int fd));

/*
 * Control profiling
 *	profiling is what mcount checks to see if
 *	all the data structures are ready.
 */
void moncontrol (int mode)
{
    struct gmonparam *p = &_gmonparam;

    /* Don't change the state if we ran into an error.  */
    if (p->state == GMON_PROF_ERROR)
	return;

    if (mode)
    {
	/* start */
	profil((void *) p->kcount, p->kcountsize, p->lowpc, s_scale);
	p->state = GMON_PROF_ON;
    }
    else
    {
	/* stop */
	profil(NULL, 0, 0, 0);
	p->state = GMON_PROF_OFF;
    }
}


void monstartup (u_long lowpc, u_long highpc)
{
    register int o;
    char *cp;
    struct gmonparam *p = &_gmonparam;

    /*
     * round lowpc and highpc to multiples of the density we're using
     * so the rest of the scaling (here and in gprof) stays in ints.
     */
    p->lowpc = ROUNDDOWN(lowpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->highpc = ROUNDUP(highpc, HISTFRACTION * sizeof(HISTCOUNTER));
    p->textsize = p->highpc - p->lowpc;
    p->kcountsize = p->textsize / HISTFRACTION;
    p->hashfraction = HASHFRACTION;
    p->log_hashfraction = -1;
    /* The following test must be kept in sync with the corresponding
       test in mcount.c.  */
    if ((HASHFRACTION & (HASHFRACTION - 1)) == 0) {
	/* if HASHFRACTION is a power of two, mcount can use shifting
	   instead of integer division.  Precompute shift amount. */
	p->log_hashfraction = ffs(p->hashfraction * sizeof(*p->froms)) - 1;
    }
    p->fromssize = p->textsize / HASHFRACTION;
    p->tolimit = p->textsize * ARCDENSITY / 100;
    if (p->tolimit < MINARCS)
	p->tolimit = MINARCS;
    else if (p->tolimit > MAXARCS)
	p->tolimit = MAXARCS;
    p->tossize = p->tolimit * sizeof(struct tostruct);

    cp = calloc (p->kcountsize + p->fromssize + p->tossize, 1);
    if (! cp)
    {
	ERR("monstartup: out of memory\n");
	p->tos = NULL;
	p->state = GMON_PROF_ERROR;
	return;
    }
    p->tos = (struct tostruct *)cp;
    cp += p->tossize;
    p->kcount = (HISTCOUNTER *)cp;
    cp += p->kcountsize;
    p->froms = (ARCINDEX *)cp;

    p->tos[0].link = 0;

    o = p->highpc - p->lowpc;
    if (p->kcountsize < (u_long) o)
    {
#ifndef hp300
	s_scale = ((float)p->kcountsize / o ) * SCALE_1_TO_1;
#else
	/* avoid floating point operations */
	int quot = o / p->kcountsize;

	if (quot >= 0x10000)
	    s_scale = 1;
	else if (quot >= 0x100)
	    s_scale = 0x10000 / quot;
	else if (o >= 0x800000)
	    s_scale = 0x1000000 / (o / (p->kcountsize >> 8));
	else
	    s_scale = 0x1000000 / ((o << 8) / p->kcountsize);
#endif
    } else
	s_scale = SCALE_1_TO_1;

    moncontrol(1);
}


/* Return frequency of ticks reported by profil. */
static int profile_frequency (void)
{
    /*
     * Discover the tick frequency of the machine if something goes wrong,
     * we return 0, an impossible hertz.
     */
    struct itimerval tim;

    tim.it_interval.tv_sec = 0;
    tim.it_interval.tv_usec = 1;
    tim.it_value.tv_sec = 0;
    tim.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &tim, 0);
    setitimer(ITIMER_REAL, 0, &tim);
    if (tim.it_interval.tv_usec < 2)
	return 0;
    return (1000000 / tim.it_interval.tv_usec);
}


static void write_hist (int fd)
{
    u_char tag = GMON_TAG_TIME_HIST;
    struct gmon_hist_hdr thdr __attribute__ ((aligned (__alignof__ (char *))));

    if (_gmonparam.kcountsize > 0)
    {
	struct iovec iov[3] =
	{
	    { &tag, sizeof (tag) },
	    { &thdr, sizeof (struct gmon_hist_hdr) },
	    { _gmonparam.kcount, _gmonparam.kcountsize }
	};

	*(char **) thdr.low_pc = (char *) _gmonparam.lowpc;
	*(char **) thdr.high_pc = (char *) _gmonparam.highpc;
	*(int32_t *) thdr.hist_size = (_gmonparam.kcountsize
		/ sizeof (HISTCOUNTER));
	*(int32_t *) thdr.prof_rate = profile_frequency ();
	strncpy (thdr.dimen, "seconds", sizeof (thdr.dimen));
	thdr.dimen_abbrev = 's';

	writev (fd, iov, 3);
    }
}


static void write_call_graph (int fd)
{
#define NARCS_PER_WRITEV	32
    u_char tag = GMON_TAG_CG_ARC;
    struct gmon_cg_arc_record raw_arc[NARCS_PER_WRITEV]
	__attribute__ ((aligned (__alignof__ (char*))));
    ARCINDEX from_index, to_index, from_len;
    u_long frompc;
    struct iovec iov[2 * NARCS_PER_WRITEV];
    int nfilled;

    for (nfilled = 0; nfilled < NARCS_PER_WRITEV; ++nfilled)
    {
	iov[2 * nfilled].iov_base = &tag;
	iov[2 * nfilled].iov_len = sizeof (tag);

	iov[2 * nfilled + 1].iov_base = &raw_arc[nfilled];
	iov[2 * nfilled + 1].iov_len = sizeof (struct gmon_cg_arc_record);
    }

    nfilled = 0;
    from_len = _gmonparam.fromssize / sizeof (*_gmonparam.froms);
    for (from_index = 0; from_index < from_len; ++from_index)
    {
	if (_gmonparam.froms[from_index] == 0)
	    continue;

	frompc = _gmonparam.lowpc;
	frompc += (from_index * _gmonparam.hashfraction
		* sizeof (*_gmonparam.froms));
	for (to_index = _gmonparam.froms[from_index];
		to_index != 0;
		to_index = _gmonparam.tos[to_index].link)
	{
	    struct arc
	    {
		char *frompc;
		char *selfpc;
		int32_t count;
	    }
	    arc;

	    arc.frompc = (char *) frompc;
	    arc.selfpc = (char *) _gmonparam.tos[to_index].selfpc;
	    arc.count  = _gmonparam.tos[to_index].count;
	    memcpy (raw_arc + nfilled, &arc, sizeof (raw_arc [0]));

	    if (++nfilled == NARCS_PER_WRITEV)
	    {
		writev (fd, iov, 2 * nfilled);
		nfilled = 0;
	    }
	}
    }
    if (nfilled > 0)
	writev (fd, iov, 2 * nfilled);
}


static void write_bb_counts (int fd)
{
    struct __bb *grp;
    u_char tag = GMON_TAG_BB_COUNT;
    size_t ncounts;
    size_t i;

    struct iovec bbhead[2] =
    {
	{ &tag, sizeof (tag) },
	{ &ncounts, sizeof (ncounts) }
    };
    struct iovec bbbody[8];
    size_t nfilled;

    for (i = 0; i < (sizeof (bbbody) / sizeof (bbbody[0])); i += 2)
    {
	bbbody[i].iov_len = sizeof (grp->addresses[0]);
	bbbody[i + 1].iov_len = sizeof (grp->counts[0]);
    }

    /* Write each group of basic-block info (all basic-blocks in a
       compilation unit form a single group). */

    for (grp = __bb_head; grp; grp = grp->next)
    {
	ncounts = grp->ncounts;
	writev (fd, bbhead, 2);
	for (nfilled = i = 0; i < ncounts; ++i)
	{
	    if (nfilled > (sizeof (bbbody) / sizeof (bbbody[0])) - 2)
	    {
		writev (fd, bbbody, nfilled);
		nfilled = 0;
	    }

	    bbbody[nfilled++].iov_base = (char *) &grp->addresses[i];
	    bbbody[nfilled++].iov_base = &grp->counts[i];
	}
	if (nfilled > 0)
	    writev (fd, bbbody, nfilled);
    }
}


static void write_gmon (void)
{
    struct gmon_hdr ghdr __attribute__ ((aligned (__alignof__ (int))));
    int fd = -1;
    char *env;

#ifndef O_NOFOLLOW
# define O_NOFOLLOW	0
#endif

    env = getenv ("GMON_OUT_PREFIX");
    if (env != NULL 
#if 0
	    && !__libc_enable_secure
#endif
	    )
    {
	size_t len = strlen (env);
	char buf[len + 20];
	sprintf (buf, "%s.%u", env, getpid ());
	fd = open (buf, O_CREAT|O_TRUNC|O_WRONLY|O_NOFOLLOW, 0666);
    }

    if (fd == -1)
    {
	fd = open ("gmon.out", O_CREAT|O_TRUNC|O_WRONLY|O_NOFOLLOW, 0666);
	if (fd < 0)
	{
	    char buf[300];
	    int errnum = errno;
	    fprintf (stderr, "_mcleanup: gmon.out: %s\n",
		    strerror_r (errnum, buf, sizeof buf));
	    return;
	}
    }

    /* write gmon.out header: */
    memset (&ghdr, '\0', sizeof (struct gmon_hdr));
    memcpy (&ghdr.cookie[0], GMON_MAGIC, sizeof (ghdr.cookie));
    *(int32_t *) ghdr.version = GMON_VERSION;
    write (fd, &ghdr, sizeof (struct gmon_hdr));

    /* write PC histogram: */
    write_hist (fd);

    /* write call-graph: */
    write_call_graph (fd);

    /* write basic-block execution counts: */
    write_bb_counts (fd);

    close (fd);
}


void write_profiling (void)
{
    int save = _gmonparam.state;
    _gmonparam.state = GMON_PROF_OFF;
    if (save == GMON_PROF_ON)
	write_gmon ();
    _gmonparam.state = save;
}


void _mcleanup (void)
{
    moncontrol (0);

    if (_gmonparam.state != GMON_PROF_ERROR)
	write_gmon ();

    /* free the memory. */
    if (_gmonparam.tos != NULL)
	free (_gmonparam.tos);
}

#ifndef SIGPROF

/* Enable statistical profiling, writing samples of the PC into at most
   SIZE bytes of SAMPLE_BUFFER; every processor clock tick while profiling
   is enabled, the system examines the user PC and increments
   SAMPLE_BUFFER[((PC - OFFSET) / 2) * SCALE / 65536].  If SCALE is zero,
   disable profiling.  Returns zero on success, -1 on error.  */

int profil (u_short *sample_buffer, size_t size, size_t offset, u_int scale)
{
    if (scale == 0)
	/* Disable profiling.  */
	return 0;

    __set_errno (ENOSYS);
    return -1;
}

#else

static u_short *samples;
static size_t nsamples;
static size_t pc_offset;
static u_int pc_scale;

static inline void profil_count (void *pc)
{
    size_t i = (pc - pc_offset - (void *) 0) / 2;

    if (sizeof (unsigned long long int) > sizeof (size_t))
	i = (unsigned long long int) i * pc_scale / 65536;
    else
	i = i / 65536 * pc_scale + i % 65536 * pc_scale / 65536;

    if (i < nsamples)
	++samples[i];
}

/* Get the machine-dependent definition of `profil_counter', the signal
   handler for SIGPROF.  It calls `profil_count' (above) with the PC of the
   interrupted code.  */
#include <bits/profil-counter.h>

/* Enable statistical profiling, writing samples of the PC into at most
   SIZE bytes of SAMPLE_BUFFER; every processor clock tick while profiling
   is enabled, the system examines the user PC and increments
   SAMPLE_BUFFER[((PC - OFFSET) / 2) * SCALE / 65536].  If SCALE is zero,
   disable profiling.  Returns zero on success, -1 on error.  */

int profil (u_short *sample_buffer, size_t size, size_t offset, u_int scale)
{
    static struct sigaction oact;
    static struct itimerval otimer;
    struct sigaction act;
    struct itimerval timer;

    if (sample_buffer == NULL)
    {
	/* Disable profiling.  */
	if (samples == NULL)
	    /* Wasn't turned on.  */
	    return 0;

	if (setitimer (ITIMER_PROF, &otimer, NULL) < 0)
	    return -1;
	samples = NULL;
	return sigaction (SIGPROF, &oact, NULL);
    }

    if (samples)
    {
	/* Was already turned on.  Restore old timer and signal handler
	   first.  */
	if (setitimer (ITIMER_PROF, &otimer, NULL) < 0
		|| sigaction (SIGPROF, &oact, NULL) < 0)
	    return -1;
    }

    samples = sample_buffer;
    nsamples = size / sizeof *samples;
    pc_offset = offset;
    pc_scale = scale;

    act.sa_handler = (__sighandler_t) &profil_counter;
    act.sa_flags = SA_RESTART;
    __sigfillset (&act.sa_mask);
    if (sigaction (SIGPROF, &act, &oact) < 0)
	return -1;

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1;
    timer.it_interval = timer.it_value;
    return setitimer (ITIMER_PROF, &timer, &otimer);
}

#endif


/* This file provides the machine-dependent definitions of the _MCOUNT_DECL
   and MCOUNT macros.  */
#include <bits/machine-gmon.h>
#include <bits/atomicity.h>

/*
 * mcount is called on entry to each function compiled with the profiling
 * switch set.  _mcount(), which is declared in a machine-dependent way
 * with _MCOUNT_DECL, does the actual work and is either inlined into a
 * C routine or called by an assembly stub.  In any case, this magic is
 * taken care of by the MCOUNT definition in <machine/profile.h>.
 *
 * _mcount updates data structures that represent traversals of the
 * program's call graph edges.  frompc and selfpc are the return
 * address and function address that represents the given call graph edge.
 *
 * Note: the original BSD code used the same variable (frompcindex) for
 * both frompcindex and frompc.  Any reasonable, modern compiler will
 * perform this optimization.
 */
_MCOUNT_DECL(frompc, selfpc)	/* _mcount; may be static, inline, etc */
{
	register ARCINDEX *frompcindex;
	register struct tostruct *top, *prevtop;
	register struct gmonparam *p;
	register ARCINDEX toindex;
	int i;

	p = &_gmonparam;
	/*
	 * check that we are profiling
	 * and that we aren't recursively invoked.
	 */
	if (! compare_and_swap (&p->state, GMON_PROF_ON, GMON_PROF_BUSY))
	  return;

	/*
	 * check that frompcindex is a reasonable pc value.
	 * for example:	signal catchers get called from the stack,
	 *		not from text space.  too bad.
	 */
	frompc -= p->lowpc;
	if (frompc > p->textsize)
		goto done;

	/* The following test used to be
		if (p->log_hashfraction >= 0)
	   But we can simplify this if we assume the profiling data
	   is always initialized by the functions in gmon.c.  But
	   then it is possible to avoid a runtime check and use the
	   smae `if' as in gmon.c.  So keep these tests in sync.  */
	if ((HASHFRACTION & (HASHFRACTION - 1)) == 0) {
	  /* avoid integer divide if possible: */
	    i = frompc >> p->log_hashfraction;
	} else {
	    i = frompc / (p->hashfraction * sizeof(*p->froms));
	}
	frompcindex = &p->froms[i];
	toindex = *frompcindex;
	if (toindex == 0) {
		/*
		 *	first time traversing this arc
		 */
		toindex = ++p->tos[0].link;
		if (toindex >= p->tolimit)
			/* halt further profiling */
			goto overflow;

		*frompcindex = toindex;
		top = &p->tos[toindex];
		top->selfpc = selfpc;
		top->count = 1;
		top->link = 0;
		goto done;
	}
	top = &p->tos[toindex];
	if (top->selfpc == selfpc) {
		/*
		 * arc at front of chain; usual case.
		 */
		top->count++;
		goto done;
	}
	/*
	 * have to go looking down chain for it.
	 * top points to what we are looking at,
	 * prevtop points to previous top.
	 * we know it is not at the head of the chain.
	 */
	for (; /* goto done */; ) {
		if (top->link == 0) {
			/*
			 * top is end of the chain and none of the chain
			 * had top->selfpc == selfpc.
			 * so we allocate a new tostruct
			 * and link it to the head of the chain.
			 */
			toindex = ++p->tos[0].link;
			if (toindex >= p->tolimit)
				goto overflow;

			top = &p->tos[toindex];
			top->selfpc = selfpc;
			top->count = 1;
			top->link = *frompcindex;
			*frompcindex = toindex;
			goto done;
		}
		/*
		 * otherwise, check the next arc on the chain.
		 */
		prevtop = top;
		top = &p->tos[top->link];
		if (top->selfpc == selfpc) {
			/*
			 * there it is.
			 * increment its count
			 * move it to the head of the chain.
			 */
			top->count++;
			toindex = prevtop->link;
			prevtop->link = top->link;
			top->link = *frompcindex;
			*frompcindex = toindex;
			goto done;
		}

	}
done:
	p->state = GMON_PROF_ON;
	return;
overflow:
	p->state = GMON_PROF_ERROR;
	return;
}

/*
 * Actual definition of mcount function.  Defined in <machine/profile.h>,
 * which is included by <sys/gmon.h>.
 */
MCOUNT

#endif


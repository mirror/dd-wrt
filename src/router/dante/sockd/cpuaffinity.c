/*
 * Copyright (c) 2012
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

static const char rcsid[] =
"$Id: cpuaffinity.c,v 1.15 2012/12/05 10:46:12 michaels Exp $";

/*
 * Access to the CPU_foo() macros on Linux requires _GNU_SOURCE to be
 * defined. This define is not compatible with _BSD_SOURCE which is
 * required for other parts of the code.
 * To avoid any unpredictable changes in the build environment, the
 * _GNU_SOURCE value is only defined in this file and the contents
 * of this file is kept as simple as possible.
 */

#define _GNU_SOURCE

#include "common.h"

#include <sched.h>

#if HAVE_SCHED_SETAFFINITY

#if !HAVE_CPU_EQUAL
int
CPU_EQUAL(const cpu_set_t *set1, const cpu_set_t *set2);

int
CPU_EQUAL(set1, set2)
   const cpu_set_t *set1;
   const cpu_set_t *set2;
{
   size_t i;

   for (i = 0; i < cpu_get_setsize(); i++)
      if (CPU_ISSET(i, set1) != CPU_ISSET(i, set2))
         return 0;

   return 1;
}
#endif /* !HAVE_CPU_EQUAL */

size_t
cpu_get_setsize(void)
{
   return CPU_SETSIZE;
}

void
cpu_set(cpu, set)
   const int cpu;
   cpu_set_t *set;
{
   CPU_SET(cpu, set);
}

void
cpu_zero(set)
   cpu_set_t *set;
{
   CPU_ZERO(set);
}

int
cpu_equal(set1, set2)
   const cpu_set_t *set1;
   const cpu_set_t *set2;
{
   return CPU_EQUAL(set1, set2);
}

int
cpu_isset(cpu, set)
   const int cpu;
   const cpu_set_t *set;
{
   return CPU_ISSET(cpu, set);
}

int
cpu_getaffinity(pid, cpusetsize, mask)
   pid_t pid;
   size_t cpusetsize;
   cpu_set_t *mask;
{
   return sched_getaffinity(pid, cpusetsize, mask);
}

int
cpu_setaffinity(pid, cpusetsize, mask)
   pid_t pid;
   size_t cpusetsize;
   const cpu_set_t *mask;
{
   return sched_setaffinity(pid, cpusetsize, mask);
}

int
sockd_cpuset_isok(set)
   const cpu_set_t *set;
{
   const char *function = "sockd_cpumask_isok()";
   const long cpus      = sysconf(_SC_NPROCESSORS_ONLN);
   const size_t setsize = cpu_get_setsize();
   int i;

   if (cpus == -1)
      serr("%s: sysconf(_SC_NPROCESSORS_ONLN) failed", function);

   for (i = 0; i < (int)setsize; ++i)
      if (cpu_isset(i, set) && i + 1 > cpus)
         return 0;

   return 1;
}

#endif /* HAVE_SCHED_SETAFFINITY */

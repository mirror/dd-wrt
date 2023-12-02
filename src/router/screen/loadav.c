/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * https://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include <sys/types.h>
#include <fcntl.h>
#ifdef ultrix
# include <sys/fixpoint.h>
#endif

/* mach stuff included here to prevent index macro conflict */
#ifdef NeXT
# include <sys/version.h>
# if KERNEL_MAJOR_VERSION > 2
#  include <mach/mach.h>
# else
#  include <mach.h>
# endif
#endif

#ifdef _AIX
#include    <sys/kinfo.h>
extern int getkerninfo(int op, char *buf, int *buf_size, int32long64_t arg);
#endif

#include "config.h"
#include "screen.h"

#include "extern.h"

#ifdef LOADAV

static int GetLoadav __P((void));

static LOADAV_TYPE loadav[LOADAV_NUM];
static int loadok;



/***************************************************************/

#if defined(linux) && !defined(LOADAV_DONE)
#define LOADAV_DONE
/*
 * This is the easy way. It relies in /proc being mounted.
 * For the big and ugly way refer to previous screen version.
 */
void
InitLoadav()
{
  loadok = 1;
}

static int
GetLoadav()
{
  FILE *fp;
  char buf[128], *s;
  int i;
  double d, e;

  if ((fp = secfopen("/proc/loadavg", "r")) == NULL)
    return 0;
  *buf = 0;
  fgets(buf, sizeof(buf), fp);
  fclose(fp);
  /* can't use fscanf because the decimal point symbol depends on
   * the locale but the kernel uses always '.'.
   */
  s = buf;
  for (i = 0; i < (LOADAV_NUM > 3 ? 3 : LOADAV_NUM); i++)
    {
      d = e = 0;
      while(*s == ' ')
	s++;
      if (*s == 0)
	break;
      for(;;)
	{
	  if (*s == '.') 
	    e = 1;
	  else if (*s >= '0' && *s <= '9') 
	    {
	      d = d * 10 + (*s - '0'); 
	      if (e)
		e *= 10;
	    }
	  else    
	    break;
	  s++;    
	}
      loadav[i] = e ? d / e : d;
    }
  return i;
}
#endif /* linux */

/***************************************************************/

#if defined(LOADAV_GETLOADAVG) && !defined(LOADAV_DONE)
#define LOADAV_DONE
void
InitLoadav()
{
  loadok = 1;
}

static int
GetLoadav()
{
  return getloadavg(loadav, LOADAV_NUM);
}
#endif

/***************************************************************/

#if defined(apollo) && !defined(LOADAV_DONE)
#define LOADAV_DONE
void
InitLoadav()
{
  loadok = 1;
}

static int
GetLoadav()
{
  proc1_$get_loadav(loadav);
  return LOADAV_NUM;
}
#endif

/***************************************************************/

#if defined(NeXT) && !defined(LOADAV_DONE)
#define LOADAV_DONE

static processor_set_t default_set;

void
InitLoadav()
{
  kern_return_t error;

  error = processor_set_default(host_self(), &default_set);
  if (error != KERN_SUCCESS)
    mach_error("Error calling processor_set_default", error);
  else
    loadok = 1;
}

static int
GetLoadav()
{
  unsigned int info_count;
  struct processor_set_basic_info info;
  host_t host;

  info_count = PROCESSOR_SET_BASIC_INFO_COUNT;
  if (processor_set_info(default_set, PROCESSOR_SET_BASIC_INFO, &host, (processor_set_info_t)&info, &info_count) != KERN_SUCCESS)
    return 0;
  loadav[0] = (float)info.load_average / LOAD_SCALE;
  return 1;
}
#endif

/***************************************************************/

#if defined(sun) && defined(SVR4) && !defined(LOADAV_DONE)
#define LOADAV_DONE

#include <kstat.h>

static kstat_ctl_t *kc;

void
InitLoadav()
{
  loadok = (kc = kstat_open()) != 0;
}

static int
GetLoadav()
{
  kstat_t *ks;
  kstat_named_t *avgs[3];
  int i;

  kstat_chain_update(kc);
  if ((ks = kstat_lookup(kc, "unix", -1, "system_misc")) == 0 || kstat_read(kc, ks, (void *)0) == -1)
    return (loadok = 0);
  avgs[0] = kstat_data_lookup(ks, "avenrun_1min");
  avgs[1] = kstat_data_lookup(ks, "avenrun_5min");
  avgs[2] = kstat_data_lookup(ks, "avenrun_15min");
  for (i = 0; i < 3; i++)
    {
      if (avgs[i] == 0 || avgs[i]->data_type != KSTAT_DATA_ULONG)
        return (loadok = 0);
      loadav[i] = avgs[i]->value.ul;
    }
  return 3;
}

#endif

/***************************************************************/

#if defined(__osf__) && defined(__alpha) && !defined(LOADAV_DONE)
#define LOADAV_DONE

struct rtentry; struct mbuf;	/* shut up gcc on OSF/1 4.0 */
#include <sys/table.h>

void
InitLoadav()
{
  loadok = 1;
}

static int
GetLoadav()
{
  struct tbl_loadavg tbl;
  int i;

  if (table(TBL_LOADAVG, 0, &tbl, 1, sizeof(struct tbl_loadavg)) != 1)
    return 0;

  if (tbl.tl_lscale)
    {
      /* in long */
      for (i = 0; i < LOADAV_NUM; i++)
        loadav[i] = (double) tbl.tl_avenrun.l[i] / tbl.tl_lscale;
    }
  else
    {
      /* in double */
      for (i = 0; i < LOADAV_NUM; i++)
        loadav[i] = tbl.tl_avenrun.d[i];
    }
  return LOADAV_NUM;
}
#endif


/***************************************************************/

#if defined(_AIX) && !defined(LOADAV_DONE)
#define LOADAV_DONE
/*
 * AIX uses KINFO_GET_AVENRUN syscall
 */
void
InitLoadav()
{
  loadok = 1;
}

static int
GetLoadav()
{
  long long avenrun[3];
  int avsize = 3 * sizeof(long long);
  int i;

  if (getkerninfo(KINFO_GET_AVENRUN, (char *)&avenrun, &avsize, 0) < 0)
    {
      return 0;
    }

  for (i = 0; i < (LOADAV_NUM > 3 ? 3 : LOADAV_NUM); i++)
    {
      loadav[i] = avenrun[i];
    }

  return i;
}
#endif /* _AIX */

/***************************************************************/

#if !defined(LOADAV_DONE)
/*
 * The old fashion way: open kernel and read avenrun
 *
 * Header File includes
 */

# ifdef NLIST_STRUCT
#  include <nlist.h>
# else
#  include <a.out.h>
# endif
# ifndef NLIST_DECLARED
extern int nlist __P((char *, struct nlist *));
# endif

#ifdef LOADAV_USE_NLIST64
# define nlist nlist64
#endif

static struct nlist nl[2];
static int kmemf;

#ifdef _IBMR2
# define nlist(u,l) knlist(l,1,sizeof(*l))
#endif

void
InitLoadav()
{
  debug("Init Kmem...\n");
  if ((kmemf = open("/dev/kmem", O_RDONLY)) == -1)
    return;
# if !defined(_AUX_SOURCE) && !defined(AUX)
#  ifdef NLIST_NAME_UNION
  nl[0].n_un.n_name = LOADAV_AVENRUN;
#  else
  nl[0].n_name = LOADAV_AVENRUN;
#  endif
# else
  strncpy(nl[0].n_name, LOADAV_AVENRUN, sizeof(nl[0].n_name));
# endif
  debug2("Searching in %s for %s\n", LOADAV_UNIX, nl[0].n_name);
  nlist(LOADAV_UNIX, nl);
  if (nl[0].n_value == 0)
    {
      close(kmemf);
      return;
    }
# if 0		/* no longer needed (Al.Smith@aeschi.ch.eu.org) */
#  ifdef sgi
  nl[0].n_value &= (unsigned long)-1 >> 1;	/* clear upper bit */
#  endif /* sgi */
# endif
  debug1("AvenrunSym found (0x%lx)!!\n", nl[0].n_value);
  loadok = 1;
}

static int
GetLoadav()
{
  if (lseek(kmemf, (off_t) nl[0].n_value, 0) == (off_t)-1)
    return 0;
  if (read(kmemf, (char *) loadav, sizeof(loadav)) != sizeof(loadav))
    return 0;
  return LOADAV_NUM;
}
#endif

/***************************************************************/

#ifndef FIX_TO_DBL
#define FIX_TO_DBL(l) ((double)(l) /  LOADAV_SCALE)
#endif

void
AddLoadav(p)
char *p;
{
  int i, j;
  if (loadok == 0)
    return;
  j = GetLoadav();
  for (i = 0; i < j; i++)
    {
      sprintf(p, " %2.2f" + !i, FIX_TO_DBL(loadav[i]));
      p += strlen(p);
    }
}

#endif /* LOADAV */

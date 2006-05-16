/*- This is a -*- C -*- compatible code file
 *
 * Code for SUNOS5_INSTRUMENTATION
 *
 * This file contains includes of standard and local system header files,
 * includes of other application header files, global variable definitions,
 * static variable definitions, static function prototypes, and function
 * definitions.
 *
 * This file contains function to obtain statistics from SunOS 5.x kernel
 *
 */

#include <net-snmp/net-snmp-config.h>
#ifdef solaris2
/*-
 * Includes of standard ANSI C header files 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*-
 * Includes of system header files (wrapped in duplicate include prevention)
 */

#include <fcntl.h>
#include <stropts.h>
#include <sys/types.h>
#include <kvm.h>
#include <sys/fcntl.h>
#include <kstat.h>
#include <errno.h>
#include <time.h>

#include <sys/sockio.h>
#include <sys/socket.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <inet/common.h>
#include <inet/mib2.h>
#include <inet/ip.h>
#include <net/if.h>
#include <netinet/in.h>

/*-
 * Includes of local application header files 
 */

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "kernel_sunos5.h"

kstat_ctl_t    *kstat_fd = 0;

/*-
 * Global variable definitions (with initialization)
 */

/*-
 * Static variable definitions (with initialization)
 */

static
mibcache        Mibcache[MIBCACHE_SIZE] = {
    {MIB_SYSTEM, 0, (void *) -1, 0, 0, 0, 0},
    {MIB_INTERFACES, 10 * sizeof(mib2_ifEntry_t), (void *) -1, 0, 10, 0,
     0},
    {MIB_AT, 0, (void *) -1, 0, 0, 0, 0},
    {MIB_IP, sizeof(mib2_ip_t), (void *) -1, 0, 20, 0, 0},
    {MIB_IP_ADDR, 20 * sizeof(mib2_ipAddrEntry_t), (void *) -1, 0, 20, 0,
     0},
    {MIB_IP_ROUTE, 200 * sizeof(mib2_ipRouteEntry_t), (void *) -1, 0, 10,
     0, 0},
    {MIB_IP_NET, 100 * sizeof(mib2_ipNetToMediaEntry_t), (void *) -1, 0,
     100, 0, 0},
    {MIB_ICMP, sizeof(mib2_icmp_t), (void *) -1, 0, 20, 0, 0},
    {MIB_TCP, sizeof(mib2_tcp_t), (void *) -1, 0, 20, 0, 0},
    {MIB_TCP_CONN, 1000 * sizeof(mib2_tcpConnEntry_t), (void *) -1, 0, 15,
     0, 0},
    {MIB_UDP, sizeof(mib2_udp_t), (void *) -1, 0, 15, 0, 0},
    {MIB_UDP_LISTEN, 1000 * sizeof(mib2_udpEntry_t), (void *) -1, 0, 15, 0,
     0},
    {MIB_EGP, 0, (void *) -1, 0, 0, 0, 0},
    {MIB_CMOT, 0, (void *) -1, 0, 0, 0, 0},
    {MIB_TRANSMISSION, 0, (void *) -1, 0, 0, 0, 0},
    {MIB_SNMP, 0, (void *) -1, 0, 0, 0, 0},
    {0},
};

static
mibmap          Mibmap[MIBCACHE_SIZE] = {
    {MIB2_SYSTEM, 0,},
    {MIB2_INTERFACES, 0,},
    {MIB2_AT, 0,},
    {MIB2_IP, 0,},
    {MIB2_IP, MIB2_IP_20,},
    {MIB2_IP, MIB2_IP_21,},
    {MIB2_IP, MIB2_IP_22,},
    {MIB2_ICMP, 0,},
    {MIB2_TCP, 0,},
    {MIB2_TCP, MIB2_TCP_13,},
    {MIB2_UDP, 0,},
    {MIB2_UDP, MIB2_UDP_5},
    {MIB2_EGP, 0,},
    {MIB2_CMOT, 0,},
    {MIB2_TRANSMISSION, 0,},
    {MIB2_SNMP, 0,},
    {0},
};

static int      sd = -1;        /* /dev/ip stream descriptor. */

/*-
 * Static function prototypes (use void as argument type if there are none)
 */

static found_e
getentry(req_e req_type, void *bufaddr, size_t len, size_t entrysize,
         void *resp, int (*comp)(void *, void *), void *arg);

static int
getmib(int groupname, int subgroupname, void *statbuf, size_t size,
       size_t entrysize, req_e req_type, void *resp, size_t *length,
       int (*comp)(void *, void *), void *arg);

static int
getif(mib2_ifEntry_t *ifbuf, size_t size, req_e req_type, mib2_ifEntry_t *resp,
      size_t *length, int (*comp)(void *, void *), void *arg);

static int
Name_cmp(void *, void *);

static void
init_mibcache_element(mibcache * cp);

#define	STREAM_DEV	"/dev/ip"
#define	BUFSIZE		40960   /* Buffer for  messages (should be modulo(pagesize) */

/*-
 * Function definitions
 */

#ifdef _STDC_COMPAT
#ifdef __cplusplus
extern          "C" {
#endif
#endif

/*
 * I profiled snmpd using Quantify on a Solaris 7 box, and it turned out that
 * the calls to time() in getMibstat() were taking 18% of the total execution
 * time of snmpd when doing simple walks over the whole tree.  I guess it must
 * be difficult for Sun hardware to tell the time or something ;-).  Anyway,
 * this seemed like it was negating the point of having the cache, so I have
 * changed the code so that it runs a periodic alarm to age the cache entries
 * instead.  The meaning of the cache_ttl and cache_time members has changed to
 * support this.  cache_ttl is now the value that cache_time gets reset to when
 * we fetch a value from the kernel; cache_time then ticks down to zero in
 * steps of period (see below).  When it reaches zero, the cache entry is no
 * longer valid and we fetch a new one.  The effect of this is the same as the
 * previous code, but more efficient (because it's not calling time() for every
 * variable fetched) when you are walking the tables.  jbpn, 20020226.
 */

static void
kernel_sunos5_cache_age(unsigned int regnumber, void *data)
{
    int i = 0, period = (int)data;

    for (i = 0; i < MIBCACHE_SIZE; i++) {
	DEBUGMSGTL(("kernel_sunos5", "cache[%d] time %ld ttl %d\n", i,
		    Mibcache[i].cache_time, Mibcache[i].cache_ttl));
	if (Mibcache[i].cache_time < period) {
	    Mibcache[i].cache_time = 0;
	} else {
	    Mibcache[i].cache_time -= period;
	}
    }
}

void
init_kernel_sunos5(void)
{
    static int creg   = 0;
    const  int period = 5;

    if (creg == 0) {
	creg = snmp_alarm_register(period, SA_REPEAT, kernel_sunos5_cache_age,
				   (void *)period);
	DEBUGMSGTL(("kernel_sunos5", "registered alarm %d with period %ds\n", 
		    creg, period));
    }
}

/*
 * Get various kernel statistics using undocumented Solaris kstat interface.
 * We need it mainly for getting network interface statistics, although it is
 * generic enough to be used for any purpose.  It knows about kstat_headers
 * module names and by the name of the statistics it tries to figure out the
 * rest of necessary information.  Returns 0 in case of success and < 0 if
 * there were any errors.
 
 * 
 * NOTE: To use this function correctly you have to know the actual type of the
 * value to be returned, so you may build the test program, figure out the type
 * and use it. Exposing kstat data types to upper layers doesn't seem to be
 * reasonable. In any case I'd expect more reasonable kstat interface. :-(
 */


int
getKstatInt(const char *classname, const char *statname, 
	    const char *varname, int *value)
{
    kstat_ctl_t    *ksc;
    kstat_t        *ks;
    kid_t           kid;
    kstat_named_t  *named;
    int             ret = 0;        /* fail unless ... */

    if (kstat_fd == 0) {
	kstat_fd = kstat_open();
	if (kstat_fd == 0) {
	    snmp_log(LOG_ERR, "kstat_open(): failed\n");
	}
    }
    if ((ksc = kstat_fd) == NULL) {
	goto Return;
    }
    ks = kstat_lookup(ksc, classname, -1, statname);
    if (ks == NULL) {
	goto Return;
    }
    kid = kstat_read(ksc, ks, NULL);
    if (kid == -1) {
	goto Return;
    }
    named = kstat_data_lookup(ks, varname);
    if (named == NULL) {
	goto Return;
    }

    ret = 1;                /* maybe successful */
    switch (named->data_type) {
#ifdef KSTAT_DATA_INT32         /* Solaris 2.6 and up */
    case KSTAT_DATA_INT32:
	*value = named->value.i32;
	break;
    case KSTAT_DATA_UINT32:
	*value = named->value.ui32;
	break;
    case KSTAT_DATA_INT64:
	*value = named->value.i64;
	break;
    case KSTAT_DATA_UINT64:
	*value = named->value.ui64;
	break;
#else
    case KSTAT_DATA_LONG:
	*value = named->value.l;
	break;
    case KSTAT_DATA_ULONG:
	*value = named->value.ul;
	break;
    case KSTAT_DATA_LONGLONG:
	*value = named->value.ll;
	break;
    case KSTAT_DATA_ULONGLONG:
	*value = named->value.ull;
	break;
#endif
    default:
	DEBUGMSGTL(("kernel_sunos5", 
		    "non-int type in kstat data: \"%s\" \"%s\" \"%s\" %d\n",
		    classname, statname, varname, named->data_type));
	ret = 0;            /* fail */
	break;
    }
 Return:
    return ret;
}

int
getKstat(const char *statname, const char *varname, void *value)
{
    kstat_ctl_t    *ksc;
    kstat_t        *ks, *kstat_data;
    kstat_named_t  *d;
    size_t          i, instance;
    char            module_name[64];
    int             ret;
    u_longlong_t    val;    /* The largest value */
    void           *v;

    if (value == NULL) {      /* Pretty useless but ... */
	v = (void *) &val;
    } else {
	v = value;
    }

    if (kstat_fd == 0) {
	kstat_fd = kstat_open();
	if (kstat_fd == 0) {
	    snmp_log(LOG_ERR, "kstat_open(): failed\n");
	}
    }
    if ((ksc = kstat_fd) == NULL) {
	ret = -10;
	goto Return;        /* kstat errors */
    }
    if (statname == NULL || varname == NULL) {
	ret = -20;
	goto Return;
    }

    /*
     * First, get "kstat_headers" statistics. It should
     * contain all available modules. 
     */

    if ((ks = kstat_lookup(ksc, "unix", 0, "kstat_headers")) == NULL) {
	ret = -10;
	goto Return;        /* kstat errors */
    }
    if (kstat_read(ksc, ks, NULL) <= 0) {
	ret = -10;
	goto Return;        /* kstat errors */
    }
    kstat_data = ks->ks_data;
    
    /*
     * Now, look for the name of our stat in the headers buf 
     */
    for (i = 0; i < ks->ks_ndata; i++) {
	DEBUGMSGTL(("kernel_sunos5",
		    "module: %s instance: %d name: %s class: %s type: %d flags: %x\n",
		    kstat_data[i].ks_module, kstat_data[i].ks_instance,
		    kstat_data[i].ks_name, kstat_data[i].ks_class,
		    kstat_data[i].ks_type, kstat_data[i].ks_flags));
	if (strcmp(statname, kstat_data[i].ks_name) == 0) {
	    strcpy(module_name, kstat_data[i].ks_module);
	    instance = kstat_data[i].ks_instance;
	    break;
	}
    }
    
    if (i == ks->ks_ndata) {
	ret = -1;
	goto Return;        /* Not found */
    }
    
    /*
     * Get the named statistics 
     */
    if ((ks = kstat_lookup(ksc, module_name, instance, statname)) == NULL) {
	ret = -10;
	goto Return;        /* kstat errors */
    }

    if (kstat_read(ksc, ks, NULL) <= 0) {
	ret = -10;
	goto Return;        /* kstat errors */
    }
    /*
     * This function expects only name/value type of statistics, so if it is
     * not the case return an error
     */
    if (ks->ks_type != KSTAT_TYPE_NAMED) {
	ret = -2;
	goto Return;        /* Invalid stat type */
    }
    
    for (i = 0, d = KSTAT_NAMED_PTR(ks); i < ks->ks_ndata; i++, d++) {
	DEBUGMSGTL(("kernel_sunos5", "variable: \"%s\" (type %d)\n", 
		    d->name, d->data_type));

	if (strcmp(d->name, varname) == 0) {
	    switch (d->data_type) {
	    case KSTAT_DATA_CHAR:
		*(char *)v = (int)d->value.c;
		DEBUGMSGTL(("kernel_sunos5", "value: %d\n", (int)d->value.c));
		break;
#ifdef KSTAT_DATA_INT32         /* Solaris 2.6 and up */
	    case KSTAT_DATA_INT32:
		*(Counter *)v = d->value.i32;
		DEBUGMSGTL(("kernel_sunos5", "value: %d\n", d->value.i32));
		break;
	    case KSTAT_DATA_UINT32:
		*(Counter *)v = d->value.ui32;
		DEBUGMSGTL(("kernel_sunos5", "value: %u\n", d->value.ui32));
		break;
	    case KSTAT_DATA_INT64:
		*(int64_t *)v = d->value.i64;
		DEBUGMSGTL(("kernel_sunos5", "value: %ld\n", d->value.i64));
		break;
	    case KSTAT_DATA_UINT64:
		*(uint64_t *)v = d->value.ui64;
		DEBUGMSGTL(("kernel_sunos5", "value: %lu\n", d->value.ui64));
		break;
#else
	    case KSTAT_DATA_LONG:
		*(Counter *)v = d->value.l;
		DEBUGMSGTL(("kernel_sunos5", "value: %ld\n", d->value.l));
		break;
	    case KSTAT_DATA_ULONG:
		*(Counter *)v = d->value.ul;
		DEBUGMSGTL(("kernel_sunos5", "value: %lu\n", d->value.ul));
		break;
	    case KSTAT_DATA_LONGLONG:
		*(Counter *)v = d->value.ll;
		DEBUGMSGTL(("kernel_sunos5", "value: %lld\n",
			    (long)d->value.ll));
		break;
	    case KSTAT_DATA_ULONGLONG:
		*(Counter *)v = d->value.ull;
		DEBUGMSGTL(("kernel_sunos5", "value: %llu\n",
			    (unsigned long)d->value.ull));
		break;
#endif
	    case KSTAT_DATA_FLOAT:
		*(float *)v = d->value.f;
		DEBUGMSGTL(("kernel_sunos5", "value: %f\n", d->value.f));
		break;
	    case KSTAT_DATA_DOUBLE:
		*(double *)v = d->value.d;
		DEBUGMSGTL(("kernel_sunos5", "value: %f\n", d->value.d));
		break;
	    default:
		DEBUGMSGTL(("kernel_sunos5",
			    "UNKNOWN TYPE %d (stat \"%s\" var \"%s\")\n",
			    d->data_type, statname, varname));
		ret = -3;
		goto Return;        /* Invalid data type */
	    }
	    ret = 0;        /* Success  */
	    goto Return;
	}
    }
    ret = -4;               /* Name not found */
 Return:
    return ret;
}


/*
 * get MIB-II statistics. It maintaines a simple cache which buffers the last
 * read block of MIB statistics (which may contain the whole table). It calls
 * *comp to compare every entry with an entry pointed by arg. *comp should
 * return 0 if comparison is successful.  Req_type may be GET_FIRST, GET_EXACT,
 * GET_NEXT.  If search is successful getMibstat returns 0, otherwise 1.
 */
int
getMibstat(mibgroup_e grid, void *resp, size_t entrysize,
	   req_e req_type, int (*comp) (void *, void *), void *arg)
{
    int             ret, rc = -1, mibgr, mibtb, cache_valid;
    size_t          length;
    mibcache       *cachep;
    found_e         result = NOT_FOUND;
    void           *ep;

    /*
     * We assume that Mibcache is initialized in mibgroup_e enum order so we
     * don't check the validity of index here.
     */

    DEBUGMSGTL(("kernel_sunos5", "getMibstat (%d, *, %d, %d, *, *)\n",
		grid, entrysize, req_type));
    cachep = &Mibcache[grid];
    mibgr = Mibmap[grid].group;
    mibtb = Mibmap[grid].table;

    if (cachep->cache_addr == (void *) -1)  /* Hasn't been initialized yet */
	init_mibcache_element(cachep);
    if (cachep->cache_size == 0) {  /* Memory allocation problems */
	cachep->cache_addr = resp;  /* So use caller supplied address instead of cache */
	cachep->cache_size = entrysize;
	cachep->cache_last_found = 0;
    }
    if (req_type != GET_NEXT)
	cachep->cache_last_found = 0;

    cache_valid = (cachep->cache_time > 0);

    DEBUGMSGTL(("kernel_sunos5","... cache_valid %d time %ld ttl %d now %ld\n",
		cache_valid, cachep->cache_time, cachep->cache_ttl,
		time(NULL)));
    if (cache_valid) {
	/*
	 * Is it really? 
	 */
	if (cachep->cache_comp != (void *)comp || cachep->cache_arg != arg) {
	    cache_valid = 0;        /* Nope. */
	}
    }

    if (cache_valid) {
	/*
	 * Entry is valid, let's try to find a match 
	 */

	if (req_type == GET_NEXT) {
	    result = getentry(req_type,
			      (void *)((char *)cachep->cache_addr +
				       (cachep->cache_last_found * entrysize)),
			      cachep->cache_length -
			      (cachep->cache_last_found * entrysize),
			      entrysize, &ep, comp, arg);
            } else {
                result = getentry(req_type, cachep->cache_addr,
				  cachep->cache_length, entrysize, &ep, comp,
				  arg);
            }
    }

    if ((cache_valid == 0) || (result == NOT_FOUND) ||
	(result == NEED_NEXT && cachep->cache_flags & CACHE_MOREDATA)) {
	/*
	 * Either the cache is old, or we haven't found anything, or need the
	 * next item which hasn't been read yet.  In any case, fill the cache
	 * up and try to find our entry.
	 */

	if (grid == MIB_INTERFACES) {
	    rc = getif((mib2_ifEntry_t *) cachep->cache_addr,
		       cachep->cache_size, req_type,
		       (mib2_ifEntry_t *) & ep, &length, comp, arg);
	} else {
	    rc = getmib(mibgr, mibtb, cachep->cache_addr,
			cachep->cache_size, entrysize, req_type, &ep,
			&length, comp, arg);
	}

	if (rc >= 0) {      /* Cache has been filled up */
	    cachep->cache_time = cachep->cache_ttl;
	    cachep->cache_length = length;
	    if (rc == 1)    /* Found but there are more unread data */
		cachep->cache_flags |= CACHE_MOREDATA;
	    else {
		cachep->cache_flags &= ~CACHE_MOREDATA;
                if (rc > 1)  {
                    cachep->cache_time = 0;
                    }
                 }
	    cachep->cache_comp = (void *) comp;
	    cachep->cache_arg = arg;
	} else {
	    cachep->cache_comp = NULL;
	    cachep->cache_arg = NULL;
	}
    }
    DEBUGMSGTL(("kernel_sunos5", "... result %d rc %d\n", result, rc));
    
    if (result == FOUND || rc == 0 || rc == 1) {
	/*
	 * Entry has been found, deliver it 
	 */
	if (resp != NULL) {
	    memcpy(resp, ep, entrysize);
	}
	ret = 0;
	cachep->cache_last_found =
	    ((char *)ep - (char *)cachep->cache_addr) / entrysize;
    } else {
	ret = 1;            /* Not found */
    }
    DEBUGMSGTL(("kernel_sunos5", "... getMibstat returns %d\n", ret));
    return ret;
}

/*
 * Get a MIB-II entry from the buffer buffaddr, which satisfies the criterion,
 * computed by (*comp), which gets arg as the first argument and pointer to the
 * current position in the buffer as the second. If found entry is pointed by
 * resp.
 */

static found_e
getentry(req_e req_type, void *bufaddr, size_t len,
	 size_t entrysize, void *resp, int (*comp)(void *, void *),
	 void *arg)
{
    void *bp = bufaddr, **rp = resp;
    int previous_found = 0;
    
    /*
     * Here we have to perform address arithmetic with pointer to void. Ugly...
     */

    for (; len > 0; len -= entrysize, bp = (char *) bp + entrysize) {
	if (rp != (void *) NULL) {
	    *rp = bp;
	}

	if (req_type == GET_FIRST || (req_type == GET_NEXT && previous_found)){
	    return FOUND;
	}

	if ((*comp)(arg, bp) == 0) {
	    if (req_type == GET_EXACT) {
		return FOUND;
	    } else {        /* GET_NEXT */
		previous_found++;
		continue;
	    }
	}
    }

    if (previous_found) {
	return NEED_NEXT;
    } else {
	return NOT_FOUND;
    }
}

/*
 * Initialize a cache element. It allocates the memory and sets the time stamp
 * to invalidate the element.
 */
static void
init_mibcache_element(mibcache * cp)
{
    if (cp == (mibcache *)NULL) {
	return;
    }
    if (cp->cache_size) {
	cp->cache_addr = malloc(cp->cache_size);
    }
    cp->cache_time = 0;
    cp->cache_comp = NULL;
    cp->cache_arg = NULL;
}

/*
 * Get MIB-II statistics from the Solaris kernel.  It uses undocumented
 * interface to TCP/IP streams modules, which provides extended MIB-II for the
 * following groups: ip, icmp, tcp, udp, egp.
 
 * 
 * Usage: groupname, subgroupname are from <inet/mib2.h>, 
 *        size%sizeof(statbuf) == 0,
 *        entrysize should be exact size of MIB-II entry,
 *        req_type:
 *                   GET_FIRST - get the first entry in the buffer
 *                   GET_EXACT - get exact match
 *                   GET_NEXT  - get next entry after the exact match
 * 
 * (*comp) is a compare function, provided by the caller, which gets arg as the
 * first argument and pointer to the current entry as th second. If compared,
 * should return 0 and found entry will be pointed by resp.
 * 
 * If search is successful and no more data to read, it returns 0,
 * if successful and there is more data -- 1,
 * if not found and end of data -- 2, any other errors -- < 0
 * (negative error numbers are pretty random).
 * 
 * NOTE: needs to be protected by a mutex in reentrant environment 
 */

static int
getmib(int groupname, int subgroupname, void *statbuf, size_t size,
       size_t entrysize, req_e req_type, void *resp,
       size_t *length, int (*comp)(void *, void *), void *arg)
{
    int             rc, ret = 0, flags;
    char            buf[BUFSIZE];
    struct strbuf   strbuf;
    struct T_optmgmt_req *tor = (struct T_optmgmt_req *) buf;
    struct T_optmgmt_ack *toa = (struct T_optmgmt_ack *) buf;
    struct T_error_ack *tea = (struct T_error_ack *) buf;
    struct opthdr  *req;
    found_e         result = FOUND;

    DEBUGMSGTL(("kernel_sunos5", "...... getmib (%d, %d, ...)\n",
		groupname, subgroupname));

    /*
     * Open the stream driver and push all MIB-related modules 
     */

    if (sd == -1) {         /* First time */
	if ((sd = open(STREAM_DEV, O_RDWR)) == -1) {
	    ret = -1;
	    goto Return;
	}
	if (ioctl(sd, I_PUSH, "arp") == -1) {
	    ret = -1;
	    goto Return;
	}
	if (ioctl(sd, I_PUSH, "tcp") == -1) {
	    ret = -1;
	    goto Return;
	}
	if (ioctl(sd, I_PUSH, "udp") == -1) {
	    ret = -1;
	    goto Return;
	}
	DEBUGMSGTL(("kernel_sunos5", "...... modules pushed OK\n"));
    }

    /*
     * First, use bigger buffer, to accelerate skipping unwanted messages
     */

    strbuf.buf = buf;
    strbuf.maxlen = BUFSIZE;
    
    tor->PRIM_type = T_OPTMGMT_REQ;
    tor->OPT_offset = sizeof(struct T_optmgmt_req);
    tor->OPT_length = sizeof(struct opthdr);
#ifdef MI_T_CURRENT
    tor->MGMT_flags = MI_T_CURRENT; /* Solaris < 2.6 */
#else
    tor->MGMT_flags = T_CURRENT;    /* Solaris 2.6 */
#endif
    req = (struct opthdr *)(tor + 1);
    req->level = groupname;
    req->name = subgroupname;
    req->len = 0;
    strbuf.len = tor->OPT_length + tor->OPT_offset;
    flags = 0;
    if ((rc = putmsg(sd, &strbuf, NULL, flags))) {
	ret = -2;
	goto Return;
    }

    req = (struct opthdr *) (toa + 1);
    for (;;) {
	flags = 0;
	if ((rc = getmsg(sd, &strbuf, NULL, &flags)) == -1) {
	    ret = -EIO;
	    break;
	}
	if (rc == 0 && strbuf.len >= sizeof(struct T_optmgmt_ack) &&
	    toa->PRIM_type  == T_OPTMGMT_ACK &&
	    toa->MGMT_flags == T_SUCCESS && req->len == 0) {
	    ret = 2;
	    break;
	}
	if (strbuf.len >= sizeof(struct T_error_ack) &&
	    tea->PRIM_type == T_ERROR_ACK) {
	    /* Protocol error */
	    ret = -((tea->TLI_error == TSYSERR) ? tea->UNIX_error : EPROTO);
	    break;
	}
	if (rc != MOREDATA || strbuf.len < sizeof(struct T_optmgmt_ack) ||
	    toa->PRIM_type != T_OPTMGMT_ACK ||
	    toa->MGMT_flags != T_SUCCESS) {
	    ret = -ENOMSG;  /* No more messages */
	    break;
	}

	/*
	 * The order in which we get the statistics is determined by the kernel
	 * and not by the group name, so we have to loop until we get the
	 * required statistics.
	 */

	if (req->level != groupname || req->name != subgroupname) {
	    strbuf.maxlen = BUFSIZE;
	    strbuf.buf = buf;
	    do {
		rc = getmsg(sd, NULL, &strbuf, &flags);
	    } while (rc == MOREDATA);
	    continue;
	}
        
	/*
	 * Now when we found our stat, switch buffer to a caller-provided
	 * one. Manipulating the size of it one can control performance,
	 * reducing the number of getmsg calls
	 */

	strbuf.buf = statbuf;
	strbuf.maxlen = size;
	strbuf.len = 0;
	flags = 0;
	do {
	    rc = getmsg(sd, NULL, &strbuf, &flags);
	    switch (rc) {
	    case -1:
		rc = -ENOSR;
		goto Return;

	    default:
		rc = -ENODATA;
		goto Return;

	    case MOREDATA:
	    case 0:
		if (req_type == GET_NEXT && result == NEED_NEXT)
		    /*
		     * End of buffer, so "next" is the first item in the next
		     * buffer  
		     */
		    req_type = GET_FIRST;
		result = getentry(req_type, (void *) strbuf.buf, strbuf.len,
				  entrysize, resp, comp, arg);
		*length = strbuf.len;       /* To use in caller for cacheing */
		break;
	    }
	} while (rc == MOREDATA && result != FOUND);

	if (result == FOUND) {      /* Search is successful */
	    if (rc != MOREDATA) {
		ret = 0;    /* Found and no more data */
	    } else {
		ret = 1;    /* Found and there is another unread data block */
	    }
	    break;
	} else {            /* Restore buffers, continue search */
	    strbuf.buf = buf;
	    strbuf.maxlen = BUFSIZE;
	}
    }
 Return:
    ioctl(sd, I_FLUSH, FLUSHRW);
    DEBUGMSGTL(("kernel_sunos5", "...... getmib returns %d\n", ret));
    return ret;
}
  
/*
 * Get info for interfaces group. Mimics getmib interface as much as possible
 * to be substituted later if SunSoft decides to extend its mib2 interface.
 */
static int
getif(mib2_ifEntry_t *ifbuf, size_t size, req_e req_type,
      mib2_ifEntry_t *resp,  size_t *length, int (*comp)(void *, void *),
      void *arg)
{
    int             i, ret, idx = 1;
    int             ifsd;
    static char    *buf = NULL;
    static int      bufsize = 0;
    struct ifconf   ifconf;
    struct ifreq   *ifrp;
    mib2_ifEntry_t *ifp;
    mib2_ipNetToMediaEntry_t Media;
    int             nentries = size / sizeof(mib2_ifEntry_t);
    found_e         result = NOT_FOUND;

    if ((ifsd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	return -1;
    }

    if (!buf) {
	bufsize = 10240;
	buf = malloc(bufsize);
	if (!buf) {
	    ret = -1;
	    goto Return;
	}
    }

    ifconf.ifc_buf = buf;
    ifconf.ifc_len = bufsize;
    while (ioctl(ifsd, SIOCGIFCONF, &ifconf) == -1) {
	bufsize += 10240;
	free(buf);
	buf = malloc(bufsize);
	if (!buf) {
	    ret = -1;
	    goto Return;
	}
	ifconf.ifc_buf = buf;
	ifconf.ifc_len = bufsize;
    }

 Again:
    for (i = 0, ifp = (mib2_ifEntry_t *) ifbuf, ifrp = ifconf.ifc_req;
	 ((char *) ifrp < ((char *) ifconf.ifc_buf + ifconf.ifc_len))
             && (i < nentries); i++, ifp++, ifrp++, idx++) {

	DEBUGMSGTL(("kernel_sunos5", "...... getif %s\n", ifrp->ifr_name));

	if (ioctl(ifsd, SIOCGIFFLAGS, ifrp) < 0) {
	    ret = -1;
	    DEBUGMSGTL(("kernel_sunos5", "...... SIOCGIFFLAGS failed\n"));
	    goto Return;
	}

	memset(ifp, 0, sizeof(mib2_ifEntry_t));
	ifp->ifIndex = idx;
	ifp->ifDescr.o_length = strlen(ifrp->ifr_name);
	strcpy(ifp->ifDescr.o_bytes, ifrp->ifr_name);
	ifp->ifAdminStatus = (ifrp->ifr_flags & IFF_RUNNING) ? 1 : 2;
	ifp->ifOperStatus = (ifrp->ifr_flags & IFF_UP) ? 1 : 2;
	ifp->ifLastChange = 0;      /* Who knows ...  */

	if (ioctl(ifsd, SIOCGIFMTU, ifrp) < 0) {
	    ret = -1;
	    DEBUGMSGTL(("kernel_sunos5", "...... SIOCGIFMTU failed\n"));
	    goto Return;
	}
	ifp->ifMtu = ifrp->ifr_metric;
	ifp->ifType = 1;
	ifp->ifSpeed = 0;

	if ((getKstatInt(NULL,ifrp->ifr_name, "ifspeed", &ifp->ifSpeed) == 0) &&
	    (ifp->ifSpeed != 0)) {
	    /*
	     * check for SunOS patch with half implemented ifSpeed 
	     */
	    if (ifp->ifSpeed < 10000) {
                    ifp->ifSpeed *= 1000000;
	    }
	} else if (getKstatInt(NULL,ifrp->ifr_name, "ifSpeed", &ifp->ifSpeed) == 0) {
	    /*
	     * this is good 
	     */
	}

	switch (ifrp->ifr_name[0]) {
	case 'l':          /* le / lo / lane (ATM LAN Emulation) */
	    if (ifrp->ifr_name[1] == 'o') {
		if (!ifp->ifSpeed)
		    ifp->ifSpeed = 127000000;
		ifp->ifType = 24;
	    } else if (ifrp->ifr_name[1] == 'e') {
		if (!ifp->ifSpeed)
		    ifp->ifSpeed = 10000000;
		ifp->ifType = 6;
	    } else if (ifrp->ifr_name[1] == 'a') {
		if (!ifp->ifSpeed)
		    ifp->ifSpeed = 155000000;
		ifp->ifType = 37;
	    }
	    break;

	case 'g':          /* ge (gigabit ethernet card)  */
	    if (!ifp->ifSpeed)
		ifp->ifSpeed = 1000000000;
	    ifp->ifType = 6;
	    break;

	case 'h':          /* hme (SBus card) */
	case 'e':          /* eri (PCI card) */
	case 'b':          /* be */
	case 'd':          /* dmfe -- found on netra X1 */
	    if (!ifp->ifSpeed)
		ifp->ifSpeed = 100000000;
	    ifp->ifType = 6;
	    break;

	case 'f':          /* fa (Fore ATM */
	    if (!ifp->ifSpeed)
		ifp->ifSpeed = 155000000;
	    ifp->ifType = 37;
	    break;

	case 'q':         /* qe (QuadEther)/qa (Fore ATM)/qfe (QuadFastEther)*/
	    if (ifrp->ifr_name[1] == 'a') {
		if (!ifp->ifSpeed)
		    ifp->ifSpeed = 155000000;
		ifp->ifType = 37;
	    } else if (ifrp->ifr_name[1] == 'e') {
		if (!ifp->ifSpeed)
		    ifp->ifSpeed = 10000000;
		ifp->ifType = 6;
	    } else if (ifrp->ifr_name[1] == 'f') {
		if (!ifp->ifSpeed)
		    ifp->ifSpeed = 100000000;
		ifp->ifType = 6;
	    }
	    break;
	}

	if (!strchr(ifrp->ifr_name, ':')) {
	    Counter l_tmp;

	    if (getKstatInt(NULL,ifrp->ifr_name, "ipackets", &ifp->ifInUcastPkts) < 0){
		ret = -1;
		goto Return;
	    }
            
	    if (getKstatInt(NULL,ifrp->ifr_name, "rbytes", &ifp->ifInOctets) < 0) {
                    ifp->ifInOctets = ifp->ifInUcastPkts * 308; /* XXX */
	    }
            
	    if (getKstatInt(NULL,ifrp->ifr_name, "opackets",&ifp->ifOutUcastPkts) < 0){
		ret = -1;
		goto Return;
	    }
            
	    if (getKstatInt(NULL,ifrp->ifr_name, "obytes", &ifp->ifOutOctets) < 0) {
		ifp->ifOutOctets = ifp->ifOutUcastPkts * 308;       /* XXX */
	    }

	    if (ifp->ifType == 24)  /* Loopback */
		continue;

	    if (getKstatInt(NULL,ifrp->ifr_name, "ierrors", &ifp->ifInErrors) < 0) {
		ret = -1;
		goto Return;
	    }

	    if (getKstatInt(NULL,ifrp->ifr_name, "oerrors", &ifp->ifOutErrors) < 0) {
		ret = -1;
		goto Return;
	    }

	    if (getKstatInt(NULL,ifrp->ifr_name, "brdcstrcv",&ifp->ifInNUcastPkts)==0&&
		getKstatInt(NULL,ifrp->ifr_name, "multircv", &l_tmp) == 0) {
		ifp->ifInNUcastPkts += l_tmp;
	    }

	    if (getKstatInt(NULL,ifrp->ifr_name,"brdcstxmt",&ifp->ifOutNUcastPkts)==0&&
		getKstatInt(NULL,ifrp->ifr_name, "multixmt", &l_tmp) == 0) {
		ifp->ifOutNUcastPkts += l_tmp;
	    }
	}

	/*
	 * An attempt to determine the physical address of the interface.
	 * There should be a more elegant solution using DLPI, but "the margin
	 * is too small to put it here ..."
	 */

	if (ioctl(ifsd, SIOCGIFADDR, ifrp) < 0) {
	    ret = -1;
	    goto Return;
	}

	if (getMibstat(MIB_IP_NET, &Media, sizeof(mib2_ipNetToMediaEntry_t),
		       GET_EXACT, &Name_cmp, ifrp) == 0) {
	    ifp->ifPhysAddress = Media.ipNetToMediaPhysAddress;
	}
    }

    if ((req_type == GET_NEXT) && (result == NEED_NEXT)) {
            /*
             * End of buffer, so "next" is the first item in the next buffer 
             */
            req_type = GET_FIRST;
    }

    result = getentry(req_type, (void *) ifbuf, size, sizeof(mib2_ifEntry_t),
		      (void *)resp, comp, arg);

    if ((result != FOUND) && (i == nentries) && 
	((char *)ifrp < (char *)ifconf.ifc_buf + ifconf.ifc_len)) {
	/*
	 * We reached the end of supplied buffer, but there is
	 * some more stuff to read, so continue.
	 */
	ifconf.ifc_len -= i * sizeof(struct ifreq);
	ifconf.ifc_req = ifrp;
	goto Again;
    }

    if (result != FOUND) {
            ret = 2;
    } else {
	if ((char *)ifrp < (char *)ifconf.ifc_buf + ifconf.ifc_len) {
	    ret = 1;        /* Found and more data to fetch */
	} else {
	    ret = 0;        /* Found and no more data */
	}
	*length = i * sizeof(mib2_ifEntry_t);       /* Actual cache length */
    }

 Return:
    close(ifsd);
    return ret;
}

/*
 * Always TRUE. May be used as a comparison function in getMibstat
 * to obtain the whole table (GET_FIRST should be used) 
 */
int
Get_everything(void *x, void *y)
{
    return 0;             /* Always TRUE */
}

/*
 * Compare name and IP address of the interface to ARP table entry.
 * Needed to obtain the physical address of the interface in getif.
 */
static int
Name_cmp(void *ifrp, void *ep)
{
    struct sockaddr_in *s = (struct sockaddr_in *)
	                                   &(((struct ifreq *)ifrp)->ifr_addr);
    mib2_ipNetToMediaEntry_t *Ep = (mib2_ipNetToMediaEntry_t *)ep;

    if ((strncmp(Ep->ipNetToMediaIfIndex.o_bytes,
		 ((struct ifreq *)ifrp)->ifr_name,
		 Ep->ipNetToMediaIfIndex.o_length) == 0) &&
	(s->sin_addr.s_addr == Ep->ipNetToMediaNetAddress)) {
	return 0;
    } else {
	return 1;
    }
}	

#ifdef _STDC_COMPAT
#ifdef __cplusplus
}
#endif
#endif

#ifdef _GETKSTAT_TEST

int
main(int argc, char **argv)
{
    int             rc = 0;
    u_long          val = 0;

    if (argc != 3) {
        snmp_log(LOG_ERR, "Usage: %s stat_name var_name\n", argv[0]);
        exit(1);
    }

    snmp_set_do_debugging(1);
    rc = getKstat(argv[1], argv[2], &val);

    if (rc == 0)
        snmp_log(LOG_ERR, "%s = %lu\n", argv[2], val);
    else
        snmp_log(LOG_ERR, "rc =%d\n", rc);
    return 0;
}
#endif /*_GETKSTAT_TEST */

#ifdef _GETMIBSTAT_TEST

int
ip20comp(void *ifname, void *ipp)
{
    return (strncmp((char *) ifname,
                    ((mib2_ipAddrEntry_t *) ipp)->ipAdEntIfIndex.o_bytes,
                    ((mib2_ipAddrEntry_t *) ipp)->ipAdEntIfIndex.
                    o_length));
}

int
ARP_Cmp_Addr(void *addr, void *ep)
{
    DEBUGMSGTL(("kernel_sunos5", "ARP: %lx <> %lx\n",
                ((mib2_ipNetToMediaEntry_t *) ep)->ipNetToMediaNetAddress,
                *(IpAddress *) addr));
    if (((mib2_ipNetToMediaEntry_t *) ep)->ipNetToMediaNetAddress ==
        *(IpAddress *)addr) {
        return 0;
    } else {
        return 1;
    }
}

int
IF_cmp(void *addr, void *ep)
{
    if (((mib2_ifEntry_t *)ep)->ifIndex ==((mib2_ifEntry_t *)addr)->ifIndex) {
        return 0;
    } else {
        return 1;
    }
}

int
main(int argc, char **argv)
{
    int             rc = 0, i, idx;
    mib2_ipAddrEntry_t ipbuf, *ipp = &ipbuf;
    mib2_ipNetToMediaEntry_t entry, *ep = &entry;
    mib2_ifEntry_t  ifstat;
    req_e           req_type;
    IpAddress       LastAddr = 0;

    if (argc != 3) {
        snmp_log(LOG_ERR,
                 "Usage: %s if_name req_type (0 first, 1 exact, 2 next) \n",
                 argv[0]);
        exit(1);
    }

    switch (atoi(argv[2])) {
    case 0:
        req_type = GET_FIRST;
        break;
    case 1:
        req_type = GET_EXACT;
        break;
    case 2:
        req_type = GET_NEXT;
        break;
    };

    snmp_set_do_debugging(0);
    while ((rc =
            getMibstat(MIB_INTERFACES, &ifstat, sizeof(mib2_ifEntry_t),
                       req_type, &IF_cmp, &idx)) == 0) {
        idx = ifstat.ifIndex;
        DEBUGMSGTL(("kernel_sunos5", "Ifname = %s\n",
                    ifstat.ifDescr.o_bytes));
        req_type = GET_NEXT;
    }
    rc = getMibstat(MIB_IP_ADDR, &ipbuf, sizeof(mib2_ipAddrEntry_t),
                    req_type, ip20comp, argv[1]);

    if (rc == 0)
        DEBUGMSGTL(("kernel_sunos5", "mtu = %ld\n",
                    ipp->ipAdEntInfo.ae_mtu));
    else
        DEBUGMSGTL(("kernel_sunos5", "rc =%d\n", rc));

    while ((rc =
            getMibstat(MIB_IP_NET, &entry,
                       sizeof(mib2_ipNetToMediaEntry_t), req_type,
                       &ARP_Cmp_Addr, &LastAddr)) == 0) {
        LastAddr = ep->ipNetToMediaNetAddress;
        DEBUGMSGTL(("kernel_sunos5", "Ipaddr = %lX\n", (u_long) LastAddr));
        req_type = GET_NEXT;
    }
    return 0;
}
#endif /*_GETMIBSTAT_TEST */
#endif                          /* SUNOS5 */


/*-
 * These variables describe the formatting of this file.  If you don't like the
 * template defaults, feel free to change them here (not in your .emacs file).
 *
 * Local Variables:
 * comment-column: 32
 * c-indent-level: 4
 * c-continued-statement-offset: 4
 * c-brace-offset: -4
 * c-argdecl-indent: 0
 * c-label-offset: -4
 * fill-column: 79
 * fill-prefix: " * "
 * End:
 */

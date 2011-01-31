/* itf.c - IP interface registry */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <linux/atmclip.h>
#include <sys/socket.h>
#define _LINUX_NETDEVICE_H /* glibc2 */
#include <linux/types.h>
#include <linux/if_arp.h>

#include "atmd.h"

#include "table.h"
#include "io.h"
#include "itf.h"
#include "arp.h"


#define COMPONENT "ITF"


ITF *lookup_itf(int number)
{
    ITF *itf;

    for (itf = itfs; itf; itf = itf->next)
	if (itf->number == number) break;
    return itf;
}


ITF *lookup_itf_by_ip(uint32_t ip)
{
    ITF *itf;

    for (itf = itfs; itf; itf = itf->next)
	if (!((itf->local_ip ^ ip) & itf->netmask)) break;
    return itf;
}


void itf_create(int number)
{
    diag(COMPONENT,DIAG_DEBUG,"ITF CREATE %d",number);
}


void itf_up(int number)
{
    ITF *itf;

    diag(COMPONENT,DIAG_DEBUG,"ITF UP %d",number);
    if (lookup_itf(number)) {
	diag(COMPONENT,DIAG_ERROR,"interface is already active");
	return;
    }
    itf = alloc_t(ITF);
    if (ip_itf_info(number,&itf->local_ip,&itf->netmask,&itf->mtu) < 0) {
	free(itf);
	return;
    }
    itf->number = number;
    memset(&itf->qos,0,sizeof(struct atm_qos));
    itf->qos.aal = ATM_AAL5;
    itf->qos.txtp.traffic_class = ATM_UBR;
    itf->qos.txtp.max_sdu = RFC1483LLC_LEN+RFC1626_MTU;
    itf->qos.rxtp = itf->qos.txtp;
    itf->sndbuf = 0; /* use system default */
    itf->table = itf->arp_srv = NULL;
    Q_INSERT_HEAD(itfs,itf);
}


static void itf_bring_down(ITF *itf)
{
    ENTRY *entry,*next;

    for (entry = itf->table; entry; entry = next) {
	next = entry->next;
	discard_entry(entry);
    }
    Q_REMOVE(itfs,itf);
    free(itf);
}


void itf_down(int number)
{
    ITF *itf;

    diag(COMPONENT,DIAG_DEBUG,"ITF DOWN %d",number);
    itf = lookup_itf(number);
    if (!itf) {
	diag(COMPONENT,DIAG_ERROR,"no such interface (%d)",number);
	return;
    }
    itf_bring_down(itf);
}


void itf_change(int number)
{
    ITF *itf;
    ENTRY *entry,*next,*disconnected;
    uint32_t local_ip,netmask;
    int mtu;

    diag(COMPONENT,DIAG_DEBUG,"ITF CHANGE %d",number);
    itf = lookup_itf(number);
    if (!itf) {
	diag(COMPONENT,DIAG_DEBUG,"no interface to change (%d)",number);
	return;
    }
    if (ip_itf_info(number,&local_ip,&netmask,&mtu) < 0) {
	itf_bring_down(itf);
	return;
    }
    disconnected = NULL;
    for (entry = itf->table; entry; entry = next) {
	next = entry->next;
	if ((entry->flags & ATF_PERM) &&
	  !((entry->ip ^ local_ip) & (netmask | itf->netmask))) continue;
	if ((entry->flags & ATF_ARPSRV) && !((entry->ip ^ local_ip) & netmask))
	  {
	    disconnected = entry;
	    discard_vccs(entry);
	    /* @@@ should adjust max_sdu if mtu changed */
	    continue;
	}
	discard_entry(entry);
    }
    itf->local_ip = local_ip;
    itf->netmask = netmask;
    itf->mtu = mtu;
    if (disconnected) vcc_detach(disconnected);
}

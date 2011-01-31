/* table.c - ATMARP table */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/atm.h>

#include "atm.h"
#include "atmd.h"
#include "atmarpd.h"

#include "table.h"


#define COMPONENT "TABLE"


const char *entry_state_name[] = { "NONE","RESOLV","INVALID","VALID" };


ENTRY *alloc_entry(int svc)
{
    ENTRY *entry;

    entry = alloc_t(ENTRY);
    entry->state = as_none;
    entry->svc = svc;
    entry->ip = 0;
    entry->addr = NULL;
    entry->flags = 0;
    entry->timer = NULL;
    entry->vccs = NULL;
    entry->notify = NULL;
    entry->itf = NULL;
    return entry;
}


ENTRY *lookup_ip(const ITF *itf,uint32_t ip)
{
    ENTRY *walk;

    for (walk = itf->table; walk; walk = walk->next)
	if (walk->ip == ip) break;
    return walk;
}


ENTRY *lookup_addr(const ITF *itf,const struct sockaddr_atmsvc *addr)
{
    ENTRY *walk;

    for (walk = itf->table; walk; walk = walk->next)
	if (walk->addr && atm_equal((struct sockaddr *) walk->addr,
	  (struct sockaddr *) addr,0,0)) break;
    return walk;
}


ENTRY *lookup_incoming(const struct sockaddr_atmsvc *addr)
{
    ENTRY *walk;

    for (walk = unknown_incoming; walk; walk = walk->next)
	if (walk->addr && atm_equal((struct sockaddr *) walk->addr,
	  (struct sockaddr *) addr,0,0)) break;
    return walk;
}


static int table_uptodate = 0; /* ATMARP table file is up to date */
static FILE *out_file = NULL;
static int out_error = 0;


static void output(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    if (!out_file) vdiag(COMPONENT,DIAG_DEBUG,fmt,ap);
    else if (vfprintf(out_file,fmt,ap) < 0 || putc('\n',out_file) < 0)
	    out_error = errno;
    va_end(ap);
}


static void dump_vcc(VCC *vcc)
{
    struct sockaddr_atmsvc addr;
    char addr_buf[MAX_ATM_ADDR_LEN+1];
    char qos_buf[MAX_ATM_QOS_LEN+1];
    struct atm_qos qos;
    int sndbuf;
    socklen_t size;

    size = sizeof(addr);
    if (getpeername(vcc->fd,(struct sockaddr *) &addr,&size) < 0) {
	diag(COMPONENT,DIAG_ERROR,"getpeername: %s",strerror(errno));
	strcpy(addr_buf,"<getsocknam error>");
    }
    else {
#if 0
	int i;

	for (i = 0; i < size; i++)
	    printf("%02X ",((unsigned char *) &addr)[i]);
	printf("\n");
#endif
	if (atm2text(addr_buf,sizeof(addr_buf),(struct sockaddr *) &addr,
	  pretty) < 0) strcpy(addr_buf,"<atm2text error>");
    }
    output("  %s%s",addr_buf,vcc->connecting ? ", connecting" :
      !vcc->entry || !vcc->entry->svc ? "" : vcc->active ? " (active)" :
      " (passive)");
    if (vcc->connecting) return;
    size = sizeof(qos);
    if (getsockopt(vcc->fd,SOL_ATM,SO_ATMQOS,&qos,&size) < 0)
	output("    QOS: <unavailable: %s>",strerror(errno));
    else if (!vcc->entry || !qos_equal(&vcc->entry->qos,&qos)) {
	    if (qos2text(qos_buf,sizeof(qos_buf),&qos,0) < 0)
	    strcpy(qos_buf,"<invalid qos>");
	    output("    QOS: %s",qos_buf);
	}
    size = sizeof(sndbuf);
    if (getsockopt(vcc->fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,&size) < 0)
	output("    Send buffer: <unavailable: %s>",strerror(errno));
    else if (!vcc->entry || vcc->entry->sndbuf != sndbuf)
	    output("    Send buffer: %d",sndbuf);
}


static void dump_vccs(VCC *vcc)
{
    while (vcc) {
	dump_vcc(vcc);
	vcc = vcc->next;
    }
}


static void dump_entries(ENTRY *list)
{
    static const char *flag_name[] = {
	"???",	"com", "PERM", "PUBL",			/* 0x0001-0x0008 */
	"trailers", "netmask", "dontpub", "magic",	/* 0x0010-0x0080 */
	"???", "???", "???", "???",			/* 0x0100-0x0800 */
	"NULL", "ARPSRV", "NOVC", "???" };		/* 0x1000-0x8000 */
    /* lower case flags are not used by ATMARP */
    ENTRY *entry;
    char addr_buf[MAX_ATM_ADDR_LEN+1];
    char qos_buf[MAX_ATM_QOS_LEN+1];
    char tmp[100]; /* large enough for all flags */
    unsigned char *ipp;
    int i;

    for (entry = list; entry ; entry = entry->next) {
	if (!entry->addr) strcpy(addr_buf,"<none>");
	else if (atm2text(addr_buf,MAX_ATM_ADDR_LEN+1,
	      (struct sockaddr *) entry->addr,pretty) < 0)
		strcpy(addr_buf,"<error>");
	ipp = (unsigned char *) &entry->ip;
	*tmp = 0;
	for (i = 0; i < 16; i++)
	    if (entry->flags & (1 << i)) {
		if (*tmp) strcat(tmp,",");
		strcat(tmp,flag_name[i]);
	    }
	output("IP %d.%d.%d.%d, state %s, addr %s, flags 0x%x<%s>",ipp[0],
	  ipp[1],ipp[2],ipp[3],entry_state_name[entry->state],addr_buf,
	  entry->flags,tmp);
	if (entry->itf && !qos_equal(&entry->itf->qos,&entry->qos)) {
	    if (qos2text(qos_buf,sizeof(qos_buf),&entry->qos,0) < 0)
		strcpy(qos_buf,"<error>");
	    output("  QOS: %s",qos_buf);
	}
	if (entry->itf && entry->sndbuf && entry->sndbuf != entry->itf->sndbuf)
	    output("  Send buffer: %d",entry->sndbuf);
	if (entry->notify) {
	    NOTIFY *notify;
	    int count;

	    count = 0;
	    for (notify = entry->notify; notify; notify = notify->next) count++;
	    output("  %d quer%s pending",count,count == 1 ? "y" : "ies");
	}
	dump_vccs(entry->vccs);
    }
}


static void dump_itf(ITF *itf)
{
    unsigned char *ipp,*nmp;
    char buf[MAX_ATM_QOS_LEN+1];

    ipp = (unsigned char *) &itf->local_ip;
    nmp = (unsigned char *) &itf->netmask;
    output("----- Itf %d (%d.%d.%d.%d, netmask %d.%d.%d.%d) -----",itf->number,
      ipp[0],ipp[1],ipp[2],ipp[3],nmp[0],nmp[1],nmp[2],nmp[3]);
    if (qos2text(buf,sizeof(buf),&itf->qos,0) < 0) strcpy(buf,"<error>");
    output("Default QOS: %s",buf);
    if (itf->sndbuf) output("Default send buffer: %d",itf->sndbuf);
    dump_entries(itf->table);
}


static void dump_all(void)
{
    ITF *itf;

    for (itf = itfs; itf; itf = itf->next) dump_itf(itf);
    output("----- Unknown incoming connections -----");
    dump_entries(unknown_incoming);
    output("----- Incoming unidirectional connections -----");
    dump_vccs(unidirectional_vccs);
    output("----- End of dump -----");
}


void table_changed(void)
{
    table_uptodate = 0;
    table_update(); /* @@@ sigh, fix this later */
    if (debug) {
	out_file = 0;
	dump_all();
    }
}


int table_update(void)
{
    if (table_uptodate) return 0;
    out_file = fopen(ATMARP_TMP_DUMP_FILE,"w");
    out_error = 0;
    dump_all();
    if (fclose(out_file) < 0) out_error = errno;
    if (!out_error) {
	if (rename(ATMARP_TMP_DUMP_FILE,ATMARP_DUMP_FILE) < 0)
	    out_error = errno;
	else table_uptodate = 1;
    }
    unlink(ATMARP_TMP_DUMP_FILE);
    return out_error;
}

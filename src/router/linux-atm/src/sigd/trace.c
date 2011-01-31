/* trace.c - Support functions for message tracing */
 
/* Written 1996-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "atm.h"
#include <linux/atmsvc.h>

#include "atmd.h"
#include "atmsap.h"
#include "trace.h"
#include "proto.h"


#define DUMP_MODE
#include "qlib.h"


typedef struct _entry {
    int number;
    struct timeval time;
    void (*print)(void *msg,int size);
    const char *comment;
    struct sockaddr_atmpvc pvc; /* unused if !sap_family */
    void *msg;
    int size;
    struct _entry *next;
} ENTRY;


int trace_size = DEFAULT_TRACE_SIZE;

static struct sockaddr_atmpvc null_pvc;
static int current_size = 0;
static int sequence = 0;
static ENTRY *first = NULL,*last = NULL;
static char *string = NULL;
static int curr_len;
static int new_line;


static inline void append_chunk(const char *str,int len)
{
    if (!string) curr_len = 0;
    if (!(string = realloc(string,curr_len+len+1))) {
	perror("realloc");
	exit(1);
    }
    memcpy(string+curr_len,str,len);
    curr_len += len;
    string[curr_len] = 0;
}


static void vappend(const char *fmt,va_list ap)
{
    const char *walk,*next;

    for (walk = next = fmt; *walk; walk++)
	if (*walk == '%') {
	    if (walk != next) append_chunk(next,walk-next);
	    if (*++walk == 's') {
		const char *str;

		str = va_arg(ap,const char *);
		append_chunk(str,strlen(str));
	    }
	    else {
		char buf[21]; /* big enough for 64 bits */
		int num;

		while (isdigit(*walk) || *walk == 'l') walk++; /* @@@ FIXME */
		if (*walk != 'd' && *walk != 'x') {
		    fprintf(stderr,"bad format character %c (%d)\n",*walk,
		      *walk);
		    exit(1);
		}
		num = va_arg(ap,int);
		sprintf(buf,*walk == 'd' ? "%d" : "%x",num);
		append_chunk(buf,strlen(buf));
	    }
	    next = walk+1;
	}
    if (walk != next) append_chunk(next,walk-next);
}


static void append(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    vappend(fmt,ap);
    va_end(ap);
}


static void print_text(void *msg,int size)
{
    append("  %s\n",msg);
}


static void append_svc(const struct sockaddr_atmsvc *svc)
{
    char buffer[MAX_ATM_ADDR_LEN+1];

    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) svc,
      A2T_NAME | A2T_LOCAL | A2T_PRETTY) < 0) strcpy(buffer,"<invalid>");
    append("%s\n",buffer);
}


static void append_sap(const struct atm_sap *sap)
{
    char buffer[MAX_ATM_SAP_LEN+1];

    if (sap2text(buffer,MAX_ATM_SAP_LEN+1,sap,S2T_NAME | S2T_LOCAL) < 0)
	strcpy(buffer,"<invalid>");
    append("%s\n",buffer);
}


static void append_qos(const struct atm_qos *qos)
{
    char buffer[MAX_ATM_QOS_LEN+1];

    if (qos2text(buffer,MAX_ATM_QOS_LEN+1,qos,0) < 0)
	strcpy(buffer,"<invalid>");
    append("%s\n",buffer);
}


static void qd_vdump(const char *msg,va_list ap)
{
    if (new_line) append("  ");
    vappend(msg,ap);
    if (string && curr_len) new_line = string[curr_len-1] == '\n';
    else new_line = 1;
}


void qd_dump(const char *msg,...)
{
    va_list ap;

    va_start(ap,msg);
    qd_vdump(msg,ap);
    va_end(ap);
}


void qd_report(int severity,const char *msg,...)
{
    va_list ap;

    if (severity > Q_ERROR) return;
    va_start(ap,msg);
    qd_vdump(msg,ap);
    va_end(ap);
    qd_dump("\n");
}


static void print_uni(void *msg,int size)
{
    Q_DSC dsc;

    (void) qd_open(&dsc,msg,size);
    qd_close(&dsc);
}


static void print_kernel(void *msg,int size)
{
    static const char *type[] = { "as_catch_null","as_bind","as_connect",
      "as_accept","as_reject","as_listen","as_okay","as_error","as_indicate",
      "as_close","as_itf_notify","as_modify","as_identify" };
    struct atmsvc_msg *m = msg;

    append("  %s (vcc %s, listen_vcc %s)\n",m->type < sizeof(type)/
      sizeof(*type) ? type[m->type] : "???",kptr_print(&m->vcc),
      kptr_print(&m->listen_vcc));
    append("  reply %d",m->reply);
    if (m->reply) {
	const char *error;

	error = strerror(m->reply > 0 ? m->reply : -m->reply);
	append(" (%s)",error ? error : "???");
    }
    append(", aal %d\n",m->qos.aal);
    append("  pvc %d.%d.%d\n",m->pvc.sap_addr.itf,m->pvc.sap_addr.vpi,
      m->pvc.sap_addr.vci);
    append("  local ");
    append_svc(&m->local);
    append("  qos ");
    append_qos(&m->qos);
    append("  svc ");
    append_svc(&m->svc);
    append("  sap ");
    append_sap(&m->sap);
}


static void store(void (*print)(void *msg,int size),const char *comment,
  const struct sockaddr_atmpvc *pvc,void *msg,int size)
{
    ENTRY *entry;

    entry = alloc_t(ENTRY);
    (void) gettimeofday(&entry->time,NULL);
    entry->number = sequence++;
    entry->print = print;
    entry->comment = comment;
    entry->pvc = *pvc;
    entry->msg = msg;
    entry->size = size;
    entry->next = NULL;
    if (current_size < trace_size) current_size++;
    else { /* handle trace_size < 1 (< 0) too */
	ENTRY *next;

	next = first->next;
	free(first->msg);
	free(first);
	if (first == last) last = NULL; /* someone set trace_size < 2 */
	first = next;
    }
    if (last) last->next = entry;
    else first = entry;
    last = entry;
}


void trace_msg(const char *msg)
{
    char *buf;

    if (!trace_size) return;
    buf = alloc(strlen(msg)+1);
    strcpy(buf,msg);
    store(&print_text,"MESSAGE",&null_pvc,buf,strlen(msg));
}


void trace_uni(const char *comment,const SIG_ENTITY *sig,const void *msg,
  int size)
{
    char *buf;

    if (!trace_size) return;
    buf = alloc(size);
    memcpy(buf,msg,size);
    store(&print_uni,comment,&sig->signaling_pvc,buf,size);
}


void trace_kernel(const char *comment,const struct atmsvc_msg *msg)
{
    struct atmsvc_msg *buf;

    if (!trace_size) return;
    buf = alloc_t(struct atmsvc_msg);
    *buf = *msg;
    store(&print_kernel,comment,&null_pvc,buf,sizeof(*msg));
}


char *get_trace(void)
{
    ENTRY *walk;

    if (string) {
	free(string);
	string = NULL;
    }
    for (walk = first; walk; walk = walk->next) {
	append("%6d (%d.%06d) %s",walk->number,walk->time.tv_sec,
	  walk->time.tv_usec,walk->comment);
	if (!walk->pvc.sap_family) append(":\n");
	else append(" (%d.%d.%d):\n",walk->pvc.sap_addr.itf,
	      walk->pvc.sap_addr.vpi,walk->pvc.sap_addr.vci);
	new_line = 1;
	walk->print(walk->msg,walk->size);
    }
    return string;
}

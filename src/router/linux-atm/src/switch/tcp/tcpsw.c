/* tcpsw.c - ATMTCP switch */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <atm.h>
#include <linux/atm_tcp.h>
#include <atmd.h>

#include "uni.h"
#include "../fab.h"
#include "../dispatch.h"
#include "../swc.h"


#define COMPONENT "FAB(tcp)"

#define PRV(call) ((FAB *) (call)->fab)

#define MAX_VCI	1024

#define PORT 2812		/* @@@ should merge with atmtcp.c */
#define MAX_PACKET (ATM_MAX_AAL5_PDU+sizeof(struct atmtcp_hdr))
#define BUFFER_SIZE (MAX_PACKET*2)

int yywrap(void)
{
        return 1;
}

typedef struct _table {
    struct _link *out;	/* output port */
    uint16_t in_vpi;	/* input VPI */
    int	in_vci;		/* input VCI (may be ATM_VCI_ANY) */
    uint16_t out_vpi;	/* output VPI */
    uint16_t out_vci;	/* output VCI */
    struct _table *next;
} TABLE;

typedef struct _link {
    enum { id_none,id_set,id_reval } state;
    int id;		/* switch port ID */
    int fd;
    int len;		/* length of data in the buffer */
    char buffer[BUFFER_SIZE];
    TABLE *table;	/* switching table */
    struct _link *next;
} LINK;

typedef struct _fab {
    int active; /* non-zero if there's an entry in the fabric */
    CALL *next; /* relay.c may not keep track of calls, but WE are */
} FAB;


static CALL *calls = NULL;
static LINK *links = NULL;
static void (*notify)(int number,int up);


static LINK *find_link(int id)
{
    LINK *lnk;

    for (lnk = links; lnk; lnk = lnk->next)
	if (lnk->state == id_set && lnk->id == id) break;
    return lnk;
}


static LINK *route(LINK *in,struct atmtcp_hdr *hdr)
{
    TABLE *entry;

    for (entry = in->table; entry; entry = entry->next)
	if (ntohs(hdr->vpi) == entry->in_vpi &&
	  (entry->in_vci == ATM_VCI_UNSPEC ||
	  ntohs(hdr->vci) == entry->in_vci)) {
	    hdr->vpi = htons(entry->out_vpi);
	    if (entry->in_vci != ATM_VCI_UNSPEC)
		hdr->vci = htons(entry->out_vci);
	    return entry->out;
	}
    return NULL;
}


static int add_entry(struct sockaddr_atmpvc *from,struct sockaddr_atmpvc *to)
{
    LINK *l_from,*l_to;
    TABLE *entry;

    l_from = find_link(from->sap_addr.itf);
    l_to = find_link(to->sap_addr.itf);
    if (!l_from || !l_to) return -ENODEV;
    for (entry = l_from->table; entry; entry = entry->next)
	if (entry->in_vpi == from->sap_addr.vpi &&
	  (entry->in_vci == ATM_VCI_UNSPEC ||
	  from->sap_addr.vci == ATM_VCI_UNSPEC ||
	  entry->in_vci == from->sap_addr.vci)) return -EEXIST;
    entry = alloc_t(TABLE);
    entry->out = l_to;
    entry->in_vpi = from->sap_addr.vpi;
    entry->in_vci = from->sap_addr.vci;
    entry->out_vpi = to->sap_addr.vpi;
    entry->out_vci = to->sap_addr.vci;
    entry->next = l_from->table;
    l_from->table = entry;
    if (entry->in_vci == ATM_VCI_UNSPEC)
	diag(COMPONENT,DIAG_INFO,"added VP %d.%d -> %d.%d",l_from->id,
	  entry->in_vpi,l_to->id,entry->out_vpi);
    else diag(COMPONENT,DIAG_INFO,"added VC %d.%d.%d -> %d.%d.%d",l_from->id,
	  entry->in_vpi,entry->in_vci,l_to->id,entry->out_vpi,entry->out_vci);
    return 0;
}


static int del_entry(struct sockaddr_atmpvc *from,struct sockaddr_atmpvc *to)
{
    LINK *l_from,*l_to;
    TABLE **entry,*this;

    l_from = find_link(from->sap_addr.itf);
    l_to = find_link(to->sap_addr.itf);
    if (!l_from || !l_to) return -ENODEV;
    for (entry = &l_from->table; *entry; entry = &(*entry)->next)
	if ((*entry)->out == l_to && (*entry)->in_vpi == from->sap_addr.vpi &&
	  (*entry)->in_vci == from->sap_addr.vci &&
	  ((*entry)->in_vci == ATM_VCI_UNSPEC ||
	  (*entry)->out_vci == to->sap_addr.vci))
	    break;
    if (!*entry) return -EHOSTUNREACH;
    this = *entry;
    *entry = this->next;
    if (this->in_vci == ATM_VCI_UNSPEC)
	diag(COMPONENT,DIAG_INFO,"deleted VP %d.%d -> %d.%d",l_from->id,
	  this->in_vpi,l_to->id,this->out_vpi);
    else diag(COMPONENT,DIAG_INFO,"deleted VC %d.%d.%d -> %d.%d.%d",l_from->id,
	  this->in_vpi,this->in_vci,l_to->id,this->out_vpi,this->out_vci);
    free(this);
    return 0;
}


static void drop_link(LINK *lnk)
{
    diag(COMPONENT,DIAG_INFO,"dropped link %d",lnk->id);
    if (lnk->state == id_set) {
	notify(lnk->id,0);
	lnk->state = id_reval;
    }
    dsp_fd_remove(lnk->fd);
#if 0
    LINK **walk,**next_link,**entry,**next_entry;

    /* remove link from the list and delete all references to link */
    for (walk = &links; *walk; walk = next_link) {
	next_link = &(*walk)->next;
	if (*walk == lnk) *walk = *next_link;
	else for (entry = &(*walk)->table; *entry; entry = next_entry) {
		next_entry = &(*entry)->next;
		if ((*entry)->out == lnk) {
		    TABLE *this;

		    this = *entry;
		    *entry = *next_entry;
		    free(this);
		}
	    }
    }
    *walk = lnk->next;
    /* remove link's switching table */
    while (lnk->table) {
	TABLE *next;

	next = lnk->table->next;
	free(lnk->table);
	lnk->table = next;
    }
    /* drop memory */
    free(*lnk);
#endif
}


static void new_data(int fd,void *user)
{
    LINK *lnk = user;
    int got;

    got = read(fd,lnk->buffer+lnk->len,BUFFER_SIZE-lnk->len);
    if (got < 0) perror("read");
    if (got <= 0) drop_link(lnk);
    lnk->len += got;
    if (lnk->state != id_set) {
	LINK *walk;
	char *end;
	int id;

	if (!memchr(lnk->buffer,0,lnk->len)) return;
	id = strtol(lnk->buffer,&end,0);
	if (*end) {
	    diag(COMPONENT,DIAG_ERROR,"invalid line id \"%s\"",lnk->buffer);
	    drop_link(lnk);
	    return;
	}
	for (walk = links; walk; walk = walk->next)
	    if (walk->state != id_none && id == walk->id) break;
	if (walk) {
	    if (walk->state == id_set) {
		diag(COMPONENT,DIAG_ERROR,"line state mismatch (id %d)",id);
		drop_link(lnk);
		return;
	    }
	    diag(COMPONENT,DIAG_INFO,"reconnected link %d",id);
	    walk->state = id_set;
	    walk->fd = lnk->fd;
	    walk->len = lnk->len;
	    memcpy(walk->buffer,lnk->buffer,lnk->len);
	    drop_link(lnk);
	    lnk = walk;
	    dsp_fd_add(lnk->fd,new_data,walk);
	}
	else {
	    diag(COMPONENT,DIAG_INFO,"new link %d",id);
	    lnk->state = id_set;
	    lnk->id = id;
	}
	notify(lnk->id,1);
	lnk->len -= strlen(lnk->buffer)+1;
	memmove(lnk->buffer,lnk->buffer-strlen(lnk->buffer)-1,lnk->len);
    }
    while (lnk->len >= sizeof(struct atmtcp_hdr)) {
	struct atmtcp_hdr *hdr = (struct atmtcp_hdr *) lnk->buffer;
	LINK *out;
	int size;

	size = sizeof(struct atmtcp_hdr)+ntohl(hdr->length);
	if (lnk->len < size) break;
	out = route(lnk,hdr);
	if (out) {
	    int sent;

	    sent = write(out->fd,lnk->buffer,size);
	    if (sent < 0) perror("write");
	    else if (sent != size)
		    diag(COMPONENT,DIAG_ERROR,"bad write: %d != %d",sent,size);
	    if (sent != size) drop_link(out);
	}
	lnk->len -= size;
	memmove(lnk->buffer,lnk->buffer+size,lnk->len);
    }
}


static void new_link(int sock,void *dummy)
{
    LINK *lnk;
    int fd;

    fd = accept(sock,NULL,NULL);
    if (fd < 0) {
	perror("accept");
	return;
    }
    lnk = alloc_t(LINK);
    lnk->state = id_none;
    lnk->id = -1;
    lnk->fd = fd;
    lnk->len = 0;
    lnk->table = NULL;
    lnk->next = links;
    links = lnk;
    dsp_fd_add(fd,new_data,lnk);
}


static int vci_exists(LINK *lnk,int vpi,int vci)
{
    TABLE *walk;

    for (walk = lnk->table; walk; walk = walk->next)
	if (walk->in_vpi == vpi && walk->in_vci == vci) return 1;
    return 0;
}


static int check_ci(struct sockaddr_atmpvc *pvc)
{
    LINK *lnk;
    TABLE *walk;
    int vci;

    lnk = find_link(pvc->sap_addr.itf);
    if (!lnk) return 0;
    if (pvc->sap_addr.vpi == ATM_VPI_ANY) pvc->sap_addr.vpi = 0;
	/* that was easy :-) */
    for (walk = lnk->table; walk; walk = walk->next)
	if (walk->in_vpi == pvc->sap_addr.vpi) break;
    if (walk && walk->in_vci == ATM_VCI_UNSPEC) return 0;
    if (pvc->sap_addr.vci == ATM_VCI_UNSPEC) return !walk;
    if (pvc->sap_addr.vci != ATM_VCI_ANY)
	return !vci_exists(lnk,pvc->sap_addr.vpi,pvc->sap_addr.vci);
    for (vci = ATM_NOT_RSV_VCI; vci < MAX_VCI; vci++)
	if (!vci_exists(lnk,pvc->sap_addr.vpi,vci)) {
	    pvc->sap_addr.vci = vci;
	    return 1;
	}
    return 0;
}


void fab_option(const char *name,const char *value)
{
    diag(COMPONENT,DIAG_FATAL,"unrecognized fabric option \"%s\"",name);
}


void fab_start(void (*port_notify)(int number,int up))
{
    struct sockaddr_in addr;
    int s_listen;

    notify = port_notify;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if ((s_listen = socket(PF_INET,SOCK_STREAM,0)) < 0)
	diag(COMPONENT,DIAG_FATAL,"socket: %s",strerror(errno));
    if (bind(s_listen,(struct sockaddr *) &addr,sizeof(addr)) < 0)
	diag(COMPONENT,DIAG_FATAL,"bind: %s",strerror(errno));
    if (listen(s_listen,5) < 0)
	diag(COMPONENT,DIAG_FATAL,"listen: %s",strerror(errno));
    dsp_fd_add(s_listen,new_link,NULL);
}


void fab_init(CALL *call)
{
    FAB *fab;
    call->fab = alloc_t(FAB);
    fab = PRV(call);
    fab->active = 0;
    fab->next = calls;
    calls = call;
}


void fab_destroy(CALL *call)
{
    CALL **walk;

    for (walk = &calls; *walk; walk = &PRV(*walk)->next)
	if (*walk == call) break;
    if (!*walk)
	diag(COMPONENT,DIAG_FATAL,"fab_destroy: call %p not found",call);
    *walk = PRV(call)->next;
    free(PRV(call));
    call->fab = NULL;
}


void fab_op(CALL *call,int op,const struct atm_qos *qos,
  void (*callback)(CALL *call,int cause,void *more,void *user),void *user)
{
    int error,error2;

    diag(COMPONENT,DIAG_INFO,"fab_op%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      !op ? " FREE" : "",op & RM_RSV(_RM_ANY) ? " RM_RSV:" : "",
      op & RM_IN_TX ? " IN_TX" : "",op & RM_IN_RX ? " IN_RX" : "",
      op & RM_OUT_TX ? " OUT_TX" : "",op & RM_OUT_RX ? " OUT_RX" : "",
      op & RM_PATH_TX ? " PATH_TX" : "",op & RM_PATH_RX ? " PATH_RX" : "",
      op & RM_CLAIM(_RM_ANY) ? " RM_CLAIM:" : "",
      op & _RM_SHIFT(RM_IN_TX) ? " IN_TX" : "",
      op & _RM_SHIFT(RM_IN_RX) ? " IN_RX" : "",
      op & _RM_SHIFT(RM_OUT_TX) ? " OUT_TX" : "",
      op & _RM_SHIFT(RM_OUT_RX) ? " OUT_RX" : "",
      op & _RM_SHIFT(RM_PATH_TX) ? " PATH_TX" : "",
      op & _RM_SHIFT(RM_PATH_RX) ? " PATH_RX" : "");
    if (op & (RM_RSV(RM_IN) | RM_CLAIM(RM_IN))) {
	if (!check_ci(&call->in.pvc)) {
	    callback(call,ATM_CV_CI_UNAVAIL,NULL,user);
	    return;
	}
	call->in.qos = *qos;
    }
    if (op & (RM_RSV(RM_OUT) | RM_CLAIM(RM_OUT))) {
	if (!check_ci(&call->out.pvc)) {
	    callback(call,ATM_CV_CI_UNAVAIL,NULL,user);
	    return;
	}
	call->out.qos = *qos;
    }
    if (op & RM_CLAIM(RM_PATH)) {
	error = 0;
	if (call->in.qos.txtp.traffic_class != ATM_NONE)
	    error = add_entry(&call->in.pvc,&call->out.pvc);
	if (!error) {
	    if (call->in.qos.rxtp.traffic_class != ATM_NONE)
		error = add_entry(&call->out.pvc,&call->in.pvc);
	    if (error && call->in.qos.txtp.traffic_class != ATM_NONE) {
		error2 = del_entry(&call->in.pvc,&call->out.pvc);
		if (error2)
		    diag(COMPONENT,DIAG_ERROR,"del_entry: %s",strerror(error2));
	    }
	}
	if (error) {
	    diag(COMPONENT,DIAG_ERROR,"add_entry: %s",strerror(error));
	    callback(call,ATM_CV_RES_UNAVAIL,NULL,user);
	    return;
	}
	PRV(call)->active = 1;
    }
    if (!op && PRV(call)->active) {
	error = error2 = 0;
	if (call->in.qos.txtp.traffic_class != ATM_NONE) {
	    error = del_entry(&call->in.pvc,&call->out.pvc);
	    if (error)
		diag(COMPONENT,DIAG_ERROR,"del_entry: %s",strerror(error));
	}
	if (call->in.qos.rxtp.traffic_class != ATM_NONE) {
	    error2 = del_entry(&call->out.pvc,&call->in.pvc);
	    if (error2)
		diag(COMPONENT,DIAG_ERROR,"del_entry: %s",strerror(error2));
	}
	if (error || error2) {
	    callback(call,ATM_CV_TEMP_FAIL,NULL,user);
	    return;
	}
    }
    callback(call,0,NULL,user);
}

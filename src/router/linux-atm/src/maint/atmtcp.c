/* atmtcp.c - control ATM on TCP emulation */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <atm.h>
#include <atmd.h>
#include <linux/atm_tcp.h>


#define DEFAULT_PORT	2812	/* IANA-assigned; old ATMTCP used 8401 & 8402 */
#define MAX_PACKET (ATM_MAX_AAL5_PDU+sizeof(struct atmtcp_control)+ \
  sizeof(int))


typedef struct _in {
    int fd;
    void (*recv)(struct _in *in);
    int link;
    unsigned char buf[MAX_PACKET];
    int bytes;
    void *user;
    struct _in *next;
} IN;


typedef struct _out {
    const struct _out_ops *ops;
    int link;
    void *user;
    struct _out *next;
} OUT;


typedef struct _out_ops {
    int (*open)(struct _out *out,int in_link,struct atmtcp_control *msg);
    void (*send)(struct _out *out,int in_link,const struct atmtcp_hdr *hdr,
      int size);
    int (*close)(struct _out *out,int in_link,struct atmtcp_control *msg);
} OUT_OPS;


static OUT *outputs = NULL;
static IN *inputs = NULL;
static fd_set in_set;
static int fds = 0;
int debug = 0;
static int links = 0;


/* misc. */


/*
 * Policy:
 *  - first link sends to everybody except itself
 *  - second link sends to everybody except itself
 *  - all other links send to everybody except the first link the sender itself
 */


static int right_link(const OUT *out,int in_link)
{
    if (out->link == in_link) return 0;
    return out != outputs || in_link == 1;
}


static void emit(int in_link,const struct atmtcp_hdr *hdr,int size)
{
    OUT *out;

    if (debug)
	fprintf(stderr,"Emit: %d.%d, %d bytes\n",ntohs(hdr->vpi),
	  ntohs(hdr->vci),(int) ntohl(hdr->length));
    for (out = outputs; out; out = out->next)
	if (out->ops->send) out->ops->send(out,in_link,hdr,size);
}


static void control(int in_link,struct atmtcp_control *msg)
{
    OUT *out;
    int changed = 0;

    if (debug)
	fprintf(stderr,"Control: (%d.%d) %s %d.%d, vcc %s\n",
	  ntohs(msg->hdr.vpi),ntohs(msg->hdr.vci),
	  msg->type == ATMTCP_CTRL_OPEN ? "OPEN" :
	  msg->type == ATMTCP_CTRL_CLOSE ? "CLOSE" : "???",
	  msg->addr.sap_addr.vpi,msg->addr.sap_addr.vci,kptr_print(&msg->vcc));
    for (out = outputs; out; out = out->next)
	switch (msg->type) {
	    case ATMTCP_CTRL_OPEN:
		if (out->ops->open)
		    changed += out->ops->open(out,in_link,msg);
		break;
	    case ATMTCP_CTRL_CLOSE:
		if (out->ops->close)
		    changed += out->ops->close(out,in_link,msg);
		break;
	    default:
		fprintf(stderr,"interval error\n");
		exit(1);
	}
    if (changed > 1) fprintf(stderr,"WARNING: multiple changes on control()\n");
}


/* ----- Registries -------------------------------------------------------- */


static IN *register_in(int fd,void (*receive)(IN *in),void *user)
{
    IN *in = alloc_t(IN); 
    IN **walk;

    in->fd = fd;
    in->recv = receive;
    in->link = links;
    in->bytes = 0;
    in->user = user;
    in->next = NULL;
    for (walk = &inputs; *walk; walk = &(*walk)->next);
    *walk = in;
    if (fd >= fds) fds = fd+1;
    FD_SET(fd,&in_set);
    return in;
}


static void unregister_in(IN *in)
{
    IN **walk;

    for (walk = &inputs; *walk != in; walk = &(*walk)->next);
    FD_CLR(in->fd,&in_set);
    if (fds > in->fd+1) return;
    fds = 0;
    for (walk = &inputs; *walk; walk = &(*walk)->next)
	if ((*walk)->fd >= fds) fds = (*walk)->fd+1;
}


static void register_out(const OUT_OPS *ops,void *user)
{
    OUT *out = alloc_t(OUT); 
    OUT **walk;

    out->ops = ops;
    out->link = links;
    out->user = user;
    out->next = NULL;
    for (walk = &outputs; *walk; walk = &(*walk)->next);
    *walk = out;
}


/* ----- Virtual (ATMTCP) interface ---------------------------------------- */


static void virtual_do_send(int fd,const void *data,int size)
{
    int wrote;

    wrote = write(fd,data,size);
    if (wrote < 0) perror("write to kernel");
    else if (wrote != size) {
            fprintf(stderr,"bad write (%d != %d)\n",wrote,size);
            exit(1);
        }
}


static void virtual_in(IN *in)
{
    const struct atmtcp_hdr *hdr = (struct atmtcp_hdr *) in->buf;
    int got;

    got = read(in->fd,in->buf,MAX_PACKET);
    if (got < 0) {
	perror("virtual interface");
	exit(1);
    }
    if (!got) exit(0); /* we don't use that yet */
    if (got < sizeof(struct atmtcp_hdr)) {
	fprintf(stderr,"kernel message too small (%d)\n",got);
	exit(1);
    }
    if (ntohl(hdr->length) == ATMTCP_HDR_MAGIC) {
	if (got < sizeof(struct atmtcp_control)) {
	    fprintf(stderr,"invalid control message\n");
	    exit(1);
	}
	control(in->link,(struct atmtcp_control *) in->buf);
	virtual_do_send(in->fd,hdr,sizeof(struct atmtcp_control));
	return;
    }
    if (got != sizeof(struct atmtcp_hdr)+ntohl(hdr->length)) {
	fprintf(stderr,"invalid kernel message\n");
	exit(1);
    }
    emit(in->link,hdr,got);
}


static void virtual_send(OUT *out,int in_link,const struct atmtcp_hdr *hdr,
  int size)
{
    if (!right_link(out,in_link)) return;
    virtual_do_send(*(int *) out->user,hdr,size);
}


static OUT_OPS virtual_ops = {
    NULL,		/* open */
    virtual_send,	/* send */
    NULL		/* close */
};


/* ----- Real ATM interface ------------------------------------------------ */


typedef struct _vcc {
    struct sockaddr_atmpvc addr;
    int fd; /* for output */
    IN *in; /* NULL is output only */
    struct _vcc *next;
} VCC;


typedef struct {
    int itf;
    VCC *vccs;
} REAL_DATA;


static VCC **real_lookup(REAL_DATA *data,const struct sockaddr_atmpvc *addr)
{
    VCC **walk;

    for (walk = &data->vccs; *walk; walk = &(*walk)->next)
	if ((*walk)->addr.sap_addr.vpi == addr->sap_addr.vpi &&
	  (*walk)->addr.sap_addr.vci == addr->sap_addr.vci) break;
    return walk;
}


static void real_in(IN *in)
{
    VCC *vcc = (VCC *) in->user;
    struct atmtcp_hdr *hdr = (struct atmtcp_hdr *) in->buf;
    int got;

    got = read(in->fd,hdr+1,MAX_PACKET-sizeof(*hdr));
    if (got < 0) {
	perror("real interface");
	exit(1);
    }
    hdr->length = htonl(got);
    hdr->vpi = htons(vcc->addr.sap_addr.vpi);
    hdr->vci = htons(vcc->addr.sap_addr.vci);
    emit(in->link,hdr,got+sizeof(*hdr));
}


static int real_open(OUT *out,int in_link,struct atmtcp_control *msg)
{
    REAL_DATA *data = (REAL_DATA *) out->user;
    VCC **vcc;
    int s;

    if (!right_link(out,in_link)) return 0;
    vcc = real_lookup(data,&msg->addr);
    if (*vcc) {
	msg->result = -EADDRINUSE;
	return 1;
    }
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	msg->result = -errno;
	if (debug) perror("socket");
	return 1;
    }
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&msg->qos,sizeof(msg->qos)) < 0) {
	msg->result = -errno;
	if (debug) perror("setsockopt SO_ATMQOS");
	return 1;
    }
    msg->addr.sap_addr.itf = data->itf;
    if (connect(s,(struct sockaddr *) &msg->addr,
      sizeof(struct sockaddr_atmpvc)) < 0) {
	msg->result = -errno;
	if (debug) perror("connect");
	return 1;
    }
    (*vcc) = alloc_t(VCC);
    (*vcc)->addr = msg->addr;
    (*vcc)->next = NULL;
    (*vcc)->fd = s;
    (*vcc)->in = msg->qos.rxtp.traffic_class == ATM_NONE ? NULL :
      register_in(s,real_in,*vcc);
    return 1;
}


static void real_send(OUT *out,int in_link,const struct atmtcp_hdr *hdr,
  int size)
{
    REAL_DATA *data = (REAL_DATA *) out->user;
    struct sockaddr_atmpvc addr;
    VCC **vcc;
    int wrote;

    if (!right_link(out,in_link)) return;
    addr.sap_addr.vpi = ntohs(hdr->vpi);
    addr.sap_addr.vci = ntohs(hdr->vci);
    vcc = real_lookup(data,&addr);
    if (!*vcc) {
	if (debug)
	    fprintf(stderr,"VCC %d.%d not found\n",addr.sap_addr.vpi,
	      addr.sap_addr.vci);
	return;
    }
    wrote = write((*vcc)->fd,hdr+1,ntohl(hdr->length));
    if (wrote < 0) {
	perror("real write");
        exit(1);
    }
    if (!wrote) exit(0); /* EOF */
    if (wrote != ntohl(hdr->length)) {
        fprintf(stderr,"bad write (%d != %d)\n",wrote,(int) ntohl(hdr->length));
        exit(1);
    }
}


static int real_close(OUT *out,int in_link,struct atmtcp_control *msg)
{
    REAL_DATA *data = (REAL_DATA *) out->user;
    VCC **vcc,*this;
    int fd;

    if (!right_link(out,in_link)) return 0;
    vcc = real_lookup(data,&msg->addr);
    if (!*vcc) {
	msg->result = -ENOENT;
	return 1;
    }
    this = *vcc;
    *vcc = this->next;
    if (this->in) unregister_in(this->in);
    fd = this->fd;
    free(this);
    if (close(fd) >= 0) return 0;
    msg->result = -errno;
    return 1;
}


static OUT_OPS real_ops = {
    real_open,		/* open */
    real_send,		/* send */
    real_close		/* close */
};


/* ----- ATMTCP connection ------------------------------------------------- */


static void tcp_do_send(int fd,const void *data,int size)
{
    int wrote;

    wrote = write(fd,data,size);
    if (wrote < 0) {
	perror("write to TCP");
	exit(1);
    }
    if (!wrote) exit(0); /* EOF */
    if (wrote != size) {
	fprintf(stderr,"bad write (%d != %d)\n",wrote,size);
	exit(1);
    }
}


static void tcp_in(IN *in)
{
    struct atmtcp_hdr *hdr = (struct atmtcp_hdr *) in->buf;
    char *msg = (char *) (hdr+1);
    int vpi,vci;
    char orig_addr[MAX_ATM_ADDR_LEN];
    char qos[MAX_ATM_QOS_LEN];
    struct atmtcp_control ctl;
    int size,got;

    size = sizeof(*hdr)-in->bytes;
    if (size <= 0) size += ntohl(hdr->length);
    got = read(in->fd,in->buf+in->bytes,size);
    if (got < 0) {
        perror("read from file");
        exit(1);
    }
    if (!got) {
	fprintf(stderr,"TCP disconnected\n");
	exit(0);
    }
    in->bytes += got;
    if (in->bytes < sizeof(*hdr)) return;
    if (ntohl(hdr->length) > ATM_MAX_AAL5_PDU) {
	fprintf(stderr,"giant PDU (length = %d) received\n",
	  (unsigned int) ntohl(hdr->length));
	exit(1);
    }
    if (in->bytes < sizeof(*hdr)+ntohl(hdr->length)) return;
    if (debug)
	fprintf(stderr,"TCP %d.%d, %d bytes\n",ntohs(hdr->vpi),
	  ntohs(hdr->vci),(unsigned int) ntohl(hdr->length));
    in->bytes = 0;
    if (hdr->vpi || hdr->vci) {
	emit(in->link,hdr,sizeof(*hdr)+ntohl(hdr->length));
    	return;
    }
    msg[ntohl(hdr->length)] = 0;
    memset(&ctl,0,sizeof(ctl));
    if (sscanf(msg,"O %d.%d %s %s",&vpi,&vci,orig_addr,qos) == 4)
	ctl.type = ATMTCP_CTRL_OPEN;
    else if (sscanf(msg,"C %d.%d",&vpi,&vci) == 2) ctl.type = ATMTCP_CTRL_CLOSE;
    else {
	fprintf(stderr,"unrecognized control message \"%s\"\n",msg);
	return;
    }
    if (debug) fprintf(stderr,"received control \"%s\"\n",msg);
    ctl.hdr.vpi = htons(vpi);
    ctl.hdr.vci = htons(vci);
    ctl.hdr.length = htonl(ATMTCP_HDR_MAGIC);
    ctl.addr.sap_family = AF_ATMPVC;
    ctl.addr.sap_addr.itf = 0;
    ctl.addr.sap_addr.vpi = vpi;
    ctl.addr.sap_addr.vci = vci;
    if (ctl.type == ATMTCP_CTRL_OPEN) {
	if (text2atm(orig_addr,(struct sockaddr *) &ctl.addr,sizeof(ctl.addr),
	  T2A_PVC) < 0) {
	    fprintf(stderr,"invalid address \"%s\"\n",orig_addr);
	    return;
	}
	if (text2qos(qos,&ctl.qos,0) < 0) {
	    fprintf(stderr,"invalid QOS \"%s\"\n",qos);
	    return;
	}
    }
    control(in->link,&ctl);
}


static void tcp_send(OUT *out,int in_link,const struct atmtcp_hdr *hdr,int size)
{
    if (!right_link(out,in_link)) return;
    tcp_do_send(*(int *) out->user,hdr,size);
}


static int tcp_control(OUT *out,int in_link,struct atmtcp_control *msg)
{
    char buf[MAX_ATM_NAME_LEN*2+MAX_ATM_NAME_LEN+5];
    struct atmtcp_hdr *hdr = (struct atmtcp_hdr *) buf;
    char *start = (char *) (hdr+1);
    char *pos = start;

    if (!right_link(out,in_link)) return 0;
    if (msg->type != ATMTCP_CTRL_OPEN && msg->type != ATMTCP_CTRL_CLOSE) {
	fprintf(stderr,"unrecognized control message %d\n",msg->type);
	return 0;
    }
    pos += sprintf(pos,"%c %d.%d",msg->type == ATMTCP_CTRL_OPEN ? 'O' : 'C',
      msg->addr.sap_addr.vpi,msg->addr.sap_addr.vci);
    if (msg->type == ATMTCP_CTRL_OPEN) {
	*pos++ = ' ';
	if (atm2text(pos,sizeof(buf)-(pos-buf),(struct sockaddr *) &msg->addr,
	  0) < 0) {
	    fprintf(stderr,"invalid ATM address\n");
	    return 0;
	}
	pos = strchr(pos,0);
	*pos++ = ' ';
	if (qos2text(pos,sizeof(buf)-(pos-buf),&msg->qos,0) < 0) {
	    fprintf(stderr,"invalid QOS\n");
	    return 0;
	}
	pos = strchr(pos,0);
    }
    hdr->vpi = hdr->vci = htons(0);
    hdr->length = htonl(pos-start);
    if (debug) fprintf(stderr,"sending control \"%s\"\n",start);
    tcp_do_send(*(int *) out->user,buf,pos-buf);
    return 0;
}


static OUT_OPS tcp_ops = {
    tcp_control,	/* open */
    tcp_send,		/* send */
    tcp_control		/* close */
};


/* ----- File -------------------------------------------------------------- */


static void file_in(IN *in)
{
    int *stream = (int *) in->buf;
    struct atmtcp_hdr *hdr = (struct atmtcp_hdr *) (stream+1);
    int size,got;

    size = sizeof(int)+sizeof(*hdr)-in->bytes;
    if (size <= 0)
	size += ntohl(hdr->length) == ATMTCP_HDR_MAGIC ?
	  sizeof(struct atmtcp_control)-sizeof(*hdr) : ntohl(hdr->length);
    got = read(in->fd,in->buf+in->bytes,size);
    if (got < 0) {
        perror("read from file");
        exit(1);
    }
    if (!got) {
	fprintf(stderr,"EOF\n");
	exit(0);
    }
    in->bytes += got;
    if (in->bytes < sizeof(int)+sizeof(*hdr)) return;
    if (ntohl(hdr->length) == ATMTCP_HDR_MAGIC) {
	if (in->bytes < sizeof(int)+sizeof(struct atmtcp_control)) return;
    }
    else {
	if (ntohl(hdr->length) > ATM_MAX_AAL5_PDU) {
	    fprintf(stderr,"giant PDU (length = %d) received\n",
	      (unsigned int) ntohl(hdr->length));
	    exit(1);
	}
	if (in->bytes < sizeof(int)+sizeof(*hdr)+ntohl(hdr->length)) return;
	if (debug)
	    fprintf(stderr,"File %d.%d, %d bytes\n",ntohs(hdr->vpi),
	      ntohs(hdr->vci),(unsigned int) ntohl(hdr->length));
    }
    in->bytes = 0;
    if (*(int *) in->user != -1 && ntohl(*stream) != *(int *) in->user) return;
    if (ntohl(hdr->length) == ATMTCP_HDR_MAGIC)
	control(in->link,(struct atmtcp_control *) hdr);
    else emit(in->link,hdr,sizeof(*hdr)+ntohl(hdr->length));
}


static void file_write(int fd,int stream,int is_control,const void *data,
  int size)
{
    int wrote;

    stream = htonl(stream);
    wrote = write(fd,&stream,sizeof(stream));
    if (wrote < 0) {
	perror("file write");
	exit(1);
    }
    if (wrote != sizeof(stream)) {
	fprintf(stderr,"short write (%d < %d)\n",wrote,sizeof(stream));
	exit(1);
    }
    wrote = write(fd,data,size);
    if (wrote < 0) {
	perror("file write");
	exit(1);
    }
    if (wrote != size) {
	fprintf(stderr,"short write (%d < %d)\n",wrote,size);
	exit(1);
    }
}


static void file_send(OUT *out,int in_link,const struct atmtcp_hdr *hdr,
  int size)
{
    file_write(*(int *) out->user,in_link,0,hdr,size);
}


static int file_control(OUT *out,int in_link,struct atmtcp_control *msg)
{
    file_write(*(int *) out->user,in_link,1,msg,sizeof(*msg));
    return 0;
}


static OUT_OPS file_ops = {
    file_control,	/* open */
    file_send,		/* send */
    file_control	/* close */
};


/* ----- Print ------------------------------------------------------------- */


static void print_send(OUT *out,int in_link,const struct atmtcp_hdr *hdr,
  int size)
{
    int length = ntohl(hdr->length);
    int i;

    printf("Link %d (from link %d), VPI %d, VCI %d, %d byte%s:",out->link,
      in_link,ntohs(hdr->vpi),ntohs(hdr->vci),length,length == 1 ? "" : "s");
    for (i = 0; i < length; i++) {
	if (!(i & 15)) printf("\n ");
	printf(" %02x",((unsigned char *) (hdr+1))[i]);
    }
    putchar('\n');
    fflush(stdout);
}


static OUT_OPS print_ops = {
    NULL,		/* open */
    print_send,		/* send */
    NULL		/* close */
};


/* ----- Initialization ---------------------------------------------------- */


static void background(void)
{
    static int backgrounding = 0;
    pid_t pid;

    if (backgrounding++) {
	fprintf(stderr,"\"bg\" only allowed once\n");
	exit(1);
    }
    pid = fork();
    if (pid < 0) {
	perror("fork");
	exit(1);
    }
    if (pid) exit(0);
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -d ] [ -v ] <cmd> ...\n",name);
    fprintf(stderr,"%6s %s -V\n\n","",name);
    fprintf(stderr,"  -d  debug\n");
    fprintf(stderr,"  -v  verbose\n");
    fprintf(stderr,"  -V  version\n\n");
    fprintf(stderr,"<cmd>: create [ <itf> ]  (persistent; default itf: 0)\n");
    fprintf(stderr,"       remove [ <itf> ]  (persistent; default itf: 0)\n");
    fprintf(stderr,"       virtual [ <itf> ]  (default itf: assigned by "
      "kernel)\n");
    fprintf(stderr,"       real [ <itf> ]  (default itf: 0)\n");
    fprintf(stderr,"       connect <host> [ <port> ]\n");
    fprintf(stderr,"       switch <host> <line> [ <port> ]  (to ATMTCP "
      "virtual switch)\n");
    fprintf(stderr,"       listen [ <port> ]\n");
    fprintf(stderr,"       read <file> [ <stream> ]\n");
    fprintf(stderr,"       write <file>\n");
    fprintf(stderr,"       print\n");
    fprintf(stderr,"       bg  (background)\n");
    fprintf(stderr,"       wait [ <secs> ]  (default: wait for [Enter])\n\n");
    fprintf(stderr,"create, remove, bg, and wait don't create new links.\n");
    exit(1);
}


#define NEXT (++optind < argc)
#define HAS_MORE (optind < argc-1)
#define NEED_NEXT { if (!NEXT) usage(*argv); }
#define ARG argv[optind]
#define NEXT_ARG argv[optind+1]


int main(int argc,char **argv)
{
    struct sockaddr_in addr;
    char *end;
    int verbose = 0;
    int do_create,do_background = 0,to_switch = 0;
    int c;

    FD_ZERO(&in_set);
    while ((c = getopt(argc,argv,"dvV")) != EOF)
	switch (c) {
	    case 'd':
		debug = 1;
		break;
	    case 'v':
		verbose = 1;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
	    default:
		usage(*argv);
	}
    optind--;
    if (!HAS_MORE) usage(*argv);
    while (NEXT) {
	if ((do_create = !strcmp(ARG,"create")) || !strcmp(ARG,"remove")) {
	    int s;
	    int itf = 0;

	    if ((s = socket(PF_ATMSVC,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return 1;
	    }
	    if (HAS_MORE) {
		itf = strtoul(NEXT_ARG,&end,10);
		if (*end) itf = 0;
		else (void) NEXT;
	    }
	    if (ioctl(s,do_create ? ATMTCP_CREATE : ATMTCP_REMOVE,itf) < 0) {
		perror(do_create ? "ioctl ATMTCP_CREATE" :
		  "ioctl ATMTCP_REMOVE");
		return 1;
	    }
	    (void) close(s);
	    if (verbose)
		fprintf(stderr,"Persistent ATMTCP interface %d %s\n",itf,
		  do_create ? "created" : "removed");
	}
	else if (!strcmp(ARG,"virtual")) {
	    int *fd = alloc_t(int);
	    int itf = -1;

	    if (HAS_MORE) {
		itf = strtoul(NEXT_ARG,&end,10);
		if (*end) itf = -1;
		else (void) NEXT;
	    }
	    if ((*fd = socket(PF_ATMSVC,SOCK_DGRAM,0)) < 0) {
		perror("socket");
		return 1;
	    }
	    if ((itf = ioctl(*fd,SIOCSIFATMTCP,itf)) < 0) {
		perror("ioctl SIOCSIFATMTCP");
		return 1;
	    }
	    (void) register_in(*fd,&virtual_in,NULL);
	    register_out(&virtual_ops,fd);
	    fprintf(stderr,"Link %d: virtual interface %d\n",links++,itf);
	}
	else if (!strcmp(ARG,"real")) {
	    REAL_DATA *data = alloc_t(REAL_DATA);
	    int itf = 0;

	    if (HAS_MORE) {
		itf = strtoul(NEXT_ARG,&end,10);
		if (*end) itf = 0;
		else (void) NEXT;
	    }
	    data->itf = itf;
	    data->vccs = NULL;
	    register_out(&real_ops,data);
	    fprintf(stderr,"Link %d: real interface %d\n",links++,itf);
	}
	else if (!strcmp(ARG,"connect") || (to_switch = !strcmp(ARG,"switch")))
	  {
	    int *fd = alloc_t(int);
	    const char *host,*line = NULL;
	    int port;

	    NEED_NEXT;
	    host = ARG;
	    if ((*fd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket");
		return 1;
	    }
	    addr.sin_family = AF_INET;
	    addr.sin_addr.s_addr = text2ip(ARG,NULL,T2I_NAME | T2I_ERROR);
	    if (addr.sin_addr.s_addr == INADDR_NONE) return 1;
	    if (to_switch) {
		NEED_NEXT;
		line = ARG;
	    }
	    if (!HAS_MORE) port = DEFAULT_PORT;
	    else {
		port = strtoul(NEXT_ARG,&end,10);
		if (*end) port = DEFAULT_PORT;
		else (void) NEXT;
	    }
	    addr.sin_port = htons(port);
	    if (connect(*fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
		perror("connect");
		return 1;
	    }
	    (void) register_in(*fd,&tcp_in,NULL);
	    register_out(&tcp_ops,fd);
	    fprintf(stderr,"Link %d: ATMTCP connection to %s\n",links++,host);
	    if (to_switch) tcp_do_send(*fd,line,strlen(line)+1);
	}
	else if (!strcmp(ARG,"listen") ||
	  (do_background = !strcmp(ARG,"listen-bg"))) {
	    int fd,port;
	    socklen_t addr_len;
	    int *fd2 = alloc_t(int);

	    if ((fd = socket(PF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket");
		return 1;
	    }
	    addr.sin_family = AF_INET;
	    addr.sin_addr.s_addr = htonl(INADDR_ANY);
	    if (!HAS_MORE) port = DEFAULT_PORT;
	    else {
		port = strtoul(NEXT_ARG,&end,10);
		if (*end) port = DEFAULT_PORT;
		else (void) NEXT;
	    }
	    addr.sin_port = htons(port);
	    if (bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	    }
	    if (listen(fd,5) < 0) {
		perror("listen");
		return 1;
	    }
	    if (verbose)
		fprintf(stderr,"Listening on TCP port %d\n",port);
	    if (do_background) background();
	    addr_len = sizeof(addr);
	    if ((*fd2 = accept(fd,(struct sockaddr *) &addr,&addr_len)) < 0) {
		perror("accept");
		return 1;
	    }
	    (void) close(fd);
	    (void) register_in(*fd2,&tcp_in,NULL);
	    register_out(&tcp_ops,fd2);
	    fprintf(stderr,"Link %d: incoming ATMTCP connection from %s\n",
	      links++,addr.sin_family == AF_INET ? inet_ntoa(addr.sin_addr) :
	      "non-IPv4 host");
	}
	else if (!strcmp(ARG,"read")) {
	    int *stream = alloc_t(int);
	    const char *name;
	    int fd;

	    NEED_NEXT;
	    name = ARG;
	    if ((fd = open(ARG,O_RDONLY)) < 0) {
		perror(ARG);
		return 1;
	    }
	    if (!HAS_MORE) *stream = -1;
	    else {
		*stream = strtoul(NEXT_ARG,&end,10);
		if (*end) *stream = -1;
		else (void) NEXT;
	    }
	    (void) register_in(fd,&file_in,stream);
	    fprintf(stderr,"Link %d: read-only file \"%s\"\n",links++,name);
	}
	else if (!strcmp(ARG,"write")) {
	    int *fd = alloc_t(int);

	    NEED_NEXT;
	    if ((*fd = open(ARG,O_CREAT | O_WRONLY | O_TRUNC,0666)) < 0) {
		perror(ARG);
		return 1;
	    }
	    register_out(&file_ops,fd);
	    fprintf(stderr,"Link %d: write-only file \"%s\"\n",links++,ARG);
	}
	else if (!strcmp(ARG,"print")) {
	    register_out(&print_ops,0);
	    fprintf(stderr,"Link %d: printing on standard output\n",links++);
	}
	else if (!strcmp(ARG,"bg")) background();
	else if (!strcmp(ARG,"wait")) {
	    int secs = -1;

	    if (HAS_MORE) {
		secs = strtoul(NEXT_ARG,&end,10);
		if (*end) secs = -1;
		else (void) NEXT;
	    }
	    if (secs != -1) {
		sleep(secs);
		continue;
	    }
	    fprintf(stderr,"Press <Return> to continue\n");
	    do c = getchar();
	    while (c != EOF && c != '\n');
	}
	else usage(*argv);
    }
    if (!fds) return 0;
    while (1) {
	IN *in;
	fd_set set;
	int ret;

	set = in_set;
	ret = select(fds,&set,NULL,NULL,NULL);
	if (ret < 0) {
	    if (errno != EINTR) perror("select");
	    continue;
	}
	for (in = inputs; in; in = in->next)
	    if (FD_ISSET(in->fd,&set)) in->recv(in);
    }
}

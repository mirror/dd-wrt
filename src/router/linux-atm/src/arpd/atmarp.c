/* atmarp.c - RFC1577 ATMARP control */
 
/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>

#include <stdint.h>
#include <linux/atmarp.h>
#include <linux/atmclip.h>

#include "atm.h"
#include "atmd.h"
#include "atmarpd.h"


#define BUF_SIZE 4096


static int query_result(struct atmarp_req *reply)
{
    unsigned char *ipp = (unsigned char *) &reply->ip;
    char buf[MAX_ATM_ADDR_LEN+1];
    int error;

    printf("IP:  %d.%d.%d.%d\n",ipp[0],ipp[1],ipp[2],ipp[3]);
    if (!atmsvc_addr_in_use(reply->addr)) return 0;
    error = atm2text(buf,sizeof(buf),(struct sockaddr *) &reply->addr,
      A2T_PRETTY | A2T_NAME) < 0;
    if (error) strcpy(buf,"<invalid>");
    printf("ATM: %s\n",buf);
    return error ? 1 : 0;
}


static int send_request(struct atmarp_req *req)
{
    struct atmarp_req reply;
    int s,len;

    s = un_attach(ATMARP_SOCKET_PATH);
    if (s < 0) {
	perror("un_attach");
	exit(1);
    }
    if (write(s,req,sizeof(*req)) < 0) {
	perror("write");
	exit(1);
    }
    len = read(s,&reply,sizeof(reply));
    if (len < 0) {
	perror("read");
	exit(1);
    }
    if (req->type == art_query) return query_result(&reply);
    if (len != sizeof(int)) {
	fprintf(stderr,"bad read: %d != %d\n",len,sizeof(int));
	exit(1);
    }
    if (*(int *) &reply < 0) {
	fprintf(stderr,"atmarp: %s\n",strerror(-*(int *) &reply));
	exit(1);
    }
    return *(int *) &reply;
}


static int print_table(void)
{
    char buffer[BUF_SIZE];
    int fd,size;

    if ((fd = open(ATMARP_DUMP_DIR "/" ATMARP_DUMP_FILE,O_RDONLY)) < 0) {
	perror("open " ATMARP_DUMP_DIR "/" ATMARP_DUMP_FILE);
	return 1;
    }
    while ((size = read(fd,buffer,BUF_SIZE))) {
	if (size < 0) {
	    perror("read " ATMARP_DUMP_DIR "/" ATMARP_DUMP_FILE);
	    return 1;
	}
	if (write(1,buffer,size) < 0) {
	    perror("write stdout");
	    return 1;
	}
    }
    return 0;
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s -a\n",name);
    fprintf(stderr,"%6s %s -c [[atm]N]\n","",name);
    fprintf(stderr,"%6s %s -q ip_addr [qos qos_spec] [sndbuf bytes]\n","",name);
    fprintf(stderr,"%6s %s -s ip_addr [itf.]vpi.vci [pcr value] [qos spec] "
      "[sndbuf bytes]\n%8s [temp] [pub] [null]\n","",name,"");
    fprintf(stderr,"%6s %s -s ip_addr atm_addr [pcr value] [qos spec] "
      "[sndbuf bytes] [temp]\n%8s [pub] [arpsrv]\n","",name,"");
    fprintf(stderr,"%6s %s -d ip_addr [arpsrv]\n","",name);
#if 0  /* undocumented */
    fprintf(stderr,"%6s %s -Q ip_addr\n","",name);
#endif
    fprintf(stderr,"%6s %s -V\n","",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct atmarp_req req;
    int c,i,num;
    char *here,*end;

    req.type = 0;
    while ((c = getopt(argc,argv,"acdqQsV")) != EOF)
	switch (c) {
	    case 'a':
		if (argc != optind || req.type) usage(argv[0]);
		req.type = art_table;
		/* (void) send_request(&req); @@@ fix this later */
		return print_table();
	    case 'c':
		if (req.type) usage(argv[0]);
		req.type = art_create;
		break;
	    case 'd':
		if (req.type) usage(argv[0]);
		req.type = art_delete;
		break;
	    case 'q':
		if (req.type) usage(argv[0]);
		req.type = art_qos;
		break;
	    case 'Q':
		if (req.type) usage(argv[0]);
		req.type = art_query;
		break;
	    case 's':
		if (req.type) usage(argv[0]);
		req.type = art_set;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
	    default:
		usage(argv[0]);
	}
    switch (req.type) {
	case art_create:
	    if (argc == optind) req.itf = -1;
	    else {
		if (argc != optind+1) usage(argv[0]);
		here = argv[optind];
		if (strlen(here) > 3 && !strncmp(here,"atm",3)) here += 3;
		req.itf = strtoul(here,&end,10);
		if (*end || (here[0] == '0' && here[1])) {
		    usage(argv[0]);
		    return 1;
		}
	    }
	    num = send_request(&req);
	    if (req.itf == -1) printf("atm%d\n",num);
	    return 0;
	case art_qos:
	    if (argc < optind+1) usage(argv[0]);
	    /* fall through */
	case art_set:
	    if (argc < optind+2) usage(argv[0]);
	    break;
	case art_query:
	    if (argc != optind+1) usage(argv[0]);
	    break;
	case art_delete:
	    if (argc < optind+1) usage(argv[0]);
	    break;
	default:
	    usage(argv[0]);
    }
    req.ip = text2ip(argv[optind],NULL,T2I_NAME | T2I_ERROR);
    if (req.ip == INADDR_NONE) return 1;
    req.flags = ATF_PERM;
    if (req.type == art_qos) {
	memset(&req.qos,0,sizeof(req.qos));
	req.sndbuf = 0;
	for (i = optind+1; i < argc; i++)
	    if (!strcmp(argv[i],"qos")) {
		    if (++i >= argc) usage(argv[0]);
		    if (text2qos(argv[i],&req.qos,0)) usage(argv[0]);
		}
	    else if (!strcmp(argv[i],"sndbuf")) {
		    if (++i >= argc) usage(argv[0]);
		    req.sndbuf = strtol(argv[i],&end,0);
		    if (*end) usage(argv[0]);
		}
	    else if (i != optind+1 || argc != optind+2 ||
		  text2qos(argv[optind+1],&req.qos,0)) usage(argv[0]);
    }
    if (req.type == art_set) {
	memset(&req.qos,0,sizeof(req.qos));
	req.sndbuf = 0;
	for (i = optind+2; i < argc; i++)
	    if (!strcmp(argv[i],"temp")) req.flags &= ~ATF_PERM;
	    else if (!strcmp(argv[i],"pub")) req.flags |= ATF_PUBL;
	    else if (!strcmp(argv[i],"null")) req.flags |= ATF_NULL;
	    else if (!strcmp(argv[i],"arpsrv")) req.flags |= ATF_ARPSRV;
	    else if (!strcmp(argv[i],"qos")) {
		    if (++i >= argc) usage(argv[0]);
		    if (text2qos(argv[i],&req.qos,0)) usage(argv[0]);
		}
	    else if (!strcmp(argv[i],"sndbuf")) {
		    if (++i >= argc) usage(argv[0]);
		    req.sndbuf = strtol(argv[i],&end,0);
		    if (*end) usage(argv[0]);
		}
	    else if (!strcmp(argv[i],"pcr")) {
		    if (++i >= argc) usage(argv[0]);
		    req.qos.txtp.traffic_class = req.qos.rxtp.traffic_class =
		      ATM_CBR;
		    req.qos.txtp.max_pcr = req.qos.rxtp.max_pcr =
		      strtol(argv[i],&end,0);
		    if (*end) usage(argv[0]);
		}
	    else usage(argv[0]);
	if (text2atm(argv[optind+1],(struct sockaddr *) &req.addr,
	  sizeof(req.addr),T2A_NAME) < 0) {
	    fprintf(stderr,"%s: invalid ATM address\n",argv[optind+1]);
	    return 1;
	}
    }
    if (req.type == art_delete && optind+1 < argc) {
	if (optind+2 < argc || strcmp(argv[optind+1],"arpsrv")) usage(argv[0]);
	req.flags |= ATF_ARPSRV;
    }
    if (!req.qos.aal) req.qos.aal = ATM_AAL5;
    send_request(&req);
    return 0;
}

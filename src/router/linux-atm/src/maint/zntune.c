/* zntune.c - ZN122x free buffer pool tuning */
 
/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <atm.h>
#include <sys/time.h> /* for struct timeval, although it's not used */
#include <linux/atm_zatm.h>


static void usage(const char *name)
{
    fprintf(stderr,"%s: [-z] itf [pool]\n",name);
    fprintf(stderr,"%s: [-l low_water] [-h high_water] [-t threshold] itf "
      "[pool]\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    char *name,*end;
    int low_water,high_water,next_thres,zero;
    int itf,pool,first,i;
    int c,s;

    low_water = high_water = next_thres = zero = 0;
    name = argv[0];
    while ((c = getopt(argc,argv,"l:h:t:z")) != EOF)
	switch (c) {
	    case 'l':
		low_water = strtol(optarg,&end,0);
		if (*end || low_water < 0) usage(name);
		break;
	    case 'h':
		high_water = strtol(optarg,&end,0);
		if (*end || high_water < 0) usage(name);
		break;
	    case 't':
		next_thres = strtol(optarg,&end,0);
		if (*end || next_thres < 0) usage(name);
		break;
	    case 'z':
		zero = 1;
	    default:
		usage(name);
	}
    if (zero && (low_water || high_water)) usage(name);
    if (argc < optind+1 || argc > optind+2) usage(name);
    itf = strtol(argv[optind],&end,0);
    if (*end || itf < 0) usage(name);
    if (argc < optind+2) pool = -1;
    else {
	pool = strtol(argv[optind+1],&end,0);
	if (*end || pool < 0 || pool > 31) usage(name);
    }
    s = socket(PF_ATMPVC,SOCK_DGRAM,0);
    if (s < 0) {
	perror("socket");
	return 1;
    }
    first = 1;
    for (i = pool == -1 ? 0 : pool; i <= (pool == -1 ? ZATM_LAST_POOL : pool);
      i++) {
	struct atmif_sioc sioc;
	struct zatm_pool_req req;

	sioc.number = itf;
	sioc.arg = &req;
	sioc.length = sizeof(req);
	req.pool_num = i;
	if (low_water || high_water) {
	    req.info.low_water = low_water;
	    req.info.high_water = high_water;
	    req.info.next_thres = next_thres;
	    if (ioctl(s,ZATM_SETPOOL,&sioc) < 0) {
		perror("ioctl ZATM_SETPOOL");
		return 1;
	    }
	}
	else {
	    int size;

	    if (ioctl(s,zero ? ZATM_GETPOOLZ : ZATM_GETPOOL,&sioc) < 0) {
		perror(zero ? "ioctl ZATM_GETPOOLZ" : "ioctl ZATM_GETPOOL");
		return 1;
	    }
	    if (first)
		printf("Pool Size Ref Low High Alarm   Under Offs NxOf Count "
		  "Thres\n");
	    printf(" %2d  ",i);
	    size = 64 << (i < 2 ? 0 : i-2);
	    if (size < 1024) printf("%4d",size);
	    else printf("%3dk",size >> 10);
	    printf(" %3d %3d %4d%8d%6d %4d %4d%6d%6d\n",
	      req.info.ref_count,req.info.low_water,req.info.high_water,
	      req.info.rqa_count,req.info.rqu_count,req.info.offset,
	      req.info.next_off,req.info.next_cnt,req.info.next_thres);
	}
	first = 0;
    }
    return 0;
}

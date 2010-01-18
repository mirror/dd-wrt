#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "../include/can.h"

int fd;

struct canfilt_t canfilt = {
	.flags = 0,
	.queid = 0,
	.cob = 0,
	.id = 0,
	.mask = 0
};

int canfilt_fl;

int query_fl;

int can_wait_sec = 5;

char *can_dev_name = "/dev/can0";

#define PRT_PREFIX_SIZE 40
char prt_prefix[PRT_PREFIX_SIZE];

char *prt_prefix_in = "CAN %s : ";

int can_fd_wait(int fd, int wait_sec)
{
	int ret;
	struct timeval timeout;
	fd_set set;

	FD_ZERO (&set);
	FD_SET (fd, &set);
	timeout.tv_sec = wait_sec;
	timeout.tv_usec = 0;
	while ((ret=select(FD_SETSIZE,&set, NULL, NULL,&timeout))==-1
          &&errno==-EINTR);
	return ret;
}


/*--- handler on SIGINT signal : the program quit with CTL-C ---*/
void sortie(int sig)
{
	close(fd);
	printf("Terminated by user\n");
	exit(0);
}

static void
usage(void)
{
  printf("usage: readburst\n");
  printf("  -d, --device  <name>     name of CAN device [/dev/can0]\n");
  printf("  -m, --mask  <num>        CAN filter mask\n");
  printf("  -i, --id  <num>          CAN filter message ID\n");
  printf("  -f, --flags <num>        CAN filter flags\n");
  printf("  -w, --wait <num>         number of seconds to wait in select call\n");
  printf("  -p, --prefix <str>       string prefix for output\n");
  printf("  -q, --query              query driver features\n");
  printf("  -V, --version            show version\n");
  printf("  -h, --help               this usage screen\n");
}


int main(int argc, char *argv[])
{
	static struct option long_opts[] = {
		{ "uldev", 1, 0, 'd' },
		{ "mask",  1, 0, 'm' },
		{ "id",    1, 0, 'i' },
		{ "flags", 1, 0, 'f' },
		{ "wait",  1, 0, 'w' },
		{ "prefix",1, 0, 'p' },
		{ "query" ,0 ,0, 'q' },
		{ "version",0,0, 'V' },
		{ "help",  0, 0, 'h' },
		{ 0, 0, 0, 0}
	};
	int opt;

	int n,ret;
	unsigned long i=0;
       #ifdef CAN_MSG_VERSION_2
	struct canmsg_t readmsg={0,0,5,{0,0},0,{0,}};
       #else /* CAN_MSG_VERSION_2 */
	struct canmsg_t readmsg={0,0,5,0,0,{0,}};
       #endif /* CAN_MSG_VERSION_2 */
	struct sigaction act;


	while ((opt = getopt_long(argc, argv, "d:m:i:f:w:p:qVh",
			    &long_opts[0], NULL)) != EOF) switch (opt) {
		case 'd':
			can_dev_name=optarg;
			break;
		case 'm':
			canfilt_fl=1;
			canfilt.mask = strtol(optarg,NULL,0);
			break;
		case 'i':
			canfilt_fl=1;
			canfilt.id = strtol(optarg,NULL,0);
			break;
		case 'f':
			canfilt_fl=1;
			canfilt.flags = strtol(optarg,NULL,0);
			break;
		case 'w':
			can_wait_sec = strtol(optarg,NULL,0);
			break;
		case 'p':
			prt_prefix_in = optarg;
			break;
		case 'q':
			query_fl=1;
			break;
		case 'V':
			fputs("LinCAN utilities v0.2\n", stdout);
			exit(0);
		case 'h':
		default:
			usage();
			exit(opt == 'h' ? 0 : 1);
	}


	/*------- register handler on SIGINT signal -------*/
	act.sa_handler=sortie;
	sigemptyset(&act.sa_mask);
	act.sa_flags=0;
	sigaction(SIGINT,&act,0);
	/*---------------------------------------*/	

	if ((fd=open(can_dev_name, O_RDWR)) < 0) {
		perror("open");
		printf("Error opening %s\n", can_dev_name);
		exit(1);	
	}
	
	if (query_fl) {
		n=ioctl(fd, CAN_DRV_QUERY, CAN_DRV_QRY_BRANCH);
		printf("CAN driver branch:  %c%c%c%c\n",(n>>24)&0xff,(n>>16)&0xff,(n>>8)&0xff,n&0xff);
		n=ioctl(fd, CAN_DRV_QUERY, CAN_DRV_QRY_VERSION);
		printf("CAN driver version: %d.%d.%d\n",(n>>16)&0xff,(n>>8)&0xff,n&0xff);
		n=ioctl(fd, CAN_DRV_QUERY, CAN_DRV_QRY_MSGFORMAT);
		printf("CAN message format: %08x\n",n);
		close(fd);
		return 0;
	}

	if (canfilt_fl) {
		ret = ioctl(fd, CANQUE_FILTER, &canfilt);
		if(ret<0) {
			perror("ioctl FILTER_QUE");
		}
	}

	snprintf(prt_prefix, PRT_PREFIX_SIZE, prt_prefix_in, can_dev_name);
	
	while (1) {
		readmsg.flags=0;
		readmsg.cob=0;
	    #if 1
		ret=can_fd_wait(fd, can_wait_sec);
		printf("%scan_fd_wait returned %d\n", prt_prefix, ret);
	    #endif
		ret=read(fd,&readmsg,sizeof(struct canmsg_t));
		if(ret <0) {
			printf("%sError reading message\n", prt_prefix);
		}
		else if(ret == 0) {
			printf("%sNo message arrived\n", prt_prefix);
		} else {
			printf("%sRx msg #%lu: id=%lX dlc=%u flg=0x%02x",
				prt_prefix,i,readmsg.id,readmsg.length,readmsg.flags);
			for(n=0 ; n<readmsg.length ; n++)
				printf(" %.2X",(unsigned char)readmsg.data[n]);
			printf("\n");
			i++;
		}
	}
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../include/can.h"

int canmsg_flags = 0;
unsigned long canmsg_id = 5;
int block = 10;
int count = 0;

int can_wait_sec = 1;
int o_sync_fl = 0;

char *can_dev_name = "/dev/can0";

#define PRT_PREFIX_SIZE 40
char prt_prefix[PRT_PREFIX_SIZE];

char *prt_prefix_in = "CAN %s : ";


static void
usage(void)
{
  printf("usage: sendburst\n");
  printf("  -d, --device  <name>     name of CAN device [/dev/can0]\n");
  printf("  -i, --id  <num>          ID of generated messages\n");
  printf("  -f, --flags <num>        CAN filter flags\n");
  printf("  -s, --sync               open in synchronous mode\n");
  printf("  -w, --wait <num>         number of seconds to wait between messages\n");
  printf("  -b, --block <num>        number of messages in  block\n");
  printf("  -c, --count <num>        number of sent blocks of messages\n");
  printf("  -p, --prefix <str>       string prefix for output\n");
  printf("  -V, --version            show version\n");
  printf("  -h, --help               this usage screen\n");
}

int main(int argc, char *argv[])
{
	static struct option long_opts[] = {
		{ "uldev", 1, 0, 'd' },
		{ "id",    1, 0, 'i' },
		{ "flags", 1, 0, 'f' },
		{ "sync",  0, 0, 's' },
		{ "wait",  1, 0, 'w' },
		{ "block", 1, 0, 'b' },
		{ "count", 1, 0, 'c' },
		{ "prefix",1, 0, 'p' },
		{ "version",0,0, 'V' },
		{ "help",  0, 0, 'h' },
		{ 0, 0, 0, 0}
	};
	int opt;

       #ifdef CAN_MSG_VERSION_2
	struct canmsg_t sendmsg={0,0,5,{0,0},8,{1,2,3,4,5,6,7,8}};
       #else /* CAN_MSG_VERSION_2 */
	struct canmsg_t sendmsg={0,0,5,0,8,{1,2,3,4,5,6,7,8}};
       #endif /* CAN_MSG_VERSION_2 */
	int fd, ret,i,j;

	while ((opt = getopt_long(argc, argv, "d:i:f:sw:b:c:p:Vh",
			    &long_opts[0], NULL)) != EOF) switch (opt) {
		case 'd':
			can_dev_name=optarg;
			break;
		case 'i':
			canmsg_id = strtol(optarg,NULL,0);
			break;
		case 'f':
			canmsg_flags = strtol(optarg,NULL,0);
			break;
		case 's':
			o_sync_fl = 1;
			break;
		case 'w':
			can_wait_sec = strtol(optarg,NULL,0);
			break;
		case 'b':
			block = strtol(optarg,NULL,0);
			break;
		case 'c':
			count = strtol(optarg,NULL,0);
			break;
		case 'p':
			prt_prefix_in = optarg;
			break;
		case 'V':
			fputs("LinCAN utilities v0.2\n", stdout);
			exit(0);
		case 'h':
		default:
			usage();
			exit(opt == 'h' ? 0 : 1);
	}

	if ((fd=open(can_dev_name, O_RDWR | (o_sync_fl? O_SYNC:0))) < 0) {
		perror("open");
		printf("Error opening %s\n", can_dev_name);
		exit(1);	
	}

	snprintf(prt_prefix, PRT_PREFIX_SIZE, prt_prefix_in, can_dev_name);

	j=0;
	while (1) {
		for(i=0;i<block;i++) {
			sendmsg.flags=canmsg_flags;
			sendmsg.id=canmsg_id;
			sendmsg.data[0]=i;
			sendmsg.data[1]=j;
			if ((ret=write(fd,&sendmsg,sizeof(struct canmsg_t))) < 0) {
				perror("write");
				printf("%sError sending message\n", prt_prefix);
				break;
			}
		}
		printf("%sSent block of %d messages #: %u\n", prt_prefix, block, j);
		j++;
		usleep(1000000*can_wait_sec);
		if(count)
			if(!--count) break;
	}
	close(fd);
	return 0;
}

	

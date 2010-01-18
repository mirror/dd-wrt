#include <rtl.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <posix/unistd.h>

#define printf rtl_printf

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

int canfilt_mask, canfilt_id, canfilt_flags;


int can_wait_sec = 5;

char *can_dev_name = "/dev/can0";

#define PRT_PREFIX_SIZE 40
char prt_prefix[PRT_PREFIX_SIZE];

char *prt_prefix_in = "CAN %s : ";

MODULE_PARM_DESC(can_dev_name,"name of CAN device [/dev/can0]");
MODULE_PARM(can_dev_name,"1s");
MODULE_PARM_DESC(canfilt_mask,"CAN filter mask");
MODULE_PARM(canfilt_mask,"1i");
MODULE_PARM_DESC(canfilt_id,"CAN filter message ID");
MODULE_PARM(canfilt_id,"1i");
MODULE_PARM_DESC(canfilt_flags,"CAN filter flags");
MODULE_PARM(canfilt_flags,"1i");
MODULE_PARM_DESC(can_wait_sec,"number of seconds to wait between messages");
MODULE_PARM(can_wait_sec,"1i");
MODULE_PARM_DESC(prt_prefix_in,"string prefix for output");
MODULE_PARM(prt_prefix_in,"1i");

MODULE_SUPPORTED_DEVICE("sendburst_rtl");
MODULE_AUTHOR("Pavel Pisa <pisa@cmp.felk.cvut.cz>");
MODULE_DESCRIPTION("readburst_rtl: receiver of CAN messages for RT-Linux interface to LinCAN driver");
MODULE_LICENSE("GPL");

void readburst_cleanup(void *arg)
{
	printf("%scleanup handler called\n", prt_prefix);
	close((int)arg);
}

#if 0
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
#endif

int readburst_main(void *arg)
{
	int n,ret;
	unsigned long i=0;
       #ifdef CAN_MSG_VERSION_2
	struct canmsg_t readmsg={0,0,5,{0,0},0,{0,}};
       #else /* CAN_MSG_VERSION_2 */
	struct canmsg_t readmsg={0,0,5,0,0,{0,}};
       #endif /* CAN_MSG_VERSION_2 */

	if(canfilt_mask || canfilt_id || canfilt_flags){
		canfilt_fl=1;
		canfilt.mask=canfilt_mask;
		canfilt.id=canfilt_id;
		canfilt.flags=canfilt_flags;
	}

	if ((fd=open(can_dev_name, O_RDWR)) < 0) {
		printf("Error opening %s\n", can_dev_name);
		return -1;	
	}
        pthread_cleanup_push(readburst_cleanup,(void*)fd);

	snprintf(prt_prefix, PRT_PREFIX_SIZE, prt_prefix_in, can_dev_name);
	
	if (canfilt_fl) {
		ret = ioctl(fd, CANQUE_FILTER, &canfilt);
		if(ret<0) {
			printf("%serror in call ioctl FILTER_QUE",prt_prefix);
		}
	}

	
	while (1) {
		readmsg.flags=0;
		readmsg.cob=0;
	    #if 0
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
		pthread_testcancel();

	}
	/* close(fd); is called by cleanup handler*/
        pthread_cleanup_pop(1);
	return 0;
}

/*===========================================================*/


void * t1_routine(void *arg)
{
  readburst_main(NULL);

  while (1) {
    pthread_wait_np ();

  }
  return 0;
}

pthread_t t1;

int init_module(void) {
	
        return pthread_create (&t1, NULL, t1_routine, 0);

}

void cleanup_module(void) {
        pthread_delete_np (t1);
	
	/*pthread_cancel(t1);
	pthread_join(t1, NULL);*/
}

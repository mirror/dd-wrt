#include <rtl.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <posix/unistd.h>

#define printf rtl_printf

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

MODULE_PARM_DESC(can_dev_name,"name of CAN device [/dev/can0]");
MODULE_PARM(can_dev_name,"1s");
MODULE_PARM_DESC(canmsg_id,"ID of generated messages");
MODULE_PARM(canmsg_id,"1i");
MODULE_PARM_DESC(canmsg_flags,"CAN filter flags");
MODULE_PARM(canmsg_flags,"1i");
MODULE_PARM_DESC(o_sync_fl,"open in synchronous mode");
MODULE_PARM(o_sync_fl,"1i");
MODULE_PARM_DESC(can_wait_sec,"number of seconds to wait between messages");
MODULE_PARM(can_wait_sec,"1i");
MODULE_PARM_DESC(block,"number of messages in  block");
MODULE_PARM(block,"1i");
MODULE_PARM_DESC(count,"number of sent blocks of messages");
MODULE_PARM(count,"1i");
MODULE_PARM_DESC(prt_prefix_in,"string prefix for output");
MODULE_PARM(prt_prefix_in,"1i");


MODULE_SUPPORTED_DEVICE("sendburst_rtl");
MODULE_AUTHOR("Pavel Pisa <pisa@cmp.felk.cvut.cz>");
MODULE_DESCRIPTION("sendburst_rtl: generatot of CAN messages for RT-Linux interface to LinCAN driver");
MODULE_LICENSE("GPL");

void sendburst_cleanup(void *arg)
{
	printf("%scleanup handler called\n", prt_prefix);
	close((int)arg);
}

int sendburst_main(void *arg)
{
       #ifdef CAN_MSG_VERSION_2
	struct canmsg_t sendmsg={0,0,5,{0,0},8,{1,2,3,4,5,6,7,8}};
       #else /* CAN_MSG_VERSION_2 */
	struct canmsg_t sendmsg={0,0,5,0,8,{1,2,3,4,5,6,7,8}};
       #endif /* CAN_MSG_VERSION_2 */
	int fd, ret,i,j;


	if ((fd=open(can_dev_name, O_RDWR | (o_sync_fl? O_SYNC:0))) < 0) {
		printf("Error opening %s\n", can_dev_name);
		return -1;
	}
        pthread_cleanup_push(sendburst_cleanup,(void*)fd);

	snprintf(prt_prefix, PRT_PREFIX_SIZE, prt_prefix_in, can_dev_name);

	j=0;
	while (1) {
		for(i=0;i<block;i++) {
			sendmsg.flags=canmsg_flags;
			sendmsg.id=canmsg_id;
			sendmsg.data[0]=i;
			sendmsg.data[1]=j;
			if ((ret=write(fd,&sendmsg,sizeof(struct canmsg_t))) < 0) {
				printf("%sError sending message\n", prt_prefix);
				break;
			}
		}
		printf("%sSent block of %d messages #: %u\n", prt_prefix, block, j);
		j++;
		pthread_testcancel();
		usleep(1000000*can_wait_sec);
		if(count)
			if(!--count) break;
		pthread_testcancel();
	}
	/* close(fd); is called by cleanup handler*/
        pthread_cleanup_pop(1);
	return 0;
}

	
/*===========================================================*/


void * t1_routine(void *arg)
{
  sendburst_main(NULL);

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
        /*pthread_delete_np (t1);*/
	
	pthread_cancel(t1);
	pthread_join(t1, NULL);
}

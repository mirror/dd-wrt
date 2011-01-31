#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h> /* for OPEN_MAX   */
#include <stdint.h>
#include <netinet/in.h> /* for ntohl()    */
#include <linux/types.h>
#include <linux/atmmpc.h>
#include <atm.h>
#include "k_interf.h"
#include "packets.h"
#include "io.h"
#include "get_vars.h"

#if 0
#define dprintf printf
#else
#define dprintf(format,args...)
#endif
                           
extern int keep_alive_sm_running;       /* from io.c   */
extern struct mpc_control mpc_control;  /* from main.c */

static void snd_mpoa_res_rqst(struct k_message *msg);
static void snd_mpoa_res_rtry(struct k_message *msg);
static void set_mps_ctrl_addr(struct k_message *msg);
static void stop_keep_alive_sm(void);
static uint32_t traff_class_to_service_category(uint8_t traffic_class);

/*
 * returns < 0 for error
 *
 */
int send_to_kernel(struct k_message *msg)
{
        if (write(mpc_control.kernel_socket, msg, sizeof(struct k_message)) !=
            sizeof(struct k_message) ) {
                printf("mpcd: k_interf.c: write to kernel failed!\n");
                return -1;
        }
        return 1;
}

/*
 * returns 0 for error
 *
 */

int msg_from_kernel(int fd)
{
        ssize_t bytes_read;
        struct k_message msg;
	memset(&msg,0,sizeof(struct k_message));
        bytes_read = read(fd, (void *)&msg, sizeof(msg));
        if (bytes_read < 0) {
                printf("mpcd: k_interf.c: read failed from kernel: %s\n", strerror(errno));
                return 0;
        }

        if (bytes_read == 0) {
                printf("mpcd: k_interf.c:EOF from kernel\n");
                return 0;
        }
        if (bytes_read != sizeof(msg)) {
                printf("mpcd: k_interf.c: msg from kernel wrong size\n");
                return 0;
        }
	dprintf("mpcd: k_interf.c: message from kernel: ");

        if (msg.type == DIE) {
                dprintf(" die\n");
                exit(0);    
        }

        /* do nothing if MPS's control ATM address is not known */
        if (msg.type != SET_MPS_CTRL_ADDR && !mpc_control.mps_ctrl_addr_set)
                return 0;

        switch(msg.type) {
        case SND_MPOA_RES_RQST:
	        dprintf("snd_mpoa_res_rqst.\n");
                snd_mpoa_res_rqst(&msg);
                return 1;
                break;               /* not reached */
	case SND_MPOA_RES_RTRY:
	        dprintf("snd_mpoa_res_rtry.\n");
	        snd_mpoa_res_rtry(&msg);
		return 1;
		break;               /* not reached */
        case SET_MPS_CTRL_ADDR:
	        dprintf("set_mps_ctrl_addr.\n");
                set_mps_ctrl_addr(&msg);
                return 1;
                break;               /* not reached */
	case STOP_KEEP_ALIVE_SM:
	        dprintf("stop_keep_alive_sm.\n");
	        stop_keep_alive_sm();
		return 1;
		break;               /* not reached */
	case EGRESS_ENTRY_REMOVED:
	        dprintf("egress_entry_removed.\n");
		remove_tag(msg.content.eg_info.tag);
		return 1;
		break;               /* not reached */
	case SND_EGRESS_PURGE:
	        dprintf("snd_egress_purge,cache_id = %u.\n",msg.content.eg_info.cache_id);
		send_egress_cache_purge_request(0, /* 1 == no reply, 0 == reply requested */
						ntohl(msg.content.eg_info.mps_ip),
						32,
						get_own_ip_addr(mpc_control.INTERFACE_NUMBER),
						msg.content.eg_info.cache_id);
		return 1;
		break;
	case OPEN_INGRESS_SVC:
	        dprintf(" open_ingress_svc\n");
		create_ingress_svc(msg.content.in_info.in_dst_ip,
				   msg.content.in_info.eg_MPC_ATM_addr,
				   msg.qos);
		return 1;
		break;
        case RELOAD:
                printf(" reload\n");
                return 0;
                break;
        default:
                dprintf("unknown message %d\n", msg.type);
                return 0;
                break;               /* not reached */
        }
        return 0;                    /* not reached */
}
         
static void snd_mpoa_res_rqst(struct k_message *msg){
        send_resolution_request(0,
				1,                      /* Source ip present */ 
				msg->content.in_info.in_dst_ip,
				0,                      /* prefix length */
				traff_class_to_service_category(msg->qos.txtp.traffic_class));

         return;
}

static void snd_mpoa_res_rtry(struct k_message *msg){
        uint32_t rqst_id = search_by_type(MPOA_RESOLUTION_REQUEST,
					  msg->content.in_info.in_dst_ip);
        send_resolution_request(rqst_id,
				1,              
				msg->content.in_info.in_dst_ip,
				0,              
				traff_class_to_service_category(msg->qos.txtp.traffic_class));

         return;
}

static void set_mps_ctrl_addr(struct k_message *msg)
{
        char buffer[ATM_ESA_LEN];
	int i;
	struct sockaddr_atmsvc mps_ctrl_addr;
	char *buff = buffer;
	memcpy(mps_ctrl_addr.sas_addr.prv, msg->MPS_ctrl, ATM_ESA_LEN);
	mps_ctrl_addr.sas_family = AF_ATMSVC;	
	if(mpc_control.mps_ctrl_addr_set && !memcmp(msg->MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN))
	        return;
	if(mpc_control.mps_ctrl_addr_set && memcmp(msg->MPS_ctrl,mpc_control.MPS_CTRL_ATM_ADDR,ATM_ESA_LEN)){
	        printf("mpcd: k_interf.c: new MPS ");
		 for (i = 0; i < ATM_ESA_LEN; i++)
		   printf("%02x", msg->MPS_ctrl[i]);
		 printf("\n");
		return;
	}
	memcpy(mpc_control.MPS_CTRL_ATM_ADDR, msg->MPS_ctrl, ATM_ESA_LEN);

	printf("mpcd: k_interf.c: setting MPS control ATM address to ");
	if(atm2text(buff,ATM_ESA_LEN, (struct sockaddr*)&mps_ctrl_addr, T2A_SVC)<0) {
            for (i = 0; i < ATM_ESA_LEN; i++)
	        printf("%02x", mpc_control.MPS_CTRL_ATM_ADDR[i]);
	    printf("\n");
	}
	else
	    printf("%s\n", buff);

	mpc_control.mps_ctrl_addr_set = 1;
        return;
}

static void stop_keep_alive_sm(){
  keep_alive_sm_running = 0;
  return;
}

/* 
 * Converts linux-ATM traffic descriptions to those used in MPOA ATM service 
 * category extension. Only UBR and CBR supported.
 */
static uint32_t traff_class_to_service_category(uint8_t traffic_class){
  switch(traffic_class){
  case ATM_NONE:
    return 0;
    break;
  case ATM_UBR:
    return 0;
    break;
  case ATM_CBR:
    return CBR;
    break;
  default:
    return 0;
  }
}


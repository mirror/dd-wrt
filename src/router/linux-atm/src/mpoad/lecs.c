/* lecs.c, get MPOA configuration info from LECS */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>     /* htons() and friends */
#include <unistd.h>
#include <errno.h>
#include <atm.h>
#include <atmsap.h>
#include <linux/types.h>
#include <linux/atmmpc.h>   /* for MPOA Device type TLV */
#include "lecs.h"
#include "k_interf.h"

#define MAXFRAME 1024

static int get_lecs_socket(struct sockaddr_atmsvc *sa);
static int send_request(int fd, char *buff, char *lec_addr, char *elan_name);
static int get_reply(int fd, char *buff, struct mpc_parameters *params);

void get_mpc_config(struct sockaddr_atmsvc *sa, char *lec_addr, char *elan_name)
{
        int s;
        char buff[MAXFRAME];
        struct k_message msg;

        s = get_lecs_socket(sa);
        if (s < 0) return;

        memset(buff, 0, sizeof(buff));
        if (send_request(s, buff, lec_addr, elan_name) < 0) {
                printf("mpcd: lecs.c: send_request failed, using defaults\n");
                return;
        }

        msg.content.params.mpc_p1 = MPC_P1;
        msg.content.params.mpc_p2 = MPC_P2;
        msg.content.params.mpc_p4 = MPC_P4;
        msg.content.params.mpc_p5 = MPC_P5;
        msg.content.params.mpc_p6 = MPC_P6;

        if (get_reply(s, buff, &msg.content.params) < 0) {
                printf("mpcd: lecs.c: get_config failed, using defaults\n");
                return;
        }

        msg.type = SET_MPC_PARAMS;
        send_to_kernel(&msg);

        printf("mpcd: lecs.c: get_config: about to return\n");
        return;
}

static int send_request(int fd, char *buff, char *lec_addr, char *elan_name)
{
        char *tmp;
        int retval;
        struct le_config_frame *frame;

        frame = (struct le_config_frame *)buff;
        frame->marker   = htons(0xff00);
        frame->protocol = 0x01;
        frame->version  = 0x01;
        frame->opcode   = htons(0x0001);
        frame->tran_id  = htonl(42);
        frame->flags    = htons(0x002);
        memcpy(frame->src_atm_addr, lec_addr, ATM_ESA_LEN);
        frame->num_tlvs = 1;
        if (elan_name != NULL) {
                strcpy(frame->elan_name, elan_name);
                frame->elan_name_size = strlen(elan_name);
        }

        /* add the MPOA device type TLV */
        tmp = buff + sizeof(struct le_config_frame);
        *(uint32_t *)tmp = htonl(TLV_MPOA_DEVICE_TYPE);
        tmp += 4;

        *tmp++ = 22; /* device type field + MPC's ATM address */
        *tmp++ = MPC;
        *tmp++ = 0;
        memcpy(tmp, lec_addr, ATM_ESA_LEN); tmp += ATM_ESA_LEN;
        
        retval = write(fd, buff, tmp - buff);
        if (retval < 0 || retval != (tmp - buff)) return -1;

        return 0;
}

static int get_reply(int fd, char *buff, struct mpc_parameters *params)
{
        int retval;
        uint32_t type;
        uint8_t length, *tlvs, *end_of_tlvs;
        struct le_config_frame *frame;

        retval = read(fd, buff, MAXFRAME);
        if (retval < 0) return -1;

        frame = (struct le_config_frame *)buff;
        if (frame->status != 0) {
                        printf("mpcd: lecs.c: get_reply: config status %d\n", frame->status);
                        return -1;
        }
        if (frame->num_tlvs == 0) {
                        printf("mpcd: lecs.c: get_reply: no TLVS\n");
                        return -1;
        }

        tlvs = buff + sizeof(struct le_config_frame);
        end_of_tlvs = buff + retval;
        while (end_of_tlvs - tlvs >= 5 && frame->num_tlvs-- > 0) {
                type = (tlvs[0] << 24) | (tlvs[1] << 16) | (tlvs[2] << 8) | tlvs[3];
                length = tlvs[4];
                tlvs += 5;
                                /* Sampo-Add: start */
                switch(type){
                case TLV_SC_SETUP_FRAME_COUNT:
                        params->mpc_p1 = (*(tlvs+1)<<8) | (*tlvs);
                        params->mpc_p1 = htons(params->mpc_p1);
                        printf("mpcd: lecs.c: get_reply: MPC_p1 = %d\n",params->mpc_p1);
                        break;                            
                case TLV_SC_SETUP_FRAME_TIME:
                        params->mpc_p2 = (*(tlvs+1)<<8) | (*tlvs);
                        params->mpc_p2 = htons(params->mpc_p2);
                        printf("mpcd: lecs.c: get_reply: MPC_p2 = %d\n",params->mpc_p2);
                        break;
                case TLV_FLOW_DETECTION_PROTOCOLS:
                        memcpy(params->mpc_p3, tlvs, length);
                        printf("mpcd: lecs.c: get_reply: MPC_p3 = %s\n",params->mpc_p3);
                        break;
                case TLV_MPC_ININTIAL_RETRY_TIME:
                        params->mpc_p4 = (*(tlvs+1)<<8) | (*tlvs);
                        params->mpc_p4 = htons(params->mpc_p4);
                        printf("mpcd: lecs.c: get_reply: MPC_p4 = %d\n",params->mpc_p4);
                        break;
                case TLV_MPC_RETRY_TIME_MAXIMUM:
                        params->mpc_p5 = (*(tlvs+1)<<8) | (*tlvs);
                        params->mpc_p5 = htons(params->mpc_p5);
                        printf("mpcd: lecs.c: get_reply: MPC_p5 = %d\n",params->mpc_p5);
                        break;
                case TLV_HOLD_DOWN_TIME:
                        params->mpc_p6 = (*(tlvs+1)<<8) | (*tlvs);
                        params->mpc_p6 = htons(params->mpc_p6);
                        printf("mpcd: lecs.c: get_reply: MPC_p6 = %d\n",params->mpc_p6);
                        break;
                default:
                        printf("mpcd: lecs.c: get_reply: TLV type 0x%x\n", type);
                        break;
                        
                }
                tlvs += length;
                /* Sampo-Add: end */
        }
        if (end_of_tlvs - tlvs != 0)
                printf("mpcd: lecs.c: get_reply: ignoring %d bytes of trailing TLV carbage\n",
                       end_of_tlvs - tlvs);
        return 1;
}

static int get_lecs_socket(struct sockaddr_atmsvc *sa)
{
        int s, retval;
        struct atm_qos qos;
        struct atm_sap sap;

        s = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if (s < 0){
                printf("mpcd: lecs.c: socket failed: %s\n", strerror(errno));
                return -1;
        }
        memset(&qos, 0, sizeof(qos));
        memset(&sap, 0, sizeof(sap));
        qos.aal = ATM_AAL5;
        qos.txtp.traffic_class = ATM_UBR;
        qos.rxtp.traffic_class = ATM_UBR;
        qos.txtp.max_sdu = 1516;
        qos.rxtp.max_sdu = 1516;
        if (setsockopt(s, SOL_ATM,SO_ATMQOS, &qos, sizeof(qos)) < 0){
                printf("mpcd: lecs.c: setsockopt SO_ATMQOS failed: %s\n", strerror(errno));
                close(s);                      
                return -1;
        }

        sap.blli[0].l2_proto = ATM_L2_NONE;
        sap.blli[0].l3_proto = ATM_L3_TR9577;
        sap.blli[0].l3.tr9577.ipi = NLPID_IEEE802_1_SNAP;
        sap.blli[0].l3.tr9577.snap[0] = 0x00;
        sap.blli[0].l3.tr9577.snap[1] = 0xa0;
        sap.blli[0].l3.tr9577.snap[2] = 0x3e;
        sap.blli[0].l3.tr9577.snap[3] = 0x00;
        sap.blli[0].l3.tr9577.snap[4] = 0x01;
        if (setsockopt(s, SOL_ATM,SO_ATMSAP, &sap, sizeof(sap)) < 0) {
                printf("mpcd: lecs.c: setsockop SO_ATMSAP failed: %s\n", strerror(errno));
                close (s);
                return -1;
        }

        retval = connect(s, (struct sockaddr *)sa, sizeof(struct sockaddr_atmsvc));
        if (retval < 0) {
                printf("mpcd: lecs.c: connect failed: %s\n", strerror(errno));
                close (s);
                return -1;
        }

        return s;
}

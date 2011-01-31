#ifndef LECS_H
#define LECS_H

void get_mpc_config(struct sockaddr_atmsvc *sa, char *lec_addr, char *elan_name);

struct le_config_frame {
        uint16_t marker;
        uint8_t  protocol;
        uint8_t  version;
        uint16_t opcode;
        uint16_t status;
        uint32_t tran_id;
        uint16_t lecid;
        uint16_t flags;
        uint8_t  src_lan[8];
        uint8_t  target_lan[8];
        uint8_t  src_atm_addr[ATM_ESA_LEN];
        uint8_t  lan_type;
        uint8_t  max_frame_size;
        uint8_t  num_tlvs;
        uint8_t  elan_name_size;
        uint8_t  target_atm_addr[ATM_ESA_LEN];
        uint8_t  elan_name[32];
        /* TLVs if any */
} __attribute__ ((packed));

/* MPOA Configuration TLVs */
#define TLV_SC_SETUP_FRAME_COUNT     0x00a03e24  /* MPC_p1 */
#define TLV_SC_SETUP_FRAME_TIME      0x00a03e25  /* MPC_p2 */
#define TLV_FLOW_DETECTION_PROTOCOLS 0x00a03e26  /* MPC_p3 */
#define TLV_MPC_ININTIAL_RETRY_TIME  0x00a03e27  /* MPC_p4 */
#define TLV_MPC_RETRY_TIME_MAXIMUM   0x00a03e28  /* MPC_p5 */
#define TLV_HOLD_DOWN_TIME           0x00a03e29  /* MPC_p6 */

#endif /* LECS_H */

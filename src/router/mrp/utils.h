// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#ifndef UTILS_H
#define UTILS_H

#include <string.h>

#include <netinet/in.h>
#include <linux/if_bridge.h>
#include <asm/byteorder.h>
#include <stdbool.h>

#define LINUX_MRP_NETLINK "br_mrp_netlink"

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

enum mrp_ring_recovery_type {
	MRP_RING_RECOVERY_500,
	MRP_RING_RECOVERY_200,
	MRP_RING_RECOVERY_30,
	MRP_RING_RECOVERY_10,
};

enum mrp_in_recovery_type {
	MRP_IN_RECOVERY_500,
	MRP_IN_RECOVERY_200,
};

enum mrp_mrm_state_type {
	/* Awaiting Connection State 1 */
	MRP_MRM_STATE_AC_STAT1 = 0x0,
	/* Primary Ring port with Link Up */
	MRP_MRM_STATE_PRM_UP = 0x1,
	/* Check Ring, Ring Open State */
	MRP_MRM_STATE_CHK_RO = 0x2,
	/* Check Ring, Ring Closed State */
	MRP_MRM_STATE_CHK_RC = 0x3,
};

enum mrp_mrc_state_type {
	/* Awaiting Connection State 1 */
	MRP_MRC_STATE_AC_STAT1 = 0x0,
	/* Data Exchange Idle state */
	MRP_MRC_STATE_DE_IDLE = 0x1,
	/* Pass Through */
	MRP_MRC_STATE_PT = 0x2,
	/* Data Exchange */
	MRP_MRC_STATE_DE = 0x3,
	/* Pass Through Idle state */
	MRP_MRC_STATE_PT_IDLE = 0x4,
};

enum mrp_mim_state_type {
	/* Awaiting Connection State 1 */
	MRP_MIM_STATE_AC_STAT1 = 0x0,
	/* Check Interconnection, Interconnection Open state */
	MRP_MIM_STATE_CHK_IO = 0x1,
	/* Check Interconnection, Interconnection Closed state */
	MRP_MIM_STATE_CHK_IC = 0x2,
};

enum mrp_mic_state_type {
	/* Awaiting Connection State 1 */
	MRP_MIC_STATE_AC_STAT1 = 0x0,
	/* Pass Through */
	MRP_MIC_STATE_PT = 0x1,
	/* Interconnection Port Idle state */
	MRP_MIC_STATE_IP_IDLE = 0x2,
};

/* utils.c */
struct frame_buf {
	unsigned char *start;
	unsigned char *data;
	uint32_t size;
};

int if_get_mac(int ifindex, unsigned char *mac);
int if_get_link(int ifindex);
struct frame_buf *fb_alloc(uint32_t size);
void *fb_put(struct frame_buf *fb, uint32_t size);
void ether_addr_copy(uint8_t *dst, const uint8_t *src);
bool ether_addr_equal(const uint8_t *addr1, const uint8_t *addr2);
uint64_t ether_addr_to_u64(const uint8_t *addr);
uint32_t get_ms(void);

int if_init(void);
void if_cleanup(void);

/* server_socket/server_cmds/mrp */
struct ctl_msg_hdr
{
	int cmd;
	int lin;
	int lout;
	int res;
};

#define set_socket_address(sa, string) do{  \
	struct sockaddr_un * tmp_sa = (sa);     \
	memset(tmp_sa, 0, sizeof(*tmp_sa));     \
	tmp_sa->sun_family = AF_UNIX;           \
	strcpy(tmp_sa->sun_path + 1, (string)); \
}while(0)

#define MRP_SERVER_SOCK_NAME ".mrp_server"

#define MAX_MRP_INSTANCES 20
struct mrp_status {
	int br;
	int ring_nr;
	int pport;
	int sport;
	int mra_support;
	int ring_role;
	int ring_state;
	int prio;
	int ring_recv;
	int react_on_link_change;
	int in_role;
	int in_state;
	int iport;
	int in_id;
};

#define CTL_DECLARE(name) \
int CTL_ ## name name ## _ARGS

#define CMD_CODE_addmrp    101
#define addmrp_ARGS (int br, int ring_nr, int pport, int sport, int ring_role, \
		     uint16_t prio, uint8_t ring_recv, uint8_t react_on_link_change, \
		     int in_role, uint16_t in_id, int iport)
struct addmrp_IN
{
	int br;
	int pport;
	int sport;
	int ring_nr;
	int ring_role;
	int prio;
	int ring_recv;
	int react_on_link_change;
	int in_role;
	int in_id;
	int iport;
};
struct addmrp_OUT
{
};
#define addmrp_COPY_IN \
    ({                                                           \
     in->br = br;                                                \
     in->pport = pport;                                          \
     in->sport = sport;                                          \
     in->ring_nr = ring_nr;                                      \
     in->ring_role = ring_role;                                  \
     in->prio = prio;                                            \
     in->ring_recv = ring_recv;                                  \
     in->react_on_link_change = react_on_link_change;            \
     in->in_role = in_role;                                      \
     in->in_id = in_id;                                          \
     in->iport = iport;                                          \
     })
#define addmrp_COPY_OUT ({ (void)0; })
#define addmrp_CALL (in->br, in->ring_nr, in->pport, in->sport, in->ring_role,\
		     in->prio, in->ring_recv, in->react_on_link_change,\
		     in->in_role, in->in_id, in->iport)
CTL_DECLARE(addmrp);

#define CMD_CODE_delmrp    102
#define delmrp_ARGS (int br, int ring_nr)
struct delmrp_IN
{
	int br;
	int ring_nr;
};
struct delmrp_OUT
{
};
#define delmrp_COPY_IN \
    ({                                                           \
     in->br = br;                                                \
     in->ring_nr = ring_nr;                                      \
     })
#define delmrp_COPY_OUT ({ (void)0; })
#define delmrp_CALL (in->br, in->ring_nr)
CTL_DECLARE(delmrp);

#define CMD_CODE_getmrp    103
#define getmrp_ARGS (int *count, struct mrp_status *status)
struct getmrp_IN
{
};
struct getmrp_OUT
{
	int count;
	struct mrp_status status[MAX_MRP_INSTANCES];
};
#define getmrp_COPY_IN ({ (void)0; })
#define getmrp_COPY_OUT ({ *count = out->count;                  \
    memcpy(status, out->status, sizeof(struct mrp_status) * (*count)); })
#define getmrp_CALL (&out->count, out->status)
CTL_DECLARE(getmrp);

#define CLIENT_SIDE_FUNCTION(name)                               \
CTL_DECLARE(name)                                                \
{                                                                \
    struct name ## _IN in0, *in = &in0;                          \
    struct name ## _OUT out0, *out = &out0;                      \
    name ## _COPY_IN;                                            \
    int res = 0;                                                 \
    int r = client_send_message(CMD_CODE_ ## name, in,           \
                                sizeof(*in), out, sizeof(*out),  \
                                &res);                           \
    if(r || res)                                                 \
        printf("Got return code %d, %d\n", r, res);   \
    if(r)                                                        \
        return r;                                                \
    if(res)                                                      \
        return res;                                              \
    name ## _COPY_OUT;                                           \
    return 0;                                                    \
}

#define SERVER_MESSAGE_CASE(name)                            \
    case CMD_CODE_ ## name : do                              \
    {                                                        \
        struct name ## _IN in0, *in = &in0;                  \
        struct name ## _OUT out0, *out = &out0;              \
        if(sizeof(*in) != lin || sizeof(*out) != lout)       \
        {                                                    \
            printf("Bad sizes lin %d != %zd or lout %d != %zd", \
                lin, sizeof(*in), lout, sizeof(*out));       \
            return -1;                                       \
        }                                                    \
        memcpy(in, inbuf, lin);                              \
        int r = CTL_ ## name name ## _CALL;                  \
        if(r)                                                \
            return r;                                        \
        if(outbuf)                                           \
            memcpy(outbuf, out, lout);                       \
        return r;                                            \
    }while(0)


#endif /* UTILS_H_ */

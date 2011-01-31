/* frames.c - handle incoming frames, prefill outgoing frames, parse TLVs etc. */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

/* Functions for handling LANE control frames used when joining an
 * ELAN are in lec.c
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <atm.h>
#include <linux/atmlec.h>
#include <atmd.h>

#include "conn.h"
#include "lec.h"
#include "frames.h"
#include "display.h"
#include "kernel.h"

#define COMPONENT "frames.c"

static uint32_t transaction_id = 0;
static void extract_tlv_value(uint16_t opcode, uint32_t type, unsigned char *tlvp, int len);
static void handle_x5(uint16_t opcode);

/* Initializes LANE Control frame of type 'type'
 */
void prefill_frame(void *buff, uint16_t type)
{
        struct frame_hdr *header;
        
        memset(buff, 0, sizeof(struct ctrl_frame));
        header = (struct frame_hdr *)buff;
        header->marker   = htons(0xff00);
        header->protocol = 0x01;
        header->version  = 0x01;
        header->opcode   = htons(type);
        header->status   = htons(0x0000);
        header->tran_id  = htonl(transaction_id);
        header->lec_id   = htons(lec_params.c14_lec_id);
        header->flags    = htons(0x0000);
        
        transaction_id++;
        
        return;
}

/* Validates incoming Control frames except READY_IND and
 * READY_QUERY which do not start with the common header.
 * Also calls display_frame() to print out conforming frames.
 * Returns < 0 for error
 */
int validate_frame(unsigned char *buff, int size)
{
        struct ready_frame *hdr; /* Ready is the shortest possible */

        if (size < sizeof(struct ready_frame)) {
                diag(COMPONENT, DIAG_DEBUG, "short frame, size %d", size);
                return -1;
        }

        hdr = (struct ready_frame *)buff;
        if (hdr->marker   != htons(0xff00) ||
            hdr->protocol != 0x01 ||
            hdr->version  != 0x01)
                return -1;

        /* READY_* frames are shorter than others */
        if (hdr->opcode == htons(READY_QUERY) ||
            hdr->opcode == htons(READY_IND)) {
                diag(COMPONENT, DIAG_DEBUG, "Received a %s", opcode2text(hdr->opcode));
                return 0;
        }

        if (size < sizeof(struct ctrl_frame)) {
                diag(COMPONENT, DIAG_DEBUG, "short frame, size %d", size);
                return -1;
        }

        display_frame(buff);

        return 0;
}

/* Handle incoming LE_FLUSH_REQUEST frames.
 */
static void handle_flush_req(struct ctrl_frame *f)
{
        if (memcmp(lec_params.c1n_my_atm_addr, f->target_atm_addr, ATM_ESA_LEN) != 0)
                return;
        f->header.opcode = htons(LE_FLUSH_RSP);
        if (send_frame(lec_params.ctrl_direct, f, sizeof(struct ctrl_frame)) < 0)
                diag(COMPONENT, DIAG_DEBUG, "could not send LE_FLUSH_RESPONSE");

        return;
}

/* Handle incoming LE_FLUSH_RESPONSE frames.
 */
static void handle_flush_rsp(struct ctrl_frame *f)
{
        struct atmlec_msg msg;

        if (f->header.lec_id != htons(lec_params.c14_lec_id)) {
                diag(COMPONENT, DIAG_DEBUG, "Wrong lec_id, ignoring");
                return;
        }

        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_flush_complete;
        msg.content.normal.flag = ntohl(f->header.tran_id);
        msg_to_kernel(&msg, sizeof(struct atmlec_msg));
        
        return;
}

/* Handle incoming READY_QUERY frames.
 */
static void handle_ready_query(Conn_t *conn, struct ready_frame *f)
{
        f->opcode = htons(READY_IND);
        send_frame(conn, f, sizeof(struct ready_frame));

        return;
}

/* Helper for handle_le_arp_req.
 * If the target_lan_dst was not our MAC address, try to
 * see if the bridging table in the kernel knows about it.
 * Returns < 0 for serious error
 */
static int check_bridge(struct ctrl_frame *frame, int size)
{
        struct atmlec_msg msg;

        if (lec_params.c4_proxy_flag == 0)
                return 0;

        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_should_bridge;
        memcpy(msg.content.proxy.mac_addr, frame->target_lan_dst.mac, ETH_ALEN);
        memcpy(msg.content.proxy.atm_addr, frame->src_atm_addr, ATM_ESA_LEN);
        msg.content.proxy.tran_id = frame->header.tran_id;
        msg.content.proxy.lec_id = frame->header.lec_id;

        return msg_to_kernel(&msg, sizeof(struct atmlec_msg));
}

/* Handles incoming LE_ARP_REQ and targetless LE_ARP_REQ.
 * See LANEv2, 7.1.5 and 7.1.30
 * Returns < 0 for serious error
 */
static int handle_le_arp_req(struct ctrl_frame *frame, int size)
{
        int sizeoftlvs, sizeofrsp, retval;
        struct ctrl_frame *rsp;
        struct atmlec_msg *msg;

        if (frame->header.lec_id == htons(lec_params.c14_lec_id)) {
                diag(COMPONENT, DIAG_DEBUG, "Ignoring own LE_ARP_REQUEST");
                return 0;
        }

        retval = 0;
        if (frame->target_lan_dst.tag == htons(LAN_DST_MAC_ADDR)) {
                if (memcmp(frame->target_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN) != 0)
                        return (check_bridge(frame, size)); /* target was not us */
                sizeofrsp = sizeof(struct ctrl_frame) + lec_params.sizeoftlvs;
                rsp = (struct ctrl_frame *)malloc(sizeofrsp);
                if (rsp == NULL) return 0;
                memcpy(rsp, frame, sizeof(struct ctrl_frame));
                rsp->header.opcode = htons(LE_ARP_RSP);
                memcpy(rsp->target_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
                rsp->num_tlvs = lec_params.num_tlvs;
                if (lec_params.num_tlvs > 0)
                        memcpy(rsp + 1, lec_params.tlvs, lec_params.sizeoftlvs);

                retval = send_frame(lec_params.ctrl_direct, rsp, sizeofrsp);
                free(rsp);
        } else if (frame->target_lan_dst.tag == htons(LAN_DST_NOT_PRESENT) &&
                   lec_params.c29_v2_capable) {
                sizeoftlvs = size - sizeof(struct ctrl_frame);
                msg = (struct atmlec_msg *)malloc(sizeof(struct atmlec_msg) + sizeoftlvs);
                if (msg == NULL) return -1;
                memset(msg, 0, sizeof(struct atmlec_msg));
                msg->type = l_arp_update;
                memcpy(msg->content.normal.mac_addr, frame->src_lan_dst.mac, ETH_ALEN);
                memcpy(msg->content.normal.atm_addr, frame->src_atm_addr, ATM_ESA_LEN);
                msg->content.normal.flag = (frame->header.flags & htons(REMOTE_ADDRESS)) ? 1 : 0;
                msg->content.normal.targetless_le_arp = 1;
                msg->sizeoftlvs = sizeoftlvs;
                if (sizeoftlvs > 0) memcpy(msg + 1, frame + 1, sizeoftlvs);

                retval = msg_to_kernel(msg, sizeof(struct atmlec_msg) + sizeoftlvs);
                free(msg);
        }
        
        return retval;
}

/* Handles incoming LE_NARP_REQUESTS frames.
 * Mandatory only in LANEv2. If we are LANEv1, we'll just ignore these.
 * See LANEv2, 7.1.31-35 LE_NARP_REQUEST/RESPONSE.
 * For no-source, i.e. no source ATM address, we remove the LE_ARP cache entry.
 * If the source is non-zero, we first remove the entry
 * and then add the new entry in the LE_ARP cache.
 * Returns < 0 for serious error.
 */
static int handle_narp_req(struct ctrl_frame *frame, int size)
{
        int sizeoftlvs, no_source = 0, retval;
        struct atmlec_msg *msg;
        unsigned char empty[ATM_ESA_LEN];

        if (frame->header.lec_id == htons(lec_params.c14_lec_id) ||
            lec_params.c29_v2_capable == 0) {
                diag(COMPONENT, DIAG_DEBUG, "Ignoring LE_NARP_REQUEST");
                return 0;
        }

        memset(empty, 0, ATM_ESA_LEN);
        if (memcmp(empty, frame->src_atm_addr, ATM_ESA_LEN) == 0)
                no_source = 1;

        sizeoftlvs = size - sizeof(struct ctrl_frame);
        msg = (struct atmlec_msg *)malloc(sizeof(struct atmlec_msg) + sizeoftlvs);
        if (msg == NULL) return -1;
        memset(msg, 0, sizeof(struct atmlec_msg));
        msg->type = l_narp_req;
        memcpy(msg->content.normal.mac_addr, frame->src_lan_dst.mac, ETH_ALEN);
        memcpy(msg->content.normal.atm_addr, frame->src_atm_addr, ATM_ESA_LEN);
        msg->content.normal.flag = (frame->header.flags & htons(REMOTE_ADDRESS)) ? 1 : 0;
        msg->content.normal.no_source_le_narp = no_source;
        msg->sizeoftlvs = sizeoftlvs;
        if (sizeoftlvs > 0) memcpy(msg + 1, frame + 1, sizeoftlvs);
        retval = msg_to_kernel(msg, sizeof(struct atmlec_msg) + sizeoftlvs);
        
        free(msg);
        
        return retval;
}

/* Handles incoming LE_ARP_RESPONSE frames.
 * Returns < 0 for serious error
 */
static int handle_arp_rsp(struct ctrl_frame *frame, int size)
{
        int sizeoftlvs, msglen, retval;
        char buff[MAX_CTRL_FRAME];
        struct atmlec_msg *msg;
        
        if (frame->header.lec_id != htons(lec_params.c14_lec_id)) {
                diag(COMPONENT, DIAG_DEBUG, "Wrong lec_id, ignoring");
                return 0;
        }

        sizeoftlvs = size - sizeof(struct ctrl_frame);
        msglen = sizeof(struct atmlec_msg) + sizeoftlvs;
        msg = (struct atmlec_msg *)buff;
        memset(msg, 0, msglen);
        
        msg->type = l_arp_update;
        memcpy(msg->content.normal.mac_addr, frame->target_lan_dst.mac, ETH_ALEN);
        memcpy(msg->content.normal.atm_addr, frame->target_atm_addr, ATM_ESA_LEN);
        msg->content.normal.flag = (frame->header.flags & htons(REMOTE_ADDRESS)) ? 1 : 0;
        msg->sizeoftlvs = sizeoftlvs;
        if (sizeoftlvs > 0) memcpy(msg + 1, frame + 1, sizeoftlvs);

        retval = msg_to_kernel(msg, msglen);

        return retval;
}

/* Handles incoming LE_TOPOLOGY_REQUEST frames.
 * Returns < 0 for serious error
 */
static int handle_topo_req(struct ctrl_frame *frame)
{
        struct atmlec_msg msg;

        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_topology_change;
        if (frame->header.flags & htons(TOPO_CHANGE))
                msg.content.normal.flag = 1;

        return(msg_to_kernel(&msg, sizeof(struct atmlec_msg)));
}

static void handle_ready_ind(Conn_t *conn)
{
	struct atmlec_msg msg;

	/* FIXME -- if its a receive only vcc we should not do this */

        diag(COMPONENT, DIAG_DEBUG, "READY_IND, on fd %d; sending LE_FLUSH_REQ", conn->fd);
        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_flush_tran_id;
        memcpy(msg.content.normal.atm_addr, conn->atm_address, ATM_ESA_LEN);
        msg.content.normal.flag = send_flush_req(conn);

        msg_to_kernel(&msg, sizeof(struct atmlec_msg));
}

/* Processes and validates incoming frames. Calls frame
 * dependant handler functions.
 * Returns < 0 for serious error
 */
int handle_frame(Conn_t *conn, char *buff, int size)
{
        struct ctrl_frame *frame;

        if (validate_frame(buff, size) < 0)
                return 0;

        frame = (struct ctrl_frame *)buff;

        switch (ntohs(frame->header.opcode)) {
        case LE_FLUSH_REQ:
                handle_flush_req(frame);
                break;
        case LE_FLUSH_RSP:
                diag(COMPONENT, DIAG_DEBUG, "LE_FLUSH_RESPONSE, on fd %d", conn->fd);
                handle_flush_rsp(frame);
                break;
        case READY_QUERY:
                handle_ready_query(conn, (struct ready_frame *)frame);
                break;
        case READY_IND:
                handle_ready_ind(conn);
                break;
        case LE_ARP_REQ:
                if (handle_le_arp_req(frame, size) < 0)
                        return -1;
                break;
        case LE_ARP_RSP:
                if (handle_arp_rsp(frame, size) < 0)
                        return -1;
                break;
        case LE_TOPO_REQ:
                if (handle_topo_req(frame) < 0)
                        return -1;
                break;
        case LE_REG_RSP:
                /* FIXME: Should we do something? */
                break;
        case LE_NARP_REQ:
                if (handle_narp_req(frame, size) < 0)
                        return -1;
                break;
        default:
                diag(COMPONENT, DIAG_ERROR,
                     "Unknown frame, opcode 0x%x %s", ntohs(frame->header.opcode),
                     opcode2text(frame->header.opcode));
                break;
        }

        return 0;
}

/* Sends a READY_INDICATION when a non-blocking connect completes.
 */
void send_ready_ind(Conn_t *conn)
{
        struct ready_frame frame;
        int retval;

        frame.marker   = htons(0xff00);
        frame.protocol = 0x01;
        frame.version  = 0x01;
        frame.opcode   = htons(READY_IND);

        retval = send_frame(conn, &frame, sizeof(struct ready_frame));
        if (retval < 0)
                diag(COMPONENT, DIAG_DEBUG, "Could not send READY_IND, fd %d", conn->fd);

        return;
}

/* Sends a LE_FLUSH_REQUEST
 * Returns the transaction used with this flush REQ/RSP pair.
 */
uint32_t send_flush_req(Conn_t *conn)
{
        struct ctrl_frame frame;

        prefill_frame(&frame, LE_FLUSH_REQ);
        memcpy(frame.src_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        memcpy(frame.target_atm_addr, conn->atm_address, ATM_ESA_LEN);

        send_frame(lec_params.mcast_send, &frame, sizeof(struct ctrl_frame));

        return ntohl(frame.header.tran_id);
}

/* Registers our MAC address and associated TLVs with LES.
 * See LANEv2, 6. Registaration Protocol
 */
void send_register_req(void)
{
        char buff[MAX_CTRL_FRAME];
        struct ctrl_frame *frame;

        frame = (struct ctrl_frame *)buff;
        prefill_frame(frame, LE_REG_REQ);
        frame->src_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
        memcpy(frame->src_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN);
        memcpy(frame->src_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        frame->num_tlvs = lec_params.num_tlvs;
        if (lec_params.sizeoftlvs > 0)
                memcpy((frame + 1), lec_params.tlvs, lec_params.sizeoftlvs);

        send_frame(lec_params.ctrl_direct, frame, sizeof(struct ctrl_frame) + lec_params.sizeoftlvs);
        
        return;
}

/* Goes through the TLVs trailing a frame while passing them
 * one by one to the TLV handler.
 */
void parse_tlvs(uint16_t opcode, unsigned char *tlvp, int numtlvs, int sizeoftlvs)
{
        uint32_t type;
        uint8_t len;
        unsigned char *end_of_tlvs;

        end_of_tlvs = tlvp + sizeoftlvs;
        while (numtlvs-- > 0 && end_of_tlvs - tlvp >= 5) {
                type = *(uint32_t *)tlvp;
                type = ntohl(type);
                len  = tlvp[4];
                tlvp += 5;
                diag(COMPONENT, DIAG_DEBUG, "parse_tlvs: type %s len %d",
                     tlv2text(type), len);
                if (tlvp + len > end_of_tlvs)
                        return; /* value too long */
                
                extract_tlv_value(opcode, type, tlvp, len);
                tlvp += len;
        }

        return;
}

/* Does something depending on the frame type this TLV arrived with,
 * TLV type, and contents of TLV.
 */
static void extract_tlv_value(uint16_t opcode, uint32_t type, unsigned char *tlvp, int len)
{

        uint16_t value16;
        uint32_t value32;

        /* LE_JOIN_RESPONSE does not support all the TLVs */
        if (opcode == htons(LE_JOIN_RSP) && type != htonl(ELAN_ID))
                return;

        switch(len) {
        case 0:
                switch (type) {
                case X5_ADJUSTMENT:
                        handle_x5(opcode);
                        break;
                default:
                        goto whine;
                        break;
                }
                break;
        case 2:
                value16 = *(uint16_t *)tlvp;
                value16 = ntohs(value16);
                diag(COMPONENT, DIAG_DEBUG, "value of TLV %d", value16);
                switch (type) {
                case MAX_CUM_CTRL_TIMEOUT:
                        lec_params.c7_ctrl_timeout = value16;
                        break;
                case MAX_UNKNOWN_FRAME_CNT:
                        lec_params.c10_max_unknown_frames = value16;
                        break;
                case MAX_UNKNOWN_FRAME_TIME:
                        lec_params.c11_max_unknown_frame_time = value16;
                        break;
                case MAX_RETRY_COUNT:
                        lec_params.c13_max_retry_count = value16;
                        break;
                case FORWARD_DELAY_TIME:
                        lec_params.c18_forward_delay_time = value16;
                        break;
                case EXPECTED_LE_ARP_TIME:
                        lec_params.c20_le_arp_response_time = value16;
                        break;
                case FLUSH_TIMEOUT:
                        lec_params.c21_flush_timeout = value16;
                        break;
                case PATH_SWITCHING_DELAY:
                        lec_params.c22_path_switching_delay = value16;
                        break;
                case LOCAL_SEGMENT_ID:
                case DEF_MCAST_SND_VCC_TYPE:
                case CONN_COMPLETION_TIMER:
                case SERVICE_CATEGORY:
                        /* do nothing */
                        break;
                default:
                        goto whine;
                        break;
                }
                break;
        case 4:
                value32 = *(uint32_t *)tlvp;
                value32 = ntohl(value32);
                diag(COMPONENT, DIAG_DEBUG, "value of TLV %d", value32);
                switch (type) {
                case VCC_TIMEOUT_PERIOD:
                        lec_params.c12_vcc_timeout = value32;
                        break;
                case AGING_TIME:
                        lec_params.c17_aging_time = value32;
                        break;
                case ELAN_ID:
                        lec_params.c31_elan_id = value32;
                        break;
                case DEF_MCAST_SND_VCC_AVG:
                case DEF_MCAST_SEND_PEAK_RT:
                        /* do nothing */
                        break;
                default:
                        goto whine;
                        break;
                }
                break;
        case 20:
                switch(type) {
                case PREFERRED_LES:
                        memcpy(lec_params.c35_preferred_les, tlvp, ATM_ESA_LEN);
                        lec_params.c35_contains_address = 1;
                        break;
                case LLC_MUXED_ATM_ADDR:
                        /* do nothing */
                        break;
                default:
                        goto whine;
                        break;
                }
                break;
        default:
                /* handle variable length TLVs */
                switch(type) {
                case CONFIG_FRAG_INFO:
                        diag(COMPONENT, DIAG_INFO, "Got Config-Frag-Info TLV");
                        break;
                case LAYER3_ADDRESS:
                        /* do nothing */
                        break;
                default:
                        goto whine;
                        break;
                }
                break;
        }

        return;

 whine:
        diag(COMPONENT, DIAG_DEBUG, "Unknown TLV, type 0x%x, len %d", type, len);

        return;
}

/* Figures out what to do when we get a X5-Adjustment TLV
 */
static void handle_x5(uint16_t opcode)
{
        if (!lec_params.c29_v2_capable)
                return;
        if (opcode != ntohs(LE_CONFIG_RSP)) {
                diag(COMPONENT, DIAG_WARN, "X5-Adjustment TLV received but not with LE_CONFIG_RSP");
                return;
        }

        switch(lec_params.c3_max_frame_size) {
        case 1:
                lec_params.c3_max_frame_size = 5; /* 1580 */
                return;
                break;
        case 2:
                lec_params.c3_max_frame_size = 5; /* 1580 */
                return;
                break;
        default:
                /* rest of the values are not affected by X5 */
                break;
        }

        return;
}

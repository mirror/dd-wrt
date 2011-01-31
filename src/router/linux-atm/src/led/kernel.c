/* kernel.c - send and receive messages from kernel and act accordingly */

/*
 * Marko Kiiskila carnil@cs.tut.fi 
 * 
 * Copyright (c) 1996
 * Tampere University of Technology - Telecommunications Laboratory
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this
 * software and its documentation is hereby granted,
 * provided that both the copyright notice and this
 * permission notice appear in all copies of the software,
 * derivative works or modified versions, and any portions
 * thereof, that both notices appear in supporting
 * documentation, and that the use of this software is
 * acknowledged in any publications resulting from using
 * the software.
 * 
 * TUT ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION AND DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS
 * SOFTWARE.
 * 
 */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

/* Atm includes */
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/atmlec.h>

#include <atmd.h>

/* Local includes */
#include "kernel.h"
#include "lec.h"
#include "frames.h"

#define COMPONENT "kernel.c"

/* Local vars */
static int lec_socket;

static const char* get_mesg_type_str(atmlec_msg_type type);
static void send_le_arp_req(unsigned char *mac_addr, int is_rdesc, int sizeoftlvs);
static void send_proxy_arp_rsp(unsigned char *target_mac, uint32_t tran_id,
                               uint16_t lec_id, unsigned char *src_atm_addr);
static void send_topology_req(int flag);
static void associate_tlvs(int sizeoftlvs);

/* Notify kernel that zeppelin is present and tell
 * kernel what MAC address this LEC uses
 */
int kernel_init(unsigned char *mac_addr, int itf)
{
        int rvalue;
        struct atmlec_msg msg;

        lec_socket = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if (lec_socket < 0) {
                diag(COMPONENT, DIAG_FATAL, "Kernel socket creation failed: %s",
                     strerror(errno));
                return -1;
        }
        itf = ioctl(lec_socket, ATMLEC_CTRL, itf);
        if (itf < 0) {
                diag(COMPONENT, DIAG_FATAL, "Socket ioctl to lecd_ctrl failed:%s",
                     strerror(errno));
                close(lec_socket);
                return -1;
        }
        msg.type = l_set_mac_addr;
        memcpy(msg.content.normal.mac_addr, mac_addr, ETH_ALEN);
        
        rvalue = write(lec_socket, &msg, sizeof(msg));
        if (rvalue < 0) {
                diag(COMPONENT, DIAG_FATAL, "Can't write mac address to LEC:%s",
                     strerror(errno));
                close(lec_socket);
                return -1;
        }
        diag(COMPONENT, DIAG_DEBUG, "Kernel interface initialized");
        
        if (conn_set_kernel_socket(lec_socket) < 0) {
                close(lec_socket);
                return -1;
        }
        
        return itf;
}

/* Send a message to kernel
 * Returns < 0 for error
 */
int msg_to_kernel(struct atmlec_msg *msg, int msg_size)
{
        int retval;

        diag(COMPONENT, DIAG_DEBUG, "msg_to_kernel, type %s", get_mesg_type_str(msg->type));
        retval = write(lec_params.kernel->fd, msg, msg_size);
        if (retval < 0) {
                diag(COMPONENT, DIAG_ERROR, "msg_to_kernel: write: %s", strerror(errno));
                return -1;
        }
        if (retval != msg_size) {
                diag(COMPONENT, DIAG_ERROR, "msg_to_kernel: partial write");
                return -1;
        }

        return retval;
}

/* Read and process message from kernel
 * Returns < 0 for error
 */
int msg_from_kernel(void)
{
        int retval;
        struct atmlec_msg msg;
        int codepoint, is_rdesc = 0;
        uint8_t bcast;

        retval = read(lec_params.kernel->fd, &msg, sizeof(struct atmlec_msg));
        if (retval < 0) {
                diag(COMPONENT, DIAG_FATAL, "msg_from_kernel: read: %s", strerror(errno));
                return -1;
        }
        if (retval != sizeof(struct atmlec_msg)) {
                diag(COMPONENT, DIAG_ERROR, "msg_from_kernel: msg size != sizeof(atmlec_msg)");
                return 0;
        }

        diag(COMPONENT, DIAG_DEBUG, "msg_from_kernel: type %s", get_mesg_type_str(msg.type));
        switch (msg.type) {
        case l_rdesc_arp_xmt:
                is_rdesc = 1;
                /* fall through */
        case l_arp_xmt:
                if (msg.content.normal.targetless_le_arp)
                        send_le_arp_req(NULL, 0, msg.sizeoftlvs);
                else
                        send_le_arp_req(msg.content.normal.mac_addr, is_rdesc, msg.sizeoftlvs);
                break;
        case l_svc_setup:
                bcast = (lec_params.c2_lan_type == LAN_TYPE_8025) ? 0x80 : 0x01; /* TR is MSB */
                codepoint = (msg.content.normal.mac_addr[0] & bcast) ? MCAST_CONN : DATA_DIRECT_CONN;
                if (create_data_svc(msg.content.normal.atm_addr, codepoint) < 0)
                        diag(COMPONENT, DIAG_ERROR, "Data direct VCC failed");
                break;
        case l_associate_req:
                associate_tlvs(msg.sizeoftlvs);
                break;
        case l_should_bridge:
                send_proxy_arp_rsp(msg.content.proxy.mac_addr, msg.content.proxy.tran_id,
                                   msg.content.proxy.lec_id, msg.content.proxy.atm_addr);
                break;
        case l_topology_change:
                send_topology_req(msg.content.normal.flag);
                break;
        default:
                diag(COMPONENT, DIAG_ERROR, "no handler for kernel msg %s", get_mesg_type_str(msg.type));
                break;
        }

        return 0;
}

/* Send LE_ARP_REQUEST. If mac_addr is NULL, sends a targetless LE_ARP.
 * If sizeoftlvs != 0, reads TLVs waiting in the kernel socket and
 * adds them to the frame. If sizeoftlvs == 0 then the TLVs (if any)
 * associated with this LEC are used.
 * FIXME: add TLV count in kernel messages
 */
static void send_le_arp_req(unsigned char *mac_addr, int is_rdesc, int sizeoftlvs)
{
        struct ctrl_frame *frame;
        int frame_size;
        char buff[MAX_CTRL_FRAME];

        diag(COMPONENT, DIAG_DEBUG, "Sending LE_ARP_REQUEST");

        if (sizeoftlvs == 0) frame_size = sizeof(struct ctrl_frame) + lec_params.sizeoftlvs;
        else frame_size = sizeof(struct ctrl_frame) + sizeoftlvs;
        frame = (struct ctrl_frame *)buff;
        memset(frame, 0, frame_size);

        prefill_frame(frame, LE_ARP_REQ);
        frame->header.lec_id = htons(lec_params.c14_lec_id);
        frame->src_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
        memcpy(frame->src_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN);
        memcpy(frame->src_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        if (mac_addr != NULL) {
                memcpy(frame->target_lan_dst.mac, mac_addr, ETH_ALEN);
                if (is_rdesc)
                        frame->target_lan_dst.tag = htons(LAN_DST_ROUTE_DESC);
                else
                        frame->target_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
        } else diag(COMPONENT, DIAG_DEBUG, "Sending targetless LE_ARP");

        if (sizeoftlvs != 0) {
                if (read(lec_params.kernel->fd, (frame + 1), sizeoftlvs) < 0) {
                        diag(COMPONENT, DIAG_ERROR, "reading TLVs from kernel failed: %s", strerror(errno));
                        return;
                }
                frame->num_tlvs++;
        } else if (lec_params.sizeoftlvs != 0) {
                memcpy((frame + 1), lec_params.tlvs, lec_params.sizeoftlvs);
                frame->num_tlvs += lec_params.num_tlvs;
        }

        send_frame(lec_params.ctrl_direct, frame, frame_size);

        return;
}

/* Associate the set of TLVs available in kernel socket
 * with this LEC. The old TLVs, if any, are removed.
 * Also, send a LE_REGISTER_REQUEST to register the TLVs
 * we just got.
 */
static void associate_tlvs(int sizeoftlvs)
{

        if (lec_params.tlvs != NULL) free (lec_params.tlvs);
        lec_params.sizeoftlvs = 0;
        lec_params.num_tlvs = 0;
        lec_params.tlvs = malloc(sizeoftlvs);
        if (lec_params.tlvs == NULL) {
                diag(COMPONENT, DIAG_ERROR, "Could not associate TLVs, out of memory");
                lec_params.sizeoftlvs = 0;
                lec_params.num_tlvs = 0;
                return;
        }
        if (read(lec_params.kernel->fd, lec_params.tlvs, sizeoftlvs) < 0) {
                diag(COMPONENT, DIAG_ERROR, "reading TLVs from kernel failed: %s", strerror(errno));
                return;
        }

        lec_params.sizeoftlvs = sizeoftlvs;
        lec_params.num_tlvs = 1; /* FIXME, add TLV count in messages */

        send_register_req();

        return;
}

/* Send a LE_ARP_RESPONSE for a LAN destination (MAC address) when
 * the LAN destination is present in kernel bridging table and we
 * are acting as a proxy lane client
 */
static void send_proxy_arp_rsp(unsigned char *target_mac, uint32_t tran_id,
                               uint16_t lec_id, unsigned char *src_atm_addr)
{
        struct ctrl_frame *frame;
        int frame_size;

        frame_size = sizeof(struct ctrl_frame) + lec_params.sizeoftlvs;
        frame = malloc(frame_size);
        if (frame == NULL) return;
        memset(frame, 0, frame_size);
        prefill_frame(frame, LE_ARP_RSP);
        frame->header.tran_id = tran_id;
        frame->header.lec_id = lec_id;
        frame->header.flags = htons(REMOTE_ADDRESS);
        memcpy(frame->target_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        frame->target_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
        memcpy(frame->target_lan_dst.mac, target_mac, ETH_ALEN);
        memcpy(frame->src_atm_addr, src_atm_addr, ATM_ESA_LEN);
        frame->num_tlvs = lec_params.num_tlvs;
        if (lec_params.num_tlvs > 0)
                memcpy(frame + 1, lec_params.tlvs, lec_params.sizeoftlvs);

        if (send_frame(lec_params.ctrl_direct, frame, frame_size) < 0)
                diag(COMPONENT, DIAG_ERROR, "send_proxy_arp_rsp: send_frame() failed");
	free(frame);

        return;
}

/* 7.1.25 Send a LE_TOPOLOGY_REQUEST */
static void send_topology_req(int flag)
{
        struct ctrl_frame frame;

        prefill_frame(&frame, LE_TOPO_REQ);
        if (flag) frame.header.flags = htons(TOPO_CHANGE);

        send_frame(lec_params.ctrl_direct, &frame, sizeof(struct ctrl_frame));

        return;
}

static const char *get_mesg_type_str(atmlec_msg_type type)
{
        switch(type) {
        case l_set_mac_addr:
                return "SET_MAC_ADDR";
        case l_del_mac_addr:
                return "DEL_MAC_ADDR";
        case l_svc_setup:
                return "SVC_SETUP";
        case l_arp_xmt:
                return "ARP_XMT";
        case l_addr_delete:
                return "ADDR_DELETE";
        case l_topology_change:
                return "TOPOLOGY_CHANGE";
        case l_flush_tran_id:
                return "FLUSH_TRANSACTION_ID";
        case l_flush_complete:
                return "FLUSH_COMPLETE";
        case l_arp_update:
                return "ARP_UPDATE";
        case l_config:
                return "CONFIG";
        case l_associate_req:
                return "LANE2_ASSOCIATE_REQ";
        case l_set_lecid:
                return "SET_LEC_ID";
        case l_narp_req:
                return "LE_NARP_REQUEST";
        case l_should_bridge:
                return "SHOULD_BRIDGE";
        default:
                return "<Unknown message type>";
        }
}

int get_lecsaddr(int itf, struct sockaddr_atmsvc *addr)
{
	int fd, howmany;
	struct atmif_sioc req;
	char txt[80];

#ifdef ATM_GETLECSADDR
	fd = socket(AF_ATMSVC, SOCK_DGRAM, 0);
	if (fd < 0) {
		diag(COMPONENT, DIAG_ERROR, "could not create AF_ATMSVC socket");
		return -1;
	}
	req.number = itf;
	req.arg = addr;
	req.length = sizeof( struct sockaddr_atmsvc );

	if (ioctl(fd, ATM_GETLECSADDR, &req) < 0) {
		diag(COMPONENT, DIAG_DEBUG, "ioctl ATM_GETLECSADDR failed" );
		close(fd);
		return -1;
	}
	close(fd);
	howmany = req.length;
	if (howmany == 0) {
		diag(COMPONENT, DIAG_ERROR, "No LECS address registered");
		return -1;
	}
	if (addr->sas_addr.pub[0] != 0){
		diag( COMPONENT, DIAG_ERROR, "Public address???" );
		addr->sas_addr.pub[0] = 0;
	}
	atm2text(txt, 80, (struct sockaddr *)addr, 0);
	diag(COMPONENT, DIAG_DEBUG, "discovered LECS address %s", txt);
	return howmany;
#else
	return -1;
#endif
}

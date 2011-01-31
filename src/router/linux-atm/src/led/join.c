/* join.c - functions which are only needed when joining an ELAN */

/* Copyright (C) 1999 Heikki Vatiainen hessu@cs.tut.fi */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h> /* for rand() */
#include <sys/ioctl.h>
#include <atm.h>

#include <linux/atmlec.h>

#include <atmd.h>

#include "conn.h"
#include "lec.h"
#include "join.h"
#include "frames.h"
#include "display.h"

#define COMPONENT "lec.c"

struct lec_params lec_params;

static unsigned char bus_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static unsigned char well_known_lecs[ATM_ESA_LEN] = {0x47,0x00,0x79,0x00,0x00,
                                                     0x00,0x00,0x00,0x00,0x00,
                                                     0x00,0x00,0x00,0x00,0xA0,
                                                     0x3E,0x00,0x00,0x01,0x00};
static int do_lec_configure(Conn_t *conn);
static int send_config_req(Conn_t *conn);
static int read_config_rsp(Conn_t *conn, char *buff, int buffsize);
static int parse_config_rsp(unsigned char *buff, int size);

static int send_join_req(Conn_t *conn);
static int read_join_rsp(char *buff, int buffsize);
static int parse_join_rsp(unsigned char *buff, int size);

static int get_bus_addr(struct sockaddr_atmsvc *addr);
static int read_bus_arp(Conn_t *conn, struct sockaddr_atmsvc *addr, char *buff, int buffsize);

/*
 * 5.1, Initial state
 */
void init_lec_params(unsigned char *mac_addr, char *elan_name,
                     unsigned char *listen_addr, int itf, char *foreId,
                     int max_frame_size, int proxy_flag, int lane_version)
{
        memcpy(lec_params.c6_mac_addr, mac_addr, ETH_ALEN);
        strcpy(lec_params.c5_elan_name, elan_name);
        memcpy(lec_params.c1n_my_atm_addr, listen_addr, ATM_ESA_LEN);
        lec_params.itf_num = itf;
        strcpy(lec_params.foreId, foreId);
        lec_params.c3_max_frame_size = max_frame_size;
        lec_params.c4_proxy_flag = proxy_flag;

        if (lane_version > 1 ) lec_params.c29_v2_capable = 1;
        else lec_params.c29_v2_capable = 0;
        
        /* then come the defaults */
        lec_params.c2_lan_type = LAN_TYPE_UNSPEC; /* Unspecified, implies Ethernet */
        if (lec_params.c29_v2_capable) {
                lec_params.c7_ctrl_timeout = 30;
                lec_params.c10_max_unknown_frames = 10;
        }
        else {
                lec_params.c7_ctrl_timeout = 10;
                lec_params.c10_max_unknown_frames = 1;
        }
        lec_params.c7i_initial_ctrl_timeout = 5;
        lec_params.c7x_timeout_multiplier = 2;
        lec_params.c7c_current_timeout = lec_params.c7i_initial_ctrl_timeout;
        lec_params.c11_max_unknown_frame_time = 1;
        lec_params.c12_vcc_timeout            = 1200;
        lec_params.c13_max_retry_count        = 2;
        lec_params.c14_lec_id                 = 0;
        lec_params.c17_aging_time             = 300;
        lec_params.c18_forward_delay_time     = 15;
        lec_params.c19_topology_change        = 0;
        lec_params.c20_le_arp_response_time   = 1;
        lec_params.c21_flush_timeout          = 4;
        lec_params.c22_path_switching_delay   = 6;
        /* LANE2 added the following, only the ones used are listed */
        memset(lec_params.c35_preferred_les, 0, ATM_ESA_LEN);
        lec_params.c35_contains_address       = 0;
        lec_params.c37_min_reconfig_delay     = 1;    /* milliseconds */
        lec_params.c38_max_reconfig_delay     = 5000; /* milliseconds */

        if (lec_params.tlvs != NULL) free (lec_params.tlvs);
        lec_params.tlvs       = NULL;
        lec_params.sizeoftlvs = 0;
        lec_params.num_tlvs   = 0;

        return;
}

/* ------------- Configure phase specific stuff starts ------------- */

/*
 * 5.2 LECS connect phase
 * Returns < 0 for error
 */
int lec_configure(int lecs_method, struct sockaddr_atmsvc *manual_atm_addr,
                  struct sockaddr_atmsvc *listen_addr)
{
        int retval;
        struct sockaddr_atmsvc addr_c5, addr_47;
        struct atm_sap sap;
        struct atm_qos qos;
        Conn_t *conn;
        
        diag(COMPONENT, DIAG_DEBUG, "entering lec_configure");
        
        /* initialize well known LECS addresses */
        memset(&addr_c5, 0, sizeof(struct sockaddr_atmsvc));
        memset(&addr_47, 0, sizeof(struct sockaddr_atmsvc));
        addr_c5.sas_family = addr_47.sas_family = AF_ATMSVC;
        memcpy(addr_c5.sas_addr.prv, well_known_lecs, ATM_ESA_LEN);
        memcpy(addr_47.sas_addr.prv, well_known_lecs, ATM_ESA_LEN);
        addr_c5.sas_addr.prv[0] = 0xC5;

        /* see if the user wants to skip LECS */
        if (lecs_method == LECS_NONE)
                return 0;
        
        init_conn_params(&sap, &qos, CONTROL_CONN);

        if ((lecs_method == LECS_MANUAL) || (lecs_method == LECS_FROM_ILMI)) {
                diag(COMPONENT, DIAG_DEBUG, "trying manual LECS address");
                conn = setup_svc(manual_atm_addr, listen_addr, &sap, &qos);
                if (conn) {
                        retval = do_lec_configure(conn);
                        close_connection(conn);
                        return retval;
                }
                else random_delay();
        }
        diag(COMPONENT, DIAG_DEBUG, "trying well-known anycast LECS address");
        conn = setup_svc(&addr_c5, listen_addr, &sap, &qos);
        if (conn) {
                retval = do_lec_configure(conn);
                close_connection(conn);
                return retval;
        }
        else random_delay();
        diag(COMPONENT, DIAG_DEBUG, "trying well-known LECS address");
        conn = setup_svc(&addr_47, listen_addr, &sap, &qos);
        if (conn) {
                retval = do_lec_configure(conn);
                close_connection(conn);
                return retval;
        }
        
        return -1;
}

/*
 * 5.3 Configuration phase, get configuration from LECS
 * Returns < 0 for error
 */
static int do_lec_configure(Conn_t *conn)
{
        int frame_size = 0;
        char buff[MAX_CTRL_FRAME];

        lec_params.c7c_current_timeout = lec_params.c7i_initial_ctrl_timeout; /* reset it */
        while (lec_params.c7c_current_timeout <= lec_params.c7_ctrl_timeout) {
                if (send_config_req(conn) < 0) return -1;
                frame_size = read_config_rsp(conn, buff, sizeof(buff));
                if (frame_size < 0) return -1;
                if (frame_size == 0) /* timeout */
                        lec_params.c7c_current_timeout *= lec_params.c7x_timeout_multiplier;
                else
                        break; /* frame in */
        }
        lec_params.c7c_current_timeout = lec_params.c7i_initial_ctrl_timeout; /* reset it */
        if (frame_size == 0) {
                diag(COMPONENT, DIAG_INFO, "Timed out while waiting for LE_CONFIGURE_RESPONSE");
                return -1; /* timeout */
        }

        if (parse_config_rsp(buff, frame_size) < 0) {
                diag(COMPONENT, DIAG_ERROR, "Parsing LE_CONFIG_RESPONSE indicates failure");
                return -1;
        }

        return 0;
}

/*
 * Compose a LE Config request frame and send it to LECS
 * Returns < 0 for error
 */
static int send_config_req(Conn_t *conn)
{
        int frame_size;
        struct ctrl_frame *config_req;
        char buff[MAX_CTRL_FRAME];

        diag(COMPONENT, DIAG_DEBUG, "Sending LE_CONFIGURE_REQUEST");

        /* TLVs make config frame variable length */
        frame_size = sizeof(struct ctrl_frame);
        if (lec_params.c3_max_frame_size == MTU_1580) frame_size += 5;
        config_req = (struct ctrl_frame *)buff;
        memset(config_req, 0, frame_size);

        prefill_frame(config_req, LE_CONFIG_REQ);
        if (lec_params.c29_v2_capable)
                config_req->header.flags = htons(V2_CAPABLE);
        config_req->src_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
        memcpy(config_req->src_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN);
        memcpy(config_req->src_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        config_req->lan_type = lec_params.c2_lan_type;
        config_req->max_frame_size = lec_params.c3_max_frame_size;
        config_req->elan_name_size = strlen(lec_params.c5_elan_name);
        if (strlen(lec_params.c5_elan_name) > 0)
                strcpy(config_req->elan_name, lec_params.c5_elan_name);
        if (lec_params.c3_max_frame_size == MTU_1580) {
                lec_params.c3_max_frame_size = MTU_1516;
                *(uint32_t *)(config_req + 1) = htonl(X5_ADJUSTMENT);
                config_req->num_tlvs++;
        }

        if (send_frame(conn, buff, frame_size) != frame_size)
                return -1;

        return 0;
}

/*
 * Reads in Config frame or timeouts
 * Returns Config frame length, < 0 for error, 0 for timeout
 */
static int read_config_rsp(Conn_t *conn, char *buff, int buffsize)
{
        int frame_size, retval;
        struct timeval tv;
        fd_set rfds;

        tv.tv_sec = lec_params.c7c_current_timeout;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(conn->fd, &rfds);

        retval = select(conn->fd + 1, &rfds, NULL, NULL, &tv);
        if (retval < 0) {
                diag(COMPONENT, DIAG_ERROR, "read_ctrl_rsp: select: %s",
                     strerror(errno));
                return retval;
        }
        if (retval == 0) return 0; /* timeout */
        
        frame_size = recv_frame(conn, buff, buffsize);
        if (frame_size < 0) {
                diag(COMPONENT, DIAG_ERROR, "read_ctrl_rsp: recv_frame: %s",
                     strerror(errno));
                return frame_size;
        }
        if (frame_size == 0) {
                diag(COMPONENT, DIAG_ERROR, "read_ctrl_rsp: conn closed: %s",
                     strerror(errno));
                return -1;
        }

        return frame_size;
}

/* Validates and parses a LE_CONFIGURE_RESPONSE.
 * See LANEv2, 5.3.x
 * Returns < 0 for error
 */
static int parse_config_rsp(unsigned char *buff, int size)
{
        struct ctrl_frame *frame;

        diag(COMPONENT, DIAG_DEBUG, "Parsing LE_CONFIG_RESPONSE");
        if (validate_frame(buff, size) < 0) {
                diag(COMPONENT, DIAG_ERROR, "parse_config_rsp: bad frame");
                return -1;
        }

        frame = (struct ctrl_frame *)buff;
        if (frame->header.opcode != htons(LE_CONFIG_RSP)) {
                diag(COMPONENT, DIAG_ERROR, "parse_config_rsp: not a LE_CONFIG_RESPONSE");
                return -1;
        }
        
        if (frame->header.status != 0) {
                diag(COMPONENT, DIAG_ERROR, "LECS said: %s",
                     status2text(frame->header.status));
                return -1;
        }
        if (frame->elan_name_size > 32) return -1;

        /* looks good, now extract the information */
        lec_params.c2_lan_type = frame->lan_type;
        lec_params.c3_max_frame_size = frame->max_frame_size;
        if (frame->elan_name_size != 0)
                strncpy(lec_params.c5_elan_name, frame->elan_name, frame->elan_name_size);
        lec_params.c5_elan_name[frame->elan_name_size] = '\0';
        memcpy(lec_params.c9_les_atm_addr, frame->target_atm_addr, ATM_ESA_LEN);

        parse_tlvs(frame->header.opcode, buff + sizeof(struct ctrl_frame),
                   frame->num_tlvs, size - sizeof(struct ctrl_frame));

        return 0;
}

/* ------------- Configure phase specific stuff ends ------------- */


/* -------------- Join phase specific stuff starts --------------- */

/*
 * 5.4 Join phase
 * Create control direct VCC and accept possible control distribute VCC
 * Returns < 0 for error
 */
int les_connect(int lecs_method, struct sockaddr_atmsvc *manual_atm_addr,
                  struct sockaddr_atmsvc *listen_addr)
{
        struct sockaddr_atmsvc les_addr;
        struct atm_sap sap;
        struct atm_qos qos;
        char buff[MAX_CTRL_FRAME];
        int frame_size = 0; /* shut up, GCC */

        diag(COMPONENT, DIAG_DEBUG, "Entering Join phase");

        if (lecs_method == LECS_NONE) {
                diag(COMPONENT, DIAG_DEBUG, "Skipping LECS, connecting straight to LES");
                memcpy(les_addr.sas_addr.prv, manual_atm_addr->sas_addr.prv, ATM_ESA_LEN);
        } else memcpy(les_addr.sas_addr.prv, lec_params.c9_les_atm_addr, ATM_ESA_LEN);
	*les_addr.sas_addr.pub = 0;

        init_conn_params(&sap, &qos, CONTROL_CONN);
        lec_params.ctrl_direct = setup_svc(&les_addr, listen_addr, &sap, &qos);
        if (lec_params.ctrl_direct == NULL) {
                diag(COMPONENT, DIAG_ERROR, "Control direct SVC failed");
                random_delay();
                return -1;
        }
        lec_params.ctrl_listen = create_listensocket(listen_addr, &sap, &qos);
        if (lec_params.ctrl_listen == NULL) {
                diag(COMPONENT, DIAG_ERROR, "Control distribute listen socket failed");
                random_delay();
                return -1;
        }

        lec_params.c7c_current_timeout = lec_params.c7i_initial_ctrl_timeout; /* reset it */
        while (lec_params.c7c_current_timeout <= lec_params.c7_ctrl_timeout) {
                if (send_join_req(lec_params.ctrl_direct) < 0) {
                        diag(COMPONENT, DIAG_ERROR, "Sending LE_JOIN_REQUEST failed");
                        random_delay();
                        return -1;
                }
                frame_size = read_join_rsp(buff, sizeof(buff));
                if (frame_size < 0) {
                        diag(COMPONENT, DIAG_ERROR, "Receiving LE_JOIN_RESPONSE failed");
                        random_delay();
                        return -1;
                } else if (frame_size == 0) /* timeout */
                        lec_params.c7c_current_timeout *= lec_params.c7x_timeout_multiplier;
                else
                        break; /* frame in */
        }
        lec_params.c7c_current_timeout = lec_params.c7i_initial_ctrl_timeout; /* reset it */
        if (frame_size == 0) {
                diag(COMPONENT, DIAG_ERROR, "LE_JOIN_RESPONSE timed out");
                return -1;
        }
        
        if (parse_join_rsp(buff, frame_size) < 0) {
                diag(COMPONENT, DIAG_ERROR, "Parsing LE_JOIN_RESPONSE failed");
                return -1;
        }

        return 0;
}

/* 5.4.1.2 Transmitting LE_JOIN_REQUEST
 * Note that the TLVs should be the same as in LE_CONFIG_REQUEST
 * excluding X5-Adjustment and Preferred-LES
 */
static int send_join_req(Conn_t *conn)
{
        char buff[MAX_CTRL_FRAME], *tlvp;
        struct ctrl_frame *frame;
        int frame_size;

        frame_size = sizeof(struct ctrl_frame);
        if (lec_params.c3_max_frame_size == 5) frame_size += 5;
        if (lec_params.c35_contains_address) frame_size += (5 + ATM_ESA_LEN);
        if (strlen(lec_params.foreId) > 0) frame_size += (5 + strlen(lec_params.foreId));

        frame = (struct ctrl_frame *)buff;
        tlvp = (char *)(frame + 1);
        prefill_frame(frame, LE_JOIN_REQ);
        frame->lan_type = lec_params.c2_lan_type;
        if (lec_params.c4_proxy_flag) frame->header.flags |= htons(PROXY_FLAG);
        frame->src_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
        strcpy(frame->elan_name, lec_params.c5_elan_name);
        frame->elan_name_size = strlen(lec_params.c5_elan_name);
        if (lec_params.c29_v2_capable) frame->header.flags |= htons(V2_CAPABLE);
        memcpy(frame->src_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        memcpy(frame->src_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN);
        frame->max_frame_size = lec_params.c3_max_frame_size;
        if (lec_params.c3_max_frame_size == 5) {
                frame->max_frame_size = 1;
                *(uint32_t *)tlvp = htonl(X5_ADJUSTMENT);
                tlvp += 4; *tlvp = 0; tlvp++;
                frame->num_tlvs++;
        }
        if (lec_params.c35_contains_address) {
                *(uint32_t *)tlvp = htonl(PREFERRED_LES);
                tlvp +=4; *tlvp = (uint8_t)ATM_ESA_LEN; tlvp++;
                memcpy(tlvp, lec_params.c35_preferred_les, ATM_ESA_LEN);
                tlvp += 20;
                frame->num_tlvs++;
        }
        if (strlen(lec_params.foreId) > 0) {
                *(uint32_t *)tlvp = htonl(FORE_NAME);
                tlvp +=4; *tlvp = (uint8_t)strlen(lec_params.foreId); tlvp++;
                memcpy(tlvp, lec_params.foreId, strlen(lec_params.foreId));
                tlvp += strlen(lec_params.foreId);
                frame->num_tlvs++;
        }

        if (send_frame(conn, frame, frame_size) != frame_size)
                return -1;

        return 0;
}

/* LE_JOIN_RESPONSE can either come over ctrl_direct or ctrl_dist.
 * However, if LES uses Control Direct VCC, we can ignore connections
 * to ctrl_listen and if it connects to ctrl_listen we can wait
 * the reponse to arrive that route. Simple :)
 * 
 * Returns < 0 for error, 0 for timeout, > 0 for frame size
 */
static int read_join_rsp(char *buff, int buffsize)
{
        Conn_t *dist;
        int n, retval, frame_size = 0;
        struct timeval tv;
        fd_set rfds;

        /* Idea here is always to listen for two sockets. One of the
           sockets is always Control Direct and the other is either
           listen socket for Control Distribute or the Control
           Distribute itself. We can do it like this, since listen
           socket gets closed as soon as it creates a new connection
        */

        dist = (lec_params.ctrl_listen != NULL) ? lec_params.ctrl_listen : lec_params.ctrl_dist;
        n = (lec_params.ctrl_direct->fd > dist->fd) ? lec_params.ctrl_direct->fd : dist->fd;
        n++;

        tv.tv_sec = lec_params.c7c_current_timeout;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(lec_params.ctrl_direct->fd, &rfds);
        FD_SET(dist->fd, &rfds);
        
        retval = select(n, &rfds, NULL, NULL, &tv);
        if (retval < 0) {
                diag(COMPONENT, DIAG_ERROR, "get_join_rsp: select: %s",
                     strerror(errno));
                return -1;
        }
        if (retval == 0) return 0; /* timeout */

        /* Be careful here. The both sockets might be readable
         * and the response can come from either one
         */
        if (FD_ISSET(lec_params.ctrl_direct->fd, &rfds)) {
                /* Control Direct was changed */
                frame_size = recv_frame(lec_params.ctrl_direct, buff, buffsize);
                if (frame_size < 0) {
                        diag(COMPONENT, DIAG_ERROR, "get_join_rsp: recv_frame: %s",
                             strerror(errno));
                        return -1;
                }
                if (frame_size == 0) {
                        diag(COMPONENT, DIAG_ERROR, "get_join_rsp: Control direct VCC closed");
                        return -1;
                }
                diag(COMPONENT, DIAG_DEBUG, "LE_JOIN_RESPONSE over Control direct VCC");
        }
        if (FD_ISSET(dist->fd, &rfds)) {
                /* Event in listen socket or Control Distribute */
                if (dist == lec_params.ctrl_listen) {
                        /* Connection to control listen */
                        lec_params.ctrl_dist = accept_conn(lec_params.ctrl_listen);
                        if (lec_params.ctrl_dist == NULL) {
                                diag(COMPONENT, DIAG_ERROR, "accept of Ctrl distribute failed");
                                return -1;
                        }
                        diag(COMPONENT, DIAG_DEBUG, "Closing listen socket for Ctrl distribute VCC");
                        close_connection(lec_params.ctrl_listen);
                        lec_params.ctrl_listen = NULL;

                        /* let's see if this new socket has something for us */
                        return (read_join_rsp(buff, buffsize));
                }
                /* Event in Control distribute */
                frame_size = recv_frame(lec_params.ctrl_dist, buff, buffsize);
                if (frame_size < 0) {
                        diag(COMPONENT, DIAG_ERROR, "get_join_rsp: recv_frame: %s",
                             strerror(errno));
                        return -1;
                }
                if (frame_size == 0) {
                        diag(COMPONENT, DIAG_ERROR, "Control distribute VCC closed");
                        return -1;
                }
                diag(COMPONENT, DIAG_DEBUG, "LE_JOIN_RESPONSE over Control distribute VCC");
        }

        return frame_size;
}

/* Validates and parses a LE_JOIN_RESPONSE.
 * See LANEv2 5.4.x
 * Returns < 0 for error
 */
static int parse_join_rsp(unsigned char *buff, int size)
{
        struct ctrl_frame *frame;

        diag(COMPONENT, DIAG_DEBUG, "Parsing LE_JOIN_RESPONSE");
        if (validate_frame(buff, size) < 0)
                return -1;

        frame = (struct ctrl_frame *)buff;
        if (frame->header.opcode != htons(LE_JOIN_RSP))
                return -1;
        
        if (frame->header.status != 0) {
                diag(COMPONENT, DIAG_ERROR, "LES said: %s",
                     status2text(frame->header.status));
                return -1;
        }
        if (frame->elan_name_size > 32) return -1;

        /* looks good, now extract the information */
        lec_params.c2_lan_type = frame->lan_type;
        lec_params.c3_max_frame_size = frame->max_frame_size;
        if (frame->elan_name_size != 0)
                strncpy(lec_params.c5_elan_name, frame->elan_name, frame->elan_name_size);
        lec_params.c5_elan_name[frame->elan_name_size] = '\0';
        lec_params.c14_lec_id = ntohs(frame->header.lec_id);

        if (!(frame->header.flags & htons(V2_REQUIRED)) && lec_params.c29_v2_capable) {
                diag(COMPONENT, DIAG_INFO, "LES did not return V2 Required Flag, acting as V1 client");
                lec_params.c29_v2_capable = 0;
        }
        if (!(frame->header.flags & htons(V2_REQUIRED)) &&
            (lec_params.c3_max_frame_size == MTU_1580)) {
                /* Against spec, but we'll accept the MTU and clear the flag */
                diag(COMPONENT, DIAG_ERROR, "LES not LANEv2 but uses MTU of 1580 bytes");
                lec_params.c29_v2_capable = 0;
        }
        parse_tlvs(frame->header.opcode, buff + sizeof(struct ctrl_frame),
                   frame->num_tlvs, size - sizeof(struct ctrl_frame));

        return 0;
}

/* --------------- Join phase specific stuff ends ---------------- */


/* -------------- Bus connect specific stuff starts --------------- */

int bus_connect(void)
{
        struct ctrl_frame *frame;
        char buff[MAX_CTRL_FRAME];
        struct sockaddr_atmsvc bus_addr, listen_addr;
        int retval, tries, n;
        struct atm_sap sap;
        struct atm_qos qos;
        struct timeval tv;
        fd_set rfds;
        Conn_t *mcast_fwd;
        struct atmlec_ioc ioc_data;

        frame = (struct ctrl_frame *)buff;
        memset(&bus_addr, 0, sizeof(struct sockaddr_atmsvc));
        memset(&listen_addr, 0, sizeof(struct sockaddr_atmsvc));
        
        /* try to arp BUS two times */
        tries = 2;
        while (tries > 0) {
                prefill_frame(frame, LE_ARP_REQ);
                frame->header.lec_id = htons(lec_params.c14_lec_id);
                frame->src_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
                memcpy(frame->src_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN);
                memcpy(frame->src_atm_addr, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
                frame->target_lan_dst.tag = htons(LAN_DST_MAC_ADDR);
                memcpy(frame->target_lan_dst.mac, bus_mac, ETH_ALEN);
                retval = send_frame(lec_params.ctrl_direct, frame, sizeof(struct ctrl_frame));
                if (retval < 0) {
                        diag(COMPONENT, DIAG_ERROR, "LE_ARP_REQUEST for BUS failed");
                        return -1;
                }
                retval = get_bus_addr(&bus_addr);
                if (retval < 0) {
                        diag(COMPONENT, DIAG_ERROR, "LE_ARP_RESPONSE for BUS failed");
                        return -1;
                } else if (retval > 0)
                        break; /* got it */
                tries--;
        }
        if (tries == 0) {
                diag(COMPONENT, DIAG_ERROR, "LE_ARP_RESPONSE for BUS timed out");
                return -1;
        }

        /* We got address for BUS. Make the listen socket for Multicast
         * Forward first and then contact BUS.
         */
        memcpy(listen_addr.sas_addr.prv, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        listen_addr.sas_family = bus_addr.sas_family = AF_ATMSVC;
        init_conn_params(&sap, &qos, MCAST_CONN);
        lec_params.mcast_listen = create_listensocket(&listen_addr, &sap, &qos);
        if (lec_params.mcast_listen == NULL) {
                diag(COMPONENT, DIAG_ERROR, "Listen socket for BUS failed");
                return -1;
        }

        lec_params.mcast_send = setup_svc(&bus_addr, &listen_addr, &sap, &qos);
        if (lec_params.mcast_send == NULL) {
                diag(COMPONENT, DIAG_ERROR, "Connect to BUS failed");
                return -1;
        }
        /* Default Multicast send VCC to BUS ready, notify kernel */
        if (ioctl(lec_params.mcast_send->fd, ATMLEC_MCAST, lec_params.itf_num) < 0) {
                diag(COMPONENT, DIAG_FATAL, "Can't change socket into LE mcast socket: %s", strerror(errno));
                return -1;
        }

        diag(COMPONENT, DIAG_DEBUG, "About to wait for BUS to connect");
        tv.tv_sec = lec_params.c7_ctrl_timeout;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(lec_params.mcast_listen->fd, &rfds);
        n = lec_params.mcast_listen->fd + 1;
        retval = select(n, &rfds, NULL, NULL, &tv);
        if (retval == 0) {
                diag(COMPONENT, DIAG_ERROR, "BUS connect to Multicast Forward listen socket timed out");
                return -1;
        }
        if (retval < 0) {
                diag(COMPONENT, DIAG_ERROR, "while waiting for Multicast Forward VCC: select: %s",
                     strerror(errno));
                return -1;
        }
        mcast_fwd = accept_conn(lec_params.mcast_listen);
        if (mcast_fwd == NULL) {
                diag(COMPONENT, DIAG_ERROR, "BUS connect to Multicast Forward listen socket failed");
                return -1;
        }
        
        memcpy(ioc_data.atm_addr, mcast_fwd->atm_address, ATM_ESA_LEN);
        ioc_data.dev_num = lec_params.itf_num;
        ioc_data.receive = 2; /* Multicast distribute */
        diag(COMPONENT, DIAG_DEBUG, "About to notify kernel about Multicast Forward VCC");
        if (ioctl(mcast_fwd->fd, ATMLEC_DATA, &ioc_data) < 0) {
                diag(COMPONENT, DIAG_DEBUG, "Could not notify kernel: %s", strerror(errno));
                return -1;
        }

        /* All done. We're in! */
        return 0;
}

/*
 * Waits for LE_ARP_RESPONSE for BUS' ATM address to arrive.
 * Returns < 0 for error, 0 for timeout > 0 for success
 * BUS ATM address will be stored in *addr */
static int get_bus_addr(struct sockaddr_atmsvc *addr)
{

        fd_set rfds;
        struct timeval tv;
        int n = 0, retval, timeout;
        char buff[MAX_CTRL_FRAME];

        timeout = 4; /* wait response for 4 seconds */
        lec_params.c7c_current_timeout = 1;
        while (lec_params.c7c_current_timeout <= timeout) {
                tv.tv_sec = lec_params.c7c_current_timeout; /* actually not specified exactly */
                tv.tv_usec = 0;
                FD_ZERO(&rfds);
                FD_SET(lec_params.ctrl_direct->fd, &rfds);
                if (lec_params.ctrl_dist != NULL) {
                        FD_SET(lec_params.ctrl_dist->fd, &rfds);
                        n = lec_params.ctrl_dist->fd;
                }
                n = (lec_params.ctrl_direct->fd > n) ? lec_params.ctrl_direct->fd : n;
                n++;
                
                retval = select(n, &rfds, NULL, NULL, &tv);
                if (retval == 0) {
                        lec_params.c7c_current_timeout++;
                        continue; /* back to waiting */
                }
                if (retval < 0) {
                        diag(COMPONENT, DIAG_ERROR, "get_bus_addr: select: %s", strerror(errno));
                        return -1;
                }
                if (FD_ISSET(lec_params.ctrl_direct->fd, &rfds)) {
                        diag(COMPONENT, DIAG_DEBUG, "get_bus_addr: ctrl.direct changed:");
                        retval = read_bus_arp(lec_params.ctrl_direct, addr, buff, sizeof(buff));
                        if (retval < 0) return -1;
                        if (retval > 0) return retval;
                }
                if (lec_params.ctrl_dist != NULL && FD_ISSET(lec_params.ctrl_dist->fd, &rfds)) {
                        diag(COMPONENT, DIAG_DEBUG, "get_bus_addr: ctrl.dist changed:");
                        retval = read_bus_arp(lec_params.ctrl_dist, addr, buff, sizeof(buff));
                        if (retval < 0) return -1;
                        if (retval > 0) return retval;
                }
                diag(COMPONENT, DIAG_DEBUG, "get_bus_addr: consumed a packet");
        }

        diag(COMPONENT, DIAG_ERROR, "Timeout while waiting for BUS LE_ARP response");
        return 0;
}
/*
 * Tries to read BUS ATM address in *addr
 * returns < 0 for error, 0 for not found > 0 for success
 */
static int read_bus_arp(Conn_t *conn, struct sockaddr_atmsvc *addr, char *buff, int buffsize)
{
        int frame_size;
        struct ctrl_frame *frame;

        frame_size = recv_frame(conn, buff, buffsize);
        if (frame_size == 0) {
                diag(COMPONENT, DIAG_ERROR, "LES Control connection closed");
                return -1;
        }
        if (frame_size < 0) {
                diag(COMPONENT, DIAG_ERROR, "get_bus_arp: recv_frame: %s", strerror(errno));
                return -1;
        }
        frame = (struct ctrl_frame *)buff;
        if (validate_frame(buff, frame_size) >= 0 &&
            frame->header.opcode == htons(LE_ARP_RSP) &&
            memcmp(frame->src_lan_dst.mac, lec_params.c6_mac_addr, ETH_ALEN) == 0) {
                memcpy(addr->sas_addr.prv, frame->target_atm_addr, ATM_ESA_LEN);
                return frame_size;
        }

        return 0; /* not found */
}

/* --------------- Bus connect specific stuff ends --------------- */

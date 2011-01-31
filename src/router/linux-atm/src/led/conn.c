/* conn.c - functions for handling SVCs, create, accept, send, etc. */

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <time.h>

#include <atm.h>
#include <atmsap.h>
#include <linux/atmlec.h>

#include <atmd.h>

#include "conn.h"
#include "display.h"
#include "lec.h"
#include "frames.h"
#include "kernel.h"

#define COMPONENT "conn.c"

/* status */
#define CONNECTED  42    /* Operational socket */
#define CONNECTING 43    /* Non-blocking socket, not yet connected */

/* type */
#define WEMADE 7
#define THEYMADE 8
#define LISTENING 9
#define KERNEL_SOCK 10

static Conn_t *connlist = NULL;

/* Local protos */
static void list_remove_conn(Conn_t *conn);
static Conn_t *list_add_conn(unsigned char *dest_atm_addr);
static Conn_t *conn_already_exists(unsigned char *atm_addr, Conn_t *current);
static const char *get_type_string(int type);
static int maxmtu2maxsdu(uint8_t mtu);
static uint16_t conn_type2codepoint(int conn_type);

static void delete_addr(unsigned char *atm_addr)
{
        struct atmlec_msg msg;

        msg.type = l_addr_delete;
        memcpy(msg.content.normal.atm_addr, atm_addr, ATM_ESA_LEN);
        msg_to_kernel(&msg, sizeof(struct atmlec_msg));

        return;
}

/* Checks if connection to atm_addr already exists. Does not
 * check against current though.
 * Returns NULL for no connection, or Conn_t of existing connection.
 */
static Conn_t *conn_already_exists(unsigned char *atm_addr, Conn_t *current)
{
        Conn_t *conn;

        conn = connlist;
        while (conn) {
                if (conn != current &&
                    conn->type != LISTENING && 
                    conn->type != KERNEL_SOCK) {
                        if (memcmp(conn->atm_address, atm_addr, ATM_ESA_LEN) == 0)
                                return conn;
                }
                conn = conn->next;
        }
        
        return NULL;
}

/* Initializes and fills in *sap and *qos according to Blli
 * code point value specified in conn_type.
 */
void init_conn_params(struct atm_sap *sap, struct atm_qos *qos,
                      uint16_t conn_type)
{

        unsigned int bllicode;
        int i, sdu;
        char qos_text[MAX_ATM_QOS_LEN + 1];

        diag(COMPONENT, DIAG_DEBUG, "init_conn_params, conn_type %x", conn_type);

        memset(qos, 0, sizeof(struct atm_qos));
        memset(sap, 0, sizeof(struct atm_sap));
        qos->aal = ATM_AAL5;
  
        /* Set the forward and backward Max CPCS-SDU Size */
        switch(conn_type) {
        case CONTROL_CONN:
                qos->rxtp.max_sdu = 1516;
                qos->txtp.max_sdu = 1516;
                break;
        case DATA_DIRECT_CONN:
        case MCAST_CONN:
                sdu = maxmtu2maxsdu(lec_params.c3_max_frame_size);
                qos->rxtp.max_sdu = sdu;
                qos->txtp.max_sdu = sdu;
                break;
        default:
                diag(COMPONENT, DIAG_ERROR, "unknown conn_type %x", conn_type);
                break;
        }
        
        /* ATM User Cell Rate/ATM Traffic Descriptor. */
        qos->txtp.traffic_class = ATM_UBR;
        qos->rxtp.traffic_class = ATM_UBR;
   
        if (get_verbosity(COMPONENT) >= DIAG_DEBUG) {
                if (qos2text(qos_text, sizeof(qos_text), qos, 0) < 0)
                        sprintf(qos_text, "<bad qos>");
                diag(COMPONENT, DIAG_DEBUG, "init_conn_params, QoS '%s'", qos_text);
        }

        /* No Broadband High Layer Information in LANE. */
        sap->bhli.hl_type = ATM_HL_NONE;

        /* Broadband Lower Layer Information. */
        sap->blli[0].l3_proto = ATM_L3_TR9577;
        sap->blli[0].l3.tr9577.ipi = NLPID_IEEE802_1_SNAP;
        sap->blli[0].l3.tr9577.snap[0] = 0x00;
        sap->blli[0].l3.tr9577.snap[1] = 0xa0;
        sap->blli[0].l3.tr9577.snap[2] = 0x3e;
        bllicode = conn_type2codepoint(conn_type);
        sap->blli[0].l3.tr9577.snap[3] = (unsigned char)(0xff&(bllicode>>8));
        sap->blli[0].l3.tr9577.snap[4] = (unsigned char)(0xff&bllicode);

        if (get_verbosity(COMPONENT) == DIAG_DEBUG) {
                for(i=0; i < 5; i++) {
                        diag(COMPONENT, DIAG_DEBUG, "snap[%d] = 0x%2.2x",
                             i, sap->blli[0].l3.tr9577.snap[i]);
                }
        }

        return;
}

/* Returns != 0 if blli indicates Data Direct
 * connection
 */
static int is_data_direct(struct atm_blli *blli)
{
        return (blli->l3.tr9577.snap[4] == DATA_DIRECT_8023 ||
                blli->l3.tr9577.snap[4] == DATA_DIRECT_8025);
}

/* Creates a socket with the specified parameters.
 * If listen_addr is non NULL binds to it.
 * Returns < 0 for error or new socket descriptor.
 */
static int get_socket(struct sockaddr_atmsvc *listen_addr,
                      struct atm_sap *sap, struct atm_qos *qos)
{
        int s, ret;

        s = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if (s < 0) {
                diag(COMPONENT, DIAG_ERROR, "socket creation failure: %s",
                     strerror(errno));
                return -1;
        }
        diag(COMPONENT, DIAG_DEBUG, "get_socket: got fd %d", s);
        
        if (setsockopt(s, SOL_ATM, SO_ATMQOS, qos, sizeof(struct atm_qos)) < 0) {
                diag(COMPONENT, DIAG_ERROR, "get_socket: setsockopt SO_ATMQOS: %s", strerror(errno));
                close(s);
                return -1;
        }
        if (setsockopt(s, SOL_ATM, SO_ATMSAP, sap, sizeof(struct atm_sap)) < 0) {
                diag(COMPONENT, DIAG_ERROR, "setup_svc setsockop(SO_ATMSAP)");
                close(s);
                return -1;
        }
        
        /* Bind the socket to our local address */
        if (listen_addr == NULL)
                return s;
        ret = bind(s, (struct sockaddr *)listen_addr, sizeof(struct sockaddr_atmsvc));
        if (ret < 0) {
                diag(COMPONENT, DIAG_ERROR, "bind error: %s", strerror(errno));
                close(s);
                return -1;
        }
        
        return s;
}


/* Does an active open to dst_addr using pre-filled
 * parameters in sap and qos.
 * If listen_addr is non NULL, binds to it.
 * Data direct SVCs are non-blocking, others block
 * Returns NULL for error or new connections.
 */
Conn_t *setup_svc(struct sockaddr_atmsvc *dst_addr,
                  struct sockaddr_atmsvc *listen_addr,
                  struct atm_sap *sap, struct atm_qos *qos)
{
        Conn_t *conn;
        int s, ret;
        char buff[MAX_ATM_ADDR_LEN+1];

        diag(COMPONENT, DIAG_DEBUG, "Outgoing call setup");
        
        dst_addr->sas_family = AF_ATMSVC;
        listen_addr->sas_family = AF_ATMSVC;
        
        switch(sap->blli[0].l3.tr9577.snap[4]) { /* Kludge.  Eh? */
        case CONTROL_CONN:
                diag(COMPONENT, DIAG_DEBUG, "LE Control SVC setup");
                break;
        case DATA_DIRECT_8023:
                diag(COMPONENT, DIAG_DEBUG, "Data direct 802.3");
                break;
        case DATA_DIRECT_8025:
                diag(COMPONENT, DIAG_DEBUG, "Data direct 802.5");
                break;
        case MCAST_CONN_8023:
                diag(COMPONENT, DIAG_DEBUG, "Multicast 802.3");
                break;
        case MCAST_CONN_8025:
                diag(COMPONENT, DIAG_DEBUG, "Multicast 802.5");
                break;
        default:
                diag(COMPONENT, DIAG_ERROR, "Unknown codepoint in svc setup");
        }
 
        s = get_socket(listen_addr, sap, qos);
        if (s < 0) return NULL;
        if (atm2text(buff, sizeof(buff), (struct sockaddr *)dst_addr, A2T_PRETTY | A2T_NAME | A2T_LOCAL) < 0)
                sprintf(buff, "<Unknown ATM address>");
        diag(COMPONENT, DIAG_DEBUG, "Call to %s", buff);

        /* Make data direct SVCs non-blocking */
        if (is_data_direct(&sap->blli[0])) {
                ret = fcntl(s, F_GETFL);
                if (ret < 0) {
                        diag(COMPONENT, DIAG_ERROR, "fcntl(s, F_GETFL)");
                        close(s);
                } else if (fcntl(s, F_SETFL, ret|O_NONBLOCK) < 0) {
                        diag(COMPONENT, DIAG_ERROR, "fcntl(s, F_SETFL, x|O_NONBLOCK)");
                        close(s);
                        return NULL;
                }
        }

        ret = connect(s, (struct sockaddr *)dst_addr, sizeof(struct sockaddr_atmsvc));
        if (ret < 0 && errno != EINPROGRESS) {
                diag(COMPONENT, DIAG_ERROR, "connect error: %s", strerror(errno));
                close(s);
                return NULL;
        }

        conn = list_add_conn(dst_addr->sas_addr.prv);
        diag(COMPONENT, DIAG_DEBUG, "Conn:%p", conn);
        if (conn == NULL) {
                close(s);
                return NULL;
        }
        conn->fd = s;
        conn->type = WEMADE;
        conn->codepoint = sap->blli[0].l3.tr9577.snap[4];

        if (is_data_direct(&sap->blli[0]))
                conn->status = CONNECTING;
        else
                conn->status = CONNECTED;
        
        return conn;
}

/* Creates listen socket for incoming data direct connections.
 * Only for data direct, not for Control or Multicast listen sockets.
 * Returns < 0 for error
 */
int create_data_listen(void)
{
        struct atm_sap sap;
        struct atm_qos qos;
        struct sockaddr_atmsvc addr;

        memset(&addr, 0, sizeof(struct sockaddr_atmsvc));
        memcpy(addr.sas_addr.prv, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        addr.sas_family = AF_ATMSVC;
        init_conn_params(&sap, &qos, DATA_DIRECT_CONN);

        lec_params.data_listen = create_listensocket(&addr, &sap, &qos);
        if (lec_params.data_listen == NULL) {
                diag(COMPONENT, DIAG_FATAL, "Could not create listen socket for incoming Data Direct VCCs");
                return -1;
        }

        return 0;
}

/* Opens a Multicast or non-blocking Data Direct VCC to atm_addr.
 * Not for Control connections.
 * Returns < 0 for error
 */
int create_data_svc(unsigned char *atm_addr, int codepoint)
{
        struct atm_sap sap;
        struct atm_qos qos;
        struct sockaddr_atmsvc my_addr, dst_addr;
        Conn_t *conn;

        memset(&my_addr, 0, sizeof(struct sockaddr_atmsvc));
        memcpy(my_addr.sas_addr.prv, lec_params.c1n_my_atm_addr, ATM_ESA_LEN);
        memset(&dst_addr, 0, sizeof(struct sockaddr_atmsvc));
        memcpy(dst_addr.sas_addr.prv, atm_addr, ATM_ESA_LEN);
        my_addr.sas_family = dst_addr.sas_family = AF_ATMSVC;
        init_conn_params(&sap, &qos, codepoint);

        conn = setup_svc(&dst_addr, &my_addr, &sap, &qos);
        if (conn == NULL) {
                diag(COMPONENT, DIAG_ERROR, "Could not create Data Direct VCC");
                delete_addr(dst_addr.sas_addr.prv);
                return -1;
        }

        return 0;
}

/* Creates a listen socket with parameters specified with
 * arguments.
 * Returns NULL for error or Conn_t for new listen socket.
 */
Conn_t *create_listensocket(struct sockaddr_atmsvc *listen_addr,
                            struct atm_sap *sap, struct atm_qos *qos)
{
        int fd, ret;
        Conn_t *conn;

        diag(COMPONENT, DIAG_DEBUG, "conn_create_listensocket");
        
        fd = get_socket(listen_addr, sap, qos);
        if (fd < 0) return NULL;
        
        ret = listen(fd, 16);
        if (ret != 0) {
                diag(COMPONENT, DIAG_DEBUG, "Listen failed: %s", strerror(errno));
                close(fd);
                return NULL;
        }
        
        conn = list_add_conn(NULL);
        if (conn == NULL) {
                diag(COMPONENT, DIAG_ERROR, "List_add_conn failed");
                close(fd);    
                return NULL;
        }

        conn->type = LISTENING;
        conn->fd = fd;
        diag(COMPONENT, DIAG_DEBUG, "Listen socket created blli:%2.2x %2.2x fd: %d",
             sap->blli[0].l3.tr9577.snap[3],
             sap->blli[0].l3.tr9577.snap[4],
             conn->fd);
        
        return conn;
}

/* Accepts a new connection from listen socket in conn.
 * Returns NULL for error
 */
Conn_t *accept_conn(Conn_t *conn)
{
        Conn_t *new;
        struct sockaddr_atmsvc addr;
        size_t len;
        int fd;
        char buff[MAX_ATM_ADDR_LEN+1];

        diag(COMPONENT, DIAG_DEBUG, "Accepting connection on fd %d", conn->fd);
        len = sizeof(addr);
        fd = accept(conn->fd, (struct sockaddr *)&addr, &len);
        diag(COMPONENT, DIAG_DEBUG, "accept returned %d", fd);
        if (fd < 0) {
                diag(COMPONENT, DIAG_ERROR, "accept: %s", strerror(errno));
                return NULL;
        }
        if (atm2text(buff, sizeof(buff), (struct sockaddr *)&addr, A2T_PRETTY | A2T_NAME | A2T_LOCAL) < 0)
                sprintf(buff, "<Unknown ATM address>");
        diag(COMPONENT, DIAG_DEBUG, "Call from %s", buff);

        new = list_add_conn(addr.sas_addr.prv);
        if (new == NULL) return NULL;
        new->fd = fd;
        new->status = CONNECTED;
        new->type = THEYMADE;
        if (conn == lec_params.ctrl_listen) new->codepoint  = CONTROL_CONN;
        if (conn == lec_params.mcast_listen) new->codepoint = MCAST_CONN;
        if (conn == lec_params.data_listen) new->codepoint  = DATA_DIRECT_CONN;

        return new;
}

/* Close all connections, except the kernel socket.
 */
void close_connections(void)
{
        Conn_t *conn, *next;

        for(conn = connlist; conn; conn = next) {
		next = conn->next;
		if (conn->type != KERNEL_SOCK) {
			diag(COMPONENT, DIAG_DEBUG, "Destroying:%p fd:%d type:%d",
			     conn, conn->fd, conn->type);
			close(conn->fd);
			list_remove_conn(conn);
			free(conn);
		}
        }

        return;
}

/* Closes a connection and checks its importance.
 * Important connections are kernel socket, LES connections,
 * BUS Default Multicast Send VCC, last Multicast Forward VCC from Bus
 * and any of the listen sockets.
 * Returns < 0 for important connection.
 */
int close_connection(Conn_t *conn)
{
        int bad = 0;
        Conn_t *mcast;

        diag(COMPONENT, DIAG_DEBUG, "close_connection %p", conn);

        if (conn == lec_params.kernel ||
            conn == lec_params.ctrl_direct  ||
            conn == lec_params.ctrl_dist    ||
            conn == lec_params.mcast_send   ||
            conn == lec_params.mcast_listen ||
            conn == lec_params.data_listen)
                bad = -1;
        else {
                bad = -1;
                for (mcast = connlist; mcast; mcast = mcast->next)
                        if (mcast != conn &&
                            mcast->type == THEYMADE &&
                            mcast->codepoint == MCAST_CONN)
                                bad = 0;
        }

        close(conn->fd);
        list_remove_conn(conn);
        free(conn);
        
        return bad;
}

/* Accepts a new incoming Data Direct or Multicast Forward connection.
 * Control connections (LECS/LES) are accepted during configuration/join.
 * Returns < 0 for serious error such as broken listen socket.
 */
static int handle_accept(Conn_t *conn)
{
        Conn_t *new, *existing = NULL;
        struct atmlec_ioc ioc;
        
        new = accept_conn(conn);
        if (new == NULL) return -1;

        if (conn == lec_params.mcast_listen) {
                diag(COMPONENT, DIAG_DEBUG, "Multicast Forward VCC accepted");
                ioc.receive = 2;
        } else {
                diag(COMPONENT, DIAG_DEBUG, "Data Direct VCC accepted");
                ioc.receive = 0;
                if ((existing = conn_already_exists(new->atm_address, new)) &&
		    /* is calling address > called address */
		    (memcmp(new->atm_address, lec_params.c1n_my_atm_addr, ATM_ESA_LEN) > 0)) {
			diag(COMPONENT, DIAG_DEBUG, "Using it only to receive, spec 8.1.13");
			ioc.receive = 1;
			existing = NULL;
		}
        }
        memcpy(ioc.atm_addr, new->atm_address, ATM_ESA_LEN);
        ioc.dev_num = lec_params.itf_num;
        diag(COMPONENT, DIAG_DEBUG, "Attaching a new VCC, fd %d", new->fd);
        if (ioctl(new->fd, ATMLEC_DATA, &ioc) < 0) {
                diag(COMPONENT, DIAG_ERROR, "VCC attach failed: ioctl: %s", strerror(errno));
                return -1;
        }

	if (existing && existing->status == CONNECTED) {
		diag(COMPONENT, DIAG_DEBUG, "Closing old data direct, fd %d", existing->fd);
		close_connection(existing);
	}

        return 0;
}

/* Reads a LE control frame from conn, usually Data Direct or
 * Multicast Forward connection. Calls the incoming packet
 * handler function.
 * Returns < 0 for serious error such as broken LES connection
 */
static int handle_data(Conn_t *conn)
{
        char buff[MAX_CTRL_FRAME];
        int retval;

        retval = recv_frame(conn, buff, sizeof(buff));
        if (retval < 0) {
                diag(COMPONENT, DIAG_ERROR, "handle_data: read: %s", strerror(errno));
                return (close_connection(conn));
        }
        if (retval == 0) {
                diag(COMPONENT, DIAG_DEBUG, "fd %d, Data or Multicast VCC closed", conn->fd);
                return (close_connection(conn));
        }

        return handle_frame(conn, buff, retval);
}

/* Checks connections in *fds. The only allowed sockets
 * in *fds are listen sockets, data direct and control
 * sockets.
 * Returns < 0 for serious error such as broken LES connection
 */
int check_connections(fd_set *fds)
{
        Conn_t *conn, *next;

        conn = connlist;
        while (conn != NULL) {
                next = conn->next;
                if (!FD_ISSET(conn->fd, fds)) {
                        conn = next;
                        continue;
                }

                switch (conn->type) {
                case LISTENING:
                        if (handle_accept(conn) < 0)
                                return -1;
                        break;
                case WEMADE:
                case THEYMADE:
                        if (handle_data(conn) < 0)
                                return -1;
                        break;
                default:
                        diag(COMPONENT, DIAG_ERROR, "check_connections: bad_type '%s'",
                             get_type_string(conn->type));
                        break;
                }
                
                conn = next;
        }
        
        return 0;
}

/* Completes a non-blocking connect.
 * Returns < 0 for serious error
 */
static int handle_connect(Conn_t *conn)
{
        int retval;
        struct sockaddr_atmsvc dummy;
        struct atmlec_msg msg;
        struct atmlec_ioc ioc;

        diag(COMPONENT, DIAG_DEBUG, "handle_connect: completing fd %d", conn->fd);
        /* this seems to be common method in Linux-ATM
         * making sure that nonblocking connect was
         * completed successfully
         */
        conn->status = CONNECTED;
        retval = connect(conn->fd, (struct sockaddr *)&dummy, sizeof(struct sockaddr_atmsvc));
        if (retval < 0) {
                diag(COMPONENT, DIAG_DEBUG, "handle_connect: connect: %s", strerror(errno));
                delete_addr(conn->atm_address);
                close_connection(conn);
                return 0;
        }

        memcpy(ioc.atm_addr, conn->atm_address, ATM_ESA_LEN);
        ioc.dev_num = lec_params.itf_num;
        ioc.receive = 0;
        diag(COMPONENT, DIAG_DEBUG, "Attaching a new active VCC, fd %d", conn->fd);
	if (conn_already_exists(conn->atm_address, conn) &&
	    /* is calling address > called address */
	    memcmp(lec_params.c1n_my_atm_addr, conn->atm_address, ATM_ESA_LEN) > 0) {
		diag(COMPONENT, DIAG_DEBUG, "Receive only, spec 8.1.13 -- Closing");
                close_connection(conn);
                return 0;
	}
        if (ioctl(conn->fd, ATMLEC_DATA, &ioc) < 0) {
                diag(COMPONENT, DIAG_ERROR, "VCC attach failed: ioctl: %s", strerror(errno));
                return -1;
        }

        send_ready_ind(conn);

        memset(&msg, 0, sizeof(struct atmlec_msg));
        msg.type = l_flush_tran_id;
        memcpy(msg.content.normal.atm_addr, conn->atm_address, ATM_ESA_LEN);
        msg.content.normal.flag = send_flush_req(conn);
        
        msg_to_kernel(&msg, sizeof(struct atmlec_msg));


        return 0;
}

/* Complete non-blocking connections in *fds.
 * Returns < 0 for serious error (problems with kernel). 
 */
int complete_connections(fd_set *fds)
{
        Conn_t *conn, *next;
        int retval;

        conn = connlist;
        while (conn) {
                next = conn->next;
                if (FD_ISSET(conn->fd, fds)) {
                        retval = handle_connect(conn);
                        if (retval < 0) return -1;
                }
                conn = next;
        }
                
        return 0;
}

/* Send a LE control frame using *conn.
 * Returns < 0 for serious error
 */
int send_frame(Conn_t *conn, void *frame, int length)
{
        struct frame_hdr *hdr;
        int ret;

        diag(COMPONENT, DIAG_DEBUG, "send_frame: fd:%d len:%ld", conn->fd, length);
        hdr = (struct frame_hdr *)frame;
        if (hdr->opcode == htons(READY_QUERY) ||
            hdr->opcode == htons(READY_IND))
                diag(COMPONENT, DIAG_DEBUG, "%s", opcode2text(hdr->opcode));
        else 
                display_frame(frame);

        ret = write(conn->fd, frame, length);
        if (ret < 0) {
                diag(COMPONENT, DIAG_ERROR, "send_frame: write: %s", strerror(errno));
                return -1;
        } 

        return ret;
}

/* Receive a LE control frame from *conn.
 * Returns < 0 for serious error.
 */
int recv_frame(Conn_t *conn, void *buff, int length)
{
        int ret;

        diag(COMPONENT, DIAG_DEBUG, "recv_frame: fd:%d", conn->fd);
        ret = read(conn->fd, buff, length);
        if (ret < 0) {
                diag(COMPONENT, DIAG_ERROR, "Read failed: %s", strerror(errno));
                return -1;
        } 

#if 0
        diag(COMPONENT, DIAG_DEBUG, "recv_frame: read %d bytes", ret);
        if (get_verbosity(COMPONENT) >= DIAG_DEBUG) {
                int i;
                for (i = 0; i < 11; i++)
                        diag(COMPONENT, DIAG_DEBUG, "0x%2x", ((unsigned char *)buff)[i]);
        }
#endif

        return ret;
}


/*
 * LANE2: 5.2.1.4 and others, sleep random time before trying to reconnect
 */
void random_delay(void)
{
        struct timeval tv;
        int millis, interval;
        
        srand(time(NULL));
        interval = lec_params.c38_max_reconfig_delay - lec_params.c37_min_reconfig_delay;
        millis = (rand() % interval) + lec_params.c37_min_reconfig_delay;
        tv.tv_sec  = (millis - (millis % 1000)) / 1000;
        tv.tv_usec = (millis % 1000) * 1000;
        
        diag(COMPONENT, DIAG_DEBUG, "random_delay: sleeping %d.%d seconds", tv.tv_sec, tv.tv_usec);
        (void)select(0, NULL, NULL, NULL, &tv);
        
        return;
}

/* Collect already connected sockets in *fds
 */
void conn_get_fds(fd_set *fds)
{
        Conn_t *conn;

        diag(COMPONENT, DIAG_DEBUG, "collecting ready fds ");
        conn = connlist;
        while (conn) {
                if (conn->status != CONNECTING) {
                        FD_SET(conn->fd, fds);
                        diag(COMPONENT, DIAG_DEBUG, "%d type %s", conn->fd, get_type_string(conn->type));
                }
                conn = conn->next;
        }
        
        return;
}

/* Collect non-blocking connecting sockets in *fds
 */
void conn_get_connecting_fds(fd_set *fds)
{
        Conn_t *conn;

        diag(COMPONENT, DIAG_DEBUG, "collecting connecting fds ");
        conn = connlist;
        while (conn) {
                if (conn->status == CONNECTING) {
                        FD_SET(conn->fd, fds);
                        diag(COMPONENT, DIAG_DEBUG, "%d", conn->fd);
                }
                conn = conn->next;
        }
        
}

/* Creates Conn_t for fd and marks it as kernel socket
 * Returns < 0 for error
 */
int conn_set_kernel_socket(int fd)
{
        Conn_t *conn;

        conn = list_add_conn(NULL);
        if (conn == NULL) {
                diag(COMPONENT, DIAG_ERROR, "conn_set_kernel_socket: list_add_conn failed");
                return -1;
        }
        conn->type = KERNEL_SOCK;
        conn->status = CONNECTED;
        conn->fd = fd;
        lec_params.kernel = conn;

        return fd;
}

/* Creates new Conn_t object and allocates memory for it.
 * atm_addr should be the ATM address of the other end
 * or NULL if not applicable
 */
static Conn_t *list_add_conn(unsigned char *atm_addr)
{
        Conn_t *conn;

        conn = (Conn_t *)malloc(sizeof(Conn_t));
        if (!conn)
                return NULL;

        memset(conn, 0, sizeof(Conn_t));
        if (atm_addr)
                memcpy(conn->atm_address, atm_addr, ATM_ESA_LEN);

        conn->next = connlist;
        conn->previous = NULL;
        if (connlist)
                connlist->previous = conn;
        connlist = conn;
        diag(COMPONENT, DIAG_DEBUG, "Added conn:%p", conn);

        return conn;
}

/* Helper for close_connection and close_connections
 */
static void list_remove_conn(Conn_t *conn)
{

        if (conn->next == NULL && conn->previous == NULL
            && connlist != conn) return;
        diag(COMPONENT, DIAG_DEBUG, "Removing conn:%p fd:%d previous:%p next:%p ",
             conn, conn->fd, conn->previous, conn->next);

        if (conn->previous) 
                diag(COMPONENT, DIAG_DEBUG, "Previous:%p, fd:%d, next:%p, previous:%p ",
                     conn->previous, conn->previous->fd,
                     conn->previous->next, conn->previous->previous);
        if (conn->next)
                diag(COMPONENT, DIAG_DEBUG, "Next:%p, fd:%d next:%p, previous:%p ",
                     conn->next, conn->next->fd,
                     conn->next->next, conn->next->previous);  
        if (conn->previous) {
                conn->previous->next = conn->next;
        } else /* First in line */
                connlist = conn->next;
        if (conn->next)
                conn->next->previous = conn->previous;  
        diag(COMPONENT, DIAG_DEBUG, "Connlist: %p", connlist);
        conn->next=conn->previous= NULL;
        
        return;
}

static const char *get_type_string(int type)
{
        switch(type) {
        case WEMADE:
                return "WEMADE";
                break;
        case THEYMADE:
                return "THEYMADE";
                break;
        case LISTENING:
                return "LISTENING";
                break;
        case KERNEL_SOCK:
                return "KERNEL_SOCK";
                break;
        default:
                break;
        }
        
        return "UNKNOWN";
}

static int maxmtu2maxsdu(uint8_t mtu)
{

    int sdu;

    switch (mtu) {
    case MTU_1516:
        sdu = 1516;
        break;
    case MTU_1580:  /* LANE2: MTU can be 1580 too (IEEE 802.1p/Q) */
        sdu = 1580;
        break;
    case MTU_4544:
        sdu = 4544;
        break;
    case MTU_9234:
        sdu = 9234;
        break;
    case MTU_18190:
        sdu = 18190;
        break;
    default:
        sdu = 1516;
        break;
    }

    return sdu;
}

int maxmtu2itfmtu(uint8_t mtu)
{

    int sdu;

    switch (mtu) {
    case MTU_1516:
        sdu = 1500;
        break;
    case MTU_1580:  /* LANE2: MTU can be 1580 too (IEEE 802.1p/Q) */
        sdu = 1500;
        break;
    case MTU_4544:
        sdu = 4528;
        break;
    case MTU_9234:
        sdu = 9218;
        break;
    case MTU_18190:
        sdu = 18174;
        break;
    default:
        sdu = 1500;
        break;
    }

    return sdu;
}

/* Convert a type of connection (CONTROL_CONN, DATA_DIRECT and MCAST_CONN)
 * to a BLLI codepoint which depends on C2 LAN Type.
 */
static uint16_t conn_type2codepoint(int conn_type)
{
        if (conn_type == CONTROL_CONN) return CONTROL_CONN;

        if (conn_type == DATA_DIRECT_CONN) {
                if (lec_params.c2_lan_type == LAN_TYPE_8023 || lec_params.c2_lan_type == LAN_TYPE_UNSPEC)
                        return DATA_DIRECT_8023;
                else if (lec_params.c2_lan_type == LAN_TYPE_8025)
                        return DATA_DIRECT_8025;
                diag(COMPONENT, DIAG_ERROR, "conn_type2codepoint, bad lan_type %d", lec_params.c2_lan_type);
                return DATA_DIRECT_8023;
        }
        else if (conn_type == MCAST_CONN) {
                if (lec_params.c2_lan_type == LAN_TYPE_8023 || lec_params.c2_lan_type == LAN_TYPE_UNSPEC)
                        return MCAST_CONN_8023;
                else if (lec_params.c2_lan_type == LAN_TYPE_8025)
                        return MCAST_CONN_8025;
                diag(COMPONENT, DIAG_ERROR, "conn_type2codepoint, bad lan_type %d", lec_params.c2_lan_type);
                return MCAST_CONN_8023;
        }

        diag(COMPONENT, DIAG_ERROR, "conn_type2codepoint, unknown type %d", conn_type);

        return DATA_DIRECT_8023;
}

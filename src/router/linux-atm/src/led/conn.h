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

#ifndef CONN_H
#define CONN_H

typedef struct _conn_t_ {
        int fd;                  /* Socket connected to this connection */
        int status;              /* Connection status */
        int type;                /* We made, they made, listen socket */
        int codepoint;           /* One of the below, MCAST_CONN etc. */
        unsigned char atm_address[ATM_ESA_LEN]; 
                                 /* Destination address, not in listen or
                                    kernel sockets */
        struct _conn_t_ *next;
        struct _conn_t_ *previous;
} Conn_t;

void close_connections(void);
void init_conn_params(struct atm_sap *sap, struct atm_qos *qos,
                      uint16_t blli_codepoint);
int create_data_svc(unsigned char *atm_addr, int codepoint);
Conn_t *setup_svc(struct sockaddr_atmsvc *dst_addr,
                  struct sockaddr_atmsvc *listen_addr,
                  struct atm_sap *sap, struct atm_qos *qos);
int create_data_listen(void);
Conn_t *create_listensocket(struct sockaddr_atmsvc *listen_addr,
                            struct atm_sap *sap, struct atm_qos *qos);
Conn_t *accept_conn(Conn_t *conn);
int close_connection(Conn_t *conn);
int check_connections(fd_set *fds);
int complete_connections(fd_set *fds);

int send_frame(Conn_t *conn, void *frame, int length);
int recv_frame(Conn_t *conn, void *buff, int length);

void conn_get_fds(fd_set *fds);
void conn_get_connecting_fds(fd_set *fds);

int conn_set_kernel_socket(int fd);
void random_delay(void);

int maxmtu2itfmtu(uint8_t mtu);

/*
 * Connection types for BLLI codepoints.
 */
#define CONTROL_CONN      1
#define DATA_DIRECT_CONN  2
#define MCAST_CONN        4

#define DATA_DIRECT_8023  2
#define DATA_DIRECT_8025  3
#define MCAST_CONN_8023   4
#define MCAST_CONN_8025   5

/*
 * MTU sizes
 */
#define MTU_UNSPEC 0
#define MTU_1516   1
#define MTU_1580   5   /* LANEv2 */
#define MTU_4544   2
#define MTU_9234   3
#define MTU_18190  4

#endif /* CONN_H */

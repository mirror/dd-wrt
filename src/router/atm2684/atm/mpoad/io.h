#ifndef _MPOA_IO_H
#define _MPOA_IO_H
#include<atm.h>

#define MAX_PACKET_LENGTH 4096   /* max size of MPOA ctrl or data packet */

void main_loop(int listen_socket);
int send_to_mps(char *buff, int length);
int send_to_dataplane(char *buff, int length, int fd);
void keep_alive_sm(unsigned keep_alive_lifetime, int sequence_number);
int get_socket(struct sockaddr_atmsvc *address);
int get_listen_socket(struct sockaddr_atmsvc *address);
void create_ingress_svc(uint32_t ipaddr, char *atm_addr, struct atm_qos qos);

/* Socket types and states */
#define NOT_USED          0x0000
#define KERNEL            0x0001
#define OUTGOING_CTRL     0x0002
#define INCOMING_CTRL     0x0004
#define LISTENING_DATA    0x0010
#define LISTENING_CTRL    0x0020
#define OUTGOING_SHORTCUT 0x0100
#define INCOMING_SHORTCUT 0x0200

#define CONNECTING        0x1000         /* For outgoing sockets */
#define CONNECTED         0x2000         /* For outgoing sockets */


/* states for outgoing ingress shortcuts */
#define INGRESS_NOT_USED         0
#define INGRESS_REQUEST_SEND   400
#define INGRESS_CONNECTING     401
#define INGRESS_CONNECTED      402


#endif /* _MPOA_IO_H */

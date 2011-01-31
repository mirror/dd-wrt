/*
 * Functions to send packets
 *
 * $Id: packet.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* System includes */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Local includes */
#include "mem.h"
#include "lane.h"
#include "dump.h"
#include "load.h"
#include "connect.h"
#include "events.h"
#include "db.h"
#include "packet.h"

int
forward_arp_request(LaneControl_t *to_forward, 
		    Proxy_t *proxyl)
{
  Proxy_t *tmp;
  int a;

  Debug_unit(&conn_unit,"Forward_arp called");

  tmp = proxyl;

  while(tmp) {
    a = write(tmp->fd, to_forward, sizeof(LaneControl_t));   
    if (a == -1)
      dump_error(&conn_unit, "Forward arp failed");
    tmp = tmp->next;
  }
  return 1;
}


int
send_arp_response(int fd, LaneControl_t *lc,
		  unsigned int status, Reg_t *found)
{
  
  Debug_unit(&conn_unit,"Send_arp_response called to %d",fd);

  lc->opcode = htons(LE_ARP_RESPONSE);
  lc->status = (unsigned short)htons((0xffff & status));

  /* Clear the potential Remote flag */
  if ((lc->flags & htons(LE_FLAG_REMOTE)) == htons(LE_FLAG_REMOTE)) {
    lc->flags = (lc->flags ^ htons(LE_FLAG_REMOTE));
  }
  if (found) {
    memcpy(&lc->target_addr,&found->atm_address, sizeof(lc->target_addr));
  }
  if (send_control_frame(fd, lc) == 0) {
    dump_error(&conn_unit,"Send arp response failed");
    return 0;
  }
  return 1;
}

int
send_register_response(int fd, LaneControl_t *lc,
		       unsigned int status, int reg)
{

  Debug_unit(&conn_unit,"Send_register_response called to %d",fd);
  if (reg == 1)
    lc->opcode = htons(LE_REGISTER_RESPONSE);
  else
    lc->opcode = htons(LE_UNREGISTER_RESPONSE);
  lc->status = (unsigned short)htons(0xffff & status);

  if (send_control_frame(fd, lc) == 0) {
    dump_error(&conn_unit,"Send register response failed");
    return 0;
  }
  return 1;
}

int
send_join_response(int fd, LaneControl_t *lc, int lecid, 
		   unsigned int status)
{
  Debug_unit(&conn_unit,"Send_join_response called to %d",fd);
  lc->opcode = htons(LE_JOIN_RESPONSE);
  lc->status = (unsigned short)htons(0xffff & status);
  lc->lecid = lecid;

  if (send_control_frame(fd, lc) == 0) {
    dump_error(&conn_unit,"Send join response failed");
    return 0;
  }
  return 1;
}

int
send_control_frame(int fd, LaneControl_t *to_send)
{
  int a;

  Debug_unit(&conn_unit,"Send control frame");
  dump_control(to_send);
  a = write(fd, to_send, sizeof(LaneControl_t));
  if (a == -1) {
    dump_error(&conn_unit,"Write error");
    return 0;
  }
  return 1;
}



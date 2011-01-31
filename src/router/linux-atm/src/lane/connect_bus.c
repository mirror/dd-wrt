/*
 * ATM connection wrapper
 *
 * $Id: connect_bus.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $
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
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>

/* Local includes */
#include "mem.h"
#include "lane.h"
#include "dump.h"
#include "load.h"
#include "connect.h"
#include "events.h"
#include "lane_atm.h"

/* Type definitions */

/* Local function prototypes */
static void conn_init0(void);
static void conn_init1(void);
static void conn_dump(void);
static void conn_release(void);
static const char *dump_conn_type(ConnType_t type);

static int data_handler(const Event_t *event, void *funcdata);
static void conn_main(void);

/* Had to remove const qualifiers since not every
 * function honored it. hessu@cs.tut.fi
 */
static int join(Conn_t *conn);
static int idle_bad(Conn_t *conn);

static int data_forward(Conn_t *conn);
static int join_close(Conn_t *conn);

/* Data */
#define BUFSIZE 20000

const Unit_t conn_unit = {
  "conn",
  &conn_init0,
  &conn_init1,
  &conn_dump,
  &conn_release
};

static Conn_t *connlist;

unsigned char *data_packet;
unsigned int data_packet_size;

static const char *rcsid = "$Id: connect_bus.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $";

static State_t
  /* CS_IDLE */
  is_1 = { CE_SVC_OPEN, 0,  "Idle->Oper", join, CS_OPERATIONAL },
  is_2 = { CE_SVC_OPEN, 0, "Idle->Idle", idle_bad, CS_IDLE },
  *idle_state[] = { &is_1, &is_2, NULL },

  /* CS_OPERATIONAL */
  os_1 = { CE_DATA, 0, "Oper->Oper", data_forward, CS_OPERATIONAL },
/*  os_2 = { CE_DATA, LE_FLUSH_REQUEST, "Active->Active", flush_request,
	     CS_ACTIVE }, */
  os_3 = { CE_SVC_CLOSE, 0, "Oper->Idle", join_close, CS_IDLE },
  *operational_state[] = { &os_1, /*&as_2,*/ &os_3, NULL },

  **transitions[CS_MAX + 1] = {
    idle_state, NULL, operational_state
  };

/* Functions */

/* Initialize local data */
static void
conn_init0(void)
{
  connlist = NULL;
}

/* Initialization for data that needs other units */
static void
conn_init1(void)
{
  set_var_str(&conn_unit, "version", rcsid);
  conn_main();
  add_event_handler(CE_DATA, &data_handler, "data_handler", NULL);
  Debug_unit(&conn_unit, "Initialized.");
}

/* Dump status, local data etc. */
static void
conn_dump(void)
{
  const char *tmp;
  const AtmAddr_t *addr;

  Debug_unit(&conn_unit, "Dumping unit");
  Debug_unit(&conn_unit, "Parameters:");
  addr = get_var_addr(&conn_unit, "S1");
  if (addr != NULL) {
    Debug_unit(&conn_unit, "S1:");
    dump_atmaddr(addr);
  }
  else {
    Debug_unit(&conn_unit, "S1: not set");
  }
  tmp = get_var_str(&conn_unit, "S2");
  if (tmp != NULL) {
    Debug_unit(&conn_unit, "S2: %s", tmp);
  }
  else {
    Debug_unit(&conn_unit, "S2: not set", tmp);
  }
  Debug_unit(&conn_unit, "S3: %d", get_var_int(&conn_unit, "S3"));
  Debug_unit(&conn_unit, "S4: %d", get_var_int(&conn_unit, "S4"));
  Debug_unit(&conn_unit, "S5: %d", get_var_int(&conn_unit, "S5"));
  addr = get_var_addr(&conn_unit, "S6");
  if (addr != NULL) {
    Debug_unit(&conn_unit, "S6:");
    dump_atmaddr(addr);
  }
  else {
    Debug_unit(&conn_unit, "S6: not set");
  }
  dump_conn(NULL);
}

/* Release allocated memory, close files etc. */
static void
conn_release(void)
{
  Conn_t *tmp;
  
  Debug_unit(&conn_unit, "Releasing unit");
  
  for(tmp = connlist; tmp != NULL; tmp = tmp->next) {
    connlist = tmp->next;
    if (tmp->fd)
      close(tmp->fd);
    if (tmp->sfd)
      close(tmp->sfd);
    mem_free(&conn_unit, tmp);
  }
}

/*
 * Handle data arrival
 * data points to Conn_t
 */
static int
data_handler(const Event_t *event, void *funcdata)
{
  Conn_t *tmp, *newconn;
  int fd;
  socklen_t nbytes;
  static char buffer[BUFSIZE];
  struct sockaddr_atmsvc addr;

  assert(event->data != NULL);
  tmp = (Conn_t *)event->data;
  dump_conn(tmp);
  if (tmp->type == CT_MAIN) {
    nbytes = sizeof(addr);
    memset(&addr, 0,nbytes);
    fd = accept(tmp->fd, (struct sockaddr *)&addr, &nbytes);
    if (fd <0) {
      dump_error(&conn_unit, "accept");
      if (errno == ENETRESET) { /* Switch reseted? */
	Debug_unit(&conn_unit,"Restart. Sleeping 10 secs...");
	sleep(10);
	event_put(&conn_unit, CE_RESTART, NULL);
      } else if (errno == EUNATCH) { /* Probably signalling daemon was abruptly killed */
	Debug_unit(&conn_unit,"Exiting...");
	event_put(&conn_unit, CE_EXIT, NULL);
      }
      return -1;
    }
    newconn = conn_add(CT_SVC_CD, fd, 0);
    newconn->state = call_state(CE_SVC_OPEN, 0, newconn);
  }
  else {
    nbytes = read(tmp->active_fd, buffer, BUFSIZE);
    if (nbytes < 0) {
      dump_error(&conn_unit, "read");
      if (errno == EUNATCH)
	event_put(&conn_unit, CE_EXIT, NULL);
      if (errno == ENETRESET) {
	Debug_unit(&conn_unit, "Restart. Sleeping 10 secs...");
	sleep(10);
	event_put(&conn_unit, CE_RESTART, NULL);
      }
    } else if (nbytes == 0) {
      /* EOF */
      Debug_unit(&conn_unit, "EOF");
      tmp->state = call_state(CE_SVC_CLOSE, 0, tmp);
    } else {
      buffer[nbytes] = '\0';
      data_packet = (unsigned char *)buffer;
      data_packet_size=nbytes;
      tmp->state = call_state(CE_DATA, 0, tmp);
    }
  }
  mem_free(&conn_unit, event);
  return 1;
}

static const char *
dump_conn_type(ConnType_t type)
{
  switch(type) {
  case CT_NONE: return "None";
  case CT_MAIN: return "Main listener";
  case CT_PVC_CD: return "PVC Control Direct";
  case CT_PVC_DD: return "PVC Data Direct";
  case CT_SVC_CD: return "SVC Control Direct";
  case CT_SVC_DD: return "SVC Data Direct";
  default: return "Bad type";
  }
}

void
dump_conn(const Conn_t *connection)
{
  Conn_t *tmp;
  
  for(tmp = connlist; tmp != NULL; tmp = tmp->next) {
    if (connection == NULL || tmp == connection) {
      Debug_unit(&conn_unit, "fd %d sfd %d state %s type %s", tmp->fd, tmp->sfd, dump_conn_state(tmp->state), dump_conn_type(tmp->type));
    }
  }
}

Conn_t *
conn_add(ConnType_t type, int fd, LecId_t dumb)
{
  Conn_t *tmp;

  tmp = (Conn_t *)mem_alloc(&conn_unit, sizeof(Conn_t));
  tmp->fd = fd;
  tmp->sfd = 0;
  tmp->state = CS_IDLE;
  tmp->type = type;
  tmp->next = connlist;
  connlist = tmp;

  event_add_fd(fd, tmp);
  return tmp;
}

void
conn_remove(const Conn_t *connection)
{
  Conn_t *tmp, *prev;

  assert(connection != NULL);
  prev = NULL;
  tmp = connlist;
  while(tmp) {
    if (tmp == connection) {
      if (prev != NULL) {
	prev->next = tmp->next;
      } else {
	connlist = tmp->next;
      }
      if (connection->fd) {
	close(connection->fd);
	event_remove_fd(connection->fd);
      }
      if (connection->sfd) {
	close(connection->sfd);
	event_remove_fd(connection->sfd);
      }
      mem_free(&conn_unit, tmp);
      return;
    }
    prev = tmp;
    tmp = tmp->next;
  }
}

void
conn_set_active(void *data, int fd)
{
  Conn_t *tmp = (Conn_t *)data;

  assert(tmp);
  tmp->active_fd = fd;
}

static void
conn_main(void)
{
  const AtmAddr_t *addr;
  int main_conn;

  addr = get_var_addr(&conn_unit, "S6");
  if (addr == NULL) {
    dump_printf(EL_ERROR, "S6 (BUS Address) must be specified");
    event_put(&conn_unit, CE_EXIT, NULL);
  } else {
    main_conn = atm_create_socket(MULTICAST_SEND_802_3,
				  get_var_addr(&conn_unit, "S6"));
    if (main_conn >= 0) {
      (void)conn_add(CT_MAIN, main_conn, 0);
    }
  }
}

ConnState_t
call_state(EventType_t event, unsigned short opcode, Conn_t *conn)
{
  State_t **tmp;
  unsigned int i = 0;
  int ret;

  Debug_unit(&conn_unit, "Call state");
  assert(conn != NULL);
  assert(conn->state <= CS_MAX);
  tmp = transitions[conn->state];

  for(; tmp[i] != NULL; i++) {
    if (tmp[i]->event == event && tmp[i]->opcode == opcode) {
      Debug_unit(&conn_unit, "Trying func %s", tmp[i]->descript);
      ret = tmp[i]->func(conn);
      if (ret != 0){
	Debug_unit(&conn_unit, "Success");
	return tmp[i]->nextstate;
      }
      else {
	Debug_unit(&conn_unit, "Failed");
      }
    }
    else {
      Debug_unit(&conn_unit, "Skipping func %s", tmp[i]->descript);
    }
  }
  return conn->state;
}

const char *
dump_conn_state(ConnState_t state)
{
  switch(state) {
  case CS_IDLE: return "Idle";
  case CS_JOINING: return "Joining";
  case CS_OPERATIONAL: return "Operational";
  default: return "Bad state";
  }
}

/*
 * State transition functions
 */
static int
join(Conn_t *conn)
{
  int rfd;

  Debug_unit(&conn_unit, "Join called");
  dump_conn(conn);

  rfd = atm_connect_back(get_var_addr(&conn_unit,"S6"),
			 conn, MULTICAST_FORWARD_802_3);
  if (rfd<0) { /* Calling back failed */
    conn_remove(conn);
    return 0;
  } else { /* Success */
    Debug_unit(&conn_unit, "Join successful");
    conn->sfd = rfd;
    event_add_fd(rfd, conn);
    return 1;
  }
}

static int
idle_bad(Conn_t *conn)
{
  Debug_unit(&conn_unit,"Idle_bad called");
  dump_conn(conn);
  conn_remove(conn);
  return 1;
}

static int
data_forward(Conn_t *conn)
{
  Conn_t *tmp;
  char packet_string[1024];
  int rvalue;
  int i;

  Debug_unit(&conn_unit,"Data forward called");
  dump_conn(conn);

  for(tmp = connlist;tmp!=NULL;tmp=tmp->next) {
    if (tmp->sfd >0 && tmp->type != CT_MAIN) {
      rvalue=write(tmp->sfd, data_packet, data_packet_size);
      if (rvalue<0) {
	dump_error(&conn_unit, "data_forward,write");
	dump_conn(tmp);
      } else
	Debug_unit(&conn_unit,"Forwarding to %d-%d bytes",tmp->sfd,
		   data_packet_size);
    }
  }
  Debug_unit(&conn_unit,"Sent packet :%ld bytes",data_packet_size);
  for (i=0;i<data_packet_size && i < 83;i++) {
    sprintf(&packet_string[3*i],"%2.2x ",0xff&data_packet[i]);
  }
  if (i==data_packet_size)
    Debug_unit(&conn_unit,"Packet:->%s<-",packet_string);
  else
    Debug_unit(&conn_unit,"Packet:->%s->",packet_string);
  Debug_unit(&conn_unit,"Returning from data forward");
  return 1;
}

static int
join_close(Conn_t *conn)
{

  Debug_unit(&conn_unit,"Join_close called");

  dump_conn(conn);
  conn_remove(conn);
  return 1;
}


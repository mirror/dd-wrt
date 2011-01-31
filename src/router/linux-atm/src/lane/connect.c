/*
 * ATM connection wrapper
 *
 * $Id: connect.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $
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
#include "db.h"
#include "packet.h"
#include "lane_atm.h"

/* Type definitions */

/* Local function prototypes */
static void conn_init0(void);
static void conn_init1(void);
static void conn_dump(void);
static void conn_release(void);
static const char *dump_conn_type(ConnType_t type);
/*
static void add_pvcs(void);
*/
static int data_handler(const Event_t *event, void *funcdata);
static int timer_handler(const Event_t *event, void *funcdata);
static void conn_main(void);

/*
 * as in connect_bus.c had to remove const qualifiers since
 * not every function honored it
 */
static int join(Conn_t *conn);
static int join_ok(Conn_t *conn);
static int join_bad(Conn_t *conn);
static int join_close(Conn_t *conn);
static int join_expire(Conn_t *conn);
static int idle_bad(Conn_t *conn);
static int topology_request(Conn_t *conn);

static int register_req(Conn_t *conn);
static int unregister_req(Conn_t *conn);

static int arp_find(Conn_t *conn);
static int arp_forward_response(Conn_t *conn);

static int forward_flush_response(Conn_t *conn);

static int is_multicast(const LaneDestination_t *to_detect);
static int proper_request(void);
static Bool_t is_proxy(void);
static int is_control(LaneControl_t *c);

/* Data */
#define BUFSIZE 256
#define PORT 8621U

const Unit_t conn_unit = {
  "conn",
  &conn_init0,
  &conn_init1,
  &conn_dump,
  &conn_release
};

static Conn_t *connlist;

extern Reg_t *reglist;

extern Proxy_t *proxylist;

extern Lecdb_t *leclist;

LaneControl_t *control_packet;

static const char *rcsid = "$Id: connect.c,v 1.2 2001/10/09 22:33:06 paulsch Exp $";

static State_t
  /* CS_IDLE */
  is_1 = { CE_SVC_OPEN, 0,  "Idle->Add_Party", join, CS_JOINING },
  is_2 = { CE_SVC_OPEN, 0, "Idle->Idle", idle_bad, CS_IDLE },
  *idle_state[] = { &is_1, &is_2, NULL },
    
  /* CS_JOINING */
  js_1 = { CE_DATA, LE_JOIN_REQUEST, "Join->Oper", join_ok, CS_OPERATIONAL },
    /*
  js_2 = { CE_DATA, LE_JOIN_REQUEST, "Join->Add_P", join_ap_ok, CS_ADD_PARTY },
  */
  js_3 = { CE_DATA, LE_JOIN_REQUEST, "Join->Join", join_bad, CS_JOINING },
  js_5 = { CE_SVC_CLOSE, 0, "Join->Idle", join_close, CS_IDLE },
  js_6 = { CE_TIMER, 0, "Join->Idle", join_expire, CS_IDLE },
  *join_state[] = { &js_1, &js_3, &js_5, &js_6, NULL },

  /* CS_OPERATIONAL */
  os_1 = { CE_DATA, LE_JOIN_REQUEST, "Oper->Oper JOIN", join_ok, CS_OPERATIONAL },
  os_2 = { CE_DATA, LE_JOIN_REQUEST, "Oper->Join", join_bad, CS_JOINING },
  os_3 = { CE_DATA, LE_REGISTER_REQUEST, "Oper->Oper REGISTER REQUEST", register_req, CS_OPERATIONAL },
  os_4 = { CE_DATA, LE_UNREGISTER_REQUEST, "Oper->Oper UNREGISTER REQUEST", unregister_req, CS_OPERATIONAL },
  os_5 = { CE_DATA, LE_ARP_REQUEST, "Oper->Oper ARP REQUEST", arp_find, CS_OPERATIONAL},
  os_6 = { CE_DATA, LE_TOPOLOGY_REQUEST, "Oper->Oper TOPOLOGY REQUEST", topology_request, CS_OPERATIONAL },
  os_7 = { CE_DATA, LE_ARP_RESPONSE, "Oper->Oper ARP RESPONSE FORWARD", arp_forward_response, CS_OPERATIONAL },
  os_8 = { CE_DATA, LE_FLUSH_RESPONSE, "Oper->Oper FLUSH FORWARD", forward_flush_response, CS_OPERATIONAL },
  os_9 = { CE_SVC_CLOSE, 0, "Oper->Idle", join_close, CS_IDLE },
  *oper_state[] = { &os_1, &os_2, &os_3, &os_4, &os_5, &os_6, &os_7, &os_8,
		      &os_9, NULL },

  **transitions[CS_MAX + 1] = {
    idle_state, join_state, oper_state
  };

/* Functions */

/* Initialize local data */
static void
conn_init0(void)
{
  connlist = NULL;
  reglist = NULL;
  proxylist = NULL;
  leclist = NULL;
}

/* Initialization for data that needs other units */
static void
conn_init1(void)
{
  set_var_str(&conn_unit, "version", rcsid);
  conn_main();
  add_event_handler(CE_DATA, &data_handler, "data_handler", NULL);
  add_event_handler(CE_TIMER, &timer_handler, "timer_handler", NULL);
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
  } else {
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
  Reg_t *rtmp;
  Lecdb_t *ltmp;
  Proxy_t *ptmp;
  
  Debug_unit(&conn_unit, "Releasing unit");
  
  for(tmp = connlist; tmp != NULL; tmp = tmp->next) {
    connlist = tmp->next;
    if (tmp->fd)
      close(tmp->fd);
    if (tmp->sfd)
      close(tmp->sfd);
    mem_free(&conn_unit, tmp);
  }
  for(rtmp = reglist; rtmp != NULL; rtmp = rtmp->next) {
    reglist = rtmp->next;
    mem_free(&conn_unit, rtmp);
  }
  for(ltmp = leclist; ltmp != NULL; ltmp = ltmp->next) {
    leclist = ltmp->next;
    mem_free(&conn_unit, ltmp);
  }
  for(ptmp = proxylist; ptmp != NULL; ptmp = ptmp->next) {
    proxylist = ptmp->next;
    mem_free(&conn_unit, ptmp);
  }

}

/* Validates control packet from header */

static int 
is_control(LaneControl_t *c)
{
  if (c->marker == htons(LE_MARKER) &&
      c->protocol == LE_PROTOCOL &&
      c->version == LE_VERSION)
    return 1;
  else
    return 0;
}

/* Used to detect if LaneDestination_t is
   multicast address */
static int 
is_multicast(const LaneDestination_t *to_detect)
{
  if (to_detect->tag == htons(LANE_DEST_MAC)) {
    return to_detect->a_r.mac_address[0] & 0x01;
  } else if (to_detect->tag == htons(LANE_DEST_RD)) {
    return to_detect->a_r.route.reserved[0] & 0x80;
  } else
  return 0;
}

/*
 * Handle new connections or data arrival
 * data points to Conn_t
 */
static int
data_handler(const Event_t *event, void *funcdata)
{
  Conn_t *tmp, *newconn;
  int fd;
  socklen_t nbytes;
  static char buffer[BUFSIZE];
  LaneControl_t *ctmp;
  struct sockaddr_atmsvc addr;

  assert(event->data != NULL);
  tmp = (Conn_t *)event->data;
  dump_conn(tmp);
  if (tmp->type == CT_MAIN) {
    nbytes = sizeof(addr);
    memset(&addr,0, nbytes);
    fd = accept(tmp->fd, (struct sockaddr *)&addr, &nbytes);
    if (fd <0) {
      dump_error(&conn_unit, "accept");
      if (errno == ENETRESET) {
	Debug_unit(&conn_unit,"Restart. Sleeping 10 secs...");
	sleep(10);
	event_put(&conn_unit, CE_RESTART, NULL);
      } else if (errno == EUNATCH) {
	Debug_unit(&conn_unit,"Exiting...");
	event_put(&conn_unit, CE_EXIT, NULL);
      }
      return -1;
    }
    newconn = conn_add(CT_SVC_CD, fd,0);
    newconn->state = call_state(CE_SVC_OPEN, 0, newconn);
  }
  else {
    /* tmp->fd or tmp->sfd ?*/
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
      Debug_unit(&conn_unit, "Data: %2.2x %2.2x %2.2x", 
		 0xff&buffer[0],0xff&buffer[1],0xff&buffer[2]);
      ctmp = (LaneControl_t *)buffer;
      if (is_control(ctmp) == 1) {
	control_packet = (LaneControl_t*)buffer;
	dump_control(ctmp);
	tmp->proxy = is_proxy();
	tmp->state = call_state(CE_DATA, ctmp->opcode, tmp);
      } else
	Debug_unit(&conn_unit,"Not a control_packet, discarding...");
    }
  }
  mem_free(&conn_unit, event);
  return 1;
}

static int
timer_handler(const Event_t *event, void *funcdata)
{
  Conn_t *tmp;

  assert(event->data != NULL);
  tmp = (Conn_t *)event->data;
  dump_conn(tmp);
  tmp->state = call_state(CE_TIMER, 0, tmp);
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
conn_add(ConnType_t type, int fd, LecId_t pvc_lecid)
{
  Conn_t *tmp;
  LecId_t lecid = 0;

  if (type != CT_MAIN && type != CT_PVC_CD) {
    /* Find next available LECID */
    for (tmp = connlist; tmp != NULL; tmp = tmp->next){
      if (lecid < tmp->lecid) {
	lecid = tmp->lecid;
      }
    }
    lecid++;
  } else if (type == CT_MAIN) {
    lecid = 0;
  } else /* PVC */
    lecid = pvc_lecid;
  tmp = (Conn_t *)mem_alloc(&conn_unit, sizeof(Conn_t));
  memset(tmp, 0, sizeof(*tmp));
  tmp->fd = fd;
  tmp->state = CS_IDLE;
  tmp->lecid = lecid;
  tmp->type = type;
  tmp->next = connlist;
  tmp->timer = timer_new(&conn_unit);
  tmp->proxy = BL_FALSE;
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
	event_remove_fd(connection->sfd);
	close(connection->sfd);
      }
      timer_free(&conn_unit, tmp->timer);
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
  const char *str;
  int i;

  addr = get_var_addr(&conn_unit, "S1");
  if (addr == NULL) {
    dump_printf(EL_ERROR, "S1 (LES Address) must be specified");
    event_put(&conn_unit, CE_EXIT, NULL);
  }
  else {
    str = get_var_str(&conn_unit,"S2");
    if (str == NULL) {
      set_var_str(&conn_unit, "S2", S2_default);
    }
    i = get_var_int(&conn_unit,"S3");
    if (i == 0) {
      set_var_int(&conn_unit,"S3", S3_default);
    }
    i = get_var_int(&conn_unit,"S4");
    if (i == 0) {
      set_var_int(&conn_unit,"S4", S4_default);
    }
    i = get_var_int(&conn_unit,"S5");
    if (i == 0) {
      set_var_int(&conn_unit,"S5", S5_default);
    }
    addr = get_var_addr(&conn_unit, "S6");
    if (addr == NULL) {
      dump_printf(EL_ERROR, "S6 (BUS Address) must be specified");
      event_put(&conn_unit, CE_EXIT, NULL);
    } else {
      main_conn = atm_create_socket(CONTROL_DIRECT, 
				    get_var_addr(&conn_unit, "S1"));
      if (main_conn >= 0) {
	(void)conn_add(CT_MAIN, main_conn,0);
      }
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
    if (tmp[i]->event == event && htons(tmp[i]->opcode) == opcode) {
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

#define get_max_frame(a) ((a==LE_MAX_FRAME_1516)?1516:((a==LE_MAX_FRAME_4544)?4544:((a==LE_MAX_FRAME_9234)?9234:18190)))

static int proper_request()
{
  if (control_packet != NULL &&

      /* Target address is our address */
      /* FI[S_ATM] == SV[S_ATM] */
/*      memcmp((char *) &control_packet->target_addr, 
	      (const char *) get_var_addr(&conn_unit,"S1"),
	      sizeof(AtmAddr_t)) == 0  && */
      
      /* Lan type is unspecified or same as configured to this server */
      /* FI[LAN Type] == "Unspecified" or FI[LAN Type] == S2 */
      (control_packet->lan_type == LE_LAN_TYPE_UNSPECIFIED ||
       strcmp(get_var_str(&conn_unit,"S2"),
	      ((control_packet->lan_type ==
		LE_LAN_TYPE_802_3)?"802.3":
	       (control_packet->lan_type ==
		LE_LAN_TYPE_802_5)?"802.5":"unspecified")) == 0) &&
      
      /* Max frame size is unspecified or FI[MAX MTU] >= S3 */
      (control_packet->max_frame == LE_MAX_FRAME_UNSPECIFIED ||
       get_var_int(&conn_unit, "S3") <=
       get_max_frame(control_packet->max_frame))
      /* TOADD: check for entry in REG-DB */
      ) {
    return 1;
  }
  return 0;
}

static Bool_t 
is_proxy(void)
{
  if ((control_packet->flags & htons(LE_FLAG_PROXY)) == htons(LE_FLAG_PROXY))
    return BL_TRUE;
  else
    return BL_FALSE;
}

/*
 * State transition functions
 */
static int
join(Conn_t *conn)
{
  int timeout;
  int rfd;

  Debug_unit(&conn_unit, "Join called");
  dump_conn(conn);
  timeout = get_var_int(&conn_unit, "S4");
  timer_alarm(&conn_unit, conn->timer, (unsigned)timeout, conn);
  rfd=atm_connect_back(get_var_addr(&conn_unit, "S1"), 
		       conn, CONTROL_DISTRIBUTE);
  if (rfd<0) {
    conn_remove(conn);
    return 0;
  }
  conn->sfd=rfd;
  event_add_fd(rfd, conn);
  return 1;
}

static int
join_ok(Conn_t *conn)
{
  Reg_t *rtmp = NULL;   /* silence gcc 2.7.2.1 */
  Lecdb_t *ltmp; 
  const char *elanname;

  Debug_unit(&conn_unit, "Join_ok called");  
  dump_conn(conn);

  if (control_packet != NULL &&
      proper_request() == 1) {
/*    dump_control(control_packet);*/

    /* Do not accept control packets with lecid set to unknown value*/
    if (control_packet->lecid != 0) {
      Debug_unit(&conn_unit,"Lecid set");
      ltmp = leciddb_find(control_packet->lecid);
      if (!ltmp || memcmp(&ltmp->address, &control_packet->source_addr,
			  sizeof(AtmAddr_t)) != 0) {
	send_join_response(conn->fd, control_packet, conn->lecid,
			   (unsigned int)LE_STATUS_BAD_LECID);
	return 0;
      }
    }

    /* Is lan destination address present? */
    if (control_packet->source.tag != htons(LANE_DEST_NP)) {
      rtmp = regdb_find_mac(control_packet->source);

      /* Do not accept lan multicast address */
      if (is_multicast(&control_packet->source)) {
	Debug_unit(&conn_unit,"Destination lan address is multicast");
	send_join_response(conn->fd, control_packet, conn->lecid,
			   ((unsigned int)LE_STATUS_BAD_DEST));
	return 0;
      }

      /* Duplicate registered Lan-destination, duplicate mac-address */
      if (rtmp && memcmp(&rtmp->atm_address,&control_packet->source_addr, 
			 sizeof(AtmAddr_t)) != 0) {
	Debug_unit(&conn_unit,"Duplicate Destination lan, duplicate mac");
	send_join_response(conn->fd, control_packet, conn->lecid,
			   ((unsigned int)LE_STATUS_DUPLICATE_REG));
	return 0;
      }
    }

    /* Duplicate atm-address */
    if ((ltmp =leciddb_find_atm(control_packet->source_addr)) != NULL &&
	ltmp->lecid != conn->lecid) {
      Debug_unit(&conn_unit,"Duplicate atmaddress");
      send_join_response(conn->fd, control_packet, conn->lecid,
			 ((unsigned int)LE_STATUS_DUPLICATE_ADDR));
      return 0;
    }
    /* If we have elan-name, check it agaist the one in join request */
    elanname = get_var_str(&conn_unit, "ELANNAME");
    if (elanname) {
      Debug_unit(&conn_unit, "Compare %s with %s",elanname, 
		 control_packet->elan_name);
      if ((strlen(elanname) != control_packet->elan_name_size) ||
	  strncmp(elanname, control_packet->elan_name, 
		  control_packet->elan_name_size)) {
	/* Don't match */
	Debug_unit(&conn_unit, "Invalid elan-name set");
	send_join_response(conn->fd, control_packet, conn->lecid,
			   ((unsigned int)LE_STATUS_NO_CONFIG));
	return 0;
      }
    }
    Debug_unit(&conn_unit,"Control_packet OK.");
    /* Control_packet OK. */
    if (leciddb_find(conn->lecid) == NULL) {
      leciddb_add(conn->lecid,
		  control_packet->source_addr, conn->fd);
    }
    if (control_packet->source.tag != htons(LANE_DEST_NP) && !rtmp) {
/*      dump_addr(&control_packet->source); */
      regdb_add(control_packet->source_addr, 
		control_packet->source);
    }
    /* Send join response */
    if ((conn->proxy == BL_TRUE || is_proxy() == BL_TRUE) &&
	proxydb_find(conn->lecid) == NULL) {
      proxydb_add((const Conn_t *)conn, conn->fd);
    }
    timer_ack(&conn_unit, conn->timer);
    send_join_response(conn->fd, control_packet, conn->lecid,
		       (unsigned short)LE_STATUS_SUCCESS);
    return 1;
  } 

  return 0;
}

static int
join_bad(Conn_t *conn)
{
  Debug_unit(&conn_unit, "Join_bad called");
  dump_conn(conn);

  if (proper_request()==0) {
    send_join_response(conn->sfd, control_packet, conn->lecid,
		       (unsigned short)LE_STATUS_BAD_REQ);
  }
  return 1;
}

static int
idle_bad(Conn_t *conn)
{
  Debug_unit(&conn_unit, "Idle bad called");
  dump_conn(conn);
  conn_remove(conn);
  return 1;
}

static int
join_close(Conn_t *conn)
{
  Lecdb_t *tmp;

  Debug_unit(&conn_unit, "Join_close called");
  dump_conn(conn);
  Debug_unit(&conn_unit,"Trying to remove PROXY_DB entry");
  if (proxydb_remove(conn) == 0)
    Debug_unit(&conn_unit,"Removal of PROXY_DB entry failed");
  else
    Debug_unit(&conn_unit,"PROXY_DB entry removed");

  tmp = leciddb_find(conn->lecid);
  if (tmp) {
    Debug_unit(&conn_unit, "Trying to remove REG_DB entry");
    if (regdb_remove(tmp->address) == 0)
      Debug_unit(&conn_unit, "Removal of REG_DB entry failed");
    else
      Debug_unit(&conn_unit, "REG_DB entry removed");
  }
/*  if (tmp) {
    Debug_unit(&conn_unit,"Trying to close connection");
    if (close(tmp->fd) < 0)
      Debug_unit(&conn_unit,"Failed");
    else
      Debug_unit(&conn_unit,"Success");
  }*/
  Debug_unit(&conn_unit,"Trying to remove LECID-DB entry");
  if (leciddb_remove(conn->lecid) == 0)
    Debug_unit(&conn_unit,"Removal of LECID-DB entry failed");
  else
    Debug_unit(&conn_unit,"LECID-DB entry removed");
  conn_remove(conn);
  Debug_unit(&conn_unit,"Conn removed");
  return 1;
}

static int
join_expire(Conn_t *conn)
{
  
  Debug_unit(&conn_unit, "Join_expire called");
  dump_conn(conn);

  conn_remove(conn);
  return 1;
}

static int
register_req(Conn_t *conn)
{
  Reg_t *tmp;
  Lecdb_t *ltmp;

  Debug_unit(&conn_unit, "Register_req called");
  dump_conn(conn);
  assert(control_packet != NULL);

  /* If trying to register a multicast or broadcast address, reject */
  if (is_multicast(&control_packet->source)) {
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_BAD_DEST, 1);
    return 1;
  }

  /* Check lecid */
  ltmp = leciddb_find(control_packet->lecid);
  if (!ltmp) {
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_BAD_LECID, 1);
    return 1;
  }
  tmp = regdb_find_mac(control_packet->source);
  if (tmp == NULL) {
    /* Unregistered MAC, registering... */
    dump_addr(&control_packet->source);
    dump_printf(EL_CONT,"\n");

    regdb_add(control_packet->source_addr, control_packet->source);
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_SUCCESS, 1);
  } else {
    if (memcmp(&tmp->atm_address,&control_packet->source_addr, 
	       sizeof(AtmAddr_t)) != 0) {
      Debug_unit(&conn_unit, "MAC Address is bound to another ATM address");
      send_register_response(conn->sfd, control_packet,
			     LE_STATUS_DUPLICATE_REG, 1);
    } else {
      Debug_unit(&conn_unit,"Duplicate registeration");
      send_register_response(conn->sfd, control_packet,
			     LE_STATUS_SUCCESS, 1);
    }
  }
  return 1;
}

static int
unregister_req(Conn_t *conn)
{
  Reg_t *tmp;
  Lecdb_t *ltmp;

  Debug_unit(&conn_unit, "Unregister_req called");
  dump_conn(conn);
  assert(control_packet != NULL);

  /* Reject attempt to unregister multicast & broadcast address */

  if (is_multicast(&control_packet->source)) {
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_BAD_DEST, 0);
    return 1;
  }
  ltmp = leciddb_find(control_packet->lecid);
  if (!ltmp) {
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_BAD_LECID, 0);
    return 1;
  }
  tmp = regdb_find_mac(control_packet->source);
  
  if (tmp && (memcmp(&tmp->atm_address, &control_packet->source_addr, 
		     sizeof(AtmAddr_t)) == 0)) {
    /* Removing registered MAC adress */
    regdb_remove(tmp->atm_address);
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_SUCCESS, 0);
    return 1;
  } else if (tmp) {
    /* MAC was registered by another client */
    send_register_response(conn->sfd, control_packet,
			   LE_STATUS_BAD_DEST, 0);
    return 1;
  } 
  /* MAC was not registered earlier */
  send_register_response(conn->sfd, control_packet,
			 LE_STATUS_SUCCESS, 0);
  return 1;
}

static int
arp_find(Conn_t *conn)
{
  Reg_t *tmp;
  Lecdb_t *ltmp;
  
  Debug_unit(&conn_unit, "Arp_find called");
  dump_conn(conn);

  Debug_unit(&conn_unit,"Arping for:");
  dump_addr(&control_packet->target);
  dump_printf(EL_CONT,"\n");

  /* If requested multicast /broadcast address, respond with BUS address */
  if (is_multicast(&control_packet->target)) {
    tmp = mem_alloc(&conn_unit, sizeof(Reg_t));
    memcpy(&tmp->atm_address, get_var_addr(&conn_unit, "S6"), 
	   sizeof(AtmAddr_t));
    Debug_unit(&conn_unit,"Arp for multicast address");
    send_arp_response(conn->sfd, control_packet,
		      LE_STATUS_SUCCESS, tmp);
    return 1;
  }

  /* Check lecid */
  ltmp = leciddb_find(control_packet->lecid);
  if (!ltmp) {
    send_arp_response(conn->sfd, control_packet,
		      LE_STATUS_BAD_LECID, NULL);
    return 1;
  }
  tmp = regdb_find_mac(control_packet->target);
  if (tmp) {
    Debug_unit(&conn_unit,"Address in databases");
    /* Send response */
    send_arp_response(conn->sfd, control_packet,
		      LE_STATUS_SUCCESS,
		      tmp);
    return 1;
  }
  forward_arp_request(control_packet, proxylist);
  return 1;
}

static int
arp_forward_response(Conn_t *conn)
{
  int a;
  Conn_t *tmp;

  Debug_unit(&conn_unit,"Arp forward response called");
  dump_conn(conn);
  /* We got response. Forward it to all */

  for (tmp = connlist;tmp;tmp=tmp->next) {
    if (tmp->sfd) {
      a = send_control_frame(tmp->sfd, control_packet);
      if (a == 0)
	dump_error(&conn_unit, "Forward arp response write failed");
    }
  }
  return 1;
}

static int
forward_flush_response(Conn_t *conn)
{
  int a;

  Debug_unit(&conn_unit, "Forward_flush_response called");
  dump_conn(conn);
  for (conn = connlist; conn; conn=conn->next) {
    if (conn->sfd) {
      a = send_control_frame(conn->sfd, control_packet);
      if (a == 0)
	dump_error(&conn_unit, "Forward flush response failed");
    }
  } 
  return 1;
}

static int
topology_request(Conn_t *conn)
{
  int a;

  Debug_unit(&conn_unit, "Topology request called");
  dump_conn(conn);

  for(conn=connlist;conn;conn=conn->next) {
    if (conn->sfd) {
      a = send_control_frame(conn->sfd, control_packet);
      if (a == 0)
	dump_error(&conn_unit, "Topology request send failed");
    }
  }
  return 1;
}

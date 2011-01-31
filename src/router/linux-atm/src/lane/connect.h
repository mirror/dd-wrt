/*
 * Connection management
 *
 * $Id: connect.h,v 1.2 2001/10/09 22:33:06 paulsch Exp $
 *
 */
#ifndef LANE_CONN_H
#define LANE_CONN_H

/* System includes needed for types */

/* Local includes needed for types */
#include "units.h"
#include "lane.h"
#include "timers.h"
#include "load.h"
#include "events.h"

/* Type definitions */
typedef enum {
  CT_NONE, CT_MAIN, CT_PVC_CD, CT_PVC_DD, CT_SVC_CD, CT_SVC_DD, CT_BUS_CD
} ConnType_t;

typedef enum {
  CS_IDLE, CS_JOINING, CS_OPERATIONAL
} ConnState_t;

#define CS_MAX CS_OPERATIONAL

/* State vector SV */
typedef struct _Conn_t {
  int fd; /* Where we receive data */
  int sfd; /* Where we send data */
  int active_fd; /* Where select() said thingies are coming. BAD BAD way */
  ConnState_t state;
  LecId_t lecid;
  ConnType_t type;
  Bool_t proxy;
  Timer_t *timer;
  struct _Conn_t *next;
} Conn_t;

/* LECID -DB */
typedef struct _Lecidb_t {
  int fd;
  AtmAddr_t address;
  LecId_t lecid;
  struct _Lecidb_t *next;
} Lecdb_t;

/* PROXY-DB */
typedef struct _Proxy_t {
  LecId_t lecid;
  int fd;
  struct _Proxy_t *next;
} Proxy_t;

/* REG-DB */
typedef struct _Reg_t {
  LaneDestination_t mac_address;
  AtmAddr_t atm_address;
  struct _Reg_t *next;
} Reg_t;
  
/* Connection state machine */
typedef struct {
  EventType_t event;
  unsigned short opcode;
  const char *descript;
  int (*func)(Conn_t *conn);   /* removed const, hessu@cs.tut.fi */
  ConnState_t nextstate;
} State_t;

/* Global function prototypes */
void dump_conn(const Conn_t *connection);
Conn_t *conn_add(ConnType_t type, int fd, LecId_t pvc_lecid);
void conn_remove(const Conn_t *connection);
ConnState_t call_state(EventType_t event, unsigned short opcode, Conn_t *conn);
const char *dump_conn_state(ConnState_t state);
const Conn_t* new_svc(const int fd);
int delete_svc(const int fd, Conn_t *conn);
void conn_set_active(void *data, int fd);

/* Global data */
extern const Unit_t conn_unit;

/* Default values for S2-S5 */
#define S2_default "unspecified"
#define S3_default 1520
#define S4_default 6
#define S5_default 6

#endif


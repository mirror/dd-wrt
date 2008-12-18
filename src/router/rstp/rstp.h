#ifndef _RSTP_H
#define _RSTP_H

/*********************************************************************
  This is an implementation of the Rapid Spanning Tree Protocol
  based on the 802.1D-2004 standard.
*********************************************************************/

struct STP_Bridge_;
typedef struct STP_Bridge_ STP_Bridge;
struct STP_Port_;
typedef struct STP_Port_ STP_Port;

typedef struct STP_macaddr
{
  unsigned char addr[6];
} STP_MacAddress;


/****** Creating and deleting ******/

/* user_ref is a pointer that the STP part uses to call the system part
   Bridge is created as disabled. You need to enable it.
 */
STP_Bridge *STP_IN_bridge_create(void *user_ref);

/* All ports will be deleted, don't reference them */
void STP_IN_bridge_delete(STP_Bridge *);

/* user_ref is a pointer that the STP part uses to call the system part
   Port is created as disabled. You need to enable it.
*/
STP_Port *STP_IN_port_create(STP_Bridge *,
                             unsigned int port_number, void *user_ref);

void STP_IN_port_delete(STP_Port *);

/****** enable/disable ******/

/* When disabled, we can set config without any immediate effect.
   State machine is reinitialized when we enable.
*/
void STP_IN_set_bridge_enable(STP_Bridge *, unsigned int enabled);

/****** Configuration ******/

typedef struct STP_BridgeConfig_
{
  int		 set_bridge_protocol_version;
  unsigned int   bridge_protocol_version;

  /* Setting bridge address dynamically is not part of the standard
     It is convenient for us to interface with Linux bridges where this
     may happen. We reinit the state machine when this is changed.
   */
  unsigned int   set_bridge_address;
  STP_MacAddress bridge_address;

  unsigned int   set_bridge_priority;
  unsigned int   bridge_priority;

  unsigned int   set_bridge_hello_time;
  unsigned int   bridge_hello_time;

  unsigned int   set_bridge_max_age;
  unsigned int   bridge_max_age;

  unsigned int   set_bridge_forward_delay;
  unsigned int   bridge_forward_delay;

  unsigned int   set_bridge_tx_hold_count;
  unsigned int   bridge_tx_hold_count;
} STP_BridgeConfig;

int STP_IN_set_bridge_config(STP_Bridge *, const STP_BridgeConfig *);

/* Alternate forms to set single fields */

/* 0 for STP, 2 for RSTP */
int STP_IN_set_protocol_version(STP_Bridge *, unsigned int version);

/* This reinitializes the bridge if bridge is enabled */
int STP_IN_set_bridge_address(STP_Bridge *, const STP_MacAddress *);

int STP_IN_set_bridge_priority(STP_Bridge *, unsigned int priority);

/* Just for compatibility interfacing. Always fails */
int STP_IN_set_bridge_hello_time(STP_Bridge *, unsigned int hello_time);

int STP_IN_set_bridge_max_age(STP_Bridge *, unsigned int max_age);

int STP_IN_set_bridge_forward_delay(STP_Bridge *, unsigned int fwd_delay);

int STP_IN_set_tx_hold_count(STP_Bridge *, unsigned int count);


typedef struct STP_PortConfig_
{
  unsigned int   set_port_priority;
  unsigned int   port_priority;

  unsigned int   set_port_pathcost;
  unsigned int   port_pathcost;

  unsigned int   set_port_admin_edge;
  unsigned int   port_admin_edge;

  unsigned int   set_port_auto_edge;
  unsigned int   port_auto_edge;

  unsigned int   set_port_admin_p2p;
  unsigned int   port_admin_p2p;
#define STP_ADMIN_P2P_FORCE_FALSE 0
#define STP_ADMIN_P2P_FORCE_TRUE 1
#define STP_ADMIN_P2P_AUTO 2
} STP_PortConfig;

int STP_IN_set_port_config(STP_Port *, const STP_PortConfig *);

/* Alternate forms to set single fields */

int STP_IN_set_port_priority(STP_Port *, unsigned int priority);

int STP_IN_set_port_pathcost(STP_Port *, unsigned int pcost);

/* edge is 0 or 1 - boolean */
int STP_IN_set_port_admin_edge(STP_Port *, unsigned int edge);

int STP_IN_set_port_auto_edge(STP_Port *, unsigned int edge);

/* See STP_PortConfig struct for allowed values of p2p */
int STP_IN_set_port_admin_p2p(STP_Port *, unsigned int p2p);


/* Force migration check */
int STP_IN_port_mcheck(STP_Port *);

/****** Notifications ******/

/* speed and duplex matter only if enabled */
void STP_IN_set_port_enable(STP_Port *, unsigned int enabled,
                            unsigned int speed, unsigned int duplex);

/* No MAC or LLC header included */
void STP_IN_rx_bpdu(STP_Port *, const void *base, unsigned int len);

/* Should be called every second, triggers timer operation. */
void STP_IN_one_second(STP_Bridge *);

/****** Status ******/

typedef struct STP_BridgeStatus_
{
  unsigned char bridge_id[8];
  unsigned int time_since_topology_change;
  unsigned int topology_change_count;
  unsigned int topology_change; /* boolean */
  unsigned char designated_root[8];
  unsigned int root_path_cost;
  /* Not in standard, but part of rootPriority */
  unsigned char designated_bridge[8];
  unsigned int root_port;
  unsigned int max_age;
  unsigned int hello_time;
  unsigned int forward_delay;
  unsigned int bridge_max_age;
  unsigned int bridge_hello_time;
  unsigned int bridge_forward_delay;
  unsigned int tx_hold_count;
  unsigned int protocol_version;
  unsigned int enabled; /* Whether we are running state machines */
} STP_BridgeStatus;

void STP_IN_get_bridge_status(STP_Bridge *, STP_BridgeStatus *);

typedef struct STP_PortStatus_
{
  unsigned int uptime;
  unsigned int state;
#define STP_PORT_STATE_DISCARDING    0
#define STP_PORT_STATE_LEARNING      1
#define STP_PORT_STATE_FORWARDING    2
  unsigned int id;
  unsigned int admin_path_cost;
  unsigned int path_cost;
  unsigned char designated_root[8];
  unsigned int designated_cost;
  unsigned char designated_bridge[8];
  unsigned int designated_port;
  unsigned int tc_ack; /* booleans */
  unsigned int admin_edge_port;
  unsigned int oper_edge_port;
  unsigned int auto_edge_port;
  unsigned int enabled; /* Is this MAC Enabled or MAC operational?
                           It is false both if ifconfig <port> down
                           and if the link status is down.
                        */
  unsigned int admin_p2p;
  unsigned int oper_p2p;
} STP_PortStatus;

void STP_IN_get_port_status(STP_Port *, STP_PortStatus *);

/****** Procedures to be provided by user ******/

/* No MAC or LLC header included */
void STP_OUT_tx_bpdu(void *port_user_ref, void *base, unsigned int len);

#define STP_PORT_STATE_FLAG_LEARNING 1
#define STP_PORT_STATE_FLAG_FORWARDING 2
void STP_OUT_port_set_state(void *user_ref, unsigned int flags);

void STP_OUT_port_fdb_flush(void *user_ref);

#define STP_LOG_LEVEL_ERROR 1
#define STP_LOG_LEVEL_DEBUG 10

void STP_OUT_logmsg(void *br_user_ref, void *port_user_ref,
                    int level, char *fmt, ...);


void *STP_OUT_mem_zalloc(unsigned int size);

void STP_OUT_mem_free(void *);

#endif

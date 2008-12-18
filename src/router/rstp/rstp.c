/*****************************************************************************
  Copyright (c) 2007 EMC Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>

******************************************************************************/

/*********************************************************************
  This is an implementation of the Rapid Spanning Tree Protocol
  based on the 802.1D-2004 standard.
  Section references in the code refer to those parts of the standard.
*********************************************************************/

#include "rstp.h"
#include <string.h>

/* Macros to be used in the later parts */

#ifndef NULL
#define NULL ((void *)0)
#endif

#define FALSE 0
#define TRUE 1


/* memcmp wrapper, validating size equality of _a and _b */
#define CMP(_a, _op, _b)                                           \
(memcmp(&(_a), &(_b),                                              \
  __builtin_choose_expr(sizeof(_a) == sizeof(_b),                  \
                        sizeof(_a),                                \
                        (void)0)                                   \
) _op 0)

#define ZERO(_a) memset(&(_a), 0, sizeof(_a))

/* memcmp wrapper, validating size equality of _a and _b */
#define CPY(_a, _b)                                           \
(memcpy(&(_a), &(_b),                                              \
  __builtin_choose_expr(sizeof(_a) == sizeof(_b),                  \
                        sizeof(_a),                                \
                        (void)0)                                   \
))

#define BRIDGE_ARGS_PROTO        Bridge *BRIDGE
#define PORT_ARGS_PROTO          Bridge *BRIDGE, Port *PORT

#define BRIDGE_ARGS              BRIDGE
#define PORT_ARGS                BRIDGE, PORT
#define FPORT_ARGS               BRIDGE, FPORT

/* Define BRIDGE and PORT as global pointers to a struct type which holds
   a user_ref of value NULL, and no other fields.
   That way log messages can use BRIDGE and PORT always.
*/

struct _dummy_user_ref_struct
{
  void *user_ref;
} _user_ref_var = { .user_ref = NULL},
  *BRIDGE = &_user_ref_var, *PORT = &_user_ref_var;

/* Logging macros */
#define DEBUG(fmt...) \
  STP_OUT_logmsg(BRIDGE->user_ref, PORT->user_ref, STP_LOG_LEVEL_DEBUG, fmt)
#define ERROR(fmt...) \
  STP_OUT_logmsg(BRIDGE->user_ref, PORT->user_ref, STP_LOG_LEVEL_ERROR, fmt)
#define ABORT(fmt...) \
  ({ ERROR(fmt); *(int *)0 = 1; })

/***************** Macros for state machine descriptions ********************/

#define _CAT2(x, y) x ## _ ## y

#define _CAT(x, y) _CAT2(x, y)

#define __STR(x) #x

#define _STR(x) __STR(x)

#define STN(_state) _CAT(STM_NAME, _state)

#define STM_LABEL(_state) _CAT(STM_NAME, label_ ## _state)

#define STM_FUNC(_name) _CAT(_name, func)

/* BEGIN is state 0 in all state machines */
#define STATES(_args...) enum _CAT(STM_NAME, states) { STN(BEGIN), _args }

/* Transition definition */
#define TR(_cond, _new_state) \
({                                                                           \
  if (_cond) {                                                               \
    new_state = STN(_new_state);                                             \
    DEBUG("%s: Transition to state %s\n", _STR(STM_NAME), #_new_state);      \
    goto STM_LABEL(_new_state);                                              \
  }                                                                          \
})

/* Global conditions: Transition conditions that don't depend on current state.
   In our state machine, they get checked after those that are specific to the
   current state
   new_state is the state we last transitioned to, or 0 if no transition
   was made.
*/
#define GC(_transitions...)      \
         global_conditions:      \
             _transitions        \
             return new_state

/* Transitions from BEGIN state */
#define BG(_transitions...)                                              \
         case STN(BEGIN):                                                \
             _transitions                                                \
             ABORT("%s: No where to go from BEGIN\n", _STR(STM_NAME));   \
             return -1

/* State definition */
#define ST(_state, _actions, _transitions) \
         STM_LABEL(_state):                \
             _actions                      \
             if (single_step)              \
                 return new_state;         \
         case STN(_state):                 \
             _transitions                  \
             goto global_conditions

#define STM_FUNC_DECL(_type, _args...) \
  int _type(int state, int single_step, _args)

/* This just barely works. _args expansion includes commas */
//int STM_FUNC(STM_NAME)(int state, int single_step, _args)

#define STM(_args, _contents...)                                 \
static STM_FUNC_DECL(STM_FUNC(STM_NAME), _args)                  \
{                                                                \
  int new_state = 0;                                             \
  switch (state) {                                               \
    _contents                                                    \
      default:                                                   \
    ABORT("%s: Got unknown state %d\n", __func__, state);        \
    return -1;                                                   \
  }                                                              \
}

#define UCT 1

#define STM_RUN(_state, _transitioned, _func, _args...) \
({                                                               \
  int new_state = _func(*_state, 0/*no single step*/, _args);    \
  if (new_state > 0) {                                           \
    *_state = new_state;                                         \
    *_transitioned = TRUE;                                       \
  }                                                              \
})

#define STM_BEGIN(_state, _func, _args...) \
({                                                               \
  *_state = _func(0/*BEGIN*/, 1/*single step*/, _args);          \
})

/************ End of Macros for state machine descriptions ***********/

//Types:

typedef unsigned char uchar;

typedef unsigned int uint;

typedef unsigned short uint16;

typedef uint bool;

typedef struct _BridgeId {
  uchar bridge_identifier_priority[2];
  uchar bridge_address[6];
} __attribute__((packed)) BridgeId;

typedef struct {
  uchar cost[4];
} __attribute__((packed)) PathCost;

static inline uint path_cost_to_uint(PathCost x)
{
  return
    (x.cost[0] << 24) + (x.cost[1] << 16) + (x.cost[2] << 8) + (x.cost[3]);

}


static inline PathCost path_cost_add(PathCost x, uint y)
{
  uint z = y +
    (x.cost[0] << 24) + (x.cost[1] << 16) + (x.cost[2] << 8) + (x.cost[3]);

  PathCost r = {
    .cost = {
      (z >> 24), (z >> 16) & 0xff, (z >> 8) & 0xff, z & 0xff
    }
  };

  return r;
}

typedef struct _PortId {
  uchar port_id[2]; /* 4 high bits of priority and 12 for Id */
} __attribute__((packed)) PortId;

static inline uint port_id_to_uint(PortId p)
{
  return (p.port_id[0] << 8) + p.port_id[1];
}

typedef struct _PriorityVector
{
  BridgeId root_bridge_id;
  PathCost root_path_cost;
  BridgeId designated_bridge_id;
  PortId   designated_port_id;
  PortId   bridge_port_id;
} __attribute__((packed)) PriorityVector;

/* First 4 components of priority vector */
typedef struct _PriorityVector4
{
  BridgeId root_bridge_id;
  PathCost root_path_cost;
  BridgeId designated_bridge_id;
  PortId   designated_port_id;
} __attribute__((packed)) PriorityVector4;


typedef struct _Times
{
  uint forward_delay;
  uint hello_time;
  uint max_age;
  uint message_age;
} Times;

typedef enum _RcvdInfo {
  SuperiorDesignatedInfo,
  RepeatedDesignatedInfo,
  InferiorDesignatedInfo,
  InferiorRootAlternateInfo,
  OtherInfo
} RcvdInfo;


typedef enum _InfoType {
  Mine,
  Aged,
  Received,
  Disabled
} InfoType;

// We accomodate the port role encoding in BPDUs as per 9.2.9
// into this type, defining an extra value of AltBackupPort valid
// only in a BPDU.
typedef enum _Role {
  UnknownPort = 0,
  AltBackupPort = 1,
  RootPort = 2,
  DesignatedPort = 3,
  AlternatePort,
  BackupPort,
  DisabledPort
} Role;

typedef enum _BPDUType {
  BPDUTypeConfig = 0,
  BPDUTypeRST = 2,
  BPDUTypeTCN = 128
} BPDUType;

/* Defining fields directly in struct _Port */
#if 0
typedef struct _ParsedBPDU
{
  uint version;
  BPDUType bpdu_type;

  /* flags */
  bool bpdu_tcFlag;           // bit 1 of flags
  bool bpdu_proposalFlag;     // bit 2 of flags
  Role bpdu_role;             // bits 3 & 4 of flags
  bool bpdu_learningFlag;     // bit 5 of flags
  bool bpdu_forwardingFlag;   // bit 6 of flags
  bool bpdu_agreementFlag;    // bit 7 of flags
  bool bpdu_tcAckFlag;        // bit 8 of flags

  PriorityVector4 bpdu_priority;
  Times bpdu_times;

} ParsedBPDU;
#endif

#define STP_PROTOCOL_ID 0

typedef struct _RawBPDU
{
  uint16 protocol_id;
  uchar version;
  uchar type;
  /* TCN has only upto this */
  uchar TCN_END[0];
  uchar flags;
#define BPDU_FLAG_TC                     0x1
#define BPDU_FLAG_PROPOSAL               0x2
#define BPDU_FLAG_ROLE_MASK              0xc
#define BPDU_FLAG_ROLE(role)             ((role << 2) & BPDU_FLAG_ROLE_MASK)
#define BPDU_FLAG_ROLE_GET(flags)        ((flags & BPDU_FLAG_ROLE_MASK)>>2)
#define BPDU_FLAG_LEARNING               0x10
#define BPDU_FLAG_FORWARDING             0x20
#define BPDU_FLAG_AGREEMENT              0x40
#define BPDU_FLAG_TC_ACK                 0x80

  PriorityVector4 priority;

  uchar message_age[2];
  uchar max_age[2];
  uchar hello_time[2];
  uchar forward_delay[2];
  /* End of Config BPDU */
  uchar CONFIG_END[0];

  uchar version1_len;
  /* End of RST BPDU */
  uchar RST_END[0];

} __attribute__((packed)) RawBPDU;



/* Values for adminPointToPointMAC */
typedef enum _AdminP2P {
  P2PForceFalse = STP_ADMIN_P2P_FORCE_FALSE,
  P2PForceTrue = STP_ADMIN_P2P_FORCE_TRUE,
  P2PAuto = STP_ADMIN_P2P_AUTO
} AdminP2P;


/* Number of per bridge and per port state machines. */
#define NUM_BRIDGE_STATE_MACHINES 1
#define NUM_PORT_STATE_MACHINES  9

/*
  Framework:

  Wherever this is a bridge context, we assume the variable BRIDGE points
  to it. And wherever there is a port context, we assume PORT points to it.

  We define all the fields in the Bridge and Port structures as starting
  with and underscore and have
  #define bridgevar BRIDGE->_bridgevar
  and
  #define portvar PORT->_portvar

  This way we keep the expressions in the state machines fairly close
  to the way they appear in the standard's figures.

*/

typedef STP_Port     Port;
typedef STP_Bridge   Bridge;

/* Per Bridge: */

struct STP_Bridge_
{

  /* Administratively set variables */

  // 17.13.2
  // - Not defining - Never used.
  // This is meant to control FDB, we don't deal with that here.
  /*int _AgeingTime;*/
  /*#define AgeingTime BRIDGE->_AgeingTime*/

  // 17.13.4
  // This is item a) in the list in the introduction in 17.13
  // As per that, we need to reinitialize state machines by asserting BEGIN
  // when changing this
  uint _ForceProtocolVersion;
#define ForceProtocolVersion BRIDGE->_ForceProtocolVersion

  // 17.13.5
  // - Not defining - this is part of BridgeTimes below
  /*int BridgeForwardDelay;*/
#define BridgeForwardDelay BridgeTimes.forward_delay

  // 17.13.6
  // - Not defining - this is part of BridgeTimes below
  /*int BridgeHelloTime;*/
#define BridgeHelloTime BridgeTimes.hello_time

  // 17.13.7 BridgeIdentifierPriority
  // This is item b) in the list in the introduction in 17.13
  // As per that, we need to clear selected and set reselect when changing this
  // Let us do it for all ports since this is a bridge variable.
  // - Not defining this variable, since it is part of BridgeIdentifier below
  /*int BridgeIdentifierPriority;*/
#define BridgeIdentifierPriority BridgeIdentifier.bridge_identifier_priority

  // 17.13.8
  // - Not defining - this is part of BridgeTimes below
  /*int BridgeMaxAge;*/
#define BridgeMaxAge BridgeTimes.max_age

  // 17.13.9
  uint _MigrateTime;
#define MigrateTime BRIDGE->_MigrateTime

  // 17.13.12
  uint _TransmitHoldCount;

#define TransmitHoldCount BRIDGE->_TransmitHoldCount

  /* Other (not administrative) */

  // 17.18.1
  // Condition that initializes all state machines.
  // But machines will spin if it continues to be asserted,
  // So we make it state that we put all machines in instead of asserting this
  /*bool BEGIN;*/

  // 17.18.1
  // The high 16 bits are BridgeIdentifierPriority, noted above (17.13.7)
  // We need to change the rest of BridgeIdentifier if the bridge MAC address
  // changes. Standard doesn't expect this. Let us reinitialize state machines
  // when this happens
  BridgeId _BridgeIdentifier; /* Includes priority */
#define BridgeIdentifier BRIDGE->_BridgeIdentifier

  // 17.18.3, = B:0:B:0:0 - where B is Bridge Identifier
  // - Not defining it .
  // - We will compute it from BridgeIdentifier in updtRolesTree()
  /*PriorityVector _BridgePriority;*/
  /*#define BridgePriority BRIDGE->_BridgePriority*/

  // 17.18.4
  // This holds BridgeForwardDelay, BridgeHelloTime, BridgeMaxAge
  // in .forward_delay, .hello_time, .max_age
  // .message_age is 0
  Times _BridgeTimes;
#define BridgeTimes BRIDGE->_BridgeTimes

  // 17.18.5
  // 5th component of root priority vector
  PortId _rootPortId;
#define rootPortId BRIDGE->_rootPortId

  // 17.18.6
  // first 4 components of root priority vector
  PriorityVector4 _rootPriority;
#define rootPriority BRIDGE->_rootPriority

  // 17.18.7
  /* operational times got from portTimes for root port, or from BridgeTimes */
  Times _rootTimes;
#define rootTimes BRIDGE->_rootTimes



  uint time_since_tc;
  uint tc_count;
  uint tcWhile_count; /* How many port tcWhile's are nonzero */

  /* Our stuff - not from standard */

  Port *port_list;
  void *user_ref;
  bool stp_on;
  int state[NUM_BRIDGE_STATE_MACHINES];
};


/* Per port */

struct STP_Port_
{

  /* Administrative variables: */

  // 17.13.2
  bool _AdminEdgePort;
#define AdminEdgePort PORT->_AdminEdgePort

  // 17.13.3
  bool _AutoEdgePort;
#define AutoEdgePort PORT->_AutoEdgePort

  // 17.13.10
  // This is item c) in the introduction to 17.13
  // As per that, we need to clear selected and set reselect for this port
  // when changing this
  // - Not defining this, it is part of portId below
  /*int PortIdentifierPriority;*/
  // Not defining macro either, it is just 4 high bits in portId.port_id[0]
  /*#define PortIdentifierPriority portId.portIdPriority*/

  // 17.13.11
  // This is item d) in the introduction to 17.13
  // As per that, we need to clear selected and set reselect for this port
  // when changing this
  // PortPathCost defined below ref 17.19.20, is set to this if this is
  // non-zero, else it is auto-set based on link speed.
  uint _AdminPortPathCost;
#define AdminPortPathCost PORT->_AdminPortPathCost

  // 6.4.2
  // MAC Enabled: Controlled by ifconfig <portif> up/down
  // We will sense and act on it
  // We set port enabled in that case

  // 6.4.3
  AdminP2P _adminPointToPointMAC;
#define adminPointToPointMAC PORT->_adminPointToPointMAC

  bool _operPointToPointMAC;
#define operPointToPointMAC PORT->_operPointToPointMAC

  /* Timers: */

  // 17.17.1
  uint _edgeDelayWhile;
#define edgeDelayWhile PORT->_edgeDelayWhile

  // 17.17.2
  uint _fdWhile;
#define fdWhile PORT->_fdWhile

  // 17.17.3
  uint _helloWhen;
#define helloWhen PORT->_helloWhen

  // 17.17.4
  uint _mdelayWhile;
#define mdelayWhile PORT->_mdelayWhile

  // 17.17.5
  uint _rbWhile;
#define rbWhile PORT->_rbWhile

  // 17.17.6
  uint _rcvdInfoWhile;
#define rcvdInfoWhile PORT->_rcvdInfoWhile

  // 17.17.7
  uint _rrWhile;
#define rrWhile PORT->_rrWhile

  // 17.17.8
  uint _tcWhile;
#define tcWhile PORT->_tcWhile

  // 17.19.1
  // - Not defining - Never used
  // This is supposed to control FDB ageing on a per port basis.
  // We don't control FDB ageing here.
  /*int _ageingTime;*/
  /*#define ageingTime PORT->_ageingTime*/

  // 17.19.2
  bool _agree;
#define agree PORT->_agree

  // 17.19.3
  bool _agreed;
#define agreed PORT->_agreed

  // 17.19.4
  // First 4 components of designated priority vector
  // 5th component is portId
  PriorityVector4 _designatedPriority;
#define designatedPriority PORT->_designatedPriority

  // 17.19.5
  Times _designatedTimes;
#define designatedTimes PORT->_designatedTimes

  // 17.19.6
  bool _disputed;
#define disputed PORT->_disputed

  // 17.19.8
 /* Set by topology change state machine to instruct filtering database to
     remove all entries for this Port, immediately if rstpVersion is TRUE
     or by rapid ageing if stpVersion is TRUE. Reset by filtering database once
     entries are removed if rstpVersion is TRUE and immediately if
     stpVersion is TRUE
  */
  bool _fdbFlush;
#define fdbFlush PORT->_fdbFlush

  // 17.19.8
  bool _forward;
#define forward PORT->_forward

  // 17.19.9
  // Setting this will involve and OUT action
  bool _forwarding;
#define forwarding PORT->_forwarding

  // 17.19.10
  // Indicates origin/state of portInfo held for the port
  InfoType _infoIs;
#define infoIs PORT->_infoIs

  // 17.19.11
  bool _learn;
#define learn PORT->_learn

  // 17.19.12
  bool _learning;
#define learning PORT->_learning

  // 17.19.13
  bool _mcheck;
#define mcheck PORT->_mcheck

  // 17.19.14
  // First 4 components of priority vector from received BPDU
  PriorityVector4 _msgPriority;
#define msgPriority PORT->_msgPriority

  // 17.19.15
  // Times from received BPDU
  Times _msgTimes;
#define msgTimes PORT->_msgTimes

  // 17.19.16
  /* Set if a BPDU is to be transmitted,
     reset by Port Transmit state machine */
  bool _newInfo;
#define newInfo PORT->_newInfo

  // 17.19.17
  bool _operEdge;
#define operEdge PORT->_operEdge

  // 17.19.18
  bool _portEnabled;
#define portEnabled PORT->_portEnabled

  // 17.19.19
  PortId _portId;
#define portId PORT->_portId

  // 17.19.20
  uint _PortPathCost;
#define PortPathCost PORT->_PortPathCost

  // 17.19.21
  // First 4 components of port's priority vector
  PriorityVector4 _portPriority;
#define portPriority PORT->_portPriority

  // 17.19.22
  /* Ports timer parameter values */
  Times _portTimes;
#define portTimes PORT->_portTimes

  // 17.19.23
  bool _proposed;
#define proposed PORT->_proposed

  // 17.19.24
  bool _proposing;
#define proposing PORT->_proposing

  // 17.19.25
  /* Set by system when Config, TCN, or RST BPDU is received
     - IN action sets this */
  /* CORR: rcvdBPDU
     But all other references in the doc use rcvdBpdu, so using that. */
  bool _rcvdBpdu;
#define rcvdBpdu PORT->_rcvdBpdu

  // 17.19.26
  /* Result of rcvInfo() procedure */
  RcvdInfo _rcvdInfo;
#define rcvdInfo PORT->_rcvdInfo

  // 17.19.27
  bool _rcvdMsg;
#define rcvdMsg PORT->_rcvdMsg

  // 17.19.28
  bool _rcvdRSTP;
#define rcvdRSTP PORT->_rcvdRSTP

  // 17.19.29
  bool _rcvdSTP;
#define rcvdSTP PORT->_rcvdSTP

  // 17.19.30
  bool _rcvdTc;
#define rcvdTc PORT->_rcvdTc

  // 17.19.31
  bool _rcvdTcAck;
#define rcvdTcAck PORT->_rcvdTcAck

  // 17.19.32
  bool _rcvdTcn;
#define rcvdTcn PORT->_rcvdTcn

  // 17.19.33
  bool _reRoot;
#define reRoot PORT->_reRoot

  // 17.19.34
  bool _reselect;
#define reselect PORT->_reselect

  // 17.19.35
  Role _role;
#define role PORT->_role

  // 17.19.36
  bool _selected;
#define selected PORT->_selected

  // 17.19.37
  Role _selectedRole;
#define selectedRole PORT->_selectedRole

  // 17.19.38
  bool _sendRSTP;
#define sendRSTP PORT->_sendRSTP

  // 17.19.39
  bool _sync;
#define sync PORT->_sync

  // 17.19.40
  bool _synced;
#define synced PORT->_synced

  // 17.19.41
  bool _tcAck;
#define tcAck PORT->_tcAck

  // 17.19.42
  bool _tcProp;
#define tcProp PORT->_tcProp

  // 17.19.43
  bool _tick;
#define tick PORT->_tick

  // 17.19.44
  uint _txCount;
#define txCount PORT->_txCount

  // 17.19.45
  bool _updtInfo;
#define updtInfo PORT->_updtInfo

  // Not in 17.19, but is used. When we receive a BPDU,
  // fill in the received BPDU into this.
  // We just define the fields directly

  uint _bpduVersion;
#define bpduVersion PORT->_bpduVersion
  BPDUType _bpduType;
#define bpduType PORT->_bpduType
  /* flags */
  bool _bpduTcFlag;           // bit 1 of flags
#define bpduTcFlag PORT->_bpduTcFlag
  bool _bpduProposalFlag;     // bit 2 of flags
#define bpduProposalFlag PORT->_bpduProposalFlag
  Role _bpduRole;             // bits 3 & 4 of flags
#define bpduRole PORT->_bpduRole
  bool _bpduLearningFlag;     // bit 5 of flags
#define bpduLearningFlag PORT->_bpduLearningFlag
  bool _bpduForwardingFlag;   // bit 6 of flags
#define bpduForwardingFlag PORT->_bpduForwardingFlag
  bool _bpduAgreementFlag;    // bit 7 of flags
#define bpduAgreementFlag PORT->_bpduAgreementFlag
  bool _bpduTcAckFlag;        // bit 8 of flags
#define bpduTcAckFlag PORT->_bpduTcAckFlag
  PriorityVector4 _bpduPriority;
#define bpduPriority PORT->_bpduPriority
  Times _bpduTimes;
#define bpduTimes PORT->_bpduTimes




  /* Our stuff - not from standard */

  Port *port_next;
  Bridge *bridge;
  void *user_ref;

  uint port_state_flags;
  uint speed;
  bool duplex;
  int state[NUM_PORT_STATE_MACHINES];

};

/* Macros for iterating over all ports */

/* Check whether expr is true for all ports */
/* PORT is not defined by this but is possible defined where this is used.
   FPORT iterates over all ports, so we need to use the FPORT->_field
   (with underscore) for for port variables in the expression here */
#define ForAllPort(expr)                                                     \
({                                                                           \
  Port *FPORT;                                                               \
  bool res = TRUE;                                                           \
  for (FPORT = BRIDGE->port_list; FPORT != NULL; FPORT = FPORT->port_next)   \
    if (!(expr)) {                                                           \
      res = FALSE;                                                           \
      break;                                                                 \
    }                                                                        \
  res;                                                                       \
})

/* Execute statements for all ports */
/* PORT is not defined by this but is possible defined where this is used.
   FPORT iterates over all ports, so we need to use the FPORT->_field
   (with underscore) for for port variables in statements here */
#define ForAllPortDo(statements...)                                          \
({                                                                           \
  Port *FPORT;                                                               \
  for (FPORT = BRIDGE->port_list; FPORT != NULL; FPORT = FPORT->port_next)   \
    { statements }                                                           \
})

// 17.20 ----- conditions and paramaters

// 17.20.1
#define AdminEdge AdminEdgePort

// 17.20.2
#define AutoEdge AutoEdgePort

// 17.20.3
#define allSynced ForAllPort(FPORT->_synced || FPORT->_role == RootPort)

// 17.20.4
#define EdgeDelay (operPointToPointMAC?MigrateTime:MaxAge)

// 17.20.5
#define forwardDelay (sendRSTP?HelloTime:FwdDelay)

// 17.20.6
#define FwdDelay designatedTimes.forward_delay

// 17.20.7
#define HelloTime designatedTimes.hello_time

// 17.20.8
#define MaxAge designatedTimes.max_age

// 17.20.9
// #define MigrateTime MigrateTime

// 17.20.10
#define reRooted ForAllPort(FPORT == PORT || FPORT->_rrWhile == 0)

// 17.20.11
#define rstpVersion (ForceProtocolVersion >= 2)

// 17.20.12
#define stpVersion (ForceProtocolVersion < 2)

// 17.20.13
#define TxHoldCount TransmitHoldCount

// 17.21 ---------- Procedures ------------------------

// 17.21.1
// Definition has an argument newInfoIs, but the uses don't seem to have it.
// So we define it without that, assuming newInfoIs == infoIs
static bool betterorsameInfo(PORT_ARGS_PROTO/*, InfoType newInfoIs*/)
{
  return
    (/*newInfoIs == Received &&*/ infoIs == Received &&
     CMP(msgPriority, <=, portPriority)) ||
    (/*newInfoIs == Mine &&*/ infoIs == Mine &&
     CMP(designatedPriority, <=, portPriority));
}

// 17.21.2
static void clearReselectTree(BRIDGE_ARGS_PROTO)
{
  ForAllPortDo(
               FPORT->_reselect = FALSE;
               );
}

// 17.21.3
static void disableForwarding(PORT_ARGS_PROTO)
{
  /* Cause forwarding process to stop forwarding frames through this port. */
  /* Complete before returning */
  /* OUT */
  PORT->port_state_flags &= ~STP_PORT_STATE_FLAG_FORWARDING;
  STP_OUT_port_set_state(PORT->user_ref, PORT->port_state_flags);
}

// 17.21.4
static void disableLearning(PORT_ARGS_PROTO)
{
  /* Cause port to stop learning process */
  /* Complete before returning */
  /* OUT */
  PORT->port_state_flags &= ~STP_PORT_STATE_FLAG_LEARNING;
  STP_OUT_port_set_state(PORT->user_ref, PORT->port_state_flags);
}

// 17.21.5
static void enableForwarding(PORT_ARGS_PROTO)
{
  /* Cause port to start forwarding. */
  /* Complete before returning */
  /* OUT */
  PORT->port_state_flags |= STP_PORT_STATE_FLAG_FORWARDING;
  STP_OUT_port_set_state(PORT->user_ref, PORT->port_state_flags);
}

// 17.21.6
static void enableLearning(PORT_ARGS_PROTO)
{
  /* Cause port to start learning. */
  /* Complete before returning */
  /* OUT */
  PORT->port_state_flags |= STP_PORT_STATE_FLAG_LEARNING;
  STP_OUT_port_set_state(PORT->user_ref, PORT->port_state_flags);
}

/**** Topology change tracking and reporting ****/

// change is +1 if a tcWhile became non-zero, -1 if a tcWhile became 0.
static inline void update_tcWhile_count(BRIDGE_ARGS_PROTO, int change)
{
  if (change == 0)
    return;

  if (BRIDGE->tcWhile_count == 0) {
    /* Becoming nonzero from zero */
    BRIDGE->tc_count++;
    BRIDGE->time_since_tc = 0;
  }

  BRIDGE->tcWhile_count+= change;
}

#define set_tcWhile(_val) \
({ uint _oldval = tcWhile; tcWhile = _val; \
   update_tcWhile_count(BRIDGE_ARGS, (tcWhile?1:0) - (_oldval?1:0)); })


// 17.21.7
static void newTcWhile(PORT_ARGS_PROTO)
{
  if (tcWhile == 0) {
    if (sendRSTP /* CORR: sendRstp */) {
      // Standard wants tcWhile rounded up
      // We rounded HelloTime up when we received it.
      //tcWhile = HelloTime + 1;
      set_tcWhile(HelloTime + 1);

      newInfo = TRUE;
    }
    else {
      //tcWhile = rootTimes.max_age + rootTimes.forward_delay;
      set_tcWhile(rootTimes.max_age + rootTimes.forward_delay);
    }
  }
}

// 17.21.8
static RcvdInfo rcvInfo(PORT_ARGS_PROTO)
{

  if (bpduType == BPDUTypeTCN) {
    // DIFF:
    // 802.1D-2004 doesn't say explicitly what we should do in this case
    // 802.1w-2001 says return OtherInfo, but just doing that won't ever
    // set rcvdTcn, or cause any effect
    // 802.1Q-2005 (MST, based on 802.1D-2004) says set rcvdTcn here
    // Going by that, we set rcvdTcn and return OtherInfo

    rcvdTcn = TRUE;
    return OtherInfo;
  }

  /* Note: config BPDU conveys Designated Port Role */
  if (bpduType == BPDUTypeConfig)
    bpduRole = DesignatedPort;

  msgPriority = bpduPriority;
  msgTimes = bpduTimes;

  if (bpduRole == DesignatedPort) {
    // a)
    if (CMP(msgPriority, <, portPriority) // 1)
        || (CMP(msgPriority, ==, portPriority) // 2)
            && CMP(msgTimes, !=, portTimes)))
      return SuperiorDesignatedInfo;

    // b)
    if (CMP(msgPriority, ==, portPriority) && CMP(msgTimes, ==, portTimes))
      return RepeatedDesignatedInfo;

    // c)
    if (CMP(msgPriority, >, portPriority))
      return InferiorDesignatedInfo;
  }

  if ((bpduRole == RootPort || bpduRole == AltBackupPort) &&
      CMP(msgPriority, >=, portPriority))
    return InferiorRootAlternateInfo;

  return OtherInfo;

}

// 17.21.9
static void recordAgreement(PORT_ARGS_PROTO)
{
  if (rstpVersion && operPointToPointMAC && bpduAgreementFlag) {
    agreed = TRUE;
    proposing = FALSE;
  }
  else
    agreed = FALSE;
}

// 17.21.10
// No one sets disputed in 802.1D-2004, but see below.
static void recordDispute(PORT_ARGS_PROTO)
{
  if (// bpduType == BPDUTypeRST && -- Implied since only RST has this flag
      bpduLearningFlag) {
    // DIFF: Going by 802.1Q-2005, and the remark at the end of
    // 17.29.3 Designated Port states, about setting disputed,
    // it looks like what we need is

    disputed = TRUE;
    agreed = FALSE;

    // and not, as written in 17.21.10,
    // agreed = TRUE;
    // proposing = FALSE;
  }
}

// 17.21.11
static void recordProposal(PORT_ARGS_PROTO)
{
  if (bpduRole == DesignatedPort && bpduProposalFlag)
    proposed = TRUE;
}

// 17.21.12
static void recordPriority(PORT_ARGS_PROTO)
{
  portPriority = msgPriority;
}

#define MIN_COMPAT_HELLO_TIME  1

// 17.21.13
static void recordTimes(PORT_ARGS_PROTO)
{
  portTimes = msgTimes;
  if (portTimes.hello_time < MIN_COMPAT_HELLO_TIME)
    portTimes.hello_time = MIN_COMPAT_HELLO_TIME;
}

/* Per bridge */
// 17.21.14
static void setSyncTree(BRIDGE_ARGS_PROTO)
{
  ForAllPortDo(
               FPORT->_sync = TRUE;
               );
}

// 17.21.15
static void setReRootTree(BRIDGE_ARGS_PROTO)
{
  ForAllPortDo(
               FPORT->_reRoot = TRUE;
               );

}

// 17.21.16
static void setSelectedTree(BRIDGE_ARGS_PROTO)
{
  if (ForAllPort(FPORT->_reselect == FALSE))
    ForAllPortDo(
                 FPORT->_selected = TRUE;
                 );
}

// 17.21.17
static void setTcFlags(PORT_ARGS_PROTO)
{
  if (bpduType == BPDUTypeTCN)
    rcvdTcn = TRUE;
  else { // Only other types are Config and RST
    //if (bpduType == BPDUTypeConfig || bpduType == BPDUTypeRST) {
    if (bpduTcFlag)
      rcvdTc = TRUE;
    if (bpduTcAckFlag)
      rcvdTcAck = TRUE;
  }
}

// 17.21.18
static void setTcPropTree(PORT_ARGS_PROTO)
{
  ForAllPortDo(
               if (FPORT != PORT) FPORT->_tcProp = TRUE;
               );
}

/***** txConfig(), txRstp() and txTcn() *****/

/* To decide how much of the RawBPDU to actually transmit */
#define offsetof(_TYPE, _MEMBER)  __builtin_offsetof (_TYPE, _MEMBER)
//#define offsetof(_TYPE, _MEMBER) ((int)&((_TYPE *)0)->_MEMBER)

/* To fill the times values in the BPDU in txConfig() and txRstp() */

static void set_bpdu_time(uchar time[2], int t)
{
  time[0] = t;
  time[1] = 0;
}

static void set_bpdu_times(RawBPDU *b, Times *t)
{
  set_bpdu_time(b->message_age, t->message_age);
  set_bpdu_time(b->max_age, t->max_age);
  set_bpdu_time(b->hello_time, t->hello_time);
  set_bpdu_time(b->forward_delay, t->forward_delay);
}

// 17.21.19
static void txConfig(PORT_ARGS_PROTO)
{
  RawBPDU b;

  b.protocol_id = STP_PROTOCOL_ID;
  b.version = 0;
  b.type = BPDUTypeConfig;

  b.priority = designatedPriority;

  int flags = 0;
  flags |= (tcWhile != 0) ? BPDU_FLAG_TC : 0;
  flags |= (tcAck) ? BPDU_FLAG_TC_ACK : 0;
  b.flags = flags;

  set_bpdu_times(&b, &designatedTimes);

  DEBUG("Sending Config BPDU\n");
  STP_OUT_tx_bpdu(PORT->user_ref, &b, offsetof(RawBPDU, CONFIG_END));
}

// 17.21.20
static void txRstp(PORT_ARGS_PROTO)
{
  RawBPDU b;

  b.protocol_id = STP_PROTOCOL_ID;
  b.version = 2;
  b.type = BPDUTypeRST;

  b.priority = designatedPriority;

  int flags = 0;

  int r = role;
  if (r == AlternatePort || r == BackupPort)
    r = AltBackupPort;

  flags |= BPDU_FLAG_ROLE(r);

  flags |= agree ? BPDU_FLAG_AGREEMENT : 0;
  flags |= proposing ? BPDU_FLAG_PROPOSAL : 0;
  flags |= (tcWhile != 0) ? BPDU_FLAG_TC : 0;
  // tcAckFlag = 0;
  flags |= learning ? BPDU_FLAG_LEARNING : 0;
  flags |= forwarding ? BPDU_FLAG_FORWARDING : 0;

  b.flags = flags;

  set_bpdu_times(&b, &designatedTimes);

  b.version1_len = 0;

  DEBUG("Sending RST BPDU\n");
  STP_OUT_tx_bpdu(PORT->user_ref, &b, offsetof(RawBPDU, RST_END));
}

// 17.21.21
static void txTcn(PORT_ARGS_PROTO)
{
  RawBPDU b;

  b.protocol_id = STP_PROTOCOL_ID;
  b.version = 0;
  b.type = BPDUTypeTCN;

  DEBUG("Sending TCN BPDU\n");
  STP_OUT_tx_bpdu(PORT->user_ref, &b, offsetof(RawBPDU, TCN_END));
}

// 17.21.22
static void updtBPDUVersion(PORT_ARGS_PROTO)
{
  if ((bpduVersion == 0 || bpduVersion == 1)
      && (bpduType == BPDUTypeTCN || bpduType == BPDUTypeConfig))
    rcvdSTP = TRUE;
  if (bpduType == BPDUTypeRST)
    rcvdRSTP = TRUE;
}

// 17.21.23
static void updtRcvdInfoWhile(PORT_ARGS_PROTO)
{
  if (portTimes.message_age + 1 <= portTimes.max_age)
    rcvdInfoWhile = 3*portTimes.hello_time;
  else
    rcvdInfoWhile = 0;
}

// 17.21.24
static void updtRoleDisabledTree(BRIDGE_ARGS_PROTO)
{
  ForAllPortDo(
               FPORT->_selectedRole = DisabledPort;
               );
}

// 17.21.25
static void updtRolesTree(BRIDGE_ARGS_PROTO)
{
  // Computes Spanning Tree Priority vectors (17.5, 17.6) and timer values
  /*
    Sets:
      bridge's rootTimes,
      designatedPriority for each port, designatedTimes for each Port
      For each port: selectedRole, possibly updtInfo
  */

  // a), b)
  PriorityVector4 root_priority;
  PortId root_port_id;
  Port *root_port = NULL;

  /* Initialize root_priority to computed BridgePriority = B:0:B:0:0 */
  ZERO(root_priority);
  ZERO(root_port_id);
  root_priority.root_bridge_id = BridgeIdentifier;
  root_priority.designated_bridge_id = BridgeIdentifier;

  ForAllPortDo(Port *PORT = FPORT; // So we can use port var names directly
               if (infoIs == Received &&
                   CMP(portPriority.designated_bridge_id, !=, BridgeIdentifier)
                   ) {
                 // a) - root path priority vector
                 PriorityVector4 root_path_priority = portPriority;
                 root_path_priority.root_path_cost =
                   path_cost_add(root_path_priority.root_path_cost,
                                 PortPathCost);
                 if (CMP(root_path_priority, <, root_priority)
                     || (CMP(root_path_priority, ==, root_priority)
                         && CMP(portId, <, root_port_id))) {
                   root_priority = root_path_priority;
                   root_port_id = portId;
                   root_port = PORT;
                 }
               }
               );
  // Now root_priority is the minimum of BridgePriority and
  // of all the root path priorities where designated bridge id is not our ID
  // b)
  rootPriority = root_priority;
  rootPortId = root_port_id;

  // c)
  if (root_port != NULL) {
    rootTimes = root_port->_portTimes;
    rootTimes.message_age += 1; /* Rounded to whole second */
  }
  else
    rootTimes = BridgeTimes;


  ForAllPortDo(Port *PORT = FPORT;
               // d)
               designatedPriority.root_bridge_id =
                 root_priority.root_bridge_id;
               designatedPriority.root_path_cost =
                 root_priority.root_path_cost;
               designatedPriority.designated_bridge_id = BridgeIdentifier;
               designatedPriority.designated_port_id = portId;

               // e)
               designatedTimes = rootTimes;
               designatedTimes.hello_time = BridgeTimes.hello_time;
               );


  ForAllPortDo(Port *PORT = FPORT;
               switch (infoIs) {
               case Disabled: // f)
                 selectedRole = DisabledPort;
                 break;
               case Aged: // g)
                 updtInfo = TRUE;
                 selectedRole = DesignatedPort;
                 break;
               case Mine: // h)
                 selectedRole = DesignatedPort;
                 if (CMP(designatedPriority, !=, portPriority) ||
                     // Maybe below should be root_port->_portTimes instead
                     // of designatedTimes, going literally by h)?
                     // designatedTimes same as that that except for hello_time
                     CMP(designatedTimes, !=, portTimes))
                   updtInfo = TRUE;
                 break;

               case Received:
                 if (PORT == root_port) { // i)
                   selectedRole = RootPort;
                   updtInfo = FALSE;
                 }
                 // In j), k), l) standard says designated priority is (or
                 // is not) _higher_ than the port priority
                 // Seems like it should mean _better_, numerically
                 // higher is worse. We assume it means better, i.e.
                 // numerically lower
                 else if (CMP(designatedPriority, >=, portPriority)) {
                   if (CMP(portPriority.designated_bridge_id,
                           !=, BridgeIdentifier)) { // j)
                     selectedRole = AlternatePort;
                   }
                   else { // k)
                     /* portPriority.designated_bridge_id is our bridge */
                     selectedRole = BackupPort;
                   }
                   updtInfo = FALSE; // for j) and k)
                 }
                 else { // l)
                   /* Not root port and designatedPriority < portPriority */
                   selectedRole = DesignatedPort;
                   updtInfo = TRUE;
                 }
                 break;
               default:
                 ERROR("Unknown value for infoIs: %d\n", infoIs);
               }
                 DEBUG("Selected Role Set to: %d\n", selectedRole);
               );
}



/* Other procedures we need */

/* Call this whenever we set fdbFlush to TRUE. This will flush the
   fdb data on the specfied port. */
static void do_fdbFlush(PORT_ARGS_PROTO)
{
  if (rstpVersion) {
    STP_OUT_port_fdb_flush(PORT->user_ref);
    fdbFlush = FALSE; /* done */
  }
  else {
    /* We need to reduce ageing time to FwdDelay for time FwdDelay */
    /* Let us just flush them right away.
       Bridge doesn't let us set ageing by port anyway */
    STP_OUT_port_fdb_flush(PORT->user_ref);
    fdbFlush = FALSE;
  }
}

/**************************** STATE MACHINES *******************************/

/*--------------- State machine diagrams ------------------------*/

// 17.22 - Fig. 17-13

#define STM_NAME PortTimers

STATES(STN(ONE_SECOND), STN(TICK));

#define dec(x) if (x) x = x - 1

STM(PORT_ARGS_PROTO,

    GC();

    BG(
       TR(UCT, ONE_SECOND);
       );

    ST(ONE_SECOND,
       tick = FALSE;
       ,
       TR(tick == TRUE, TICK);
       );

    ST(TICK,
       dec(helloWhen);
       //dec(tcWhile);
       if (tcWhile) {
         set_tcWhile(tcWhile - 1);
       }
       dec(fdWhile);
       dec(rcvdInfoWhile);
       dec(rrWhile);
       dec(rbWhile);
       dec(mdelayWhile);
       dec(edgeDelayWhile);
       dec(txCount);
       ,
       TR(UCT, ONE_SECOND);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.23 - Fig. 17-14

#define STM_NAME PortReceive

STATES(STN(DISCARD), STN(RECEIVE));

STM(PORT_ARGS_PROTO,

    GC(
       TR((rcvdBpdu || (edgeDelayWhile != MigrateTime)) && !portEnabled,
          DISCARD);
       );

    BG(
       TR(UCT, DISCARD);
       );

    ST(DISCARD,
       rcvdBpdu = rcvdRSTP = rcvdSTP = FALSE;
       rcvdMsg = FALSE;
       edgeDelayWhile = MigrateTime;
       ,
       TR(rcvdBpdu && portEnabled, RECEIVE);
       );

    ST(RECEIVE,
       updtBPDUVersion(PORT_ARGS);
       operEdge = rcvdBpdu = FALSE;
       rcvdMsg = TRUE;
       edgeDelayWhile = MigrateTime;
       ,
       TR(rcvdBpdu && portEnabled && !rcvdMsg, RECEIVE);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.24 - Fig. 17-15

#define STM_NAME PortProtocolMigration

STATES(STN(CHECKING_RSTP), STN(SELECTING_STP), STN(SENSING));

STM(PORT_ARGS_PROTO,

    GC();

    BG(
       TR(UCT, CHECKING_RSTP);
       );

    ST(CHECKING_RSTP,
       mcheck = FALSE;
       sendRSTP = (rstpVersion);
       mdelayWhile = MigrateTime;
       ,
       TR(mdelayWhile == 0, SENSING);
       TR((mdelayWhile != MigrateTime) && !portEnabled, CHECKING_RSTP);
       );

    ST(SELECTING_STP,
       sendRSTP = FALSE;
       mdelayWhile = MigrateTime;
       ,
       TR(mdelayWhile == 0 || !portEnabled || mcheck, SENSING);
       );

    ST(SENSING,
       rcvdRSTP = rcvdSTP = FALSE;
       ,
       TR(!portEnabled || mcheck || ((rstpVersion) && !sendRSTP && rcvdRSTP),
          CHECKING_RSTP);
       TR(sendRSTP && rcvdSTP, SELECTING_STP);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.25 - Fig. 17-16

#define STM_NAME BridgeDetection

STATES(STN(EDGE), STN(NOT_EDGE));

STM(PORT_ARGS_PROTO,

    GC();

    BG(
       TR(AdminEdge, EDGE);
       TR(!AdminEdge, NOT_EDGE);
       );

    ST(EDGE,
       operEdge = TRUE;
       ,
       TR((!portEnabled && !AdminEdge) || !operEdge, NOT_EDGE);
       );

    ST(NOT_EDGE,
       operEdge = FALSE;
       ,
       TR((!portEnabled && AdminEdge) ||
          ((edgeDelayWhile == 0) && AutoEdge
           && sendRSTP /* CORR: sendRstp */ && proposing),
          EDGE);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.26 - Fig. 17-17

#define xc selected && !updtInfo
#define xc2 newInfo && (txCount < TxHoldCount) && (helloWhen != 0)

#define STM_NAME PortTransmit

STATES(STN(TRANSMIT_INIT), STN(TRANSMIT_PERIODIC), STN(TRANSMIT_CONFIG),
       STN(TRANSMIT_TCN), STN(TRANSMIT_RSTP), STN(IDLE));

STM(PORT_ARGS_PROTO,

    GC();

    BG(
       TR(UCT, TRANSMIT_INIT);
       );

    ST(TRANSMIT_INIT,
       newInfo = TRUE;
       txCount = 0;
       ,
       TR(UCT, IDLE);
       );

    ST(TRANSMIT_PERIODIC,
       newInfo = newInfo || (role == DesignatedPort ||
                             (role == RootPort && (tcWhile != 0)));
       ,
       TR(UCT, IDLE);
       );

    ST(TRANSMIT_CONFIG,
       newInfo = FALSE;
       txConfig(PORT_ARGS);
       txCount += 1;
       tcAck = FALSE;
       ,
       TR(UCT, IDLE);
       );

    ST(TRANSMIT_TCN,
       newInfo = FALSE;
       txTcn(PORT_ARGS);
       txCount += 1;
       ,
       TR(UCT, IDLE);
       );

    ST(TRANSMIT_RSTP,
       newInfo = FALSE;
       txRstp(PORT_ARGS);
       txCount += 1;
       tcAck = FALSE;
       ,
       TR(UCT, IDLE);
       );

    ST(IDLE,
       helloWhen = HelloTime;
       ,
       TR(helloWhen == 0 && xc, TRANSMIT_PERIODIC);
       TR(sendRSTP && xc2 && xc, TRANSMIT_RSTP);
       TR(!sendRSTP && role == RootPort && xc2 && xc, TRANSMIT_TCN);
       TR(!sendRSTP && role == DesignatedPort && xc2 && xc, TRANSMIT_CONFIG);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.27 - Fig. 17-18

#define STM_NAME PortInformation

STATES(STN(DISABLED), STN(AGED), STN(UPDATE),
       STN(SUPERIOR_DESIGNATED), STN(REPEATED_DESIGNATED),
       STN(INFERIOR_DESIGNATED), STN(NOT_DESIGNATED),
       STN(OTHER), STN(CURRENT), STN(RECEIVE)
       );

STM(PORT_ARGS_PROTO,

    GC(
       TR(!portEnabled && (infoIs != Disabled), DISABLED);
       );

    BG(
       TR(UCT, DISABLED);
       );

    ST(DISABLED,
       rcvdMsg = FALSE;
       proposing = proposed = agree = agreed = FALSE;
       rcvdInfoWhile = 0;
       infoIs = Disabled; reselect = TRUE; selected = FALSE;
       ,
       TR(portEnabled, AGED);
       TR(rcvdMsg, DISABLED);
       );

    ST(AGED,
       infoIs = Aged;
       reselect = TRUE; selected = FALSE;
       ,
       TR(selected && updtInfo, UPDATE);
       );

    ST(UPDATE,
       proposing = proposed = FALSE;
       agreed = agreed && betterorsameInfo(PORT_ARGS);
       synced = synced && agreed;
       portPriority = designatedPriority;
       portTimes = designatedTimes;
       updtInfo = FALSE; infoIs = Mine; newInfo = TRUE;
       ,
       TR(UCT, CURRENT);
       );

    ST(SUPERIOR_DESIGNATED,
       agreed = proposing = FALSE;
       recordProposal(PORT_ARGS);
       setTcFlags(PORT_ARGS);
       agreed = agreed && betterorsameInfo(PORT_ARGS);
       recordPriority(PORT_ARGS);
       recordTimes(PORT_ARGS);
       updtRcvdInfoWhile(PORT_ARGS);
       infoIs = Received; reselect = TRUE; selected = FALSE;
       rcvdMsg = FALSE;
       ,
       TR(UCT, CURRENT);
       );

    ST(REPEATED_DESIGNATED,
       recordProposal(PORT_ARGS);
       setTcFlags(PORT_ARGS);
       updtRcvdInfoWhile(PORT_ARGS);
       rcvdMsg = FALSE;
       ,
       TR(UCT, CURRENT);
       );

    ST(INFERIOR_DESIGNATED,
       recordDispute(PORT_ARGS);
       rcvdMsg = FALSE;
       ,
       TR(UCT, CURRENT);
       );

    ST(NOT_DESIGNATED,
       recordAgreement(PORT_ARGS);
       setTcFlags(PORT_ARGS);
       rcvdMsg = FALSE;
       ,
       TR(UCT, CURRENT);
       );

    ST(OTHER,
       rcvdMsg = FALSE;
       ,
       TR(UCT, CURRENT);
       );

    ST(CURRENT,
       ,
       TR(selected && updtInfo, UPDATE);
       TR((infoIs == Received) && (rcvdInfoWhile == 0) &&
          !updtInfo && !rcvdMsg,
          AGED);
       TR(rcvdMsg && !updtInfo, RECEIVE);
       );

    ST(RECEIVE,
       rcvdInfo = rcvInfo(PORT_ARGS);
       ,
       TR(rcvdInfo == SuperiorDesignatedInfo, SUPERIOR_DESIGNATED);
       TR(rcvdInfo == RepeatedDesignatedInfo, REPEATED_DESIGNATED);
       TR(rcvdInfo == InferiorDesignatedInfo, INFERIOR_DESIGNATED);
       TR(rcvdInfo == InferiorRootAlternateInfo, NOT_DESIGNATED);
       TR(rcvdInfo == OtherInfo, OTHER);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.28 - Fig. 17-19

#define STM_NAME PortRoleSelection

STATES(STN(INIT_BRIDGE), STN(ROLE_SELECTION));

STM(BRIDGE_ARGS_PROTO,

    GC();

    BG(
       TR(UCT, INIT_BRIDGE);
       );

    ST(INIT_BRIDGE,
       updtRoleDisabledTree(BRIDGE_ARGS);
       ,
       TR(UCT, ROLE_SELECTION);
       );

    ST(ROLE_SELECTION,
       clearReselectTree(BRIDGE_ARGS);
       updtRolesTree(BRIDGE_ARGS);
       setSelectedTree(BRIDGE_ARGS);
       ,
       TR(!ForAllPort(!FPORT->_reselect), ROLE_SELECTION);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.29
// 17.29.1 - Fig. 17-20  Disabled Port role transitions
// 17.29.2 - Fig. 17-21  Root Port role transitions
// 17.29.3 - Fig. 17-22  Designated Port role transitions
// 17.29.4 - Fig. 17-23  Alternate and Backup Port role transitions

#define STM_NAME PortRoleTransitions

STATES(/* Disabled Port role transitions */
       STN(INIT_PORT), STN(DISABLE_PORT), STN(DISABLED_PORT),
       /* Root Port role transitions */
       STN(ROOT_PROPOSED), STN(ROOT_AGREED), STN(ROOT_SYNCED),
       STN(REROOT), STN(REROOTED),
       STN(ROOT_LEARN), STN(ROOT_FORWARD), STN(ROOT_PORT),
       /* Designated port role transitions */
       STN(DESIGNATED_PROPOSE), STN(DESIGNATED_SYNCED),
       STN(DESIGNATED_RETIRED), STN(DESIGNATED_DISCARD),
       STN(DESIGNATED_LEARN), STN(DESIGNATED_FORWARD), STN(DESIGNATED_PORT),
       /* Alternate and Backup Port role transitions */
       STN(BLOCK_PORT), STN(ALTERNATE_PROPOSED), STN(ALTERNATE_AGREED),
       STN(BACKUP_PORT), STN(ALTERNATE_PORT)
       );

STM(PORT_ARGS_PROTO,

    GC(
       // Let us put role != selectedRole first in the && of conditions
       // Since this will be false in the stable state, so condition check
       // will fail quicker.

       /* Disabled Port role transitions */
       TR((role != selectedRole) && (selectedRole == DisabledPort) && xc,
          DISABLE_PORT);
       /* Root Port role transitions */
       TR((role != selectedRole) && (selectedRole == RootPort) && xc,
          ROOT_PORT);
       /* Designated port role transitions */
       TR((role != selectedRole) && (selectedRole == DesignatedPort) && xc,
          DESIGNATED_PORT);
       /* Alternate and Backup Port role transitions */
       TR((role !=selectedRole) && ((selectedRole == AlternatePort)
                                    || (selectedRole == BackupPort)) && xc,
          BLOCK_PORT);
       );


    /* Disabled Port role transitions */

    BG(
       TR(UCT, INIT_PORT);
       );

    ST(INIT_PORT,
       role = DisabledPort;
       learn = forward = FALSE;
       synced = FALSE;
       sync = reRoot = TRUE;
       rrWhile = FwdDelay;
       fdWhile= MaxAge;
       rbWhile = 0;
       ,
       TR(UCT, DISABLE_PORT);
       );

    ST(DISABLE_PORT,
       role = selectedRole;
       learn = forward = FALSE;
       ,
       TR(!learning && !forwarding && xc, DISABLED_PORT);
       );

    ST(DISABLED_PORT,
       fdWhile = MaxAge;
       synced = TRUE;
       rrWhile = 0;
       sync = reRoot = FALSE;
       ,
       TR((fdWhile != MaxAge || sync || reRoot || !synced) && xc,
          DISABLED_PORT);
       );

    /* Root Port role transitions */

    ST(ROOT_PROPOSED,
       setSyncTree(BRIDGE_ARGS);
       proposed = FALSE;
       ,
       TR(UCT, ROOT_PORT);
       );

    ST(ROOT_AGREED,
       proposed = sync = FALSE;
       agree = TRUE;
       newInfo = TRUE;
       ,
       TR(UCT, ROOT_PORT);
       );

    // This state is there in the 802.1Q-2005 state machine
    ST(ROOT_SYNCED,
       synced = TRUE;
       sync = FALSE;
       ,
       TR(UCT, ROOT_PORT);
       );

    ST(REROOT,
       setReRootTree(BRIDGE_ARGS);
       ,
       TR(UCT, ROOT_PORT);
       );

    ST(REROOTED,
       reRoot = FALSE;
       ,
       TR(UCT, ROOT_PORT);
       );

    ST(ROOT_LEARN,
       fdWhile = forwardDelay;
       learn = TRUE;
       ,
       TR(UCT, ROOT_PORT);
       );

    ST(ROOT_FORWARD,
       fdWhile = 0;
       forward = TRUE;
       ,
       TR(UCT, ROOT_PORT);
       );

    ST(ROOT_PORT,
       role = RootPort;
       rrWhile = FwdDelay;
       ,
       TR(proposed && !agree && xc, ROOT_PROPOSED);
       TR(((allSynced && !agree) || (proposed && agree)) && xc, ROOT_AGREED);
       TR(((agreed && !synced) || (sync && synced)) && xc, ROOT_SYNCED);
       TR(!forward && !reRoot && xc, REROOT);
       TR(reRoot && forward && xc, REROOTED);
       TR((fdWhile == 0 ||
           ((reRooted && (rbWhile == 0)) && (rstpVersion))) && !learn && xc,
          ROOT_LEARN);
       TR((fdWhile == 0 || ((reRooted && (rbWhile == 0)) && (rstpVersion)))
          && learn && !forward && xc,
          ROOT_FORWARD);
       TR(rrWhile != FwdDelay && xc, ROOT_PORT);
       );


    /* Designated port role transitions */

    ST(DESIGNATED_PROPOSE,
       proposing = TRUE;
       edgeDelayWhile = EdgeDelay;
       newInfo = TRUE;
       ,
       TR(UCT, DESIGNATED_PORT);
       );

    ST(DESIGNATED_SYNCED,
       rrWhile = 0;
       synced = TRUE;
       sync = FALSE;
       ,
       TR(UCT, DESIGNATED_PORT);
       );

    ST(DESIGNATED_RETIRED,
       reRoot = FALSE;
       ,
       TR(UCT, DESIGNATED_PORT);
       );

    ST(DESIGNATED_DISCARD,
       learn = forward = disputed = FALSE;
       fdWhile = forwardDelay;
       ,
       TR(UCT, DESIGNATED_PORT);
       );

    ST(DESIGNATED_LEARN,
       learn = TRUE;
       fdWhile = forwardDelay;
       ,
       TR(UCT, DESIGNATED_PORT);
       );

    ST(DESIGNATED_FORWARD,
       forward = TRUE;
       fdWhile = 0;
       agreed = sendRSTP /* CORR: sendRstp */;
       ,
       TR(UCT, DESIGNATED_PORT);
       );

    ST(DESIGNATED_PORT,
       role = DesignatedPort;
       ,
       TR(!forward && !agreed && !proposing && !operEdge && xc,
          DESIGNATED_PROPOSE);
       TR(((!learning && !forwarding && !synced)
           || (agreed && !synced) || (operEdge && !synced)
           || (sync && synced)) && xc,
          DESIGNATED_SYNCED);
       TR(rrWhile == 0 && reRoot && xc, DESIGNATED_RETIRED);
       TR(((sync && !synced) || (reRoot && (rrWhile != 0)) || disputed)
          && !operEdge && (learn || forward) && xc,
          DESIGNATED_DISCARD);
       TR(((fdWhile == 0) || agreed || operEdge)
          && ((rrWhile ==0) || !reRoot) && !sync && !learn && xc,
          DESIGNATED_LEARN);
       TR(((fdWhile == 0) || agreed || operEdge)
          && ((rrWhile ==0) || !reRoot) && !sync && (learn && !forward) && xc,
          DESIGNATED_FORWARD);
       );


    /* Alternate and Backup Port role transitions */

    ST(BLOCK_PORT,
       role = selectedRole;
       learn = forward = FALSE;
       ,
       TR(!learning && !forwarding && xc, ALTERNATE_PORT);
       );

    ST(ALTERNATE_PROPOSED,
       setSyncTree(BRIDGE_ARGS);
       proposed = FALSE;
       ,
       TR(UCT, ALTERNATE_PORT);
       );

    ST(ALTERNATE_AGREED,
       proposed = FALSE;
       agree = TRUE;
       newInfo = TRUE;
       ,
       TR(UCT, ALTERNATE_PORT);
       );

    ST(BACKUP_PORT,
       rbWhile = 2*HelloTime;
       ,
       TR(UCT, ALTERNATE_PORT);
       );

    ST(ALTERNATE_PORT,
       // DIFF: std says FwdDelay, but that leads to infinite loop here
       // when sendRSTP is set, since FwdDelay != forwardDelay then
       // 802.1Q-2005 says forwardDelay here
       fdWhile = forwardDelay;
       synced = TRUE;
       rrWhile = 0;
       sync = reRoot = FALSE;
       ,
       TR(proposed && !agree && xc,
          ALTERNATE_PROPOSED);
       TR(((allSynced && !agree) || (proposed && agree)) && xc,
          ALTERNATE_AGREED);
       TR((rbWhile != 2*HelloTime) && (role == BackupPort) && xc,
          BACKUP_PORT);
       TR(((fdWhile != forwardDelay) || sync || reRoot || !synced) && xc,
          ALTERNATE_PORT);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.30 - Fig. 17-24

#define STM_NAME PortStateTransition

STATES(STN(DISCARDING), STN(LEARNING), STN(FORWARDING));

STM(PORT_ARGS_PROTO,

    GC();

    BG(
       TR(UCT, DISCARDING);
       );

    ST(DISCARDING,
       disableLearning(PORT_ARGS);
       learning = FALSE;
       disableForwarding(PORT_ARGS);
       forwarding = FALSE;
       ,
       TR(learn, LEARNING);
       );

    ST(LEARNING,
       enableLearning(PORT_ARGS);
       learning = TRUE;
       ,
       TR(!learn, DISCARDING);
       TR(forward, FORWARDING);
       );

    ST(FORWARDING,
       enableForwarding(PORT_ARGS);
       forwarding = TRUE;
       ,
       TR(!forward, DISCARDING);
       );

    );

#undef STM_NAME

/*------------------------------------------------------------*/

// 17.31 - Fig. 17-25

#define STM_NAME TopologyChange

STATES(STN(INACTIVE), STN(LEARNING), STN(DETECTED), STN(ACKNOWLEDGED),
       STN(PROPAGATING), STN(NOTIFIED_TC), STN(NOTIFIED_TCN), STN(ACTIVE));

STM(PORT_ARGS_PROTO,

    GC();

    BG(
       TR(UCT, INACTIVE);
       );

    ST(INACTIVE,
       fdbFlush = TRUE;
       do_fdbFlush(PORT_ARGS); /* We add this to trigger the fdb flush */
       //tcWhile = 0;
       set_tcWhile(0);
       tcAck = FALSE;
       ,
       TR(learn && !fdbFlush, LEARNING);
       );

    ST(LEARNING,
       rcvdTc = rcvdTcn = rcvdTcAck = FALSE;
       rcvdTc = tcProp = FALSE;
       ,
       TR(((role == RootPort) || (role == DesignatedPort))
          && forward && !operEdge,
          DETECTED);

       TR((role != RootPort) &&
          (role != DesignatedPort) &&
          !(learn || learning) &&
          !(rcvdTc || rcvdTcn || rcvdTcAck || tcProp),
          INACTIVE);

       TR(rcvdTc ||
          rcvdTcn ||
          rcvdTcAck ||
          tcProp,
          LEARNING);
       );

    ST(DETECTED,
       newTcWhile(PORT_ARGS);
       setTcPropTree(PORT_ARGS);
       newInfo = TRUE;
       ,
       TR(UCT, ACTIVE);
       );

    ST(ACKNOWLEDGED,
       //tcWhile = 0;
       set_tcWhile(0);
       rcvdTcAck = FALSE;
       ,
       TR(UCT, ACTIVE);
       );

    ST(PROPAGATING,
       newTcWhile(PORT_ARGS);
       fdbFlush = TRUE;
       do_fdbFlush(PORT_ARGS); /* We add this to trigger the fdb flush */
       tcProp = FALSE;
       ,
       TR(UCT, ACTIVE);
       );

    ST(NOTIFIED_TC,
       rcvdTcn = rcvdTc = FALSE;
       if (role == DesignatedPort) tcAck = TRUE;
       /*
         Fig 17-25 has setTcPropBridge();
         Must mean setTcPropTree().
         The 802.1w-2001 standard has setTcPropBridge() defined to be the
         same thing as setTcPropTree() in 802.1D-2004, and also has
         setTcPropBridge() in both DETECTED and NOTIFIED_TC states.
         802.1D-2004 has setTcPropTree() in DETECTED state, but
         setTcPropBridge() in NOTIFIED_TC state.
       */
       setTcPropTree(PORT_ARGS); /* CORR: setTcPropBridge() */
       ,
       TR(UCT, ACTIVE);
       );

    ST(NOTIFIED_TCN,
       newTcWhile(PORT_ARGS);
       ,
       TR(UCT, NOTIFIED_TC);
       );

    ST(ACTIVE,
       ,
       TR(((role != RootPort) && (role != DesignatedPort) ) || operEdge,
          LEARNING);
       TR(rcvdTcAck, ACKNOWLEDGED);
       TR(tcProp && !operEdge, PROPAGATING);
       TR(rcvdTc, NOTIFIED_TC);
       TR(rcvdTcn, NOTIFIED_TCN);
       );

    );

#undef STM_NAME


/***** Running the state machines *****/

static STM_FUNC_DECL((*bridge_stms[NUM_BRIDGE_STATE_MACHINES]),
                     BRIDGE_ARGS_PROTO)
     = {
       STM_FUNC(PortRoleSelection),
     };

static STM_FUNC_DECL((*port_stms[NUM_PORT_STATE_MACHINES]), PORT_ARGS_PROTO)
     = {
       STM_FUNC(PortTimers),
       STM_FUNC(PortReceive),
       STM_FUNC(PortProtocolMigration),
       STM_FUNC(BridgeDetection),
       STM_FUNC(PortTransmit),
       STM_FUNC(PortInformation),
       STM_FUNC(PortRoleTransitions),
       STM_FUNC(PortStateTransition),
       STM_FUNC(TopologyChange),
     };

static void state_machines_run(BRIDGE_ARGS_PROTO)
{
  if (!BRIDGE->stp_on)
    return;

  int i;
  bool transitioned;
  do {
    transitioned = FALSE;
    for (i = 0; i < NUM_BRIDGE_STATE_MACHINES; i++)
      STM_RUN(&BRIDGE->state[i], &transitioned, bridge_stms[i], BRIDGE_ARGS);

    ForAllPortDo(
                 for (i = 0; i < NUM_PORT_STATE_MACHINES; i++)
                   STM_RUN(&FPORT->state[i], &transitioned,
                           port_stms[i], FPORT_ARGS);
                 );
  } while (transitioned);
}

static void state_machines_begin(BRIDGE_ARGS_PROTO)
{
  if (!BRIDGE->stp_on)
    return;

  int i;
  for (i = 0; i < NUM_BRIDGE_STATE_MACHINES; i++)
    STM_BEGIN(&BRIDGE->state[i], bridge_stms[i], BRIDGE_ARGS);

  ForAllPortDo(
               for (i = 0; i < NUM_PORT_STATE_MACHINES; i++)
                 STM_BEGIN(&FPORT->state[i], port_stms[i], FPORT_ARGS);
               );

  /* Initialized. Now run them. */
  state_machines_run(BRIDGE_ARGS);
}

void STP_IN_set_bridge_enable(Bridge *BRIDGE, unsigned int enabled)
{
  uint on = enabled?1:0;
  if (BRIDGE->stp_on == on)
    return;

  BRIDGE->stp_on = on;

  if (on)
    state_machines_begin(BRIDGE_ARGS);
  else {

  }
}


/*
  PortPathCost and operPointToPointMAC can change through configuration
  as well as link notifications (when configured to auto).
  Common functions to handle these.
*/

// 17.14 - Table 17-3, also NOTE 3
static uint compute_pcost(int speed)
{
  /* speed is in MB/s*/
  if (speed > 0)
    return speed < 20000000 ? 20000000/speed : 1;
  else
    return 200000000;
}

/* Returns TRUE if PostPathCost changed */
static bool check_port_path_cost(PORT_ARGS_PROTO)
{
  int pcost;
  if (AdminPortPathCost == 0) {
    pcost = compute_pcost(PORT->speed);
  }
  else {
    pcost = AdminPortPathCost;
  }

  if (PortPathCost != pcost) {
    PortPathCost = pcost;
    selected = FALSE; reselect = TRUE; // 17.13
    return TRUE;
  }

  return FALSE;
}

/* Returns TRUE if operPointToPointMAC changed */
static bool check_port_p2p(PORT_ARGS_PROTO)
{
  bool p2p;
  if (adminPointToPointMAC == P2PAuto) {
    p2p = !!PORT->duplex;
  }
  else {
    p2p = (adminPointToPointMAC == P2PForceTrue);
  }

  if (operPointToPointMAC != p2p) {
    operPointToPointMAC = p2p;
    return TRUE;
  }

  return FALSE;
}

/******************** Event notifications ******************************/

void STP_IN_one_second(BRIDGE_ARGS_PROTO)
{
  if (BRIDGE->tcWhile_count == 0)
    BRIDGE->time_since_tc++;

  ForAllPortDo(
               FPORT->_tick = TRUE;
               );
  state_machines_run(BRIDGE_ARGS);
}

void STP_IN_set_port_enable(Port *PORT, unsigned int enabled,
                            unsigned int speed, unsigned int duplex)
{
  Bridge *BRIDGE = PORT->bridge;

  bool changed = FALSE;

  if (enabled) {
    if (!portEnabled) {
      portEnabled = TRUE;
      changed = TRUE;
    }

    if (PORT->speed != speed) {
      PORT->speed = speed;
      if (check_port_path_cost(PORT_ARGS))
        changed = TRUE;
    }

    if (PORT->duplex != duplex) {
      PORT->duplex = duplex;
      if (check_port_p2p(PORT_ARGS))
        changed = TRUE;
    }
  }
  else {
    if (portEnabled) {
      portEnabled = FALSE;
      changed = TRUE;
    }
  }
  if (changed)
    state_machines_run(BRIDGE_ARGS);
}

/* Rounding up. What should we really be doing with fractional times? */
static uint get_bpdu_time(const uchar time[2])
{
  uint t = (time[0] << 8) + time[1];
  return (t + 0xff) >> 8;
}


void STP_IN_rx_bpdu(Port *PORT, const void *base, unsigned int len)
{
  Bridge *BRIDGE = PORT->bridge;
  const RawBPDU *p = base;

  if (!BRIDGE->stp_on)
    return; // No action - else we might fail the next test

  if (rcvdBpdu) {
    ABORT("Port hasn't processed old BPDU\n");
    return;
  }

  // 9.3.4 - Validate
  if (len < 4)
    return;
  if (p->protocol_id != STP_PROTOCOL_ID)
    return;

  DEBUG("Received BPDU of type %d\n", p->type);

  if (// 9.3.4 a)
      (p->type == BPDUTypeConfig &&
       len >= 35 &&
       CMP(p->message_age, <, p->max_age) &&
       !(CMP(p->priority.designated_bridge_id.bridge_address, ==,
             BridgeIdentifier.bridge_address) &&
         (port_id_to_uint(p->priority.designated_port_id) & 0xfff) ==
         (port_id_to_uint(portId) & 0xfff)
         ))
      ||
      // 9.3.4 b)
      (p->type == BPDUTypeTCN)
      ||
      // 9.3.4 c)
      (p->type == BPDUTypeRST && len >= 36)
      ) {
    /* BPDU has been validated */
  }
  else {
    DEBUG("BPDU validation failed\n");
    return;
  }

  // 9.3.4 d)
  bpduVersion = p->version <= 2 ? p->version : 2;

  bpduType = p->type;

  bpduTcFlag =
    (p->type != BPDUTypeTCN) ? ((p->flags & BPDU_FLAG_TC) != 0) : 0;
  bpduProposalFlag =
    (p->type == BPDUTypeRST) ? ((p->flags & BPDU_FLAG_PROPOSAL) != 0) : 0;
  bpduRole =
    (p->type == BPDUTypeRST) ? BPDU_FLAG_ROLE_GET(p->flags) : 0;
  bpduLearningFlag =
    (p->type == BPDUTypeRST) ? ((p->flags & BPDU_FLAG_LEARNING) != 0) : 0;
  bpduForwardingFlag =
    (p->type == BPDUTypeRST) ? ((p->flags & BPDU_FLAG_FORWARDING) != 0) : 0;
  bpduAgreementFlag =
    (p->type == BPDUTypeRST) ? ((p->flags & BPDU_FLAG_AGREEMENT) != 0) : 0;
  bpduTcAckFlag =
    (p->type != BPDUTypeTCN) ? ((p->flags & BPDU_FLAG_TC_ACK) != 0) : 0;

  if (p->type == BPDUTypeTCN) {
    ZERO(bpduPriority);
    ZERO(bpduTimes);

  }
  else {
    bpduPriority = p->priority;

    bpduTimes.message_age = get_bpdu_time(p->message_age);
    bpduTimes.max_age = get_bpdu_time(p->max_age);
    bpduTimes.forward_delay = get_bpdu_time(p->forward_delay);
    bpduTimes.hello_time = get_bpdu_time(p->hello_time);
  }

  rcvdBpdu = TRUE;

  state_machines_run(BRIDGE_ARGS);

}

/************************** Configuration *************************/

// 17.14
static int check_times(BRIDGE_ARGS_PROTO, int max_age, int fwd_delay)
{
  if (2*(fwd_delay - 1) < max_age) {
    ERROR("Configured BridgeTimes doesn't meet "
          "2 * (Bridge Foward Delay - 1) >= Bridge Max Age\n");
    return -1;
  }

  // This condition never fails, with hello_time == 2 && max_age >= 6.
  // So no need to check.
  //if (max_age < 2*(BridgeHelloTime + 1)) {
  //  ERROR("Doesn't meet Bridge Max Age >= 2 * (Bridge Hello Time + 1) ");
  //  return -1;
  //}

  return 0;
}

// 14.8.1.2
int STP_IN_set_bridge_config(Bridge *BRIDGE, const STP_BridgeConfig *cfg)
{
  int r = 0;
  bool changed = FALSE, init = FALSE;
  /* First, validation */

  if (cfg->set_bridge_protocol_version) {
    if (cfg->bridge_protocol_version != 0 && cfg->bridge_protocol_version != 2) {
      ERROR("Protocol version must be 0 or 2\n");
      r = -1;
    }
  }

  /* No validation of bridge_address. It can be any 6 bytes */

  if (cfg->set_bridge_priority) {
    // 17.14 - Table 17-2
    if (cfg->bridge_priority & ~0xf000) {
      ERROR("Bridge Priority restricted to 0-61440 in steps of 4096\n");
      r = -1;
    }
  }

  Times new_times = BridgeTimes;
  bool set_times = FALSE;

  if (cfg->set_bridge_hello_time) {
    // 17.14 - Table 17-1
    if (cfg->bridge_hello_time != BridgeHelloTime) {
      ERROR("Hello Time cannot be changed from 2s in RSTP\n");
      r = -1;
    }
    new_times.hello_time = cfg->bridge_hello_time;
    set_times = TRUE;
  }

  if (cfg->set_bridge_max_age) {
    // 17.14 - Table 17-1
    if (cfg->bridge_max_age < 6 || cfg->bridge_max_age > 40) {
      ERROR("Bridge Max Age must be between 6 and 40\n");
      r = -1;
    }
    new_times.max_age = cfg->bridge_max_age;
    set_times = TRUE;
  }

  if (cfg->set_bridge_forward_delay) {
    // 17.14 - Table 17-1
    if (cfg->bridge_forward_delay < 4 || cfg->bridge_forward_delay > 30) {
      ERROR("Bridge Forward Delay must be between 4 and 30\n");
      r = -1;
    }
    new_times.forward_delay = cfg->bridge_forward_delay;
    set_times = TRUE;
  }

  if (check_times(BRIDGE_ARGS, new_times.max_age, new_times.forward_delay))
    r = -1;

  if (cfg->set_bridge_tx_hold_count) {
    // 17.14 - Table 17-1
    if (cfg->bridge_tx_hold_count < 1 || cfg->bridge_tx_hold_count > 10) {
      ERROR("Transmit Hold Count must be between 1 and 10\n");
      r = -1;
    }
  }

  /* If not valid, return error */
  if (r)
    return r;

  /* Set things */
  if (cfg->set_bridge_protocol_version &&
      ForceProtocolVersion != cfg->bridge_protocol_version) {

    ForceProtocolVersion = cfg->bridge_protocol_version;

    changed = TRUE;
    // 17.13 a)
    init = TRUE;
  }

  if (cfg->set_bridge_address &&
      CMP(BridgeIdentifier.bridge_address, !=, cfg->bridge_address)) {

    CPY(BridgeIdentifier.bridge_address, cfg->bridge_address);

    changed = TRUE;
    init = TRUE;
  }

  if (cfg->set_bridge_priority &&
      ((BridgeIdentifierPriority[0] & 0xf0) << 8) != cfg->bridge_priority) {

    BridgeIdentifierPriority[0] &= ~0xf0;
    BridgeIdentifierPriority[0] |= cfg->bridge_priority >> 8;

    changed = TRUE;
    /* 802.1Q-2005, 12.8.1.3.4 c) (Management, setting bridge protocol params)
       says this should be done for any management change, so we do this below.

    // 17.13 b)
    ForAllPortDo(
                 FPORT->_selected = FALSE;
                 FPORT->_reselect = TRUE;
                 );
    */
  }

  if (set_times &&
      CMP(new_times, !=, BridgeTimes)) {

    BridgeTimes = new_times;

    changed = TRUE;
  }

  if (cfg->set_bridge_tx_hold_count &&
      TxHoldCount != cfg->bridge_tx_hold_count) {

    TxHoldCount = cfg->bridge_tx_hold_count;

    changed = TRUE;
    // 17.13 introduction
    ForAllPortDo(
                 FPORT->_txCount = 0;
                 );
  }

  if (changed && BRIDGE->stp_on) {
    // Do this for any change.
    // Seems like 802.1Q-2005, 12.8.1.3.4 c) wants this.
    /*
       Otherwise we fail UNH rstp.op_D test 3.2 since when administratively
       setting BridgeForwardDelay, etc, the values don't propagate from
       rootTimes to designatedTimes immediately without this change.

     */
    ForAllPortDo(
                 FPORT->_selected = FALSE;
                 FPORT->_reselect = TRUE;
                 );

    if (init)
      state_machines_begin(BRIDGE_ARGS);
    else
      state_machines_run(BRIDGE_ARGS);
  }

  return 0;
}

// 14.8.2.3
int STP_IN_set_port_config(Port *PORT, const STP_PortConfig *cfg)
{
  Bridge *BRIDGE = PORT->bridge;

  int r = 0;
  bool changed = FALSE;
  /* First, validation */

  if (cfg->set_port_priority) {
    // 17.14 - Table 17-2
    if (cfg->port_priority & ~0xf0) {
      ERROR("Port Priority restricted to 0-240 in steps of 16\n");
      r = -1;
    }
  }

  if (cfg->set_port_pathcost) {
    // 17.14 - Note 3 and Table 17-3
    if (cfg->port_pathcost > 200000000) {
      ERROR("Port path cost restricted to 1-200,000,000 or 0 for auto\n");
      r = -1;
    }
  }

  if (cfg->set_port_admin_edge) {
    if (cfg->port_admin_edge != 0 && cfg->port_admin_edge != 1) {
      ERROR("Admin Edge must be 0 or 1\n");
      r = -1;
    }
  }

  if (cfg->set_port_auto_edge) {
    if (cfg->port_auto_edge != 0 && cfg->port_auto_edge != 1) {
      ERROR("Auto Edge must be 0 or 1\n");
      r = -1;
    }
  }

  if (cfg->set_port_admin_p2p) {
    if (cfg->port_admin_p2p < 0 || cfg->port_admin_p2p > 2) {
      ERROR("Admin P2P must be "
            "0 (force false), 1 (force true), or 2 (auto)\n");
      r = -1;
    }
  }

  /* If not valid, return error */
  if (r)
    return r;

  /* Set things */
  if (cfg->set_port_priority &&
      (portId.port_id[0] & 0xf0) != cfg->port_priority) {

    portId.port_id[0] &= ~0xf0;
    portId.port_id[0] |= cfg->port_priority;

    changed = TRUE;
    // 17.13 c)
    selected = FALSE; reselect = TRUE;
  }

  if (cfg->set_port_pathcost &&
      AdminPortPathCost != cfg->port_pathcost) {

    AdminPortPathCost = cfg->port_pathcost;

    if (check_port_path_cost(PORT_ARGS)) {
      /* PortPathCost was changed */
      changed = TRUE;
      // 17.13 d) - Done by check_port_path_cost()
      // selected = FALSE; reselect = TRUE;
    }
  }

  if (cfg->set_port_admin_edge &&
      AdminEdge != cfg->port_admin_edge) {

    AdminEdge = cfg->port_admin_edge;

    changed = TRUE;
  }

  if (cfg->set_port_auto_edge &&
      AutoEdge != cfg->port_auto_edge) {

    AutoEdge = cfg->port_auto_edge;

    changed = TRUE;
  }

  if (cfg->set_port_admin_p2p &&
      adminPointToPointMAC != cfg->port_admin_p2p) {

    adminPointToPointMAC = cfg->port_admin_p2p;

    if (check_port_p2p(PORT_ARGS)) {
      /* operPointToPointMAC was changed */
      changed = TRUE;
    }
  }

  if (changed)
    state_machines_run(BRIDGE_ARGS);

  return 0;
}

/* To make this #define work */
typedef STP_BridgeConfig  STP_bridgeConfig;
typedef STP_PortConfig    STP_portConfig;

#define SET_CFG(_type, _arg, _field, _value) \
({                                                           \
  STP_ ## _type ## Config cfg;                               \
  ZERO(cfg);                                                 \
  cfg._type ## _ ## _field = _value;                         \
  cfg. set_ ## _type ## _ ## _field = 1;                     \
  STP_IN_set_ ## _type ## _config(_arg, &cfg);               \
})

/* Single field Bridge config functions */

int STP_IN_set_protocol_version(Bridge *BRIDGE, unsigned int version)
{
  return SET_CFG(bridge, BRIDGE, protocol_version, version);
}

int STP_IN_set_bridge_address(Bridge *BRIDGE, const STP_MacAddress *addr)
{
  return SET_CFG(bridge, BRIDGE, address, *addr);
}

int STP_IN_set_bridge_priority(Bridge *BRIDGE, unsigned int priority)
{
  return SET_CFG(bridge, BRIDGE, priority, priority);
}

int STP_IN_set_bridge_hello_time(Bridge *BRIDGE, unsigned int hello_time)
{
  return SET_CFG(bridge, BRIDGE, hello_time, hello_time);
}

int STP_IN_set_bridge_max_age(Bridge *BRIDGE, unsigned int max_age)
{
  return SET_CFG(bridge, BRIDGE, max_age, max_age);
}


int STP_IN_set_bridge_forward_delay(Bridge *BRIDGE, unsigned int fwd_delay)
{
  return SET_CFG(bridge, BRIDGE, forward_delay, fwd_delay);
}


int STP_IN_set_tx_hold_count(Bridge *BRIDGE, unsigned int count)
{
  return SET_CFG(bridge, BRIDGE, tx_hold_count, count);
}


/* Single field Port config functions */

int STP_IN_set_port_priority(Port *PORT, unsigned int priority)
{
  return SET_CFG(port, PORT, priority, priority);
}

int STP_IN_set_port_pathcost(Port *PORT, unsigned int pcost)
{
  return SET_CFG(port, PORT, pathcost, pcost);
}

/* edge is 0 or 1 */
int STP_IN_set_port_admin_edge(Port *PORT, unsigned int edge)
{
  return SET_CFG(port, PORT, admin_edge, edge);
}

int STP_IN_set_port_auto_edge(Port *PORT, unsigned int edge)
{
  return SET_CFG(port, PORT, auto_edge, edge);
}

int STP_IN_set_port_admin_p2p(Port *PORT, unsigned int p2p)
{
  return SET_CFG(port, PORT, admin_p2p, p2p);
}

/* Force migration check */
// 14.8.2.4
int STP_IN_port_mcheck(Port *PORT)
{
  Bridge *BRIDGE = PORT->bridge;

  if (!BRIDGE->stp_on) {
    ERROR("Bridge is down\n");
    return -1;
  }

  if (rstpVersion) {
    mcheck = TRUE;

    state_machines_run(BRIDGE_ARGS);
  }

  return 0;
}


/********************* Status ************************/

// 14.8.1.1
void STP_IN_get_bridge_status(Bridge *BRIDGE, STP_BridgeStatus *status)
{
  CPY(status->bridge_id, BridgeIdentifier);

  status->time_since_topology_change = BRIDGE->time_since_tc;
  status->topology_change_count = BRIDGE->tc_count;
  status->topology_change = BRIDGE->tcWhile_count?1:0;

  CPY(status->designated_root, rootPriority.root_bridge_id);

  status->root_path_cost = path_cost_to_uint(rootPriority.root_path_cost);

  CPY(status->designated_bridge, rootPriority.designated_bridge_id);

  status->root_port =
    ((rootPortId.port_id[0] & 0x0f) << 8) + rootPortId.port_id[1];

  status->max_age = rootTimes.max_age;
  status->hello_time = rootTimes.hello_time;
  status->forward_delay = rootTimes.forward_delay;

  status->bridge_max_age = BridgeTimes.max_age;
  status->bridge_hello_time = BridgeTimes.hello_time;
  status->bridge_forward_delay = BridgeTimes.forward_delay;

  status->tx_hold_count = TxHoldCount;

  status->protocol_version = ForceProtocolVersion;

  status->enabled = BRIDGE->stp_on;
}

// 14.8.2.1
void STP_IN_get_port_status(Port *PORT, STP_PortStatus *status)
{
  status->uptime = -1; // XXX: Don't know

  status->state = PORT->port_state_flags;
  if (status->state & STP_PORT_STATE_FLAG_FORWARDING)
    status->state &= ~STP_PORT_STATE_FLAG_LEARNING;

  status->id = port_id_to_uint(portId);

  // admin_path_cost is not in 14.18.2.1 but it is the one piece of config
  // information missing there, so it is useful to have it.
  status->admin_path_cost = AdminPortPathCost;
  status->path_cost = PortPathCost;

  CPY(status->designated_root, designatedPriority.root_bridge_id);
  status->designated_cost =
    path_cost_to_uint(designatedPriority.root_path_cost);
  CPY(status->designated_bridge, designatedPriority.designated_bridge_id);
  status->designated_port =
    port_id_to_uint(designatedPriority.designated_port_id);

  status->tc_ack = tcAck;
  status->admin_edge_port = AdminEdge;
  status->oper_edge_port = operEdge;
  status->auto_edge_port = AutoEdge;

  status->enabled = portEnabled;

  status->admin_p2p = adminPointToPointMAC;
  status->oper_p2p = operPointToPointMAC;
}


/**************** Creating and deleting ********************/

/* user_ref is a pointer that the STP part uses to call the system part
   Bridge is created as disabled. You need to enable it.
 */
Bridge *STP_IN_bridge_create(void *user_ref)
{
  Bridge *b = STP_OUT_mem_zalloc(sizeof(Bridge));

  if (!b) {
    ERROR("Couldn't allocate memory for bridge\n");
    return NULL;
  }

  b->user_ref = user_ref;

  Bridge *BRIDGE = b;

  /* Default configuration values */
  BridgeIdentifierPriority[0] = 0x80; // Default bridge priority = 32768
  MigrateTime = 3;
  BridgeHelloTime = 2;
  BridgeMaxAge = 20;
  BridgeForwardDelay = 15;
  TxHoldCount = 6;
  ForceProtocolVersion = 2;

  return BRIDGE;
}

/* All ports will be deleted, don't reference them */
void STP_IN_bridge_delete(Bridge *BRIDGE)
{
  /* Disable ports - for what it is worth */
  ForAllPortDo(
               FPORT->_portEnabled = FALSE;
               );

  state_machines_run(BRIDGE_ARGS);

  /* Delete all ports */
  while (BRIDGE->port_list)
    STP_IN_port_delete(BRIDGE->port_list);

  STP_OUT_mem_free(BRIDGE);
}

/* user_ref is a pointer that the STP part uses to call the system part
   Port is created as disabled. You need to enable it.
*/
Port *STP_IN_port_create(Bridge *BRIDGE,
                         unsigned int port_number, void *user_ref)
{
  if (port_number == 0 || (port_number & ~0x0fff)) {
    ERROR("Port id should be in the range 1 - 1023\n");
    return NULL;
  }

  Port *p = STP_OUT_mem_zalloc(sizeof(Port));

  if (!p) {
    ERROR("Couldn't allocate memory for port\n");
    return NULL;
  }

  p->user_ref = user_ref;
  p->bridge = BRIDGE;

  p->port_next = BRIDGE->port_list;
  BRIDGE->port_list = p;

  Port *PORT = p;

  portId.port_id[0] = 0x80 | (port_number >> 8); // Default port priority = 128
  portId.port_id[1] = port_number & 0xff;

  // Don't need zero assignments since we zalloc'ed

  //AdminPathCost = 0;

  adminPointToPointMAC = P2PAuto;

  if (BRIDGE->stp_on) {
    /* Try and initialize some things */
    int i;
    for (i = 0; i < NUM_PORT_STATE_MACHINES; i++)
      STM_BEGIN(&PORT->state[i], port_stms[i], PORT_ARGS);

    // selected = FALSE
    reselect = TRUE;
  }

  return PORT;
}

void STP_IN_port_delete(Port *PORT)
{
  Bridge *BRIDGE = PORT->bridge;

  /* Disable */
  if (portEnabled) {
    portEnabled = FALSE;
    state_machines_run(BRIDGE_ARGS);
  }

  /* Find position in list */
  Port **prev = &BRIDGE->port_list;
  while (*prev != PORT && *prev != NULL)
    prev = &(*prev)->port_next;

  if (*prev == NULL)
    ABORT("port not in its bridge's list\n");

  /* Remove from list */
  *prev = PORT->port_next;

  STP_OUT_mem_free(PORT);
}


/* proto.h - Common protocol functions and structures */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <linux/atmsvc.h>

#include "atmsap.h"
#include "atmd.h"
#include "saal.h"

#ifdef MULTIPOINT
typedef enum { p2p, p2mp } CALL_TYPE;
#endif

typedef enum { /* formatting aligned with state_map and others */
	ss_invalid,	ss_null,	ss_listening,	ss_connecting,
	ss_connected,	ss_indicated,	ss_accepting,	ss_zombie,
	ss_wait_rel,	ss_wait_close,	ss_rel_req,	ss_rel_ind,
	ss_proceeding,	ss_listen_zombie,
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
	ss_mod_lcl,	ss_mod_req,	ss_mod_rcv,	ss_mod_fin_ack,
	ss_mod_fin_ok,	ss_mod_fin_fail
#endif
} STATE;

typedef enum { cs_null,cs_call_init,cs_out_proc = 3,cs_conn_req = 8,
  cs_in_proc,cs_active,cs_rel_req,cs_rel_ind,
#if defined(Q2963_1) || defined (DYNAMIC_UNI)
  cs_mod_req,cs_mod_rcv
#endif
  } CALL_STATE;

typedef enum { ps_null,ps_add_init,ps_add_recv = 6,ps_drop_init = 11,
  ps_drop_recv,ps_active = 10 } PARTY_STATE;

#define S_UNI30		1	/* UNI 3.0 or ALLOW_UNI30 */
#define S_UNI31		2	/* UNI 3.1 or ALLOW_UNI30 */
#define S_UNI40		4	/* UNI 4.0 */
#define S_Q2963_1	8	/* UNI 4.0 plus Q.2963.1 */

typedef enum { sm_unknown,sm_user,sm_net,sm_switch } SIGNALING_MODE;

typedef struct {
    enum { ls_unused,ls_added,ls_removed,ls_same } state;
    struct sockaddr_atmsvc addr;
} LOCAL_ADDR;

#define MAX_LOCAL_ADDRS 32

typedef struct _vpci {
    int vpci;
    int itf;
//    struct _sig_entity *sig; /* back pointer */
    LOCAL_ADDR local_addr[MAX_LOCAL_ADDRS+1];
    struct _vpci *next;
} VPCI;

typedef struct _sig_entity {
    int signaling; /* fd */
    int uni;
    SIGNALING_MODE mode;
    int sig_pcr; /* @@@ remove soon */
    const char *sig_qos;
    int max_rate;
    struct sockaddr_atmpvc signaling_pvc;
    SAAL_DSC saal;
    VPCI *vpcis;
    struct _sig_entity *next;
} SIG_ENTITY;

typedef struct _socket {
    STATE state;
    SIG_ENTITY *sig;
    struct sockaddr_atmpvc pvc;
    /* --- socket layer information ---------------------------------------- */
    atm_kptr_t id;
    struct sockaddr_atmsvc local; /* local address */
    struct sockaddr_atmsvc remote; /* remote address */
#ifdef MULTIPOINT
    CALL_TYPE ct;
#endif
    struct atm_sap sap; /* SAP (BHLI and BLLI) */
    struct atm_qos qos; /* QOS parameters */
#if defined(Q2963_1) || defined(DYNAMIC_UNI)
    struct atm_qos new_qos; /* during modification */
    int owner;		/* non-zero if connection owner */
#endif
    int error; /* error code for close */
    /* --- UNI information ------------------------------------------------- */
    CALL_STATE call_state;
    unsigned long call_ref; /* bit 24 like when sending */
    int ep_ref; /* endpoint reference value when sending ; -1 for p2p */
    TIMER *conn_timer; /* current connection timer */
    /* --- some meta-information ------------------------------------------- */
    struct _socket *listen; /* to pending connections, also used for "more" */
    struct _socket *next; /* next socket */
} SOCKET;

extern SOCKET *sockets;

/*
 * SOCKET uses a horrible linked list structure. Lists should be at least
 * doubly-linked and there should be a few hashes (by id and by call_ref) for
 * reasonable fast lookup. All this will have to wait till that version is
 * stable enough to be useful to test the "real" thing against it.
 */

extern atm_kptr_t kptr_null;

#define S_PVC(e) \
  (e)->signaling_pvc.sap_addr.itf, \
  (e)->signaling_pvc.sap_addr.vpi, \
  (e)->signaling_pvc.sap_addr.vci

extern SIG_ENTITY *entities;
extern SIG_ENTITY _entity;

extern const CALL_STATE state_map[];
extern const PARTY_STATE eps_map[];
extern const char *state_name[],*cs_name[],*as_name[];

extern unsigned char q_buffer[];

#define DEFAULT_TRACE_SIZE 20
#define DEFAULT_DUMP_DIR "/var/tmp"

extern int pretty;
extern const char *dump_dir;

extern int stop;


#define SEND_ERROR(vcc,code) \
  send_kernel(vcc,kptr_null,as_error,code,NULL,NULL,NULL,NULL,NULL)


void poll_signals(void);

void from_kernel(struct atmsvc_msg *msg,int size);
void sync_addr(VPCI *vpci);

void to_uni(SIG_ENTITY *sig,void *msg,int size);
void send_kernel(atm_kptr_t vcc,atm_kptr_t listen_vcc,
  enum atmsvc_msg_type type,int reply,const struct sockaddr_atmpvc *pvc,
  const struct sockaddr_atmsvc *svc,const struct sockaddr_atmsvc *local,
  const struct atm_sap *sap,const struct atm_qos *qos);
void from_net(SIG_ENTITY *sig,void *msg,int size);
void to_signaling(SIG_ENTITY *sig,void *msg,int size);
void saal_failure(SIG_ENTITY *sig);
void saal_okay(SIG_ENTITY *sig);
void clear_all_calls(SIG_ENTITY *sig);
void clear_all_calls_on_T309(SIG_ENTITY *sig);

SOCKET *new_sock(atm_kptr_t id);
void free_sock(SOCKET *sock);
void free_leaves(atm_kptr_t *id);
int count_leaves(atm_kptr_t *id);
void new_state(SOCKET *sock,STATE state);
SOCKET *lookup_sap(const struct sockaddr_atmsvc *addr,
  const struct atm_sap *sap,const struct atm_qos *qos,
  struct sockaddr_atmsvc *res_addr,struct atm_sap *res_sap,
  struct atm_qos *res_qos,int exact_match);

void send_release(SOCKET *sock,unsigned char reason,...);
void send_release_complete(SIG_ENTITY *sig,unsigned long call_ref,
  unsigned char cause,...);
int send_call_proceeding(SOCKET *sock);
void send_modify_reject(SOCKET *sock,unsigned char reason);

const char *mid2name(unsigned char mid);

void set_error(SOCKET *sock,int code);
void send_close(SOCKET *sock);

int get_vci(int itf);

void enter_vpci(SIG_ENTITY *sig,int vpci,int itf);
void set_vpi_0(SIG_ENTITY *sig);
int get_itf(SIG_ENTITY *sig,int *vpci);
void init_addr(SIG_ENTITY *sig);
void itf_reload(int itf);
struct sockaddr_atmsvc *get_local(SIG_ENTITY *sig);

void add_route(SIG_ENTITY *sig,struct sockaddr_atmsvc *addr,int len);
SIG_ENTITY *route_remote(struct sockaddr_atmsvc *addr);
SIG_ENTITY *route_local(struct sockaddr_atmsvc *addr);

int get_max_rate(SIG_ENTITY *sig);

#endif

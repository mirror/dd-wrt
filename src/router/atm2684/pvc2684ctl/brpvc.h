/* brpvcd.h -- header file for brpvc.c and brpvcd.c

   written by Chuck Musser <chuckie@well.com>
*/

#define BRPVC_SOCKPATH  "/var/br2684"
#define MAX_GROUPNAME_LEN 32
#define  LIST_END          0
#define  FOUND             1

enum messages { HELLO, 
		ADD, DELETE, DELETE_GROUP, 
		LIST_GROUP, LIST_ALL,
		MEM_STATS,
                OK, VC_NOT_FOUND, GROUP_NOT_FOUND, END_OF_LIST, 
		NOT_OWNER, UNKNOWN_CMD, NOMEM, SOCK_FAILED, INTERFACE_FAILED};


/* internal representation for a bridge-encapsulation circuit  */

struct be_vc {
  int                    nas_idx;  /* interface number          */
  struct sockaddr_atmpvc pvc;      /* PVC descriptor            */
  int                    sock;     /* file descriptor for VC    */
  uid_t                  uid;      /* VC owner name             */
  struct be_vc           *next;    /* next VC in list           */
  // brcm
  int			proto_filter;	/* protocol filter flag, current only PPPOE */
  int 			encap;		/* encapsulation: LLC/VC */
  int			mode;		/* Bridging/Routing */
  unsigned short        vlan_id;        /* vlan id (0-4096) */
};

/* br2684_circuits are grouped by a textual name, which can be blank  */

struct be_group {
  char            name[MAX_GROUPNAME_LEN];  /* name of group        */
  struct be_vc    *head;                    /* head of the VC list  */
  struct be_group *next;                    /* next group in list   */
};

/* message format for talking to the daemon */

struct be_msg {
  enum messages          msgtype;   /* message (from enum above) */
  int                    nas_idx;   /* interface number          */
  struct sockaddr_atmpvc pvc;       /* PVC descriptor            */
  uid_t                  uid;       /* VC owner name (server->client only)  */
  char                   name[MAX_GROUPNAME_LEN]; /* name of group          */
  // brcm
  int			proto_filter;	/* protocol filter flag, current only PPPOE */
  int 			encap;		/* encapsulation: LLC/VC */
  int			mode;		/* Bridging/Routing */
  int			extif;	/* ext interface */
  unsigned short        vlan_id;        /* vlan id (0-4096) */
};

/* Special message for dumping memory usage statistics */

struct be_memstat {
  int                    vc_mallocs; 
  int                    vc_frees;  
  int                    group_mallocs;
  int                    group_frees;  
};

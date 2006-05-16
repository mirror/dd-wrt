/* -*- C -*-
     SNMP.xs -- Perl 5 interface to the UCD SNMP toolkit

     written by G. S. Marzot (gmarzot@nortelnetworks.com)

     Copyright (c) 1995-1999 G. S. Marzot. All rights reserved.
     This program is free software; you can redistribute it and/or
     modify it under the same terms as Perl itself.
*/
#define WIN32SCK_IS_STDSCK
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#ifndef MSVC_PERL
	#include <signal.h>
#endif
#include <stdio.h>
#include <ctype.h>
#ifdef I_SYS_TIME
#include <sys/time.h>
#endif
#include <netdb.h>
#include <stdlib.h>
#ifndef MSVC_PERL
	#include <unistd.h>
#endif
/* XXX This is a problem if regex.h is not on the system. */
#include <regex.h>

#ifndef __P
#define __P(x) x
#endif

#ifndef na
#define na PL_na
#endif

#ifndef sv_undef
#define sv_undef PL_sv_undef
#endif

#ifndef stack_base
#define stack_base PL_stack_base
#endif

#ifndef G_VOID
#define G_VOID G_DISCARD
#endif

#ifdef WIN32
#define SOCK_STARTUP winsock_startup()
#define SOCK_CLEANUP winsock_cleanup()
#define DLL_IMPORT   __declspec( dllimport )
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#define SOCK_STARTUP
#define SOCK_CLEANUP
#define DLL_IMPORT
#endif

DLL_IMPORT extern struct tree *Mib;
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "perlsnmp.h"

#define SUCCESS 1
#define FAILURE 0

#define ZERO_BUT_TRUE "0 but true"

#define VARBIND_TAG_F 0
#define VARBIND_IID_F 1
#define VARBIND_VAL_F 2
#define VARBIND_TYPE_F 3

#define TYPE_UNKNOWN 0
#define MAX_TYPE_NAME_LEN 16
#define STR_BUF_SIZE 1024
#define ENG_ID_BUF_SIZE 32

#define SYS_UPTIME_OID_LEN 9
#define SNMP_TRAP_OID_LEN 11
#define NO_RETRY_NOSUCH 0
static oid sysUpTime[SYS_UPTIME_OID_LEN] = {1, 3, 6, 1, 2, 1, 1, 3, 0};
static oid snmpTrapOID[SNMP_TRAP_OID_LEN] = {1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0};

/* Internal flag to determine snmp_main_loop() should return after callback */
static int mainloop_finish = 0;

/* these should be part of transform_oids.h ? */
#define USM_AUTH_PROTO_MD5_LEN 10
#define USM_AUTH_PROTO_SHA_LEN 10
#define USM_PRIV_PROTO_DES_LEN 10

/* why does ucd-snmp redefine sockaddr_in ??? */
#define SIN_ADDR(snmp_addr) (((struct sockaddr_in *) &(snmp_addr))->sin_addr)

typedef netsnmp_session SnmpSession;
typedef struct tree SnmpMibNode;
typedef struct snmp_xs_cb_data {
    SV* perl_cb;
    SV* sess_ref;
} snmp_xs_cb_data;

static void __recalc_timeout _((struct timeval*,struct timeval*,
                                struct timeval*,struct timeval*, int* ));
static in_addr_t __parse_address _((char*));
static int __is_numeric_oid _((char*));
static int __is_leaf _((struct tree*));
static int __translate_appl_type _((char*));
static int __translate_asn_type _((int));
static int __snprint_value _((char *, size_t,
                              netsnmp_variable_list*, struct tree *,
                             int, int));
static int __sprint_num_objid _((char *, oid *, int));
static int __scan_num_objid _((char *, oid *, int *));
static int __get_type_str _((int, char *));
static int __get_label_iid _((char *, char **, char **, int));
static int __oid_cmp _((oid *, int, oid *, int));
static int __tp_sprint_num_objid _((char*,SnmpMibNode *));
static SnmpMibNode * __get_next_mib_node _((SnmpMibNode *));
static struct tree * __oid2tp _((oid*, int, struct tree *, int*));
static struct tree * __tag2oid _((char *, char *, oid  *, int  *, int *, int));
static int __concat_oid_str _((oid *, int *, char *));
static int __add_var_val_str _((netsnmp_pdu *, oid *, int, char *,
                                 int, int));
static int __send_sync_pdu _((netsnmp_session *, netsnmp_pdu *,
                              netsnmp_pdu **, int , SV *, SV *, SV *));
static int __snmp_xs_cb __P((int, netsnmp_session *, int,
                             netsnmp_pdu *, void *));
static int __callback_wrapper __P((int, netsnmp_session *, int,
	                             netsnmp_pdu *, void *));
static SV* __push_cb_args2 _((SV * sv, SV * esv, SV * tsv));
#define __push_cb_args(a,b) __push_cb_args2(a,b,NULL)
static int __call_callback _((SV * sv, int flags));
static char* __av_elem_pv _((AV * av, I32 key, char *dflt));
static u_int compute_match _((const char *, const char *));

#define USE_NUMERIC_OIDS 0x08
#define NON_LEAF_NAME 0x04
#define USE_LONG_NAMES 0x02
#define FAIL_ON_NULL_IID 0x01
#define NO_FLAGS 0x00

/* Structures used by snmp_bulkwalk method to track requested OID's/subtrees. */
typedef struct bulktbl {
   oid	req_oid[MAX_OID_LEN];	/* The OID originally requested.    */
   oid	last_oid[MAX_OID_LEN];	/* Last-seen OID under this branch. */
   AV	*vars;			/* Array of Varbinds for this OID.  */
   char	req_len;		/* Length of requested OID.         */
   char	last_len;		/* Length of last-seen OID.         */
   char norepeat;		/* Is this a non-repeater OID?      */
   char	complete;		/* Non-zero if this tree complete.  */
   char	ignore;			/* Ignore this OID, not requested.  */
} bulktbl;

/* Context for bulkwalk() sessions.  Used to store state across callbacks. */
typedef struct walk_context {
   SV		*sess_ref;	/* Reference to Perl SNMP session object.   */
   SV		*perl_cb;	/* Pointer to Perl callback func or array.  */
   bulktbl	*req_oids;	/* Pointer to bulktbl[] for requested OIDs. */
   bulktbl	*repbase;	/* Pointer to first repeater in req_oids[]. */
   bulktbl	*reqbase;	/* Pointer to start of requests req_oids[]. */
   int	  	nreq_oids;	/* Number of valid bulktbls in req_oids[].  */
   int	  	req_remain;	/* Number of outstanding requests remaining */
   int		non_reps;	/* Number of nonrepeater vars in req_oids[] */
   int		repeaters;	/* Number of repeater vars in req_oids[].   */
   int		max_reps;	/* Maximum repetitions of variable per PDU. */
   int		exp_reqid;	/* Expect a response to this request only.  */
   int		getlabel_f;	/* Flag long/numeric names for get_label(). */
   int		sprintval_f;	/* Flag enum/sprint values for sprintval(). */
   int		pkts_exch;	/* Number of packet exchanges with agent.   */
   int		oid_total;	/* Total number of OIDs received this walk. */
   int		oid_saved;	/* Total number of OIDs saved as results.   */
} walk_context;

/* Prototypes for bulkwalk support functions. */
static netsnmp_pdu *_bulkwalk_send_pdu _((walk_context *context));
static int _bulkwalk_done     _((walk_context *context));
static int _bulkwalk_recv_pdu _((walk_context *context, netsnmp_pdu *pdu));
static int _bulkwalk_finish   _((walk_context *context, int okay));
static int _bulkwalk_async_cb _((int op, SnmpSession *ss, int reqid,
				     netsnmp_pdu *pdu, void *context_ptr));

/* Structure to hold valid context sessions. */
struct valid_contexts {
   walk_context	**valid;	/* Array of valid walk_context pointers.    */
   int		sz_valid;	/* Maximum size of valid contexts array.    */
   int		num_valid;	/* Count of valid contexts in the array.    */
};
static struct valid_contexts  *_valid_contexts = NULL;
static int _context_add       _((walk_context *context));
static int _context_del       _((walk_context *context));
static int _context_okay      _((walk_context *context));

/* Wrapper around fprintf(stderr, ...) for clean and easy debug output. */
#ifdef	DEBUGGING
static int _debug_level = 0;
#define DBOUT PerlIO_stderr(),
#define	DBPRT(severity, otherargs)					\
	do {								\
	    if (_debug_level && severity <= _debug_level) {		\
		(void)PerlIO_fprintf(PerlIO_stderr(), otherargs);		\
	    }								\
	} while (/*CONSTCOND*/0)

char	_debugx[1024];	/* Space to sprintf() into - used by sprint_objid(). */

#else	/* DEBUGGING */
#define DBOUT
#define	DBPRT(severity, otherargs)	/* Ignore */

#endif	/* DEBUGGING */

void
__libraries_init(char *appname)
    {
        static int have_inited = 0;

        if (have_inited)
            return;
        have_inited = 1;

        snmp_set_quick_print(1);
        init_snmp(appname);
    
        netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS, 1);
        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_SUFFIX_ONLY, 1);

        SOCK_STARTUP;
    
    }

static void
__recalc_timeout (tvp, ctvp, ltvp, itvp, block)
struct timeval* tvp;
struct timeval* ctvp;
struct timeval* ltvp;
struct timeval* itvp;
int *block;
{
   struct timeval now;

   if (!timerisset(itvp)) return;  /* interval zero means loop forever */
   *block = 0;
   gettimeofday(&now,(struct timezone *)0);

   if (ctvp->tv_sec < 0) { /* first time or callback just fired */
      timersub(&now,ltvp,ctvp);
      timersub(ctvp,itvp,ctvp);
      timersub(itvp,ctvp,ctvp);
      timeradd(ltvp,itvp,ltvp);
   } else {
      timersub(&now,ltvp,ctvp);
      timersub(itvp,ctvp,ctvp);
   }

   /* flag is set for callback but still hasnt fired so set to something
    * small and we will service packets first if there are any ready
    * (also guard against negative timeout - should never happen?)
    */
   if (!timerisset(ctvp) || ctvp->tv_sec < 0 || ctvp->tv_usec < 0) {
      ctvp->tv_sec = 0;
      ctvp->tv_usec = 10;
   }

   /* if snmp timeout > callback timeout or no more requests to process */
   if (timercmp(tvp, ctvp, >) || !timerisset(tvp)) {
      *tvp = *ctvp; /* use the smaller non-zero timeout */
      timerclear(ctvp); /* used as a flag to let callback fire on timeout */
   }
}

static in_addr_t
__parse_address(address)
char *address;
{
    in_addr_t addr;
    struct sockaddr_in saddr;
    struct hostent *hp;

    if ((addr = inet_addr(address)) != -1)
	return addr;
    hp = gethostbyname(address);
    if (hp == NULL){
        return (-1); /* error value */
    } else {
	memcpy(&saddr.sin_addr, hp->h_addr, hp->h_length);
	return saddr.sin_addr.s_addr;
    }

}

static int
__is_numeric_oid (oidstr)
char* oidstr;
{
  if (!oidstr) return 0;
  for (; *oidstr; oidstr++) {
     if (isalpha((int)*oidstr)) return 0;
  }
  return(1);
}

static int
__is_leaf (tp)
struct tree* tp;
{
   char buf[MAX_TYPE_NAME_LEN];
   return (tp && __get_type_str(tp->type,buf));
}

static SnmpMibNode*
__get_next_mib_node (tp)
SnmpMibNode* tp;
{
   /* printf("tp = %lX, parent = %lX, peer = %lX, child = %lX\n",
              tp, tp->parent, tp->next_peer, tp->child_list); */
   if (tp->child_list) return(tp->child_list);
   if (tp->next_peer) return(tp->next_peer);
   if (!tp->parent) return(NULL);
   for (tp = tp->parent; !tp->next_peer; tp = tp->parent) {
      if (!tp->parent) return(NULL);
   }
   return(tp->next_peer);
}

static int
__translate_appl_type(typestr)
char* typestr;
{
	if (typestr == NULL || *typestr == '\0') return TYPE_UNKNOWN;

	if (!strncasecmp(typestr,"INTEGER32",8))
            return(TYPE_INTEGER32);
	if (!strncasecmp(typestr,"INTEGER",3))
            return(TYPE_INTEGER);
	if (!strncasecmp(typestr,"UNSIGNED32",3))
            return(TYPE_UNSIGNED32);
	if (!strcasecmp(typestr,"COUNTER")) /* check all in case counter64 */
            return(TYPE_COUNTER);
	if (!strncasecmp(typestr,"GAUGE",3))
            return(TYPE_GAUGE);
	if (!strncasecmp(typestr,"IPADDR",3))
            return(TYPE_IPADDR);
	if (!strncasecmp(typestr,"OCTETSTR",3))
            return(TYPE_OCTETSTR);
	if (!strncasecmp(typestr,"TICKS",3))
            return(TYPE_TIMETICKS);
	if (!strncasecmp(typestr,"OPAQUE",3))
            return(TYPE_OPAQUE);
	if (!strncasecmp(typestr,"OBJECTID",3))
            return(TYPE_OBJID);
	if (!strncasecmp(typestr,"NETADDR",3))
	    return(TYPE_NETADDR);
	if (!strncasecmp(typestr,"COUNTER64",3))
	    return(TYPE_COUNTER64);
	if (!strncasecmp(typestr,"NULL",3))
	    return(TYPE_NULL);
	if (!strncasecmp(typestr,"BITS",3))
	    return(TYPE_BITSTRING);
	if (!strncasecmp(typestr,"ENDOFMIBVIEW",3))
	    return(SNMP_ENDOFMIBVIEW);
	if (!strncasecmp(typestr,"NOSUCHOBJECT",7))
	    return(SNMP_NOSUCHOBJECT);
	if (!strncasecmp(typestr,"NOSUCHINSTANCE",7))
	    return(SNMP_NOSUCHINSTANCE);
	if (!strncasecmp(typestr,"UINTEGER",3))
	    return(TYPE_UINTEGER); /* historic - should not show up */
                                   /* but it does?                  */
	if (!strncasecmp(typestr, "NOTIF", 3))
		return(TYPE_NOTIFTYPE);
	if (!strncasecmp(typestr, "TRAP", 4))
		return(TYPE_TRAPTYPE);
        return(TYPE_UNKNOWN);
}

static int
__translate_asn_type(type)
int type;
{
   switch (type) {
        case ASN_INTEGER:
            return(TYPE_INTEGER);
	    break;
	case ASN_OCTET_STR:
            return(TYPE_OCTETSTR);
	    break;
	case ASN_OPAQUE:
            return(TYPE_OPAQUE);
	    break;
	case ASN_OBJECT_ID:
            return(TYPE_OBJID);
	    break;
	case ASN_TIMETICKS:
            return(TYPE_TIMETICKS);
	    break;
	case ASN_GAUGE:
            return(TYPE_GAUGE);
	    break;
	case ASN_COUNTER:
            return(TYPE_COUNTER);
	    break;
	case ASN_IPADDRESS:
            return(TYPE_IPADDR);
	    break;
	case ASN_BIT_STR:
            return(TYPE_BITSTRING);
	    break;
	case ASN_NULL:
            return(TYPE_NULL);
	    break;
	/* no translation for these exception type values */
	case SNMP_ENDOFMIBVIEW:
	case SNMP_NOSUCHOBJECT:
	case SNMP_NOSUCHINSTANCE:
	    return(type);
	    break;
	case ASN_UINTEGER:
            return(TYPE_UINTEGER);
	    break;
	case ASN_COUNTER64:
            return(TYPE_COUNTER64);
	    break;
	default:
            warn("translate_asn_type: unhandled asn type (%d)\n",type);
            return(TYPE_OTHER);
            break;
        }
}

#define USE_BASIC 0
#define USE_ENUMS 1
#define USE_SPRINT_VALUE 2
static int
__snprint_value (buf, buf_len, var, tp, type, flag)
char * buf;
size_t buf_len;
netsnmp_variable_list * var;
struct tree * tp;
int type;
int flag;
{
   int len = 0;
   u_char* ip;
   struct enum_list *ep;


   buf[0] = '\0';
   if (flag == USE_SPRINT_VALUE) {
	snprint_value(buf, buf_len, var->name, var->name_length, var);
	len = strlen(buf);
   } else {
     switch (var->type) {
        case ASN_INTEGER:
           if (flag == USE_ENUMS) {
              for(ep = tp->enums; ep; ep = ep->next) {
                 if (ep->value == *var->val.integer) {
                    strcpy(buf, ep->label);
                    len = strlen(buf);
                    break;
                 }
              }
           }
           if (!len) {
              sprintf(buf,"%ld", *var->val.integer);
              len = strlen(buf);
           }
           break;

        case ASN_GAUGE:
        case ASN_COUNTER:
        case ASN_TIMETICKS:
        case ASN_UINTEGER:
           sprintf(buf,"%lu", (unsigned long) *var->val.integer);
           len = strlen(buf);
           break;

        case ASN_OCTET_STR:
        case ASN_OPAQUE:
           memcpy(buf, (char*)var->val.string, var->val_len);
           len = var->val_len;
           break;

        case ASN_IPADDRESS:
          ip = (u_char*)var->val.string;
          sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
          len = strlen(buf);
          break;

        case ASN_NULL:
           break;

        case ASN_OBJECT_ID:
          __sprint_num_objid(buf, (oid *)(var->val.objid),
                             var->val_len/sizeof(oid));
          len = strlen(buf);
          break;

	case SNMP_ENDOFMIBVIEW:
          sprintf(buf,"%s", "ENDOFMIBVIEW");
	  break;
	case SNMP_NOSUCHOBJECT:
	  sprintf(buf,"%s", "NOSUCHOBJECT");
	  break;
	case SNMP_NOSUCHINSTANCE:
	  sprintf(buf,"%s", "NOSUCHINSTANCE");
	  break;

        case ASN_COUNTER64:
          printU64(buf,(struct counter64 *)var->val.counter64);
          len = strlen(buf);
          break;

        case ASN_BIT_STR:
            snprint_bitstring(buf, sizeof(buf), var, NULL, NULL, NULL);
            len = strlen(buf);
            break;

        case ASN_NSAP:
        default:
           warn("snprint_value: asn type not handled %d\n",var->type);
     }
   }
   return(len);
}

static int
__sprint_num_objid (buf, objid, len)
char *buf;
oid *objid;
int len;
{
   int i;
   buf[0] = '\0';
   for (i=0; i < len; i++) {
	sprintf(buf,".%lu",*objid++);
	buf += strlen(buf);
   }
   return SUCCESS;
}

static int
__tp_sprint_num_objid (buf, tp)
char *buf;
SnmpMibNode *tp;
{
   oid newname[MAX_OID_LEN], *op;
   /* code taken from get_node in snmp_client.c */
   for (op = newname + MAX_OID_LEN - 1; op >= newname; op--) {
      *op = tp->subid;
      tp = tp->parent;
      if (tp == NULL) break;
   }
   return __sprint_num_objid(buf, op, newname + MAX_OID_LEN - op);
}

static int
__scan_num_objid (buf, objid, len)
char *buf;
oid *objid;
int *len;
{
   char *cp;
   *len = 0;
   if (*buf == '.') buf++;
   cp = buf;
   while (*buf) {
      if (*buf++ == '.') {
         sscanf(cp, "%lu", objid++);
         /* *objid++ = atoi(cp); */
         (*len)++;
         cp = buf;
      } else {
         if (isalpha((int)*buf)) {
	    return FAILURE;
         }
      }
   }
   sscanf(cp, "%lu", objid++);
   /* *objid++ = atoi(cp); */
   (*len)++;
   return SUCCESS;
}

static int
__get_type_str (type, str)
int type;
char * str;
{
   switch (type) {
	case TYPE_OBJID:
       		strcpy(str, "OBJECTID");
	        break;
	case TYPE_OCTETSTR:
       		strcpy(str, "OCTETSTR");
	        break;
	case TYPE_INTEGER:
       		strcpy(str, "INTEGER");
	        break;
	case TYPE_INTEGER32:
       		strcpy(str, "INTEGER32");
	        break;
	case TYPE_UNSIGNED32:
       		strcpy(str, "UNSIGNED32");
	        break;
	case TYPE_NETADDR:
       		strcpy(str, "NETADDR");
	        break;
	case TYPE_IPADDR:
       		strcpy(str, "IPADDR");
	        break;
	case TYPE_COUNTER:
       		strcpy(str, "COUNTER");
	        break;
	case TYPE_GAUGE:
       		strcpy(str, "GAUGE");
	        break;
	case TYPE_TIMETICKS:
       		strcpy(str, "TICKS");
	        break;
	case TYPE_OPAQUE:
       		strcpy(str, "OPAQUE");
	        break;
	case TYPE_COUNTER64:
       		strcpy(str, "COUNTER64");
	        break;
	case TYPE_NULL:
                strcpy(str, "NULL");
                break;
	case SNMP_ENDOFMIBVIEW:
                strcpy(str, "ENDOFMIBVIEW");
                break;
	case SNMP_NOSUCHOBJECT:
                strcpy(str, "NOSUCHOBJECT");
                break;
	case SNMP_NOSUCHINSTANCE:
                strcpy(str, "NOSUCHINSTANCE");
                break;
	case TYPE_UINTEGER:
                strcpy(str, "UINTEGER"); /* historic - should not show up */
                                          /* but it does?                  */
                break;
	case TYPE_NOTIFTYPE:
		strcpy(str, "NOTIF");
		break;
	case TYPE_BITSTRING:
		strcpy(str, "BITS");
		break;
	case TYPE_TRAPTYPE:
		strcpy(str, "TRAP");
		break;
	case TYPE_OTHER: /* not sure if this is a valid leaf type?? */
	case TYPE_NSAPADDRESS:
        default: /* unsupported types for now */
           strcpy(str, "");
           return(FAILURE);
   }
   return SUCCESS;
}

/* does a destructive disection of <label1>...<labeln>.<iid> returning
   <labeln> and <iid> in seperate strings (note: will destructively
   alter input string, 'name') */
static int
__get_label_iid (name, last_label, iid, flag)
char * name;
char ** last_label;
char ** iid;
int flag;
{
   char *lcp;
   char *icp;
   int len = strlen(name);
   int found_label = 0;

   *last_label = *iid = NULL;

   if (len == 0) return(FAILURE);

   /* Handle case where numeric oid's have been requested.  The input 'name'
   ** in this case should be a numeric OID -- return failure if not.
   */
   if ((flag & USE_NUMERIC_OIDS)) {
      if (!__is_numeric_oid(name))
       return(FAILURE);

      /* Walk backward through the string, looking for first two '.' chars */
      lcp = &(name[len]);
      icp = NULL;
      while (lcp > name) {
       if (*lcp == '.') {

          /* If this is the first occurence of '.', note it in icp.
          ** Otherwise, this must be the second occurrence, so break
          ** out of the loop.
          */
          if (icp == NULL)
             icp = lcp;
          else
             break;
       }
       lcp --;
      }

      /* Make sure we found at least a label and index. */
      if (!icp)
         return(FAILURE);

      /* Push forward past leading '.' chars and separate the strings. */
      lcp ++;
      *icp ++ = '\0';

      *last_label = (flag & USE_LONG_NAMES) ? name : lcp;
      *iid        = icp;

      return(SUCCESS);
   }

   lcp = icp = &(name[len]);

   while (lcp > name) {
      if (*lcp == '.') {
	if (found_label) {
	   lcp++;
           break;
        } else {
           icp = lcp;
        }
      }
      if (!found_label && isalpha((int)*lcp)) found_label = 1;
      lcp--;
   }

   if (!found_label || (!isdigit((int)*(icp+1)) && (flag & FAIL_ON_NULL_IID)))
      return(FAILURE);

   if (flag & NON_LEAF_NAME) { /* dont know where to start instance id */
     /* put the whole thing in label */
     icp = &(name[len]);
     flag |= USE_LONG_NAMES;
     /* special hack in case no mib loaded - object identifiers will
      * start with .iso.<num>.<num>...., in which case it is preferable
      * to make the label entirely numeric (i.e., convert "iso" => "1")
      */
      if (*lcp == '.' && lcp == name) {
         if (!strncmp(".ccitt.",lcp,7)) {
            name += 2;
            *name = '.';
            *(name+1) = '0';
         } else if (!strncmp(".iso.",lcp,5)) {
            name += 2;
            *name = '.';
            *(name+1) = '1';
         } else if (!strncmp(".joint-iso-ccitt.",lcp,17)) {
            name += 2;
            *name = '.';
            *(name+1) = '2';
         }
      }
   } else if (*icp) {
      *(icp++) = '\0';
   }
   *last_label = (flag & USE_LONG_NAMES ? name : lcp);

   *iid = icp;

   return(SUCCESS);
}


static int
__oid_cmp(oida_arr, oida_arr_len, oidb_arr, oidb_arr_len)
oid *oida_arr;
int oida_arr_len;
oid *oidb_arr;
int oidb_arr_len;
{
   for (;oida_arr_len && oidb_arr_len;
	oida_arr++, oida_arr_len--, oidb_arr++, oidb_arr_len--) {
	if (*oida_arr == *oidb_arr) continue;
	return(*oida_arr > *oidb_arr ? 1 : -1);
   }
   if (oida_arr_len == oidb_arr_len) return(0);
   return(oida_arr_len > oidb_arr_len ? 1 : -1);
}

#define MAX_BAD 0xffffff

static u_int
compute_match(search_base, key)
const char *search_base;
const char *key;
{
   int rc;
   regex_t parsetree;
   regmatch_t pmatch;

   rc = regcomp(&parsetree, key, REG_ICASE | REG_EXTENDED);
   if (rc == 0)
      rc = regexec(&parsetree, search_base, 1, &pmatch, 0);
   regfree(&parsetree);
   if (rc == 0) {
      return pmatch.rm_so;
   }

   return MAX_BAD;
}

static struct tree *
__tag2oid(tag, iid, oid_arr, oid_arr_len, type, best_guess)
char * tag;
char * iid;
oid  * oid_arr;
int  * oid_arr_len;
int  * type;
int    best_guess;
{
   struct tree *tp = NULL;
   struct tree *rtp = NULL;
   DLL_IMPORT extern struct tree *tree_head;
   oid newname[MAX_OID_LEN], *op;
   int newname_len = 0;

   if (type) *type = TYPE_UNKNOWN;
   if (oid_arr_len) *oid_arr_len = 0;
   if (!tag) goto done;

   if (best_guess) {
      tp = rtp = find_best_tree_node(tag, tree_head, NULL);
      if (tp) {
	 if (type) *type = tp->type;
	 if ((oid_arr == NULL) || (oid_arr_len == NULL)) return rtp;
	 for (op = newname + MAX_OID_LEN - 1; op >= newname; op--) {
            *op = tp->subid;
	    tp = tp->parent;
	    if (tp == NULL)
	       break;
	 }
	 *oid_arr_len = newname + MAX_OID_LEN - op;
	 memcpy(oid_arr, op, *oid_arr_len * sizeof(oid));
      }
      return(rtp);
   }

   if (strchr(tag,'.')) { /* if multi part tag  */
      if (!__scan_num_objid(tag, newname, &newname_len)) { /* numeric tag */
         newname_len = MAX_OID_LEN;
         read_objid(tag, newname, &newname_len); /* long name */
      }
      if (type) *type = (tp ? tp->type : TYPE_UNKNOWN);
      if ((oid_arr == NULL) || (oid_arr_len == NULL)) return rtp;
      memcpy(oid_arr,(char*)newname,newname_len*sizeof(oid));
      *oid_arr_len = newname_len;
   } else { /* else it is a leaf */
      rtp = tp = find_node(tag, Mib);
      if (tp) {
         if (type) *type = tp->type;
         if ((oid_arr == NULL) || (oid_arr_len == NULL)) return rtp;
         /* code taken from get_node in snmp_client.c */
         for(op = newname + MAX_OID_LEN - 1; op >= newname; op--){
           *op = tp->subid;
	   tp = tp->parent;
	   if (tp == NULL)
	      break;
         }
         *oid_arr_len = newname + MAX_OID_LEN - op;
         memcpy(oid_arr, op, *oid_arr_len * sizeof(oid));
      } else {
         return(rtp);   /* HACK: otherwise, concat_oid_str confuses things */
      }
   }
 done:
   if (iid && *iid) __concat_oid_str(oid_arr, oid_arr_len, iid);
   return(rtp);
}
/* searches down the mib tree for the given oid
   returns the last found tp and its index in lastind
 */
static struct tree *
__oid2tp (oidp, len, subtree, lastind)
oid* oidp;
int len;
struct tree * subtree;
int* lastind;
{
    struct tree    *return_tree = NULL;


    for (; subtree; subtree = subtree->next_peer) {
	if (*oidp == subtree->subid){
	    goto found;
	}
    }
    *lastind=0;
    return NULL;

found:
    if (len > 1){
       return_tree =
          __oid2tp(oidp + 1, len - 1, subtree->child_list, lastind);
       (*lastind)++;
    } else {
       *lastind=1;
    }
    if (return_tree)
	return return_tree;
    else
	return subtree;
}

/* function: __concat_oid_str
 *
 * This function converts a dotted-decimal string, soid_str, to an array
 * of oid types and concatenates them on doid_arr begining at the index
 * specified by doid_arr_len.
 *
 * returns : SUCCESS, FAILURE
 */
static int
__concat_oid_str(doid_arr, doid_arr_len, soid_str)
oid *doid_arr;
int *doid_arr_len;
char * soid_str;
{
   char soid_buf[STR_BUF_SIZE];
   char *cp;

   if (!soid_str || !*soid_str) return SUCCESS;/* successfully added nothing */
   if (*soid_str == '.') soid_str++;
   strcpy(soid_buf, soid_str);
   cp = strtok(soid_buf,".");
   while (cp) {
     sscanf(cp, "%lu", &(doid_arr[(*doid_arr_len)++]));
     /* doid_arr[(*doid_arr_len)++] =  atoi(cp); */
     cp = strtok(NULL,".");
   }
   return(SUCCESS);
}

/*
 * add a varbind to PDU
 */
static int
__add_var_val_str(pdu, name, name_length, val, len, type)
    netsnmp_pdu *pdu;
    oid *name;
    int name_length;
    char * val;
    int len;
    int type;
{
    netsnmp_variable_list *vars;
    oid oidbuf[MAX_OID_LEN];
    int ret = SUCCESS;
    struct tree *tp;

    if (pdu->variables == NULL){
	pdu->variables = vars =
           (netsnmp_variable_list *)calloc(1,sizeof(netsnmp_variable_list));
    } else {
	for(vars = pdu->variables;
            vars->next_variable;
            vars = vars->next_variable)
	    /*EXIT*/;
	vars->next_variable =
           (netsnmp_variable_list *)calloc(1,sizeof(netsnmp_variable_list));
	vars = vars->next_variable;
    }

    vars->next_variable = NULL;
    vars->name = (oid *)malloc(name_length * sizeof(oid));
    memcpy((char *)vars->name, (char *)name, name_length * sizeof(oid));
    vars->name_length = name_length;
    switch (type) {
      case TYPE_INTEGER:
      case TYPE_INTEGER32:
        vars->type = ASN_INTEGER;
        vars->val.integer = (long *)malloc(sizeof(long));
        if (val)
            *(vars->val.integer) = strtol(val,NULL,0);
        else {
            ret = FAILURE;
            *(vars->val.integer) = 0;
        }
        vars->val_len = sizeof(long);
        break;

      case TYPE_GAUGE:
      case TYPE_UNSIGNED32:
        vars->type = ASN_GAUGE;
        goto UINT;
      case TYPE_COUNTER:
        vars->type = ASN_COUNTER;
        goto UINT;
      case TYPE_TIMETICKS:
        vars->type = ASN_TIMETICKS;
        goto UINT;
      case TYPE_UINTEGER:
        vars->type = ASN_UINTEGER;
UINT:
        vars->val.integer = (long *)malloc(sizeof(long));
        if (val)
            sscanf(val,"%lu",vars->val.integer);
        else {
            ret = FAILURE;
            *(vars->val.integer) = 0;
        }
        vars->val_len = sizeof(long);
        break;

      case TYPE_OCTETSTR:
	vars->type = ASN_OCTET_STR;
	goto OCT;

      case TYPE_BITSTRING:
	vars->type = ASN_OCTET_STR;
	goto OCT;

      case TYPE_OPAQUE:
        vars->type = ASN_OCTET_STR;
OCT:
        vars->val.string = (u_char *)malloc(len);
        vars->val_len = len;
        if (val && len)
            memcpy((char *)vars->val.string, val, len);
        else {
            ret = FAILURE;
            vars->val.string = strdup("");
            vars->val_len = 0;
        }
        break;

      case TYPE_IPADDR:
        vars->type = ASN_IPADDRESS;
        vars->val.integer = (long *)malloc(sizeof(long));
        if (val)
            *(vars->val.integer) = inet_addr(val);
        else {
            ret = FAILURE;
            *(vars->val.integer) = 0;
        }
        vars->val_len = sizeof(long);
        break;

      case TYPE_OBJID:
        vars->type = ASN_OBJECT_ID;
	vars->val_len = MAX_OID_LEN;
        /* if (read_objid(val, oidbuf, &(vars->val_len))) { */
	/* tp = __tag2oid(val,NULL,oidbuf,&(vars->val_len),NULL,0); */
        if (!val || !snmp_parse_oid(val, oidbuf, &vars->val_len)) {
            vars->val.objid = NULL;
	    ret = FAILURE;
        } else {
            vars->val_len *= sizeof(oid);
            vars->val.objid = (oid *)malloc(vars->val_len);
            memcpy((char *)vars->val.objid, (char *)oidbuf, vars->val_len);
        }
        break;

      default:
        vars->type = ASN_NULL;
	vars->val_len = 0;
	vars->val.string = NULL;
	ret = FAILURE;
    }

     return ret;
}

/* takes ss and pdu as input and updates the 'response' argument */
/* the input 'pdu' argument will be freed */
static int
__send_sync_pdu(ss, pdu, response, retry_nosuch,
	        err_str_sv, err_num_sv, err_ind_sv)
netsnmp_session *ss;
netsnmp_pdu *pdu;
netsnmp_pdu **response;
int retry_nosuch;
SV * err_str_sv;
SV * err_num_sv;
SV * err_ind_sv;
{
   int status;
   long command = pdu->command;
   *response = NULL;
retry:

   status = snmp_synch_response(ss, pdu, response);

   if ((*response == NULL) && (status == STAT_SUCCESS)) status = STAT_ERROR;

   switch (status) {
      case STAT_SUCCESS:
	 switch ((*response)->errstat) {
	    case SNMP_ERR_NOERROR:
	       break;

            case SNMP_ERR_NOSUCHNAME:
               if (retry_nosuch && (pdu = snmp_fix_pdu(*response, command))) {
                  if (*response) snmp_free_pdu(*response);
                  goto retry;
               }

            /* Pv1, SNMPsec, Pv2p, v2c, v2u, v2*, and SNMPv3 PDUs */
            case SNMP_ERR_TOOBIG:
            case SNMP_ERR_BADVALUE:
            case SNMP_ERR_READONLY:
            case SNMP_ERR_GENERR:
            /* in SNMPv2p, SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs */
            case SNMP_ERR_NOACCESS:
            case SNMP_ERR_WRONGTYPE:
            case SNMP_ERR_WRONGLENGTH:
            case SNMP_ERR_WRONGENCODING:
            case SNMP_ERR_WRONGVALUE:
            case SNMP_ERR_NOCREATION:
            case SNMP_ERR_INCONSISTENTVALUE:
            case SNMP_ERR_RESOURCEUNAVAILABLE:
            case SNMP_ERR_COMMITFAILED:
            case SNMP_ERR_UNDOFAILED:
            case SNMP_ERR_AUTHORIZATIONERROR:
            case SNMP_ERR_NOTWRITABLE:
            /* in SNMPv2c, SNMPv2u, SNMPv2*, and SNMPv3 PDUs */
            case SNMP_ERR_INCONSISTENTNAME:
            default:
               sv_catpv(err_str_sv,
                        (char*)snmp_errstring((*response)->errstat));
               sv_setiv(err_num_sv, (*response)->errstat);
	       sv_setiv(err_ind_sv, (*response)->errindex);
               status = (*response)->errstat;
               break;
	 }
         break;

      case STAT_TIMEOUT:
      case STAT_ERROR:
          sv_catpv(err_str_sv, (char*)snmp_api_errstring(ss->s_snmp_errno));
          sv_setiv(err_num_sv, ss->s_snmp_errno);
         break;

      default:
         sv_catpv(err_str_sv, "send_sync_pdu: unknown status");
         sv_setiv(err_num_sv, ss->s_snmp_errno);
         break;
   }

   return(status);
}

static int
__callback_wrapper (op, ss, reqid, pdu, cb_data)
int op;
netsnmp_session *ss;
int reqid;
netsnmp_pdu *pdu;
void *cb_data;
{
  /* we should probably just increment the reference counter... */
  /*  sv_inc(cb_data); */
  return __snmp_xs_cb(op, ss, reqid, pdu, newSVsv(cb_data));
}


static int
__snmp_xs_cb (op, ss, reqid, pdu, cb_data)
int op;
netsnmp_session *ss;
int reqid;
netsnmp_pdu *pdu;
void *cb_data;
{
  SV *varlist_ref;
  AV *varlist;
  SV *varbind_ref;
  AV *varbind;
  SV *traplist_ref = NULL;
  AV *traplist = NULL;
  netsnmp_variable_list *vars;
  struct tree *tp;
  int len;
  SV *tmp_sv;
  int type;
  char tmp_type_str[MAX_TYPE_NAME_LEN];
  u_char str_buf[STR_BUF_SIZE], *str_bufp = str_buf;
  size_t str_buf_len = sizeof(str_buf);
  size_t out_len = 0;
  int buf_over = 0;
  char *label;
  char *iid;
  char *cp;
  int getlabel_flag = NO_FLAGS;
  int sprintval_flag = USE_BASIC;
  netsnmp_pdu *reply_pdu;
  int old_numeric, old_printfull;
  netsnmp_transport *transport = NULL;

  SV* cb = ((struct snmp_xs_cb_data*)cb_data)->perl_cb;
  SV* sess_ref = ((struct snmp_xs_cb_data*)cb_data)->sess_ref;
  SV **err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
  SV **err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
  SV **err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);

  dSP;
  ENTER;
  SAVETMPS;

  free(cb_data);

  sv_setpv(*err_str_svp, (char*)snmp_errstring(pdu->errstat));
  sv_setiv(*err_num_svp, pdu->errstat);
  sv_setiv(*err_ind_svp, pdu->errindex);

  varlist_ref = &sv_undef;	/* Prevent unintialized use below. */

  switch (op) {
  case NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE:
    traplist_ref = NULL;
    switch (pdu->command) {
    case SNMP_MSG_INFORM:
      /*
       * Ideally, we would use the return value from the callback to
       * decide what response, if any, we send, and what the error status
       * and error index should be.
       */
      reply_pdu = snmp_clone_pdu(pdu);
      if (reply_pdu) {
        reply_pdu->command = SNMP_MSG_RESPONSE;
        reply_pdu->reqid = pdu->reqid;
        reply_pdu->errstat = reply_pdu->errindex = 0;
        snmp_send(ss, reply_pdu);
      } else {
        warn("Couldn't clone PDU for inform response");
      }
      /* FALLTHRU */
    case SNMP_MSG_TRAP2:
      traplist = newAV();
      traplist_ref = newRV_noinc((SV*)traplist);
#if 0
      /* of dubious utility... */
      av_push(traplist, newSViv(pdu->command));
#endif
      av_push(traplist, newSViv(pdu->reqid));
      if ((transport = snmp_sess_transport(snmp_sess_pointer(ss))) != NULL) {
	cp = transport->f_fmtaddr(transport, pdu->transport_data,
				  pdu->transport_data_length);
	av_push(traplist, newSVpv(cp, strlen(cp)));
	free(cp);
      } else {
	/*  This shouldn't ever happen; every session has a transport.  */
	av_push(traplist, newSVpv("", 0));
      }
      av_push(traplist, newSVpv((char*) pdu->community, pdu->community_len));
      /* FALLTHRU */
    case SNMP_MSG_RESPONSE:
      {
      varlist = newAV();
      varlist_ref = newRV_noinc((SV*)varlist);

      /*
      ** Set up for numeric OID's, if necessary.  Save the old values
      ** so that they can be restored when we finish -- these are
      ** library-wide globals, and have to be set/restored for each
      ** session.
      */
      old_numeric = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS);
      old_printfull = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_FULL_OID);
      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1))) {
         getlabel_flag |= USE_LONG_NAMES;
         netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_FULL_OID, 1);
      }
      /* Setting UseNumeric forces UseLongNames on so check for UseNumeric
         after UseLongNames (above) to make sure the final outcome of 
         NETSNMP_DS_LIB_OID_OUTPUT_FORMAT is NETSNMP_OID_OUTPUT_NUMERIC */
      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1))) {
         getlabel_flag |= USE_NUMERIC_OIDS;
         netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS, 1);
      }

      sv_bless(varlist_ref, gv_stashpv("SNMP::VarList",0));
      for(vars = (pdu?pdu->variables:NULL); vars; vars = vars->next_variable) {
         varbind = newAV();
         varbind_ref = newRV_noinc((SV*)varbind);
         sv_bless(varbind_ref, gv_stashpv("SNMP::Varbind",0));
         av_push(varlist, varbind_ref);
         *str_buf = '.';
         *(str_buf+1) = '\0';
         out_len = 0;
         tp = netsnmp_sprint_realloc_objid_tree(&str_bufp, &str_buf_len,
                                                &out_len, 0, &buf_over,
                                                vars->name,vars->name_length);
         str_buf[sizeof(str_buf)-1] = '\0';
         if (__is_leaf(tp)) {
            type = tp->type;
         } else {
            getlabel_flag |= NON_LEAF_NAME;
            type = __translate_asn_type(vars->type);
         }
         __get_label_iid(str_buf,&label,&iid,getlabel_flag);
         av_store(varbind, VARBIND_TAG_F,
                  newSVpv(label, strlen(label)));
         av_store(varbind, VARBIND_IID_F,
                  newSVpv(iid, strlen(iid)));
         __get_type_str(type, tmp_type_str);
         tmp_sv = newSVpv(tmp_type_str, strlen(tmp_type_str));
         av_store(varbind, VARBIND_TYPE_F, tmp_sv);
         len = __snprint_value(str_buf, sizeof(str_buf),
                              vars, tp, type, sprintval_flag);
         tmp_sv = newSVpv((char*)str_buf, len);
         av_store(varbind, VARBIND_VAL_F, tmp_sv);
      } /* for */

      /* Reset the library's behavior for numeric/symbolic OID's. */
      netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS, old_numeric );
      netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_FULL_OID, old_printfull);

      } /* case SNMP_MSG_RESPONSE */
      break;
    default:;
    } /* switch pdu->command */
    break;

  case NETSNMP_CALLBACK_OP_TIMED_OUT:
    varlist_ref = &sv_undef;
    break;
  default:;
  } /* switch op */
  sv_2mortal(cb);
  cb = __push_cb_args2(cb,
                 (SvTRUE(varlist_ref) ? sv_2mortal(varlist_ref):varlist_ref),
	         (SvTRUE(traplist_ref) ? sv_2mortal(traplist_ref):traplist_ref));
  __call_callback(cb, G_DISCARD);

  FREETMPS;
  LEAVE;
  sv_2mortal(sess_ref);
  return 1;
}

static SV *
__push_cb_args2(sv,esv,tsv)
SV *sv;
SV *esv;
SV *tsv;
{
   dSP;
   if (SvTYPE(SvRV(sv)) != SVt_PVCV) sv = SvRV(sv);

   PUSHMARK(sp);
   if (SvTYPE(sv) == SVt_PVAV) {
      AV *av = (AV *) sv;
      int n = av_len(av) + 1;
      SV **x = av_fetch(av, 0, 0);
      if (x) {
         int i = 1;
         sv = *x;

         for (i = 1; i < n; i++) {
            x = av_fetch(av, i, 0);
            if (x) {
               SV *arg = *x;
               XPUSHs(sv_mortalcopy(arg));
            } else {
               XPUSHs(&sv_undef);
            }
         }
      } else {
         sv = &sv_undef;
      }
   }
   if (esv) XPUSHs(sv_mortalcopy(esv));
   if (tsv) XPUSHs(sv_mortalcopy(tsv));
   PUTBACK;
   return sv;
}

static int
__call_callback(sv, flags)
SV *sv;
int flags;
{
 dSP;
 I32 myframe = TOPMARK;
 I32 count;
 ENTER;
 if (SvTYPE(sv) == SVt_PVCV)
  {
   count = perl_call_sv(sv, flags);
  }
 else if (SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVCV)
  {
   count = perl_call_sv(SvRV(sv), flags);
  }
 else
  {

   SV **top = stack_base + myframe + 1;
   SV *obj = *top;
   if (SvPOK(sv) && SvROK(obj) && SvOBJECT(SvRV(obj)))
    {
     count = perl_call_method(SvPV(sv, na), flags);
    }
   else if (SvPOK(obj) && SvROK(sv) && SvOBJECT(SvRV(sv)))
    {
     /* We have obj method ...
        Used to be used instead of LangMethodCall()
      */
     *top = sv;
     count = perl_call_method(SvPV(obj, na), flags);
    }
   else
    {
     count = perl_call_sv(sv, flags);
    }
 }
 LEAVE;
 return count;
}

/* Bulkwalk support routines */

/* Add a context pointer to the list of valid pointers.  Place it in the first
** NULL slot in the array.
*/
static int
_context_add(walk_context *context)
{
    int i, j, new_sz;

    if ((i = _context_okay(context)) != 0)	/* Already exists?  Okay. */
	return i;

    /* Initialize the array if necessary. */
    if (_valid_contexts == NULL) {

	/* Create the _valid_contexts structure. */
	Newz(0, _valid_contexts, 1, struct valid_contexts);
	assert(_valid_contexts != NULL);

	/* Populate the original valid contexts array. */
	Newz(0, _valid_contexts->valid, 4, walk_context *);
	assert(_valid_contexts->valid != NULL);

	/* Computer number of slots in the array. */
	_valid_contexts->sz_valid = sizeof(*_valid_contexts->valid) /
							sizeof(walk_context *);

	for (i = 0; i < _valid_contexts->sz_valid; i++)
	    _valid_contexts->valid[i] = NULL;

	DBPRT(3, (DBOUT "Created valid_context array 0x%p (%d slots)\n",
			    _valid_contexts->valid, _valid_contexts->sz_valid));
    }

    /* Search through the list, looking for NULL's -- unused slots. */
    for (i = 0; i < _valid_contexts->sz_valid; i++)
	if (_valid_contexts->valid[i] == NULL)
	    break;

    /* Did we walk off the end of the list?  Need to grow the list.  Double
    ** it for now.
    */
    if (i == _valid_contexts->sz_valid) {
	new_sz = _valid_contexts->sz_valid * 2;

	Renew(_valid_contexts->valid, new_sz, walk_context *);
	assert(_valid_contexts->valid != NULL);

	DBPRT(3, (DBOUT "Resized valid_context array 0x%p from %d to %d slots\n",
		    _valid_contexts->valid, _valid_contexts->sz_valid, new_sz));

	_valid_contexts->sz_valid = new_sz;

	/* Initialize the new half of the resized array. */
	for (j = i; j < new_sz; j++)
	    _valid_contexts->valid[j] = NULL;
    }

    /* Store the context pointer in the array and return 0 (success). */
    _valid_contexts->valid[i] = context;
    DBPRT(3,( "Add context 0x%p to valid context list\n", context));
    return 0;
}

/* Remove a context pointer from the valid list.  Replace the pointer with
** NULL in the valid pointer list.
*/
static int
_context_del(walk_context *context)
{
    int i;

    if (_valid_contexts == NULL)	/* Make sure it was initialized. */
	return 1;

    for (i = 0; i < _valid_contexts->sz_valid; i++) {
	if (_valid_contexts->valid[i] == context) {
	    DBPRT(3,( "Remove context 0x%p from valid context list\n", context));
	    _valid_contexts->valid[i] = NULL;	/* Remove it from the list.  */
	    return 0;				/* Return successful status. */
	}
    }
    return 1;
}

/* Check if a specific context pointer is in the valid list.  Return true (1)
** if the context is still in the valid list, or 0 if not (or context is NULL).
*/
static int
_context_okay(walk_context *context)
{
    int i;

    if (_valid_contexts == NULL)	/* Make sure it was initialized. */
	return 0;

    if (context == NULL)		/* Asked about a NULL context? Fail. */
	return 0;

    for (i = 0; i < _valid_contexts->sz_valid; i++)
	if (_valid_contexts->valid[i] == context)
	    return 1;			/* Found it! */

    return 0;				/* No match -- return failure. */
}

/* Check if the walk is completed, based upon the context.  Also set the
** ignore flag on any completed variables -- this prevents them from being
** being sent in later packets.
*/
static int
_bulkwalk_done(walk_context *context)
{
   int is_done = 1;
   int i;
   bulktbl *bt_entry;		/* bulktbl requested OID entry */

   /* Don't consider walk done until at least one packet has been exchanged. */
   if (context->pkts_exch == 0)
      return 0;

   /* Fix up any requests that have completed.  If the complete flag is set,
   ** or it is a non-repeater OID, set the ignore flag so that it will not
   ** be considered further.  Assume we are done with the walk, and note
   ** otherwise if we aren't.  Return 1 if all requests are complete, or 0
   ** if there's more to do.
   */
   for (i = 0; i < context->nreq_oids; i ++) {
      bt_entry = &context->req_oids[i];

      if (bt_entry->complete || bt_entry->norepeat) {

 	/* This request is complete.  Remove it from list of
 	** walks still in progress.
 	*/
 	DBPRT(1, (DBOUT "Ignoring %s request oid %s\n",
 	      bt_entry->norepeat? "nonrepeater" : "completed",
 	      snprint_objid(_debugx, sizeof(_debugx), bt_entry->req_oid,
 				    bt_entry->req_len)));

 	/* Ignore this OID in any further packets. */
 	bt_entry->ignore = 1;
      }

      /* If any OID is not being ignored, the walk is not done.  Must loop
      ** through all requests to do the fixup -- no early return possible.
      */
      if (!bt_entry->ignore)
 	 is_done = 0;
   }

   return is_done;		/* Did the walk complete? */
}

/* Callback registered with SNMP.  Return 1 from this callback to cause the
** current request to be deleted from the retransmit queue.
*/
static int
_bulkwalk_async_cb(int		op,
		  SnmpSession	*ss,
		  int 		reqid,
		  netsnmp_pdu *pdu,
		  void		*context_ptr)
{
   walk_context *context;
   int	done = 0;
   int	npushed;
   SV **err_str_svp;
   SV **err_num_svp;

   /* Handle callback request for asynchronous bulkwalk.  If the bulkwalk has
   ** not completed, and has not timed out, send the next request packet in
   ** the walk.
   **
   ** Return 0 to indicate success (caller ignores return value).
   */

   DBPRT(2, (DBOUT "bulkwalk_async_cb(op %d, reqid 0x%08X, context 0x%p)\n",
							op, reqid, context_ptr));

   context = (walk_context *)context_ptr;

   /* Make certain this is a valid context pointer.  This pdu may
   ** have been retransmitted after the bulkwalk was completed
   ** (and the context was destroyed).  If so, just return.
   */
   if (!_context_okay(context)) {
      DBPRT(2,( "Ignoring PDU for dead context 0x%p...\n", context));
      return 1;
   }

   /* Is this a retransmission of a request we've already seen or some
   ** unexpected request id?  If so, just ignore it.
   */
   if (reqid != context->exp_reqid) {
       DBPRT(2,
             ("Got reqid 0x%08X, expected reqid 0x%08X.  Ignoring...\n", reqid,
              context->exp_reqid));
      return 1;
   }
   /* Ignore any future packets for this reqid. */
   context->exp_reqid = -1;

   err_str_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorStr", 8, 1);
   err_num_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorNum", 8, 1);

   switch (op) {
      case NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE:
      {
	 DBPRT(1,( "Received message for reqid 0x%08X ...\n", reqid));

	 switch (pdu->command)
	 {
	    case SNMP_MSG_RESPONSE:
	    {
	       DBPRT(2, (DBOUT "Calling bulkwalk_recv_pdu(context 0x%p, pdu 0x%p)\n",
							   context_ptr, pdu));

	       /* Handle the response PDU.  If an error occurs or there were
	       ** no variables in the response, consider the walk done.  If
	       ** the response was okay, check if we have any more to do after
	       ** this response.
	       */
	       if (_bulkwalk_recv_pdu(context, pdu) <= 0)
		  done = 1;
	       else
		  done = _bulkwalk_done(context); /* Also set req ignore flags */
	       break;
	    }
	    default:
	    {
	       DBPRT(1,( "unexpected pdu->command %d\n", pdu->command));
	       done = 1;   /* "This can't happen!", so bail out when it does. */
	       break;
	    }
	 }

	 break;
      }

      case NETSNMP_CALLBACK_OP_TIMED_OUT:
      {
	 DBPRT(1,( "\n*** Timeout for reqid 0x%08X\n\n", reqid));

         sv_setpv(*err_str_svp, (char*)snmp_api_errstring(SNMPERR_TIMEOUT));
         sv_setiv(*err_num_svp, SNMPERR_TIMEOUT);

	 /* Timeout means something bad has happened.  Return a not-okay
	 ** result to the async callback.
	 */
	 npushed = _bulkwalk_finish(context, 0 /* NOT OKAY */);
	 return 1;
      }

      default:
      {
	 DBPRT(1,( "unexpected callback op %d\n", op));
         sv_setpv(*err_str_svp, (char*)snmp_api_errstring(SNMPERR_GENERR));
         sv_setiv(*err_num_svp, SNMPERR_GENERR);
	 npushed = _bulkwalk_finish(context, 0 /* NOT OKAY */);
	 return 1;
      }
   }

   /* We have either timed out, or received and parsed in a response.  Now,
   ** if we have more variables to test, call bulkwalk_send_pdu() to enqueue
   ** another async packet, and return.
   **
   ** If, however, the bulkwalk has completed (or an error has occurred that
   ** cuts the walk short), call bulkwalk_finish() to push the results onto
   ** the Perl call stack.  Then explicitly call the Perl callback that was
   ** passed in by the user oh-so-long-ago.
   */
   if (!done) {
      DBPRT(1,( "bulkwalk not complete -- send next pdu from callback\n"));

      if (_bulkwalk_send_pdu(context) != NULL)
	 return 1;

      DBPRT(1,( "send_pdu() failed!\n"));
      /* Fall through and return what we have so far. */
   }

   /* Call the perl callback with the return values and we're done. */
   npushed = _bulkwalk_finish(context, 1 /* OKAY */);

   return 1;
}

static netsnmp_pdu *
_bulkwalk_send_pdu(walk_context *context)
{
   netsnmp_pdu *pdu = NULL;
   netsnmp_pdu *response = NULL;
   struct bulktbl  *bt_entry;
   int	nvars = 0;
   int	reqid;
   int	status;
   int	i;

   /* Send a pdu requesting any remaining variables in the context.
   **
   ** In synchronous mode, returns a pointer to the response packet.
   **
   ** In asynchronous mode, it returns the request ID, cast to a struct snmp *,
   **   not a valid SNMP response packet.  The async code should not be trying
   **   to get variables out of this "response".
   **
   ** In either case, return a NULL pointer on error or failure.
   */

   SV **sess_ptr_sv = hv_fetch((HV*)SvRV(context->sess_ref), "SessPtr", 7, 1);
   netsnmp_session *ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
   SV **err_str_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorStr", 8, 1);
   SV **err_num_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorNum", 8, 1);
   SV **err_ind_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorInd", 8, 1);

   /* Create a new PDU and send the remaining set of requests to the agent. */
   pdu = snmp_pdu_create(SNMP_MSG_GETBULK);
   if (pdu == NULL) {
      sv_setpv(*err_str_svp, "snmp_pdu_create(GETBULK) failed: ");
      sv_catpv(*err_str_svp, strerror(errno));
      sv_setiv(*err_num_svp, SNMPERR_MALLOC);
      goto err;
   }

   /* Request non-repeater variables only in the first packet exchange. */
   pdu->errstat  = (context->pkts_exch == 0) ? context->non_reps : 0;
   pdu->errindex = context->max_reps;

   for (i = 0; i < context->nreq_oids; i++) {
      bt_entry = &context->req_oids[i];
      if (bt_entry->ignore)
	 continue;

      assert(bt_entry->complete == 0);

      if (!snmp_add_null_var(pdu, bt_entry->last_oid, bt_entry->last_len)) {
	 sv_setpv(*err_str_svp, "snmp_add_null_var() failed");
	 sv_setiv(*err_num_svp, SNMPERR_GENERR);
	 sv_setiv(*err_ind_svp, i);
	 goto err;
      }

      nvars ++;

      DBPRT(1, (DBOUT "   Add %srepeater %s\n", bt_entry->norepeat ? "non" : "",
	         snprint_objid(_debugx, sizeof(_debugx), bt_entry->last_oid, bt_entry->last_len)));
   }

   /* Make sure variables are actually being requested in the packet. */
   assert (nvars != 0);

   context->pkts_exch ++;

   DBPRT(1, (DBOUT "Sending %ssynchronous request %d...\n",
		     SvTRUE(context->perl_cb) ? "a" : "", context->pkts_exch));

   /* We handle the asynchronous and synchronous requests differently here.
   ** For async, we simply enqueue the packet with a callback to handle the
   ** returned response, then return.  Note that this we call the bulkwalk
   ** callback, and hand it the walk_context, not the Perl callback.  The
   ** snmp_async_send() function returns the reqid on success, 0 on failure.
   */
   if (SvTRUE(context->perl_cb)) {
      reqid = snmp_async_send(ss, pdu, _bulkwalk_async_cb, (void *)context);

      DBPRT(2,( "bulkwalk_send_pdu(): snmp_async_send => 0x%08X\n", reqid));

      if (reqid == 0) {
	 sv_setpv(*err_str_svp, (char*)snmp_api_errstring(ss->s_snmp_errno));
	 sv_setiv(*err_num_svp, ss->s_snmp_errno);
	 goto err;
      }

      /* Make a note of the request we expect to be answered. */
      context->exp_reqid = reqid;

      /* Callbacks take care of the rest.  Let the caller know how many vars
      ** we sent in this request.  Note that this is not a valid SNMP PDU,
      ** but that's because a response has not yet been received.
      */
      return (netsnmp_pdu *)reqid;
   }

   /* This code is for synchronous mode support.
   **
   ** Send the PDU and block awaiting the response.  Return the response
   ** packet back to the caller.  Note that snmp_sess_read() frees the pdu.
   */
   status = __send_sync_pdu(ss, pdu, &response, NO_RETRY_NOSUCH,
				    *err_str_svp, *err_num_svp, *err_ind_svp);

   pdu = NULL;

   /* Check for a failed request.  __send_sync_pdu() will set the appropriate
   ** values in the error string and number SV's.
   */
   if (status != STAT_SUCCESS) {
      DBPRT(1,( "__send_sync_pdu() -> %d\n",(int)status));
      goto err;
   }

   DBPRT(1, (DBOUT "%d packets exchanged, response 0x%p\n", context->pkts_exch,
								    response));
   return response;


   err:
   if (pdu)
      snmp_free_pdu(pdu);
   return NULL;
}

/* Handle an incoming GETBULK response PDU.  This function just pulls the
** variables off of the PDU and builds up the arrays of returned values
** that are stored in the context.
**
** Returns the number of variables found in this packet, or -1 on error.
** Note that the caller is expected to free the pdu.
*/
static int
_bulkwalk_recv_pdu(walk_context *context, netsnmp_pdu *pdu)
{
   netsnmp_variable_list *vars;
   struct tree	*tp;
   char		type_str[MAX_TYPE_NAME_LEN];
   u_char	str_buf[STR_BUF_SIZE], *str_bufp = str_buf;
   size_t str_buf_len = sizeof(str_buf);
   size_t out_len = 0;
   int buf_over = 0;
   char		*label;
   char		*iid;
   bulktbl	*expect = NULL;
   int		old_numeric;
   int		old_printfull;
   int		old_format;
   int		getlabel_flag;
   int		type;
   int		pix;
   int		len;
   int		i;
   AV		*varbind;
   SV		*rv;
   SV **sess_ptr_sv = hv_fetch((HV*)SvRV(context->sess_ref), "SessPtr", 7, 1);
   SV **err_str_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorStr", 8, 1);
   SV **err_num_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorNum", 8, 1);
   SV **err_ind_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorInd", 8, 1);

   DBPRT(3, (DBOUT "bulkwalk: sess_ref = 0x%p, sess_ptr_sv = 0x%p\n",
             context->sess_ref, sess_ptr_sv));

   /* Set up for numeric OID's, if necessary.  Save the old values
   ** so that they can be restored when we finish -- these are
   ** library-wide globals, and have to be set/restored for each
   ** session.
   */
   old_numeric   = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS);
   old_printfull = netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_FULL_OID);
   old_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);
   if (context->getlabel_f & USE_NUMERIC_OIDS) {
      DBPRT(2,( "Using numeric oid's\n"));
      netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS, 1);
      netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_FULL_OID, 1);
      netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT, NETSNMP_OID_OUTPUT_NUMERIC);
   }

   /* Parse through the list of variables returned, adding each return to
   ** the appropriate array (as a VarBind).  Also keep track of which
   ** repeated OID we're expecting to see, and check if that tree walk has
   ** been completed (i.e. we've walked past the root of our request).  If
   ** so, mark the request complete so that we don't send it again in any
   ** subsequent request packets.
   */
   if (context->pkts_exch == 1)
      context->reqbase = context->req_oids;	/* Request with non-repeaters */
   else
      context->reqbase = context->repbase;	/* Request only repeater vars */

   /* Note the first variable we expect to see.  Should be reqbase. */
   expect = context->reqbase;

   for (vars = pdu->variables, pix = 0;
	vars != NULL;
	vars = vars->next_variable, pix ++)
   {

      /* If no outstanding requests remain, we're done.  This works, but it
      ** causes the reported total variable count to be wrong (since the
      ** remaining vars on the last packet are not counted).  In practice
      ** this is probably worth the win, but for debugging it's not.
      */
      if (context->req_remain == 0) {
	 DBPRT(2,( "No outstanding requests remain.  Terminating processing.\n"));
	 while (vars) {
	    pix ++;
	    vars = vars->next_variable;
	 }
	 break;
      }

      /* Determine which OID we expect to see next.  We assert that the OID's
      ** must be returned in the expected order.  The first nreq_oids returns
      ** should match the req_oids array, after that, we must cycle through
      ** the repeaters in order.  Non-repeaters are not included in later
      ** packets, so cannot have the "ignore" flag set.
      */

      if (context->oid_saved < context->non_reps) {
	 assert(context->pkts_exch == 1);

	 expect = context->reqbase ++;
	 assert(expect->norepeat);

      } else {
	 /* Must be a repeater.  Look for the first one that is not being
	 ** ignored.  Make sure we don't loop around to where we started.
	 ** If we get here but everything is being ignored, there's a problem.
	 **
	 ** Note that we *do* accept completed but not ignored OID's -- these
	 ** are OID's for trees that have been completed sometime in this
	 ** response, but must be looked at to maintain ordering.
	 */

	 if (pix == 0) {
	    /* Special case code for no non-repeater case.  This
	    ** is necessary because expect normally points to the
	    ** last non-repeater upon entry to this code (so the
	    ** '++expect' below increments it into the repeaters
	    ** section of the req_oids[] array).
	    ** If there are no non-repeaters, the expect pointer
	    ** is never initialized.  This addresses this problem.
	    */
	    expect = context->reqbase;

	 } else {

	    /* Find the repeater OID we expect to see.  Ignore any
	    ** OID's marked 'ignore' -- these have been completed
	    ** and were not requested in this iteration.
	    */
	    for (i = 0; i < context->repeaters; i++) {

	       /* Loop around to first repeater if we hit the end. */
	       if (++ expect == &context->req_oids[context->nreq_oids])
		  expect = context->reqbase = context->repbase;

	       /* Stop if this OID is not being ignored. */
	       if (!expect->ignore)
		  break;
	    }

	    /* Make sure we did find an expected OID. */
	    assert(i <= context->repeaters);
	 }
      }

      DBPRT(2, (DBOUT "Var %03d request %s\n", pix, snprint_objid(_debugx, sizeof(_debugx), 
					     expect->req_oid, expect->req_len)));

      /* Did we receive an error condition for this variable?
      ** If it's a repeated variable, mark it as complete and
      ** fall through to the block below.
      */
      if ((vars->type == SNMP_ENDOFMIBVIEW) ||
	  (vars->type == SNMP_NOSUCHOBJECT) ||
	  (vars->type == SNMP_NOSUCHINSTANCE))
      {
	 DBPRT(2,( "error type %d\n", (int)vars->type));

	 /* ENDOFMIBVIEW should be okay for a repeater - just walked off the
	 ** end of the tree.  Mark the request as complete, and go on to the
	 ** next one.
	 */
	 if ((context->oid_saved >= context->non_reps) &&
	     (vars->type == SNMP_ENDOFMIBVIEW))
	 {
	    expect->complete = 1;
	    DBPRT(2, (DBOUT "Ran out of tree for oid %s\n",
			   snprint_objid(_debugx, sizeof(_debugx), vars->name,vars->name_length)));

	    context->req_remain --;

	    /* Go on to the next variable. */
	    continue;

	 }
	 sv_setpv(*err_str_svp,
			      (char*)snmp_api_errstring(SNMPERR_UNKNOWN_OBJID));
	 sv_setiv(*err_num_svp, SNMPERR_UNKNOWN_OBJID);
	 sv_setiv(*err_ind_svp, pix);
	 goto err;
      }

      /* If this is not the first packet, skip any duplicated OID values, if
      ** present.  These should be the seed values copied from the last OID's
      ** of the previous packet.  In practice we don't see this, but it is
      ** easy enough to do, and will avoid confusion for the caller from mis-
      ** behaving agents (badly misbehaving... ;^).
      */
      if ((context->pkts_exch > 1) && (pix < context->repeaters)) {
	 if (__oid_cmp(vars->name, vars->name_length,
				   context->reqbase[pix].last_oid,
				   context->reqbase[pix].last_len) == 0)
	 {
	    DBPRT(2, (DBOUT "Ignoring repeat oid: %s\n",
			snprint_objid(_debugx, sizeof(_debugx), vars->name,vars->name_length)));

	    continue;
	 }
      }

      context->oid_total ++;	/* Count each variable received. */

      /* If this is a non-repeater, handle it.  Otherwise, if it is a
      ** repeater, has the walk wandered off of the requested tree?  If so,
      ** this request is complete, so mark it as such.  Ignore any other
      ** variables in a completed request.  In order to maintain the correct
      ** ordering of which variables we expect to see in this packet, we must
      ** not set the ignore flags immediately.  It is done in bulkwalk_done().
      ** XXX Can we use 'expect' instead of 'context->req_oids[pix]'?
      */
      if (context->oid_saved < context->non_reps) {
	 DBPRT(2, (DBOUT "   expected var %s (nonrepeater %d/%d)\n",
		     snprint_objid(_debugx, sizeof(_debugx), context->req_oids[pix].req_oid,
					   context->req_oids[pix].req_len),
		     pix, context->non_reps));
	 DBPRT(2, (DBOUT "   received var %s\n",
		     snprint_objid(_debugx, sizeof(_debugx), vars->name, vars->name_length)));

	 /* This non-repeater has now been seen, so mark the sub-tree as
	 ** completed.  Note that this may not be the same oid as requested,
	 ** since non-repeaters act like GETNEXT requests, not GET's. <sigh>
	 */
	 context->req_oids[pix].complete = 1;
	 context->req_remain --;

      } else {		/* Must be a repeater variable. */

	 DBPRT(2, (DBOUT "   received oid %s\n",
	       snprint_objid(_debugx, sizeof(_debugx), vars->name, vars->name_length)));

	 /* Are we already done with this tree?  If so, just ignore this
	 ** variable and move on to the next expected variable.
	 */
	 if (expect->complete) {
	    DBPRT(2,( "      this branch is complete - ignoring.\n"));
	    continue;
	 }

	 /* If the base oid of this variable doesn't match the expected oid,
	 ** assume that we've walked past the end of the subtree.  Set this
	 ** subtree to be completed, and go on to the next variable.
	 */
	 if ((vars->name_length < expect->req_len) ||
	     (memcmp(vars->name, expect->req_oid, expect->req_len*sizeof(oid))))
	 {
	    DBPRT(2,( "      walked off branch - marking subtree as complete.\n"));
	    expect->complete = 1;
	    context->req_remain --;
	    continue;
	 }

	 /* Still interested in the tree -- we need to keep track of the
	 ** last-seen value in case we need to send an additional request
	 ** packet.
	 */
	 (void)memcpy(expect->last_oid, vars->name,
					     vars->name_length * sizeof(oid));
	 expect->last_len = vars->name_length;

      }

      /* Create a new Varbind and populate it with the parsed information
      ** returned by the agent.  This Varbind is then pushed onto the arrays
      ** maintained for each request OID in the context.  These varbinds are
      ** collected into a return array by bulkwalk_finish().
      */
      varbind = (AV*) newAV();
      if (varbind == NULL) {
	 sv_setpv(*err_str_svp, "newAV() failed: ");
	 sv_catpv(*err_str_svp, (char*)strerror(errno));
	 sv_setiv(*err_num_svp, SNMPERR_MALLOC);
	 goto err;
      }

      *str_buf = '.';
      *(str_buf+1) = '\0';
      out_len = 0;
      tp = netsnmp_sprint_realloc_objid_tree(&str_bufp, &str_buf_len,
                                             &out_len, 0, &buf_over,
                                             vars->name,vars->name_length);
      str_buf[sizeof(str_buf)-1] = '\0';

      getlabel_flag = context->getlabel_f;

      if (__is_leaf(tp)) {
	 type = tp->type;
      } else {
	 getlabel_flag |= NON_LEAF_NAME;
	 type = __translate_asn_type(vars->type);
      }
      if (__get_label_iid(str_buf, &label, &iid, getlabel_flag) == FAILURE) {
          label = str_buf;
          iid = label + strlen(label);
      }

      DBPRT(2,( "       save var %s.%s = ", label, iid));

      av_store(varbind, VARBIND_TAG_F, newSVpv(label, strlen(label)));
      av_store(varbind, VARBIND_IID_F, newSVpv(iid, strlen(iid)));

      __get_type_str(type, type_str);
      av_store(varbind, VARBIND_TYPE_F, newSVpv(type_str, strlen(type_str)));

      len=__snprint_value(str_buf, sizeof(str_buf),
                         vars, tp, type, context->sprintval_f);
      av_store(varbind, VARBIND_VAL_F, newSVpv((char*)str_buf, len));

      str_buf[len] = '\0';
      DBPRT(3,( "'%s' (%s)\n", str_buf, type_str));

#if 0
    /* huh? */
      /* If necessary, store a timestamp as the semi-documented 5th element. */
      if (sv_timestamp)
	  av_store(varbind, VARBIND_TIME_F, SvREFCNT_inc(sv_timestamp));
#endif

      /* Push ref to the varbind onto the list of vars for OID. */
      rv = newRV_noinc((SV *)varbind);
      sv_bless(rv, gv_stashpv("SNMP::Varbind", 0));
      av_push(expect->vars, rv);

      context->oid_saved ++;	/* Count this as a saved variable. */

   } /* next variable in response packet */

   DBPRT(1, (DBOUT "-- pkt %d saw %d vars, total %d (%d saved)\n", context->pkts_exch,
			   pix, context->oid_total, context->oid_saved));

   /* We assert that all non-repeaters must be returned in
   ** the initial response (they are not repeated in additional
   ** packets, so would be dropped).  If nonrepeaters still
   ** exist, consider it a fatal error.
   */
   if ((context->pkts_exch == 1) && (context->oid_saved < context->non_reps)) {
      /* Re-use space from the value string for error message. */
      sprintf(str_buf, "%d non-repeaters went unanswered", context->non_reps);
      sv_setpv(*err_str_svp, str_buf);
      sv_setiv(*err_num_svp, SNMPERR_GENERR);
      sv_setiv(*err_num_svp, context->oid_saved);
      goto err;
   }

   /* Reset the library's behavior for numeric/symbolic OID's. */
   if (context->getlabel_f & USE_NUMERIC_OIDS) {
      netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS, old_numeric);
      netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_FULL_OID, old_printfull);
      netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT, old_format);
   }

   return pix;

   err:
   if (pdu)
      snmp_free_pdu(pdu);
   return -1;

}

/* Once the bulkwalk has completed, extend the stack and push references to
** each of the arrays of SNMP::Varbind's onto the stack.  Return the number
** of arrays pushed on the stack.  The caller should return to Perl, or call
** the Perl callback function.
**
** Note that this function free()'s the walk_context and request bulktbl's.
*/
static int
_bulkwalk_finish(walk_context *context, int okay)
{
   int		npushed = 0;
   int		i;
   int		async = 0;
   bulktbl	*bt_entry;
   AV		*ary = NULL;
   SV		*rv;
   SV		*perl_cb;

   SV **err_str_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorStr", 8, 1);
   SV **err_num_svp = hv_fetch((HV*)SvRV(context->sess_ref), "ErrorNum", 8, 1);

   dXSARGS;

   async = SvTRUE(context->perl_cb);

   /* Successfully completed the bulkwalk.  For synchronous calls, push each
   ** of the request value arrays onto the stack, and return the number of
   ** items pushed onto the stack.  For async, create a new array and push
   ** the references onto it.  The array is then passed to the Perl callback.
   */
   if (!async)
      SP -= items;

   DBPRT(1, (DBOUT "Bulwalk %s (saved %d/%d), ", okay ? "completed" : "had error",
					context->oid_saved, context->oid_total));

   if (okay) {
       DBPRT(1, (DBOUT "%s %d varbind refs %s\n",
				async ? "pass ref to array of" : "return",
				context->nreq_oids,
				async ? "to callback" : "on stack to caller"));

       /* Create the array to hold the responses for the asynchronous callback,
       ** or pre-extend the stack enough to hold responses for synch return.
       */
       if (async) {
	   ary = (AV *)newAV();
	  if (ary == NULL) {
	     sv_setpv(*err_str_svp, "newAV(): ");
	     sv_catpv(*err_str_svp, (char *)strerror(errno));
	     sv_setiv(*err_num_svp, errno);
	  }

	  /* NULL ary pointer is okay -- we'll handle it below... */

       } else {
	   EXTEND(sp, context->nreq_oids);

       }

       /* Push a reference to each array of varbinds onto the stack, in
       ** the order requested.  Note that these arrays may be empty.
       */
       for (i = 0; i < context->nreq_oids; i++) {
	  bt_entry = &context->req_oids[i];

	  DBPRT(2, (DBOUT "  %sreq #%d (%s) => %d var%s\n",
		 bt_entry->complete ? "" : "incomplete ", i,
		 snprint_objid(_debugx, sizeof(_debugx), bt_entry->req_oid, bt_entry->req_len),
		 (int)av_len(bt_entry->vars) + 1,
		 (int)av_len(bt_entry->vars) > 0 ? "s" : ""));

	  if (async && ary == NULL) {
	     DBPRT(2,( "    [dropped due to newAV() failure]\n"));
	     continue;
	  }

	  /* Get a reference to the varlist, and push it onto array or stack */
	  rv = newRV_noinc((SV *)bt_entry->vars);
	  sv_bless(rv, gv_stashpv("SNMP::VarList",0));

	  if (async)
	     av_push(ary, rv);
	  else
	     PUSHs(sv_2mortal((SV *)rv));

	  npushed ++;
       }

   } else {	/* Not okay -- push a single undef on the stack if not async */

      if (!async) {
	 XPUSHs(&sv_undef);
	 npushed = 1;
      }
   }

   /* XXX Future enhancement -- make statistics (pkts exchanged, vars
   ** saved vs. received, total time, etc) available to caller so they
   ** can adjust their request parameters and/or re-order requests.
   */

   PUTBACK;

   if (async) {
       /* Asynchronous callback.  Push the caller's arglist onto the stack,
       ** and follow it with the contents of the array (or undef if newAV()
       ** failed or the session had an error).  Then mortalize the Perl
       ** callback pointer, and call the callback.
       */
       if (!okay || ary == NULL)
          rv = &sv_undef;
       else
	  rv = newRV_noinc((SV *)ary);

       sv_2mortal(perl_cb = context->perl_cb);
       perl_cb = __push_cb_args(perl_cb, (SvTRUE(rv) ? sv_2mortal(rv) : rv));

       __call_callback(perl_cb, G_DISCARD);
   }
   sv_2mortal(context->sess_ref);

   /* Free the allocated space for the request states and return number of
   ** variables found.  Remove the context from the valid context list.
   */
   _context_del(context);
   DBPRT(2,( "Free() context->req_oids\n"));
   Safefree(context->req_oids);
   DBPRT(2,( "Free() context 0x%p\n", context));
   Safefree(context);
   return npushed;
}

/* End of bulkwalk support routines */

static char *
__av_elem_pv(AV *av, I32 key, char *dflt)
{
   SV **elem = av_fetch(av, key, 0);

   return (elem && SvOK(*elem)) ? SvPV(*elem, na) : dflt;
}

static int
not_here(s)
char *s;
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant(name, arg)
char *name;
int arg;
{
    errno = 0;
    switch (*name) {
    case 'R':
	if (strEQ(name, "NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE"))
#ifdef NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE
	    return NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE;
#else
	    goto not_there;
#endif
	break;
    case 'S':
	if (strEQ(name, "SNMPERR_BAD_ADDRESS"))
#ifdef SNMPERR_BAD_ADDRESS
	    return SNMPERR_BAD_ADDRESS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMPERR_BAD_LOCPORT"))
#ifdef SNMPERR_BAD_LOCPORT
	    return SNMPERR_BAD_LOCPORT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMPERR_BAD_SESSION"))
#ifdef SNMPERR_BAD_SESSION
	    return SNMPERR_BAD_SESSION;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMPERR_GENERR"))
#ifdef SNMPERR_GENERR
	    return SNMPERR_GENERR;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMPERR_TOO_LONG"))
#ifdef SNMPERR_TOO_LONG
	    return SNMPERR_TOO_LONG;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_ADDRESS"))
#ifdef SNMP_DEFAULT_ADDRESS
	    return SNMP_DEFAULT_ADDRESS;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_COMMUNITY_LEN"))
#ifdef SNMP_DEFAULT_COMMUNITY_LEN
	    return SNMP_DEFAULT_COMMUNITY_LEN;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_ENTERPRISE_LENGTH"))
#ifdef SNMP_DEFAULT_ENTERPRISE_LENGTH
	    return SNMP_DEFAULT_ENTERPRISE_LENGTH;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_ERRINDEX"))
#ifdef SNMP_DEFAULT_ERRINDEX
	    return SNMP_DEFAULT_ERRINDEX;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_ERRSTAT"))
#ifdef SNMP_DEFAULT_ERRSTAT
	    return SNMP_DEFAULT_ERRSTAT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_PEERNAME"))
#ifdef SNMP_DEFAULT_PEERNAME
	    return 0;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_REMPORT"))
#ifdef SNMP_DEFAULT_REMPORT
	    return SNMP_DEFAULT_REMPORT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_REQID"))
#ifdef SNMP_DEFAULT_REQID
	    return SNMP_DEFAULT_REQID;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_RETRIES"))
#ifdef SNMP_DEFAULT_RETRIES
	    return SNMP_DEFAULT_RETRIES;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_TIME"))
#ifdef SNMP_DEFAULT_TIME
	    return SNMP_DEFAULT_TIME;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_TIMEOUT"))
#ifdef SNMP_DEFAULT_TIMEOUT
	    return SNMP_DEFAULT_TIMEOUT;
#else
	    goto not_there;
#endif
	if (strEQ(name, "SNMP_DEFAULT_VERSION"))
#ifdef DEFAULT_SNMP_VERSION
	    return DEFAULT_SNMP_VERSION;
#else
#ifdef SNMP_DEFAULT_VERSION
	    return SNMP_DEFAULT_VERSION;
#else
	    goto not_there;
#endif
#endif
	break;
    case 'T':
	if (strEQ(name, "NETSNMP_CALLBACK_OP_TIMED_OUT"))
#ifdef NETSNMP_CALLBACK_OP_TIMED_OUT
	    return NETSNMP_CALLBACK_OP_TIMED_OUT;
#else
	    goto not_there;
#endif
	break;
    default:
	break;
    }
    errno = EINVAL;
    return 0;

#ifndef NETSNMP_CALLBACK_OP_TIMED_OUT
not_there:
#endif
    errno = ENOENT;
    return 0;
}


MODULE = SNMP		PACKAGE = SNMP		PREFIX = snmp

BOOT:
# first blank line terminates bootstrap code
    Mib = 0;


double
constant(name,arg)
	char *		name
	int		arg

long
snmp_sys_uptime()
	CODE:
	RETVAL = get_uptime();
	OUTPUT:
	RETVAL

void
init_snmp(appname)
        char *appname
    CODE:
        __libraries_init(appname);


SnmpSession *
snmp_new_session(version, community, peer, lport, retries, timeout)
        char *	version
        char *	community
        char *	peer
        int	lport
        int	retries
        int	timeout
	CODE:
	{
	   SnmpSession session = {0};
	   SnmpSession *ss = NULL;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

           __libraries_init("perl");
           
	   if (!strcmp(version, "1")) {
		session.version = SNMP_VERSION_1;
           } else if ((!strcmp(version, "2")) || (!strcmp(version, "2c"))) {
		session.version = SNMP_VERSION_2c;
           } else if (!strcmp(version, "3")) {
	        session.version = SNMP_VERSION_3;
	   } else {
		if (verbose)
                   warn("error:snmp_new_session:Unsupported SNMP version (%s)\n", version);
                goto end;
	   }

           session.community_len = strlen((char *)community);
           session.community = (u_char *)community;
	   session.peername = peer;
	   session.local_port = lport;
           session.retries = retries; /* 5 */
           session.timeout = timeout; /* 1000000L */
           session.authenticator = NULL;

           ss = snmp_open(&session);

           if (ss == NULL) {
	      if (verbose) warn("error:snmp_new_session: Couldn't open SNMP session");
           }
        end:
           RETVAL = ss;
	}
        OUTPUT:
        RETVAL

SnmpSession *
snmp_new_v3_session(version, peer, retries, timeout, sec_name, sec_level, sec_eng_id, context_eng_id, context, auth_proto, auth_pass, priv_proto, priv_pass, eng_boots, eng_time)
        int	version
        char *	peer
        int	retries
        int	timeout
        char *  sec_name
        int     sec_level
        char *  sec_eng_id
        char *  context_eng_id
        char *  context
        char *  auth_proto
        char *  auth_pass
        char *  priv_proto
        char *  priv_pass
	int     eng_boots
	int     eng_time
	CODE:
	{
/*             u_char sec_eng_id_buf[ENG_ID_BUF_SIZE]; */
/*             u_char context_eng_id_buf[ENG_ID_BUF_SIZE]; */
	   SnmpSession session = {0};
	   SnmpSession *ss = NULL;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

           __libraries_init("perl");

	   if (version == 3) {
		session.version = SNMP_VERSION_3;
           } else {
		if (verbose)
                   warn("error:snmp_new_v3_session:Unsupported SNMP version (%d)\n", version);
                goto end;
	   }

	   session.peername = strdup(peer);
           session.retries = retries; /* 5 */
           session.timeout = timeout; /* 1000000L */
           session.authenticator = NULL;
           session.contextNameLen = strlen(context);
           session.contextName = context;
           session.securityNameLen = strlen(sec_name);
           session.securityName = sec_name;
           session.securityLevel = sec_level;
           session.securityModel = USM_SEC_MODEL_NUMBER;
           /* session.securityEngineID = sec_eng_id_buf;*/
           session.securityEngineIDLen =
              hex_to_binary2(sec_eng_id, strlen(sec_eng_id),
                             (char **) &session.securityEngineID);
           /* session.contextEngineID = context_eng_id_buf; */
           session.contextEngineIDLen =
              hex_to_binary2(sec_eng_id, strlen(sec_eng_id),
                             (char **) &session.contextEngineID);
           session.engineBoots = eng_boots;
           session.engineTime = eng_time;
           if (!strcmp(auth_proto, "MD5")) {
               session.securityAuthProto = 
                  snmp_duplicate_objid(usmHMACMD5AuthProtocol,
                                          USM_AUTH_PROTO_MD5_LEN);
              session.securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
           } else if (!strcmp(auth_proto, "SHA")) {
               session.securityAuthProto = 
                   snmp_duplicate_objid(usmHMACSHA1AuthProtocol,
                                        USM_AUTH_PROTO_SHA_LEN);
              session.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
           } else {
              if (verbose)
                 warn("error:snmp_new_v3_session:Unsupported authentication protocol(%s)\n", auth_proto);
              goto end;
           }
           if (session.securityLevel >= SNMP_SEC_LEVEL_AUTHNOPRIV) {
              session.securityAuthKeyLen = USM_AUTH_KU_LEN;
              if (generate_Ku(session.securityAuthProto,
                              session.securityAuthProtoLen,
                              (u_char *)auth_pass, strlen(auth_pass),
                              session.securityAuthKey,
                              &session.securityAuthKeyLen) != SNMPERR_SUCCESS) {
                 if (verbose)
                    warn("error:snmp_new_v3_session:Error generating Ku from authentication password.\n");
                 goto end;
              }
           }
           if (!strcmp(priv_proto, "DES")) {
              session.securityPrivProto =
                  snmp_duplicate_objid(usmDESPrivProtocol,
                                       USM_PRIV_PROTO_DES_LEN);
              session.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
           } else {
              if (verbose)
                 warn("error:snmp_new_v3_session:Unsupported privacy protocol(%s)\n", priv_proto);
              goto end;
           }
           if (session.securityLevel >= SNMP_SEC_LEVEL_AUTHPRIV) {
             session.securityPrivKeyLen = USM_PRIV_KU_LEN;
              if (generate_Ku(session.securityAuthProto,
                              session.securityAuthProtoLen,
                              (u_char *)priv_pass, strlen(priv_pass),
                              session.securityPrivKey,
                              &session.securityPrivKeyLen) != SNMPERR_SUCCESS) {
                 if (verbose)
                    warn("error:snmp_new_v3_session:Error generating Ku from privacy pass phrase.\n");
                 goto end;
               }
            }

           ss = snmp_open(&session);

           if (ss == NULL) {
	      if (verbose) warn("error:snmp_new_v3_session:Couldn't open SNMP session");
           }
        end:
           RETVAL = ss;
	   free (session.contextEngineID);
	}
        OUTPUT:
        RETVAL


SnmpSession *
snmp_update_session(sess_ref, version, community, peer, lport, retries, timeout)
        SV *	sess_ref
        char *	version
        char *	community
        char *	peer
        int	lport
        int	retries
        int	timeout
	CODE:
	{
           SV **sess_ptr_sv;
	   SnmpSession *ss;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

           sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
           ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));

           if (!ss) goto update_end;

           if (!strcmp(version, "1")) {
		ss->version = SNMP_VERSION_1;
           } else if (!strcmp(version, "2") || !strcmp(version, "2c")) {
		ss->version = SNMP_VERSION_2c;
	   } else if (!strcmp(version, "3")) {
	        ss->version = SNMP_VERSION_3;
	   } else {
		if (verbose)
                   warn("Unsupported SNMP version (%s)\n", version);
                goto update_end;
	   }
           /* WARNING LEAKAGE but I cant free lib memory under win32 */
           ss->community_len = strlen((char *)community);
           ss->community = (u_char *)strdup(community);
	   ss->peername = strdup(peer);
	   ss->local_port = lport;
           ss->retries = retries; /* 5 */
           ss->timeout = timeout; /* 1000000L */
           ss->authenticator = NULL;

    update_end:
	   RETVAL = ss;
        }
        OUTPUT:
           RETVAL

int
snmp_add_mib_dir(mib_dir,force=0)
	char *		mib_dir
	int		force
	CODE:
        {
	int result = 0;      /* Avoid use of uninitialized variable below. */
        int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

        if (mib_dir && *mib_dir) {
	   result = add_mibdir(mib_dir);
        }
        if (result) {
           if (verbose) warn("Added mib dir %s\n", mib_dir);
        } else {
           if (verbose) warn("Failed to add %s\n", mib_dir);
        }
        RETVAL = (I32)result;
        }
        OUTPUT:
        RETVAL

void
snmp_init_mib_internals()
	CODE:
        {
        int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

        /* should test better to see if it has been done already */
	if (Mib == NULL) {
           if (verbose) warn("initializing MIB internals (empty)\n");
           /* init_mib_internals(); */
        }
        }


int
snmp_read_mib(mib_file, force=0)
	char *		mib_file
	int		force
	CODE:
        {
        int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

        /* if (Mib && force) __free_tree(Mib); needs more work to cleanup */

        if ((mib_file == NULL) || (*mib_file == '\0')) {
           if (Mib == NULL) {
              if (verbose) warn("initializing MIB\n");
              /* init_mib_internals(); */
              init_mib();
              if (Mib) {
                 if (verbose) warn("done\n");
              } else {
                 if (verbose) warn("failed\n");
              }
	   }
        } else {
           if (verbose) warn("reading MIB: %s [%s:%s]\n", mib_file, DEFAULT_MIBDIRS, DEFAULT_MIBS);
           /* if (Mib == NULL) init_mib_internals();*/
           if (strcmp("ALL",mib_file))
              read_mib(mib_file);
           else
             read_all_mibs();
           if (Mib) {
              if (verbose) warn("done\n");
           } else {
              if (verbose) warn("failed\n");
           }
        }
        RETVAL = (I32)Mib;
        }
        OUTPUT:
        RETVAL


int
snmp_read_module(module)
	char *		module
	CODE:
        {
        int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

        if (!strcmp(module,"ALL")) {
           read_all_mibs();
        } else {
           read_module(module);
        }
        if (Mib) {
           if (verbose) warn("Read %s\n", module);
        } else {
           if (verbose) warn("Failed reading %s\n", module);
        }
        RETVAL = (I32)Mib;
        }
        OUTPUT:
        RETVAL


int
snmp_set(sess_ref, varlist_ref, perl_callback)
        SV *	sess_ref
        SV *	varlist_ref
        SV *	perl_callback
	PPCODE:
	{
           AV *varlist;
           SV **varbind_ref;
           SV **varbind_val_f;
           AV *varbind;
	   I32 varlist_len;
	   I32 varlist_ind;
           SnmpSession *ss;
           netsnmp_pdu *pdu, *response;
           struct tree *tp;
	   oid *oid_arr;
	   int oid_arr_len = MAX_OID_LEN;
           char *tag_pv;
           snmp_xs_cb_data *xs_cb_data;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int status = 0;
           int type;
	   int res;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
           int use_enums;
           struct enum_list *ep;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref) && SvROK(varlist_ref)) {

	      use_enums = SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums",8,1));
              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);

              pdu = snmp_pdu_create(SNMP_MSG_SET);

              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
	      for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);
                    tag_pv = __av_elem_pv(varbind, VARBIND_TAG_F,NULL);
                    tp=__tag2oid(tag_pv,
                                 __av_elem_pv(varbind, VARBIND_IID_F,NULL),
                                 oid_arr, &oid_arr_len, &type,0);

                    if (oid_arr_len==0) {
                       if (verbose)
                          warn("error: set: unknown object ID (%s)",
                                (tag_pv?tag_pv:"<null>"));
	               sv_catpv(*err_str_svp,
                               (char*)snmp_api_errstring(SNMPERR_UNKNOWN_OBJID));
                       sv_setiv(*err_num_svp, SNMPERR_UNKNOWN_OBJID);
                       XPUSHs(&sv_undef); /* unknown OID */
		       snmp_free_pdu(pdu);
		       goto done;
		    }


                    if (type == TYPE_UNKNOWN) {
                      type = __translate_appl_type(
                                __av_elem_pv(varbind, VARBIND_TYPE_F, NULL));
                      if (type == TYPE_UNKNOWN) {
                         if (verbose)
                            warn("error: set: no type found for object");
	                 sv_catpv(*err_str_svp,
                                  (char*)snmp_api_errstring(SNMPERR_VAR_TYPE));
                         sv_setiv(*err_num_svp, SNMPERR_VAR_TYPE);
                         XPUSHs(&sv_undef); /* unknown OID */
		         snmp_free_pdu(pdu);
		         goto done;
                      }
                    }

	            varbind_val_f = av_fetch(varbind, VARBIND_VAL_F, 0);

                    if (type==TYPE_INTEGER && use_enums && tp && tp->enums) {
                      for(ep = tp->enums; ep; ep = ep->next) {
                        if (varbind_val_f && SvOK(*varbind_val_f) &&
                            !strcmp(ep->label, SvPV(*varbind_val_f,na))) {
                          sv_setiv(*varbind_val_f, ep->value);
                          break;
                        }
                      }
                    }

                    res = __add_var_val_str(pdu, oid_arr, oid_arr_len,
				     (varbind_val_f && SvOK(*varbind_val_f) ?
				      SvPV(*varbind_val_f,na):NULL),
				      (varbind_val_f && SvOK(*varbind_val_f) ?
				       SvCUR(*varbind_val_f):0), type);

		    if (verbose && res == FAILURE)
		      warn("error: adding variable/value to PDU");
                 } /* if var_ref is ok */
              } /* for all the vars */

              if (SvTRUE(perl_callback)) {
                  xs_cb_data =
                      (snmp_xs_cb_data*)malloc(sizeof(snmp_xs_cb_data));
                 xs_cb_data->perl_cb = newSVsv(perl_callback);
                 xs_cb_data->sess_ref = newRV_inc(SvRV(sess_ref));

                 status = snmp_async_send(ss, pdu, __snmp_xs_cb,
                                          (void*)xs_cb_data);
                 if (status != 0) {
                    XPUSHs(sv_2mortal(newSViv(status))); /* push the reqid?? */
                 } else {
                    snmp_free_pdu(pdu);
                    sv_catpv(*err_str_svp,
                             (char*)snmp_api_errstring(ss->s_snmp_errno));
                    sv_setiv(*err_num_svp, ss->s_snmp_errno);
                    XPUSHs(&sv_undef);
                 }
		 goto done;
              }

	      status = __send_sync_pdu(ss, pdu, &response,
				       NO_RETRY_NOSUCH,
                                       *err_str_svp, *err_num_svp,
                                       *err_ind_svp);

              if (response) snmp_free_pdu(response);

              if (status) {
		 XPUSHs(&sv_undef);
	      } else {
                 XPUSHs(sv_2mortal(newSVpv(ZERO_BUT_TRUE,0)));
              }
           } else {

              /* BUG!!! need to return an error value */
              XPUSHs(&sv_undef); /* no mem or bad args */
           }
done:
           Safefree(oid_arr);
        }

void
snmp_catch(sess_ref, perl_callback)
	SV *	sess_ref
        SV *    perl_callback
	PPCODE:
	{
	   netsnmp_session *ss;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;

           if (SvROK(sess_ref)) {
              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);

              snmp_synch_reset(ss);
              ss->callback = NULL;
              ss->callback_magic = NULL;

              if (SvTRUE(perl_callback)) {
                 perl_callback = newSVsv(perl_callback);
                 # it might be more efficient to pass the varbind_ref to
                 # __snmp_xs_cb as part of perl_callback so it is not freed
                 # and reconstructed for each call
                 ss->callback = __callback_wrapper;
                 ss->callback_magic = perl_callback;
                 sv_2mortal(newSViv(1));
                 goto done;
              }
           }
           sv_2mortal(newSViv(0));
        done:
           ;
        }

void
snmp_get(sess_ref, retry_nosuch, varlist_ref, perl_callback)
        SV *    sess_ref
        int     retry_nosuch
        SV *    varlist_ref
        SV *    perl_callback
        PPCODE:
        {
           AV *varlist;
           SV **varbind_ref;
           AV *varbind;
           I32 varlist_len;
           I32 varlist_ind;
           netsnmp_session *ss;
           netsnmp_pdu *pdu, *response;
           netsnmp_variable_list *vars;
           netsnmp_variable_list *last_vars;
           struct tree *tp;
           int len;
           oid *oid_arr = NULL;
           int oid_arr_len = MAX_OID_LEN;
           SV *tmp_sv;
           char *tag_pv;
           int type;
           char tmp_type_str[MAX_TYPE_NAME_LEN];
           snmp_xs_cb_data *xs_cb_data;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int status;
	   u_char str_buf[STR_BUF_SIZE], *str_bufp = str_buf;
           size_t str_buf_len = sizeof(str_buf);
           size_t out_len = 0;
           int buf_over = 0;
           char *label;
           char *iid;
           int getlabel_flag = NO_FLAGS;
           int sprintval_flag = USE_BASIC;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
	   int old_format;
	   SV *sv_timestamp = NULL;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref) && SvROK(varlist_ref)) {

              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
              ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1)))
                 getlabel_flag |= USE_LONG_NAMES;
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1)))
                 getlabel_flag |= USE_NUMERIC_OIDS;
              if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums", 8, 1)))
                 sprintval_flag = USE_ENUMS;
              if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseSprintValue", 14, 1)))
                 sprintval_flag = USE_SPRINT_VALUE;

              pdu = snmp_pdu_create(SNMP_MSG_GET);

              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
              for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);
                    tag_pv = __av_elem_pv(varbind, VARBIND_TAG_F,NULL);
                    tp = __tag2oid(tag_pv,
                                   __av_elem_pv(varbind, VARBIND_IID_F,NULL),
                                   oid_arr, &oid_arr_len, NULL,0);

                    if (oid_arr_len) {
                       snmp_add_null_var(pdu, oid_arr, oid_arr_len);
                    } else {
                       if (verbose)
                          warn("error: get: unknown object ID (%s)",
                                (tag_pv?tag_pv:"<null>"));
	               sv_catpv(*err_str_svp,
                                (char*)snmp_api_errstring(SNMPERR_UNKNOWN_OBJID));
                       sv_setiv(*err_num_svp, SNMPERR_UNKNOWN_OBJID);
                       XPUSHs(&sv_undef); /* unknown OID */
		       snmp_free_pdu(pdu);
		       goto done;
                    }
                 } /* if var_ref is ok */
              } /* for all the vars */

              if (perl_callback && SvTRUE(perl_callback)) {
                  xs_cb_data =
                      (snmp_xs_cb_data*)malloc(sizeof(snmp_xs_cb_data));
                 xs_cb_data->perl_cb = newSVsv(perl_callback);
                 xs_cb_data->sess_ref = newSVsv(sess_ref);

                 status = snmp_async_send(ss, pdu, __snmp_xs_cb,
                                          (void*)xs_cb_data);
                 if (status != 0) {
                    XPUSHs(sv_2mortal(newSViv(status))); /* push the reqid?? */
                 } else {
                    snmp_free_pdu(pdu);
                    sv_catpv(*err_str_svp,
                             (char*)snmp_api_errstring(ss->s_snmp_errno));
                    sv_setiv(*err_num_svp, ss->s_snmp_errno);
                    XPUSHs(&sv_undef);
                 }
                 goto done;
              }

              status = __send_sync_pdu(ss, pdu, &response, retry_nosuch,
                                       *err_str_svp,*err_num_svp,*err_ind_svp);

              last_vars = (response ? response->variables : NULL);

	      /*
	      ** Set up for numeric or full OID's, if necessary.  Save the old
	      ** output format so that it can be restored when we finish -- this
	      ** is a library-wide global, and has to be set/restored for each
	      ** session.
	      */
	      old_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);

	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1))) {
	         getlabel_flag |= USE_LONG_NAMES;

	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    NETSNMP_OID_OUTPUT_FULL);
	      }
              /* Setting UseNumeric forces UseLongNames on so check for UseNumeric
                 after UseLongNames (above) to make sure the final outcome of 
                 NETSNMP_DS_LIB_OID_OUTPUT_FORMAT is NETSNMP_OID_OUTPUT_NUMERIC */
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1))) {
	         getlabel_flag |= USE_LONG_NAMES;
	         getlabel_flag |= USE_NUMERIC_OIDS;

	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    NETSNMP_OID_OUTPUT_NUMERIC);
	      }

	      if (SvIOK(*hv_fetch((HV*)SvRV(sess_ref),"TimeStamp", 9, 1)) &&
                  SvIV(*hv_fetch((HV*)SvRV(sess_ref),"TimeStamp", 9, 1)))
	         sv_timestamp = newSViv((IV)time(NULL));

              for(vars = (response?response->variables:NULL), varlist_ind = 0;
                  vars && (varlist_ind <= varlist_len);
                  vars = vars->next_variable, varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);

                    *str_buf = '.';
                    *(str_buf+1) = '\0';
                    out_len = 0;
                    tp = netsnmp_sprint_realloc_objid_tree(&str_bufp, &str_buf_len,
                                                           &out_len, 0, &buf_over,
                                                           vars->name,vars->name_length);
                    str_buf[sizeof(str_buf)-1] = '\0';

                    if (__is_leaf(tp)) {
                       type = tp->type;
                    } else {
                       getlabel_flag |= NON_LEAF_NAME;
                       type = __translate_asn_type(vars->type);
                    }
                    __get_label_iid(str_buf,&label,&iid,getlabel_flag);
                    if (label) {
                        av_store(varbind, VARBIND_TAG_F,
                                 newSVpv(label, strlen(label)));
                    } else {
                        av_store(varbind, VARBIND_TAG_F,
                                 newSVpv("", 0));
                    }
                    if (iid) {
                        av_store(varbind, VARBIND_IID_F,
                                 newSVpv(iid, strlen(iid)));
                    } else {
                        av_store(varbind, VARBIND_IID_F,
                                 newSVpv("", 0));
                    }                        
                    __get_type_str(type, tmp_type_str);
                    tmp_sv = newSVpv(tmp_type_str, strlen(tmp_type_str));
                    av_store(varbind, VARBIND_TYPE_F, tmp_sv);
                    len=__snprint_value(str_buf,sizeof(str_buf),
                                       vars,tp,type,sprintval_flag);
                    tmp_sv = newSVpv((char*)str_buf, len);
                    av_store(varbind, VARBIND_VAL_F, tmp_sv);
		    if (sv_timestamp)
                       av_store(varbind, VARBIND_TYPE_F, sv_timestamp);
                    XPUSHs(sv_mortalcopy(tmp_sv));
                 } else {
		    /* Return undef for this variable. */
                    XPUSHs(&sv_undef);
                 }
              }

	      /* Reset the library's behavior for numeric/symbolic OID's. */
	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    old_format);

              if (response) snmp_free_pdu(response);
           } else {
              XPUSHs(&sv_undef); /* no mem or bad args */
           }
     done:
           Safefree(oid_arr);
        }

int
snmp_getnext(sess_ref, varlist_ref, perl_callback)
        SV *    sess_ref
        SV *    varlist_ref
        SV *    perl_callback
        PPCODE:
        {
           AV *varlist;
           SV **varbind_ref;
           AV *varbind;
           I32 varlist_len;
           I32 varlist_ind;
           netsnmp_session *ss;
           netsnmp_pdu *pdu, *response;
           netsnmp_variable_list *vars;
           struct tree *tp;
           int len;
	   oid *oid_arr;
	   int oid_arr_len = MAX_OID_LEN;
           SV *tmp_sv;
           int type;
	   char tmp_type_str[MAX_TYPE_NAME_LEN];
           snmp_xs_cb_data *xs_cb_data;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int status;
	   u_char str_buf[STR_BUF_SIZE], *str_bufp = str_buf;
           size_t str_buf_len = sizeof(str_buf);
           size_t out_len = 0;
           int buf_over = 0;
           char *label;
           char *iid;
           int getlabel_flag = NO_FLAGS;
           int sprintval_flag = USE_BASIC;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
	   int old_format;
	   SV *sv_timestamp = NULL;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref) && SvROK(varlist_ref)) {

              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1)))
                 getlabel_flag |= USE_LONG_NAMES;
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums", 8, 1)))
                 sprintval_flag = USE_ENUMS;
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseSprintValue", 14, 1)))
                 sprintval_flag = USE_SPRINT_VALUE;

              pdu = snmp_pdu_create(SNMP_MSG_GETNEXT);

              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
	      for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);

                    tp = __tag2oid(__av_elem_pv(varbind, VARBIND_TAG_F, ".0"),
                              __av_elem_pv(varbind, VARBIND_IID_F, NULL),
                              oid_arr, &oid_arr_len, NULL,0);

      		    if (oid_arr_len) {
  		       snmp_add_null_var(pdu, oid_arr, oid_arr_len);
		    } else {
                       if (verbose)
                          warn("error: set: unknown object ID");
	               sv_catpv(*err_str_svp,
                               (char*)snmp_api_errstring(SNMPERR_UNKNOWN_OBJID));
                       sv_setiv(*err_num_svp, SNMPERR_UNKNOWN_OBJID);
                       XPUSHs(&sv_undef); /* unknown OID */
		       snmp_free_pdu(pdu);
		       goto done;
		    }

                 } /* if var_ref is ok */
              } /* for all the vars */

              if (SvTRUE(perl_callback)) {
                  xs_cb_data =
                      (snmp_xs_cb_data*)malloc(sizeof(snmp_xs_cb_data));
                 xs_cb_data->perl_cb = newSVsv(perl_callback);
                 xs_cb_data->sess_ref = newSVsv(sess_ref);

                 status = snmp_async_send(ss, pdu, __snmp_xs_cb,
                                          (void*)xs_cb_data);
                 if (status != 0) {
                    XPUSHs(sv_2mortal(newSViv(status))); /* push the reqid?? */
                 } else {
                    snmp_free_pdu(pdu);
                    sv_catpv(*err_str_svp,
                             (char*)snmp_api_errstring(ss->s_snmp_errno));
                    sv_setiv(*err_num_svp, ss->s_snmp_errno);
                    XPUSHs(&sv_undef);
                 }
		 goto done;
              }

	      status = __send_sync_pdu(ss, pdu, &response,
				       NO_RETRY_NOSUCH,
                                       *err_str_svp, *err_num_svp,
				       *err_ind_svp);

	      /*
	      ** Set up for numeric OID's, if necessary.  Save the old values
	      ** so that they can be restored when we finish -- these are
	      ** library-wide globals, and have to be set/restored for each
	      ** session.
	      */
	      old_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);

	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1))) {
	         getlabel_flag |= USE_LONG_NAMES;

	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    NETSNMP_OID_OUTPUT_FULL);
	      }
              /* Setting UseNumeric forces UseLongNames on so check
                 for UseNumeric after UseLongNames (above) to make
                 sure the final outcome of
                 NETSNMP_DS_LIB_OID_OUTPUT_FORMAT is
                 NETSNMP_OID_OUTPUT_NUMERIC */
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1))) {
	         getlabel_flag |= USE_LONG_NAMES;
	         getlabel_flag |= USE_NUMERIC_OIDS;

	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    NETSNMP_OID_OUTPUT_NUMERIC);
	      }

	      if (SvIOK(*hv_fetch((HV*)SvRV(sess_ref),"TimeStamp", 9, 1)) &&
                  SvIV(*hv_fetch((HV*)SvRV(sess_ref),"TimeStamp", 9, 1)))
	         sv_timestamp = newSViv((IV)time(NULL));

              for(vars = (response?response->variables:NULL), varlist_ind = 0;
                  vars && (varlist_ind <= varlist_len);
                  vars = vars->next_variable, varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);

                    *str_buf = '.';
                    *(str_buf+1) = '\0';
                    out_len = 0;
                    tp = netsnmp_sprint_realloc_objid_tree(&str_bufp, &str_buf_len,
                                                           &out_len, 0, &buf_over,
                                                           vars->name,vars->name_length);
                    str_buf[sizeof(str_buf)-1] = '\0';

                    if (__is_leaf(tp)) {
                       type = tp->type;
                    } else {
                       getlabel_flag |= NON_LEAF_NAME;
                       type = __translate_asn_type(vars->type);
                    }
                    __get_label_iid(str_buf,&label,&iid,getlabel_flag);
                    if (label) {
                        av_store(varbind, VARBIND_TAG_F,
                                 newSVpv(label, strlen(label)));
                    } else {
                        av_store(varbind, VARBIND_TAG_F,
                                 newSVpv("", 0));
                    }
                    if (iid) {
                        av_store(varbind, VARBIND_IID_F,
                                 newSVpv(iid, strlen(iid)));
                    } else {
                        av_store(varbind, VARBIND_IID_F,
                                 newSVpv("", 0));
                    }                        
                    __get_type_str(type, tmp_type_str);
                    tmp_sv = newSVpv(tmp_type_str, strlen(tmp_type_str));
                    av_store(varbind, VARBIND_TYPE_F, tmp_sv);
                    len=__snprint_value(str_buf,sizeof(str_buf),
                                       vars,tp,type,sprintval_flag);
                    tmp_sv = newSVpv((char*)str_buf, len);
                    av_store(varbind, VARBIND_VAL_F, tmp_sv);
		    if (sv_timestamp)
                       av_store(varbind, VARBIND_TYPE_F, sv_timestamp);
                    XPUSHs(sv_mortalcopy(tmp_sv));
                 } else {
		    /* Return undef for this variable. */
                    XPUSHs(&sv_undef);
                 }
              }

	      /* Reset the library's behavior for numeric/symbolic OID's. */
              netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                 NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                 old_format);

              if (response) snmp_free_pdu(response);

           } else {
              XPUSHs(&sv_undef); /* no mem or bad args */
	   }
done:
	Safefree(oid_arr);
	}

int
snmp_getbulk(sess_ref, nonrepeaters, maxrepetitions, varlist_ref, perl_callback)
        SV *	sess_ref
	int nonrepeaters
	int maxrepetitions
        SV *	varlist_ref
        SV *	perl_callback
	PPCODE:
	{
           AV *varlist;
           SV **varbind_ref;
           AV *varbind;
	   I32 varlist_len;
	   I32 varlist_ind;
           netsnmp_session *ss;
           netsnmp_pdu *pdu, *response;
           netsnmp_variable_list *vars;
           struct tree *tp;
           int len;
	   oid *oid_arr;
	   int oid_arr_len = MAX_OID_LEN;
           SV *tmp_sv;
           int type;
	   char tmp_type_str[MAX_TYPE_NAME_LEN];
           snmp_xs_cb_data *xs_cb_data;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int status;
	   u_char str_buf[STR_BUF_SIZE], *str_bufp = str_buf;
           size_t str_buf_len = sizeof(str_buf);
           size_t out_len = 0;
           int buf_over = 0;
           char *label;
           char *iid;
           int getlabel_flag = NO_FLAGS;
           int sprintval_flag = USE_BASIC;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
	   int old_format;
	   SV *rv;
	   SV *sv_timestamp = NULL;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref) && SvROK(varlist_ref)) {

              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1)))
                 getlabel_flag |= USE_LONG_NAMES;
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1)))
		 getlabel_flag |= USE_NUMERIC_OIDS;
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums", 8, 1)))
                 sprintval_flag = USE_ENUMS;
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseSprintValue", 14, 1)))
                 sprintval_flag = USE_SPRINT_VALUE;

              pdu = snmp_pdu_create(SNMP_MSG_GETBULK);

	      pdu->errstat = nonrepeaters;
	      pdu->errindex = maxrepetitions;

              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
	      for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);
                    __tag2oid(__av_elem_pv(varbind, VARBIND_TAG_F, "0"),
                              __av_elem_pv(varbind, VARBIND_IID_F, NULL),
                              oid_arr, &oid_arr_len, NULL,0);


                    if (oid_arr_len) {
  		       snmp_add_null_var(pdu, oid_arr, oid_arr_len);
		    } else {
                       if (verbose)
                          warn("error: set: unknown object ID");
	               sv_catpv(*err_str_svp,
                               (char*)snmp_api_errstring(SNMPERR_UNKNOWN_OBJID));
                       sv_setiv(*err_num_svp, SNMPERR_UNKNOWN_OBJID);
                       XPUSHs(&sv_undef); /* unknown OID */
		       snmp_free_pdu(pdu);
		       goto done;
		    }


                 } /* if var_ref is ok */
              } /* for all the vars */

              if (SvTRUE(perl_callback)) {
                  xs_cb_data =
                      (snmp_xs_cb_data*)malloc(sizeof(snmp_xs_cb_data));
                 xs_cb_data->perl_cb = newSVsv(perl_callback);
                 xs_cb_data->sess_ref = newSVsv(sess_ref);

                 status = snmp_async_send(ss, pdu, __snmp_xs_cb,
                                          (void*)xs_cb_data);
                 if (status != 0) {
                    XPUSHs(sv_2mortal(newSViv(status))); /* push the reqid?? */
                 } else {
                    snmp_free_pdu(pdu);
                    sv_catpv(*err_str_svp,
                             (char*)snmp_api_errstring(ss->s_snmp_errno));
                    sv_setiv(*err_num_svp, ss->s_snmp_errno);
                    XPUSHs(&sv_undef);
                 }
		 goto done;
              }

	      status = __send_sync_pdu(ss, pdu, &response,
				       NO_RETRY_NOSUCH,
                                       *err_str_svp, *err_num_svp,
				       *err_ind_svp);

	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"TimeStamp", 9, 1)))
	         sv_timestamp = newSViv((IV)time(NULL));

	      av_clear(varlist);

	      /*
	      ** Set up for numeric or full OID's, if necessary.  Save the old
	      ** output format so that it can be restored when we finish -- this
	      ** is a library-wide global, and has to be set/restored for each
	      ** session.
	      */
	      old_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID,
                                              NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);

	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1))) {
	         getlabel_flag |= USE_LONG_NAMES;

	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    NETSNMP_OID_OUTPUT_FULL);
	      }
              /* Setting UseNumeric forces UseLongNames on so check
                 for UseNumeric after UseLongNames (above) to make
                 sure the final outcome of
                 NETSNMP_DS_LIB_OID_OUTPUT_FORMAT is
                 NETSNMP_OID_OUTPUT_NUMERIC */
	      if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1))) {
	         getlabel_flag |= USE_LONG_NAMES;
	         getlabel_flag |= USE_NUMERIC_OIDS;

	         netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                    NETSNMP_OID_OUTPUT_NUMERIC);
	      }

	      if(response && response->variables) {
              for(vars = response->variables;
                  vars;
                  vars = vars->next_variable) {

                    varbind = (AV*) newAV();
                    *str_buf = '.';
                    *(str_buf+1) = '\0';
                    out_len = 0;
                    tp = netsnmp_sprint_realloc_objid_tree(&str_bufp, &str_buf_len,
                                                           &out_len, 0, &buf_over,
                                                           vars->name,vars->name_length);
                    str_buf[sizeof(str_buf)-1] = '\0';
                    if (__is_leaf(tp)) {
                       type = tp->type;
                    } else {
                       getlabel_flag |= NON_LEAF_NAME;
                       type = __translate_asn_type(vars->type);
                    }
                    __get_label_iid(str_buf,&label,&iid,getlabel_flag);

		    av_store(varbind, VARBIND_TAG_F,
			     newSVpv(label, strlen(label)));
		    av_store(varbind, VARBIND_IID_F,
			     newSVpv(iid, strlen(iid)));

                    __get_type_str(type, tmp_type_str);
		    av_store(varbind, VARBIND_TYPE_F, newSVpv(tmp_type_str,
				     strlen(tmp_type_str)));

                    len=__snprint_value(str_buf,sizeof(str_buf),
                                       vars,tp,type,sprintval_flag);
                    tmp_sv = newSVpv((char*)str_buf, len);
		    av_store(varbind, VARBIND_VAL_F, tmp_sv);
		    if (sv_timestamp)
		       av_store(varbind, VARBIND_TYPE_F, SvREFCNT_inc(sv_timestamp));

		    rv = newRV_noinc((SV *)varbind);
		    sv_bless(rv, gv_stashpv("SNMP::Varbind",0));
		    av_push(varlist, rv);

                    XPUSHs(sv_mortalcopy(tmp_sv));
                 }
              } else {
                    XPUSHs(&sv_undef);
	      }

	      /* Reset the library's behavior for numeric/symbolic OID's. */
              netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID,
                                 NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                 old_format);

              if (response) snmp_free_pdu(response);

           } else {
              XPUSHs(&sv_undef); /* no mem or bad args */
	   }
done:
	Safefree(oid_arr);
	}

int
snmp_bulkwalk(sess_ref, nonrepeaters, maxrepetitions, varlist_ref,perl_callback)
        SV *	sess_ref
	int nonrepeaters
	int maxrepetitions
        SV *	varlist_ref
        SV *	perl_callback
	PPCODE:
	{
           AV *varlist;
           SV **varbind_ref;
           AV *varbind;
	   I32 varlist_len;
	   I32 varlist_ind;
           netsnmp_session *ss;
           netsnmp_pdu *pdu = NULL;
	   oid oid_arr[MAX_OID_LEN];
	   int oid_arr_len;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
	   char str_buf[STR_BUF_SIZE];
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
	   walk_context *context = NULL;	/* Context for this bulkwalk */
	   bulktbl *bt_entry;			/* Current bulktbl/OID entry */
	   int i;				/* General purpose iterator  */
	   int npushed;				/* Number of return arrays   */
	   int okay;				/* Did bulkwalk complete okay */

           if (!SvROK(sess_ref) || !SvROK(varlist_ref)) {
	      if (verbose)
		 warn("Bad session or varlist reference!\n");

	      XSRETURN_UNDEF;
	   }

	   sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	   ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
	   err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
	   err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
	   err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
	   sv_setpv(*err_str_svp, "");
	   sv_setiv(*err_num_svp, 0);
	   sv_setiv(*err_ind_svp, 0);

	   /* Create and initialize a new session context for this bulkwalk.
	   ** This will be used to carry state between callbacks.
	   */
	   Newz(0x57616b6c /* "Walk" */, context, 1, walk_context);
	   if (context == NULL) {
	      sprintf(str_buf, "malloc(context) failed (%s)", strerror(errno));
	      sv_setpv(*err_str_svp, str_buf);
	      sv_setiv(*err_num_svp, SNMPERR_MALLOC);
	      goto err;
	   }

	   /* Store the Perl callback and session reference in the context. */
	   context->perl_cb  = newSVsv(perl_callback);
	   context->sess_ref = newSVsv(sess_ref);

	   DBPRT(3,("bulkwalk: sess_ref = 0x%p, sess_ptr_sv = 0x%p, ss = 0x%p\n",
						    sess_ref, sess_ptr_sv, ss));

           context->getlabel_f  = NO_FLAGS;	/* long/numeric name flags */
           context->sprintval_f = USE_BASIC;	/* Don't do fancy printing */
	   context->req_oids    = NULL;		/* List of oid's requested */
	   context->repbase     = NULL;		/* Repeaters in req_oids[] */
	   context->reqbase     = NULL;		/* Ptr to start of requests */
	   context->nreq_oids   = 0;		/* Number of oid's in list */
	   context->repeaters   = 0;		/* Repeater count (see below) */
	   context->non_reps    = nonrepeaters;	/* Non-repeater var count */
	   context->max_reps    = maxrepetitions; /* Max repetition/var count */
	   context->pkts_exch   = 0;		/* Packets exchanged in walk */
	   context->oid_total   = 0;		/* OID's received during walk */
	   context->oid_saved   = 0;		/* OID's saved as results */

	   if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseLongNames", 12, 1)))
	      context->getlabel_f |= USE_LONG_NAMES;
	   if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseNumeric", 10, 1)))
	      context->getlabel_f |= USE_NUMERIC_OIDS;
	   if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums", 8, 1)))
	      context->sprintval_f = USE_ENUMS;
	   if (SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseSprintValue", 14, 1)))
	      context->sprintval_f = USE_SPRINT_VALUE;

	   /* Set up an array of bulktbl's to hold the original list of
	   ** requested OID's.  This is used to populate the PDU's with
	   ** oid values, to contain/sort the return values, and (through
	   ** last_oid/last_len) to determine when the bulkwalk for each
	   ** variable has completed.
	   */
	   varlist = (AV*) SvRV(varlist_ref);
	   varlist_len = av_len(varlist) + 1;	/* XXX av_len returns index of
						** last element not #elements */

	   Newz(0, context->req_oids, varlist_len, bulktbl);

	   if (context->req_oids == NULL) {
	      sprintf(str_buf, "Newz(req_oids) failed (%s)", strerror(errno));
	      if (verbose)
	         warn(str_buf);
	      sv_setpv(*err_str_svp, str_buf);
	      sv_setiv(*err_num_svp, SNMPERR_MALLOC);
	      goto err;
	   }

	   /* Walk through the varbind_list, parsing and copying each OID
	   ** into a bulktbl slot in the req_oids array.  Bail if there's
	   ** some error.  Create the initial packet to send out, which
	   ** includes the non-repeaters.
	   */
	   DBPRT(1,( "Building request table:\n"));
	   for (varlist_ind = 0; varlist_ind < varlist_len; varlist_ind++) {
	      /* Get a handle on this entry in the request table. */
	      bt_entry = &context->req_oids[context->nreq_oids];

	      DBPRT(1,( "  request %d: ", (int)varlist_ind));

	      /* Get the request varbind from the varlist, parse it out to
	      ** tag and index, and copy it to the req_oid[] array slots.
	      */
	      varbind_ref = av_fetch(varlist, varlist_ind, 0);
	      if (!SvROK(*varbind_ref)) {
		 sv_setpv(*err_str_svp, \
		       (char*)snmp_api_errstring(SNMPERR_BAD_NAME));
		 sv_setiv(*err_num_svp, SNMPERR_BAD_NAME);
		 goto err;
	      }

	      varbind = (AV*) SvRV(*varbind_ref);
	      __tag2oid(__av_elem_pv(varbind, VARBIND_TAG_F, "0"),
			__av_elem_pv(varbind, VARBIND_IID_F, NULL),
			oid_arr, &oid_arr_len, NULL, 0);

	      if ((oid_arr_len == 0) || (oid_arr_len > MAX_OID_LEN)) {
		 if (verbose)
		    warn("error: bulkwalk(): unknown object ID");
		 sv_setpv(*err_str_svp, \
		       (char*)snmp_api_errstring(SNMPERR_UNKNOWN_OBJID));
		 sv_setiv(*err_num_svp, SNMPERR_UNKNOWN_OBJID);
		 goto err;
	      }

	      /* Copy the now-parsed OID into the first available slot
	      ** in the req_oids[] array.  Set both the req_oid (original
	      ** request) and the last_oid (last requested/seen oid) to
	      ** the initial value.  We build packets using last_oid (see
	      ** below), so initialize last_oid to the initial request.
	      */
	      Copy((void *)oid_arr, (void *)bt_entry->req_oid,
							oid_arr_len, oid);
	      Copy((void *)oid_arr, (void *)bt_entry->last_oid,
							oid_arr_len, oid);

	      bt_entry->req_len  = oid_arr_len;
	      bt_entry->last_len = oid_arr_len;

	      /* Adjust offset to and count of repeaters.  Note non-repeater
	      ** OID's in the list, if appropriate.
	      */
	      if (varlist_ind >= context->non_reps) {

		 /* Store a pointer to the first repeater value. */
		 if (context->repbase == NULL)
		    context->repbase = bt_entry;

		 context->repeaters ++;

	      } else {
		 bt_entry->norepeat = 1;
		 DBPRT(1,( "(nonrepeater) "));
	      }

	      /* Initialize the array in which to hold the Varbinds to be
	      ** returned for the OID or subtree.
	      */
	      if ((bt_entry->vars = (AV*) newAV()) == NULL) {
		 sv_setpv(*err_str_svp, "newAV() failed: ");
		 sv_catpv(*err_str_svp, strerror(errno));
		 sv_setiv(*err_num_svp, SNMPERR_MALLOC);
		 goto err;
	      }

	      DBPRT(1,( "%s\n", snprint_objid(_debugx, sizeof(_debugx), oid_arr, oid_arr_len)));

	      context->nreq_oids ++;
	   }

	   /* Keep track of the number of outstanding requests.  This lets us
	   ** finish processing early if we're done with all requests.
	   */
	   context->req_remain = context->nreq_oids;
	   DBPRT(1,( "Total %d variable requests added\n", context->nreq_oids));

	   /* If no good variable requests were found, return an error. */
	   if (context->nreq_oids == 0) {
		 sv_setpv(*err_str_svp, "No variables found in varlist");
		 sv_setiv(*err_num_svp, SNMPERR_NO_VARS);
		 goto err;
	   }

	   /* Note that this is a good context.  This allows later callbacks
	   ** to ignore re-sent PDU's that correspond to completed (and hence
	   ** destroyed) bulkwalk contexts.
	   */
	   _context_add(context);

	   /* For asynchronous bulkwalk requests, all we have to do at this
	   ** point is enqueue the asynchronous GETBULK request with our
	   ** bulkwalk-specific callback and return.  Remember that the
	   ** bulkwalk_send_pdu() function returns the reqid cast to an
	   ** snmp_pdu pointer, or NULL on failure.  Return undef if the
	   ** initial send fails; bulkwalk_send_pdu() takes care of setting
	   ** the various error values.
	   **
	   ** From here, the callbacks do all the work, including sending
	   ** requests for variables and handling responses.  The caller's
	   ** callback will be invoked as soon as the walk completes.
	   */
	   if (SvTRUE(perl_callback)) {
	      DBPRT(1,( "Starting asynchronous bulkwalk...\n"));

	      pdu = _bulkwalk_send_pdu(context);

	      if (pdu == NULL) {
		 DBPRT(1,( "Initial asynchronous send failed...\n"));
		 XSRETURN_UNDEF;
	      }

	      /* Sent okay...  Return the request ID in 'pdu' as an SvIV. */
	      DBPRT(1,( "Okay, request id is %d\n", (int)pdu));
/*	      XSRETURN_IV((int)pdu); */
	      XPUSHs(sv_2mortal(newSViv((int)pdu)));
	      XSRETURN(1);
	   }

	   /* For synchronous bulkwalk, we perform the basic send/receive
	   ** iteration right here.  Once the walk has been completed, the
	   ** bulkwalk_finish() function will push the return values onto
	   ** the Perl call stack, and we return.
	   */
	   DBPRT(1,( "Starting synchronous bulkwalk...\n"));

	   while (!(okay = _bulkwalk_done(context))) {

	      /* Send a request for the next batch of variables. */
	      DBPRT(1, (DBOUT "Building %s GETBULK bulkwalk PDU (%d)...\n",
					context->pkts_exch ? "next" : "first",
					context->pkts_exch));
	      pdu = _bulkwalk_send_pdu(context);

	      /* If the request failed, consider the walk done. */
	      if (pdu == NULL) {
		 DBPRT(1,( "bulkwalk_send_pdu() failed!\n"));
		 break;
	      }

	      /* Handle the variables in this response packet.  Break out
	      ** of the loop if an error occurs or no variables are found
	      ** in the response.
	      */
	      if ((i = _bulkwalk_recv_pdu(context, pdu)) <= 0) {
		 DBPRT(2,( "bulkwalk_recv_pdu() returned %d (error/empty)\n", i));
		 break;
	      }

              /* Free the returned pdu.  Don't bother to do this for the async
	      ** case, since the SNMP callback mechanism itself does the free
	      ** for us.
	      */
	      snmp_free_pdu(pdu);

	      /* And loop.  The call to bulkwalk_done() sets the ignore flags
	      ** for any completed request subtrees.  Next time around, they
	      ** won't be added to the request sent to the agent.
	      */
	      continue;
	   }

	   DBPRT(1, (DBOUT "Bulkwalk done... calling bulkwalk_finish(%s)...\n",
	       okay ? "okay" : "error"));
	   npushed = _bulkwalk_finish(context, okay);

	   DBPRT(2,( "Returning %d values on the stack.\n", npushed));
	   XSRETURN(npushed);

	/* Handle error cases and clean up after ourselves. */
        err:
	   if (context->req_oids && context->nreq_oids) {
	      bt_entry = context->req_oids;
	      for (i = 0; i < context->nreq_oids; i++, bt_entry++)
		 av_clear(bt_entry->vars);
	   }
	   if (context->req_oids)
	      Safefree(context->req_oids);
	   if (context)
	      Safefree(context);
	   if (pdu)
	      snmp_free_pdu(pdu);

           XSRETURN_UNDEF;
	}


int
snmp_trapV1(sess_ref,enterprise,agent,generic,specific,uptime,varlist_ref)
        SV *	sess_ref
        char *	enterprise
        char *	agent
        int	generic
        int	specific
        long	uptime
        SV *	varlist_ref
	PPCODE:
	{
           AV *varlist;
           SV **varbind_ref;
           SV **varbind_val_f;
           AV *varbind;
	   I32 varlist_len;
	   I32 varlist_ind;
           SnmpSession *ss;
           netsnmp_pdu *pdu = NULL;
           struct tree *tp;
	   oid *oid_arr;
	   int oid_arr_len = MAX_OID_LEN;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int type;
           int res;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
           int use_enums = SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums",8,1));
           struct enum_list *ep;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref)) {

              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);

              pdu = snmp_pdu_create(SNMP_MSG_TRAP);

              if (SvROK(varlist_ref)) {
              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
	      for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);

                    tp=__tag2oid(__av_elem_pv(varbind, VARBIND_TAG_F, NULL),
                                 __av_elem_pv(varbind, VARBIND_IID_F, NULL),
                                 oid_arr, &oid_arr_len, &type,0);

                    if (oid_arr_len == 0) {
                       if (verbose)
                        warn("error:trap: unable to determine oid for object");
                       goto err;
                    }

                    if (type == TYPE_UNKNOWN) {
                      type = __translate_appl_type(
                              __av_elem_pv(varbind, VARBIND_TYPE_F, NULL));
                      if (type == TYPE_UNKNOWN) {
                         if (verbose)
                            warn("error:trap: no type found for object");
                         goto err;
                      }
                    }

	            varbind_val_f = av_fetch(varbind, VARBIND_VAL_F, 0);

                    if (type==TYPE_INTEGER && use_enums && tp && tp->enums) {
                      for(ep = tp->enums; ep; ep = ep->next) {
                        if (varbind_val_f && SvOK(*varbind_val_f) &&
                            !strcmp(ep->label, SvPV(*varbind_val_f,na))) {
                          sv_setiv(*varbind_val_f, ep->value);
                          break;
                        }
                      }
                    }

                    res = __add_var_val_str(pdu, oid_arr, oid_arr_len,
                                  (varbind_val_f && SvOK(*varbind_val_f) ?
                                   SvPV(*varbind_val_f,na):NULL),
                                  (varbind_val_f && SvOK(*varbind_val_f) ?
                                   SvCUR(*varbind_val_f):0),
                                  type);

                    if(res == FAILURE) {
                        if(verbose) warn("error:trap: adding varbind");
                        goto err;
                    }

                 } /* if var_ref is ok */
              } /* for all the vars */
              }

	      pdu->enterprise = (oid *)malloc( MAX_OID_LEN * sizeof(oid));
              tp = __tag2oid(enterprise,NULL, pdu->enterprise,
                             &pdu->enterprise_length, NULL,0);
  	      if (pdu->enterprise_length == 0) {
		  if (verbose) warn("error:trap:invalid enterprise id: %s", enterprise);
                  goto err;
	      }
	      /*  If agent is given then set the v1-TRAP specific
		  agent-address field to that.  Otherwise set it to
		  our address.  */
              if (agent && strlen(agent)) {
                 if (__parse_address(agent) == -1 && verbose) {
		   warn("error:trap:invalid agent address: %s", agent);
		   goto err;
                 } else {
		   *((in_addr_t *)pdu->agent_addr) = __parse_address(agent);
		 }
              } else {
                 *((in_addr_t *)pdu->agent_addr) = get_myaddr();
              }
              pdu->trap_type = generic;
              pdu->specific_type = specific;
              pdu->time = uptime;

              if (snmp_send(ss, pdu) == 0) {
	         snmp_free_pdu(pdu);
              }
              XPUSHs(sv_2mortal(newSVpv(ZERO_BUT_TRUE,0)));
           } else {
err:
              XPUSHs(&sv_undef); /* no mem or bad args */
              if (pdu) snmp_free_pdu(pdu);
           }
	Safefree(oid_arr);
        }


int
snmp_trapV2(sess_ref,uptime,trap_oid,varlist_ref)
        SV *	sess_ref
        char *	uptime
        char *	trap_oid
        SV *	varlist_ref
	PPCODE:
	{
           AV *varlist;
           SV **varbind_ref;
           SV **varbind_val_f;
           AV *varbind;
	   I32 varlist_len;
	   I32 varlist_ind;
           SnmpSession *ss;
           netsnmp_pdu *pdu = NULL;
           struct tree *tp;
	   oid *oid_arr;
	   int oid_arr_len = MAX_OID_LEN;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int type;
           int res;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
           int use_enums = SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums",8,1));
           struct enum_list *ep;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref) && SvROK(varlist_ref)) {

              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);

              pdu = snmp_pdu_create(SNMP_MSG_TRAP2);

              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
	      /************************************************/
              res = __add_var_val_str(pdu, sysUpTime, SYS_UPTIME_OID_LEN,
				uptime, strlen(uptime), TYPE_TIMETICKS);

              if(res == FAILURE) {
                if(verbose) warn("error:trap v2: adding sysUpTime varbind");
		goto err;
              }

	      res = __add_var_val_str(pdu, snmpTrapOID, SNMP_TRAP_OID_LEN,
				trap_oid ,strlen(trap_oid) ,TYPE_OBJID);

              if(res == FAILURE) {
                if(verbose) warn("error:trap v2: adding snmpTrapOID varbind");
		goto err;
              }


	      /******************************************************/

	      for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);

                    tp=__tag2oid(__av_elem_pv(varbind, VARBIND_TAG_F,NULL),
                                 __av_elem_pv(varbind, VARBIND_IID_F,NULL),
                                 oid_arr, &oid_arr_len, &type,0);

                    if (oid_arr_len == 0) {
                       if (verbose)
                        warn("error:trap v2: unable to determine oid for object");
                       goto err;
                    }

                    if (type == TYPE_UNKNOWN) {
                      type = __translate_appl_type(
                                 __av_elem_pv(varbind, VARBIND_TYPE_F, NULL));
                      if (type == TYPE_UNKNOWN) {
                         if (verbose)
                            warn("error:trap v2: no type found for object");
                         goto err;
                      }
                    }

	            varbind_val_f = av_fetch(varbind, VARBIND_VAL_F, 0);

                    if (type==TYPE_INTEGER && use_enums && tp && tp->enums) {
                      for(ep = tp->enums; ep; ep = ep->next) {
                        if (varbind_val_f && SvOK(*varbind_val_f) &&
                            !strcmp(ep->label, SvPV(*varbind_val_f,na))) {
                          sv_setiv(*varbind_val_f, ep->value);
                          break;
                        }
                      }
                    }

                    res = __add_var_val_str(pdu, oid_arr, oid_arr_len,
                                  (varbind_val_f && SvOK(*varbind_val_f) ?
                                   SvPV(*varbind_val_f,na):NULL),
                                  (varbind_val_f && SvOK(*varbind_val_f) ?
                                   SvCUR(*varbind_val_f):0),
                                  type);

                    if(res == FAILURE) {
                        if(verbose) warn("error:trap v2: adding varbind");
                        goto err;
                    }

                 } /* if var_ref is ok */
              } /* for all the vars */

              if (snmp_send(ss, pdu) == 0) {
	         snmp_free_pdu(pdu);
              }

              XPUSHs(sv_2mortal(newSVpv(ZERO_BUT_TRUE,0)));
           } else {
err:
              XPUSHs(&sv_undef); /* no mem or bad args */
              if (pdu) snmp_free_pdu(pdu);
           }
	Safefree(oid_arr);
        }



int
snmp_inform(sess_ref,uptime,trap_oid,varlist_ref,perl_callback)
        SV *	sess_ref
        char *	uptime
        char *	trap_oid
        SV *	varlist_ref
        SV *	perl_callback
	PPCODE:
	{
           AV *varlist;
           SV **varbind_ref;
           SV **varbind_val_f;
           AV *varbind;
	   I32 varlist_len;
	   I32 varlist_ind;
           SnmpSession *ss;
           netsnmp_pdu *pdu = NULL;
           netsnmp_pdu *response;
           struct tree *tp;
	   oid *oid_arr;
	   int oid_arr_len = MAX_OID_LEN;
           snmp_xs_cb_data *xs_cb_data;
           SV **sess_ptr_sv;
           SV **err_str_svp;
           SV **err_num_svp;
           SV **err_ind_svp;
           int status = 0;
           int type;
           int res;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));
           int use_enums = SvIV(*hv_fetch((HV*)SvRV(sess_ref),"UseEnums",8,1));
           struct enum_list *ep;

           New (0, oid_arr, MAX_OID_LEN, oid);

           if (oid_arr && SvROK(sess_ref) && SvROK(varlist_ref)) {

              sess_ptr_sv = hv_fetch((HV*)SvRV(sess_ref), "SessPtr", 7, 1);
	      ss = (SnmpSession *)SvIV((SV*)SvRV(*sess_ptr_sv));
              err_str_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorStr", 8, 1);
              err_num_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorNum", 8, 1);
              err_ind_svp = hv_fetch((HV*)SvRV(sess_ref), "ErrorInd", 8, 1);
              sv_setpv(*err_str_svp, "");
              sv_setiv(*err_num_svp, 0);
              sv_setiv(*err_ind_svp, 0);

              pdu = snmp_pdu_create(SNMP_MSG_INFORM);

              varlist = (AV*) SvRV(varlist_ref);
              varlist_len = av_len(varlist);
	      /************************************************/
              res = __add_var_val_str(pdu, sysUpTime, SYS_UPTIME_OID_LEN,
				uptime, strlen(uptime), TYPE_TIMETICKS);

              if(res == FAILURE) {
                if(verbose) warn("error:inform: adding sysUpTime varbind");
		goto err;
              }

	      res = __add_var_val_str(pdu, snmpTrapOID, SNMP_TRAP_OID_LEN,
				trap_oid ,strlen(trap_oid) ,TYPE_OBJID);

              if(res == FAILURE) {
                if(verbose) warn("error:inform: adding snmpTrapOID varbind");
		goto err;
              }


	      /******************************************************/

	      for(varlist_ind = 0; varlist_ind <= varlist_len; varlist_ind++) {
                 varbind_ref = av_fetch(varlist, varlist_ind, 0);
                 if (SvROK(*varbind_ref)) {
                    varbind = (AV*) SvRV(*varbind_ref);

                    tp=__tag2oid(__av_elem_pv(varbind, VARBIND_TAG_F,NULL),
                                 __av_elem_pv(varbind, VARBIND_IID_F,NULL),
                                 oid_arr, &oid_arr_len, &type,0);

                    if (oid_arr_len == 0) {
                       if (verbose)
                        warn("error:inform: unable to determine oid for object");
                       goto err;
                    }

                    if (type == TYPE_UNKNOWN) {
                      type = __translate_appl_type(
                                 __av_elem_pv(varbind, VARBIND_TYPE_F, NULL));
                      if (type == TYPE_UNKNOWN) {
                         if (verbose)
                            warn("error:inform: no type found for object");
                         goto err;
                      }
                    }

	            varbind_val_f = av_fetch(varbind, VARBIND_VAL_F, 0);

                    if (type==TYPE_INTEGER && use_enums && tp && tp->enums) {
                      for(ep = tp->enums; ep; ep = ep->next) {
                        if (varbind_val_f && SvOK(*varbind_val_f) &&
                            !strcmp(ep->label, SvPV(*varbind_val_f,na))) {
                          sv_setiv(*varbind_val_f, ep->value);
                          break;
                        }
                      }
                    }

                    res = __add_var_val_str(pdu, oid_arr, oid_arr_len,
                                  (varbind_val_f && SvOK(*varbind_val_f) ?
                                   SvPV(*varbind_val_f,na):NULL),
                                  (varbind_val_f && SvOK(*varbind_val_f) ?
                                   SvCUR(*varbind_val_f):0),
                                  type);

                    if(res == FAILURE) {
                        if(verbose) warn("error:inform: adding varbind");
                        goto err;
                    }

                 } /* if var_ref is ok */
              } /* for all the vars */


              if (SvTRUE(perl_callback)) {
                  xs_cb_data =
                      (snmp_xs_cb_data*)malloc(sizeof(snmp_xs_cb_data));
                 xs_cb_data->perl_cb = newSVsv(perl_callback);
                 xs_cb_data->sess_ref = newRV_inc(SvRV(sess_ref));

                 status = snmp_async_send(ss, pdu, __snmp_xs_cb,
                                          (void*)xs_cb_data);
                 if (status != 0) {
                    XPUSHs(sv_2mortal(newSViv(status))); /* push the reqid?? */
                 } else {
                    snmp_free_pdu(pdu);
                    sv_catpv(*err_str_svp,
                             (char*)snmp_api_errstring(ss->s_snmp_errno));
                    sv_setiv(*err_num_svp, ss->s_snmp_errno);
                    XPUSHs(&sv_undef);
                 }
		 goto done;
              }

	      status = __send_sync_pdu(ss, pdu, &response,
				       NO_RETRY_NOSUCH,
                                       *err_str_svp, *err_num_svp,
                                       *err_ind_svp);

              if (response) snmp_free_pdu(response);

              if (status) {
		 XPUSHs(&sv_undef);
	      } else {
                 XPUSHs(sv_2mortal(newSVpv(ZERO_BUT_TRUE,0)));
              }
           } else {
err:
              XPUSHs(&sv_undef); /* no mem or bad args */
              if (pdu) snmp_free_pdu(pdu);
           }
done:
	Safefree(oid_arr);
        }



char *
snmp_get_type(tag)
	char *		tag
	CODE:
	{
	   struct tree *tp  = NULL;
	   static char type_str[MAX_TYPE_NAME_LEN];
           char *ret = NULL;

           if (tag && *tag) tp = __tag2oid(tag, NULL, NULL, NULL, NULL,0);
           if (tp) __get_type_str(tp->type, ret = type_str);
	   RETVAL = ret;
	}
	OUTPUT:
        RETVAL


void
snmp_dump_packet(flag)
	int		flag
	CODE:
	{
	   snmp_set_dump_packet(flag);
	}


char *
snmp_map_enum(tag, val, iflag)
	char *		tag
	char *		val
	int		iflag
	CODE:
	{
	   struct tree *tp  = NULL;
           struct enum_list *ep;
           char str_buf[STR_BUF_SIZE];
           int ival;

           RETVAL = NULL;

           if (tag && *tag) tp = __tag2oid(tag, NULL, NULL, NULL, NULL,0);

           if (tp) {
              if (iflag) {
                 ival = atoi(val);
                 for(ep = tp->enums; ep; ep = ep->next) {
                    if (ep->value == ival) {
                       RETVAL = ep->label;
                       break;
                    }
                 }
              } else {
                 for(ep = tp->enums; ep; ep = ep->next) {
                    if (strEQ(ep->label, val)) {
                       sprintf(str_buf,"%d", ep->value);
                       RETVAL = str_buf;
                       break;
                    }
                 }
              }
           }
	}
	OUTPUT:
        RETVAL

#define SNMP_XLATE_MODE_OID2TAG 1
#define SNMP_XLATE_MODE_TAG2OID 0

char *
snmp_translate_obj(var,mode,use_long,auto_init,best_guess)
	char *		var
	int		mode
	int		use_long
	int		auto_init
	int             best_guess
	CODE:
	{
	   char str_buf[STR_BUF_SIZE];
	   oid oid_arr[MAX_OID_LEN];
           int oid_arr_len = MAX_OID_LEN;
           char * label;
           char * iid;
           int status = FAILURE;
           int verbose = SvIV(perl_get_sv("SNMP::verbose", 0x01 | 0x04));

           str_buf[0] = '\0';
  	   switch (mode) {
              case SNMP_XLATE_MODE_TAG2OID:
		if (!__tag2oid(var, NULL, oid_arr, &oid_arr_len, NULL, best_guess)) {
		   if (verbose) warn("error:snmp_translate_obj:Unknown OID %s\n",var);
                } else {
                   status = __sprint_num_objid(str_buf, oid_arr, oid_arr_len);
                }
                break;
             case SNMP_XLATE_MODE_OID2TAG:
		oid_arr_len = 0;
		__concat_oid_str(oid_arr, &oid_arr_len, var);
		snprint_objid(str_buf, sizeof(str_buf), oid_arr, oid_arr_len);
		if (!use_long) {
                  label = NULL; iid = NULL;
		  if (((status=__get_label_iid(str_buf,
		       &label, &iid, NO_FLAGS)) == SUCCESS)
		      && label) {
		     strcpy(str_buf, label);
		     if (iid && *iid) {
		       strcat(str_buf, ".");
		       strcat(str_buf, iid);
		     }
 	          }
	        }
                break;
             default:
	       if (verbose) warn("snmp_translate_obj:unknown translation mode: %s\n", mode);
           }
           if (*str_buf) {
              RETVAL = (char*)str_buf;
           } else {
              RETVAL = (char*)NULL;
           }
	}
        OUTPUT:
        RETVAL

void
snmp_set_replace_newer(val)
	int val
	CODE:
	{
	   netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_REPLACE);
	}

void
snmp_set_save_descriptions(val)
	int	val
	CODE:
	{
	   snmp_set_save_descriptions(val);
	}

void
snmp_set_debugging(val)
	int	val
	CODE:
	{
	   snmp_set_do_debugging(val);
	}

void
snmp_debug_internals(val)
	int     val
	CODE:
	{
#ifdef		DEBUGGING
	   _debug_level = val;
#endif		/* DEBUGGING */
	}


void
snmp_sock_cleanup()
	CODE:
	{
	   SOCK_CLEANUP;
	}

void
snmp_mainloop_finish()
	CODE:
	{
	    mainloop_finish = 1;
	}


void
snmp_main_loop(timeout_sec,timeout_usec,perl_callback)
	int 	timeout_sec
	int 	timeout_usec
	SV *	perl_callback
	CODE:
	{
        int numfds, fd_count;
        fd_set fdset;
        struct timeval time_val, *tvp;
        struct timeval last_time, *ltvp;
        struct timeval ctimeout, *ctvp;
        struct timeval interval, *itvp;
        int block;
	SV *cb;


 	mainloop_finish = 0;

	itvp = &interval;
	itvp->tv_sec = timeout_sec;
	itvp->tv_usec = timeout_usec;
        ctvp = &ctimeout;
        ctvp->tv_sec = -1;
        ltvp = &last_time;
        gettimeofday(ltvp,(struct timezone*)0);
	timersub(ltvp,itvp,ltvp);
        while (1) {
           numfds = 0;
           FD_ZERO(&fdset);
           block = 1;
           tvp = &time_val;
           timerclear(tvp);
           snmp_select_info(&numfds, &fdset, tvp, &block);
           __recalc_timeout(tvp,ctvp,ltvp,itvp,&block);
           # printf("pre-select: numfds = %ld, block = %ld\n", numfds, block);
           if (block == 1) tvp = NULL; /* block without timeout */
           fd_count = select(numfds, &fdset, 0, 0, tvp);
           #printf("post-select: fd_count = %ld,block = %ld\n",fd_count,block);
           if (fd_count > 0) {
                       dSP;
                       ENTER;
                       SAVETMPS;
              snmp_read(&fdset);
                       FREETMPS;
                       LEAVE;

           } else switch(fd_count) {
              case 0:
		 SPAGAIN;
		 ENTER;
		 SAVETMPS;
                 snmp_timeout();
                 if (!timerisset(ctvp)) {
                    if (SvTRUE(perl_callback)) {
                       /* sv_2mortal(perl_callback); */
                       cb = __push_cb_args(perl_callback, NULL);
                       __call_callback(cb, G_DISCARD);
                       ctvp->tv_sec = -1;

                    } else {
                       FREETMPS;
                       LEAVE;
                       goto done;
                    }
                 }
                 FREETMPS;
                 LEAVE;
                 break;
              case -1:
                 if (errno == EINTR) {
                    continue;
                 } else {
                    /* snmp_set_detail(strerror(errno)); */
                    /* snmp_errno = SNMPERR_GENERR; */
                 }
              default:;
           }

	   /* A call to snmp_mainloop_finish() in the callback sets the
	   ** mainloop_finish flag.  Exit the loop after the callback returns.
	   */
	   if (mainloop_finish)
	      goto done;

        }
     done:
           return;
	}


void
snmp_get_select_info()
	PPCODE:
	{
        int numfds;
        fd_set fdset;
        struct timeval time_val, *tvp;
        int block;
	int i;

        numfds = 0;
        block = 1;
        tvp = &time_val;
        FD_ZERO(&fdset);
        snmp_select_info(&numfds, &fdset, tvp, &block);
	XPUSHs(sv_2mortal(newSViv(block)));
	if(block){
            XPUSHs(sv_2mortal(newSViv(0)));
            XPUSHs(sv_2mortal(newSViv(0)));
	} else {
            XPUSHs(sv_2mortal(newSViv(tvp->tv_sec)));
            XPUSHs(sv_2mortal(newSViv(tvp->tv_usec)));
	}
	if ( numfds ) {
            for(i=0; i<numfds ; i++) {
                if(FD_ISSET(i, &fdset)){
                    XPUSHs(sv_2mortal(newSViv(i)));
                }
            }
	} else {
            XPUSHs(&sv_undef);  /* no mem or bad args */
	}
	}

void
snmp_read_on_fd(fd)
	int fd
	CODE:
	{
           fd_set fdset;

           FD_ZERO(&fdset);
           FD_SET(fd, &fdset);

           snmp_read(&fdset);
	}

void
snmp_check_timeout()
	CODE:
	{
          snmp_timeout();
	}

MODULE = SNMP	PACKAGE = SNMP::MIB::NODE 	PREFIX = snmp_mib_node_
SV *
snmp_mib_node_TIEHASH(class,key,tp=0)
	char *	class
	char *	key
        IV tp
	CODE:
	{
            __libraries_init("perl");
           if (!tp) tp = (IV)__tag2oid(key, NULL, NULL, NULL, NULL,0);
           if (tp) {
              ST(0) = sv_newmortal();
              sv_setref_iv(ST(0), class, tp);
           } else {
              ST(0) = &sv_undef;
           }

	}

SV *
snmp_mib_node_FETCH(tp_ref, key)
	SV *	tp_ref
	char *	key
	CODE:
	{
	   char c = *key;
	   char str_buf[STR_BUF_SIZE];
           SnmpMibNode *tp = NULL;
           struct index_list *ip;
           struct enum_list *ep;
           struct range_list *rp;
	   struct varbind_list *vp;
           struct module *mp;
           SV *child_list_aref, *next_node_href, *mib_tied_href = NULL;
	   SV **nn_hrefp;
           HV *mib_hv, *enum_hv, *range_hv;
           AV *index_av, *varbind_av, *ranges_av;
           MAGIC *mg = NULL;

           if (SvROK(tp_ref)) tp = (SnmpMibNode*)SvIV((SV*)SvRV(tp_ref));

	   ST(0) = sv_newmortal();
           if (tp)
	   switch (c) {
	      case 'a': /* access */
                  if (strncmp("access", key, strlen(key)) == 0) {
                 switch	(tp->access) {
                   case MIB_ACCESS_READONLY:
                     sv_setpv(ST(0),"ReadOnly");
                     break;
                   case MIB_ACCESS_READWRITE:
                     sv_setpv(ST(0),"ReadWrite");
                     break;
                   case MIB_ACCESS_WRITEONLY:
                     sv_setpv(ST(0),"WriteOnly");
                     break;
                   case MIB_ACCESS_NOACCESS:
                     sv_setpv(ST(0),"NoAccess");
                     break;
                   case MIB_ACCESS_NOTIFY:
                     sv_setpv(ST(0),"Notify");
                     break;
                   case MIB_ACCESS_CREATE:
                     sv_setpv(ST(0),"Create");
                     break;
                   default:
                     break;
                 }
                  } else if (strncmp("augments", key, strlen(key)) == 0) {
                      sv_setpv(ST(0),tp->augments);
                  }
                 break;
  	      case 'c': /* children */
                 if (strncmp("children", key, strlen(key))) break;
                 child_list_aref = newRV((SV*)newAV());
                 for (tp = tp->child_list; tp; tp = tp->next_peer) {
                    mib_hv = perl_get_hv("SNMP::MIB", FALSE);
                    if (SvMAGICAL(mib_hv)) mg = mg_find((SV*)mib_hv, 'P');
                    if (mg) mib_tied_href = (SV*)mg->mg_obj;
                    next_node_href = newRV((SV*)newHV());
                    __tp_sprint_num_objid(str_buf, tp);
                    nn_hrefp = hv_fetch((HV*)SvRV(mib_tied_href),
                                        str_buf, strlen(str_buf), 1);
                    if (!SvROK(*nn_hrefp)) {
                       sv_setsv(*nn_hrefp, next_node_href);
                       ENTER ;
                       SAVETMPS ;
                       PUSHMARK(sp) ;
                       XPUSHs(SvRV(*nn_hrefp));
                       XPUSHs(sv_2mortal(newSVpv("SNMP::MIB::NODE",0)));
                       XPUSHs(sv_2mortal(newSVpv(str_buf,0)));
                       XPUSHs(sv_2mortal(newSViv((IV)tp)));
                       PUTBACK ;
                       perl_call_pv("SNMP::_tie",G_VOID);
                       /* pp_tie(ARGS); */
                       SPAGAIN ;
                       FREETMPS ;
                       LEAVE ;
                    } /* if SvROK */
                    av_push((AV*)SvRV(child_list_aref), *nn_hrefp);
                 } /* for child_list */
                 sv_setsv(ST(0), child_list_aref);
                 break;
	      case 'v':
	         if (strncmp("varbinds", key, strlen(key))) break;
		 varbind_av = newAV();
		 for (vp = tp->varbinds; vp; vp = vp->next) {
	            av_push(varbind_av, newSVpv((vp->vblabel),strlen(vp->vblabel)));
		 }
		 sv_setsv(ST(0), newRV((SV*)varbind_av));
		 break;
	      case 'd': /* description */
                  if (strncmp("description", key, strlen(key))) {
                      if(!(strncmp("defaultValue",key,strlen(key)))) {
                          /* We're looking at defaultValue */
                          sv_setpv(ST(0), tp->defaultValue);
                          break;
                      } /* end if */
                  } /* end if */
	          /* we must be looking at description */
                 sv_setpv(ST(0),tp->description);
                 break;
              case 'i': /* indexes */
                 if (strncmp("indexes", key, strlen(key))) break;
                 index_av = newAV();
                 for(ip=tp->indexes; ip != NULL; ip = ip->next) {
                    av_push(index_av,newSVpv((ip->ilabel),strlen(ip->ilabel)));
                 }
                sv_setsv(ST(0), newRV((SV*)index_av));
                break;
	      case 'l': /* label */
                 if (strncmp("label", key, strlen(key))) break;
                 sv_setpv(ST(0),tp->label);
                 break;
	      case 'm': /* moduleID */
                 if (strncmp("moduleID", key, strlen(key))) break;
                 mp = find_module(tp->modid);
                 if (mp) sv_setpv(ST(0), mp->name);
                 break;
	      case 'n': /* nextNode */
                 if (strncmp("nextNode", key, strlen(key))) break;
                 tp = __get_next_mib_node(tp);
                 if (tp == NULL) {
                    sv_setsv(ST(0), &sv_undef);
                    break;
                 }
                 mib_hv = perl_get_hv("SNMP::MIB", FALSE);
                 if (SvMAGICAL(mib_hv)) mg = mg_find((SV*)mib_hv, 'P');
                 if (mg) mib_tied_href = (SV*)mg->mg_obj;
                 __tp_sprint_num_objid(str_buf, tp);

                 nn_hrefp = hv_fetch((HV*)SvRV(mib_tied_href),
                                     str_buf, strlen(str_buf), 1);
                 /* if (!SvROK(*nn_hrefp)) { */ /* bug in ucd - 2 .0.0 nodes */
                 next_node_href = newRV((SV*)newHV());
                 sv_setsv(*nn_hrefp, next_node_href);
                 ENTER ;
                 SAVETMPS ;
                 PUSHMARK(sp) ;
                 XPUSHs(SvRV(*nn_hrefp));
                 XPUSHs(sv_2mortal(newSVpv("SNMP::MIB::NODE",0)));
                 XPUSHs(sv_2mortal(newSVpv(str_buf,0)));
                 XPUSHs(sv_2mortal(newSViv((IV)tp)));
                 PUTBACK ;
                 perl_call_pv("SNMP::_tie",G_VOID);
                 /* pp_tie(ARGS); */
                 SPAGAIN ;
                 FREETMPS ;
                 LEAVE ;
                 /* } */
                 sv_setsv(ST(0), *nn_hrefp);
                 break;
	      case 'o': /* objectID */
                 if (strncmp("objectID", key, strlen(key))) break;
                 __tp_sprint_num_objid(str_buf, tp);
                 sv_setpv(ST(0),str_buf);
                 break;
	      case 'p': /* parent */
                 if (strncmp("parent", key, strlen(key))) break;
                 tp = tp->parent;
                 if (tp == NULL) {
                    sv_setsv(ST(0), &sv_undef);
                    break;
                 }
                 mib_hv = perl_get_hv("SNMP::MIB", FALSE);
                 if (SvMAGICAL(mib_hv)) mg = mg_find((SV*)mib_hv, 'P');
                 if (mg) mib_tied_href = (SV*)mg->mg_obj;
                 next_node_href = newRV((SV*)newHV());
                 __tp_sprint_num_objid(str_buf, tp);
                 nn_hrefp = hv_fetch((HV*)SvRV(mib_tied_href),
                                     str_buf, strlen(str_buf), 1);
                 if (!SvROK(*nn_hrefp)) {
                 sv_setsv(*nn_hrefp, next_node_href);
                 ENTER ;
                 SAVETMPS ;
                 PUSHMARK(sp) ;
                 XPUSHs(SvRV(*nn_hrefp));
                 XPUSHs(sv_2mortal(newSVpv("SNMP::MIB::NODE",0)));
                 XPUSHs(sv_2mortal(newSVpv(str_buf,0)));
                 XPUSHs(sv_2mortal(newSViv((IV)tp)));
                 PUTBACK ;
                 perl_call_pv("SNMP::_tie",G_VOID);
                 /* pp_tie(ARGS); */
                 SPAGAIN ;
                 FREETMPS ;
                 LEAVE ;
                 }
                 sv_setsv(ST(0), *nn_hrefp);
                 break;
	      case 'r': /* ranges */
                 if (strncmp("ranges", key, strlen(key))) break;
                 ranges_av = newAV();
                 for(rp=tp->ranges; rp ; rp = rp->next) {
		   range_hv = newHV();
                   hv_store(range_hv, "low", strlen("low"), newSViv(rp->low), 0);
                   hv_store(range_hv, "high", strlen("high"), newSViv(rp->high), 0);
		   av_push(ranges_av, newRV((SV*)range_hv));
                 }
                 sv_setsv(ST(0), newRV((SV*)ranges_av));
                 break;
	      case 's': /* subID */
                 if (strncmp("subID", key, strlen(key))) {
                   if (strncmp("status", key, strlen(key))) {
                      if (strncmp("syntax", key, strlen(key))) break;
                      if (tp->tc_index >= 0) {
                         sv_setpv(ST(0), get_tc_descriptor(tp->tc_index));
                      } else {
                         __get_type_str(tp->type, str_buf);
                         sv_setpv(ST(0), str_buf);
                      }
                      break;
                   }

                   switch(tp->status) {
                     case MIB_STATUS_MANDATORY:
                       sv_setpv(ST(0),"Mandatory");
                       break;
                     case MIB_STATUS_OPTIONAL:
                       sv_setpv(ST(0),"Optional");
                       break;
                     case MIB_STATUS_OBSOLETE:
                       sv_setpv(ST(0),"Obsolete");
                       break;
                     case MIB_STATUS_DEPRECATED:
                       sv_setpv(ST(0),"Deprecated");
                       break;
		     case MIB_STATUS_CURRENT:
                       sv_setpv(ST(0),"Current");
                       break;
                     default:
                       break;
                   }
                 } else {
                   sv_setiv(ST(0),(I32)tp->subid);
                 }
                 break;
	      case 't': /* type */
                 if (strncmp("type", key, strlen(key))) {
                    if (strncmp("textualConvention", key, strlen(key))) break;
                    sv_setpv(ST(0), get_tc_descriptor(tp->tc_index));
                    break;
                 }
                 __get_type_str(tp->type, str_buf);
                 sv_setpv(ST(0), str_buf);
                 break;
	      case 'u': /* units */
                 if (strncmp("units", key, strlen(key))) break;
                 sv_setpv(ST(0),tp->units);
                 break;
	      case 'h': /* hint */
                 if (strncmp("hint", key, strlen(key))) break;
                 sv_setpv(ST(0),tp->hint);
                 break;
	      case 'e': /* enums */
                 if (strncmp("enums", key, strlen(key))) break;
                 enum_hv = newHV();
                 for(ep=tp->enums; ep != NULL; ep = ep->next) {
                   hv_store(enum_hv, ep->label, strlen(ep->label),
                                newSViv(ep->value), 0);
                 }
                 sv_setsv(ST(0), newRV((SV*)enum_hv));
                 break;
              default:
                 break;
	   }
	}

MODULE = SNMP	PACKAGE = SnmpSessionPtr	PREFIX = snmp_session_
void
snmp_session_DESTROY(sess_ptr)
	SnmpSession *sess_ptr
	CODE:
	{
           snmp_close( sess_ptr );
	}


/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2004, 2005, 2006, 2008,
 *               2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2019, 2020,
 *               2021
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

%{

#include "yacconfig.h"

#if !SOCKS_CLIENT

#include "monitor.h"

#endif /* !SOCKS_CLIENT */

static const char rcsid[] =
"$Id: config_parse.y,v 1.703.4.8.2.8.4.14.4.2 2024/11/21 10:22:42 michaels Exp $";

#if HAVE_LIBWRAP && (!SOCKS_CLIENT)
   extern jmp_buf tcpd_buf;
#endif /* HAVE_LIBWRAP && (!SOCKS_CLIENT) */

extern void yyrestart(FILE *fp);

typedef enum { from, to, bounce } addresscontext_t;

static int
ipaddr_requires_netmask(const addresscontext_t context,
                        const objecttype_t objecttype);
/*
 * Returns true if an ipaddress used in the context of "objecttype" requires
 * a netmask, or false otherwise.
 *
 * "isfrom" is true if the address is to be used in the source/from
 * context, and false otherwise.
 */

static void
addnumber(size_t *numberc, long long *numberv[], const long long number);

static void
addrinit(ruleaddr_t *addr, const int netmask_required);

static void
gwaddrinit(sockshost_t *addr);

static void
routeinit(route_t *route);

#if SOCKS_CLIENT
static void parseclientenv(int *haveproxyserver);
/*
 * parses client environment, if any.
 * If a proxy server is configured in environment, "haveproxyserver" is set
 * to true upon return.  If not, it is set to false.
 */

static char *serverstring2gwstring(const char *server, const int version,
                                   char *gw, const size_t gwsize);
/*
 * Converts a gateway specified in environment to the format expected
 * in a socks.conf file.
 * "server" is the address specified in the environment,
 * "version" the kind of server address,
 * "gw", of size "gwsize", is the string to store the converted address in.
 *
 * Returns "gw" on success, exits on error.
 */

#define alarminit()
#define SET_TCPOPTION(logobject, level, attr)

#else /* !SOCKS_CLIENT */

/*
 * Reset pointers to point away from object-specific memory to global
 * memory.  Should be called after adding the object.
 */
static void post_addrule(void);

/*
 * Sets up various things after a object has been parsed, but before it has
 * been added.  Should be called before adding the object.
 *
 */
static void pre_addrule(struct rule_t *rule);
static void pre_addmonitor(monitor_t *monitor);

/*
 * Prepare pointers to point to the correct memory for adding a
 * new objects.  Should always be called once we know what type of
 * object we are dealing with.
 */
static void ruleinit(rule_t *rule);
static void monitorinit(monitor_t *monitor);
static void alarminit(void);

static int configure_privileges(void);
/*
 * Sets up privileges/userids.
 */

static int
checkugid(uid_t *uid, gid_t *gid, unsigned char *isset, const char *type);

#define SET_TCPOPTION(tcp, level, attr)                                        \
do {                                                                           \
   (tcp)->isconfigured              = 1;                                       \
                                                                               \
   (tcp)->attr                      = 1;                                       \
   (tcp)->__CONCAT(attr, _loglevel) = cloglevel;                               \
} while (/* CONSTCOND */ 0)

/*
 * Let commandline-options override configfile-options.
 * Currently there's only one such option.
 */
#define LOG_CMDLINE_OVERRIDE(name, newvalue, oldvalue, fmt)                    \
do {                                                                           \
   slog(LOG_NOTICE,                                                            \
        "%s: %s commandline value \"" fmt "\" overrides "                      \
        "config-file value \"" fmt "\" set in file %s",                        \
        function, name, (newvalue), (oldvalue), sockscf.option.configfile);    \
} while (/* CONSTCOND */ 0 )

#define CMDLINE_OVERRIDE(cmdline, option)                                      \
do {                                                                           \
   if ((cmdline)->debug_isset) {                                               \
      if ((option)->debug != (cmdline)->debug)                                 \
         LOG_CMDLINE_OVERRIDE("debug",                                         \
                              (cmdline)->debug,                                \
                              (option)->debug,                                 \
                              "%d");                                           \
                                                                               \
      (option)->debug      = (cmdline)->debug;                                 \
      (option)->debug_isset= (cmdline)->debug_isset;                           \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#endif /* !SOCKS_CLIENT */

extern int  yylineno;
extern char *yytext;
extern char currentlexline[];
extern char previouslexline[];

static const char *function = "configparsing()";

/*
 * Globals because used by functions for reporting parsing errors in
 * parse_util.c
 */
unsigned char   *atype;         /* atype of new address.               */
unsigned char  parsingconfig;   /* currently parsing config?          */

/*
 * for case we are unable to (re-)open logfiles operator specifies.
 */

#if !SOCKS_CLIENT
static logtype_t       old_log,           old_errlog;
#endif /* !SOCKS_CLIENT */

static int             failed_to_add_log, failed_to_add_errlog;

static unsigned char   add_to_errlog;   /* adding file to errlog or regular?  */

static objecttype_t    objecttype;      /* current object_type we are parsing.*/


#if !SOCKS_CLIENT
static  logspecial_t                *logspecial;
static warn_protocol_tcp_options_t  *tcpoptions;

static interfaceprotocol_t *ifproto;  /* new interfaceprotocol settings.      */

static monitor_t       monitor;       /* new monitor.                         */
static monitor_if_t    *monitorif;    /* new monitor interface.               */
static int             *alarmside;    /* data-side to monitor (read/write).   */

static int             cloglevel;     /* current loglevel.                    */

static rule_t          rule;          /* new rule.                            */

static shmem_object_t  ss;
static int session_isset;
static shmem_object_t  bw;
static int bw_isset;


#endif /* !SOCKS_CLIENT */

static unsigned char   *hostidoption_isset;

static long long       *numberv;
static size_t          numberc;

#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
static unsigned char   *hostindex;
#endif /* !SOCKS_CLIENT && HAVE_SOCKS_HOSTID  */

static timeout_t       *timeout = &sockscf.timeout;           /* default.     */

static socketoption_t  socketopt;

static serverstate_t   *state;
static route_t         route;         /* new route.                           */
static sockshost_t     gw;            /* new gateway.                         */

static ruleaddr_t      src;            /* new src.                            */
static ruleaddr_t      dst;            /* new dst.                            */
static ruleaddr_t      hostid;         /* new hostid.                         */
static ruleaddr_t      rdr_from;       /* new redirect from.                  */
static ruleaddr_t      rdr_to;         /* new redirect to.                    */

#if BAREFOOTD
static ruleaddr_t      bounceto;       /* new bounce-to address.              */
#endif /* BAREFOOTD */

static ruleaddr_t      *ruleaddr;      /* current ruleaddr                    */
static extension_t     *extension;     /* new extensions                      */


static struct in_addr  *ipv4;          /* new ip address                      */
static struct in_addr  *netmask_v4;    /* new netmask                         */

static struct in6_addr *ipv6;          /* new ip address                      */
static unsigned int    *netmask_v6;    /* new netmask                         */
static uint32_t        *scopeid_v6;    /* new scopeid.                        */

static struct in_addr  *ipvany;        /* new ip address                      */
static struct in_addr  *netmask_vany;  /* new netmask                         */

static int             netmask_required;/*
                                         * netmask required for this
                                         * address?
                                         */
static char            *domain;        /* new domain.                         */
static char            *ifname;        /* new ifname.                         */
static char            *url;           /* new url.                            */

static in_port_t       *port_tcp;      /* new TCP port number.                */
static in_port_t       *port_udp;      /* new UDP port number.                */

static int             *cmethodv;      /* new client authmethods.             */
static size_t          *cmethodc;      /* number of them.                     */
static int             *smethodv;      /* new socks authmethods.              */
static size_t          *smethodc;      /* number of them.                     */

static enum operator_t *operator;      /* new port operator.                  */

#if HAVE_GSSAPI
static char            *gssapiservicename; /* new gssapiservice.              */
static char            *gssapikeytab;      /* new gssapikeytab.               */
static gssapi_enc_t    *gssapiencryption;  /* new encryption status.          */
#endif /* HAVE_GSSAPI */

#if !SOCKS_CLIENT && HAVE_LDAP
/*
 * new ldapauthorisation server details.  Used for checking if an already
 * (GSSAPI) authenticated user is member of the appropriate LDAP group.
 */
static ldapauthorisation_t    *ldapauthorisation;


/*
 * new ldapauthorisation auth server details.
 * Used for doing LDAP-based authentication of a new client.
 */
static ldapauthentication_t   *ldapauthentication;

#endif /* SOCKS_SERVER && HAVE_LDAP */

#if !SOCKS_CLIENT && HAVE_PAC
static char            *b64;        /* new b64 encoded sid.                   */
#endif /* !SOCKS_CLIENT && HAVE_PAC */

#if DEBUG
#define YYDEBUG 1
#endif /* DEBUG */

#define ADDMETHOD(method, methodc, methodv)                                    \
do {                                                                           \
   if (methodisset((method), (methodv), (methodc)))                            \
      yywarnx("duplicate method: %s.  Already set on this methodline",         \
              method2string((method)));                                        \
   else {                                                                      \
      if ((methodc) >= METHODS_KNOWN) {                                        \
         yyerrorx("too many authmethods (%lu, max is %ld)",                    \
                  (unsigned long)(methodc), (long)METHODS_KNOWN);              \
         SERRX(methodc);                                                       \
      }                                                                        \
                                                                               \
      /*                                                                       \
       * check if we have the external libraries required for the method.      \
       */                                                                      \
      switch (method) {                                                        \
         case AUTHMETHOD_BSDAUTH:                                              \
            if (!HAVE_BSDAUTH)                                                 \
               yyerrorx_nolib("bsdauth");                                      \
            break;                                                             \
                                                                               \
         case AUTHMETHOD_GSSAPI:                                               \
            if (!HAVE_GSSAPI)                                                  \
               yyerrorx_nolib("GSSAPI");                                       \
                                                                               \
            break;                                                             \
                                                                               \
         case AUTHMETHOD_RFC931:                                               \
            if (!HAVE_LIBWRAP)                                                 \
               yyerrorx_nolib("libwrap");                                      \
            break;                                                             \
                                                                               \
         case AUTHMETHOD_PAM_ANY:                                              \
         case AUTHMETHOD_PAM_ADDRESS:                                          \
         case AUTHMETHOD_PAM_USERNAME:                                         \
            if (!HAVE_PAM)                                                     \
               yyerrorx_nolib("PAM");                                          \
            break;                                                             \
                                                                               \
         case AUTHMETHOD_LDAPAUTH:                                             \
            if (!HAVE_LDAP)                                                    \
               yyerrorx_nolib("LDAP");                                         \
            break;                                                             \
      }                                                                        \
                                                                               \
      methodv[(methodc)++] = method;                                           \
   }                                                                           \
} while (0)

#define ASSIGN_NUMBER(number, op, checkagainst, object, issigned)              \
do {                                                                           \
   if (!((number) op (checkagainst)))                                          \
      yyerrorx("number (%lld) must be " #op " %lld (" #checkagainst ")",       \
               (long long)(number), (long long)(checkagainst));                \
                                                                               \
   if (issigned) {                                                             \
      if ((long long)(number) < minvalueoftype(sizeof(object)))                \
         yyerrorx("number %lld is too small.  Minimum is %lld",                \
                  (long long)number, minvalueoftype(sizeof(object)));          \
                                                                               \
      if ((long long)(number) > maxvalueoftype(sizeof(object)))                \
         yyerrorx("number %lld is too large.  Maximum is %lld",                \
                  (long long)number,  maxvalueoftype(sizeof(object)));         \
   }                                                                           \
   else  {                                                                     \
      if ((unsigned long long)(number) < uminvalueoftype(sizeof(object)))      \
         yyerrorx("number %llu is too small.  Minimum is %llu",                \
                  (unsigned long long)number, uminvalueoftype(sizeof(object)));\
                                                                               \
      if ((unsigned long long)(number) > umaxvalueoftype(sizeof(object)))      \
         yyerrorx("number %llu is too large.  Maximum is %llu",                \
                  (unsigned long long)number, umaxvalueoftype(sizeof(object)));\
   }                                                                           \
                                                                               \
   (object) = (number);                                                        \
} while (0)

#define ASSIGN_PORTNUMBER(portnumber, object)                                  \
do {                                                                           \
   /* includes 0 and MAXPORT because the exp might be "> 0" or "< MAXPORT". */ \
   ASSIGN_NUMBER(portnumber, >=,  0,         (object), 0);                     \
   ASSIGN_NUMBER(portnumber, <=, IP_MAXPORT, (object), 0);                     \
                                                                               \
   (object) = htons((in_port_t)(portnumber));                                  \
} while (0)

#define ASSIGN_THROTTLE_SECONDS(number, obj, issigned)     \
            ASSIGN_NUMBER((number), >, 0, obj, issigned)
#define ASSIGN_THROTTLE_CLIENTS(number, obj, issigned)     \
            ASSIGN_NUMBER((number), >, 0, obj, issigned)
#define ASSIGN_MAXSESSIONS(number, obj, issigned)          \
            ASSIGN_NUMBER((number), >, 0, obj, issigned)
%}


%union {
   struct {
      uid_t   uid;
      gid_t   gid;
   } uid;

   struct {
      valuetype_t valuetype;
      const int   *valuev;
   } error;

   struct {
      const char *oldname;
      const char *newname;
   } deprecated;

   char       *string;
   int        method;
   long long  number;
};


%type   <string> bsdauthstylename
%type   <string> command commands commandname
%type   <string> configtype deprecated
%type   <string> debugging
%type   <string> ifprotocol ifprotocols
%type   <string> group groupname groupnames
%type   <string> gssapienctype
%type   <string> gssapikeytab
%type   <string> gssapiservicename
%type   <string> pamservicename
%type   <string> protocol protocols protocolname
%type   <string> proxyprotocol proxyprotocolname proxyprotocols
%type   <string> realm
%type   <string> resolveprotocol resolveprotocolname
%type   <string> socketside socketoption socketoptionname socketoptionvalue
%type   <string> srchost srchostoption srchostoptions
%type   <string> udpportrange udpportrange_start udpportrange_end
%type   <string> user username usernames

   /* clientconfig exclusive. */
%type   <string> clientoptions clientoption

   /*
    * serveroption exclusive.
    */
%type   <string> alarm alarm_data alarm_disconnect alarm_test
%type   <string> networkproblem
%type   <number> alarmperiod alarmside
%type   <number> number numbers
%type   <string> clientmethod clientmethods clientmethodname
%type   <string> socksmethod socksmethods socksmethodname
%type   <string> bandwidth
%type   <string> childstate
%type   <string> clientcompatibility clientcompatibilityname
                 clientcompatibilitynames
%type   <string> compatibility compatibilityname compatibilitynames
%type   <string> cpu cpuschedule cpuaffinity
%type   <string> extension extensionname extensions
%type   <string> external externalinit external_protocol external_rotation
%type   <string> global_socksmethod global_clientmethod
%type   <string> internal internalinit internal_protocol
%type   <string> lbasedn lbasedn_hex lbasedn_hex_all
%type   <string> ldapattribute ldapattribute_ad ldapattribute_hex
                 ldapattribute_ad_hex
%type   <string> ldapauto ldapdebug ldapdepth ldapport ldapportssl
%type   <string> ldapcertfile ldapcertpath ldapkeytab
%type   <string> ldapdomain
%type   <string> ldapfilter ldapfilter_ad ldapfilter_hex ldapfilter_ad_hex
%type   <string> libwrap_hosts_access
%type   <string> libwrapfiles libwrap_allowfile libwrap_denyfile
%type   <string> logoutput errorlog
%type   <string> internal_if_logoption external_if_logoption
%type   <string> logspecial loglevel errors errorobject
%type   <string> lserver lgroup lgroup_hex lgroup_hex_all
%type   <string> ldapurl ldapssl ldapcertcheck ldapkeeprealm
%type   <string> ldapauthbasedn ldapauthbasedn_hex ldapauthbasedn_hex_all
%type   <string> ldapauthserver ldapauthkeytab
%type   <string> ldapauthurl ldapauthdebug ldapauthport
%type   <string> ldapauthportssl ldapauthssl ldapauthauto ldapauthfilter
%type   <string> ldapauthcertcheck ldapauthdomain ldapauthkeeprealm
%type   <string> ldapauthcertfile ldapauthcertpath
%type   <string> monitor monitoroption monitoroptions monitorside
%type   <string> psid psid_b64 psid_off
%type   <string> redirect
%type   <string> serveroption serveroptions serverobject serverobjects
%type   <string> sessionoption crulesessionoption sockssessionoption sessionmax                  sessioninheritable sessionthrottle sessionstate
                 sessionstate_key sessionstate_keyinfo sessionstate_throttle
                 sessionstate_max
%type   <string> timeout iotimeout negotiatetimeout connecttimeout
                 tcp_fin_timeout
%type   <string> udpconnectdst
%type   <string> userids user_privileged user_unprivileged user_libwrap
%type   <uid>    userid


%token   <string> ALARM ALARMTYPE_DATA ALARMTYPE_DISCONNECT
                  ALARMIF_INTERNAL ALARMIF_EXTERNAL
%token   <string> TCPOPTION_DISABLED ECN SACK TIMESTAMPS WSCALE
%token   <string> MTU_ERROR
%token   <string> CLIENTCOMPATIBILITY NECGSSAPI
%token   <string> CLIENTRULE HOSTIDRULE SOCKSRULE
%token   <string> COMPATIBILITY SAMEPORT DRAFT_5_05
%token   <string> CONNECTTIMEOUT TCP_FIN_WAIT
%token   <string> CPU MASK SCHEDULE CPUMASK_ANYCPU
%token   <string> DEBUGGING
%token   <deprecated> DEPRECATED
%token   <string> ERRORLOG LOGOUTPUT LOGFILE LOGTYPE_ERROR
                  LOGTYPE_TCP_DISABLED LOGTYPE_TCP_ENABLED
                  LOGIF_INTERNAL LOGIF_EXTERNAL
%token   <error>  ERRORVALUE
%token   <string> EXTENSION BIND PRIVILEGED
%token   <string> EXTERNAL_PROTOCOL INTERNAL_PROTOCOL
%token   <string> EXTERNAL_ROTATION SAMESAME
%token   <string> GROUPNAME
%token   <string> HOSTID HOSTINDEX
%token   <string> INTERFACE SOCKETOPTION_SYMBOLICVALUE
%token   <string> INTERNAL EXTERNAL
%token   <string> INTERNALSOCKET EXTERNALSOCKET
%token   <string> IOTIMEOUT IOTIMEOUT_TCP IOTIMEOUT_UDP NEGOTIATETIMEOUT
%token   <string> LIBWRAP_FILE
%token   <number> LOGLEVEL
%token   <string> SOCKSMETHOD CLIENTMETHOD METHOD
%token   <method> METHODNAME NONE BSDAUTH GSSAPI
                  PAM_ADDRESS PAM_ANY PAM_USERNAME
                  RFC931 UNAME
%token   <string> MONITOR
%token   <number> PROCESSTYPE
%token   <string> PROC_MAXREQUESTS PROC_MAXLIFETIME
%token   <string> REALM REALNAME RESOLVEPROTOCOL
%token   <string> REQUIRED
%token   <number> SCHEDULEPOLICY
%token   <string> SERVERCONFIG CLIENTCONFIG
%token   <string> SOCKET CLIENTSIDE_SOCKET SNDBUF RCVBUF
%token   <number> SOCKETPROTOCOL SOCKETOPTION_OPTID
%token   <string> SRCHOST NODNSMISMATCH NODNSUNKNOWN CHECKREPLYAUTH
%token   <string> USERNAME
%token   <string> USER_PRIVILEGED USER_UNPRIVILEGED USER_LIBWRAP
%token   <string> WORD__IN

   /* route */
%type   <string> global_routeoption
%type   <string> via gateway route routes routeoption routeoptions routemethod

%token   <string> ROUTE VIA
%token   <string> GLOBALROUTEOPTION BADROUTE_EXPIRE MAXFAIL

   /* rulelines */
%type   <number> port gwport portrange portstart portnumber portservice
%type   <string> address ipaddress ipv4 ipv6 ipvany domain ifname url
%type   <string> gwaddress bouncetoaddress
%type   <string> address_without_port srcaddress hostid_srcaddress dstaddress
%type   <string> bounce bounceto
%type   <string> crule cruleoption cruleoptions
%type   <string> externaladdress
%type   <string> from to
%type   <string> fromto hostid_fromto
%type   <string> hrule hostidoption hostindex hostid
%type   <string> genericruleoption
%type   <string> ldapoption ldapauthoption libwrap
%type   <string> log logs logname
%type   <string> netmask_v4 netmask_v6
%type   <string> portoperator
%type   <string> srule sruleoption sruleoptions
%type   <string> verdict
%type   <string> rule_internal_logoption rule_external_logoption

%token <number> PORT NUMBER
%token <string> BANDWIDTH
%token <string> BOUNCE
%token <string> BSDAUTHSTYLE
%token <string> BSDAUTHSTYLENAME
%token <string> COMMAND COMMAND_BIND COMMAND_CONNECT COMMAND_UDPASSOCIATE
                COMMAND_BINDREPLY COMMAND_UDPREPLY %token <string> ACTION
%token <string> FROM TO
%token <string> GSSAPIENCTYPE
%token <string> GSSAPIENC_ANY GSSAPIENC_CLEAR GSSAPIENC_INTEGRITY                               GSSAPIENC_CONFIDENTIALITY GSSAPIENC_PERMESSAGE
%token <string> GSSAPIKEYTAB
%token <string> GSSAPISERVICE
%token <string> GSSAPISERVICENAME GSSAPIKEYTABNAME
%token <string> IPV4 IPV6 IPVANY DOMAINNAME IFNAME URL
%token <string> LDAPATTRIBUTE LDAPATTRIBUTE_AD LDAPATTRIBUTE_HEX
                LDAPATTRIBUTE_AD_HEX
%token <string> LDAPBASEDN LDAP_BASEDN
%token <string> LDAPBASEDN_HEX LDAPBASEDN_HEX_ALL
%token <string> LDAPCERTFILE LDAPCERTPATH LDAPPORT LDAPPORTSSL
%token <string> LDAPDEBUG LDAPDEPTH LDAPAUTO LDAPSEARCHTIME
%token <string> LDAPDOMAIN LDAP_DOMAIN
%token <string> LDAPFILTER LDAPFILTER_AD LDAPFILTER_HEX LDAPFILTER_AD_HEX
%token <string> LDAPGROUP LDAPGROUP_NAME
%token <string> LDAPGROUP_HEX LDAPGROUP_HEX_ALL
%token <string> LDAPKEYTAB LDAPKEYTABNAME LDAPDEADTIME
%token <string> LDAPSERVER LDAPSERVER_NAME
%token <string> LDAPAUTHSERVER LDAPAUTHKEYTAB
%token <string> LDAPSSL LDAPCERTCHECK LDAPKEEPREALM
%token <string> LDAPTIMEOUT LDAPCACHE LDAPCACHEPOS LDAPCACHENEG
%token <string> LDAPURL LDAP_URL
%token <string> LDAPAUTHBASEDN 
%token <string> LDAPAUTHBASEDN_HEX LDAPAUTHBASEDN_HEX_ALL
%token <string> LDAPAUTHURL LDAPAUTHPORT LDAPAUTHPORTSSL
%token <string> LDAPAUTHDEBUG LDAPAUTHSSL LDAPAUTHAUTO LDAPAUTHCERTCHECK
%token <string> LDAPAUTHFILTER LDAPAUTHDOMAIN
%token <string> LDAPAUTHCERTFILE LDAPAUTHCERTPATH LDAPAUTHKEEPREALM
%token <string> LDAP_FILTER LDAP_ATTRIBUTE LDAP_CERTFILE LDAP_CERTPATH
%token <string> LIBWRAPSTART LIBWRAP_ALLOW LIBWRAP_DENY LIBWRAP_HOSTS_ACCESS
%token <string> LINE
%token <string> OPERATOR
%token <string> PACSID PACSID_B64 PACSID_FLAG PACSID_NAME
%token <string> PAMSERVICENAME
%token <string> PROTOCOL PROTOCOL_TCP PROTOCOL_UDP PROTOCOL_FAKE
%token <string> PROXYPROTOCOL PROXYPROTOCOL_SOCKS_V4 PROXYPROTOCOL_SOCKS_V5
                PROXYPROTOCOL_HTTP PROXYPROTOCOL_UPNP
%token <string> REDIRECT
%token <string> SENDSIDE RECVSIDE
%token <string> SERVICENAME
%token <string> SESSION_INHERITABLE SESSIONMAX SESSIONTHROTTLE
                SESSIONSTATE_KEY SESSIONSTATE_MAX SESSIONSTATE_THROTTLE
%token <string> RULE_LOG RULE_LOG_CONNECT RULE_LOG_DATA
                RULE_LOG_DISCONNECT RULE_LOG_ERROR RULE_LOG_IOOPERATION
                RULE_LOG_TCPINFO
%token <string> STATEKEY
%token <string> UDPPORTRANGE UDPCONNECTDST
%token <string> USER GROUP
%token <string> VERDICT_BLOCK VERDICT_PASS
%token <string> YES NO


%%


   /*
    * first token we get should say whether we are parsing for client
    * or server.  Init as appropriate.
    */

configtype:   SERVERCONFIG {
#if !SOCKS_CLIENT
      extension = &sockscf.extension;
#endif /* !SOCKS_CLIENT*/
   } serveroptions  serverobjects
   | CLIENTCONFIG clientoptions routes
   ;

serverobjects: { $$ = NULL; }
   |           serverobjects serverobject
   ;

serverobject: crule
   |          hrule
   |          srule
   |          monitor
   |          route
   ;

serveroptions:  { $$ = NULL; }
   |            serveroption serveroptions

serveroption:  childstate
   |           compatibility
   |           cpu
   |           debugging
   |           deprecated
   |           errorlog
   |           extension
   |           external
   |           external_protocol
   |           external_rotation
   |           external_if_logoption
   |           global_clientmethod
   |           global_socksmethod
   |           global_routeoption
   |           internal
   |           internal_protocol
   |           internal_if_logoption
   |           libwrap_hosts_access
   |           libwrapfiles
   |           logoutput
   |           realm
   |           resolveprotocol
   |           srchost
   |           timeout
   |           udpconnectdst
   |           userids
   |           socketoption {
      if (!addedsocketoption(&sockscf.socketoptionc,
                             &sockscf.socketoptionv,
                             &socketopt))
         yywarn("could not add socket option");
   }
   ;

logspecial: LOGTYPE_ERROR        ':' errors
          | LOGTYPE_TCP_DISABLED ':' {
#if !SOCKS_CLIENT
                                tcpoptions = &logspecial->protocol.tcp.disabled;
#endif /* !SOCKS_CLIENT */
          } tcpoptions
          | LOGTYPE_TCP_ENABLED ':' {
#if !SOCKS_CLIENT
                                tcpoptions = &logspecial->protocol.tcp.enabled;
#endif /* !SOCKS_CLIENT */
          } tcpoptions
          ;


internal_if_logoption: LOGIF_INTERNAL {
#if !SOCKS_CLIENT

      logspecial = &sockscf.internal.log;

#endif /* !SOCKS_CLIENT */

   } '.' loglevel '.' logspecial
   ;

external_if_logoption: LOGIF_EXTERNAL {
#if !SOCKS_CLIENT

      logspecial = &sockscf.external.log;

#endif /* !SOCKS_CLIENT */

   } '.' loglevel '.' logspecial
   ;

rule_internal_logoption: LOGIF_INTERNAL {
#if !SOCKS_CLIENT

      logspecial = &rule.internal.log;

#endif /* !SOCKS_CLIENT */

   } '.' loglevel '.' logspecial
   ;

rule_external_logoption: LOGIF_EXTERNAL {
#if !SOCKS_CLIENT

      logspecial = &rule.external.log;

#endif /* !SOCKS_CLIENT */

   } '.' loglevel '.' logspecial
   ;


loglevel: LOGLEVEL {
#if !SOCKS_CLIENT
   SASSERTX($1 >= 0);
   SASSERTX($1 < MAXLOGLEVELS);

   cloglevel = $1;
#endif /* !SOCKS_CLIENT */
   }
   ;

tcpoptions: tcpoption
   |        tcpoption tcpoptions
   ;

tcpoption: ECN {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, ecn);
#endif /* !SOCKS_CLIENT */
   }
   ;

tcpoption: SACK {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, sack);
#endif /* !SOCKS_CLIENT */
   }
   ;

tcpoption: TIMESTAMPS {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, timestamps);
#endif /* !SOCKS_CLIENT */
   }
   ;

tcpoption: WSCALE {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, wscale);
#endif /* !SOCKS_CLIENT */
   }
   ;



errors: errorobject
   |    errorobject errors
   ;

errorobject: ERRORVALUE {
#if !SOCKS_CLIENT

   if ($1.valuev == NULL)
      yywarnx("unknown error symbol specified");
   else {
      size_t *ec, ec_max, i;
      int *ev;

      switch ($1.valuetype) {
         case VALUETYPE_ERRNO:
            ev     = logspecial->errno_loglevelv[cloglevel];
            ec     = &logspecial->errno_loglevelc[cloglevel];
            ec_max = ELEMENTS(logspecial->errno_loglevelv[cloglevel]);
            break;

         case VALUETYPE_GAIERR:
            ev     = logspecial->gaierr_loglevelv[cloglevel];
            ec     = &logspecial->gaierr_loglevelc[cloglevel];
            ec_max = ELEMENTS(logspecial->gaierr_loglevelv[cloglevel]);
            break;

         default:
            SERRX($1.valuetype);
      }

      for (i = 0; $1.valuev[i] != 0; ++i) {
         /*
          * If the value is already set in the array, e.g. because some
          * errno-symbols have the same values, ignore this value.
          */
         size_t j;

         for (j = 0; j < *ec; ++j) {
            if (ev[j] == $1.valuev[i])
               break;
         }

         if (j < *ec)
            continue; /* error-value already set in array. */

         SASSERTX(*ec < ec_max);

         ev[(*ec)] = $1.valuev[i];
         ++(*ec);
      }
   }
#endif /* !SOCKS_CLIENT */
   }
   ;


timeout: connecttimeout
   |  iotimeout
   |  negotiatetimeout
   |  tcp_fin_timeout
   ;

deprecated:   DEPRECATED {
      yyerrorx("given keyword \"%s\" is deprecated.  New keyword is %s.  "
               "Please see %s's manual for more information",
               $1.oldname, $1.newname, PRODUCT);
   }
   ;

route:   ROUTE { objecttype = object_route; } '{'
         { routeinit(&route); } routeoptions fromto gateway routeoptions '}' {
      route.src       = src;
      route.dst       = dst;
      route.gw.addr   = gw;

      route.rdr_from  = rdr_from;

      socks_addroute(&route, 1);
   }
   ;

routes: { $$ = NULL; }
   |    routes route
   ;
proxyprotocol:   PROXYPROTOCOL ':' proxyprotocols
   ;

proxyprotocolname:   PROXYPROTOCOL_SOCKS_V4 {
         state->proxyprotocol.socks_v4 = 1;
   }
   |   PROXYPROTOCOL_SOCKS_V5 {
         state->proxyprotocol.socks_v5 = 1;
   }
   |  PROXYPROTOCOL_HTTP {
         state->proxyprotocol.http     = 1;
   }
   |  PROXYPROTOCOL_UPNP {
         state->proxyprotocol.upnp     = 1;
   }
   | deprecated
   ;

proxyprotocols: proxyprotocolname
   |   proxyprotocolname proxyprotocols
   ;

user: USER ':' usernames
   ;

username:   USERNAME {
#if !SOCKS_CLIENT
      if (addlinkedname(&rule.user, $1) == NULL)
         yyerror(NOMEM);
#endif /* !SOCKS_CLIENT */
   }
   ;

usernames:   username
   |         usernames username
   ;

group: GROUP ':' groupnames
   ;

groupname:   GROUPNAME {
#if !SOCKS_CLIENT
      if (addlinkedname(&rule.group, $1) == NULL)
         yyerror(NOMEM);
#endif /* !SOCKS_CLIENT */
   }
   ;

groupnames:   groupname
   |          groupnames groupname
   ;

extension:   EXTENSION ':' extensions
   ;

extensionname:   BIND {
         yywarnx("we are currently considering deprecating the Dante-specific "
                 "SOCKS bind extension.  If you are using it, please let us "
                 "know on the public dante-misc@inet.no mailinglist");

         extension->bind = 1;
   }
   ;

extensions:   extensionname
   |          extensionname extensions
   ;

ifprotocols: ifprotocol
   |         ifprotocol ifprotocols
   ;


ifprotocol: IPV4 {
#if !SOCKS_CLIENT
      ifproto->ipv4  = 1;
   }
   |  IPV6 {
      ifproto->ipv6  = 1;
#endif /* SOCKS_SERVER */
   }
   ;

internal:   INTERNAL internalinit ':' address {
#if !SOCKS_CLIENT
#if BAREFOOTD
      yyerrorx("\"internal:\" specification is not used in %s", PRODUCT);
#endif /* BAREFOOTD */

      interfaceprotocol_t ifprotozero;

      bzero(&ifprotozero, sizeof(ifprotozero));
      if (memcmp(&ifprotozero,
                 &sockscf.internal.protocol,
                 sizeof(sockscf.internal.protocol)) == 0) {
         slog(LOG_DEBUG, "%s: no address families explicitly enabled on "
                         "internal interface.  Enabling default address "
                         "families",
                         function);

         sockscf.internal.protocol.ipv4 = sockscf.internal.protocol.ipv6 = 1;
      }

      addinternal(ruleaddr, SOCKS_TCP);
#endif /* !SOCKS_CLIENT */
   }
   ;

internalinit: {
#if !SOCKS_CLIENT
   static ruleaddr_t mem;
   struct servent    *service;
   serverstate_t     statemem;

   bzero(&statemem, sizeof(statemem));
   state               = &statemem;
   state->protocol.tcp = 1;

   bzero(&logspecial, sizeof(logspecial));

   bzero(&mem, sizeof(mem));
   addrinit(&mem, 0);

   /* set default port. */
   if ((service = getservbyname("socks", "tcp")) == NULL)
      *port_tcp = htons(SOCKD_PORT);
   else
      *port_tcp = (in_port_t)service->s_port;
#endif /* !SOCKS_CLIENT */
   }
   ;

internal_protocol: INTERNAL_PROTOCOL ':' {
#if !SOCKS_CLIENT
      if (sockscf.internal.addrc > 0) {
         if (sockscf.state.inited) {
            /*
             * Must be running due to SIGHUP.  The internal interface requires
             * special considerations, so let the SIGHUP code deal with this
             * later when we know if the change in protocol also results in.
             * adding a new interface.
             */
            ;
         }
         else {
            log_interfaceprotocol_set_too_late(INTERNALIF);
            exit(1);
         }
      }

      ifproto = &sockscf.internal.protocol;
#endif /* !SOCKS_CLIENT */
   } ifprotocols
   ;



external:   EXTERNAL externalinit ':' externaladdress {
#if !SOCKS_CLIENT
      addexternal(ruleaddr);
#endif /* !SOCKS_CLIENT */
   }
   ;

externalinit: {
#if !SOCKS_CLIENT
      static ruleaddr_t mem;
      interfaceprotocol_t ifprotozero = { 0 };

      bzero(&mem, sizeof(mem));
      addrinit(&mem, 0);

      if (memcmp(&ifprotozero,
                 &sockscf.external.protocol,
                 sizeof(sockscf.external.protocol)) == 0) {
         slog(LOG_DEBUG, "%s: no address families explicitly enabled on "
                         "external interface.  Enabling default address "
                         "families",
                         function);

         sockscf.external.protocol.ipv4 = sockscf.external.protocol.ipv6 = 1;
      }
#endif /* !SOCKS_CLIENT */
   }
   ;

external_protocol: EXTERNAL_PROTOCOL ':' {
#if !SOCKS_CLIENT
      if (sockscf.external.addrc > 0) {
         log_interfaceprotocol_set_too_late(EXTERNALIF);
         sockdexit(EXIT_FAILURE);
      }

      ifproto = &sockscf.external.protocol;
#endif /* !SOCKS_CLIENT */
   } ifprotocols
   ;


external_rotation:   EXTERNAL_ROTATION ':' NONE {
#if !SOCKS_CLIENT
      sockscf.external.rotation = ROTATION_NONE;
   }
   |   EXTERNAL_ROTATION ':' SAMESAME {
      sockscf.external.rotation = ROTATION_SAMESAME;
   }
   |   EXTERNAL_ROTATION ':' ROUTE {
      sockscf.external.rotation = ROTATION_ROUTE;
#endif /* SOCKS_SERVER */
   }
   ;

clientoption: debugging
   |          deprecated
   |          global_routeoption
   |          errorlog
   |          logoutput
   |          resolveprotocol
   |          timeout;
   ;

clientoptions: { $$ = NULL; }
   |           clientoption clientoptions
   ;

global_routeoption: GLOBALROUTEOPTION MAXFAIL ':' NUMBER {
      if ($4 < 0)
         yyerrorx("max route fails can not be negative (%ld)  Use \"0\" to "
                  "indicate routes should never be marked as bad",
                  (long)$4);

      sockscf.routeoptions.maxfail = $4;
   }
   | GLOBALROUTEOPTION BADROUTE_EXPIRE  ':' NUMBER {
      if ($4 < 0)
         yyerrorx("route failure expiry time can not be negative (%ld).  "
                  "Use \"0\" to indicate bad route marking should never expire",
                  (long)$4);

      sockscf.routeoptions.badexpire = $4;
   }
   ;

errorlog: ERRORLOG   ':' { add_to_errlog = 1; } logoutputdevices
   ;

logoutput: LOGOUTPUT ':' { add_to_errlog = 0; } logoutputdevices
   ;

logoutputdevice: LOGFILE {
   int p;

   if ((add_to_errlog && failed_to_add_errlog)
   ||      (!add_to_errlog && failed_to_add_log)) {
      yywarnx("not adding logfile \"%s\"", $1);

      slog(LOG_ALERT,
           "%s: not trying to add logfile \"%s\" due to having already failed "
           "adding logfiles during this SIGHUP.  Only if all logfiles "
           "specified in the config can be added will we switch to using "
           "the new logfiles.  Until then, we will continue using only the "
           "old logfiles",
           function, $1);
   }
   else {
      p = socks_addlogfile(add_to_errlog ? &sockscf.errlog : &sockscf.log, $1);

#if !SOCKS_CLIENT
      if (sockscf.state.inited) {
         if (p == -1) {
            if (add_to_errlog) {
               sockscf.errlog       = old_errlog;
               failed_to_add_errlog = 1;
            }
            else {
               sockscf.log          = old_log;
               failed_to_add_log    = 1;
            }
         }
         else {
            sockd_freelogobject(add_to_errlog ?  &old_errlog : &old_log, 1);
            slog(LOG_DEBUG, "%s: added logfile \"%s\" to %s",
                 function, $1, add_to_errlog ? "errlog" : "logoutput");
         }
      }

      if (p == -1)
         slog(LOG_ALERT, "%s: could not (re)open logfile \"%s\": %s%s  %s",
              function,
              $1,
              strerror(errno),
              sockscf.state.inited ?
                  "." : "",
              sockscf.state.inited ?
                  "Will continue using old logfiles" : "");

#else /* SOCKS_CLIENT  */
      if (p == -1)
         /*
          * bad, but don't consider it fatal in the client.
          */
         yywarn("failed to add logfile %s", $1);
#endif /* SOCKS_CLIENT */
   }
}

logoutputdevices:   logoutputdevice
   |   logoutputdevice logoutputdevices
   ;

childstate: PROC_MAXREQUESTS ':' NUMBER {
#if !SOCKS_CLIENT

      ASSIGN_NUMBER($3, >=, 0, sockscf.child.maxrequests, 0);

#endif /* !SOCKS_CLIENT */
   }
   | PROC_MAXLIFETIME ':' NUMBER {
#if !SOCKS_CLIENT

      ASSIGN_NUMBER($3, >=, 0, sockscf.child.maxlifetime, 0);

#endif /* !SOCKS_CLIENT */
   }
   ;

userids:   user_privileged
   |   user_unprivileged
   |   user_libwrap
   ;

user_privileged:   USER_PRIVILEGED ':' userid {
#if !SOCKS_CLIENT
#if HAVE_PRIVILEGES
      yyerrorx("userid-settings not used on platforms with privileges");
#else
      sockscf.uid.privileged_uid   = $3.uid;
      sockscf.uid.privileged_gid   = $3.gid;
      sockscf.uid.privileged_isset = 1;
#endif /* !HAVE_PRIVILEGES */
#endif /* !SOCKS_CLIENT */
   }
   ;

user_unprivileged:   USER_UNPRIVILEGED ':' userid {
#if !SOCKS_CLIENT
#if HAVE_PRIVILEGES
      yyerrorx("userid-settings not used on platforms with privileges");
#else
      sockscf.uid.unprivileged_uid   = $3.uid;
      sockscf.uid.unprivileged_gid   = $3.gid;
      sockscf.uid.unprivileged_isset = 1;
#endif /* !HAVE_PRIVILEGES */
#endif /* !SOCKS_CLIENT */
   }
   ;

user_libwrap:   USER_LIBWRAP ':' userid {
#if HAVE_LIBWRAP && (!SOCKS_CLIENT)

#if HAVE_PRIVILEGES
      yyerrorx("userid-settings not used on platforms with privileges");

#else
      sockscf.uid.libwrap_uid   = $3.uid;
      sockscf.uid.libwrap_gid   = $3.gid;
      sockscf.uid.libwrap_isset = 1;
#endif /* !HAVE_PRIVILEGES */

#else  /* !HAVE_LIBWRAP && (!SOCKS_CLIENT) */
      yyerrorx_nolib("libwrap");
#endif /* !HAVE_LIBWRAP (!SOCKS_CLIENT)*/
   }
   ;


userid:   USERNAME {
      struct passwd *pw;

      if ((pw = getpwnam($1)) == NULL)
         yyerror("getpwnam(3) says no such user \"%s\"", $1);

      $$.uid = pw->pw_uid;

      if ((pw = getpwuid($$.uid)) == NULL)
         yyerror("getpwuid(3) says no such uid %lu (from user \"%s\")",
                 (unsigned long)$$.uid, $1);

      $$.gid = pw->pw_gid;
   }
   ;

iotimeout:   IOTIMEOUT ':' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER($3, >=, 0, timeout->tcpio, 1);
      timeout->udpio = timeout->tcpio;
   }
   | IOTIMEOUT_TCP ':' NUMBER  {
      ASSIGN_NUMBER($3, >=, 0, timeout->tcpio, 1);
   }
   | IOTIMEOUT_UDP ':' NUMBER  {
      ASSIGN_NUMBER($3, >=, 0, timeout->udpio, 1);
#endif /* !SOCKS_CLIENT */
   }
   ;

negotiatetimeout:   NEGOTIATETIMEOUT ':' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER($3, >=, 0, timeout->negotiate, 1);
#endif /* !SOCKS_CLIENT */
   }
   ;

connecttimeout:   CONNECTTIMEOUT ':' NUMBER {
      ASSIGN_NUMBER($3, >=, 0, timeout->connect, 1);
   }
   ;

tcp_fin_timeout:   TCP_FIN_WAIT ':' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER($3, >=, 0, timeout->tcp_fin_wait, 1);
#endif /* !SOCKS_CLIENT */
   }
   ;


debugging: DEBUGGING ':' NUMBER {
#if SOCKS_CLIENT

       sockscf.option.debug = (int)$3;

#else /* !SOCKS_CLIENT */

      if (sockscf.initial.cmdline.debug_isset
      &&  sockscf.initial.cmdline.debug != $3)
         LOG_CMDLINE_OVERRIDE("debug",
                              sockscf.initial.cmdline.debug,
                              (int)$3,
                              "%d");
      else
         sockscf.option.debug = (int)$3;

#endif /* !SOCKS_CLIENT */
   }
   ;

libwrapfiles: libwrap_allowfile
   |  libwrap_denyfile
   ;

libwrap_allowfile: LIBWRAP_ALLOW ':' LIBWRAP_FILE {
#if !SOCKS_CLIENT
#if HAVE_LIBWRAP
      if ((hosts_allow_table  = strdup($3)) == NULL)
         yyerror(NOMEM);

      slog(LOG_DEBUG, "%s: libwrap.allow: %s", function, hosts_allow_table);
#else
      yyerrorx_nolib("libwrap");
#endif /* HAVE_LIBWRAP */
#endif /* !SOCKS_CLIENT */
   }
   ;

libwrap_denyfile: LIBWRAP_DENY ':' LIBWRAP_FILE {
#if !SOCKS_CLIENT
#if HAVE_LIBWRAP
      if ((hosts_deny_table  = strdup($3)) == NULL)
         yyerror(NOMEM);

      slog(LOG_DEBUG, "%s: libwrap.deny: %s", function, hosts_deny_table);
#else
      yyerrorx_nolib("libwrap");
#endif /* HAVE_LIBWRAP */
#endif /* !SOCKS_CLIENT */
   }
   ;

libwrap_hosts_access: LIBWRAP_HOSTS_ACCESS ':' YES {
#if !SOCKS_CLIENT
#if HAVE_LIBWRAP
      sockscf.option.hosts_access = 1;
#else
      yyerrorx("libwrap.hosts_access requires libwrap library");
#endif /* HAVE_LIBWRAP */
   }
   | LIBWRAP_HOSTS_ACCESS ':' NO {
#if HAVE_LIBWRAP
      sockscf.option.hosts_access = 0;
#else
      yyerrorx_nolib("libwrap");
#endif /* HAVE_LIBWRAP */
#endif /* !SOCKS_CLIENT */
   }
   ;

udpconnectdst: UDPCONNECTDST ':' YES {
#if !SOCKS_CLIENT
      sockscf.udpconnectdst = 1;
   }
   | UDPCONNECTDST ':' NO {
      sockscf.udpconnectdst = 0;
#endif /* !SOCKS_CLIENT */
   }
   ;


compatibility:   COMPATIBILITY ':' compatibilitynames
   ;

compatibilityname: SAMEPORT {
#if !SOCKS_CLIENT
      sockscf.compat.sameport = 1;
   }
   |  DRAFT_5_05 {
      sockscf.compat.draft_5_05 = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;

compatibilitynames:   compatibilityname
   |   compatibilityname compatibilitynames
   ;

resolveprotocol:   RESOLVEPROTOCOL ':' resolveprotocolname
   ;

resolveprotocolname:   PROTOCOL_FAKE {
         sockscf.resolveprotocol = RESOLVEPROTOCOL_FAKE;
   }
   |  PROTOCOL_TCP {
#if HAVE_NO_RESOLVESTUFF
         yyerrorx("resolveprotocol keyword not supported on this system");
#else
         sockscf.resolveprotocol = RESOLVEPROTOCOL_TCP;
#endif /* !HAVE_NO_RESOLVESTUFF */
   }
   |   PROTOCOL_UDP {
         sockscf.resolveprotocol = RESOLVEPROTOCOL_UDP;
   }
   ;

cpu: cpuschedule
   | cpuaffinity
   ;

cpuschedule: CPU '.' SCHEDULE '.' PROCESSTYPE ':' SCHEDULEPOLICY '/' NUMBER {
#if !SOCKS_CLIENT
#if !HAVE_SCHED_SETSCHEDULER
      yyerrorx("cpu scheduling policy is not supported on this system");
#else /* HAVE_SCHED_SETSCHEDULER */
      cpusetting_t *cpusetting;

      switch ($5) {
         case PROC_MOTHER:
            cpusetting = &sockscf.cpu.mother;
            break;

         case PROC_MONITOR:
            cpusetting = &sockscf.cpu.monitor;
            break;

         case PROC_NEGOTIATE:
            cpusetting = &sockscf.cpu.negotiate;
            break;

         case PROC_REQUEST:
            cpusetting = &sockscf.cpu.request;
            break;

         case PROC_IO:
            cpusetting = &sockscf.cpu.io;
            break;

         default:
            SERRX($5);
      }

      bzero(&cpusetting->param, sizeof(cpusetting->param));

      cpusetting->scheduling_isset     = 1;
      cpusetting->policy               = $7;
      cpusetting->param.sched_priority = (int)$9;
#endif /* HAVE_SCHED_SETSCHEDULER */
#endif /* !SOCKS_CLIENT */
   }
   ;

cpuaffinity: CPU '.' MASK '.' PROCESSTYPE ':' numbers {
#if !SOCKS_CLIENT
#if !HAVE_SCHED_SETAFFINITY
      yyerrorx("cpu scheduling affinity is not supported on this system");
#else /* HAVE_SCHED_SETAFFINITY */
      cpusetting_t *cpusetting;

      switch ($5) {
         case PROC_MOTHER:
            cpusetting = &sockscf.cpu.mother;
            break;

         case PROC_MONITOR:
            cpusetting = &sockscf.cpu.monitor;
            break;

         case PROC_NEGOTIATE:
            cpusetting = &sockscf.cpu.negotiate;
            break;

         case PROC_REQUEST:
            cpusetting = &sockscf.cpu.request;
            break;

         case PROC_IO:
            cpusetting = &sockscf.cpu.io;
            break;

         default:
            SERRX($5);
      }

      cpu_zero(&cpusetting->mask);
      while (numberc-- > 0)
         if (numberv[numberc] == CPUMASK_ANYCPU) {
            const long cpus = sysconf(_SC_NPROCESSORS_ONLN);
            long i;

            if (cpus == -1)
               yyerror("sysconf(_SC_NPROCESSORS_ONLN) failed");

            for (i = 0; i < cpus; ++i)
               cpu_set((int)i, &cpusetting->mask);
         }
         else if (numberv[numberc] < 0)
            yyerrorx("invalid CPU number: %ld.  The CPU number can not be "
                     "negative", (long)numberv[numberc]);
         else
            cpu_set(numberv[numberc], &cpusetting->mask);

      free(numberv);
      numberv = NULL;
      numberc = 0;

      cpusetting->affinity_isset = 1;

#endif /* HAVE_SCHED_SETAFFINITY */
#endif /* !SOCKS_CLIENT */
   }
   ;

socketoption: socketside SOCKETPROTOCOL '.' {
#if !SOCKS_CLIENT
      socketopt.level = $2;
#endif /* !SOCKS_CLIENT */
   } socketoptionname ':' socketoptionvalue
   ;

socketoptionname: NUMBER {
#if !SOCKS_CLIENT
   socketopt.optname = $1;
   socketopt.info    = optval2sockopt(socketopt.level, socketopt.optname);

   if (socketopt.info == NULL)
      slog(LOG_DEBUG,
           "%s: unknown/unsupported socket option: level %d, value %d",
           function, socketopt.level, socketopt.optname);
   else
      socketoptioncheck(&socketopt);
   }
   | SOCKETOPTION_OPTID {
      socketopt.info           = optid2sockopt((size_t)$1);
      SASSERTX(socketopt.info != NULL);

      socketopt.optname        = socketopt.info->value;

      socketoptioncheck(&socketopt);
#endif /* !SOCKS_CLIENT */
   }
   ;

socketoptionvalue: NUMBER {
      socketopt.optval.int_val = (int)$1;
      socketopt.opttype        = int_val;
   }
   | SOCKETOPTION_SYMBOLICVALUE {
      const sockoptvalsym_t *p;

      if (socketopt.info == NULL)
         yyerrorx("the given socket option is unknown, so can not lookup "
                  "symbolic option value");

      if ((p = optval2valsym(socketopt.info->optid, $1)) == NULL)
         yyerrorx("symbolic value \"%s\" is unknown for socket option %s",
                  $1, sockopt2string(&socketopt, NULL, 0));

      socketopt.optval  = p->symval;
      socketopt.opttype = socketopt.info->opttype;
   }
   ;


socketside: INTERNALSOCKET { bzero(&socketopt, sizeof(socketopt));
                             socketopt.isinternalside = 1;
   }
   |        EXTERNALSOCKET { bzero(&socketopt, sizeof(socketopt));
                             socketopt.isinternalside = 0;
   }
   ;


srchost: SRCHOST ':' srchostoptions
   ;

srchostoption:   NODNSMISMATCH {
#if !SOCKS_CLIENT
         sockscf.srchost.nodnsmismatch = 1;
   }
   |  NODNSUNKNOWN {
         sockscf.srchost.nodnsunknown = 1;
   }
   |  CHECKREPLYAUTH {
         sockscf.srchost.checkreplyauth = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;

srchostoptions:   srchostoption
   |   srchostoption srchostoptions
   ;

realm: REALM ':' REALNAME {
#if COVENANT
   STRCPY_CHECKLEN(sockscf.realmname,
                   $3,
                   sizeof(sockscf.realmname) - 1,
                   yyerrorx);
#else /* !COVENANT */
   yyerrorx("unknown keyword \"%s\"", $1);
#endif /* !COVENANT */
}
   ;

global_clientmethod:   CLIENTMETHOD ':' {
#if !SOCKS_CLIENT

   cmethodv  = sockscf.cmethodv;
   cmethodc  = &sockscf.cmethodc;
  *cmethodc  = 0; /* reset. */

#endif /* !SOCKS_CLIENT */
   } clientmethods
   ;

global_socksmethod:   SOCKSMETHOD ':' {
#if HAVE_SOCKS_RULES

      smethodv  = sockscf.smethodv;
      smethodc  = &sockscf.smethodc;
     *smethodc  = 0; /* reset. */

#else
      yyerrorx("\"socksmethod\" is not used in %s.  Only \"clientmethod\" "
               "is used",
               PRODUCT);
#endif /* !HAVE_SOCKS_RULES */
   } socksmethods
   ;

socksmethod: SOCKSMETHOD ':' socksmethods
   ;

socksmethods:  socksmethodname
   |           socksmethodname socksmethods
   ;

socksmethodname: METHODNAME {
      if (methodisvalid($1, object_srule))
         ADDMETHOD($1, *smethodc, smethodv);
      else
         yyerrorx("method %s (%d) is not a valid method for socksmethods",
                  method2string($1), $1);
   };


clientmethod: CLIENTMETHOD ':' clientmethods
   ;

clientmethods:   clientmethodname
   |   clientmethodname clientmethods
   ;


clientmethodname:   METHODNAME {
      if (methodisvalid($1, object_crule))
         ADDMETHOD($1, *cmethodc, cmethodv);
      else
         yyerrorx("method %s (%d) is not a valid method for clientmethods",
                  method2string($1), $1);
   };

monitor: MONITOR { objecttype = object_monitor; } '{' {
#if !SOCKS_CLIENT
                        monitorinit(&monitor);
#endif /* !SOCKS_CLIENT */
}                    monitoroptions fromto monitoroptions  '}'
{
#if !SOCKS_CLIENT
   pre_addmonitor(&monitor);

   addmonitor(&monitor);
#endif /* !SOCKS_CLIENT */
}

   /*
    * ACL-rules
    */

crule: CLIENTRULE { objecttype = object_crule; } verdict '{'
                  cruleoptions fromto cruleoptions '}' {
#if !SOCKS_CLIENT
#if BAREFOOTD
      if (bounceto.atype == SOCKS_ADDR_NOTSET) {
         if (rule.verdict == VERDICT_PASS)
            yyerrorx("no address traffic should bounce to has been given");
         else {
            /*
             * allow no bounce-to address if it is a block, as the bounce-to
             * address will not be used in any case then.
             */
            bounceto.atype               = SOCKS_ADDR_IPV4;
            bounceto.addr.ipv4.ip.s_addr = htonl(INADDR_ANY);
            bounceto.port.tcp            = htons(0);
            bounceto.port.udp            = htons(0);
         }
      }

      rule.extra.bounceto = bounceto;
#endif /* BAREFOOTD */

      pre_addrule(&rule);
      addclientrule(&rule);
      post_addrule();
#endif /* !SOCKS_CLIENT */
   }
   ;

alarm: alarm_data
   |   alarm_disconnect
   |   alarm_test
   ;

monitorside: {
#if !SOCKS_CLIENT
         monitorif = NULL;
   }
   |  ALARMIF_INTERNAL {
         monitorif = &monitor.mstats->object.monitor.internal;
   }
   | ALARMIF_EXTERNAL {
         monitorif = &monitor.mstats->object.monitor.external;
#endif /* !SOCKS_CLIENT */
   }
   ;

alarmside: {
#if !SOCKS_CLIENT
      alarmside = NULL;
   }
   | RECVSIDE {
      *alarmside = RECVSIDE;
   }
   | SENDSIDE {
      *alarmside = SENDSIDE;
#endif /* !SOCKS_CLIENT */
   }
   ;

alarm_data: monitorside ALARMTYPE_DATA { alarminit(); } alarmside ':'
            NUMBER WORD__IN NUMBER  {
#if !SOCKS_CLIENT
   alarm_data_limit_t limit;

   ASSIGN_NUMBER($6, >=, 0, limit.bytes, 0);
   ASSIGN_NUMBER($8, >, 0, limit.seconds, 1);

   monitor.alarmsconfigured |= ALARM_DATA;

   if (monitor.alarm_data_aggregate != 0)
      yyerrorx("one aggregated data alarm has already been specified.  "
               "No more data alarms can be specified in this monitor");

   if (monitorif == NULL) {
      monitor.alarm_data_aggregate = ALARM_INTERNAL | ALARM_EXTERNAL;

      if (alarmside == NULL)
         monitor.alarm_data_aggregate |= ALARM_RECV | ALARM_SEND;

      if (alarmside == NULL || *alarmside == RECVSIDE) {
         monitor.mstats->object.monitor.internal.alarm.data.recv.isconfigured
         = 1;
         monitor.mstats->object.monitor.internal.alarm.data.recv.limit = limit;
      }

      if (alarmside == NULL || *alarmside == SENDSIDE) {
         monitor.mstats->object.monitor.internal.alarm.data.send.isconfigured
         = 1;
         monitor.mstats->object.monitor.internal.alarm.data.send.limit = limit;
      }

      if (alarmside == NULL || *alarmside == RECVSIDE) {
         monitor.mstats->object.monitor.external.alarm.data.recv.isconfigured
         = 1;
         monitor.mstats->object.monitor.external.alarm.data.recv.limit = limit;
      }

      if (alarmside == NULL || *alarmside == SENDSIDE) {
         monitor.mstats->object.monitor.external.alarm.data.send.isconfigured
         = 1;
         monitor.mstats->object.monitor.external.alarm.data.send.limit = limit;
      }
   }
   else {
      if (alarmside == NULL)
         monitor.alarm_data_aggregate = ALARM_RECV | ALARM_SEND;

      if (alarmside == NULL || *alarmside == RECVSIDE) {
         monitorif->alarm.data.recv.isconfigured = 1;
         monitorif->alarm.data.recv.limit        = limit;
      }

      if (alarmside == NULL || *alarmside == SENDSIDE) {
         monitorif->alarm.data.send.isconfigured = 1;
         monitorif->alarm.data.send.limit        = limit;
      }
   }
#endif /* !SOCKS_CLIENT */
   }
   ;

alarm_test: monitorside ALARM '.' networkproblem
          ;

networkproblem: MTU_ERROR {
#if !SOCKS_CLIENT
   monitor.alarmsconfigured |= ALARM_TEST;

   if (monitorif == NULL) {
      monitor.mstats->object.monitor.internal.alarm.test.mtu.dotest = 1;
      monitor.mstats->object.monitor.external.alarm.test.mtu.dotest = 1;
   }
   else {
      monitorif->alarm.test.mtu.dotest = 1;
      monitorif->alarm.test.mtu.dotest = 1;
   }
#endif /* !SOCKS_CLIENT */
   }
   ;


alarm_disconnect:
   monitorside ALARMTYPE_DISCONNECT ':' NUMBER '/' NUMBER alarmperiod {
#if !SOCKS_CLIENT
   alarm_disconnect_limit_t limit;

   ASSIGN_NUMBER($6, >, 0, limit.sessionc, 0);
   ASSIGN_NUMBER($4, >, 0, limit.disconnectc, 0);
   ASSIGN_NUMBER($7, >, 0, limit.seconds, 1);

   if (monitor.alarm_disconnect_aggregate != 0)
      yyerrorx("one aggregated disconnect alarm has already been specified.  "
               "No more disconnect alarms can be specified in this monitor");

   monitor.alarmsconfigured |= ALARM_DISCONNECT;

   if (monitorif == NULL) {
      monitor.alarm_disconnect_aggregate = ALARM_INTERNAL | ALARM_EXTERNAL;

      monitor.mstats->object.monitor.internal.alarm.disconnect.isconfigured = 1;
      monitor.mstats->object.monitor.internal.alarm.disconnect.limit = limit;

        monitor.mstats->object.monitor.external.alarm.disconnect
      = monitor.mstats->object.monitor.internal.alarm.disconnect;
   }
   else {
      monitorif->alarm.disconnect.isconfigured = 1;
      monitorif->alarm.disconnect.limit        = limit;
   }
#endif /* !SOCKS_CLIENT */
   }
   ;

alarmperiod: {
#if !SOCKS_CLIENT
               $$ = DEFAULT_ALARM_PERIOD;
#endif /* !SOCKS_CLIENT */
   }
   | WORD__IN NUMBER { $$ = $2; }
   ;

monitoroption: alarm
   |           command
   |           hostidoption { *hostidoption_isset = 1; }
   |           protocol
   ;

monitoroptions:   { $$ = NULL; }
   |   monitoroption monitoroptions;
   ;

cruleoption: bounce  {
#if !BAREFOOTD
                  yyerrorx("unsupported option");
#endif /* !BAREFOOTD */
   }
   |         protocol {
#if !BAREFOOTD
                  yyerrorx("unsupported option");
#endif /* !BAREFOOTD */
   }
   |         clientcompatibility
   |         crulesessionoption {
#if !SOCKS_CLIENT
                  session_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   |         genericruleoption
   ;

hrule: HOSTIDRULE {

#if SOCKS_CLIENT || !HAVE_SOCKS_HOSTID
      yyerrorx("hostid is not supported on this system");
#endif /* SOCKS_CLIENT || !HAVE_SOCKS_HOSTID */

      objecttype = object_hrule;
} verdict '{' cruleoptions hostid_fromto cruleoptions '}' {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
      if (hostid.atype != SOCKS_ADDR_NOTSET)
         yyerrorx("it does not make sense to set the hostid address in a "
                  "hostid-rule.  Use the \"from\" address to match the hostid "
                  "of the client");

      *hostidoption_isset = 1;

      pre_addrule(&rule);
      addhostidrule(&rule);
      post_addrule();
#endif /* !SOCKS_CLIENT && HAVE_SOCKS_HOSTID */
   }
   ;



cruleoptions:   { $$ = NULL; }
   |   cruleoption cruleoptions
   ;

hostidoption:   hostid
   | hostindex;
   ;

hostid: HOSTID ':' {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
      addrinit(&hostid, 1);

#else /* HAVE_SOCKS_HOSTID */
      yyerrorx("hostid is not supported on this system");
#endif /* HAVE_SOCKS_HOSTID */

   } address_without_port
   ;

hostindex: HOSTINDEX ':' NUMBER {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
   ASSIGN_NUMBER($3, >=, 0, *hostindex, 0);
   ASSIGN_NUMBER($3, <=, HAVE_MAX_HOSTIDS, *hostindex, 0);

#else
   yyerrorx("hostid is not supported on this system");
#endif /* !SOCKS_CLIENT && HAVE_SOCKS_HOSTID */
}
   ;


srule: SOCKSRULE { objecttype = object_srule; } verdict '{'
                 sruleoptions fromto sruleoptions '}' {
#if !SOCKS_CLIENT
#if !HAVE_SOCKS_RULES
   yyerrorx("socks-rules are not used in %s", PRODUCT);
#endif /* !HAVE_SOCKS_RULES */

      pre_addrule(&rule);
      addsocksrule(&rule);
      post_addrule();
#endif /* !SOCKS_CLIENT */
   }
   ;


sruleoptions:   { $$ = NULL; }
   |            sruleoption sruleoptions
   ;


sruleoption: bsdauthstylename
   |         command
   |         genericruleoption
   |         ldapoption
   |         ldapauthoption
   |         protocol
   |         proxyprotocol
   |         sockssessionoption {
#if !SOCKS_CLIENT
                  session_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   |         udpportrange
   ;


genericruleoption:  bandwidth {
#if !SOCKS_CLIENT
                        checkmodule("bandwidth");
                        bw_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   |        clientmethod
   |        socksmethod
   |        rule_external_logoption
   |        group
   |        gssapienctype
   |        gssapikeytab
   |        gssapiservicename
   |        hostidoption { *hostidoption_isset = 1; }
   |        rule_internal_logoption
   |        libwrap
   |        log
   |        pamservicename
   |   psid {
#if !SOCKS_CLIENT
                     checkmodule("pac");
#endif /* !SOCKS_CLIENT */
   }
   |   psid_b64 {
#if !SOCKS_CLIENT
                     checkmodule("pac");
#endif /* !SOCKS_CLIENT */
   }
   |        psid_off {

#if !SOCKS_CLIENT

                     checkmodule("pac");

#endif /* !SOCKS_CLIENT */
   }
   |        redirect   {
#if !SOCKS_CLIENT
                     checkmodule("redirect");
#endif /* !SOCKS_CLIENT */
   }
   |        socketoption {
#if !SOCKS_CLIENT
         if (rule.verdict == VERDICT_BLOCK && !socketopt.isinternalside)
            yyerrorx("it does not make sense to set a socket option for the "
                     "external side in a rule that blocks access; the external "
                     "side will never be accessed as the rule blocks access "
                     "to it");

         if (socketopt.isinternalside)
            if (socketopt.info != NULL && socketopt.info->calltype == preonly)
               yywarnx("to our knowledge the socket option \"%s\" can only be "
                       "correctly applied at pre-connection establishment "
                       "time, but by the time this rule is matched, the "
                       "connection will already have been established",
                       socketopt.info == NULL ? "unknown" :
                                                socketopt.info->name);

         if (!addedsocketoption(&rule.socketoptionc,
                                &rule.socketoptionv,
                                &socketopt))
            yywarn("could not add socketoption");
#endif /* !SOCKS_CLIENT */
   }
   |        timeout
   |        user
   ;

ldapauthoption: ldapauthserver
   |            ldapauthkeytab
   |            ldapauthurl
   |            ldapauthbasedn
   |            ldapauthbasedn_hex
   |            ldapauthbasedn_hex_all
   |            ldapauthdomain
   |            ldapauthdebug
   |            ldapauthport
   |            ldapauthportssl
   |            ldapauthssl
   |            ldapauthauto
   |            ldapauthfilter
   |            ldapauthcertcheck
   |            ldapauthcertfile
   |            ldapauthcertpath
   |            ldapauthkeeprealm
   ;

ldapoption: ldapattribute
   |        ldapattribute_ad
   |        ldapattribute_ad_hex
   |        ldapattribute_hex
   |        ldapauto
   |        lbasedn
   |        lbasedn_hex
   |        lbasedn_hex_all
   |        ldapcertcheck
   |        ldapcertfile
   |        ldapcertpath
   |        ldapdebug
   |        ldapdepth
   |        ldapdomain
   |        ldapfilter
   |        ldapfilter_ad
   |        ldapfilter_ad_hex
   |        ldapfilter_hex
   |        ldapkeeprealm
   |        ldapkeytab
   |        ldapport
   |        ldapportssl
   |        ldapssl
   |        lgroup
   |        lgroup_hex
   |        lgroup_hex_all
   |        lserver
   |        ldapurl
   ;


ldapdebug: LDAPDEBUG ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP && HAVE_OPENLDAP
      ldapauthorisation->debug = (int)$3;
   }
   | LDAPDEBUG ':' '-'NUMBER {
      ldapauthorisation->debug = (int)-$4;
 #else /* !HAVE_LDAP */
      yyerrorx_nolib("openldap");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthdebug: LDAPAUTHDEBUG ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP && HAVE_OPENLDAP
      ldapauthentication->debug = (int)$3;
   }
   | LDAPAUTHDEBUG ':' '-'NUMBER {
      ldapauthentication->debug = (int)-$4;
 #else /* !HAVE_LDAP */
      yyerrorx_nolib("openldap");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapdomain: LDAPDOMAIN ':' LDAP_DOMAIN {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthorisation.domain,
                      $3,
                      sizeof(state->ldapauthorisation.domain) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthdomain: LDAPAUTHDOMAIN ':' LDAP_DOMAIN {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthentication.domain,
                      $3,
                      sizeof(state->ldapauthentication.domain) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapdepth: LDAPDEPTH ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP && HAVE_OPENLDAP
      ldapauthorisation->mdepth = (int)$3;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("openldap");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapcertfile: LDAPCERTFILE ':' LDAP_CERTFILE {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthorisation.certfile,
                      $3,
                      sizeof(state->ldapauthorisation.certfile) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthcertfile: LDAPAUTHCERTFILE ':' LDAP_CERTFILE {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthentication.certfile,
                      $3,
                      sizeof(state->ldapauthentication.certfile) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapcertpath: LDAPCERTPATH ':' LDAP_CERTPATH {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthorisation.certpath,
                      $3,
                      sizeof(state->ldapauthorisation.certpath) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthcertpath: LDAPAUTHCERTPATH ':' LDAP_CERTPATH {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthentication.certpath,
                      $3,
                      sizeof(state->ldapauthentication.certpath) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */

      yyerrorx_nolib("LDAP");

#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapurl: LDAPURL ':' LDAP_URL {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapurl, $3) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthurl: LDAPAUTHURL ':' LDAP_URL {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapurl, $3) == NULL)
         yyerror(NOMEM);
      if (sockscf.state.ldapauthentication.ldapurl == NULL)
         sockscf.state.ldapauthentication.ldapurl = state->ldapauthentication.ldapurl;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthbasedn: LDAPAUTHBASEDN ':' LDAP_BASEDN {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapbasedn, $3) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthbasedn_hex: LDAPAUTHBASEDN_HEX ':' LDAP_BASEDN {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapbasedn, hextoutf8($3, 0)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthbasedn_hex_all: LDAPAUTHBASEDN_HEX_ALL ':' LDAP_BASEDN {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapbasedn, hextoutf8($3, 1)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lbasedn: LDAPBASEDN ':' LDAP_BASEDN {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapbasedn, $3) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lbasedn_hex: LDAPBASEDN_HEX ':' LDAP_BASEDN {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapbasedn, hextoutf8($3, 0)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lbasedn_hex_all: LDAPBASEDN_HEX_ALL ':' LDAP_BASEDN {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapbasedn, hextoutf8($3, 1)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthport: LDAPAUTHPORT ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthentication->port = (int)$3;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapport: LDAPPORT ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthorisation->port = (int)$3;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthportssl: LDAPAUTHPORTSSL ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthentication->portssl = (int)$3;
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapportssl: LDAPPORTSSL ':' NUMBER {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthorisation->portssl = (int)$3;
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapssl: LDAPSSL ':' YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->ssl = 1;
   }
   | LDAPSSL ':' NO {
      ldapauthorisation->ssl = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthssl: LDAPAUTHSSL ':' YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->ssl = 1;
   }
   | LDAPAUTHSSL ':' NO {
      ldapauthentication->ssl = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauto: LDAPAUTO ':' YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->auto_off = 1;
   }
   | LDAPAUTO ':' NO {
      ldapauthorisation->auto_off = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthauto: LDAPAUTHAUTO ':' YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->auto_off = 1;
   }
   | LDAPAUTHAUTO ':' NO {
      ldapauthentication->auto_off = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapcertcheck: LDAPCERTCHECK ':'  YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->certcheck = 1;
   }
   | LDAPCERTCHECK ':' NO {
      ldapauthorisation->certcheck = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthcertcheck: LDAPAUTHCERTCHECK ':'  YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->certcheck = 1;
   }
   | LDAPAUTHCERTCHECK ':' NO {
      ldapauthentication->certcheck = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthkeeprealm: LDAPAUTHKEEPREALM ':'  YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->keeprealm = 1;
   }
   | LDAPAUTHKEEPREALM ':' NO {
      ldapauthentication->keeprealm = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;


ldapkeeprealm: LDAPKEEPREALM ':'  YES {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->keeprealm = 1;
   }
   | LDAPKEEPREALM ':' NO {
      ldapauthorisation->keeprealm = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapfilter: LDAPFILTER ':' LDAP_FILTER {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKLEN(ldapauthorisation->filter, $3, sizeof(state->ldapauthorisation.filter) - 1, yyerrorx);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthfilter: LDAPAUTHFILTER ':' LDAP_FILTER {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKLEN(ldapauthentication->filter, $3, sizeof(state->ldapauthentication.filter) - 1, yyerrorx);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapfilter_ad: LDAPFILTER_AD ':' LDAP_FILTER {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(ldapauthorisation->filter_AD,
                      $3,
                      sizeof(state->ldapauthorisation.filter_AD) - 1,
                      yyerrorx);

#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapfilter_hex: LDAPFILTER_HEX ':' LDAP_FILTER {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKUTFLEN(ldapauthorisation->filter,
                          $3,
                          sizeof(state->ldapauthorisation.filter) - 1,
                          yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapfilter_ad_hex: LDAPFILTER_AD_HEX ':' LDAP_FILTER {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKUTFLEN(ldapauthorisation->filter_AD,
                        $3,
                        sizeof(state->ldapauthorisation.filter_AD) - 1,
                        yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapattribute: LDAPATTRIBUTE ':' LDAP_ATTRIBUTE {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(ldapauthorisation->attribute,
                      $3,
                      sizeof(state->ldapauthorisation.attribute) - 1,
                      yyerrorx);

#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapattribute_ad: LDAPATTRIBUTE_AD ':' LDAP_ATTRIBUTE {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(ldapauthorisation->attribute_AD,
                      $3,
                      sizeof(state->ldapauthorisation.attribute_AD) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapattribute_hex: LDAPATTRIBUTE_HEX ':' LDAP_ATTRIBUTE {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKUTFLEN(ldapauthorisation->attribute,
                      $3,
                      sizeof(state->ldapauthorisation.attribute) -1,
                      yyerrorx);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapattribute_ad_hex: LDAPATTRIBUTE_AD_HEX ':' LDAP_ATTRIBUTE {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKUTFLEN(ldapauthorisation->attribute_AD,
                      $3,
                      sizeof(state->ldapauthorisation.attribute_AD) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lgroup_hex: LDAPGROUP_HEX ':' LDAPGROUP_NAME {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&rule.ldapgroup, hextoutf8($3, 0)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lgroup_hex_all: LDAPGROUP_HEX_ALL ':' LDAPGROUP_NAME {
#if SOCKS_SERVER
#if HAVE_LDAP
      checkmodule("ldap");

      if (addlinkedname(&rule.ldapgroup, hextoutf8($3, 1)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lgroup: LDAPGROUP ':' LDAPGROUP_NAME {
#if SOCKS_SERVER
#if HAVE_LDAP
      checkmodule("ldap");

      if (addlinkedname(&rule.ldapgroup, asciitoutf8($3)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

lserver: LDAPSERVER ':' LDAPSERVER_NAME {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapserver, $3) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapauthserver: LDAPAUTHSERVER ':' LDAPSERVER_NAME {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapserver, $3) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

ldapkeytab: LDAPKEYTAB ':' LDAPKEYTABNAME {
#if HAVE_LDAP
#if SOCKS_SERVER
   STRCPY_CHECKLEN(state->ldapauthorisation.keytab,
                   $3,
                   sizeof(state->ldapauthorisation.keytab) - 1, yyerrorx);
#else
   yyerrorx("LDAP keytab only applicable to Dante server");
#endif /* SOCKS_SERVER */
#else
      yyerrorx_nolib("LDAP");
#endif /* HAVE_LDAP */
   }
   ;

ldapauthkeytab: LDAPAUTHKEYTAB ':' LDAPKEYTABNAME {
#if HAVE_LDAP
#if SOCKS_SERVER
   STRCPY_CHECKLEN(state->ldapauthentication.keytab,
                   $3,
                   sizeof(state->ldapauthentication.keytab) - 1, yyerrorx);
#else
   yyerrorx("LDAP keytab only applicable to Dante server");
#endif /* SOCKS_SERVER */
#else
      yyerrorx_nolib("LDAP");
#endif /* HAVE_LDAP */
   }
   ;

psid: PACSID ':' PACSID_NAME {
#if SOCKS_SERVER
#if HAVE_PAC
      char b64[MAX_BASE64_LEN];

      checkmodule("pac");

      if (sidtob64($3, b64, sizeof(b64)) != 0)
         yyerrorx("invalid input: %s)", $3);
      if (addlinkedname(&rule.objectsids, b64) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("PAC");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

psid_b64: PACSID_B64 ':' PACSID_NAME {
#if SOCKS_SERVER
#if HAVE_PAC
      char sid[MAX_BASE64_LEN];
      checkmodule("pac");

      /* attempt conversion to check if input makes sense */
      if (b64tosid($3, sid, sizeof(sid)) != 0)
         yyerrorx("invalid input: %s)", $3);
      if (addlinkedname(&rule.objectsids, $3) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("PAC");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
   ;

psid_off: PACSID_FLAG ':' YES {
#if SOCKS_SERVER
#if HAVE_PAC
      checkmodule("pac");
      rule.pacoff = 1;
   }
   | PACSID_FLAG ':' NO {
      checkmodule("pac");
      rule.pacoff = 0;
#else /* !HAVE_PAC */
      yyerrorx_nolib("PAC");
#endif /* !HAVE_PAC */
#endif /* SOCKS_SERVER */
   }
   ;

clientcompatibility:   CLIENTCOMPATIBILITY ':' clientcompatibilitynames
   ;

clientcompatibilityname: NECGSSAPI {
#if HAVE_GSSAPI
      gssapiencryption->nec = 1;
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
   ;

clientcompatibilitynames:   clientcompatibilityname
   |                        clientcompatibilityname clientcompatibilitynames
   ;


verdict:   VERDICT_BLOCK {
#if !SOCKS_CLIENT
      ruleinit(&rule);
      rule.verdict   = VERDICT_BLOCK;
   }
   |   VERDICT_PASS {
      ruleinit(&rule);
      rule.verdict   = VERDICT_PASS;
#endif /* !SOCKS_CLIENT */
   }
   ;

command:   COMMAND ':' commands
   ;

commands:   commandname
   |        commandname commands
   ;

commandname:   COMMAND_BIND {
         state->command.bind = 1;
   }
   |   COMMAND_CONNECT {
         state->command.connect = 1;
   }
   |   COMMAND_UDPASSOCIATE {
         state->command.udpassociate = 1;
   }

   /* pseudocommands */

   |   COMMAND_BINDREPLY   {
         state->command.bindreply = 1;
   }

   |   COMMAND_UDPREPLY {
         state->command.udpreply = 1;
   }
   ;


protocol:   PROTOCOL ':'  protocols
   ;

protocols:   protocolname
   |   protocolname protocols
   ;

protocolname: PROTOCOL_TCP {
      state->protocol.tcp = 1;
   }
   |          PROTOCOL_UDP {
      state->protocol.udp = 1;
   }
   ;


fromto:   srcaddress dstaddress
   ;

hostid_fromto:   hostid_srcaddress dstaddress
   ;

redirect:   REDIRECT rdr_fromaddress rdr_toaddress
   |        REDIRECT rdr_fromaddress
   |        REDIRECT rdr_toaddress
   ;

sessionoption: sessionmax
   |           sessionthrottle
   |           sessionstate
   ;

sockssessionoption: sessionoption;
   ;

crulesessionoption: sessioninheritable
   |                sessionoption
   ;

sessioninheritable: SESSION_INHERITABLE ':' YES {
#if !SOCKS_CLIENT
                        rule.ss_isinheritable = 1;
   }
   | SESSION_INHERITABLE ':' NO {
                        rule.ss_isinheritable = 0;
#endif /* !SOCKS_CLIENT */
   }
   ;

sessionmax: SESSIONMAX ':' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_MAXSESSIONS($3, ss.object.ss.max, 0);
      ss.object.ss.max       = $3;
      ss.object.ss.max_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;

sessionthrottle: SESSIONTHROTTLE ':' NUMBER '/' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_THROTTLE_SECONDS($3, ss.object.ss.throttle.limit.clients, 0);
      ASSIGN_THROTTLE_CLIENTS($5, ss.object.ss.throttle.limit.seconds, 0);
      ss.object.ss.throttle_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;

sessionstate: sessionstate_key
   |          sessionstate_keyinfo
   |          sessionstate_throttle
   |          sessionstate_max
   ;

sessionstate_key: SESSIONSTATE_KEY ':' STATEKEY {
#if !SOCKS_CLIENT
      if ((ss.keystate.key = string2statekey($3)) == key_unset)
         yyerrorx("%s is not a valid state key", $3);

      if (ss.keystate.key == key_hostid) {
#if HAVE_SOCKS_HOSTID

         *hostidoption_isset           = 1;
         ss.keystate.keyinfo.hostindex = DEFAULT_HOSTINDEX;

#else /* !HAVE_SOCKS_HOSTID */

         yyerrorx("hostid is not supported on this system");

#endif /* HAVE_SOCKS_HOSTID */
      }




#else /* SOCKS_CLIENT */

   SERRX(0);
#endif /* SOCKS_CLIENT */
   }
   ;

sessionstate_keyinfo: SESSIONSTATE_KEY '.' {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
      hostindex = &ss.keystate.keyinfo.hostindex;
   }
   hostindex {
      hostindex = &rule.hostindex; /* reset */
#endif /* !SOCKS_CLIENT && HAVE_SOCKS_HOSTID */
   }
   ;


sessionstate_max: SESSIONSTATE_MAX ':' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_MAXSESSIONS($3, ss.object.ss.max_perstate, 0);
      ss.object.ss.max_perstate_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;

sessionstate_throttle: SESSIONSTATE_THROTTLE ':' NUMBER '/' NUMBER {
#if !SOCKS_CLIENT
   ASSIGN_THROTTLE_SECONDS($3, ss.object.ss.throttle_perstate.limit.clients, 0);
   ASSIGN_THROTTLE_CLIENTS($5, ss.object.ss.throttle_perstate.limit.seconds, 0);
   ss.object.ss.throttle_perstate_isset = 1;
#endif /* !SOCKS_CLIENT */
}


bandwidth:   BANDWIDTH ':' NUMBER {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER($3, >=, 0, bw.object.bw.maxbps, 0);
      bw.object.bw.maxbps_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;


log:   RULE_LOG ':' logs
   ;

logname:  RULE_LOG_CONNECT {
#if !SOCKS_CLIENT
         rule.log.connect = 1;
   }
   |   RULE_LOG_DATA {
         rule.log.data = 1;
   }
   |   RULE_LOG_DISCONNECT {
         rule.log.disconnect = 1;
   }
   |   RULE_LOG_ERROR {
         rule.log.error = 1;
   }
   |   RULE_LOG_IOOPERATION {
         rule.log.iooperation = 1;
   }
   |   RULE_LOG_TCPINFO {
         rule.log.tcpinfo = 1;
#endif /* !SOCKS_CLIENT */
   }
   ;

logs:   logname
   |  logname logs
   ;


pamservicename: PAMSERVICENAME ':' SERVICENAME {
#if HAVE_PAM && (!SOCKS_CLIENT)
      STRCPY_CHECKLEN(state->pamservicename,
                      $3,
                      sizeof(state->pamservicename) -1,
                      yyerrorx);
#else
      yyerrorx_nolib("PAM");
#endif /* HAVE_PAM && (!SOCKS_CLIENT) */
   }
   ;

bsdauthstylename: BSDAUTHSTYLE ':' BSDAUTHSTYLENAME {
#if HAVE_BSDAUTH && SOCKS_SERVER
      STRCPY_CHECKLEN(state->bsdauthstylename,
                      $3,
                      sizeof(state->bsdauthstylename) - 1,
                      yyerrorx);
#else
      yyerrorx_nolib("bsdauth");
#endif /* HAVE_BSDAUTH && SOCKS_SERVER */
   }
   ;


gssapiservicename: GSSAPISERVICE ':' GSSAPISERVICENAME {
#if HAVE_GSSAPI
      STRCPY_CHECKLEN(gssapiservicename,
                      $3,
                      sizeof(state->gssapiservicename) - 1,
                      yyerrorx);
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
   ;

gssapikeytab: GSSAPIKEYTAB ':' GSSAPIKEYTABNAME {
#if HAVE_GSSAPI
#if SOCKS_SERVER
      STRCPY_CHECKLEN(gssapikeytab,
                       $3,
                       sizeof(state->gssapikeytab) - 1,
                       yyerrorx);
#else
      yyerrorx("gssapi keytab setting is only applicable to Dante server");
#endif /* SOCKS_SERVER */
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
   ;

gssapienctype: GSSAPIENCTYPE':' gssapienctypes
   ;

gssapienctypename: GSSAPIENC_ANY {
#if HAVE_GSSAPI
      gssapiencryption->clear           = 1;
      gssapiencryption->integrity       = 1;
      gssapiencryption->confidentiality = 1;
   }
   |  GSSAPIENC_CLEAR {
      gssapiencryption->clear = 1;
   }
   |  GSSAPIENC_INTEGRITY {
      gssapiencryption->integrity = 1;
   }
   |  GSSAPIENC_CONFIDENTIALITY {
      gssapiencryption->confidentiality = 1;
   }
   |  GSSAPIENC_PERMESSAGE {
      yyerrorx("gssapi per-message encryption not supported");
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
   ;

gssapienctypes: gssapienctypename
   |  gssapienctypename gssapienctypes
   ;

bounce: BOUNCE bounceto ':' bouncetoaddress
   ;

libwrap:   LIBWRAPSTART ':' LINE {
#if HAVE_LIBWRAP && (!SOCKS_CLIENT)
      struct request_info request;
      char tmp[LIBWRAPBUF];
      int errno_s, devnull;

      STRCPY_CHECKLEN(rule.libwrap,
                      $3,
                      sizeof(rule.libwrap) - 1,
                      yyerrorx);

      /* libwrap modifies the passed buffer, to test with a tmp one. */
      STRCPY_ASSERTSIZE(tmp, rule.libwrap);

      devnull = open("/dev/null", O_RDWR, 0);
      ++dry_run;
      errno_s = errno;

      errno = 0;

      request_init(&request, RQ_FILE, devnull, RQ_DAEMON, __progname, 0);
      if (setjmp(tcpd_buf) != 0)
         yyerror("bad libwrap line");
      process_options(tmp, &request);

      if (errno != 0)
         yywarn("possible libwrap/tcp-wrappers related configuration error");

      --dry_run;
      close(devnull);
      errno = errno_s;

#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_LIBWRAP && (!SOCKS_CLIENT) */

   }
   ;


srcaddress:   from ':' address
   ;

hostid_srcaddress:   from ':' address_without_port
   ;

dstaddress:   to ':' address
   ;

rdr_fromaddress: rdr_from ':' address
   ;

rdr_toaddress: rdr_to ':' address {
#if BAREFOOTD
      yyerrorx("redirecting \"to\" an address does not make any sense in %s.  "
               "Instead specify the address you wanted to \"redirect\" "
               "data to as the \"bounce to\" address, as normal",
               PRODUCT);
#endif /* BAREFOOT */
   }
   ;

gateway:   via ':' gwaddress
   ;

routeoption: routemethod
   |         command
   |         clientcompatibility
   |         extension
   |         protocol
   |         gssapiservicename
   |         gssapikeytab
   |         gssapienctype
   |         proxyprotocol
   |         REDIRECT rdr_fromaddress
   |         socketoption {
               if (!addedsocketoption(&route.socketoptionc,
                                      &route.socketoptionv,
                                      &socketopt))
                  yywarn("could not add socketoption");
   }
   ;

routeoptions:  { $$ = NULL; }
   |           routeoption routeoptions
   ;

routemethod: METHOD ':' socksmethods
   ;

from:   FROM {
      addrinit(&src, 1);
   }
   ;

to:   TO {
      addrinit(&dst, ipaddr_requires_netmask(to, objecttype));
   }
   ;

rdr_from:   FROM {
      addrinit(&rdr_from, 1);
   }
   ;

rdr_to:   TO {
      addrinit(&rdr_to, 0);
   }
   ;

bounceto:   TO {
#if BAREFOOTD
      addrinit(&bounceto, 0);
#endif /* BAREFOOTD */
   }
   ;


via:   VIA {
      gwaddrinit(&gw);
   }
   ;

externaladdress:  ipv4
   |              ipv6
   |              domain
   |              ifname
   ;

address_without_port: ipaddress
   |                  domain
   |                  ifname
   ;

address: address_without_port port
   ;


ipaddress: ipv4 '/' netmask_v4 { if (!netmask_required) yyerrorx_hasnetmask(); }
   |       ipv4                { if (netmask_required)  yyerrorx_nonetmask();  }
   |       ipv6 '/' netmask_v6 { if (!netmask_required) yyerrorx_hasnetmask(); }
   |       ipv6                { if (netmask_required)  yyerrorx_nonetmask();  }
   |       ipvany '/' netmask_vany { if (!netmask_required)
                                       yyerrorx_hasnetmask(); }
   |       ipvany              { if (netmask_required)  yyerrorx_nonetmask();  }

gwaddress:   ipaddress    gwport
   |         domain       gwport
   |         ifname { /* for upnp; broadcasts on interface. */ }
   |         url
   ;

bouncetoaddress:   ipaddress  gwport
   |               domain     gwport
   ;


ipv4:   IPV4 {
      *atype = SOCKS_ADDR_IPV4;

      if (socks_inet_pton(AF_INET, $1, ipv4, NULL) != 1)
         yyerror("bad %s: %s", atype2string(*atype), $1);
   }
   ;

netmask_v4:   NUMBER {
      if ($1 < 0 || $1 > 32)
         yyerrorx("bad %s netmask: %ld.  Legal range is 0 - 32",
                  atype2string(*atype), (long)$1);

      netmask_v4->s_addr = $1 == 0 ? 0 : htonl(IPV4_FULLNETMASK << (32 - $1));
   }
   |          IPV4 {
      if (socks_inet_pton(AF_INET, $1, netmask_v4, NULL) != 1)
         yyerror("bad %s netmask: %s", atype2string(*atype), $1);
   }
   ;

ipv6:   IPV6 {
      *atype = SOCKS_ADDR_IPV6;

      if (socks_inet_pton(AF_INET6, $1, ipv6, scopeid_v6) != 1)
         yyerror("bad %s: %s", atype2string(*atype), $1);
   }
   ;

netmask_v6:   NUMBER {
      if ($1 < 0 || $1 > IPV6_NETMASKBITS)
         yyerrorx("bad %s netmask: %d.  Legal range is 0 - %d",
                  atype2string(*atype), (int)$1, IPV6_NETMASKBITS);

      *netmask_v6 = $1;
   }
   ;

ipvany:   IPVANY {
      SASSERTX(strcmp($1, "0") == 0);

      *atype = SOCKS_ADDR_IPVANY;
      ipvany->s_addr = htonl(0);
   }
   ;

netmask_vany:   NUMBER {
      if ($1 != 0)
         yyerrorx("bad %s netmask: %d.  Only legal value is 0",
                  atype2string(*atype), (int)$1);

      netmask_vany->s_addr = htonl($1);
   }
   ;


domain:   DOMAINNAME {
      *atype = SOCKS_ADDR_DOMAIN;
      STRCPY_CHECKLEN(domain, $1, MAXHOSTNAMELEN - 1, yyerrorx);
   }
   ;

ifname:   IFNAME {
      *atype = SOCKS_ADDR_IFNAME;
      STRCPY_CHECKLEN(ifname, $1, MAXIFNAMELEN - 1, yyerrorx);
   }
   ;


url:   URL {
      *atype = SOCKS_ADDR_URL;
      STRCPY_CHECKLEN(url, $1, MAXURLLEN - 1, yyerrorx);
   }
   ;


port: { $$ = 0; }
   |   PORT ':' portnumber
   |   PORT portoperator portnumber
   |   PORT portrange
   ;

gwport: { $$ = 0; }
   |  PORT portoperator portnumber
   ;

portnumber:   portservice
   |   portstart
   ;

portrange:   portstart '-' portend {
   if (ntohs(*port_tcp) > ntohs(ruleaddr->portend))
      yyerrorx("end port (%u) can not be less than start port (%u)",
      ntohs(*port_tcp), ntohs(ruleaddr->portend));
   }
   ;


portstart:   NUMBER {
      ASSIGN_PORTNUMBER($1, *port_tcp);
      ASSIGN_PORTNUMBER($1, *port_udp);
   }
   ;

portend:   NUMBER {
      ASSIGN_PORTNUMBER($1, ruleaddr->portend);
      ruleaddr->operator   = range;
   }
   ;

portservice:   SERVICENAME {
      struct servent   *service;

      if ((service = getservbyname($1, "tcp")) == NULL) {
         if (state->protocol.tcp)
            yyerrorx("unknown tcp protocol: %s", $1);

         *port_tcp = htons(0);
      }
      else
         *port_tcp = (in_port_t)service->s_port;

      if ((service = getservbyname($1, "udp")) == NULL) {
         if (state->protocol.udp)
               yyerrorx("unknown udp protocol: %s", $1);

            *port_udp = htons(0);
      }
      else
         *port_udp = (in_port_t)service->s_port;

      if (*port_tcp == htons(0) && *port_udp == htons(0))
         yyerrorx("unknown tcp/udp protocol");

      /* if one protocol is unset, set to same as the other. */
      if (*port_tcp == htons(0))
         *port_tcp = *port_udp;
      else if (*port_udp == htons(0))
         *port_udp = *port_tcp;

      $$ = (size_t)*port_udp;
   }
   ;


portoperator:   OPERATOR {
      *operator = string2operator($1);
   }
   ;

   /* XXX should support <operator> <range end> also. */
udpportrange: UDPPORTRANGE ':' udpportrange_start '-' udpportrange_end
   ;

udpportrange_start: NUMBER {
#if SOCKS_SERVER
   ASSIGN_PORTNUMBER($1, rule.udprange.start);
#endif /* SOCKS_SERVER */
   }
   ;

udpportrange_end: NUMBER {
#if SOCKS_SERVER
   ASSIGN_PORTNUMBER($1, rule.udprange.end);
   rule.udprange.op  = range;

   if (ntohs(rule.udprange.start) > ntohs(rule.udprange.end))
      yyerrorx("end port (%d) can not be less than start port (%u)",
               (int)$1, ntohs(rule.udprange.start));
#endif /* SOCKS_SERVER */
   }
   ;

number: NUMBER {
      addnumber(&numberc, &numberv, $1);
   }
   ;

numbers: number
   | number numbers
   ;


%%

#define INTERACTIVE      0

extern FILE *yyin;

int lex_dorestart; /* global for Lex. */

int
parseconfig(filename)
   const char *filename;
{
   struct stat statbuf;
   int haveconfig;

#if SOCKS_CLIENT /* assume server admin can set things up correctly himself. */
   parseclientenv(&haveconfig);

   if (haveconfig)
      return 0;

#else /* !SOCKS_CLIENT */
   SASSERTX(pidismainmother(sockscf.state.pid));

   if (sockscf.state.inited)
      /* in case we need something special to (re)open config-file. */
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
#endif /* !SOCKS_CLIENT */

   yyin = fopen(filename, "r");

#if !SOCKS_CLIENT
   if (sockscf.state.inited)
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);
#endif /* SERVER */

   if (yyin == NULL
   ||  (stat(filename, &statbuf) == 0 && statbuf.st_size == 0)) {
      if (yyin == NULL)
         slog(sockscf.state.inited ? LOG_WARNING : LOG_ERR,
              "%s: could not open config file %s", function, filename);
      else
         slog((sockscf.state.inited || SOCKS_CLIENT) ? LOG_WARNING : LOG_ERR,
              "%s: config file %s is empty.  Not parsing", function, filename);

#if SOCKS_CLIENT

      if (yyin == NULL) {
         if (sockscf.option.directfallback)
            slog(LOG_DEBUG,
                 "%s: no %s, but direct fallback enabled, continuing",
                 function, filename);
         else
            exit(0);
      }
      else {
         slog(LOG_DEBUG, "%s: empty %s, assuming direct fallback wanted",
              function, filename);

         sockscf.option.directfallback = 1;
      }

      SASSERTX(sockscf.option.directfallback == 1);
#else /* !SOCKS_CLIENT */

      if (!sockscf.state.inited)
         sockdexit(EXIT_FAILURE);

      /*
       * Might possibly continue with old config.
       */

#endif /* !SOCKS_CLIENT */

      haveconfig = 0;
   }
   else {
#if YYDEBUG
      yydebug       = 0;
#endif /* YYDEBUG */

      yylineno      = 1;
      errno         = 0;   /* don't report old errors in yyparse(). */
      haveconfig    = 1;

      /*
       * Special and delayed as long as we can, till immediately before
       * parsing new config.
       * Want to keep a backup of old ones until we know there were no
       * errors adding new logfiles.
       */

#if !SOCKS_CLIENT
      old_log              = sockscf.log;
      old_errlog           = sockscf.errlog;
#endif /* !SOCKS_CLIENT */

      failed_to_add_errlog = failed_to_add_log = 0;

      slog(LOG_DEBUG, "%s: parsing config in file %s", function, filename);

      bzero(&sockscf.log,    sizeof(sockscf.log));
      bzero(&sockscf.errlog, sizeof(sockscf.errlog));

      lex_dorestart = 1;

      parsingconfig = 1;

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      yyparse();

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      parsingconfig = 0;

#if !SOCKS_CLIENT
      CMDLINE_OVERRIDE(&sockscf.initial.cmdline, &sockscf.option);

#if !HAVE_PRIVILEGES
      if (!sockscf.state.inited) {
         /*
          * first time.
          */
         if (sockscf.uid.privileged_isset && !sockscf.option.verifyonly) {
            /*
             * If we created any logfiles (rather than just opened already
             * existing ones), they will have been created with the euid/egid
             * we are started with.  If logfiles created by that euid/egid are
             * not writable by our configured privileged userid (if any), it
             * means that upon SIGHUP we will be unable to re-open our own
             * logfiles.  We therefor check whether the logfile(s) were created
             * by ourselves, and if so, make sure they have the right owner.
             */
            logtype_t *logv[] = { &sockscf.log, &sockscf.errlog };
            size_t i;

            for (i = 0; i < ELEMENTS(logv); ++i) {
               size_t fi;

               for (fi = 0; fi < logv[i]->filenoc; ++fi) {
                  if (logv[i]->createdv[fi]) {
                     slog(LOG_DEBUG,
                          "%s: chown(2)-ing created logfile %s to %lu/%lu",
                          function,
                          logv[i]->fnamev[fi],
                          (unsigned long)sockscf.uid.privileged_uid,
                          (unsigned long)sockscf.uid.privileged_gid);

                     if (fchown(logv[i]->filenov[fi],
                                (unsigned long)sockscf.uid.privileged_uid,
                                (unsigned long)sockscf.uid.privileged_gid) != 0)
                        serr("%s: could not fchown(2) created logfile %s to "
                             "privileged uid/gid %lu/%lu.  This means that "
                             "upon SIGHUP, we would not be unable to re-open "
                             "our own logfiles.  This should not happen",
                             function,
                             logv[i]->fnamev[fi],
                             (unsigned long)sockscf.uid.privileged_uid,
                             (unsigned long)sockscf.uid.privileged_gid);
                  }
               }
            }
         }
      }
#endif /* !HAVE_PRIVILEGES */

      if (configure_privileges() != 0) {
         if (sockscf.state.inited) {
            swarn("%s: could not reinitialize privileges after SIGHUP.  "
                  "Will continue without privileges",
                  function);

            sockscf.state.haveprivs = 0;
         }
         else
            serr("%s: could not configure privileges", function);
      }
#endif /* !SOCKS_CLIENT */
   }

   if (yyin != NULL)
      fclose(yyin);

   errno = 0;
   return haveconfig ? 0 : -1;
}

static int
ipaddr_requires_netmask(context, type)
   const addresscontext_t context;
   const objecttype_t type;
{

   switch (type) {
      case object_crule:
#if HAVE_SOCKS_RULES

         return 1;

#else /* !HAVE_SOCKS_RULES */

         switch (context) {
            case from:
               return 1;

            case to:
               return 0; /* address we accept clients on. */

            case bounce:
               return 0; /* address we connect to.        */

            default:
               SERRX(context);
         }
#endif /* !HAVE_SOCKS_RULES */


#if HAVE_SOCKS_HOSTID
      case object_hrule:
         return 1;
#endif /* HAVE_SOCKS_HOSTID */

#if HAVE_SOCKS_RULES
      case object_srule:
         return 1;
#endif /* HAVE_SOCKS_RULES */

      case object_route:
      case object_monitor:
         return 1;

      default:
         SERRX(type);
   }


   /* NOTREACHED */
   return 0;
}


static void
addnumber(nc, nv, number)
   size_t *nc;
   long long *nv[];
   const long long number;
{
   const char *_function = "addnumber()";

   if ((*nv = realloc(*nv, sizeof(**nv) * ((*nc) + 1)))
   == NULL)
      yyerror("%s: could not allocate %lu bytes of memory for adding "
              "number %lld",
              _function, (unsigned long)(sizeof(**nv) * ((*nc) + 1)),
              number);

   (*nv)[(*nc)++] = number;
}


static void
addrinit(addr, _netmask_required)
   ruleaddr_t *addr;
   const int _netmask_required;
{

   atype            = &addr->atype;

   ipv4             = &addr->addr.ipv4.ip;
   netmask_v4       = &addr->addr.ipv4.mask;

   ipv6             = &addr->addr.ipv6.ip;
   netmask_v6       = &addr->addr.ipv6.maskbits;
   scopeid_v6       = &addr->addr.ipv6.scopeid;

   ipvany           = &addr->addr.ipvany.ip;
   netmask_vany     = &addr->addr.ipvany.mask;

   if (!_netmask_required) {
      netmask_v4->s_addr   = htonl(IPV4_FULLNETMASK);
      *netmask_v6          = IPV6_NETMASKBITS;
      netmask_vany->s_addr = htonl(IPV4_FULLNETMASK);
   }

   domain           = addr->addr.domain;
   ifname           = addr->addr.ifname;

   port_tcp         = &addr->port.tcp;
   port_udp         = &addr->port.udp;
   operator         = &addr->operator;

   netmask_required = _netmask_required;
   ruleaddr         = addr;
}

static void
gwaddrinit(addr)
   sockshost_t *addr;
{
   static enum operator_t operatormem;

   netmask_required = 0;

   atype            = &addr->atype;

   ipv4             = &addr->addr.ipv4;
   ipv6             = &addr->addr.ipv6.ip;
   domain           = addr->addr.domain;
   ifname           = addr->addr.ifname;
   url              = addr->addr.urlname;

   port_tcp         = &addr->port;
   port_udp         = &addr->port;
   operator         = &operatormem; /* no operator in gwaddr and not used. */
}

static void
routeinit(r)
   route_t *r;
{
   bzero(r, sizeof(*r));

   state               = &r->gw.state;
   extension           = &state->extension;

   cmethodv            = state->cmethodv;
   cmethodc            = &state->cmethodc;
   smethodv            = state->smethodv;
   smethodc            = &state->smethodc;

#if HAVE_GSSAPI
   gssapiservicename = state->gssapiservicename;
   gssapikeytab      = state->gssapikeytab;
   gssapiencryption  = &state->gssapiencryption;
#endif /* HAVE_GSSAPI */

#if !SOCKS_CLIENT && HAVE_LDAP
   ldapauthorisation              = &state->ldapauthorisation;
   ldapauthentication          = &state->ldapauthentication;
#endif /* !SOCKS_CLIENT && HAVE_LDAP*/

   bzero(&src, sizeof(src));
   bzero(&dst, sizeof(dst));
   src.atype = SOCKS_ADDR_IPV4;
   dst.atype = SOCKS_ADDR_IPV4;

   bzero(&gw, sizeof(gw));
   bzero(&rdr_from, sizeof(rdr_from));
   bzero(&hostid, sizeof(hostid));
}


#if SOCKS_CLIENT
static void
parseclientenv(haveproxyserver)
   int *haveproxyserver;
{
   const char *function = "parseclientenv()";
   const char *fprintf_error = "could not write to tmpfile used to hold "
                               "settings set in environment for parsing";
   const char *p;
   size_t i;
   FILE *fp;
   char rdr_from[512], extrarouteinfo[sizeof(rdr_from) + sizeof("\n")],
        gw[MAXSOCKSHOSTLEN + sizeof(" port = 65535")];
   int fd;


#if 1

#if SOCKS_CLIENT
   p = "yaccenv-client-XXXXXX";
#else /* !SOCKS_CLIENT */
   p = "yaccenv-server-XXXXXX";
#endif /* !SOCKS_CLIENT */

   if ((fd = socks_mklock(p, NULL, 0)) == -1)
      yyerror("socks_mklock() failed to create tmpfile using base %s", p);

#else /* for debugging file-generation problems. */
   if ((fd = open("/tmp/dante-envfile",
                  O_CREAT | O_TRUNC | O_RDWR,
                  S_IRUSR | S_IWUSR)) == -1)
      serr("%s: could not open file", function);
#endif

   if ((fp = fdopen(fd, "r+")) == NULL)
      serr("%s: fdopen(fd %d) failed", function, fd);

   if ((p = socks_getenv(ENV_SOCKS_LOGOUTPUT, dontcare)) != NULL && *p != NUL)
      if (fprintf(fp, "logoutput: %s\n", p) == -1)
         serr("%s: %s", function, fprintf_error);

   if ((p = socks_getenv(ENV_SOCKS_ERRLOGOUTPUT, dontcare)) != NULL
   && *p != NUL)
      if (fprintf(fp, "errorlog: %s\n", p) == -1)
         serr("%s: %s", function, fprintf_error);

   if ((p = socks_getenv(ENV_SOCKS_DEBUG, dontcare)) != NULL && *p != NUL)
      if (fprintf(fp, "debug: %s\n", p) == -1)
         serr("%s: %s", function, fprintf_error);

   *rdr_from = NUL;
   if ((p = socks_getenv(ENV_SOCKS_REDIRECT_FROM, dontcare)) != NULL
   && *p != NUL) {
      const char *prefix = "redirect from";

      if (strlen(prefix) + strlen(p) + 1 > sizeof(rdr_from))
         serr("%s: %s value is too long.  Max length is %lu",
              function,
              ENV_SOCKS_REDIRECT_FROM,
              (unsigned long)sizeof(rdr_from) - (strlen(prefix) + 1));

      snprintf(rdr_from, sizeof(rdr_from), "%s: %s\n", prefix, p);
   }

   snprintf(extrarouteinfo, sizeof(extrarouteinfo),
            "%s", rdr_from);

   /*
    * Check if there is a proxy server configured in the environment.
    * Initially assume there is none.
    */

   *haveproxyserver = 0;

   i = 1;
   while (1) {
      /* 640 routes should be enough for anyone. */
      char name[sizeof(ENV_SOCKS_ROUTE_) + sizeof("640")];

      snprintf(name, sizeof(name), "%s%lu", ENV_SOCKS_ROUTE_, (unsigned long)i);

      if ((p = socks_getenv(name, dontcare)) == NULL)
         break;

      if (*p != NUL) {
         if (fprintf(fp, "route { %s }\n", p) == -1)
            serr("%s: %s", function, fprintf_error);

         *haveproxyserver = 1;
      }

      ++i;
   }

   if ((p = socks_getenv(ENV_SOCKS4_SERVER, dontcare)) != NULL && *p != NUL) {
      if (fprintf(fp,
"route {\n"
"         from: 0.0.0.0/0 to: 0.0.0.0/0 via: %s\n"
"         proxyprotocol: socks_v4\n"
"         %s"
"}\n",            serverstring2gwstring(p, PROXY_SOCKS_V4, gw, sizeof(gw)),
                  extrarouteinfo) == -1)
         serr("%s: %s", function, fprintf_error);

      *haveproxyserver = 1;
   }

   if ((p = socks_getenv(ENV_SOCKS5_SERVER, dontcare)) != NULL && *p != NUL) {
      if (fprintf(fp,
"route {\n"
"         from: 0.0.0.0/0 to: 0.0.0.0/0 via: %s\n"
"         proxyprotocol: socks_v5\n"
"         %s"
"}\n",            serverstring2gwstring(p, PROXY_SOCKS_V5, gw, sizeof(gw)),
                  extrarouteinfo) == -1)
         serr("%s: %s", function, fprintf_error);

      *haveproxyserver = 1;
   }

   if ((p = socks_getenv(ENV_SOCKS_SERVER, dontcare)) != NULL && *p != NUL) {
      if (fprintf(fp,
"route {\n"
"         from: 0.0.0.0/0 to: 0.0.0.0/0 via: %s\n"
"         %s"
"}\n",            serverstring2gwstring(p, PROXY_SOCKS_V5, gw, sizeof(gw)),
                  extrarouteinfo) == -1)
         serr("%s: %s", function, fprintf_error);

      *haveproxyserver = 1;
   }

   if ((p = socks_getenv(ENV_HTTP_PROXY, dontcare)) != NULL && *p != NUL) {
      struct sockaddr_storage sa;
      int gaierr;
      char emsg[512];

      if (urlstring2sockaddr(p, &sa, &gaierr, emsg, sizeof(emsg)) == NULL)
         serr("%s: could not convert to %s to an Internet address",
              function, p);

      if (fprintf(fp,
"route {\n"
"         from: 0.0.0.0/0 to: 0.0.0.0/0 via: %s port = %d\n"
"         proxyprotocol: http_v1.0\n"
"         %s"
"}\n",
                  sockaddr2string2(&sa, 0, NULL, 0),
                  ntohs(GET_SOCKADDRPORT(&sa)),
                  extrarouteinfo)
      == -1)
         serr("%s: %s", function, fprintf_error);

      *haveproxyserver = 1;
   }

   if ((p = socks_getenv(ENV_UPNP_IGD, dontcare)) != NULL && *p != NUL) {
      if (fprintf(fp,
"route {\n"
"         from: 0.0.0.0/0 to: 0.0.0.0/0 via: %s\n"
"         proxyprotocol: upnp\n"
"         %s"
"}\n",            p, extrarouteinfo) == -1)
         serr("%s: %s", function, fprintf_error);

      *haveproxyserver = 1;
   }


   /*
    * End of possible settings we want to parse with yacc/lex.
    */

   if (fseek(fp, 0, SEEK_SET) != 0)
      yyerror("fseek(3) on tmpfile used to hold environment-settings failed");

   yyin = fp;

   lex_dorestart             = 1;
   parsingconfig             = 1;
   p                         = sockscf.option.configfile;
   sockscf.option.configfile = "<generated socks.conf>";

#if SOCKSLIBRARY_DYNAMIC
   socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

   yyparse();

#if SOCKSLIBRARY_DYNAMIC
   socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

   sockscf.option.configfile = p;
   parsingconfig             = 0;

   fclose(fp);

   if (socks_getenv(ENV_SOCKS_AUTOADD_LANROUTES, isfalse) == NULL) {
      /*
       * assume it's good to add direct routes for the lan also.
       */
      struct ifaddrs *ifap;

      slog(LOG_DEBUG, "%s: auto-adding direct routes for lan ...", function);

      if (getifaddrs(&ifap) == 0) {
         command_t commands;
         protocol_t protocols;
         struct ifaddrs *iface;

         bzero(&commands, sizeof(commands));
         bzero(&protocols, sizeof(protocols));

         protocols.tcp = 1;
         protocols.udp = 1;

         commands.connect      = 1;
         commands.udpassociate = 1;

         for (iface = ifap; iface != NULL; iface = iface->ifa_next)
            if (iface->ifa_addr            != NULL
            &&  iface->ifa_addr->sa_family == AF_INET) {
               if (iface->ifa_netmask == NULL) {
                  swarn("interface %s missing netmask, skipping",
                        iface->ifa_name);
                  continue;
               }

               socks_autoadd_directroute(&commands,
                                         &protocols,
                                         TOCSS(iface->ifa_addr),
                                         TOCSS(iface->ifa_netmask));
            }

         freeifaddrs(ifap);
      }
   }
   else
      slog(LOG_DEBUG, "%s: not auto-adding direct routes for lan", function);
}

static char *
serverstring2gwstring(serverstring, version, gw, gwsize)
   const char *serverstring;
   const int version;
   char *gw;
   const size_t gwsize;
{
   const char *function = "serverstring2gwstring()";
   char *sep, emsg[256];

   if (version != PROXY_SOCKS_V4 && version != PROXY_SOCKS_V5)
      return gw; /* should be in desired format already. */

   if (strlen(serverstring) >= gwsize)
      serrx("%s: value of proxyserver (%s) set in environment is too long.  "
            "Max length is %lu",
            function, serverstring, (unsigned long)(gwsize - 1));

   if ((sep = strrchr(serverstring, ':')) != NULL && *(sep + 1) != NUL) {
      long port;

      if ((port = string2portnumber(sep + 1, emsg, sizeof(emsg))) == -1)
         yyerrorx("%s: %s", function, emsg);

      memcpy(gw, serverstring, sep - serverstring);
      snprintf(&gw[sep - serverstring],
               gwsize - (sep - serverstring),
               " port = %u",
               (in_port_t)port);
   }
   else {
      char visbuf[256];

      yyerrorx("%s: could not find portnumber in %s serverstring \"%s\"",
               function,
               proxyprotocol2string(version),
               str2vis(sep == NULL ? serverstring : sep,
                       strlen(sep == NULL ? serverstring : sep),
                       visbuf,
                       sizeof(visbuf)));
   }

   return gw;
}

#else /* !SOCKS_CLIENT */

static void
pre_addrule(rule)
   rule_t *rule;
{

   rule->src   = src;
   rule->dst   = dst;

#if HAVE_SOCKS_HOSTID
   rule->hostid      = hostid;
#endif /* HAVE_SOCKS_HOSTID */

   rule->rdr_from    = rdr_from;
   rule->rdr_to      = rdr_to;

   if (session_isset) {
      if ((rule->ss = malloc(sizeof(*rule->ss))) == NULL)
         yyerror("failed to malloc(3) %lu bytes for session memory",
                 (unsigned long)sizeof(*rule->ss));

      *rule->ss = ss;
   }

   if (bw_isset) {
      if ((rule->bw = malloc(sizeof(*rule->bw))) == NULL)
         yyerror("failed to malloc(3) %lu bytes for bw memory",
                 (unsigned long)sizeof(*rule->bw));

      *rule->bw = bw;
   }
}


static void
post_addrule(void)
{

   timeout = &sockscf.timeout; /* default is global timeout, unless in a rule */
}

static void
ruleinit(rule)
   rule_t *rule;
{
   bzero(rule, sizeof(*rule));

   rule->linenumber  = yylineno;

#if HAVE_SOCKS_HOSTID

   rule->hostindex          = DEFAULT_HOSTINDEX;
   hostindex                = &rule->hostindex;

   rule->hostidoption_isset = 0;
   hostidoption_isset       = &rule->hostidoption_isset;

#endif /* HAVE_SOCKS_HOSTID */

   state          = &rule->state;

   cmethodv       = state->cmethodv;
   cmethodc       = &state->cmethodc;

   smethodv       = state->smethodv;
   smethodc       = &state->smethodc;

   /*
    * default values: same as global.
    */

   timeout       = &rule->timeout;
   *timeout      = sockscf.timeout;

#if HAVE_GSSAPI

   gssapiservicename = state->gssapiservicename;
   gssapikeytab      = state->gssapikeytab;
   gssapiencryption  = &state->gssapiencryption;

#endif /* HAVE_GSSAPI */

#if HAVE_LDAP

   ldapauthorisation              = &state->ldapauthorisation;
   ldapauthentication             = &state->ldapauthentication;

   /*
    * Common attribute settings, LDAP authentication/authorisation. 
    */

   ldapauthorisation->auto_off    = ldapauthentication->auto_off  = -1;
   ldapauthorisation->certcheck   = ldapauthentication->certcheck = -1;

   ldapauthorisation->debug       = ldapauthentication->debug
   = LDAP_UNSET_DEBUG_VALUE;

   ldapauthorisation->keeprealm   = ldapauthorisation->keeprealm  = -1;
   ldapauthorisation->port        = ldapauthentication->port      = -1;
   ldapauthorisation->portssl     = ldapauthentication->portssl   = -1;
   ldapauthorisation->ssl         = ldapauthentication->ssl       = -1;

   /*
    * Only in LDAP authorisation.
    */
   ldapauthorisation->mdepth                                      = -1;

   /*
    * Rest should be char arrays and NUL already due to bzero(3).
    */

#endif /* HAVE_LDAP */

#if HAVE_PAC

   rule->objectsids  = NULL;
   rule->pacoff      = 1;

#endif /* HAVE_PAC */

   bzero(&src, sizeof(src));
   bzero(&dst, sizeof(dst));
   bzero(&hostid, sizeof(hostid));

   bzero(&rdr_from, sizeof(rdr_from));
   bzero(&rdr_to, sizeof(rdr_to));

#if BAREFOOTD
   bzero(&bounceto, sizeof(bounceto));
#endif /* BAREFOOTD */

   rule->bw_isinheritable   = rule->ss_isinheritable = 1;

   bzero(&ss, sizeof(ss));
   bzero(&bw, sizeof(bw));

   bw_isset = session_isset = 0;
   bw.type  = SHMEM_BW;
   ss.type  = SHMEM_SS;
}

void
alarminit(void)
{
    static int alarmside_mem;

   alarmside  = &alarmside_mem;
   *alarmside = 0;
}

static void
monitorinit(monitor)
   monitor_t *monitor;
{
   static int alarmside_mem;

   alarmside = &alarmside_mem;

   bzero(monitor, sizeof(*monitor));

   monitor->linenumber = yylineno;

   state                       = &monitor->state;

#if HAVE_SOCKS_HOSTID
   monitor->hostindex          = DEFAULT_HOSTINDEX;
   hostindex                   = &monitor->hostindex;

   monitor->hostidoption_isset = 0;
   hostidoption_isset          = &monitor->hostidoption_isset;
#endif /* HAVE_SOCKS_HOSTID */

   bzero(&src, sizeof(src));
   bzero(&dst, sizeof(dst));
   bzero(&hostid, sizeof(hostid));

   if ((monitor->mstats = malloc(sizeof(*monitor->mstats))) == NULL)
      yyerror("failed to malloc(3) %lu bytes for monitor stats memory",
              (unsigned long)sizeof(*monitor->mstats));
   else
      bzero(monitor->mstats, sizeof(*monitor->mstats));

   monitor->mstats->type = SHMEM_MONITOR;
}

static void
pre_addmonitor(monitor)
   monitor_t *monitor;
{
   monitor->src    = src;
   monitor->dst    = dst;

#if HAVE_SOCKS_HOSTID
   monitor->hostid = hostid;
#endif /* HAVE_SOCKS_HOSTID */
}

static int
configure_privileges(void)
{
   const char *function = "configure_privileges()";
   static int isfirsttime = 1;

   if (sockscf.option.verifyonly)
      return 0;

#if !HAVE_PRIVILEGES
   uid_t uid; /* for debugging. */
   gid_t gid; /* for debugging. */

   SASSERTX(sockscf.state.euid == (uid = geteuid()));
   SASSERTX(sockscf.state.egid == (gid = getegid()));

   /*
    * Check all configured uids/gids work.
    */

   checkugid(&sockscf.uid.privileged_uid,
             &sockscf.uid.privileged_gid,
             &sockscf.uid.privileged_isset,
             "privileged");

   checkugid(&sockscf.uid.unprivileged_uid,
             &sockscf.uid.unprivileged_gid,
             &sockscf.uid.unprivileged_isset,
             "unprivileged");

#if HAVE_LIBWRAP
   if (!sockscf.uid.libwrap_isset
   &&  sockscf.uid.unprivileged_isset) {
      sockscf.uid.libwrap_uid   = sockscf.uid.unprivileged_uid;
      sockscf.uid.libwrap_gid   = sockscf.uid.unprivileged_gid;
      sockscf.uid.libwrap_isset = sockscf.uid.unprivileged_isset;
   }
   else
      checkugid(&sockscf.uid.libwrap_uid,
                &sockscf.uid.libwrap_gid,
                &sockscf.uid.libwrap_isset,
                "libwrap");
#endif /* HAVE_LIBWRAP */

   SASSERTX(sockscf.state.euid == (uid = geteuid()));
   SASSERTX(sockscf.state.egid == (gid = getegid()));

#endif /* !HAVE_PRIVILEGES */

   if (isfirsttime) {
      if (sockd_initprivs() != 0) {
         slog(HAVE_PRIVILEGES ? LOG_INFO : LOG_WARNING,
              "%s: could not initialize privileges (%s)%s",
              function,
              strerror(errno),
              geteuid() == 0 ?
                   "" : ".  Usually we need to be started by root if "
                        "special privileges are to be available");

#if HAVE_PRIVILEGES
         /*
          * assume failure in this case is not fatal; some privileges will
          * not be available to us, and perhaps that is the intention too.
          */
         return 0;

#else
         return -1;
#endif /* !HAVE_PRIVILEGES */
      }

      isfirsttime = 0;
   }

   return 0;
}

static int
checkugid(uid, gid, isset, type)
   uid_t *uid;
   gid_t *gid;
   unsigned char *isset;
   const char *type;
{
   const char *function = "checkugid()";

   SASSERTX(sockscf.state.euid == geteuid());
   SASSERTX(sockscf.state.egid == getegid());

   if (sockscf.option.verifyonly)
      return 0;

   if (!(*isset)) {
      *uid   = sockscf.state.euid;
      *gid   = sockscf.state.egid;
      *isset = 1;

      return 0;
   }

   if (*uid != sockscf.state.euid) {
      if (seteuid(*uid) != 0) {
         swarn("%s: could not seteuid(2) to %s uid %lu",
               function, type, (unsigned long)*uid);

         return -1;
      }

      (void)seteuid(0);

      if (seteuid(sockscf.state.euid) != 0) {
         swarn("%s: could not revert to euid %lu from euid %lu",
               function,
               (unsigned long)sockscf.state.euid,
               (unsigned long)geteuid());
         SWARN(0);

         sockscf.state.euid = geteuid();
         return -1;
      }
   }

   if (*gid != sockscf.state.egid) {
      (void)seteuid(0);

      if (setegid(*gid) != 0) {
         swarn("%s: could not setegid(2) to %s gid %lu",
               function, type, (unsigned long)*gid);

         return -1;
      }

      (void)seteuid(0);

      if (setegid(sockscf.state.egid) != 0) {
         swarn("%s: could not revert to egid %lu from euid %lu",
               function,
               (unsigned long)sockscf.state.egid,
               (unsigned long)geteuid());
         SWARN(0);

         sockscf.state.egid = getegid();
         return -1;
      }

      if (seteuid(sockscf.state.euid) != 0) {
         swarn("%s: could not revert to euid %lu from euid %lu",
               function,
               (unsigned long)sockscf.state.euid,
               (unsigned long)geteuid());
         SWARN(0);

         sockscf.state.euid = geteuid();
         return -1;
      }
   }

   SASSERTX(sockscf.state.euid == geteuid());
   SASSERTX(sockscf.state.egid == getegid());

   return 0;
}

#endif /* !SOCKS_CLIENT */

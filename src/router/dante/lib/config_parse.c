#include "common.h"
/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         socks_yyparse
#define yylex           socks_yylex
#define yyerror         socks_yyerror
#define yydebug         socks_yydebug
#define yynerrs         socks_yynerrs
#define yylval          socks_yylval
#define yychar          socks_yychar

/* First part of user prologue.  */
#line 46 "config_parse.y"


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

#line 448 "config_parse.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_SOCKS_YY_Y_TAB_H_INCLUDED
# define YY_SOCKS_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int socks_yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ALARM = 258,                   /* ALARM  */
    ALARMTYPE_DATA = 259,          /* ALARMTYPE_DATA  */
    ALARMTYPE_DISCONNECT = 260,    /* ALARMTYPE_DISCONNECT  */
    ALARMIF_INTERNAL = 261,        /* ALARMIF_INTERNAL  */
    ALARMIF_EXTERNAL = 262,        /* ALARMIF_EXTERNAL  */
    TCPOPTION_DISABLED = 263,      /* TCPOPTION_DISABLED  */
    ECN = 264,                     /* ECN  */
    SACK = 265,                    /* SACK  */
    TIMESTAMPS = 266,              /* TIMESTAMPS  */
    WSCALE = 267,                  /* WSCALE  */
    MTU_ERROR = 268,               /* MTU_ERROR  */
    CLIENTCOMPATIBILITY = 269,     /* CLIENTCOMPATIBILITY  */
    NECGSSAPI = 270,               /* NECGSSAPI  */
    CLIENTRULE = 271,              /* CLIENTRULE  */
    HOSTIDRULE = 272,              /* HOSTIDRULE  */
    SOCKSRULE = 273,               /* SOCKSRULE  */
    COMPATIBILITY = 274,           /* COMPATIBILITY  */
    SAMEPORT = 275,                /* SAMEPORT  */
    DRAFT_5_05 = 276,              /* DRAFT_5_05  */
    CONNECTTIMEOUT = 277,          /* CONNECTTIMEOUT  */
    TCP_FIN_WAIT = 278,            /* TCP_FIN_WAIT  */
    CPU = 279,                     /* CPU  */
    MASK = 280,                    /* MASK  */
    SCHEDULE = 281,                /* SCHEDULE  */
    CPUMASK_ANYCPU = 282,          /* CPUMASK_ANYCPU  */
    DEBUGGING = 283,               /* DEBUGGING  */
    DEPRECATED = 284,              /* DEPRECATED  */
    ERRORLOG = 285,                /* ERRORLOG  */
    LOGOUTPUT = 286,               /* LOGOUTPUT  */
    LOGFILE = 287,                 /* LOGFILE  */
    LOGTYPE_ERROR = 288,           /* LOGTYPE_ERROR  */
    LOGTYPE_TCP_DISABLED = 289,    /* LOGTYPE_TCP_DISABLED  */
    LOGTYPE_TCP_ENABLED = 290,     /* LOGTYPE_TCP_ENABLED  */
    LOGIF_INTERNAL = 291,          /* LOGIF_INTERNAL  */
    LOGIF_EXTERNAL = 292,          /* LOGIF_EXTERNAL  */
    ERRORVALUE = 293,              /* ERRORVALUE  */
    EXTENSION = 294,               /* EXTENSION  */
    BIND = 295,                    /* BIND  */
    PRIVILEGED = 296,              /* PRIVILEGED  */
    EXTERNAL_PROTOCOL = 297,       /* EXTERNAL_PROTOCOL  */
    INTERNAL_PROTOCOL = 298,       /* INTERNAL_PROTOCOL  */
    EXTERNAL_ROTATION = 299,       /* EXTERNAL_ROTATION  */
    SAMESAME = 300,                /* SAMESAME  */
    GROUPNAME = 301,               /* GROUPNAME  */
    HOSTID = 302,                  /* HOSTID  */
    HOSTINDEX = 303,               /* HOSTINDEX  */
    INTERFACE = 304,               /* INTERFACE  */
    SOCKETOPTION_SYMBOLICVALUE = 305, /* SOCKETOPTION_SYMBOLICVALUE  */
    INTERNAL = 306,                /* INTERNAL  */
    EXTERNAL = 307,                /* EXTERNAL  */
    INTERNALSOCKET = 308,          /* INTERNALSOCKET  */
    EXTERNALSOCKET = 309,          /* EXTERNALSOCKET  */
    IOTIMEOUT = 310,               /* IOTIMEOUT  */
    IOTIMEOUT_TCP = 311,           /* IOTIMEOUT_TCP  */
    IOTIMEOUT_UDP = 312,           /* IOTIMEOUT_UDP  */
    NEGOTIATETIMEOUT = 313,        /* NEGOTIATETIMEOUT  */
    LIBWRAP_FILE = 314,            /* LIBWRAP_FILE  */
    LOGLEVEL = 315,                /* LOGLEVEL  */
    SOCKSMETHOD = 316,             /* SOCKSMETHOD  */
    CLIENTMETHOD = 317,            /* CLIENTMETHOD  */
    METHOD = 318,                  /* METHOD  */
    METHODNAME = 319,              /* METHODNAME  */
    NONE = 320,                    /* NONE  */
    BSDAUTH = 321,                 /* BSDAUTH  */
    GSSAPI = 322,                  /* GSSAPI  */
    PAM_ADDRESS = 323,             /* PAM_ADDRESS  */
    PAM_ANY = 324,                 /* PAM_ANY  */
    PAM_USERNAME = 325,            /* PAM_USERNAME  */
    RFC931 = 326,                  /* RFC931  */
    UNAME = 327,                   /* UNAME  */
    MONITOR = 328,                 /* MONITOR  */
    PROCESSTYPE = 329,             /* PROCESSTYPE  */
    PROC_MAXREQUESTS = 330,        /* PROC_MAXREQUESTS  */
    PROC_MAXLIFETIME = 331,        /* PROC_MAXLIFETIME  */
    REALM = 332,                   /* REALM  */
    REALNAME = 333,                /* REALNAME  */
    RESOLVEPROTOCOL = 334,         /* RESOLVEPROTOCOL  */
    REQUIRED = 335,                /* REQUIRED  */
    SCHEDULEPOLICY = 336,          /* SCHEDULEPOLICY  */
    SERVERCONFIG = 337,            /* SERVERCONFIG  */
    CLIENTCONFIG = 338,            /* CLIENTCONFIG  */
    SOCKET = 339,                  /* SOCKET  */
    CLIENTSIDE_SOCKET = 340,       /* CLIENTSIDE_SOCKET  */
    SNDBUF = 341,                  /* SNDBUF  */
    RCVBUF = 342,                  /* RCVBUF  */
    SOCKETPROTOCOL = 343,          /* SOCKETPROTOCOL  */
    SOCKETOPTION_OPTID = 344,      /* SOCKETOPTION_OPTID  */
    SRCHOST = 345,                 /* SRCHOST  */
    NODNSMISMATCH = 346,           /* NODNSMISMATCH  */
    NODNSUNKNOWN = 347,            /* NODNSUNKNOWN  */
    CHECKREPLYAUTH = 348,          /* CHECKREPLYAUTH  */
    USERNAME = 349,                /* USERNAME  */
    USER_PRIVILEGED = 350,         /* USER_PRIVILEGED  */
    USER_UNPRIVILEGED = 351,       /* USER_UNPRIVILEGED  */
    USER_LIBWRAP = 352,            /* USER_LIBWRAP  */
    WORD__IN = 353,                /* WORD__IN  */
    ROUTE = 354,                   /* ROUTE  */
    VIA = 355,                     /* VIA  */
    GLOBALROUTEOPTION = 356,       /* GLOBALROUTEOPTION  */
    BADROUTE_EXPIRE = 357,         /* BADROUTE_EXPIRE  */
    MAXFAIL = 358,                 /* MAXFAIL  */
    PORT = 359,                    /* PORT  */
    NUMBER = 360,                  /* NUMBER  */
    BANDWIDTH = 361,               /* BANDWIDTH  */
    BOUNCE = 362,                  /* BOUNCE  */
    BSDAUTHSTYLE = 363,            /* BSDAUTHSTYLE  */
    BSDAUTHSTYLENAME = 364,        /* BSDAUTHSTYLENAME  */
    COMMAND = 365,                 /* COMMAND  */
    COMMAND_BIND = 366,            /* COMMAND_BIND  */
    COMMAND_CONNECT = 367,         /* COMMAND_CONNECT  */
    COMMAND_UDPASSOCIATE = 368,    /* COMMAND_UDPASSOCIATE  */
    COMMAND_BINDREPLY = 369,       /* COMMAND_BINDREPLY  */
    COMMAND_UDPREPLY = 370,        /* COMMAND_UDPREPLY  */
    ACTION = 371,                  /* ACTION  */
    FROM = 372,                    /* FROM  */
    TO = 373,                      /* TO  */
    GSSAPIENCTYPE = 374,           /* GSSAPIENCTYPE  */
    GSSAPIENC_ANY = 375,           /* GSSAPIENC_ANY  */
    GSSAPIENC_CLEAR = 376,         /* GSSAPIENC_CLEAR  */
    GSSAPIENC_INTEGRITY = 377,     /* GSSAPIENC_INTEGRITY  */
    GSSAPIENC_CONFIDENTIALITY = 378, /* GSSAPIENC_CONFIDENTIALITY  */
    GSSAPIENC_PERMESSAGE = 379,    /* GSSAPIENC_PERMESSAGE  */
    GSSAPIKEYTAB = 380,            /* GSSAPIKEYTAB  */
    GSSAPISERVICE = 381,           /* GSSAPISERVICE  */
    GSSAPISERVICENAME = 382,       /* GSSAPISERVICENAME  */
    GSSAPIKEYTABNAME = 383,        /* GSSAPIKEYTABNAME  */
    IPV4 = 384,                    /* IPV4  */
    IPV6 = 385,                    /* IPV6  */
    IPVANY = 386,                  /* IPVANY  */
    DOMAINNAME = 387,              /* DOMAINNAME  */
    IFNAME = 388,                  /* IFNAME  */
    URL = 389,                     /* URL  */
    LDAPATTRIBUTE = 390,           /* LDAPATTRIBUTE  */
    LDAPATTRIBUTE_AD = 391,        /* LDAPATTRIBUTE_AD  */
    LDAPATTRIBUTE_HEX = 392,       /* LDAPATTRIBUTE_HEX  */
    LDAPATTRIBUTE_AD_HEX = 393,    /* LDAPATTRIBUTE_AD_HEX  */
    LDAPBASEDN = 394,              /* LDAPBASEDN  */
    LDAP_BASEDN = 395,             /* LDAP_BASEDN  */
    LDAPBASEDN_HEX = 396,          /* LDAPBASEDN_HEX  */
    LDAPBASEDN_HEX_ALL = 397,      /* LDAPBASEDN_HEX_ALL  */
    LDAPCERTFILE = 398,            /* LDAPCERTFILE  */
    LDAPCERTPATH = 399,            /* LDAPCERTPATH  */
    LDAPPORT = 400,                /* LDAPPORT  */
    LDAPPORTSSL = 401,             /* LDAPPORTSSL  */
    LDAPDEBUG = 402,               /* LDAPDEBUG  */
    LDAPDEPTH = 403,               /* LDAPDEPTH  */
    LDAPAUTO = 404,                /* LDAPAUTO  */
    LDAPSEARCHTIME = 405,          /* LDAPSEARCHTIME  */
    LDAPDOMAIN = 406,              /* LDAPDOMAIN  */
    LDAP_DOMAIN = 407,             /* LDAP_DOMAIN  */
    LDAPFILTER = 408,              /* LDAPFILTER  */
    LDAPFILTER_AD = 409,           /* LDAPFILTER_AD  */
    LDAPFILTER_HEX = 410,          /* LDAPFILTER_HEX  */
    LDAPFILTER_AD_HEX = 411,       /* LDAPFILTER_AD_HEX  */
    LDAPGROUP = 412,               /* LDAPGROUP  */
    LDAPGROUP_NAME = 413,          /* LDAPGROUP_NAME  */
    LDAPGROUP_HEX = 414,           /* LDAPGROUP_HEX  */
    LDAPGROUP_HEX_ALL = 415,       /* LDAPGROUP_HEX_ALL  */
    LDAPKEYTAB = 416,              /* LDAPKEYTAB  */
    LDAPKEYTABNAME = 417,          /* LDAPKEYTABNAME  */
    LDAPDEADTIME = 418,            /* LDAPDEADTIME  */
    LDAPSERVER = 419,              /* LDAPSERVER  */
    LDAPSERVER_NAME = 420,         /* LDAPSERVER_NAME  */
    LDAPAUTHSERVER = 421,          /* LDAPAUTHSERVER  */
    LDAPAUTHKEYTAB = 422,          /* LDAPAUTHKEYTAB  */
    LDAPSSL = 423,                 /* LDAPSSL  */
    LDAPCERTCHECK = 424,           /* LDAPCERTCHECK  */
    LDAPKEEPREALM = 425,           /* LDAPKEEPREALM  */
    LDAPTIMEOUT = 426,             /* LDAPTIMEOUT  */
    LDAPCACHE = 427,               /* LDAPCACHE  */
    LDAPCACHEPOS = 428,            /* LDAPCACHEPOS  */
    LDAPCACHENEG = 429,            /* LDAPCACHENEG  */
    LDAPURL = 430,                 /* LDAPURL  */
    LDAP_URL = 431,                /* LDAP_URL  */
    LDAPAUTHBASEDN = 432,          /* LDAPAUTHBASEDN  */
    LDAPAUTHBASEDN_HEX = 433,      /* LDAPAUTHBASEDN_HEX  */
    LDAPAUTHBASEDN_HEX_ALL = 434,  /* LDAPAUTHBASEDN_HEX_ALL  */
    LDAPAUTHURL = 435,             /* LDAPAUTHURL  */
    LDAPAUTHPORT = 436,            /* LDAPAUTHPORT  */
    LDAPAUTHPORTSSL = 437,         /* LDAPAUTHPORTSSL  */
    LDAPAUTHDEBUG = 438,           /* LDAPAUTHDEBUG  */
    LDAPAUTHSSL = 439,             /* LDAPAUTHSSL  */
    LDAPAUTHAUTO = 440,            /* LDAPAUTHAUTO  */
    LDAPAUTHCERTCHECK = 441,       /* LDAPAUTHCERTCHECK  */
    LDAPAUTHFILTER = 442,          /* LDAPAUTHFILTER  */
    LDAPAUTHDOMAIN = 443,          /* LDAPAUTHDOMAIN  */
    LDAPAUTHCERTFILE = 444,        /* LDAPAUTHCERTFILE  */
    LDAPAUTHCERTPATH = 445,        /* LDAPAUTHCERTPATH  */
    LDAPAUTHKEEPREALM = 446,       /* LDAPAUTHKEEPREALM  */
    LDAP_FILTER = 447,             /* LDAP_FILTER  */
    LDAP_ATTRIBUTE = 448,          /* LDAP_ATTRIBUTE  */
    LDAP_CERTFILE = 449,           /* LDAP_CERTFILE  */
    LDAP_CERTPATH = 450,           /* LDAP_CERTPATH  */
    LIBWRAPSTART = 451,            /* LIBWRAPSTART  */
    LIBWRAP_ALLOW = 452,           /* LIBWRAP_ALLOW  */
    LIBWRAP_DENY = 453,            /* LIBWRAP_DENY  */
    LIBWRAP_HOSTS_ACCESS = 454,    /* LIBWRAP_HOSTS_ACCESS  */
    LINE = 455,                    /* LINE  */
    OPERATOR = 456,                /* OPERATOR  */
    PACSID = 457,                  /* PACSID  */
    PACSID_B64 = 458,              /* PACSID_B64  */
    PACSID_FLAG = 459,             /* PACSID_FLAG  */
    PACSID_NAME = 460,             /* PACSID_NAME  */
    PAMSERVICENAME = 461,          /* PAMSERVICENAME  */
    PROTOCOL = 462,                /* PROTOCOL  */
    PROTOCOL_TCP = 463,            /* PROTOCOL_TCP  */
    PROTOCOL_UDP = 464,            /* PROTOCOL_UDP  */
    PROTOCOL_FAKE = 465,           /* PROTOCOL_FAKE  */
    PROXYPROTOCOL = 466,           /* PROXYPROTOCOL  */
    PROXYPROTOCOL_SOCKS_V4 = 467,  /* PROXYPROTOCOL_SOCKS_V4  */
    PROXYPROTOCOL_SOCKS_V5 = 468,  /* PROXYPROTOCOL_SOCKS_V5  */
    PROXYPROTOCOL_HTTP = 469,      /* PROXYPROTOCOL_HTTP  */
    PROXYPROTOCOL_UPNP = 470,      /* PROXYPROTOCOL_UPNP  */
    REDIRECT = 471,                /* REDIRECT  */
    SENDSIDE = 472,                /* SENDSIDE  */
    RECVSIDE = 473,                /* RECVSIDE  */
    SERVICENAME = 474,             /* SERVICENAME  */
    SESSION_INHERITABLE = 475,     /* SESSION_INHERITABLE  */
    SESSIONMAX = 476,              /* SESSIONMAX  */
    SESSIONTHROTTLE = 477,         /* SESSIONTHROTTLE  */
    SESSIONSTATE_KEY = 478,        /* SESSIONSTATE_KEY  */
    SESSIONSTATE_MAX = 479,        /* SESSIONSTATE_MAX  */
    SESSIONSTATE_THROTTLE = 480,   /* SESSIONSTATE_THROTTLE  */
    RULE_LOG = 481,                /* RULE_LOG  */
    RULE_LOG_CONNECT = 482,        /* RULE_LOG_CONNECT  */
    RULE_LOG_DATA = 483,           /* RULE_LOG_DATA  */
    RULE_LOG_DISCONNECT = 484,     /* RULE_LOG_DISCONNECT  */
    RULE_LOG_ERROR = 485,          /* RULE_LOG_ERROR  */
    RULE_LOG_IOOPERATION = 486,    /* RULE_LOG_IOOPERATION  */
    RULE_LOG_TCPINFO = 487,        /* RULE_LOG_TCPINFO  */
    STATEKEY = 488,                /* STATEKEY  */
    UDPPORTRANGE = 489,            /* UDPPORTRANGE  */
    UDPCONNECTDST = 490,           /* UDPCONNECTDST  */
    USER = 491,                    /* USER  */
    GROUP = 492,                   /* GROUP  */
    VERDICT_BLOCK = 493,           /* VERDICT_BLOCK  */
    VERDICT_PASS = 494,            /* VERDICT_PASS  */
    YES = 495,                     /* YES  */
    NO = 496                       /* NO  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define ALARM 258
#define ALARMTYPE_DATA 259
#define ALARMTYPE_DISCONNECT 260
#define ALARMIF_INTERNAL 261
#define ALARMIF_EXTERNAL 262
#define TCPOPTION_DISABLED 263
#define ECN 264
#define SACK 265
#define TIMESTAMPS 266
#define WSCALE 267
#define MTU_ERROR 268
#define CLIENTCOMPATIBILITY 269
#define NECGSSAPI 270
#define CLIENTRULE 271
#define HOSTIDRULE 272
#define SOCKSRULE 273
#define COMPATIBILITY 274
#define SAMEPORT 275
#define DRAFT_5_05 276
#define CONNECTTIMEOUT 277
#define TCP_FIN_WAIT 278
#define CPU 279
#define MASK 280
#define SCHEDULE 281
#define CPUMASK_ANYCPU 282
#define DEBUGGING 283
#define DEPRECATED 284
#define ERRORLOG 285
#define LOGOUTPUT 286
#define LOGFILE 287
#define LOGTYPE_ERROR 288
#define LOGTYPE_TCP_DISABLED 289
#define LOGTYPE_TCP_ENABLED 290
#define LOGIF_INTERNAL 291
#define LOGIF_EXTERNAL 292
#define ERRORVALUE 293
#define EXTENSION 294
#define BIND 295
#define PRIVILEGED 296
#define EXTERNAL_PROTOCOL 297
#define INTERNAL_PROTOCOL 298
#define EXTERNAL_ROTATION 299
#define SAMESAME 300
#define GROUPNAME 301
#define HOSTID 302
#define HOSTINDEX 303
#define INTERFACE 304
#define SOCKETOPTION_SYMBOLICVALUE 305
#define INTERNAL 306
#define EXTERNAL 307
#define INTERNALSOCKET 308
#define EXTERNALSOCKET 309
#define IOTIMEOUT 310
#define IOTIMEOUT_TCP 311
#define IOTIMEOUT_UDP 312
#define NEGOTIATETIMEOUT 313
#define LIBWRAP_FILE 314
#define LOGLEVEL 315
#define SOCKSMETHOD 316
#define CLIENTMETHOD 317
#define METHOD 318
#define METHODNAME 319
#define NONE 320
#define BSDAUTH 321
#define GSSAPI 322
#define PAM_ADDRESS 323
#define PAM_ANY 324
#define PAM_USERNAME 325
#define RFC931 326
#define UNAME 327
#define MONITOR 328
#define PROCESSTYPE 329
#define PROC_MAXREQUESTS 330
#define PROC_MAXLIFETIME 331
#define REALM 332
#define REALNAME 333
#define RESOLVEPROTOCOL 334
#define REQUIRED 335
#define SCHEDULEPOLICY 336
#define SERVERCONFIG 337
#define CLIENTCONFIG 338
#define SOCKET 339
#define CLIENTSIDE_SOCKET 340
#define SNDBUF 341
#define RCVBUF 342
#define SOCKETPROTOCOL 343
#define SOCKETOPTION_OPTID 344
#define SRCHOST 345
#define NODNSMISMATCH 346
#define NODNSUNKNOWN 347
#define CHECKREPLYAUTH 348
#define USERNAME 349
#define USER_PRIVILEGED 350
#define USER_UNPRIVILEGED 351
#define USER_LIBWRAP 352
#define WORD__IN 353
#define ROUTE 354
#define VIA 355
#define GLOBALROUTEOPTION 356
#define BADROUTE_EXPIRE 357
#define MAXFAIL 358
#define PORT 359
#define NUMBER 360
#define BANDWIDTH 361
#define BOUNCE 362
#define BSDAUTHSTYLE 363
#define BSDAUTHSTYLENAME 364
#define COMMAND 365
#define COMMAND_BIND 366
#define COMMAND_CONNECT 367
#define COMMAND_UDPASSOCIATE 368
#define COMMAND_BINDREPLY 369
#define COMMAND_UDPREPLY 370
#define ACTION 371
#define FROM 372
#define TO 373
#define GSSAPIENCTYPE 374
#define GSSAPIENC_ANY 375
#define GSSAPIENC_CLEAR 376
#define GSSAPIENC_INTEGRITY 377
#define GSSAPIENC_CONFIDENTIALITY 378
#define GSSAPIENC_PERMESSAGE 379
#define GSSAPIKEYTAB 380
#define GSSAPISERVICE 381
#define GSSAPISERVICENAME 382
#define GSSAPIKEYTABNAME 383
#define IPV4 384
#define IPV6 385
#define IPVANY 386
#define DOMAINNAME 387
#define IFNAME 388
#define URL 389
#define LDAPATTRIBUTE 390
#define LDAPATTRIBUTE_AD 391
#define LDAPATTRIBUTE_HEX 392
#define LDAPATTRIBUTE_AD_HEX 393
#define LDAPBASEDN 394
#define LDAP_BASEDN 395
#define LDAPBASEDN_HEX 396
#define LDAPBASEDN_HEX_ALL 397
#define LDAPCERTFILE 398
#define LDAPCERTPATH 399
#define LDAPPORT 400
#define LDAPPORTSSL 401
#define LDAPDEBUG 402
#define LDAPDEPTH 403
#define LDAPAUTO 404
#define LDAPSEARCHTIME 405
#define LDAPDOMAIN 406
#define LDAP_DOMAIN 407
#define LDAPFILTER 408
#define LDAPFILTER_AD 409
#define LDAPFILTER_HEX 410
#define LDAPFILTER_AD_HEX 411
#define LDAPGROUP 412
#define LDAPGROUP_NAME 413
#define LDAPGROUP_HEX 414
#define LDAPGROUP_HEX_ALL 415
#define LDAPKEYTAB 416
#define LDAPKEYTABNAME 417
#define LDAPDEADTIME 418
#define LDAPSERVER 419
#define LDAPSERVER_NAME 420
#define LDAPAUTHSERVER 421
#define LDAPAUTHKEYTAB 422
#define LDAPSSL 423
#define LDAPCERTCHECK 424
#define LDAPKEEPREALM 425
#define LDAPTIMEOUT 426
#define LDAPCACHE 427
#define LDAPCACHEPOS 428
#define LDAPCACHENEG 429
#define LDAPURL 430
#define LDAP_URL 431
#define LDAPAUTHBASEDN 432
#define LDAPAUTHBASEDN_HEX 433
#define LDAPAUTHBASEDN_HEX_ALL 434
#define LDAPAUTHURL 435
#define LDAPAUTHPORT 436
#define LDAPAUTHPORTSSL 437
#define LDAPAUTHDEBUG 438
#define LDAPAUTHSSL 439
#define LDAPAUTHAUTO 440
#define LDAPAUTHCERTCHECK 441
#define LDAPAUTHFILTER 442
#define LDAPAUTHDOMAIN 443
#define LDAPAUTHCERTFILE 444
#define LDAPAUTHCERTPATH 445
#define LDAPAUTHKEEPREALM 446
#define LDAP_FILTER 447
#define LDAP_ATTRIBUTE 448
#define LDAP_CERTFILE 449
#define LDAP_CERTPATH 450
#define LIBWRAPSTART 451
#define LIBWRAP_ALLOW 452
#define LIBWRAP_DENY 453
#define LIBWRAP_HOSTS_ACCESS 454
#define LINE 455
#define OPERATOR 456
#define PACSID 457
#define PACSID_B64 458
#define PACSID_FLAG 459
#define PACSID_NAME 460
#define PAMSERVICENAME 461
#define PROTOCOL 462
#define PROTOCOL_TCP 463
#define PROTOCOL_UDP 464
#define PROTOCOL_FAKE 465
#define PROXYPROTOCOL 466
#define PROXYPROTOCOL_SOCKS_V4 467
#define PROXYPROTOCOL_SOCKS_V5 468
#define PROXYPROTOCOL_HTTP 469
#define PROXYPROTOCOL_UPNP 470
#define REDIRECT 471
#define SENDSIDE 472
#define RECVSIDE 473
#define SERVICENAME 474
#define SESSION_INHERITABLE 475
#define SESSIONMAX 476
#define SESSIONTHROTTLE 477
#define SESSIONSTATE_KEY 478
#define SESSIONSTATE_MAX 479
#define SESSIONSTATE_THROTTLE 480
#define RULE_LOG 481
#define RULE_LOG_CONNECT 482
#define RULE_LOG_DATA 483
#define RULE_LOG_DISCONNECT 484
#define RULE_LOG_ERROR 485
#define RULE_LOG_IOOPERATION 486
#define RULE_LOG_TCPINFO 487
#define STATEKEY 488
#define UDPPORTRANGE 489
#define UDPCONNECTDST 490
#define USER 491
#define GROUP 492
#define VERDICT_BLOCK 493
#define VERDICT_PASS 494
#define YES 495
#define NO 496

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 417 "config_parse.y"

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

#line 1004 "config_parse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE socks_yylval;


int socks_yyparse (void);


#endif /* !YY_SOCKS_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ALARM = 3,                      /* ALARM  */
  YYSYMBOL_ALARMTYPE_DATA = 4,             /* ALARMTYPE_DATA  */
  YYSYMBOL_ALARMTYPE_DISCONNECT = 5,       /* ALARMTYPE_DISCONNECT  */
  YYSYMBOL_ALARMIF_INTERNAL = 6,           /* ALARMIF_INTERNAL  */
  YYSYMBOL_ALARMIF_EXTERNAL = 7,           /* ALARMIF_EXTERNAL  */
  YYSYMBOL_TCPOPTION_DISABLED = 8,         /* TCPOPTION_DISABLED  */
  YYSYMBOL_ECN = 9,                        /* ECN  */
  YYSYMBOL_SACK = 10,                      /* SACK  */
  YYSYMBOL_TIMESTAMPS = 11,                /* TIMESTAMPS  */
  YYSYMBOL_WSCALE = 12,                    /* WSCALE  */
  YYSYMBOL_MTU_ERROR = 13,                 /* MTU_ERROR  */
  YYSYMBOL_CLIENTCOMPATIBILITY = 14,       /* CLIENTCOMPATIBILITY  */
  YYSYMBOL_NECGSSAPI = 15,                 /* NECGSSAPI  */
  YYSYMBOL_CLIENTRULE = 16,                /* CLIENTRULE  */
  YYSYMBOL_HOSTIDRULE = 17,                /* HOSTIDRULE  */
  YYSYMBOL_SOCKSRULE = 18,                 /* SOCKSRULE  */
  YYSYMBOL_COMPATIBILITY = 19,             /* COMPATIBILITY  */
  YYSYMBOL_SAMEPORT = 20,                  /* SAMEPORT  */
  YYSYMBOL_DRAFT_5_05 = 21,                /* DRAFT_5_05  */
  YYSYMBOL_CONNECTTIMEOUT = 22,            /* CONNECTTIMEOUT  */
  YYSYMBOL_TCP_FIN_WAIT = 23,              /* TCP_FIN_WAIT  */
  YYSYMBOL_CPU = 24,                       /* CPU  */
  YYSYMBOL_MASK = 25,                      /* MASK  */
  YYSYMBOL_SCHEDULE = 26,                  /* SCHEDULE  */
  YYSYMBOL_CPUMASK_ANYCPU = 27,            /* CPUMASK_ANYCPU  */
  YYSYMBOL_DEBUGGING = 28,                 /* DEBUGGING  */
  YYSYMBOL_DEPRECATED = 29,                /* DEPRECATED  */
  YYSYMBOL_ERRORLOG = 30,                  /* ERRORLOG  */
  YYSYMBOL_LOGOUTPUT = 31,                 /* LOGOUTPUT  */
  YYSYMBOL_LOGFILE = 32,                   /* LOGFILE  */
  YYSYMBOL_LOGTYPE_ERROR = 33,             /* LOGTYPE_ERROR  */
  YYSYMBOL_LOGTYPE_TCP_DISABLED = 34,      /* LOGTYPE_TCP_DISABLED  */
  YYSYMBOL_LOGTYPE_TCP_ENABLED = 35,       /* LOGTYPE_TCP_ENABLED  */
  YYSYMBOL_LOGIF_INTERNAL = 36,            /* LOGIF_INTERNAL  */
  YYSYMBOL_LOGIF_EXTERNAL = 37,            /* LOGIF_EXTERNAL  */
  YYSYMBOL_ERRORVALUE = 38,                /* ERRORVALUE  */
  YYSYMBOL_EXTENSION = 39,                 /* EXTENSION  */
  YYSYMBOL_BIND = 40,                      /* BIND  */
  YYSYMBOL_PRIVILEGED = 41,                /* PRIVILEGED  */
  YYSYMBOL_EXTERNAL_PROTOCOL = 42,         /* EXTERNAL_PROTOCOL  */
  YYSYMBOL_INTERNAL_PROTOCOL = 43,         /* INTERNAL_PROTOCOL  */
  YYSYMBOL_EXTERNAL_ROTATION = 44,         /* EXTERNAL_ROTATION  */
  YYSYMBOL_SAMESAME = 45,                  /* SAMESAME  */
  YYSYMBOL_GROUPNAME = 46,                 /* GROUPNAME  */
  YYSYMBOL_HOSTID = 47,                    /* HOSTID  */
  YYSYMBOL_HOSTINDEX = 48,                 /* HOSTINDEX  */
  YYSYMBOL_INTERFACE = 49,                 /* INTERFACE  */
  YYSYMBOL_SOCKETOPTION_SYMBOLICVALUE = 50, /* SOCKETOPTION_SYMBOLICVALUE  */
  YYSYMBOL_INTERNAL = 51,                  /* INTERNAL  */
  YYSYMBOL_EXTERNAL = 52,                  /* EXTERNAL  */
  YYSYMBOL_INTERNALSOCKET = 53,            /* INTERNALSOCKET  */
  YYSYMBOL_EXTERNALSOCKET = 54,            /* EXTERNALSOCKET  */
  YYSYMBOL_IOTIMEOUT = 55,                 /* IOTIMEOUT  */
  YYSYMBOL_IOTIMEOUT_TCP = 56,             /* IOTIMEOUT_TCP  */
  YYSYMBOL_IOTIMEOUT_UDP = 57,             /* IOTIMEOUT_UDP  */
  YYSYMBOL_NEGOTIATETIMEOUT = 58,          /* NEGOTIATETIMEOUT  */
  YYSYMBOL_LIBWRAP_FILE = 59,              /* LIBWRAP_FILE  */
  YYSYMBOL_LOGLEVEL = 60,                  /* LOGLEVEL  */
  YYSYMBOL_SOCKSMETHOD = 61,               /* SOCKSMETHOD  */
  YYSYMBOL_CLIENTMETHOD = 62,              /* CLIENTMETHOD  */
  YYSYMBOL_METHOD = 63,                    /* METHOD  */
  YYSYMBOL_METHODNAME = 64,                /* METHODNAME  */
  YYSYMBOL_NONE = 65,                      /* NONE  */
  YYSYMBOL_BSDAUTH = 66,                   /* BSDAUTH  */
  YYSYMBOL_GSSAPI = 67,                    /* GSSAPI  */
  YYSYMBOL_PAM_ADDRESS = 68,               /* PAM_ADDRESS  */
  YYSYMBOL_PAM_ANY = 69,                   /* PAM_ANY  */
  YYSYMBOL_PAM_USERNAME = 70,              /* PAM_USERNAME  */
  YYSYMBOL_RFC931 = 71,                    /* RFC931  */
  YYSYMBOL_UNAME = 72,                     /* UNAME  */
  YYSYMBOL_MONITOR = 73,                   /* MONITOR  */
  YYSYMBOL_PROCESSTYPE = 74,               /* PROCESSTYPE  */
  YYSYMBOL_PROC_MAXREQUESTS = 75,          /* PROC_MAXREQUESTS  */
  YYSYMBOL_PROC_MAXLIFETIME = 76,          /* PROC_MAXLIFETIME  */
  YYSYMBOL_REALM = 77,                     /* REALM  */
  YYSYMBOL_REALNAME = 78,                  /* REALNAME  */
  YYSYMBOL_RESOLVEPROTOCOL = 79,           /* RESOLVEPROTOCOL  */
  YYSYMBOL_REQUIRED = 80,                  /* REQUIRED  */
  YYSYMBOL_SCHEDULEPOLICY = 81,            /* SCHEDULEPOLICY  */
  YYSYMBOL_SERVERCONFIG = 82,              /* SERVERCONFIG  */
  YYSYMBOL_CLIENTCONFIG = 83,              /* CLIENTCONFIG  */
  YYSYMBOL_SOCKET = 84,                    /* SOCKET  */
  YYSYMBOL_CLIENTSIDE_SOCKET = 85,         /* CLIENTSIDE_SOCKET  */
  YYSYMBOL_SNDBUF = 86,                    /* SNDBUF  */
  YYSYMBOL_RCVBUF = 87,                    /* RCVBUF  */
  YYSYMBOL_SOCKETPROTOCOL = 88,            /* SOCKETPROTOCOL  */
  YYSYMBOL_SOCKETOPTION_OPTID = 89,        /* SOCKETOPTION_OPTID  */
  YYSYMBOL_SRCHOST = 90,                   /* SRCHOST  */
  YYSYMBOL_NODNSMISMATCH = 91,             /* NODNSMISMATCH  */
  YYSYMBOL_NODNSUNKNOWN = 92,              /* NODNSUNKNOWN  */
  YYSYMBOL_CHECKREPLYAUTH = 93,            /* CHECKREPLYAUTH  */
  YYSYMBOL_USERNAME = 94,                  /* USERNAME  */
  YYSYMBOL_USER_PRIVILEGED = 95,           /* USER_PRIVILEGED  */
  YYSYMBOL_USER_UNPRIVILEGED = 96,         /* USER_UNPRIVILEGED  */
  YYSYMBOL_USER_LIBWRAP = 97,              /* USER_LIBWRAP  */
  YYSYMBOL_WORD__IN = 98,                  /* WORD__IN  */
  YYSYMBOL_ROUTE = 99,                     /* ROUTE  */
  YYSYMBOL_VIA = 100,                      /* VIA  */
  YYSYMBOL_GLOBALROUTEOPTION = 101,        /* GLOBALROUTEOPTION  */
  YYSYMBOL_BADROUTE_EXPIRE = 102,          /* BADROUTE_EXPIRE  */
  YYSYMBOL_MAXFAIL = 103,                  /* MAXFAIL  */
  YYSYMBOL_PORT = 104,                     /* PORT  */
  YYSYMBOL_NUMBER = 105,                   /* NUMBER  */
  YYSYMBOL_BANDWIDTH = 106,                /* BANDWIDTH  */
  YYSYMBOL_BOUNCE = 107,                   /* BOUNCE  */
  YYSYMBOL_BSDAUTHSTYLE = 108,             /* BSDAUTHSTYLE  */
  YYSYMBOL_BSDAUTHSTYLENAME = 109,         /* BSDAUTHSTYLENAME  */
  YYSYMBOL_COMMAND = 110,                  /* COMMAND  */
  YYSYMBOL_COMMAND_BIND = 111,             /* COMMAND_BIND  */
  YYSYMBOL_COMMAND_CONNECT = 112,          /* COMMAND_CONNECT  */
  YYSYMBOL_COMMAND_UDPASSOCIATE = 113,     /* COMMAND_UDPASSOCIATE  */
  YYSYMBOL_COMMAND_BINDREPLY = 114,        /* COMMAND_BINDREPLY  */
  YYSYMBOL_COMMAND_UDPREPLY = 115,         /* COMMAND_UDPREPLY  */
  YYSYMBOL_ACTION = 116,                   /* ACTION  */
  YYSYMBOL_FROM = 117,                     /* FROM  */
  YYSYMBOL_TO = 118,                       /* TO  */
  YYSYMBOL_GSSAPIENCTYPE = 119,            /* GSSAPIENCTYPE  */
  YYSYMBOL_GSSAPIENC_ANY = 120,            /* GSSAPIENC_ANY  */
  YYSYMBOL_GSSAPIENC_CLEAR = 121,          /* GSSAPIENC_CLEAR  */
  YYSYMBOL_GSSAPIENC_INTEGRITY = 122,      /* GSSAPIENC_INTEGRITY  */
  YYSYMBOL_GSSAPIENC_CONFIDENTIALITY = 123, /* GSSAPIENC_CONFIDENTIALITY  */
  YYSYMBOL_GSSAPIENC_PERMESSAGE = 124,     /* GSSAPIENC_PERMESSAGE  */
  YYSYMBOL_GSSAPIKEYTAB = 125,             /* GSSAPIKEYTAB  */
  YYSYMBOL_GSSAPISERVICE = 126,            /* GSSAPISERVICE  */
  YYSYMBOL_GSSAPISERVICENAME = 127,        /* GSSAPISERVICENAME  */
  YYSYMBOL_GSSAPIKEYTABNAME = 128,         /* GSSAPIKEYTABNAME  */
  YYSYMBOL_IPV4 = 129,                     /* IPV4  */
  YYSYMBOL_IPV6 = 130,                     /* IPV6  */
  YYSYMBOL_IPVANY = 131,                   /* IPVANY  */
  YYSYMBOL_DOMAINNAME = 132,               /* DOMAINNAME  */
  YYSYMBOL_IFNAME = 133,                   /* IFNAME  */
  YYSYMBOL_URL = 134,                      /* URL  */
  YYSYMBOL_LDAPATTRIBUTE = 135,            /* LDAPATTRIBUTE  */
  YYSYMBOL_LDAPATTRIBUTE_AD = 136,         /* LDAPATTRIBUTE_AD  */
  YYSYMBOL_LDAPATTRIBUTE_HEX = 137,        /* LDAPATTRIBUTE_HEX  */
  YYSYMBOL_LDAPATTRIBUTE_AD_HEX = 138,     /* LDAPATTRIBUTE_AD_HEX  */
  YYSYMBOL_LDAPBASEDN = 139,               /* LDAPBASEDN  */
  YYSYMBOL_LDAP_BASEDN = 140,              /* LDAP_BASEDN  */
  YYSYMBOL_LDAPBASEDN_HEX = 141,           /* LDAPBASEDN_HEX  */
  YYSYMBOL_LDAPBASEDN_HEX_ALL = 142,       /* LDAPBASEDN_HEX_ALL  */
  YYSYMBOL_LDAPCERTFILE = 143,             /* LDAPCERTFILE  */
  YYSYMBOL_LDAPCERTPATH = 144,             /* LDAPCERTPATH  */
  YYSYMBOL_LDAPPORT = 145,                 /* LDAPPORT  */
  YYSYMBOL_LDAPPORTSSL = 146,              /* LDAPPORTSSL  */
  YYSYMBOL_LDAPDEBUG = 147,                /* LDAPDEBUG  */
  YYSYMBOL_LDAPDEPTH = 148,                /* LDAPDEPTH  */
  YYSYMBOL_LDAPAUTO = 149,                 /* LDAPAUTO  */
  YYSYMBOL_LDAPSEARCHTIME = 150,           /* LDAPSEARCHTIME  */
  YYSYMBOL_LDAPDOMAIN = 151,               /* LDAPDOMAIN  */
  YYSYMBOL_LDAP_DOMAIN = 152,              /* LDAP_DOMAIN  */
  YYSYMBOL_LDAPFILTER = 153,               /* LDAPFILTER  */
  YYSYMBOL_LDAPFILTER_AD = 154,            /* LDAPFILTER_AD  */
  YYSYMBOL_LDAPFILTER_HEX = 155,           /* LDAPFILTER_HEX  */
  YYSYMBOL_LDAPFILTER_AD_HEX = 156,        /* LDAPFILTER_AD_HEX  */
  YYSYMBOL_LDAPGROUP = 157,                /* LDAPGROUP  */
  YYSYMBOL_LDAPGROUP_NAME = 158,           /* LDAPGROUP_NAME  */
  YYSYMBOL_LDAPGROUP_HEX = 159,            /* LDAPGROUP_HEX  */
  YYSYMBOL_LDAPGROUP_HEX_ALL = 160,        /* LDAPGROUP_HEX_ALL  */
  YYSYMBOL_LDAPKEYTAB = 161,               /* LDAPKEYTAB  */
  YYSYMBOL_LDAPKEYTABNAME = 162,           /* LDAPKEYTABNAME  */
  YYSYMBOL_LDAPDEADTIME = 163,             /* LDAPDEADTIME  */
  YYSYMBOL_LDAPSERVER = 164,               /* LDAPSERVER  */
  YYSYMBOL_LDAPSERVER_NAME = 165,          /* LDAPSERVER_NAME  */
  YYSYMBOL_LDAPAUTHSERVER = 166,           /* LDAPAUTHSERVER  */
  YYSYMBOL_LDAPAUTHKEYTAB = 167,           /* LDAPAUTHKEYTAB  */
  YYSYMBOL_LDAPSSL = 168,                  /* LDAPSSL  */
  YYSYMBOL_LDAPCERTCHECK = 169,            /* LDAPCERTCHECK  */
  YYSYMBOL_LDAPKEEPREALM = 170,            /* LDAPKEEPREALM  */
  YYSYMBOL_LDAPTIMEOUT = 171,              /* LDAPTIMEOUT  */
  YYSYMBOL_LDAPCACHE = 172,                /* LDAPCACHE  */
  YYSYMBOL_LDAPCACHEPOS = 173,             /* LDAPCACHEPOS  */
  YYSYMBOL_LDAPCACHENEG = 174,             /* LDAPCACHENEG  */
  YYSYMBOL_LDAPURL = 175,                  /* LDAPURL  */
  YYSYMBOL_LDAP_URL = 176,                 /* LDAP_URL  */
  YYSYMBOL_LDAPAUTHBASEDN = 177,           /* LDAPAUTHBASEDN  */
  YYSYMBOL_LDAPAUTHBASEDN_HEX = 178,       /* LDAPAUTHBASEDN_HEX  */
  YYSYMBOL_LDAPAUTHBASEDN_HEX_ALL = 179,   /* LDAPAUTHBASEDN_HEX_ALL  */
  YYSYMBOL_LDAPAUTHURL = 180,              /* LDAPAUTHURL  */
  YYSYMBOL_LDAPAUTHPORT = 181,             /* LDAPAUTHPORT  */
  YYSYMBOL_LDAPAUTHPORTSSL = 182,          /* LDAPAUTHPORTSSL  */
  YYSYMBOL_LDAPAUTHDEBUG = 183,            /* LDAPAUTHDEBUG  */
  YYSYMBOL_LDAPAUTHSSL = 184,              /* LDAPAUTHSSL  */
  YYSYMBOL_LDAPAUTHAUTO = 185,             /* LDAPAUTHAUTO  */
  YYSYMBOL_LDAPAUTHCERTCHECK = 186,        /* LDAPAUTHCERTCHECK  */
  YYSYMBOL_LDAPAUTHFILTER = 187,           /* LDAPAUTHFILTER  */
  YYSYMBOL_LDAPAUTHDOMAIN = 188,           /* LDAPAUTHDOMAIN  */
  YYSYMBOL_LDAPAUTHCERTFILE = 189,         /* LDAPAUTHCERTFILE  */
  YYSYMBOL_LDAPAUTHCERTPATH = 190,         /* LDAPAUTHCERTPATH  */
  YYSYMBOL_LDAPAUTHKEEPREALM = 191,        /* LDAPAUTHKEEPREALM  */
  YYSYMBOL_LDAP_FILTER = 192,              /* LDAP_FILTER  */
  YYSYMBOL_LDAP_ATTRIBUTE = 193,           /* LDAP_ATTRIBUTE  */
  YYSYMBOL_LDAP_CERTFILE = 194,            /* LDAP_CERTFILE  */
  YYSYMBOL_LDAP_CERTPATH = 195,            /* LDAP_CERTPATH  */
  YYSYMBOL_LIBWRAPSTART = 196,             /* LIBWRAPSTART  */
  YYSYMBOL_LIBWRAP_ALLOW = 197,            /* LIBWRAP_ALLOW  */
  YYSYMBOL_LIBWRAP_DENY = 198,             /* LIBWRAP_DENY  */
  YYSYMBOL_LIBWRAP_HOSTS_ACCESS = 199,     /* LIBWRAP_HOSTS_ACCESS  */
  YYSYMBOL_LINE = 200,                     /* LINE  */
  YYSYMBOL_OPERATOR = 201,                 /* OPERATOR  */
  YYSYMBOL_PACSID = 202,                   /* PACSID  */
  YYSYMBOL_PACSID_B64 = 203,               /* PACSID_B64  */
  YYSYMBOL_PACSID_FLAG = 204,              /* PACSID_FLAG  */
  YYSYMBOL_PACSID_NAME = 205,              /* PACSID_NAME  */
  YYSYMBOL_PAMSERVICENAME = 206,           /* PAMSERVICENAME  */
  YYSYMBOL_PROTOCOL = 207,                 /* PROTOCOL  */
  YYSYMBOL_PROTOCOL_TCP = 208,             /* PROTOCOL_TCP  */
  YYSYMBOL_PROTOCOL_UDP = 209,             /* PROTOCOL_UDP  */
  YYSYMBOL_PROTOCOL_FAKE = 210,            /* PROTOCOL_FAKE  */
  YYSYMBOL_PROXYPROTOCOL = 211,            /* PROXYPROTOCOL  */
  YYSYMBOL_PROXYPROTOCOL_SOCKS_V4 = 212,   /* PROXYPROTOCOL_SOCKS_V4  */
  YYSYMBOL_PROXYPROTOCOL_SOCKS_V5 = 213,   /* PROXYPROTOCOL_SOCKS_V5  */
  YYSYMBOL_PROXYPROTOCOL_HTTP = 214,       /* PROXYPROTOCOL_HTTP  */
  YYSYMBOL_PROXYPROTOCOL_UPNP = 215,       /* PROXYPROTOCOL_UPNP  */
  YYSYMBOL_REDIRECT = 216,                 /* REDIRECT  */
  YYSYMBOL_SENDSIDE = 217,                 /* SENDSIDE  */
  YYSYMBOL_RECVSIDE = 218,                 /* RECVSIDE  */
  YYSYMBOL_SERVICENAME = 219,              /* SERVICENAME  */
  YYSYMBOL_SESSION_INHERITABLE = 220,      /* SESSION_INHERITABLE  */
  YYSYMBOL_SESSIONMAX = 221,               /* SESSIONMAX  */
  YYSYMBOL_SESSIONTHROTTLE = 222,          /* SESSIONTHROTTLE  */
  YYSYMBOL_SESSIONSTATE_KEY = 223,         /* SESSIONSTATE_KEY  */
  YYSYMBOL_SESSIONSTATE_MAX = 224,         /* SESSIONSTATE_MAX  */
  YYSYMBOL_SESSIONSTATE_THROTTLE = 225,    /* SESSIONSTATE_THROTTLE  */
  YYSYMBOL_RULE_LOG = 226,                 /* RULE_LOG  */
  YYSYMBOL_RULE_LOG_CONNECT = 227,         /* RULE_LOG_CONNECT  */
  YYSYMBOL_RULE_LOG_DATA = 228,            /* RULE_LOG_DATA  */
  YYSYMBOL_RULE_LOG_DISCONNECT = 229,      /* RULE_LOG_DISCONNECT  */
  YYSYMBOL_RULE_LOG_ERROR = 230,           /* RULE_LOG_ERROR  */
  YYSYMBOL_RULE_LOG_IOOPERATION = 231,     /* RULE_LOG_IOOPERATION  */
  YYSYMBOL_RULE_LOG_TCPINFO = 232,         /* RULE_LOG_TCPINFO  */
  YYSYMBOL_STATEKEY = 233,                 /* STATEKEY  */
  YYSYMBOL_UDPPORTRANGE = 234,             /* UDPPORTRANGE  */
  YYSYMBOL_UDPCONNECTDST = 235,            /* UDPCONNECTDST  */
  YYSYMBOL_USER = 236,                     /* USER  */
  YYSYMBOL_GROUP = 237,                    /* GROUP  */
  YYSYMBOL_VERDICT_BLOCK = 238,            /* VERDICT_BLOCK  */
  YYSYMBOL_VERDICT_PASS = 239,             /* VERDICT_PASS  */
  YYSYMBOL_YES = 240,                      /* YES  */
  YYSYMBOL_NO = 241,                       /* NO  */
  YYSYMBOL_242_ = 242,                     /* ':'  */
  YYSYMBOL_243_ = 243,                     /* '.'  */
  YYSYMBOL_244_ = 244,                     /* '{'  */
  YYSYMBOL_245_ = 245,                     /* '}'  */
  YYSYMBOL_246_ = 246,                     /* '/'  */
  YYSYMBOL_247_ = 247,                     /* '-'  */
  YYSYMBOL_YYACCEPT = 248,                 /* $accept  */
  YYSYMBOL_configtype = 249,               /* configtype  */
  YYSYMBOL_250_1 = 250,                    /* $@1  */
  YYSYMBOL_serverobjects = 251,            /* serverobjects  */
  YYSYMBOL_serverobject = 252,             /* serverobject  */
  YYSYMBOL_serveroptions = 253,            /* serveroptions  */
  YYSYMBOL_serveroption = 254,             /* serveroption  */
  YYSYMBOL_logspecial = 255,               /* logspecial  */
  YYSYMBOL_256_2 = 256,                    /* $@2  */
  YYSYMBOL_257_3 = 257,                    /* $@3  */
  YYSYMBOL_internal_if_logoption = 258,    /* internal_if_logoption  */
  YYSYMBOL_259_4 = 259,                    /* $@4  */
  YYSYMBOL_external_if_logoption = 260,    /* external_if_logoption  */
  YYSYMBOL_261_5 = 261,                    /* $@5  */
  YYSYMBOL_rule_internal_logoption = 262,  /* rule_internal_logoption  */
  YYSYMBOL_263_6 = 263,                    /* $@6  */
  YYSYMBOL_rule_external_logoption = 264,  /* rule_external_logoption  */
  YYSYMBOL_265_7 = 265,                    /* $@7  */
  YYSYMBOL_loglevel = 266,                 /* loglevel  */
  YYSYMBOL_tcpoptions = 267,               /* tcpoptions  */
  YYSYMBOL_tcpoption = 268,                /* tcpoption  */
  YYSYMBOL_errors = 269,                   /* errors  */
  YYSYMBOL_errorobject = 270,              /* errorobject  */
  YYSYMBOL_timeout = 271,                  /* timeout  */
  YYSYMBOL_deprecated = 272,               /* deprecated  */
  YYSYMBOL_route = 273,                    /* route  */
  YYSYMBOL_274_8 = 274,                    /* $@8  */
  YYSYMBOL_275_9 = 275,                    /* $@9  */
  YYSYMBOL_routes = 276,                   /* routes  */
  YYSYMBOL_proxyprotocol = 277,            /* proxyprotocol  */
  YYSYMBOL_proxyprotocolname = 278,        /* proxyprotocolname  */
  YYSYMBOL_proxyprotocols = 279,           /* proxyprotocols  */
  YYSYMBOL_user = 280,                     /* user  */
  YYSYMBOL_username = 281,                 /* username  */
  YYSYMBOL_usernames = 282,                /* usernames  */
  YYSYMBOL_group = 283,                    /* group  */
  YYSYMBOL_groupname = 284,                /* groupname  */
  YYSYMBOL_groupnames = 285,               /* groupnames  */
  YYSYMBOL_extension = 286,                /* extension  */
  YYSYMBOL_extensionname = 287,            /* extensionname  */
  YYSYMBOL_extensions = 288,               /* extensions  */
  YYSYMBOL_ifprotocols = 289,              /* ifprotocols  */
  YYSYMBOL_ifprotocol = 290,               /* ifprotocol  */
  YYSYMBOL_internal = 291,                 /* internal  */
  YYSYMBOL_internalinit = 292,             /* internalinit  */
  YYSYMBOL_internal_protocol = 293,        /* internal_protocol  */
  YYSYMBOL_294_10 = 294,                   /* $@10  */
  YYSYMBOL_external = 295,                 /* external  */
  YYSYMBOL_externalinit = 296,             /* externalinit  */
  YYSYMBOL_external_protocol = 297,        /* external_protocol  */
  YYSYMBOL_298_11 = 298,                   /* $@11  */
  YYSYMBOL_external_rotation = 299,        /* external_rotation  */
  YYSYMBOL_clientoption = 300,             /* clientoption  */
  YYSYMBOL_clientoptions = 301,            /* clientoptions  */
  YYSYMBOL_global_routeoption = 302,       /* global_routeoption  */
  YYSYMBOL_errorlog = 303,                 /* errorlog  */
  YYSYMBOL_304_12 = 304,                   /* $@12  */
  YYSYMBOL_logoutput = 305,                /* logoutput  */
  YYSYMBOL_306_13 = 306,                   /* $@13  */
  YYSYMBOL_logoutputdevice = 307,          /* logoutputdevice  */
  YYSYMBOL_logoutputdevices = 308,         /* logoutputdevices  */
  YYSYMBOL_childstate = 309,               /* childstate  */
  YYSYMBOL_userids = 310,                  /* userids  */
  YYSYMBOL_user_privileged = 311,          /* user_privileged  */
  YYSYMBOL_user_unprivileged = 312,        /* user_unprivileged  */
  YYSYMBOL_user_libwrap = 313,             /* user_libwrap  */
  YYSYMBOL_userid = 314,                   /* userid  */
  YYSYMBOL_iotimeout = 315,                /* iotimeout  */
  YYSYMBOL_negotiatetimeout = 316,         /* negotiatetimeout  */
  YYSYMBOL_connecttimeout = 317,           /* connecttimeout  */
  YYSYMBOL_tcp_fin_timeout = 318,          /* tcp_fin_timeout  */
  YYSYMBOL_debugging = 319,                /* debugging  */
  YYSYMBOL_libwrapfiles = 320,             /* libwrapfiles  */
  YYSYMBOL_libwrap_allowfile = 321,        /* libwrap_allowfile  */
  YYSYMBOL_libwrap_denyfile = 322,         /* libwrap_denyfile  */
  YYSYMBOL_libwrap_hosts_access = 323,     /* libwrap_hosts_access  */
  YYSYMBOL_udpconnectdst = 324,            /* udpconnectdst  */
  YYSYMBOL_compatibility = 325,            /* compatibility  */
  YYSYMBOL_compatibilityname = 326,        /* compatibilityname  */
  YYSYMBOL_compatibilitynames = 327,       /* compatibilitynames  */
  YYSYMBOL_resolveprotocol = 328,          /* resolveprotocol  */
  YYSYMBOL_resolveprotocolname = 329,      /* resolveprotocolname  */
  YYSYMBOL_cpu = 330,                      /* cpu  */
  YYSYMBOL_cpuschedule = 331,              /* cpuschedule  */
  YYSYMBOL_cpuaffinity = 332,              /* cpuaffinity  */
  YYSYMBOL_socketoption = 333,             /* socketoption  */
  YYSYMBOL_334_14 = 334,                   /* $@14  */
  YYSYMBOL_socketoptionname = 335,         /* socketoptionname  */
  YYSYMBOL_socketoptionvalue = 336,        /* socketoptionvalue  */
  YYSYMBOL_socketside = 337,               /* socketside  */
  YYSYMBOL_srchost = 338,                  /* srchost  */
  YYSYMBOL_srchostoption = 339,            /* srchostoption  */
  YYSYMBOL_srchostoptions = 340,           /* srchostoptions  */
  YYSYMBOL_realm = 341,                    /* realm  */
  YYSYMBOL_global_clientmethod = 342,      /* global_clientmethod  */
  YYSYMBOL_343_15 = 343,                   /* $@15  */
  YYSYMBOL_global_socksmethod = 344,       /* global_socksmethod  */
  YYSYMBOL_345_16 = 345,                   /* $@16  */
  YYSYMBOL_socksmethod = 346,              /* socksmethod  */
  YYSYMBOL_socksmethods = 347,             /* socksmethods  */
  YYSYMBOL_socksmethodname = 348,          /* socksmethodname  */
  YYSYMBOL_clientmethod = 349,             /* clientmethod  */
  YYSYMBOL_clientmethods = 350,            /* clientmethods  */
  YYSYMBOL_clientmethodname = 351,         /* clientmethodname  */
  YYSYMBOL_monitor = 352,                  /* monitor  */
  YYSYMBOL_353_17 = 353,                   /* $@17  */
  YYSYMBOL_354_18 = 354,                   /* $@18  */
  YYSYMBOL_crule = 355,                    /* crule  */
  YYSYMBOL_356_19 = 356,                   /* $@19  */
  YYSYMBOL_alarm = 357,                    /* alarm  */
  YYSYMBOL_monitorside = 358,              /* monitorside  */
  YYSYMBOL_alarmside = 359,                /* alarmside  */
  YYSYMBOL_alarm_data = 360,               /* alarm_data  */
  YYSYMBOL_361_20 = 361,                   /* $@20  */
  YYSYMBOL_alarm_test = 362,               /* alarm_test  */
  YYSYMBOL_networkproblem = 363,           /* networkproblem  */
  YYSYMBOL_alarm_disconnect = 364,         /* alarm_disconnect  */
  YYSYMBOL_alarmperiod = 365,              /* alarmperiod  */
  YYSYMBOL_monitoroption = 366,            /* monitoroption  */
  YYSYMBOL_monitoroptions = 367,           /* monitoroptions  */
  YYSYMBOL_cruleoption = 368,              /* cruleoption  */
  YYSYMBOL_hrule = 369,                    /* hrule  */
  YYSYMBOL_370_21 = 370,                   /* $@21  */
  YYSYMBOL_cruleoptions = 371,             /* cruleoptions  */
  YYSYMBOL_hostidoption = 372,             /* hostidoption  */
  YYSYMBOL_hostid = 373,                   /* hostid  */
  YYSYMBOL_374_22 = 374,                   /* $@22  */
  YYSYMBOL_hostindex = 375,                /* hostindex  */
  YYSYMBOL_srule = 376,                    /* srule  */
  YYSYMBOL_377_23 = 377,                   /* $@23  */
  YYSYMBOL_sruleoptions = 378,             /* sruleoptions  */
  YYSYMBOL_sruleoption = 379,              /* sruleoption  */
  YYSYMBOL_genericruleoption = 380,        /* genericruleoption  */
  YYSYMBOL_ldapauthoption = 381,           /* ldapauthoption  */
  YYSYMBOL_ldapoption = 382,               /* ldapoption  */
  YYSYMBOL_ldapdebug = 383,                /* ldapdebug  */
  YYSYMBOL_ldapauthdebug = 384,            /* ldapauthdebug  */
  YYSYMBOL_ldapdomain = 385,               /* ldapdomain  */
  YYSYMBOL_ldapauthdomain = 386,           /* ldapauthdomain  */
  YYSYMBOL_ldapdepth = 387,                /* ldapdepth  */
  YYSYMBOL_ldapcertfile = 388,             /* ldapcertfile  */
  YYSYMBOL_ldapauthcertfile = 389,         /* ldapauthcertfile  */
  YYSYMBOL_ldapcertpath = 390,             /* ldapcertpath  */
  YYSYMBOL_ldapauthcertpath = 391,         /* ldapauthcertpath  */
  YYSYMBOL_ldapurl = 392,                  /* ldapurl  */
  YYSYMBOL_ldapauthurl = 393,              /* ldapauthurl  */
  YYSYMBOL_ldapauthbasedn = 394,           /* ldapauthbasedn  */
  YYSYMBOL_ldapauthbasedn_hex = 395,       /* ldapauthbasedn_hex  */
  YYSYMBOL_ldapauthbasedn_hex_all = 396,   /* ldapauthbasedn_hex_all  */
  YYSYMBOL_lbasedn = 397,                  /* lbasedn  */
  YYSYMBOL_lbasedn_hex = 398,              /* lbasedn_hex  */
  YYSYMBOL_lbasedn_hex_all = 399,          /* lbasedn_hex_all  */
  YYSYMBOL_ldapauthport = 400,             /* ldapauthport  */
  YYSYMBOL_ldapport = 401,                 /* ldapport  */
  YYSYMBOL_ldapauthportssl = 402,          /* ldapauthportssl  */
  YYSYMBOL_ldapportssl = 403,              /* ldapportssl  */
  YYSYMBOL_ldapssl = 404,                  /* ldapssl  */
  YYSYMBOL_ldapauthssl = 405,              /* ldapauthssl  */
  YYSYMBOL_ldapauto = 406,                 /* ldapauto  */
  YYSYMBOL_ldapauthauto = 407,             /* ldapauthauto  */
  YYSYMBOL_ldapcertcheck = 408,            /* ldapcertcheck  */
  YYSYMBOL_ldapauthcertcheck = 409,        /* ldapauthcertcheck  */
  YYSYMBOL_ldapauthkeeprealm = 410,        /* ldapauthkeeprealm  */
  YYSYMBOL_ldapkeeprealm = 411,            /* ldapkeeprealm  */
  YYSYMBOL_ldapfilter = 412,               /* ldapfilter  */
  YYSYMBOL_ldapauthfilter = 413,           /* ldapauthfilter  */
  YYSYMBOL_ldapfilter_ad = 414,            /* ldapfilter_ad  */
  YYSYMBOL_ldapfilter_hex = 415,           /* ldapfilter_hex  */
  YYSYMBOL_ldapfilter_ad_hex = 416,        /* ldapfilter_ad_hex  */
  YYSYMBOL_ldapattribute = 417,            /* ldapattribute  */
  YYSYMBOL_ldapattribute_ad = 418,         /* ldapattribute_ad  */
  YYSYMBOL_ldapattribute_hex = 419,        /* ldapattribute_hex  */
  YYSYMBOL_ldapattribute_ad_hex = 420,     /* ldapattribute_ad_hex  */
  YYSYMBOL_lgroup_hex = 421,               /* lgroup_hex  */
  YYSYMBOL_lgroup_hex_all = 422,           /* lgroup_hex_all  */
  YYSYMBOL_lgroup = 423,                   /* lgroup  */
  YYSYMBOL_lserver = 424,                  /* lserver  */
  YYSYMBOL_ldapauthserver = 425,           /* ldapauthserver  */
  YYSYMBOL_ldapkeytab = 426,               /* ldapkeytab  */
  YYSYMBOL_ldapauthkeytab = 427,           /* ldapauthkeytab  */
  YYSYMBOL_psid = 428,                     /* psid  */
  YYSYMBOL_psid_b64 = 429,                 /* psid_b64  */
  YYSYMBOL_psid_off = 430,                 /* psid_off  */
  YYSYMBOL_clientcompatibility = 431,      /* clientcompatibility  */
  YYSYMBOL_clientcompatibilityname = 432,  /* clientcompatibilityname  */
  YYSYMBOL_clientcompatibilitynames = 433, /* clientcompatibilitynames  */
  YYSYMBOL_verdict = 434,                  /* verdict  */
  YYSYMBOL_command = 435,                  /* command  */
  YYSYMBOL_commands = 436,                 /* commands  */
  YYSYMBOL_commandname = 437,              /* commandname  */
  YYSYMBOL_protocol = 438,                 /* protocol  */
  YYSYMBOL_protocols = 439,                /* protocols  */
  YYSYMBOL_protocolname = 440,             /* protocolname  */
  YYSYMBOL_fromto = 441,                   /* fromto  */
  YYSYMBOL_hostid_fromto = 442,            /* hostid_fromto  */
  YYSYMBOL_redirect = 443,                 /* redirect  */
  YYSYMBOL_sessionoption = 444,            /* sessionoption  */
  YYSYMBOL_sockssessionoption = 445,       /* sockssessionoption  */
  YYSYMBOL_crulesessionoption = 446,       /* crulesessionoption  */
  YYSYMBOL_sessioninheritable = 447,       /* sessioninheritable  */
  YYSYMBOL_sessionmax = 448,               /* sessionmax  */
  YYSYMBOL_sessionthrottle = 449,          /* sessionthrottle  */
  YYSYMBOL_sessionstate = 450,             /* sessionstate  */
  YYSYMBOL_sessionstate_key = 451,         /* sessionstate_key  */
  YYSYMBOL_sessionstate_keyinfo = 452,     /* sessionstate_keyinfo  */
  YYSYMBOL_453_24 = 453,                   /* $@24  */
  YYSYMBOL_sessionstate_max = 454,         /* sessionstate_max  */
  YYSYMBOL_sessionstate_throttle = 455,    /* sessionstate_throttle  */
  YYSYMBOL_bandwidth = 456,                /* bandwidth  */
  YYSYMBOL_log = 457,                      /* log  */
  YYSYMBOL_logname = 458,                  /* logname  */
  YYSYMBOL_logs = 459,                     /* logs  */
  YYSYMBOL_pamservicename = 460,           /* pamservicename  */
  YYSYMBOL_bsdauthstylename = 461,         /* bsdauthstylename  */
  YYSYMBOL_gssapiservicename = 462,        /* gssapiservicename  */
  YYSYMBOL_gssapikeytab = 463,             /* gssapikeytab  */
  YYSYMBOL_gssapienctype = 464,            /* gssapienctype  */
  YYSYMBOL_gssapienctypename = 465,        /* gssapienctypename  */
  YYSYMBOL_gssapienctypes = 466,           /* gssapienctypes  */
  YYSYMBOL_bounce = 467,                   /* bounce  */
  YYSYMBOL_libwrap = 468,                  /* libwrap  */
  YYSYMBOL_srcaddress = 469,               /* srcaddress  */
  YYSYMBOL_hostid_srcaddress = 470,        /* hostid_srcaddress  */
  YYSYMBOL_dstaddress = 471,               /* dstaddress  */
  YYSYMBOL_rdr_fromaddress = 472,          /* rdr_fromaddress  */
  YYSYMBOL_rdr_toaddress = 473,            /* rdr_toaddress  */
  YYSYMBOL_gateway = 474,                  /* gateway  */
  YYSYMBOL_routeoption = 475,              /* routeoption  */
  YYSYMBOL_routeoptions = 476,             /* routeoptions  */
  YYSYMBOL_routemethod = 477,              /* routemethod  */
  YYSYMBOL_from = 478,                     /* from  */
  YYSYMBOL_to = 479,                       /* to  */
  YYSYMBOL_rdr_from = 480,                 /* rdr_from  */
  YYSYMBOL_rdr_to = 481,                   /* rdr_to  */
  YYSYMBOL_bounceto = 482,                 /* bounceto  */
  YYSYMBOL_via = 483,                      /* via  */
  YYSYMBOL_externaladdress = 484,          /* externaladdress  */
  YYSYMBOL_address_without_port = 485,     /* address_without_port  */
  YYSYMBOL_address = 486,                  /* address  */
  YYSYMBOL_ipaddress = 487,                /* ipaddress  */
  YYSYMBOL_gwaddress = 488,                /* gwaddress  */
  YYSYMBOL_bouncetoaddress = 489,          /* bouncetoaddress  */
  YYSYMBOL_ipv4 = 490,                     /* ipv4  */
  YYSYMBOL_netmask_v4 = 491,               /* netmask_v4  */
  YYSYMBOL_ipv6 = 492,                     /* ipv6  */
  YYSYMBOL_netmask_v6 = 493,               /* netmask_v6  */
  YYSYMBOL_ipvany = 494,                   /* ipvany  */
  YYSYMBOL_netmask_vany = 495,             /* netmask_vany  */
  YYSYMBOL_domain = 496,                   /* domain  */
  YYSYMBOL_ifname = 497,                   /* ifname  */
  YYSYMBOL_url = 498,                      /* url  */
  YYSYMBOL_port = 499,                     /* port  */
  YYSYMBOL_gwport = 500,                   /* gwport  */
  YYSYMBOL_portnumber = 501,               /* portnumber  */
  YYSYMBOL_portrange = 502,                /* portrange  */
  YYSYMBOL_portstart = 503,                /* portstart  */
  YYSYMBOL_portend = 504,                  /* portend  */
  YYSYMBOL_portservice = 505,              /* portservice  */
  YYSYMBOL_portoperator = 506,             /* portoperator  */
  YYSYMBOL_udpportrange = 507,             /* udpportrange  */
  YYSYMBOL_udpportrange_start = 508,       /* udpportrange_start  */
  YYSYMBOL_udpportrange_end = 509,         /* udpportrange_end  */
  YYSYMBOL_number = 510,                   /* number  */
  YYSYMBOL_numbers = 511                   /* numbers  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  30
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   766

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  248
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  264
/* YYNRULES -- Number of rules.  */
#define YYNRULES  510
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  847

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   496


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,   247,   243,   246,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   242,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   244,     2,   245,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   653,   653,   653,   658,   661,   662,   665,   666,   667,
     668,   669,   672,   673,   675,   676,   677,   678,   679,   680,
     681,   682,   683,   684,   685,   686,   687,   688,   689,   690,
     691,   692,   693,   694,   695,   696,   697,   698,   699,   700,
     701,   709,   710,   710,   715,   715,   723,   723,   733,   733,
     743,   743,   753,   753,   764,   774,   775,   778,   785,   792,
     799,   808,   809,   812,   864,   865,   866,   867,   870,   877,
     878,   877,   889,   890,   892,   895,   898,   901,   904,   907,
     910,   911,   914,   917,   925,   926,   929,   932,   940,   941,
     944,   947,   956,   957,   960,   961,   965,   969,   975,  1000,
    1024,  1024,  1049,  1056,  1078,  1078,  1091,  1095,  1098,  1104,
    1105,  1106,  1107,  1108,  1109,  1110,  1113,  1114,  1117,  1125,
    1135,  1135,  1138,  1138,  1141,  1198,  1199,  1202,  1209,  1218,
    1219,  1220,  1223,  1236,  1249,  1268,  1284,  1289,  1292,  1298,
    1305,  1310,  1318,  1338,  1339,  1342,  1356,  1370,  1378,  1388,
    1392,  1399,  1402,  1406,  1412,  1413,  1416,  1419,  1422,  1429,
    1434,  1435,  1438,  1480,  1541,  1541,  1548,  1560,  1571,  1575,
    1592,  1595,  1601,  1604,  1608,  1611,  1617,  1618,  1621,  1633,
    1633,  1644,  1644,  1659,  1662,  1663,  1666,  1675,  1678,  1679,
    1683,  1691,  1691,  1691,  1708,  1708,  1737,  1738,  1739,  1742,
    1746,  1749,  1755,  1759,  1762,  1768,  1768,  1830,  1833,  1851,
    1882,  1887,  1890,  1891,  1892,  1893,  1896,  1897,  1900,  1905,
    1910,  1911,  1916,  1919,  1919,  1944,  1945,  1948,  1949,  1952,
    1952,  1963,  1975,  1975,  1990,  1991,  1995,  1996,  1997,  1998,
    1999,  2000,  2001,  2002,  2007,  2011,  2017,  2018,  2019,  2020,
    2021,  2022,  2023,  2024,  2025,  2026,  2027,  2028,  2029,  2034,
    2039,  2047,  2052,  2075,  2076,  2079,  2080,  2081,  2082,  2083,
    2084,  2085,  2086,  2087,  2088,  2089,  2090,  2091,  2092,  2093,
    2094,  2095,  2098,  2099,  2100,  2101,  2102,  2103,  2104,  2105,
    2106,  2107,  2108,  2109,  2110,  2111,  2112,  2113,  2114,  2115,
    2116,  2117,  2118,  2119,  2120,  2121,  2122,  2123,  2124,  2125,
    2129,  2134,  2143,  2148,  2157,  2171,  2185,  2196,  2210,  2224,
    2238,  2254,  2266,  2280,  2292,  2304,  2316,  2328,  2340,  2352,
    2363,  2374,  2385,  2396,  2401,  2410,  2415,  2424,  2429,  2438,
    2443,  2452,  2457,  2466,  2471,  2480,  2485,  2495,  2500,  2509,
    2520,  2531,  2546,  2560,  2574,  2589,  2603,  2617,  2631,  2643,
    2657,  2671,  2683,  2695,  2710,  2725,  2743,  2761,  2767,  2777,
    2780,  2789,  2790,  2794,  2799,  2806,  2809,  2810,  2813,  2816,
    2819,  2825,  2829,  2835,  2838,  2839,  2842,  2845,  2851,  2854,
    2857,  2858,  2859,  2862,  2863,  2864,  2867,  2870,  2871,  2874,
    2878,  2884,  2893,  2902,  2903,  2904,  2905,  2908,  2936,  2936,
    2947,  2955,  2964,  2973,  2976,  2980,  2983,  2986,  2989,  2992,
    2998,  2999,  3003,  3015,  3028,  3040,  3056,  3059,  3065,  3068,
    3071,  3074,  3082,  3083,  3086,  3089,  3129,  3132,  3135,  3138,
    3141,  3151,  3154,  3155,  3156,  3157,  3158,  3159,  3160,  3161,
    3162,  3163,  3164,  3172,  3173,  3176,  3179,  3184,  3189,  3194,
    3199,  3207,  3212,  3213,  3214,  3215,  3218,  3219,  3220,  3223,
    3227,  3228,  3229,  3230,  3231,  3233,  3235,  3236,  3237,  3238,
    3241,  3242,  3246,  3254,  3261,  3267,  3275,  3284,  3292,  3302,
    3308,  3315,  3322,  3323,  3324,  3325,  3328,  3329,  3332,  3333,
    3336,  3344,  3350,  3356,  3391,  3397,  3400,  3407,  3419,  3424,
    3425
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "ALARM",
  "ALARMTYPE_DATA", "ALARMTYPE_DISCONNECT", "ALARMIF_INTERNAL",
  "ALARMIF_EXTERNAL", "TCPOPTION_DISABLED", "ECN", "SACK", "TIMESTAMPS",
  "WSCALE", "MTU_ERROR", "CLIENTCOMPATIBILITY", "NECGSSAPI", "CLIENTRULE",
  "HOSTIDRULE", "SOCKSRULE", "COMPATIBILITY", "SAMEPORT", "DRAFT_5_05",
  "CONNECTTIMEOUT", "TCP_FIN_WAIT", "CPU", "MASK", "SCHEDULE",
  "CPUMASK_ANYCPU", "DEBUGGING", "DEPRECATED", "ERRORLOG", "LOGOUTPUT",
  "LOGFILE", "LOGTYPE_ERROR", "LOGTYPE_TCP_DISABLED",
  "LOGTYPE_TCP_ENABLED", "LOGIF_INTERNAL", "LOGIF_EXTERNAL", "ERRORVALUE",
  "EXTENSION", "BIND", "PRIVILEGED", "EXTERNAL_PROTOCOL",
  "INTERNAL_PROTOCOL", "EXTERNAL_ROTATION", "SAMESAME", "GROUPNAME",
  "HOSTID", "HOSTINDEX", "INTERFACE", "SOCKETOPTION_SYMBOLICVALUE",
  "INTERNAL", "EXTERNAL", "INTERNALSOCKET", "EXTERNALSOCKET", "IOTIMEOUT",
  "IOTIMEOUT_TCP", "IOTIMEOUT_UDP", "NEGOTIATETIMEOUT", "LIBWRAP_FILE",
  "LOGLEVEL", "SOCKSMETHOD", "CLIENTMETHOD", "METHOD", "METHODNAME",
  "NONE", "BSDAUTH", "GSSAPI", "PAM_ADDRESS", "PAM_ANY", "PAM_USERNAME",
  "RFC931", "UNAME", "MONITOR", "PROCESSTYPE", "PROC_MAXREQUESTS",
  "PROC_MAXLIFETIME", "REALM", "REALNAME", "RESOLVEPROTOCOL", "REQUIRED",
  "SCHEDULEPOLICY", "SERVERCONFIG", "CLIENTCONFIG", "SOCKET",
  "CLIENTSIDE_SOCKET", "SNDBUF", "RCVBUF", "SOCKETPROTOCOL",
  "SOCKETOPTION_OPTID", "SRCHOST", "NODNSMISMATCH", "NODNSUNKNOWN",
  "CHECKREPLYAUTH", "USERNAME", "USER_PRIVILEGED", "USER_UNPRIVILEGED",
  "USER_LIBWRAP", "WORD__IN", "ROUTE", "VIA", "GLOBALROUTEOPTION",
  "BADROUTE_EXPIRE", "MAXFAIL", "PORT", "NUMBER", "BANDWIDTH", "BOUNCE",
  "BSDAUTHSTYLE", "BSDAUTHSTYLENAME", "COMMAND", "COMMAND_BIND",
  "COMMAND_CONNECT", "COMMAND_UDPASSOCIATE", "COMMAND_BINDREPLY",
  "COMMAND_UDPREPLY", "ACTION", "FROM", "TO", "GSSAPIENCTYPE",
  "GSSAPIENC_ANY", "GSSAPIENC_CLEAR", "GSSAPIENC_INTEGRITY",
  "GSSAPIENC_CONFIDENTIALITY", "GSSAPIENC_PERMESSAGE", "GSSAPIKEYTAB",
  "GSSAPISERVICE", "GSSAPISERVICENAME", "GSSAPIKEYTABNAME", "IPV4", "IPV6",
  "IPVANY", "DOMAINNAME", "IFNAME", "URL", "LDAPATTRIBUTE",
  "LDAPATTRIBUTE_AD", "LDAPATTRIBUTE_HEX", "LDAPATTRIBUTE_AD_HEX",
  "LDAPBASEDN", "LDAP_BASEDN", "LDAPBASEDN_HEX", "LDAPBASEDN_HEX_ALL",
  "LDAPCERTFILE", "LDAPCERTPATH", "LDAPPORT", "LDAPPORTSSL", "LDAPDEBUG",
  "LDAPDEPTH", "LDAPAUTO", "LDAPSEARCHTIME", "LDAPDOMAIN", "LDAP_DOMAIN",
  "LDAPFILTER", "LDAPFILTER_AD", "LDAPFILTER_HEX", "LDAPFILTER_AD_HEX",
  "LDAPGROUP", "LDAPGROUP_NAME", "LDAPGROUP_HEX", "LDAPGROUP_HEX_ALL",
  "LDAPKEYTAB", "LDAPKEYTABNAME", "LDAPDEADTIME", "LDAPSERVER",
  "LDAPSERVER_NAME", "LDAPAUTHSERVER", "LDAPAUTHKEYTAB", "LDAPSSL",
  "LDAPCERTCHECK", "LDAPKEEPREALM", "LDAPTIMEOUT", "LDAPCACHE",
  "LDAPCACHEPOS", "LDAPCACHENEG", "LDAPURL", "LDAP_URL", "LDAPAUTHBASEDN",
  "LDAPAUTHBASEDN_HEX", "LDAPAUTHBASEDN_HEX_ALL", "LDAPAUTHURL",
  "LDAPAUTHPORT", "LDAPAUTHPORTSSL", "LDAPAUTHDEBUG", "LDAPAUTHSSL",
  "LDAPAUTHAUTO", "LDAPAUTHCERTCHECK", "LDAPAUTHFILTER", "LDAPAUTHDOMAIN",
  "LDAPAUTHCERTFILE", "LDAPAUTHCERTPATH", "LDAPAUTHKEEPREALM",
  "LDAP_FILTER", "LDAP_ATTRIBUTE", "LDAP_CERTFILE", "LDAP_CERTPATH",
  "LIBWRAPSTART", "LIBWRAP_ALLOW", "LIBWRAP_DENY", "LIBWRAP_HOSTS_ACCESS",
  "LINE", "OPERATOR", "PACSID", "PACSID_B64", "PACSID_FLAG", "PACSID_NAME",
  "PAMSERVICENAME", "PROTOCOL", "PROTOCOL_TCP", "PROTOCOL_UDP",
  "PROTOCOL_FAKE", "PROXYPROTOCOL", "PROXYPROTOCOL_SOCKS_V4",
  "PROXYPROTOCOL_SOCKS_V5", "PROXYPROTOCOL_HTTP", "PROXYPROTOCOL_UPNP",
  "REDIRECT", "SENDSIDE", "RECVSIDE", "SERVICENAME", "SESSION_INHERITABLE",
  "SESSIONMAX", "SESSIONTHROTTLE", "SESSIONSTATE_KEY", "SESSIONSTATE_MAX",
  "SESSIONSTATE_THROTTLE", "RULE_LOG", "RULE_LOG_CONNECT", "RULE_LOG_DATA",
  "RULE_LOG_DISCONNECT", "RULE_LOG_ERROR", "RULE_LOG_IOOPERATION",
  "RULE_LOG_TCPINFO", "STATEKEY", "UDPPORTRANGE", "UDPCONNECTDST", "USER",
  "GROUP", "VERDICT_BLOCK", "VERDICT_PASS", "YES", "NO", "':'", "'.'",
  "'{'", "'}'", "'/'", "'-'", "$accept", "configtype", "$@1",
  "serverobjects", "serverobject", "serveroptions", "serveroption",
  "logspecial", "$@2", "$@3", "internal_if_logoption", "$@4",
  "external_if_logoption", "$@5", "rule_internal_logoption", "$@6",
  "rule_external_logoption", "$@7", "loglevel", "tcpoptions", "tcpoption",
  "errors", "errorobject", "timeout", "deprecated", "route", "$@8", "$@9",
  "routes", "proxyprotocol", "proxyprotocolname", "proxyprotocols", "user",
  "username", "usernames", "group", "groupname", "groupnames", "extension",
  "extensionname", "extensions", "ifprotocols", "ifprotocol", "internal",
  "internalinit", "internal_protocol", "$@10", "external", "externalinit",
  "external_protocol", "$@11", "external_rotation", "clientoption",
  "clientoptions", "global_routeoption", "errorlog", "$@12", "logoutput",
  "$@13", "logoutputdevice", "logoutputdevices", "childstate", "userids",
  "user_privileged", "user_unprivileged", "user_libwrap", "userid",
  "iotimeout", "negotiatetimeout", "connecttimeout", "tcp_fin_timeout",
  "debugging", "libwrapfiles", "libwrap_allowfile", "libwrap_denyfile",
  "libwrap_hosts_access", "udpconnectdst", "compatibility",
  "compatibilityname", "compatibilitynames", "resolveprotocol",
  "resolveprotocolname", "cpu", "cpuschedule", "cpuaffinity",
  "socketoption", "$@14", "socketoptionname", "socketoptionvalue",
  "socketside", "srchost", "srchostoption", "srchostoptions", "realm",
  "global_clientmethod", "$@15", "global_socksmethod", "$@16",
  "socksmethod", "socksmethods", "socksmethodname", "clientmethod",
  "clientmethods", "clientmethodname", "monitor", "$@17", "$@18", "crule",
  "$@19", "alarm", "monitorside", "alarmside", "alarm_data", "$@20",
  "alarm_test", "networkproblem", "alarm_disconnect", "alarmperiod",
  "monitoroption", "monitoroptions", "cruleoption", "hrule", "$@21",
  "cruleoptions", "hostidoption", "hostid", "$@22", "hostindex", "srule",
  "$@23", "sruleoptions", "sruleoption", "genericruleoption",
  "ldapauthoption", "ldapoption", "ldapdebug", "ldapauthdebug",
  "ldapdomain", "ldapauthdomain", "ldapdepth", "ldapcertfile",
  "ldapauthcertfile", "ldapcertpath", "ldapauthcertpath", "ldapurl",
  "ldapauthurl", "ldapauthbasedn", "ldapauthbasedn_hex",
  "ldapauthbasedn_hex_all", "lbasedn", "lbasedn_hex", "lbasedn_hex_all",
  "ldapauthport", "ldapport", "ldapauthportssl", "ldapportssl", "ldapssl",
  "ldapauthssl", "ldapauto", "ldapauthauto", "ldapcertcheck",
  "ldapauthcertcheck", "ldapauthkeeprealm", "ldapkeeprealm", "ldapfilter",
  "ldapauthfilter", "ldapfilter_ad", "ldapfilter_hex", "ldapfilter_ad_hex",
  "ldapattribute", "ldapattribute_ad", "ldapattribute_hex",
  "ldapattribute_ad_hex", "lgroup_hex", "lgroup_hex_all", "lgroup",
  "lserver", "ldapauthserver", "ldapkeytab", "ldapauthkeytab", "psid",
  "psid_b64", "psid_off", "clientcompatibility", "clientcompatibilityname",
  "clientcompatibilitynames", "verdict", "command", "commands",
  "commandname", "protocol", "protocols", "protocolname", "fromto",
  "hostid_fromto", "redirect", "sessionoption", "sockssessionoption",
  "crulesessionoption", "sessioninheritable", "sessionmax",
  "sessionthrottle", "sessionstate", "sessionstate_key",
  "sessionstate_keyinfo", "$@24", "sessionstate_max",
  "sessionstate_throttle", "bandwidth", "log", "logname", "logs",
  "pamservicename", "bsdauthstylename", "gssapiservicename",
  "gssapikeytab", "gssapienctype", "gssapienctypename", "gssapienctypes",
  "bounce", "libwrap", "srcaddress", "hostid_srcaddress", "dstaddress",
  "rdr_fromaddress", "rdr_toaddress", "gateway", "routeoption",
  "routeoptions", "routemethod", "from", "to", "rdr_from", "rdr_to",
  "bounceto", "via", "externaladdress", "address_without_port", "address",
  "ipaddress", "gwaddress", "bouncetoaddress", "ipv4", "netmask_v4",
  "ipv6", "netmask_v6", "ipvany", "netmask_vany", "domain", "ifname",
  "url", "port", "gwport", "portnumber", "portrange", "portstart",
  "portend", "portservice", "portoperator", "udpportrange",
  "udpportrange_start", "udpportrange_end", "number", "numbers", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-694)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-217)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      10,  -694,   158,    41,   326,  -196,  -194,  -192,  -694,  -179,
    -167,  -127,  -117,  -115,  -105,   -99,     9,  -694,  -694,   158,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,   -95,   -80,  -694,  -694,   -73,   -65,   -63,   -19,  -694,
    -694,  -694,  -694,     7,    11,    16,    81,    85,    86,    87,
      89,    90,    91,    92,    93,    94,  -694,   326,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,   174,  -694,
    -694,  -694,  -694,   161,   178,   183,  -694,  -694,   188,   202,
     205,   211,    -7,   100,   102,  -694,   135,   197,   203,   103,
     104,   286,  -694,  -694,    24,   110,   116,  -694,  -694,   225,
     255,   283,   129,   270,   270,   270,   307,   308,    -2,     5,
      43,  -694,   128,  -694,  -694,  -694,   340,   340,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,   268,   269,  -694,  -694,
    -694,  -694,   197,  -694,   132,   133,   325,   325,  -694,   286,
    -694,   118,   118,  -694,  -694,  -694,   -11,    67,   322,   327,
    -694,  -694,  -694,  -694,  -694,  -694,   129,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
     340,  -694,  -694,  -694,  -694,   145,  -694,   316,   318,  -694,
     152,   153,  -694,  -694,  -694,  -694,   118,  -694,  -694,  -694,
    -694,  -694,  -694,   296,  -694,  -694,   160,   162,   163,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,   322,  -694,
    -694,   327,  -694,    22,    22,    22,   166,     8,  -694,  -694,
     165,   169,   198,   198,  -694,   -67,  -694,   -15,   299,   309,
    -694,  -694,  -694,  -694,   168,   171,   173,  -694,  -694,  -694,
     176,    19,   314,   332,   182,   184,   186,  -694,  -694,  -694,
    -694,   -66,  -694,   185,   -66,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,    48,    48,   529,    29,    -6,   187,   189,   191,
     192,   193,   194,   196,   199,   303,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,    19,   313,  -694,  -694,   314,
    -694,   179,   399,  -694,  -694,  -694,  -694,  -694,  -694,   334,
    -694,  -694,  -694,   200,   201,   204,   206,   207,   329,   208,
     209,   210,   212,   213,   139,   214,   215,   216,    39,   217,
     218,   219,   220,   221,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,    48,   313,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,   313,   222,   223,   224,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   237,   239,   240,
     241,   242,   243,   244,   245,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   271,   273,   275,   276,
    -694,   313,   529,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
     238,  -694,  -694,  -694,    18,   313,  -694,  -694,  -694,  -694,
    -694,  -694,   425,   322,    46,    88,   317,   353,    78,    -8,
    -694,  -694,   277,  -694,  -694,   344,   335,   278,  -694,   362,
    -694,  -694,   399,   140,   140,  -694,  -694,   284,   285,  -694,
     392,   322,   327,   405,  -694,   287,   311,   321,   328,    49,
     293,  -694,   412,  -694,   289,    51,   427,   429,   302,  -694,
     431,   432,   -37,   444,   493,  -694,    48,    48,   335,   298,
     433,   348,   350,   351,   352,   406,   407,   408,   355,   358,
     445,   449,   -77,   450,    57,   404,   365,   366,   368,   370,
     409,   410,   411,   401,   413,   414,   402,    59,    61,    63,
     394,   440,   441,   448,   395,   467,   468,   -71,    65,    68,
      72,   397,   442,   398,   400,    77,   488,   529,  -694,   354,
    -694,   356,  -694,    20,  -694,   425,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,    46,  -694,  -694,  -694,  -694,  -694,
      88,  -694,  -694,  -694,  -694,  -694,  -694,    78,  -694,  -694,
    -694,  -694,  -694,    -8,  -694,   -11,  -694,    19,   357,  -694,
    -694,   359,   -11,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
     140,  -694,   325,   325,   -11,  -694,  -694,  -694,  -694,    53,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,   -11,  -694,  -694,
    -694,   360,  -694,   548,  -694,   361,  -694,  -694,  -694,  -694,
    -694,  -694,   -37,  -694,  -694,  -694,   444,  -694,  -694,   493,
     363,   364,  -694,   -11,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,   495,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,   497,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,   367,   371,   590,    97,   499,   372,  -694,  -694,  -694,
    -694,  -694,  -694,   373,   146,   -11,  -694,  -694,   369,   378,
    -694,   501,  -694,   501,  -694,   505,  -694,   506,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,   508,  -694,  -694,  -694,
    -694,  -694,   380,   377,  -694,  -694,  -694,   501,  -694,   501,
    -694,  -694,  -694,   198,   198,   423,  -694,  -694,  -694,  -694,
    -694,  -694,   510,   520,  -694,  -694,  -694,  -694,   -66,   528,
     530,  -694,   522,   524,  -694,  -694,  -694
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       0,     2,   116,     0,    12,     0,     0,     0,    68,     0,
       0,     0,     0,     0,     0,     0,     0,   115,   110,   116,
      72,   111,   112,   113,    65,    66,    64,    67,   109,   114,
       1,     0,     0,    46,    48,     0,     0,     0,     0,    99,
     103,   170,   171,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     5,    12,    30,    24,
      37,    18,    20,    28,    29,    21,    22,    23,    27,    19,
      33,    14,    39,   129,   130,   131,    17,    32,   143,   144,
      31,    38,    15,    35,    16,   160,   161,    40,     0,    36,
      34,    25,    26,     0,     0,     0,   120,   122,     0,     0,
       0,     0,     0,     0,     0,   117,     4,     0,     0,     0,
       0,     0,   104,   100,     0,     0,     0,   181,   179,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       3,    13,     0,   140,   141,   142,     0,     0,   136,   137,
     138,   139,   158,   159,   157,   156,     0,     0,    69,    73,
     152,   153,   154,   151,     0,     0,     0,     0,    91,    92,
      90,     0,     0,   107,   106,   108,     0,     0,     0,     0,
     127,   128,   178,   173,   174,   175,   176,   172,   135,   132,
     133,   134,   145,   146,   147,   148,   149,   150,   194,   223,
     232,   191,     6,    11,    10,     7,     8,     9,   164,   124,
     125,   121,   123,   119,   118,     0,   155,     0,     0,    54,
       0,     0,    93,    96,    97,   105,    94,   101,   482,   485,
     487,   489,   490,   492,    98,   466,   471,   473,   475,   467,
     468,   102,   462,   463,   464,   465,   186,   182,   184,   190,
     180,   188,   177,     0,     0,     0,     0,     0,   126,    70,
       0,     0,     0,     0,    95,     0,   469,     0,     0,     0,
     185,   189,   373,   374,     0,     0,     0,   192,   167,   166,
       0,   453,     0,     0,     0,     0,     0,    47,    49,   501,
     504,     0,   495,     0,     0,   483,   484,   470,   486,   472,
     488,   474,   225,   225,   234,   199,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   450,   445,   452,   444,
     443,   446,   447,   448,   449,   453,     0,   442,   508,   509,
     163,     0,     0,    42,    44,   503,   493,   499,   498,     0,
     494,    50,    52,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   254,   248,   263,   264,   249,   262,
     247,   246,   225,     0,   253,   227,   228,   222,   258,   259,
     260,   220,   219,   261,   398,   221,   397,   393,   394,   395,
     403,   404,   406,   405,   245,   256,   257,   252,   251,   250,
     218,   255,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     242,     0,   234,   238,   240,   239,   293,   272,   295,   271,
     294,   291,   279,   292,   280,   309,   267,   268,   269,   270,
     287,   288,   289,   273,   302,   274,   303,   304,   275,   286,
     276,   290,   278,   281,   300,   296,   277,   297,   299,   298,
     282,   283,   285,   284,   306,   307,   305,   308,   265,   301,
     266,   237,   241,   396,   243,   236,   244,   200,   201,   212,
       0,   196,   198,   197,   199,     0,   214,   213,   215,   169,
     168,   165,     0,     0,     0,     0,     0,     0,     0,     0,
     458,   451,     0,   454,   456,     0,     0,     0,   510,     0,
      63,    41,    61,     0,     0,   502,   500,     0,     0,   229,
       0,     0,     0,     0,   460,     0,     0,     0,     0,     0,
       0,   459,   391,   392,     0,     0,     0,     0,     0,   408,
       0,     0,     0,     0,     0,   226,   225,   225,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   234,   235,     0,
     205,     0,   217,   199,   370,   371,   369,   455,   378,   379,
     380,   381,   382,   375,   376,   427,   428,   429,   430,   431,
     432,   426,   425,   424,   386,   387,   383,   384,    75,    76,
      77,    78,    79,    80,    74,     0,   461,   453,     0,   457,
     388,     0,     0,   162,    62,    57,    58,    59,    60,    43,
      55,    45,     0,     0,     0,   231,   183,   187,   412,     0,
     435,   365,   366,   367,   368,   422,   390,     0,   399,   400,
     401,     0,   407,     0,   410,     0,   414,   415,   416,   417,
     418,   419,   420,   413,    83,    84,    82,    87,    88,    86,
       0,     0,   389,     0,   423,   354,   355,   356,   357,   326,
     327,   328,   317,   319,   330,   332,   310,     0,   316,   337,
     338,   314,   349,   351,   352,   353,   360,   358,   359,   363,
     361,   362,   364,   333,   334,   341,   342,   347,   348,   321,
     323,   324,   325,   322,   329,   331,   312,     0,   335,   336,
     339,   340,   343,   344,   350,   315,   318,   320,   345,   346,
     506,     0,     0,     0,   202,     0,     0,   372,   377,   433,
     385,    81,   439,     0,     0,     0,   436,    56,     0,     0,
     230,   496,   434,   496,   440,     0,   409,     0,   421,    85,
      89,   195,   224,   437,   311,   313,     0,   233,   208,   207,
     204,   203,     0,     0,   193,    71,   491,   496,   441,   496,
     478,   479,   438,     0,     0,     0,   480,   481,   402,   411,
     507,   505,     0,     0,   476,   477,    51,    53,     0,     0,
     210,   497,     0,     0,   209,   206,   211
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -694,  -694,  -694,  -694,  -694,   573,  -694,  -249,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -151,  -502,
    -694,    99,  -694,    79,    -3,   502,  -694,  -694,  -694,  -278,
    -694,   -20,  -694,   -72,  -694,  -694,   -69,  -694,    60,  -694,
     477,  -122,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,   619,    74,    75,  -694,    76,  -694,  -694,
     -92,  -694,  -694,  -694,  -694,  -694,   195,  -694,  -694,  -694,
    -694,    83,  -694,  -694,  -694,  -694,  -694,  -694,  -694,   489,
      84,  -694,  -694,  -694,  -694,    -4,  -694,  -694,  -694,  -694,
    -694,  -694,   466,  -694,  -694,  -694,  -694,  -694,  -694,  -219,
    -694,  -694,  -221,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -467,
    -694,  -694,  -694,  -271,  -285,  -694,  -694,   -50,  -694,  -694,
    -419,  -694,  -277,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,  -694,  -694,  -263,  -694,    21,    80,  -280,    13,  -694,
    -264,     2,  -694,  -334,  -694,  -694,  -276,  -694,  -694,  -694,
    -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,  -694,
    -694,   -58,  -694,  -694,  -260,  -259,  -258,  -694,    12,  -694,
    -694,  -694,  -694,    95,   301,   105,  -694,  -694,  -306,  -694,
     295,  -694,  -694,  -694,  -694,  -694,  -694,  -627,  -613,  -636,
    -694,  -694,   484,  -694,   491,  -694,  -694,  -694,  -165,  -164,
    -694,  -694,  -693,  -279,  -694,   424,  -694,  -694,  -169,  -694,
    -694,  -694,  -694,   341
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     3,     4,   130,   192,    56,    57,   277,   533,   534,
      58,   109,    59,   110,   354,   537,   355,   538,   210,   669,
     670,   531,   532,   356,    18,   149,   205,   271,   106,   306,
     653,   654,   357,   705,   706,   358,   708,   709,   307,   159,
     160,   215,   216,    63,   115,    64,   162,    65,   116,    66,
     161,    67,    19,    20,    21,    22,   136,    23,   137,   200,
     201,    71,    72,    73,    74,    75,   179,    24,    25,    26,
      27,    28,    77,    78,    79,    80,    81,    82,   152,   153,
      29,   145,    84,    85,    86,   359,   247,   270,   511,    88,
      89,   176,   177,    90,    91,   169,    92,   168,   360,   237,
     238,   361,   240,   241,   194,   246,   295,   195,   243,   499,
     500,   812,   501,   774,   502,   809,   503,   844,   504,   505,
     362,   196,   244,   363,   364,   365,   674,   366,   197,   245,
     441,   442,   367,   444,   445,   446,   447,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   469,   470,
     471,   472,   473,   474,   475,   476,   477,   478,   479,   480,
     481,   482,   483,   484,   485,   486,   487,   488,   489,   490,
     368,   369,   370,   371,   625,   626,   264,   310,   633,   634,
     372,   646,   647,   525,   567,   373,   374,   494,   375,   376,
     377,   378,   379,   380,   381,   693,   382,   383,   384,   385,
     702,   703,   386,   495,   387,   388,   389,   640,   641,   390,
     391,   526,   568,   660,   521,   553,   657,   315,   316,   317,
     527,   661,   522,   554,   545,   658,   231,   223,   224,   225,
     818,   792,   226,   287,   227,   289,   228,   291,   229,   230,
     821,   256,   826,   326,   282,   327,   536,   328,   284,   496,
     771,   831,   319,   320
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      87,    61,   234,   235,   278,   330,   211,   311,   309,   523,
     506,   312,   313,   314,   491,   507,   440,   443,   493,   260,
     261,     8,   392,   618,   497,   498,   497,   498,   726,   566,
     492,   508,   671,   297,   756,   497,   498,   622,   279,   279,
     217,    30,   782,   791,   509,   202,    93,   790,    94,   786,
      95,   311,   309,    87,    61,   312,   313,   314,    35,   188,
     189,   190,   297,    96,    62,   333,   334,   333,   334,   163,
       5,     6,    41,    42,   794,    97,   333,   334,    68,    69,
      70,    17,   298,    60,   331,   332,   803,    76,    83,   164,
     285,   565,     1,     2,   254,   333,   334,   268,    17,   510,
     827,    41,    42,    11,    12,    13,    14,   617,   248,   335,
     336,   103,   104,   269,   286,    98,   191,    62,   218,   219,
     220,   221,   222,   165,   834,    99,   835,   100,   299,   299,
     299,    68,    69,    70,   280,  -216,    60,   101,   300,   299,
      76,    83,   148,   102,   301,   302,  -216,   107,   817,   665,
     666,   667,   668,   325,   337,   338,   776,   628,   629,   630,
     631,   632,   491,   108,   440,   443,   493,   300,   787,   111,
     727,   623,   822,   301,   302,   281,   757,   112,   492,   113,
       5,     6,   218,   219,   220,   221,     7,     8,     9,    10,
     696,   697,   698,   699,   700,   701,   218,   219,   772,   221,
     222,   142,   143,   144,   648,   649,   650,   651,   635,   636,
     637,   638,   639,    11,    12,    13,    14,   150,   151,   506,
     173,   174,   175,   114,   507,   303,   303,   303,   154,   155,
     304,   274,   275,   276,   148,   305,   303,    15,   184,   185,
     508,   619,   620,   621,   339,   186,   187,   213,   214,   117,
     340,   341,   342,   118,   343,   303,   520,   551,   119,    16,
     262,   263,   132,  -216,   344,  -216,   133,   308,   345,   346,
     347,   348,   349,   350,   351,   218,   219,   220,   221,   222,
     816,   558,   559,   134,   352,   353,   644,   645,   135,   683,
     684,   688,   689,   138,   627,   710,   711,   729,   730,   743,
     744,   745,   746,   747,   748,   758,   759,   139,   760,   761,
     140,   308,   762,   763,   810,   811,   141,   768,   769,   180,
     181,   677,   676,   120,   265,   266,   158,   121,   122,   123,
     170,   124,   125,   126,   127,   128,   129,   491,   506,   440,
     443,   493,   146,   507,   147,    31,   156,   157,     5,     6,
      32,   783,   166,   492,     7,     8,     9,    10,   167,   508,
     171,   172,    33,    34,   178,    35,   182,   183,    36,    37,
      38,   198,   199,   203,   204,   207,   208,    39,    40,    41,
      42,    11,    12,    13,    14,   209,   236,    43,    44,   249,
     250,   239,   251,   311,   309,   252,   253,   312,   313,   314,
     255,    45,    46,    47,   288,    15,   257,   272,   258,   259,
     267,   273,   292,   321,   290,   293,    48,   294,   296,   318,
     520,    49,    50,    51,   322,   529,   323,    16,   324,   512,
     524,   513,   329,   514,   515,   516,   517,   530,   518,   535,
     624,   519,   539,   540,   656,   642,   541,   544,   542,   543,
     546,   547,   548,   659,   549,   550,   555,   556,   557,   560,
     561,   562,   563,   564,   570,   571,   572,   663,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     643,   585,   586,   587,   588,   589,   590,   591,   592,   593,
     594,   595,   596,   597,   598,   599,   600,   675,   601,   602,
     603,   604,   605,   606,   607,   608,   609,   610,   611,   612,
     678,   680,   685,   613,   793,   614,   652,   615,   616,   655,
     662,   788,   789,    52,    53,    54,   681,   672,   673,   679,
     551,   687,   690,   682,   691,   692,   694,   695,   704,   707,
     713,   715,   714,   716,   717,   718,   719,   720,   721,   722,
     724,     5,     6,   723,   725,   728,   731,   732,   733,   841,
     734,    55,   735,   739,   742,   331,   332,   736,   737,   738,
     749,   753,   754,   755,   836,   837,   333,   334,   740,   741,
     750,   751,    41,    42,    11,    12,    13,    14,   752,   764,
     335,   336,   766,   770,   765,   767,   334,   773,   775,   784,
     804,   785,   805,   808,   813,   825,   795,   797,   801,   802,
     828,   829,   823,   830,   806,   839,   807,   814,   815,   819,
     820,   824,   832,   833,   280,   840,   842,   845,   843,   846,
     131,   664,   193,   781,   799,   337,   212,   393,   105,   299,
     800,   206,   242,   796,   798,   552,   777,   778,   300,   780,
     652,   232,   779,   308,   301,   302,   838,   686,   233,     0,
     528,     0,     0,   712,   394,   395,   396,   397,   398,     0,
     399,   400,   401,   402,   403,   404,   405,   406,   407,   283,
     408,     0,   409,   410,   411,   412,   413,   569,   414,   415,
     416,     0,     0,   417,     0,   418,   419,   420,   421,   422,
       0,     0,     0,     0,   423,     0,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,     0,     0,     0,     0,   339,     0,     0,     0,     0,
       0,   340,   341,   342,     0,   343,   303,     0,     0,     0,
     304,     0,     0,     0,     0,   344,     0,     0,     0,     0,
     346,   347,   348,   349,   350,   351,     0,     0,     0,     0,
       0,     0,     0,   439,     0,   352,   353
};

static const yytype_int16 yycheck[] =
{
       4,     4,   167,   167,   253,   284,   157,   271,   271,   315,
     295,   271,   271,   271,   294,   295,   294,   294,   294,   238,
     241,    29,   293,   442,     6,     7,     6,     7,   105,   363,
     294,   295,   534,    14,   105,     6,     7,   504,   105,   105,
     162,     0,   655,   679,    50,   137,   242,   674,   242,   662,
     242,   315,   315,    57,    57,   315,   315,   315,    39,    16,
      17,    18,    14,   242,     4,    47,    48,    47,    48,    45,
      22,    23,    53,    54,   687,   242,    47,    48,     4,     4,
       4,     2,    63,     4,    36,    37,   713,     4,     4,    65,
     105,   362,    82,    83,   216,    47,    48,    89,    19,   105,
     793,    53,    54,    55,    56,    57,    58,   441,   200,    61,
      62,   102,   103,   105,   129,   242,    73,    57,   129,   130,
     131,   132,   133,    99,   817,   242,   819,   242,   110,   110,
     110,    57,    57,    57,   201,   117,    57,   242,   119,   110,
      57,    57,    99,   242,   125,   126,   117,   242,   784,     9,
      10,    11,    12,   219,   106,   107,   623,   111,   112,   113,
     114,   115,   442,   243,   442,   442,   442,   119,   670,   242,
     247,   505,   785,   125,   126,   242,   247,   242,   442,   242,
      22,    23,   129,   130,   131,   132,    28,    29,    30,    31,
     227,   228,   229,   230,   231,   232,   129,   130,   617,   132,
     133,   208,   209,   210,   212,   213,   214,   215,   120,   121,
     122,   123,   124,    55,    56,    57,    58,    20,    21,   504,
      91,    92,    93,   242,   504,   207,   207,   207,    25,    26,
     211,    33,    34,    35,    99,   216,   207,    79,   240,   241,
     504,     3,     4,     5,   196,   240,   241,   129,   130,   242,
     202,   203,   204,   242,   206,   207,   117,   118,   242,   101,
     238,   239,    88,   245,   216,   245,   105,   271,   220,   221,
     222,   223,   224,   225,   226,   129,   130,   131,   132,   133,
     134,   242,   243,   105,   236,   237,   208,   209,   105,   240,
     241,   240,   241,   105,   513,   566,   567,   240,   241,   240,
     241,   240,   241,   240,   241,   240,   241,   105,   240,   241,
     105,   315,   240,   241,   217,   218,   105,   240,   241,   124,
     125,   542,   541,   242,   244,   245,    40,   242,   242,   242,
     105,   242,   242,   242,   242,   242,   242,   617,   623,   617,
     617,   617,   242,   623,   242,    19,   243,   243,    22,    23,
      24,   657,   242,   617,    28,    29,    30,    31,   242,   623,
     105,    78,    36,    37,    94,    39,    59,    59,    42,    43,
      44,   243,    32,   105,   105,   243,   243,    51,    52,    53,
      54,    55,    56,    57,    58,    60,    64,    61,    62,   244,
      74,    64,    74,   657,   657,   243,   243,   657,   657,   657,
     104,    75,    76,    77,   105,    79,   246,   242,   246,   246,
     244,   242,   244,    81,   105,   244,    90,   244,   242,   105,
     117,    95,    96,    97,   242,   246,   242,   101,   242,   242,
     117,   242,   247,   242,   242,   242,   242,    38,   242,   105,
      15,   242,   242,   242,   100,   128,   242,   118,   242,   242,
     242,   242,   242,   118,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   105,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     127,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   105,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     105,   200,   219,   242,   679,   242,   519,   242,   242,   242,
     242,   672,   673,   197,   198,   199,   205,   243,   243,   242,
     118,   242,   105,   205,   105,   233,   105,   105,    94,    46,
     242,   193,   109,   193,   193,   193,   140,   140,   140,   194,
     105,    22,    23,   195,   105,   105,   152,   192,   192,   838,
     192,   235,   192,   162,   162,    36,    37,   158,   158,   158,
     176,   176,   105,   105,   823,   824,    47,    48,   165,   165,
     140,   140,    53,    54,    55,    56,    57,    58,   140,   192,
      61,    62,   194,   105,   152,   195,    48,   243,   242,   242,
     105,   242,   105,    13,   105,   104,   246,   246,   245,   245,
     105,   105,   243,   105,   247,   105,   245,   245,   245,   784,
     784,   243,   242,   246,   201,   105,    98,   105,    98,   105,
      57,   532,   130,   653,   706,   106,   159,   108,    19,   110,
     709,   152,   176,   693,   702,   344,   625,   634,   119,   647,
     653,   167,   640,   657,   125,   126,   825,   552,   167,    -1,
     319,    -1,    -1,   568,   135,   136,   137,   138,   139,    -1,
     141,   142,   143,   144,   145,   146,   147,   148,   149,   255,
     151,    -1,   153,   154,   155,   156,   157,   392,   159,   160,
     161,    -1,    -1,   164,    -1,   166,   167,   168,   169,   170,
      -1,    -1,    -1,    -1,   175,    -1,   177,   178,   179,   180,
     181,   182,   183,   184,   185,   186,   187,   188,   189,   190,
     191,    -1,    -1,    -1,    -1,   196,    -1,    -1,    -1,    -1,
      -1,   202,   203,   204,    -1,   206,   207,    -1,    -1,    -1,
     211,    -1,    -1,    -1,    -1,   216,    -1,    -1,    -1,    -1,
     221,   222,   223,   224,   225,   226,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   234,    -1,   236,   237
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,    82,    83,   249,   250,    22,    23,    28,    29,    30,
      31,    55,    56,    57,    58,    79,   101,   271,   272,   300,
     301,   302,   303,   305,   315,   316,   317,   318,   319,   328,
       0,    19,    24,    36,    37,    39,    42,    43,    44,    51,
      52,    53,    54,    61,    62,    75,    76,    77,    90,    95,
      96,    97,   197,   198,   199,   235,   253,   254,   258,   260,
     271,   272,   286,   291,   293,   295,   297,   299,   302,   303,
     305,   309,   310,   311,   312,   313,   319,   320,   321,   322,
     323,   324,   325,   328,   330,   331,   332,   333,   337,   338,
     341,   342,   344,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   102,   103,   301,   276,   242,   243,   259,
     261,   242,   242,   242,   242,   292,   296,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     251,   253,    88,   105,   105,   105,   304,   306,   105,   105,
     105,   105,   208,   209,   210,   329,   242,   242,    99,   273,
      20,    21,   326,   327,    25,    26,   243,   243,    40,   287,
     288,   298,   294,    45,    65,    99,   242,   242,   345,   343,
     105,   105,    78,    91,    92,    93,   339,   340,    94,   314,
     314,   314,    59,    59,   240,   241,   240,   241,    16,    17,
      18,    73,   252,   273,   352,   355,   369,   376,   243,    32,
     307,   308,   308,   105,   105,   274,   327,   243,   243,    60,
     266,   266,   288,   129,   130,   289,   290,   289,   129,   130,
     131,   132,   133,   485,   486,   487,   490,   492,   494,   496,
     497,   484,   490,   492,   496,   497,    64,   347,   348,    64,
     350,   351,   340,   356,   370,   377,   353,   334,   308,   244,
      74,    74,   243,   243,   289,   104,   499,   246,   246,   246,
     347,   350,   238,   239,   434,   434,   434,   244,    89,   105,
     335,   275,   242,   242,    33,    34,    35,   255,   255,   105,
     201,   242,   502,   503,   506,   105,   129,   491,   105,   493,
     105,   495,   244,   244,   244,   354,   242,    14,    63,   110,
     119,   125,   126,   207,   211,   216,   277,   286,   333,   431,
     435,   438,   462,   463,   464,   475,   476,   477,   105,   510,
     511,    81,   242,   242,   242,   219,   501,   503,   505,   247,
     501,    36,    37,    47,    48,    61,    62,   106,   107,   196,
     202,   203,   204,   206,   216,   220,   221,   222,   223,   224,
     225,   226,   236,   237,   262,   264,   271,   280,   283,   333,
     346,   349,   368,   371,   372,   373,   375,   380,   428,   429,
     430,   431,   438,   443,   444,   446,   447,   448,   449,   450,
     451,   452,   454,   455,   456,   457,   460,   462,   463,   464,
     467,   468,   371,   108,   135,   136,   137,   138,   139,   141,
     142,   143,   144,   145,   146,   147,   148,   149,   151,   153,
     154,   155,   156,   157,   159,   160,   161,   164,   166,   167,
     168,   169,   170,   175,   177,   178,   179,   180,   181,   182,
     183,   184,   185,   186,   187,   188,   189,   190,   191,   234,
     277,   378,   379,   380,   381,   382,   383,   384,   385,   386,
     387,   388,   389,   390,   391,   392,   393,   394,   395,   396,
     397,   398,   399,   400,   401,   402,   403,   404,   405,   406,
     407,   408,   409,   410,   411,   412,   413,   414,   415,   416,
     417,   418,   419,   420,   421,   422,   423,   424,   425,   426,
     427,   435,   438,   444,   445,   461,   507,     6,     7,   357,
     358,   360,   362,   364,   366,   367,   372,   435,   438,    50,
     105,   336,   242,   242,   242,   242,   242,   242,   242,   242,
     117,   472,   480,   476,   117,   441,   469,   478,   511,   246,
      38,   269,   270,   256,   257,   105,   504,   263,   265,   242,
     242,   242,   242,   242,   118,   482,   242,   242,   242,   242,
     242,   118,   472,   473,   481,   242,   242,   242,   242,   243,
     242,   242,   242,   242,   242,   371,   441,   442,   470,   478,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   242,   242,   242,
     242,   242,   242,   242,   242,   242,   242,   441,   378,     3,
       4,     5,   367,   441,    15,   432,   433,   347,   111,   112,
     113,   114,   115,   436,   437,   120,   121,   122,   123,   124,
     465,   466,   128,   127,   208,   209,   439,   440,   212,   213,
     214,   215,   272,   278,   279,   242,   100,   474,   483,   118,
     471,   479,   242,   105,   269,     9,    10,    11,    12,   267,
     268,   267,   243,   243,   374,   105,   347,   350,   105,   242,
     200,   205,   205,   240,   241,   219,   473,   242,   240,   241,
     105,   105,   233,   453,   105,   105,   227,   228,   229,   230,
     231,   232,   458,   459,    94,   281,   282,    46,   284,   285,
     371,   371,   471,   242,   109,   193,   193,   193,   193,   140,
     140,   140,   194,   195,   105,   105,   105,   247,   105,   240,
     241,   152,   192,   192,   192,   192,   158,   158,   158,   162,
     165,   165,   162,   240,   241,   240,   241,   240,   241,   176,
     140,   140,   140,   176,   105,   105,   105,   247,   240,   241,
     240,   241,   240,   241,   192,   152,   194,   195,   240,   241,
     105,   508,   378,   243,   361,   242,   367,   433,   436,   466,
     439,   279,   486,   476,   242,   242,   486,   267,   266,   266,
     485,   487,   489,   496,   486,   246,   375,   246,   459,   281,
     284,   245,   245,   485,   105,   105,   247,   245,    13,   363,
     217,   218,   359,   105,   245,   245,   134,   487,   488,   496,
     497,   498,   486,   243,   243,   104,   500,   500,   105,   105,
     105,   509,   242,   246,   500,   500,   255,   255,   506,   105,
     105,   501,    98,    98,   365,   105,   105
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   248,   250,   249,   249,   251,   251,   252,   252,   252,
     252,   252,   253,   253,   254,   254,   254,   254,   254,   254,
     254,   254,   254,   254,   254,   254,   254,   254,   254,   254,
     254,   254,   254,   254,   254,   254,   254,   254,   254,   254,
     254,   255,   256,   255,   257,   255,   259,   258,   261,   260,
     263,   262,   265,   264,   266,   267,   267,   268,   268,   268,
     268,   269,   269,   270,   271,   271,   271,   271,   272,   274,
     275,   273,   276,   276,   277,   278,   278,   278,   278,   278,
     279,   279,   280,   281,   282,   282,   283,   284,   285,   285,
     286,   287,   288,   288,   289,   289,   290,   290,   291,   292,
     294,   293,   295,   296,   298,   297,   299,   299,   299,   300,
     300,   300,   300,   300,   300,   300,   301,   301,   302,   302,
     304,   303,   306,   305,   307,   308,   308,   309,   309,   310,
     310,   310,   311,   312,   313,   314,   315,   315,   315,   316,
     317,   318,   319,   320,   320,   321,   322,   323,   323,   324,
     324,   325,   326,   326,   327,   327,   328,   329,   329,   329,
     330,   330,   331,   332,   334,   333,   335,   335,   336,   336,
     337,   337,   338,   339,   339,   339,   340,   340,   341,   343,
     342,   345,   344,   346,   347,   347,   348,   349,   350,   350,
     351,   353,   354,   352,   356,   355,   357,   357,   357,   358,
     358,   358,   359,   359,   359,   361,   360,   362,   363,   364,
     365,   365,   366,   366,   366,   366,   367,   367,   368,   368,
     368,   368,   368,   370,   369,   371,   371,   372,   372,   374,
     373,   375,   377,   376,   378,   378,   379,   379,   379,   379,
     379,   379,   379,   379,   379,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   380,   380,   380,   380,   380,
     380,   380,   380,   380,   380,   381,   381,   381,   381,   381,
     381,   381,   381,   381,   381,   381,   381,   381,   381,   381,
     381,   381,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     382,   382,   382,   382,   382,   382,   382,   382,   382,   382,
     383,   383,   384,   384,   385,   386,   387,   388,   389,   390,
     391,   392,   393,   394,   395,   396,   397,   398,   399,   400,
     401,   402,   403,   404,   404,   405,   405,   406,   406,   407,
     407,   408,   408,   409,   409,   410,   410,   411,   411,   412,
     413,   414,   415,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   426,   427,   428,   429,   430,   430,   431,
     432,   433,   433,   434,   434,   435,   436,   436,   437,   437,
     437,   437,   437,   438,   439,   439,   440,   440,   441,   442,
     443,   443,   443,   444,   444,   444,   445,   446,   446,   447,
     447,   448,   449,   450,   450,   450,   450,   451,   453,   452,
     454,   455,   456,   457,   458,   458,   458,   458,   458,   458,
     459,   459,   460,   461,   462,   463,   464,   465,   465,   465,
     465,   465,   466,   466,   467,   468,   469,   470,   471,   472,
     473,   474,   475,   475,   475,   475,   475,   475,   475,   475,
     475,   475,   475,   476,   476,   477,   478,   479,   480,   481,
     482,   483,   484,   484,   484,   484,   485,   485,   485,   486,
     487,   487,   487,   487,   487,   487,   488,   488,   488,   488,
     489,   489,   490,   491,   491,   492,   493,   494,   495,   496,
     497,   498,   499,   499,   499,   499,   500,   500,   501,   501,
     502,   503,   504,   505,   506,   507,   508,   509,   510,   511,
     511
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     4,     3,     0,     2,     1,     1,     1,
       1,     1,     0,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     0,     4,     0,     4,     0,     6,     0,     6,
       0,     6,     0,     6,     1,     1,     2,     1,     1,     1,
       1,     1,     2,     1,     1,     1,     1,     1,     1,     0,
       0,     9,     0,     2,     3,     1,     1,     1,     1,     1,
       1,     2,     3,     1,     1,     2,     3,     1,     1,     2,
       3,     1,     1,     2,     1,     2,     1,     1,     4,     0,
       0,     4,     4,     0,     0,     4,     3,     3,     3,     1,
       1,     1,     1,     1,     1,     1,     0,     2,     4,     4,
       0,     4,     0,     4,     1,     1,     2,     3,     3,     1,
       1,     1,     3,     3,     3,     1,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     2,     3,     1,     1,     1,
       1,     1,     9,     7,     0,     7,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     2,     3,     0,
       4,     0,     4,     3,     1,     2,     1,     3,     1,     2,
       1,     0,     0,     8,     0,     8,     1,     1,     1,     0,
       1,     1,     0,     1,     1,     0,     8,     4,     1,     7,
       0,     2,     1,     1,     1,     1,     0,     2,     1,     1,
       1,     1,     1,     0,     8,     0,     2,     1,     1,     0,
       4,     3,     0,     8,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     4,     3,     4,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     1,     2,     1,     1,     3,     1,     2,     1,     1,
       1,     1,     1,     3,     1,     2,     1,     1,     2,     2,
       3,     2,     2,     1,     1,     1,     1,     1,     1,     3,
       3,     3,     5,     1,     1,     1,     1,     3,     0,     4,
       3,     5,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     1,     1,     2,     4,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     1,     0,     2,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       3,     1,     3,     1,     3,     1,     2,     2,     1,     1,
       2,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     0,     3,     3,     2,     0,     3,     1,     1,
       3,     1,     1,     1,     1,     5,     1,     1,     1,     1,
       2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* $@1: %empty  */
#line 653 "config_parse.y"
                           {
#if !SOCKS_CLIENT
      extension = &sockscf.extension;
#endif /* !SOCKS_CLIENT*/
   }
#line 3232 "config_parse.c"
    break;

  case 5: /* serverobjects: %empty  */
#line 661 "config_parse.y"
               { (yyval.string) = NULL; }
#line 3238 "config_parse.c"
    break;

  case 12: /* serveroptions: %empty  */
#line 672 "config_parse.y"
                { (yyval.string) = NULL; }
#line 3244 "config_parse.c"
    break;

  case 40: /* serveroption: socketoption  */
#line 701 "config_parse.y"
                            {
      if (!addedsocketoption(&sockscf.socketoptionc,
                             &sockscf.socketoptionv,
                             &socketopt))
         yywarn("could not add socket option");
   }
#line 3255 "config_parse.c"
    break;

  case 42: /* $@2: %empty  */
#line 710 "config_parse.y"
                                     {
#if !SOCKS_CLIENT
                                tcpoptions = &logspecial->protocol.tcp.disabled;
#endif /* !SOCKS_CLIENT */
          }
#line 3265 "config_parse.c"
    break;

  case 44: /* $@3: %empty  */
#line 715 "config_parse.y"
                                    {
#if !SOCKS_CLIENT
                                tcpoptions = &logspecial->protocol.tcp.enabled;
#endif /* !SOCKS_CLIENT */
          }
#line 3275 "config_parse.c"
    break;

  case 46: /* $@4: %empty  */
#line 723 "config_parse.y"
                                      {
#if !SOCKS_CLIENT

      logspecial = &sockscf.internal.log;

#endif /* !SOCKS_CLIENT */

   }
#line 3288 "config_parse.c"
    break;

  case 48: /* $@5: %empty  */
#line 733 "config_parse.y"
                                      {
#if !SOCKS_CLIENT

      logspecial = &sockscf.external.log;

#endif /* !SOCKS_CLIENT */

   }
#line 3301 "config_parse.c"
    break;

  case 50: /* $@6: %empty  */
#line 743 "config_parse.y"
                                        {
#if !SOCKS_CLIENT

      logspecial = &rule.internal.log;

#endif /* !SOCKS_CLIENT */

   }
#line 3314 "config_parse.c"
    break;

  case 52: /* $@7: %empty  */
#line 753 "config_parse.y"
                                        {
#if !SOCKS_CLIENT

      logspecial = &rule.external.log;

#endif /* !SOCKS_CLIENT */

   }
#line 3327 "config_parse.c"
    break;

  case 54: /* loglevel: LOGLEVEL  */
#line 764 "config_parse.y"
                   {
#if !SOCKS_CLIENT
   SASSERTX((yyvsp[0].number) >= 0);
   SASSERTX((yyvsp[0].number) < MAXLOGLEVELS);

   cloglevel = (yyvsp[0].number);
#endif /* !SOCKS_CLIENT */
   }
#line 3340 "config_parse.c"
    break;

  case 57: /* tcpoption: ECN  */
#line 778 "config_parse.y"
               {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, ecn);
#endif /* !SOCKS_CLIENT */
   }
#line 3350 "config_parse.c"
    break;

  case 58: /* tcpoption: SACK  */
#line 785 "config_parse.y"
                {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, sack);
#endif /* !SOCKS_CLIENT */
   }
#line 3360 "config_parse.c"
    break;

  case 59: /* tcpoption: TIMESTAMPS  */
#line 792 "config_parse.y"
                      {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, timestamps);
#endif /* !SOCKS_CLIENT */
   }
#line 3370 "config_parse.c"
    break;

  case 60: /* tcpoption: WSCALE  */
#line 799 "config_parse.y"
                  {
#if !SOCKS_CLIENT
   SET_TCPOPTION(tcpoptions, cloglevel, wscale);
#endif /* !SOCKS_CLIENT */
   }
#line 3380 "config_parse.c"
    break;

  case 63: /* errorobject: ERRORVALUE  */
#line 812 "config_parse.y"
                        {
#if !SOCKS_CLIENT

   if ((yyvsp[0].error).valuev == NULL)
      yywarnx("unknown error symbol specified");
   else {
      size_t *ec, ec_max, i;
      int *ev;

      switch ((yyvsp[0].error).valuetype) {
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
            SERRX((yyvsp[0].error).valuetype);
      }

      for (i = 0; (yyvsp[0].error).valuev[i] != 0; ++i) {
         /*
          * If the value is already set in the array, e.g. because some
          * errno-symbols have the same values, ignore this value.
          */
         size_t j;

         for (j = 0; j < *ec; ++j) {
            if (ev[j] == (yyvsp[0].error).valuev[i])
               break;
         }

         if (j < *ec)
            continue; /* error-value already set in array. */

         SASSERTX(*ec < ec_max);

         ev[(*ec)] = (yyvsp[0].error).valuev[i];
         ++(*ec);
      }
   }
#endif /* !SOCKS_CLIENT */
   }
#line 3434 "config_parse.c"
    break;

  case 68: /* deprecated: DEPRECATED  */
#line 870 "config_parse.y"
                         {
      yyerrorx("given keyword \"%s\" is deprecated.  New keyword is %s.  "
               "Please see %s's manual for more information",
               (yyvsp[0].deprecated).oldname, (yyvsp[0].deprecated).newname, PRODUCT);
   }
#line 3444 "config_parse.c"
    break;

  case 69: /* $@8: %empty  */
#line 877 "config_parse.y"
               { objecttype = object_route; }
#line 3450 "config_parse.c"
    break;

  case 70: /* $@9: %empty  */
#line 878 "config_parse.y"
         { routeinit(&route); }
#line 3456 "config_parse.c"
    break;

  case 71: /* route: ROUTE $@8 '{' $@9 routeoptions fromto gateway routeoptions '}'  */
#line 878 "config_parse.y"
                                                                             {
      route.src       = src;
      route.dst       = dst;
      route.gw.addr   = gw;

      route.rdr_from  = rdr_from;

      socks_addroute(&route, 1);
   }
#line 3470 "config_parse.c"
    break;

  case 72: /* routes: %empty  */
#line 889 "config_parse.y"
        { (yyval.string) = NULL; }
#line 3476 "config_parse.c"
    break;

  case 75: /* proxyprotocolname: PROXYPROTOCOL_SOCKS_V4  */
#line 895 "config_parse.y"
                                            {
         state->proxyprotocol.socks_v4 = 1;
   }
#line 3484 "config_parse.c"
    break;

  case 76: /* proxyprotocolname: PROXYPROTOCOL_SOCKS_V5  */
#line 898 "config_parse.y"
                              {
         state->proxyprotocol.socks_v5 = 1;
   }
#line 3492 "config_parse.c"
    break;

  case 77: /* proxyprotocolname: PROXYPROTOCOL_HTTP  */
#line 901 "config_parse.y"
                         {
         state->proxyprotocol.http     = 1;
   }
#line 3500 "config_parse.c"
    break;

  case 78: /* proxyprotocolname: PROXYPROTOCOL_UPNP  */
#line 904 "config_parse.y"
                         {
         state->proxyprotocol.upnp     = 1;
   }
#line 3508 "config_parse.c"
    break;

  case 83: /* username: USERNAME  */
#line 917 "config_parse.y"
                     {
#if !SOCKS_CLIENT
      if (addlinkedname(&rule.user, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#endif /* !SOCKS_CLIENT */
   }
#line 3519 "config_parse.c"
    break;

  case 87: /* groupname: GROUPNAME  */
#line 932 "config_parse.y"
                       {
#if !SOCKS_CLIENT
      if (addlinkedname(&rule.group, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#endif /* !SOCKS_CLIENT */
   }
#line 3530 "config_parse.c"
    break;

  case 91: /* extensionname: BIND  */
#line 947 "config_parse.y"
                      {
         yywarnx("we are currently considering deprecating the Dante-specific "
                 "SOCKS bind extension.  If you are using it, please let us "
                 "know on the public dante-misc@inet.no mailinglist");

         extension->bind = 1;
   }
#line 3542 "config_parse.c"
    break;

  case 96: /* ifprotocol: IPV4  */
#line 965 "config_parse.y"
                 {
#if !SOCKS_CLIENT
      ifproto->ipv4  = 1;
   }
#line 3551 "config_parse.c"
    break;

  case 97: /* ifprotocol: IPV6  */
#line 969 "config_parse.y"
           {
      ifproto->ipv6  = 1;
#endif /* SOCKS_SERVER */
   }
#line 3560 "config_parse.c"
    break;

  case 98: /* internal: INTERNAL internalinit ':' address  */
#line 975 "config_parse.y"
                                              {
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
#line 3588 "config_parse.c"
    break;

  case 99: /* internalinit: %empty  */
#line 1000 "config_parse.y"
              {
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
#line 3615 "config_parse.c"
    break;

  case 100: /* $@10: %empty  */
#line 1024 "config_parse.y"
                                         {
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
   }
#line 3641 "config_parse.c"
    break;

  case 102: /* external: EXTERNAL externalinit ':' externaladdress  */
#line 1049 "config_parse.y"
                                                      {
#if !SOCKS_CLIENT
      addexternal(ruleaddr);
#endif /* !SOCKS_CLIENT */
   }
#line 3651 "config_parse.c"
    break;

  case 103: /* externalinit: %empty  */
#line 1056 "config_parse.y"
              {
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
#line 3676 "config_parse.c"
    break;

  case 104: /* $@11: %empty  */
#line 1078 "config_parse.y"
                                         {
#if !SOCKS_CLIENT
      if (sockscf.external.addrc > 0) {
         log_interfaceprotocol_set_too_late(EXTERNALIF);
         sockdexit(EXIT_FAILURE);
      }

      ifproto = &sockscf.external.protocol;
#endif /* !SOCKS_CLIENT */
   }
#line 3691 "config_parse.c"
    break;

  case 106: /* external_rotation: EXTERNAL_ROTATION ':' NONE  */
#line 1091 "config_parse.y"
                                                {
#if !SOCKS_CLIENT
      sockscf.external.rotation = ROTATION_NONE;
   }
#line 3700 "config_parse.c"
    break;

  case 107: /* external_rotation: EXTERNAL_ROTATION ':' SAMESAME  */
#line 1095 "config_parse.y"
                                      {
      sockscf.external.rotation = ROTATION_SAMESAME;
   }
#line 3708 "config_parse.c"
    break;

  case 108: /* external_rotation: EXTERNAL_ROTATION ':' ROUTE  */
#line 1098 "config_parse.y"
                                   {
      sockscf.external.rotation = ROTATION_ROUTE;
#endif /* SOCKS_SERVER */
   }
#line 3717 "config_parse.c"
    break;

  case 116: /* clientoptions: %empty  */
#line 1113 "config_parse.y"
               { (yyval.string) = NULL; }
#line 3723 "config_parse.c"
    break;

  case 118: /* global_routeoption: GLOBALROUTEOPTION MAXFAIL ':' NUMBER  */
#line 1117 "config_parse.y"
                                                         {
      if ((yyvsp[0].number) < 0)
         yyerrorx("max route fails can not be negative (%ld)  Use \"0\" to "
                  "indicate routes should never be marked as bad",
                  (long)(yyvsp[0].number));

      sockscf.routeoptions.maxfail = (yyvsp[0].number);
   }
#line 3736 "config_parse.c"
    break;

  case 119: /* global_routeoption: GLOBALROUTEOPTION BADROUTE_EXPIRE ':' NUMBER  */
#line 1125 "config_parse.y"
                                                   {
      if ((yyvsp[0].number) < 0)
         yyerrorx("route failure expiry time can not be negative (%ld).  "
                  "Use \"0\" to indicate bad route marking should never expire",
                  (long)(yyvsp[0].number));

      sockscf.routeoptions.badexpire = (yyvsp[0].number);
   }
#line 3749 "config_parse.c"
    break;

  case 120: /* $@12: %empty  */
#line 1135 "config_parse.y"
                         { add_to_errlog = 1; }
#line 3755 "config_parse.c"
    break;

  case 122: /* $@13: %empty  */
#line 1138 "config_parse.y"
                         { add_to_errlog = 0; }
#line 3761 "config_parse.c"
    break;

  case 124: /* logoutputdevice: LOGFILE  */
#line 1141 "config_parse.y"
                         {
   int p;

   if ((add_to_errlog && failed_to_add_errlog)
   ||      (!add_to_errlog && failed_to_add_log)) {
      yywarnx("not adding logfile \"%s\"", (yyvsp[0].string));

      slog(LOG_ALERT,
           "%s: not trying to add logfile \"%s\" due to having already failed "
           "adding logfiles during this SIGHUP.  Only if all logfiles "
           "specified in the config can be added will we switch to using "
           "the new logfiles.  Until then, we will continue using only the "
           "old logfiles",
           function, (yyvsp[0].string));
   }
   else {
      p = socks_addlogfile(add_to_errlog ? &sockscf.errlog : &sockscf.log, (yyvsp[0].string));

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
                 function, (yyvsp[0].string), add_to_errlog ? "errlog" : "logoutput");
         }
      }

      if (p == -1)
         slog(LOG_ALERT, "%s: could not (re)open logfile \"%s\": %s%s  %s",
              function,
              (yyvsp[0].string),
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
         yywarn("failed to add logfile %s", (yyvsp[0].string));
#endif /* SOCKS_CLIENT */
   }
}
#line 3822 "config_parse.c"
    break;

  case 127: /* childstate: PROC_MAXREQUESTS ':' NUMBER  */
#line 1202 "config_parse.y"
                                        {
#if !SOCKS_CLIENT

      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, sockscf.child.maxrequests, 0);

#endif /* !SOCKS_CLIENT */
   }
#line 3834 "config_parse.c"
    break;

  case 128: /* childstate: PROC_MAXLIFETIME ':' NUMBER  */
#line 1209 "config_parse.y"
                                 {
#if !SOCKS_CLIENT

      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, sockscf.child.maxlifetime, 0);

#endif /* !SOCKS_CLIENT */
   }
#line 3846 "config_parse.c"
    break;

  case 132: /* user_privileged: USER_PRIVILEGED ':' userid  */
#line 1223 "config_parse.y"
                                              {
#if !SOCKS_CLIENT
#if HAVE_PRIVILEGES
      yyerrorx("userid-settings not used on platforms with privileges");
#else
      sockscf.uid.privileged_uid   = (yyvsp[0].uid).uid;
      sockscf.uid.privileged_gid   = (yyvsp[0].uid).gid;
      sockscf.uid.privileged_isset = 1;
#endif /* !HAVE_PRIVILEGES */
#endif /* !SOCKS_CLIENT */
   }
#line 3862 "config_parse.c"
    break;

  case 133: /* user_unprivileged: USER_UNPRIVILEGED ':' userid  */
#line 1236 "config_parse.y"
                                                  {
#if !SOCKS_CLIENT
#if HAVE_PRIVILEGES
      yyerrorx("userid-settings not used on platforms with privileges");
#else
      sockscf.uid.unprivileged_uid   = (yyvsp[0].uid).uid;
      sockscf.uid.unprivileged_gid   = (yyvsp[0].uid).gid;
      sockscf.uid.unprivileged_isset = 1;
#endif /* !HAVE_PRIVILEGES */
#endif /* !SOCKS_CLIENT */
   }
#line 3878 "config_parse.c"
    break;

  case 134: /* user_libwrap: USER_LIBWRAP ':' userid  */
#line 1249 "config_parse.y"
                                        {
#if HAVE_LIBWRAP && (!SOCKS_CLIENT)

#if HAVE_PRIVILEGES
      yyerrorx("userid-settings not used on platforms with privileges");

#else
      sockscf.uid.libwrap_uid   = (yyvsp[0].uid).uid;
      sockscf.uid.libwrap_gid   = (yyvsp[0].uid).gid;
      sockscf.uid.libwrap_isset = 1;
#endif /* !HAVE_PRIVILEGES */

#else  /* !HAVE_LIBWRAP && (!SOCKS_CLIENT) */
      yyerrorx_nolib("libwrap");
#endif /* !HAVE_LIBWRAP (!SOCKS_CLIENT)*/
   }
#line 3899 "config_parse.c"
    break;

  case 135: /* userid: USERNAME  */
#line 1268 "config_parse.y"
                   {
      struct passwd *pw;

      if ((pw = getpwnam((yyvsp[0].string))) == NULL)
         yyerror("getpwnam(3) says no such user \"%s\"", (yyvsp[0].string));

      (yyval.uid).uid = pw->pw_uid;

      if ((pw = getpwuid((yyval.uid).uid)) == NULL)
         yyerror("getpwuid(3) says no such uid %lu (from user \"%s\")",
                 (unsigned long)(yyval.uid).uid, (yyvsp[0].string));

      (yyval.uid).gid = pw->pw_gid;
   }
#line 3918 "config_parse.c"
    break;

  case 136: /* iotimeout: IOTIMEOUT ':' NUMBER  */
#line 1284 "config_parse.y"
                                  {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, timeout->tcpio, 1);
      timeout->udpio = timeout->tcpio;
   }
#line 3928 "config_parse.c"
    break;

  case 137: /* iotimeout: IOTIMEOUT_TCP ':' NUMBER  */
#line 1289 "config_parse.y"
                               {
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, timeout->tcpio, 1);
   }
#line 3936 "config_parse.c"
    break;

  case 138: /* iotimeout: IOTIMEOUT_UDP ':' NUMBER  */
#line 1292 "config_parse.y"
                               {
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, timeout->udpio, 1);
#endif /* !SOCKS_CLIENT */
   }
#line 3945 "config_parse.c"
    break;

  case 139: /* negotiatetimeout: NEGOTIATETIMEOUT ':' NUMBER  */
#line 1298 "config_parse.y"
                                                {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, timeout->negotiate, 1);
#endif /* !SOCKS_CLIENT */
   }
#line 3955 "config_parse.c"
    break;

  case 140: /* connecttimeout: CONNECTTIMEOUT ':' NUMBER  */
#line 1305 "config_parse.y"
                                            {
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, timeout->connect, 1);
   }
#line 3963 "config_parse.c"
    break;

  case 141: /* tcp_fin_timeout: TCP_FIN_WAIT ':' NUMBER  */
#line 1310 "config_parse.y"
                                           {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, timeout->tcp_fin_wait, 1);
#endif /* !SOCKS_CLIENT */
   }
#line 3973 "config_parse.c"
    break;

  case 142: /* debugging: DEBUGGING ':' NUMBER  */
#line 1318 "config_parse.y"
                                {
#if SOCKS_CLIENT

       sockscf.option.debug = (int)(yyvsp[0].number);

#else /* !SOCKS_CLIENT */

      if (sockscf.initial.cmdline.debug_isset
      &&  sockscf.initial.cmdline.debug != (yyvsp[0].number))
         LOG_CMDLINE_OVERRIDE("debug",
                              sockscf.initial.cmdline.debug,
                              (int)(yyvsp[0].number),
                              "%d");
      else
         sockscf.option.debug = (int)(yyvsp[0].number);

#endif /* !SOCKS_CLIENT */
   }
#line 3996 "config_parse.c"
    break;

  case 145: /* libwrap_allowfile: LIBWRAP_ALLOW ':' LIBWRAP_FILE  */
#line 1342 "config_parse.y"
                                                  {
#if !SOCKS_CLIENT
#if HAVE_LIBWRAP
      if ((hosts_allow_table  = strdup((yyvsp[0].string))) == NULL)
         yyerror(NOMEM);

      slog(LOG_DEBUG, "%s: libwrap.allow: %s", function, hosts_allow_table);
#else
      yyerrorx_nolib("libwrap");
#endif /* HAVE_LIBWRAP */
#endif /* !SOCKS_CLIENT */
   }
#line 4013 "config_parse.c"
    break;

  case 146: /* libwrap_denyfile: LIBWRAP_DENY ':' LIBWRAP_FILE  */
#line 1356 "config_parse.y"
                                                {
#if !SOCKS_CLIENT
#if HAVE_LIBWRAP
      if ((hosts_deny_table  = strdup((yyvsp[0].string))) == NULL)
         yyerror(NOMEM);

      slog(LOG_DEBUG, "%s: libwrap.deny: %s", function, hosts_deny_table);
#else
      yyerrorx_nolib("libwrap");
#endif /* HAVE_LIBWRAP */
#endif /* !SOCKS_CLIENT */
   }
#line 4030 "config_parse.c"
    break;

  case 147: /* libwrap_hosts_access: LIBWRAP_HOSTS_ACCESS ':' YES  */
#line 1370 "config_parse.y"
                                                   {
#if !SOCKS_CLIENT
#if HAVE_LIBWRAP
      sockscf.option.hosts_access = 1;
#else
      yyerrorx("libwrap.hosts_access requires libwrap library");
#endif /* HAVE_LIBWRAP */
   }
#line 4043 "config_parse.c"
    break;

  case 148: /* libwrap_hosts_access: LIBWRAP_HOSTS_ACCESS ':' NO  */
#line 1378 "config_parse.y"
                                 {
#if HAVE_LIBWRAP
      sockscf.option.hosts_access = 0;
#else
      yyerrorx_nolib("libwrap");
#endif /* HAVE_LIBWRAP */
#endif /* !SOCKS_CLIENT */
   }
#line 4056 "config_parse.c"
    break;

  case 149: /* udpconnectdst: UDPCONNECTDST ':' YES  */
#line 1388 "config_parse.y"
                                     {
#if !SOCKS_CLIENT
      sockscf.udpconnectdst = 1;
   }
#line 4065 "config_parse.c"
    break;

  case 150: /* udpconnectdst: UDPCONNECTDST ':' NO  */
#line 1392 "config_parse.y"
                          {
      sockscf.udpconnectdst = 0;
#endif /* !SOCKS_CLIENT */
   }
#line 4074 "config_parse.c"
    break;

  case 152: /* compatibilityname: SAMEPORT  */
#line 1402 "config_parse.y"
                            {
#if !SOCKS_CLIENT
      sockscf.compat.sameport = 1;
   }
#line 4083 "config_parse.c"
    break;

  case 153: /* compatibilityname: DRAFT_5_05  */
#line 1406 "config_parse.y"
                 {
      sockscf.compat.draft_5_05 = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 4092 "config_parse.c"
    break;

  case 157: /* resolveprotocolname: PROTOCOL_FAKE  */
#line 1419 "config_parse.y"
                                     {
         sockscf.resolveprotocol = RESOLVEPROTOCOL_FAKE;
   }
#line 4100 "config_parse.c"
    break;

  case 158: /* resolveprotocolname: PROTOCOL_TCP  */
#line 1422 "config_parse.y"
                   {
#if HAVE_NO_RESOLVESTUFF
         yyerrorx("resolveprotocol keyword not supported on this system");
#else
         sockscf.resolveprotocol = RESOLVEPROTOCOL_TCP;
#endif /* !HAVE_NO_RESOLVESTUFF */
   }
#line 4112 "config_parse.c"
    break;

  case 159: /* resolveprotocolname: PROTOCOL_UDP  */
#line 1429 "config_parse.y"
                    {
         sockscf.resolveprotocol = RESOLVEPROTOCOL_UDP;
   }
#line 4120 "config_parse.c"
    break;

  case 162: /* cpuschedule: CPU '.' SCHEDULE '.' PROCESSTYPE ':' SCHEDULEPOLICY '/' NUMBER  */
#line 1438 "config_parse.y"
                                                                            {
#if !SOCKS_CLIENT
#if !HAVE_SCHED_SETSCHEDULER
      yyerrorx("cpu scheduling policy is not supported on this system");
#else /* HAVE_SCHED_SETSCHEDULER */
      cpusetting_t *cpusetting;

      switch ((yyvsp[-4].number)) {
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
            SERRX((yyvsp[-4].number));
      }

      bzero(&cpusetting->param, sizeof(cpusetting->param));

      cpusetting->scheduling_isset     = 1;
      cpusetting->policy               = (yyvsp[-2].number);
      cpusetting->param.sched_priority = (int)(yyvsp[0].number);
#endif /* HAVE_SCHED_SETSCHEDULER */
#endif /* !SOCKS_CLIENT */
   }
#line 4165 "config_parse.c"
    break;

  case 163: /* cpuaffinity: CPU '.' MASK '.' PROCESSTYPE ':' numbers  */
#line 1480 "config_parse.y"
                                                      {
#if !SOCKS_CLIENT
#if !HAVE_SCHED_SETAFFINITY
      yyerrorx("cpu scheduling affinity is not supported on this system");
#else /* HAVE_SCHED_SETAFFINITY */
      cpusetting_t *cpusetting;

      switch ((yyvsp[-2].number)) {
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
            SERRX((yyvsp[-2].number));
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
#line 4229 "config_parse.c"
    break;

  case 164: /* $@14: %empty  */
#line 1541 "config_parse.y"
                                            {
#if !SOCKS_CLIENT
      socketopt.level = (yyvsp[-1].number);
#endif /* !SOCKS_CLIENT */
   }
#line 4239 "config_parse.c"
    break;

  case 166: /* socketoptionname: NUMBER  */
#line 1548 "config_parse.y"
                         {
#if !SOCKS_CLIENT
   socketopt.optname = (yyvsp[0].number);
   socketopt.info    = optval2sockopt(socketopt.level, socketopt.optname);

   if (socketopt.info == NULL)
      slog(LOG_DEBUG,
           "%s: unknown/unsupported socket option: level %d, value %d",
           function, socketopt.level, socketopt.optname);
   else
      socketoptioncheck(&socketopt);
   }
#line 4256 "config_parse.c"
    break;

  case 167: /* socketoptionname: SOCKETOPTION_OPTID  */
#line 1560 "config_parse.y"
                        {
      socketopt.info           = optid2sockopt((size_t)(yyvsp[0].number));
      SASSERTX(socketopt.info != NULL);

      socketopt.optname        = socketopt.info->value;

      socketoptioncheck(&socketopt);
#endif /* !SOCKS_CLIENT */
   }
#line 4270 "config_parse.c"
    break;

  case 168: /* socketoptionvalue: NUMBER  */
#line 1571 "config_parse.y"
                          {
      socketopt.optval.int_val = (int)(yyvsp[0].number);
      socketopt.opttype        = int_val;
   }
#line 4279 "config_parse.c"
    break;

  case 169: /* socketoptionvalue: SOCKETOPTION_SYMBOLICVALUE  */
#line 1575 "config_parse.y"
                                {
      const sockoptvalsym_t *p;

      if (socketopt.info == NULL)
         yyerrorx("the given socket option is unknown, so can not lookup "
                  "symbolic option value");

      if ((p = optval2valsym(socketopt.info->optid, (yyvsp[0].string))) == NULL)
         yyerrorx("symbolic value \"%s\" is unknown for socket option %s",
                  (yyvsp[0].string), sockopt2string(&socketopt, NULL, 0));

      socketopt.optval  = p->symval;
      socketopt.opttype = socketopt.info->opttype;
   }
#line 4298 "config_parse.c"
    break;

  case 170: /* socketside: INTERNALSOCKET  */
#line 1592 "config_parse.y"
                           { bzero(&socketopt, sizeof(socketopt));
                             socketopt.isinternalside = 1;
   }
#line 4306 "config_parse.c"
    break;

  case 171: /* socketside: EXTERNALSOCKET  */
#line 1595 "config_parse.y"
                           { bzero(&socketopt, sizeof(socketopt));
                             socketopt.isinternalside = 0;
   }
#line 4314 "config_parse.c"
    break;

  case 173: /* srchostoption: NODNSMISMATCH  */
#line 1604 "config_parse.y"
                               {
#if !SOCKS_CLIENT
         sockscf.srchost.nodnsmismatch = 1;
   }
#line 4323 "config_parse.c"
    break;

  case 174: /* srchostoption: NODNSUNKNOWN  */
#line 1608 "config_parse.y"
                   {
         sockscf.srchost.nodnsunknown = 1;
   }
#line 4331 "config_parse.c"
    break;

  case 175: /* srchostoption: CHECKREPLYAUTH  */
#line 1611 "config_parse.y"
                     {
         sockscf.srchost.checkreplyauth = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 4340 "config_parse.c"
    break;

  case 178: /* realm: REALM ':' REALNAME  */
#line 1621 "config_parse.y"
                          {
#if COVENANT
   STRCPY_CHECKLEN(sockscf.realmname,
                   (yyvsp[0].string),
                   sizeof(sockscf.realmname) - 1,
                   yyerrorx);
#else /* !COVENANT */
   yyerrorx("unknown keyword \"%s\"", (yyvsp[-2].string));
#endif /* !COVENANT */
}
#line 4355 "config_parse.c"
    break;

  case 179: /* $@15: %empty  */
#line 1633 "config_parse.y"
                                        {
#if !SOCKS_CLIENT

   cmethodv  = sockscf.cmethodv;
   cmethodc  = &sockscf.cmethodc;
  *cmethodc  = 0; /* reset. */

#endif /* !SOCKS_CLIENT */
   }
#line 4369 "config_parse.c"
    break;

  case 181: /* $@16: %empty  */
#line 1644 "config_parse.y"
                                      {
#if HAVE_SOCKS_RULES

      smethodv  = sockscf.smethodv;
      smethodc  = &sockscf.smethodc;
     *smethodc  = 0; /* reset. */

#else
      yyerrorx("\"socksmethod\" is not used in %s.  Only \"clientmethod\" "
               "is used",
               PRODUCT);
#endif /* !HAVE_SOCKS_RULES */
   }
#line 4387 "config_parse.c"
    break;

  case 186: /* socksmethodname: METHODNAME  */
#line 1666 "config_parse.y"
                            {
      if (methodisvalid((yyvsp[0].method), object_srule))
         ADDMETHOD((yyvsp[0].method), *smethodc, smethodv);
      else
         yyerrorx("method %s (%d) is not a valid method for socksmethods",
                  method2string((yyvsp[0].method)), (yyvsp[0].method));
   }
#line 4399 "config_parse.c"
    break;

  case 190: /* clientmethodname: METHODNAME  */
#line 1683 "config_parse.y"
                               {
      if (methodisvalid((yyvsp[0].method), object_crule))
         ADDMETHOD((yyvsp[0].method), *cmethodc, cmethodv);
      else
         yyerrorx("method %s (%d) is not a valid method for clientmethods",
                  method2string((yyvsp[0].method)), (yyvsp[0].method));
   }
#line 4411 "config_parse.c"
    break;

  case 191: /* $@17: %empty  */
#line 1691 "config_parse.y"
                 { objecttype = object_monitor; }
#line 4417 "config_parse.c"
    break;

  case 192: /* $@18: %empty  */
#line 1691 "config_parse.y"
                                                      {
#if !SOCKS_CLIENT
                        monitorinit(&monitor);
#endif /* !SOCKS_CLIENT */
}
#line 4427 "config_parse.c"
    break;

  case 193: /* monitor: MONITOR $@17 '{' $@18 monitoroptions fromto monitoroptions '}'  */
#line 1696 "config_parse.y"
{
#if !SOCKS_CLIENT
   pre_addmonitor(&monitor);

   addmonitor(&monitor);
#endif /* !SOCKS_CLIENT */
}
#line 4439 "config_parse.c"
    break;

  case 194: /* $@19: %empty  */
#line 1708 "config_parse.y"
                  { objecttype = object_crule; }
#line 4445 "config_parse.c"
    break;

  case 195: /* crule: CLIENTRULE $@19 verdict '{' cruleoptions fromto cruleoptions '}'  */
#line 1709 "config_parse.y"
                                                       {
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
#line 4476 "config_parse.c"
    break;

  case 199: /* monitorside: %empty  */
#line 1742 "config_parse.y"
             {
#if !SOCKS_CLIENT
         monitorif = NULL;
   }
#line 4485 "config_parse.c"
    break;

  case 200: /* monitorside: ALARMIF_INTERNAL  */
#line 1746 "config_parse.y"
                       {
         monitorif = &monitor.mstats->object.monitor.internal;
   }
#line 4493 "config_parse.c"
    break;

  case 201: /* monitorside: ALARMIF_EXTERNAL  */
#line 1749 "config_parse.y"
                      {
         monitorif = &monitor.mstats->object.monitor.external;
#endif /* !SOCKS_CLIENT */
   }
#line 4502 "config_parse.c"
    break;

  case 202: /* alarmside: %empty  */
#line 1755 "config_parse.y"
           {
#if !SOCKS_CLIENT
      alarmside = NULL;
   }
#line 4511 "config_parse.c"
    break;

  case 203: /* alarmside: RECVSIDE  */
#line 1759 "config_parse.y"
              {
      *alarmside = RECVSIDE;
   }
#line 4519 "config_parse.c"
    break;

  case 204: /* alarmside: SENDSIDE  */
#line 1762 "config_parse.y"
              {
      *alarmside = SENDSIDE;
#endif /* !SOCKS_CLIENT */
   }
#line 4528 "config_parse.c"
    break;

  case 205: /* $@20: %empty  */
#line 1768 "config_parse.y"
                                       { alarminit(); }
#line 4534 "config_parse.c"
    break;

  case 206: /* alarm_data: monitorside ALARMTYPE_DATA $@20 alarmside ':' NUMBER WORD__IN NUMBER  */
#line 1769 "config_parse.y"
                                    {
#if !SOCKS_CLIENT
   alarm_data_limit_t limit;

   ASSIGN_NUMBER((yyvsp[-2].number), >=, 0, limit.bytes, 0);
   ASSIGN_NUMBER((yyvsp[0].number), >, 0, limit.seconds, 1);

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
#line 4598 "config_parse.c"
    break;

  case 208: /* networkproblem: MTU_ERROR  */
#line 1833 "config_parse.y"
                          {
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
#line 4617 "config_parse.c"
    break;

  case 209: /* alarm_disconnect: monitorside ALARMTYPE_DISCONNECT ':' NUMBER '/' NUMBER alarmperiod  */
#line 1851 "config_parse.y"
                                                                      {
#if !SOCKS_CLIENT
   alarm_disconnect_limit_t limit;

   ASSIGN_NUMBER((yyvsp[-1].number), >, 0, limit.sessionc, 0);
   ASSIGN_NUMBER((yyvsp[-3].number), >, 0, limit.disconnectc, 0);
   ASSIGN_NUMBER((yyvsp[0].number), >, 0, limit.seconds, 1);

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
#line 4651 "config_parse.c"
    break;

  case 210: /* alarmperiod: %empty  */
#line 1882 "config_parse.y"
             {
#if !SOCKS_CLIENT
               (yyval.number) = DEFAULT_ALARM_PERIOD;
#endif /* !SOCKS_CLIENT */
   }
#line 4661 "config_parse.c"
    break;

  case 211: /* alarmperiod: WORD__IN NUMBER  */
#line 1887 "config_parse.y"
                     { (yyval.number) = (yyvsp[0].number); }
#line 4667 "config_parse.c"
    break;

  case 214: /* monitoroption: hostidoption  */
#line 1892 "config_parse.y"
                            { *hostidoption_isset = 1; }
#line 4673 "config_parse.c"
    break;

  case 216: /* monitoroptions: %empty  */
#line 1896 "config_parse.y"
                  { (yyval.string) = NULL; }
#line 4679 "config_parse.c"
    break;

  case 218: /* cruleoption: bounce  */
#line 1900 "config_parse.y"
                     {
#if !BAREFOOTD
                  yyerrorx("unsupported option");
#endif /* !BAREFOOTD */
   }
#line 4689 "config_parse.c"
    break;

  case 219: /* cruleoption: protocol  */
#line 1905 "config_parse.y"
                      {
#if !BAREFOOTD
                  yyerrorx("unsupported option");
#endif /* !BAREFOOTD */
   }
#line 4699 "config_parse.c"
    break;

  case 221: /* cruleoption: crulesessionoption  */
#line 1911 "config_parse.y"
                                {
#if !SOCKS_CLIENT
                  session_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 4709 "config_parse.c"
    break;

  case 223: /* $@21: %empty  */
#line 1919 "config_parse.y"
                  {

#if SOCKS_CLIENT || !HAVE_SOCKS_HOSTID
      yyerrorx("hostid is not supported on this system");
#endif /* SOCKS_CLIENT || !HAVE_SOCKS_HOSTID */

      objecttype = object_hrule;
}
#line 4722 "config_parse.c"
    break;

  case 224: /* hrule: HOSTIDRULE $@21 verdict '{' cruleoptions hostid_fromto cruleoptions '}'  */
#line 1926 "config_parse.y"
                                                          {
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
#line 4741 "config_parse.c"
    break;

  case 225: /* cruleoptions: %empty  */
#line 1944 "config_parse.y"
                { (yyval.string) = NULL; }
#line 4747 "config_parse.c"
    break;

  case 229: /* $@22: %empty  */
#line 1952 "config_parse.y"
                   {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
      addrinit(&hostid, 1);

#else /* HAVE_SOCKS_HOSTID */
      yyerrorx("hostid is not supported on this system");
#endif /* HAVE_SOCKS_HOSTID */

   }
#line 4761 "config_parse.c"
    break;

  case 231: /* hostindex: HOSTINDEX ':' NUMBER  */
#line 1963 "config_parse.y"
                                {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
   ASSIGN_NUMBER((yyvsp[0].number), >=, 0, *hostindex, 0);
   ASSIGN_NUMBER((yyvsp[0].number), <=, HAVE_MAX_HOSTIDS, *hostindex, 0);

#else
   yyerrorx("hostid is not supported on this system");
#endif /* !SOCKS_CLIENT && HAVE_SOCKS_HOSTID */
}
#line 4775 "config_parse.c"
    break;

  case 232: /* $@23: %empty  */
#line 1975 "config_parse.y"
                 { objecttype = object_srule; }
#line 4781 "config_parse.c"
    break;

  case 233: /* srule: SOCKSRULE $@23 verdict '{' sruleoptions fromto sruleoptions '}'  */
#line 1976 "config_parse.y"
                                                      {
#if !SOCKS_CLIENT
#if !HAVE_SOCKS_RULES
   yyerrorx("socks-rules are not used in %s", PRODUCT);
#endif /* !HAVE_SOCKS_RULES */

      pre_addrule(&rule);
      addsocksrule(&rule);
      post_addrule();
#endif /* !SOCKS_CLIENT */
   }
#line 4797 "config_parse.c"
    break;

  case 234: /* sruleoptions: %empty  */
#line 1990 "config_parse.y"
                { (yyval.string) = NULL; }
#line 4803 "config_parse.c"
    break;

  case 243: /* sruleoption: sockssessionoption  */
#line 2002 "config_parse.y"
                                {
#if !SOCKS_CLIENT
                  session_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 4813 "config_parse.c"
    break;

  case 245: /* genericruleoption: bandwidth  */
#line 2011 "config_parse.y"
                              {
#if !SOCKS_CLIENT
                        checkmodule("bandwidth");
                        bw_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 4824 "config_parse.c"
    break;

  case 253: /* genericruleoption: hostidoption  */
#line 2024 "config_parse.y"
                         { *hostidoption_isset = 1; }
#line 4830 "config_parse.c"
    break;

  case 258: /* genericruleoption: psid  */
#line 2029 "config_parse.y"
            {
#if !SOCKS_CLIENT
                     checkmodule("pac");
#endif /* !SOCKS_CLIENT */
   }
#line 4840 "config_parse.c"
    break;

  case 259: /* genericruleoption: psid_b64  */
#line 2034 "config_parse.y"
                {
#if !SOCKS_CLIENT
                     checkmodule("pac");
#endif /* !SOCKS_CLIENT */
   }
#line 4850 "config_parse.c"
    break;

  case 260: /* genericruleoption: psid_off  */
#line 2039 "config_parse.y"
                     {

#if !SOCKS_CLIENT

                     checkmodule("pac");

#endif /* !SOCKS_CLIENT */
   }
#line 4863 "config_parse.c"
    break;

  case 261: /* genericruleoption: redirect  */
#line 2047 "config_parse.y"
                       {
#if !SOCKS_CLIENT
                     checkmodule("redirect");
#endif /* !SOCKS_CLIENT */
   }
#line 4873 "config_parse.c"
    break;

  case 262: /* genericruleoption: socketoption  */
#line 2052 "config_parse.y"
                         {
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
#line 4901 "config_parse.c"
    break;

  case 310: /* ldapdebug: LDAPDEBUG ':' NUMBER  */
#line 2129 "config_parse.y"
                                {
#if SOCKS_SERVER
#if HAVE_LDAP && HAVE_OPENLDAP
      ldapauthorisation->debug = (int)(yyvsp[0].number);
   }
#line 4911 "config_parse.c"
    break;

  case 311: /* ldapdebug: LDAPDEBUG ':' '-' NUMBER  */
#line 2134 "config_parse.y"
                             {
      ldapauthorisation->debug = (int)-(yyvsp[0].number);
 #else /* !HAVE_LDAP */
      yyerrorx_nolib("openldap");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 4923 "config_parse.c"
    break;

  case 312: /* ldapauthdebug: LDAPAUTHDEBUG ':' NUMBER  */
#line 2143 "config_parse.y"
                                        {
#if SOCKS_SERVER
#if HAVE_LDAP && HAVE_OPENLDAP
      ldapauthentication->debug = (int)(yyvsp[0].number);
   }
#line 4933 "config_parse.c"
    break;

  case 313: /* ldapauthdebug: LDAPAUTHDEBUG ':' '-' NUMBER  */
#line 2148 "config_parse.y"
                                 {
      ldapauthentication->debug = (int)-(yyvsp[0].number);
 #else /* !HAVE_LDAP */
      yyerrorx_nolib("openldap");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 4945 "config_parse.c"
    break;

  case 314: /* ldapdomain: LDAPDOMAIN ':' LDAP_DOMAIN  */
#line 2157 "config_parse.y"
                                       {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthorisation.domain,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.domain) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 4962 "config_parse.c"
    break;

  case 315: /* ldapauthdomain: LDAPAUTHDOMAIN ':' LDAP_DOMAIN  */
#line 2171 "config_parse.y"
                                               {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthentication.domain,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthentication.domain) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 4979 "config_parse.c"
    break;

  case 316: /* ldapdepth: LDAPDEPTH ':' NUMBER  */
#line 2185 "config_parse.y"
                                {
#if SOCKS_SERVER
#if HAVE_LDAP && HAVE_OPENLDAP
      ldapauthorisation->mdepth = (int)(yyvsp[0].number);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("openldap");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 4993 "config_parse.c"
    break;

  case 317: /* ldapcertfile: LDAPCERTFILE ':' LDAP_CERTFILE  */
#line 2196 "config_parse.y"
                                             {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthorisation.certfile,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.certfile) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5010 "config_parse.c"
    break;

  case 318: /* ldapauthcertfile: LDAPAUTHCERTFILE ':' LDAP_CERTFILE  */
#line 2210 "config_parse.y"
                                                     {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthentication.certfile,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthentication.certfile) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5027 "config_parse.c"
    break;

  case 319: /* ldapcertpath: LDAPCERTPATH ':' LDAP_CERTPATH  */
#line 2224 "config_parse.y"
                                             {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthorisation.certpath,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.certpath) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5044 "config_parse.c"
    break;

  case 320: /* ldapauthcertpath: LDAPAUTHCERTPATH ':' LDAP_CERTPATH  */
#line 2238 "config_parse.y"
                                                     {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(state->ldapauthentication.certpath,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthentication.certpath) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */

      yyerrorx_nolib("LDAP");

#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5063 "config_parse.c"
    break;

  case 321: /* ldapurl: LDAPURL ':' LDAP_URL  */
#line 2254 "config_parse.y"
                              {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapurl, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5078 "config_parse.c"
    break;

  case 322: /* ldapauthurl: LDAPAUTHURL ':' LDAP_URL  */
#line 2266 "config_parse.y"
                                      {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapurl, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
      if (sockscf.state.ldapauthentication.ldapurl == NULL)
         sockscf.state.ldapauthentication.ldapurl = state->ldapauthentication.ldapurl;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5095 "config_parse.c"
    break;

  case 323: /* ldapauthbasedn: LDAPAUTHBASEDN ':' LDAP_BASEDN  */
#line 2280 "config_parse.y"
                                               {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapbasedn, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5110 "config_parse.c"
    break;

  case 324: /* ldapauthbasedn_hex: LDAPAUTHBASEDN_HEX ':' LDAP_BASEDN  */
#line 2292 "config_parse.y"
                                                       {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapbasedn, hextoutf8((yyvsp[0].string), 0)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5125 "config_parse.c"
    break;

  case 325: /* ldapauthbasedn_hex_all: LDAPAUTHBASEDN_HEX_ALL ':' LDAP_BASEDN  */
#line 2304 "config_parse.y"
                                                               {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapbasedn, hextoutf8((yyvsp[0].string), 1)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5140 "config_parse.c"
    break;

  case 326: /* lbasedn: LDAPBASEDN ':' LDAP_BASEDN  */
#line 2316 "config_parse.y"
                                    {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapbasedn, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5155 "config_parse.c"
    break;

  case 327: /* lbasedn_hex: LDAPBASEDN_HEX ':' LDAP_BASEDN  */
#line 2328 "config_parse.y"
                                            {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapbasedn, hextoutf8((yyvsp[0].string), 0)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5170 "config_parse.c"
    break;

  case 328: /* lbasedn_hex_all: LDAPBASEDN_HEX_ALL ':' LDAP_BASEDN  */
#line 2340 "config_parse.y"
                                                    {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapbasedn, hextoutf8((yyvsp[0].string), 1)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5185 "config_parse.c"
    break;

  case 329: /* ldapauthport: LDAPAUTHPORT ':' NUMBER  */
#line 2352 "config_parse.y"
                                      {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthentication->port = (int)(yyvsp[0].number);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5199 "config_parse.c"
    break;

  case 330: /* ldapport: LDAPPORT ':' NUMBER  */
#line 2363 "config_parse.y"
                              {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthorisation->port = (int)(yyvsp[0].number);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5213 "config_parse.c"
    break;

  case 331: /* ldapauthportssl: LDAPAUTHPORTSSL ':' NUMBER  */
#line 2374 "config_parse.y"
                                            {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthentication->portssl = (int)(yyvsp[0].number);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5227 "config_parse.c"
    break;

  case 332: /* ldapportssl: LDAPPORTSSL ':' NUMBER  */
#line 2385 "config_parse.y"
                                    {
#if SOCKS_SERVER
#if HAVE_LDAP
   ldapauthorisation->portssl = (int)(yyvsp[0].number);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5241 "config_parse.c"
    break;

  case 333: /* ldapssl: LDAPSSL ':' YES  */
#line 2396 "config_parse.y"
                         {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->ssl = 1;
   }
#line 5251 "config_parse.c"
    break;

  case 334: /* ldapssl: LDAPSSL ':' NO  */
#line 2401 "config_parse.y"
                    {
      ldapauthorisation->ssl = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5263 "config_parse.c"
    break;

  case 335: /* ldapauthssl: LDAPAUTHSSL ':' YES  */
#line 2410 "config_parse.y"
                                 {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->ssl = 1;
   }
#line 5273 "config_parse.c"
    break;

  case 336: /* ldapauthssl: LDAPAUTHSSL ':' NO  */
#line 2415 "config_parse.y"
                        {
      ldapauthentication->ssl = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5285 "config_parse.c"
    break;

  case 337: /* ldapauto: LDAPAUTO ':' YES  */
#line 2424 "config_parse.y"
                           {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->auto_off = 1;
   }
#line 5295 "config_parse.c"
    break;

  case 338: /* ldapauto: LDAPAUTO ':' NO  */
#line 2429 "config_parse.y"
                     {
      ldapauthorisation->auto_off = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5307 "config_parse.c"
    break;

  case 339: /* ldapauthauto: LDAPAUTHAUTO ':' YES  */
#line 2438 "config_parse.y"
                                   {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->auto_off = 1;
   }
#line 5317 "config_parse.c"
    break;

  case 340: /* ldapauthauto: LDAPAUTHAUTO ':' NO  */
#line 2443 "config_parse.y"
                         {
      ldapauthentication->auto_off = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5329 "config_parse.c"
    break;

  case 341: /* ldapcertcheck: LDAPCERTCHECK ':' YES  */
#line 2452 "config_parse.y"
                                      {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->certcheck = 1;
   }
#line 5339 "config_parse.c"
    break;

  case 342: /* ldapcertcheck: LDAPCERTCHECK ':' NO  */
#line 2457 "config_parse.y"
                          {
      ldapauthorisation->certcheck = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5351 "config_parse.c"
    break;

  case 343: /* ldapauthcertcheck: LDAPAUTHCERTCHECK ':' YES  */
#line 2466 "config_parse.y"
                                              {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->certcheck = 1;
   }
#line 5361 "config_parse.c"
    break;

  case 344: /* ldapauthcertcheck: LDAPAUTHCERTCHECK ':' NO  */
#line 2471 "config_parse.y"
                              {
      ldapauthentication->certcheck = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5373 "config_parse.c"
    break;

  case 345: /* ldapauthkeeprealm: LDAPAUTHKEEPREALM ':' YES  */
#line 2480 "config_parse.y"
                                              {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthentication->keeprealm = 1;
   }
#line 5383 "config_parse.c"
    break;

  case 346: /* ldapauthkeeprealm: LDAPAUTHKEEPREALM ':' NO  */
#line 2485 "config_parse.y"
                              {
      ldapauthentication->keeprealm = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5395 "config_parse.c"
    break;

  case 347: /* ldapkeeprealm: LDAPKEEPREALM ':' YES  */
#line 2495 "config_parse.y"
                                      {
#if SOCKS_SERVER
#if HAVE_LDAP
      ldapauthorisation->keeprealm = 1;
   }
#line 5405 "config_parse.c"
    break;

  case 348: /* ldapkeeprealm: LDAPKEEPREALM ':' NO  */
#line 2500 "config_parse.y"
                          {
      ldapauthorisation->keeprealm = 0;
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5417 "config_parse.c"
    break;

  case 349: /* ldapfilter: LDAPFILTER ':' LDAP_FILTER  */
#line 2509 "config_parse.y"
                                       {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKLEN(ldapauthorisation->filter, (yyvsp[0].string), sizeof(state->ldapauthorisation.filter) - 1, yyerrorx);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5431 "config_parse.c"
    break;

  case 350: /* ldapauthfilter: LDAPAUTHFILTER ':' LDAP_FILTER  */
#line 2520 "config_parse.y"
                                               {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKLEN(ldapauthentication->filter, (yyvsp[0].string), sizeof(state->ldapauthentication.filter) - 1, yyerrorx);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5445 "config_parse.c"
    break;

  case 351: /* ldapfilter_ad: LDAPFILTER_AD ':' LDAP_FILTER  */
#line 2531 "config_parse.y"
                                             {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(ldapauthorisation->filter_AD,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.filter_AD) - 1,
                      yyerrorx);

#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5463 "config_parse.c"
    break;

  case 352: /* ldapfilter_hex: LDAPFILTER_HEX ':' LDAP_FILTER  */
#line 2546 "config_parse.y"
                                               {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKUTFLEN(ldapauthorisation->filter,
                          (yyvsp[0].string),
                          sizeof(state->ldapauthorisation.filter) - 1,
                          yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5480 "config_parse.c"
    break;

  case 353: /* ldapfilter_ad_hex: LDAPFILTER_AD_HEX ':' LDAP_FILTER  */
#line 2560 "config_parse.y"
                                                     {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKUTFLEN(ldapauthorisation->filter_AD,
                        (yyvsp[0].string),
                        sizeof(state->ldapauthorisation.filter_AD) - 1,
                        yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5497 "config_parse.c"
    break;

  case 354: /* ldapattribute: LDAPATTRIBUTE ':' LDAP_ATTRIBUTE  */
#line 2574 "config_parse.y"
                                                {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(ldapauthorisation->attribute,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.attribute) - 1,
                      yyerrorx);

#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5515 "config_parse.c"
    break;

  case 355: /* ldapattribute_ad: LDAPATTRIBUTE_AD ':' LDAP_ATTRIBUTE  */
#line 2589 "config_parse.y"
                                                      {
#if SOCKS_SERVER
#if HAVE_LDAP
      STRCPY_CHECKLEN(ldapauthorisation->attribute_AD,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.attribute_AD) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5532 "config_parse.c"
    break;

  case 356: /* ldapattribute_hex: LDAPATTRIBUTE_HEX ':' LDAP_ATTRIBUTE  */
#line 2603 "config_parse.y"
                                                        {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKUTFLEN(ldapauthorisation->attribute,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.attribute) -1,
                      yyerrorx);
#else /* !HAVE_LDAP */
   yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5549 "config_parse.c"
    break;

  case 357: /* ldapattribute_ad_hex: LDAPATTRIBUTE_AD_HEX ':' LDAP_ATTRIBUTE  */
#line 2617 "config_parse.y"
                                                              {
#if SOCKS_SERVER
#if HAVE_LDAP
   STRCPY_CHECKUTFLEN(ldapauthorisation->attribute_AD,
                      (yyvsp[0].string),
                      sizeof(state->ldapauthorisation.attribute_AD) - 1,
                      yyerrorx);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5566 "config_parse.c"
    break;

  case 358: /* lgroup_hex: LDAPGROUP_HEX ':' LDAPGROUP_NAME  */
#line 2631 "config_parse.y"
                                             {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&rule.ldapgroup, hextoutf8((yyvsp[0].string), 0)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5581 "config_parse.c"
    break;

  case 359: /* lgroup_hex_all: LDAPGROUP_HEX_ALL ':' LDAPGROUP_NAME  */
#line 2643 "config_parse.y"
                                                     {
#if SOCKS_SERVER
#if HAVE_LDAP
      checkmodule("ldap");

      if (addlinkedname(&rule.ldapgroup, hextoutf8((yyvsp[0].string), 1)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5598 "config_parse.c"
    break;

  case 360: /* lgroup: LDAPGROUP ':' LDAPGROUP_NAME  */
#line 2657 "config_parse.y"
                                     {
#if SOCKS_SERVER
#if HAVE_LDAP
      checkmodule("ldap");

      if (addlinkedname(&rule.ldapgroup, asciitoutf8((yyvsp[0].string))) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5615 "config_parse.c"
    break;

  case 361: /* lserver: LDAPSERVER ':' LDAPSERVER_NAME  */
#line 2671 "config_parse.y"
                                        {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthorisation.ldapserver, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5630 "config_parse.c"
    break;

  case 362: /* ldapauthserver: LDAPAUTHSERVER ':' LDAPSERVER_NAME  */
#line 2683 "config_parse.y"
                                                   {
#if SOCKS_SERVER
#if HAVE_LDAP
      if (addlinkedname(&state->ldapauthentication.ldapserver, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("LDAP");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5645 "config_parse.c"
    break;

  case 363: /* ldapkeytab: LDAPKEYTAB ':' LDAPKEYTABNAME  */
#line 2695 "config_parse.y"
                                          {
#if HAVE_LDAP
#if SOCKS_SERVER
   STRCPY_CHECKLEN(state->ldapauthorisation.keytab,
                   (yyvsp[0].string),
                   sizeof(state->ldapauthorisation.keytab) - 1, yyerrorx);
#else
   yyerrorx("LDAP keytab only applicable to Dante server");
#endif /* SOCKS_SERVER */
#else
      yyerrorx_nolib("LDAP");
#endif /* HAVE_LDAP */
   }
#line 5663 "config_parse.c"
    break;

  case 364: /* ldapauthkeytab: LDAPAUTHKEYTAB ':' LDAPKEYTABNAME  */
#line 2710 "config_parse.y"
                                                  {
#if HAVE_LDAP
#if SOCKS_SERVER
   STRCPY_CHECKLEN(state->ldapauthentication.keytab,
                   (yyvsp[0].string),
                   sizeof(state->ldapauthentication.keytab) - 1, yyerrorx);
#else
   yyerrorx("LDAP keytab only applicable to Dante server");
#endif /* SOCKS_SERVER */
#else
      yyerrorx_nolib("LDAP");
#endif /* HAVE_LDAP */
   }
#line 5681 "config_parse.c"
    break;

  case 365: /* psid: PACSID ':' PACSID_NAME  */
#line 2725 "config_parse.y"
                             {
#if SOCKS_SERVER
#if HAVE_PAC
      char b64[MAX_BASE64_LEN];

      checkmodule("pac");

      if (sidtob64((yyvsp[0].string), b64, sizeof(b64)) != 0)
         yyerrorx("invalid input: %s)", (yyvsp[0].string));
      if (addlinkedname(&rule.objectsids, b64) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("PAC");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5702 "config_parse.c"
    break;

  case 366: /* psid_b64: PACSID_B64 ':' PACSID_NAME  */
#line 2743 "config_parse.y"
                                     {
#if SOCKS_SERVER
#if HAVE_PAC
      char sid[MAX_BASE64_LEN];
      checkmodule("pac");

      /* attempt conversion to check if input makes sense */
      if (b64tosid((yyvsp[0].string), sid, sizeof(sid)) != 0)
         yyerrorx("invalid input: %s)", (yyvsp[0].string));
      if (addlinkedname(&rule.objectsids, (yyvsp[0].string)) == NULL)
         yyerror(NOMEM);
#else /* !HAVE_LDAP */
      yyerrorx_nolib("PAC");
#endif /* !HAVE_LDAP */
#endif /* SOCKS_SERVER */
   }
#line 5723 "config_parse.c"
    break;

  case 367: /* psid_off: PACSID_FLAG ':' YES  */
#line 2761 "config_parse.y"
                              {
#if SOCKS_SERVER
#if HAVE_PAC
      checkmodule("pac");
      rule.pacoff = 1;
   }
#line 5734 "config_parse.c"
    break;

  case 368: /* psid_off: PACSID_FLAG ':' NO  */
#line 2767 "config_parse.y"
                        {
      checkmodule("pac");
      rule.pacoff = 0;
#else /* !HAVE_PAC */
      yyerrorx_nolib("PAC");
#endif /* !HAVE_PAC */
#endif /* SOCKS_SERVER */
   }
#line 5747 "config_parse.c"
    break;

  case 370: /* clientcompatibilityname: NECGSSAPI  */
#line 2780 "config_parse.y"
                                   {
#if HAVE_GSSAPI
      gssapiencryption->nec = 1;
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
#line 5759 "config_parse.c"
    break;

  case 373: /* verdict: VERDICT_BLOCK  */
#line 2794 "config_parse.y"
                         {
#if !SOCKS_CLIENT
      ruleinit(&rule);
      rule.verdict   = VERDICT_BLOCK;
   }
#line 5769 "config_parse.c"
    break;

  case 374: /* verdict: VERDICT_PASS  */
#line 2799 "config_parse.y"
                    {
      ruleinit(&rule);
      rule.verdict   = VERDICT_PASS;
#endif /* !SOCKS_CLIENT */
   }
#line 5779 "config_parse.c"
    break;

  case 378: /* commandname: COMMAND_BIND  */
#line 2813 "config_parse.y"
                            {
         state->command.bind = 1;
   }
#line 5787 "config_parse.c"
    break;

  case 379: /* commandname: COMMAND_CONNECT  */
#line 2816 "config_parse.y"
                       {
         state->command.connect = 1;
   }
#line 5795 "config_parse.c"
    break;

  case 380: /* commandname: COMMAND_UDPASSOCIATE  */
#line 2819 "config_parse.y"
                            {
         state->command.udpassociate = 1;
   }
#line 5803 "config_parse.c"
    break;

  case 381: /* commandname: COMMAND_BINDREPLY  */
#line 2825 "config_parse.y"
                           {
         state->command.bindreply = 1;
   }
#line 5811 "config_parse.c"
    break;

  case 382: /* commandname: COMMAND_UDPREPLY  */
#line 2829 "config_parse.y"
                        {
         state->command.udpreply = 1;
   }
#line 5819 "config_parse.c"
    break;

  case 386: /* protocolname: PROTOCOL_TCP  */
#line 2842 "config_parse.y"
                           {
      state->protocol.tcp = 1;
   }
#line 5827 "config_parse.c"
    break;

  case 387: /* protocolname: PROTOCOL_UDP  */
#line 2845 "config_parse.y"
                           {
      state->protocol.udp = 1;
   }
#line 5835 "config_parse.c"
    break;

  case 399: /* sessioninheritable: SESSION_INHERITABLE ':' YES  */
#line 2874 "config_parse.y"
                                                {
#if !SOCKS_CLIENT
                        rule.ss_isinheritable = 1;
   }
#line 5844 "config_parse.c"
    break;

  case 400: /* sessioninheritable: SESSION_INHERITABLE ':' NO  */
#line 2878 "config_parse.y"
                                {
                        rule.ss_isinheritable = 0;
#endif /* !SOCKS_CLIENT */
   }
#line 5853 "config_parse.c"
    break;

  case 401: /* sessionmax: SESSIONMAX ':' NUMBER  */
#line 2884 "config_parse.y"
                                  {
#if !SOCKS_CLIENT
      ASSIGN_MAXSESSIONS((yyvsp[0].number), ss.object.ss.max, 0);
      ss.object.ss.max       = (yyvsp[0].number);
      ss.object.ss.max_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 5865 "config_parse.c"
    break;

  case 402: /* sessionthrottle: SESSIONTHROTTLE ':' NUMBER '/' NUMBER  */
#line 2893 "config_parse.y"
                                                       {
#if !SOCKS_CLIENT
      ASSIGN_THROTTLE_SECONDS((yyvsp[-2].number), ss.object.ss.throttle.limit.clients, 0);
      ASSIGN_THROTTLE_CLIENTS((yyvsp[0].number), ss.object.ss.throttle.limit.seconds, 0);
      ss.object.ss.throttle_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 5877 "config_parse.c"
    break;

  case 407: /* sessionstate_key: SESSIONSTATE_KEY ':' STATEKEY  */
#line 2908 "config_parse.y"
                                                {
#if !SOCKS_CLIENT
      if ((ss.keystate.key = string2statekey((yyvsp[0].string))) == key_unset)
         yyerrorx("%s is not a valid state key", (yyvsp[0].string));

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
#line 5908 "config_parse.c"
    break;

  case 408: /* $@24: %empty  */
#line 2936 "config_parse.y"
                                           {
#if !SOCKS_CLIENT && HAVE_SOCKS_HOSTID
      hostindex = &ss.keystate.keyinfo.hostindex;
   }
#line 5917 "config_parse.c"
    break;

  case 409: /* sessionstate_keyinfo: SESSIONSTATE_KEY '.' $@24 hostindex  */
#line 2940 "config_parse.y"
             {
      hostindex = &rule.hostindex; /* reset */
#endif /* !SOCKS_CLIENT && HAVE_SOCKS_HOSTID */
   }
#line 5926 "config_parse.c"
    break;

  case 410: /* sessionstate_max: SESSIONSTATE_MAX ':' NUMBER  */
#line 2947 "config_parse.y"
                                              {
#if !SOCKS_CLIENT
      ASSIGN_MAXSESSIONS((yyvsp[0].number), ss.object.ss.max_perstate, 0);
      ss.object.ss.max_perstate_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 5937 "config_parse.c"
    break;

  case 411: /* sessionstate_throttle: SESSIONSTATE_THROTTLE ':' NUMBER '/' NUMBER  */
#line 2955 "config_parse.y"
                                                                   {
#if !SOCKS_CLIENT
   ASSIGN_THROTTLE_SECONDS((yyvsp[-2].number), ss.object.ss.throttle_perstate.limit.clients, 0);
   ASSIGN_THROTTLE_CLIENTS((yyvsp[0].number), ss.object.ss.throttle_perstate.limit.seconds, 0);
   ss.object.ss.throttle_perstate_isset = 1;
#endif /* !SOCKS_CLIENT */
}
#line 5949 "config_parse.c"
    break;

  case 412: /* bandwidth: BANDWIDTH ':' NUMBER  */
#line 2964 "config_parse.y"
                                  {
#if !SOCKS_CLIENT
      ASSIGN_NUMBER((yyvsp[0].number), >=, 0, bw.object.bw.maxbps, 0);
      bw.object.bw.maxbps_isset = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 5960 "config_parse.c"
    break;

  case 414: /* logname: RULE_LOG_CONNECT  */
#line 2976 "config_parse.y"
                           {
#if !SOCKS_CLIENT
         rule.log.connect = 1;
   }
#line 5969 "config_parse.c"
    break;

  case 415: /* logname: RULE_LOG_DATA  */
#line 2980 "config_parse.y"
                     {
         rule.log.data = 1;
   }
#line 5977 "config_parse.c"
    break;

  case 416: /* logname: RULE_LOG_DISCONNECT  */
#line 2983 "config_parse.y"
                           {
         rule.log.disconnect = 1;
   }
#line 5985 "config_parse.c"
    break;

  case 417: /* logname: RULE_LOG_ERROR  */
#line 2986 "config_parse.y"
                      {
         rule.log.error = 1;
   }
#line 5993 "config_parse.c"
    break;

  case 418: /* logname: RULE_LOG_IOOPERATION  */
#line 2989 "config_parse.y"
                            {
         rule.log.iooperation = 1;
   }
#line 6001 "config_parse.c"
    break;

  case 419: /* logname: RULE_LOG_TCPINFO  */
#line 2992 "config_parse.y"
                        {
         rule.log.tcpinfo = 1;
#endif /* !SOCKS_CLIENT */
   }
#line 6010 "config_parse.c"
    break;

  case 422: /* pamservicename: PAMSERVICENAME ':' SERVICENAME  */
#line 3003 "config_parse.y"
                                               {
#if HAVE_PAM && (!SOCKS_CLIENT)
      STRCPY_CHECKLEN(state->pamservicename,
                      (yyvsp[0].string),
                      sizeof(state->pamservicename) -1,
                      yyerrorx);
#else
      yyerrorx_nolib("PAM");
#endif /* HAVE_PAM && (!SOCKS_CLIENT) */
   }
#line 6025 "config_parse.c"
    break;

  case 423: /* bsdauthstylename: BSDAUTHSTYLE ':' BSDAUTHSTYLENAME  */
#line 3015 "config_parse.y"
                                                    {
#if HAVE_BSDAUTH && SOCKS_SERVER
      STRCPY_CHECKLEN(state->bsdauthstylename,
                      (yyvsp[0].string),
                      sizeof(state->bsdauthstylename) - 1,
                      yyerrorx);
#else
      yyerrorx_nolib("bsdauth");
#endif /* HAVE_BSDAUTH && SOCKS_SERVER */
   }
#line 6040 "config_parse.c"
    break;

  case 424: /* gssapiservicename: GSSAPISERVICE ':' GSSAPISERVICENAME  */
#line 3028 "config_parse.y"
                                                       {
#if HAVE_GSSAPI
      STRCPY_CHECKLEN(gssapiservicename,
                      (yyvsp[0].string),
                      sizeof(state->gssapiservicename) - 1,
                      yyerrorx);
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
#line 6055 "config_parse.c"
    break;

  case 425: /* gssapikeytab: GSSAPIKEYTAB ':' GSSAPIKEYTABNAME  */
#line 3040 "config_parse.y"
                                                {
#if HAVE_GSSAPI
#if SOCKS_SERVER
      STRCPY_CHECKLEN(gssapikeytab,
                       (yyvsp[0].string),
                       sizeof(state->gssapikeytab) - 1,
                       yyerrorx);
#else
      yyerrorx("gssapi keytab setting is only applicable to Dante server");
#endif /* SOCKS_SERVER */
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
#line 6074 "config_parse.c"
    break;

  case 427: /* gssapienctypename: GSSAPIENC_ANY  */
#line 3059 "config_parse.y"
                                 {
#if HAVE_GSSAPI
      gssapiencryption->clear           = 1;
      gssapiencryption->integrity       = 1;
      gssapiencryption->confidentiality = 1;
   }
#line 6085 "config_parse.c"
    break;

  case 428: /* gssapienctypename: GSSAPIENC_CLEAR  */
#line 3065 "config_parse.y"
                      {
      gssapiencryption->clear = 1;
   }
#line 6093 "config_parse.c"
    break;

  case 429: /* gssapienctypename: GSSAPIENC_INTEGRITY  */
#line 3068 "config_parse.y"
                          {
      gssapiencryption->integrity = 1;
   }
#line 6101 "config_parse.c"
    break;

  case 430: /* gssapienctypename: GSSAPIENC_CONFIDENTIALITY  */
#line 3071 "config_parse.y"
                                {
      gssapiencryption->confidentiality = 1;
   }
#line 6109 "config_parse.c"
    break;

  case 431: /* gssapienctypename: GSSAPIENC_PERMESSAGE  */
#line 3074 "config_parse.y"
                           {
      yyerrorx("gssapi per-message encryption not supported");
#else
      yyerrorx_nolib("GSSAPI");
#endif /* HAVE_GSSAPI */
   }
#line 6120 "config_parse.c"
    break;

  case 435: /* libwrap: LIBWRAPSTART ':' LINE  */
#line 3089 "config_parse.y"
                                 {
#if HAVE_LIBWRAP && (!SOCKS_CLIENT)
      struct request_info request;
      char tmp[LIBWRAPBUF];
      int errno_s, devnull;

      STRCPY_CHECKLEN(rule.libwrap,
                      (yyvsp[0].string),
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
#line 6162 "config_parse.c"
    break;

  case 440: /* rdr_toaddress: rdr_to ':' address  */
#line 3141 "config_parse.y"
                                  {
#if BAREFOOTD
      yyerrorx("redirecting \"to\" an address does not make any sense in %s.  "
               "Instead specify the address you wanted to \"redirect\" "
               "data to as the \"bounce to\" address, as normal",
               PRODUCT);
#endif /* BAREFOOT */
   }
#line 6175 "config_parse.c"
    break;

  case 452: /* routeoption: socketoption  */
#line 3164 "config_parse.y"
                          {
               if (!addedsocketoption(&route.socketoptionc,
                                      &route.socketoptionv,
                                      &socketopt))
                  yywarn("could not add socketoption");
   }
#line 6186 "config_parse.c"
    break;

  case 453: /* routeoptions: %empty  */
#line 3172 "config_parse.y"
               { (yyval.string) = NULL; }
#line 6192 "config_parse.c"
    break;

  case 456: /* from: FROM  */
#line 3179 "config_parse.y"
             {
      addrinit(&src, 1);
   }
#line 6200 "config_parse.c"
    break;

  case 457: /* to: TO  */
#line 3184 "config_parse.y"
         {
      addrinit(&dst, ipaddr_requires_netmask(to, objecttype));
   }
#line 6208 "config_parse.c"
    break;

  case 458: /* rdr_from: FROM  */
#line 3189 "config_parse.y"
                 {
      addrinit(&rdr_from, 1);
   }
#line 6216 "config_parse.c"
    break;

  case 459: /* rdr_to: TO  */
#line 3194 "config_parse.y"
             {
      addrinit(&rdr_to, 0);
   }
#line 6224 "config_parse.c"
    break;

  case 460: /* bounceto: TO  */
#line 3199 "config_parse.y"
               {
#if BAREFOOTD
      addrinit(&bounceto, 0);
#endif /* BAREFOOTD */
   }
#line 6234 "config_parse.c"
    break;

  case 461: /* via: VIA  */
#line 3207 "config_parse.y"
           {
      gwaddrinit(&gw);
   }
#line 6242 "config_parse.c"
    break;

  case 470: /* ipaddress: ipv4 '/' netmask_v4  */
#line 3227 "config_parse.y"
                               { if (!netmask_required) yyerrorx_hasnetmask(); }
#line 6248 "config_parse.c"
    break;

  case 471: /* ipaddress: ipv4  */
#line 3228 "config_parse.y"
                               { if (netmask_required)  yyerrorx_nonetmask();  }
#line 6254 "config_parse.c"
    break;

  case 472: /* ipaddress: ipv6 '/' netmask_v6  */
#line 3229 "config_parse.y"
                               { if (!netmask_required) yyerrorx_hasnetmask(); }
#line 6260 "config_parse.c"
    break;

  case 473: /* ipaddress: ipv6  */
#line 3230 "config_parse.y"
                               { if (netmask_required)  yyerrorx_nonetmask();  }
#line 6266 "config_parse.c"
    break;

  case 474: /* ipaddress: ipvany '/' netmask_vany  */
#line 3231 "config_parse.y"
                                   { if (!netmask_required)
                                       yyerrorx_hasnetmask(); }
#line 6273 "config_parse.c"
    break;

  case 475: /* ipaddress: ipvany  */
#line 3233 "config_parse.y"
                               { if (netmask_required)  yyerrorx_nonetmask();  }
#line 6279 "config_parse.c"
    break;

  case 478: /* gwaddress: ifname  */
#line 3237 "config_parse.y"
                    { /* for upnp; broadcasts on interface. */ }
#line 6285 "config_parse.c"
    break;

  case 482: /* ipv4: IPV4  */
#line 3246 "config_parse.y"
             {
      *atype = SOCKS_ADDR_IPV4;

      if (socks_inet_pton(AF_INET, (yyvsp[0].string), ipv4, NULL) != 1)
         yyerror("bad %s: %s", atype2string(*atype), (yyvsp[0].string));
   }
#line 6296 "config_parse.c"
    break;

  case 483: /* netmask_v4: NUMBER  */
#line 3254 "config_parse.y"
                     {
      if ((yyvsp[0].number) < 0 || (yyvsp[0].number) > 32)
         yyerrorx("bad %s netmask: %ld.  Legal range is 0 - 32",
                  atype2string(*atype), (long)(yyvsp[0].number));

      netmask_v4->s_addr = (yyvsp[0].number) == 0 ? 0 : htonl(IPV4_FULLNETMASK << (32 - (yyvsp[0].number)));
   }
#line 6308 "config_parse.c"
    break;

  case 484: /* netmask_v4: IPV4  */
#line 3261 "config_parse.y"
                   {
      if (socks_inet_pton(AF_INET, (yyvsp[0].string), netmask_v4, NULL) != 1)
         yyerror("bad %s netmask: %s", atype2string(*atype), (yyvsp[0].string));
   }
#line 6317 "config_parse.c"
    break;

  case 485: /* ipv6: IPV6  */
#line 3267 "config_parse.y"
             {
      *atype = SOCKS_ADDR_IPV6;

      if (socks_inet_pton(AF_INET6, (yyvsp[0].string), ipv6, scopeid_v6) != 1)
         yyerror("bad %s: %s", atype2string(*atype), (yyvsp[0].string));
   }
#line 6328 "config_parse.c"
    break;

  case 486: /* netmask_v6: NUMBER  */
#line 3275 "config_parse.y"
                     {
      if ((yyvsp[0].number) < 0 || (yyvsp[0].number) > IPV6_NETMASKBITS)
         yyerrorx("bad %s netmask: %d.  Legal range is 0 - %d",
                  atype2string(*atype), (int)(yyvsp[0].number), IPV6_NETMASKBITS);

      *netmask_v6 = (yyvsp[0].number);
   }
#line 6340 "config_parse.c"
    break;

  case 487: /* ipvany: IPVANY  */
#line 3284 "config_parse.y"
                 {
      SASSERTX(strcmp((yyvsp[0].string), "0") == 0);

      *atype = SOCKS_ADDR_IPVANY;
      ipvany->s_addr = htonl(0);
   }
#line 6351 "config_parse.c"
    break;

  case 488: /* netmask_vany: NUMBER  */
#line 3292 "config_parse.y"
                       {
      if ((yyvsp[0].number) != 0)
         yyerrorx("bad %s netmask: %d.  Only legal value is 0",
                  atype2string(*atype), (int)(yyvsp[0].number));

      netmask_vany->s_addr = htonl((yyvsp[0].number));
   }
#line 6363 "config_parse.c"
    break;

  case 489: /* domain: DOMAINNAME  */
#line 3302 "config_parse.y"
                     {
      *atype = SOCKS_ADDR_DOMAIN;
      STRCPY_CHECKLEN(domain, (yyvsp[0].string), MAXHOSTNAMELEN - 1, yyerrorx);
   }
#line 6372 "config_parse.c"
    break;

  case 490: /* ifname: IFNAME  */
#line 3308 "config_parse.y"
                 {
      *atype = SOCKS_ADDR_IFNAME;
      STRCPY_CHECKLEN(ifname, (yyvsp[0].string), MAXIFNAMELEN - 1, yyerrorx);
   }
#line 6381 "config_parse.c"
    break;

  case 491: /* url: URL  */
#line 3315 "config_parse.y"
           {
      *atype = SOCKS_ADDR_URL;
      STRCPY_CHECKLEN(url, (yyvsp[0].string), MAXURLLEN - 1, yyerrorx);
   }
#line 6390 "config_parse.c"
    break;

  case 492: /* port: %empty  */
#line 3322 "config_parse.y"
      { (yyval.number) = 0; }
#line 6396 "config_parse.c"
    break;

  case 496: /* gwport: %empty  */
#line 3328 "config_parse.y"
        { (yyval.number) = 0; }
#line 6402 "config_parse.c"
    break;

  case 500: /* portrange: portstart '-' portend  */
#line 3336 "config_parse.y"
                                   {
   if (ntohs(*port_tcp) > ntohs(ruleaddr->portend))
      yyerrorx("end port (%u) can not be less than start port (%u)",
      ntohs(*port_tcp), ntohs(ruleaddr->portend));
   }
#line 6412 "config_parse.c"
    break;

  case 501: /* portstart: NUMBER  */
#line 3344 "config_parse.y"
                    {
      ASSIGN_PORTNUMBER((yyvsp[0].number), *port_tcp);
      ASSIGN_PORTNUMBER((yyvsp[0].number), *port_udp);
   }
#line 6421 "config_parse.c"
    break;

  case 502: /* portend: NUMBER  */
#line 3350 "config_parse.y"
                  {
      ASSIGN_PORTNUMBER((yyvsp[0].number), ruleaddr->portend);
      ruleaddr->operator   = range;
   }
#line 6430 "config_parse.c"
    break;

  case 503: /* portservice: SERVICENAME  */
#line 3356 "config_parse.y"
                           {
      struct servent   *service;

      if ((service = getservbyname((yyvsp[0].string), "tcp")) == NULL) {
         if (state->protocol.tcp)
            yyerrorx("unknown tcp protocol: %s", (yyvsp[0].string));

         *port_tcp = htons(0);
      }
      else
         *port_tcp = (in_port_t)service->s_port;

      if ((service = getservbyname((yyvsp[0].string), "udp")) == NULL) {
         if (state->protocol.udp)
               yyerrorx("unknown udp protocol: %s", (yyvsp[0].string));

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

      (yyval.number) = (size_t)*port_udp;
   }
#line 6467 "config_parse.c"
    break;

  case 504: /* portoperator: OPERATOR  */
#line 3391 "config_parse.y"
                         {
      *operator = string2operator((yyvsp[0].string));
   }
#line 6475 "config_parse.c"
    break;

  case 506: /* udpportrange_start: NUMBER  */
#line 3400 "config_parse.y"
                           {
#if SOCKS_SERVER
   ASSIGN_PORTNUMBER((yyvsp[0].number), rule.udprange.start);
#endif /* SOCKS_SERVER */
   }
#line 6485 "config_parse.c"
    break;

  case 507: /* udpportrange_end: NUMBER  */
#line 3407 "config_parse.y"
                         {
#if SOCKS_SERVER
   ASSIGN_PORTNUMBER((yyvsp[0].number), rule.udprange.end);
   rule.udprange.op  = range;

   if (ntohs(rule.udprange.start) > ntohs(rule.udprange.end))
      yyerrorx("end port (%d) can not be less than start port (%u)",
               (int)(yyvsp[0].number), ntohs(rule.udprange.start));
#endif /* SOCKS_SERVER */
   }
#line 6500 "config_parse.c"
    break;

  case 508: /* number: NUMBER  */
#line 3419 "config_parse.y"
               {
      addnumber(&numberc, &numberv, (yyvsp[0].number));
   }
#line 6508 "config_parse.c"
    break;


#line 6512 "config_parse.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 3429 "config_parse.y"


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

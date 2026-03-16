/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_SOCKS_YY_CONFIG_PARSE_H_INCLUDED
# define YY_SOCKS_YY_CONFIG_PARSE_H_INCLUDED
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

#line 570 "config_parse.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE socks_yylval;


int socks_yyparse (void);


#endif /* !YY_SOCKS_YY_CONFIG_PARSE_H_INCLUDED  */

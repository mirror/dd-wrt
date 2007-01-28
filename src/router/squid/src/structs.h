
/*
 * $Id: structs.h,v 1.507.2.1 2007/01/21 10:26:44 hno Exp $
 *
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#ifndef SQUID_STRUCTS_H
#define SQUID_STRUCTS_H

#include "config.h"
#include "splay.h"

struct _dlink_node {
    void *data;
    dlink_node *prev;
    dlink_node *next;
};

struct _dlink_list {
    dlink_node *head;
    dlink_node *tail;
};

#if USE_SSL
struct _acl_cert_data {
    splayNode *values;
    char *attribute;
};

#endif

struct _acl_user_data {
    splayNode *names;
    struct {
	unsigned int case_insensitive:1;
	unsigned int required:1;
    } flags;
};

struct _acl_user_ip_data {
    int max;
    struct {
	unsigned int strict:1;
    } flags;
};

struct _acl_ip_data {
    struct in_addr addr1;	/* if addr2 non-zero then its a range */
    struct in_addr addr2;
    struct in_addr mask;
    acl_ip_data *next;		/* used for parsing, not for storing */
};

struct _acl_time_data {
    int weekbits;
    int start;
    int stop;
    acl_time_data *next;
};

struct _acl_name_list {
    char name[ACL_NAME_SZ];
    acl_name_list *next;
};

struct _acl_proxy_auth_match_cache {
    dlink_node link;
    int matchrv;
    void *acl_data;
};

struct _acl_hdr_data {
    acl_hdr_data *next;
    relist *reglist;
    http_hdr_type hdr_id;
    const char *hdr_name;
};

struct _auth_user_hash_pointer {
    /* first two items must be same as hash_link */
    char *key;
    auth_user_hash_pointer *next;
    auth_user_t *auth_user;
    dlink_node link;		/* other hash entries that point to the same auth_user */
};

struct _auth_user_ip_t {
    dlink_node node;
    /* IP addr this user authenticated from */
    struct in_addr ipaddr;
    time_t ip_expiretime;
};

struct _auth_user_t {
    /* extra fields for proxy_auth */
    /* this determines what scheme owns the user data. */
    auth_type_t auth_type;
    /* the index +1 in the authscheme_list to the authscheme entry */
    int auth_module;
    /* we only have one username associated with a given auth_user struct */
    auth_user_hash_pointer *usernamehash;
    /* cache of acl lookups on this username */
    dlink_list proxy_match_cache;
    /* what ip addresses has this user been seen at?, plus a list length cache */
    dlink_list ip_list;
    int ipcount;
    long expiretime;
    /* how many references are outstanding to this instance */
    int references;
    /* the auth scheme has it's own private data area */
    void *scheme_data;
    /* the auth_user_request structures that link to this. Yes it could be a splaytree
     * but how many requests will a single username have in parallel? */
    dlink_list requests;
};

struct _auth_user_request_t {
    /* this is the object passed around by client_side and acl functions */
    /* it has request specific data, and links to user specific data */
    /* the user */
    auth_user_t *auth_user;
    /* return a message on the 407 error pages */
    char *message;
    /* any scheme specific request related data */
    void *scheme_data;
    /* how many 'processes' are working on this data */
    int references;
    /* We only attempt authentication once per http request. This 
     * is to allow multiple auth acl references from different _access areas
     * when using connection based authentication
     */
    auth_acl_t lastReply;
};


/*
 * This defines an auth scheme module
 */

struct _authscheme_entry {
    const char *typestr;
    AUTHSACTIVE *Active;
    AUTHSADDHEADER *AddHeader;
    AUTHSADDTRAILER *AddTrailer;
    AUTHSAUTHED *authenticated;
    AUTHSAUTHUSER *authAuthenticate;
    AUTHSCONFIGURED *configured;
    AUTHSDUMP *dump;
    AUTHSFIXERR *authFixHeader;
    AUTHSFREE *FreeUser;
    AUTHSFREECONFIG *freeconfig;
    AUTHSUSERNAME *authUserUsername;
    AUTHSONCLOSEC *oncloseconnection;	/*optional */
    AUTHSCONNLASTHEADER *authConnLastHeader;
    AUTHSDECODE *decodeauth;
    AUTHSDIRECTION *getdirection;
    AUTHSPARSE *parse;
    AUTHSCHECKCONFIG *checkconfig;
    AUTHSINIT *init;
    AUTHSREQFREE *requestFree;
    AUTHSSHUTDOWN *donefunc;
    AUTHSSTART *authStart;
    AUTHSSTATS *authStats;
};

/*
 * This is a configured auth scheme
 */

/* private data types */
struct _authScheme {
    /* pointer to the authscheme_list's string entry */
    const char *typestr;
    /* the scheme id in the authscheme_list */
    int Id;
    /* the scheme's configuration details. */
    void *scheme_data;
};

struct _acl_deny_info_list {
    int err_page_id;
    char *err_page_name;
    acl_name_list *acl_list;
    acl_deny_info_list *next;
};

#if USE_ARP_ACL

struct _acl_arp_data {
    char eth[6];
};

#endif

struct _String {
    /* never reference these directly! */
    unsigned short int size;	/* buffer size; 64K limit */
    unsigned short int len;	/* current length  */
    char *buf;
};

struct _header_mangler {
    acl_access *access_list;
    char *replacement;
    /* What follows is only used by HDR_OTHER to build a list of named headers */
    char *name;
    header_mangler *next;
};

struct _body_size {
    dlink_node node;
    acl_access *access_list;
    squid_off_t maxsize;
};

struct _http_version_t {
    unsigned int major;
    unsigned int minor;
};

#if SQUID_SNMP

struct _snmp_request_t {
    u_char *buf;
    u_char *outbuf;
    int len;
    int sock;
    long reqid;
    int outlen;
    struct sockaddr_in from;
    struct snmp_pdu *PDU;
    aclCheck_t *acl_checklist;
    u_char *community;
    struct snmp_session session;
};

#endif

struct _acl {
    char name[ACL_NAME_SZ];
    squid_acl type;
    void *data;
    char *cfgline;
    acl *next;
};

struct _acl_list {
    int op;
    acl *acl;
    acl_list *next;
};

struct _acl_access {
    int allow;
    acl_list *acl_list;
    char *cfgline;
    acl_access *next;
};

struct _acl_address {
    acl_address *next;
    acl_list *acl_list;
    struct in_addr addr;
};

struct _acl_tos {
    acl_tos *next;
    acl_list *acl_list;
    int tos;
};

struct _aclCheck_t {
    const acl_access *access_list;
    struct in_addr src_addr;
    struct in_addr dst_addr;
    struct in_addr my_addr;
    unsigned short my_port;
    request_t *request;
    /* for acls that look at reply data */
    HttpReply *reply;
    ConnStateData *conn;	/* hack for ident and NTLM */
    char rfc931[USER_IDENT_SZ];
    auth_user_request_t *auth_user_request;
    acl_lookup_state state[ACL_ENUM_MAX];
#if SQUID_SNMP
    char *snmp_community;
#endif
    PF *callback;
    void *callback_data;
    external_acl_entry *extacl_entry;
    acl *current_acl;		/* private, used by aclCheck */
};

struct _wordlist {
    char *key;
    wordlist *next;
};

struct _intlist {
    int i;
    intlist *next;
};

struct _intrange {
    int i;
    int j;
    intrange *next;
};

struct _ushortlist {
    u_short i;
    ushortlist *next;
};

struct _relist {
    char *pattern;
    regex_t regex;
    relist *next;
};

struct _acl_request_type {
    unsigned int accelerated:1;
    unsigned int transparent:1;
    unsigned int internal:1;
};

struct _sockaddr_in_list {
    struct sockaddr_in s;
    sockaddr_in_list *next;
};

struct _http_port_list {
    http_port_list *next;
    struct sockaddr_in s;
    char *protocol;		/* protocol name */
    char *name;			/* visible name */
    char *defaultsite;		/* default web site */
    char *urlgroup;		/* default urlgroup */
    unsigned int transparent;	/* transparent proxy */
    unsigned int accel;		/* HTTP accelerator */
    unsigned int vhost;		/* uses host header */
    unsigned int vport;		/* virtual port support */
    unsigned int no_connection_auth;	/* Don't support connection oriented auth */
#if LINUX_TPROXY
    unsigned int tproxy;
#endif
};

#if USE_SSL
struct _https_port_list {
    http_port_list http;	/* must be first */
    char *cert;
    char *key;
    int version;
    char *cipher;
    char *options;
    char *clientca;
    char *cafile;
    char *capath;
    char *crlfile;
    char *dhfile;
    char *sslflags;
    char *sslcontext;
    SSL_CTX *sslContext;
};

#endif

#if DELAY_POOLS
struct _delaySpec {
    int restore_bps;
    int max_bytes;
};

/* malloc()'d only as far as used (class * sizeof(delaySpec)!
 * order of elements very important!
 */
struct _delaySpecSet {
    delaySpec aggregate;
    delaySpec individual;
    delaySpec network;
};

struct _delayConfig {
    unsigned short pools;
    unsigned short initial;
    unsigned char *class;
    delaySpecSet **rates;
    acl_access **access;
};

#endif

struct _RemovalPolicySettings {
    char *type;
    wordlist *args;
};

struct _SquidConfig {
    struct {
	squid_off_t maxSize;
	int highWaterMark;
	int lowWaterMark;
    } Swap;
    squid_off_t memMaxSize;
    struct {
	char *relayHost;
	u_short relayPort;
	peer *peer;
    } Wais;
    struct {
	squid_off_t min;
	int pct;
	squid_off_t max;
    } quickAbort;
    squid_off_t readAheadGap;
    RemovalPolicySettings *replPolicy;
    RemovalPolicySettings *memPolicy;
    time_t negativeTtl;
    time_t negativeDnsTtl;
    time_t positiveDnsTtl;
    time_t shutdownLifetime;
    struct {
	time_t read;
	time_t lifetime;
	time_t forward;
	time_t connect;
	time_t peer_connect;
	time_t request;
	time_t persistent_request;
	time_t pconn;
	time_t siteSelect;
	time_t deadPeer;
	int icp_query;		/* msec */
	int icp_query_max;	/* msec */
	int mcast_icp_query;	/* msec */
#if USE_IDENT
	time_t ident;
#endif
#if !USE_DNSSERVERS
	time_t idns_retransmit;
	time_t idns_query;
#endif
    } Timeout;
    squid_off_t maxRequestHeaderSize;
    squid_off_t maxRequestBodySize;
    squid_off_t maxReplyHeaderSize;
    dlink_list ReplyBodySize;
    struct {
	u_short icp;
#if USE_HTCP
	u_short htcp;
#endif
#if SQUID_SNMP
	u_short snmp;
#endif
    } Port;
    struct {
	http_port_list *http;
#if USE_SSL
	https_port_list *https;
#endif
    } Sockaddr;
#if SQUID_SNMP
    struct {
	char *configFile;
	char *agentInfo;
    } Snmp;
#endif
#if USE_WCCP
    struct {
	struct in_addr router;
	struct in_addr address;
	int version;
    } Wccp;
#endif
#if USE_WCCPv2
    struct {
	sockaddr_in_list *router;
	struct in_addr address;
	int forwarding_method;
	int return_method;
	int assignment_method;
	int weight;
	int rebuildwait;
	void *info;
    } Wccp2;
#endif
    char *as_whois_server;
    struct {
	char *log;
	char *store;
	char *swap;
#if USE_USERAGENT_LOG
	char *useragent;
#endif
#if USE_REFERER_LOG
	char *referer;
#endif
#if WIP_FWD_LOG
	char *forward;
#endif
	logformat *logformats;
	customlog *accesslogs;
	int rotateNumber;
    } Log;
    char *adminEmail;
    char *EmailFrom;
    char *EmailProgram;
    char *effectiveUser;
    char *visible_appname_string;
    char *effectiveGroup;
    struct {
#if USE_DNSSERVERS
	char *dnsserver;
#endif
	struct {
	    wordlist *command;
	    int children;
	    int concurrency;
	} url_rewrite;
	struct {
	    wordlist *command;
	    int children;
	    int concurrency;
	} location_rewrite;
#if USE_ICMP
	char *pinger;
#endif
#if USE_UNLINKD
	char *unlinkd;
#endif
	char *diskd;
#if USE_SSL
	char *ssl_password;
#endif
    } Program;
#if USE_DNSSERVERS
    int dnsChildren;
#endif
    time_t authenticateGCInterval;
    time_t authenticateTTL;
    time_t authenticateIpTTL;
    char *appendDomain;
    int appendDomainLen;
    char *debugOptions;
    char *pidFilename;
    char *mimeTablePathname;
    char *etcHostsPath;
    char *visibleHostname;
    char *uniqueHostname;
    wordlist *hostnameAliases;
    char *errHtmlText;
    struct {
	char *host;
	char *file;
	time_t period;
	u_short port;
    } Announce;
    struct {
	struct in_addr udp_incoming;
	struct in_addr udp_outgoing;
#if SQUID_SNMP
	struct in_addr snmp_incoming;
	struct in_addr snmp_outgoing;
#endif
	struct in_addr client_netmask;
    } Addrs;
    squid_off_t tcpRcvBufsz;
    squid_off_t udpMaxHitObjsz;
    wordlist *hierarchy_stoplist;
    wordlist *mcast_group_list;
    wordlist *dns_testname_list;
    wordlist *dns_nameservers;
    peer *peers;
    int npeers;
    struct {
	int size;
	int low;
	int high;
    } ipcache;
    struct {
	int size;
    } fqdncache;
    int minDirectHops;
    int minDirectRtt;
    cachemgr_passwd *passwd_list;
    struct {
	int objectsPerBucket;
	squid_off_t avgObjectSize;
	squid_off_t maxObjectSize;
	squid_off_t minObjectSize;
	squid_off_t maxInMemObjSize;
    } Store;
    struct {
	int high;
	int low;
	time_t period;
    } Netdb;
    struct {
	int log_udp;
	int res_defnames;
	int anonymizer;
	int client_db;
	int query_icmp;
	int icp_hit_stale;
	int buffered_logs;
#if ALLOW_SOURCE_PING
	int source_ping;
#endif
	int common_log;
	int log_mime_hdrs;
	int log_fqdn;
	int announce;
	int mem_pools;
	int test_reachability;
	int half_closed_clients;
#if HTTP_VIOLATIONS
	int reload_into_ims;
#endif
	int offline;
	int redir_rewrites_host;
	int prefer_direct;
	int nonhierarchical_direct;
	int strip_query_terms;
	int redirector_bypass;
	int ignore_unknown_nameservers;
	int client_pconns;
	int server_pconns;
	int error_pconns;
#if USE_CACHE_DIGESTS
	int digest_generation;
#endif
	int log_ip_on_direct;
	int ie_refresh;
	int vary_ignore_expire;
	int pipeline_prefetch;
	int request_entities;
	int detect_broken_server_pconns;
	int balance_on_multiple_ip;
	int collapsed_forwarding;
	int relaxed_header_parser;
	int accel_no_pmtu_disc;
	int global_internal_static;
	int httpd_suppress_version_string;
	int via;
	int check_hostnames;
	int allow_underscore;
	int cache_vary;
#if FOLLOW_X_FORWARDED_FOR
	int acl_uses_indirect_client;
	int delay_pool_uses_indirect_client;
	int log_uses_indirect_client;
#endif
    } onoff;
    acl *aclList;
    struct {
	acl_access *http;
	acl_access *http2;
	acl_access *icp;
	acl_access *miss;
	acl_access *NeverDirect;
	acl_access *AlwaysDirect;
	acl_access *ASlists;
	acl_access *noCache;
	acl_access *log;
#if SQUID_SNMP
	acl_access *snmp;
#endif
	acl_access *brokenPosts;
#if USE_IDENT
	acl_access *identLookup;
#endif
	acl_access *url_rewrite;
	acl_access *location_rewrite;
	acl_access *reply;
	acl_address *outgoing_address;
	acl_tos *outgoing_tos;
#if USE_HTCP
	acl_access *htcp;
	acl_access *htcp_clr;
#endif
#if FOLLOW_X_FORWARDED_FOR
	acl_access *followXFF;
#endif
	acl_access *vary_encoding;
    } accessList;
    acl_deny_info_list *denyInfoList;
    struct _authConfig {
	authScheme *schemes;
	int n_allocated;
	int n_configured;
    } authConfig;
    struct {
	int list_width;
	int list_wrap;
	char *anon_user;
	int passive;
	int sanitycheck;
	int telnet;
    } Ftp;
    refresh_t *Refresh;
    struct _cacheSwap {
	SwapDir *swapDirs;
	int n_allocated;
	int n_configured;
    } cacheSwap;
    struct {
	char *directory;
	int use_short_names;
    } icons;
    char *errorDirectory;
    struct {
	int maxtries;
	int onerror;
    } retry;
    struct {
	squid_off_t limit;
    } MemPools;
#if DELAY_POOLS
    delayConfig Delay;
#endif
    struct {
	int icp_average;
	int dns_average;
	int http_average;
	int icp_min_poll;
	int dns_min_poll;
	int http_min_poll;
    } comm_incoming;
    int max_open_disk_fds;
    int uri_whitespace;
    squid_off_t rangeOffsetLimit;
#if MULTICAST_MISS_STREAM
    struct {
	struct in_addr addr;
	int ttl;
	unsigned short port;
	char *encode_key;
    } mcast_miss;
#endif
    header_mangler header_access[HDR_ENUM_END];
    char *coredump_dir;
    char *chroot_dir;
#if USE_CACHE_DIGESTS
    struct {
	int bits_per_entry;
	time_t rebuild_period;
	time_t rewrite_period;
	squid_off_t swapout_chunk_size;
	int rebuild_chunk_percentage;
    } digest;
#endif
#if USE_SSL
    struct {
	int unclean_shutdown;
	char *ssl_engine;
    } SSL;
#endif
    struct {
	int high_rptm;
	int high_pf;
	squid_off_t high_memory;
    } warnings;
    char *store_dir_select_algorithm;
    int sleep_after_fork;	/* microseconds */
    time_t minimum_expiry_time;	/* seconds */
    external_acl *externalAclHelperList;
    errormap *errorMapList;
#if USE_SSL
    struct {
	char *cert;
	char *key;
	int version;
	char *options;
	char *cipher;
	char *cafile;
	char *capath;
	char *crlfile;
	char *flags;
	SSL_CTX *sslContext;
    } ssl_client;
#endif
    time_t refresh_stale_window;
    int umask;
};

struct _SquidConfig2 {
    struct {
	int enable_purge;
    } onoff;
    uid_t effectiveUserID;
    gid_t effectiveGroupID;
};

struct _close_handler {
    PF *handler;
    void *data;
    close_handler *next;
};

struct _dread_ctrl {
    int fd;
    off_t file_offset;
    size_t req_len;
    char *buf;
    int end_of_file;
    DRCB *handler;
    void *client_data;
};

struct _dwrite_q {
    off_t file_offset;
    char *buf;
    size_t len;
    size_t buf_offset;
    dwrite_q *next;
    FREE *free_func;
};

struct _CommWriteStateData {
    int valid;
    char *buf;
    size_t size;
    size_t offset;
    CWCB *handler;
    void *handler_data;
    FREE *free_func;
};


/* ETag support is rudimantal;
 * this struct is likely to change
 * Note: "str" points to memory in HttpHeaderEntry (for now)
 *       so ETags should be used as tmp variables only (for now) */
struct _ETag {
    const char *str;		/* quoted-string */
    int weak;			/* true if it is a weak validator */
};

struct _fde {
    unsigned int type;
    u_short local_port;
    u_short remote_port;
    struct in_addr local_addr;
    unsigned char tos;
    char ipaddr[16];		/* dotted decimal address of peer */
    char desc[FD_DESC_SZ];
    struct {
	unsigned int open:1;
	unsigned int close_request:1;
	unsigned int write_daemon:1;
	unsigned int closing:1;
	unsigned int socket_eof:1;
	unsigned int nolinger:1;
	unsigned int nonblocking:1;
	unsigned int ipc:1;
	unsigned int called_connect:1;
	unsigned int nodelay:1;
	unsigned int close_on_exec:1;
	unsigned int backoff:1;	/* keep track of whether the fd is backed off */
    } flags;
    comm_pending read_pending;
    comm_pending write_pending;
    squid_off_t bytes_read;
    squid_off_t bytes_written;
    int uses;			/* ie # req's over persistent conn */
    struct _fde_disk {
	DWCB *wrt_handle;
	void *wrt_handle_data;
	dwrite_q *write_q;
	dwrite_q *write_q_tail;
	off_t offset;
    } disk;
    PF *read_handler;
    void *read_data;
    PF *write_handler;
    void *write_data;
    PF *timeout_handler;
    time_t timeout;
    void *timeout_data;
    void *lifetime_data;
    close_handler *close_handler;	/* linked list */
    DEFER *defer_check;		/* check if we should defer read */
    void *defer_data;
    struct _CommWriteStateData rwstate;		/* State data for comm_write */
    READ_HANDLER *read_method;
    WRITE_HANDLER *write_method;
#if USE_SSL
    SSL *ssl;
#endif
#ifdef _SQUID_MSWIN_
    struct {
	long handle;
    } win32;
#endif
#if DELAY_POOLS
    int slow_id;
#endif
};

struct _fileMap {
    int max_n_files;
    int n_files_in_map;
    int toggle;
    int nwords;
    unsigned long *file_map;
};

/* auto-growing memory-resident buffer with printf interface */
/* note: when updating this struct, update MemBufNULL #define */
struct _MemBuf {
    /* public, read-only */
    char *buf;
    mb_size_t size;		/* used space, does not count 0-terminator */

    /* private, stay away; use interface function instead */
    mb_size_t max_capacity;	/* when grows: assert(new_capacity <= max_capacity) */
    mb_size_t capacity;		/* allocated space */
    unsigned stolen:1;		/* the buffer has been stolen for use by someone else */
};

/* see Packer.c for description */
struct _Packer {
    /* protected, use interface functions instead */
    append_f append;
    vprintf_f packer_vprintf;
    void *real_handle;		/* first parameter to real append and vprintf */
};

/* http status line */
struct _HttpStatusLine {
    /* public, read only */
    http_version_t version;
    const char *reason;		/* points to a _constant_ string (default or supplied), never free()d */
    http_status status;
};

/*
 * Note: HttpBody is used only for messages with a small content that is
 * known a priory (e.g., error messages).
 */
struct _HttpBody {
    /* private */
    MemBuf mb;
};

/* http header extention field */
struct _HttpHdrExtField {
    String name;		/* field-name  from HTTP/1.1 (no column after name) */
    String value;		/* field-value from HTTP/1.1 */
};

/* http cache control header field */
struct _HttpHdrCc {
    int mask;
    int max_age;
    int s_maxage;
    int max_stale;
    String other;
};

/* http byte-range-spec */
struct _HttpHdrRangeSpec {
    squid_off_t offset;
    squid_off_t length;
};

/* There may be more than one byte range specified in the request.
 * This object holds all range specs in order of their appearence
 * in the request because we SHOULD preserve that order.
 */
struct _HttpHdrRange {
    Stack specs;
};

/* http content-range header field */
struct _HttpHdrContRange {
    HttpHdrRangeSpec spec;
    squid_off_t elength;	/* entity length, not content length */
};

/* some fields can hold either time or etag specs (e.g. If-Range) */
struct _TimeOrTag {
    const char *tag;		/* entity tag */
    time_t time;
    int valid;			/* true if struct is usable */
};

/* data for iterating thru range specs */
struct _HttpHdrRangeIter {
    HttpHdrRangePos pos;
    const HttpHdrRangeSpec *spec;	/* current spec at pos */
    squid_off_t debt_size;	/* bytes left to send from the current spec */
    squid_off_t prefix_size;	/* the size of the incoming HTTP msg prefix */
    String boundary;		/* boundary for multipart responses */
};

/* constant attributes of http header fields */
struct _HttpHeaderFieldAttrs {
    const char *name;
    http_hdr_type id;
    field_type type;
};

/* per field statistics */
struct _HttpHeaderFieldStat {
    int aliveCount;		/* created but not destroyed (count) */
    int seenCount;		/* #fields we've seen */
    int parsCount;		/* #parsing attempts */
    int errCount;		/* #pasring errors */
    int repCount;		/* #repetitons */
};

/* compiled version of HttpHeaderFieldAttrs plus stats */
struct _HttpHeaderFieldInfo {
    http_hdr_type id;
    String name;
    field_type type;
    HttpHeaderFieldStat stat;
};

struct _HttpHeaderEntry {
    http_hdr_type id;
    String name;
    String value;
};

struct _HttpHeader {
    /* protected, do not use these, use interface functions instead */
    Array entries;		/* parsed fields in raw format */
    HttpHeaderMask mask;	/* bit set <=> entry present */
    http_hdr_owner_type owner;	/* request or reply */
    int len;			/* length when packed, not counting terminating '\0' */
};

struct _HttpReply {
    /* unsupported, writable, may disappear/change in the future */
    int hdr_sz;			/* sums _stored_ status-line, headers, and <CRLF> */

    /* public, readable; never update these or their .hdr equivalents directly */
    squid_off_t content_length;
    time_t date;
    time_t last_modified;
    time_t expires;
    String content_type;
    HttpHdrCc *cache_control;
    HttpHdrContRange *content_range;
    short int keep_alive;

    /* public, readable */
    HttpMsgParseState pstate;	/* the current parsing state */

    /* public, writable, but use httpReply* interfaces when possible */
    HttpStatusLine sline;
    HttpHeader header;
    HttpBody body;		/* for small constant memory-resident text bodies only */
};

struct _http_state_flags {
    unsigned int proxying:1;
    unsigned int keepalive:1;
    unsigned int only_if_cached:1;
    unsigned int keepalive_broken:1;
    unsigned int abuse_detected:1;
    unsigned int request_sent:1;
    unsigned int front_end_https:2;
    unsigned int originpeer:1;
};

struct _HttpStateData {
    StoreEntry *entry;
    request_t *request;
    MemBuf reply_hdr;
    int reply_hdr_state;
    peer *peer;			/* peer request made to */
    int eof;			/* reached end-of-object? */
    request_t *orig_request;
    int fd;
    http_state_flags flags;
    FwdState *fwd;
    char *body_buf;
    int body_buf_sz;
};

struct _icpUdpData {
    struct sockaddr_in address;
    void *msg;
    size_t len;
    icpUdpData *next;
#ifndef LESS_TIMING
    struct timeval start;
#endif
    log_type logcode;
    struct timeval queue_time;
};

struct _ping_data {
    struct timeval start;
    struct timeval stop;
    int n_sent;
    int n_recv;
    int n_replies_expected;
    int timeout;		/* msec */
    int timedout;
    int w_rtt;
    int p_rtt;
};

struct _HierarchyLogEntry {
    hier_code code;
    char host[SQUIDHOSTNAMELEN];
    ping_data ping;
    char cd_host[SQUIDHOSTNAMELEN];	/* the host of selected by cd peer */
    lookup_t cd_lookup;		/* cd prediction: none, miss, hit */
    int n_choices;		/* #peers we selected from (cd only) */
    int n_ichoices;		/* #peers with known rtt we selected from (cd only) */
    struct timeval peer_select_start;
    struct timeval store_complete_stop;
};

struct _AccessLogEntry {
    const char *url;
    struct {
	method_t method;
	int code;
	const char *content_type;
	http_version_t version;
    } http;
    struct {
	icp_opcode opcode;
    } icp;
    struct {
	struct in_addr caddr;
	squid_off_t size;
	log_type code;
	int msec;
	const char *rfc931;
	const char *authuser;
#if USE_SSL
	const char *ssluser;
#endif
    } cache;
    struct {
	char *request;
	char *reply;
    } headers;
    struct {
	const char *method_str;
    } private;
    HierarchyLogEntry hier;
    HttpReply *reply;
    request_t *request;
};

struct _clientHttpRequest {
    ConnStateData *conn;
    dlink_node node;
    request_t *request;		/* Parsed URL ... */
    request_t *orig_request;	/* Parsed URL ... */
    store_client *sc;		/* The store_client we're using */
    store_client *old_sc;	/* ... for entry to be validated */
    char *uri;
    char *log_uri;
    struct {
	squid_off_t offset;
	squid_off_t size;
    } out;
    HttpReply *reply;		/* it is important for clientHttpRequest
				 * to have its own HttpReply for
				 * logging, especially in cases
				 * where the reply headers sent to
				 * the client are different than
				 * those received from the origin server. */
    HttpHdrRangeIter range_iter;	/* data for iterating thru range specs */
    size_t req_sz;		/* raw request size on input, not current request size */
    StoreEntry *entry;
    StoreEntry *old_entry;
    log_type log_type;
#if USE_CACHE_DIGESTS
    const char *lookup_type;	/* temporary hack: storeGet() result: HIT/MISS/NONE */
#endif
    struct timeval start;
    http_version_t http_ver;
    int redirect_state;
    aclCheck_t *acl_checklist;	/* need ptr back so we can unreg if needed */
    AccessLogEntry al;
    struct {
	unsigned int accel:1;
	unsigned int transparent:1;
	unsigned int internal:1;
	unsigned int done_copying:1;
	unsigned int purging:1;
	unsigned int hit:1;
    } flags;
    struct {
	http_status status;
	char *location;
    } redirect;
    dlink_node active;
    squid_off_t maxBodySize;
    /*
     * This can be changed into a pointer later on when the memory allocator
     * has been taught the difference between buffers to zero and buffers not to
     * zero.. [ahc]
     */
    char readbuf[CLIENT_SOCK_SZ];
};

struct _ConnStateData {
    int fd;
    struct {
	char *buf;
	size_t offset;
	size_t size;
    } in;
    struct {
	squid_off_t size_left;	/* How much body left to process */
	request_t *request;	/* Parameters passed to clientReadBody */
	char *buf;
	size_t bufsize;
	CBCB *callback;
	void *cbdata;
    } body;
    auth_type_t auth_type;	/* Is this connection based authentication ? if so 
				 * what type it is. */
    /* note this is ONLY connection based because NTLM is against HTTP spec */
    /* the user details for connection based authentication */
    auth_user_request_t *auth_user_request;
    dlink_list reqs;
    struct sockaddr_in peer;
    struct sockaddr_in me;
    struct in_addr log_addr;
    char rfc931[USER_IDENT_SZ];
    int nrequests;
    struct {
	int n;
	time_t until;
    } defer;
    http_port_list *port;
    int transparent;
    struct {
	int fd;			/* pinned server side connection */
	char *host;		/* host name of pinned connection */
	int port;		/* port of pinned connection */
	int pinned;		/* this connection was pinned */
	int auth;		/* pinned for www authentication */
	peer *peer;		/* peer the connection goes via */
    } pinning;
};

struct _ipcache_addrs {
    struct in_addr *in_addrs;
    unsigned char *bad_mask;
    unsigned char count;
    unsigned char cur;
    unsigned char badcount;
};

struct _domain_ping {
    char *domain;
    int do_ping;		/* boolean */
    domain_ping *next;
};

struct _domain_type {
    char *domain;
    peer_t type;
    domain_type *next;
};

#if USE_CACHE_DIGESTS
struct _Version {
    short int current;		/* current version */
    short int required;		/* minimal version that can safely handle current version */
};

/* digest control block; used for transmission and storage */
struct _StoreDigestCBlock {
    Version ver;
    int capacity;
    int count;
    int del_count;
    int mask_size;
    unsigned char bits_per_entry;
    unsigned char hash_func_count;
    short int reserved_short;
    int reserved[32 - 6];
};

struct _DigestFetchState {
    PeerDigest *pd;
    StoreEntry *entry;
    StoreEntry *old_entry;
    store_client *sc;
    store_client *old_sc;
    request_t *request;
    squid_off_t offset;
    squid_off_t mask_offset;
    time_t start_time;
    time_t resp_time;
    time_t expires;
    struct {
	int msg;
	int bytes;
    } sent, recv;
};

/* statistics for cache digests and other hit "predictors" */
struct _cd_guess_stats {
    /* public, read-only */
    int true_hits;
    int false_hits;
    int true_misses;
    int false_misses;
    int close_hits;		/* tmp, remove it later */
};

struct _PeerDigest {
    peer *peer;			/* pointer back to peer structure, argh */
    CacheDigest *cd;		/* actual digest structure */
    String host;		/* copy of peer->host */
    const char *req_result;	/* text status of the last request */
    struct {
	unsigned int needed:1;	/* there were requests for this digest */
	unsigned int usable:1;	/* can be used for lookups */
	unsigned int requested:1;	/* in process of receiving [fresh] digest */
    } flags;
    struct {
	/* all times are absolute unless augmented with _delay */
	time_t initialized;	/* creation */
	time_t needed;		/* first lookup/use by a peer */
	time_t next_check;	/* next scheduled check/refresh event */
	time_t retry_delay;	/* delay before re-checking _invalid_ digest */
	time_t requested;	/* requested a fresh copy of a digest */
	time_t req_delay;	/* last request response time */
	time_t received;	/* received the current copy of a digest */
	time_t disabled;	/* disabled for good */
    } times;
    struct {
	cd_guess_stats guess;
	int used_count;
	struct {
	    int msgs;
	    kb_t kbytes;
	} sent, recv;
    } stats;
};

#endif

struct _peer {
    char *name;
    char *host;
    peer_t type;
    struct sockaddr_in in_addr;
    struct {
	int pings_sent;
	int pings_acked;
	int fetches;
	int rtt;
	int ignored_replies;
	int n_keepalives_sent;
	int n_keepalives_recv;
	time_t probe_start;
	time_t last_query;
	time_t last_reply;
	time_t last_connect_failure;
	time_t last_connect_probe;
	int logged_state;	/* so we can print dead/revived msgs */
	int conn_open;		/* current opened connections */
    } stats;
    struct {
	int version;
	int counts[ICP_END];
	u_short port;
    } icp;
#if USE_HTCP
    struct {
	double version;
	int counts[2];
	u_short port;
    } htcp;
#endif
    u_short http_port;
    domain_ping *peer_domain;
    domain_type *typelist;
    acl_access *access;
    struct {
	unsigned int proxy_only:1;
	unsigned int no_query:1;
	unsigned int no_digest:1;
	unsigned int default_parent:1;
	unsigned int roundrobin:1;
	unsigned int mcast_responder:1;
	unsigned int closest_only:1;
#if USE_HTCP
	unsigned int htcp:1;
	unsigned int htcp_oldsquid:1;
#endif
	unsigned int no_netdb_exchange:1;
#if DELAY_POOLS
	unsigned int no_delay:1;
#endif
	unsigned int allow_miss:1;
	unsigned int originserver:1;
	unsigned int userhash:1;
	unsigned int sourcehash:1;
#if USE_CARP
	unsigned int carp:1;
#endif
    } options;
    int weight;
    struct {
	double avg_n_members;
	int n_times_counted;
	int n_replies_expected;
	int ttl;
	int id;
	struct {
	    unsigned int count_event_pending:1;
	    unsigned int counting:1;
	} flags;
    } mcast;
#if USE_CACHE_DIGESTS
    PeerDigest *digest;
    char *digest_url;
#endif
    int tcp_up;			/* 0 if a connect() fails */
    struct in_addr addresses[10];
    int n_addresses;
    int rr_count;
    int rr_lastcount;
    peer *next;
    int test_fd;
#if USE_CARP
    struct {
	unsigned int hash;
	double load_multiplier;
	double load_factor;	/* normalized weight value */
    } carp;
#endif
    struct {
	unsigned int hash;
	double load_multiplier;
	double load_factor;
    } userhash;
    struct {
	unsigned int hash;
	double load_multiplier;
	double load_factor;
    } sourcehash;
    char *login;		/* Proxy authorization */
    time_t connect_timeout;
    int max_conn;
    struct {
	char *url;
	int min, max;
	int interval;
	int timeout;
	int state;
	time_t last;
	PeerMonitor *data;
    } monitor;
    char *domain;		/* Forced domain */
#if USE_SSL
    int use_ssl;
    char *sslcert;
    char *sslkey;
    int sslversion;
    char *ssloptions;
    char *sslcipher;
    char *sslcafile;
    char *sslcapath;
    char *sslcrlfile;
    char *sslflags;
    char *ssldomain;
    SSL_CTX *sslContext;
    SSL_SESSION *sslSession;
#endif
    int front_end_https;
    int connection_auth;
};

struct _net_db_name {
    hash_link hash;		/* must be first */
    net_db_name *next;
    netdbEntry *net_db_entry;
};

struct _net_db_peer {
    const char *peername;
    double hops;
    double rtt;
    time_t expires;
};

struct _netdbEntry {
    hash_link hash;		/* must be first */
    char network[16];
    int pings_sent;
    int pings_recv;
    double hops;
    double rtt;
    time_t next_ping_time;
    time_t last_use_time;
    int link_count;
    net_db_name *hosts;
    net_db_peer *peers;
    int n_peers_alloc;
    int n_peers;
};

struct _ps_state {
    request_t *request;
    StoreEntry *entry;
    int always_direct;
    int never_direct;
    int direct;
    PSC *callback;
    void *callback_data;
    FwdServer *servers;
    /*
     * Why are these struct sockaddr_in instead of peer *?  Because a
     * peer structure can become invalid during the peer selection
     * phase, specifically after a reconfigure.  Thus we need to lookup
     * the peer * based on the address when we are finally ready to
     * reference the peer structure.
     */
    struct sockaddr_in first_parent_miss;
    struct sockaddr_in closest_parent_miss;
    /*
     * ->hit and ->secho can be peer* because they should only be
     * accessed during the thread when they are set
     */
    peer *hit;
    peer_t hit_type;
#if ALLOW_SOURCE_PING
    peer *secho;
#endif
    ping_data ping;
    aclCheck_t *acl_checklist;
};

#if USE_ICMP
struct _pingerEchoData {
    struct in_addr to;
    unsigned char opcode;
    int psize;
    char payload[PINGER_PAYLOAD_SZ];
};

struct _pingerReplyData {
    struct in_addr from;
    unsigned char opcode;
    int rtt;
    int hops;
    int psize;
    char payload[PINGER_PAYLOAD_SZ];
};

#endif

struct _icp_common_t {
    unsigned char opcode;	/* opcode */
    unsigned char version;	/* version number */
    unsigned short length;	/* total length (bytes) */
    u_num32 reqnum;		/* req number (req'd for UDP) */
    u_num32 flags;
    u_num32 pad;
    u_num32 shostid;		/* sender host id */
};

struct _iostats {
    struct {
	int reads;
	int reads_deferred;
	int read_hist[16];
	int writes;
	int write_hist[16];
    } Http, Ftp, Gopher, Wais;
};

struct _mem_node {
    char data[SM_PAGE_SIZE];
    int len;
    int uses;
    mem_node *next;
};

struct _mem_hdr {
    mem_node *head;
    mem_node *tail;
    squid_off_t origin_offset;
};

/* keep track each client receiving data from that particular StoreEntry */
struct _store_client {
    int type;
    squid_off_t copy_offset;
    squid_off_t seen_offset;
    size_t copy_size;
    char *copy_buf;
    STCB *callback;
    void *callback_data;
    StoreEntry *entry;		/* ptr to the parent StoreEntry, argh! */
    storeIOState *swapin_sio;
    struct {
	unsigned int disk_io_pending:1;
	unsigned int store_copying:1;
	unsigned int copy_event_pending:1;
    } flags;
#if DELAY_POOLS
    delay_id delay_id;
#endif
    dlink_node node;
#if STORE_CLIENT_LIST_DEBUG
    void *owner;
#endif
};


/* Removal policies */

struct _RemovalPolicyNode {
    void *data;
};

struct _RemovalPolicy {
    const char *_type;
    void *_data;
    void (*Free) (RemovalPolicy * policy);
    void (*Add) (RemovalPolicy * policy, StoreEntry * entry, RemovalPolicyNode * node);
    void (*Remove) (RemovalPolicy * policy, StoreEntry * entry, RemovalPolicyNode * node);
    void (*Referenced) (RemovalPolicy * policy, const StoreEntry * entry, RemovalPolicyNode * node);
    void (*Dereferenced) (RemovalPolicy * policy, const StoreEntry * entry, RemovalPolicyNode * node);
    RemovalPolicyWalker *(*WalkInit) (RemovalPolicy * policy);
    RemovalPurgeWalker *(*PurgeInit) (RemovalPolicy * policy, int max_scan);
    void (*Stats) (RemovalPolicy * policy, StoreEntry * entry);
};

struct _RemovalPolicyWalker {
    RemovalPolicy *_policy;
    void *_data;
    const StoreEntry *(*Next) (RemovalPolicyWalker * walker);
    void (*Done) (RemovalPolicyWalker * walker);
};

struct _RemovalPurgeWalker {
    RemovalPolicy *_policy;
    void *_data;
    int scanned, max_scan, locked;
    StoreEntry *(*Next) (RemovalPurgeWalker * walker);
    void (*Done) (RemovalPurgeWalker * walker);
};

/* This structure can be freed while object is purged out from memory */
struct _MemObject {
    method_t method;
    char *url;
    mem_hdr data_hdr;
    squid_off_t inmem_hi;
    squid_off_t inmem_lo;
    int serverfd;		/* Record the server's fd if we have too much
				 * data waiting to send to the client */
    dlink_list clients;
    int nclients;
    struct {
	squid_off_t queue_offset;	/* relative to in-mem data */
	mem_node *memnode;	/* which node we're currently paging out */
	storeIOState *sio;
    } swapout;
    HttpReply *reply;
    request_t *request;
    struct timeval start_ping;
    IRCB *ping_reply_callback;
    void *ircb_data;
    struct {
	STABH *callback;
	void *data;
    } abort;
    char *log_url;
    RemovalPolicyNode repl;
    int id;
    squid_off_t object_sz;
    size_t swap_hdr_sz;
#if URL_CHECKSUM_DEBUG
    unsigned int chksum;
#endif
    const char *vary_hdr;
    const char *vary_headers;
    const char *vary_encoding;
    StoreEntry *ims_entry;
    time_t refresh_timestamp;
};

struct _StoreEntry {
    hash_link hash;		/* must be first */
    MemObject *mem_obj;
    RemovalPolicyNode repl;
    /* START OF ON-DISK STORE_META_STD TLV field */
    time_t timestamp;
    time_t lastref;
    time_t expires;
    time_t lastmod;
    squid_file_sz swap_file_sz;
    u_short refcount;
    u_short flags;
    /* END OF ON-DISK STORE_META_STD */
    sfileno swap_filen:25;
    sdirno swap_dirn:7;
    u_short lock_count;		/* Assume < 65536! */
    mem_status_t mem_status:3;
    ping_status_t ping_status:3;
    store_status_t store_status:3;
    swap_status_t swap_status:3;
};

struct _SwapDir {
    const char *type;
    int cur_size;
    int low_size;
    int max_size;
    char *path;
    int index;			/* This entry's index into the swapDirs array */
    squid_off_t max_objsize;
    RemovalPolicy *repl;
    int removals;
    int scanned;
    struct {
	unsigned int selected:1;
	unsigned int read_only:1;
    } flags;
    STINIT *init;		/* Initialise the fs */
    STCHECKCONFIG *checkconfig;	/* Verify configuration */
    STNEWFS *newfs;		/* Create a new fs */
    STDUMP *dump;		/* Dump fs config snippet */
    STFREE *freefs;		/* Free the fs data */
    STDBLCHECK *dblcheck;	/* Double check the obj integrity */
    STSTATFS *statfs;		/* Dump fs statistics */
    STMAINTAINFS *maintainfs;	/* Replacement maintainence */
    STCHECKOBJ *checkobj;	/* Check if the fs will store an object */
    STCHECKLOADAV *checkload;	/* Check if the fs is getting overloaded .. */
    /* These two are notifications */
    STREFOBJ *refobj;		/* Reference this object */
    STUNREFOBJ *unrefobj;	/* Unreference this object */
    STCALLBACK *callback;	/* Handle pending callbacks */
    STSYNC *sync;		/* Sync the directory */
    struct {
	STOBJCREATE *create;
	STOBJOPEN *open;
	STOBJCLOSE *close;
	STOBJREAD *read;
	STOBJWRITE *write;
	STOBJUNLINK *unlink;
	STOBJRECYCLE *recycle;
    } obj;
    struct {
	STLOGOPEN *open;
	STLOGCLOSE *close;
	STLOGWRITE *write;
	struct {
	    STLOGCLEANSTART *start;
	    STLOGCLEANNEXTENTRY *nextentry;
	    STLOGCLEANWRITE *write;
	    STLOGCLEANDONE *done;
	    void *state;
	} clean;
	int writes_since_clean;
    } log;
    struct {
	int blksize;
    } fs;
    void *fsdata;
};

struct _request_flags {
    unsigned int range:1;
    unsigned int nocache:1;
    unsigned int ims:1;
    unsigned int auth:1;
    unsigned int cachable:1;
    unsigned int hierarchical:1;
    unsigned int loopdetect:1;
    unsigned int proxy_keepalive:1;
    unsigned int proxying:1;	/* this should be killed, also in httpstateflags */
    unsigned int refresh:1;
    unsigned int redirected:1;
    unsigned int need_validation:1;
#if HTTP_VIOLATIONS
    unsigned int nocache_hack:1;	/* for changing/ignoring no-cache requests */
#endif
    unsigned int accelerated:1;
    unsigned int transparent:1;
    unsigned int internal:1;
    unsigned int body_sent:1;
    unsigned int reset_tcp:1;
    unsigned int must_keepalive:1;
    unsigned int connection_auth:1;	/* Request wants connection oriented auth */
    unsigned int connection_proxy_auth:1;	/* Request wants connection oriented auth */
    unsigned int no_connection_auth:1;	/* Connection oriented auth can not be supported */
    unsigned int pinned:1;	/* Request seont on a pinned connection */
    unsigned int auth_sent:1;	/* Authentication forwarded */
#if LINUX_TPROXY
    unsigned int tproxy:1;
#endif
    unsigned int collapsed:1;	/* This request was collapsed. Don't trust the store entry to be valid */
};

struct _link_list {
    void *ptr;
    struct _link_list *next;
};

struct _storeIOState {
    sdirno swap_dirn;
    sfileno swap_filen;
    StoreEntry *e;		/* Need this so the FS layers can play god */
    mode_t mode;
    squid_off_t st_size;	/* do stat(2) after read open */
    squid_off_t offset;		/* current on-disk offset pointer */
    squid_off_t write_offset;	/* current storeWrite offset pointer */
    STFNCB *file_callback;	/* called on delayed sfileno assignments */
    STIOCB *callback;
    void *callback_data;
    struct {
	STRCB *callback;
	void *callback_data;
    } read;
    struct {
	unsigned int closing:1;	/* debugging aid */
    } flags;
    void *fsstate;
};

struct _request_t {
    method_t method;
    protocol_t protocol;
    char login[MAX_LOGIN_SZ];
    char host[SQUIDHOSTNAMELEN + 1];
    auth_user_request_t *auth_user_request;
    u_short port;
    String urlpath;
    char *canonical;
    int link_count;		/* free when zero */
    request_flags flags;
    HttpHdrCc *cache_control;
    HttpHdrRange *range;
    http_version_t http_ver;
    time_t ims;
    int imslen;
    int max_forwards;
    /* these in_addr's could probably be sockaddr_in's */
    in_port_t client_port;
    struct in_addr client_addr;
#if FOLLOW_X_FORWARDED_FOR
    struct in_addr indirect_client_addr;	/* after following X-Forwarded-For */
#endif				/* FOLLOW_X_FORWARDED_FOR */
    struct in_addr my_addr;
    unsigned short my_port;
    HttpHeader header;
    squid_off_t content_length;
    HierarchyLogEntry hier;
    err_type err_type;
    char *peer_login;		/* Configured peer login:password */
    time_t lastmod;		/* Used on refreshes */
    char *vary_hdr;		/* Used when varying entities are detected. Changes how the store key is calculated */
    char *vary_headers;		/* Used when varying entities are detected. Changes how the store key is calculated */
    String vary_encoding;	/* Used when varying entities are detected. Changes how the store key is calculated. */
    VaryData *vary;
    Array *etags;		/* possible known entity tags (Vary MISS) */
    char *etag;			/* current entity tag, cache validation */
    unsigned int done_etag:1;	/* We have done clientProcessETag on this, don't attempt it again */
    char *urlgroup;		/* urlgroup, returned by redirectors */
    char *peer_domain;		/* Configured peer forceddomain */
    BODY_HANDLER *body_reader;
    void *body_reader_data;
    String extacl_log;		/* String to be used for access.log purposes */
    const char *extacl_user;	/* User name returned by extacl lookup */
    const char *extacl_passwd;	/* Password returned by extacl lookup */
#if FOLLOW_X_FORWARDED_FOR
    /* XXX a list of IP addresses would be a better data structure
     * than this String */
    String x_forwarded_for_iterator;
#endif				/* FOLLOW_X_FORWARDED_FOR */
    ConnStateData *pinned_connection;	/* If set then this request is tighly tied to the corresponding client side connetion */
};

struct _cachemgr_passwd {
    char *passwd;
    wordlist *actions;
    cachemgr_passwd *next;
};

struct _refresh_t {
    const char *pattern;
    regex_t compiled_pattern;
    time_t min;
    double pct;
    time_t max;
    refresh_t *next;
    struct {
	unsigned int icase:1;
#if HTTP_VIOLATIONS
	unsigned int override_expire:1;
	unsigned int override_lastmod:1;
	unsigned int reload_into_ims:1;
	unsigned int ignore_reload:1;
	unsigned int ignore_no_cache:1;
	unsigned int ignore_private:1;
	unsigned int ignore_auth:1;
#endif
    } flags;
};

struct _ErrorState {
    err_type type;
    int page_id;
    http_status http_status;
    auth_user_request_t *auth_user_request;
    request_t *request;
    char *url;
    int xerrno;
    char *dnsserver_msg;
    time_t ttl;
    struct in_addr src_addr;
    char *redirect_url;
    ERCB *callback;
    void *callback_data;
    struct {
	unsigned int flag_cbdata:1;
    } flags;
    struct {
	wordlist *server_msg;
	char *request;
	char *reply;
    } ftp;
    char *request_hdrs;
};

/*
 * "very generic" histogram; 
 * see important comments on hbase_f restrictions in StatHist.c
 */
struct _StatHist {
    int *bins;
    int capacity;
    double min;
    double max;
    double scale;
    hbase_f *val_in;		/* e.g., log() for log-based histogram */
    hbase_f *val_out;		/* e.g., exp() for log based histogram */
};

/*
 * if you add a field to StatCounters, 
 * you MUST sync statCountersInitSpecial, statCountersClean, and statCountersCopy
 */
struct _StatCounters {
    struct {
	int clients;
	int requests;
	int hits;
	int mem_hits;
	int disk_hits;
	int errors;
	kb_t kbytes_in;
	kb_t kbytes_out;
	kb_t hit_kbytes_out;
	StatHist miss_svc_time;
	StatHist nm_svc_time;
	StatHist nh_svc_time;
	StatHist hit_svc_time;
	StatHist all_svc_time;
    } client_http;
    struct {
	struct {
	    int requests;
	    int errors;
	    kb_t kbytes_in;
	    kb_t kbytes_out;
	} all , http, ftp, other;
    } server;
    struct {
	int pkts_sent;
	int queries_sent;
	int replies_sent;
	int pkts_recv;
	int queries_recv;
	int replies_recv;
	int hits_sent;
	int hits_recv;
	int replies_queued;
	int replies_dropped;
	kb_t kbytes_sent;
	kb_t q_kbytes_sent;
	kb_t r_kbytes_sent;
	kb_t kbytes_recv;
	kb_t q_kbytes_recv;
	kb_t r_kbytes_recv;
	StatHist query_svc_time;
	StatHist reply_svc_time;
	int query_timeouts;
	int times_used;
    } icp;
    struct {
	int pkts_sent;
	int pkts_recv;
    } htcp;
    struct {
	int requests;
    } unlink;
    struct {
	StatHist svc_time;
    } dns;
    struct {
	int times_used;
	kb_t kbytes_sent;
	kb_t kbytes_recv;
	kb_t memory;
	int msgs_sent;
	int msgs_recv;
#if USE_CACHE_DIGESTS
	cd_guess_stats guess;
#endif
	StatHist on_xition_count;
    } cd;
    struct {
	int times_used;
    } netdb;
    int page_faults;
    int select_loops;
    int select_fds;
    double select_time;
    double cputime;
    struct timeval timestamp;
    StatHist comm_icp_incoming;
    StatHist comm_dns_incoming;
    StatHist comm_http_incoming;
    StatHist select_fds_hist;
    struct {
	struct {
	    int opens;
	    int closes;
	    int reads;
	    int writes;
	    int seeks;
	    int unlinks;
	} disk;
	struct {
	    int accepts;
	    int sockets;
	    int connects;
	    int binds;
	    int closes;
	    int reads;
	    int writes;
	    int recvfroms;
	    int sendtos;
	} sock;
	int polls;
	int selects;
    } syscalls;
    int aborted_requests;
    struct {
	int files_cleaned;
	int outs;
	int ins;
    } swap;
};

/* per header statistics */
struct _HttpHeaderStat {
    const char *label;
    HttpHeaderMask *owner_mask;

    StatHist hdrUCountDistr;
    StatHist fieldTypeDistr;
    StatHist ccTypeDistr;

    int parsedCount;
    int ccParsedCount;
    int destroyedCount;
    int busyDestroyedCount;
};


struct _tlv {
    char type;
    int length;
    void *value;
    tlv *next;
};

/*
 * Do we need to have the dirn in here? I don't think so, since we already
 * know the dirn .. 
 */
struct _storeSwapLogData {
    char op;
    sfileno swap_filen;
    time_t timestamp;
    time_t lastref;
    time_t expires;
    time_t lastmod;
    squid_file_sz swap_file_sz;
    u_short refcount;
    u_short flags;
    unsigned char key[MD5_DIGEST_CHARS];
};

struct _storeSwapLogHeader {
    char op;
    int version;
    int record_size;
};

#if SIZEOF_SQUID_FILE_SZ != SIZEOF_SIZE_T
struct _storeSwapLogDataOld {
    char op;
    sfileno swap_filen;
    time_t timestamp;
    time_t lastref;
    time_t expires;
    time_t lastmod;
    size_t swap_file_sz;
    u_short refcount;
    u_short flags;
    unsigned char key[MD5_DIGEST_CHARS];
};

#endif


/* object to track per-action memory usage (e.g. #idle objects) */
struct _MemMeter {
    ssize_t level;		/* current level (count or volume) */
    ssize_t hwater_level;	/* high water mark */
    time_t hwater_stamp;	/* timestamp of last high water mark change */
};

/* object to track per-pool memory usage (alloc = inuse+idle) */
struct _MemPoolMeter {
    MemMeter alloc;
    MemMeter inuse;
    MemMeter idle;
    gb_t saved;
    gb_t total;
};

/* a pool is a [growing] space for objects of the same size */
struct _MemPool {
    const char *label;
    size_t obj_size;
#if DEBUG_MEMPOOL
    size_t real_obj_size;	/* with alignment */
#endif
    Stack pstack;		/* stack for free pointers */
    MemPoolMeter meter;
#if DEBUG_MEMPOOL
    MemPoolMeter diff_meter;
#endif
};

struct _ClientInfo {
    hash_link hash;		/* must be first */
    struct in_addr addr;
    struct {
	int result_hist[LOG_TYPE_MAX];
	int n_requests;
	kb_t kbytes_in;
	kb_t kbytes_out;
	kb_t hit_kbytes_out;
    } Http, Icp;
    struct {
	time_t time;
	int n_req;
	int n_denied;
    } cutoff;
    int n_established;		/* number of current established connections */
    time_t last_seen;
};

struct _CacheDigest {
    /* public, read-only */
    char *mask;			/* bit mask */
    int mask_size;		/* mask size in bytes */
    int capacity;		/* expected maximum for .count, not a hard limit */
    int bits_per_entry;		/* number of bits allocated for each entry from capacity */
    int count;			/* number of digested entries */
    int del_count;		/* number of deletions performed so far */
};

struct _FwdServer {
    peer *peer;			/* NULL --> origin server */
    hier_code code;
    FwdServer *next;
};

struct _FwdState {
    int client_fd;		/* XXX unnecessary */
    StoreEntry *entry;
    request_t *request;
    FwdServer *servers;
    int server_fd;
    ErrorState *err;
    time_t start;
    int n_tries;
    int origin_tries;
#if WIP_FWD_LOG
    http_status last_status;
#endif
    struct {
	unsigned int dont_retry:1;
	unsigned int ftp_pasv_failed:1;
    } flags;
#if LINUX_NETFILTER
    struct sockaddr_in src;
#endif
};

#if USE_HTCP
struct _htcpReplyData {
    int hit;
    HttpHeader hdr;
    u_num32 msg_id;
    double version;
    struct {
	/* cache-to-origin */
	double rtt;
	int samp;
	int hops;
    } cto;
};

#endif


struct _helper_request {
    char *buf;
    HLPCB *callback;
    void *data;
    struct timeval dispatch_time;
};

struct _helper_stateful_request {
    char *buf;
    HLPSCB *callback;
    void *data;
    struct timeval dispatch_time;
};


struct _helper {
    wordlist *cmdline;
    dlink_list servers;
    dlink_list queue;
    const char *id_name;
    int n_to_start;
    int n_running;
    int n_active;
    int ipc_type;
    int concurrency;
    time_t last_queue_warn;
    struct {
	int requests;
	int replies;
	int queue_size;
	int max_queue_size;
	int avg_svc_time;
    } stats;
    time_t last_restart;
};

struct _helper_stateful {
    wordlist *cmdline;
    dlink_list servers;
    dlink_list queue;
    const char *id_name;
    int n_to_start;
    int n_running;
    int n_active;
    int ipc_type;
    int concurrency;
    MemPool *datapool;
    HLPSAVAIL *IsAvailable;
    HLPSRESET *Reset;
    time_t last_queue_warn;
    struct {
	int requests;
	int replies;
	int queue_size;
	int max_queue_size;
	int avg_svc_time;
    } stats;
    time_t last_restart;
};

struct _helper_server {
    int index;
    int pid;
    int rfd;
    int wfd;
    MemBuf wqueue;
    char *rbuf;
    size_t rbuf_sz;
    int roffset;
    dlink_node link;
    helper *parent;
    helper_request **requests;
    struct _helper_flags {
	unsigned int writing:1;
	unsigned int closing:1;
	unsigned int shutdown:1;
    } flags;
    struct {
	int uses;
	unsigned int pending;
    } stats;
    void *hIpc;
};


struct _helper_stateful_server {
    int index;
    int pid;
    int rfd;
    int wfd;
    char *buf;
    size_t buf_sz;
    int offset;
    struct timeval dispatch_time;
    struct timeval answer_time;
    dlink_node link;
    statefulhelper *parent;
    helper_stateful_request *request;
    struct _helper_stateful_flags {
	unsigned int alive:1;
	unsigned int busy:1;
	unsigned int closing:1;
	unsigned int shutdown:1;
	unsigned int reserved:1;
    } flags;
    struct {
	int uses;
	int submits;
	int releases;
    } stats;
    void *data;			/* State data used by the calling routines */
    void *hIpc;
};

/*
 * use this when you need to pass callback data to a blocking
 * operation, but you don't want to add that pointer to cbdata
 */
struct _generic_cbdata {
    void *data;
};

struct _store_rebuild_data {
    int objcount;		/* # objects successfully reloaded */
    int expcount;		/* # objects expired */
    int scancount;		/* # entries scanned or read from state file */
    int clashcount;		/* # swapfile clashes avoided */
    int dupcount;		/* # duplicates purged */
    int cancelcount;		/* # SWAP_LOG_DEL objects purged */
    int invalid;		/* # bad lines */
    int badflags;		/* # bad e->flags */
    int bad_log_op;
    int zero_object_sz;
};

/*
 * This defines an fs type
 */

struct _storefs_entry {
    const char *typestr;
    STFSPARSE *parsefunc;
    STFSRECONFIGURE *reconfigurefunc;
    STFSSHUTDOWN *donefunc;
};

/*
 * This defines an repl type
 */

struct _storerepl_entry {
    const char *typestr;
    REMOVALPOLICYCREATE *create;
};

/*
 * Async disk IO - this defines a async disk io queue
 */

struct _diskd_queue {
    int smsgid;			/* send sysvmsg id */
    int rmsgid;			/* recv sysvmsg id */
    int wfd;			/* queue file descriptor ? */
    int away;			/* number of requests away */
    int sent_count;		/* number of messages sent */
    int recv_count;		/* number of messages received */
    struct {
	char *buf;		/* shm buffer */
	link_list *stack;
	int id;			/* sysvshm id */
    } shm;
};

struct _Logfile {
    int fd;
    char path[MAXPATHLEN];
    char *buf;
    size_t bufsz;
    ssize_t offset;
    struct {
	unsigned int fatal;
	unsigned int syslog;
    } flags;
    int syslog_priority;
};

struct _logformat {
    char *name;
    logformat_token *format;
    logformat *next;
};

struct _customlog {
    char *filename;
    acl_list *aclList;
    logformat *logFormat;
    Logfile *logfile;
    customlog *next;
    enum {
	CLF_UNKNOWN,
	CLF_AUTO,
	CLF_CUSTOM,
	CLF_SQUID,
	CLF_COMMON,
	CLF_NONE
    } type;
};

struct cache_dir_option {
    const char *name;
    void (*parse) (SwapDir * sd, const char *option, const char *value, int reconfiguring);
    void (*dump) (StoreEntry * e, const char *option, SwapDir * sd);
};

struct error_map_entry {
    struct error_map_entry *next;
    char *value;
    int status;
};
struct _errormap {
    errormap *next;
    char *url;
    struct error_map_entry *map;
};

struct _VaryData {
    char *key;
    char *etag;
    Array etags;
};

#endif /* SQUID_STRUCTS_H */

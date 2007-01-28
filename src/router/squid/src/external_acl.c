
/*
 * $Id: external_acl.c,v 1.29 2007/01/01 23:32:13 hno Exp $
 *
 * DEBUG: section 82    External ACL
 * AUTHOR: Henrik Nordstrom, MARA Systems AB
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  The contents of this file is Copyright (C) 2002 by MARA Systems AB,
 *  Sweden, unless otherwise is indicated in the specific function. The
 *  author gives his full permission to include this file into the Squid
 *  software product under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
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

#include "squid.h"

#ifndef DEFAULT_EXTERNAL_ACL_TTL
#define DEFAULT_EXTERNAL_ACL_TTL 1 * 60 * 60
#endif
#ifndef DEFAULT_EXTERNAL_ACL_CHILDREN
#define DEFAULT_EXTERNAL_ACL_CHILDREN 5
#endif

typedef struct _external_acl_format external_acl_format;
typedef struct _external_acl_data external_acl_data;

static char *makeExternalAclKey(aclCheck_t * ch, external_acl_data * acl_data);
static void external_acl_cache_delete(external_acl * def, external_acl_entry * entry);
static int external_acl_entry_expired(external_acl * def, external_acl_entry * entry);
static int external_acl_grace_expired(external_acl * def, external_acl_entry * entry);
static void external_acl_cache_touch(external_acl * def, external_acl_entry * entry);
static int external_acl_is_pending(external_acl * def, const char *key);

/*******************************************************************
 * external_acl cache entry
 * Used opaqueue in the interface
 */
struct _external_acl_entry {
    hash_link hash;
    dlink_node lru;
    int result;
    time_t date;
    char *user;
    char *passwd;
    char *message;
    char *log;
    external_acl *def;
};

/******************************************************************
 * external_acl directive
 */
struct _external_acl {
    external_acl *next;
    int ttl;
    int negative_ttl;
    int grace;
    char *name;
    external_acl_format *format;
    wordlist *cmdline;
    int children;
    int concurrency;
    helper *helper;
    hash_table *cache;
    dlink_list lru_list;
    int cache_size;
    int cache_entries;
    dlink_list queue;
    int require_auth;
    enum {
	QUOTE_METHOD_SHELL = 1,
	QUOTE_METHOD_URL
    } quote;
};

struct _external_acl_format {
    enum {
	EXT_ACL_UNKNOWN,
	EXT_ACL_LOGIN,
#if USE_IDENT
	EXT_ACL_IDENT,
#endif
	EXT_ACL_SRC,
	EXT_ACL_SRCPORT,
	EXT_ACL_MYADDR,
	EXT_ACL_MYPORT,
	EXT_ACL_DST,
	EXT_ACL_PROTO,
	EXT_ACL_PORT,
	EXT_ACL_PATH,
	EXT_ACL_METHOD,
	EXT_ACL_HEADER,
	EXT_ACL_HEADER_MEMBER,
	EXT_ACL_HEADER_ID,
	EXT_ACL_HEADER_ID_MEMBER,
#if USE_SSL
	EXT_ACL_USER_CERT,
	EXT_ACL_CA_CERT,
	EXT_ACL_USER_CERT_RAW,
	EXT_ACL_USER_CERTCHAIN_RAW,
#endif
	EXT_ACL_EXT_USER,
	EXT_ACL_ACL,
	EXT_ACL_DATA,
	EXT_ACL_END
    } type;
    external_acl_format *next;
    char *header;
    char *member;
    char separator;
    http_hdr_type header_id;
};

/* FIXME: These are not really cbdata, but it is an easy way
 * to get them pooled, refcounted, accounted and freed properly...
 */
CBDATA_TYPE(external_acl);
CBDATA_TYPE(external_acl_format);

static void
free_external_acl_format(void *data)
{
    external_acl_format *p = data;
    safe_free(p->header);
}

static void
free_external_acl(void *data)
{
    external_acl *p = data;
    safe_free(p->name);
    while (p->format) {
	external_acl_format *f = p->format;
	p->format = f->next;
	cbdataFree(f);
    }
    wordlistDestroy(&p->cmdline);
    if (p->helper) {
	helperShutdown(p->helper);
	helperFree(p->helper);
	p->helper = NULL;
    }
    while (p->lru_list.tail)
	external_acl_cache_delete(p, p->lru_list.tail->data);
    if (p->cache)
	hashFreeMemory(p->cache);
}

void
parse_externalAclHelper(external_acl ** list)
{
    external_acl *a;
    char *token;
    external_acl_format **p;

    CBDATA_INIT_TYPE_FREECB(external_acl, free_external_acl);
    CBDATA_INIT_TYPE_FREECB(external_acl_format, free_external_acl_format);

    a = cbdataAlloc(external_acl);

    a->ttl = DEFAULT_EXTERNAL_ACL_TTL;
    a->negative_ttl = -1;
    a->children = DEFAULT_EXTERNAL_ACL_CHILDREN;

    token = strtok(NULL, w_space);
    if (!token)
	self_destruct();
    a->name = xstrdup(token);
    a->quote = QUOTE_METHOD_URL;

    token = strtok(NULL, w_space);
    /* Parse options */
    while (token) {
	if (strncmp(token, "ttl=", 4) == 0) {
	    a->ttl = atoi(token + 4);
	} else if (strncmp(token, "negative_ttl=", 13) == 0) {
	    a->negative_ttl = atoi(token + 13);
	} else if (strncmp(token, "children=", 9) == 0) {
	    a->children = atoi(token + 9);
	} else if (strncmp(token, "concurrency=", 12) == 0) {
	    a->concurrency = atoi(token + 12);
	} else if (strncmp(token, "cache=", 6) == 0) {
	    a->cache_size = atoi(token + 6);
	} else if (strcmp(token, "protocol=2.5") == 0) {
	    a->quote = QUOTE_METHOD_SHELL;
	} else if (strcmp(token, "protocol=3.0") == 0) {
	    a->quote = QUOTE_METHOD_URL;
	} else if (strcmp(token, "quote=url") == 0) {
	    a->quote = QUOTE_METHOD_URL;
	} else if (strcmp(token, "quote=shell") == 0) {
	    a->quote = QUOTE_METHOD_SHELL;
	} else if (strncmp(token, "grace=", 6) == 0) {
	    a->grace = atoi(token + 6);
	} else {
	    break;
	}

	token = strtok(NULL, w_space);
    }
    if (a->negative_ttl == -1)
	a->negative_ttl = a->ttl;

    /* Parse format */
    p = &a->format;
    while (token) {
	external_acl_format *format;

	/* stop on first non-format token found */
	if (*token != '%')
	    break;

	format = cbdataAlloc(external_acl_format);

	if (strncmp(token, "%{", 2) == 0) {
	    /* header format */
	    char *header, *member, *end;
	    header = token + 2;
	    end = strchr(header, '}');
	    /* cut away the terminating } */
	    if (end && strlen(end) == 1)
		*end = '\0';
	    else
		self_destruct();

	    member = strchr(header, ':');
	    if (member) {
		/* Split in header and member */
		*member++ = '\0';
		if (!xisalnum(*member))
		    format->separator = *member++;
		else
		    format->separator = ',';
		format->member = xstrdup(member);
		format->type = EXT_ACL_HEADER_MEMBER;
	    } else {
		format->type = EXT_ACL_HEADER;
	    }
	    format->header = xstrdup(header);
	    format->header_id = httpHeaderIdByNameDef(header, strlen(header));
	    if (format->header_id != -1) {
		if (member)
		    format->type = EXT_ACL_HEADER_ID_MEMBER;
		else
		    format->type = EXT_ACL_HEADER_ID;
	    }
	} else if (strcmp(token, "%LOGIN") == 0) {
	    format->type = EXT_ACL_LOGIN;
	    a->require_auth = 1;
	}
#if USE_IDENT
	else if (strcmp(token, "%IDENT") == 0)
	    format->type = EXT_ACL_IDENT;
#endif
	else if (strcmp(token, "%SRC") == 0)
	    format->type = EXT_ACL_SRC;
	else if (strcmp(token, "%SRCPORT") == 0)
	    format->type = EXT_ACL_SRCPORT;
	else if (strcmp(token, "%MYADDR") == 0)
	    format->type = EXT_ACL_MYADDR;
	else if (strcmp(token, "%MYPORT") == 0)
	    format->type = EXT_ACL_MYPORT;
	else if (strcmp(token, "%DST") == 0)
	    format->type = EXT_ACL_DST;
	else if (strcmp(token, "%PROTO") == 0)
	    format->type = EXT_ACL_PROTO;
	else if (strcmp(token, "%PORT") == 0)
	    format->type = EXT_ACL_PORT;
	else if (strcmp(token, "%PATH") == 0)
	    format->type = EXT_ACL_PATH;
	else if (strcmp(token, "%METHOD") == 0)
	    format->type = EXT_ACL_METHOD;
#if USE_SSL
	else if (strcmp(token, "%USER_CERT") == 0)
	    format->type = EXT_ACL_USER_CERT_RAW;
	else if (strcmp(token, "%USER_CERTCHAIN") == 0)
	    format->type = EXT_ACL_USER_CERTCHAIN_RAW;
	else if (strncmp(token, "%USER_CERT_", 11) == 0) {
	    format->type = EXT_ACL_USER_CERT;
	    format->header = xstrdup(token + 11);
	} else if (strncmp(token, "%CA_CERT_", 9) == 0) {
	    format->type = EXT_ACL_CA_CERT;
	    format->header = xstrdup(token + 9);
	}
#endif
	else if (strcmp(token, "%EXT_USER") == 0)
	    format->type = EXT_ACL_EXT_USER;
	else if (strcmp(token, "%ACL") == 0)
	    format->type = EXT_ACL_ACL;
	else if (strcmp(token, "%DATA") == 0)
	    format->type = EXT_ACL_DATA;
	else {
	    self_destruct();
	}
	*p = format;
	p = &format->next;
	token = strtok(NULL, w_space);
    }

    /* There must be at least one format token */
    if (!a->format)
	self_destruct();

    /* helper */
    if (!token)
	self_destruct();
    wordlistAdd(&a->cmdline, token);

    /* arguments */
    parse_wordlist(&a->cmdline);

    while (*list)
	list = &(*list)->next;
    *list = a;
}

void
dump_externalAclHelper(StoreEntry * sentry, const char *name, const external_acl * list)
{
    const external_acl *node;
    const external_acl_format *format;
    const wordlist *word;
    for (node = list; node; node = node->next) {
	storeAppendPrintf(sentry, "%s %s", name, node->name);
	if (node->ttl != DEFAULT_EXTERNAL_ACL_TTL)
	    storeAppendPrintf(sentry, " ttl=%d", node->ttl);
	if (node->negative_ttl != node->ttl)
	    storeAppendPrintf(sentry, " negative_ttl=%d", node->negative_ttl);
	if (node->grace)
	    storeAppendPrintf(sentry, " grace=%d", node->grace);
	if (node->children != DEFAULT_EXTERNAL_ACL_CHILDREN)
	    storeAppendPrintf(sentry, " children=%d", node->children);
	if (node->concurrency)
	    storeAppendPrintf(sentry, " concurrency=%d", node->concurrency);
	if (node->cache_size)
	    storeAppendPrintf(sentry, " cache=%d", node->cache_size);
	for (format = node->format; format; format = format->next) {
	    switch (format->type) {
	    case EXT_ACL_HEADER:
	    case EXT_ACL_HEADER_ID:
		storeAppendPrintf(sentry, " %%{%s}", format->header);
		break;
	    case EXT_ACL_HEADER_MEMBER:
	    case EXT_ACL_HEADER_ID_MEMBER:
		storeAppendPrintf(sentry, " %%{%s:%s}", format->header, format->member);
		break;
#define DUMP_EXT_ACL_TYPE(a) \
	    case EXT_ACL_##a: \
		storeAppendPrintf(sentry, " %%%s", #a); \
		break
		DUMP_EXT_ACL_TYPE(LOGIN);
#if USE_IDENT
		DUMP_EXT_ACL_TYPE(IDENT);
#endif
		DUMP_EXT_ACL_TYPE(SRC);
		DUMP_EXT_ACL_TYPE(SRCPORT);
		DUMP_EXT_ACL_TYPE(MYADDR);
		DUMP_EXT_ACL_TYPE(MYPORT);
		DUMP_EXT_ACL_TYPE(DST);
		DUMP_EXT_ACL_TYPE(PROTO);
		DUMP_EXT_ACL_TYPE(PORT);
		DUMP_EXT_ACL_TYPE(PATH);
		DUMP_EXT_ACL_TYPE(METHOD);
		DUMP_EXT_ACL_TYPE(EXT_USER);
#if USE_SSL
	    case EXT_ACL_USER_CERT_RAW:
		storeAppendPrintf(sentry, " %%USER_CERT");
		break;
	    case EXT_ACL_USER_CERTCHAIN_RAW:
		storeAppendPrintf(sentry, " %%USER_CERTCHAIN");
		break;
	    case EXT_ACL_USER_CERT:
		storeAppendPrintf(sentry, " %%USER_CERT_%s", format->header);
		break;
	    case EXT_ACL_CA_CERT:
		storeAppendPrintf(sentry, " %%CA_CERT_%s", format->header);
		break;
#endif
		DUMP_EXT_ACL_TYPE(ACL);
		DUMP_EXT_ACL_TYPE(DATA);
	    case EXT_ACL_UNKNOWN:
	    case EXT_ACL_END:
		fatal("unknown external_acl format error");
		break;
	    }
	}
	for (word = node->cmdline; word; word = word->next)
	    storeAppendPrintf(sentry, " %s", word->key);
	storeAppendPrintf(sentry, "\n");
    }
}

void
free_externalAclHelper(external_acl ** list)
{
    while (*list) {
	external_acl *node = *list;
	*list = node->next;
	node->next = NULL;
	cbdataFree(node);
    }
}

static external_acl *
find_externalAclHelper(const char *name)
{
    external_acl *node;

    for (node = Config.externalAclHelperList; node; node = node->next) {
	if (strcmp(node->name, name) == 0)
	    return node;
    }
    return NULL;
}


/******************************************************************
 * external acl type
 */

struct _external_acl_data {
    external_acl *def;
    const char *name;
    wordlist *arguments;
};

CBDATA_TYPE(external_acl_data);
static void
free_external_acl_data(void *data)
{
    external_acl_data *p = data;
    wordlistDestroy(&p->arguments);
    cbdataUnlock(p->def);
    p->def = NULL;
}

void
aclParseExternal(void *dataptr, const char *name)
{
    external_acl_data **datap = dataptr;
    external_acl_data *data;
    char *token;
    if (*datap)
	self_destruct();
    CBDATA_INIT_TYPE_FREECB(external_acl_data, free_external_acl_data);
    data = cbdataAlloc(external_acl_data);
    token = strtok(NULL, w_space);
    if (!token)
	self_destruct();
    data->name = name;
    data->def = find_externalAclHelper(token);
    cbdataLock(data->def);
    if (!data->def)
	self_destruct();
    while ((token = strtokFile())) {
	wordlistAdd(&data->arguments, token);
    }
    *datap = data;
}

void
aclDestroyExternal(void **dataptr)
{
    cbdataFree(*dataptr);
}

static inline int
externalAclOverload(external_acl * def)
{
    return def->helper->stats.queue_size > def->helper->n_running;
}

int
aclMatchExternal(void *data, aclCheck_t * ch)
{
    int result;
    external_acl_entry *entry = NULL;
    external_acl_data *acl = data;
    request_t *request;
    const char *key = "";
    debug(82, 9) ("aclMatchExternal: acl=\"%s\"\n", acl->def->name);
    request = ch->request;
    if (ch->extacl_entry) {
	entry = ch->extacl_entry;
	if (!cbdataValid(entry))
	    entry = NULL;
	cbdataUnlock(ch->extacl_entry);
	ch->extacl_entry = NULL;
    }
    if (acl->def->require_auth) {
	int ti;
	/* Make sure the user is authenticated */
	if ((ti = aclAuthenticated(ch)) != 1) {
	    debug(82, 2) ("aclMatchExternal: %s user not authenticated (%d)\n", acl->def->name, ti);
	    return ti;
	}
    }
    key = makeExternalAclKey(ch, acl);
    if (!key) {
	/* Not sufficient data to process */
	return -1;
    }
    if (entry) {
	if (entry->def != acl->def || strcmp(entry->hash.key, key) != 0) {
	    /* Not ours.. get rid of it */
	    cbdataUnlock(ch->extacl_entry);
	    ch->extacl_entry = NULL;
	    entry = NULL;
	}
    }
    if (!entry) {
	int lookup_needed = 1;
	entry = hash_lookup(acl->def->cache, key);
	if (entry && !external_acl_entry_expired(acl->def, entry)) {
	    lookup_needed = external_acl_grace_expired(acl->def, entry);
	    /* Don't make graceful lookups if already pending */
	    if (lookup_needed && external_acl_is_pending(acl->def, key))
		lookup_needed = 0;
	    /* Don't make graceful lookups when under high load */
	    if (acl->def->helper->stats.queue_size > acl->def->helper->n_running * 2 / 3)
		lookup_needed = 0;
	}
	if (lookup_needed) {
	    debug(82, 2) ("aclMatchExternal: %s(\"%s\") = lookup needed\n", acl->def->name, key);
	    if (!externalAclOverload(acl->def)) {
		ch->state[ACL_EXTERNAL] = ACL_LOOKUP_NEEDED;
		return -1;
	    } else {
		if (!entry) {
		    debug(82, 1) ("aclMatchExternal: '%s' queue overload. Request rejected '%s'.\n", acl->def->name, key);
		    return -1;
		} else {
		    debug(82, 1) ("aclMatchExternal: '%s' queue overload. Using stale result. '%s'\n", acl->def->name, key);
		    /* Fall thru to processing below */
		}
	    }
	}
    }
    external_acl_cache_touch(acl->def, entry);
    result = entry->result;
    external_acl_message = entry->message;

    debug(82, 2) ("aclMatchExternal: %s = %d\n", acl->def->name, result);
    if (entry->user) {
	safe_free(request->extacl_user);
	request->extacl_user = xstrdup(entry->user);
    }
    if (entry->passwd) {
	safe_free(request->extacl_passwd);
	request->extacl_passwd = xstrdup(entry->passwd);
    }
    if (entry->log)
	stringReset(&ch->request->extacl_log, entry->log);
    return result;
}

wordlist *
aclDumpExternal(void *data)
{
    external_acl_data *acl = data;
    wordlist *result = NULL;
    wordlist *arg;
    MemBuf mb;
    memBufDefInit(&mb);
    memBufPrintf(&mb, "%s", acl->def->name);
    for (arg = acl->arguments; arg; arg = arg->next) {
	memBufPrintf(&mb, " %s", arg->key);
    }
    wordlistAdd(&result, mb.buf);
    memBufClean(&mb);
    return result;
}

/******************************************************************
 * external_acl cache
 */

CBDATA_TYPE(external_acl_entry);

static void
external_acl_cache_touch(external_acl * def, external_acl_entry * entry)
{
    dlinkDelete(&entry->lru, &def->lru_list);
    dlinkAdd(entry, &entry->lru, &def->lru_list);
}

static char *
makeExternalAclKey(aclCheck_t * ch, external_acl_data * acl_data)
{
    static MemBuf mb = MemBufNULL;
    char buf[256];
    int first = 1;
    wordlist *arg;
    external_acl_format *format;
    request_t *request = ch->request;
    String sb = StringNull;
    int data_used = 0;
    memBufReset(&mb);
    for (format = acl_data->def->format; format; format = format->next) {
	const char *str = NULL;
	switch (format->type) {
	case EXT_ACL_LOGIN:
	    str = authenticateUserRequestUsername(request->auth_user_request);
	    break;
#if USE_IDENT
	case EXT_ACL_IDENT:
	    str = ch->rfc931;
	    if (!str || !*str) {
		ch->state[ACL_IDENT] = ACL_LOOKUP_NEEDED;
		return NULL;
	    }
	    break;
#endif
	case EXT_ACL_SRC:
	    str = inet_ntoa(ch->src_addr);
	    break;
	case EXT_ACL_SRCPORT:
	    snprintf(buf, sizeof(buf), "%d", request->client_port);
	    str = buf;
	    break;
	case EXT_ACL_MYADDR:
	    str = inet_ntoa(request->my_addr);
	    break;
	case EXT_ACL_MYPORT:
	    snprintf(buf, sizeof(buf), "%d", request->my_port);
	    str = buf;
	    break;
	case EXT_ACL_DST:
	    str = request->host;
	    break;
	case EXT_ACL_PROTO:
	    str = ProtocolStr[request->protocol];
	    break;
	case EXT_ACL_PORT:
	    snprintf(buf, sizeof(buf), "%d", request->port);
	    str = buf;
	    break;
	case EXT_ACL_PATH:
	    str = strBuf(request->urlpath);
	    break;
	case EXT_ACL_METHOD:
	    str = RequestMethodStr[request->method];
	    break;
	case EXT_ACL_HEADER:
	    sb = httpHeaderGetByName(&request->header, format->header);
	    str = strBuf(sb);
	    break;
	case EXT_ACL_HEADER_ID:
	    sb = httpHeaderGetStrOrList(&request->header, format->header_id);
	    str = strBuf(sb);
	    break;
	case EXT_ACL_HEADER_MEMBER:
	    sb = httpHeaderGetByNameListMember(&request->header, format->header, format->member, format->separator);
	    str = strBuf(sb);
	    break;
	case EXT_ACL_HEADER_ID_MEMBER:
	    sb = httpHeaderGetListMember(&request->header, format->header_id, format->member, format->separator);
	    str = strBuf(sb);
	    break;
#if USE_SSL
	case EXT_ACL_USER_CERT_RAW:
	    if (cbdataValid(ch->conn)) {
		SSL *ssl = fd_table[ch->conn->fd].ssl;
		if (ssl)
		    str = sslGetUserCertificatePEM(ssl);
	    }
	    break;
	case EXT_ACL_USER_CERTCHAIN_RAW:
	    if (cbdataValid(ch->conn)) {
		SSL *ssl = fd_table[ch->conn->fd].ssl;
		if (ssl)
		    str = sslGetUserCertificateChainPEM(ssl);
	    }
	    break;
	case EXT_ACL_USER_CERT:
	    if (cbdataValid(ch->conn)) {
		SSL *ssl = fd_table[ch->conn->fd].ssl;
		if (ssl)
		    str = sslGetUserAttribute(ssl, format->header);
	    }
	    break;
	case EXT_ACL_CA_CERT:
	    if (cbdataValid(ch->conn)) {
		SSL *ssl = fd_table[ch->conn->fd].ssl;
		if (ssl)
		    str = sslGetCAAttribute(ssl, format->header);
	    }
	    break;
#endif
	case EXT_ACL_EXT_USER:
	    str = request->extacl_user;
	    break;
	case EXT_ACL_DATA:
	    data_used = 1;
	    for (arg = acl_data->arguments; arg; arg = arg->next) {
		if (arg != acl_data->arguments)
		    stringAppend(&sb, " ", 1);
		if (acl_data->def->quote == QUOTE_METHOD_URL) {
		    const char *quoted = rfc1738_escape(arg->key);
		    stringAppend(&sb, quoted, strlen(quoted));
		} else {
		    static MemBuf mb2 = MemBufNULL;
		    strwordquote(&mb2, arg->key);
		    stringAppend(&sb, mb2.buf, mb2.size);
		    memBufClean(&mb2);
		}
		first = 0;
	    }
	    str = strBuf(sb);
	    break;

	case EXT_ACL_ACL:
	    str = acl_data->name;
	    break;

	case EXT_ACL_UNKNOWN:
	case EXT_ACL_END:
	    fatal("unknown external_acl format error");
	    break;
	}
	if (str)
	    if (!*str)
		str = NULL;
	if (!str)
	    str = "-";
	if (!first)
	    memBufAppend(&mb, " ", 1);
	if (acl_data->def->quote == QUOTE_METHOD_URL) {
	    const char *quoted = rfc1738_escape(str);
	    memBufAppend(&mb, quoted, strlen(quoted));
	} else {
	    strwordquote(&mb, str);
	}
	stringClean(&sb);
	first = 0;
    }
    if (!data_used) {
	for (arg = acl_data->arguments; arg; arg = arg->next) {
	    if (!first)
		memBufAppend(&mb, " ", 1);
	    if (acl_data->def->quote == QUOTE_METHOD_URL) {
		const char *quoted = rfc1738_escape(arg->key);
		memBufAppend(&mb, quoted, strlen(quoted));
	    } else {
		strwordquote(&mb, arg->key);
	    }
	    first = 0;
	}
    }
    return mb.buf;
}

static int
external_acl_entry_expired(external_acl * def, external_acl_entry * entry)
{
    if (entry->date + (entry->result == 1 ? def->ttl : def->negative_ttl) < squid_curtime)
	return 1;
    else
	return 0;
}

static int
external_acl_grace_expired(external_acl * def, external_acl_entry * entry)
{
    int ttl;
    ttl = entry->result == 1 ? def->ttl : def->negative_ttl;
    ttl = (ttl * (100 - def->grace)) / 100;
    if (entry->date + ttl < squid_curtime)
	return 1;
    else
	return 0;
}

static void
free_external_acl_entry(void *data)
{
    external_acl_entry *entry = data;
    safe_free(entry->hash.key);
    safe_free(entry->user);
    safe_free(entry->message);
    safe_free(entry->log);
}

static external_acl_entry *
external_acl_cache_add(external_acl * def, const char *key, int result, char *user, char *passwd, char *message, char *log)
{
    external_acl_entry *entry = hash_lookup(def->cache, key);
    debug(82, 2) ("external_acl_cache_add: Adding '%s' = %d\n", key, result);
    if (entry) {
	debug(82, 3) ("external_acl_cache_add: updating existing entry\n");
	entry->date = squid_curtime;
	entry->result = result;
	safe_free(entry->user);
	if (user)
	    entry->user = xstrdup(user);
	safe_free(entry->passwd);
	if (passwd)
	    entry->passwd = xstrdup(passwd);
	safe_free(entry->message);
	if (message)
	    entry->message = xstrdup(message);
	safe_free(entry->log);
	if (log)
	    entry->log = xstrdup(log);
	external_acl_cache_touch(def, entry);
	return entry;
    }
    CBDATA_INIT_TYPE_FREECB(external_acl_entry, free_external_acl_entry);
    /* Maintain cache size */
    if (def->cache_size && def->cache_entries >= def->cache_size)
	external_acl_cache_delete(def, def->lru_list.tail->data);
    entry = cbdataAlloc(external_acl_entry);
    entry->hash.key = xstrdup(key);
    entry->date = squid_curtime;
    entry->result = result;
    if (user)
	entry->user = xstrdup(user);
    if (passwd)
	entry->passwd = xstrdup(passwd);
    if (message)
	entry->message = xstrdup(message);
    if (log)
	entry->log = xstrdup(log);
    entry->def = def;
    hash_join(def->cache, &entry->hash);
    dlinkAdd(entry, &entry->lru, &def->lru_list);
    def->cache_entries += 1;
    return entry;
}

static void
external_acl_cache_delete(external_acl * def, external_acl_entry * entry)
{
    hash_remove_link(def->cache, &entry->hash);
    dlinkDelete(&entry->lru, &def->lru_list);
    def->cache_entries -= 1;
    cbdataFree(entry);
}

/******************************************************************
 * external_acl helpers
 */

typedef struct _externalAclState externalAclState;
struct _externalAclState {
    EAH *callback;
    void *callback_data;
    char *key;
    external_acl *def;
    dlink_node list;
    externalAclState *queue;
};

CBDATA_TYPE(externalAclState);
static void
free_externalAclState(void *data)
{
    externalAclState *state = data;
    safe_free(state->key);
    cbdataUnlock(state->callback_data);
    cbdataUnlock(state->def);
}

/*
 * The helper program receives queries on stdin, one
 * per line, and must return the result on on stdout
 *
 * General result syntax:
 *
 *   OK/ERR keyword=value ...
 *
 * Keywords:
 *
 *   user=        The users name (login)
 *   message=     Message describing the reason
 *   log=         A string to be used in access logging
 *
 * Other keywords may be added to the protocol later
 *
 * value needs to be enclosed in quotes if it may contain whitespace, or 
 * the whitespace escaped using \ (\ escaping obviously also applies to  
 * any " characters)
 */

static void
externalAclHandleReply(void *data, char *reply)
{
    externalAclState *state = data;
    externalAclState *next;
    int result = 0;
    char *status;
    char *token;
    char *value;
    char *t;
    char *user = NULL;
    char *passwd = NULL;
    char *message = NULL;
    char *log = NULL;
    external_acl_entry *entry = NULL;

    debug(82, 2) ("externalAclHandleReply: reply=\"%s\"\n", reply);

    if (reply) {
	status = strwordtok(reply, &t);
	if (status && strcmp(status, "OK") == 0)
	    result = 1;

	while ((token = strwordtok(NULL, &t))) {
	    value = strchr(token, '=');
	    if (value) {
		*value++ = '\0';	/* terminate the token, and move up to the value */
		if (state->def->quote == QUOTE_METHOD_URL)
		    rfc1738_unescape(value);
		if (strcmp(token, "user") == 0)
		    user = value;
		else if (strcmp(token, "login") == 0)
		    user = value;
		else if (strcmp(token, "error") == 0)
		    message = value;
		else if (strcmp(token, "message") == 0)
		    message = value;
		else if (strcmp(token, "log") == 0)
		    log = value;
		else if (strcmp(token, "password") == 0)
		    passwd = value;
		else if (strcmp(token, "passwd") == 0)
		    passwd = value;
	    }
	}
    }
    dlinkDelete(&state->list, &state->def->queue);
    if (cbdataValid(state->def)) {
	if (reply)
	    entry = external_acl_cache_add(state->def, state->key, result, user, passwd, message, log);
	else {
	    external_acl_entry *oldentry = hash_lookup(state->def->cache, state->key);
	    if (oldentry)
		external_acl_cache_delete(state->def, oldentry);
	}
    }
    do {
	cbdataUnlock(state->def);
	state->def = NULL;

	if (entry)
	    external_acl_message = entry->message;
	else
	    external_acl_message = NULL;

	if (state->callback && cbdataValid(state->callback_data))
	    state->callback(state->callback_data, entry);
	cbdataUnlock(state->callback_data);
	state->callback_data = NULL;

	next = state->queue;
	cbdataFree(state);
	state = next;
    } while (state);
}

const char *
externalAclMessage(external_acl_entry * entry)
{
    return entry->message;
}

static int
external_acl_is_pending(external_acl * def, const char *key)
{
    /* Check for a pending lookup */
    dlink_node *node;
    for (node = def->queue.head; node; node = node->next) {
	externalAclState *oldstatetmp = node->data;
	if (strcmp(key, oldstatetmp->key) == 0) {
	    return 1;
	}
    }
    return 0;
}

void
externalAclLookup(aclCheck_t * ch, void *acl_data, EAH * callback, void *callback_data)
{
    MemBuf buf;
    external_acl_data *acl = acl_data;
    external_acl *def = acl->def;
    const char *key;
    external_acl_entry *entry;
    externalAclState *state;
    dlink_node *node;
    externalAclState *oldstate = NULL;
    int graceful = 0;

    if (acl->def->require_auth) {
	int ti;
	/* Make sure the user is authenticated */
	if ((ti = aclAuthenticated(ch)) != 1) {
	    debug(82, 1) ("externalAclLookup: %s user authentication failure (%d)\n", acl->def->name, ti);
	    callback(callback_data, NULL);
	    return;
	}
    }
    key = makeExternalAclKey(ch, acl);
    if (!key) {
	debug(82, 1) ("externalAclLookup: lookup in '%s', prerequisit failure\n", def->name);
	external_acl_message = "MISSING REQUIRED INFORMATION";
	callback(callback_data, NULL);
	return;
    }
    debug(82, 2) ("externalAclLookup: lookup in '%s' for '%s'\n", def->name, key);
    entry = hash_lookup(def->cache, key);
    if (entry && external_acl_entry_expired(def, entry))
	entry = NULL;

    /* Check for a pending lookup to hook into */
    for (node = def->queue.head; node; node = node->next) {
	externalAclState *oldstatetmp = node->data;
	if (strcmp(key, oldstatetmp->key) == 0) {
	    oldstate = oldstatetmp;
	    break;
	}
    }

    /* No need to refresh a already pending lookup during grace period */
    if (entry && external_acl_grace_expired(def, entry)) {
	if (oldstate) {
	    callback(callback_data, entry);
	    return;
	} else {
	    graceful = 1;
	}
    }
    /* No pending lookup found. Sumbit to helper */
    state = cbdataAlloc(externalAclState);
    state->def = def;
    cbdataLock(state->def);
    state->key = xstrdup(key);
    if (!graceful) {
	state->callback = callback;
	state->callback_data = callback_data;
	cbdataLock(state->callback_data);
    }
    if (oldstate) {
	/* Hook into pending lookup */
	state->queue = oldstate->queue;
	oldstate->queue = state;
    } else {
	/* Check for queue overload */
	if (externalAclOverload(def)) {
	    debug(82, 1) ("externalAclLookup: '%s' queue overload\n", def->name);
	    cbdataFree(state);
	    callback(callback_data, entry);
	    return;
	}
	/* Send it off to the helper */
	memBufDefInit(&buf);
	memBufPrintf(&buf, "%s\n", key);
	helperSubmit(def->helper, buf.buf, externalAclHandleReply, state);
	dlinkAdd(state, &state->list, &def->queue);
	memBufClean(&buf);
    }
    if (graceful) {
	/* No need to wait during grace period */
	callback(callback_data, entry);
	return;
    } else {
	ch->state[ACL_EXTERNAL] = ACL_LOOKUP_PENDING;
    }
}

int
externalAclRequiresAuth(void *acl_data)
{
    external_acl_data *acl = acl_data;
    return acl->def->require_auth;
}

static void
externalAclStats(StoreEntry * sentry)
{
    external_acl *p;

    for (p = Config.externalAclHelperList; p; p = p->next) {
	storeAppendPrintf(sentry, "External ACL Statistics: %s\n", p->name);
	storeAppendPrintf(sentry, "Cache size: %d\n", p->cache->count);
	helperStats(sentry, p->helper);
	storeAppendPrintf(sentry, "\n");
    }
}

void
externalAclConfigure(void)
{
    external_acl *p;
    for (p = Config.externalAclHelperList; p; p = p->next) {
	requirePathnameExists("external_acl_type", p->cmdline->key);
    }
}

void
externalAclInit(void)
{
    static int firstTimeInit = 1;
    external_acl *p;

    for (p = Config.externalAclHelperList; p; p = p->next) {
	if (!p->cache)
	    p->cache = hash_create((HASHCMP *) strcmp, hashPrime(1024), hash4);
	if (!p->helper)
	    p->helper = helperCreate(p->name);
	p->helper->cmdline = p->cmdline;
	p->helper->n_to_start = p->children;
	p->helper->concurrency = p->concurrency;
	p->helper->ipc_type = IPC_STREAM;
	helperOpenServers(p->helper);
    }
    if (firstTimeInit) {
	firstTimeInit = 0;
	cachemgrRegister("external_acl",
	    "External ACL stats",
	    externalAclStats, 0, 1);
	CBDATA_INIT_TYPE_FREECB(externalAclState, free_externalAclState);
    }
}

void
externalAclShutdown(void)
{
    external_acl *p;
    for (p = Config.externalAclHelperList; p; p = p->next) {
	helperShutdown(p->helper);
    }
}

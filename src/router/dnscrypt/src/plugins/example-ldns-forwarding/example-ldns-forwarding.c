
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#ifdef _WIN32
# include <ws2tcpip.h>
#endif

#include <dnscrypt/plugin.h>
#include <ldns/ldns.h>

#ifdef _MSC_VER
# define strncasecmp _strnicmp
# define strcasecmp _stricmp
#endif

DCPLUGIN_MAIN(__FILE__);

#define UPSTREAM_TIMEOUT_SEC 1

typedef struct Forwarder_ {
    ldns_resolver *resolver;
    char          *domains;
} Forwarder;

static struct option getopt_long_options[] = {
    { "domains", 1, NULL, 'd' },
    { "resolvers", 1, NULL, 'r' },
    { NULL, 0, NULL, 0 }
};
static const char *getopt_options = "dr";

const char *
dcplugin_description(DCPlugin * const dcplugin)
{
    return "Forward queries for some domains to specific resolvers";
}

const char *
dcplugin_long_description(DCPlugin * const dcplugin)
{
    return
        "This plugin redirects queries for specific zones to a set of\n"
        "non-DNSCrypt resolvers. This can be useful for private zones that\n"
        "can only be resolved by a local DNS server.\n"
        "\n"
        "Plugin parameters:\n"
        "--domains=\"<domain> [<domain>...]\"\n"
        "--resolvers=\"<ip> [<ip>...]\"\n"
        "\n"
        "Requests are synchronous, so this plugin should not be enabled with\n"
        "untrusted clients.";
}

static void
str_tolower(char *str)
{
    while (*str != 0) {
        *str = (char) tolower((unsigned char) *str);
        str++;
    }
}

static char *
skip_spaces(char *str)
{
    while (*str != 0 && isspace((int) (unsigned char) *str)) {
        str++;
    }
    return str;
}

static char *
skip_chars(char *str)
{
    while (*str != 0 && !isspace((int) (unsigned char) *str)) {
        str++;
    }
    return str;
}

static char *
trim(char *str)
{
    char *ptr;
    char *s1;
    char *s2;

    str = skip_spaces(str);
    if (*str == 0) {
        return str;
    }
    s1 = skip_chars(str);
    if (*(s2 = skip_spaces(s1)) == 0) {
        *s1 = 0;
        return str;
    }
    *skip_chars(s2) = 0;

    return s2;
}

static int
parse_domain_list(char ** const domains_p,  char * const str)
{
    char   *domain = str;
    char   *sep;
    char   *tmp;
    char    sc;
    size_t  domain_len;
    size_t  domains_size;

    domains_size = 0U;
    *domains_p = NULL;
    for (;;) {
        domain = skip_spaces(domain);
        sep = domain;
        for (;;) {
            if (*sep == 0) {
                sep = NULL;
                break;
            }
            if (isspace((unsigned char) *sep)) {
                break;
            }
            sep++;
        }
        if (sep != NULL) {
            sc = *sep;
            *sep = 0;
        }
        if (*(domain = trim(domain)) == 0) {
            return 0;
        }
        domain_len = strlen(domain);
        if (domain_len > 255U) {
            free(*domains_p);
            *domains_p = NULL;
            return -1;
        }
        if ((tmp = realloc(*domains_p, domains_size + 1U + domain_len + 1U)) == NULL) {
            if (sep != NULL) {
                *sep = sc;
            }
            free(*domains_p);
            *domains_p = NULL;
            return -1;
        }
        *domains_p = tmp;
        *((*domains_p) + domains_size) = (char) domain_len;
        memcpy((*domains_p) + 1U + domains_size, domain, domain_len);
        domains_size += 1U + domain_len;
        *((*domains_p) + domains_size) = (char) 0;
        domain += domain_len;
        if (sep != NULL) {
            domain++;
            *sep = sc;
        }
    }
    return 0;
}

static int
parse_resolver_list(Forwarder * const forwarder, char * const str)
{
    ldns_rdf *rdf;
    char     *resolver = str;
    char     *sep;
    char      sc;

    for (;;) {
        resolver = skip_spaces(resolver);
        sep = resolver;
        for (;;) {
            if (*sep == 0) {
                sep = NULL;
                break;
            }
            if (isspace((unsigned char) *sep)) {
                break;
            }
            sep++;
        }
        if (sep != NULL) {
            sc = *sep;
            *sep = 0;
        }
        if (*(resolver = trim(resolver)) == 0) {
            return 0;
        }
        if ((rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_AAAA, resolver)) == NULL &&
            (rdf = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, resolver)) == NULL) {
            if (sep != NULL) {
                *sep = sc;
            }
            return -1;
        }
        (void) ldns_resolver_push_nameserver(forwarder->resolver, rdf);
        ldns_rdf_deep_free(rdf);
        resolver += strlen(resolver);
        if (sep != NULL) {
            resolver++;
            *sep = sc;
        }
    }
}

static _Bool
has_matching_domain(const char *domains, const char *qname)
{
    const char *scanned = domains;
    size_t      scanned_len;
    size_t      qname_len;

    if (domains == NULL || *domains == 0 || (qname_len = strlen(qname)) < 2U) {
        return 0;
    }
    qname_len--;
    while ((scanned_len = (size_t) *scanned) != 0U) {
        scanned++;
        if (scanned_len > qname_len ||
            strncasecmp(scanned, qname + qname_len - scanned_len, scanned_len) != 0) {
            scanned += scanned_len;
            continue;
        }
        if (scanned_len == qname_len ||
            *(qname + qname_len - scanned_len - 1U) == '.') {
            return 1;
        }
        scanned += scanned_len;
    }
    return 0;
}

int
dcplugin_init(DCPlugin * const dcplugin, int argc, char *argv[])
{
    Forwarder *forwarder;
    char      *resolvers = NULL;
    int        opt_flag;
    int        option_index = 0;

    forwarder = calloc(1U, sizeof *forwarder);
    forwarder->domains = NULL;
    forwarder->resolver = NULL;
    dcplugin_set_user_data(dcplugin, forwarder);
    if (forwarder == NULL) {
        return -1;
    }
    optind = 0;
#ifdef _OPTRESET
    optreset = 1;
#endif
    while ((opt_flag = getopt_long(argc, argv,
                                   getopt_options, getopt_long_options,
                                   &option_index)) != -1) {
        switch (opt_flag) {
        case 'd':
            if (parse_domain_list(&forwarder->domains, optarg) != 0) {
                return -1;
            }
            break;
        case 'r':
            resolvers = optarg;
            break;
        default:
            return -1;
        }
    }
    if (forwarder->domains == NULL || resolvers == NULL) {
        return -1;
    }
    if ((forwarder->resolver = ldns_resolver_new()) == NULL) {
        return -1;
    }
    if (parse_resolver_list(forwarder, resolvers) != 0) {
        ldns_resolver_deep_free(forwarder->resolver);
        forwarder->resolver = NULL;
        return -1;
    }
    ldns_resolver_set_retry(forwarder->resolver, 1);
    ldns_resolver_set_timeout(forwarder->resolver, (struct timeval) {
        .tv_sec = UPSTREAM_TIMEOUT_SEC, .tv_usec = 0
    });
    return 0;
}

int
dcplugin_destroy(DCPlugin *dcplugin)
{
    Forwarder *forwarder = dcplugin_get_user_data(dcplugin);

    if (forwarder == NULL) {
        return 0;
    }
    if (forwarder->resolver != NULL) {
        ldns_resolver_deep_free(forwarder->resolver);
        forwarder->resolver = NULL;
    }
    free(forwarder->domains);
    forwarder->domains = NULL;
    free(forwarder);

    return 0;
}

DCPluginSyncFilterResult
dcplugin_sync_pre_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    Forwarder                *forwarder = dcplugin_get_user_data(dcplugin);
    char                     *owner_str;
    ldns_pkt                 *query = NULL;
    uint8_t                  *query_wire;
    ldns_pkt                 *response = NULL;
    uint8_t                  *response_wire = NULL;
    ldns_rr                  *question;
    ldns_rr_list             *questions;
    size_t                    response_wire_len;
    DCPluginSyncFilterResult  result = DCP_SYNC_FILTER_RESULT_OK;
    int                       i;
    _Bool                     has_reachable_ns = 0;

    query_wire = dcplugin_get_wire_data(dcp_packet);
    if (ldns_wire2pkt(&query, query_wire, dcplugin_get_wire_data_len(dcp_packet))
        != LDNS_STATUS_OK) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    questions = ldns_pkt_question(query);
    if (ldns_rr_list_rr_count(questions) != (size_t) 1U) {
        ldns_pkt_free(query);
        return DCP_SYNC_FILTER_RESULT_OK;
    }
    question = ldns_rr_list_rr(questions, 0U);
    if ((owner_str = ldns_rdf2str(ldns_rr_owner(question))) == NULL) {
        ldns_pkt_free(query);        
        return DCP_SYNC_FILTER_RESULT_FATAL;
    }
    if (has_matching_domain(forwarder->domains, owner_str) == 0) {
        free(owner_str);
        ldns_pkt_free(query);        
        return DCP_SYNC_FILTER_RESULT_OK;
    }
    free(owner_str);
    owner_str = NULL;
    /* If all nameservers have been marked as unreachable, reset them and try again */
    for (i = 0; i < ldns_resolver_nameserver_count(forwarder->resolver); i++) {
        if (ldns_resolver_nameserver_rtt(forwarder->resolver, i) != LDNS_RESOLV_RTT_INF) {
            has_reachable_ns = 1;
            break;
        }
    }
    if (!has_reachable_ns) {
        for (i = 0; i < ldns_resolver_nameserver_count(forwarder->resolver); i++) {
            ldns_resolver_set_nameserver_rtt(forwarder->resolver, i, LDNS_RESOLV_RTT_MIN);
        }
    }
    if (ldns_send(&response, forwarder->resolver, query) != LDNS_STATUS_OK) {
        ldns_pkt_free(query);
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    ldns_pkt_free(query);
    if (ldns_pkt2wire(&response_wire, response, &response_wire_len)
        != LDNS_STATUS_OK) {
        ldns_pkt_free(response);
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    ldns_pkt_free(response);
    if (response_wire_len > dcplugin_get_wire_data_max_len(dcp_packet)) {
        free(response_wire);
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    dcplugin_set_wire_data(dcp_packet, response_wire, response_wire_len);
    free(response_wire);

    return DCP_SYNC_FILTER_RESULT_DIRECT;
}

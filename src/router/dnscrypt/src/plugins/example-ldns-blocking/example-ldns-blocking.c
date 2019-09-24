
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netinet/in.h>
#endif

#include <dnscrypt/plugin.h>
#include <ldns/ldns.h>

#ifdef _MSC_VER
# define strncasecmp _strnicmp
# define strcasecmp _stricmp
#endif

#include "fpst.h"

DCPLUGIN_MAIN(__FILE__);

#ifndef putc_unlocked
# define putc_unlocked(c, stream) putc((c), (stream))
#endif

#define MAX_QNAME_LENGTH 255U

typedef enum BlockType {
    BLOCKTYPE_UNDEFINED,
    BLOCKTYPE_EXACT,
    BLOCKTYPE_PREFIX,
    BLOCKTYPE_SUFFIX,
    BLOCKTYPE_SUBSTRING
} BlockType;

typedef struct Blocking_ {
    FPST *domains;
    FPST *domains_rev;
    FPST *domains_substr;
    FPST *ips;
    FILE *fp;
    _Bool ltsv;
} Blocking;

static struct option getopt_long_options[] = {
    { "domains", 1, NULL, 'd' },
    { "ips", 1, NULL, 'i' },
    { "logfile", 1, NULL, 'l' },
    { NULL, 0, NULL, 0 }
};
static const char *getopt_options = "dil";

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

static void
str_tolower(char *str)
{
    while (*str != 0) {
        *str = (char) tolower((unsigned char) *str);
        str++;
    }
}

static void
str_reverse(char *str)
{
    size_t i = 0;
    size_t j = strlen(str);
    char   t;

    while (i < j) {
        t = str[i];
        str[i++] = str[--j];
        str[j] = t;
    }
}

static char *
untab(char *line)
{
    char *ptr;

    while ((ptr = strchr(line, '\t')) != NULL) {
        *ptr = ' ';
    }
    return line;
}

static char *
trim_comments(char *line)
{
    char *ptr;
    char *s1;
    char *s2;

    while ((ptr = strchr(line, '\n')) != NULL ||
           (ptr = strchr(line, '\r')) != NULL) {
        *ptr = 0;
    }
    line = skip_spaces(line);
    if (*line == 0 || *line == '#') {
        return NULL;
    }
    s1 = skip_chars(line);
    if (*(s2 = skip_spaces(s1)) == 0) {
        *s1 = 0;
        return line;
    }
    if (*s2 == '#') {
        return NULL;
    }
    *skip_chars(s2) = 0;

    return s2;
}

static void
free_list(const char *key, uint32_t val)
{
    (void) val;
    free((void *) key);
}

static int
parse_domain_list(FPST ** const domain_list_p,
                  FPST ** const domain_rev_list_p,
                  FPST ** const domain_substr_list_p,
                  const char * const file)
{
    char       buf[MAX_QNAME_LENGTH + 1U];
    char      *line;
    FILE      *fp;
    char      *ptr;
    FPST      *domain_list;
    FPST      *domain_list_tmp;
    FPST      *domain_rev_list;
    FPST      *domain_rev_list_tmp;
    FPST      *domain_substr_list;
    FPST      *domain_substr_list_tmp;
    size_t     line_len;
    BlockType  block_type = BLOCKTYPE_UNDEFINED;
    int        ret = -1;

    assert(domain_list_p != NULL);
    assert(domain_rev_list_p != NULL);
    assert(domain_substr_list_p != NULL);
    *domain_list_p = NULL;
    *domain_rev_list_p = NULL;
    *domain_substr_list_p = NULL;
    domain_list = fpst_new();
    domain_rev_list = fpst_new();
    domain_substr_list = fpst_new();
    if ((fp = fopen(file, "r")) == NULL) {
        return -1;
    }
    while (fgets(buf, (int) sizeof buf, fp) != NULL) {
        if ((line = trim_comments(untab(buf))) == NULL || *line == 0) {
            continue;
        }
        line_len = strlen(line);
        if (line[0] == '*' && line[line_len - 1] == '*') {
            line[line_len - 1] = 0;
            line++;
            block_type = BLOCKTYPE_SUBSTRING;
        } else if (line[line_len - 1] == '*') {
            line[line_len - 1] = 0;
            block_type = BLOCKTYPE_PREFIX;
        } else {
            if (line[0] == '*') {
                line++;
            }
            if (line[0] == '.') {
                line++;
            }
            str_reverse(line);
            block_type = BLOCKTYPE_SUFFIX;
        }
        if (*line == 0) {
            continue;
        }
        str_tolower(line);
        if ((line = strdup(line)) == NULL) {
            break;
        }
        if (block_type == BLOCKTYPE_SUFFIX) {
            if ((domain_rev_list_tmp = fpst_insert_str(domain_rev_list, line,
                                                       (uint32_t) block_type)) == NULL) {
                free(line);
                break;
            }
            domain_rev_list = domain_rev_list_tmp;
        } else if (block_type == BLOCKTYPE_PREFIX) {
            if ((domain_list_tmp = fpst_insert_str(domain_list, line,
                                                   (uint32_t) block_type)) == NULL) {
                free(line);
                break;
            }
            domain_list = domain_list_tmp;
        } else if (block_type == BLOCKTYPE_SUBSTRING) {
            if ((domain_substr_list_tmp = fpst_insert_str(domain_substr_list, line,
                                                          (uint32_t) block_type)) == NULL) {
                free(line);
                break;
            }
            domain_substr_list = domain_substr_list_tmp;
        } else {
            free(line);
        }
    }
    if (!feof(fp)) {
        fpst_free(domain_list, free_list);
        fpst_free(domain_rev_list, free_list);
        fpst_free(domain_substr_list, free_list);
    } else {
        *domain_list_p = domain_list;
        *domain_rev_list_p = domain_rev_list;
        *domain_substr_list_p = domain_substr_list;
        ret = 0;
    }
    fclose(fp);

    return ret;
}

static int
parse_ip_list(FPST ** const ip_list_p, const char * const file)
{
    char       buf[MAX_QNAME_LENGTH + 1U];
    char      *line;
    FILE      *fp;
    char      *ptr;
    FPST      *ip_list;
    FPST      *ip_list_tmp;
    size_t     line_len;
    BlockType  block_type = BLOCKTYPE_UNDEFINED;
    int        ret = -1;

    assert(ip_list_p != NULL);
    *ip_list_p = NULL;
    ip_list = fpst_new();
    if ((fp = fopen(file, "r")) == NULL) {
        return -1;
    }
    while (fgets(buf, (int) sizeof buf, fp) != NULL) {
        if ((line = trim_comments(untab(buf))) == NULL || *line == 0) {
            continue;
        }
        line_len = strlen(line);
        if (line[line_len - 1] == '*') {
            line[line_len - 1] = 0;
            block_type = BLOCKTYPE_PREFIX;
        } else {
            block_type = BLOCKTYPE_EXACT;
        }
        str_tolower(line);
        if ((line = strdup(line)) == NULL) {
            break;
        }
        if ((ip_list_tmp = fpst_insert_str(ip_list, line,
                                           (uint32_t) block_type)) == NULL) {
            free(line);
            break;
        }
        ip_list = ip_list_tmp;
    }
    if (!feof(fp)) {
        fpst_free(ip_list, free_list);
    } else {
        *ip_list_p = ip_list;
        ret = 0;
    }
    fclose(fp);

    return ret;
}

static _Bool
substr_match(FPST *list, const char *str,
             const char **found_key_p, uint32_t *found_block_type_p)
{
    size_t i;

    while (*str != 0) {
        if (fpst_str_starts_with_existing_key(list, str, found_key_p,
                                              found_block_type_p)) {
            return 1;
        }
        str++;
    }
    return 0;
}

const char *
dcplugin_description(DCPlugin * const dcplugin)
{
    return "Block specific domains and IP addresses";
}

const char *
dcplugin_long_description(DCPlugin * const dcplugin)
{
    return
        "This plugin returns a REFUSED response if the query name is in a\n"
        "list of blacklisted names, or if at least one of the returned IP\n"
        "addresses happens to be in a list of blacklisted IPs.\n"
        "\n"
        "Plugin parameters:\n"
        "--domains=<file>\n"
        "--ips=<file>\n"
        "--logfile=[ltsv:]<file>\n"
        "\n"
        "A file should list one entry per line.\n"
        "\n"
        "IPv4 and IPv6 addresses are supported.\n"
        "For names, leading and trailing wildcards (*) are also supported\n"
        "(e.g. *xxx*, *.example.com, ads.*)\n"
        "\n"
        "For IP addresses, trailing wildcards are supported\n"
        "(e.g. 192.168.*, 10.0.0.*)\n"
        "\n"
        "# dnscrypt-proxy --plugin \\\n"
        "  libdcplugin_example_ldns_blocking.la,--ips=/etc/blk-ips,--domains=/etc/blk-names"
        "\n"
        "By default, logs are written in a human-readable format.\n"
        "Prepending ltsv: to the file name changes the log format to LTSV.";
}

int
dcplugin_init(DCPlugin * const dcplugin, int argc, char *argv[])
{
    Blocking *blocking;
    int       opt_flag;
    int       option_index = 0;

    if ((blocking = calloc((size_t) 1U, sizeof *blocking)) == NULL) {
        return -1;
    }
    dcplugin_set_user_data(dcplugin, blocking);
    blocking->fp = NULL;
    blocking->domains = NULL;
    blocking->domains_rev = NULL;
    blocking->domains_substr = NULL;
    blocking->ips = NULL;
    blocking->ltsv = 0;
    optind = 0;
#ifdef _OPTRESET
    optreset = 1;
#endif
    while ((opt_flag = getopt_long(argc, argv,
                                   getopt_options, getopt_long_options,
                                   &option_index)) != -1) {
        switch (opt_flag) {
        case 'd':
            if (parse_domain_list(&blocking->domains, &blocking->domains_rev,
                                  &blocking->domains_substr, optarg) != 0) {
                return -1;
            }
            break;
        case 'i':
            if (parse_ip_list(&blocking->ips, optarg) != 0) {
                return -1;
            }
            break;
        case 'l':
            if (strncmp(optarg, "ltsv:", (sizeof "ltsv:") - 1U) == 0) {
                blocking->ltsv = 1;
                optarg += (sizeof "ltsv:") - 1U;
            }
            if ((blocking->fp = fopen(optarg, "a")) == NULL) {
                return -1;
            }
            break;
        default:
            return -1;
        }
    }
    return 0;
}

int
dcplugin_destroy(DCPlugin * const dcplugin)
{
    Blocking *blocking = dcplugin_get_user_data(dcplugin);

    if (blocking == NULL) {
        return 0;
    }
    fpst_free(blocking->domains, free_list);
    blocking->domains = NULL;
    fpst_free(blocking->domains_rev, free_list);
    blocking->domains_rev = NULL;
    fpst_free(blocking->domains_substr, free_list);
    blocking->domains_substr = NULL;
    fpst_free(blocking->ips, free_list);
    blocking->ips = NULL;
    if (blocking->fp != NULL) {
        fclose(blocking->fp);
        blocking->fp = NULL;
    }
    free(blocking);

    return 0;
}

static int
ltsv_prop(FILE * const fp, const char * const prop,
          const Blocking * const blocking)
{
    if (blocking->ltsv == 0) {
        return 0;
    }
    fprintf(fp, "%s:", prop);

    return 0;
}

static int
timestamp_fprint(FILE * const fp, _Bool unix_ts)
{
    char now_s[128];

    time_t     now;
    struct tm *tm;

    if (time(&now) == (time_t) -1) {
        putc_unlocked('-', fp);
        return -1;
    }
    if (unix_ts) {
        fprintf(fp, "%lu", (unsigned long) now);
    } else {
        tm = localtime(&now);
        strftime(now_s, sizeof now_s, "%c", tm);
        fprintf(fp, "%s", now_s);
    }
    return 0;
}

static void
util_ntohl(uint32_t * const xp)
{
    uint8_t p[4U];

    memcpy(p, xp, 4U);
    *xp = (((uint32_t) p[0]) << 24) | (((uint32_t) p[1]) << 16) |
          (((uint32_t) p[2]) <<  8) | (((uint32_t) p[3]));
}

static int
ip_fprint(FILE * const fp,
          const struct sockaddr_storage * const client_addr,
          const size_t client_addr_len)
{
    if (client_addr->ss_family == AF_INET) {
        struct sockaddr_in in;
        uint32_t           a;

        memcpy(&in, client_addr, sizeof in);
        a = (uint32_t) in.sin_addr.s_addr;
        util_ntohl(&a);
        fprintf(fp, "%u.%u.%u.%u",
                (a >> 24) & 0xff, (a >> 16) & 0xff,
                (a >> 8) & 0xff, a  & 0xff);
    } else if (client_addr->ss_family == AF_INET6) {
        struct sockaddr_in6  in6;
        const unsigned char *a;
        int                  i;
        uint16_t             w;
        _Bool                blanks;

        memcpy(&in6, client_addr, sizeof in6);
        a = in6.sin6_addr.s6_addr;
        blanks = (a[0] | a[1]) == 0;
        putc_unlocked('[', fp);
        for (i = 0; i < 16; i += 2) {
            w = ((uint16_t) a[i] << 8) | (uint16_t) a[i + 1];
            if (blanks) {
                if (w == 0U) {
                    continue;
                }
                putc_unlocked(':', fp);
                blanks = 0;
            }
            if (i != 0) {
                putc_unlocked(':', fp);
            }
            if (blanks == 0) {
                fprintf(fp, "%x", (unsigned int) w);
            }
        }
        putc_unlocked(']', fp);
    } else {
        putc_unlocked('-', fp);
    }
    return 0;
}

static int
log_blocked_rr(const Blocking * const blocking,
               const char * const blocked_question,
               const char * const rule, BlockType block_type,
               const struct sockaddr_storage * const client_addr,
               const size_t client_addr_len)
{
    if (blocking->fp == NULL) {
        return 0;
    }
    ltsv_prop(blocking->fp, "time", blocking);
    timestamp_fprint(blocking->fp, blocking->ltsv);
    putc_unlocked('\t', blocking->fp);
    ltsv_prop(blocking->fp, "host", blocking);
    ip_fprint(blocking->fp, client_addr, client_addr_len);
    putc_unlocked('\t', blocking->fp);
    ltsv_prop(blocking->fp, "qname", blocking);
    fprintf(blocking->fp, "%s\t", blocked_question);
    ltsv_prop(blocking->fp, "message", blocking);
    switch (block_type) {
    case BLOCKTYPE_PREFIX:
        fprintf(blocking->fp, "%s*\n", rule);
        break;
    case BLOCKTYPE_SUFFIX:
        fprintf(blocking->fp, "*%s\n", rule);
        break;
    case BLOCKTYPE_SUBSTRING:
        fprintf(blocking->fp, "*%s*\n", rule);
        break;
    default:
        fprintf(blocking->fp, "%s\n", rule);
        break;
    }
    fflush(blocking->fp);

    return 0;
}

void
str_lcpy(char *dst, const char *src, size_t dsize)
{
    size_t nleft = dsize;

    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == 0) {
                break;
            }
        }
    }
    if (nleft == 0 && dsize != 0) {
        *dst = 0;
    }
}

static DCPluginSyncFilterResult
apply_block_domains(DCPluginDNSPacket *dcp_packet, Blocking * const blocking,
                    ldns_pkt * const packet)
{
    char                      rev[MAX_QNAME_LENGTH + 1U];
    ldns_rr                  *question;
    ldns_rr_list             *questions;
    char                     *owner_str;
    const char               *found_key;
    uint8_t                  *wire_data;
    size_t                    owner_str_len;
    uint32_t                  found_block_type;
    DCPluginSyncFilterResult  result = DCP_SYNC_FILTER_RESULT_OK;
    _Bool                     block = 0;

    rev[MAX_QNAME_LENGTH] = 0;
    questions = ldns_pkt_question(packet);
    if (ldns_rr_list_rr_count(questions) != (size_t) 1U) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    question = ldns_rr_list_rr(questions, 0U);
    if ((owner_str = ldns_rdf2str(ldns_rr_owner(question))) == NULL) {
        return DCP_SYNC_FILTER_RESULT_FATAL;
    }
    owner_str_len = strlen(owner_str);
    if (owner_str_len >= sizeof rev) {
        free(owner_str);
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    if (owner_str_len > (size_t) 1U && owner_str[owner_str_len - 1U] == '.') {
        owner_str[--owner_str_len] = 0;
    }
    if (owner_str_len <= 0) {
        free(owner_str);
        return DCP_SYNC_FILTER_RESULT_OK;
    }
    str_tolower(owner_str);
    do {
        str_lcpy(rev, owner_str, sizeof rev);
        str_reverse(rev);
        if (fpst_starts_with_existing_key(blocking->domains_rev,
                                          rev, owner_str_len,
                                          &found_key, &found_block_type)) {
            const size_t found_key_len = strlen(found_key);

            assert(found_block_type == BLOCKTYPE_SUFFIX);
            if (found_key_len <= owner_str_len &&
                (rev[found_key_len] == 0 || rev[found_key_len] == '.')) {
                block = 1;
                break;
            }
            if (found_key_len < owner_str_len) {
                size_t owner_part_len = owner_str_len;

                while (owner_part_len > 0U && rev[owner_part_len] != '.') {
                    owner_part_len--;
                }
                rev[owner_part_len] = 0;
                if (owner_part_len > 0U && fpst_starts_with_existing_key
                    (blocking->domains_rev, rev, owner_part_len,
                     &found_key, &found_block_type)) {
                    const size_t found_key_len = strlen(found_key);
                    if (found_key_len <= owner_part_len &&
                        (rev[found_key_len] == 0 || rev[found_key_len] == '.')) {
                        block = 1;
                        break;
                    }
                }
            }
        }
        if (fpst_starts_with_existing_key(blocking->domains,
                                          owner_str, owner_str_len,
                                          &found_key, &found_block_type)) {
            assert(found_block_type == BLOCKTYPE_PREFIX);
            block = 1;
            break;
        }
        if (blocking->domains_substr != NULL &&
            substr_match(blocking->domains_substr, owner_str,
                         &found_key, &found_block_type)) {
            assert(found_block_type == BLOCKTYPE_SUBSTRING);
            block = 1;
            break;
        }
    } while (0);
    if (block) {
        wire_data = dcplugin_get_wire_data(dcp_packet);
        LDNS_QR_SET(wire_data);
        LDNS_RA_SET(wire_data);
        LDNS_RCODE_SET(wire_data, LDNS_RCODE_REFUSED);
        result = DCP_SYNC_FILTER_RESULT_DIRECT;
        if (found_block_type == BLOCKTYPE_SUFFIX) {
            str_lcpy(rev, found_key, sizeof rev);
            str_reverse(rev);
            log_blocked_rr(blocking, owner_str, rev, found_block_type,
                           dcplugin_get_client_address(dcp_packet),
                           dcplugin_get_client_address_len(dcp_packet));
        } else {
            log_blocked_rr(blocking, owner_str, found_key, found_block_type,
                           dcplugin_get_client_address(dcp_packet),
                           dcplugin_get_client_address_len(dcp_packet));
        }
    }
    free(owner_str);

    return result;
}

static DCPluginSyncFilterResult
apply_block_ips(DCPluginDNSPacket *dcp_packet, Blocking * const blocking,
                ldns_pkt * const packet)
{
    ldns_rr_list *answers;
    ldns_rr      *answer;
    const char   *found_key;
    char         *answer_str;
    ldns_rr_type  type;
    uint32_t      found_block_type;
    size_t        answers_count;
    size_t        i;

    answers = ldns_pkt_answer(packet);
    answers_count = ldns_rr_list_rr_count(answers);
    for (i = (size_t) 0U; i < answers_count; i++) {
        answer = ldns_rr_list_rr(answers, i);
        type = ldns_rr_get_type(answer);
        if (type != LDNS_RR_TYPE_A && type != LDNS_RR_TYPE_AAAA) {
            continue;
        }
        if ((answer_str = ldns_rdf2str(ldns_rr_a_address(answer))) == NULL) {
            return DCP_SYNC_FILTER_RESULT_FATAL;
        }
        if (type == LDNS_RR_TYPE_AAAA) {
            struct in_addr i4;
            struct sockaddr_in6 si6;
            struct sockaddr_storage *ss;
            size_t ss_len;

            ss = ldns_rdf2native_sockaddr_storage(ldns_rr_a_address(answer), 0, &ss_len);
            if (ss == NULL || ss_len > sizeof si6) {
                return DCP_SYNC_FILTER_RESULT_FATAL;
            }
            memcpy(&si6, ss, ss_len);
            if (IN6_IS_ADDR_V4MAPPED(&si6.sin6_addr)) {
                free(answer_str);
                memcpy(&i4, 12 + (unsigned char *) &si6.sin6_addr.s6_addr, sizeof i4);
                if ((answer_str = strdup(inet_ntoa(i4))) == NULL) {
                    return DCP_SYNC_FILTER_RESULT_FATAL;
                }
            }
            free(ss);
        }
        str_tolower(answer_str);
        if (fpst_str_starts_with_existing_key(blocking->ips, answer_str,
                                              &found_key, &found_block_type)) {
            if (found_block_type == BLOCKTYPE_PREFIX ||
                strlen(found_key) == strlen(answer_str)) {
                LDNS_RCODE_SET(dcplugin_get_wire_data(dcp_packet),
                               LDNS_RCODE_REFUSED);
                if (blocking->fp != NULL) {
                    ldns_rr      *question;
                    ldns_rr_list *questions;
                    char         *owner_str;
                    size_t        owner_str_len;

                    questions = ldns_pkt_question(packet);
                    if (ldns_rr_list_rr_count(questions) != (size_t) 1U) {
                        free(answer_str);
                        return DCP_SYNC_FILTER_RESULT_ERROR;
                    }
                    question = ldns_rr_list_rr(questions, 0U);
                    if ((owner_str = ldns_rdf2str(ldns_rr_owner(question))) == NULL) {
                        free(answer_str);
                        return DCP_SYNC_FILTER_RESULT_FATAL;
                    }
                    owner_str_len = strlen(owner_str);
                    if (owner_str_len > (size_t) 1U && owner_str[owner_str_len - 1U] == '.') {
                        owner_str[--owner_str_len] = 0;
                    }
                    log_blocked_rr(blocking, owner_str, found_key, found_block_type,
                                   dcplugin_get_client_address(dcp_packet),
                                   dcplugin_get_client_address_len(dcp_packet));
                    free(owner_str);
                }
                free(answer_str);
                answer_str = NULL;
                break;
            }
        }
        free(answer_str);
    }
    return DCP_SYNC_FILTER_RESULT_OK;
}

DCPluginSyncFilterResult
dcplugin_sync_pre_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    Blocking                 *blocking = dcplugin_get_user_data(dcplugin);
    ldns_pkt                 *packet = NULL;
    DCPluginSyncFilterResult  result = DCP_SYNC_FILTER_RESULT_OK;

    if (blocking->domains == NULL && blocking->domains_rev == NULL &&
        blocking->domains_substr == NULL) {
        return DCP_SYNC_FILTER_RESULT_OK;
    }
    if (ldns_wire2pkt(&packet, dcplugin_get_wire_data(dcp_packet),
                      dcplugin_get_wire_data_len(dcp_packet))
        != LDNS_STATUS_OK) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    if ((result = apply_block_domains(dcp_packet, blocking, packet))
         != DCP_SYNC_FILTER_RESULT_OK) {
        ldns_pkt_free(packet);
        return result;
    }
    ldns_pkt_free(packet);

    return DCP_SYNC_FILTER_RESULT_OK;
}

DCPluginSyncFilterResult
dcplugin_sync_post_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    Blocking                 *blocking = dcplugin_get_user_data(dcplugin);
    ldns_pkt                 *packet = NULL;
    DCPluginSyncFilterResult  result = DCP_SYNC_FILTER_RESULT_OK;

    if (blocking->ips == NULL) {
        return DCP_SYNC_FILTER_RESULT_OK;
    }
    if (ldns_wire2pkt(&packet, dcplugin_get_wire_data(dcp_packet),
                      dcplugin_get_wire_data_len(dcp_packet))
        != LDNS_STATUS_OK) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    if (blocking->ips != NULL &&
        (result = apply_block_ips(dcp_packet, blocking, packet))
         != DCP_SYNC_FILTER_RESULT_OK) {
        ldns_pkt_free(packet);
        return result;
    }
    ldns_pkt_free(packet);

    return DCP_SYNC_FILTER_RESULT_OK;
}

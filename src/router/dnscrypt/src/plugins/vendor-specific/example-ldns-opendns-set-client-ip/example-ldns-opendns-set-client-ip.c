
#include <sys/types.h>
#ifndef _WIN32
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
# include <ws2tcpip.h>
#endif

#include <dnscrypt/plugin.h>
#include <ldns/ldns.h>
#include <ldns/util.h>

DCPLUGIN_MAIN(__FILE__);

#define EDNS_HEADER           "4f56" "0014" "4f444e5300" "00"
#define EDNS_HEADER_CLIENT_IP "10"
#define EDNS_CLIENT_IP        "7f000001"
#define EDNS_HEADER_FODDER    "40"
#define EDNS_FODDER           "deadbeefabad1dea"

#define EDNS_DATA EDNS_HEADER \
    EDNS_HEADER_CLIENT_IP EDNS_CLIENT_IP EDNS_HEADER_FODDER EDNS_FODDER

#define EDNS_CLIENT_IP_OFFSET (sizeof EDNS_HEADER - 1U + \
                               sizeof EDNS_HEADER_CLIENT_IP - 1U)

#define EDNS_FODDER_OFFSET (sizeof EDNS_HEADER - 1U + \
                            sizeof EDNS_HEADER_CLIENT_IP - 1U + \
                            sizeof EDNS_CLIENT_IP - 1U + \
                            sizeof EDNS_HEADER_FODDER - 1U)

const char *
dcplugin_description(DCPlugin * const dcplugin)
{
    return "Apply the OpenDNS settings defined for a specific IP address";
}

const char *
dcplugin_long_description(DCPlugin * const dcplugin)
{
    return
        "The IP address must be an IPv4 address.\n"
        "\n"
        "Usage:\n"
        "\n"
        "# dnscrypt-proxy --plugin \\\n"
        "  libdcplugin_example_ldns_opendns_set_client_ip.la,192.30.252.130";
}

static int
_inet_pton(const int af, const char * const src, void * const dst)
{
    unsigned char  *dstc;
    unsigned int    a, b, c, d;
    char            more;

    if (af != AF_INET) {
        errno = EAFNOSUPPORT;
        return -1;
    }
    if (sscanf(src, "%u.%u.%u.%u%c", &a, &b, &c, &d, &more) != 4) {
        return 0;
    }
    if (a > 0xff || b > 0xff || c > 0xff || d > 0xff) {
        return 0;
    }
    dstc = (unsigned char *) dst;
    assert(sizeof(struct in_addr) >= 4U);
    dstc[0] = (unsigned char) a;
    dstc[1] = (unsigned char) b;
    dstc[2] = (unsigned char) c;
    dstc[3] = (unsigned char) d;

    return 1;
}

static void
ip4tohex(char hex[4 * 2 + 1], unsigned char ip[4])
{
    static const char HEX[16] = "0123456789ABCDEF";
    size_t            i;

    for (i = 0U; i < 4U; i++) {
        hex[i * 2U] = HEX[(ip[i] >> 4) & 0xf];
        hex[i * 2U + 1U] = HEX[ip[i] & 0xf];
    }
    hex[i * 2U] = 0;
}

static int
parse_client_ip(const char *ip_s, char * const edns_hex)
{
    char            ip_hex[8U + 1U];
    struct in_addr  ip_in_addr;
    unsigned char  *sa;
    const size_t    ip_s_len = strlen(ip_s);

    if (ip_s_len <= INET_ADDRSTRLEN && strchr(ip_s, '.') != NULL &&
        _inet_pton(AF_INET, ip_s, &ip_in_addr) > 0) {
        sa = (unsigned char *) &ip_in_addr.s_addr;
        ip4tohex(ip_hex, sa);
        memcpy(edns_hex + EDNS_CLIENT_IP_OFFSET,
               ip_hex, sizeof EDNS_CLIENT_IP - 1U);
    } else if (ip_s_len == sizeof EDNS_CLIENT_IP - 1U) {
        memcpy(edns_hex + EDNS_CLIENT_IP_OFFSET,
               ip_s, sizeof EDNS_CLIENT_IP - 1U);
    } else {
        return -1;
    }
    return 0;
}

int
dcplugin_init(DCPlugin * const dcplugin, int argc, char *argv[])
{
    char   *edns_hex;
    size_t  edns_hex_size = sizeof EDNS_DATA;

    ldns_init_random(NULL, 0U);
    edns_hex = malloc(edns_hex_size);
    dcplugin_set_user_data(dcplugin, edns_hex);
    if (edns_hex == NULL) {
        return -1;
    }
    memcpy(edns_hex, EDNS_DATA, edns_hex_size);
    assert(sizeof EDNS_CLIENT_IP - 1U == (size_t) 8U);
    if (argc > 1 && argv[1] != NULL) {
        parse_client_ip(argv[1], edns_hex);
    }
    return 0;
}

int
dcplugin_destroy(DCPlugin *dcplugin)
{
    free(dcplugin_get_user_data(dcplugin));

    return 0;
}

static void
fill_with_random_hex_data(char * const str, size_t size)
{
    size_t   i = (size_t) 0U;
    uint16_t rnd;

    while (i < size) {
        rnd = ldns_get_random();
        str[i++] = "0123456789abcdef"[rnd & 0xf];
        str[i++] = "0123456789abcdef"[(rnd >> 8) & 0xf];
    }
}

DCPluginSyncFilterResult
dcplugin_sync_pre_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    uint8_t  *new_packet;
    ldns_rdf *edns_data;
    char     *edns_data_str;
    ldns_pkt *packet = NULL;
    size_t    new_packet_size;

    if (ldns_wire2pkt(&packet, dcplugin_get_wire_data(dcp_packet),
                      dcplugin_get_wire_data_len(dcp_packet))
        != LDNS_STATUS_OK) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    edns_data_str = dcplugin_get_user_data(dcplugin);
    fill_with_random_hex_data(edns_data_str + EDNS_FODDER_OFFSET,
                              sizeof EDNS_FODDER - 1U);
    edns_data = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_HEX, edns_data_str);
    ldns_pkt_set_edns_data(packet, edns_data);

    if (ldns_pkt2wire(&new_packet, packet, &new_packet_size)
        != LDNS_STATUS_OK) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }
    if (dcplugin_get_wire_data_max_len(dcp_packet) >= new_packet_size) {
        dcplugin_set_wire_data(dcp_packet, new_packet, new_packet_size);
    }

    free(new_packet);
    ldns_pkt_free(packet);

    return DCP_SYNC_FILTER_RESULT_OK;
}

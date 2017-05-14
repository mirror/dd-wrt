
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
# include <ws2tcpip.h>
#endif

#include <dnscrypt/plugin.h>
#include <ldns/ldns.h>

DCPLUGIN_MAIN(__FILE__);

#define EDNS_HEADER "0004000f4f70656e444e53"
#define EDNS_DEV_ID "cafebabedeadbeef"

const char *
dcplugin_description(DCPlugin * const dcplugin)
{
    return "Add a Cisco Umbrella device identifier to outgoing queries";
}

const char *
dcplugin_long_description(DCPlugin * const dcplugin)
{
    return
        "This plugin tags outgoing packets with the 8 bytes password,\n"
        "that the Cisco Umbrella service uses to identify their users.\n"
        "\n"
        "If you happen to have an OpenDNS VPN or Cisco Umbrella account,\n"
        "your password ('device') can be displayed with:\n"
        "\n"
        "$ dig txt debug.opendns.com.\n"
        "\n"
        "# dnscrypt-proxy --plugin \\\n"
        "      libdcplugin_example_ldns_opendns_deviceid.la,/etc/umbrella-password.txt\n"
        "\n"
        "The password should be present in the given file, as a hexadecimal string\n"
        "(16 hex characters).\n";
}

static char *
load_device_id_from_file(const char * const file_name)
{
    FILE *fp;
    char *device_id;

    if ((fp = fopen(file_name, "r")) == NULL) {
        return NULL;
    }
    if ((device_id = calloc(1U, sizeof EDNS_DEV_ID)) == NULL) {
        return NULL;
    }
    if (fread(device_id, 1U, sizeof EDNS_DEV_ID, fp) !=
        sizeof EDNS_DEV_ID) {
        free(device_id);
        return NULL;
    }
    fclose(fp);

    return device_id;
}

int
dcplugin_init(DCPlugin * const dcplugin, int argc, char *argv[])
{
    char   *device_id = NULL;
    char   *device_id_env = NULL;
    char   *edns_hex;
    size_t  edns_hex_size = sizeof EDNS_HEADER EDNS_DEV_ID;

    edns_hex = malloc(sizeof EDNS_HEADER EDNS_DEV_ID);
    dcplugin_set_user_data(dcplugin, edns_hex);
    if (edns_hex == NULL) {
        return -1;
    }
    memcpy(edns_hex, EDNS_HEADER EDNS_DEV_ID, edns_hex_size);
    assert(sizeof EDNS_DEV_ID - 1U == (size_t) 16U);
    if (argc == 2U) {
        device_id = load_device_id_from_file(argv[1]);
    }
    if (device_id == NULL) {
        if ((device_id_env = getenv("OPENDNS_DEVICE_ID")) == NULL) {
            return -1;
        }
        if ((device_id = strdup(device_id_env)) == NULL) {
            return -1;
        }
    }
    if (device_id != NULL) {
        memcpy(edns_hex + sizeof EDNS_HEADER - (size_t) 1U,
               device_id, sizeof EDNS_DEV_ID);
        free(device_id);
    }
    if (device_id_env != NULL) {
        memset(device_id_env, 0, strlen(device_id_env));
    }
    return 0;
}

int
dcplugin_destroy(DCPlugin *dcplugin)
{
    free(dcplugin_get_user_data(dcplugin));

    return 0;
}

DCPluginSyncFilterResult
dcplugin_sync_pre_filter(DCPlugin *dcplugin, DCPluginDNSPacket *dcp_packet)
{
    uint8_t  *new_packet;
    ldns_rdf *edns_data;
    ldns_pkt *packet = NULL;
    size_t    new_packet_size;

    if (ldns_wire2pkt(&packet, dcplugin_get_wire_data(dcp_packet),
                      dcplugin_get_wire_data_len(dcp_packet))
        != LDNS_STATUS_OK) {
        return DCP_SYNC_FILTER_RESULT_ERROR;
    }

    edns_data = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_HEX,
                                     dcplugin_get_user_data(dcplugin));
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

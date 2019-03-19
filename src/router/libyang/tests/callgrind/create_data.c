#include <stdlib.h>
#include <valgrind/callgrind.h>

#include "libyang.h"
#include "tests/config.h"

#define SCHEMA TESTS_DIR "/callgrind/files/ietf-interfaces.yang"
#define SCHEMA2 TESTS_DIR "/callgrind/files/ietf-ip.yang"
#define SCHEMA3 TESTS_DIR "/callgrind/files/iana-if-type.yang"

int
main(void)
{
    int ret = 0;
    struct ly_ctx *ctx = NULL;
    struct lyd_node *data = NULL, *node;

    ctx = ly_ctx_new(NULL, 0);
    if (!ctx) {
        ret = 1;
        goto finish;
    }

    if (!lys_parse_path(ctx, SCHEMA, LYS_YANG)) {
        ret = 1;
        goto finish;
    }

    if (!lys_parse_path(ctx, SCHEMA2, LYS_YANG)) {
        ret = 1;
        goto finish;
    }

    if (!lys_parse_path(ctx, SCHEMA3, LYS_YANG)) {
        ret = 1;
        goto finish;
    }

    CALLGRIND_START_INSTRUMENTATION;
    data = lyd_new_path(NULL, ctx, "/ietf-interfaces:interfaces/interface[name='eth0']/ietf-ip:ipv4/ietf-ip:address[ietf-ip:ip='47.250.10.1']/ietf-ip:prefix-length", "16", LYD_ANYDATA_CONSTSTRING, 0);
    if (!data) {
        ret = 1;
        goto finish;
    }

    node = lyd_new_path(data, ctx, "/ietf-interfaces:interfaces/interface[name='eth0']/type", "iana-if-type:ethernetCsmacd", LYD_ANYDATA_CONSTSTRING, 0);
    if (!node) {
        ret = 1;
        goto finish;
    }

    node = lyd_new_path(data, ctx, "/ietf-interfaces:interfaces/interface[name='eth0']/ietf-ip:ipv6/ietf-ip:address[ietf-ip:ip='fec0::1']/ietf-ip:prefix-length", "48", LYD_ANYDATA_CONSTSTRING, 0);
    if (!node) {
        ret = 1;
        goto finish;
    }

    node = lyd_new_path(data, ctx, "/ietf-interfaces:interfaces/interface[name='eth0']/ietf-ip:ipv6/ietf-ip:mtu", "1500", LYD_ANYDATA_CONSTSTRING, 0);
    if (!node) {
        ret = 1;
        goto finish;
    }

    if (lyd_validate(&data, LYD_OPT_DATA | LYD_OPT_DATA_NO_YANGLIB, NULL)) {
        ret = 1;
        goto finish;
    }
    CALLGRIND_STOP_INSTRUMENTATION;

finish:
    lyd_free_withsiblings(data);
    ly_ctx_destroy(ctx, NULL);
    return ret;
}

/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdio.h>
#include <linux/if_ether.h>
#include <linux/if_bridge.h>
#include <asm/byteorder.h>

#include "mstp.h"
#include "common.h"

static const bpdu_t tcn_bpdu = {
    .protocolIdentifier = 0x0000,
    .protocolVersion = protoRSTP,
    .bpduType = 0x0,
    .flags = (1 << offsetTc),
    .MessageAge = { 0x1, 0x0 },
    .MaxAge = { 0x14, 0x0 },
    .HelloTime = { 0x2, 0x0 },
    .ForwardDelay = { 0xf, 0x0 },
};

/* Ensure receiving a TCN message on a port that is in alternate/discarding
 * does not result in an infinite loop in the Topology Change State Machine for
 * any defined MSTs.
 *
 * See ef0add3d78f4 ("TCSM: fix infinite recursion for MSTI per-tree ports")
 * for details. 
 */
void tcn_bpdu_from_alternate_discarding(void **state)
{
    port_t *br0p[2], *br1p[2];
    bridge_t *br0, *br1;

    alloc_bridge_ports(state, &br0, "br0", 0x200000000001, &br0p, 2);
    alloc_bridge_ports(state, &br1, "br1", 0x200000000002, &br1p, 2);

    link_ports(br0p[0], br1p[0]);
    link_ports(br0p[1], br1p[1]);

    assert_true(MSTP_IN_create_msti(br1, 1));

    MSTP_IN_set_bridge_enable(br0, true);
    MSTP_IN_set_bridge_enable(br1, true);

    set_port_state(br0p[0], true, 1000, true);
    set_port_state(br0p[1], true, 1000, true);

    for (int i = 0; i < 3; i++)
        test_one_second(state);

    CIST_BridgeStatus status_br0, status_br1;
    MSTP_IN_get_cist_bridge_status(br0, &status_br0);
    MSTP_IN_get_cist_bridge_status(br1, &status_br1);

    assert_uint_equal(status_br0.designated_root.u,
                      status_br1.designated_root.u);

    CIST_PortStatus port_status;

    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_int_equal(port_status.state, BR_STATE_FORWARDING);
    assert_int_equal(port_status.role, roleDesignated);

    MSTP_IN_get_cist_port_status(br0p[1], &port_status);
    assert_int_equal(port_status.state, BR_STATE_FORWARDING);
    assert_int_equal(port_status.role, roleDesignated);

    MSTP_IN_get_cist_port_status(br1p[0], &port_status);
    assert_int_equal(port_status.state, BR_STATE_FORWARDING);
    assert_int_equal(port_status.role, roleRoot);

    MSTP_IN_get_cist_port_status(br1p[1], &port_status);
    assert_int_equal(port_status.state, BR_STATE_BLOCKING);
    assert_int_equal(port_status.role, roleAlternate);

    port_rx_bpdu(br0p[1], &tcn_bpdu, RST_BPDU_SIZE);
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(tcn_bpdu_from_alternate_discarding, prepare_test, teardown_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

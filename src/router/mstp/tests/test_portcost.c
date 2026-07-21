/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdio.h>
#include <linux/if_bridge.h>
#include <linux/if_ether.h>

#include "mstp.h"
#include "common.h"

/* Test setting adminExternalPathCost */
static void admin_ext_portcost(void **state)
{
    port_t *br0p[2];
    bridge_t *br0;

    alloc_bridge_ports(state, &br0, "br0", 0x200000000001, &br0p, 2);

    MSTP_IN_set_bridge_enable(br0, true);

    set_port_state(br0p[0], true, 1000, true);
    set_port_state(br0p[1], true, 1000, true);

    CIST_PortStatus port_status;

    /* default should be 0 */
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    MSTP_IN_get_cist_port_status(br0p[1], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    /* set br0p1.adminPortPathCost to 40'000 */
    CIST_PortConfig port_config = {
       .set_admin_external_port_path_cost = true,
       .admin_external_port_path_cost = 40000,
    };
    MSTP_IN_set_cist_port_config(br0p[0], &port_config);

    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 40000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 40000);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    /* br0p2 should be unaffected */
    MSTP_IN_get_cist_port_status(br0p[1], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    /* reset br0p1.adminPortPathCost to 0 */
    port_config.admin_external_port_path_cost = 0;
    MSTP_IN_set_cist_port_config(br0p[0], &port_config);

    /* br0p2.portPathCost should be back to its default value */
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);
}

static void admin_int_portcost(void **state)
{
    port_t *br0p[2];
    bridge_t *br0;

    alloc_bridge_ports(state, &br0, "br0", 0x200000000001, &br0p, 2);

    MSTP_IN_set_bridge_enable(br0, true);

    set_port_state(br0p[0], true, 1000, true);
    set_port_state(br0p[1], true, 1000, true);

    CIST_PortStatus port_status;

    /* default should be 0 */
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    MSTP_IN_get_cist_port_status(br0p[1], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    /* set br0p1.ist.adminInternalPortPathCost to 40'000 */
    MSTI_PortConfig port_config = {
        .set_admin_internal_port_path_cost = true,
        .admin_internal_port_path_cost = 40000,
    };
    MSTP_IN_set_msti_port_config(GET_CIST_PTP_FROM_PORT(br0p[0]), &port_config);

    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 40000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 40000);

    /* br0p2.ist should be unaffected */
    MSTP_IN_get_cist_port_status(br0p[1], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);

    /* reset br0p1.ist.adminIntPortPathCost to 0 */
    port_config.admin_internal_port_path_cost = 0;
    MSTP_IN_set_msti_port_config(GET_CIST_PTP_FROM_PORT(br0p[0]), &port_config);

    /* br0p2.ist.intPortPathCost should be back to its default value */
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
    assert_uint_equal(port_status.internal_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_internal_port_path_cost, 0);
}

/* Test recommended costs according to Table 13-4 — Port Path Cost values */
static void portcost_values(void **state)
{
    port_t *br0p[1];
    bridge_t *br0;

    alloc_bridge_ports(state, &br0, "br0", 0x200000000001, &br0p, 1);

    MSTP_IN_set_bridge_enable(br0, true);

    /* lowest representable speed is 1 Mbit/s */
    set_port_state(br0p[0], true, 1, true);

    CIST_PortStatus port_status;

    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000000);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 10, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 2000000);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 100, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 200000);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 1000, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 10000, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 2000);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 100000, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 200);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 1000000, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20);

    set_port_state(br0p[0], false, 0, false);
    set_port_state(br0p[0], true, 10000000, true);
    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 2);
}


/* Test only cost in valid range according to Table 13-4 — Port Path Cost
 * values are accepted */
static void portcost_invalid(void **state)
{
    port_t *br0p[1];
    bridge_t *br0;

    alloc_bridge_ports(state, &br0, "br0", 0x200000000001, &br0p, 1);

    MSTP_IN_set_bridge_enable(br0, true);

    set_port_state(br0p[0], true, 1000, true);

    CIST_PortStatus port_status;

    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);

    CIST_PortConfig port_config = {
       .set_admin_external_port_path_cost = true,
       .admin_external_port_path_cost = MAX_PATH_COST + 1,
    };
    assert_int_not_equal(MSTP_IN_set_cist_port_config(br0p[0], &port_config), 0);

    MSTP_IN_get_cist_port_status(br0p[0], &port_status);
    assert_uint_equal(port_status.external_port_path_cost, 20000);
    assert_uint_equal(port_status.admin_external_port_path_cost, 0);
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(admin_ext_portcost, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(admin_int_portcost, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(portcost_values, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(portcost_invalid, prepare_test, teardown_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <string.h>

#include "mstp.h"
#include "common.h"

/* Tests based IEEE 802.1Q-2022, Table 13-2: "Example Configuration Digests" */

/* All VIDs map to the CIST, no VID mapped to any MSTI */
static void all_vid_cist_digest(void **state)
{
    const __u8 expected_digest[] = {
        0xAC, 0x36, 0x17, 0x7F, 0x50, 0x28, 0x3C, 0xD4, 0xB8, 0x38, 0x21, 0xD8,
        0xAB, 0x26, 0xDE, 0x62
    };
    bridge_t *br;
    int i;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);

    for (i = 1; i <= MAX_VID; i++)
        MSTP_IN_set_vid2fid(br, i, 1);

    MSTP_IN_set_fid2mstid(br, 1, 0);

    assert_memory_equal(br->MstConfigId.s.configuration_digest,
                        expected_digest, 16);
}

/* All VIDs map to MSTID 1 */
static void all_vid_mst_1_digest(void **state)
{
    const __u8 expected_digest[] = {
        0xE1, 0x3A, 0x80, 0xF1, 0x1E, 0xD0, 0x85, 0x6A, 0xCD, 0x4E, 0xE3, 0x47,
        0x69, 0x41, 0xC7, 0x3B
    };
    bridge_t *br;
    int i;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);

    for (i = 1; i <= MAX_VID; i++)
        MSTP_IN_set_vid2fid(br, i, 1);

    assert_true(MSTP_IN_create_msti(br, 1));
    MSTP_IN_set_fid2mstid(br, 1, 1);

    assert_memory_equal(br->MstConfigId.s.configuration_digest,
                        expected_digest, 16);
}

/* Every VID maps to the MSTID equal to (VID modulo 32) + 1 */
static void all_vid_mst_mod32_digest(void **state)
{
    const __u8 expected_digest[] = {
        0x9D, 0x14, 0x5C, 0x26, 0x7D, 0xBE, 0x9F, 0xB5, 0xD8, 0x93, 0x44, 0x1B,
        0xE3, 0xBA, 0x08, 0xCE
    };
    bridge_t *br;
    int i;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);

    for (i = 1; i <= 32; i++)
        assert_true(MSTP_IN_create_msti(br, i));

    for (i = 1; i <= MAX_VID; i++)
        MSTP_IN_set_vid2fid(br, i, i);
    for (i = 1; i <= MAX_FID; i++)
        MSTP_IN_set_fid2mstid(br, i, (i % 32) + 1);

    assert_memory_equal(br->MstConfigId.s.configuration_digest,
                        expected_digest, 16);
}

/* Setting a configuration name should set the name as expected */
static void configuration_name(void **state)
{
    char conf_name[CONFIGURATION_NAME_LEN + 1];
    bridge_t *br;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);

    MSTP_IN_set_mst_config_id(br, 0, (__u8 *)"mstpd_configuration");

    strncpy(conf_name, "mstpd_configuration", CONFIGURATION_NAME_LEN);
    assert_memory_equal(br->MstConfigId.s.configuration_name, conf_name,
                        CONFIGURATION_NAME_LEN);
}

/* Setting a zero-lengths configuration name results in it being all-zero */
static void configuration_name_min(void **state)
{
    char conf_name[CONFIGURATION_NAME_LEN + 1] = { 0 };
    bridge_t *br;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);


    MSTP_IN_set_mst_config_id(br, 0, (__u8 *)"");

    assert_memory_equal(br->MstConfigId.s.configuration_name, conf_name,
                        CONFIGURATION_NAME_LEN);
}

/* Setting a 32 character long name results in the name being set with no NULL
 * terminator */
static void configuration_name_max(void **state)
{
    char conf_name[CONFIGURATION_NAME_LEN + 1];
    bridge_t *br;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);

    MSTP_IN_set_mst_config_id(br, 0, (__u8 *)"ThisIsAVeryLongConfigurationName");

    strncpy(conf_name, "ThisIsAVeryLongConfigurationName", sizeof(conf_name));
    assert_uint_equal(strlen(conf_name), CONFIGURATION_NAME_LEN);
    assert_memory_equal(br->MstConfigId.s.configuration_name, conf_name,
                        CONFIGURATION_NAME_LEN);
}

/* Setting a name longer than 32 characters results in the name being trunctated */
static void configuration_name_over(void **state)
{
    char conf_name[CONFIGURATION_NAME_LEN + 3];
    bridge_t *br;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);


    MSTP_IN_set_mst_config_id(br, 0, (__u8 *)"ThisIsAReallyLongConfigurationName");

    /* TODO: should this be truncated or rejected? */
    strncpy(conf_name, "ThisIsAReallyLongConfigurationName", sizeof(conf_name));
    assert_uint_equal(strlen(conf_name), CONFIGURATION_NAME_LEN + 2);
    assert_memory_equal(br->MstConfigId.s.configuration_name, conf_name,
                        CONFIGURATION_NAME_LEN);
}
/* Check that Any data after the null terminator is ignored/cleared */
static void configuration_name_garbage(void **state)
{
    char conf_name[CONFIGURATION_NAME_LEN + 1];
    bridge_t *br;

    alloc_bridge_ports(state, &br, "BR_TEST", 0x200000000001, NULL, 0);

    MSTP_IN_set_mst_config_id(br, 0, (__u8 *)"ThisIsAVeryLongConfigurationName");
    MSTP_IN_set_mst_config_id(br, 0, (__u8*)"ShortName");

    strncpy(conf_name, "ShortName", sizeof(conf_name));
    assert_memory_equal(br->MstConfigId.s.configuration_name, conf_name,
                        CONFIGURATION_NAME_LEN);
}

int main(int argc, char *argv[])
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(all_vid_cist_digest, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(all_vid_mst_1_digest, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(all_vid_mst_mod32_digest, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(configuration_name, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(configuration_name_min, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(configuration_name_max, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(configuration_name_over, prepare_test, teardown_test),
        cmocka_unit_test_setup_teardown(configuration_name_garbage, prepare_test, teardown_test),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

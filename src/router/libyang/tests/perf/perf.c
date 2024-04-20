/**
 * @file perf.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief performance tests
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "libyang.h"
#include "tests_config.h"

#ifdef HAVE_CALLGRIND
# include <valgrind/callgrind.h>
#endif

#define TEMP_FILE "perf_tmp"

/**
 * @brief Test state structure.
 */
struct test_state {
    const struct lys_module *mod;
    uint32_t count;
    struct lyd_node *data1;
    struct lyd_node *data2;
};

typedef LY_ERR (*setup_cb)(const struct lys_module *mod, uint32_t count, struct test_state *state);

typedef LY_ERR (*test_cb)(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end);

/**
 * @brief Single test structure.
 */
struct test {
    const char *name;
    setup_cb setup;
    test_cb test;
};

/**
 * @brief Get current time as timespec.
 *
 * @param[out] ts Timespect to fill.
 */
static void
time_get(struct timespec *ts)
{
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, ts);
#elif defined (CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, ts);
#elif defined (CLOCK_REALTIME)
    /* no monotonic clock available, return realtime */
    clock_gettime(CLOCK_REALTIME, ts);
#else
    int rc;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    ts->tv_sec = (time_t)tv.tv_sec;
    ts->tv_nsec = 1000L * (long)tv.tv_usec;
#endif
}

/**
 * @brief Get the difference of 2 timespecs in microseconds.
 *
 * @param[in] ts1 Smaller (older) timespec.
 * @param[in] ts2 Larger (later) timespec.
 * @return Difference of timespecs in usec.
 */
static uint64_t
time_diff(const struct timespec *ts1, const struct timespec *ts2)
{
    uint64_t usec_diff = 0;
    int64_t nsec_diff;

    assert(ts1->tv_sec <= ts2->tv_sec);

    /* seconds diff */
    usec_diff += (ts2->tv_sec - ts1->tv_sec) * 1000000;

    /* nanoseconds diff */
    nsec_diff = ts2->tv_nsec - ts1->tv_nsec;
    usec_diff += nsec_diff ? nsec_diff / 1000 : 0;

    return usec_diff;
}

/**
 * @brief Create data tree with list instances.
 *
 * @param[in] mod Module of the top-level node.
 * @param[in] offset Starting offset of the identifier number values.
 * @param[in] count Number of list instances to create, with increasing identifier numbers.
 * @param[out] data Created data.
 * @return LY_ERR value.
 */
static LY_ERR
create_list_inst(const struct lys_module *mod, uint32_t offset, uint32_t count, struct lyd_node **data)
{
    LY_ERR ret;
    uint32_t i;
    char k1_val[32], k2_val[32], l_val[32], lfl_val[32];
    struct lyd_node *list;

    if ((ret = lyd_new_inner(NULL, mod, "cont", 0, data))) {
        return ret;
    }

    for (i = 0; i < count; ++i) {
        sprintf(k1_val, "%" PRIu32, i + offset);
        sprintf(k2_val, "str%" PRIu32, i + offset);
        sprintf(l_val, "l%" PRIu32, i + offset);

        if ((ret = lyd_new_list(*data, NULL, "lst", 0, &list, k1_val, k2_val))) {
            return ret;
        }
        if ((ret = lyd_new_term(list, NULL, "l", l_val, 0, NULL))) {
            return ret;
        }
    }

    /* Last list contains a "lfl" leaf-list with @p count terms. */
    for (i = 0; i < count; ++i) {
        sprintf(lfl_val, "%" PRIu32, i + offset);
        if ((ret = lyd_new_term(list, NULL, "lfl", lfl_val, 0, NULL))) {
            return ret;
        }
    }

    return LY_SUCCESS;
}

/**
 * @brief Execute a test.
 *
 * @param[in] setup Setup callback to call once.
 * @param[in] test Test callback.
 * @param[in] name Name of the test.
 * @param[in] mod Module of testing data.
 * @param[in] count Count of list instances, size of the testing data set.
 * @param[in] tries Number of (re)tries of the test to get more accurate measurements.
 * @return LY_ERR value.
 */
static LY_ERR
exec_test(setup_cb setup, test_cb test, const char *name, const struct lys_module *mod, uint32_t count, uint32_t tries)
{
    LY_ERR ret;
    struct timespec ts_start, ts_end;
    struct test_state state = {0};
    const uint32_t name_fixed_len = 38;
    char str[name_fixed_len + 1];
    uint32_t i, printed;
    uint64_t time_usec = 0;

    /* print test start */
    printed = sprintf(str, "| %s ", name);
    while (printed + 2 < name_fixed_len) {
        printed += sprintf(str + printed, ".");
    }
    if (printed + 1 < name_fixed_len) {
        printed += sprintf(str + printed, " ");
    }
    sprintf(str + printed, "|");
    fputs(str, stdout);
    fflush(stdout);

    /* setup */
    if ((ret = setup(mod, count, &state))) {
        return ret;
    }

    /* test */
    for (i = 0; i < tries; ++i) {
        if ((ret = test(&state, &ts_start, &ts_end))) {
            return ret;
        }
        time_usec += time_diff(&ts_start, &ts_end);
    }
    time_usec /= tries;

    /* teardown */
    lyd_free_siblings(state.data1);
    lyd_free_siblings(state.data2);

    /* print time */
    printf(" %" PRIu64 ".%06" PRIu64 " s |\n", time_usec / 1000000, time_usec % 1000000);

    return LY_SUCCESS;
}

static void
TEST_START(struct timespec *ts)
{
    time_get(ts);

#ifdef HAVE_CALLGRIND
    CALLGRIND_START_INSTRUMENTATION;
#endif
}

static void
TEST_END(struct timespec *ts)
{
    time_get(ts);

#ifdef HAVE_CALLGRIND
    CALLGRIND_STOP_INSTRUMENTATION;
#endif
}

/* TEST SETUP */
static LY_ERR
setup_basic(const struct lys_module *mod, uint32_t count, struct test_state *state)
{
    state->mod = mod;
    state->count = count;

    return LY_SUCCESS;
}

static LY_ERR
setup_data_single_tree(const struct lys_module *mod, uint32_t count, struct test_state *state)
{
    state->mod = mod;
    state->count = count;

    return create_list_inst(mod, 0, count, &state->data1);
}

static LY_ERR
setup_data_same_trees(const struct lys_module *mod, uint32_t count, struct test_state *state)
{
    LY_ERR ret;

    state->mod = mod;
    state->count = count;

    if ((ret = create_list_inst(mod, 0, count, &state->data1))) {
        return ret;
    }
    if ((ret = create_list_inst(mod, 0, count, &state->data2))) {
        return ret;
    }

    return LY_SUCCESS;
}

static LY_ERR
setup_data_no_same_trees(const struct lys_module *mod, uint32_t count, struct test_state *state)
{
    LY_ERR ret;

    state->mod = mod;
    state->count = count;

    if ((ret = create_list_inst(mod, 0, count, &state->data1))) {
        return ret;
    }
    if ((ret = create_list_inst(mod, count, count, &state->data2))) {
        return ret;
    }

    return LY_SUCCESS;
}

static LY_ERR
setup_data_empty_and_full_trees(const struct lys_module *mod, uint32_t count, struct test_state *state)
{
    LY_ERR ret;

    state->mod = mod;
    state->count = count;

    if ((ret = create_list_inst(mod, 0, 0, &state->data1))) {
        return ret;
    }
    if ((ret = create_list_inst(mod, 0, count, &state->data2))) {
        return ret;
    }

    return LY_SUCCESS;
}

static LY_ERR
setup_data_offset_tree(const struct lys_module *mod, uint32_t count, struct test_state *state)
{
    LY_ERR ret;

    state->mod = mod;
    state->count = count;

    if ((ret = create_list_inst(mod, count, count, &state->data2))) {
        return ret;
    }

    return LY_SUCCESS;
}

/* TEST CB */
static LY_ERR
test_create_new_text(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data = NULL;

    TEST_START(ts_start);

    if ((r = create_list_inst(state->mod, 0, state->count, &data))) {
        return r;
    }

    TEST_END(ts_end);

    lyd_free_siblings(data);

    return LY_SUCCESS;
}

static LY_ERR
test_create_new_bin(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data = NULL;
    uint32_t i, k2_len, l_len;
    char k2_val[32], l_val[32];
    struct lyd_node *list;

    TEST_START(ts_start);

    if ((r = lyd_new_inner(NULL, state->mod, "cont", 0, &data))) {
        return r;
    }

    for (i = 0; i < state->count; ++i) {
        k2_len = sprintf(k2_val, "str%" PRIu32, i);
        l_len = sprintf(l_val, "l%" PRIu32, i);

        if ((r = lyd_new_list(data, NULL, "lst", LYD_NEW_VAL_BIN, &list, &i, sizeof i, k2_val, k2_len))) {
            return r;
        }
        if ((r = lyd_new_term_bin(list, NULL, "l", l_val, l_len, 0, NULL))) {
            return r;
        }
    }

    TEST_END(ts_end);

    lyd_free_siblings(data);

    return LY_SUCCESS;
}

static LY_ERR
test_create_path(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data = NULL;
    uint32_t i;
    char path[64], l_val[32];

    TEST_START(ts_start);

    if ((r = lyd_new_inner(NULL, state->mod, "cont", 0, &data))) {
        return r;
    }

    for (i = 0; i < state->count; ++i) {
        sprintf(path, "/perf:cont/lst[k1='%" PRIu32 "'][k2='str%" PRIu32 "']/l", i, i);
        sprintf(l_val, "l%" PRIu32, i);

        if ((r = lyd_new_path(data, NULL, path, l_val, 0, NULL))) {
            return r;
        }
    }

    TEST_END(ts_end);

    lyd_free_siblings(data);

    return LY_SUCCESS;
}

static LY_ERR
test_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;

    TEST_START(ts_start);

    if ((r = lyd_validate_all(&state->data1, NULL, LYD_VALIDATE_PRESENT, NULL))) {
        return r;
    }

    TEST_END(ts_end);

    return LY_SUCCESS;
}

static LY_ERR
_test_parse(struct test_state *state, LYD_FORMAT format, ly_bool use_file, uint32_t print_options, uint32_t parse_options,
        uint32_t validate_options, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR ret = LY_SUCCESS;
    struct lyd_node *data = NULL;
    char *buf = NULL;
    struct ly_in *in = NULL;

    if (use_file) {
        if ((ret = lyd_print_path(TEMP_FILE, state->data1, format, print_options))) {
            goto cleanup;
        }
        if ((ret = ly_in_new_filepath(TEMP_FILE, 0, &in))) {
            goto cleanup;
        }
    } else {
        if ((ret = lyd_print_mem(&buf, state->data1, format, print_options))) {
            goto cleanup;
        }
        if ((ret = ly_in_new_memory(buf, &in))) {
            goto cleanup;
        }
    }

    TEST_START(ts_start);

    if ((ret = lyd_parse_data(state->mod->ctx, NULL, in, format, parse_options, validate_options, &data))) {
        goto cleanup;
    }

    TEST_END(ts_end);

cleanup:
    free(buf);
    ly_in_free(in, 0);
    lyd_free_siblings(data);
    return ret;
}

static LY_ERR
test_parse_xml_mem_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_XML, 0, LYD_PRINT_SHRINK, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, ts_start, ts_end);
}

static LY_ERR
test_parse_xml_mem_no_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_XML, 0, LYD_PRINT_SHRINK, LYD_PARSE_STRICT | LYD_PARSE_ONLY | LYD_PARSE_ORDERED, 0,
            ts_start, ts_end);
}

static LY_ERR
test_parse_xml_file_no_validate_format(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_XML, 1, 0, LYD_PARSE_STRICT | LYD_PARSE_ONLY | LYD_PARSE_ORDERED, 0, ts_start, ts_end);
}

static LY_ERR
test_parse_json_mem_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_JSON, 0, LYD_PRINT_SHRINK, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, ts_start, ts_end);
}

static LY_ERR
test_parse_json_mem_no_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_JSON, 0, LYD_PRINT_SHRINK, LYD_PARSE_STRICT | LYD_PARSE_ONLY | LYD_PARSE_ORDERED, 0,
            ts_start, ts_end);
}

static LY_ERR
test_parse_json_file_no_validate_format(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_JSON, 1, 0, LYD_PARSE_STRICT | LYD_PARSE_ONLY | LYD_PARSE_ORDERED, 0, ts_start, ts_end);
}

static LY_ERR
test_parse_lyb_mem_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_LYB, 0, LYD_PRINT_SHRINK, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, ts_start, ts_end);
}

static LY_ERR
test_parse_lyb_mem_no_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_LYB, 0, LYD_PRINT_SHRINK, LYD_PARSE_STRICT | LYD_PARSE_ONLY | LYD_PARSE_ORDERED, 0,
            ts_start, ts_end);
}

static LY_ERR
test_parse_lyb_file_no_validate(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_parse(state, LYD_LYB, 1, 0, LYD_PARSE_STRICT | LYD_PARSE_ONLY | LYD_PARSE_ORDERED, 0, ts_start, ts_end);
}

static LY_ERR
_test_print(struct test_state *state, LYD_FORMAT format, uint32_t print_options, struct timespec *ts_start,
        struct timespec *ts_end)
{
    LY_ERR ret = LY_SUCCESS;
    char *buf = NULL;

    TEST_START(ts_start);

    if ((ret = lyd_print_mem(&buf, state->data1, format, print_options))) {
        goto cleanup;
    }

    TEST_END(ts_end);

cleanup:
    free(buf);
    return ret;
}

static LY_ERR
test_print_xml(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_print(state, LYD_XML, LYD_PRINT_SHRINK, ts_start, ts_end);
}

static LY_ERR
test_print_json(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_print(state, LYD_JSON, LYD_PRINT_SHRINK, ts_start, ts_end);
}

static LY_ERR
test_print_lyb(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    return _test_print(state, LYD_LYB, LYD_PRINT_SHRINK, ts_start, ts_end);
}

static LY_ERR
test_dup(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data;

    TEST_START(ts_start);

    if ((r = lyd_dup_siblings(state->data1, NULL, LYD_DUP_RECURSIVE, &data))) {
        return r;
    }

    TEST_END(ts_end);

    lyd_free_siblings(data);

    return LY_SUCCESS;
}

static LY_ERR
test_dup_siblings_to_empty(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;

    TEST_START(ts_start);

    if ((r = lyd_dup_siblings(lyd_child(state->data2), (struct lyd_node_inner *)state->data1, 0, NULL))) {
        return r;
    }

    TEST_END(ts_end);

    return LY_SUCCESS;
}

static LY_ERR
test_free(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data;

    if ((r = create_list_inst(state->mod, 0, state->count, &data))) {
        return r;
    }

    TEST_START(ts_start);

    lyd_free_siblings(data);

    TEST_END(ts_end);

    return LY_SUCCESS;
}

static LY_ERR
test_xpath_find(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct ly_set *set;
    char path[64];

    sprintf(path, "/perf:cont/lst[k1=%" PRIu32 " and k2='str%" PRIu32 "']", state->count / 2, state->count / 2);

    TEST_START(ts_start);

    if ((r = lyd_find_xpath(state->data1, path, &set))) {
        return r;
    }

    TEST_END(ts_end);

    ly_set_free(set, NULL);

    return LY_SUCCESS;
}

static LY_ERR
test_xpath_find_hash(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct ly_set *set;
    char path[64];

    sprintf(path, "/perf:cont/lst[k1=%" PRIu32 "][k2='str%" PRIu32 "']", state->count / 2, state->count / 2);

    TEST_START(ts_start);

    if ((r = lyd_find_xpath(state->data1, path, &set))) {
        return r;
    }

    TEST_END(ts_end);

    ly_set_free(set, NULL);

    return LY_SUCCESS;
}

static LY_ERR
test_compare_same(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;

    TEST_START(ts_start);

    if ((r = lyd_compare_siblings(state->data1, state->data2, LYD_COMPARE_FULL_RECURSION))) {
        return r;
    }

    TEST_END(ts_end);

    return LY_SUCCESS;
}

static LY_ERR
test_diff_same(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *diff;

    TEST_START(ts_start);

    if ((r = lyd_diff_siblings(state->data1, state->data2, 0, &diff))) {
        return r;
    }

    TEST_END(ts_end);

    lyd_free_siblings(diff);

    return LY_SUCCESS;
}

static LY_ERR
test_diff_no_same(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *diff;

    TEST_START(ts_start);

    if ((r = lyd_diff_siblings(state->data1, state->data2, 0, &diff))) {
        return r;
    }

    TEST_END(ts_end);

    lyd_free_siblings(diff);

    return LY_SUCCESS;
}

static LY_ERR
test_merge_same(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;

    TEST_START(ts_start);

    if ((r = lyd_merge_siblings(&state->data1, state->data2, 0))) {
        return r;
    }

    TEST_END(ts_end);

    return LY_SUCCESS;
}

static LY_ERR
test_merge_no_same(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data1;

    if ((r = create_list_inst(state->mod, 0, state->count, &data1))) {
        return r;
    }

    TEST_START(ts_start);

    if ((r = lyd_merge_siblings(&data1, state->data2, 0))) {
        return r;
    }

    TEST_END(ts_end);

    lyd_free_siblings(data1);

    return LY_SUCCESS;
}

static LY_ERR
test_merge_no_same_destruct(struct test_state *state, struct timespec *ts_start, struct timespec *ts_end)
{
    LY_ERR r;
    struct lyd_node *data1, *data2;

    if ((r = create_list_inst(state->mod, 0, state->count, &data1))) {
        return r;
    }
    if ((r = create_list_inst(state->mod, state->count, state->count, &data2))) {
        return r;
    }

    TEST_START(ts_start);

    if ((r = lyd_merge_siblings(&data1, data2, LYD_MERGE_DESTRUCT))) {
        return r;
    }

    TEST_END(ts_end);

    lyd_free_siblings(data1);

    return LY_SUCCESS;
}

struct test tests[] = {
    {"create new text", setup_basic, test_create_new_text},
    {"create new bin", setup_basic, test_create_new_bin},
    {"create path", setup_basic, test_create_path},
    {"validate", setup_data_single_tree, test_validate},
    {"parse xml mem validate", setup_data_single_tree, test_parse_xml_mem_validate},
    {"parse xml mem no validate", setup_data_single_tree, test_parse_xml_mem_no_validate},
    {"parse xml file no validate format", setup_data_single_tree, test_parse_xml_file_no_validate_format},
    {"parse json mem validate", setup_data_single_tree, test_parse_json_mem_validate},
    {"parse json mem no validate", setup_data_single_tree, test_parse_json_mem_no_validate},
    {"parse json file no validate format", setup_data_single_tree, test_parse_json_file_no_validate_format},
    {"parse lyb mem validate", setup_data_single_tree, test_parse_lyb_mem_validate},
    {"parse lyb mem no validate", setup_data_single_tree, test_parse_lyb_mem_no_validate},
    {"parse lyb file no validate", setup_data_single_tree, test_parse_lyb_file_no_validate},
    {"print xml", setup_data_single_tree, test_print_xml},
    {"print json", setup_data_single_tree, test_print_json},
    {"print lyb", setup_data_single_tree, test_print_lyb},
    {"dup", setup_data_single_tree, test_dup},
    {"dup_siblings_to_empty", setup_data_empty_and_full_trees, test_dup_siblings_to_empty},
    {"free", setup_basic, test_free},
    {"xpath find", setup_data_single_tree, test_xpath_find},
    {"xpath find hash", setup_data_single_tree, test_xpath_find_hash},
    {"compare same", setup_data_same_trees, test_compare_same},
    {"diff same", setup_data_same_trees, test_diff_same},
    {"diff no same", setup_data_no_same_trees, test_diff_no_same},
    {"merge same", setup_data_same_trees, test_merge_same},
    {"merge no same", setup_data_offset_tree, test_merge_no_same},
    {"merge no same destruct", setup_basic, test_merge_no_same_destruct},
};

int
main(int argc, char **argv)
{
    LY_ERR ret = LY_SUCCESS;
    struct ly_ctx *ctx = NULL;
    const struct lys_module *mod;
    uint32_t i, count, tries;

    if (argc < 3) {
        fprintf(stderr, "Usage:\n%s list-instance-count test-tries\n\n", argv[0]);
        return LY_EINVAL;
    }

    count = atoi(argv[1]);
    if (!count) {
        fprintf(stderr, "Invalid count \"%s\".\n", argv[1]);
        return LY_EINVAL;
    }

    tries = atoi(argv[2]);
    if (!tries) {
        fprintf(stderr, "Invalid tries \"%s\".\n", argv[2]);
        return LY_EINVAL;
    }

    printf("\nly_perf:\n\tdata set size: %" PRIu32 "\n\teach test executed: %" PRIu32 " %s\n\n", count, tries,
            (tries > 1) ? "times" : "time");

    /* create context */
    if ((ret = ly_ctx_new(TESTS_SRC "/perf", 0, &ctx))) {
        goto cleanup;
    }

    /* load modules */
    if (!(mod = ly_ctx_load_module(ctx, "perf", NULL, NULL))) {
        ret = LY_ENOTFOUND;
        goto cleanup;
    }

    /* tests */
    for (i = 0; i < (sizeof tests / sizeof(struct test)); ++i) {
        if ((ret = exec_test(tests[i].setup, tests[i].test, tests[i].name, mod, count, tries))) {
            goto cleanup;
        }
    }

    printf("\n");

cleanup:
    ly_ctx_destroy(ctx);
    return ret;
}

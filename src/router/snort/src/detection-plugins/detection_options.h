/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**/

/**
**  @file        detection_options.h
**
**  @author      Steven Sturges
**
**  @brief       Support functions for rule option tree
**
**  This implements tree processing for rule options, evaluating common
**  detection options only once per pattern match.
**
*/

#ifndef DETECTION_OPTIONS_H_
#define DETECTION_OPTIONS_H_

#include "sf_types.h"
#include "decode.h"
#include "sfutil/sfxhash.h"
#include "rule_option_types.h"

#define DETECTION_OPTION_EQUAL 0
#define DETECTION_OPTION_NOT_EQUAL 1

#define DETECTION_OPTION_NO_MATCH 0
#define DETECTION_OPTION_MATCH 1
#define DETECTION_OPTION_NO_ALERT 2
#define DETECTION_OPTION_FAILED_BIT 3

#include "sfutil/sfhashfcn.h"

typedef int (*eval_func_t)(void *option_data, Packet *p);

typedef struct _detection_option_tree_node
{
    void *option_data;
    option_type_t option_type;
    eval_func_t evaluate;
    int num_children;
    struct _detection_option_tree_node **children;
    int relative_children;
    int result;
    struct
    {
        struct timeval ts;
        uint64_t packet_number;
        uint32_t rebuild_flag;
        char result;
        char is_relative;
        char flowbit_failed;
        char pad; /* Keep 4 byte alignment */
    } last_check;
#ifdef PERF_PROFILING
    uint64_t ticks;
    uint64_t ticks_match;
    uint64_t ticks_no_match;
    uint64_t checks;
#endif
#ifdef PPM_MGR
    uint64_t ppm_disable_cnt; /*PPM */
    uint64_t ppm_enable_cnt; /*PPM */
#endif
} detection_option_tree_node_t;

typedef struct _detection_option_tree_root
{
    int num_children;
    detection_option_tree_node_t **children;

#ifdef PPM_MGR
    uint64_t ppm_suspend_time; /* PPM */
    uint64_t ppm_disable_cnt; /*PPM */
    int tree_state;
#endif
} detection_option_tree_root_t;

typedef struct _detection_option_eval_data
{
    void *pomd;
    void *pmd;
    Packet *p;
    char flowbit_failed;
    char flowbit_noalert;
    uint8_t detection_filter_count;
} detection_option_eval_data_t;

int add_detection_option(struct _SnortConfig *, option_type_t type, void *option_data, void **existing_data);
int add_detection_option_tree(struct _SnortConfig *, detection_option_tree_node_t *option_tree, void **existing_data);
int detection_option_node_evaluate(detection_option_tree_node_t *node, detection_option_eval_data_t *eval_data);
void DetectionHashTableFree(SFXHASH *);
void DetectionTreeHashTableFree(SFXHASH *);
#ifdef DEBUG_OPTION_TREE
void print_option_tree(detection_option_tree_node_t *node, int level);
#endif
#ifdef PERF_PROFILING
void detection_option_tree_update_otn_stats(SFXHASH *);
#endif

#endif /* DETECTION_OPTIONS_H_ */


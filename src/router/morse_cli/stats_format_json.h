/*
 * Copyright 2022-2026 Morse Micro
 *
 * This file contains declarations related to formatting the decoded statistics output as JSON
 */

#pragma once

/** Get item format function table  */
const struct format_table* stats_format_get_formatter_table_json(enum format_type format);

/** Start formatting output */
void stats_format_start_json(enum format_type format);

/** Output approrpriate seperator between items */
void stats_format_separate_json(enum format_type format);

/** Finish formatting output */
void stats_format_finish_json(enum format_type format);

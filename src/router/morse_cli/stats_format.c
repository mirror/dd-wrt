/*
 * Copyright 2026 Morse Micro
 *
 * This file contains implementation related to formatting the decoded statistics output
 */

#include "stats_format.h"
#include "stats_format_json.h"
#include "stats_format_regular.h"

const struct format_table* stats_format_get_formatter_table(enum format_type format)
{
    switch (format)
    {
        case FORMAT_JSON:
        case FORMAT_JSON_PPRINT:
            return stats_format_get_formatter_table_json(format);
            break;

        case FORMAT_REGULAR:
        default:
            return stats_format_get_formatter_table_regular(format);
    }
}


void stats_format_start(enum format_type format)
{
    switch (format)
    {
        case FORMAT_JSON:
        case FORMAT_JSON_PPRINT:
            stats_format_start_json(format);
            break;

        case FORMAT_REGULAR:
        default:
            stats_format_start_regular(format);
    }
}


void stats_format_separate(enum format_type format)
{
    switch (format)
    {
        case FORMAT_JSON:
        case FORMAT_JSON_PPRINT:
            stats_format_separate_json(format);
            break;

        case FORMAT_REGULAR:
        default:
            stats_format_separate_regular(format);
    }
}


void stats_format_finish(enum format_type format)
{
    switch (format)
    {
        case FORMAT_JSON:
        case FORMAT_JSON_PPRINT:
            stats_format_finish_json(format);
            break;

        case FORMAT_REGULAR:
        default:
            stats_format_finish_regular(format);
    }
}


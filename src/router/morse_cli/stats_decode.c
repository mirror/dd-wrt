/*
 * Copyright 2020-2026 Morse Micro
 *
 * This file contains implementation for functions to load and use statistics definitions and data
 */

#include <errno.h>

#ifndef MORSE_WIN_BUILD
#include <regex.h>
#endif

#include "stats_decode.h"
#include "elf_file.h"


int load_offchip_statistics_definitions(struct morsectrl *mors, const char *filename)
{
    FILE *infile;

    if (filename == NULL)
    {
        mctrl_err("No statistics definitions filename\n");
        return -1;
    }

    infile = fopen(filename, "rb");
    if (infile)
    {
        uint8_t *buf = NULL;

        load_file(infile, &buf);
        if (buf)
        {
            morse_stats_load(&mors->stats, &mors->n_stats, buf);
            free(buf);
        }
        fclose(infile);

        if (mors->n_stats == 0)
        {
            mctrl_err("No stats metadata found in '%s'\n", filename);
            return -1;
        }
    }
    else
    {
        mctrl_err("Open error: %s '%s'\n", strerror(errno), filename);
        return -1;
    }
    return 0;
}


void free_offchip_statistics_definitions(struct morsectrl *mors)
{
    if (mors->stats != NULL)
    {
        free(mors->stats);
        mors->stats = NULL;
    }
    mors->n_stats = 0;
}

#ifndef MORSE_WIN_BUILD
static regex_t *filter_re = NULL;

int stats_filter_init(const char *filter_string)
{
    int ret = 0;

#ifdef CONFIG_ANDROID
    /* In android, regcomp returns an error "empty (sub)expression" as empty string is
     * not considered as valid regex.
     */
    if (filter_string[0] == '\0')
    filter_string = "\\(\\)";
#endif

    filter_re = malloc(sizeof(*filter_re));
    ret = regcomp(filter_re, filter_string, 0);

    if (ret)
    {
        size_t len = regerror(ret, filter_re, NULL, 0);
        char *re_err_buf = malloc(len);
        regerror(ret, filter_re, re_err_buf, len);
        mctrl_err("Invalid filter string: %s\n", re_err_buf);
        free(re_err_buf);
        free(filter_re);
        filter_re = NULL;
    }

    return ret;
}

int stats_filter_is_match(const char *key)
{
    return regexec(filter_re, key, 0, NULL, 0);
}

void stats_filter_deinit(void)
{
    if (filter_re)
    {
        regfree(filter_re);
        free(filter_re);
    }
    filter_re = NULL;
}

bool stats_filter_is_set(void)
{
    return (filter_re != NULL);
}

const char *stats_filter_help(void)
{
    return "uses a regular expression";
}
#else
const char *filter_str = NULL;

int stats_filter_init(const char *filter_string)
{
    filter_str = filter_string;

    return 0;
}

int stats_filter_is_match(const char *key)
{
    return strcmp(key, filter_str);
}

void stats_filter_deinit(void)
{
    filter_str = NULL;
}

bool stats_filter_is_set(void)
{
    return (filter_str != NULL);
}

const char *stats_filter_help(void)
{
    return "case sensitive, match from start of key";
}
#endif

int load_offchip_statistics_data(const char *statsdump_filename,
                        uint8_t **statsbin_buffer, size_t *statsbin_length)
{
    int ret = 0;
    uint8_t *statsfile_buf = NULL;
    size_t statsfile_len = 0;
    size_t statsfile_chars_read = 0;

    *statsbin_buffer = NULL;
    *statsbin_length = 0;

    if (statsdump_filename == NULL)
    {
        mctrl_err("No statistics data filename'\n");
        return -1;
    }

    FILE *infile = fopen(statsdump_filename, "r");

    if (infile == NULL)
    {
        mctrl_err("Failed to open '%s'\n", statsdump_filename);
        return -1;
    }

    load_file_with_size(infile, &statsfile_buf, &statsfile_len);
    fclose(infile);

    if (statsfile_buf == NULL)
    {
        mctrl_err("Failed to load '%s'\n", statsdump_filename);
        return -1;
    }

    /* The translation of hex strings into binary happens in-place as a maximum of half
     * the available space is needed
     */
    uint8_t *statsbin_next_byte = statsfile_buf;
    char hexchars[2];

    while (statsfile_chars_read < statsfile_len)
    {
        hexchars[0] = statsfile_buf[statsfile_chars_read++];
#ifdef STATS_LOAD_DISCARD_DEBUG
        mctrl_print("hexchars[0] value %d\n", hexchars[0]);
#endif

        if (hexchars[0] == '#')
        {
            /* Ignore comment to end-of-line */
            while ((statsfile_chars_read < statsfile_len) &&
                   (statsfile_buf[statsfile_chars_read] != '\n'))
            {
#ifdef STATS_LOAD_DISCARD_DEBUG
                mctrl_print("Discard comment: '%c'\n", statsfile_buf[statsfile_chars_read]);
#endif
                statsfile_chars_read++;
            }
        }
        else if (hexchars[0] == '\0')
        {
            mctrl_print("Unexpected nul character. Only UTF-8 encoding is supported.\n");
            ret = -1;
            break;
        }
        else if (!isxdigit(hexchars[0]))
        {
            /* Ignore anything that isn't hex digits, predominantly intended for whitespace */
#ifdef STATS_LOAD_DISCARD_DEBUG
            mctrl_print("Discard not a hex digit: '%c'\n", hexchars[0]);
#endif
        }
        else
        {
            /* Tolerate white space including between the two hex nybbles */
            while ((statsfile_chars_read < statsfile_len) &&
                   isspace(statsfile_buf[statsfile_chars_read]))
            {
#ifdef STATS_LOAD_DISCARD_DEBUG
                mctrl_print("Discard whitespace in hex digit: '%c'\n",
                            statsfile_buf[statsfile_chars_read]);
#endif
                statsfile_chars_read++;
            }

            if (statsfile_chars_read >= statsfile_len)
            {
                mctrl_print("Unexpected EOF with half a hex byte: '%c'\n", hexchars[0]);
                ret = -1;
                break;
            }

            hexchars[1] = statsfile_buf[statsfile_chars_read];
#ifdef STATS_LOAD_DISCARD_DEBUG
        mctrl_print("hexchars[1] value %d\n", hexchars[1]);
#endif
            statsfile_chars_read++;

            if (hexchars[1] == '#')
            {
                mctrl_print("Unexpected comment after half a hex byte: '%c%c'\n",
                    hexchars[0], hexchars[1]);
                ret = -1;
                break;
            }

            if (hexchars[1] == '\0')
            {
                mctrl_print("Unexpected nul character. Only UTF-8 encoding is supported.\n");
                ret = -1;
                break;
            }

            if (!isxdigit(hexchars[1]))
            {
                mctrl_print("Unexpected character after half a hex byte: '%c%c'\n",
                    hexchars[0], hexchars[1]);
                ret = -1;
                break;
            }

            if (hexstr2bin(hexchars, statsbin_next_byte, 1) < 0)
            {
#ifdef STATS_LOAD_DISCARD_DEBUG
                mctrl_err("Discard '%c%c'\n", hexchars[0], hexchars[1]);
#endif
            }
            else
            {
                statsbin_next_byte++;
            }
        }
    }

    *statsbin_length = statsbin_next_byte - statsfile_buf;
    if (*statsbin_length > STATS_TLV_OVERHEAD)
    {
        *statsbin_buffer = statsfile_buf;
    }
    else
    {
        mctrl_err("Ignoring %zu bytes of data\n", *statsbin_length);
        ret = -1;
        free(statsfile_buf);
        *statsbin_length = 0;
    }

    return ret;
}


int statistics_data_decode(struct morsectrl *mors, enum format_type format_val,
                 const uint8_t *stats, size_t stats_len)
{
    const uint8_t *buf = stats;
    int resp_sz = (int) stats_len;
    const struct format_table *table;
    int result = 0;

    table = stats_format_get_formatter_table(format_val);

    while (resp_sz > STATS_TLV_OVERHEAD )
    {
        stats_tlv_tag_t tag =  *((stats_tlv_tag_t *)buf);
        buf += sizeof(stats_tlv_tag_t);

        stats_tlv_len_t len =  le16toh(*((__force __le16 *)buf));
        buf += sizeof(stats_tlv_len_t);

        if ((len > resp_sz) || (len == 0))
        {
            mctrl_err("error: malformed TLV (tag %d/0x%x, len %u/0x%x, size %u)\n",
                    tag, tag, len, len, resp_sz);
            result += 1;
            break;
        }

        struct statistics_offchip_data *tag_definition = get_stats_offchip(mors, tag);
        if (tag_definition)
        {
            uint32_t morse_stat_fmt = le32toh((__force __le32)tag_definition->format);
            if ((morse_stat_fmt == MORSE_STATS_FMT_DEC) &&
                    !strncmp(tag_definition->type_str, "uint", 4))
            {
                morse_stat_fmt = MORSE_STATS_FMT_U_DEC;
            }

            if (!stats_filter_is_set() || !stats_filter_is_match(tag_definition->key))
            {
                stats_format_separate(format_val);

                if (morse_stat_fmt > MORSE_STATS_FMT_LAST)
                {
                    morse_stat_fmt = MORSE_STATS_FMT_LAST;
                }
                if (!table->format_func[morse_stat_fmt]((const char *) tag_definition->key,
                                                            (const uint8_t *)buf, len))
                {
                    result += 1;
                }
            }
        }
        else
        {
#define UNKNOWN_KEY_FOR_TAG_STR_LEN 26
            char unknown_key_str[UNKNOWN_KEY_FOR_TAG_STR_LEN];
            if (snprintf(unknown_key_str, sizeof(unknown_key_str), "UNKNOWN KEY for tag %d", tag)
                         >= sizeof(unknown_key_str))
            {
                /* Programming error, bail out */
                mctrl_err("'%.*s' is truncated!\n", (int) sizeof(unknown_key_str), unknown_key_str);
                result += 1;
                break;
            }
            else
            {
                if (mctrl_print_get_stream() != stdout)
                {
                    /* Output error to highlight what has happened */
                    mctrl_err("%s\n", unknown_key_str);
                }
                result += 1;
            }

            /* ... but continue with output in valid format */
            stats_format_separate(format_val);
            (void) table->format_func[MORSE_STATS_FMT_LAST](unknown_key_str,
                                                     (const uint8_t *)buf, len);
        }
        buf += len;

        resp_sz -= (STATS_TLV_OVERHEAD + len);
    }

    return result;
}


void statistics_types_dump(struct morsectrl *mors)
{
    int ii;

    mctrl_print("Stats types\n");
    for (ii = 0; ii < mors->n_stats; ii++)
    {
        mctrl_print("Type: %s\n", mors->stats[ii].type_str);
        mctrl_print("Name: %s\n", mors->stats[ii].name);
        mctrl_print("Key: %s\n\n", mors->stats[ii].key);
    }
}



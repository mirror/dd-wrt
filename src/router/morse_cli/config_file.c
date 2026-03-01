/*
 * Copyright 2022 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

/*
 * Read a config file and obtain the transport/interface/config options used.
 *
 * Values read will not override any provided on the command line.
 *
 * Transport and interface values will be stripped off the end and be passed
 * through. Config options will have the <transport>_ text stripped off the
 * front and added to a comma separated list (with no whitespace). For an
 * example see example.config.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "utilities.h"

#define MAX_LINE_LENGTH         (255)
#define MAX_CFG_LENGTH          (255)


static const char *trans_str = "transport";
static const char *iface_str = "interface";

/**
 * @brief Remove trailing whitespace from a string.
 *
 * @param buff  Buffer containing string to process
 */
static void config_file_remove_trailing_ws(char *buff)
{
    int ii;
    int len;

    if (!buff)
        return;

    len = strlen(buff);

    for (ii = (len - 1); ii >= 0; ii--)
    {
        if (!isspace(buff[ii]))
            return;

        buff[ii] = '\0';
    }
}

/**
 * @brief Remove leading whitespace from a string.
 *
 * @param buff  Buffer containing string to process
 *
 * @return      NULL if @ref buff is NULL otherwise a pointer to the first non-whitespace non-'='
 *              character.
 */
static const char *config_file_remove_leading_ws(const char *buff)
{
    int ii;

    if (!buff)
        return NULL;

    for (ii = 0; ii < MAX_LINE_LENGTH; ii++)
    {
        if (buff[ii] == '=')
            continue;

        if ((buff[ii] == '\0') || !isspace(buff[ii]))
            return &buff[ii];
    }

    return buff;
}

/**
 * @brief Copy a string into new memory and remove trailing whitespace.
 *
 * @param src   Source string to copy
 * @param dst   Destination to copy string to
 *
 * @return      0 on success otherwise -1
 */
static int config_file_passthrough(const char *src, char **dst)
{
    const char *ptr;

    if (!dst || !src)
        return -1;

    if (*dst)
        free(*dst);

    ptr = strchr(src, '=');

    if (!ptr)
    {
        mctrl_err("No '=' in line\n");
        return -1;
    }

    ptr = config_file_remove_leading_ws(ptr);

    if (!ptr)
    {
        mctrl_err("Option missing after '='\n");
        return -1;
    }

    *dst = malloc(strlen(ptr) + 1);
    if (!(*dst))
        return -1;

    snprintf(*dst, MAX_CFG_LENGTH, "%s", ptr);
    config_file_remove_trailing_ws(*dst);
    return 0;
}

/**
 * @brief Parse the transport from a buffer and copy to the output string
 *
 * @param buff          Source buffer containing the transport
 * @param trans_opts    Destination to return allocated string in
 * @param debug         Whether or not to print debug
 *
 * @return              0 on success otherwise -1
 */
static int config_file_trans(const char *buff, char **trans_opts, bool debug)
{
    int ret = config_file_passthrough(buff, trans_opts);

    if (debug)
    {
        if (!ret)
            mctrl_print("Config file transport: '%s'\n", *trans_opts);
        else
            mctrl_print("Config file invalid transport\n");
    }

    return ret;
}

/**
 * @brief Parse the interface from a buffer and copy to the output string
 *
 * @param buff          Source buffer containing the interface
 * @param iface_opts    Destination to return allocated string in
 * @param debug         Whether or not to print debug
 *
 * @return              0 on success otherwise -1
 */
static int config_file_iface(const char *buff, char **iface_opts, bool debug)
{
    int ret = config_file_passthrough(buff, iface_opts);

    if (debug)
    {
        if (!ret)
            mctrl_print("Config file interface: '%s'\n", *iface_opts);
        else
            mctrl_err("Config file invalid interface\n");
    }

    return ret;
}

/**
 * @brief Parse the additional configuration from a buffer and copy to the output string
 *
 * @param buff          Source buffer containing the additional configuration
 * @param cfg_opts      Destination to return allocated string in
 * @param debug         Whether or not to print debug
 *
 * @return              0 on success otherwise -1
 */
static int config_file_cfg(char *buff, char **cfg_opts, bool debug)
{
    int current_size;
    int total_size;
    char *ptr = *cfg_opts;

    if (!ptr)
    {
        if (debug)
            mctrl_print("Config options allocate buffer\n");

        /* Allocate memory and initialise first element to null terminator */
        ptr = malloc(MAX_CFG_LENGTH);

        if (!ptr)
            return -1;

        ptr[0] = '\0';
        *cfg_opts = ptr;
    }

    config_file_remove_trailing_ws(buff);
    if (debug)
        mctrl_print("New config options: '%s'\n", buff);

    current_size = strlen(ptr);

    /* Make sure the extra options fit in the memory. */
    total_size = current_size + strlen(buff) + 2;
    if (debug)
        mctrl_print("Total size after additional option: %d (%d + %d + 2)\n",
               total_size, current_size, (int) strlen(buff));

    if (total_size > MAX_CFG_LENGTH)
        return -1;

    /* Add config option separator if not the first option. */
    if (current_size)
    {
        ptr[current_size++] = ',';
        ptr[current_size] = '\0';
    }

    snprintf(&ptr[current_size], MAX_CFG_LENGTH - current_size, "%s", buff);

    if (debug)
        mctrl_print("Config options: '%s'\n", *cfg_opts);

    return 0;
}

int morsectrl_config_file_parse(const char *file_opts,
                                char **trans_opts,
                                char **iface_opts,
                                char **cfg_opts,
                                bool debug)
{
    int ret = 0;
    FILE *cfg_file;
    char *line;
    char *ptr;

    if (!trans_opts || !iface_opts || !cfg_opts)
        return -1;

    if (debug)
        mctrl_print("Start parsing config file\n");

    cfg_file = fopen(file_opts, "r");

    if (cfg_file == NULL)
        return -1;

    line = malloc(MAX_LINE_LENGTH);

    if (!line)
    {
        fclose(cfg_file);
        return -1;
    }

    while (fgets(line, MAX_LINE_LENGTH, cfg_file))
    {
        if (debug)
            mctrl_print("Line: %s", line);

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
            continue;

        ptr = (char *) config_file_remove_leading_ws(line);

        if (!strncmp(ptr, trans_str, strlen(trans_str)) && !*trans_opts)
        {
            ret = config_file_trans(&ptr[strlen(trans_str)], trans_opts, debug);
            if (ret)
                break;

            continue;
        }

        if (!strncmp(ptr, iface_str, strlen(iface_str)) && !*iface_opts)
        {
            ret = config_file_iface(&ptr[strlen(iface_str)], iface_opts, debug);
            if (ret)
                break;

            continue;
        }

        if (*trans_opts && !strncmp(ptr, *trans_opts, strlen(*trans_opts)))
        {
            ret = config_file_cfg(&ptr[strlen(*trans_opts) + 1], cfg_opts, debug);
            if (ret)
            {
                mctrl_err("Failed to parse config file transport config options\n");
                break;
            }

            continue;
        }
    }

    fclose(cfg_file);
    free(line);
    if (debug)
        mctrl_print("\nFinished parsing config file\n");

    return ret;
}

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
*/

/**
**  @file        sp_cvs.c
**
**  @author      Taimur Aslam
**  @author      Todd Wease
**
**  @brief       Decode and detect CVS vulnerabilities
**
**  This CVS detection plugin provides support for detecting published CVS vulnerabilities. The
**  vulnerabilities that can be detected are:
**  Bugtraq-10384, CVE-2004-0396: "Malformed Entry Modified and Unchanged flag insertion"
**
**  Detection Functions:
**
**  cvs: invalid-entry;
**
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>

#include "sf_types.h"
#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "mstring.h"
#include "snort_bounds.h"

#include "sp_cvs.h"

#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats cvsPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

/* function prototypes */
static void CvsInit(struct _SnortConfig *, char *, OptTreeNode *, int);
static void CvsRuleParse(char *, CvsRuleOption *);
static int CvsDetect(void *option_data, Packet *p);
static int CvsDecode(const uint8_t *, uint16_t, CvsRuleOption *);
static void CvsGetCommand(const uint8_t *, const uint8_t *, CvsCommand *);
static int CvsCmdCompare(const char *, const uint8_t *, int);
static int CvsValidateEntry(const uint8_t *, const uint8_t *);
static void CvsGetEOL(const uint8_t *, const uint8_t *,
                      const uint8_t **, const uint8_t **);

uint32_t CvsHash(void *d)
{
    uint32_t a,b,c;
    CvsRuleOption *data = (CvsRuleOption *)d;

    a = data->type;
    b = RULE_OPTION_TYPE_CVS;
    c = 0;

    final(a,b,c);

    return c;
}

int CvsCompare(void *l, void *r)
{
    CvsRuleOption *left = (CvsRuleOption *)l;
    CvsRuleOption *right = (CvsRuleOption *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->type == right->type)
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/*
**  NAME
**     SetupCvs
**        Register the CVS detection plugin.
**
*/
/**
**
**  @return None
**
*/

void SetupCvs(void)
{
    RegisterRuleOption("cvs", CvsInit, NULL, OPT_TYPE_DETECTION, NULL);

#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("cvs", &cvsPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: CVS Setup\n"););
}



/*
**  NAME
**    CvsInit
**       Initialize the CVS context and set it up so we can detect commands.
**
*/
/**
**
**  @return None
**
*/

static void CvsInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    CvsRuleOption *cvs_rule_option;
    void *ds_ptr_dup;
    OptFpList *ofl;

    cvs_rule_option = (CvsRuleOption *)SnortAlloc(sizeof(CvsRuleOption));

    CvsRuleParse(data, cvs_rule_option);

    if (add_detection_option(sc, RULE_OPTION_TYPE_CVS, (void *)cvs_rule_option, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(cvs_rule_option);
        cvs_rule_option = ds_ptr_dup;
    }

    /* Attach detection function to rule's detect function ptr */
    ofl = AddOptFuncToList(CvsDetect, otn);
    ofl->type = RULE_OPTION_TYPE_CVS;
    ofl->context = (void *)cvs_rule_option;
}



/*
**  NAME
**    CvsRuleParse
**       Parse the CVS rules and set the threshold criteria.
*/
/**
**
**  @return None
**
*/

static void CvsRuleParse(char *rule_args, CvsRuleOption *cvs_rule_option)
{
    char **toks;
    int num_toks = 0;


    toks = mSplit(rule_args, CVS_CONFIG_DELIMITERS, 2, &num_toks, 0);

    switch (num_toks)
    {
        /* no arguments */
        case 1:
            if (strcasecmp(toks[0], CVS_CONF_INVALID_ENTRY_STR) == 0)
            {
                cvs_rule_option->type = CVS_INVALID_ENTRY;
            }
            else
            {
                FatalError("%s(%d) => Invalid argument specified for CVS rule: %s\n",
                           file_name, file_line, toks[0]);
            }

            break;

        default:
            FatalError("%s(%d) => No or wrong number of arguments "
                       "specified for CVS rule\n",
                       file_name, file_line);
            break;
    }

    mSplitFree(&toks, num_toks);
}


/*
**  NAME
**    CvsDetect
**    This function is called on a per rule basis for CVS detection.
**
*/
/**
**
**  @return integer
**  @retval CVS_NO_ALERT
**  @retval CVS_ALERT
**
*/
static int CvsDetect(void *option_data, Packet *p)
{
    int ret;
    int rval = DETECTION_OPTION_NO_MATCH;
    CvsRuleOption *cvs_rule_option = (CvsRuleOption *)option_data;


    if (p == NULL)
    {
        return rval;
    }

    if ((p->tcph == NULL) || (p->data == NULL) || (p->dsize == 0))
    {
        return rval;
    }

    if (cvs_rule_option == NULL)
    {
        return rval;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "CVS begin detection\n"););

    ret = CvsDecode(p->data, p->dsize, cvs_rule_option);

    if (ret == CVS_ALERT)
    {
        rval = DETECTION_OPTION_MATCH;
    }

    return rval;
}


/*
**  NAME
**    CvsDecode
**    This main decode function. Decode the CVS commands and detect the vulnerabilities.
**
*/
/**
**
**  @return integer
**
*/

static int CvsDecode(const uint8_t *data, uint16_t data_len,
                     CvsRuleOption *cvs_rule_option)
{
    const uint8_t *line, *end;
    const uint8_t *eol = NULL, *eolm = NULL;
    CvsCommand command;
    int ret;


    line = data;
    end = data + data_len;

    /* loop through data, analyzing a line at a time */
    while (line < end)
    {
        /* CVS commands are delimited by \n so break them up */
        CvsGetEOL(line, end, &eol, &eolm);

        /* Put command and argument into structure */
        CvsGetCommand(line, eolm, &command);

        /* shouldn't happen as long as line < end, but ... */
        if (command.cmd_str == NULL)
            return CVS_NO_ALERT;

        DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "CVS command\n"
                                              "  comand: %.*s\n"
                                              "argument: %.*s\n",
                                command.cmd_str_len, (char *)command.cmd_str,
                                command.cmd_arg == NULL ? 4 : command.cmd_arg_len,
                                command.cmd_arg == NULL ? "none" : (char *)command.cmd_arg););

        switch (cvs_rule_option->type)
        {
            case CVS_INVALID_ENTRY:
                if (CvsCmdCompare(CVS_ENTRY_STR, command.cmd_str, command.cmd_str_len) == 0)
                {
                    ret = CvsValidateEntry(command.cmd_arg,
                                           (command.cmd_arg + command.cmd_arg_len));

                    if ((ret == CVS_ENTRY_INVALID)&&(eol < end))
                    {
                        return CVS_ALERT;
                    }
                }

                break;

            default:
                break;
        }

        line = eol;
    }

    return CVS_NO_ALERT;
}


/*
**  NAME
**    CvsCmdCompare
**       Compares two pointers to char to see if they are equal.
**       The first arg is NULL terminated.  The second is not and
**       it's length is passed in.
**
*/
/**
**  @return 0 if equal
**  @return 1 if not equal
**
*/

static int CvsCmdCompare(const char *cmd, const uint8_t *pkt_cmd, int pkt_cmd_len)
{
    if (((size_t)pkt_cmd_len == strlen(cmd)) &&
        (memcmp(pkt_cmd, cmd, pkt_cmd_len) == 0))
    {
        return 0;
    }

    return 1;
}


/*
**  NAME
**    CvsGetCommand
**       Takes a line and breaks it up into command and argument.
**       It modifies the data in the string by replacing the first
**       space character it sees with '\0'.  A pointer to the string
**       created by the replacement is put in the CvsCommand structure's
**       command member.  A pointer to the rest of the string after
**       the replacement '\0' is put into the structure's command
**       argument member.  If there isn't a space, the entire line
**       is put in the command and the command argument is set to
**       NULL.
**
*/
/**
**  @return None
**
*/

static void CvsGetCommand(const uint8_t *line, const uint8_t *end, CvsCommand *cmd)
{
    const uint8_t *cmd_end;


    if (cmd == NULL)
        return;

    /* no line, no command or args */
    if (line == NULL)
    {
        cmd->cmd_str = NULL;
        cmd->cmd_str_len = 0;
        cmd->cmd_arg = NULL;
        cmd->cmd_arg_len = 0;

        return;
    }

    cmd->cmd_str = line;

    cmd_end = (const uint8_t *)memchr(line, CVS_COMMAND_SEPARATOR, end - line);
    if (cmd_end != NULL)
    {
        cmd->cmd_str_len = cmd_end - line;
        cmd->cmd_arg = cmd_end + 1;
        cmd->cmd_arg_len = end - cmd_end - 1;
    }
    else
    {
        cmd->cmd_str_len = end - line;
        cmd->cmd_arg = NULL;
        cmd->cmd_arg_len = 0;
    }
}


/*
**  NAME
**    CvsValidateEntry
**       Checks Entry argument to make sure it is well formed
**       An entry sent to the server should look like:
**       /file/version///
**       e.g. '/cvs.c/1.5///'
**       There should be nothing between the third and
**       fourth slashes
**
*/
/**
**  @return CVS_ENTRY_VALID if valid
**  @return CVS_ENTRY_INVALID if invalid
**
*/

static int CvsValidateEntry(const uint8_t *entry_arg, const uint8_t *end_arg)
{
    int slashes = 0;


    if ((entry_arg == NULL) || (end_arg == NULL))
    {
        return CVS_ENTRY_VALID;
    }

    /* There should be exactly 5 slashes in the string */
    while (entry_arg < end_arg)
    {
        /* if on the 3rd slash, check for next char == '/' or '+'
         * This is where the heap overflow on multiple Is-Modified
         * commands occurs */
        if (slashes == 3)
        {
            if((*entry_arg != '/')&&(*entry_arg != '+'))
            {
                return CVS_ENTRY_INVALID;
            }
        }
        if (*entry_arg != '/')
        {
            entry_arg = memchr(entry_arg, '/', end_arg - entry_arg);
            if (entry_arg == NULL)
                break;
        }

        slashes++;
        entry_arg++;
    }

    if (slashes != 5)
    {
        return CVS_ENTRY_INVALID;
    }

    return CVS_ENTRY_VALID;
}


/*
**  NAME
**    CvsGetEOL
**       Gets a line from the data string.
**       Sets an end-of-line marker to point to the marker
**       and an end-of-line pointer to point after marker
**
*/
/**
**  @return void
**
*/

static void CvsGetEOL(const uint8_t *ptr, const uint8_t *end,
                      const uint8_t **eol, const uint8_t **eolm)
{
    *eolm = (uint8_t *)memchr(ptr, CVS_COMMAND_DELIMITER, end - ptr);
    if (*eolm == NULL)
    {
        *eolm = end;
        *eol = end;
    }
    else
    {
        *eol = *eolm + 1;
    }
}


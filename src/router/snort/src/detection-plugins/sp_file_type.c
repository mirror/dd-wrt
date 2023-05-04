/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2013-2013 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

// 
// file     sp_file_type.c
// author   Victor Roemer <vroemer@sourcefire.com>
//

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <assert.h>

#include "sf_types.h"
#include "snort_bounds.h"
#include "rules.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "mstring.h"
#include "strvec.h"

#include "file-process/file_api.h"
#include "file-process/file_service_config.h"
#include "file-process/libs/file_lib.h"
#include "file-process/libs/file_config.h"

typedef struct _FileTypeOpt
{
    uint32_t *ids;
    unsigned count;
} FileTypeOpt;

static void FileType_Init (struct _SnortConfig*, char*, OptTreeNode*, int);
static void FileGroup_Init (struct _SnortConfig*, char*, OptTreeNode*, int);
static void FileType_Parse (const void*, const char*, FileTypeOpt*);
static void FileGroup_Parse (const void*, const char*, FileTypeOpt*);
static int FileType_Check (void*, Packet*);

#if defined(PERF_PROFILING)
PreprocStats _sp_file_type_perf_stats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif // PERF_PROFILING

// qsort() callback 
static int Compare_Ids(const void *p1, const void *p2)
{
    return (*(const uint32_t *)p1 - *(const uint32_t *)p2);
}

// Sort the rule option file id's 
static void Sort_Ids(uint32_t *ids, size_t cnt)
{
    size_t i;

    assert( ids );
    assert( cnt );

    qsort(ids, cnt, sizeof(*ids), Compare_Ids);

    for ( i = 1; i < cnt; ++i )
    {
        // XXX I am not sure that this check is necessary anymore.
        if ( ids[i] == ids[i-1] )
            ParseError("Duplicate file type configured in rule.");
    }
}

// Check if 'type' was previously configured in the rule option.
static bool isPrevType(const char *type, void *prev_types)
{
    const char * prev;
    int i = 0;

    while ( (prev = StringVector_Get(prev_types, i++)) )
        if ( prev && strcmp(type, prev) == 0 )
            return true;
    return false;
}

// Generate a unique digest of the rule option. 
uint32_t FileTypeHash(const void *option)
{
    uint32_t abc[3];
    unsigned i;
    const FileTypeOpt *opt = (FileTypeOpt*)option;

    abc[0] = opt->count;
    abc[1] = RULE_OPTION_TYPE_FILE_TYPE;
    abc[2] = 0;

    mix(abc[0], abc[1], abc[2]);

    for ( i = 0; i < opt->count; ++i )
        abc[i % 3] += opt->ids[i];

    final(abc[0], abc[1], abc[2]);

    return abc[2];
}

// Compare 2 rule options for duplicity 
int FileTypeCompare(const void * _left, const void * _right)
{
    unsigned i;
    const FileTypeOpt *left = (FileTypeOpt*)_left;
    const FileTypeOpt *right = (FileTypeOpt*)_right;

    if ( !left || !right )
        return DETECTION_OPTION_NOT_EQUAL;

    if ( left->count != right->count )
        return DETECTION_OPTION_NOT_EQUAL;

    for ( i = 0; i < left->count; ++i )
        if ( left->ids[i] != right->ids[i] )
            return DETECTION_OPTION_NOT_EQUAL;

    return DETECTION_OPTION_EQUAL;
}

// Free options 
void FileTypeFree(void *option)
{
    FileTypeOpt *opt = (FileTypeOpt*)option;

    assert( opt );
    assert( opt->ids );
    assert( opt->count > 0 );

    free(opt->ids);
    free(opt);
}

// Register options 
void SetupFileType(void)
{
    RegisterRuleOption("file_type", FileType_Init, NULL, OPT_TYPE_ACTION, NULL);
    RegisterRuleOption("file_group", FileGroup_Init, NULL, OPT_TYPE_ACTION, NULL);

#if defined(PERF_PROFILING)
    RegisterPreprocessorProfile("file_type", &_sp_file_type_perf_stats,
            3, &ruleOTNEvalPerfStats, NULL);
#endif // PERF_PROFILING
}

// Common code used for initialization 
static void Rule_Init(struct _SnortConfig *conf, OptTreeNode *otn, void *option)
{
    int ret;
    OptFpList *fpl;
    void *temp = NULL;
   
    // Auto-magically enable file-type detection when no explicit
    // configuration exists.
    file_api->enable_file_type(conf, NULL);

    fpl = AddOptFuncToList(FileType_Check, otn); 
    fpl->type = RULE_OPTION_TYPE_FILE_TYPE;

    ret = add_detection_option(conf, fpl->type, option, &temp);
    if ( ret == DETECTION_OPTION_EQUAL )
    {
        FileTypeFree(option);
        option = temp;
    }

    fpl->context = option;
}

// Initialize the "file_type" rule option 
static void FileType_Init(struct _SnortConfig *conf, char *in,
        OptTreeNode *otn, int pro)
{
    FileTypeOpt *opt = SnortAlloc(sizeof(*opt));
    FileType_Parse(conf->file_config, in, opt);
    Rule_Init(conf, otn, opt);
}

// Initialize the "file_group" rule option 
static void FileGroup_Init(struct _SnortConfig *conf, char *in,
        OptTreeNode *otn, int pro)
{
    FileTypeOpt *opt = SnortAlloc(sizeof(*opt));
    FileGroup_Parse(conf->file_config, in, opt);
    Rule_Init(conf, otn, opt);
}

static int args_look_ok( const char * src, const char **err )
{

    enum { ST0, ST1, ERR } state;

    assert( err != NULL );
    state = ST0;

    while ( *src )
    {
        char c = *src;
        src += 1;

        switch ( state ) {
            case ST0:
                if ( IS_RULE_TYPE_IDENT((int)c) )
                {
                    state = ST1;
                }
                else
                {
                    *err = "Expected a file type or version.",
                    state = ERR;
                }
                break;

            case ST1:
                if ( c == ',' || c == '|' )
                {
                    state = ST0;
                }
                else if ( !IS_RULE_TYPE_IDENT((int)c) )
                {
                    *err = "Expected a file type, version or ',' or '|' character.";
                    state = ERR;
                }
                break;

            case ERR:
                // Cant get here; silence compiler noise.
                break;
        }

        if ( state == ERR )
            return -1;
    }

    if ( state == ST0 )
    {
        *err = "Trailing ',' or '|' character.";
        return -1;
    }

    return 0;
}

// Parse "file_type" keyword 
static void FileType_Parse(const void *conf, const char *in, FileTypeOpt *opt)
{
    int i;
    int num_toks = 0;
    char **toks;
    void *prev_types;
    const char *err;

    toks = mSplit(in, "|", 0, &num_toks, 0);

    if ( num_toks < 1 )
    {
        ParseError("Missing argument to 'file_type' option.");
    }

    if ( args_look_ok(in, &err) == -1 )
    {
        assert( err != NULL );
        ParseError("Error found in 'file_type' arguments: %s\n", err);
    }

    prev_types = StringVector_New();

    for ( i = 0; i < num_toks; ++i )
    {
        char *temp;
        char **subs;
        int num_subs = 0;
        bool found = false;

        temp = SnortStrdup(toks[i]);
        subs = mSplit(temp, ",", 0, &num_subs, 0);

        if ( num_subs >= 2 )
        {
            // Collect all rule ids for a specific version of a type.
            int j;
            const char *type = subs[0];

            if ( isPrevType(type, prev_types) )
            {
                ParseError("file type \'%s\' was specified multiple times.\n", type);
            }
        
            StringVector_Add(prev_types, type);

            for ( j = 1; j < num_subs; j++ )
            {
                const char *version = subs[j];
                found = file_IDs_from_type_version(conf, type, version,
                            &opt->ids, &opt->count);

                if ( !found )
                {
                    ParseError("\'%s\' version \'%s\' is not a configured "
                            "file type.", type, version);
                }
            }
        }
        else
        {
            const char * type = toks[i];

            if ( isPrevType(type, prev_types) )
            {
                ParseError("file type \'%s\' is specified multiple times.\n", type);
            }

            StringVector_Add(prev_types, type);

            // Collect all rule ids for a type.
            found = file_IDs_from_type(conf, type, &opt->ids, &opt->count);
        }

        if ( !found )
        {
            ParseError("\'%s\' is not a configured file type.", toks[i]);
        }

        mSplitFree(&subs, num_subs);
        free(temp);
    }

    StringVector_Delete(prev_types);
    mSplitFree(&toks, num_toks);
    Sort_Ids(opt->ids, opt->count);
}

// Parse "file_group" keyword 
static void FileGroup_Parse(const void *conf, const char *in, FileTypeOpt *opt)
{
    if ( IsEmptyStr(in) )
        ParseError("Missing argument to 'file_group' option.");

    // Collect all rule ids in group.
    if ( !file_IDs_from_group(conf, in, &opt->ids, &opt->count) )
        ParseError("\'%s\' is not a configured file group.", in);

    Sort_Ids(opt->ids, opt->count);
}

// Check if the session is transferring the file type 
static int FileType_Check(void *option, Packet *p)
{
    uint32_t id;
    const FileTypeOpt *opt = (FileTypeOpt*)option;

    PROFILE_VARS;
    PREPROC_PROFILE_START(_sp_file_type_perf_stats);

    assert( file_api->is_file_service_enabled() );
    assert( file_api->get_max_file_depth(NULL, false) >= 0 );

    assert( opt->ids );
    assert( opt->count > 0 );

    if ( !p->ssnptr )
    {
        PREPROC_PROFILE_END(_sp_file_type_perf_stats);
        return DETECTION_OPTION_NO_MATCH;
    }

    id = file_api->get_file_type_id(p->ssnptr);
    if ( id != SNORT_FILE_TYPE_UNKNOWN )
    {
        void *found = bsearch(&id, opt->ids, opt->count, sizeof(opt->ids[0]),
                        Compare_Ids);
        if ( found )
        {
            PREPROC_PROFILE_END(_sp_file_type_perf_stats);
            return DETECTION_OPTION_MATCH;
        }
    }

    PREPROC_PROFILE_END(_sp_file_type_perf_stats);
    return DETECTION_OPTION_NO_MATCH;
}

/*
 **
 **
 **  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 **  Copyright (C) 2012-2013 Sourcefire, Inc.
 **
 **  This program is free software; you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License Version 2 as
 **  published by the Free Software Foundation.  You may not use, modify or
 **  distribute this program under any other version of the GNU General
 **  Public License.
 **
 **  This program is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with this program; if not, write to the Free Software
 **  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  5.25.2012 - Initial Source Code. Hcao
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sf_types.h"
#include "util.h"
#include "mstring.h"
#include "parser.h"
#include "memory_stats.h"

#include "sfutil/strvec.h"

#include "file_lib.h"
#include "file_identifier.h"
#include "file_config.h"

typedef void (*ParseFileOptFunc)(RuleInfo*, char *);

typedef struct _FileOptFunc
{
    char *name;
    int args_required;
    int only_once;  /*the option is one per file rule*/
    ParseFileOptFunc parse_func;

} FileOptFunc;


#define FILE_OPT__TYPE              "type"
#define FILE_OPT__ID                "id"
#define FILE_OPT__VERSION           "ver"
#define FILE_OPT__CATEGORY          "category"
#define FILE_OPT__MSG               "msg"
#define FILE_OPT__REVISION          "rev"
#define FILE_OPT__CONTENT           "content"
#define FILE_OPT__OFFSET            "offset"
#define FILE_OPT__GROUP            "group"

#define FILE_REVISION_MAX    UINT32_MAX
#define FILE_OFFSET_MAX      UINT32_MAX

static void ParseFileRuleType(RuleInfo *, char *);
static void ParseFileRuleID(RuleInfo *, char *);
static void ParseFileRuleVersion(RuleInfo *, char *);
static void ParseFileRuleCategory(RuleInfo *, char *);
static void ParseFileRuleMessage(RuleInfo *, char *);
static void ParseFileRevision(RuleInfo *, char *);
static void ParseFileContent(RuleInfo *, char *);
static void ParseFileOffset(RuleInfo *, char *);
static void ParseFileGroup(RuleInfo *, char *);

static const FileOptFunc file_options[] =
{
    { FILE_OPT__TYPE,     1, 1, ParseFileRuleType },
    { FILE_OPT__ID,       1, 1, ParseFileRuleID },
    { FILE_OPT__VERSION,  0, 1, ParseFileRuleVersion },
    { FILE_OPT__CATEGORY, 1, 1, ParseFileRuleCategory },
    { FILE_OPT__MSG,      0, 1, ParseFileRuleMessage },
    { FILE_OPT__REVISION, 0, 1, ParseFileRevision },
    { FILE_OPT__CONTENT,  1, 0, ParseFileContent },
    { FILE_OPT__OFFSET,   1, 0, ParseFileOffset },
    { FILE_OPT__GROUP,    1, 0, ParseFileGroup },
    { NULL, 0, 0, NULL }   /* Marks end of array */
};


/* Used for content modifiers that are used as rule options - need to get the
 * last magic which is the one they are modifying.  If there isn't a last magic
 * error that a content must be specified before the modifier */

static inline MagicData * GetLastMagic(RuleInfo *rule, const char *option)
{
    MagicData *mdata;
    MagicData *lastMagic = NULL;

    if ((rule) && (rule->magics))
    {
        for (mdata = rule->magics; mdata->next != NULL; mdata = mdata->next);
        lastMagic = mdata;
    }
    if (lastMagic == NULL)
    {
        ParseError("Please place \"content\" rules before \"%s\" modifier",
                option == NULL ? "unknown" : option);
    }
    return lastMagic;
}

static inline bool valid_rule_type_str( const char *src )
{
    assert( src );
    assert( *src );

    while ( *src ) {
        if ( !IS_RULE_TYPE_IDENT((int)*src) )
            return false;
        src++;
    }

    return true;
}

static void ParseFileRuleType(RuleInfo *rule, char *args)
{

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Type args: %s\n", args););

    if (args == NULL)
        ParseError("Type rule option requires an argument.");

    if ( !valid_rule_type_str(args) )
    {
        ParseError("Invalid argument to 'type' rule option: '%s'.  "
            "Can only contain '0-9','A-Z','a-z','_' and '.' characters.", args);
    }

    rule->type = SnortStrdup(args);
}

static void ParseFileRuleID(RuleInfo *rule, char *args)
{
    unsigned long int id;
    char *endptr;

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"ID args: %s\n", args););

    if (args == NULL)
        ParseError("ID rule option requires an argument.");

    id = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0')||(id > FILE_ID_MAX))
    {
        ParseError("Invalid argument to 'id' rule option: %s.  "
                "Must be a positive integer.", args);
    }

    rule->id = (uint32_t)id;
}

static void ParseFileRuleCategory(RuleInfo *rule, char *args)
{

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Category args: %s\n", args););

    if (args == NULL)
        ParseError("Category rule option requires an argument.");

    rule->category = SnortStrdup(args);
}

static void ParseFileRuleVersion(RuleInfo *rule, char *args)
{

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Version args: %s\n", args););

    if (args == NULL)
        ParseError("Version rule option requires an argument.");

    if ( !valid_rule_type_str(args) )
    {
        ParseError("Invalid argument to 'ver' rule option: '%s'.  "
            "Can only contain '0-9','A-Z','a-z','_' and '.' characters.", args);
    }

    rule->version = SnortStrdup(args);
}

static void ParseFileRuleMessage(RuleInfo *rule, char *args)
{
    size_t i;
    int escaped = 0;
    char msg_buf[2048];  /* Arbitrary length, but should be enough */

    if (args == NULL)
        ParseError("Message rule option requires an argument.");

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Msg args: %s\n", args););

    if (*args == '"')
    {
        /* Have to have at least quote, char, quote */
        if (strlen(args) < 3)
            ParseError("Empty argument passed to rule option 'msg'.");

        if (args[strlen(args) - 1] != '"')
        {
            ParseError("Unmatch quote in rule option 'msg'.");
        }

        /* Move past first quote and NULL terminate last quote */
        args++;
        args[strlen(args) - 1] = '\0';

        /* If last quote is escaped, fatal error.
         * Make sure the backslash is not escaped */
        if ((args[strlen(args) - 1] == '\\') &&
                (strlen(args) > 1) && (args[strlen(args) - 2] != '\\'))
        {
            ParseError("Unmatch quote in rule option 'msg'.");
        }
    }

    /* Only valid escaped chars are ';', '"' and '\' */
    /* Would be ok except emerging threats rules are escaping other chars */
    for (i = 0; (i < sizeof(msg_buf)) && (*args != '\0');)
    {
        if (escaped)
        {
            msg_buf[i++] = *args;
            escaped = 0;
        }
        else if (*args == '\\')
        {
            escaped = 1;
        }
        else
        {
            msg_buf[i++] = *args;
        }

        args++;
    }

    if (escaped)
    {
        ParseError("Message in 'msg' rule option has invalid escape character."
                "\n");
    }

    if (i == sizeof(msg_buf))
    {
        ParseError("Message in 'msg' rule option too long.  Please limit "
                "to %d characters.", sizeof(msg_buf));
    }

    msg_buf[i] = '\0';

    DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Message: %s\n", msg_buf););

    rule->message = SnortStrdup(msg_buf);
}

static void ParseFileRevision(RuleInfo *rule, char *args)
{
    unsigned long int rev;
    char *endptr;

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Revision args: %s\n", args););

    if (args == NULL)
        ParseError("Revision rule option requires an argument.");

    rev = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0') || (rev > FILE_REVISION_MAX))
    {
        ParseError("Invalid argument to 'rev' rule option: %s.  "
                "Must be a positive integer.", args);
    }

    rule->rev = (uint32_t)rev;
}

static uint8_t* convertTextToHex(char *text, int *size)
{

    int i;
    char **toks;
    int num_toks;
    char hex_buf[3];
    uint8_t *hex;

    toks = mSplit(text, " ", 0, &num_toks, 0);

    if (num_toks <= 0)
    {
        ParseError("No hexmode argument.");
    }

    hex = (uint8_t*) SnortPreprocAlloc(1, num_toks, PP_FILE, PP_MEM_CATEGORY_SESSION);
    *size = num_toks;

    memset(hex_buf, 0, sizeof(hex_buf));

    for (i = 0; i < num_toks; i++)
    {
        char *current_ptr = toks[i];
        if (2 != strlen(current_ptr))
        {
            ParseError("Content hexmode argument has invalid "
                    "number of hex digits.  The argument '%s' "
                    "must contain a full even byte string.", current_ptr);
        }

        if(isxdigit((int) *current_ptr))
        {
            hex_buf[0] = *current_ptr;
        }
        else
        {
            ParseError("\"%c\" is not a valid hex value, "
                    "please input hex values (0x0 - 0xF)",
                    (char) *current_ptr);
        }

        current_ptr++;

        if(isxdigit((int) *current_ptr))
        {
            hex_buf[1] = *current_ptr;
        }
        else
        {
            ParseError("\"%c\" is not a valid hex value, "
                    "please input hex values (0x0 - 0xF)",
                    (char) *current_ptr);
        }
        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Hex buffer: %s\n", hex_buf););
        hex[i] = (uint8_t) strtol(hex_buf, (char **) NULL, 16)&0xFF;
        memset(hex_buf, 0, sizeof(hex_buf));
        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Hex value: %x\n", hex[i]););

    }
    mSplitFree(&toks, num_toks);
    return hex;
}

static void ParseFileContent(RuleInfo *rule, char *args)
{
    MagicData *predata = NULL;
    MagicData *newdata;
    char *start_ptr;
    char *end_ptr;
    char *tmp;

    if (args == NULL)
        ParseError("Parse File Magic Got Null enclosed in vertical bar (|)!");

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Content args: %s\n", args););

    while(isspace((int)*args))
        args++;

    /* find the start of the data */
    start_ptr = strchr(args, '|');
    if (start_ptr != args)
        ParseError("Content data needs to be enclosed in vertical bar (|)!");

    /* move the start up from the beggining quotes */
    start_ptr++;

    /* find the end of the data */
    end_ptr = strrchr(start_ptr, '|');

    if (end_ptr == NULL)
        ParseError("Content data needs to be enclosed in vertical bar (|)!");

    /* Move the null termination up a bit more */
    *end_ptr = '\0';

    /* Is there anything other than whitespace after the trailing
     * double quote? */
    tmp = end_ptr + 1;
    while (*tmp != '\0' && isspace ((int)*tmp))
        tmp++;

    if (strlen (tmp) > 0)
    {
        ParseError("Bad data (possibly due to missing semicolon) after "
                "trailing double quote.");
    }

    if (rule->magics)
    {
        for (predata = rule->magics; predata->next != NULL;
                predata = predata->next);
    }
    
    newdata = SnortPreprocAlloc(1, sizeof(*newdata), PP_FILE, PP_MEM_CATEGORY_SESSION);

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Content args: %s\n", start_ptr););

    newdata->content =  convertTextToHex(start_ptr, &(newdata->content_len));

    if (predata)
    {
        predata->next = newdata;
    }
    else
    {
        rule->magics = newdata;
    }

}

static void ParseFileOffset(RuleInfo *rule, char *args)
{
    unsigned long int offset;
    char *endptr;
    MagicData *mdata;

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Offset args: %s\n", args););

    if (args == NULL)
        ParseError("Offset rule option requires an argument.");

    offset = SnortStrtoul(args, &endptr, 0);
    if ((errno == ERANGE) || (*endptr != '\0')|| (offset > FILE_OFFSET_MAX))
    {
        ParseError("Invalid argument to 'offset' rule option: %s.  "
                "Must be a positive integer.", args);
    }
    mdata = GetLastMagic(rule, "offset");
    mdata->offset = (uint32_t)offset;
}

static void ParseFileGroup(RuleInfo * rule, char * args)
{
    char **toks;
    int num_toks, i;

    DEBUG_WRAP( DebugMessage(DEBUG_FILE,"Group args: %s\n", args); );

    toks = mSplit(args, ",", 0, &num_toks, 0);

    if (num_toks < 1)
    {
        ParseError("Group rule option requires an argument.");
    }

    rule->groups = StringVector_New();

    for (i = 0; i < num_toks; i++)
        StringVector_Add(rule->groups, toks[i]);

    mSplitFree(&toks, num_toks);
}

static void parse_options(char *option_name, char *option_args,
        char *configured, RuleInfo *rule)
{
    int i;
    for (i = 0; file_options[i].name != NULL; i++)
    {
        if (strcasecmp(option_name, file_options[i].name))
            continue;

        if (configured[i] && file_options[i].only_once)
        {
            ParseError("Only one '%s' rule option per rule.", option_name);
        }

        if ((option_args == NULL) && file_options[i].args_required)
        {
            ParseError("No argument passed to keyword \"%s\".  "
                    "Make sure you didn't forget a ':' or the "
                    "argument to this keyword.\n",option_name);
        }

        file_options[i].parse_func(rule, option_args);
        configured[i] = 1;
        return;
    }
    /* Unrecognized rule option */
    ParseError("Unknown rule option: '%s'.", option_name);

}

#ifdef DEBUG_MSGS
static int file_rule_print(RuleInfo *rule)
{
    MagicData *mdata;

    if (!rule)
    {
        DebugMessage(DEBUG_FILE,"Rule is NULL!\n");
        return 0;
    }
    DebugMessage(DEBUG_FILE,"File type Id: %d\n", rule->id);
    DebugMessage(DEBUG_FILE,"File type name: %s\n", rule->type);
    DebugMessage(DEBUG_FILE,"File type Category: %s\n", rule->category);
    DebugMessage(DEBUG_FILE,"Rule revision: %d\n", rule->rev);
    DebugMessage(DEBUG_FILE,"Rule message: %s\n", rule->message);

    if (!rule->magics)
    {
        DebugMessage(DEBUG_FILE,"No megic defined in rule!\n");
    }

    for (mdata = rule->magics; mdata != NULL; mdata = mdata->next)
    {
        int i;
        int buff_size = mdata->content_len * 2 + 1;
	char *buff = SnortPreprocAlloc(1, buff_size, PP_FILE, 
                PP_MEM_CATEGORY_SESSION);
        char *start_ptr = buff;

        DebugMessage(DEBUG_FILE,"Magic offset: %d\n", mdata->offset);
        DebugMessage(DEBUG_FILE,"Magic length: %d\n", mdata->content_len);
        for (i = 0; (i < mdata->content_len) && (buff_size > 0); i++)
        {
            int num_read;
            num_read = snprintf(start_ptr, buff_size, "%x",mdata->content[i]);
            start_ptr += num_read;
            buff_size -= num_read;
        }
        DebugMessage(DEBUG_FILE,"Magic content: %s\n", buff);
        SnortPreprocFree(buff, buff_size, PP_FILE, PP_MEM_CATEGORY_SESSION);
    }
    return rule->id;
}
#endif

static inline void
__add_id_to_list( uint32_t **list, uint32_t *list_size, const uint32_t id )
{
    uint32_t *_temp;

    (*list_size)++;
    _temp = *list;
    
    /* Not accounting this realloc and free for memory serviceability because of infra limitation*/
    if ( (*list = realloc(_temp, sizeof(**list)*(*list_size))) == NULL )
    {
        free(_temp);
        FatalError("Failed realloc!");
    }

    (*list)[(*list_size)-1] = id;
}

bool get_ids_from_type(const FileConfig* file_config, const char *type, uint32_t **ids,
        uint32_t *count)
{
    bool status = false;
    int i;

    if ( !file_config )
        return NULL;

    /* Search for the matching rules */
    for ( i = 0; i <= FILE_ID_MAX; i++ ) {
        const RuleInfo * rule = file_config->FileRules[i];

        if ( !rule )
            continue;

        if ( strcmp(rule->type, type) )
            continue;

        __add_id_to_list( ids, count, rule->id );
        status = true;
    }

    return status;
}

bool get_ids_from_type_version(const FileConfig* file_config, const char *type,
        const char *version, uint32_t **ids, uint32_t *count)
{
    bool status = false;
    int i;

    if ( !file_config )
        return NULL;

    /* Search for the matching rules */
    for ( i = 0; i <= FILE_ID_MAX; i++ )
    {
        const RuleInfo *rule = file_config->FileRules[i];

        if ( !rule || !rule->version )
            continue;

        if ( strcmp(rule->type, type) )
            continue;

        if ( strcmp(rule->version, version) )
            continue;

        __add_id_to_list( ids, count, rule->id );
        status = true;
    }

    return status;
}

bool get_ids_from_group(const FileConfig* file_config, const char *group, uint32_t **ids,
        uint32_t *count)
{
    bool status = false;
    int i;

    if ( !file_config )
        return NULL;

    /* Search for the matching rules */
    for ( i = 0; i <= FILE_ID_MAX; i++ )
    {
        const RuleInfo *rule = file_config->FileRules[i];
        const char *_group;
        int j = 0;

        if ( !rule || !rule->groups )
            continue;

        /* Check if this rule belongs to the caller provided group */
        while( (_group = StringVector_Get(rule->groups, j++)) )
        {
            if ( _group && !strcmp(_group, group) )
                break;
        }

        if ( !_group )
            continue;

        __add_id_to_list( ids, count, rule->id );
        status = true;
    }

    return status;
}

/*The main function for parsing rule option*/
void file_rule_parse(char *args, FileConfig* file_config)
{
    char **toks;
    int num_toks;
    int i;
    char configured[sizeof(file_options) / sizeof(FileOptFunc)];
    RuleInfo *rule;

    if (!file_config)
    {
        return;
    }
    rule = SnortPreprocAlloc(1, sizeof (*rule), PP_FILE, PP_MEM_CATEGORY_SESSION);
    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Loading file configuration: %s\n",
            args););

    toks = mSplit(args, ";", 0, &num_toks, 0);  /* get rule option pairs */


    /* Used to determine if a rule option has already been configured
     * in the rule.  Some can only be configured once */
    memset(configured, 0, sizeof(configured));

    for (i = 0; i < num_toks; i++)
    {
        char **opts;
        int num_opts;
        char *option_args = NULL;

        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"   option: %s\n", toks[i]););

        /* break out the option name from its data */
        opts = mSplit(toks[i], ":", 2, &num_opts, '\\');

        DEBUG_WRAP(DebugMessage(DEBUG_FILE,"   option name: %s\n", opts[0]););

        if (num_opts == 2)
        {
            option_args = opts[1];
            DEBUG_WRAP(DebugMessage(DEBUG_FILE,"   option args: %s\n",
                    option_args););
        }
        parse_options(opts[0], option_args, configured, rule);
        mSplitFree(&opts, num_opts);

    }

    if (file_config->FileRules[rule->id])
    {
        ParseError("File type: duplicated rule id %d defined!", rule->id);
    }
    file_config->FileRules[rule->id] = rule;

    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Rule parsed: %d\n", file_rule_print(rule)););
    file_identifers_update(rule,file_config);
    DEBUG_WRAP(DebugMessage(DEBUG_FILE,"Total memory used for identifiers: "
            "%d\n", file_identifiers_usage()););
    mSplitFree(&toks, num_toks);
}

RuleInfo *file_rule_get(FileConfig* file_config, uint32_t id)
{
    if (file_config)
    {
        return (file_config->FileRules[id]);
    }

    return NULL;
}

static void _free_file_magic (MagicData  *magics)
{
    if (!magics)
        return;
    _free_file_magic(magics->next);
    SnortPreprocFree(magics->content, sizeof(magics->content_len), PP_FILE, PP_MEM_CATEGORY_SESSION);
    SnortPreprocFree(magics , sizeof(MagicData), PP_FILE, PP_MEM_CATEGORY_SESSION);
}

static void _free_file_rule(RuleInfo *rule)
{
    if ( !rule )
        return;
    /* Not changing the free here because memory is allocated using strdup */
    if ( rule->category )
        free(rule->category);

    if ( rule->message )
        free(rule->message);

    if ( rule->type )
        free(rule->type);

    if ( rule->version )
        free(rule->version);

    if ( rule->groups )
            StringVector_Delete(rule->groups);

    _free_file_magic(rule->magics);
    SnortPreprocFree(rule, sizeof(RuleInfo), PP_FILE, PP_MEM_CATEGORY_SESSION);
}

void file_rule_free(FileConfig* file_config)
{
    int id;

    if (!file_config)
        return;

    for (id = 0; id < FILE_ID_MAX + 1; id++)
    {
        _free_file_rule (file_config->FileRules[id]);
        file_config->FileRules[id] = NULL;
    }
}


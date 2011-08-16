/* $Id$ */
/*
** Copyright (C) 2003 Brian Caswell <bmc@snort.org>
** Copyright (C) 2003 Michael J. Pomraning <mjp@securepipe.com>
** Copyright (C) 2003-2011 Sourcefire, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bounds.h"
#include "rules.h"
#include "treenodes.h"
#include "debug.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "plugin_enum.h"
#include "util.h"
#include "mstring.h"
#include "sfhashfcn.h"
#include <sys/types.h>

#ifdef WIN32
#define PCRE_DEFINITION
#endif

#include "sp_pcre.h"

#include <pcre.h>

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
PreprocStats pcrePerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"
#include "detection_util.h"

/* 
 * we need to specify the vector length for our pcre_exec call.  we only care 
 * about the first vector, which if the match is successful will include the
 * offset to the end of the full pattern match.  If we decide to store other
 * matches, make *SURE* that this is a multiple of 3 as pcre requires it.
 */
#define SNORT_PCRE_OVECTOR_SIZE 3

void SnortPcreInit(char *, OptTreeNode *, int);
void SnortPcreParse(char *, PcreData *, OptTreeNode *);
void SnortPcreDump(PcreData *);
int SnortPcre(void *option_data, Packet *p);

void PcreFree(void *d)
{
    PcreData *data = (PcreData *)d;

    free(data->expression);
    free(data->re);
    free(data->pe);
    free(data);
}

uint32_t PcreHash(void *d)
{
    int i,j,k,l,expression_len;
    uint32_t a,b,c,tmp;
    PcreData *data = (PcreData *)d;

    expression_len = strlen(data->expression);
    a = b = c = 0;

    for (i=0,j=0;i<expression_len;i+=4)
    {
        tmp = 0;
        k = expression_len - i;
        if (k > 4)
            k=4;

        for (l=0;l<k;l++)
        {
            tmp |= *(data->expression + i + l) << l*8;
        }

        switch (j)
        {
            case 0:
                a += tmp;
                break;
            case 1:
                b += tmp;
                break;
            case 2:
                c += tmp;
                break;
        }
        j++;

        if (j == 3)
        {
            mix(a,b,c);
            j=0;
        }
    }

    if (j != 0)
    {
        mix(a,b,c);
    }

    a += RULE_OPTION_TYPE_PCRE;
    b += data->options;

    final(a,b,c);

    return c;
}

int PcreCompare(void *l, void *r)
{
    PcreData *left = (PcreData *)l;
    PcreData *right = (PcreData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (( strcmp(left->expression, right->expression) == 0) &&
        ( left->options == right->options))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

void PcreDuplicatePcreData(void *src, PcreData *pcre_dup)
{
    PcreData *pcre_src = (PcreData *)src;

    pcre_dup->expression = pcre_src->expression;
    pcre_dup->options = pcre_src->options;
    pcre_dup->search_offset = 0;
    pcre_dup->pe = pcre_src->pe;
    pcre_dup->re = pcre_src->re;
}

int PcreAdjustRelativeOffsets(PcreData *pcre, uint32_t search_offset)
{
    if ((pcre->options & (SNORT_PCRE_INVERT | SNORT_PCRE_ANCHORED)))
    {
        return 0; /* Don't search again */
    }

    if (pcre->options & ( SNORT_PCRE_URI_BUFS ))
    {
        return 0;
    }

    /* What's coming in has the absolute offset */
    pcre->search_offset += search_offset;

    return 1; /* Continue searcing */
}

void SetupPcre(void)
{
    RegisterRuleOption("pcre", SnortPcreInit, NULL, OPT_TYPE_DETECTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("pcre", &pcrePerfStats, 3, &ruleOTNEvalPerfStats);
#endif
}

void SnortPcreInit(char *data, OptTreeNode *otn, int protocol)
{
    PcreData *pcre_data;
    OptFpList *fpl;
    void *pcre_dup;

    /* 
     * allocate the data structure for pcre
     */
    pcre_data = (PcreData *) SnortAlloc(sizeof(PcreData));

    SnortPcreParse(data, pcre_data, otn);

    otn->pcre_flag = 1;

    fpl = AddOptFuncToList(SnortPcre, otn);
    fpl->type = RULE_OPTION_TYPE_PCRE;

    if (add_detection_option(RULE_OPTION_TYPE_PCRE, (void *)pcre_data, &pcre_dup) == DETECTION_OPTION_EQUAL)
    {
#ifdef DEBUG_RULE_OPTION_TREE
        LogMessage("Duplicate PCRE:\n%d %s\n%d %s\n\n",
            pcre_data->options, pcre_data->expression,
            ((PcreData *)pcre_dup)->options,
            ((PcreData *)pcre_dup)->expression);
#endif

        if (pcre_data->expression)
            free(pcre_data->expression);
        if (pcre_data->pe)
            free(pcre_data->pe);
        if (pcre_data->re)
            free(pcre_data->re);

        free(pcre_data);
        pcre_data = pcre_dup;
    }

    /*
     * attach it to the context node so that we can call each instance
     * individually
     */
    fpl->context = (void *) pcre_data;

    if (pcre_data->options & SNORT_PCRE_RELATIVE)
        fpl->isRelative = 1;

    if (otn->ds_list[PLUGIN_PCRE] == NULL)
        otn->ds_list[PLUGIN_PCRE] = (void *)pcre_data;

    return;
}

static INLINE void ValidatePcreHttpContentModifiers(PcreData *pcre_data)
{
    if( pcre_data->options & SNORT_PCRE_RELATIVE )
        FatalError("%s(%d): PCRE unsupported configuration : both relative & uri options specified\n", 
                file_name, file_line);

    if( pcre_data->options & SNORT_PCRE_RAWBYTES )
        FatalError("%s(%d): PCRE unsupported configuration : both rawbytes & uri options specified\n",
                file_name, file_line);

    if( (pcre_data->options & SNORT_PCRE_HTTP_URI) &&
            (pcre_data->options & SNORT_PCRE_HTTP_RAW_URI))
        FatalError("%s(%d): PCRE unsupported configuration : Cannot use http uri and raw uri modifiers for "
                "the same content\n", file_name, file_line);

    if( (pcre_data->options & SNORT_PCRE_HTTP_HEADER) &&
            (pcre_data->options & SNORT_PCRE_HTTP_RAW_HEADER))
        FatalError("%s(%d): PCRE unsupported configuration : Cannot use http header and raw header modifiers for "
                "the same content\n", file_name, file_line);

    if( (pcre_data->options & SNORT_PCRE_HTTP_COOKIE) &&
            (pcre_data->options & SNORT_PCRE_HTTP_RAW_COOKIE))
        FatalError("%s(%d): PCRE unsupported configuration : Cannot use http cookie and raw cookie modifiers for "
                "the same content\n", file_name, file_line);
}

void SnortPcreParse(char *data, PcreData *pcre_data, OptTreeNode *otn)
{
    const char *error;
    char *re, *free_me;
    char *opts;
    char delimit = '/';
    int erroffset;
    int compile_flags = 0;

    if(data == NULL) 
    {
        FatalError("%s (%d): pcre requires a regular expression\n", 
                   file_name, file_line);
    }

    free_me = SnortStrdup(data);
    re = free_me;

    /* get rid of starting and ending whitespace */
    while (isspace((int)re[strlen(re)-1])) re[strlen(re)-1] = '\0';
    while (isspace((int)*re)) re++;

    if(*re == '!') { 
        pcre_data->options |= SNORT_PCRE_INVERT;
        re++;
        while(isspace((int)*re)) re++;
    }

    /* now we wrap the RE in double quotes.  stupid snort parser.... */
    if(*re != '"') {
        printf("It isn't \"\n");
        goto syntax;
    }
    re++;

    if(re[strlen(re)-1] != '"')
    {
        printf("It isn't \"\n");
        goto syntax;
    }
    
    /* remove the last quote from the string */
    re[strlen(re) - 1] = '\0';
    
    /* 'm//' or just '//' */
        
    if(*re == 'm')
    {
        re++;
        if(! *re) goto syntax;
        
        /* Space as a ending delimiter?  Uh, no. */
        if(isspace((int)*re)) goto syntax;
        /* using R would be bad, as it triggers RE */
        if(*re == 'R') goto syntax;   

        delimit = *re;
    } 
    else if(*re != delimit)
        goto syntax;

    pcre_data->expression = SnortStrdup(re);

    /* find ending delimiter, trim delimit chars */
    opts = strrchr(re, delimit);
    if(!((opts - re) > 1)) /* empty regex(m||) or missing delim not OK */
        goto syntax;

    re++;
    *opts++ = '\0';

    /* process any /regex/ismxR options */
    while(*opts != '\0') {
        switch(*opts) {
        case 'i':  compile_flags |= PCRE_CASELESS;            break;
        case 's':  compile_flags |= PCRE_DOTALL;              break;
        case 'm':  compile_flags |= PCRE_MULTILINE;           break;
        case 'x':  compile_flags |= PCRE_EXTENDED;            break;
            
            /* 
             * these are pcre specific... don't work with perl
             */ 
        case 'A':  compile_flags |= PCRE_ANCHORED;            break;
        case 'E':  compile_flags |= PCRE_DOLLAR_ENDONLY;      break;
        case 'G':  compile_flags |= PCRE_UNGREEDY;            break;

            /*
             * these are snort specific don't work with pcre or perl
             */
        case 'R':  pcre_data->options |= SNORT_PCRE_RELATIVE; break;
        case 'U':  pcre_data->options |= SNORT_PCRE_HTTP_URI; break;
        case 'B':  pcre_data->options |= SNORT_PCRE_RAWBYTES; break;
        case 'P':  pcre_data->options |= SNORT_PCRE_HTTP_BODY;  break;
        case 'O':  pcre_data->options |= SNORT_OVERRIDE_MATCH_LIMIT; break;
        case 'H':  pcre_data->options |= SNORT_PCRE_HTTP_HEADER;  break;
        case 'M':  pcre_data->options |= SNORT_PCRE_HTTP_METHOD;  break;
        case 'C':  pcre_data->options |= SNORT_PCRE_HTTP_COOKIE;  break;
        case 'I':  pcre_data->options |= SNORT_PCRE_HTTP_RAW_URI; break;
        case 'D':  pcre_data->options |= SNORT_PCRE_HTTP_RAW_HEADER; break;
        case 'K':  pcre_data->options |= SNORT_PCRE_HTTP_RAW_COOKIE; break;
        case 'S':  pcre_data->options |= SNORT_PCRE_HTTP_STAT_CODE; break;
        case 'Y':  pcre_data->options |= SNORT_PCRE_HTTP_STAT_MSG; break;


        default:
            FatalError("%s (%d): unknown/extra pcre option encountered\n", file_name, file_line);
        }
        opts++;
    }

    if(pcre_data->options & (SNORT_PCRE_URI_BUFS))
        ValidatePcreHttpContentModifiers(pcre_data);

    /* now compile the re */
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "pcre: compiling %s\n", re););
    pcre_data->re = pcre_compile(re, compile_flags, &error, &erroffset, NULL);

    if(pcre_data->re == NULL) 
    {
        FatalError("%s(%d) : pcre compile of \"%s\" failed at offset "
                   "%d : %s\n", file_name, file_line, re, erroffset, error);
    }


    /* now study it... */
    pcre_data->pe = pcre_study(pcre_data->re, 0, &error);

    if (pcre_data->pe)
    {
        if ((ScPcreMatchLimit() != -1) && !(pcre_data->options & SNORT_OVERRIDE_MATCH_LIMIT))
        {
            if (pcre_data->pe->flags & PCRE_EXTRA_MATCH_LIMIT)
            {
                pcre_data->pe->match_limit = ScPcreMatchLimit();
            }
            else
            {
                pcre_data->pe->flags |= PCRE_EXTRA_MATCH_LIMIT;
                pcre_data->pe->match_limit = ScPcreMatchLimit();
            }
        }

#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
        if ((ScPcreMatchLimitRecursion() != -1) && !(pcre_data->options & SNORT_OVERRIDE_MATCH_LIMIT))
        {
            if (pcre_data->pe->flags & PCRE_EXTRA_MATCH_LIMIT_RECURSION)
            {
                pcre_data->pe->match_limit_recursion = ScPcreMatchLimitRecursion();
            }
            else
            {
                pcre_data->pe->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
                pcre_data->pe->match_limit_recursion = ScPcreMatchLimitRecursion();
            }
        }
#endif
    }
    else
    {
        if (!(pcre_data->options & SNORT_OVERRIDE_MATCH_LIMIT) && 
             ((ScPcreMatchLimit() != -1) || (ScPcreMatchLimitRecursion() != -1)))
        {
            pcre_data->pe = (pcre_extra *)SnortAlloc(sizeof(pcre_extra));
            if (ScPcreMatchLimit() != -1)
            {
                pcre_data->pe->flags |= PCRE_EXTRA_MATCH_LIMIT;
                pcre_data->pe->match_limit = ScPcreMatchLimit();
            }
            
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
            if (ScPcreMatchLimitRecursion() != -1)
            {
                pcre_data->pe->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
                pcre_data->pe->match_limit_recursion = ScPcreMatchLimitRecursion();
            }
#endif
        }
    }

    if(error != NULL) 
    {
        FatalError("%s(%d) : pcre study failed : %s\n", file_name, 
                   file_line, error);
    }

    PcreCheckAnchored(pcre_data);

    free(free_me);

    return;

 syntax:
    if(free_me) free(free_me);

    FatalError("%s Line %d => unable to parse pcre regex %s\n", 
               file_name, file_line, data);

}

void PcreCheckAnchored(PcreData *pcre_data)
{
    int rc;
    unsigned long int options = 0;

    if ((pcre_data == NULL) || (pcre_data->re == NULL) || (pcre_data->pe == NULL))
        return;

    rc = pcre_fullinfo(pcre_data->re, pcre_data->pe, PCRE_INFO_OPTIONS, (void *)&options);
    switch (rc)
    {
        /* pcre_fullinfo fails for the following:
         * PCRE_ERROR_NULL - the argument code was NULL
         *                   the argument where was NULL
         * PCRE_ERROR_BADMAGIC - the "magic number" was not found
         * PCRE_ERROR_BADOPTION - the value of what was invalid
         * so a failure here means we passed in bad values and we should
         * probably fatal error */

        case 0:
            /* This is the success code */
            break;

        case PCRE_ERROR_NULL:
            FatalError("%s(%d) pcre_fullinfo: code and/or where were NULL.\n",
                       __FILE__, __LINE__);

        case PCRE_ERROR_BADMAGIC:
            FatalError("%s(%d) pcre_fullinfo: compiled code didn't have "
                       "correct magic.\n", __FILE__, __LINE__);

        case PCRE_ERROR_BADOPTION:
            FatalError("%s(%d) pcre_fullinfo: option type is invalid.\n",
                       __FILE__, __LINE__);

        default:
            FatalError("%s(%d) pcre_fullinfo: Unknown error code.\n",
                       __FILE__, __LINE__);
    }

    if ((options & PCRE_ANCHORED) && !(options & PCRE_MULTILINE))
    {
        /* This means that this pcre rule option shouldn't be reevaluted
         * even if any of it's relative children should fail to match.
         * It is anchored to the cursor set by the previous cursor setting
         * rule option */
        pcre_data->options |= SNORT_PCRE_ANCHORED;
    }
}

/** 
 * Perform a search of the PCRE data.
 * 
 * @param pcre_data structure that options and patterns are passed in
 * @param buf buffer to search
 * @param len size of buffer
 * @param start_offset initial offset into the buffer
 * @param found_offset pointer to an integer so that we know where the search ended
 *
 * *found_offset will be set to -1 when the find is unsucessful OR the routine is inverted
 *
 * @return 1 when we find the string, 0 when we don't (unless we've been passed a flag to invert)
 */
static int pcre_search(const PcreData *pcre_data,
                       const char *buf,
                       int len,
                       int start_offset,
                       int *found_offset)
{
    int ovector[SNORT_PCRE_OVECTOR_SIZE];
    int matched;
    int result;
  
    if(pcre_data == NULL
       || buf == NULL
       || len <= 0
       || start_offset < 0
       || start_offset >= len
       || found_offset == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Returning 0 because we didn't have the required parameters!\n"););
        return 0;
    }

    *found_offset = -1;

    result = pcre_exec(pcre_data->re,            /* result of pcre_compile() */
                       pcre_data->pe,            /* result of pcre_study()   */
                       buf,                      /* the subject string */
                       len,                      /* the length of the subject string */
                       start_offset,             /* start at offset 0 in the subject */
                       0,                        /* options(handled at compile time */
                       ovector,                  /* vector for substring information */
                       SNORT_PCRE_OVECTOR_SIZE); /* number of elements in the vector */

    if(result >= 0)
    {
        matched = 1;
        /* From the PCRE man page:
         * When a match is successful, information about captured substrings is returned in pairs of integers,
         * starting at the beginning of ovector, and continuing up to two-thirds of its length at the most.
         * The first element of a pair is set to the offset of the first character in a substring, and the
         * second is set to the offset of the first character after the end of a substring. The first pair,
         * ovector[0] and ovector[1], identify the portion of the subject string matched by the entire pattern.
         * The next pair is used for the first capturing subpattern, and so on. The value returned by
         * pcre_exec() is the number of pairs that have been set. If there are no capturing subpatterns, the
         * return value from a successful match is 1, indicating that just the first pair of offsets has been set. 
         *
         * In Snort's case, the ovector size only allows for the first pair and a single int for scratch space.
         */
        *found_offset = ovector[1];
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "Setting Doe_ptr and found_offset: %p %d\n",
                                doe_ptr, found_offset););
    }
    else if(result == PCRE_ERROR_NOMATCH)
    {
        matched = 0;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, "pcre_exec error : %d \n", result););
        return 0;
    }

    /* invert sense of match */
    if(pcre_data->options & SNORT_PCRE_INVERT) 
    {
        matched = !matched;
    }

    return matched;
}

int SnortPcre(void *option_data, Packet *p)
{
    PcreData *pcre_data = (PcreData *)option_data;
    int found_offset = -1;  /* where is the ending location of the pattern */
    const uint8_t *base_ptr, *end_ptr, *start_ptr;
    int dsize;
    int length; /* length of the buffer pointed to by base_ptr  */
    int matched = 0;
    uint8_t rst_doe_flags = 1;
    DEBUG_WRAP(char *hexbuf;)

    PROFILE_VARS;
    PREPROC_PROFILE_START(pcrePerfStats);

    //short circuit this for testing pcre performance impact
    if (ScNoPcre())
    {
        PREPROC_PROFILE_END(pcrePerfStats);
        return DETECTION_OPTION_NO_MATCH;
    }
    
    /* This is the HTTP case */
    if(pcre_data->options & SNORT_PCRE_URI_BUFS)
    {
        int i;
        for (i=0; i<p->uri_count; i++)
        {
            switch (i)
            {
                case HTTP_BUFFER_URI:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_URI))
                        continue;
                    break;
                case HTTP_BUFFER_HEADER:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_HEADER))
                        continue;
                    break;
                case HTTP_BUFFER_CLIENT_BODY:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_BODY))
                        continue;
                    break;
                case HTTP_BUFFER_METHOD:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_METHOD))
                        continue;
                    break;
                case HTTP_BUFFER_COOKIE:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_COOKIE))
                        continue;
                    break;
                case HTTP_BUFFER_RAW_URI:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_RAW_URI))
                        continue;
                    break;
                case HTTP_BUFFER_RAW_HEADER:
                    if(!(pcre_data->options & SNORT_PCRE_HTTP_RAW_HEADER))
                        continue;
                    break;
                case HTTP_BUFFER_RAW_COOKIE:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_RAW_COOKIE))
                        continue;
                    break;
                case HTTP_BUFFER_STAT_CODE:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_STAT_CODE))
                        continue;
                    break;
                case HTTP_BUFFER_STAT_MSG:
                    if (!(pcre_data->options & SNORT_PCRE_HTTP_STAT_MSG))
                        continue;
                    break;
                default:
                    /* Uh, what buffer is this */
                    PREPROC_PROFILE_END(pcrePerfStats);
                    return DETECTION_OPTION_NO_MATCH;
            }

            if (!UriBufs[i].uri || UriBufs[i].length == 0)
                continue;

            matched = pcre_search(pcre_data,
                              (const char *)UriBufs[i].uri,
                              UriBufs[i].length,
                              0,
                              &found_offset);
            
            PREPROC_PROFILE_END(pcrePerfStats);
            if(matched)
            {
                /* don't touch doe_ptr on URI contents */
                return DETECTION_OPTION_MATCH;
            }
        }

        return DETECTION_OPTION_NO_MATCH;
    }
    /* end of the HTTP case */
    if( !(pcre_data->options & SNORT_PCRE_RAWBYTES))
    {
        if( IsMimeDecodeBuf(doe_ptr) )
        {
            dsize = mime_decode_size;
            start_ptr = file_data_ptr;
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "Using MIME Decode Buffer in pcre!\n"););
        }
        else if(  IsBase64DecodeBuf(doe_ptr))
        {
            dsize = base64_decode_size;
            start_ptr = base64_decode_buf;
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "Using Base64 Decode Buffer in pcre!\n"););
        }
        else if(p->packet_flags & PKT_ALT_DECODE)
        {
            dsize = DecodeBuffer.len;
            start_ptr = DecodeBuffer.data;
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                    "using alternative decode buffer in pcre!\n"););
        }
        else
        {
            if(IsLimitedDetect(p))
                dsize = p->alt_dsize;
            else
                dsize = p->dsize;
            start_ptr = p->data;
        }
    }
    else
    {
        dsize = p->dsize;
        start_ptr = p->data;
    }

    base_ptr = start_ptr;
    end_ptr = start_ptr + dsize;

    /* doe_ptr's would be set by the previous content option */
    if(pcre_data->options & SNORT_PCRE_RELATIVE && doe_ptr)
    {
        if(!inBounds(start_ptr, end_ptr, doe_ptr))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH, 
                                    "pcre bounds check failed on a relative content match\n"););
            PREPROC_PROFILE_END(pcrePerfStats);
            return DETECTION_OPTION_NO_MATCH;
        }
        
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "pcre ... checking relative offset\n"););
        base_ptr = doe_ptr;
        rst_doe_flags = 0;
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                                "pcre ... checking absolute offset\n"););
        base_ptr = start_ptr;
    }

    length = end_ptr - base_ptr;
    
    DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                            "pcre ... base: %p start: %p end: %p doe: %p length: %d\n",
                            base_ptr, start_ptr, end_ptr, doe_ptr, length););

    DEBUG_WRAP(hexbuf = hex(base_ptr, length);
               DebugMessage(DEBUG_PATTERN_MATCH, "pcre payload: %s\n", hexbuf);
               free(hexbuf);
               );

    matched = pcre_search(pcre_data, (const char *)base_ptr, length, pcre_data->search_offset, &found_offset);

    /* set the doe_ptr if we have a valid offset */
    if(found_offset > 0)
    {
        UpdateDoePtr(((uint8_t *) base_ptr + found_offset), rst_doe_flags);
    }

    if (matched)
    {
        PREPROC_PROFILE_END(pcrePerfStats);
        return DETECTION_OPTION_MATCH;
    }

    /* finally return 0 */
    PREPROC_PROFILE_END(pcrePerfStats);
    return DETECTION_OPTION_NO_MATCH;
}

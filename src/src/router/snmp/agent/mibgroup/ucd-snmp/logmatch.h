/*
 *  Template MIB group interface - logmatch.h
 *
 */
#ifndef _MIBGROUP_LOGMATCH_H
#define _MIBGROUP_LOGMATCH_H

#include "mibdefs.h"
#include <regex.h>

struct logmatchstat {
    char            filename[256];
    char            regEx[256];
    char            name[256];
    FILE           *logfile;
    long            currentFilePosition;
    unsigned long   globalMatchCounter;
    unsigned long   currentMatchCounter;
    unsigned long   matchCounter;
    regex_t         regexBuffer;
    int             myRegexError;
    int             virgin;
    int             thisIndex;
    int             frequency;
};
void            init_logmatch(void);


/*
 * config logmatch parsing routines 
 */
void            logmatch_free_config(void);
void            logmatch_parse_config(const char *, char *);
void            updateLogmatch_Scheduled(unsigned int,
                                         struct logmatchstat *);
extern FindVarMethod var_logmatch_table;



#define LOGMATCH_ERROR_MSG  "%s: size exceeds %dkb (= %dkb)"

#define LOGMATCH_INFO       0
#define LOGMATCH_INDEX      1
#define LOGMATCH_NAME       2
#define LOGMATCH_FILENAME   3
#define LOGMATCH_REGEX      4
#define LOGMATCH_GLOBALCTR  5
#define LOGMATCH_GLOBALCNT  6
#define LOGMATCH_CURRENTCTR 7
#define LOGMATCH_CURRENTCNT 8
#define LOGMATCH_COUNTER    9
#define LOGMATCH_COUNT      10
#define LOGMATCH_FREQ       11
#define LOGMATCH_ERROR      100
#define LOGMATCH_MSG        101

#endif                          /* _MIBGROUP_LOGMATCH_H */

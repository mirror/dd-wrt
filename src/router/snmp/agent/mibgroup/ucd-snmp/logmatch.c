/* Portions of this file are subject to the following copyrights.  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <net-snmp/net-snmp-config.h>

#ifdef HAVE_REGEX_H

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "struct.h"
#include "util_funcs.h"
#include "logmatch.h"
#include "util_funcs.h"

#define MAXLOGMATCH   50

struct logmatchstat logmatchTable[MAXLOGMATCH];
int             logmatchCount;

void
init_logmatch(void)
{
    struct variable2 logmatch_info[] = {
        {LOGMATCH_INFO, ASN_INTEGER, RONLY, var_logmatch_table, 0}
    };

    struct variable2 logmatch_table[] = {
        {LOGMATCH_INDEX, ASN_INTEGER, RONLY, var_logmatch_table, 1, {1}},
        {LOGMATCH_NAME, ASN_OCTET_STR, RONLY, var_logmatch_table, 1, {2}},
        {LOGMATCH_FILENAME, ASN_OCTET_STR, RONLY, var_logmatch_table, 1,
         {3}},
        {LOGMATCH_REGEX, ASN_OCTET_STR, RONLY, var_logmatch_table, 1, {4}},
        {LOGMATCH_GLOBALCTR, ASN_COUNTER, RONLY, var_logmatch_table, 1,
         {5}},
        {LOGMATCH_GLOBALCNT, ASN_INTEGER, RONLY, var_logmatch_table, 1,
         {6}},
        {LOGMATCH_CURRENTCTR, ASN_COUNTER, RONLY, var_logmatch_table, 1,
         {7}},
        {LOGMATCH_CURRENTCNT, ASN_INTEGER, RONLY, var_logmatch_table, 1,
         {8}},
        {LOGMATCH_COUNTER, ASN_COUNTER, RONLY, var_logmatch_table, 1, {9}},
        {LOGMATCH_COUNT, ASN_INTEGER, RONLY, var_logmatch_table, 1, {10}},
        {LOGMATCH_FREQ, ASN_INTEGER, RONLY, var_logmatch_table, 1, {11}},
        {LOGMATCH_ERROR, ASN_INTEGER, RONLY, var_logmatch_table, 1, {100}},
        {LOGMATCH_MSG, ASN_OCTET_STR, RONLY, var_logmatch_table, 1, {101}}
    };

    /*
     * Define the OID pointer to the top of the mib tree that we're
     * registering underneath 
     */
    oid             logmatch_info_oid[] = { NETSNMP_UCDAVIS_MIB, 16, 1 };
    oid             logmatch_variables_oid[] = { NETSNMP_UCDAVIS_MIB, 16, 2, 1 };

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("ucd-snmp/logmatch", logmatch_info, variable2,
                 logmatch_info_oid);
    REGISTER_MIB("ucd-snmp/logmatch", logmatch_table, variable2,
                 logmatch_variables_oid);

    snmpd_register_config_handler("logmatch", logmatch_parse_config,
                                  logmatch_free_config,
                                  "logmatch name path cycletime regex");

}

/***************************************************************
*                                                              *
* !!!---!!! PUBLIC !!! --- !!!                                 *
*                                                              *
* logmatch_free_config                                         *
* free memory allocated by this mib module                     *
*                                                              *
***************************************************************/

void
logmatch_free_config(void)
{
    int             i;

    /*
     * ------------------------------------ 
     * the only memory we have allocated    
     * is the memory allocated by regcomp   
     * ------------------------------------ 
     */

    for (i = 0; i < logmatchCount; i++) {

        regfree(&(logmatchTable[i].regexBuffer));
    }
    logmatchCount = 0;
}

/***************************************************************
*                                                              *
* !!!---!!! PUBLIC !!! --- !!!                                 *
*                                                              *
* logmatch_parse_config                                        *
* parse one line from snmpd.conf                               *
*                                                              *
***************************************************************/

void
logmatch_parse_config(const char *token, char *cptr)
{

    char space_name;
    char space_path;

    if (logmatchCount < MAXLOGMATCH) {
        logmatchTable[logmatchCount].frequency = 30;
        logmatchTable[logmatchCount].thisIndex = logmatchCount;


        /*
         * ------------------------------------ 
         * be careful this counter needs to be  
         * reset from persistent storage         
         * ------------------------------------ 
         */

        logmatchTable[logmatchCount].globalMatchCounter = 0;
        logmatchTable[logmatchCount].currentMatchCounter = 0;
        logmatchTable[logmatchCount].matchCounter = 0;
        logmatchTable[logmatchCount].virgin = TRUE;
        logmatchTable[logmatchCount].currentFilePosition = 0;


        /*
         * ------------------------------------ 
         * be careful: the flag 255 must fit to 
         * the size of regEx as definded in     
         * logmatch.h                           
         * ------------------------------------ 
         */

        sscanf(cptr, "%255s%c%255s%c %d %255c\n",
               logmatchTable[logmatchCount].name,
	       &space_name,
               logmatchTable[logmatchCount].filename,
	       &space_path,
               &(logmatchTable[logmatchCount].frequency),
               logmatchTable[logmatchCount].regEx);

	/*
	 * Log an error then return if any of the strings scanned in were
	 * larger then they should have been.
	 */
	if (space_name != ' ') {
		snmp_log(LOG_ERR, "logmatch_parse_config: the name scanned " \
		 "in from line %s is too large. logmatchCount = %d\n",
		 cptr, logmatchCount);
		return;
	} else if (space_path != ' ') {
		snmp_log(LOG_ERR, "logmatch_parse_config: the file name " \
		 "scanned in from line %s is too large. logmatchCount = %d\n",
		    cptr, logmatchCount);
		return;
	}

        /*
         * ------------------------------------ 
         * just to be safe "NULL" the end of    
         * the arary regEx as sscanf won't do   
         * it with the %c modifier              
         * ------------------------------------ 
         */

        logmatchTable[logmatchCount].regEx[255] = '\0';


        /*
         * ------------------------------------ 
         * now compile the regular expression   
         * ------------------------------------ 
         */

        logmatchTable[logmatchCount].myRegexError =
            regcomp(&(logmatchTable[logmatchCount].regexBuffer),
                    logmatchTable[logmatchCount].regEx,
                    REG_EXTENDED | REG_NOSUB);

        if (logmatchTable[logmatchCount].frequency > 0) {
            snmp_alarm_register(logmatchTable[logmatchCount].frequency,
                                SA_REPEAT,
                                (SNMPAlarmCallback *)
                                updateLogmatch_Scheduled,
                                &(logmatchTable[logmatchCount])
                );
        }

        logmatchCount++;
    }
}


/***************************************************************
*                                                              *
* !!!---!!! PUBLIC !!! --- !!!                                 *
*                                                              *
* updateLogmatch                                               *
* this function is called back by snmpd alarms                 *
*                                                              *
***************************************************************/


void
updateLogmatch(int iindex)
{

    regmatch_t      myMatch;
    int             matchResultCode;
    char            inbuf[1024];
    char            perfilename[1024];
    FILE           *perfile;
    unsigned long   pos, ccounter, counter;
    int             result;
    int             toobig;
    int             anyChanges = FALSE;
    struct stat     sb;

    /*
     * ------------------------------------ 
     * we can never be sure if this is the  
     * last time we are being called here,  
     * so we always update a persistent     
     * data file with our current file      
     * position                             
     * ------------------------------------ 
     */

    snprintf(perfilename, sizeof(perfilename), "%s/snmpd_logmatch_%s.pos",
	get_persistent_directory(), logmatchTable[iindex].name);

    if (logmatchTable[iindex].virgin) {

        /*
         * ------------------------------------ 
         * this is the first time we are being  
         * called; let's try to find an old     
         * file position stored in a persistent 
         * data file and restore it             
         * ------------------------------------ 
         */

        if ((perfile = fopen(perfilename, "r"))) {


            /*
             * ------------------------------------ 
             * the persistent data file exists so   
             * let's read it out                    
             * ------------------------------------ 
             */


            pos = counter = ccounter = 0;

            if (fscanf(perfile, "%lu %lu %lu", &pos, &ccounter, &counter)) {


                /*
                 * ------------------------------------ 
                 * the data could be read; now let's    
                 * try to open the  logfile to be       
                 * scanned                              
                 * ------------------------------------ 
                 */

                if ((logmatchTable[iindex].logfile =
                    fopen(logmatchTable[iindex].filename, "r"))) {


                    /*
                     * ------------------------------------ 
                     * the log file could be opened; now    
                     * let's try to set the pointer         
                     * ------------------------------------ 
                     */

                    if (!fseek
                        (logmatchTable[iindex].logfile, pos, SEEK_SET)) {


                        /*
                         * ------------------------------------ 
                         * the pointer could be set - this is   
                         * the most that we can do: if the      
                         * pointer is smaller than the file     
                         * size we must assume that the pointer 
                         * still points to where it read the    
                         * file last time; let's restore the    
                         * data                                 
                         * ------------------------------------ 
                         */

                        logmatchTable[iindex].currentFilePosition = pos;
                        logmatchTable[iindex].currentMatchCounter =
                            ccounter;
                        logmatchTable[iindex].globalMatchCounter = counter;
                    }

                    fclose(logmatchTable[iindex].logfile);
                }
            }

            fclose(perfile);
        }

        logmatchTable[iindex].virgin = FALSE;
    }


    /*
     * ------------------------------------ 
     * now the pointer and the counter are  
     * set either zero or reset to old      
     * value; now let's try to read some    
     * data                                 
     * ------------------------------------ 
     */

    if (stat(logmatchTable[iindex].filename, &sb) == 0) {

        if (logmatchTable[iindex].currentFilePosition > sb.st_size) {
            toobig = TRUE;
        } else {
            toobig = FALSE;
        }

        if ((logmatchTable[iindex].logfile =
            fopen(logmatchTable[iindex].filename, "r"))) {

            result =
                fseek(logmatchTable[iindex].logfile,
                      logmatchTable[iindex].currentFilePosition, SEEK_SET);

            if (result || toobig || (errno == EINVAL)
                || feof(logmatchTable[iindex].logfile)) {


                /*
                 * ------------------------------------ 
                 * when we are here that means we       
                 * could't set the file position maybe  
                 * the file was rotated; let's reset    
                 * the filepointer, but not the counter 
                 * ------------------------------------ 
                 */


                logmatchTable[iindex].currentFilePosition = 0;
                logmatchTable[iindex].currentMatchCounter = 0;
                fseek(logmatchTable[iindex].logfile, 0, SEEK_SET);
                anyChanges = TRUE;
            }

            while (fgets
                   (inbuf, sizeof(inbuf), logmatchTable[iindex].logfile)) {

                matchResultCode =
                    regexec(&(logmatchTable[iindex].regexBuffer),
                            inbuf, 0, &myMatch, REG_NOTEOL);

                if (matchResultCode == 0) {
                    logmatchTable[iindex].globalMatchCounter++;
                    logmatchTable[iindex].currentMatchCounter++;
                    logmatchTable[iindex].matchCounter++;
                    anyChanges = TRUE;
                }
            }

            logmatchTable[iindex].currentFilePosition =
                ftell(logmatchTable[iindex].logfile);
            fclose(logmatchTable[iindex].logfile);
        }
    }


    /*
     * ------------------------------------ 
     * at this point we can be safe that    
     * our current file position is         
     * straightened out o.k. - we never     
     * know if this is the last time we are 
     * being called so save the position    
     * in a file                            
     * ------------------------------------ 
     */

    if (anyChanges && (perfile = fopen(perfilename, "w"))) {


        /*
         * ------------------------------------ 
         * o.k. lets write out our variable     
         * ------------------------------------ 
         */

        fprintf(perfile, "%lu %lu %lu\n",
                logmatchTable[iindex].currentFilePosition,
                logmatchTable[iindex].currentMatchCounter,
                logmatchTable[iindex].globalMatchCounter);

        fclose(perfile);
    }

}


void
updateLogmatch_Scheduled(unsigned int registrationNumber,
                         struct logmatchstat *logmatchtable)
{

    updateLogmatch(logmatchtable->thisIndex);
}




/*
 * OID functions 
 */

u_char         *
var_logmatch_table(struct variable *vp,
                   oid * name,
                   size_t * length,
                   int exact,
                   size_t * var_len, WriteMethod ** write_method)
{
    static long     long_ret;
    static char     message[1024];
    int             iindex;
    struct logmatchstat *logmatch;

    if (vp->magic == LOGMATCH_INFO) {
        if (header_generic(vp, name, length, exact, var_len, write_method)
            == MATCH_FAILED)
            return (NULL);
    } else {
        if (header_simple_table
            (vp, name, length, exact, var_len, write_method,
             logmatchCount))
            return (NULL);
    }


    iindex = name[*length - 1] - 1;
    logmatch = &logmatchTable[iindex];

    if (logmatch->myRegexError == 0)
        updateLogmatch(iindex);

    switch (vp->magic) {
    case LOGMATCH_INFO:
        long_ret = MAXLOGMATCH;
        return (u_char *) & long_ret;

    case LOGMATCH_INDEX:
        long_ret = iindex + 1;
        return (u_char *) & long_ret;

    case LOGMATCH_NAME:
        *var_len = strlen(logmatch->name);
        return (u_char *) logmatch->name;

    case LOGMATCH_FILENAME:
        *var_len = strlen(logmatch->filename);
        return (u_char *) logmatch->filename;

    case LOGMATCH_REGEX:
        *var_len = strlen(logmatch->regEx);
        return (u_char *) logmatch->regEx;

    case LOGMATCH_GLOBALCTR:
    case LOGMATCH_GLOBALCNT:
        long_ret = (logmatch->globalMatchCounter);
        return (u_char *) & long_ret;

    case LOGMATCH_CURRENTCTR:
    case LOGMATCH_CURRENTCNT:
        long_ret = (logmatch->currentMatchCounter);
        return (u_char *) & long_ret;

    case LOGMATCH_COUNTER:
    case LOGMATCH_COUNT:
        long_ret = (logmatch->matchCounter);
        logmatch->matchCounter = 0;
        return (u_char *) & long_ret;

    case LOGMATCH_FREQ:
        long_ret = logmatch->frequency;
        return (u_char *) & long_ret;

    case LOGMATCH_ERROR:
        if (logmatch->frequency >= 0 && logmatch->myRegexError == 0)
            long_ret = 0;
        else
            long_ret = 1;

        return (u_char *) & long_ret;

    case LOGMATCH_MSG:

        regerror(logmatch->myRegexError, &(logmatch->regexBuffer), message,
                 sizeof(message));

        *var_len = strlen(message);
        return (u_char *) message;

    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_logmatch_table\n",
                    vp->magic));
    }

    return NULL;
}

#endif /* HAVE_REGEX */

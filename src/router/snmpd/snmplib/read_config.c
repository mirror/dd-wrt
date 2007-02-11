/*
 * read_config.c
 */
/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

/** @defgroup read_config parsing various configuration files at run time
 *  @ingroup library
 *
 * The read_config related functions are a fairly extensible  system  of
 * parsing various configuration files at the run time.
 *
 * The idea is that the calling application is able to register
 * handlers for certain tokens specified in certain types
 * of files.  The read_configs function can then be  called
 * to  look  for all the files that it has registrations for,
 * find the first word on each line, and pass  the  remainder
 * to the appropriately registered handler.
 *
 * For persistent configuration storage you will need to use the
 * read_config_read_data, read_config_store, and read_config_store_data
 * APIs in conjunction with first registering a
 * callback so when the agent shutsdown for whatever reason data is written
 * to your configuration files.  The following explains in more detail the
 * sequence to make this happen.
 *
 * This is the callback registration API, you need to call this API with
 * the appropriate parameters in order to configure persistent storage needs.
 *
 *        int snmp_register_callback(int major, int minor,
 *                                   SNMPCallback *new_callback,
 *                                   void *arg);
 *
 * You will need to set major to SNMP_CALLBACK_LIBRARY, minor to
 * SNMP_CALLBACK_STORE_DATA. arg is whatever you want.
 *
 * Your callback function's prototype is:
 * int     (SNMPCallback) (int majorID, int minorID, void *serverarg,
 *                        void *clientarg);
 *
 * The majorID, minorID and clientarg are what you passed in the callback
 * registration above.  When the callback is called you have to essentially
 * transfer all your state from memory to disk. You do this by generating
 * configuration lines into a buffer.  The lines are of the form token
 * followed by token parameters.
 * 
 * Finally storing is done using read_config_store(type, buffer);
 * type is the application name this can be obtained from:
 *
 * netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE);
 *
 * Now, reading back the data: This is done by registering a config handler
 * for your token using the register_config_handler function. Your
 * handler will be invoked and you can parse in the data using the
 * read_config_read APIs.
 *
 *  @{
 */
#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <ctype.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#include <errno.h>

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/library/read_config.h>       /* for "internal" definitions */
#include <net-snmp/utilities.h>

#include <net-snmp/library/mib.h>
#include <net-snmp/library/parse.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/callback.h>

static int      config_errors;

struct config_files *config_files = NULL;

struct config_line *
register_prenetsnmp_mib_handler(const char *type,
                                const char *token,
                                void (*parser) (const char *, char *),
                                void (*releaser) (void), const char *help)
{
    struct config_line *ltmp;
    ltmp = register_config_handler(type, token, parser, releaser, help);
    if (ltmp != NULL)
        ltmp->config_time = PREMIB_CONFIG;
    return (ltmp);
}

struct config_line *
register_app_prenetsnmp_mib_handler(const char *token,
                                    void (*parser) (const char *, char *),
                                    void (*releaser) (void),
                                    const char *help)
{
    return (register_prenetsnmp_mib_handler
            (NULL, token, parser, releaser, help));
}

/**
 * register_config_handler registers handlers for certain tokens specified in
 * certain types of files.
 *
 * Allows a module writer use/register multiple configuration files based off
 * of the type parameter.  A module writer may want to set up multiple
 * configuration files to separate out related tasks/variables or just for
 * management of where to put tokens as the module or modules get more complex
 * in regard to handling token registrations.
 *
 * @param type_param the configuration file used, e.g., if snmp.conf is the file
 *                 where the token is located use "snmp" here.
 *                 If NULL the configuration file used will be snmpd.conf.
 *
 * @param token    the token being parsed from the file.  Must be non-NULL.
 *
 * @param parser   the handler function pointer that use  the specified
 *                 token and the rest of the line to do whatever is required
 *                 Should be non-NULL in order to make use of this API.
 *
 * @param releaser if non-NULL, the function specified is called if
 *                 and when the configuration files are re-read.  This function
 *                 should free any resources allocated by the token handler
 *                 function.
 *
 * @param help     if non-NULL, used to display help information on the expected 
 *	           arguments after the token.
 *      
 * @return Pointer to a new config line entry or NULL on error.
 */
struct config_line *
register_config_handler(const char *type_param,
                        const char *token,
                        void (*parser) (const char *, char *),
                        void (*releaser) (void), const char *help)
{
    struct config_files **ctmp = &config_files;
    struct config_line **ltmp, *ltmp2;
    const char     *type = type_param;
    char           *cptr, buf[STRINGMAX];
    char           *st;

    if (type == NULL) {
        type = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				     NETSNMP_DS_LIB_APPTYPE);
    }

    /*
     * Handle multiple types (recursively)
     */
    cptr = strchr( type, ':' );
    if (cptr) {
        strncpy(buf, type, STRINGMAX);
        buf[STRINGMAX - 1] = '\0';
        ltmp2 = NULL;
        cptr = strtok_r(buf, ":", &st);
        while (cptr) {
            ltmp2 = register_config_handler(cptr, token, parser, releaser, help);
            cptr  = strtok_r(NULL, ":", &st);
        }
        return ltmp2;
    }
    
    /*
     * Find type in current list  -OR-  create a new file type.
     */
    while (*ctmp != NULL && strcmp((*ctmp)->fileHeader, type)) {
        ctmp = &((*ctmp)->next);
    }

    if (*ctmp == NULL) {
        *ctmp = (struct config_files *)
            calloc(1, sizeof(struct config_files));
        if (!*ctmp) {
            return NULL;
        }

        (*ctmp)->fileHeader = strdup(type);
    }

    /*
     * Find parser type in current list  -OR-  create a new
     * line parser entry.
     */
    ltmp = &((*ctmp)->start);

    while (*ltmp != NULL && strcmp((*ltmp)->config_token, token)) {
        ltmp = &((*ltmp)->next);
    }

    if (*ltmp == NULL) {
        *ltmp = (struct config_line *)
            calloc(1, sizeof(struct config_line));
        if (!*ltmp) {
            return NULL;
        }

        (*ltmp)->config_time = NORMAL_CONFIG;
        (*ltmp)->config_token = strdup(token);
        if (help != NULL)
            (*ltmp)->help = strdup(help);
    }

    /*
     * Add/Replace the parse/free functions for the given line type
     * in the given file type.
     */
    (*ltmp)->parse_line = parser;
    (*ltmp)->free_func = releaser;

    return (*ltmp);

}                               /* end register_config_handler() */

struct config_line *
register_app_config_handler(const char *token,
                            void (*parser) (const char *, char *),
                            void (*releaser) (void), const char *help)
{
    return (register_config_handler(NULL, token, parser, releaser, help));
}



/**
 * uregister_config_handler un-registers handlers given a specific type_param
 * and token.
 *
 * @param type_param the configuration file used where the token is located.
 *                   Used to lookup the config file entry
 * 
 * @param token      the token that is being unregistered
 *
 * @return void
 */
void
unregister_config_handler(const char *type_param, const char *token)
{
    struct config_files **ctmp = &config_files;
    struct config_line **ltmp, *ltmp2;
    const char     *type = type_param;
    char           *cptr, buf[STRINGMAX];
    char           *st;

    if (type == NULL) {
        type = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				     NETSNMP_DS_LIB_APPTYPE);
    }

    /*
     * Handle multiple types (recursively)
     */
    cptr = strchr( type, ':' );
    if (cptr) {
        strncpy(buf, type, STRINGMAX);
        buf[STRINGMAX - 1] = '\0';
        cptr = strtok_r(buf, ":", &st);
        while (cptr) {
            unregister_config_handler(cptr, token);
            cptr  = strtok_r(NULL, ":", &st);
        }
        return;
    }
    
    /*
     * find type in current list 
     */
    while (*ctmp != NULL && strcmp((*ctmp)->fileHeader, type)) {
        ctmp = &((*ctmp)->next);
    }

    if (*ctmp == NULL) {
        /*
         * Not found, return. 
         */
        return;
    }

    ltmp = &((*ctmp)->start);
    if (*ltmp == NULL) {
        /*
         * Not found, return. 
         */
        return;
    }
    if (strcmp((*ltmp)->config_token, token) == 0) {
        /*
         * found it at the top of the list 
         */
        ltmp2 = (*ltmp)->next;
        SNMP_FREE((*ltmp)->config_token);
        SNMP_FREE((*ltmp)->help);
        SNMP_FREE(*ltmp);
        (*ctmp)->start = ltmp2;
        return;
    }
    while ((*ltmp)->next != NULL
           && strcmp((*ltmp)->next->config_token, token)) {
        ltmp = &((*ltmp)->next);
    }
    if ((*ltmp)->next != NULL) {
        SNMP_FREE((*ltmp)->next->config_token);
        SNMP_FREE((*ltmp)->next->help);
        ltmp2 = (*ltmp)->next->next;
        SNMP_FREE((*ltmp)->next);
        (*ltmp)->next = ltmp2;
    }
}

void
unregister_app_config_handler(const char *token)
{
    unregister_config_handler(NULL, token);
}

void
unregister_all_config_handlers()
{
    struct config_files *ctmp, *save;
    struct config_line *ltmp;

    free_config();

    /*
     * Keep using config_files until there are no more! 
     */
    for (ctmp = config_files; ctmp;) {
        for (ltmp = ctmp->start; ltmp; ltmp = ctmp->start) {
            unregister_config_handler(ctmp->fileHeader,
                                      ltmp->config_token);
        }
        SNMP_FREE(ctmp->fileHeader);
        save = ctmp->next;
        SNMP_FREE(ctmp);
        ctmp = save;
        config_files = save;
    }
}

#ifdef TESTING
void
print_config_handlers(void)
{
    struct config_files *ctmp = config_files;
    struct config_line *ltmp;

    for (; ctmp != NULL; ctmp = ctmp->next) {
        DEBUGMSGTL(("read_config", "read_conf: %s\n", ctmp->fileHeader));
        for (ltmp = ctmp->start; ltmp != NULL; ltmp = ltmp->next)
            DEBUGMSGTL(("read_config", "                   %s\n",
                        ltmp->config_token));
    }
}
#endif

int             linecount;
const char     *curfilename;

struct config_line *
read_config_get_handlers(const char *type)
{
    struct config_files *ctmp = config_files;
    for (; ctmp != NULL && strcmp(ctmp->fileHeader, type);
         ctmp = ctmp->next);
    if (ctmp)
        return ctmp->start;
    return NULL;
}

void
read_config_with_type_when(const char *filename, const char *type, int when)
{
    struct config_line *ctmp = read_config_get_handlers(type);
    if (ctmp)
        read_config(filename, ctmp, when);
    else
        DEBUGMSGTL(("read_config",
                    "read_config: I have no registrations for type:%s,file:%s\n",
                    type, filename));
}

void
read_config_with_type(const char *filename, const char *type)
{
    read_config_with_type_when(filename, type, EITHER_CONFIG);
}


struct config_line *
read_config_find_handler(struct config_line *line_handlers,
                         const char *token)
{
    struct config_line *lptr;

    for (lptr = line_handlers; lptr != NULL; lptr = lptr->next) {
        if (!strcasecmp(token, lptr->config_token)) {
            return lptr;
        }
    }
    return NULL;
}


/*
 * searches a config_line linked list for a match 
 */
int
run_config_handler(struct config_line *lptr,
                   const char *token, char *cptr, int when)
{
    char            tmpbuf[STRINGMAX];
    char           *cp;
    lptr = read_config_find_handler(lptr, token);
    if (lptr != NULL) {
        if (when == EITHER_CONFIG || lptr->config_time == when) {
            DEBUGMSGTL(("read_config",
                        "Found a parser.  Calling it: %s / %s\n", token,
                        cptr));
            /*
             * Stomp on any trailing whitespace
             */
            cp = &(cptr[strlen(cptr)-1]);
            while (isspace(*cp)) {
                *(cp--) = '\0';
            }
            (*(lptr->parse_line)) (token, cptr);
        }
    } else if (when != PREMIB_CONFIG && 
	       !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, 
				       NETSNMP_DS_LIB_NO_TOKEN_WARNINGS)) {
        snprintf(tmpbuf, sizeof(tmpbuf), "Unknown token: %s.", token);
        tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
        config_pwarn(tmpbuf);
        return SNMPERR_GENERR;
    }
    return SNMPERR_SUCCESS;
}

/*
 * takens an arbitrary string and tries to intepret it based on the
 * known configuration handlers for all registered types.  May produce
 * inconsistent results when multiple tokens of the same name are
 * registered under different file types. 
 */

/*
 * we allow = delimeters here 
 */
#define SNMP_CONFIG_DELIMETERS " \t="

int
snmp_config_when(char *line, int when)
{
    char           *cptr, buf[STRINGMAX], tmpbuf[STRINGMAX];
    struct config_line *lptr = NULL;
    struct config_files *ctmp = config_files;
    char           *st;

    if (line == NULL) {
        config_perror("snmp_config() called with a null string.");
        return SNMPERR_GENERR;
    }

    strncpy(buf, line, STRINGMAX);
    buf[STRINGMAX - 1] = '\0';
    cptr = strtok_r(buf, SNMP_CONFIG_DELIMETERS, &st);
    if (cptr && cptr[0] == '[') {
        if (cptr[strlen(cptr) - 1] != ']') {
            snprintf(tmpbuf, sizeof(tmpbuf),
                    "no matching ']' for type %s.",
                    cptr + 1);
            tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
            config_perror(tmpbuf);
            return SNMPERR_GENERR;
        }
        cptr[strlen(cptr) - 1] = '\0';
        lptr = read_config_get_handlers(cptr + 1);
        if (lptr == NULL) {
            snprintf(tmpbuf,  sizeof(tmpbuf),
                     "No handlers regestered for type %s.",
                    cptr + 1);
            tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
            config_perror(tmpbuf);
            return SNMPERR_GENERR;
        }
        cptr = strtok_r(NULL, SNMP_CONFIG_DELIMETERS, &st);
        lptr = read_config_find_handler(lptr, cptr);
    } else {
        /*
         * we have to find a token 
         */
        for (; ctmp != NULL && lptr == NULL; ctmp = ctmp->next)
            lptr = read_config_find_handler(ctmp->start, cptr);
    }
    if (lptr == NULL && netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, 
					  NETSNMP_DS_LIB_NO_TOKEN_WARNINGS)) {
        snprintf(tmpbuf, sizeof(tmpbuf), "Unknown token: %s.", cptr);
        tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
        config_pwarn(tmpbuf);
        return SNMPERR_GENERR;
    }

    /*
     * use the original string instead since strtok_r messed up the original 
     */
    line = skip_white(line + (cptr - buf) + strlen(cptr) + 1);

    return (run_config_handler(lptr, cptr, line, when));
}

int
netsnmp_config(char *line)
{
    int             ret = SNMP_ERR_NOERROR;
    DEBUGMSGTL(("snmp_config", "remembering line \"%s\"\n", line));
    netsnmp_config_remember(line);      /* always remember it so it's read
                                         * processed after a free_config()
                                         * call */
    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, 
			       NETSNMP_DS_LIB_HAVE_READ_CONFIG)) {
        DEBUGMSGTL(("snmp_config", "  ... processing it now\n"));
        ret = snmp_config_when(line, NORMAL_CONFIG);
    }
    return ret;
}

void
netsnmp_config_remember_in_list(char *line,
                                struct read_config_memory **mem)
{
    if (mem == NULL)
        return;

    while (*mem != NULL)
        mem = &((*mem)->next);

    *mem = SNMP_MALLOC_STRUCT(read_config_memory);
    if (line)
        (*mem)->line = strdup(line);
}

void
netsnmp_config_remember_free_list(struct read_config_memory **mem)
{
    struct read_config_memory *tmpmem;
    while (*mem) {
        SNMP_FREE((*mem)->line);
        tmpmem = (*mem)->next;
        SNMP_FREE(*mem);
        *mem = tmpmem;
    }
}

void
netsnmp_config_process_memory_list(struct read_config_memory **memp,
                                   int when, int clear)
{

    struct read_config_memory *mem;

    if (!memp)
        return;

    mem = *memp;

    while (mem) {
        DEBUGMSGTL(("read_config", "processing memory: %s\n", mem->line));
        snmp_config_when(mem->line, when);
        mem = mem->next;
    }

    if (clear)
        netsnmp_config_remember_free_list(memp);
}

/*
 * default storage location implementation 
 */
static struct read_config_memory *memorylist = NULL;

void
netsnmp_config_remember(char *line)
{
    netsnmp_config_remember_in_list(line, &memorylist);
}

void
netsnmp_config_process_memories(void)
{
    netsnmp_config_process_memory_list(&memorylist, EITHER_CONFIG, 1);
}

void
netsnmp_config_process_memories_when(int when, int clear)
{
    netsnmp_config_process_memory_list(&memorylist, when, clear);
}

/*******************************************************************-o-******
 * read_config
 *
 * Parameters:
 *	*filename
 *	*line_handler
 *	 when
 *
 * Read <filename> and process each line in accordance with the list of
 * <line_handler> functions.
 *
 *
 * For each line in <filename>, search the list of <line_handler>'s 
 * for an entry that matches the first token on the line.  This comparison is
 * case insensitive.
 *
 * For each match, check that <when> is the designated time for the
 * <line_handler> function to be executed before processing the line.
 */
void
read_config(const char *filename,
            struct config_line *line_handler, int when)
{

    FILE           *ifile;
    char            line[STRINGMAX], token[STRINGMAX], tmpbuf[STRINGMAX];
    char           *cptr;
    int             i;
    struct config_line *lptr;

    linecount = 0;
    curfilename = filename;

    if ((ifile = fopen(filename, "r")) == NULL) {
#ifdef ENOENT
        if (errno == ENOENT) {
            DEBUGMSGTL(("read_config", "%s: %s\n", filename,
                        strerror(errno)));
        } else
#endif                          /* ENOENT */
#ifdef EACCES
        if (errno == EACCES) {
            DEBUGMSGTL(("read_config", "%s: %s\n", filename,
                        strerror(errno)));
        } else
#endif                          /* EACCES */
#if defined(ENOENT) || defined(EACCES)
        {
            snmp_log_perror(filename);
        }
#else                           /* defined(ENOENT) || defined(EACCES) */
            snmp_log_perror(filename);
#endif                          /* ENOENT */
        return;
    } else {
        DEBUGMSGTL(("read_config", "Reading configuration %s\n",
                    filename));
    }

    while (fgets(line, sizeof(line), ifile) != NULL) {
        lptr = line_handler;
        linecount++;
        cptr = line;
        i = strlen(line) - 1;
        if (line[i] == '\n')
            line[i] = 0;
        /*
         * check blank line or # comment 
         */
        if ((cptr = skip_white(cptr))) {
            cptr = copy_nword(cptr, token, sizeof(token));
            if (token[0] == '[') {
                if (token[strlen(token) - 1] != ']') {
                    snprintf(tmpbuf, sizeof(tmpbuf),
                            "no matching ']' for type %s.",
                            &token[1]);
                    tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
                    config_perror(tmpbuf);
                    continue;
                }
                token[strlen(token) - 1] = '\0';
                lptr = read_config_get_handlers(&token[1]);
                if (lptr == NULL) {
                    snprintf(tmpbuf, sizeof(tmpbuf),
                            "No handlers regestered for type %s.",
                            &token[1]);
                    tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
                    config_perror(tmpbuf);
                    continue;
                }
                DEBUGMSGTL(("read_config",
                            "Switching to new context: %s%s\n",
                            ((cptr) ? "(this line only) " : ""),
                            &token[1]));
                if (cptr == NULL) {
                    /*
                     * change context permanently 
                     */
                    line_handler = lptr;
                    continue;
                } else {
                    /*
                     * the rest of this line only applies. 
                     */
                    cptr = copy_nword(cptr, token, sizeof(token));
                }
            } else {
                lptr = line_handler;
            }
            if (cptr == NULL) {
                snprintf(tmpbuf, sizeof(tmpbuf),
                        "Blank line following %s token.", token);
                tmpbuf[ sizeof(tmpbuf)-1 ] = 0;
                config_perror(tmpbuf);
            } else {
                DEBUGMSGTL(("read_config", "%s:%d examining: %s\n",
                            filename, linecount, line));
                run_config_handler(lptr, token, cptr, when);
            }
        }
    }
    fclose(ifile);
    return;

}                               /* end read_config() */



void
free_config(void)
{
    struct config_files *ctmp = config_files;
    struct config_line *ltmp;

    for (; ctmp != NULL; ctmp = ctmp->next)
        for (ltmp = ctmp->start; ltmp != NULL; ltmp = ltmp->next)
            if (ltmp->free_func)
                (*(ltmp->free_func)) ();
}

void
read_configs_optional(const char *optional_config, int when)
{
    char *newp, *cp, *st = NULL;
    char *type = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				       NETSNMP_DS_LIB_APPTYPE);

    if ((NULL == optional_config) || (NULL == type))
        return;

    DEBUGMSGTL(("read_configs_optional",
                "reading optional configuration tokens for %s\n", type));
    
    newp = strdup(optional_config);      /* strtok_r messes it up */
    cp = strtok_r(newp, ",", &st);
    while (cp) {
        struct stat     statbuf;
        if (stat(cp, &statbuf)) {
            DEBUGMSGTL(("read_config",
                        "Optional File \"%s\" does not exist.\n", cp));
            snmp_log_perror(cp);
        } else {
            DEBUGMSGTL(("read_config",
                        "Reading optional config file: \"%s\"\n", cp));
            read_config_with_type_when(cp, type, when);
        }
        cp = strtok_r(NULL, ",", &st);
    }
    free(newp);
    
}

void
read_configs(void)
{
    char *optional_config = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					       NETSNMP_DS_LIB_OPTIONALCONFIG);

    DEBUGMSGTL(("read_config", "reading normal configuration tokens\n"));

    if ((NULL != optional_config) && (*optional_config == '-')) {
        read_configs_optional(++optional_config, NORMAL_CONFIG);
        optional_config = NULL; /* clear, so we don't read them twice */
    }

    read_config_files(NORMAL_CONFIG);

    /*
     * do this even when the normal above wasn't done 
     */
    if (NULL != optional_config)
        read_configs_optional(optional_config, NORMAL_CONFIG);

    netsnmp_config_process_memories_when(NORMAL_CONFIG, 1);

    netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
			   NETSNMP_DS_LIB_HAVE_READ_CONFIG, 1);
    snmp_call_callbacks(SNMP_CALLBACK_LIBRARY,
                        SNMP_CALLBACK_POST_READ_CONFIG, NULL);
}

void
read_premib_configs(void)
{
    char *optional_config = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					       NETSNMP_DS_LIB_OPTIONALCONFIG);

    DEBUGMSGTL(("read_config", "reading premib configuration tokens\n"));

    if ((NULL != optional_config) && (*optional_config == '-')) {
        read_configs_optional(++optional_config, PREMIB_CONFIG);
        optional_config = NULL; /* clear, so we don't read them twice */
    }

    read_config_files(PREMIB_CONFIG);

    if (NULL != optional_config)
        read_configs_optional(optional_config, PREMIB_CONFIG);

    netsnmp_config_process_memories_when(PREMIB_CONFIG, 0);

    netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
			   NETSNMP_DS_LIB_HAVE_READ_PREMIB_CONFIG, 1);
    snmp_call_callbacks(SNMP_CALLBACK_LIBRARY,
                        SNMP_CALLBACK_POST_PREMIB_READ_CONFIG, NULL);
}

/*******************************************************************-o-******
 * set_configuration_directory
 *
 * Parameters:
 *      char *dir - value of the directory
 * Sets the configuration directory. Multiple directories can be
 * specified, but need to be seperated by 'ENV_SEPARATOR_CHAR'.
 */
void
set_configuration_directory(const char *dir)
{
    netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, 
			  NETSNMP_DS_LIB_CONFIGURATION_DIR, dir);
}

/*******************************************************************-o-******
 * get_configuration_directory
 *
 * Parameters: -
 * Retrieve the configuration directory or directories.
 * (For backwards compatibility that is:
 *       SNMPCONFPATH, SNMPSHAREPATH, SNMPLIBPATH, HOME/.snmp
 * First check whether the value is set.
 * If not set give it the default value.
 * Return the value.
 * We always retrieve it new, since we have to do it anyway if it is just set.
 */
const char     *
get_configuration_directory()
{
    char            defaultPath[SPRINT_MAX_LEN];
    char           *homepath;

    if (NULL == netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				      NETSNMP_DS_LIB_CONFIGURATION_DIR)) {
        homepath = netsnmp_getenv("HOME");
        snprintf(defaultPath, sizeof(defaultPath), "%s%c%s%c%s%s%s%s",
                SNMPCONFPATH, ENV_SEPARATOR_CHAR,
                SNMPSHAREPATH, ENV_SEPARATOR_CHAR, SNMPLIBPATH,
                ((homepath == NULL) ? "" : ENV_SEPARATOR),
                ((homepath == NULL) ? "" : homepath),
                ((homepath == NULL) ? "" : "/.snmp"));
        defaultPath[ sizeof(defaultPath)-1 ] = 0;
        set_configuration_directory(defaultPath);
    }
    return (netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				  NETSNMP_DS_LIB_CONFIGURATION_DIR));
}

/*******************************************************************-o-******
 * set_persistent_directory
 *
 * Parameters:
 *      char *dir - value of the directory
 * Sets the configuration directory. 
 * No multiple directories may be specified.
 * (However, this is not checked)
 */
void
set_persistent_directory(const char *dir)
{
    netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, 
			  NETSNMP_DS_LIB_PERSISTENT_DIR, dir);
}

/*******************************************************************-o-******
 * get_persistent_directory
 *
 * Parameters: -
 * Function will retrieve the persisten directory value.
 * First check whether the value is set.
 * If not set give it the default value.
 * Return the value. 
 * We always retrieve it new, since we have to do it anyway if it is just set.
 */
const char     *
get_persistent_directory()
{
    if (NULL == netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				      NETSNMP_DS_LIB_PERSISTENT_DIR)) {
        char *persdir = netsnmp_getenv("SNMP_PERSISTENT_DIR");
        if (NULL == persdir)
            persdir = NETSNMP_PERSISTENT_DIRECTORY;
        set_persistent_directory(persdir);
    }
    return (netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				  NETSNMP_DS_LIB_PERSISTENT_DIR));
}

/*******************************************************************-o-******
 * set_temp_file_pattern
 *
 * Parameters:
 *      char *pattern - value of the file pattern
 * Sets the temp file pattern. 
 * Multiple patterns may not be specified.
 * (However, this is not checked)
 */
void
set_temp_file_pattern(const char *pattern)
{
    netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, 
			  NETSNMP_DS_LIB_TEMP_FILE_PATTERN, pattern);
}

/*******************************************************************-o-******
 * get_temp_file_pattern
 *
 * Parameters: -
 * Function will retrieve the temp file pattern value.
 * First check whether the value is set.
 * If not set give it the default value.
 * Return the value. 
 * We always retrieve it new, since we have to do it anyway if it is just set.
 */
const char     *
get_temp_file_pattern()
{
    if (NULL == netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				      NETSNMP_DS_LIB_TEMP_FILE_PATTERN)) {
        set_temp_file_pattern(NETSNMP_TEMP_FILE_PATTERN);
    }
    return (netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				  NETSNMP_DS_LIB_TEMP_FILE_PATTERN));
}

/**
 * utility routine for read_config_files
 */
static void
read_config_files_in_path(const char *path, struct config_files *ctmp,
                          int when, const char *perspath, const char *persfile)
{
    int             done, j;
    char            configfile[300];
    char           *cptr1, *cptr2, *envconfpath;
    struct stat     statbuf;

    if ((NULL == path) || (NULL == ctmp))
        return;

    envconfpath = strdup(path);

    DEBUGMSGTL(("read_config", " config path used for %s:%s (persistent path:%s)\n",
                ctmp->fileHeader, envconfpath, perspath));
    cptr1 = cptr2 = envconfpath;
    done = 0;
    while (*cptr2 != 0) {
        while (*cptr1 != 0 && *cptr1 != ENV_SEPARATOR_CHAR)
            cptr1++;
        if (*cptr1 == 0)
            done = 1;
        else
            *cptr1 = 0;
        /*
         * for proper persistent storage retrival, we need to read old backup
         * copies of the previous storage files.  If the application in
         * question has died without the proper call to snmp_clean_persistent,
         * then we read all the configuration files we can, starting with
         * the oldest first.
         */
        if (strncmp(cptr2, perspath, strlen(perspath)) == 0 ||
            (persfile != NULL &&
             strncmp(cptr2, persfile, strlen(persfile)) == 0)) {
            /*
             * limit this to the known storage directory only 
             */
            for (j = 0; j <= NETSNMP_MAX_PERSISTENT_BACKUPS; j++) {
                snprintf(configfile, sizeof(configfile),
                         "%s/%s.%d.conf", cptr2,
                         ctmp->fileHeader, j);
                configfile[ sizeof(configfile)-1 ] = 0;
                if (stat(configfile, &statbuf) != 0) {
                    /*
                     * file not there, continue 
                     */
                    break;
                } else {
                    /*
                     * backup exists, read it 
                     */
                    DEBUGMSGTL(("read_config_files",
                                "old config file found: %s, parsing\n",
                                configfile));
                    read_config(configfile, ctmp->start, when);
                }
            }
        }
        snprintf(configfile, sizeof(configfile),
                 "%s/%s.conf", cptr2, ctmp->fileHeader);
        configfile[ sizeof(configfile)-1 ] = 0;
        read_config(configfile, ctmp->start, when);
        snprintf(configfile, sizeof(configfile),
                 "%s/%s.local.conf", cptr2, ctmp->fileHeader);
        configfile[ sizeof(configfile)-1 ] = 0;
        read_config(configfile, ctmp->start, when);

        if(done)
            break;

        cptr2 = ++cptr1;
    }
    SNMP_FREE(envconfpath);
}

/*******************************************************************-o-******
 * read_config_files
 *
 * Parameters:
 *	when	== PREMIB_CONFIG, NORMAL_CONFIG  -or-  EITHER_CONFIG
 *
 *
 * Traverse the list of config file types, performing the following actions
 * for each --
 *
 * First, build a search path for config files.  If the contents of 
 * environment variable SNMPCONFPATH are NULL, then use the following
 * path list (where the last entry exists only if HOME is non-null):
 *
 *	SNMPSHAREPATH:SNMPLIBPATH:${HOME}/.snmp
 *
 * Then, In each of these directories, read config files by the name of:
 *
 *	<dir>/<fileHeader>.conf		-AND-
 *	<dir>/<fileHeader>.local.conf
 *
 * where <fileHeader> is taken from the config file type structure.
 *
 *
 * PREMIB_CONFIG causes free_config() to be invoked prior to any other action.
 *
 *
 * EXITs if any 'config_errors' are logged while parsing config file lines.
 */
void
read_config_files(int when)
{
    const char     *confpath, *perspath, *persfile, *envconfpath;
    struct config_files *ctmp = config_files;

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DONT_PERSIST_STATE)
     || netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DISABLE_CONFIG_LOAD)) return;

    config_errors = 0;

    if (when == PREMIB_CONFIG)
        free_config();

    /*
     * these shouldn't change
     */
    confpath = get_configuration_directory();
    persfile = netsnmp_getenv("SNMP_PERSISTENT_FILE");
    envconfpath = netsnmp_getenv("SNMPCONFPATH");

    /*
     * read all config file types 
     */
    for (; ctmp != NULL; ctmp = ctmp->next) {

        /*
         * read the config files 
         */
        perspath = get_persistent_directory();
        if (envconfpath == NULL) {
            /*
             * read just the config files (no persistent stuff), since
             * persistent path can change via conf file. Then get the
             * current persistent directory, and read files there.
             */
            read_config_files_in_path(confpath, ctmp, when, perspath,
                                      persfile);
            perspath = get_persistent_directory();
            read_config_files_in_path(perspath, ctmp, when, perspath,
                                      persfile);
        }
        else {
            /*
             * only read path specified by user
             */
            read_config_files_in_path(envconfpath, ctmp, when, perspath,
                                      persfile);
        }
    }

    if (config_errors) {
        snmp_log(LOG_ERR, "net-snmp: %d error(s) in config file(s)\n",
                 config_errors);
    }
}

void
read_config_print_usage(const char *lead)
{
    struct config_files *ctmp = config_files;
    struct config_line *ltmp;

    if (lead == NULL)
        lead = "";

    for (ctmp = config_files; ctmp != NULL; ctmp = ctmp->next) {
        snmp_log(LOG_INFO, "%sIn %s.conf and %s.local.conf:\n", lead,
                 ctmp->fileHeader, ctmp->fileHeader);
        for (ltmp = ctmp->start; ltmp != NULL; ltmp = ltmp->next) {
            DEBUGIF("read_config_usage") {
                if (ltmp->config_time == PREMIB_CONFIG)
                    DEBUGMSG(("read_config_usage", "*"));
                else
                    DEBUGMSG(("read_config_usage", " "));
            }
            if (ltmp->help) {
                snmp_log(LOG_INFO, "%s%s%-24s %s\n", lead, lead,
                         ltmp->config_token, ltmp->help);
            } else {
                DEBUGIF("read_config_usage") {
                    snmp_log(LOG_INFO, "%s%s%-24s [NO HELP]\n", lead, lead,
                             ltmp->config_token);
                }
            }
        }
    }
}

/**
 * read_config_store intended for use by applications to store permenant
 * configuration information generated by sets or persistent counters.
 * Appends line to a file named either ENV(SNMP_PERSISTENT_FILE) or
 *   "<NETSNMP_PERSISTENT_DIRECTORY>/<type>.conf".
 * Adds a trailing newline to the stored file if necessary.
 *
 * @param type is the application name
 * @param line is the configuration line written to the application name's
 * configuration file
 *      
 * @return void
  */
void
read_config_store(const char *type, const char *line)
{
#ifdef NETSNMP_PERSISTENT_DIRECTORY
    char            file[512], *filep;
    FILE           *fout;
#ifdef NETSNMP_PERSISTENT_MASK
    mode_t          oldmask;
#endif

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DONT_PERSIST_STATE)
     || netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DISABLE_PERSISTENT_LOAD)) return;

    /*
     * store configuration directives in the following order of preference:
     * 1. ENV variable SNMP_PERSISTENT_FILE
     * 2. configured <NETSNMP_PERSISTENT_DIRECTORY>/<type>.conf
     */
    if ((filep = netsnmp_getenv("SNMP_PERSISTENT_FILE")) == NULL) {
        snprintf(file, sizeof(file),
                 "%s/%s.conf", get_persistent_directory(), type);
        file[ sizeof(file)-1 ] = 0;
        filep = file;
    }
#ifdef NETSNMP_PERSISTENT_MASK
    oldmask = umask(NETSNMP_PERSISTENT_MASK);
#endif
    if (mkdirhier(filep, NETSNMP_AGENT_DIRECTORY_MODE, 1)) {
        snmp_log(LOG_ERR,
                 "Failed to create the persistent directory for %s\n",
                 file);
    }
    if ((fout = fopen(filep, "a")) != NULL) {
        fprintf(fout, "%s", line);
        if (line[strlen(line)] != '\n')
            fprintf(fout, "\n");
        DEBUGMSGTL(("read_config", "storing: %s\n", line));
        fclose(fout);
    } else {
        snmp_log(LOG_ERR, "read_config_store open failure on %s\n", filep);
    }
#ifdef NETSNMP_PERSISTENT_MASK
    umask(oldmask);
#endif

#endif
}                               /* end read_config_store() */

void
read_app_config_store(const char *line)
{
    read_config_store(netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
					    NETSNMP_DS_LIB_APPTYPE), line);
}




/*******************************************************************-o-******
 * snmp_save_persistent
 *
 * Parameters:
 *	*type
 *      
 *
 * Save the file "<NETSNMP_PERSISTENT_DIRECTORY>/<type>.conf" into a backup copy
 * called "<NETSNMP_PERSISTENT_DIRECTORY>/<type>.%d.conf", which %d is an
 * incrementing number on each call, but less than NETSNMP_MAX_PERSISTENT_BACKUPS.
 *
 * Should be called just before all persistent information is supposed to be
 * written to move aside the existing persistent cache.
 * snmp_clean_persistent should then be called afterward all data has been
 * saved to remove these backup files.
 *
 * Note: on an rename error, the files are removed rather than saved.
 *
 */
void
snmp_save_persistent(const char *type)
{
    char            file[512], fileold[SPRINT_MAX_LEN];
    struct stat     statbuf;
    int             j;

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DONT_PERSIST_STATE)
     || netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DISABLE_PERSISTENT_SAVE)) return;

    DEBUGMSGTL(("snmp_save_persistent", "saving %s files...\n", type));
    snprintf(file, sizeof(file),
             "%s/%s.conf", get_persistent_directory(), type);
    file[ sizeof(file)-1 ] = 0;
    if (stat(file, &statbuf) == 0) {
        for (j = 0; j <= NETSNMP_MAX_PERSISTENT_BACKUPS; j++) {
            snprintf(fileold, sizeof(fileold),
                     "%s/%s.%d.conf", get_persistent_directory(), type, j);
            fileold[ sizeof(fileold)-1 ] = 0;
            if (stat(fileold, &statbuf) != 0) {
                DEBUGMSGTL(("snmp_save_persistent",
                            " saving old config file: %s -> %s.\n", file,
                            fileold));
                if (rename(file, fileold)) {
                    snmp_log(LOG_ERR, "Cannot rename %s to %s\n", file, fileold);
                     /* moving it failed, try nuking it, as leaving
                      * it around is very bad. */
                    if (unlink(file) == -1)
                        snmp_log(LOG_ERR, "Cannot unlink %s\n", file);
                }
                break;
            }
        }
    }
    /*
     * save a warning header to the top of the new file 
     */
    snprintf(fileold, sizeof(fileold),
            "#\n# net-snmp (or ucd-snmp) persistent data file.\n#\n############################################################################\n# STOP STOP STOP STOP STOP STOP STOP STOP STOP \n#\n#          **** DO NOT EDIT THIS FILE ****\n#\n# STOP STOP STOP STOP STOP STOP STOP STOP STOP \n############################################################################\n#\n# DO NOT STORE CONFIGURATION ENTRIES HERE.\n# Please save normal configuration tokens for %s in SNMPCONFPATH/%s.conf.\n# Only \"createUser\" tokens should be placed here by %s administrators.\n# (Did I mention: do not edit this file?)\n#\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n",
            type, type, type);
    fileold[ sizeof(fileold)-1 ] = 0;
    read_config_store(type, fileold);
}


/*******************************************************************-o-******
 * snmp_clean_persistent
 *
 * Parameters:
 *	*type
 *      
 *
 * Unlink all backup files called "<NETSNMP_PERSISTENT_DIRECTORY>/<type>.%d.conf".
 *
 * Should be called just after we successfull dumped the last of the
 * persistent data, to remove the backup copies of previous storage dumps.
 *
 * XXX  Worth overwriting with random bytes first?  This would
 *	ensure that the data is destroyed, even a buffer containing the
 *	data persists in memory or swap.  Only important if secrets
 *	will be stored here.
 */
void
snmp_clean_persistent(const char *type)
{
    char            file[512];
    struct stat     statbuf;
    int             j;

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DONT_PERSIST_STATE)
     || netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                               NETSNMP_DS_LIB_DISABLE_PERSISTENT_SAVE)) return;

    DEBUGMSGTL(("snmp_clean_persistent", "cleaning %s files...\n", type));
    snprintf(file, sizeof(file),
             "%s/%s.conf", get_persistent_directory(), type);
    file[ sizeof(file)-1 ] = 0;
    if (stat(file, &statbuf) == 0) {
        for (j = 0; j <= NETSNMP_MAX_PERSISTENT_BACKUPS; j++) {
            snprintf(file, sizeof(file),
                     "%s/%s.%d.conf", get_persistent_directory(), type, j);
            file[ sizeof(file)-1 ] = 0;
            if (stat(file, &statbuf) == 0) {
                DEBUGMSGTL(("snmp_clean_persistent",
                            " removing old config file: %s\n", file));
                if (unlink(file) == -1)
                    snmp_log(LOG_ERR, "Cannot unlink %s\n", file);
            }
        }
    }
}




/*
 * config_perror: prints a warning string associated with a file and
 * line number of a .conf file and increments the error count. 
 */
void
config_perror(const char *str)
{
    snmp_log(LOG_ERR, "%s: line %d: Error: %s\n", curfilename, linecount,
             str);
    config_errors++;
}

void
config_pwarn(const char *str)
{
    snmp_log(LOG_WARNING, "%s: line %d: Warning: %s\n", curfilename,
             linecount, str);
}

/*
 * skip all white spaces and return 1 if found something either end of
 * line or a comment character 
 */
char           *
skip_white(char *ptr)
{
    if (ptr == NULL)
        return (NULL);
    while (*ptr != 0 && isspace(*ptr))
        ptr++;
    if (*ptr == 0 || *ptr == '#')
        return (NULL);
    return (ptr);
}

char           *
skip_not_white(char *ptr)
{
    if (ptr == NULL)
        return (NULL);
    while (*ptr != 0 && !isspace(*ptr))
        ptr++;
    if (*ptr == 0 || *ptr == '#')
        return (NULL);
    return (ptr);
}

char           *
skip_token(char *ptr)
{
    ptr = skip_white(ptr);
    ptr = skip_not_white(ptr);
    ptr = skip_white(ptr);
    return (ptr);
}

/*
 * copy_word
 * copies the next 'token' from 'from' into 'to', maximum len-1 characters.
 * currently a token is anything seperate by white space
 * or within quotes (double or single) (i.e. "the red rose" 
 * is one token, \"the red rose\" is three tokens)
 * a '\' character will allow a quote character to be treated
 * as a regular character 
 * It returns a pointer to first non-white space after the end of the token
 * being copied or to 0 if we reach the end.
 * Note: Partially copied words (greater than len) still returns a !NULL ptr
 * Note: partially copied words are, however, null terminated.
 */

char           *
copy_nword(char *from, char *to, int len)
{
    char            quote;
    if (!from || !to)
        return NULL;
    if ((*from == '\"') || (*from == '\'')) {
        quote = *(from++);
        while ((*from != quote) && (*from != 0)) {
            if ((*from == '\\') && (*(from + 1) != 0)) {
                if (len > 0) {  /* don't copy beyond len bytes */
                    *to++ = *(from + 1);
                    if (--len == 0)
                        *(to - 1) = '\0';       /* null protect the last spot */
                }
                from = from + 2;
            } else {
                if (len > 0) {  /* don't copy beyond len bytes */
                    *to++ = *from++;
                    if (--len == 0)
                        *(to - 1) = '\0';       /* null protect the last spot */
                } else
                    from++;
            }
        }
        if (*from == 0) {
            DEBUGMSGTL(("read_config_copy_word",
                        "no end quote found in config string\n"));
        } else
            from++;
    } else {
        while (*from != 0 && !isspace(*from)) {
            if ((*from == '\\') && (*(from + 1) != 0)) {
                if (len > 0) {  /* don't copy beyond len bytes */
                    *to++ = *(from + 1);
                    if (--len == 0)
                        *(to - 1) = '\0';       /* null protect the last spot */
                }
                from = from + 2;
            } else {
                if (len > 0) {  /* don't copy beyond len bytes */
                    *to++ = *from++;
                    if (--len == 0)
                        *(to - 1) = '\0';       /* null protect the last spot */
                } else
                    from++;
            }
        }
    }
    if (len > 0)
        *to = 0;
    from = skip_white(from);
    return (from);
}                               /* copy_word */

/*
 * copy_word
 * copies the next 'token' from 'from' into 'to'.
 * currently a token is anything seperate by white space
 * or within quotes (double or single) (i.e. "the red rose" 
 * is one token, \"the red rose\" is three tokens)
 * a '\' character will allow a quote character to be treated
 * as a regular character 
 * It returns a pointer to first non-white space after the end of the token
 * being copied or to 0 if we reach the end.
 */

static int      have_warned = 0;
char           *
copy_word(char *from, char *to)
{
    if (!have_warned) {
        snmp_log(LOG_INFO,
                 "copy_word() called.  Use copy_nword() instead.\n");
        have_warned = 1;
    }
    return copy_nword(from, to, SPRINT_MAX_LEN);
}                               /* copy_word */

/*
 * read_config_save_octet_string(): saves an octet string as a length
 * followed by a string of hex 
 */
char           *
read_config_save_octet_string(char *saveto, u_char * str, size_t len)
{
    int             i;
    u_char         *cp;

    /*
     * is everything easily printable 
     */
    for (i = 0, cp = str; i < (int) len && cp &&
         (isalpha(*cp) || isdigit(*cp) || *cp == ' '); cp++, i++);

    if (len != 0 && i == (int) len) {
        *saveto++ = '"';
        memcpy(saveto, str, len);
        saveto += len;
        *saveto++ = '"';
        *saveto = '\0';
    } else {
        if (str != NULL) {
            sprintf(saveto, "0x");
            saveto += 2;
            for (i = 0; i < (int) len; i++) {
                sprintf(saveto, "%02x", str[i]);
                saveto = saveto + 2;
            }
        } else {
            sprintf(saveto, "\"\"");
            saveto += 2;
        }
    }
    return saveto;
}

/*
 * read_config_read_octet_string(): reads an octet string that was
 * saved by the read_config_save_octet_string() function 
 */
char           *
read_config_read_octet_string(char *readfrom, u_char ** str, size_t * len)
{
    u_char         *cptr = NULL;
    char           *cptr1;
    u_int           tmp;
    int             i;
    size_t          ilen;

    if (readfrom == NULL || str == NULL)
        return NULL;

    if (strncasecmp(readfrom, "0x", 2) == 0) {
        /*
         * A hex string submitted. How long? 
         */
        readfrom += 2;
        cptr1 = skip_not_white(readfrom);
        if (cptr1)
            ilen = (cptr1 - readfrom);
        else
            ilen = strlen(readfrom);

        if (ilen % 2) {
            snmp_log(LOG_WARNING,"invalid hex string: wrong length\n");
            DEBUGMSGTL(("read_config_read_octet_string",
                        "invalid hex string: wrong length"));
            return NULL;
        }
        ilen = ilen / 2;

        /*
         * malloc data space if needed (+1 for good measure) 
         */
        if (*str == NULL) {
            if ((cptr = (u_char *) malloc(ilen + 1)) == NULL) {
                return NULL;
            }
            *str = cptr;
        } else {
            /*
             * don't require caller to have +1 for good measure, and 
             * bail if not enough space.
             */
            if (ilen > *len) {
                snmp_log(LOG_WARNING,"buffer too small to read octet string (%d < %d)\n",
                         *len, ilen);
                DEBUGMSGTL(("read_config_read_octet_string",
                            "buffer too small (%lu < %lu)", (unsigned long)*len, (unsigned long)ilen));
                cptr1 = skip_not_white(readfrom);
                return skip_white(cptr1);
            }
            cptr = *str;
        }
        *len = ilen;

        /*
         * copy validated data 
         */
        for (i = 0; i < (int) *len; i++) {
            if (1 == sscanf(readfrom, "%2x", &tmp))
                *cptr++ = (u_char) tmp;
            else {
                /*
                 * we may lose memory, but don't know caller's buffer XX free(cptr); 
                 */
                return (NULL);
            }
            readfrom += 2;
        }
        /*
         * only null terminate if we have the space
         */
        if (ilen > *len) {
            ilen = *len-1;
            *cptr++ = '\0';
        }
        readfrom = skip_white(readfrom);
    } else {
        /*
         * Normal string 
         */

        /*
         * malloc string space if needed (including NULL terminator) 
         */
        if (*str == NULL) {
            char            buf[SNMP_MAXBUF];
            readfrom = copy_nword(readfrom, buf, sizeof(buf));

            *len = strlen(buf);
            if ((cptr = (u_char *) malloc(*len + 1)) == NULL)
                return NULL;
            *str = cptr;
            if (cptr) {
                memcpy(cptr, buf, *len + 1);
            }
        } else {
            readfrom = copy_nword(readfrom, (char *) *str, *len);
            *len = strlen((char *) *str);
        }
    }

    return readfrom;
}


/*
 * read_config_save_objid(): saves an objid as a numerical string 
 */
char           *
read_config_save_objid(char *saveto, oid * objid, size_t len)
{
    int             i;

    if (len == 0) {
        strcat(saveto, "NULL");
        saveto += strlen(saveto);
        return saveto;
    }

    /*
     * in case len=0, this makes it easier to read it back in 
     */
    for (i = 0; i < (int) len; i++) {
        sprintf(saveto, ".%ld", objid[i]);
        saveto += strlen(saveto);
    }
    return saveto;
}

/*
 * read_config_read_objid(): reads an objid from a format saved by the above 
 */
char           *
read_config_read_objid(char *readfrom, oid ** objid, size_t * len)
{

    if (objid == NULL || readfrom == NULL || len == NULL)
        return NULL;

    if (*objid == NULL) {
        *len = 0;
        if ((*objid = (oid *) malloc(MAX_OID_LEN * sizeof(oid))) == NULL)
            return NULL;
        *len = MAX_OID_LEN;
    }

    if (strncmp(readfrom, "NULL", 4) == 0) {
        /*
         * null length oid 
         */
        *len = 0;
    } else {
        /*
         * qualify the string for read_objid 
         */
        char            buf[SPRINT_MAX_LEN];
        copy_nword(readfrom, buf, sizeof(buf));

        if (!read_objid(buf, *objid, len)) {
            DEBUGMSGTL(("read_config_read_objid", "Invalid OID"));
            *len = 0;
            return NULL;
        }
    }

    readfrom = skip_token(readfrom);
    return readfrom;
}

/**
 * read_config_read_data reads data of a given type from a token(s) on a
 * configuration line.  The supported types are:
 *
 *    - ASN_INTEGER
 *    - ASN_TIMETICKS
 *    - ASN_UNSIGNED
 *    - ASN_OCTET_STR
 *    - ASN_BIT_STR
 *    - ASN_OBJECT_ID
 *
 * @param type the asn data type to be read in.
 *
 * @param readfrom the configuration line data to be read.
 *
 * @param dataptr an allocated pointer expected to match the type being read
 *        (int *, u_int *, char **, oid **)
 *
 * @param len is the length of an asn oid or octet/bit string, not required
 *            for the asn integer, unsigned integer, and timeticks types
 *
 * @return the next token in the configuration line.  NULL if none left or
 * if an unknown type.
 * 
 */
char           *
read_config_read_data(int type, char *readfrom, void *dataptr,
                      size_t * len)
{
    int            *intp;
    char          **charpp;
    oid           **oidpp;
    unsigned int   *uintp;

    if (dataptr && readfrom)
        switch (type) {
        case ASN_INTEGER:
            intp = (int *) dataptr;
            *intp = atoi(readfrom);
            readfrom = skip_token(readfrom);
            return readfrom;

        case ASN_TIMETICKS:
        case ASN_UNSIGNED:
            uintp = (unsigned int *) dataptr;
            *uintp = strtoul(readfrom, NULL, 0);
            readfrom = skip_token(readfrom);
            return readfrom;

        case ASN_IPADDRESS:
            intp = (int *) dataptr;
            *intp = inet_addr(readfrom);
            if ((*intp == -1) &&
                (strncmp(readfrom, "255.255.255.255", 15) != 0))
                return NULL;
            readfrom = skip_token(readfrom);
            return readfrom;

        case ASN_OCTET_STR:
        case ASN_BIT_STR:
            charpp = (char **) dataptr;
            return read_config_read_octet_string(readfrom,
                                                 (u_char **) charpp, len);

        case ASN_OBJECT_ID:
            oidpp = (oid **) dataptr;
            return read_config_read_objid(readfrom, oidpp, len);

        default:
            DEBUGMSGTL(("read_config_read_data", "Fail: Unknown type: %d",
                        type));
            return NULL;
        }
    return NULL;
}

/*
 * read_config_read_memory():
 * 
 * similar to read_config_read_data, but expects a generic memory
 * pointer rather than a specific type of pointer.  Len is expected to
 * be the amount of available memory.
 */
char           *
read_config_read_memory(int type, char *readfrom,
                        char *dataptr, size_t * len)
{
    int            *intp;
    unsigned int   *uintp;
    char            buf[SPRINT_MAX_LEN];

    if (!dataptr || !readfrom)
        return NULL;

    switch (type) {
    case ASN_INTEGER:
        if (*len < sizeof(int))
            return NULL;
        intp = (int *) dataptr;
        readfrom = copy_nword(readfrom, buf, sizeof(buf));
        *intp = atoi(buf);
        *len = sizeof(int);
        return readfrom;

    case ASN_COUNTER:
    case ASN_TIMETICKS:
    case ASN_UNSIGNED:
        if (*len < sizeof(unsigned int))
            return NULL;
        uintp = (unsigned int *) dataptr;
        readfrom = copy_nword(readfrom, buf, sizeof(buf));
        *uintp = strtoul(buf, NULL, 0);
        *len = sizeof(unsigned int);
        return readfrom;

    case ASN_IPADDRESS:
        if (*len < sizeof(int))
            return NULL;
        intp = (int *) dataptr;
        readfrom = copy_nword(readfrom, buf, sizeof(buf));
        *intp = inet_addr(buf);
        if ((*intp == -1) && (strcmp(buf, "255.255.255.255") != 0))
            return NULL;
        *len = sizeof(int);
        return readfrom;

    case ASN_OCTET_STR:
    case ASN_BIT_STR:
    case ASN_PRIV_IMPLIED_OCTET_STR:
        return read_config_read_octet_string(readfrom,
                                             (u_char **) & dataptr, len);

    case ASN_PRIV_IMPLIED_OBJECT_ID:
    case ASN_OBJECT_ID:
        readfrom =
            read_config_read_objid(readfrom, (oid **) & dataptr, len);
        *len *= sizeof(oid);
        return readfrom;

    case ASN_COUNTER64:
    {
        if (*len < sizeof(U64))
            return NULL;
        *len = sizeof(U64);
        read64((U64 *) dataptr, readfrom);
        readfrom = skip_token(readfrom);
        return readfrom;
    }

    default:
        DEBUGMSGTL(("read_config_read_memory", "Fail: Unknown type: %d",
                    type));
        return NULL;
    }
    return NULL;
}

/**
 * read_config_store_data stores data of a given type to a configuration line
 * into the storeto buffer.
 * Calls read_config_store_data_prefix with the prefix parameter set to a char
 * space.  The supported types are:
 *
 *    - ASN_INTEGER
 *    - ASN_TIMETICKS
 *    - ASN_UNSIGNED
 *    - ASN_OCTET_STR
 *    - ASN_BIT_STR
 *    - ASN_OBJECT_ID
 *
 * @param type    the asn data type to be stored
 *
 * @param storeto a pre-allocated char buffer which will contain the data
 *                to be stored
 *
 * @param dataptr contains the value to be stored, the supported pointers:
 *                (int *, u_int *, char **, oid **)
 *
 * @param len     is the length of the value to be stored
 *                (not required for the asn integer, unsigned integer,
 *                 and timeticks types)
 *
 * @return character pointer to the end of the line. NULL if an unknown type.
 */
char           *
read_config_store_data(int type, char *storeto, void *dataptr, size_t * len)
{
    return read_config_store_data_prefix(' ', type, storeto, dataptr,
                                                         (len ? *len : 0));
}

char           *
read_config_store_data_prefix(char prefix, int type, char *storeto,
                              void *dataptr, size_t len)
{
    int            *intp;
    u_char        **charpp;
    unsigned int   *uintp;
    struct in_addr  in;
    oid           **oidpp;

    if (dataptr && storeto)
        switch (type) {
        case ASN_INTEGER:
            intp = (int *) dataptr;
            sprintf(storeto, "%c%d", prefix, *intp);
            return (storeto + strlen(storeto));

        case ASN_TIMETICKS:
        case ASN_UNSIGNED:
            uintp = (unsigned int *) dataptr;
            sprintf(storeto, "%c%u", prefix, *uintp);
            return (storeto + strlen(storeto));

        case ASN_IPADDRESS:
            in.s_addr = *(unsigned int *) dataptr; 
            sprintf(storeto, "%c%s", prefix, inet_ntoa(in));
            return (storeto + strlen(storeto));

        case ASN_OCTET_STR:
        case ASN_BIT_STR:
            *storeto++ = prefix;
            charpp = (u_char **) dataptr;
            return read_config_save_octet_string(storeto, *charpp, len);

        case ASN_OBJECT_ID:
            *storeto++ = prefix;
            oidpp = (oid **) dataptr;
            return read_config_save_objid(storeto, *oidpp, len);

        default:
            DEBUGMSGTL(("read_config_store_data_prefix",
                        "Fail: Unknown type: %d", type));
            return NULL;
        }
    return NULL;
}
/** @} */

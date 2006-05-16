#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/library/snmp_debug.h>        /* For this file's "internal" definitions */
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/mib.h>
#include <net-snmp/library/snmp_api.h>

static int      dodebug = SNMP_ALWAYS_DEBUG;
static int      debug_num_tokens = 0;
static char    *debug_tokens[MAX_DEBUG_TOKENS];
static int      debug_print_everything = 0;

/*
 * indent debugging:  provide a space padded section to return an indent for 
 */
static int      debugindent = 0;
#define INDENTMAX 80
static char     debugindentchars[] =
    "                                                                                ";

/*
 * Prototype definitions 
 */
void            debug_config_register_tokens(const char *configtoken,
                                             char *tokens);
void            debug_config_turn_on_debugging(const char *configtoken,
                                               char *line);

char           *
debug_indent(void)
{
    return debugindentchars;
}

void
debug_indent_add(int amount)
{
    if (debugindent + amount >= 0 && debugindent + amount < 80) {
        debugindentchars[debugindent] = ' ';
        debugindent += amount;
        debugindentchars[debugindent] = '\0';
    }
}

void
debug_config_register_tokens(const char *configtoken, char *tokens)
{
    debug_register_tokens(tokens);
}

void
debug_config_turn_on_debugging(const char *configtoken, char *line)
{
    snmp_set_do_debugging(atoi(line));
}

void
snmp_debug_init(void)
{
    debugindentchars[0] = '\0'; /* zero out the debugging indent array. */
    register_prenetsnmp_mib_handler("snmp", "doDebugging",
                                    debug_config_turn_on_debugging, NULL,
                                    "(1|0)");
    register_prenetsnmp_mib_handler("snmp", "debugTokens",
                                    debug_config_register_tokens, NULL,
                                    "token[,token...]");
}

void
debug_register_tokens(char *tokens)
{
    char           *newp, *cp;

    if (tokens == 0 || *tokens == 0)
        return;

    newp = strdup(tokens);      /* strtok messes it up */
    cp = strtok(newp, DEBUG_TOKEN_DELIMITER);
    while (cp) {
        if (strlen(cp) < MAX_DEBUG_TOKEN_LEN) {
            if (strcasecmp(cp, DEBUG_ALWAYS_TOKEN) == 0)
                debug_print_everything = 1;
            else if (debug_num_tokens < MAX_DEBUG_TOKENS)
                debug_tokens[debug_num_tokens++] = strdup(cp);
        }
        cp = strtok(NULL, DEBUG_TOKEN_DELIMITER);
    }
    free(newp);
}


/*
 * debug_is_token_registered(char *TOKEN):
 * 
 * returns SNMPERR_SUCCESS
 * or SNMPERR_GENERR
 * 
 * if TOKEN has been registered and debugging support is turned on.
 */
int
debug_is_token_registered(const char *token)
{
    int             i;

    /*
     * debugging flag is on or off 
     */
    if (!dodebug)
        return SNMPERR_GENERR;

    if (debug_num_tokens == 0 || debug_print_everything) {
        /*
         * no tokens specified, print everything 
         */
        return SNMPERR_SUCCESS;
    } else {
        for (i = 0; i < debug_num_tokens; i++) {
            if (strncmp(debug_tokens[i], token, strlen(debug_tokens[i])) ==
                0) {
                return SNMPERR_SUCCESS;
            }
        }
    }
    return SNMPERR_GENERR;
}

void
#if HAVE_STDARG_H
debugmsg(const char *token, const char *format, ...)
#else
debugmsg(va_alist)
     va_dcl
#endif
{
    va_list         debugargs;

#if HAVE_STDARG_H
    va_start(debugargs, format);
#else
    const char     *format;
    const char     *token;

    va_start(debugargs);
    token = va_arg(debugargs, const char *);
    format = va_arg(debugargs, const char *);   /* ??? */
#endif

    if (debug_is_token_registered(token) == SNMPERR_SUCCESS) {
        snmp_vlog(LOG_DEBUG, format, debugargs);
    }
    va_end(debugargs);
}

void
debugmsg_oid(const char *token, const oid * theoid, size_t len)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;

    if (sprint_realloc_objid(&buf, &buf_len, &out_len, 1, theoid, len)) {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_var(const char *token, netsnmp_variable_list * var)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;

    if (var == NULL || token == NULL) {
        return;
    }

    if (sprint_realloc_variable(&buf, &buf_len, &out_len, 1,
                                var->name, var->name_length, var)) {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_oidrange(const char *token, const oid * theoid, size_t len,
                  size_t var_subid, oid range_ubound)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0, i = 0;
    int             rc = 0;

    if (var_subid == 0) {
        rc = sprint_realloc_objid(&buf, &buf_len, &out_len, 1, theoid,
                                  len);
    } else {
        char            tmpbuf[128];
        rc = sprint_realloc_objid(&buf, &buf_len, &out_len, 1, theoid,
                                  var_subid);
        if (rc) {
            sprintf(tmpbuf, ".%lu--%lu", theoid[var_subid - 1],
                    range_ubound);
            rc = snmp_strcat(&buf, &buf_len, &out_len, 1, tmpbuf);
            if (rc) {
                for (i = var_subid; i < len; i++) {
                    sprintf(tmpbuf, ".%lu", theoid[i]);
                    if (!snmp_strcat(&buf, &buf_len, &out_len, 1, tmpbuf)) {
                        break;
                    }
                }
            }
        }
    }


    if (buf != NULL) {
        debugmsg(token, "%s%s", buf, rc ? "" : " [TRUNCATED]");
        free(buf);
    }
}

void
debugmsg_hex(const char *token, u_char * thedata, size_t len)
{
    u_char         *buf = NULL;
    size_t          buf_len = 0, out_len = 0;

    if (sprint_realloc_hexstring
        (&buf, &buf_len, &out_len, 1, thedata, len)) {
        if (buf != NULL) {
            debugmsg(token, "%s", buf);
        }
    } else {
        if (buf != NULL) {
            debugmsg(token, "%s [TRUNCATED]", buf);
        }
    }

    if (buf != NULL) {
        free(buf);
    }
}

void
debugmsg_hextli(const char *token, u_char * thedata, size_t len)
{
    char            buf[SPRINT_MAX_LEN], token2[SPRINT_MAX_LEN];
    u_char         *b3 = NULL;
    size_t          b3_len = 0, o3_len = 0;
    int             incr;
    sprintf(token2, "dumpx_%s", token);

    /*
     * XX tracing lines removed from this function DEBUGTRACE; 
     */
    DEBUGIF(token2) {
        for (incr = 16; len > 0; len -= incr, thedata += incr) {
            if ((int) len < incr) {
                incr = len;
            }
            /*
             * XXnext two lines were DEBUGPRINTINDENT(token);
             */
            sprintf(buf, "dumpx%s", token);
            debugmsg(buf, "%s: %s", token2, debug_indent());
            if (sprint_realloc_hexstring
                (&b3, &b3_len, &o3_len, 1, thedata, incr)) {
                if (b3 != NULL) {
                    debugmsg(token2, "%s", b3);
                }
            } else {
                if (b3 != NULL) {
                    debugmsg(token2, "%s [TRUNCATED]", b3);
                }
            }
            o3_len = 0;
        }
    }
    if (b3 != NULL) {
        free(b3);
    }
}

void
#if HAVE_STDARG_H
debugmsgtoken(const char *token, const char *format, ...)
#else
debugmsgtoken(va_alist)
     va_dcl
#endif
{
    va_list         debugargs;

#if HAVE_STDARG_H
    va_start(debugargs, format);
#else
    const char     *token;

    va_start(debugargs);
    token = va_arg(debugargs, const char *);
#endif

    debugmsg(token, "%s: ", token);

    va_end(debugargs);
}

/*
 * for speed, these shouldn't be in default_storage space 
 */
void
snmp_set_do_debugging(int val)
{
    dodebug = val;
}

int
snmp_get_do_debugging(void)
{
    return dodebug;
}

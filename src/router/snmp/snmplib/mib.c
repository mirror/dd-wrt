/*
 * mib.c
 *
 * $Id: mib.c,v 1.1.2.1 2004/06/20 21:55:01 nikki Exp $
 *
 * Update: 1998-07-17 <jhy@gsu.edu>
 * Added print_oid_report* functions.
 */
/* Portions of this file are subject to the following copyrights.  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/**********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */
#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
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
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/config_api.h>
#include <net-snmp/utilities.h>

#include <net-snmp/library/asn1.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/mib.h>
#include <net-snmp/library/parse.h>
#include <net-snmp/library/int64.h>
#include <net-snmp/library/snmp_client.h>

/** @defgroup mib_utilities mib parsing and datatype manipulation routines.
 *  @ingroup library
 *
 *  @{
 */

static char    *uptimeString(u_long, char *);

static struct tree *_get_realloc_symbol(const oid * objid, size_t objidlen,
                                        struct tree *subtree,
                                        u_char ** buf, size_t * buf_len,
                                        size_t * out_len,
                                        int allow_realloc,
                                        int *buf_overflow,
                                        struct index_list *in_dices,
                                        size_t * end_of_known);

static void     print_tree_node(FILE *, struct tree *, int);
static void     handle_mibdirs_conf(const char *token, char *line);
static void     handle_mibs_conf(const char *token, char *line);
static void     handle_mibfile_conf(const char *token, char *line);


/*
 * helper functions for get_module_node 
 */
static int      node_to_oid(struct tree *, oid *, size_t *);
static int      _add_strings_to_oid(struct tree *, char *,
                                    oid *, size_t *, size_t);

extern struct tree *tree_head;
static struct tree *tree_top;

struct tree    *Mib;            /* Backwards compatibility */

oid             RFC1213_MIB[] = { 1, 3, 6, 1, 2, 1 };
static char     Standard_Prefix[] = ".1.3.6.1.2.1";

/*
 * Set default here as some uses of read_objid require valid pointer. 
 */
static char    *Prefix = &Standard_Prefix[0];
typedef struct _PrefixList {
    const char     *str;
    int             len;
}              *PrefixListPtr, PrefixList;

/*
 * Here are the prefix strings.
 * Note that the first one finds the value of Prefix or Standard_Prefix.
 * Any of these MAY start with period; all will NOT end with period.
 * Period is added where needed.  See use of Prefix in this module.
 */
PrefixList      mib_prefixes[] = {
    {&Standard_Prefix[0]},      /* placeholder for Prefix data */
    {".iso.org.dod.internet.mgmt.mib-2"},
    {".iso.org.dod.internet.experimental"},
    {".iso.org.dod.internet.private"},
    {".iso.org.dod.internet.snmpParties"},
    {".iso.org.dod.internet.snmpSecrets"},
    {NULL, 0}                   /* end of list */
};


/**
 * @internal
 * Converts timeticks to hours, minutes, seconds string.
 *
 * @param timeticks    The timeticks to convert.
 * @param buf          Buffer to write to, has to be at 
 *                     least 64 Bytes large.
 *       
 * @return The buffer.
 */
static char    *
uptimeString(u_long timeticks, char *buf)
{
    int             centisecs, seconds, minutes, hours, days;

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NUMERIC_TIMETICKS)) {
        sprintf(buf, "%lu", timeticks);
        return buf;
    }


    centisecs = timeticks % 100;
    timeticks /= 100;
    days = timeticks / (60 * 60 * 24);
    timeticks %= (60 * 60 * 24);

    hours = timeticks / (60 * 60);
    timeticks %= (60 * 60);

    minutes = timeticks / 60;
    seconds = timeticks % 60;

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT))
        sprintf(buf, "%d:%d:%02d:%02d.%02d",
                days, hours, minutes, seconds, centisecs);
    else {
        if (days == 0) {
            sprintf(buf, "%d:%02d:%02d.%02d",
                    hours, minutes, seconds, centisecs);
        } else if (days == 1) {
            sprintf(buf, "%d day, %d:%02d:%02d.%02d",
                    days, hours, minutes, seconds, centisecs);
        } else {
            sprintf(buf, "%d days, %d:%02d:%02d.%02d",
                    days, hours, minutes, seconds, centisecs);
        }
    }
    return buf;
}



/**
 * @internal
 * Prints the character pointed to if in human-readable ASCII range,
 * otherwise prints a dot.
 *
 * @param buf Buffer to print the character to.
 * @param ch  Character to print.
 */
static void
sprint_char(char *buf, const u_char ch)
{
    if (isprint(ch)) {
        sprintf(buf, "%c", (int) ch);
    } else {
        sprintf(buf, ".");
    }
}



/**
 * Prints a hexadecimal string into a buffer.
 *
 * The characters pointed by *cp are encoded as hexadecimal string.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      address of the buffer to print to.
 * @param buf_len  address to an integer containing the size of buf.
 * @param out_len  incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param cp       the array of characters to encode.
 * @param len      the array length of cp.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_hexstring(u_char ** buf, size_t * buf_len, size_t * out_len,
                         int allow_realloc, const u_char * cp, size_t len)
{
    const u_char   *tp;
    size_t          lenleft;

    for (; len >= 16; len -= 16) {
        while ((*out_len + 50) >= *buf_len) {
            if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                return 0;
            }
        }

        sprintf((char *) (*buf + *out_len),
                "%02X %02X %02X %02X %02X %02X %02X %02X ", cp[0], cp[1],
                cp[2], cp[3], cp[4], cp[5], cp[6], cp[7]);
        *out_len += strlen((char *) (*buf + *out_len));
        cp += 8;
        sprintf((char *) (*buf + *out_len),
                "%02X %02X %02X %02X %02X %02X %02X %02X", cp[0], cp[1],
                cp[2], cp[3], cp[4], cp[5], cp[6], cp[7]);
        *out_len += strlen((char *) (*buf + *out_len));
        cp += 8;

        if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_HEX_TEXT)) {
            while ((*out_len + 21) >= *buf_len) {
                if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                    return 0;
                }
            }
            sprintf((char *) (*buf + *out_len), "  [");
            *out_len += strlen((char *) (*buf + *out_len));
            for (tp = cp - 16; tp < cp; tp++) {
                sprint_char((char *) (*buf + *out_len), *tp);
                (*out_len)++;
            }
            sprintf((char *) (*buf + *out_len), "]");
            *out_len += strlen((char *) (*buf + *out_len));
        }
        if (len > 16) {
            while ((*out_len + 2) >= *buf_len) {
                if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                    return 0;
                }
            }
            *(*buf + (*out_len)++) = '\n';
            *(*buf + *out_len) = 0;
        }
    }

    lenleft = len;
    for (; len > 0; len--) {
        while ((*out_len + 4) >= *buf_len) {
            if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                return 0;
            }
        }
        sprintf((char *) (*buf + *out_len), "%02X ", *cp++);
        *out_len += strlen((char *) (*buf + *out_len));
    }

    if ((lenleft > 0)
        && netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_HEX_TEXT)) {
        while ((*out_len + 5 + lenleft) >= *buf_len) {
            if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                return 0;
            }
        }
        sprintf((char *) (*buf + *out_len), "  [");
        *out_len += strlen((char *) (*buf + *out_len));
        for (tp = cp - lenleft; tp < cp; tp++) {
            sprint_char((char *) (*buf + *out_len), *tp);
            (*out_len)++;
        }
        sprintf((char *) (*buf + *out_len), "]");
        *out_len += strlen((char *) (*buf + *out_len));
    }
    return 1;
}



/**
 * Prints an ascii string into a buffer.
 *
 * The characters pointed by *cp are encoded as an ascii string.
 * 
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      address of the buffer to print to.
 * @param buf_len  address to an integer containing the size of buf.
 * @param out_len  incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param cp       the array of characters to encode.
 * @param len      the array length of cp.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_asciistring(u_char ** buf, size_t * buf_len,
                           size_t * out_len, int allow_realloc,
                           const u_char * cp, size_t len)
{
    int             i;

    for (i = 0; i < (int) len; i++) {
        if (isprint(*cp)) {
            if (*cp == '\\' || *cp == '"') {
                if ((*out_len >= *buf_len) &&
                    !(allow_realloc && snmp_realloc(buf, buf_len))) {
                    return 0;
                }
                *(*buf + (*out_len)++) = '\\';
            }
            if ((*out_len >= *buf_len) &&
                !(allow_realloc && snmp_realloc(buf, buf_len))) {
                return 0;
            }
            *(*buf + (*out_len)++) = *cp++;
        } else {
            if ((*out_len >= *buf_len) &&
                !(allow_realloc && snmp_realloc(buf, buf_len))) {
                return 0;
            }
            *(*buf + (*out_len)++) = '.';
            cp++;
        }
    }
    if ((*out_len >= *buf_len) &&
        !(allow_realloc && snmp_realloc(buf, buf_len))) {
        return 0;
    }
    *(*buf + *out_len) = '\0';
    return 1;
}

/**
 * Prints an octet string into a buffer.
 *
 * The variable var is encoded as octet string.
 * 
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_octet_string(u_char ** buf, size_t * buf_len,
                            size_t * out_len, int allow_realloc,
                            const netsnmp_variable_list * var,
                            const struct enum_list *enums, const char *hint,
                            const char *units)
{
    size_t          saved_out_len = *out_len;
    const char     *saved_hint = hint;
    int             hex = 0, x = 0;
    u_char         *cp;
    int             output_format;

    if ((var->type != ASN_OCTET_STR) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        const char      str[] = "Wrong Type (should be OCTET STRING): ";
        if (snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }


    if (hint) {
        int             repeat, width = 1;
        long            value;
        char            code = 'd', separ = 0, term = 0, ch, intbuf[16];
        u_char         *ecp;

        if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_strcat(buf, buf_len, out_len, allow_realloc,
                             (const u_char *) "STRING: ")) {
                return 0;
            }
        }
        cp = var->val.string;
        ecp = cp + var->val_len;

        while (cp < ecp) {
            repeat = 1;
            if (*hint) {
                if (*hint == '*') {
                    repeat = *cp++;
                    hint++;
                }
                width = 0;
                while ('0' <= *hint && *hint <= '9')
                    width = (width * 10) + (*hint++ - '0');
                code = *hint++;
                if ((ch = *hint) && ch != '*' && (ch < '0' || ch > '9')
                    && (width != 0
                        || (ch != 'x' && ch != 'd' && ch != 'o')))
                    separ = *hint++;
                else
                    separ = 0;
                if ((ch = *hint) && ch != '*' && (ch < '0' || ch > '9')
                    && (width != 0
                        || (ch != 'x' && ch != 'd' && ch != 'o')))
                    term = *hint++;
                else
                    term = 0;
                if (width == 0)
                    width = 1;
            }

            while (repeat && cp < ecp) {
                value = 0;
                if (code != 'a' && code != 't') {
                    for (x = 0; x < width; x++) {
                        value = value * 256 + *cp++;
                    }
                }
                switch (code) {
                case 'x':
                    sprintf(intbuf, "%lx", value);
                    if (!snmp_strcat
                        (buf, buf_len, out_len, allow_realloc,
                         (u_char *) intbuf)) {
                        return 0;
                    }
                    break;
                case 'd':
                    sprintf(intbuf, "%ld", value);
                    if (!snmp_strcat
                        (buf, buf_len, out_len, allow_realloc,
                         (u_char *) intbuf)) {
                        return 0;
                    }
                    break;
                case 'o':
                    sprintf(intbuf, "%lo", value);
                    if (!snmp_strcat
                        (buf, buf_len, out_len, allow_realloc,
                         (u_char *) intbuf)) {
                        return 0;
                    }
                    break;
                case 't': /* new in rfc 3411 */
                case 'a':
                    while ((*out_len + width + 1) >= *buf_len) {
                        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                            return 0;
                        }
                    }
                    for (x = 0; x < width && cp < ecp; x++) {
                        *(*buf + *out_len) = *cp++;
                        (*out_len)++;
                    }
                    *(*buf + *out_len) = '\0';
                    break;
                default:
                    *out_len = saved_out_len;
                    if (snmp_strcat(buf, buf_len, out_len, allow_realloc,
                                    (const u_char *) "(Bad hint ignored: ")
                        && snmp_strcat(buf, buf_len, out_len,
                                       allow_realloc,
                                       (const u_char *) saved_hint)
                        && snmp_strcat(buf, buf_len, out_len,
                                       allow_realloc,
                                       (const u_char *) ") ")) {
                        return sprint_realloc_octet_string(buf, buf_len,
                                                           out_len,
                                                           allow_realloc,
                                                           var, enums,
                                                           NULL, NULL);
                    } else {
                        return 0;
                    }
                }

                if (cp < ecp && separ) {
                    while ((*out_len + 1) >= *buf_len) {
                        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                            return 0;
                        }
                    }
                    *(*buf + *out_len) = separ;
                    (*out_len)++;
                    *(*buf + *out_len) = '\0';
                }
                repeat--;
            }

            if (term && cp < ecp) {
                while ((*out_len + 1) >= *buf_len) {
                    if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                        return 0;
                    }
                }
                *(*buf + *out_len) = term;
                (*out_len)++;
                *(*buf + *out_len) = '\0';
            }
        }

        if (units) {
            return (snmp_strcat
                    (buf, buf_len, out_len, allow_realloc,
                     (const u_char *) " ")
                    && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                                   (const u_char *) units));
        }
        if ((*out_len >= *buf_len) &&
            !(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
        *(*buf + *out_len) = '\0';

        return 1;
    }

    output_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_STRING_OUTPUT_FORMAT);
    if (0 == output_format) {
        output_format = NETSNMP_STRING_OUTPUT_GUESS;
    }
    switch (output_format) {
    case NETSNMP_STRING_OUTPUT_GUESS:
        hex = 0;
        for (cp = var->val.string, x = 0; x < (int) var->val_len; x++, cp++) {
            if (!isprint(*cp)) {
                hex = 1;
            }
        }
        break;

    case NETSNMP_STRING_OUTPUT_ASCII:
        hex = 0;
        break;

    case NETSNMP_STRING_OUTPUT_HEX:
        hex = 1;
        break;
    }

    if (var->val_len == 0) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *) "\"\"");
    }

    if (hex) {
        if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "\"")) {
                return 0;
            }
        } else {
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "Hex-STRING: ")) {
                return 0;
            }
        }

        if (!sprint_realloc_hexstring(buf, buf_len, out_len, allow_realloc,
                                      var->val.string, var->val_len)) {
            return 0;
        }

        if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "\"")) {
                return 0;
            }
        }
    } else {
        if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            if (!snmp_strcat(buf, buf_len, out_len, allow_realloc,
                             (const u_char *) "STRING: ")) {
                return 0;
            }
        }
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) "\"")) {
            return 0;
        }
        if (!sprint_realloc_asciistring
            (buf, buf_len, out_len, allow_realloc, var->val.string,
             var->val_len)) {
            return 0;
        }
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) "\"")) {
            return 0;
        }
    }

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}

#ifdef OPAQUE_SPECIAL_TYPES

/**
 * Prints a float into a buffer.
 *
 * The variable var is encoded as a floating point value.
 * 
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_float(u_char ** buf, size_t * buf_len,
                     size_t * out_len, int allow_realloc,
                     const netsnmp_variable_list * var,
                     const struct enum_list *enums,
                     const char *hint, const char *units)
{
    if ((var->type != ASN_OPAQUE_FLOAT) &&
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be Float): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) "Opaque: Float: ")) {
            return 0;
        }
    }


    /*
     * How much space needed for max. length float?  128 is overkill.  
     */

    while ((*out_len + 128 + 1) >= *buf_len) {
        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
    }

    sprintf((char *) (*buf + *out_len), "%f", *var->val.floatVal);
    *out_len += strlen((char *) (*buf + *out_len));

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints a double into a buffer.
 *
 * The variable var is encoded as a double precision floating point value.
 * 
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_double(u_char ** buf, size_t * buf_len,
                      size_t * out_len, int allow_realloc,
                      const netsnmp_variable_list * var,
                      const struct enum_list *enums,
                      const char *hint, const char *units)
{
    if ((var->type != ASN_OPAQUE_DOUBLE) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        const char      str[] = "Wrong Type (should be Double): ";
        if (snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) "Opaque: Float: ")) {
            return 0;
        }
    }

    /*
     * How much space needed for max. length double?  128 is overkill.  
     */

    while ((*out_len + 128 + 1) >= *buf_len) {
        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
    }

    sprintf((char *) (*buf + *out_len), "%f", *var->val.doubleVal);
    *out_len += strlen((char *) (*buf + *out_len));

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}

#endif                          /* OPAQUE_SPECIAL_TYPES */


/**
 * Prints a counter into a buffer.
 *
 * The variable var is encoded as a counter value.
 * 
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_counter64(u_char ** buf, size_t * buf_len, size_t * out_len,
                         int allow_realloc,
                         const netsnmp_variable_list * var,
                         const struct enum_list *enums,
                         const char *hint, const char *units)
{
    char            a64buf[I64CHARSZ + 1];

    if ((var->type != ASN_COUNTER64
#ifdef OPAQUE_SPECIAL_TYPES
        && var->type != ASN_OPAQUE_COUNTER64
        && var->type != ASN_OPAQUE_I64 && var->type != ASN_OPAQUE_U64
#endif
        ) && (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be Counter64): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
#ifdef OPAQUE_SPECIAL_TYPES
        if (var->type != ASN_COUNTER64) {
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "Opaque: ")) {
                return 0;
            }
        }
#endif
#ifdef OPAQUE_SPECIAL_TYPES
        switch (var->type) {
        case ASN_OPAQUE_U64:
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "UInt64: ")) {
                return 0;
            }
            break;
        case ASN_OPAQUE_I64:
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "Int64: ")) {
                return 0;
            }
            break;
        case ASN_COUNTER64:
        case ASN_OPAQUE_COUNTER64:
#endif
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) "Counter64: ")) {
                return 0;
            }
#ifdef OPAQUE_SPECIAL_TYPES
        }
#endif
    }
#ifdef OPAQUE_SPECIAL_TYPES
    if (var->type == ASN_OPAQUE_I64) {
        printI64(a64buf, var->val.counter64);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) a64buf)) {
            return 0;
        }
    } else {
#endif
        printU64(a64buf, var->val.counter64);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) a64buf)) {
            return 0;
        }
#ifdef OPAQUE_SPECIAL_TYPES
    }
#endif

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints an object identifier into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_opaque(u_char ** buf, size_t * buf_len,
                      size_t * out_len, int allow_realloc,
                      const netsnmp_variable_list * var,
                      const struct enum_list *enums,
                      const char *hint, const char *units)
{
    if ((var->type != ASN_OPAQUE
#ifdef OPAQUE_SPECIAL_TYPES
        && var->type != ASN_OPAQUE_COUNTER64
        && var->type != ASN_OPAQUE_U64
        && var->type != ASN_OPAQUE_I64
        && var->type != ASN_OPAQUE_FLOAT && var->type != ASN_OPAQUE_DOUBLE
#endif                          /* OPAQUE_SPECIAL_TYPES */
        ) && (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be Opaque): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }
#ifdef OPAQUE_SPECIAL_TYPES
    switch (var->type) {
    case ASN_OPAQUE_COUNTER64:
    case ASN_OPAQUE_U64:
    case ASN_OPAQUE_I64:
        return sprint_realloc_counter64(buf, buf_len, out_len,
                                        allow_realloc, var, enums, hint,
                                        units);
        break;

    case ASN_OPAQUE_FLOAT:
        return sprint_realloc_float(buf, buf_len, out_len, allow_realloc,
                                    var, enums, hint, units);
        break;

    case ASN_OPAQUE_DOUBLE:
        return sprint_realloc_double(buf, buf_len, out_len, allow_realloc,
                                     var, enums, hint, units);
        break;

    case ASN_OPAQUE:
#endif
        if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
            u_char          str[] = "OPAQUE: ";
            if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
                return 0;
            }
        }
        if (!sprint_realloc_hexstring(buf, buf_len, out_len, allow_realloc,
                                      var->val.string, var->val_len)) {
            return 0;
        }
#ifdef OPAQUE_SPECIAL_TYPES
    }
#endif
    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints an object identifier into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_object_identifier(u_char ** buf, size_t * buf_len,
                                 size_t * out_len, int allow_realloc,
                                 const netsnmp_variable_list * var,
                                 const struct enum_list *enums,
                                 const char *hint, const char *units)
{
    int             buf_overflow = 0;

    if ((var->type != ASN_OBJECT_ID) &&
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] =
            "Wrong Type (should be OBJECT IDENTIFIER): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "OID: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }

    netsnmp_sprint_realloc_objid_tree(buf, buf_len, out_len, allow_realloc,
                                      &buf_overflow,
                                      (oid *) (var->val.objid),
                                      var->val_len / sizeof(oid));

    if (buf_overflow) {
        return 0;
    }

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}



/**
 * Prints a timetick variable into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_timeticks(u_char ** buf, size_t * buf_len, size_t * out_len,
                         int allow_realloc,
                         const netsnmp_variable_list * var,
                         const struct enum_list *enums,
                         const char *hint, const char *units)
{
    char            timebuf[32];

    if ((var->type != ASN_TIMETICKS) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be Timeticks): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NUMERIC_TIMETICKS)) {
        char            str[16];
        sprintf(str, "%lu", *(u_long *) var->val.integer);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) str)) {
            return 0;
        }
        return 1;
    }
    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        char            str[32];
        sprintf(str, "Timeticks: (%lu) ", *(u_long *) var->val.integer);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) str)) {
            return 0;
        }
    }
    uptimeString(*(u_long *) (var->val.integer), timebuf);
    if (!snmp_strcat
        (buf, buf_len, out_len, allow_realloc, (const u_char *) timebuf)) {
        return 0;
    }
    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints an integer according to the hint into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may _NOT_ be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_hinted_integer(u_char ** buf, size_t * buf_len,
                              size_t * out_len, int allow_realloc,
                              long val, const char decimaltype,
                              const char *hint, const char *units)
{
    char            fmt[10] = "%l@", tmp[256];
    int             shift, len;

    if (hint[1] == '-') {
        shift = atoi(hint + 2);
    } else {
        shift = 0;
    }

    if (hint[0] == 'd') {
        /*
         * We might *actually* want a 'u' here.  
         */
        fmt[2] = decimaltype;
    } else {
        /*
         * DISPLAY-HINT character is 'b', 'o', or 'x'.  
         */
        fmt[2] = hint[0];
    }

    sprintf(tmp, fmt, val);
    if (shift != 0) {
        len = strlen(tmp);
        if (shift <= len) {
            tmp[len + 1] = 0;
            while (shift--) {
                tmp[len] = tmp[len - 1];
                len--;
            }
            tmp[len] = '.';
        } else {
            tmp[shift + 1] = 0;
            while (shift) {
                if (len-- > 0) {
                    tmp[shift] = tmp[len];
                } else {
                    tmp[shift] = '0';
                }
                shift--;
            }
            tmp[0] = '.';
        }
    }
    return snmp_strcat(buf, buf_len, out_len, allow_realloc, tmp);
}


/**
 * Prints an integer into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_integer(u_char ** buf, size_t * buf_len, size_t * out_len,
                       int allow_realloc,
                       const netsnmp_variable_list * var,
                       const struct enum_list *enums,
                       const char *hint, const char *units)
{
    char           *enum_string = NULL;

    if ((var->type != ASN_INTEGER) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be INTEGER): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }
    for (; enums; enums = enums->next) {
        if (enums->value == *var->val.integer) {
            enum_string = enums->label;
            break;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc,
                         (const u_char *) "INTEGER: ")) {
            return 0;
        }
    }

    if (enum_string == NULL ||
        netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM)) {
        if (hint) {
            if (!(sprint_realloc_hinted_integer(buf, buf_len, out_len,
                                                allow_realloc,
                                                *var->val.integer, 'd',
                                                hint, units))) {
                return 0;
            }
        } else {
            char            str[16];
            sprintf(str, "%ld", *var->val.integer);
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) str)) {
                return 0;
            }
        }
    } else if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) enum_string)) {
            return 0;
        }
    } else {
        char            str[16];
        sprintf(str, "(%ld)", *var->val.integer);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) enum_string)) {
            return 0;
        }
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) str)) {
            return 0;
        }
    }

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints an unsigned integer into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_uinteger(u_char ** buf, size_t * buf_len, size_t * out_len,
                        int allow_realloc,
                        const netsnmp_variable_list * var,
                        const struct enum_list *enums,
                        const char *hint, const char *units)
{
    char           *enum_string = NULL;

    if ((var->type != ASN_UINTEGER) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be UInteger32): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    for (; enums; enums = enums->next) {
        if (enums->value == *var->val.integer) {
            enum_string = enums->label;
            break;
        }
    }

    if (enum_string == NULL ||
        netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM)) {
        if (hint) {
            if (!(sprint_realloc_hinted_integer(buf, buf_len, out_len,
                                                allow_realloc,
                                                *var->val.integer, 'u',
                                                hint, units))) {
                return 0;
            }
        } else {
            char            str[16];
            sprintf(str, "%lu", *var->val.integer);
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) str)) {
                return 0;
            }
        }
    } else if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) enum_string)) {
            return 0;
        }
    } else {
        char            str[16];
        sprintf(str, "(%lu)", *var->val.integer);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc,
             (const u_char *) enum_string)) {
            return 0;
        }
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) str)) {
            return 0;
        }
    }

    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints a gauge value into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_gauge(u_char ** buf, size_t * buf_len, size_t * out_len,
                     int allow_realloc,
                     const netsnmp_variable_list * var,
                     const struct enum_list *enums,
                     const char *hint, const char *units)
{
    char            tmp[32];

    if ((var->type != ASN_GAUGE) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] =
            "Wrong Type (should be Gauge32 or Unsigned32): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "Gauge32: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }
    if (hint) {
        if (!sprint_realloc_hinted_integer(buf, buf_len, out_len,
                                           allow_realloc,
                                           *var->val.integer, 'u', hint,
                                           units)) {
            return 0;
        }
    } else {
        sprintf(tmp, "%lu", *var->val.integer);
        if (!snmp_strcat
            (buf, buf_len, out_len, allow_realloc, (const u_char *) tmp)) {
            return 0;
        }
    }
    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints a counter value into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_counter(u_char ** buf, size_t * buf_len, size_t * out_len,
                       int allow_realloc,
                       const netsnmp_variable_list * var,
                       const struct enum_list *enums,
                       const char *hint, const char *units)
{
    char            tmp[32];

    if ((var->type != ASN_COUNTER) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be Counter32): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "Counter32: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }
    sprintf(tmp, "%lu", *var->val.integer);
    if (!snmp_strcat
        (buf, buf_len, out_len, allow_realloc, (const u_char *) tmp)) {
        return 0;
    }
    if (units) {
        return (snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " ")
                && snmp_strcat(buf, buf_len, out_len, allow_realloc,
                               (const u_char *) units));
    }
    return 1;
}


/**
 * Prints a network address into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_networkaddress(u_char ** buf, size_t * buf_len,
                              size_t * out_len, int allow_realloc,
                              const netsnmp_variable_list * var,
                              const struct enum_list *enums, const char *hint,
                              const char *units)
{
    size_t          i;

    if ((var->type != ASN_IPADDRESS) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be NetworkAddress): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "Network Address: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }

    while ((*out_len + (var->val_len * 3) + 2) >= *buf_len) {
        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
    }

    for (i = 0; i < var->val_len; i++) {
        sprintf((char *) (*buf + *out_len), "%02X", var->val.string[i]);
        *out_len += 2;
        if (i < var->val_len - 1) {
            *(*buf + *out_len) = ':';
            (*out_len)++;
        }
    }
    return 1;
}


/**
 * Prints an ip-address into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_ipaddress(u_char ** buf, size_t * buf_len, size_t * out_len,
                         int allow_realloc,
                         const netsnmp_variable_list * var,
                         const struct enum_list *enums,
                         const char *hint, const char *units)
{
    u_char         *ip = var->val.string;

    if ((var->type != ASN_IPADDRESS) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be IpAddress): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "IpAddress: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }
    while ((*out_len + 17) >= *buf_len) {
        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
    }
    sprintf((char *) (*buf + *out_len), "%d.%d.%d.%d", ip[0], ip[1], ip[2],
            ip[3]);
    *out_len += strlen((char *) (*buf + *out_len));
    return 1;
}


/**
 * Prints a null value into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_null(u_char ** buf, size_t * buf_len, size_t * out_len,
                    int allow_realloc,
                    const netsnmp_variable_list * var,
                    const struct enum_list *enums,
                    const char *hint, const char *units)
{
    if ((var->type != ASN_NULL) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be NULL): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    } else {
        u_char          str[] = "NULL";
        return snmp_strcat(buf, buf_len, out_len, allow_realloc, str);
    }
}


/**
 * Prints a bit string into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_bitstring(u_char ** buf, size_t * buf_len, size_t * out_len,
                         int allow_realloc,
                         const netsnmp_variable_list * var,
                         const struct enum_list *enums,
                         const char *hint, const char *units)
{
    int             len, bit;
    u_char         *cp;
    char           *enum_string;

    if ((var->type != ASN_BIT_STR && var->type != ASN_OCTET_STR) &&
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be BITS): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "\"";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    } else {
        u_char          str[] = "BITS: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }
    if (!sprint_realloc_hexstring(buf, buf_len, out_len, allow_realloc,
                                  var->val.bitstring, var->val_len)) {
        return 0;
    }

    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "\"";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    } else {
        cp = var->val.bitstring;
        for (len = 0; len < (int) var->val_len; len++) {
            for (bit = 0; bit < 8; bit++) {
                if (*cp & (0x80 >> bit)) {
                    enum_string = NULL;
                    for (; enums; enums = enums->next) {
                        if (enums->value == (len * 8) + bit) {
                            enum_string = enums->label;
                            break;
                        }
                    }
                    if (enum_string == NULL ||
                        netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                       NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM)) {
                        char            str[16];
                        sprintf(str, "%d ", (len * 8) + bit);
                        if (!snmp_strcat
                            (buf, buf_len, out_len, allow_realloc,
                             (const u_char *) str)) {
                            return 0;
                        }
                    } else {
                        char            str[16];
                        sprintf(str, "(%d) ", (len * 8) + bit);
                        if (!snmp_strcat
                            (buf, buf_len, out_len, allow_realloc,
                             (const u_char *) enum_string)) {
                            return 0;
                        }
                        if (!snmp_strcat
                            (buf, buf_len, out_len, allow_realloc,
                             (const u_char *) str)) {
                            return 0;
                        }
                    }
                }
            }
            cp++;
        }
    }
    return 1;
}

int
sprint_realloc_nsapaddress(u_char ** buf, size_t * buf_len,
                           size_t * out_len, int allow_realloc,
                           const netsnmp_variable_list * var,
                           const struct enum_list *enums, const char *hint,
                           const char *units)
{
    if ((var->type != ASN_NSAP) && 
        (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT))) {
        u_char          str[] = "Wrong Type (should be NsapAddress): ";
        if (snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, var, NULL, NULL,
                                          NULL);
        } else {
            return 0;
        }
    }

    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
        u_char          str[] = "NsapAddress: ";
        if (!snmp_strcat(buf, buf_len, out_len, allow_realloc, str)) {
            return 0;
        }
    }

    return sprint_realloc_hexstring(buf, buf_len, out_len, allow_realloc,
                                    var->val.string, var->val_len);
}


/**
 * Fallback routine for a bad type, prints "Variable has bad type" into a buffer.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_badtype(u_char ** buf, size_t * buf_len, size_t * out_len,
                       int allow_realloc,
                       const netsnmp_variable_list * var,
                       const struct enum_list *enums,
                       const char *hint, const char *units)
{
    u_char          str[] = "Variable has bad type";

    return snmp_strcat(buf, buf_len, out_len, allow_realloc, str);
}



/**
 * Universal print routine, prints a variable into a buffer according to the variable 
 * type.
 *
 * If allow_realloc is true the buffer will be (re)allocated to fit in the 
 * needed size. (Note: *buf may change due to this.)
 * 
 * @param buf      Address of the buffer to print to.
 * @param buf_len  Address to an integer containing the size of buf.
 * @param out_len  Incremented by the number of characters printed.
 * @param allow_realloc if not zero reallocate the buffer to fit the 
 *                      needed size.
 * @param var      The variable to encode.
 * @param enums    The enumeration ff this variable is enumerated. may be NULL.
 * @param hint     Contents of the DISPLAY-HINT clause of the MIB.
 *                 See RFC 1903 Section 3.1 for details. may be NULL.
 * @param units    Contents of the UNITS clause of the MIB. may be NULL.
 * 
 * @return 1 on success, or 0 on failure (out of memory, or buffer to
 *         small when not allowed to realloc.)
 */
int
sprint_realloc_by_type(u_char ** buf, size_t * buf_len, size_t * out_len,
                       int allow_realloc,
                       const netsnmp_variable_list * var,
                       const struct enum_list *enums,
                       const char *hint, const char *units)
{
    DEBUGMSGTL(("output", "sprint_by_type, type %d\n", var->type));

    switch (var->type) {
    case ASN_INTEGER:
        return sprint_realloc_integer(buf, buf_len, out_len, allow_realloc,
                                      var, enums, hint, units);
    case ASN_OCTET_STR:
        return sprint_realloc_octet_string(buf, buf_len, out_len,
                                           allow_realloc, var, enums, hint,
                                           units);
    case ASN_BIT_STR:
        return sprint_realloc_bitstring(buf, buf_len, out_len,
                                        allow_realloc, var, enums, hint,
                                        units);
    case ASN_OPAQUE:
        return sprint_realloc_opaque(buf, buf_len, out_len, allow_realloc,
                                     var, enums, hint, units);
    case ASN_OBJECT_ID:
        return sprint_realloc_object_identifier(buf, buf_len, out_len,
                                                allow_realloc, var, enums,
                                                hint, units);
    case ASN_TIMETICKS:
        return sprint_realloc_timeticks(buf, buf_len, out_len,
                                        allow_realloc, var, enums, hint,
                                        units);
    case ASN_GAUGE:
        return sprint_realloc_gauge(buf, buf_len, out_len, allow_realloc,
                                    var, enums, hint, units);
    case ASN_COUNTER:
        return sprint_realloc_counter(buf, buf_len, out_len, allow_realloc,
                                      var, enums, hint, units);
    case ASN_IPADDRESS:
        return sprint_realloc_ipaddress(buf, buf_len, out_len,
                                        allow_realloc, var, enums, hint,
                                        units);
    case ASN_NULL:
        return sprint_realloc_null(buf, buf_len, out_len, allow_realloc,
                                   var, enums, hint, units);
    case ASN_UINTEGER:
        return sprint_realloc_uinteger(buf, buf_len, out_len,
                                       allow_realloc, var, enums, hint,
                                       units);
    case ASN_COUNTER64:
#ifdef OPAQUE_SPECIAL_TYPES
    case ASN_OPAQUE_U64:
    case ASN_OPAQUE_I64:
    case ASN_OPAQUE_COUNTER64:
#endif                          /* OPAQUE_SPECIAL_TYPES */
        return sprint_realloc_counter64(buf, buf_len, out_len,
                                        allow_realloc, var, enums, hint,
                                        units);
#ifdef OPAQUE_SPECIAL_TYPES
    case ASN_OPAQUE_FLOAT:
        return sprint_realloc_float(buf, buf_len, out_len, allow_realloc,
                                    var, enums, hint, units);
    case ASN_OPAQUE_DOUBLE:
        return sprint_realloc_double(buf, buf_len, out_len, allow_realloc,
                                     var, enums, hint, units);
#endif                          /* OPAQUE_SPECIAL_TYPES */
    default:
        DEBUGMSGTL(("sprint_by_type", "bad type: %d\n", var->type));
        return sprint_realloc_badtype(buf, buf_len, out_len, allow_realloc,
                                      var, enums, hint, units);
    }
}


/**
 * Retrieves the tree head.
 *
 * @return the tree head.
 */
struct tree    *
get_tree_head(void)
{
    return (tree_head);
}

static char    *confmibdir = NULL;
static char    *confmibs = NULL;

static void
handle_mibdirs_conf(const char *token, char *line)
{
    char           *ctmp;

    if (confmibdir) {
        ctmp = (char *) malloc(strlen(confmibdir) + strlen(line) + 1);
        if (!ctmp) {
            DEBUGMSGTL(("read_config:initmib", "mibdir conf malloc failed"));
            return;
        }
        if (*line == '+')
            line++;
        sprintf(ctmp, "%s%c%s", confmibdir, ENV_SEPARATOR_CHAR, line);
        free(confmibdir);
        confmibdir = ctmp;
    } else {
        confmibdir = strdup(line);
    }
    DEBUGMSGTL(("read_config:initmib", "using mibdirs: %s\n", confmibdir));
}

static void
handle_mibs_conf(const char *token, char *line)
{
    char           *ctmp;

    if (confmibs) {
        ctmp = (char *) malloc(strlen(confmibs) + strlen(line) + 1);
        if (!ctmp) {
            DEBUGMSGTL(("read_config:initmib", "mibs conf malloc failed"));
            return;
        }
        if (*line == '+')
            line++;
        sprintf(ctmp, "%s%c%s", confmibs, ENV_SEPARATOR_CHAR, line);
        free(confmibs);
        confmibs = ctmp;
    } else {
        confmibs = strdup(line);
    }
    DEBUGMSGTL(("read_config:initmib", "using mibs: %s\n", confmibs));
}


static void
handle_mibfile_conf(const char *token, char *line)
{
    DEBUGMSGTL(("read_config:initmib", "reading mibfile: %s\n", line));
    read_mib(line);
}

static void
handle_print_numeric(const char *token, char *line)
{
    const char *value;

    value = strtok(line, " \t\n");
    if ((strcasecmp(value, "yes")  == 0) || 
	(strcasecmp(value, "true") == 0) ||
	(*value == '1')) {

        netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                                  NETSNMP_OID_OUTPUT_NUMERIC);
    }
}

char           *
snmp_out_toggle_options(char *options)
{
    while (*options) {
        switch (*options++) {
        case 'a':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_STRING_OUTPUT_FORMAT,
                                                      NETSNMP_STRING_OUTPUT_ASCII);
            break;
        case 'b':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS);
            break;
        case 'e':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM);
            break;
        case 'E':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_ESCAPE_QUOTES);
            break;
        case 'f':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                                      NETSNMP_OID_OUTPUT_FULL);
            break;
        case 'n':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                                      NETSNMP_OID_OUTPUT_NUMERIC);
            break;
        case 'q':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT);
            break;
        case 'Q':
            netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT, 1);
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT);
            break;
        case 's':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                                      NETSNMP_OID_OUTPUT_SUFFIX);
            break;
        case 'S':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                                      NETSNMP_OID_OUTPUT_MODULE);
            break;
        case 't':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NUMERIC_TIMETICKS);
            break;
        case 'T':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_HEX_TEXT);
            break;
        case 'u':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT,
                                                      NETSNMP_OID_OUTPUT_UCD);
            break;
        case 'U':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_PRINT_UNITS);
            break;
        case 'v':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_BARE_VALUE);
            break;
        case 'x':
            netsnmp_ds_set_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_STRING_OUTPUT_FORMAT,
                                                      NETSNMP_STRING_OUTPUT_HEX);
            break;
        case 'X':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_EXTENDED_INDEX);
            break;
        default:
            return options - 1;
        }
    }
    return NULL;
}

void
snmp_out_toggle_options_usage(const char *lead, FILE * outf)
{
    fprintf(outf, "%sa:  print all strings in ascii format\n", lead);
    fprintf(outf, "%sb:  do not break OID indexes down\n", lead);
    fprintf(outf, "%se:  print enums numerically\n", lead);
    fprintf(outf, "%sE:  escape quotes in string indices\n", lead);
    fprintf(outf, "%sf:  print full OIDs on output\n", lead);
    fprintf(outf, "%sn:  print OIDs numerically\n", lead);
    fprintf(outf, "%sq:  quick print for easier parsing\n", lead);
    fprintf(outf, "%sQ:  quick print with equal-signs\n", lead);    /* @@JDW */
    fprintf(outf, "%ss:  print only last symbolic element of OID\n", lead);
    fprintf(outf, "%sS:  print MIB module-id plus last element\n", lead);
    fprintf(outf, "%st:  print timeticks unparsed as numeric integers\n",
            lead);
    fprintf(outf,
            "%sT:  print human-readable text along with hex strings\n",
            lead);
    fprintf(outf, "%su:  print OIDs using UCD-style prefix suppression\n",
            lead);
    fprintf(outf, "%sU:  don't print units\n", lead);
    fprintf(outf, "%sv:  print values only (not OID = value)\n", lead);
    fprintf(outf, "%sx:  print all strings in hex format\n", lead);
    fprintf(outf, "%sX:  extended index format\n", lead);
}

char           *
snmp_in_toggle_options(char *options)
{
    while (*options) {
        switch (*options++) {
        case 'b':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_REGEX_ACCESS);
            break;
        case 'R':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_RANDOM_ACCESS);
            break;
        case 'r':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_CHECK_RANGE);
            break;
        case 'h':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NO_DISPLAY_HINT);
            break;
        case 'u':
            netsnmp_ds_toggle_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_READ_UCD_STYLE_OID);
            break;
        default:
            return options - 1;
        }
    }
    return NULL;
}


/**
 * Prints out a help usage for the in* toggle options.
 *
 * @param lead      The lead to print for every line.
 * @param outf      The file descriptor to write to.
 * 
 */
void
snmp_in_toggle_options_usage(const char *lead, FILE * outf)
{
    fprintf(outf, "%sb:  do best/regex matching to find a MIB node\n",
            lead);
    fprintf(outf, "%sr:  do not check values for range/type legality\n",
            lead);
    fprintf(outf, "%sR:  do random access to OID labels\n", lead);
    fprintf(outf, "%sh:  don't apply DISPLAY-HINTs\n", lead);
    fprintf(outf,
            "%su:  top-level OIDs must have '.' prefix (UCD-style)\n",
            lead);
}

/***
 *
 */ 
void
register_mib_handlers(void)
{
    register_prenetsnmp_mib_handler("snmp", "mibdirs",
                                    handle_mibdirs_conf, NULL,
                                    "[mib-dirs|+mib-dirs]");
    register_prenetsnmp_mib_handler("snmp", "mibs",
                                    handle_mibs_conf, NULL,
                                    "[mib-tokens|+mib-tokens]");
    register_config_handler("snmp", "mibfile",
                            handle_mibfile_conf, NULL, "mibfile-to-read");

    /*
     * register the snmp.conf configuration handlers for default
     * parsing behaviour 
     */

    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "showMibErrors",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_ERRORS);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "strictCommentTerm",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_COMMENT_TERM);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "mibAllowUnderline",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_PARSE_LABEL);
    netsnmp_ds_register_premib(ASN_INTEGER, "snmp", "mibWarningLevel",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_WARNINGS);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "mibReplaceWithLatest",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIB_REPLACE);

    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "printNumericEnums",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM);
    register_prenetsnmp_mib_handler("snmp", "printNumericOids",
                       handle_print_numeric, NULL, "(1|yes|true|0|no|false)");
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "escapeQuotes",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_ESCAPE_QUOTES);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "dontBreakdownOids",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "quickPrinting",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "numericTimeticks",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NUMERIC_TIMETICKS);
    netsnmp_ds_register_premib(ASN_INTEGER, "snmp", "oidOutputFormat",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);
    netsnmp_ds_register_premib(ASN_INTEGER, "snmp", "suffixPrinting",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "extendedIndex",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_EXTENDED_INDEX);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "printHexText",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_HEX_TEXT);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "printValueOnly",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_BARE_VALUE);
    netsnmp_ds_register_premib(ASN_BOOLEAN, "snmp", "dontPrintUnits",
                       NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_PRINT_UNITS);

}

/*
 * function : netsnmp_set_mib_directory
 *            - This function sets the string of the directories
 *              from which the MIB modules will be searched or
 *              loaded.
 * arguments: const char *dir, which are the directories
 *              from which the MIB modules will be searched or
 *              loaded.
 * returns  : -
 */
void
netsnmp_set_mib_directory(const char *dir)
{
    const char *newdir;
    char *olddir, *tmpdir = NULL;

    DEBUGTRACE;
    if (NULL == dir) {
        return;
    }
    
    olddir = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID,
				   NETSNMP_DS_LIB_MIBDIRS);
    if (olddir) {
        if (*dir == '+') {
            /** New dir starts with '+', thus we add it. */
            tmpdir = malloc(strlen(dir) + strlen(olddir) + 1);
            if (!tmpdir) {
                DEBUGMSGTL(("read_config:initmib", "set mibdir malloc failed"));
                return;
            }
            sprintf(tmpdir, "%s%c%s", ++dir, ENV_SEPARATOR_CHAR, olddir);
            newdir = tmpdir;
        } else {
            newdir = dir;
        }
    } else {
        /** If dir starts with '+' skip '+' it. */
        newdir = ((*dir == '+') ? ++dir : dir);
    }
    netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIBDIRS,
                          newdir);

    /** set_string calls strdup, so if we allocated memory, free it */
    if (tmpdir == newdir) {
        free(tmpdir);
    }
}

/*
 * function : netsnmp_get_mib_directory
 *            - This function returns a string of the directories
 *              from which the MIB modules will be searched or
 *              loaded.
 *              If the value still does not exists, it will be made
 *              from the evironment variable 'MIBDIRS' and/or the
 *              default.
 * arguments: -
 * returns  : char * of the directories in which the MIB modules
 *            will be searched/loaded.
 */

char *
netsnmp_get_mib_directory(void)
{
    char *dir;

    DEBUGTRACE;
    dir = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIBDIRS);
    if (dir == NULL) {
        DEBUGMSGTL(("get_mib_directory", "no mib directories set\n"));

        /** Check if the environment variable is set */
        dir = getenv("MIBDIRS");
        if (dir == NULL) {
            DEBUGMSGTL(("get_mib_directory", "no mib directories set by environment\n"));
            /** Not set use hard coded path */
            if (confmibdir == NULL) {
                DEBUGMSGTL(("get_mib_directory", "no mib directories set by config\n"));
                netsnmp_set_mib_directory(DEFAULT_MIBDIRS);
            }
            else if (*confmibdir == '+') {
                DEBUGMSGTL(("get_mib_directory", "mib directories set by config (but added)\n"));
                netsnmp_set_mib_directory(DEFAULT_MIBDIRS);
                netsnmp_set_mib_directory(confmibdir);
            }
            else {
                DEBUGMSGTL(("get_mib_directory", "mib directories set by config\n"));
                netsnmp_set_mib_directory(confmibdir);
            }
        } else if (*dir == '+') {
            DEBUGMSGTL(("get_mib_directory", "mib directories set by environment (but added)\n"));
            netsnmp_set_mib_directory(DEFAULT_MIBDIRS);
            netsnmp_set_mib_directory(dir);
        } else {
            DEBUGMSGTL(("get_mib_directory", "mib directories set by environment\n"));
            netsnmp_set_mib_directory(dir);
        }
        dir = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_MIBDIRS);
    }
    DEBUGMSGTL(("get_mib_directory", "mib directories set '%s'\n", dir));
    return(dir);
}

/*
 * function : netsnmp_fixup_mib_directory
 * arguments: -
 * returns  : -
 */
void
netsnmp_fixup_mib_directory(void)
{
    char *homepath = getenv("HOME");
    char *mibpath = netsnmp_get_mib_directory();
    char *oldmibpath = NULL;
    char *ptr_home;
    char *new_mibpath;

    DEBUGTRACE;
    if (homepath && mibpath) {
        DEBUGMSGTL(("fixup_mib_directory", "mib directories '%s'\n", mibpath));
        while ((ptr_home = strstr(mibpath, "$HOME"))) {
            new_mibpath = (char *)malloc(strlen(mibpath) - strlen("$HOME") +
					 strlen(homepath)+1);
            if (new_mibpath) {
                *ptr_home = 0; /* null out the spot where we stop copying */
                sprintf(new_mibpath, "%s%s%s", mibpath, homepath,
			ptr_home + strlen("$HOME"));
                /** swap in the new value and repeat */
                mibpath = new_mibpath;
		if (oldmibpath != NULL) {
		    free(oldmibpath);
		}
		oldmibpath = new_mibpath;
            } else {
                break;
            }
        }

        netsnmp_set_mib_directory(mibpath);
	
	/*  The above copies the mibpath for us, so...  */

	if (oldmibpath != NULL) {
	    free(oldmibpath);
	}
    }

}

/**
 * Initialises the mib reader.
 *
 * Reads in all settings from the environment.
 */
void
init_mib(void)
{
    const char     *prefix;
    char           *env_var, *entry;
    PrefixListPtr   pp = &mib_prefixes[0];

    if (Mib)
        return;
    init_mib_internals();

    /*
     * Initialise the MIB directory/ies 
     */
    netsnmp_fixup_mib_directory();
    env_var = strdup(netsnmp_get_mib_directory());

    DEBUGMSGTL(("init_mib",
                "Seen MIBDIRS: Looking in '%s' for mib dirs ...\n",
                env_var));

    entry = strtok(env_var, ENV_SEPARATOR);
    while (entry) {
        add_mibdir(entry);
        entry = strtok(NULL, ENV_SEPARATOR);
    }
    free(env_var);

    init_mib_internals();

    /*
     * Read in any modules or mibs requested 
     */

    env_var = getenv("MIBS");
    if (env_var == NULL) {
        if (confmibs != NULL)
            env_var = strdup(confmibs);
        else
            env_var = strdup(DEFAULT_MIBS);
    } else {
        env_var = strdup(env_var);
    }
    if (env_var && *env_var == '+') {
        entry =
            (char *) malloc(strlen(DEFAULT_MIBS) + strlen(env_var) + 2);
        if (!entry) {
            DEBUGMSGTL(("init_mib", "env mibs malloc failed"));
            return;
        } else
            sprintf(entry, "%s%c%s", DEFAULT_MIBS, ENV_SEPARATOR_CHAR,
                env_var + 1);
        free(env_var);
        env_var = entry;
    }

    DEBUGMSGTL(("init_mib",
                "Seen MIBS: Looking in '%s' for mib files ...\n",
                env_var));
    entry = strtok(env_var, ENV_SEPARATOR);
    while (entry) {
        if (strcasecmp(entry, DEBUG_ALWAYS_TOKEN) == 0) {
            read_all_mibs();
        } else if (strstr(entry, "/") != 0) {
            read_mib(entry);
        } else {
            read_module(entry);
        }
        entry = strtok(NULL, ENV_SEPARATOR);
    }
    adopt_orphans();
    free(env_var);

    env_var = getenv("MIBFILES");
    if (env_var != NULL) {
        if (*env_var == '+') {
#ifdef DEFAULT_MIBFILES
            entry =
                (char *) malloc(strlen(DEFAULT_MIBFILES) +
                                strlen(env_var) + 2);
            if (!entry) {
                DEBUGMSGTL(("init_mib", "env mibfiles malloc failed"));
            } else
                sprintf(entry, "%s%c%s", DEFAULT_MIBFILES, ENV_SEPARATOR_CHAR,
                    env_var + 1);
            free(env_var);
            env_var = entry;
#else
            env_var = strdup(env_var + 1);
#endif
        } else {
            env_var = strdup(env_var);
        }
    } else {
#ifdef DEFAULT_MIBFILES
        env_var = strdup(DEFAULT_MIBFILES);
#endif
    }

    if (env_var != 0) {
        DEBUGMSGTL(("init_mib",
                    "Seen MIBFILES: Looking in '%s' for mib files ...\n",
                    env_var));
        entry = strtok(env_var, ENV_SEPARATOR);
        while (entry) {
            read_mib(entry);
            entry = strtok(NULL, ENV_SEPARATOR);
        }
        free(env_var);
    }

    prefix = getenv("PREFIX");

    if (!prefix)
        prefix = Standard_Prefix;

    Prefix = (char *) malloc(strlen(prefix) + 2);
    if (!Prefix)
        DEBUGMSGTL(("init_mib", "Prefix malloc failed"));
    else
        strcpy(Prefix, prefix);

    DEBUGMSGTL(("init_mib",
                "Seen PREFIX: Looking in '%s' for prefix ...\n", Prefix));

    /*
     * remove trailing dot 
     */
    if (Prefix) {
        env_var = &Prefix[strlen(Prefix) - 1];
        if (*env_var == '.')
            *env_var = '\0';
    }

    pp->str = Prefix;           /* fixup first mib_prefix entry */
    /*
     * now that the list of prefixes is built, save each string length. 
     */
    while (pp->str) {
        pp->len = strlen(pp->str);
        pp++;
    }

    Mib = tree_head;            /* Backwards compatibility */
    tree_top = (struct tree *) calloc(1, sizeof(struct tree));
    /*
     * XX error check ? 
     */
    if (tree_top) {
        tree_top->label = strdup("(top)");
        tree_top->child_list = tree_head;
    }
}

/**
 * Unloads all mibs.
 */
void
shutdown_mib(void)
{
    unload_all_mibs();
    if (tree_top) {
        if (tree_top->label)
            free(tree_top->label);
        free(tree_top);
        tree_top = NULL;
    }
    tree_head = NULL;
    Mib = NULL;
    if (Prefix != NULL && Prefix != &Standard_Prefix[0])
        free(Prefix);
    if (Prefix)
        Prefix = NULL;
}

/**
 * Prints the MIBs to the file fp.
 *
 * @param fp   The file descriptor to print to.
 */
void
print_mib(FILE * fp)
{
    print_subtree(fp, tree_head, 0);
}

void
print_ascii_dump(FILE * fp)
{
    fprintf(fp, "dump DEFINITIONS ::= BEGIN\n");
    print_ascii_dump_tree(fp, tree_head, 0);
    fprintf(fp, "END\n");
}


/**
 * Set's the printing function printomat in a subtree according
 * it's type
 *
 * @param subtree    The subtree to set.
 */
void
set_function(struct tree *subtree)
{
    subtree->printer = NULL;
    switch (subtree->type) {
    case TYPE_OBJID:
        subtree->printomat = sprint_realloc_object_identifier;
        break;
    case TYPE_OCTETSTR:
        subtree->printomat = sprint_realloc_octet_string;
        break;
    case TYPE_INTEGER:
        subtree->printomat = sprint_realloc_integer;
        break;
    case TYPE_INTEGER32:
        subtree->printomat = sprint_realloc_integer;
        break;
    case TYPE_NETADDR:
        subtree->printomat = sprint_realloc_networkaddress;
        break;
    case TYPE_IPADDR:
        subtree->printomat = sprint_realloc_ipaddress;
        break;
    case TYPE_COUNTER:
        subtree->printomat = sprint_realloc_counter;
        break;
    case TYPE_GAUGE:
        subtree->printomat = sprint_realloc_gauge;
        break;
    case TYPE_TIMETICKS:
        subtree->printomat = sprint_realloc_timeticks;
        break;
    case TYPE_OPAQUE:
        subtree->printomat = sprint_realloc_opaque;
        break;
    case TYPE_NULL:
        subtree->printomat = sprint_realloc_null;
        break;
    case TYPE_BITSTRING:
        subtree->printomat = sprint_realloc_bitstring;
        break;
    case TYPE_NSAPADDRESS:
        subtree->printomat = sprint_realloc_nsapaddress;
        break;
    case TYPE_COUNTER64:
        subtree->printomat = sprint_realloc_counter64;
        break;
    case TYPE_UINTEGER:
        subtree->printomat = sprint_realloc_uinteger;
        break;
    case TYPE_UNSIGNED32:
        subtree->printomat = sprint_realloc_gauge;
        break;
    case TYPE_OTHER:
    default:
        subtree->printomat = sprint_realloc_by_type;
        break;
    }
}

/**
 * Reads an object identifier from an input string into internal OID form.
 * 
 * When called, out_len must hold the maximum length of the output array.
 *
 * @param input     the input string.
 * @param output    the oid wirte.
 * @param out_len   number of subid's in output.
 * 
 * @return 1 if successful.
 * 
 * If an error occurs, this function returns 0 and MAY set snmp_errno.
 * snmp_errno is NOT set if SET_SNMP_ERROR evaluates to nothing.
 * This can make multi-threaded use a tiny bit more robust.
 */
int
read_objid(const char *input, oid * output, size_t * out_len)
{                               /* number of subid's in "output" */
    struct tree    *root = tree_top;
    char            buf[SPRINT_MAX_LEN];
    int             ret, max_out_len;
    char           *name, ch;
    const char     *cp;

    cp = input;
    while ((ch = *cp))
        if (('0' <= ch && ch <= '9')
            || ('a' <= ch && ch <= 'z')
            || ('A' <= ch && ch <= 'Z')
            || ch == '-')
            cp++;
        else
            break;
    if (ch == ':')
        return get_node(input, output, out_len);

    if (*input == '.')
        input++;
    else if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_READ_UCD_STYLE_OID)) {
        /*
         * get past leading '.', append '.' to Prefix. 
         */
        if (*Prefix == '.')
            strncpy(buf, Prefix + 1, sizeof(buf)-1);
        else
            strncpy(buf, Prefix, sizeof(buf)-1);
        buf[ sizeof(buf)-1 ] = 0;
        strcat(buf, ".");
        buf[ sizeof(buf)-1 ] = 0;
        strncat(buf, input, sizeof(buf)-strlen(buf));
        buf[ sizeof(buf)-1 ] = 0;
        input = buf;
    }

    if (root == NULL) {
        SET_SNMP_ERROR(SNMPERR_NOMIB);
        *out_len = 0;
        return 0;
    }
    name = strdup(input);
    max_out_len = *out_len;
    *out_len = 0;
    if ((ret =
         _add_strings_to_oid(root, name, output, out_len,
                             max_out_len)) <= 0) {
        if (ret == 0)
            ret = SNMPERR_UNKNOWN_OBJID;
        SET_SNMP_ERROR(ret);
        free(name);
        return 0;
    }
    free(name);

    return 1;
}

/**
 * 
 */
struct tree    *
netsnmp_sprint_realloc_objid_tree(u_char ** buf, size_t * buf_len,
                                  size_t * out_len, int allow_realloc,
                                  int *buf_overflow,
                                  const oid * objid, size_t objidlen)
{
    u_char         *tbuf = NULL, *cp = NULL;
    size_t          tbuf_len = 256, tout_len = 0;
    struct tree    *subtree = tree_head;
    size_t          midpoint_offset = 0;
    int             tbuf_overflow = 0;
    int             output_format;

    if ((tbuf = (u_char *) calloc(tbuf_len, 1)) == NULL) {
        tbuf_overflow = 1;
    } else {
        *tbuf = '.';
        tout_len = 1;
    }

    subtree = _get_realloc_symbol(objid, objidlen, subtree,
                                  &tbuf, &tbuf_len, &tout_len,
                                  allow_realloc, &tbuf_overflow, NULL,
                                  &midpoint_offset);

    if (tbuf_overflow) {
        if (!*buf_overflow) {
            snmp_strcat(buf, buf_len, out_len, allow_realloc, tbuf);
            *buf_overflow = 1;
        }
        free(tbuf);
        return subtree;
    }

    output_format = netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);
    if (0 == output_format) {
        output_format = NETSNMP_OID_OUTPUT_MODULE;
    }
    switch (output_format) {
    case NETSNMP_OID_OUTPUT_FULL:
    case NETSNMP_OID_OUTPUT_NUMERIC:
        cp = tbuf;
        break;

    case NETSNMP_OID_OUTPUT_SUFFIX:
    case NETSNMP_OID_OUTPUT_MODULE:
        for (cp = tbuf; *cp; cp++);

        if (midpoint_offset != 0) {
            cp = tbuf + midpoint_offset - 2;    /*  beyond the '.'  */
        } else {
            while (cp >= tbuf) {
                if (isalpha(*cp)) {
                    break;
                }
                cp--;
            }
        }

        while (cp >= tbuf) {
            if (*cp == '.') {
                break;
            }
            cp--;
        }

        cp++;

        if ((NETSNMP_OID_OUTPUT_MODULE == output_format)
            && cp > tbuf) {
            char            modbuf[256] = { 0 }, *mod =
                module_name(subtree->modid, modbuf);

            /*
             * Don't add the module ID if it's just numeric (i.e. we couldn't look
             * it up properly.  
             */

            if (!*buf_overflow && modbuf[0] != '#') {
                if (!snmp_strcat
                    (buf, buf_len, out_len, allow_realloc,
                     (const u_char *) mod)
                    || !snmp_strcat(buf, buf_len, out_len, allow_realloc,
                                    (const u_char *) "::")) {
                    *buf_overflow = 1;
                }
            }
        }
        break;

    case NETSNMP_OID_OUTPUT_UCD:
    {
        PrefixListPtr   pp = &mib_prefixes[0];
        size_t          ilen, tlen;
        const char     *testcp;

        cp = tbuf;
        tlen = strlen((char *) tbuf);

        while (pp->str) {
            ilen = pp->len;
            testcp = pp->str;

            if ((tlen > ilen) && memcmp(tbuf, testcp, ilen) == 0) {
                cp += (ilen + 1);
                break;
            }
            pp++;
        }
        break;
    }

    case NETSNMP_OID_OUTPUT_NONE:
    default:
        cp = NULL;
    }

    if (!*buf_overflow &&
        !snmp_strcat(buf, buf_len, out_len, allow_realloc, cp)) {
        *buf_overflow = 1;
    }
    free(tbuf);
    return subtree;
}

int
sprint_realloc_objid(u_char ** buf, size_t * buf_len,
                     size_t * out_len, int allow_realloc,
                     const oid * objid, size_t objidlen)
{
    int             buf_overflow = 0;

    netsnmp_sprint_realloc_objid_tree(buf, buf_len, out_len, allow_realloc,
                                      &buf_overflow, objid, objidlen);
    return !buf_overflow;
}

int
snprint_objid(char *buf, size_t buf_len,
              const oid * objid, size_t objidlen)
{
    size_t          out_len = 0;

    if (sprint_realloc_objid((u_char **) & buf, &buf_len, &out_len, 0,
                             objid, objidlen)) {
        return (int) out_len;
    } else {
        return -1;
    }
}

/**
 * Prints an oid to stdout.
 *
 * @param objid      The oid to print
 * @param objidlen   The length of oidid.
 */
void
print_objid(const oid * objid, size_t objidlen)
{                               /* number of subidentifiers */
    fprint_objid(stdout, objid, objidlen);
}


/**
 * Prints an oid to a file descriptor.
 *
 * @param f          The file descriptor to print to.
 * @param objid      The oid to print
 * @param objidlen   The length of oidid.
 */
void
fprint_objid(FILE * f, const oid * objid, size_t objidlen)
{                               /* number of subidentifiers */
    u_char         *buf = NULL;
    size_t          buf_len = 256, out_len = 0;
    int             buf_overflow = 0;

    if ((buf = (u_char *) calloc(buf_len, 1)) == NULL) {
        fprintf(f, "[TRUNCATED]\n");
        return;
    } else {
        netsnmp_sprint_realloc_objid_tree(&buf, &buf_len, &out_len, 1,
                                          &buf_overflow, objid, objidlen);
        if (buf_overflow) {
            fprintf(f, "%s [TRUNCATED]\n", buf);
        } else {
            fprintf(f, "%s\n", buf);
        }
    }

    free(buf);
}

int
sprint_realloc_variable(u_char ** buf, size_t * buf_len,
                        size_t * out_len, int allow_realloc,
                        const oid * objid, size_t objidlen,
                        const netsnmp_variable_list * variable)
{
    struct tree    *subtree = tree_head;
    int             buf_overflow = 0;

    subtree =
        netsnmp_sprint_realloc_objid_tree(buf, buf_len, out_len,
                                          allow_realloc, &buf_overflow,
                                          objid, objidlen);

    if (buf_overflow) {
        return 0;
    }
    if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_PRINT_BARE_VALUE)) {
        if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICKE_PRINT)) {
            if (!snmp_strcat
                (buf, buf_len, out_len, allow_realloc,
                 (const u_char *) " = ")) {
                return 0;
            }
        } else {
            if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT)) {
                if (!snmp_strcat
                    (buf, buf_len, out_len, allow_realloc,
                     (const u_char *) " ")) {
                    return 0;
                }
            } else {
                if (!snmp_strcat
                    (buf, buf_len, out_len, allow_realloc,
                     (const u_char *) " = ")) {
                    return 0;
                }
            }                   /* end if-else NETSNMP_DS_LIB_QUICK_PRINT */
        }                       /* end if-else NETSNMP_DS_LIB_QUICKE_PRINT */
    } else {
        *out_len = 0;
    }

    if (variable->type == SNMP_NOSUCHOBJECT) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *)
                           "No Such Object available on this agent at this OID");
    } else if (variable->type == SNMP_NOSUCHINSTANCE) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *)
                           "No Such Instance currently exists at this OID");
    } else if (variable->type == SNMP_ENDOFMIBVIEW) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *)
                           "No more variables left in this MIB View (It is past the end of the MIB tree)");
    } else if (subtree) {
        const char *units = NULL;
        if (!netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                    NETSNMP_DS_LIB_DONT_PRINT_UNITS)) {
            units = subtree->units;
        }
        if (subtree->printomat) {
            return (*subtree->printomat) (buf, buf_len, out_len,
                                          allow_realloc, variable,
                                          subtree->enums, subtree->hint,
                                          units);
        } else {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, variable,
                                          subtree->enums, subtree->hint,
                                          units);
        }
    } else {
        /*
         * Handle rare case where tree is empty.  
         */
        return sprint_realloc_by_type(buf, buf_len, out_len, allow_realloc,
                                      variable, 0, 0, 0);
    }
}

int
snprint_variable(char *buf, size_t buf_len,
                 const oid * objid, size_t objidlen,
                 const netsnmp_variable_list * variable)
{
    size_t          out_len = 0;

    if (sprint_realloc_variable((u_char **) & buf, &buf_len, &out_len, 0,
                                objid, objidlen, variable)) {
        return (int) out_len;
    } else {
        return -1;
    }
}

/**
 * Prints a variable to stdout.
 *
 * @param objid     The object id.
 * @param objidlen  The length of teh object id.
 * @param variable  The variable to print.
 */
void
print_variable(const oid * objid,
               size_t objidlen, const netsnmp_variable_list * variable)
{
    fprint_variable(stdout, objid, objidlen, variable);
}


/**
 * Prints a variable to a file descriptor.
 *
 * @param f         The file descriptor to print to.
 * @param objid     The object id.
 * @param objidlen  The length of teh object id.
 * @param variable  The variable to print.
 */
void
fprint_variable(FILE * f,
                const oid * objid,
                size_t objidlen, const netsnmp_variable_list * variable)
{
    u_char         *buf = NULL;
    size_t          buf_len = 256, out_len = 0;

    if ((buf = (u_char *) calloc(buf_len, 1)) == NULL) {
        fprintf(f, "[TRUNCATED]\n");
        return;
    } else {
        if (sprint_realloc_variable(&buf, &buf_len, &out_len, 1,
                                    objid, objidlen, variable)) {
            fprintf(f, "%s\n", buf);
        } else {
            fprintf(f, "%s [TRUNCATED]\n", buf);
        }
    }

    free(buf);
}

int
sprint_realloc_value(u_char ** buf, size_t * buf_len,
                     size_t * out_len, int allow_realloc,
                     const oid * objid, size_t objidlen,
                     const netsnmp_variable_list * variable)
{
    struct tree    *subtree = tree_head;

    if (variable->type == SNMP_NOSUCHOBJECT) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *)
                           "No Such Object available on this agent at this OID");
    } else if (variable->type == SNMP_NOSUCHINSTANCE) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *)
                           "No Such Instance currently exists at this OID");
    } else if (variable->type == SNMP_ENDOFMIBVIEW) {
        return snmp_strcat(buf, buf_len, out_len, allow_realloc,
                           (const u_char *)
                           "No more variables left in this MIB View (It is past the end of the MIB tree)");
    } else {
        const char *units = NULL;
        subtree = get_tree(objid, objidlen, subtree);
        if (subtree && !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID,
                                            NETSNMP_DS_LIB_DONT_PRINT_UNITS)) {
            units = subtree->units;
        }
        if (subtree && subtree->printomat) {
            return (*subtree->printomat) (buf, buf_len, out_len,
                                          allow_realloc, variable,
                                          subtree->enums, subtree->hint,
                                          units);
        } else {
            return sprint_realloc_by_type(buf, buf_len, out_len,
                                          allow_realloc, variable,
                                          subtree->enums, subtree->hint,
                                          units);
        }
    }
}

int
snprint_value(char *buf, size_t buf_len,
              const oid * objid, size_t objidlen,
              const netsnmp_variable_list * variable)
{
    size_t          out_len = 0;

    if (sprint_realloc_value((u_char **) & buf, &buf_len, &out_len, 0,
                             objid, objidlen, variable)) {
        return (int) out_len;
    } else {
        return -1;
    }
}

void
print_value(const oid * objid,
            size_t objidlen, const netsnmp_variable_list * variable)
{
    fprint_value(stdout, objid, objidlen, variable);
}

void
fprint_value(FILE * f,
             const oid * objid,
             size_t objidlen, const netsnmp_variable_list * variable)
{
    u_char         *buf = NULL;
    size_t          buf_len = 256, out_len = 0;

    if ((buf = (u_char *) calloc(buf_len, 1)) == NULL) {
        fprintf(f, "[TRUNCATED]\n");
        return;
    } else {
        if (sprint_realloc_value(&buf, &buf_len, &out_len, 1,
                                 objid, objidlen, variable)) {
            fprintf(f, "%s\n", buf);
        } else {
            fprintf(f, "%s [TRUNCATED]\n", buf);
        }
    }

    free(buf);
}


/**
 * Takes the value in VAR and turns it into an OID segment in var->name.
 *  
 * @param var    The variable.
 *
 * @return SNMPERR_SUCCESS or SNMPERR_GENERR 
 */
int
build_oid_segment(netsnmp_variable_list * var)
{
    int             i;

    if (var->name && var->name != var->name_loc)
        SNMP_FREE(var->name);
    switch (var->type) {
    case ASN_INTEGER:
    case ASN_COUNTER:
    case ASN_GAUGE:
    case ASN_TIMETICKS:
        var->name_length = 1;
        var->name = var->name_loc;
        var->name[0] = *(var->val.integer);
        break;

    case ASN_IPADDRESS:
        var->name_length = 4;
        var->name = var->name_loc;
        var->name[0] =
            (((unsigned int) *(var->val.integer)) & 0xff000000) >> 24;
        var->name[1] =
            (((unsigned int) *(var->val.integer)) & 0x00ff0000) >> 16;
        var->name[2] =
            (((unsigned int) *(var->val.integer)) & 0x0000ff00) >> 8;
        var->name[3] =
            (((unsigned int) *(var->val.integer)) & 0x000000ff);
        break;
        
    case ASN_PRIV_IMPLIED_OBJECT_ID:
        var->name_length = var->val_len / sizeof(oid);
        if (var->name_length > (sizeof(var->name_loc) / sizeof(oid)))
            var->name = (oid *) malloc(sizeof(oid) * (var->name_length));
        else
            var->name = var->name_loc;
        if (var->name == NULL)
            return SNMPERR_GENERR;

        for (i = 0; i < (int) var->name_length; i++)
            var->name[i] = var->val.objid[i];
        break;

    case ASN_OBJECT_ID:
        var->name_length = var->val_len / sizeof(oid) + 1;
        if (var->name_length > (sizeof(var->name_loc) / sizeof(oid)))
            var->name = (oid *) malloc(sizeof(oid) * (var->name_length));
        else
            var->name = var->name_loc;
        if (var->name == NULL)
            return SNMPERR_GENERR;

        var->name[0] = var->name_length - 1;
        for (i = 0; i < (int) var->name_length - 1; i++)
            var->name[i + 1] = var->val.objid[i];
        break;

    case ASN_PRIV_IMPLIED_OCTET_STR:
        var->name_length = var->val_len;
        if (var->name_length > (sizeof(var->name_loc) / sizeof(oid)))
            var->name = (oid *) malloc(sizeof(oid) * (var->name_length));
        else
            var->name = var->name_loc;
        if (var->name == NULL)
            return SNMPERR_GENERR;

        for (i = 0; i < (int) var->val_len; i++)
            var->name[i] = (oid) var->val.string[i];
        break;

    case ASN_OPAQUE:
    case ASN_OCTET_STR:
        var->name_length = var->val_len + 1;
        if (var->name_length > (sizeof(var->name_loc) / sizeof(oid)))
            var->name = (oid *) malloc(sizeof(oid) * (var->name_length));
        else
            var->name = var->name_loc;
        if (var->name == NULL)
            return SNMPERR_GENERR;

        var->name[0] = (oid) var->val_len;
        for (i = 0; i < (int) var->val_len; i++)
            var->name[i + 1] = (oid) var->val.string[i];
        break;

    default:
        DEBUGMSGTL(("build_oid_segment",
                    "invalid asn type: %d\n", var->type));
        return SNMPERR_GENERR;
    }

    if (var->name_length > MAX_OID_LEN) {
        DEBUGMSGTL(("build_oid_segment",
                    "Something terribly wrong, namelen = %d\n",
                    var->name_length));
        return SNMPERR_GENERR;
    }

    return SNMPERR_SUCCESS;
}


int
build_oid_noalloc(oid * in, size_t in_len, size_t * out_len,
                  oid * prefix, size_t prefix_len,
                  netsnmp_variable_list * indexes)
{
    netsnmp_variable_list *var;

    if (prefix) {
        if (in_len < prefix_len)
            return SNMPERR_GENERR;
        memcpy(in, prefix, prefix_len * sizeof(oid));
        *out_len = prefix_len;
    } else {
        *out_len = 0;
    }

    for (var = indexes; var != NULL; var = var->next_variable) {
        if (build_oid_segment(var) != SNMPERR_SUCCESS)
            return SNMPERR_GENERR;
        if (var->name_length + *out_len < in_len) {
            memcpy(&(in[*out_len]), var->name,
                   sizeof(oid) * var->name_length);
            *out_len += var->name_length;
        } else {
            return SNMPERR_GENERR;
        }
    }

    DEBUGMSGTL(("build_oid_noalloc", "generated: "));
    DEBUGMSGOID(("build_oid_noalloc", in, *out_len));
    DEBUGMSG(("build_oid_noalloc", "\n"));
    return SNMPERR_SUCCESS;
}

int
build_oid(oid ** out, size_t * out_len,
          oid * prefix, size_t prefix_len, netsnmp_variable_list * indexes)
{
    oid             tmpout[MAX_OID_LEN];

    if (build_oid_noalloc(tmpout, sizeof(tmpout), out_len,
                          prefix, prefix_len, indexes) != SNMPERR_SUCCESS)
        return SNMPERR_GENERR;

    snmp_clone_mem((void **) out, (void *) tmpout, *out_len * sizeof(oid));

    return SNMPERR_SUCCESS;
}

/*
 * vblist_out must contain a pre-allocated string of variables into
 * which indexes can be extracted based on the previously existing
 * types in the variable chain
 * returns:
 * SNMPERR_GENERR  on error
 * SNMPERR_SUCCESS on success
 */

int
parse_oid_indexes(oid * oidIndex, size_t oidLen,
                  netsnmp_variable_list * data)
{
    netsnmp_variable_list *var = data;

    while (var && oidLen > 0) {

        if (parse_one_oid_index(&oidIndex, &oidLen, var, 0) !=
            SNMPERR_SUCCESS)
            break;

        var = var->next_variable;
    }

    if (var != NULL || oidLen != 0)
        return SNMPERR_GENERR;
    return SNMPERR_SUCCESS;
}


int
parse_one_oid_index(oid ** oidStart, size_t * oidLen,
                    netsnmp_variable_list * data, int complete)
{
    netsnmp_variable_list *var = data;
    oid             tmpout[MAX_OID_LEN];
    unsigned int    i;
    unsigned int    uitmp = 0;

    oid            *oidIndex = *oidStart;

    if (var == NULL || ((*oidLen == 0) && (complete == 0)))
        return SNMPERR_GENERR;
    else {
        switch (var->type) {
        case ASN_INTEGER:
        case ASN_COUNTER:
        case ASN_GAUGE:
        case ASN_TIMETICKS:
            if (*oidLen) {
                snmp_set_var_value(var, (u_char *) oidIndex++,
                                   sizeof(long));
                --(*oidLen);
            } else {
                snmp_set_var_value(var, (u_char *) oidLen, sizeof(long));
            }
            DEBUGMSGTL(("parse_oid_indexes",
                        "Parsed int(%d): %d\n", var->type,
                        *var->val.integer));
            break;

        case ASN_IPADDRESS:
            if ((4 > *oidLen) && (complete == 0))
                return SNMPERR_GENERR;
            
            for (i = 0; i < 4 && i < *oidLen; ++i) {
                if (oidIndex[i] > 255) {
                    DEBUGMSGTL(("parse_oid_indexes",
                                "illegal oid in index: %d\n", oidIndex[0]));
                        return SNMPERR_GENERR;  /* sub-identifier too large */
                    }
                    uitmp = uitmp + (oidIndex[i] << (8*(3-i)));
                }
            if (4 > (int) (*oidLen)) {
                oidIndex += *oidLen;
                (*oidLen) = 0;
            } else {
                oidIndex += 4;
                (*oidLen) -= 4;
            }
            uitmp = 
                snmp_set_var_value(var, (u_char *) &uitmp, 4);
            DEBUGMSGTL(("parse_oid_indexes",
                        "Parsed ipaddr(%d): %d.%d.%d.%d\n", var->type,
                        var->val.string[0], var->val.string[1],
                        var->val.string[2], var->val.string[3]));
            break;

        case ASN_OBJECT_ID:
        case ASN_PRIV_IMPLIED_OBJECT_ID:
            if (var->type == ASN_PRIV_IMPLIED_OBJECT_ID) {
                uitmp = *oidLen;
            } else {
                if (*oidLen) {
                    uitmp = *oidIndex++;
                    --(*oidLen);
                } else {
                    uitmp = 0;
                }
                if ((uitmp > *oidLen) && (complete == 0))
                    return SNMPERR_GENERR;
            }

            if (uitmp > MAX_OID_LEN)
                return SNMPERR_GENERR;  /* too big and illegal */

            if (uitmp > *oidLen) {
                memcpy(tmpout, oidIndex, sizeof(oid) * (*oidLen));
                memset(&tmpout[*oidLen], 0x00,
                       sizeof(oid) * (uitmp - *oidLen));
                snmp_set_var_value(var, (u_char *) tmpout,
                                   sizeof(oid) * uitmp);
                oidIndex += *oidLen;
                (*oidLen) = 0;
            } else {
                snmp_set_var_value(var, (u_char *) oidIndex,
                                   sizeof(oid) * uitmp);
                oidIndex += uitmp;
                (*oidLen) -= uitmp;
            }

            DEBUGMSGTL(("parse_oid_indexes", "Parsed oid: "));
            DEBUGMSGOID(("parse_oid_indexes",
                         var->val.objid, var->val_len / sizeof(oid)));
            DEBUGMSG(("parse_oid_indexes", "\n"));
            break;

        case ASN_OPAQUE:
        case ASN_OCTET_STR:
        case ASN_PRIV_IMPLIED_OCTET_STR:
            if (var->type == ASN_PRIV_IMPLIED_OCTET_STR) {
                uitmp = *oidLen;
            } else {
                if (*oidLen) {
                    uitmp = *oidIndex++;
                    --(*oidLen);
                } else {
                    uitmp = 0;
                }
                if ((uitmp > *oidLen) && (complete == 0))
                    return SNMPERR_GENERR;
            }

            /*
             * we handle this one ourselves since we don't have
             * pre-allocated memory to copy from using
             * snmp_set_var_value() 
             */

            if (uitmp == 0)
                break;          /* zero length strings shouldn't malloc */

            if (uitmp > MAX_OID_LEN)
                return SNMPERR_GENERR;  /* too big and illegal */

            /*
             * malloc by size+1 to allow a null to be appended. 
             */
            var->val_len = uitmp;
            var->val.string = (u_char *) calloc(1, uitmp + 1);
            if (var->val.string == NULL)
                return SNMPERR_GENERR;

            if (uitmp > (int) (*oidLen)) {
                for (i = 0; i < *oidLen; ++i)
                    var->val.string[i] = (u_char) * oidIndex++;
                for (i = 0; i < uitmp; ++i)
                    var->val.string[i] = '\0';
                (*oidLen) = 0;
            } else {
                for (i = 0; i < uitmp; ++i)
                    var->val.string[i] = (u_char) * oidIndex++;
                (*oidLen) -= uitmp;
            }
            var->val.string[uitmp] = '\0';

            DEBUGMSGTL(("parse_oid_indexes",
                        "Parsed str(%d): %s\n", var->type,
                        var->val.string));
            break;

        default:
            DEBUGMSGTL(("parse_oid_indexes",
                        "invalid asn type: %d\n", var->type));
            return SNMPERR_GENERR;
        }
    }
    (*oidStart) = oidIndex;
    return SNMPERR_SUCCESS;
}

int
dump_realloc_oid_to_string(const oid * objid, size_t objidlen,
                           u_char ** buf, size_t * buf_len,
                           size_t * out_len, int allow_realloc,
                           char quotechar)
{
    if (buf) {
        int             i, alen;

        for (i = 0, alen = 0; i < (int) objidlen; i++) {
            oid             tst = objid[i];
            if ((tst > 254) || (!isprint(tst))) {
                tst = (oid) '.';
            }

            if (alen == 0) {
                if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_ESCAPE_QUOTES)) {
                    while ((*out_len + 2) >= *buf_len) {
                        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                            return 0;
                        }
                    }
                    *(*buf + *out_len) = '\\';
                    (*out_len)++;
                }
                while ((*out_len + 2) >= *buf_len) {
                    if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                        return 0;
                    }
                }
                *(*buf + *out_len) = quotechar;
                (*out_len)++;
            }

            while ((*out_len + 2) >= *buf_len) {
                if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                    return 0;
                }
            }
            *(*buf + *out_len) = (char) tst;
            (*out_len)++;
            alen++;
        }

        if (alen) {
            if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_ESCAPE_QUOTES)) {
                while ((*out_len + 2) >= *buf_len) {
                    if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                        return 0;
                    }
                }
                *(*buf + *out_len) = '\\';
                (*out_len)++;
            }
            while ((*out_len + 2) >= *buf_len) {
                if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
                    return 0;
                }
            }
            *(*buf + *out_len) = quotechar;
            (*out_len)++;
        }

        *(*buf + *out_len) = '\0';
    }

    return 1;
}

static struct tree *
_get_realloc_symbol(const oid * objid, size_t objidlen,
                    struct tree *subtree,
                    u_char ** buf, size_t * buf_len, size_t * out_len,
                    int allow_realloc, int *buf_overflow,
                    struct index_list *in_dices, size_t * end_of_known)
{
    struct tree    *return_tree = NULL;
    int             extended_index =
        netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_EXTENDED_INDEX);
    int             output_format =
        netsnmp_ds_get_int(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT);
    char            intbuf[64];

    if (!objid || !buf) {
        return NULL;
    }

    for (; subtree; subtree = subtree->next_peer) {
        if (*objid == subtree->subid) {
	    while (subtree->next_peer && subtree->next_peer->subid == *objid)
		subtree = subtree->next_peer;
            if (subtree->indexes) {
                in_dices = subtree->indexes;
            } else if (subtree->augments) {
                struct tree    *tp2 =
                    find_tree_node(subtree->augments, -1);
                if (tp2) {
                    in_dices = tp2->indexes;
                }
            }

            if (!strncmp(subtree->label, ANON, ANON_LEN) ||
                (NETSNMP_OID_OUTPUT_NUMERIC == output_format)) {
                sprintf(intbuf, "%lu", subtree->subid);
                if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                                   allow_realloc,
                                                   (const u_char *)
                                                   intbuf)) {
                    *buf_overflow = 1;
                }
            } else {
                if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                                   allow_realloc,
                                                   (const u_char *)
                                                   subtree->label)) {
                    *buf_overflow = 1;
                }
            }

            if (objidlen > 1) {
                if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                                   allow_realloc,
                                                   (const u_char *) ".")) {
                    *buf_overflow = 1;
                }

                return_tree = _get_realloc_symbol(objid + 1, objidlen - 1,
                                                  subtree->child_list,
                                                  buf, buf_len, out_len,
                                                  allow_realloc,
                                                  buf_overflow, in_dices,
                                                  end_of_known);
            }

            if (return_tree != NULL) {
                return return_tree;
            } else {
                return subtree;
            }
        }
    }


    if (end_of_known) {
        *end_of_known = *out_len;
    }

    /*
     * Subtree not found.  
     */

    while (in_dices && (objidlen > 0) &&
           (NETSNMP_OID_OUTPUT_NUMERIC != output_format) &&
           !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS)) {
        size_t          numids;
        struct tree    *tp;

        tp = find_tree_node(in_dices->ilabel, -1);

        if (!tp) {
            /*
             * Can't find an index in the mib tree.  Bail.  
             */
            goto finish_it;
        }

        if (extended_index) {
            if (*buf != NULL && *(*buf + *out_len - 1) == '.') {
                (*out_len)--;
            }
            if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                               allow_realloc,
                                               (const u_char *) "[")) {
                *buf_overflow = 1;
            }
        }

        switch (tp->type) {
        case TYPE_OCTETSTR:
            if (extended_index && tp->hint) {
                netsnmp_variable_list var;
                u_char          buffer[1024];
                int             i;

                memset(&var, 0, sizeof var);
                if (in_dices->isimplied) {
                    numids = objidlen;
                    if (numids > objidlen)
                        goto finish_it;
                } else if (tp->ranges && !tp->ranges->next
                           && tp->ranges->low == tp->ranges->high) {
                    numids = tp->ranges->low;
                    if (numids > objidlen)
                        goto finish_it;
                } else {
                    numids = *objid;
                    if (numids >= objidlen)
                        goto finish_it;
                    objid++;
                    objidlen--;
                }
                if (numids > objidlen)
                    goto finish_it;
                for (i = 0; i < (int) numids; i++)
                    buffer[i] = (u_char) objid[i];
                var.type = ASN_OCTET_STR;
                var.val.string = buffer;
                var.val_len = numids;
                if (!*buf_overflow) {
                    if (!sprint_realloc_octet_string(buf, buf_len, out_len,
                                                     allow_realloc, &var,
                                                     NULL, tp->hint,
                                                     NULL)) {
                        *buf_overflow = 1;
                    }
                }
            } else if (in_dices->isimplied) {
                numids = objidlen;
                if (numids > objidlen)
                    goto finish_it;

                if (!*buf_overflow) {
                    if (!dump_realloc_oid_to_string
                        (objid, numids, buf, buf_len, out_len,
                         allow_realloc, '\'')) {
                        *buf_overflow = 1;
                    }
                }
            } else if (tp->ranges && !tp->ranges->next
                       && tp->ranges->low == tp->ranges->high) {
                /*
                 * a fixed-length object string 
                 */
                numids = tp->ranges->low;
                if (numids > objidlen)
                    goto finish_it;

                if (!*buf_overflow) {
                    if (!dump_realloc_oid_to_string
                        (objid, numids, buf, buf_len, out_len,
                         allow_realloc, '\'')) {
                        *buf_overflow = 1;
                    }
                }
            } else {
                numids = (size_t) * objid + 1;
                if (numids > objidlen)
                    goto finish_it;
                if (numids == 1) {
                    if (netsnmp_ds_get_boolean
                        (NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_ESCAPE_QUOTES)) {
                        if (!*buf_overflow
                            && !snmp_strcat(buf, buf_len, out_len,
                                            allow_realloc,
                                            (const u_char *) "\\")) {
                            *buf_overflow = 1;
                        }
                    }
                    if (!*buf_overflow
                        && !snmp_strcat(buf, buf_len, out_len,
                                        allow_realloc,
                                        (const u_char *) "\"")) {
                        *buf_overflow = 1;
                    }
                    if (netsnmp_ds_get_boolean
                        (NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_ESCAPE_QUOTES)) {
                        if (!*buf_overflow
                            && !snmp_strcat(buf, buf_len, out_len,
                                            allow_realloc,
                                            (const u_char *) "\\")) {
                            *buf_overflow = 1;
                        }
                    }
                    if (!*buf_overflow
                        && !snmp_strcat(buf, buf_len, out_len,
                                        allow_realloc,
                                        (const u_char *) "\"")) {
                        *buf_overflow = 1;
                    }
                } else {
                    if (!*buf_overflow) {
                        if (!dump_realloc_oid_to_string
                            (objid + 1, numids - 1, buf, buf_len, out_len,
                             allow_realloc, '"')) {
                            *buf_overflow = 1;
                        }
                    }
                }
            }
            objid += numids;
            objidlen -= numids;
            break;

        case TYPE_INTEGER32:
        case TYPE_UINTEGER:
        case TYPE_UNSIGNED32:
        case TYPE_GAUGE:
        case TYPE_INTEGER:
            if (tp->enums) {
                struct enum_list *ep = tp->enums;
                while (ep && ep->value != (int) (*objid)) {
                    ep = ep->next;
                }
                if (ep) {
                    if (!*buf_overflow
                        && !snmp_strcat(buf, buf_len, out_len,
                                        allow_realloc,
                                        (const u_char *) ep->label)) {
                        *buf_overflow = 1;
                    }
                } else {
                    sprintf(intbuf, "%lu", *objid);
                    if (!*buf_overflow
                        && !snmp_strcat(buf, buf_len, out_len,
                                        allow_realloc,
                                        (const u_char *) intbuf)) {
                        *buf_overflow = 1;
                    }
                }
            } else {
                sprintf(intbuf, "%lu", *objid);
                if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                                   allow_realloc,
                                                   (const u_char *)
                                                   intbuf)) {
                    *buf_overflow = 1;
                }
            }
            objid++;
            objidlen--;
            break;

        case TYPE_OBJID:
            if (in_dices->isimplied) {
                numids = objidlen;
            } else {
                numids = (size_t) * objid + 1;
            }
            if (numids > objidlen)
                goto finish_it;
            if (extended_index) {
                if (in_dices->isimplied) {
                    if (!*buf_overflow
                        && !netsnmp_sprint_realloc_objid_tree(buf, buf_len,
                                                              out_len,
                                                              allow_realloc,
                                                              buf_overflow,
                                                              objid,
                                                              numids)) {
                        *buf_overflow = 1;
                    }
                } else {
                    if (!*buf_overflow
                        && !netsnmp_sprint_realloc_objid_tree(buf, buf_len,
                                                              out_len,
                                                              allow_realloc,
                                                              buf_overflow,
                                                              objid + 1,
                                                              numids -
                                                              1)) {
                        *buf_overflow = 1;
                    }
                }
            } else {
                _get_realloc_symbol(objid, numids, NULL, buf, buf_len,
                                    out_len, allow_realloc, buf_overflow,
                                    NULL, NULL);
            }
            objid += (numids);
            objidlen -= (numids);
            break;

        case TYPE_IPADDR:
            if (objidlen < 4)
                goto finish_it;
            sprintf(intbuf, "%lu.%lu.%lu.%lu",
                    objid[0], objid[1], objid[2], objid[3]);
            objid += 4;
            objidlen -= 4;
            if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                               allow_realloc,
                                               (const u_char *) intbuf)) {
                *buf_overflow = 1;
            }
            break;

        case TYPE_NETADDR:{
                oid             ntype = *objid++;

                objidlen--;
                sprintf(intbuf, "%lu.", ntype);
                if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                                   allow_realloc,
                                                   (const u_char *)
                                                   intbuf)) {
                    *buf_overflow = 1;
                }

                if (ntype == 1 && objidlen >= 4) {
                    sprintf(intbuf, "%lu.%lu.%lu.%lu",
                            objid[0], objid[1], objid[2], objid[3]);
                    if (!*buf_overflow
                        && !snmp_strcat(buf, buf_len, out_len,
                                        allow_realloc,
                                        (const u_char *) intbuf)) {
                        *buf_overflow = 1;
                    }
                    objid += 4;
                    objidlen -= 4;
                } else {
                    goto finish_it;
                }
            }
            break;

        case TYPE_NSAPADDRESS:
        default:
            goto finish_it;
            break;
        }

        if (extended_index) {
            if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                               allow_realloc,
                                               (const u_char *) "]")) {
                *buf_overflow = 1;
            }
        } else {
            if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                               allow_realloc,
                                               (const u_char *) ".")) {
                *buf_overflow = 1;
            }
        }
        in_dices = in_dices->next;
    }

  finish_it:
    if (*buf != NULL && *(*buf + *out_len - 1) != '.') {
        if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                           allow_realloc,
                                           (const u_char *) ".")) {
            *buf_overflow = 1;
        }
    }

    while (objidlen-- > 0) {    /* output rest of name, uninterpreted */
        sprintf(intbuf, "%lu.", *objid++);
        if (!*buf_overflow && !snmp_strcat(buf, buf_len, out_len,
                                           allow_realloc,
                                           (const u_char *) intbuf)) {
            *buf_overflow = 1;
        }
    }

    if (*buf != NULL) {
        *(*buf + *out_len - 1) = '\0';  /* remove trailing dot */
    }
    return NULL;
}

/**
 * Clone of get_symbol that doesn't take a buffer argument.
 *
 * @see get_symbol
 */
struct tree    *
get_tree(const oid * objid, size_t objidlen, struct tree *subtree)
{
    struct tree    *return_tree = NULL;

    for (; subtree; subtree = subtree->next_peer) {
        if (*objid == subtree->subid)
            goto found;
    }

    return NULL;

  found:
    while (subtree->next_peer && subtree->next_peer->subid == *objid)
	subtree = subtree->next_peer;
    if (objidlen > 1)
        return_tree =
            get_tree(objid + 1, objidlen - 1, subtree->child_list);
    if (return_tree != NULL)
        return return_tree;
    else
        return subtree;
}

/**
 * Prints on oid description on stdout.
 *
 * @see fprint_description
 */
void
print_description(oid * objid, size_t objidlen, /* number of subidentifiers */
                  int width)
{
    fprint_description(stdout, objid, objidlen, width);
}


/**
 * Prints on oid description into a file descriptor.
 * 
 * @param f         The file descriptor to print to.
 * @param objid     The object identifier.
 * @param objidlen  The object id length.
 * @param width     Number of subidentifiers.
 */
void
fprint_description(FILE * f, oid * objid, size_t objidlen,
                   int width)
{
    struct tree    *tp = get_tree(objid, objidlen, tree_head);
    struct tree    *subtree = tree_head;
    int             pos, len;
    char            buf[128];
    const char     *cp;

    if (tp->type <= TYPE_SIMPLE_LAST)
        cp = "OBJECT-TYPE";
    else
        switch (tp->type) {
        case TYPE_TRAPTYPE:
            cp = "TRAP-TYPE";
            break;
        case TYPE_NOTIFTYPE:
            cp = "NOTIFICATION-TYPE";
            break;
        case TYPE_OBJGROUP:
            cp = "OBJECT-GROUP";
            break;
        case TYPE_AGENTCAP:
            cp = "AGENT-CAPABILITIES";
            break;
        case TYPE_MODID:
            cp = "MODULE-IDENTITY";
            break;
        case TYPE_MODCOMP:
            cp = "MODULE-COMPLIANCE";
            break;
        default:
            sprintf(buf, "type_%d", tp->type);
            cp = buf;
        }
    fprintf(f, "%s %s\n", tp->label, cp);
    print_tree_node(f, tp, width);
    fprintf(f, "::= {");
    pos = 5;
    while (objidlen > 1) {
        for (; subtree; subtree = subtree->next_peer) {
            if (*objid == subtree->subid) {
                while (subtree->next_peer && subtree->next_peer->subid == *objid)
                    subtree = subtree->next_peer;
                if (strncmp(subtree->label, ANON, ANON_LEN)) {
                    snprintf(buf, sizeof(buf), " %s(%lu)", subtree->label, subtree->subid);
                    buf[ sizeof(buf)-1 ] = 0;
                } else
                    sprintf(buf, " %lu", subtree->subid);
                len = strlen(buf);
                if (pos + len + 2 > width) {
                    fprintf(f, "\n     ");
                    pos = 5;
                }
                fprintf(f, "%s", buf);
                pos += len;
                objid++;
                objidlen--;
                break;
            }
        }
        if (subtree)
            subtree = subtree->child_list;
        else
            break;
    }
    while (objidlen > 1) {
        sprintf(buf, " %lu", *objid);
        len = strlen(buf);
        if (pos + len + 2 > width) {
            fprintf(f, "\n     ");
            pos = 5;
        }
        fprintf(f, "%s", buf);
        pos += len;
        objid++;
        objidlen--;
    }
    sprintf(buf, " %lu }", *objid);
    len = strlen(buf);
    if (pos + len + 2 > width) {
        fprintf(f, "\n     ");
        pos = 5;
    }
    fprintf(f, "%s\n", buf);
}

static void
print_tree_node(FILE * f, struct tree *tp, int width)
{
    const char     *cp;
    char            str[MAXTOKEN];
    int             i, prevmod, pos, len;

    if (tp) {
        module_name(tp->modid, str);
        fprintf(f, "  -- FROM\t%s", str);
        pos = 16+strlen(str);
        for (i = 1, prevmod = tp->modid; i < tp->number_modules; i++) {
            if (prevmod != tp->module_list[i]) {
                module_name(tp->module_list[i], str);
                len = strlen(str);
                if (pos + len + 2 > width) {
                    fprintf(f, ",\n  --\t\t");
                    pos = 16;
                }
                else {
                    fprintf(f, ", ");
                    pos += 2;
                }
                fprintf(f, "%s", str);
                pos += len;
            }
            prevmod = tp->module_list[i];
        }
        fprintf(f, "\n");
        if (tp->tc_index != -1) {
            fprintf(f, "  -- TEXTUAL CONVENTION %s\n",
                    get_tc_descriptor(tp->tc_index));
        }
        switch (tp->type) {
        case TYPE_OBJID:
            cp = "OBJECT IDENTIFIER";
            break;
        case TYPE_OCTETSTR:
            cp = "OCTET STRING";
            break;
        case TYPE_INTEGER:
            cp = "INTEGER";
            break;
        case TYPE_NETADDR:
            cp = "NetworkAddress";
            break;
        case TYPE_IPADDR:
            cp = "IpAddress";
            break;
        case TYPE_COUNTER:
            cp = "Counter32";
            break;
        case TYPE_GAUGE:
            cp = "Gauge32";
            break;
        case TYPE_TIMETICKS:
            cp = "TimeTicks";
            break;
        case TYPE_OPAQUE:
            cp = "Opaque";
            break;
        case TYPE_NULL:
            cp = "NULL";
            break;
        case TYPE_COUNTER64:
            cp = "Counter64";
            break;
        case TYPE_BITSTRING:
            cp = "BITS";
            break;
        case TYPE_NSAPADDRESS:
            cp = "NsapAddress";
            break;
        case TYPE_UINTEGER:
            cp = "UInteger32";
            break;
        case TYPE_UNSIGNED32:
            cp = "Unsigned32";
            break;
        case TYPE_INTEGER32:
            cp = "Integer32";
            break;
        default:
            cp = NULL;
            break;
        }
#if SNMP_TESTING_CODE
        if (!cp && (tp->ranges || tp->enums)) { /* ranges without type ? */
            sprintf(str, "?0 with %s %s ?",
                    tp->ranges ? "Range" : "", tp->enums ? "Enum" : "");
            cp = str;
        }
#endif                          /* SNMP_TESTING_CODE */
        if (cp)
            fprintf(f, "  SYNTAX\t%s", cp);
        if (tp->ranges) {
            struct range_list *rp = tp->ranges;
            int             first = 1;
            fprintf(f, " (");
            while (rp) {
                if (first)
                    first = 0;
                else
                    fprintf(f, " | ");
                if (rp->low == rp->high)
                    fprintf(f, "%d", rp->low);
                else
                    fprintf(f, "%d..%d", rp->low, rp->high);
                rp = rp->next;
            }
            fprintf(f, ") ");
        }
        if (tp->enums) {
            struct enum_list *ep = tp->enums;
            int             first = 1;
            fprintf(f, " { ");
            pos = 16 + strlen(cp) + 2;
            while (ep) {
                if (first)
                    first = 0;
                else
                    fprintf(f, ", ");
                snprintf(str, sizeof(str), "%s(%d)", ep->label, ep->value);
                str[ sizeof(str)-1 ] = 0;
                len = strlen(str);
                if (pos + len + 2 > width) {
                    fprintf(f, "\n\t\t  ");
                    pos = 18;
                }
                fprintf(f, "%s", str);
                pos += len + 2;
                ep = ep->next;
            }
            fprintf(f, " } ");
        }
        if (cp)
            fprintf(f, "\n");
        if (tp->hint)
            fprintf(f, "  DISPLAY-HINT\t\"%s\"\n", tp->hint);
        if (tp->units)
            fprintf(f, "  UNITS\t\"%s\"\n", tp->units);
        switch (tp->access) {
        case MIB_ACCESS_READONLY:
            cp = "read-only";
            break;
        case MIB_ACCESS_READWRITE:
            cp = "read-write";
            break;
        case MIB_ACCESS_WRITEONLY:
            cp = "write-only";
            break;
        case MIB_ACCESS_NOACCESS:
            cp = "not-accessible";
            break;
        case MIB_ACCESS_NOTIFY:
            cp = "accessible-for-notify";
            break;
        case MIB_ACCESS_CREATE:
            cp = "read-create";
            break;
        case 0:
            cp = NULL;
            break;
        default:
            sprintf(str, "access_%d", tp->access);
            cp = str;
        }
        if (cp)
            fprintf(f, "  MAX-ACCESS\t%s\n", cp);
        switch (tp->status) {
        case MIB_STATUS_MANDATORY:
            cp = "mandatory";
            break;
        case MIB_STATUS_OPTIONAL:
            cp = "optional";
            break;
        case MIB_STATUS_OBSOLETE:
            cp = "obsolete";
            break;
        case MIB_STATUS_DEPRECATED:
            cp = "deprecated";
            break;
        case MIB_STATUS_CURRENT:
            cp = "current";
            break;
        case 0:
            cp = NULL;
            break;
        default:
            sprintf(str, "status_%d", tp->status);
            cp = str;
        }
#if SNMP_TESTING_CODE
        if (!cp && (tp->indexes)) {     /* index without status ? */
            sprintf(str, "?0 with %s ?", tp->indexes ? "Index" : "");
            cp = str;
        }
#endif                          /* SNMP_TESTING_CODE */
        if (cp)
            fprintf(f, "  STATUS\t%s\n", cp);
        if (tp->augments)
            fprintf(f, "  AUGMENTS\t{ %s }\n", tp->augments);
        if (tp->indexes) {
            struct index_list *ip = tp->indexes;
            int             first = 1;
            fprintf(f, "  INDEX\t\t");
            fprintf(f, "{ ");
            pos = 16 + 2;
            while (ip) {
                if (first)
                    first = 0;
                else
                    fprintf(f, ", ");
                snprintf(str, sizeof(str), "%s%s",
                        ip->isimplied ? "IMPLIED " : "",
                        ip->ilabel);
                str[ sizeof(str)-1 ] = 0;
                len = strlen(str);
                if (pos + len + 2 > width) {
                    fprintf(f, "\n\t\t  ");
                    pos = 16 + 2;
                }
                fprintf(f, "%s", str);
                pos += len + 2;
                ip = ip->next;
            }
            fprintf(f, " }\n");
        }
        if (tp->varbinds) {
            struct varbind_list *vp = tp->varbinds;
            int             first = 1;
            fprintf(f, "  %s\t", tp->type == TYPE_TRAPTYPE ?
                    "VARIABLES" : "OBJECTS");
            fprintf(f, "{ ");
            pos = 16 + 2;
            while (vp) {
                if (first)
                    first = 0;
                else
                    fprintf(f, ", ");
                snprintf(str, sizeof(str), "%s", vp->vblabel);
                str[ sizeof(str)-1 ] = 0;
                len = strlen(str);
                if (pos + len + 2 > width) {
                    fprintf(f, "\n\t\t  ");
                    pos = 16 + 2;
                }
                fprintf(f, "%s", str);
                pos += len + 2;
                vp = vp->next;
            }
            fprintf(f, " }\n");
        }
        if (tp->description)
            fprintf(f, "  DESCRIPTION\t\"%s\"\n", tp->description);
        if (tp->defaultValue)
            fprintf(f, "  DEFVAL\t{ %s }\n", tp->defaultValue);
    } else
        fprintf(f, "No description\n");
}

int
get_module_node(const char *fname,
                const char *module, oid * objid, size_t * objidlen)
{
    int             modid, rc = 0;
    struct tree    *tp;
    char           *name, *cp;

    if (!strcmp(module, "ANY"))
        modid = -1;
    else {
        read_module(module);
        modid = which_module(module);
        if (modid == -1)
            return 0;
    }

    /*
     * Isolate the first component of the name ... 
     */
    name = strdup(fname);
    cp = strchr(name, '.');
    if (cp != NULL) {
        *cp = '\0';
        cp++;
    }
    /*
     * ... and locate it in the tree. 
     */
    tp = find_tree_node(name, modid);
    if (tp) {
        size_t          maxlen = *objidlen;

        /*
         * Set the first element of the object ID 
         */
        if (node_to_oid(tp, objid, objidlen)) {
            rc = 1;

            /*
             * If the name requested was more than one element,
             * tag on the rest of the components 
             */
            if (cp != NULL)
                rc = _add_strings_to_oid(tp, cp, objid, objidlen, maxlen);
        }
    }

    free(name);
    return (rc);
}


/**
 * @internal
 *
 * Populates the object identifier from a node in the MIB hierarchy.
 * Builds up the object ID, working backwards,
 * starting from the end of the objid buffer.
 * When the top of the MIB tree is reached, the buffer is adjusted.
 *
 * The buffer length is set to the number of subidentifiers
 * for the object identifier associated with the MIB node.
 * 
 * @return the number of subidentifiers copied.
 *
 * If 0 is returned, the objid buffer is too small,
 * and the buffer contents are indeterminate.
 * The buffer length can be used to create a larger buffer.
 */
static int
node_to_oid(struct tree *tp, oid * objid, size_t * objidlen)
{
    int             numids, lenids;
    oid            *op;

    if (!tp || !objid || !objidlen)
        return 0;

    lenids = (int) *objidlen;
    op = objid + lenids;        /* points after the last element */

    for (numids = 0; tp; tp = tp->parent, numids++) {
        if (numids >= lenids)
            continue;
        --op;
        *op = tp->subid;
    }

    *objidlen = (size_t) numids;
    if (numids > lenids) {
        return 0;
    }

    if (numids < lenids)
        memmove(objid, op, numids * sizeof(oid));

    return (numids);
}

/*
 * Replace \x with x stop at eos_marker
 * return NULL if eos_marker not found
 */
static char *_apply_escapes(char *src, char eos_marker)
{
    char *dst;
    int backslash = 0;
    
    dst = src;
    while (*src) {
	if (backslash) {
	    backslash = 0;
	    *dst++ = *src;
	} else {
	    if (eos_marker == *src) break;
	    if ('\\' == *src) {
		backslash = 1;
	    } else {
		*dst++ = *src;
	    }
	}
	src++;
    }
    if (!*src) {
	/* never found eos_marker */
	return NULL;
    } else {
	*dst = 0;
	return src;
    }
}

static int
_add_strings_to_oid(struct tree *tp, char *cp,
                    oid * objid, size_t * objidlen, size_t maxlen)
{
    oid             subid;
    int             len_index = 1000000;
    struct tree    *tp2 = NULL;
    struct index_list *in_dices = NULL;
    char           *fcp, *ecp, *cp2 = NULL;
    char            doingquote;
    int             len = -1, pos = -1;
    int             check =
        !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_DONT_CHECK_RANGE);
    int             do_hint = !netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_NO_DISPLAY_HINT);

    while (cp && tp && tp->child_list) {
        fcp = cp;
        tp2 = tp->child_list;
        /*
         * Isolate the next entry 
         */
        cp2 = strchr(cp, '.');
        if (cp2)
            *cp2++ = '\0';

        /*
         * Search for the appropriate child 
         */
        if (isdigit(*cp)) {
            subid = strtoul(cp, &ecp, 0);
            if (*ecp)
                goto bad_id;
            while (tp2 && tp2->subid != subid)
                tp2 = tp2->next_peer;
        } else {
            while (tp2 && strcmp(tp2->label, fcp))
                tp2 = tp2->next_peer;
            if (!tp2)
                goto bad_id;
            subid = tp2->subid;
        }
        if (*objidlen >= maxlen)
            goto bad_id;
	while (tp2 && tp2->next_peer && tp2->next_peer->subid == subid)
	    tp2 = tp2->next_peer;
        objid[*objidlen] = subid;
        (*objidlen)++;

        cp = cp2;
        if (!tp2)
            break;
        tp = tp2;
    }

    if (tp && !tp->child_list) {
        if ((tp2 = tp->parent)) {
            if (tp2->indexes)
                in_dices = tp2->indexes;
            else if (tp2->augments) {
                tp2 = find_tree_node(tp2->augments, -1);
                if (tp2)
                    in_dices = tp2->indexes;
            }
        }
        tp = NULL;
    }

    while (cp && in_dices) {
        fcp = cp;

        tp = find_tree_node(in_dices->ilabel, -1);
        if (!tp)
            break;
        switch (tp->type) {
        case TYPE_INTEGER:
        case TYPE_INTEGER32:
        case TYPE_UINTEGER:
        case TYPE_UNSIGNED32:
        case TYPE_TIMETICKS:
            /*
             * Isolate the next entry 
             */
            cp2 = strchr(cp, '.');
            if (cp2)
                *cp2++ = '\0';
            if (isdigit(*cp)) {
                subid = strtoul(cp, &ecp, 0);
                if (*ecp)
                    goto bad_id;
            } else {
                if (tp->enums) {
                    struct enum_list *ep = tp->enums;
                    while (ep && strcmp(ep->label, cp))
                        ep = ep->next;
                    if (!ep)
                        goto bad_id;
                    subid = ep->value;
                } else
                    goto bad_id;
            }
            if (check && tp->ranges) {
                struct range_list *rp = tp->ranges;
                int             ok = 0;
                while (!ok && rp)
                    if ((rp->low <= (int) subid)
                        && ((int) subid <= rp->high))
                        ok = 1;
                    else
                        rp = rp->next;
                if (!ok)
                    goto bad_id;
            }
            if (*objidlen >= maxlen)
                goto bad_id;
            objid[*objidlen] = subid;
            (*objidlen)++;
            break;
        case TYPE_IPADDR:
            if (*objidlen + 4 > maxlen)
                goto bad_id;
            for (subid = 0; cp && subid < 4; subid++) {
                fcp = cp;
                cp2 = strchr(cp, '.');
                if (cp2)
                    *cp2++ = 0;
                objid[*objidlen] = strtoul(cp, &ecp, 0);
                if (*ecp)
                    goto bad_id;
                if (check && objid[*objidlen] > 255)
                    goto bad_id;
                (*objidlen)++;
                cp = cp2;
            }
            break;
        case TYPE_OCTETSTR:
            if (tp->ranges && !tp->ranges->next
                && tp->ranges->low == tp->ranges->high)
                len = tp->ranges->low;
            else
                len = -1;
            pos = 0;
            if (*cp == '"' || *cp == '\'') {
                doingquote = *cp++;
                /*
                 * insert length if requested 
                 */
                if (!in_dices->isimplied && len == -1) {
                    if (doingquote == '\'') {
                        snmp_set_detail
                            ("'-quote is for fixed length strings");
                        return 0;
                    }
                    if (*objidlen >= maxlen)
                        goto bad_id;
                    len_index = *objidlen;
                    (*objidlen)++;
                } else if (doingquote == '"') {
                    snmp_set_detail
                        ("\"-quote is for variable length strings");
                    return 0;
                }

		cp2 = _apply_escapes(cp, doingquote);
		if (!cp2) goto bad_id;
		else {
		    unsigned char *new_val;
		    int new_val_len;
		    int parsed_hint = 0;
		    const char *parsed_value;

		    if (do_hint && tp->hint) {
			parsed_value = parse_octet_hint(tp->hint, cp,
			                                &new_val, &new_val_len);
			parsed_hint = parsed_value == NULL;
		    }
		    if (parsed_hint) {
			int i;
			for (i = 0; i < new_val_len; i++) {
			    if (*objidlen >= maxlen) goto bad_id;
			    objid[ *objidlen ] = new_val[i];
			    (*objidlen)++;
			    pos++;
			}
			free(new_val);
		    } else {
			while(*cp) {
			    if (*objidlen >= maxlen) goto bad_id;
			    objid[ *objidlen ] = *cp++;
			    (*objidlen)++;
			    pos++;
			}
		    }
		}
		
		cp2++;
                if (!*cp2)
                    cp2 = NULL;
                else if (*cp2 != '.')
                    goto bad_id;
                else
                    cp2++;
		if (check) {
                    if (len == -1) {
                        struct range_list *rp = tp->ranges;
                        int             ok = 0;
                        while (rp && !ok)
                            if (rp->low <= pos && pos <= rp->high)
                                ok = 1;
                            else
                                rp = rp->next;
                        if (!ok)
                            goto bad_id;
                        if (!in_dices->isimplied)
                            objid[len_index] = pos;
                    } else if (pos != len)
                        goto bad_id;
		}
		else if (len == -1 && !in_dices->isimplied)
		    objid[len_index] = pos;
            } else {
                if (!in_dices->isimplied && len == -1) {
                    fcp = cp;
                    cp2 = strchr(cp, '.');
                    if (cp2)
                        *cp2++ = 0;
                    len = strtoul(cp, &ecp, 0);
                    if (*ecp)
                        goto bad_id;
                    if (*objidlen + len + 1 >= maxlen)
                        goto bad_id;
                    objid[*objidlen] = len;
                    (*objidlen)++;
                    cp = cp2;
                }
                while (len && cp) {
                    fcp = cp;
                    cp2 = strchr(cp, '.');
                    if (cp2)
                        *cp2++ = 0;
                    objid[*objidlen] = strtoul(cp, &ecp, 0);
                    if (*ecp)
                        goto bad_id;
                    if (check && objid[*objidlen] > 255)
                        goto bad_id;
                    (*objidlen)++;
                    len--;
                    cp = cp2;
                }
            }
            break;
        case TYPE_OBJID:
            in_dices = NULL;
            cp2 = cp;
            break;
	case TYPE_NETADDR:
	    fcp = cp;
	    cp2 = strchr(cp, '.');
	    if (cp2)
		*cp2++ = 0;
	    subid = strtoul(cp, &ecp, 0);
	    if (*ecp)
		goto bad_id;
	    if (*objidlen + 1 >= maxlen)
		goto bad_id;
	    objid[*objidlen] = subid;
	    (*objidlen)++;
	    cp = cp2;
	    if (subid == 1) {
		for (len = 0; len < 4; len++) {
		    fcp = cp;
		    cp2 = strchr(cp, '.');
		    if (cp2)
			*cp2++ = 0;
		    subid = strtoul(cp, &ecp, 0);
		    if (*ecp)
			goto bad_id;
		    if (*objidlen + 1 >= maxlen)
			goto bad_id;
		    if (subid > 255)
			goto bad_id;
		    objid[*objidlen] = subid;
	            (*objidlen)++;
		    cp = cp2;
		}
	    }
	    else {
		in_dices = NULL;
	    }
	    break;
        default:
            snmp_log(LOG_ERR, "Unexpected index type: %d %s %s\n",
                     tp->type, in_dices->ilabel, cp);
            in_dices = NULL;
            cp2 = cp;
            break;
        }
        cp = cp2;
        if (in_dices)
            in_dices = in_dices->next;
    }

    while (cp) {
        fcp = cp;
        switch (*cp) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            cp2 = strchr(cp, '.');
            if (cp2)
                *cp2++ = 0;
            subid = strtoul(cp, &ecp, 0);
            if (*ecp)
                goto bad_id;
            if (*objidlen >= maxlen)
                goto bad_id;
            objid[*objidlen] = subid;
            (*objidlen)++;
            break;
        case '"':
        case '\'':
            doingquote = *cp++;
            /*
             * insert length if requested 
             */
            if (doingquote == '"') {
                if (*objidlen >= maxlen)
                    goto bad_id;
                objid[*objidlen] = len = strchr(cp, doingquote) - cp;
                (*objidlen)++;
            }

            while (*cp && *cp != doingquote) {
                if (*objidlen >= maxlen)
                    goto bad_id;
                objid[*objidlen] = *cp++;
                (*objidlen)++;
            }
            if (!cp)
                goto bad_id;
            cp2 = cp + 1;
            if (!*cp2)
                cp2 = NULL;
            else if (*cp2 == '.')
                cp2++;
            else
                goto bad_id;
            break;
        default:
            goto bad_id;
        }
        cp = cp2;
    }
    return 1;

  bad_id:
    {
        char            buf[256];
        if (in_dices)
            snprintf(buf, sizeof(buf), "Index out of range: %s (%s)",
                    fcp, in_dices->ilabel);
        else if (tp)
            snprintf(buf, sizeof(buf), "Sub-id not found: %s -> %s", tp->label, fcp);
        else
            snprintf(buf, sizeof(buf), "%s", fcp);
        buf[ sizeof(buf)-1 ] = 0;

        snmp_set_detail(buf);
    }
    return 0;
}


/**
 * @see comments on find_best_tree_node for usage after first time.
 */
int
get_wild_node(const char *name, oid * objid, size_t * objidlen)
{
    struct tree    *tp = find_best_tree_node(name, tree_head, NULL);
    if (!tp)
        return 0;
    return get_node(tp->label, objid, objidlen);
}

int
get_node(const char *name, oid * objid, size_t * objidlen)
{
    const char     *cp;
    char            ch;
    int             res;

    cp = name;
    while ((ch = *cp))
        if (('0' <= ch && ch <= '9')
            || ('a' <= ch && ch <= 'z')
            || ('A' <= ch && ch <= 'Z')
            || ch == '-')
            cp++;
        else
            break;
    if (ch != ':')
        if (*name == '.')
            res = get_module_node(name + 1, "ANY", objid, objidlen);
        else
            res = get_module_node(name, "ANY", objid, objidlen);
    else {
        char           *module;
        /*
         *  requested name is of the form
         *      "module:subidentifier"
         */
        module = (char *) malloc((size_t) (cp - name + 1));
        if (!module)
            return SNMPERR_GENERR;
        memcpy(module, name, (size_t) (cp - name));
        module[cp - name] = 0;
        cp++;                   /* cp now point to the subidentifier */
        if (*cp == ':')
            cp++;

        /*
         * 'cp' and 'name' *do* go that way round! 
         */
        res = get_module_node(cp, module, objid, objidlen);
        free(module);
    }
    if (res == 0) {
        SET_SNMP_ERROR(SNMPERR_UNKNOWN_OBJID);
    }

    return res;
}

#ifdef testing

main(int argc, char *argv[])
{
    oid             objid[MAX_OID_LEN];
    int             objidlen = MAX_OID_LEN;
    int             count;
    netsnmp_variable_list variable;

    init_mib();
    if (argc < 2)
        print_subtree(stdout, tree_head, 0);
    variable.type = ASN_INTEGER;
    variable.val.integer = 3;
    variable.val_len = 4;
    for (argc--; argc; argc--, argv++) {
        objidlen = MAX_OID_LEN;
        printf("read_objid(%s) = %d\n",
               argv[1], read_objid(argv[1], objid, &objidlen));
        for (count = 0; count < objidlen; count++)
            printf("%d.", objid[count]);
        printf("\n");
        print_variable(objid, objidlen, &variable);
    }
}

#endif                          /* testing */

/*
 * initialize: no peers included in the report. 
 */
void
clear_tree_flags(register struct tree *tp)
{
    for (; tp; tp = tp->next_peer) {
        tp->reported = 0;
        if (tp->child_list)
            clear_tree_flags(tp->child_list);
     /*RECURSE*/}
}

/*
 * Update: 1998-07-17 <jhy@gsu.edu>
 * Added print_oid_report* functions.
 */
static int      print_subtree_oid_report_labeledoid = 0;
static int      print_subtree_oid_report_oid = 0;
static int      print_subtree_oid_report_symbolic = 0;
static int      print_subtree_oid_report_suffix = 0;

/*
 * These methods recurse. 
 */
static void     print_parent_labeledoid(FILE *, struct tree *);
static void     print_parent_oid(FILE *, struct tree *);
static void     print_parent_label(FILE *, struct tree *);
static void     print_subtree_oid_report(FILE *, struct tree *, int);


void
print_oid_report(FILE * fp)
{
    struct tree    *tp;
    clear_tree_flags(tree_head);
    for (tp = tree_head; tp; tp = tp->next_peer)
        print_subtree_oid_report(fp, tp, 0);
}

void
print_oid_report_enable_labeledoid(void)
{
    print_subtree_oid_report_labeledoid = 1;
}

void
print_oid_report_enable_oid(void)
{
    print_subtree_oid_report_oid = 1;
}

void
print_oid_report_enable_suffix(void)
{
    print_subtree_oid_report_suffix = 1;
}

void
print_oid_report_enable_symbolic(void)
{
    print_subtree_oid_report_symbolic = 1;
}

/*
 * helper methods for print_subtree_oid_report()
 * each one traverses back up the node tree
 * until there is no parent.  Then, the label combination
 * is output, such that the parent is displayed first.
 *
 * Warning: these methods are all recursive.
 */

static void
print_parent_labeledoid(FILE * f, struct tree *tp)
{
    if (tp) {
        if (tp->parent) {
            print_parent_labeledoid(f, tp->parent);
         /*RECURSE*/}
        fprintf(f, ".%s(%lu)", tp->label, tp->subid);
    }
}

static void
print_parent_oid(FILE * f, struct tree *tp)
{
    if (tp) {
        if (tp->parent) {
            print_parent_oid(f, tp->parent);
         /*RECURSE*/}
        fprintf(f, ".%lu", tp->subid);
    }
}

static void
print_parent_label(FILE * f, struct tree *tp)
{
    if (tp) {
        if (tp->parent) {
            print_parent_label(f, tp->parent);
         /*RECURSE*/}
        fprintf(f, ".%s", tp->label);
    }
}

/**
 * @internal
 * This methods generates variations on the original print_subtree() report.
 * Traverse the tree depth first, from least to greatest sub-identifier.
 * Warning: this methods recurses and calls methods that recurse.
 *
 * @param f       File descriptor to print to.
 * @param tree    ???
 * @param count   ???
 */

static void
print_subtree_oid_report(FILE * f, struct tree *tree, int count)
{
    struct tree    *tp;

    count++;

    /*
     * sanity check 
     */
    if (!tree) {
        return;
    }

    /*
     * find the not reported peer with the lowest sub-identifier.
     * if no more, break the loop and cleanup.
     * set "reported" flag, and create report for this peer.
     * recurse using the children of this peer, if any.
     */
    while (1) {
        register struct tree *ntp;

        tp = 0;
        for (ntp = tree->child_list; ntp; ntp = ntp->next_peer) {
            if (ntp->reported)
                continue;

            if (!tp || (tp->subid > ntp->subid))
                tp = ntp;
        }
        if (!tp)
            break;

        tp->reported = 1;

        if (print_subtree_oid_report_labeledoid) {
            print_parent_labeledoid(f, tp);
            fprintf(f, "\n");
        }
        if (print_subtree_oid_report_oid) {
            print_parent_oid(f, tp);
            fprintf(f, "\n");
        }
        if (print_subtree_oid_report_symbolic) {
            print_parent_label(f, tp);
            fprintf(f, "\n");
        }
        if (print_subtree_oid_report_suffix) {
            int             i;
            for (i = 0; i < count; i++)
                fprintf(f, "  ");
            fprintf(f, "%s(%ld) type=%d", tp->label, tp->subid, tp->type);
            if (tp->tc_index != -1)
                fprintf(f, " tc=%d", tp->tc_index);
            if (tp->hint)
                fprintf(f, " hint=%s", tp->hint);
            if (tp->units)
                fprintf(f, " units=%s", tp->units);

            fprintf(f, "\n");
        }
        print_subtree_oid_report(f, tp, count);
     /*RECURSE*/}
}


/**
 * Converts timeticks to hours, minutes, seconds string.
 * CMU compatible does not show centiseconds.
 *
 * @param timeticks    The timeticks to convert.
 * @param buf          Buffer to write to, has to be at 
 *                     least 64 Bytes large.
 *       
 * @return The buffer
 *
 * @see uptimeString
 */
char           *
uptime_string(u_long timeticks, char *buf)
{
    char            tbuf[64];
    char           *cp;
    uptimeString(timeticks, tbuf);
    cp = strrchr(tbuf, '.');
#ifdef CMU_COMPATIBLE
    if (cp)
        *cp = '\0';
#endif
    strlcpy(buf, tbuf, sizeof(buf));
    return buf;
}

oid            *
snmp_parse_oid(const char *argv, oid * root, size_t * rootlen)
{
    size_t          savlen = *rootlen;
    if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_RANDOM_ACCESS)
        || strchr(argv, ':')) {
        if (get_node(argv, root, rootlen)) {
            return root;
        }
    } else if (netsnmp_ds_get_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_REGEX_ACCESS)) {
        if (get_wild_node(argv, root, rootlen)) {
            return root;
        }
    } else {
        if (read_objid(argv, root, rootlen)) {
            return root;
        }
        *rootlen = savlen;
        if (get_node(argv, root, rootlen)) {
            return root;
        }
        *rootlen = savlen;
        DEBUGMSGTL(("parse_oid", "wildly parsing\n"));
        if (get_wild_node(argv, root, rootlen)) {
            return root;
        }
    }
    return NULL;
}

/*
 * Use DISPLAY-HINT to parse a value into an octet string.
 *
 * note that "1d1d", "11" could have come from an octet string that
 * looked like { 1, 1 } or an octet string that looked like { 11 }
 * because of this, it's doubtful that anyone would use such a display
 * string. Therefore, the parser ignores this case.
 */

struct parse_hints {
    int length;
    int repeat;
    int format;
    int separator;
    int terminator;
    unsigned char *result;
    int result_max;
    int result_len;
};

static void parse_hints_reset(struct parse_hints *ph)
{
    ph->length = 0;
    ph->repeat = 0;
    ph->format = 0;
    ph->separator = 0;
    ph->terminator = 0;
}

static void parse_hints_ctor(struct parse_hints *ph)
{
    parse_hints_reset(ph);
    ph->result = NULL;
    ph->result_max = 0;
    ph->result_len = 0;
}

static int parse_hints_add_result_octet(struct parse_hints *ph, unsigned char octet)
{
    if (!(ph->result_len < ph->result_max)) {
	ph->result_max = ph->result_len + 32;
	if (!ph->result) {
	    ph->result = (unsigned char *)malloc(ph->result_max);
	} else {
	    ph->result = (unsigned char *)realloc(ph->result, ph->result_max);
	}
    }
    
    if (!ph->result) {
	return 0;		/* failed */
    }

    ph->result[ph->result_len++] = octet;
    return 1;			/* success */
}

static int parse_hints_parse(struct parse_hints *ph, const char **v_in_out)
{
    const char *v = *v_in_out;
    char *nv;
    int base;
    int repeats = 0;
    int repeat_fixup = ph->result_len;
    
    if (ph->repeat) {
	if (!parse_hints_add_result_octet(ph, 0)) {
	    return 0;
	}
    }
    do {
	base = 0;
	switch (ph->format) {
	case 'x': base += 6;	/* fall through */
	case 'd': base += 2;	/* fall through */
	case 'o': base += 8;	/* fall through */
	    {
		int i;
		unsigned long number = strtol(v, &nv, base);
		if (nv == v) return 0;
		v = nv;
		for (i = 0; i < ph->length; i++) {
		    int shift = 8 * (ph->length - 1 - i);
		    if (!parse_hints_add_result_octet(ph, (number >> shift) & 0xFF)) {
			return 0; /* failed */
		    }
		}
	    }
	    break;

	case 'a':
	    {
		int i;
		    
		for (i = 0; i < ph->length && *v; i++) {
		    if (!parse_hints_add_result_octet(ph, *v++)) {
			return 0;	/* failed */
		    }
		}
	    }
	    break;
	}

	repeats++;

	if (ph->separator && *v) {
	    if (*v == ph->separator) {
		v++;
	    } else {
		return 0;		/* failed */
	    }
	}

	if (ph->terminator) {
	    if (*v == ph->terminator) {
		v++;
		break;
	    }
	}
    } while (ph->repeat && *v);
    if (ph->repeat) {
	ph->result[repeat_fixup] = repeats;
    }

    *v_in_out = v;
    return 1;
}

static void parse_hints_length_add_digit(struct parse_hints *ph, int digit)
{
    ph->length *= 10;
    ph->length += digit - '0';
}

const char *parse_octet_hint(const char *hint, const char *value, unsigned char **new_val, int *new_val_len)
{
    const char *h = hint;
    const char *v = value;
    struct parse_hints ph;
    int retval = 1;
    /* See RFC 1443 */
    enum {
	HINT_1_2,
	HINT_2_3,
	HINT_1_2_4,
	HINT_1_2_5,
    } state = HINT_1_2;

    parse_hints_ctor(&ph);
    while (*h && *v && retval) {
	switch (state) {
	case HINT_1_2:
	    if ('*' == *h) {
		ph.repeat = 1;
		state = HINT_2_3;
	    } else if (isdigit(*h)) {
		parse_hints_length_add_digit(&ph, *h);
		state = HINT_2_3;
	    } else {
		return v;	/* failed */
	    }
	    break;

	case HINT_2_3:
	    if (isdigit(*h)) {
		parse_hints_length_add_digit(&ph, *h);
		/* state = HINT_2_3 */
	    } else if ('x' == *h || 'd' == *h || 'o' == *h || 'a' == *h) {
		ph.format = *h;
		state = HINT_1_2_4;
	    } else {
		return v;	/* failed */
	    }
	    break;

	case HINT_1_2_4:
	    if ('*' == *h) {
		retval = parse_hints_parse(&ph, &v);
		parse_hints_reset(&ph);
		
		ph.repeat = 1;
		state = HINT_2_3;
	    } else if (isdigit(*h)) {
		retval = parse_hints_parse(&ph, &v);
		parse_hints_reset(&ph);
		
		parse_hints_length_add_digit(&ph, *h);
		state = HINT_2_3;
	    } else {
		ph.separator = *h;
		state = HINT_1_2_5;
	    }
	    break;

	case HINT_1_2_5:
	    if ('*' == *h) {
		retval = parse_hints_parse(&ph, &v);
		parse_hints_reset(&ph);
		
		ph.repeat = 1;
		state = HINT_2_3;
	    } else if (isdigit(*h)) {
		retval = parse_hints_parse(&ph, &v);
		parse_hints_reset(&ph);
		
		parse_hints_length_add_digit(&ph, *h);
		state = HINT_2_3;
	    } else {
		ph.terminator = *h;

		retval = parse_hints_parse(&ph, &v);
		parse_hints_reset(&ph);

		state = HINT_1_2;
	    }
	    break;
	}
	h++;
    }
    while (*v && retval) {
	retval = parse_hints_parse(&ph, &v);
    }
    if (retval) {
	*new_val = ph.result;
	*new_val_len = ph.result_len;
    } else {
	if (ph.result) {
	    free(ph.result);
	}
	*new_val = NULL;
	*new_val_len = 0;
    }
    return retval ? NULL : v;
}

#ifdef test_display_hint

int main(int argc, const char **argv)
{
    const char *hint;
    const char *value;
    unsigned char *new_val;
    int new_val_len;
    char *r;
    
    if (argc < 3) {
	fprintf(stderr, "usage: dh <hint> <value>\n");
	exit(2);
    }
    hint = argv[1];
    value = argv[2];
    r = parse_octet_hint(hint, value, &new_val, &new_val_len);
    printf("{\"%s\", \"%s\"}: \n\t", hint, value);
    if (r) {
        *r = 0;
    	printf("returned failed\n");
	printf("value syntax error at: %s\n", value);
    }
    else {
	int i;
	printf("returned success\n");
	for (i = 0; i < new_val_len; i++) {
	    int c = new_val[i] & 0xFF;
	    printf("%02X(%c) ", c, isprint(c) ? c : ' ');
	}
	free(new_val);
    }
    printf("\n");
    exit(0);
}

#endif /* test_display_hint */

u_char
mib_to_asn_type(int mib_type)
{
    switch (mib_type) {
    case TYPE_OBJID:
        return ASN_OBJECT_ID;

    case TYPE_OCTETSTR:
    case TYPE_IPADDR:
        return ASN_OCTET_STR;

    case TYPE_NETADDR:
        return ASN_IPADDRESS;

    case TYPE_INTEGER32:
    case TYPE_INTEGER:
        return ASN_INTEGER;

    case TYPE_COUNTER:
        return ASN_COUNTER;

    case TYPE_GAUGE:
        return ASN_GAUGE;

    case TYPE_TIMETICKS:
        return ASN_TIMETICKS;

    case TYPE_OPAQUE:
        return ASN_OPAQUE;

    case TYPE_NULL:
        return ASN_NULL;

    case TYPE_COUNTER64:
        return ASN_COUNTER64;

    case TYPE_BITSTRING:
        return ASN_BIT_STR;

    case TYPE_UINTEGER:
    case TYPE_UNSIGNED32:
        return ASN_UINTEGER;

    case TYPE_NSAPADDRESS:
        return ASN_NSAP;

    }
    return -1;
}

/**
 * Converts a string to its OID form.
 * in example  "hello" = 5 . 'h' . 'e' . 'l' . 'l' . 'o'
 *
 * @param S   The string.
 * @param O   The oid.
 * @param L   The length of the oid.
 *
 * @return 0 on Sucess, 1 on failure.
 */
int
netsnmp_str2oid(const char *S, oid * O, int L)
{
    const char     *c = S;
    oid            *o = &O[1];

    --L;                        /* leave room for length prefix */

    for (; *c && L; --L, ++o, ++c)
        *o = *c;

    /*
     * make sure we got to the end of the string 
     */
    if (*c != 0)
        return 1;

    /*
     * set the length of the oid 
     */
    *O = c - S;

    return 0;
}

/**
 * Converts an OID to its character form.
 * in example  5 . 1 . 2 . 3 . 4 . 5 = 12345
 *
 * @param C   The character buffer.
 * @param L   The length of the buffer.
 * @param O   The oid.
 *
 * @return 0 on Sucess, 1 on failure.
 */
int
netsnmp_oid2chars(char *C, int L, const oid * O)
{
    char           *c = C;
    const oid      *o = &O[1];

    if (L < *O)
        return 1;

    L = *O; /** length */
    for (; L; --L, ++o, ++c) {
        if (*o > 0xFF)
            return 1;
        *c = *o;
    }
    return 0;
}

/**
 * Converts an OID to its string form.
 * in example  5 . 'h' . 'e' . 'l' . 'l' . 'o' = "hello\0" (null terminated)
 *
 * @param S   The character string buffer.
 * @param L   The length of the string buffer.
 * @param O   The oid.
 *
 * @return 0 on Sucess, 1 on failure.
 */
int
netsnmp_oid2str(char *S, int L, oid * O)
{
    int            rc;

    if (L <= *O)
        return 1;

    rc = netsnmp_oid2chars(S, L, O);
    if (rc)
        return 1;

    S[ *O ] = 0;

    return 0;
}

int
snprint_by_type(char *buf, size_t buf_len,
                netsnmp_variable_list * var,
                const struct enum_list *enums,
                const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_by_type((u_char **) & buf, &buf_len, &out_len, 0,
                               var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_hexstring(char *buf, size_t buf_len, const u_char * cp, size_t len)
{
    size_t          out_len = 0;
    if (sprint_realloc_hexstring((u_char **) & buf, &buf_len, &out_len, 0,
                                 cp, len))
        return (int) out_len;
    else
        return -1;
}

int
snprint_asciistring(char *buf, size_t buf_len,
                    const u_char * cp, size_t len)
{
    size_t          out_len = 0;
    if (sprint_realloc_asciistring
        ((u_char **) & buf, &buf_len, &out_len, 0, cp, len))
        return (int) out_len;
    else
        return -1;
}

int
snprint_octet_string(char *buf, size_t buf_len,
                     const netsnmp_variable_list * var, const struct enum_list *enums,
                     const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_octet_string
        ((u_char **) & buf, &buf_len, &out_len, 0, var, enums, hint,
         units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_opaque(char *buf, size_t buf_len,
               const netsnmp_variable_list * var, const struct enum_list *enums,
               const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_opaque((u_char **) & buf, &buf_len, &out_len, 0,
                              var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_object_identifier(char *buf, size_t buf_len,
                          const netsnmp_variable_list * var,
                          const struct enum_list *enums, const char *hint,
                          const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_object_identifier
        ((u_char **) & buf, &buf_len, &out_len, 0, var, enums, hint,
         units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_timeticks(char *buf, size_t buf_len,
                  const netsnmp_variable_list * var, const struct enum_list *enums,
                  const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_timeticks((u_char **) & buf, &buf_len, &out_len, 0,
                                 var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_hinted_integer(char *buf, size_t buf_len,
                       long val, const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_hinted_integer
        ((u_char **) & buf, &buf_len, &out_len, 0, val, 'd', hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_integer(char *buf, size_t buf_len,
                const netsnmp_variable_list * var, const struct enum_list *enums,
                const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_integer((u_char **) & buf, &buf_len, &out_len, 0,
                               var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_uinteger(char *buf, size_t buf_len,
                 const netsnmp_variable_list * var, const struct enum_list *enums,
                 const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_uinteger((u_char **) & buf, &buf_len, &out_len, 0,
                                var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_gauge(char *buf, size_t buf_len,
              const netsnmp_variable_list * var, const struct enum_list *enums,
              const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_gauge((u_char **) & buf, &buf_len, &out_len, 0,
                             var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_counter(char *buf, size_t buf_len,
                const netsnmp_variable_list * var, const struct enum_list *enums,
                const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_counter((u_char **) & buf, &buf_len, &out_len, 0,
                               var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_networkaddress(char *buf, size_t buf_len,
                       const netsnmp_variable_list * var,
                       const struct enum_list *enums, const char *hint,
                       const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_networkaddress
        ((u_char **) & buf, &buf_len, &out_len, 0, var, enums, hint,
         units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_ipaddress(char *buf, size_t buf_len,
                  const netsnmp_variable_list * var, const struct enum_list *enums,
                  const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_ipaddress((u_char **) & buf, &buf_len, &out_len, 0,
                                 var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_null(char *buf, size_t buf_len,
             const netsnmp_variable_list * var, const struct enum_list *enums,
             const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_null((u_char **) & buf, &buf_len, &out_len, 0,
                            var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_bitstring(char *buf, size_t buf_len,
                  const netsnmp_variable_list * var, const struct enum_list *enums,
                  const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_bitstring((u_char **) & buf, &buf_len, &out_len, 0,
                                 var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_nsapaddress(char *buf, size_t buf_len,
                    const netsnmp_variable_list * var, const struct enum_list *enums,
                    const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_nsapaddress
        ((u_char **) & buf, &buf_len, &out_len, 0, var, enums, hint,
         units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_counter64(char *buf, size_t buf_len,
                  const netsnmp_variable_list * var, const struct enum_list *enums,
                  const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_counter64((u_char **) & buf, &buf_len, &out_len, 0,
                                 var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_badtype(char *buf, size_t buf_len,
                const netsnmp_variable_list * var, const struct enum_list *enums,
                const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_badtype((u_char **) & buf, &buf_len, &out_len, 0,
                               var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

#ifdef OPAQUE_SPECIAL_TYPES
int
snprint_float(char *buf, size_t buf_len,
              const netsnmp_variable_list * var, const struct enum_list *enums,
              const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_float((u_char **) & buf, &buf_len, &out_len, 0,
                             var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}

int
snprint_double(char *buf, size_t buf_len,
               const netsnmp_variable_list * var, const struct enum_list *enums,
               const char *hint, const char *units)
{
    size_t          out_len = 0;
    if (sprint_realloc_double((u_char **) & buf, &buf_len, &out_len, 0,
                              var, enums, hint, units))
        return (int) out_len;
    else
        return -1;
}
#endif

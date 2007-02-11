/*
 * tools.c
 */

#define NETSNMP_TOOLS_C 1 /* dont re-define malloc wrappers here */

#include <net-snmp/net-snmp-config.h>

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
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
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef cygwin
#include <windows.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/types.h>
#include <net-snmp/output_api.h>
#include <net-snmp/utilities.h>
#include <net-snmp/library/tools.h>     /* for "internal" definitions */

#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/mib.h>
#include <net-snmp/library/scapi.h>

#ifdef WIN32
/**
 * This function is a wrapper for the strdup function.
 */
char * netsnmp_strdup( const char * ptr)
{
    return strdup(ptr);
}
/**
 * This function is a wrapper for the calloc function.
 */
void * netsnmp_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

/**
 * This function is a wrapper for the malloc function.
 */
void * netsnmp_malloc(size_t size)
{
    return malloc(size);
}

/**
 * This function is a wrapper for the realloc function.
 */
void * netsnmp_realloc( void * ptr, size_t size)
{
    return realloc(ptr, size);
}

/**
 * This function is a wrapper for the free function.
 * It calls free only if the calling parameter has a non-zero value.
 */
void netsnmp_free( void * ptr)
{
    if (ptr)
        free(ptr);
}
#endif /* WIN32 */

/**
 * This function increase the size of the buffer pointed at by *buf, which is
 * initially of size *buf_len.  Contents are preserved **AT THE BOTTOM END OF
 * THE BUFFER**.  If memory can be (re-)allocated then it returns 1, else it
 * returns 0.
 * 
 * @param buf  pointer to a buffer pointer
 * @param buf_len      pointer to current size of buffer in bytes
 * 
 * @note
 * The current re-allocation algorithm is to increase the buffer size by
 * whichever is the greater of 256 bytes or the current buffer size, up to
 * a maximum increase of 8192 bytes.  
 */
int
snmp_realloc(u_char ** buf, size_t * buf_len)
{
    u_char         *new_buf = NULL;
    size_t          new_buf_len = 0;

    if (buf == NULL) {
        return 0;
    }

    if (*buf_len <= 255) {
        new_buf_len = *buf_len + 256;
    } else if (*buf_len > 255 && *buf_len <= 8191) {
        new_buf_len = *buf_len * 2;
    } else if (*buf_len > 8191) {
        new_buf_len = *buf_len + 8192;
    }

    if (*buf == NULL) {
        new_buf = (u_char *) malloc(new_buf_len);
    } else {
        new_buf = (u_char *) realloc(*buf, new_buf_len);
    }

    if (new_buf != NULL) {
        *buf = new_buf;
        *buf_len = new_buf_len;
        return 1;
    } else {
        return 0;
    }
}

int
snmp_strcat(u_char ** buf, size_t * buf_len, size_t * out_len,
            int allow_realloc, const u_char * s)
{
    if (buf == NULL || buf_len == NULL || out_len == NULL) {
        return 0;
    }

    if (s == NULL) {
        /*
         * Appending a NULL string always succeeds since it is a NOP.  
         */
        return 1;
    }

    while ((*out_len + strlen((const char *) s) + 1) >= *buf_len) {
        if (!(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
    }

    strcpy((char *) (*buf + *out_len), (const char *) s);
    *out_len += strlen((char *) (*buf + *out_len));
    return 1;
}

/** zeros memory before freeing it.
 *
 *	@param *buf	Pointer at bytes to free.
 *	@param size	Number of bytes in buf.
 */
void
free_zero(void *buf, size_t size)
{
    if (buf) {
        memset(buf, 0, size);
        free(buf);
    }

}                               /* end free_zero() */

/**
 * Returns pointer to allocaed & set buffer on success, size contains
 * number of random bytes filled.  buf is NULL and *size set to KMT
 * error value upon failure.
 *
 *	@param size	Number of bytes to malloc() and fill with random bytes.
 *
 * @return a malloced buffer
 *
 */
u_char         *
malloc_random(size_t * size)
{
    int             rval = SNMPERR_SUCCESS;
    u_char         *buf = (u_char *) calloc(1, *size);

    if (buf) {
        rval = sc_random(buf, size);

        if (rval < 0) {
            free_zero(buf, *size);
            buf = NULL;
        } else {
            *size = rval;
        }
    }

    return buf;

}                               /* end malloc_random() */

/** Duplicates a memory block.
 *  Copies a existing memory location from a pointer to another, newly
    malloced, pointer.

 *	@param to       Pointer to allocate and copy memory to.
 *      @param from     Pointer to copy memory from.
 *      @param size     Size of the data to be copied.
 *      
 *	@return SNMPERR_SUCCESS	on success, SNMPERR_GENERR on failure.
 */
int
memdup(u_char ** to, const u_char * from, size_t size)
{
    if (to == NULL)
        return SNMPERR_GENERR;
    if (from == NULL) {
        *to = NULL;
        return SNMPERR_SUCCESS;
    }
    if ((*to = (u_char *) malloc(size)) == NULL)
        return SNMPERR_GENERR;
    memcpy(*to, from, size);
    return SNMPERR_SUCCESS;

}                               /* end memdup() */

/** copies a (possible) unterminated string of a given length into a
 *  new buffer and null terminates it as well (new buffer MAY be one
 *  byte longer to account for this */
char           *
netsnmp_strdup_and_null(const u_char * from, size_t from_len)
{
    char         *ret;

    if (from_len == 0 || from[from_len - 1] != '\0') {
        ret = (char *)malloc(from_len + 1);
        if (!ret)
            return NULL;
        ret[from_len] = '\0';
    } else {
        ret = (char *)malloc(from_len);
        if (!ret)
            return NULL;
        ret[from_len - 1] = '\0';
    }
    memcpy(ret, from, from_len);
    return ret;
}

/** converts binary to hexidecimal
 *
 *	@param *input		Binary data.
 *	@param len		Length of binary data.
 *	@param **output	NULL terminated string equivalent in hex.
 *      
 * @return olen	Length of output string not including NULL terminator.
 *
 * FIX	Is there already one of these in the UCD SNMP codebase?
 *	The old one should be used, or this one should be moved to
 *	snmplib/snmp_api.c.
 */
u_int
binary_to_hex(const u_char * input, size_t len, char **output)
{
    u_int           olen = (len * 2) + 1;
    char           *s = (char *) calloc(1, olen), *op = s;
    const u_char   *ip = input;


    while (ip - input < (int) len) {
        *op++ = VAL2HEX((*ip >> 4) & 0xf);
        *op++ = VAL2HEX(*ip & 0xf);
        ip++;
    }
    *op = '\0';

    *output = s;
    return olen;

}                               /* end binary_to_hex() */




/**
 * hex_to_binary2
 *	@param *input		Printable data in base16.
 *	@param len		Length in bytes of data.
 *	@param **output	Binary data equivalent to input.
 *      
 * @return SNMPERR_GENERR on failure, otherwise length of allocated string.
 *
 * Input of an odd length is right aligned.
 *
 * FIX	Another version of "hex-to-binary" which takes odd length input
 *	strings.  It also allocates the memory to hold the binary data.
 *	Should be integrated with the official hex_to_binary() function.
 */
int
hex_to_binary2(const u_char * input, size_t len, char **output)
{
    u_int           olen = (len / 2) + (len % 2);
    char           *s = (char *) calloc(1, (olen) ? olen : 1), *op = s;
    const u_char   *ip = input;


    *output = NULL;
    *op = 0;
    if (len % 2) {
        if (!isxdigit(*ip))
            goto hex_to_binary2_quit;
        *op++ = HEX2VAL(*ip);
        ip++;
    }

    while (ip - input < (int) len) {
        if (!isxdigit(*ip))
            goto hex_to_binary2_quit;
        *op = HEX2VAL(*ip) << 4;
        ip++;

        if (!isxdigit(*ip))
            goto hex_to_binary2_quit;
        *op++ += HEX2VAL(*ip);
        ip++;
    }

    *output = s;
    return olen;

  hex_to_binary2_quit:
    free_zero(s, olen);
    return -1;

}                               /* end hex_to_binary2() */

int
snmp_decimal_to_binary(u_char ** buf, size_t * buf_len, size_t * out_len,
                       int allow_realloc, const char *decimal)
{
    int             subid = 0;
    const char     *cp = decimal;

    if (buf == NULL || buf_len == NULL || out_len == NULL
        || decimal == NULL) {
        return 0;
    }

    while (*cp != '\0') {
        if (isspace((int) *cp) || *cp == '.') {
            cp++;
            continue;
        }
        if (!isdigit((int) *cp)) {
            return 0;
        }
        if ((subid = atoi(cp)) > 255) {
            return 0;
        }
        if ((*out_len >= *buf_len) &&
            !(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
        *(*buf + *out_len) = (u_char) subid;
        (*out_len)++;
        while (isdigit((int) *cp)) {
            cp++;
        }
    }
    return 1;
}

/**
 * convert an ASCII hex string (with specified delimiters) to binary
 *
 * @param buf     address of a pointer (pointer to pointer) for the output buffer.
 *                If allow_realloc is set, the buffer may be grown via snmp_realloc
 *                to accomodate the data.
 *
 * @param buf_len pointer to a size_t containing the initial size of buf.
 *
 * @param offset On input, a pointer to a size_t indicating an offset into buf.
 *                The  binary data will be stored at this offset.
 *                On output, this pointer will have updated the offset to be
 *                the first byte after the converted data.
 *
 * @param allow_realloc If true, the buffer can be reallocated. If false, and
 *                      the buffer is not large enough to contain the string,
 *                      an error will be returned.
 *
 * @param hex     pointer to hex string to be converted. May be prefixed by
 *                "0x" or "0X".
 *
 * @param delim   point to a string of allowed delimiters between bytes.
 *                If not specified, any non-hex characters will be an error.
 *
 * @retval 1  success
 * @retval 0  error
 */
int
netsnmp_hex_to_binary(u_char ** buf, size_t * buf_len, size_t * offset,
                      int allow_realloc, const char *hex, const char *delim)
{
    int             subid = 0;
    const char     *cp = hex;

    if (buf == NULL || buf_len == NULL || offset == NULL || hex == NULL) {
        return 0;
    }

    if ((*cp == '0') && ((*(cp + 1) == 'x') || (*(cp + 1) == 'X'))) {
        cp += 2;
    }

    while (*cp != '\0') {
        if (!isxdigit((int) *cp)) {
            if ((NULL != delim) && (NULL != strchr(delim, *cp))) {
                cp++;
                continue;
            }
            return 0;
        }
        if (sscanf(cp, "%2x", &subid) == 0) {
            return 0;
        }
        /*
         * if we dont' have enough space, realloc.
         * (snmp_realloc will adjust buf_len to new size)
         */
        if ((*offset >= *buf_len) &&
            !(allow_realloc && snmp_realloc(buf, buf_len))) {
            return 0;
        }
        *(*buf + *offset) = (u_char) subid;
        (*offset)++;
        if (*++cp == '\0') {
            /*
             * Odd number of hex digits is an error.  
             */
            return 0;
        } else {
            cp++;
        }
    }
    return 1;
}

/**
 * convert an ASCII hex string to binary
 *
 * @note This is a wrapper which calls netsnmp_hex_to_binary with a
 * delimiter string of " ".
 *
 * See netsnmp_hex_to_binary for parameter descriptions.
 *
 * @retval 1  success
 * @retval 0  error
 */
int
snmp_hex_to_binary(u_char ** buf, size_t * buf_len, size_t * offset,
                   int allow_realloc, const char *hex)
{
    return netsnmp_hex_to_binary(buf, buf_len, offset, allow_realloc, hex, " ");
}

/*******************************************************************-o-******
 * dump_chunk
 *
 * Parameters:
 *	*title	(May be NULL.)
 *	*buf
 *	 size
 */
void
dump_chunk(const char *debugtoken, const char *title, const u_char * buf,
           int size)
{
    u_int           printunit = 64;     /* XXX  Make global. */
    char            chunk[SNMP_MAXBUF], *s, *sp;

    if (title && (*title != '\0')) {
        DEBUGMSGTL((debugtoken, "%s\n", title));
    }


    memset(chunk, 0, SNMP_MAXBUF);
    size = binary_to_hex(buf, size, &s);
    sp = s;

    while (size > 0) {
        if (size > (int) printunit) {
            strncpy(chunk, sp, printunit);
            chunk[printunit] = '\0';
            DEBUGMSGTL((debugtoken, "\t%s\n", chunk));
        } else {
            DEBUGMSGTL((debugtoken, "\t%s\n", sp));
        }

        sp += printunit;
        size -= printunit;
    }


    SNMP_FREE(s);

}                               /* end dump_chunk() */




/*******************************************************************-o-******
 * dump_snmpEngineID
 *
 * Parameters:
 *	*estring
 *	*estring_len
 *      
 * Returns:
 *	Allocated memory pointing to a string of buflen char representing
 *	a printf'able form of the snmpEngineID.
 *
 *	-OR- NULL on error.
 *
 *
 * Translates the snmpEngineID TC into a printable string.  From RFC 2271,
 * Section 5 (pp. 36-37):
 *
 * First bit:	0	Bit string structured by means non-SNMPv3.
 *  		1	Structure described by SNMPv3 SnmpEngineID TC.
 *  
 * Bytes 1-4:		Enterprise ID.  (High bit of first byte is ignored.)
 *  
 * Byte 5:	0	(RESERVED by IANA.)
 *  		1	IPv4 address.		(   4 octets)
 *  		2	IPv6 address.		(  16 octets)
 *  		3	MAC address.		(   6 octets)
 *  		4	Locally defined text.	(0-27 octets)
 *  		5	Locally defined octets.	(0-27 octets)
 *  		6-127	(RESERVED for enterprise.)
 *  
 * Bytes 6-32:		(Determined by byte 5.)
 *  
 *
 * Non-printable characters are given in hex.  Text is given in quotes.
 * IP and MAC addresses are given in standard (UN*X) conventions.  Sections
 * are comma separated.
 *
 * esp, remaining_len and s trace the state of the constructed buffer.
 * s will be defined if there is something to return, and it will point
 * to the end of the constructed buffer.
 *
 *
 * ASSUME  "Text" means printable characters.
 *
 * XXX	Must the snmpEngineID always have a minimum length of 12?
 *	(Cf. part 2 of the TC definition.)
 * XXX	Does not enforce upper-bound of 32 bytes.
 * XXX	Need a switch to decide whether to use DNS name instead of a simple
 *	IP address.
 *
 * FIX	Use something other than snprint_hexstring which doesn't add 
 *	trailing spaces and (sometimes embedded) newlines...
 */
#ifdef NETSNMP_ENABLE_TESTING_CODE
char           *
dump_snmpEngineID(const u_char * estring, size_t * estring_len)
{
#define eb(b)	( *(esp+b) & 0xff )

    int             rval = SNMPERR_SUCCESS, gotviolation = 0, slen = 0;
    u_int           remaining_len;

    char            buf[SNMP_MAXBUF], *s = NULL, *t;
    const u_char   *esp = estring;

    struct in_addr  iaddr;



    /*
     * Sanity check.
     */
    if (!estring || (*estring_len <= 0)) {
        QUITFUN(SNMPERR_GENERR, dump_snmpEngineID_quit);
    }
    remaining_len = *estring_len;
    memset(buf, 0, SNMP_MAXBUF);



    /*
     * Test first bit.  Return immediately with a hex string, or
     * begin by formatting the enterprise ID.
     */
    if (!(*esp & 0x80)) {
        snprint_hexstring(buf, SNMP_MAXBUF, esp, remaining_len);
        s = strchr(buf, '\0');
        s -= 1;
        goto dump_snmpEngineID_quit;
    }

    s = buf;
    s += sprintf(s, "enterprise %d, ", ((*(esp + 0) & 0x7f) << 24) |
                 ((*(esp + 1) & 0xff) << 16) |
                 ((*(esp + 2) & 0xff) << 8) | ((*(esp + 3) & 0xff)));
    /*
     * XXX  Ick. 
     */

    if (remaining_len < 5) {    /* XXX  Violating string. */
        goto dump_snmpEngineID_quit;
    }

    esp += 4;                   /* Incremented one more in the switch below. */
    remaining_len -= 5;



    /*
     * Act on the fifth byte.
     */
    switch ((int) *esp++) {
    case 1:                    /* IPv4 address. */

        if (remaining_len < 4)
            goto dump_snmpEngineID_violation;
        memcpy(&iaddr.s_addr, esp, 4);

        if (!(t = inet_ntoa(iaddr)))
            goto dump_snmpEngineID_violation;
        s += sprintf(s, "%s", t);

        esp += 4;
        remaining_len -= 4;
        break;

    case 2:                    /* IPv6 address. */

        if (remaining_len < 16)
            goto dump_snmpEngineID_violation;

        s += sprintf(s,
                     "%02X%02X %02X%02X %02X%02X %02X%02X::"
                     "%02X%02X %02X%02X %02X%02X %02X%02X",
                     eb(0), eb(1), eb(2), eb(3),
                     eb(4), eb(5), eb(6), eb(7),
                     eb(8), eb(9), eb(10), eb(11),
                     eb(12), eb(13), eb(14), eb(15));

        esp += 16;
        remaining_len -= 16;
        break;

    case 3:                    /* MAC address. */

        if (remaining_len < 6)
            goto dump_snmpEngineID_violation;

        s += sprintf(s, "%02X:%02X:%02X:%02X:%02X:%02X",
                     eb(0), eb(1), eb(2), eb(3), eb(4), eb(5));

        esp += 6;
        remaining_len -= 6;
        break;

    case 4:                    /* Text. */

        /*
         * Doesn't exist on all (many) architectures 
         */
        /*
         * s += snprintf(s, remaining_len+3, "\"%s\"", esp); 
         */
        s += sprintf(s, "\"%s\"", esp);
        goto dump_snmpEngineID_quit;
        break;
     /*NOTREACHED*/ case 5:    /* Octets. */

        snprint_hexstring(s, (SNMP_MAXBUF - (s-buf)),
                          esp, remaining_len);
        s = strchr(buf, '\0');
        s -= 1;
        goto dump_snmpEngineID_quit;
        break;
       /*NOTREACHED*/ dump_snmpEngineID_violation:
    case 0:                    /* Violation of RESERVED, 
                                 * *   -OR- of expected length.
                                 */
        gotviolation = 1;
        s += sprintf(s, "!!! ");

    default:                   /* Unknown encoding. */

        if (!gotviolation) {
            s += sprintf(s, "??? ");
        }
        snprint_hexstring(s, (SNMP_MAXBUF - (s-buf)),
                          esp, remaining_len);
        s = strchr(buf, '\0');
        s -= 1;

        goto dump_snmpEngineID_quit;

    }                           /* endswitch */



    /*
     * Cases 1-3 (IP and MAC addresses) should not have trailing
     * octets, but perhaps they do.  Throw them in too.  XXX
     */
    if (remaining_len > 0) {
        s += sprintf(s, " (??? ");

        snprint_hexstring(s, (SNMP_MAXBUF - (s-buf)),
                          esp, remaining_len);
        s = strchr(buf, '\0');
        s -= 1;

        s += sprintf(s, ")");
    }



  dump_snmpEngineID_quit:
    if (s) {
        slen = s - buf + 1;
        s = calloc(1, slen);
        memcpy(s, buf, (slen) - 1);
    }

    memset(buf, 0, SNMP_MAXBUF);        /* XXX -- Overkill? XXX: Yes! */

    return s;

#undef eb
}                               /* end dump_snmpEngineID() */
#endif                          /* NETSNMP_ENABLE_TESTING_CODE */


/**
 * create a new time marker.
 * NOTE: Caller must free time marker when no longer needed.
 */
marker_t
atime_newMarker(void)
{
    marker_t        pm = (marker_t) calloc(1, sizeof(struct timeval));
    gettimeofday((struct timeval *) pm, 0);
    return pm;
}

/**
 * set a time marker.
 */
void
atime_setMarker(marker_t pm)
{
    if (!pm)
        return;

    gettimeofday((struct timeval *) pm, 0);
}


/**
 * Returns the difference (in msec) between the two markers
 */
long
atime_diff(marker_t first, marker_t second)
{
    struct timeval *tv1, *tv2, diff;

    tv1 = (struct timeval *) first;
    tv2 = (struct timeval *) second;

    diff.tv_sec = tv2->tv_sec - tv1->tv_sec - 1;
    diff.tv_usec = tv2->tv_usec - tv1->tv_usec + 1000000;

    return (diff.tv_sec * 1000 + diff.tv_usec / 1000);
}

/**
 * Returns the difference (in u_long msec) between the two markers
 */
u_long
uatime_diff(marker_t first, marker_t second)
{
    struct timeval *tv1, *tv2, diff;

    tv1 = (struct timeval *) first;
    tv2 = (struct timeval *) second;

    diff.tv_sec = tv2->tv_sec - tv1->tv_sec - 1;
    diff.tv_usec = tv2->tv_usec - tv1->tv_usec + 1000000;

    return (((u_long) diff.tv_sec) * 1000 + diff.tv_usec / 1000);
}

/**
 * Returns the difference (in u_long 1/100th secs) between the two markers
 * (functionally this is what sysUpTime needs)
 */
u_long
uatime_hdiff(marker_t first, marker_t second)
{
    struct timeval *tv1, *tv2, diff;
    u_long          res;

    tv1 = (struct timeval *) first;
    tv2 = (struct timeval *) second;

    diff.tv_sec = tv2->tv_sec - tv1->tv_sec - 1;
    diff.tv_usec = tv2->tv_usec - tv1->tv_usec + 1000000;

    res = ((u_long) diff.tv_sec) * 100 + diff.tv_usec / 10000;
    return res;
}

/**
 * Test: Has (marked time plus delta) exceeded current time (in msec) ?
 * Returns 0 if test fails or cannot be tested (no marker).
 */
int
atime_ready(marker_t pm, int deltaT)
{
    marker_t        now;
    long            diff;
    if (!pm)
        return 0;

    now = atime_newMarker();

    diff = atime_diff(pm, now);
    free(now);
    if (diff < deltaT)
        return 0;

    return 1;
}

/**
 * Test: Has (marked time plus delta) exceeded current time (in msec) ?
 * Returns 0 if test fails or cannot be tested (no marker).
 */
int
uatime_ready(marker_t pm, unsigned int deltaT)
{
    marker_t        now;
    u_long          diff;
    if (!pm)
        return 0;

    now = atime_newMarker();

    diff = uatime_diff(pm, now);
    free(now);
    if (diff < deltaT)
        return 0;

    return 1;
}


        /*
         * Time-related utility functions
         */

/**
 * Return the number of timeTicks since the given marker 
 */
int
marker_tticks(marker_t pm)
{
    int             res;
    marker_t        now = atime_newMarker();

    res = atime_diff(pm, now);
    free(now);
    return res / 10;            /* atime_diff works in msec, not csec */
}

int
timeval_tticks(struct timeval *tv)
{
    return marker_tticks((marker_t) tv);
}

/**
 * Non Windows:  Returns a pointer to the desired environment variable  
 *               or NULL if the environment variable does not exist.  
 *               
 * Windows:      Returns a pointer to the desired environment variable  
 *               if it exists.  If it does not, the variable is looked up
 *               in the registry in HKCU\\Net-SNMP or HKLM\\Net-SNMP
 *               (whichever it finds first) and stores the result in the 
 *               environment variable.  It then returns a pointer to 
 *               environment variable.
 */

char *netsnmp_getenv(const char *name)
{
#if !defined (WIN32) && !defined (cygwin)
  return (getenv(name));
#else
  char *temp = NULL;  
  HKEY hKey;
  unsigned char * key_value = NULL;
  DWORD key_value_size = 0;
  DWORD key_value_type = 0;
  DWORD getenv_worked = 0;

  DEBUGMSGTL(("read_config", "netsnmp_getenv called with name: %s\n",name));

  if (!(name))
    return NULL;
  
  /* Try environment variable first */ 
  temp = getenv(name);
  if (temp) {
    getenv_worked = 1;
    DEBUGMSGTL(("read_config", "netsnmp_getenv will return from ENV: %s\n",temp));
  }
  
  /* Next try HKCU */
  if (temp == NULL)
  {
    if (RegOpenKeyExA(
          HKEY_CURRENT_USER, 
          "SOFTWARE\\Net-SNMP", 
          0, 
          KEY_QUERY_VALUE, 
          &hKey) == ERROR_SUCCESS) {   
      
      if (RegQueryValueExA(
            hKey, 
            name, 
            NULL, 
            &key_value_type, 
            NULL,               /* Just get the size */
            &key_value_size) == ERROR_SUCCESS) {

        if (key_value)
          SNMP_FREE(key_value);

        /* Allocate memory needed +1 to allow RegQueryValueExA to NULL terminate the
         * string data in registry is missing one (which is unlikely).
         */
        key_value = (char *) malloc((sizeof(char) * key_value_size)+sizeof(char));
        
        if (RegQueryValueExA(
              hKey, 
              name, 
              NULL, 
              &key_value_type, 
              key_value, 
              &key_value_size) == ERROR_SUCCESS) {
        }
        temp = key_value;
      }
      RegCloseKey(hKey);
      if (temp)
        DEBUGMSGTL(("read_config", "netsnmp_getenv will return from HKCU: %s\n",temp));
    }
  }

  /* Next try HKLM */
  if (temp == NULL)
  {
    if (RegOpenKeyExA(
          HKEY_LOCAL_MACHINE, 
          "SOFTWARE\\Net-SNMP", 
          0, 
          KEY_QUERY_VALUE, 
          &hKey) == ERROR_SUCCESS) {   
      
      if (RegQueryValueExA(
            hKey, 
            name, 
            NULL, 
            &key_value_type, 
            NULL,               /* Just get the size */
            &key_value_size) == ERROR_SUCCESS) {

        if (key_value)
          SNMP_FREE(key_value);

        /* Allocate memory needed +1 to allow RegQueryValueExA to NULL terminate the
         * string data in registry is missing one (which is unlikely).
         */
        key_value = (char *) malloc((sizeof(char) * key_value_size)+sizeof(char));
        
        if (RegQueryValueExA(
              hKey, 
              name, 
              NULL, 
              &key_value_type, 
              key_value, 
              &key_value_size) == ERROR_SUCCESS) {
        }
        temp = key_value;

      }
      RegCloseKey(hKey);
      if (temp)
        DEBUGMSGTL(("read_config", "netsnmp_getenv will return from HKLM: %s\n",temp));
    }
  }
  
  if (temp && !getenv_worked) {
    setenv(name, temp, 1);
    SNMP_FREE(temp);
  }

  DEBUGMSGTL(("read_config", "netsnmp_getenv returning: %s\n",getenv(name)));

  return(getenv(name));
#endif
}

/*
 * swap the order of an inet addr string
 */
int
netsnmp_addrstr_hton(char *ptr, size_t len)
{
#ifndef WORDS_BIGENDIAN
    char tmp[8];
    
    if (8 == len) {
        tmp[0] = ptr[6];
        tmp[1] = ptr[7];
        tmp[2] = ptr[4];
        tmp[3] = ptr[5];
        tmp[4] = ptr[2];
        tmp[5] = ptr[3];
        tmp[6] = ptr[0];
        tmp[7] = ptr[1];
        memcpy (ptr, &tmp, 8);
    }
    else if (32 == len) {
        netsnmp_addrstr_hton(ptr   , 8);
        netsnmp_addrstr_hton(ptr+8 , 8);
        netsnmp_addrstr_hton(ptr+16, 8);
        netsnmp_addrstr_hton(ptr+24, 8);
    }
    else
        return -1;
#endif

    return 0;
}

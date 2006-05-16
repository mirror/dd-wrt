/*
 * misctest.c
 *
 * Expected SUCCESSes for all tests:    0
 *
 * Returns:
 *      Number of FAILUREs.
 *
 * Test of dump_snmpEngineID().                 SUCCESSes:  0
 */

static char    *rcsid = "$Id: misctest.c,v 1.1.2.1 2004/06/20 21:55:08 nikki Exp $";     /* */

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "tools.h"
#include "transform_oids.h"
#include "callback.h"

#include <stdlib.h>

extern char    *optarg;
extern int      optind, optopt, opterr;



/*
 * Globals, &c...
 */
char           *local_progname;

#define USAGE	"Usage: %s [-h][-1a]"
#define OPTIONLIST	"1ah"

int             doalltests = 0, dodumpseid = 0;

#define	ALLOPTIONS	(doalltests + dodumpseid)



#define LOCAL_MAXBUF	(1024 * 8)
#define NL		"\n"

#define OUTPUT(o)	fprintf(stdout, "\n\n%s\n\n", o);

#define SUCCESS(s)					\
{							\
	if (!failcount)					\
		fprintf(stdout, "\nSUCCESS: %s\n", s);	\
}

#define FAILED(e, f)					\
{							\
	if (e != SNMPERR_SUCCESS) {			\
		fprintf(stdout, "\nFAILED: %s\n", f);	\
		failcount += 1;				\
	}						\
}




#define IDBLAT_4	"00010203"

#define IDVIOLATE1	"8000000300deedcafe"

#define IDIPv4		"80000003010a090807"
#define IDIPv6		"8000000302100f0e0d0c0b0a090807060504030201"
#define IDMAC		"8000000303ffeeddccbbaa"

#define IDTEXT		"8000000304"
#define PRINTABLE	"Let this be printable."

#define IDOCTETS_7	"80000003050001020304050607"

#define IDLOCAL_11	"8000000306000102030405060708090a0b"

#define IDIPv4_EXTRA3	"80000003010a090807010203"

#define ID_NUMSTRINGS		10




/*
 * Prototypes.
 */
void            usage(FILE * ofp);

int             test_dumpseid(void);





int
main(int argc, char **argv)
{
    int             rval = SNMPERR_SUCCESS, failcount = 0;
    char            ch;

    local_progname = argv[0];

    /*
     * Parse.
     */
    while ((ch = getopt(argc, argv, OPTIONLIST)) != EOF) {
        switch (ch) {
        case '1':
            dodumpseid = 1;
            break;
        case 'a':
            doalltests = 1;
            break;
        case 'h':
            rval = 0;
        default:
            usage(stdout);
            exit(rval);
        }

        argc -= 1;
        argv += 1;
        if (optarg) {
            argc -= 1;
            argv += 1;
            optarg = NULL;
        }
        optind = 1;
    }                           /* endwhile getopt */

    if ((argc > 1)) {
        usage(stdout);
        exit(1000);

    } else if (ALLOPTIONS != 1) {
        usage(stdout);
        exit(1000);
    }


    /*
     * Test stuff.
     */
    if (dodumpseid || doalltests) {
        failcount += test_dumpseid();
    }


    /*
     * Cleanup.
     */
    return failcount;

}                               /* end main() */





void
usage(FILE * ofp)
{
    fprintf(ofp,
            USAGE
            "" NL
            "	-1		Test dump_snmpEngineID()." NL
            "	-a		All tests." NL
            "	-h		Help." NL "" NL, local_progname);

}                               /* end usage() */




#ifdef EXAMPLE
/*******************************************************************-o-******
 * test_dosomething
 *
 * Returns:
 *	Number of failures.
 *
 *
 * Test template.
 */
int
test_dosomething(void)
{
    int             rval = SNMPERR_SUCCESS, failcount = 0;

    EM0(1, "UNIMPLEMENTED");    /* EM(1); /* */

  test_dosomething_quit:
    return failcount;

}                               /* end test_dosomething() */
#endif                          /* EXAMPLE */




/*******************************************************************-o-******
 * test_dumpseid
 *
 * Returns:
 *	Number of failures.
 *
 * Test dump_snmpEngineID().
 */
int
test_dumpseid(void)
{
    int                         /* rval = SNMPERR_SUCCESS, */
                    failcount = 0, tlen, count = 0;

    char            buf[SNMP_MAXBUF],
        *s, *t, *ris, *rawid_set[ID_NUMSTRINGS + 1] = {
        IDBLAT_4,
        IDVIOLATE1,
        IDIPv4,
        IDIPv6,
        IDMAC,
        IDTEXT,
        IDOCTETS_7,
        IDLOCAL_11,
        IDIPv4_EXTRA3,
        NULL
    };

    OUTPUT("Test of dump_snmpEngineID.  "
           "(Does not report failure or success.)");


    while ((ris = rawid_set[count++])) {
        tlen = hex_to_binary2(ris, strlen(ris), &t);

        if (ris == IDTEXT) {
            memset(buf, 0, SNMP_MAXBUF);
            memcpy(buf, t, tlen);
            tlen += sprintf(buf + tlen, "%s", PRINTABLE);

            SNMP_FREE(t);
            t = buf;
        }
#ifdef SNMP_TESTING_CODE
        s = dump_snmpEngineID(t, &tlen);
        printf("%s    (len=%d)\n", s, tlen);
#endif

        SNMP_FREE(s);
        if (t != buf) {
            SNMP_FREE(t);
        }
    }


    return failcount;

}                               /* end test_dumpseid() */

/*
 * scapitest.c
 *
 * Expected SUCCESSes:  2 + 10 + 1 for all tests.
 *
 * Returns:
 *      Number of FAILUREs.
 *
 *
 * ASSUMES  No key management functions return non-zero success codes.
 *
 * XXX  Split into individual modules?
 * XXX  Error/fringe conditions should be tested.
 *
 *
 * Test of sc_random.                                           SUCCESSes: 2.
 *      REQUIRES a human to spot check for obvious non-randomness...
 *
 * Test of sc_generate_keyed_hash and sc_check_keyed_hash.      SUCCESSes: 10.
 *
 * Test of sc_encrypt and sc_decrypt.                           SUCCESSes: 1.
 */

static char    *rcsid = "$Id: scapitest.c,v 5.0 2002/04/20 07:30:22 hardaker Exp $";    /* */


#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "keytools.h"
#include "tools.h"
#include "scapi.h"
#include "transform_oids.h"
#include "callback.h"

#include <stdlib.h>

extern char    *optarg;
extern int      optind, optopt, opterr;

#define DEBUG                   /* */



/*
 * Globals, &c...
 */
char           *local_progname;

#define USAGE	"Usage: %s [-h][-acHr]"
#define OPTIONLIST	"achHr"

int             doalltests = 0, docrypt = 0, dokeyedhash = 0, dorandom = 0;

#define	ALLOPTIONS	(doalltests + docrypt + dokeyedhash + dorandom)



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


#define BIGSTRING							\
    "   A port may be a pleasant retreat for any mind grown weary of"	\
    "the struggle for existence.  The vast expanse of sky, the"		\
    "mobile architecture of the clouds, the chameleon coloration"	\
    "of the sea, the beacons flashing on the shore, together make"	\
    "a prism which is marvellously calculated to entertain but not"	\
    "fatigue the eye.  The lofty ships with their complex webs of"	\
    "rigging, swayed to and fro by the swell in harmonious dance,"	\
    "all help to maintain a taste for rhythm and beauty in the"		\
    "mind.  And above all there is a mysterious, aristrocratic kind"	\
    "of pleasure to be had, for those who have lost all curiosity"	\
    "or ambition, as they strech on the belvedere or lean over the"	\
    "mole to watch the arrivals and departures of other men, those"	\
    "who still have sufficient strength of purpose in them, the"	\
    "urge to travel or enrich themselves."				\
    "	-- Baudelaire"							\
    "	   From _The_Poems_in_Prose_, \"The Port\" (XLI)."

#define BIGSECRET	"Shhhh... Don't tell *anyone* about this.  Not a soul."
#define BKWDSECRET	".luos a toN  .siht tuoba *enoyna* llet t'noD ...hhhhS"

#define MLCOUNT_MAX	6       /* MAC Length Count Maximum. */



/*
 * Prototypes.
 */
void            usage(FILE * ofp);

int             test_docrypt(void);
int             test_dokeyedhash(void);
int             test_dorandom(void);




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
        case 'a':
            doalltests = 1;
            break;
        case 'c':
            docrypt = 1;
            break;
        case 'H':
            dokeyedhash = 1;
            break;
        case 'r':
            dorandom = 1;
            break;
        case 'h':
            rval = 0;
        default:
            usage(stdout);
            exit(rval);
        }

        argc -= 1;
        argv += 1;
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
    rval = sc_init();
    FAILED(rval, "sc_init().");


    if (docrypt || doalltests) {
        failcount += test_docrypt();
    }
    if (dokeyedhash || doalltests) {
        failcount += test_dokeyedhash();
    }
    if (dorandom || doalltests) {
        failcount += test_dorandom();
    }


    /*
     * Cleanup.
     */
    rval = sc_shutdown(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_SHUTDOWN,
                       NULL, NULL);
    FAILED(rval, "sc_shutdown().");

    return failcount;

}                               /* end main() */





void
usage(FILE * ofp)
{
    fprintf(ofp,
            USAGE
            "" NL
            "	-a		All tests." NL
            "	-c		Test of sc_encrypt()/sc_decrypt()."
            NL
            "	-h		Help."
            NL
            "	-H              Test sc_{generate,check}_keyed_hash()."
            NL
            "	-r              Test sc_random()."
            NL "" NL, local_progname);

}                               /* end usage() */




#ifdef EXAMPLE
/*******************************************************************-o-******
 * test_dosomething
 *
 * Test template.
 *
 * Returns:
 *	Number of failures.
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
 * test_dorandom
 *
 * One large request, one set of short requests.
 *
 * Returns:
 *	Number of failures.
 *
 * XXX	probably should split up into individual options.
 */
int
test_dorandom(void)
{
    int             rval = SNMPERR_SUCCESS,
        failcount = 0,
        origrequest = (1024 * 2),
        origrequest_short = 19, nbytes = origrequest, shortcount = 7, i;
    char            buf[LOCAL_MAXBUF];

    OUTPUT("Random test -- large request:");

    rval = sc_random(buf, &nbytes);
    FAILED(rval, "sc_random().");

    if (nbytes != origrequest) {
        FAILED(SNMPERR_GENERR,
               "sc_random() returned different than requested.");
    }

    dump_chunk("scapitest", NULL, buf, nbytes);

    SUCCESS("Random test -- large request.");


    OUTPUT("Random test -- short requests:");
    origrequest_short = 16;

    for (i = 0; i < shortcount; i++) {
        nbytes = origrequest_short;
        rval = sc_random(buf, &nbytes);
        FAILED(rval, "sc_random().");

        if (nbytes != origrequest_short) {
            FAILED(SNMPERR_GENERR,
                   "sc_random() returned different " "than requested.");
        }

        dump_chunk("scapitest", NULL, buf, nbytes);
    }                           /* endfor */

    SUCCESS("Random test -- short requests.");


    return failcount;

}                               /* end test_dorandom() */



/*******************************************************************-o-******
 * test_dokeyedhash
 *
 * Returns:
 *	Number of failures.
 *
 *
 * Test keyed hashes with a variety of MAC length requests.
 *
 *
 * NOTE Both tests intentionally use the same secret
 *
 * FIX	Get input or output from some other package which hashes...
 * XXX	Could cut this in half with a little indirection...
 */
int
test_dokeyedhash(void)
{
    int             rval = SNMPERR_SUCCESS, failcount = 0, bigstring_len = strlen(BIGSTRING), secret_len = strlen(BIGSECRET), properlength, mlcount = 0,        /* MAC Length count.   */
                    hblen;      /* Hash Buffer length. */

    u_int           hashbuf_len[MLCOUNT_MAX] = {
        LOCAL_MAXBUF,
        BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1),
        BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5),
        BYTESIZE(SNMP_TRANS_AUTHLEN_HMAC96),
        7,
        0,
    };

    u_char          hashbuf[LOCAL_MAXBUF];
    char           *s;

  test_dokeyedhash_again:

    OUTPUT("Keyed hash test using MD5 --");

    memset(hashbuf, 0, LOCAL_MAXBUF);
    hblen = hashbuf_len[mlcount];
    properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5);

    rval =
        sc_generate_keyed_hash(usmHMACMD5AuthProtocol,
                               USM_LENGTH_OID_TRANSFORM, BIGSECRET,
                               secret_len, BIGSTRING, bigstring_len,
                               hashbuf, &hblen);
    FAILED(rval, "sc_generate_keyed_hash().");

    if (hashbuf_len[mlcount] > properlength) {
        if (hblen != properlength) {
            FAILED(SNMPERR_GENERR, "Wrong MD5 hash length returned.  (1)");
        }

    } else if (hblen != hashbuf_len[mlcount]) {
        FAILED(SNMPERR_GENERR, "Wrong MD5 hash length returned.  (2)");
    }

    rval =
        sc_check_keyed_hash(usmHMACMD5AuthProtocol,
                            USM_LENGTH_OID_TRANSFORM, BIGSECRET,
                            secret_len, BIGSTRING, bigstring_len, hashbuf,
                            hblen);
    FAILED(rval, "sc_check_keyed_hash().");

    binary_to_hex(hashbuf, hblen, &s);
    fprintf(stdout, "hash buffer (len=%d, request=%d):   %s\n",
            hblen, hashbuf_len[mlcount], s);
    SNMP_FREE(s);

    SUCCESS("Keyed hash test using MD5.");



    OUTPUT("Keyed hash test using SHA1 --");

    memset(hashbuf, 0, LOCAL_MAXBUF);
    hblen = hashbuf_len[mlcount];
    properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1);

    rval =
        sc_generate_keyed_hash(usmHMACSHA1AuthProtocol,
                               USM_LENGTH_OID_TRANSFORM, BIGSECRET,
                               secret_len, BIGSTRING, bigstring_len,
                               hashbuf, &hblen);
    FAILED(rval, "sc_generate_keyed_hash().");

    if (hashbuf_len[mlcount] > properlength) {
        if (hblen != properlength) {
            FAILED(SNMPERR_GENERR,
                   "Wrong SHA1 hash length returned.  (1)");
        }

    } else if (hblen != hashbuf_len[mlcount]) {
        FAILED(SNMPERR_GENERR, "Wrong SHA1 hash length returned.  (2)");
    }

    rval =
        sc_check_keyed_hash(usmHMACSHA1AuthProtocol,
                            USM_LENGTH_OID_TRANSFORM, BIGSECRET,
                            secret_len, BIGSTRING, bigstring_len, hashbuf,
                            hblen);
    FAILED(rval, "sc_check_keyed_hash().");

    binary_to_hex(hashbuf, hblen, &s);
    fprintf(stdout, "hash buffer (len=%d, request=%d):   %s\n",
            hblen, hashbuf_len[mlcount], s);
    SNMP_FREE(s);

    SUCCESS("Keyed hash test using SHA1.");



    /*
     * Run the basic hash tests but vary the size MAC requests.
     */
    if (hashbuf_len[++mlcount] != 0) {
        goto test_dokeyedhash_again;
    }


    return failcount;

}                               /* end test_dokeyedhash() */





/*******************************************************************-o-******
 * test_docrypt
 *
 * Returns:
 *	Number of failures.
 */
int
test_docrypt(void)
{
    int             rval = SNMPERR_SUCCESS,
        failcount = 0,
        bigstring_len = strlen(BIGSTRING),
        secret_len = BYTESIZE(SNMP_TRANS_PRIVLEN_1DES),
        iv_len = BYTESIZE(SNMP_TRANS_PRIVLEN_1DES_IV);

    u_int           buf_len = LOCAL_MAXBUF, cryptbuf_len = LOCAL_MAXBUF;

    char            buf[LOCAL_MAXBUF],
        cryptbuf[LOCAL_MAXBUF], secret[LOCAL_MAXBUF], iv[LOCAL_MAXBUF];

    OUTPUT("Test 1DES-CBC --");


    memset(buf, 0, LOCAL_MAXBUF);

    memcpy(secret, BIGSECRET, secret_len);
    memcpy(iv, BKWDSECRET, iv_len);


    rval = sc_encrypt(usmDESPrivProtocol, USM_LENGTH_OID_TRANSFORM,
                      secret, secret_len,
                      iv, iv_len,
                      BIGSTRING, bigstring_len, cryptbuf, &cryptbuf_len);
    FAILED(rval, "sc_encrypt().");

    rval = sc_decrypt(usmDESPrivProtocol, USM_LENGTH_OID_TRANSFORM,
                      secret, secret_len,
                      iv, iv_len, cryptbuf, cryptbuf_len, buf, &buf_len);
    FAILED(rval, "sc_decrypt().");

    if (buf_len != bigstring_len) {
        FAILED(SNMPERR_GENERR, "Decrypted buffer is the wrong length.");
    }
    if (memcmp(buf, BIGSTRING, bigstring_len)) {
        FAILED(SNMPERR_GENERR,
               "Decrypted buffer is not equal to original plaintext.");
    }


    SUCCESS("Test 1DES-CBC --");

    return failcount;

}                               /* end test_docrypt() */

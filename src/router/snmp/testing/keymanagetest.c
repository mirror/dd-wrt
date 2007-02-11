/*
 * keymanagetest.c
 *
 * Expected SUCCESSes:  2 + 2 + 3 for all tests.
 *
 * Returns:
 *      Number of FAILUREs.
 * 
 *
 * FIX  Or how about passing a usmUser name and looking up the entry as
 *      a means of getting key material?  This means the userList is
 *      available from an application...
 *
 * ASSUMES  No key management functions return non-zero success codes.
 *
 * Test of generate_Ku().                       SUCCESSes: 2
 * Test of generate_kul().                      SUCCESSes: 2
 * Test of {encode,decode}_keychange().         SUCCESSes: 3
 */

static char    *rcsid = "$Id: keymanagetest.c,v 5.0 2002/04/20 07:30:22 hardaker Exp $";        /* */

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



/*
 * Globals, &c...
 */
char           *local_progname;

#define USAGE	"Usage: %s [-h][-aklu][-E <engineID>][-N <newkey>][-O <oldkey>][-P <passphrase>]"
#define OPTIONLIST	"aqE:hklN:O:P:u"

int             doalltests = 0, dogenKu = 0, dogenkul = 0, dokeychange = 0;

#define	ALLOPTIONS	(doalltests + dogenKu + dogenkul + dokeychange)


#define LOCAL_MAXBUF	(1024 * 8)
#define NL		"\n"

#define OUTPUTALWAYS(o)	fprintf(stdout, "\n\n%s\n\n", o);
#define OUTPUT(o)	if (!bequiet) { OUTPUTALWAYS(o); }

#define SUCCESS(s)					\
{							\
	if (!failcount && !bequiet)					\
		fprintf(stdout, "\nSUCCESS: %s\n", s);	\
}

#define FAILED(e, f)					\
{							\
	if (e != SNMPERR_SUCCESS && !bequiet) {			\
		fprintf(stdout, "\nFAILED: %s\n", f);	\
		failcount += 1;				\
	}						\
}



/*
 * Test specific globals.
 */
#define	ENGINEID_DEFAULT	"1.2.3.4wild"
#define PASSPHRASE_DEFAULT	"Clay's Conclusion: Creativity is great, " \
					"but plagiarism is faster."
#define OLDKEY_DEFAULT		"This is a very old key."
#define NEWKEY_DEFAULT		"This key, on the other hand, is very new."

char           *engineID = NULL;
char           *passphrase = NULL;
char           *oldkey = NULL;
char           *newkey = NULL;
int             bequiet = 0;


/*
 * Prototypes.
 */
void            usage(FILE * ofp);

int             test_genkul(void);
int             test_genKu(void);
int             test_keychange(void);




int
main(int argc, char **argv)
{
    int             rval = SNMPERR_SUCCESS, failcount = 0;
    char            ch;

    local_progname = argv[0];
    optarg = NULL;

    /*
     * Parse.
     */
    while ((ch = getopt(argc, argv, OPTIONLIST)) != EOF) {
        switch (ch) {
        case 'a':
            doalltests = 1;
            break;
        case 'E':
            engineID = optarg;
            break;
        case 'k':
            dokeychange = 1;
            break;
        case 'l':
            dogenkul = 1;
            break;
        case 'N':
            newkey = optarg;
            break;
        case 'O':
            oldkey = optarg;
            break;
        case 'P':
            passphrase = optarg;
            break;
        case 'u':
            dogenKu = 1;
            break;
        case 'q':
            bequiet = 1;
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
        }

        optind = 1;
        optarg = NULL;
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

    if (dogenKu || doalltests) {
        failcount += test_genKu();
    }
    if (dogenkul || doalltests) {
        failcount += test_genkul();
    }
    if (dokeychange || doalltests) {
        failcount += test_keychange();
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
            "    -a			All tests." NL
            "    -E [0x]<engineID>	snmpEngineID string."
            NL
            "    -k			Test {encode,decode}_keychange()."
            NL
            "    -l			generate_kul()."
            NL
            "    -h			Help."
            NL
            "    -N [0x]<newkey>	New key (for testing KeyChange TC)."
            NL
            "    -O [0x]<oldkey>	Old key (for testing KeyChange TC)."
            NL
            "    -P <passphrase>	Source string for usmUser master key."
            NL
            "    -u			generate_Ku()."
            NL
            "    -q			be quiet."
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
 * test_genKu
 *
 * Returns:
 *	Number of failures.
 *
 *
 * Test generation of usmUser master key from a passphrase.
 *
 * ASSUMES  Passphrase is made of printable characters!
 */
int
test_genKu(void)
{
    int             rval = SNMPERR_SUCCESS,
        failcount = 0,
        properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5), kulen;
    char           *hashname = "usmHMACMD5AuthProtocol.", *s;
    u_char          Ku[LOCAL_MAXBUF];
    oid            *hashtype = usmHMACMD5AuthProtocol;

    OUTPUT("Test of generate_Ku --");

    /*
     * Set passphrase.
     */
    if (!passphrase) {
        passphrase = PASSPHRASE_DEFAULT;
    }
    if (!bequiet)
        fprintf(stdout, "Passphrase%s:\n\t%s\n\n",
                (passphrase == PASSPHRASE_DEFAULT) ? " (default)" : "",
                passphrase);


  test_genKu_again:
    memset(Ku, 0, LOCAL_MAXBUF);
    kulen = LOCAL_MAXBUF;

    rval = generate_Ku(hashtype, USM_LENGTH_OID_TRANSFORM,
                       passphrase, strlen(passphrase), Ku, &kulen);
    FAILED(rval, "generate_Ku().");

    if (kulen != properlength) {
        FAILED(SNMPERR_GENERR, "Ku length is wrong for this hashtype.");
    }

    binary_to_hex(Ku, kulen, &s);
    if (!bequiet)
        fprintf(stdout, "Ku (len=%d):  %s\n", kulen, s);
    free_zero(s, kulen);

    SUCCESS(hashname);
    if (!bequiet)
        fprintf(stdout, "\n");

    if (hashtype == usmHMACMD5AuthProtocol) {
        hashtype = usmHMACSHA1AuthProtocol;
        hashname = "usmHMACSHA1AuthProtocol.";
        properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1);
        goto test_genKu_again;
    }

    return failcount;

}                               /* end test_genKu() */




/*******************************************************************-o-******
 * test_genkul
 *
 * Returns:
 *	Number of failures.
 *
 *
 * Test of generate_kul().
 *
 * A passphrase and engineID are hashed into a master key Ku using
 * both known hash transforms.  Localized keys, also using both hash
 * transforms, are generated from each of these master keys.
 *
 * ASSUME  generate_Ku is already tested.
 * ASSUME  engineID is initially a NULL terminated string.
 */
int
test_genkul(void)
{
    int             rval = SNMPERR_SUCCESS,
        failcount = 0,
        properlength, kulen, kul_len, engineID_len, isdefault = FALSE;

    char           *s = NULL,
        *testname = "Using HMACMD5 to create master key.",
        *hashname_Ku = "usmHMACMD5AuthProtocol", *hashname_kul;

    u_char          Ku[LOCAL_MAXBUF], kul[LOCAL_MAXBUF];

    oid            *hashtype_Ku = usmHMACMD5AuthProtocol, *hashtype_kul;

    OUTPUT("Test of generate_kul --");


    /*
     * Set passphrase and engineID.
     *
     * If engineID begins with 0x, assume it is written in (printable)
     * hex and convert it to binary data.
     */
    if (!passphrase) {
        passphrase = PASSPHRASE_DEFAULT;
    }
    if (!bequiet)
        fprintf(stdout, "Passphrase%s:\n\t%s\n\n",
                (passphrase == PASSPHRASE_DEFAULT) ? " (default)" : "",
                passphrase);

    if (!engineID) {
        engineID = ENGINEID_DEFAULT;
        isdefault = TRUE;
    }

    engineID_len = strlen(engineID);

    if (tolower(*(engineID + 1)) == 'x') {
        engineID_len = hex_to_binary2(engineID + 2, engineID_len - 2, &s);
        if (engineID_len < 0) {
            FAILED((rval = SNMPERR_GENERR),
                   "Could not resolve hex engineID.");
        }
        engineID = s;
        binary_to_hex(engineID, engineID_len, &s);
    }

    if (!bequiet)
        fprintf(stdout, "engineID%s (len=%d):  %s\n\n",
                (isdefault) ? " (default)" : "",
                engineID_len, (s) ? s : engineID);
    if (s) {
        SNMP_FREE(s);
    }



    /*
     * Create a master key using both hash transforms; create localized
     * keys using both hash transforms from each master key.
     */
  test_genkul_again_master:
    memset(Ku, 0, LOCAL_MAXBUF);
    kulen = LOCAL_MAXBUF;
    hashname_kul = "usmHMACMD5AuthProtocol";
    hashtype_kul = usmHMACMD5AuthProtocol;
    properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5);


    rval = generate_Ku(hashtype_Ku, USM_LENGTH_OID_TRANSFORM,
                       passphrase, strlen(passphrase), Ku, &kulen);
    FAILED(rval, "generate_Ku().");

    binary_to_hex(Ku, kulen, &s);
    if (!bequiet)
        fprintf(stdout,
                "\n\nMaster Ku using \"%s\":\n\t%s\n\n", hashname_Ku, s);
    free_zero(s, kulen);


  test_genkul_again_local:
    memset(kul, 0, LOCAL_MAXBUF);
    kul_len = LOCAL_MAXBUF;

    rval = generate_kul(hashtype_kul, USM_LENGTH_OID_TRANSFORM,
                        engineID, engineID_len, Ku, kulen, kul, &kul_len);

    if ((hashtype_Ku == usmHMACMD5AuthProtocol)
        && (hashtype_kul == usmHMACSHA1AuthProtocol)) {
        if (rval == SNMPERR_SUCCESS) {
            FAILED(SNMPERR_GENERR,
                   "generate_kul SHOULD fail when Ku length is "
                   "less than hash transform length.");
        }

    } else {
        FAILED(rval, "generate_kul().");

        if (kul_len != properlength) {
            FAILED(SNMPERR_GENERR,
                   "kul length is wrong for the given hashtype.");
        }

        binary_to_hex(kul, kul_len, &s);
        fprintf(stdout, "kul (%s) (len=%d):  %s\n",
                ((hashtype_Ku == usmHMACMD5AuthProtocol) ? "MD5" : "SHA"),
                kul_len, s);
        free_zero(s, kul_len);
    }


    /*
     * Create localized key using the other hash transform, but from
     * * the same master key.
     */
    if (hashtype_kul == usmHMACMD5AuthProtocol) {
        hashtype_kul = usmHMACSHA1AuthProtocol;
        hashname_kul = "usmHMACSHA1AuthProtocol";
        properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1);
        goto test_genkul_again_local;
    }

    SUCCESS(testname);


    /*
     * Re-create the master key using the other hash transform.
     */
    if (hashtype_Ku == usmHMACMD5AuthProtocol) {
        hashtype_Ku = usmHMACSHA1AuthProtocol;
        hashname_Ku = "usmHMACSHA1AuthProtocol";
        testname = "Using HMACSHA1 to create master key.";
        goto test_genkul_again_master;
    }

    return failcount;

}                               /* end test_genkul() */




/*******************************************************************-o-******
 * test_keychange
 *
 * Returns:
 *	Number of failures.
 *
 *
 * Test of KeyChange TC implementation.
 *
 * ASSUME newkey and oldkey begin as NULL terminated strings.
 */
int
test_keychange(void)
{
    int             rval = SNMPERR_SUCCESS,
        failcount = 0,
        properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACMD5),
        oldkey_len,
        newkey_len,
        keychange_len,
        temp_len, isdefault_new = FALSE, isdefault_old = FALSE;

    char           *hashname = "usmHMACMD5AuthProtocol.", *s;

    u_char          oldkey_buf[LOCAL_MAXBUF],
        newkey_buf[LOCAL_MAXBUF],
        temp_buf[LOCAL_MAXBUF], keychange_buf[LOCAL_MAXBUF];

    oid            *hashtype = usmHMACMD5AuthProtocol;

    OUTPUT("Test of KeyChange TC --");


    /*
     * Set newkey and oldkey.
     */
    if (!newkey) {              /* newkey */
        newkey = NEWKEY_DEFAULT;
        isdefault_new = TRUE;
    }
    newkey_len = strlen(newkey);

    if (tolower(*(newkey + 1)) == 'x') {
        newkey_len = hex_to_binary2(newkey + 2, newkey_len - 2, &s);
        if (newkey_len < 0) {
            FAILED((rval = SNMPERR_GENERR),
                   "Could not resolve hex newkey.");
        }
        newkey = s;
        binary_to_hex(newkey, newkey_len, &s);
    }

    if (!oldkey) {              /* oldkey */
        oldkey = OLDKEY_DEFAULT;
        isdefault_old = TRUE;
    }
    oldkey_len = strlen(oldkey);

    if (tolower(*(oldkey + 1)) == 'x') {
        oldkey_len = hex_to_binary2(oldkey + 2, oldkey_len - 2, &s);
        if (oldkey_len < 0) {
            FAILED((rval = SNMPERR_GENERR),
                   "Could not resolve hex oldkey.");
        }
        oldkey = s;
        binary_to_hex(oldkey, oldkey_len, &s);
    }



  test_keychange_again:
    memset(oldkey_buf, 0, LOCAL_MAXBUF);
    memset(newkey_buf, 0, LOCAL_MAXBUF);
    memset(keychange_buf, 0, LOCAL_MAXBUF);
    memset(temp_buf, 0, LOCAL_MAXBUF);

    memcpy(oldkey_buf, oldkey, SNMP_MIN(oldkey_len, properlength));
    memcpy(newkey_buf, newkey, SNMP_MIN(newkey_len, properlength));
    keychange_len = LOCAL_MAXBUF;


    binary_to_hex(oldkey_buf, properlength, &s);
    fprintf(stdout, "\noldkey%s (len=%d):  %s\n",
            (isdefault_old) ? " (default)" : "", properlength, s);
    SNMP_FREE(s);

    binary_to_hex(newkey_buf, properlength, &s);
    fprintf(stdout, "newkey%s (len=%d):  %s\n\n",
            (isdefault_new) ? " (default)" : "", properlength, s);
    SNMP_FREE(s);


    rval = encode_keychange(hashtype, USM_LENGTH_OID_TRANSFORM,
                            oldkey_buf, properlength,
                            newkey_buf, properlength,
                            keychange_buf, &keychange_len);
    FAILED(rval, "encode_keychange().");

    if (keychange_len != (properlength * 2)) {
        FAILED(SNMPERR_GENERR,
               "KeyChange string (encoded) is not proper length "
               "for this hash transform.");
    }

    binary_to_hex(keychange_buf, keychange_len, &s);
    fprintf(stdout, "(%s) KeyChange string:  %s\n\n",
            ((hashtype == usmHMACMD5AuthProtocol) ? "MD5" : "SHA"), s);
    SNMP_FREE(s);

    temp_len = properlength;
    rval = decode_keychange(hashtype, USM_LENGTH_OID_TRANSFORM,
                            oldkey_buf, properlength,
                            keychange_buf, properlength * 2,
                            temp_buf, &temp_len);
    FAILED(rval, "decode_keychange().");

    if (temp_len != properlength) {
        FAILED(SNMPERR_GENERR,
               "decoded newkey is not proper length for "
               "this hash transform.");
    }

    binary_to_hex(temp_buf, temp_len, &s);
    fprintf(stdout, "decoded newkey:  %s\n\n", s);
    SNMP_FREE(s);


    if (memcmp(newkey_buf, temp_buf, temp_len)) {
        FAILED(SNMPERR_GENERR, "newkey did not decode properly.");
    }


    SUCCESS(hashname);
    fprintf(stdout, "\n");


    /*
     * Multiplex different test combinations.
     *
     * First clause is for Test #2, second clause is for (last) Test #3.
     */
    if (hashtype == usmHMACMD5AuthProtocol) {
        hashtype = usmHMACSHA1AuthProtocol;
        hashname = "usmHMACSHA1AuthProtocol (w/DES length kul's).";
        properlength = BYTESIZE(SNMP_TRANS_PRIVLEN_1DES)
            + BYTESIZE(SNMP_TRANS_PRIVLEN_1DES_IV);
        goto test_keychange_again;

    } else if (properlength < BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1)) {
        hashtype = usmHMACSHA1AuthProtocol;
        hashname = "usmHMACSHA1AuthProtocol.";
        properlength = BYTESIZE(SNMP_TRANS_AUTHLEN_HMACSHA1);
        goto test_keychange_again;
    }

    return failcount;

}                               /* end test_keychange() */

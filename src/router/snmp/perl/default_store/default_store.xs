#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <net-snmp/library/default_store.h>

static int
not_here(char *s)
{
    croak("%s not implemented on this architecture", s);
    return -1;
}

static double
constant_NETSNMP_DS_S(char *name, int len, int arg)
{
    if (12 + 12 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[12 + 12]) {
    case '1':
	if (strEQ(name + 12, "NMP_VERSION_1")) {	/* NETSNMP_DS_S removed */
#ifdef NETSNMP_DS_SNMP_VERSION_1
	    return NETSNMP_DS_SNMP_VERSION_1;
#else
	    goto not_there;
#endif
	}
    case '2':
	if (strEQ(name + 12, "NMP_VERSION_2c")) {	/* NETSNMP_DS_S removed */
#ifdef NETSNMP_DS_SNMP_VERSION_2c
	    return NETSNMP_DS_SNMP_VERSION_2c;
#else
	    goto not_there;
#endif
	}
    case '3':
	if (strEQ(name + 12, "NMP_VERSION_3")) {	/* NETSNMP_DS_S removed */
#ifdef NETSNMP_DS_SNMP_VERSION_3
	    return NETSNMP_DS_SNMP_VERSION_3;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_N(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'O':
	if (strEQ(name + 16, "O_TOKEN_WARNINGS")) {	/* NETSNMP_DS_LIB_N removed */
#ifdef NETSNMP_DS_LIB_NO_TOKEN_WARNINGS
	    return NETSNMP_DS_LIB_NO_TOKEN_WARNINGS;
#else
	    goto not_there;
#endif
	}
    case 'U':
	if (strEQ(name + 16, "UMERIC_TIMETICKS")) {	/* NETSNMP_DS_LIB_N removed */
#ifdef NETSNMP_DS_LIB_NUMERIC_TIMETICKS
	    return NETSNMP_DS_LIB_NUMERIC_TIMETICKS;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_O(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'I':
	if (strEQ(name + 16, "ID_OUTPUT_FORMAT")) {	/* NETSNMP_DS_LIB_O removed */
#ifdef NETSNMP_DS_LIB_OID_OUTPUT_FORMAT
	    return NETSNMP_DS_LIB_OID_OUTPUT_FORMAT;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (strEQ(name + 16, "PTIONALCONFIG")) {	/* NETSNMP_DS_LIB_O removed */
#ifdef NETSNMP_DS_LIB_OPTIONALCONFIG
	    return NETSNMP_DS_LIB_OPTIONALCONFIG;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_PRINT_N(char *name, int len, int arg)
{
    if (22 + 7 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[22 + 7]) {
    case 'E':
	if (strEQ(name + 22, "UMERIC_ENUM")) {	/* NETSNMP_DS_LIB_PRINT_N removed */
#ifdef NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM
	    return NETSNMP_DS_LIB_PRINT_NUMERIC_ENUM;
#else
	    goto not_there;
#endif
	}
    case 'O':
	if (strEQ(name + 22, "UMERIC_OIDS")) {	/* NETSNMP_DS_LIB_PRINT_N removed */
#ifdef NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS
	    return NETSNMP_DS_LIB_PRINT_NUMERIC_OIDS;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_PRIN(char *name, int len, int arg)
{
    if (19 + 2 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[19 + 2]) {
    case 'B':
	if (strEQ(name + 19, "T_BARE_VALUE")) {	/* NETSNMP_DS_LIB_PRIN removed */
#ifdef NETSNMP_DS_LIB_PRINT_BARE_VALUE
	    return NETSNMP_DS_LIB_PRINT_BARE_VALUE;
#else
	    goto not_there;
#endif
	}
    case 'F':
	if (strEQ(name + 19, "T_FULL_OID")) {	/* NETSNMP_DS_LIB_PRIN removed */
#ifdef NETSNMP_DS_LIB_PRINT_FULL_OID
	    return NETSNMP_DS_LIB_PRINT_FULL_OID;
#else
	    goto not_there;
#endif
	}
    case 'H':
	if (strEQ(name + 19, "T_HEX_TEXT")) {	/* NETSNMP_DS_LIB_PRIN removed */
#ifdef NETSNMP_DS_LIB_PRINT_HEX_TEXT
	    return NETSNMP_DS_LIB_PRINT_HEX_TEXT;
#else
	    goto not_there;
#endif
	}
    case 'N':
	if (!strnEQ(name + 19,"T_", 2))
	    break;
	return constant_NETSNMP_DS_LIB_PRINT_N(name, len, arg);
    case 'S':
	if (strEQ(name + 19, "T_SUFFIX_ONLY")) {	/* NETSNMP_DS_LIB_PRIN removed */
#ifdef NETSNMP_DS_LIB_PRINT_SUFFIX_ONLY
	    return NETSNMP_DS_LIB_PRINT_SUFFIX_ONLY;
#else
	    goto not_there;
#endif
	}
    case 'U':
	if (strEQ(name + 19, "T_UCD_STYLE_OID")) {	/* NETSNMP_DS_LIB_PRIN removed */
#ifdef NETSNMP_DS_LIB_PRINT_UCD_STYLE_OID
	    return NETSNMP_DS_LIB_PRINT_UCD_STYLE_OID;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_PR(char *name, int len, int arg)
{
    if (17 + 1 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[17 + 1]) {
    case 'N':
	if (!strnEQ(name + 17,"I", 1))
	    break;
	return constant_NETSNMP_DS_LIB_PRIN(name, len, arg);
    case 'V':
	if (strEQ(name + 17, "IVPASSPHRASE")) {	/* NETSNMP_DS_LIB_PR removed */
#ifdef NETSNMP_DS_LIB_PRIVPASSPHRASE
	    return NETSNMP_DS_LIB_PRIVPASSPHRASE;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_P(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'A':
	if (strEQ(name + 16, "ASSPHRASE")) {	/* NETSNMP_DS_LIB_P removed */
#ifdef NETSNMP_DS_LIB_PASSPHRASE
	    return NETSNMP_DS_LIB_PASSPHRASE;
#else
	    goto not_there;
#endif
	}
    case 'E':
	if (strEQ(name + 16, "ERSISTENT_DIR")) {	/* NETSNMP_DS_LIB_P removed */
#ifdef NETSNMP_DS_LIB_PERSISTENT_DIR
	    return NETSNMP_DS_LIB_PERSISTENT_DIR;
#else
	    goto not_there;
#endif
	}
    case 'R':
	return constant_NETSNMP_DS_LIB_PR(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_Q(char *name, int len, int arg)
{
    if (16 + 4 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[16 + 4]) {
    case 'E':
	if (strEQ(name + 16, "UICKE_PRINT")) {	/* NETSNMP_DS_LIB_Q removed */
#ifdef NETSNMP_DS_LIB_QUICKE_PRINT
	    return NETSNMP_DS_LIB_QUICKE_PRINT;
#else
	    goto not_there;
#endif
	}
    case '_':
	if (strEQ(name + 16, "UICK_PRINT")) {	/* NETSNMP_DS_LIB_Q removed */
#ifdef NETSNMP_DS_LIB_QUICK_PRINT
	    return NETSNMP_DS_LIB_QUICK_PRINT;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_A(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'L':
	if (strEQ(name + 16, "LARM_DONT_USE_SIG")) {	/* NETSNMP_DS_LIB_A removed */
#ifdef NETSNMP_DS_LIB_ALARM_DONT_USE_SIG
	    return NETSNMP_DS_LIB_ALARM_DONT_USE_SIG;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (strEQ(name + 16, "PPTYPE")) {	/* NETSNMP_DS_LIB_A removed */
#ifdef NETSNMP_DS_LIB_APPTYPE
	    return NETSNMP_DS_LIB_APPTYPE;
#else
	    goto not_there;
#endif
	}
    case 'U':
	if (strEQ(name + 16, "UTHPASSPHRASE")) {	/* NETSNMP_DS_LIB_A removed */
#ifdef NETSNMP_DS_LIB_AUTHPASSPHRASE
	    return NETSNMP_DS_LIB_AUTHPASSPHRASE;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_RE(char *name, int len, int arg)
{
    switch (name[17 + 0]) {
    case 'A':
	if (strEQ(name + 17, "AD_UCD_STYLE_OID")) {	/* NETSNMP_DS_LIB_RE removed */
#ifdef NETSNMP_DS_LIB_READ_UCD_STYLE_OID
	    return NETSNMP_DS_LIB_READ_UCD_STYLE_OID;
#else
	    goto not_there;
#endif
	}
    case 'G':
	if (strEQ(name + 17, "GEX_ACCESS")) {	/* NETSNMP_DS_LIB_RE removed */
#ifdef NETSNMP_DS_LIB_REGEX_ACCESS
	    return NETSNMP_DS_LIB_REGEX_ACCESS;
#else
	    goto not_there;
#endif
	}
    case 'V':
	if (strEQ(name + 17, "VERSE_ENCODE")) {	/* NETSNMP_DS_LIB_RE removed */
#ifdef NETSNMP_DS_LIB_REVERSE_ENCODE
	    return NETSNMP_DS_LIB_REVERSE_ENCODE;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_R(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'A':
	if (strEQ(name + 16, "ANDOM_ACCESS")) {	/* NETSNMP_DS_LIB_R removed */
#ifdef NETSNMP_DS_LIB_RANDOM_ACCESS
	    return NETSNMP_DS_LIB_RANDOM_ACCESS;
#else
	    goto not_there;
#endif
	}
    case 'E':
	return constant_NETSNMP_DS_LIB_RE(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_SE(char *name, int len, int arg)
{
    if (17 + 1 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[17 + 1]) {
    case 'L':
	if (strEQ(name + 17, "CLEVEL")) {	/* NETSNMP_DS_LIB_SE removed */
#ifdef NETSNMP_DS_LIB_SECLEVEL
	    return NETSNMP_DS_LIB_SECLEVEL;
#else
	    goto not_there;
#endif
	}
    case 'M':
	if (strEQ(name + 17, "CMODEL")) {	/* NETSNMP_DS_LIB_SE removed */
#ifdef NETSNMP_DS_LIB_SECMODEL
	    return NETSNMP_DS_LIB_SECMODEL;
#else
	    goto not_there;
#endif
	}
    case 'N':
	if (strEQ(name + 17, "CNAME")) {	/* NETSNMP_DS_LIB_SE removed */
#ifdef NETSNMP_DS_LIB_SECNAME
	    return NETSNMP_DS_LIB_SECNAME;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_S(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'A':
	if (strEQ(name + 16, "AVE_MIB_DESCRS")) {	/* NETSNMP_DS_LIB_S removed */
#ifdef NETSNMP_DS_LIB_SAVE_MIB_DESCRS
	    return NETSNMP_DS_LIB_SAVE_MIB_DESCRS;
#else
	    goto not_there;
#endif
	}
    case 'E':
	return constant_NETSNMP_DS_LIB_SE(name, len, arg);
    case 'N':
	if (strEQ(name + 16, "NMPVERSION")) {	/* NETSNMP_DS_LIB_S removed */
#ifdef NETSNMP_DS_LIB_SNMPVERSION
	    return NETSNMP_DS_LIB_SNMPVERSION;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_CON(char *name, int len, int arg)
{
    switch (name[18 + 0]) {
    case 'F':
	if (strEQ(name + 18, "FIGURATION_DIR")) {	/* NETSNMP_DS_LIB_CON removed */
#ifdef NETSNMP_DS_LIB_CONFIGURATION_DIR
	    return NETSNMP_DS_LIB_CONFIGURATION_DIR;
#else
	    goto not_there;
#endif
	}
    case 'T':
	if (strEQ(name + 18, "TEXT")) {	/* NETSNMP_DS_LIB_CON removed */
#ifdef NETSNMP_DS_LIB_CONTEXT
	    return NETSNMP_DS_LIB_CONTEXT;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_C(char *name, int len, int arg)
{
    if (16 + 1 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[16 + 1]) {
    case 'M':
	if (strEQ(name + 16, "OMMUNITY")) {	/* NETSNMP_DS_LIB_C removed */
#ifdef NETSNMP_DS_LIB_COMMUNITY
	    return NETSNMP_DS_LIB_COMMUNITY;
#else
	    goto not_there;
#endif
	}
    case 'N':
	if (!strnEQ(name + 16,"O", 1))
	    break;
	return constant_NETSNMP_DS_LIB_CON(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_DO(char *name, int len, int arg)
{
    if (17 + 3 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[17 + 3]) {
    case 'B':
	if (strEQ(name + 17, "NT_BREAKDOWN_OIDS")) {	/* NETSNMP_DS_LIB_DO removed */
#ifdef NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS
	    return NETSNMP_DS_LIB_DONT_BREAKDOWN_OIDS;
#else
	    goto not_there;
#endif
	}
    case 'C':
	if (strEQ(name + 17, "NT_CHECK_RANGE")) {	/* NETSNMP_DS_LIB_DO removed */
#ifdef NETSNMP_DS_LIB_DONT_CHECK_RANGE
	    return NETSNMP_DS_LIB_DONT_CHECK_RANGE;
#else
	    goto not_there;
#endif
	}
    case 'R':
	if (strEQ(name + 17, "NT_READ_CONFIGS")) {	/* NETSNMP_DS_LIB_DO removed */
#ifdef NETSNMP_DS_LIB_DONT_READ_CONFIGS
	    return NETSNMP_DS_LIB_DONT_READ_CONFIGS;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_D(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'E':
	if (strEQ(name + 16, "EFAULT_PORT")) {	/* NETSNMP_DS_LIB_D removed */
#ifdef NETSNMP_DS_LIB_DEFAULT_PORT
	    return NETSNMP_DS_LIB_DEFAULT_PORT;
#else
	    goto not_there;
#endif
	}
    case 'O':
	return constant_NETSNMP_DS_LIB_DO(name, len, arg);
    case 'U':
	if (strEQ(name + 16, "UMP_PACKET")) {	/* NETSNMP_DS_LIB_D removed */
#ifdef NETSNMP_DS_LIB_DUMP_PACKET
	    return NETSNMP_DS_LIB_DUMP_PACKET;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_E(char *name, int len, int arg)
{
    switch (name[16 + 0]) {
    case 'S':
	if (strEQ(name + 16, "SCAPE_QUOTES")) {	/* NETSNMP_DS_LIB_E removed */
#ifdef NETSNMP_DS_LIB_ESCAPE_QUOTES
	    return NETSNMP_DS_LIB_ESCAPE_QUOTES;
#else
	    goto not_there;
#endif
	}
    case 'X':
	if (strEQ(name + 16, "XTENDED_INDEX")) {	/* NETSNMP_DS_LIB_E removed */
#ifdef NETSNMP_DS_LIB_EXTENDED_INDEX
	    return NETSNMP_DS_LIB_EXTENDED_INDEX;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_H(char *name, int len, int arg)
{
    if (16 + 9 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[16 + 9]) {
    case 'C':
	if (strEQ(name + 16, "AVE_READ_CONFIG")) {	/* NETSNMP_DS_LIB_H removed */
#ifdef NETSNMP_DS_LIB_HAVE_READ_CONFIG
	    return NETSNMP_DS_LIB_HAVE_READ_CONFIG;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (strEQ(name + 16, "AVE_READ_PREMIB_CONFIG")) {	/* NETSNMP_DS_LIB_H removed */
#ifdef NETSNMP_DS_LIB_HAVE_READ_PREMIB_CONFIG
	    return NETSNMP_DS_LIB_HAVE_READ_PREMIB_CONFIG;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_MIB_(char *name, int len, int arg)
{
    switch (name[19 + 0]) {
    case 'C':
	if (strEQ(name + 19, "COMMENT_TERM")) {	/* NETSNMP_DS_LIB_MIB_ removed */
#ifdef NETSNMP_DS_LIB_MIB_COMMENT_TERM
	    return NETSNMP_DS_LIB_MIB_COMMENT_TERM;
#else
	    goto not_there;
#endif
	}
    case 'E':
	if (strEQ(name + 19, "ERRORS")) {	/* NETSNMP_DS_LIB_MIB_ removed */
#ifdef NETSNMP_DS_LIB_MIB_ERRORS
	    return NETSNMP_DS_LIB_MIB_ERRORS;
#else
	    goto not_there;
#endif
	}
    case 'P':
	if (strEQ(name + 19, "PARSE_LABEL")) {	/* NETSNMP_DS_LIB_MIB_ removed */
#ifdef NETSNMP_DS_LIB_MIB_PARSE_LABEL
	    return NETSNMP_DS_LIB_MIB_PARSE_LABEL;
#else
	    goto not_there;
#endif
	}
    case 'R':
	if (strEQ(name + 19, "REPLACE")) {	/* NETSNMP_DS_LIB_MIB_ removed */
#ifdef NETSNMP_DS_LIB_MIB_REPLACE
	    return NETSNMP_DS_LIB_MIB_REPLACE;
#else
	    goto not_there;
#endif
	}
    case 'W':
	if (strEQ(name + 19, "WARNINGS")) {	/* NETSNMP_DS_LIB_MIB_ removed */
#ifdef NETSNMP_DS_LIB_MIB_WARNINGS
	    return NETSNMP_DS_LIB_MIB_WARNINGS;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_M(char *name, int len, int arg)
{
    if (16 + 2 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[16 + 2]) {
    case 'D':
	if (strEQ(name + 16, "IBDIRS")) {	/* NETSNMP_DS_LIB_M removed */
#ifdef NETSNMP_DS_LIB_MIBDIRS
	    return NETSNMP_DS_LIB_MIBDIRS;
#else
	    goto not_there;
#endif
	}
    case '_':
	if (!strnEQ(name + 16,"IB", 2))
	    break;
	return constant_NETSNMP_DS_LIB_MIB_(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_LIB_(char *name, int len, int arg)
{
    switch (name[15 + 0]) {
    case 'A':
	return constant_NETSNMP_DS_LIB_A(name, len, arg);
    case 'C':
	return constant_NETSNMP_DS_LIB_C(name, len, arg);
    case 'D':
	return constant_NETSNMP_DS_LIB_D(name, len, arg);
    case 'E':
	return constant_NETSNMP_DS_LIB_E(name, len, arg);
    case 'H':
	return constant_NETSNMP_DS_LIB_H(name, len, arg);
    case 'L':
	if (strEQ(name + 15, "LOG_TIMESTAMP")) {	/* NETSNMP_DS_LIB_ removed */
#ifdef NETSNMP_DS_LIB_LOG_TIMESTAMP
	    return NETSNMP_DS_LIB_LOG_TIMESTAMP;
#else
	    goto not_there;
#endif
	}
    case 'M':
	return constant_NETSNMP_DS_LIB_M(name, len, arg);
    case 'N':
	return constant_NETSNMP_DS_LIB_N(name, len, arg);
    case 'O':
	return constant_NETSNMP_DS_LIB_O(name, len, arg);
    case 'P':
	return constant_NETSNMP_DS_LIB_P(name, len, arg);
    case 'Q':
	return constant_NETSNMP_DS_LIB_Q(name, len, arg);
    case 'R':
	return constant_NETSNMP_DS_LIB_R(name, len, arg);
    case 'S':
	return constant_NETSNMP_DS_LIB_S(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_L(char *name, int len, int arg)
{
    if (12 + 2 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[12 + 2]) {
    case 'R':
	if (strEQ(name + 12, "IBRARY_ID")) {	/* NETSNMP_DS_L removed */
#ifdef NETSNMP_DS_LIBRARY_ID
	    return NETSNMP_DS_LIBRARY_ID;
#else
	    goto not_there;
#endif
	}
    case '_':
	if (!strnEQ(name + 12,"IB", 2))
	    break;
	return constant_NETSNMP_DS_LIB_(name, len, arg);
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant_NETSNMP_DS_M(char *name, int len, int arg)
{
    if (12 + 3 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[12 + 3]) {
    case 'I':
	if (strEQ(name + 12, "AX_IDS")) {	/* NETSNMP_DS_M removed */
#ifdef NETSNMP_DS_MAX_IDS
	    return NETSNMP_DS_MAX_IDS;
#else
	    goto not_there;
#endif
	}
    case 'S':
	if (strEQ(name + 12, "AX_SUBIDS")) {	/* NETSNMP_DS_M removed */
#ifdef NETSNMP_DS_MAX_SUBIDS
	    return NETSNMP_DS_MAX_SUBIDS;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}

static double
constant(char *name, int len, int arg)
{
    errno = 0;
    if (0 + 11 >= len ) {
	errno = EINVAL;
	return 0;
    }
    switch (name[0 + 11]) {
    case 'A':
	if (strEQ(name + 0, "NETSNMP_DS_APPLICATION_ID")) {	/*  removed */
#ifdef NETSNMP_DS_APPLICATION_ID
	    return NETSNMP_DS_APPLICATION_ID;
#else
	    goto not_there;
#endif
	}
    case 'L':
	if (!strnEQ(name + 0,"NETSNMP_DS_", 11))
	    break;
	return constant_NETSNMP_DS_L(name, len, arg);
    case 'M':
	if (!strnEQ(name + 0,"NETSNMP_DS_", 11))
	    break;
	return constant_NETSNMP_DS_M(name, len, arg);
    case 'S':
	if (!strnEQ(name + 0,"NETSNMP_DS_", 11))
	    break;
	return constant_NETSNMP_DS_S(name, len, arg);
    case 'T':
	if (strEQ(name + 0, "NETSNMP_DS_TOKEN_ID")) {	/*  removed */
#ifdef NETSNMP_DS_TOKEN_ID
	    return NETSNMP_DS_TOKEN_ID;
#else
	    goto not_there;
#endif
	}
    }
    errno = EINVAL;
    return 0;

not_there:
    errno = ENOENT;
    return 0;
}


MODULE = NetSNMP::default_store		PACKAGE = NetSNMP::default_store		


double
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
	int		arg
    CODE:
	RETVAL = constant(s,len,arg);
    OUTPUT:
	RETVAL

int
netsnmp_ds_get_boolean(storeid, which)
	int	storeid
	int	which

int
netsnmp_ds_get_int(storeid, which)
	int	storeid
	int	which

char *
netsnmp_ds_get_string(storeid, which)
	int	storeid
	int	which

void *
netsnmp_ds_get_void(storeid, which)
	int	storeid
	int	which

int
netsnmp_ds_register_config(type, ftype, token, storeid, which)
	unsigned char	type
	const char *	ftype
	const char *	token
	int	storeid
	int	which

int
netsnmp_ds_register_premib(type, ftype, token, storeid, which)
	unsigned char	type
	const char *	ftype
	const char *	token
	int	storeid
	int	which

int
netsnmp_ds_set_boolean(storeid, which, value)
	int	storeid
	int	which
	int	value

int
netsnmp_ds_set_int(storeid, which, value)
	int	storeid
	int	which
	int	value

int
netsnmp_ds_set_string(storeid, which, value)
	int	storeid
	int	which
	const char *	value

int
netsnmp_ds_set_void(storeid, which, value)
	int	storeid
	int	which
	void *	value

void
netsnmp_ds_shutdown()

int
netsnmp_ds_toggle_boolean(storeid, which)
	int	storeid
	int	which

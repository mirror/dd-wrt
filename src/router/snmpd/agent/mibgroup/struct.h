#ifndef UCD_SNMP_STRUCT
#define UCD_SNMP_STRUCT

#define STRMAX 1024
#define SHPROC 1
#define EXECPROC 2
#define PASSTHRU 3
#define PASSTHRU_PERSIST 4
#define MIBMAX 30

struct extensible {
    char            name[STRMAX];
    char            command[STRMAX];
    char            fixcmd[STRMAX];
    int             type;
    int             result;
    char            output[STRMAX];
    struct extensible *next;
    unsigned long   miboid[MIBMAX];
    size_t          miblen;
    int             pid;
};

struct myproc {
    char            name[STRMAX];
    char            fixcmd[STRMAX];
    int             min;
    int             max;
    struct myproc  *next;
};

/*
 * struct mibinfo 
 * {
 * int numid;
 * unsigned long mibid[10];
 * char *name;
 * void (*handle) ();
 * };
 */

#endif

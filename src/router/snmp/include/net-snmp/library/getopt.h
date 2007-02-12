#ifndef _GETOPT_H_
#define _GETOPT_H_ 1

#ifdef __cplusplus
extern          "C" {
#endif

    extern int      getopt(int, char *const *, const char *);
    extern char    *optarg;
    extern int      optind, opterr, optopt, optreset;

#ifdef __cplusplus
}
#endif
#endif

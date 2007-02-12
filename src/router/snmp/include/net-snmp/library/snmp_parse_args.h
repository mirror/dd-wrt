#ifndef SNMP_PARSE_ARGS_H
#define SNMP_PARSE_ARGS_H
#ifdef __cplusplus
extern          "C" {
#endif

    int             snmp_parse_args(int, char *const *, netsnmp_session *,
                                    const char *, void (*)(int,
                                                           char *const *,
                                                           int));
    void            snmp_parse_args_descriptions(FILE *);
    void            snmp_parse_args_usage(FILE *);

#ifdef __cplusplus
}
#endif
#endif

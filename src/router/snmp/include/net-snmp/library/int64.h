#ifndef INT64_INCLUDED
#define INT64_INCLUDED

#ifdef __cplusplus
extern          "C" {
#endif

    typedef struct counter64 U64;

#define I64CHARSZ 21

    void            divBy10(U64, U64 *, unsigned int *);
    void            multBy10(U64, U64 *);
    void            incrByU16(U64 *, unsigned int);
    void            incrByU32(U64 *, unsigned int);
    void            zeroU64(U64 *);
    int             isZeroU64(U64 *);
    void            printU64(char *, U64 *);
    void            printI64(char *, U64 *);
    int             read64(U64 *, const char *);
    void            u64Subtract(U64 * pu64one, U64 * pu64two,
                                U64 * pu64out);

#ifdef __cplusplus
}
#endif
#endif

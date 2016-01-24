#ifndef HEXDUMP_H
#define HEXDUMP_H 1

#define PRNT_NONE 0
#define PRNT_SPAC 1
#define PRNT_HIGH 2

extern void ex_hexdump_reset(void);
extern int ex_hexdump_process(Vstr_base *, size_t,
                              Vstr_base *, size_t, size_t,
                              unsigned int, size_t, int, int);

#endif

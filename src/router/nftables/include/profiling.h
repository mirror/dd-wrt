#ifndef NFTABLES_PROFILING_H
#define NFTABLES_PROFILING_H

#ifdef BUILD_PROFILING
int get_signalfd(void);
void check_signalfd(int fd);
#else
static inline int get_signalfd(void) { return -1; }
static inline void check_signalfd(int fd) { /* empty */ }
#endif

#endif /* NFTABLES_PROFILING_H */

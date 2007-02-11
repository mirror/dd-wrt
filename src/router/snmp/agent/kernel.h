
#ifdef NETSNMP_CAN_USE_NLIST
extern void     init_kmem(const char *);
extern int      klookup(unsigned long, char *, int);
#define NETSNMP_KLOOKUP(x,y,z) klookup((unsigned long) x,y,z)
#else
#define NETSNMP_KLOOKUP(x,y,z) (0)
#endif

#if HAVE_KVM_H
#include <kvm.h>
extern kvm_t   *kd;
#endif

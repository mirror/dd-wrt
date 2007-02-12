
#ifndef linux
extern void     init_kmem(const char *);
extern int      klookup(unsigned long, char *, int);
#endif

#if HAVE_KVM_H
#include <kvm.h>
extern kvm_t   *kd;
#endif

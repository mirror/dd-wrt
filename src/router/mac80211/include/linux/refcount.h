/* Automatically created during backport process */
#ifndef CPTCFG_BPAUTO_REFCOUNT
#include_next <linux/refcount.h>
#else
#undef refcount_add_not_zero
#define refcount_add_not_zero LINUX_BACKPORT(refcount_add_not_zero)
#undef refcount_add
#define refcount_add LINUX_BACKPORT(refcount_add)
#undef refcount_inc_not_zero
#define refcount_inc_not_zero LINUX_BACKPORT(refcount_inc_not_zero)
#undef refcount_inc
#define refcount_inc LINUX_BACKPORT(refcount_inc)
#undef refcount_sub_and_test
#define refcount_sub_and_test LINUX_BACKPORT(refcount_sub_and_test)
#undef refcount_dec_and_test
#define refcount_dec_and_test LINUX_BACKPORT(refcount_dec_and_test)
#undef refcount_dec
#define refcount_dec LINUX_BACKPORT(refcount_dec)
#undef refcount_dec_if_one
#define refcount_dec_if_one LINUX_BACKPORT(refcount_dec_if_one)
#undef refcount_dec_not_one
#define refcount_dec_not_one LINUX_BACKPORT(refcount_dec_not_one)
#undef refcount_dec_and_mutex_lock
#define refcount_dec_and_mutex_lock LINUX_BACKPORT(refcount_dec_and_mutex_lock)
#undef refcount_dec_and_lock
#define refcount_dec_and_lock LINUX_BACKPORT(refcount_dec_and_lock)
#include <linux/backport-refcount.h>
#endif /* CPTCFG_BPAUTO_REFCOUNT */

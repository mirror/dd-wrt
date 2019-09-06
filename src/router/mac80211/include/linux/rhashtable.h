/* Automatically created during backport process */
#ifndef CPTCFG_BPAUTO_RHASHTABLE
#include_next <linux/rhashtable.h>
#else
#undef lockdep_rht_mutex_is_held
#define lockdep_rht_mutex_is_held LINUX_BACKPORT(lockdep_rht_mutex_is_held)
#undef lockdep_rht_bucket_is_held
#define lockdep_rht_bucket_is_held LINUX_BACKPORT(lockdep_rht_bucket_is_held)
#undef rhashtable_insert_slow
#define rhashtable_insert_slow LINUX_BACKPORT(rhashtable_insert_slow)
#undef rhashtable_walk_enter
#define rhashtable_walk_enter LINUX_BACKPORT(rhashtable_walk_enter)
#undef rhashtable_walk_exit
#define rhashtable_walk_exit LINUX_BACKPORT(rhashtable_walk_exit)
#undef rhashtable_walk_start
#define rhashtable_walk_start LINUX_BACKPORT(rhashtable_walk_start)
#undef rhashtable_walk_next
#define rhashtable_walk_next LINUX_BACKPORT(rhashtable_walk_next)
#undef rhashtable_walk_stop
#define rhashtable_walk_stop LINUX_BACKPORT(rhashtable_walk_stop)
#undef rhashtable_init
#define rhashtable_init LINUX_BACKPORT(rhashtable_init)
#undef rhltable_init
#define rhltable_init LINUX_BACKPORT(rhltable_init)
#undef rhashtable_free_and_destroy
#define rhashtable_free_and_destroy LINUX_BACKPORT(rhashtable_free_and_destroy)
#undef rhashtable_destroy
#define rhashtable_destroy LINUX_BACKPORT(rhashtable_destroy)
#include <linux/backport-rhashtable.h>
#endif /* CPTCFG_BPAUTO_RHASHTABLE */

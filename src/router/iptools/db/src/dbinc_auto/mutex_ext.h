/* DO NOT EDIT: automatically built by dist/s_include. */
#ifndef	_mutex_ext_h_
#define	_mutex_ext_h_

#if defined(__cplusplus)
extern "C" {
#endif

int __mutex_alloc __P((ENV *, int, u_int32_t, db_mutex_t *));
int __mutex_alloc_int __P((ENV *, int, int, u_int32_t, db_mutex_t *));
int __mutex_free __P((ENV *, db_mutex_t *));
int __mutex_free_int __P((ENV *, int, db_mutex_t *));
int __mutex_died __P((ENV *, db_mutex_t));
int __mutex_refresh __P((ENV *, db_mutex_t));
int __mutex_record_lock __P((ENV *, db_mutex_t, DB_THREAD_INFO *, MUTEX_ACTION, MUTEX_STATE **));
int __mutex_record_unlock __P((ENV *, db_mutex_t, DB_THREAD_INFO *));
int __mutex_record_print __P((ENV *, DB_THREAD_INFO *));
int __mutex_failchk __P((ENV *));
int __mutex_failchk_thread __P((ENV *, DB_THREAD_INFO *));
int __mutex_alloc_pp __P((DB_ENV *, u_int32_t, db_mutex_t *));
int __mutex_free_pp __P((DB_ENV *, db_mutex_t));
int __mutex_lock_pp __P((DB_ENV *, db_mutex_t));
int __mutex_unlock_pp __P((DB_ENV *, db_mutex_t));
int __mutex_get_align __P((DB_ENV *, u_int32_t *));
int __mutex_set_align __P((DB_ENV *, u_int32_t));
int __mutex_get_increment __P((DB_ENV *, u_int32_t *));
int __mutex_set_increment __P((DB_ENV *, u_int32_t));
int __mutex_get_init __P((DB_ENV *, u_int32_t *));
int __mutex_set_init __P((DB_ENV *, u_int32_t));
int __mutex_get_max __P((DB_ENV *, u_int32_t *));
int __mutex_set_max __P((DB_ENV *, u_int32_t));
int __mutex_get_tas_spins __P((DB_ENV *, u_int32_t *));
int __mutex_set_tas_spins __P((DB_ENV *, u_int32_t));
#ifdef HAVE_ERROR_HISTORY
int __mutex_diags __P((ENV *, db_mutex_t, int));
#endif
#if !defined(HAVE_ATOMIC_SUPPORT) && defined(HAVE_MUTEX_SUPPORT)
atomic_value_t __atomic_add_int __P((ENV *, db_atomic_t *, int));
#endif
#if !defined(HAVE_ATOMIC_SUPPORT) && defined(HAVE_MUTEX_SUPPORT)
int __atomic_compare_exchange_int __P((ENV *, db_atomic_t *, atomic_value_t, atomic_value_t));
#endif
int __db_pthread_mutex_init __P((ENV *, db_mutex_t, u_int32_t));
#ifndef HAVE_MUTEX_HYBRID
int __db_pthread_mutex_lock __P((ENV *, db_mutex_t, db_timeout_t, u_int32_t flags));
#endif
#if defined(HAVE_SHARED_LATCHES)
int __db_pthread_mutex_readlock __P((ENV *, db_mutex_t, u_int32_t));
#endif
#ifdef HAVE_MUTEX_HYBRID
int __db_hybrid_mutex_suspend  __P((ENV *, db_mutex_t, db_timespec *, DB_THREAD_INFO *, int));
#endif
int __db_pthread_mutex_unlock  __P((ENV *, db_mutex_t, DB_THREAD_INFO *, u_int32_t));
int __db_pthread_mutex_destroy __P((ENV *, db_mutex_t));
int __mutex_open __P((ENV *, int));
int __mutex_region_detach __P((ENV *, DB_MUTEXMGR *));
int __mutex_env_refresh __P((ENV *));
void __mutex_resource_return __P((ENV *, REGINFO *));
int __mutex_stat_pp __P((DB_ENV *, DB_MUTEX_STAT **, u_int32_t));
int __mutex_stat_print_pp __P((DB_ENV *, u_int32_t));
int __mutex_stat_print __P((ENV *, u_int32_t));
void __mutex_print_debug_single __P((ENV *, const char *, db_mutex_t, u_int32_t));
void __mutex_print_debug_stats __P((ENV *, DB_MSGBUF *, db_mutex_t, u_int32_t));
void __mutex_set_wait_info __P((ENV *, db_mutex_t, uintmax_t *, uintmax_t *));
void __mutex_clear __P((ENV *, db_mutex_t));
char *__mutex_describe __P((ENV *, db_mutex_t, char *));
int __db_tas_mutex_init __P((ENV *, db_mutex_t, u_int32_t));
int __db_tas_mutex_lock __P((ENV *, db_mutex_t, db_timeout_t, u_int32_t));
int __db_tas_mutex_readlock __P((ENV *, db_mutex_t, u_int32_t));
int __db_tas_mutex_unlock __P((ENV *, db_mutex_t, DB_THREAD_INFO *, u_int32_t));
int __db_tas_mutex_destroy __P((ENV *, db_mutex_t));
int __db_win32_mutex_lock __P((ENV *, db_mutex_t, db_timeout_t, int));
int __db_win32_mutex_init __P((ENV *, db_mutex_t, u_int32_t));
int __db_win32_mutex_readlock __P((ENV *, db_mutex_t, u_int32_t));
int __db_win32_mutex_unlock __P((ENV *, db_mutex_t, DB_THREAD_INFO *, u_int32_t));
int __db_win32_mutex_destroy __P((ENV *, db_mutex_t));

#if defined(__cplusplus)
}
#endif
#endif /* !_mutex_ext_h_ */

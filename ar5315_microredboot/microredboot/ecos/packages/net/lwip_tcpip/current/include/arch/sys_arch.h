#ifndef __SYS_ECOS_H__
#define __SYS_ECOS_H__

#include <cyg/kernel/kapi.h>

#define SYS_MBOX_NULL (sys_mbox_t)NULL 
#define SYS_SEM_NULL  (sys_sem_t)NULL

typedef cyg_sem_t * sys_sem_t;
typedef cyg_handle_t sys_mbox_t;
typedef cyg_thread * sys_thread_t;
#endif /* __SYS_ECOS_H__ */


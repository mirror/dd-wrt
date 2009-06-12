#ifndef __ASM_SEMAPHORE_H__
#define __ASM_SEMAPHORE_H__

#include <cyg/hal/drv_api.h>

struct semaphore {
	cyg_drv_mutex_t x;
};

#define DECLARE_MUTEX(x) struct semaphore x = { { 0 } };
#define DECLARE_MUTEX_LOCKED(x) struct semaphore x = { { 1 } };

#define init_MUTEX(sem) cyg_drv_mutex_init((cyg_drv_mutex_t *)sem)
#define init_MUTEX_LOCKED(sem) do { cyg_drv_mutex_init((cyg_drv_mutex_t *)sem); cyg_drv_mutex_lock((cyg_drv_mutex_t *)sem); } while(0)
#define down(sem) cyg_drv_mutex_lock((cyg_drv_mutex_t *)sem)
#define down_interruptible(sem) ({ cyg_drv_mutex_lock((cyg_drv_mutex_t *)sem), 0; })
#define down_trylock(sem) cyg_drv_mutex_trylock((cyg_drv_mutex_t *)sem)
#define up(sem) cyg_drv_mutex_unlock((cyg_drv_mutex_t *)sem)

#endif /* __ASM_SEMAPHORE_H__ */

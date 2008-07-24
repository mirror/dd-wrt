
#ifndef _NPREAL2_H
#define _NPREAL2_H

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#define CLEAR_FUNC 	cleanup_module
#define CLEAR_FUNC_RET	void
#else
#define CLEAR_FUNC 	npreal2_module_exit
#define CLEAR_FUNC_RET	static void __exit
#endif


#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#define INIT_FUNC 	init_module
#define INIT_FUNC_RET	int
#else
#define INIT_FUNC 	npreal2_module_init
#define INIT_FUNC_RET	static int __init
#endif


#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#define DRV_VAR		(&npvar_sdriver)
#define DRV_VAR_P(x)	npvar_sdriver.x
#else
#define DRV_VAR		(npvar_sdriver)
#define DRV_VAR_P(x)	npvar_sdriver->x
#endif

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,20))  
#ifndef INIT_WORK
#define INIT_WORK(_work, _func, _data){	\
	(_work)->routine = _func;\
	(_work)->data = _data;\
	}
#endif
#else
#ifndef INIT_WORK
#define INIT_WORK(_work, _func){	\
	(_work)->routine = _func;\
	}
#endif
#endif

#ifndef set_current_state
#define	set_current_state(x) 		current->state = x
#endif


#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#define IRQ_RET void
#define IRQ_RETVAL(x)
#else
#define IRQ_RET irqreturn_t
#endif


#if 0
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)) // >=2.4.0
#define	MXQ_TASK() {\
		MOD_INC_USE_COUNT;\
		if (schedule_task(&info->tqueue) == 0)\
			MOD_DEC_USE_COUNT;\
	}
#else   // < 2.4.0
#define MXQ_TASK()	queue_task(&info->tqueue,&tq_scheduler)
#endif
#else   // >=2.6.0
#define	MXQ_TASK()	schedule_work(&info->tqueue)
#endif
#else
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0)) // >=2.4.0
#define	MXQ_TASK(queue) {\
		MOD_INC_USE_COUNT;\
		if (schedule_task(queue) == 0)\
			MOD_DEC_USE_COUNT;\
	}
#else   // < 2.4.0
#define MXQ_TASK(queue)	queue_task(queue,&tq_scheduler)
#endif
#else   // >=2.6.0
#define	MXQ_TASK(queue)	schedule_work(queue)
#endif
#endif

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
#define MX_MOD_INC	MOD_INC_USE_COUNT
#define MX_MOD_DEC	MOD_DEC_USE_COUNT
#else
#define MX_MOD_INC	try_module_get(THIS_MODULE)
#define MX_MOD_DEC	module_put(THIS_MODULE)
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
#ifndef ASYNC_CALLOUT_ACTIVE
#define ASYNC_CALLOUT_ACTIVE 0
#endif
#endif


#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
#define MX_TTY_DRV(x)	tty->driver->x
#else
#define MX_TTY_DRV(x)	tty->driver.x
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,5))
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,20))
#define MX_SESSION()	tty->session
#else
#define MX_SESSION()	current->signal->session
#endif
#else
#define MX_SESSION()	current->session
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
#define MX_CGRP()	process_group(current)
#else
#define MX_CGRP()	current->pgrp
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
#define GET_FPAGE	__get_free_page
#else
#define GET_FPAGE	get_free_page
#endif

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,4,0))
#define DOWN(tx_lock, flags)    spin_lock_irqsave(&tx_lock, flags);
#define UP(tx_lock, flags)      spin_unlock_irqrestore(&tx_lock, flags);
#else
#define DOWN(tx_lock, flags)    down(&tx_lock);
#define UP(tx_lock, flags)      up(&tx_lock);
#endif

#ifndef atomic_read
#define atomic_read(v)	v
#endif


#ifndef UCHAR
typedef unsigned char	UCHAR;
#endif

// Scott: 2005-09-13 begin
// Added the debug print management
#define MX_DEBUG_OFF        0
#define MX_DEBUG_ERROR      1		// 1~19 for ERROR level
#define MX_DEBUG_WARN       20		// 20~39 for WARN level
#define MX_DEBUG_TRACE      40		// 40~59 for TRACE level
#define MX_DEBUG_INFO       60		// 60~79 for INFO level
#define MX_DEBUG_LOUD       80		// 80~ for LOUD level

#ifdef MX_DBG
extern int	MXDebugLevel;
#define DBGPRINT(level, fmt, args...)			\
{							\
    if ((level) <= MXDebugLevel)				\
    {							\
	printk("Real TTY: %s %d> ", __FUNCTION__, __LINE__);	\
	printk(KERN_DEBUG fmt, ## args);		\
    }							\
}
#else
#define DBGPRINT(level, fmt, args...)	while (0) ;
#endif
// Scott: 2005-09-13 end

#endif

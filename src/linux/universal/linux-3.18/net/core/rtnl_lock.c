#include <linux/rtnetlink.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/netdevice.h>

DEFINE_MUTEX(rtnl_mutex);

void rtnl_lock(void)
{
	mutex_lock(&rtnl_mutex);
}
EXPORT_SYMBOL(rtnl_lock);

void __rtnl_unlock(void)
{
	mutex_unlock(&rtnl_mutex);
}

void rtnl_unlock(void)
{
	/* This fellow will unlock it for us. */
	netdev_run_todo();
}
EXPORT_SYMBOL(rtnl_unlock);

int rtnl_trylock(void)
{
	return mutex_trylock(&rtnl_mutex);
}
EXPORT_SYMBOL(rtnl_trylock);

int rtnl_is_locked(void)
{
	return mutex_is_locked(&rtnl_mutex);
}
EXPORT_SYMBOL(rtnl_is_locked);

#ifdef CONFIG_PROVE_LOCKING
int lockdep_rtnl_is_held(void)
{
	return lockdep_is_held(&rtnl_mutex);
}
EXPORT_SYMBOL(lockdep_rtnl_is_held);
#endif /* #ifdef CONFIG_PROVE_LOCKING */

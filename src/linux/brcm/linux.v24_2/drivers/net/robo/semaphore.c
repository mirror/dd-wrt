#include <linux/sched.h>

void __up(struct semaphore *sem)
{ 
up(sem);     
}
/*void __up_wakeup(struct semaphore *sem)
{ 
up_wakeup(sem);     
}*/
void __down(struct semaphore * sem)
{
down(sem);
} 
/*void __down_failed(struct semaphore * sem)
{
down_failed(sem);
} */
/*int __down_failed_interruptible(struct semaphore * sem)
{
return down_failed_interruptible(sem);
} */
int __down_interruptible(struct semaphore * sem)
{
return down_interruptible(sem);
} 
int __down_trylock(struct semaphore * sem)
{
return down_trylock(sem);
} 

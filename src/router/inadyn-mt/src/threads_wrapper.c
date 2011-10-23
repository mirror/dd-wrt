/*
Copyright (C) 2009 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*
	A thin wrapper around Windows threads, and pthreads routines.

	Author: bhoover@wecs.com
	Date: Oct. 2009

	History:
		- first implemetnation
*/

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <memory.h>
#include <stdlib.h>

#include "debug_if.h"

#include "threads_wrapper.h"

static mutex_t *create_mutex_serial(mutex_t *mutex);
static mutex_t	crit_sec_create_mutex=MUTEX_T_INIT;
static void create_semaphore_serial(semaphore_t *sem);
static mutex_t	crit_sec_create_sem=MUTEX_T_INIT;


int threads_wrapper_init()
{

	create_mutex(&crit_sec_create_mutex);
	create_mutex(&crit_sec_create_sem);

	return 1;
}

/*
	functions get_mutex, get_mutex_try, init/create mutex_t mutex parameter, if not already created.  Simialar
	functionality is applied for signal, and wait semaphore.

	Alternatively, each client to the present module could be augmented to include an initialization routine.  
	This may appear in a future implementation.
*/
int get_mutex(mutex_t *mutex)
{

	if (!(mutex))

		return -1;

	if (!(mutex->is_init))

		create_mutex_serial(mutex);

#ifndef _WIN32

	return pthread_mutex_lock(&mutex->mutex);
#else

	return (!(WaitForSingleObject(mutex->mutex,INFINITE)==WAIT_OBJECT_0));
#endif
}

int get_mutex_try(mutex_t *mutex)
{

	if (!(mutex))

		return -1;

	if (!(mutex->is_init))

		create_mutex_serial(mutex);

#ifndef _WIN32

	return pthread_mutex_trylock(&mutex->mutex);
#else

	return (!(WaitForSingleObject(mutex->mutex,0)==WAIT_OBJECT_0));
#endif
}

int release_mutex(mutex_t *mutex)
{

	if (!(mutex))

		return -1;

#ifndef _WIN32

	if (!(mutex->is_init))

		return EINVAL;

	return pthread_mutex_unlock(&mutex->mutex);
#else
	return (!(ReleaseMutex(mutex->mutex)));
#endif
}

int signal_sem(semaphore_t *sem)
{

	if (!(sem))

		return -1;

	if (!(sem->is_init))

		create_semaphore_serial(sem);

#ifndef _WIN32

	return sem_post(&sem->sem);
#else

	return (!(ReleaseSemaphore(sem->sem,1,NULL)));
#endif
}

int wait_sem(semaphore_t *sem)
{

	if (!(sem))

		return -1;

	if (!(sem->is_init))

		create_semaphore_serial(sem);

#ifndef _WIN32

	return sem_wait(&sem->sem);
#else

	return (!(WaitForSingleObject(sem->sem,INFINITE)==WAIT_OBJECT_0));
#endif
}

mutex_t *create_mutex(mutex_t *mutex)
{

	if (!(mutex))

		return NULL;

	if (!(mutex->is_init)) {

		memset(mutex,0,sizeof(mutex_t));

		mutex->is_init=1;

#ifdef _WIN32

		mutex->mutex=CreateMutex(NULL,0,NULL);
#else

		pthread_mutex_init(&mutex->mutex,NULL);
#endif	
	}

	return mutex;
}

static mutex_t *create_mutex_serial(mutex_t *mutex)
{

	if (!(mutex))

		return NULL;

	/*
		critical section to avoid remote possibility of race on same *mutex --
		see get_mutex comments above.  module clients must initialize the
		present module with threads_wrapper_init().
	*/

	get_mutex(&crit_sec_create_mutex);

	if (!(mutex->is_init)) 

		create_mutex(mutex);


	release_mutex(&crit_sec_create_mutex);

	return mutex;
}

void create_semaphore(semaphore_t *sem)
{

	if (!(sem))

		return;

	if (!(sem->is_init)) {

		memset(sem,0,sizeof(semaphore_t));

		sem->is_init=1;

#ifdef _WIN32

		sem->sem=CreateSemaphore(NULL,0,1,NULL);	
#else

		sem_init(&sem->sem,0,0);
#endif	
	}
}

static void create_semaphore_serial(semaphore_t *sem)
{

	if (!(sem))

		return;

	get_mutex(&crit_sec_create_sem);

	if (!(sem->is_init))

		create_semaphore(sem);

	release_mutex(&crit_sec_create_sem);
}

void destroy_mutex(mutex_t *mutex)
{

	if (!(mutex))

		return;
	
	if (!(mutex->is_init))

		return;

	mutex->is_init=0;

#ifdef _WIN32

	if (WaitForSingleObject(mutex->mutex,INFINITE)==WAIT_OBJECT_0) {

		mutex->is_init=0;

		CloseHandle(mutex->mutex);
	}
#else

	if (pthread_mutex_lock(&mutex->mutex)==0) {

		mutex->is_init=0;

		pthread_mutex_unlock(&mutex->mutex);
		pthread_mutex_destroy(&mutex->mutex);
	}
#endif
}

void destroy_semaphore(semaphore_t *sem)
{
	if (!(sem))

		return;

	if (!(sem->is_init))

		return;

	sem->is_init=0;

#ifdef _WIN32	

	CloseHandle(sem->sem);
#else	

	sem_destroy(&sem->sem);
#endif
	
}


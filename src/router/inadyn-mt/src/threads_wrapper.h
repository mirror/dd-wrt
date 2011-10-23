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

#ifndef _THREADS_WRAPPER_H_INCLUDED
#define _THREADS_WRAPPER_H_INCLUDED

#ifndef _WIN32

#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <errno.h>

#define MUTEX_T_INIT		{PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,1}
#define SEMAPHORE_T_INIT	(0,0,0}

#else

#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <process.h>

#define MUTEX_T_INIT		{NULL,NULL,0}
#define SEMAPHORE_T_INIT	{NULL,NULL,0}

#endif

typedef struct {

#ifndef _WIN32
	pthread_mutex_t	op_mutex;
	pthread_mutex_t	mutex;
#else
	void *		op_mutex;
	void *		mutex;
#endif

	volatile int	is_init;	
} mutex_type;

typedef struct {

#ifndef _WIN32
	pthread_mutex_t	op_mutex;
	sem_t			sem;
#else
	void *		op_mutex;
	void *		sem;
#endif

	volatile int	is_init;	
} semaphore_type;

typedef mutex_type		mutex_t;
typedef semaphore_type	semaphore_t;

/*
	NOTE the get_mutex functions call create_mutex
	on mutex if mutex not already created
*/

int get_mutex(mutex_t *mutex);
int get_mutex_try(mutex_t *mutex);
int release_mutex(mutex_t *mutex);
int signal_sem(semaphore_t *sem);
int wait_sem(semaphore_t *sem);
mutex_t *create_mutex(mutex_t *mutex);
void create_semaphore(semaphore_t *sem);
void destroy_mutex(mutex_t *mutex);
void destroy_semaphore(semaphore_t *sem);
int threads_wrapper_init();

#endif /*_THREADS_WRAPPER_H_INCLUDED*/

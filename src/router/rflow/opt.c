#include "headers.h"
#include "storage.h"
#include "wrap_oclose.h"

volatile sig_atomic_t daemon_mode = 0;
volatile sig_atomic_t display_now = 0;
volatile sig_atomic_t signoff_now = 0;

volatile int max_clients = 10;
pthread_mutex_t max_clients_lock;
pthread_mutex_t packet_sources_list_lock;
pthread_mutex_t pndev_lock;

pthread_attr_t	thread_attr_detach;

int
init_pthread_options() {

	if(pthread_mutex_init(&max_clients_lock, NULL) == -1)
		return -1;

	if(pthread_mutex_init(&packet_sources_list_lock, NULL) == -1)
		return -1;

	if(pthread_mutex_init(&pndev_lock, NULL) == -1)
		return -1;

#if 0
	if(pthread_mutex_init(&fd_pool_mutex, NULL) == -1)
		return -1;
#endif
#if AFLOW
	if(pthread_mutex_init(&active_storage.storage_lock, NULL) == -1)
		return -1;

	if(pthread_mutex_init(&checkpoint_storage.storage_lock, NULL) == -1)
		return -1;
#endif

	if(pthread_mutex_init(&netflow_storage.storage_lock, NULL) == -1)
		return -1;

	/* Init thread attribute: Detach thread */
	if(pthread_attr_init(&thread_attr_detach) == -1)
		return -1;

	if(pthread_attr_setdetachstate(&thread_attr_detach, 1) == -1)
		return -1;

	return 0;
}



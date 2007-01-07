/*
 * pthread.c:
 * Tiny test program to see whether POSIX threads work.
 */

static const char rcsid[] = "$Id: pthread.c,v 1.4 2005/10/26 22:56:05 chris Exp $";

#include <sys/types.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int return_value = -1;

void *worker_thread(void *v) {
    /* Record successful return and signal parent to wake up. */
    return_value = 0;
    pthread_mutex_lock(&mtx);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    while (1) {
        sleep(1);
        pthread_testcancel();
    }
}

/* Start a thread, and have it set a variable to some other value, then signal
 * a condition variable. If this doesn't happen within some set time, we assume
 * that something's gone badly wrong and abort (for instance, the thread never
 * got started). */
int main(void) {
    pthread_t thr;
    int res;
    struct timespec deadline = {0};
    if ((res = pthread_mutex_lock(&mtx)) != 0
        || (res = pthread_create(&thr, NULL, worker_thread, NULL)) != 0) {
        fprintf(stderr, "%s\n", strerror(res));
        return -1;
    }

    /* Thread should now be running; we should wait on the condition
     * variable. */
    do
        deadline.tv_sec = 2 + time(NULL);
    while ((res = pthread_cond_timedwait(&cond, &mtx, &deadline)) == EINTR);
    
    if (res != 0) {
        fprintf(stderr, "%s\n", strerror(res));
        return -1;
    }

    if ((res = pthread_cancel(thr)) != 0
        || (res = pthread_join(thr, NULL)) != 0) {
        fprintf(stderr, "%s\n", strerror(res));
        return -1;
    }
    
    return return_value;
}

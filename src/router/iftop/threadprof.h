#ifdef PROFILING

#define pthread_create(a, b, c, d) gprof_pthread_create(a, b, c, d)

int gprof_pthread_create(pthread_t * thread, pthread_attr_t * attr,
                         void * (*start_routine)(void *), void * arg);

#endif

int monitor_insert_file(const char *name, const char *path);
int monitor_insert_directory(int fd, char *name, const char * path);
int monitor_remove_file(const char * path);
int monitor_remove_directory(int fd, const char * path);

#if defined(HAVE_INOTIFY) || defined(HAVE_KQUEUE)
#define	HAVE_WATCH 1
int	add_watch(int, const char *);
#endif

#ifdef HAVE_INOTIFY
void *
start_inotify();
#endif

#ifdef HAVE_KQUEUE
void	kqueue_monitor_start();
#endif

/*
 * Copyright (C) 2005-2015 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * dmeventd - dm event daemon to monitor active mapped devices
 */


#include "configure.h"
#include "libdevmapper-event.h"
#include "dmeventd.h"

#include "libdm/misc/dm-logging.h"
#include "base/memory/zalloc.h"

#include <dlfcn.h>
#include <pthread.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <arpa/inet.h>		/* for htonl, ntohl */
#include <fcntl.h>		/* for musl libc */
#include <unistd.h>
#include <syslog.h>

#ifdef __linux__
/*
 * Kernel version 2.6.36 and higher has
 * new OOM killer adjustment interface.
 */
#  define OOM_ADJ_FILE_OLD "/proc/self/oom_adj"
#  define OOM_ADJ_FILE "/proc/self/oom_score_adj"

/* From linux/oom.h */
/* Old interface */
#  define OOM_DISABLE (-17)
#  define OOM_ADJUST_MIN (-16)
/* New interface */
#  define OOM_SCORE_ADJ_MIN (-1000)

/* Systemd on-demand activation support */
#  define SD_RUNTIME_UNIT_FILE_DIR DEFAULT_DM_RUN_DIR "/systemd/system/"
#  define SD_ACTIVATION_ENV_VAR_NAME "SD_ACTIVATION"
#  define SD_LISTEN_PID_ENV_VAR_NAME "LISTEN_PID"
#  define SD_LISTEN_FDS_ENV_VAR_NAME "LISTEN_FDS"
#  define SD_LISTEN_FDS_START 3
#  define SD_FD_FIFO_SERVER SD_LISTEN_FDS_START
#  define SD_FD_FIFO_CLIENT (SD_LISTEN_FDS_START + 1)

#endif

#define DM_SIGNALED_EXIT  1
#define DM_SCHEDULED_EXIT 2
static volatile sig_atomic_t _exit_now = 0;	/* set to '1' when signal is given to exit */

/* List (un)link macros. */
#define	LINK(x, head)		dm_list_add(head, &(x)->list)
#define	LINK_DSO(dso)		LINK(dso, &_dso_registry)
#define	LINK_THREAD(thread)	LINK(thread, &_thread_registry)

#define	UNLINK(x)		dm_list_del(&(x)->list)
#define	UNLINK_DSO(x)		UNLINK(x)
#define	UNLINK_THREAD(x)	UNLINK(x)

#define DAEMON_NAME "dmeventd"

/*
  Global mutex for thread list access. Has to be held when:
  - iterating thread list
  - adding or removing elements from thread list
  - changing or reading thread_status's fields:
    processing, status, events
  Use _lock_mutex() and _unlock_mutex() to hold/release it
*/
static pthread_mutex_t _global_mutex;

static const size_t THREAD_STACK_SIZE = 300 * 1024;

/* Default idle exit timeout 1 hour (in seconds) */
static const time_t DMEVENTD_IDLE_EXIT_TIMEOUT = 60 * 60;

static int _debug_level = 0;
static int _use_syslog = 1;
static int _systemd_activation = 0;
static int _foreground = 0;
static int _restart = 0;
static time_t _idle_since = 0;
static char **_initial_registrations = 0;

/* FIXME Make configurable at runtime */

/* All libdm messages */
__attribute__((format(printf, 5, 6)))
static void _libdm_log(int level, const char *file, int line,
		       int dm_errno_or_class, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	dm_event_log("#dm", level, file, line, dm_errno_or_class, format, ap);
	va_end(ap);
}

/* All dmeventd messages */
#undef LOG_MESG
#define LOG_MESG(l, f, ln, e, x...) _dmeventd_log(l, f, ln, e, ## x)
__attribute__((format(printf, 5, 6)))
static void _dmeventd_log(int level, const char *file, int line,
			  int dm_errno_or_class, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	dm_event_log("dmeventd", level, file, line, dm_errno_or_class, format, ap);
	va_end(ap);
}

#ifdef DEBUG
#  define DEBUGLOG  log_debug
static const char *decode_cmd(uint32_t cmd)
{
	switch (cmd) {
	case DM_EVENT_CMD_ACTIVE:			return "ACTIVE";
	case DM_EVENT_CMD_REGISTER_FOR_EVENT:		return "REGISTER_FOR_EVENT";
	case DM_EVENT_CMD_UNREGISTER_FOR_EVENT:		return "UNREGISTER_FOR_EVENT";
	case DM_EVENT_CMD_GET_REGISTERED_DEVICE:	return "GET_REGISTERED_DEVICE";
	case DM_EVENT_CMD_GET_NEXT_REGISTERED_DEVICE:	return "GET_NEXT_REGISTERED_DEVICE";
	case DM_EVENT_CMD_SET_TIMEOUT:			return "SET_TIMEOUT";
	case DM_EVENT_CMD_GET_TIMEOUT:			return "GET_TIMEOUT";
	case DM_EVENT_CMD_HELLO:			return "HELLO";
	case DM_EVENT_CMD_DIE:				return "DIE";
	case DM_EVENT_CMD_GET_STATUS:			return "GET_STATUS";
	case DM_EVENT_CMD_GET_PARAMETERS:		return "GET_PARAMETERS";
	default:					return "unknown";
	}
}

#else
#  define DEBUGLOG(fmt, args...) do { } while (0)
#endif

/* Data kept about a DSO. */
struct dso_data {
	struct dm_list list;

	char *dso_name;		/* DSO name (eg, "evms", "dmraid", "lvm2"). */

	void *dso_handle;	/* Opaque handle as returned from dlopen(). */
	unsigned int ref_count;	/* Library reference count. */

	/*
	 * Event processing.
	 *
	 * The DSO can do whatever appropriate steps if an event
	 * happens such as changing the mapping in case a mirror
	 * fails, update the application metadata etc.
	 *
	 * This function gets a dm_task that is a result of
	 * DM_DEVICE_WAITEVENT ioctl (results equivalent to
	 * DM_DEVICE_STATUS). It should not destroy it.
	 * The caller must dispose of the task.
	 */
	void (*process_event)(struct dm_task *dmt, enum dm_event_mask event, void **user);

	/*
	 * Device registration.
	 *
	 * When an application registers a device for an event, the DSO
	 * can carry out appropriate steps so that a later call to
	 * the process_event() function is sane (eg, read metadata
	 * and activate a mapping).
	 */
	int (*register_device)(const char *device, const char *uuid, int major,
			       int minor, void **user);

	/*
	 * Device unregistration.
	 *
	 * In case all devices of a mapping (eg, RAID10) are unregistered
	 * for events, the DSO can recognize this and carry out appropriate
	 * steps (eg, deactivate mapping, metadata update).
	 */
	int (*unregister_device)(const char *device, const char *uuid,
				 int major, int minor, void **user);
};
static DM_LIST_INIT(_dso_registry);

/* Structure to keep parsed register variables from client message. */
struct message_data {
	char *id;
	char *dso_name;		/* Name of DSO. */
	char *device_uuid;	/* Mapped device path. */
	char *events_str;	/* Events string as fetched from message. */
	enum dm_event_mask events_field;	/* Events bitfield. */
	char *timeout_str;
	uint32_t timeout_secs;
	struct dm_event_daemon_message *msg;	/* Pointer to message buffer. */
};

/* There are three states a thread can attain. */
enum {
	DM_THREAD_REGISTERING,	/* Registering, transitions to RUNNING */
	DM_THREAD_RUNNING,	/* Working on events, transitions to DONE */
	DM_THREAD_DONE		/* Terminated and cleanup is pending */
};

/*
 * Housekeeping of thread+device states.
 *
 * One thread per mapped device which can block on it until an event
 * occurs and the event processing function of the DSO gets called.
 */
struct thread_status {
	struct dm_list list;

	pthread_t thread;

	struct dso_data *dso_data;	/* DSO this thread accesses. */

	struct {
		char *uuid;
		char *name;
		int major, minor;
	} device;
	int processing;		/* Set when event is being processed */

	int status;		/* See DM_THREAD_{REGISTERING,RUNNING,DONE} */

	int events;		/* bitfield for event filter. */
	int current_events;	/* bitfield for occured events. */
	struct dm_task *wait_task;
	int pending;		/* Set when event filter change is pending */
	time_t next_time;
	uint32_t timeout;
	struct dm_list timeout_list;
	void *dso_private; /* dso per-thread status variable */
	/* TODO per-thread mutex */
};

static DM_LIST_INIT(_thread_registry);
static DM_LIST_INIT(_thread_registry_unused);

static int _timeout_running;
static DM_LIST_INIT(_timeout_registry);
static pthread_mutex_t _timeout_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _timeout_cond = PTHREAD_COND_INITIALIZER;


/**********
 *   DSO
 **********/

/* DSO data allocate/free. */
static void _free_dso_data(struct dso_data *data)
{
	free(data->dso_name);
	free(data);
}

static struct dso_data *_alloc_dso_data(struct message_data *data)
{
	struct dso_data *ret = (typeof(ret)) zalloc(sizeof(*ret));

	if (!ret)
		return_NULL;

	if (!(ret->dso_name = strdup(data->dso_name))) {
		free(ret);
		return_NULL;
	}

	return ret;
}

/* DSO reference counting. */
static void _lib_get(struct dso_data *data)
{
	data->ref_count++;
}

static void _lib_put(struct dso_data *data)
{
	if (!--data->ref_count) {
		dlclose(data->dso_handle);
		UNLINK_DSO(data);
		_free_dso_data(data);

		/* Close control device if there is no plugin in-use */
		if (dm_list_empty(&_dso_registry)) {
			DEBUGLOG("Unholding control device.");
			dm_hold_control_dev(0);
			dm_lib_release();
			_idle_since = time(NULL);
		}
	}
}

/* Find DSO data. */
static struct dso_data *_lookup_dso(struct message_data *data)
{
	struct dso_data *dso_data, *ret = NULL;

	dm_list_iterate_items(dso_data, &_dso_registry)
		if (!strcmp(data->dso_name, dso_data->dso_name)) {
			ret = dso_data;
			break;
		}

	return ret;
}

/* Lookup DSO symbols we need. */
static int _lookup_symbol(void *dl, void **symbol, const char *name)
{
	if (!(*symbol = dlsym(dl, name)))
		return_0;

	return 1;
}

static int _lookup_symbols(void *dl, struct dso_data *data)
{
	return _lookup_symbol(dl, (void *) &data->process_event,
			     "process_event") &&
	    _lookup_symbol(dl, (void *) &data->register_device,
			  "register_device") &&
	    _lookup_symbol(dl, (void *) &data->unregister_device,
			  "unregister_device");
}

/* Load an application specific DSO. */
static struct dso_data *_load_dso(struct message_data *data)
{
	void *dl;
	struct dso_data *ret;
	const char *dlerr;

	if (!(dl = dlopen(data->dso_name, RTLD_NOW))) {
		dlerr = dlerror();
		goto_bad;
	}

	if (!(ret = _alloc_dso_data(data))) {
		dlclose(dl);
		dlerr = "no memory";
		goto_bad;
	}

	if (!(_lookup_symbols(dl, ret))) {
		_free_dso_data(ret);
		dlclose(dl);
		dlerr = "symbols missing";
		goto_bad;
	}

	/* Keep control device open until last user closes */
	if (dm_list_empty(&_dso_registry)) {
		DEBUGLOG("Holding control device open.");
		dm_hold_control_dev(1);
		_idle_since = 0;
	}

	/*
	 * Keep handle to close the library once
	 * we've got no references to it any more.
	 */
	ret->dso_handle = dl;
	LINK_DSO(ret);

	return ret;
bad:
	log_error("dmeventd %s dlopen failed: %s.", data->dso_name, dlerr);
	data->msg->size = dm_asprintf(&(data->msg->data), "%s %s dlopen failed: %s",
				      data->id, data->dso_name, dlerr);
	return NULL;
}

/************
 *  THREAD
 ************/

/* Allocate/free the thread status structure for a monitoring thread. */
static void _free_thread_status(struct thread_status *thread)
{

	_lib_put(thread->dso_data);
	if (thread->wait_task)
		dm_task_destroy(thread->wait_task);
	free(thread->device.uuid);
	free(thread->device.name);
	free(thread);
}

/* Note: events_field must not be 0, ensured by caller */
static struct thread_status *_alloc_thread_status(const struct message_data *data,
						  struct dso_data *dso_data)
{
	struct thread_status *thread;

	if (!(thread = zalloc(sizeof(*thread)))) {
		log_error("Cannot create new thread, out of memory.");
		return NULL;
	}

	_lib_get(dso_data);
	thread->dso_data = dso_data;

	if (!(thread->wait_task = dm_task_create(DM_DEVICE_WAITEVENT)))
		goto_out;

	if (!dm_task_set_uuid(thread->wait_task, data->device_uuid))
		goto_out;

	if (!(thread->device.uuid = strdup(data->device_uuid)))
		goto_out;

	/* Until real name resolved, use UUID */
	if (!(thread->device.name = strdup(data->device_uuid)))
		goto_out;

	/* runs ioctl and may register lvm2 pluging */
	thread->processing = 1;
	thread->status = DM_THREAD_REGISTERING;

	thread->events = data->events_field;
	thread->pending = DM_EVENT_REGISTRATION_PENDING;
	thread->timeout = data->timeout_secs;
	dm_list_init(&thread->timeout_list);

	return thread;

out:
	_free_thread_status(thread);

	return NULL;
}

/*
 * Create a device monitoring thread.
 * N.B.  Error codes returned are positive.
 */
static int _pthread_create_smallstack(pthread_t *t, void *(*fun)(void *), void *arg)
{
	int r;
	pthread_t tmp;
	pthread_attr_t attr;

	/*
	 * From pthread_attr_init man page:
	 * POSIX.1-2001 documents an ENOMEM error for pthread_attr_init(); on
	 * Linux these functions always succeed (but portable and future-proof
	 * applications should nevertheless handle a possible error return).
	 */
	if ((r = pthread_attr_init(&attr)) != 0) {
		log_sys_error("pthread_attr_init", "");
		return r;
	}

	/*
	 * We use a smaller stack since it gets preallocated in its entirety
	 */
	pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE + getpagesize());

	/*
	 * If no-one will be waiting, we need to detach.
	 */
	if (!t) {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		t = &tmp;
	}

	if ((r = pthread_create(t, &attr, fun, arg)))
		log_sys_error("pthread_create", "");

	pthread_attr_destroy(&attr);

	return r;
}

/*
 * Fetch a string off src and duplicate it into *ptr.
 * Pay attention to zero-length and 'empty' strings ('-').
 */
/* FIXME? move to libdevmapper to share with the client lib (need to
   make delimiter a parameter then) */
static int _fetch_string(char **ptr, char **src, const int delimiter)
{
	int ret = 1;
	char *p;
	size_t len;
	*ptr = NULL; /* Empty field returns NULL pointer */

	if ((*src)[0] == '-') {
		/* Could be empty field '-', handle without allocation */
		if ((*src)[1] == '\0') {
			(*src)++;
			goto out;
		} else if ((*src)[1] == delimiter) {
			(*src) += 2;
			goto out;
		}
	}

	if ((p = strchr(*src, delimiter))) {
		if (*src < p) {
			*p = 0; /* Temporary exit with \0 */
			if (!(*ptr = strdup(*src))) {
				log_error("Failed to fetch item %s.", *src);
				ret = 0; /* Allocation fail */
			}
			*p = delimiter;
			*src = p;
		}
		(*src)++; /* Skip delmiter, next field */
	} else if ((len = strlen(*src))) {
		/* No delimiter, item ends with '\0' */
		if (!(*ptr = strdup(*src))) {
			log_error("Failed to fetch last item %s.", *src);
			ret = 0; /* Fail */
		}
		*src += len + 1;
	}
out:
	return ret;
}

/* Free message memory. */
static void _free_message(struct message_data *message_data)
{
	free(message_data->id);
	free(message_data->dso_name);
	free(message_data->device_uuid);
	free(message_data->events_str);
	free(message_data->timeout_str);
}

/* Parse a register message from the client. */
static int _parse_message(struct message_data *message_data)
{
	int ret = 0;
	struct dm_event_daemon_message *msg = message_data->msg;
	char *p = msg->data;

	if (!msg->data)
		return 0;

	/*
	 * Retrieve application identifier, mapped device
	 * path and events # string from message.
	 */
	if (_fetch_string(&message_data->id, &p, ' ') &&
	    _fetch_string(&message_data->dso_name, &p, ' ') &&
	    _fetch_string(&message_data->device_uuid, &p, ' ') &&
	    _fetch_string(&message_data->events_str, &p, ' ') &&
	    _fetch_string(&message_data->timeout_str, &p, ' ')) {
		if (message_data->events_str)
			message_data->events_field =
				atoi(message_data->events_str);
		if (message_data->timeout_str)
			message_data->timeout_secs =
				atoi(message_data->timeout_str)
				? : DM_EVENT_DEFAULT_TIMEOUT;
		ret = 1;
	}

	free(msg->data);
	msg->data = NULL;

	return ret;
}

/* Global mutex to lock access to lists et al. See _global_mutex
   above. */
static int _lock_mutex(void)
{
	return pthread_mutex_lock(&_global_mutex);
}

static int _unlock_mutex(void)
{
	return pthread_mutex_unlock(&_global_mutex);
}

/* Check, if a device exists. */
static int _fill_device_data(struct thread_status *ts)
{
	struct dm_task *dmt;
	struct dm_info dmi;
	int ret = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_INFO)))
		return 0;

	if (!dm_task_set_uuid(dmt, ts->device.uuid))
		goto fail;

	if (!dm_task_run(dmt))
		goto fail;

	free(ts->device.name);
	if (!(ts->device.name = strdup(dm_task_get_name(dmt))))
		goto fail;

	if (!dm_task_get_info(dmt, &dmi))
		goto fail;

	ts->device.major = dmi.major;
	ts->device.minor = dmi.minor;
	dm_task_set_event_nr(ts->wait_task, dmi.event_nr);

	ret = 1;
fail:
	dm_task_destroy(dmt);

	return ret;
}

static struct dm_task *_get_device_status(struct thread_status *ts)
{
	struct dm_task *dmt = dm_task_create(DM_DEVICE_STATUS);

	if (!dmt)
		return_NULL;

	if (!dm_task_set_uuid(dmt, ts->device.uuid)) {
		dm_task_destroy(dmt);
		return_NULL;
	}

	/* Non-blocking status read */
	if (!dm_task_no_flush(dmt))
		log_warn("WARNING: Can't set no_flush for dm status.");

	if (!dm_task_run(dmt)) {
		dm_task_destroy(dmt);
		return_NULL;
	}

	return dmt;
}

/*
 * Find an existing thread for a device.
 *
 * Mutex must be held when calling this.
 */
static struct thread_status *_lookup_thread_status(struct message_data *data)
{
	struct thread_status *thread;

	dm_list_iterate_items(thread, &_thread_registry)
		if (!strcmp(data->device_uuid, thread->device.uuid))
			return thread;

	return NULL;
}

static int _get_status(struct message_data *message_data)
{
	struct dm_event_daemon_message *msg = message_data->msg;
	struct thread_status *thread;
	int i = 0, j;
	int ret = -ENOMEM;
	int count;
	int size = 0, current;
	size_t len;
	char **buffers;
	char *message;

	_lock_mutex();
	count = dm_list_size(&_thread_registry);
	buffers = alloca(sizeof(char*) * count);
	dm_list_iterate_items(thread, &_thread_registry) {
		if ((current = dm_asprintf(buffers + i, "0:%d %s %s %u %" PRIu32 ";",
					   i, thread->dso_data->dso_name,
					   thread->device.uuid, thread->events,
					   thread->timeout)) < 0) {
			_unlock_mutex();
			goto out;
		}
		++i;
		size += current; /* count with trailing '\0' */
	}
	_unlock_mutex();

	len = strlen(message_data->id);
	msg->size = size + len + 1;
	free(msg->data);
	if (!(msg->data = malloc(msg->size)))
		goto out;

	memcpy(msg->data, message_data->id, len);
	message = msg->data + len;
	*message++ = ' ';
	for (j = 0; j < i; ++j) {
		len = strlen(buffers[j]);
		memcpy(message, buffers[j], len);
		message += len;
	}

	ret = 0;
 out:
	for (j = 0; j < i; ++j)
		free(buffers[j]);

	return ret;
}

static int _get_parameters(struct message_data *message_data) {
	struct dm_event_daemon_message *msg = message_data->msg;
	int size;

	free(msg->data);
	if ((size = dm_asprintf(&msg->data, "%s pid=%d daemon=%s exec_method=%s",
				message_data->id, getpid(),
				_foreground ? "no" : "yes",
				_systemd_activation ? "systemd" : "direct")) < 0) {
		stack;
		return -ENOMEM;
	}

	msg->size = (uint32_t) size;

	return 0;
}

/* Cleanup at exit. */
static void _exit_dm_lib(void)
{
	dm_lib_release();
	dm_lib_exit();
}

static void _exit_timeout(void *unused __attribute__((unused)))
{
	_timeout_running = 0;
	pthread_mutex_unlock(&_timeout_mutex);
}

/* Wake up monitor threads every so often. */
static void *_timeout_thread(void *unused __attribute__((unused)))
{
	struct thread_status *thread;
	struct timespec timeout;
	time_t curr_time;
	int ret;

	DEBUGLOG("Timeout thread starting.");
	pthread_cleanup_push(_exit_timeout, NULL);
	pthread_mutex_lock(&_timeout_mutex);

	while (!dm_list_empty(&_timeout_registry)) {
		timeout.tv_sec = 0;
		timeout.tv_nsec = 0;
		curr_time = time(NULL);

		dm_list_iterate_items_gen(thread, &_timeout_registry, timeout_list) {
			if (thread->next_time <= curr_time) {
				thread->next_time = curr_time + thread->timeout;
				_lock_mutex();
				if (thread->processing) {
					/* Cannot signal processing monitoring thread */
					log_debug("Skipping SIGALRM to processing Thr %x for timeout.",
						  (int) thread->thread);
				} else {
					DEBUGLOG("Sending SIGALRM to Thr %x for timeout.",
						 (int) thread->thread);
					ret = pthread_kill(thread->thread, SIGALRM);
					if (ret && (ret != ESRCH))
						log_error("Unable to wakeup Thr %x for timeout: %s.",
							  (int) thread->thread, strerror(ret));
				}
				_unlock_mutex();
			}

			if (thread->next_time < timeout.tv_sec || !timeout.tv_sec)
				timeout.tv_sec = thread->next_time;
		}

		pthread_cond_timedwait(&_timeout_cond, &_timeout_mutex,
				       &timeout);
	}

	DEBUGLOG("Timeout thread finished.");
	pthread_cleanup_pop(1);

	return NULL;
}

static int _register_for_timeout(struct thread_status *thread)
{
	int ret = 0;

	pthread_mutex_lock(&_timeout_mutex);

	if (dm_list_empty(&thread->timeout_list)) {
		thread->next_time = time(NULL) + thread->timeout;
		dm_list_add(&_timeout_registry, &thread->timeout_list);
		if (_timeout_running)
			pthread_cond_signal(&_timeout_cond);
	}

	if (!_timeout_running &&
	    !(ret = _pthread_create_smallstack(NULL, _timeout_thread, NULL)))
		_timeout_running = 1;

	pthread_mutex_unlock(&_timeout_mutex);

	return ret;
}

static void _unregister_for_timeout(struct thread_status *thread)
{
	pthread_mutex_lock(&_timeout_mutex);
	if (!dm_list_empty(&thread->timeout_list)) {
		dm_list_del(&thread->timeout_list);
		dm_list_init(&thread->timeout_list);
		if (dm_list_empty(&_timeout_registry))
			/* No more work -> wakeup to finish quickly */
			pthread_cond_signal(&_timeout_cond);
	}
	pthread_mutex_unlock(&_timeout_mutex);
}

#ifdef DEBUG_SIGNALS
/* Print list of signals within a signal set */
static void _print_sigset(const char *prefix, const sigset_t *sigset)
{
	int sig, cnt = 0;

	for (sig = 1; sig < NSIG; sig++)
		if (!sigismember(sigset, sig)) {
			cnt++;
			log_debug("%s%d (%s)", prefix, sig, strsignal(sig));
		}

	if (!cnt)
		log_debug("%s<empty signal set>", prefix);
}
#endif

enum {
	DM_WAIT_RETRY,
	DM_WAIT_INTR,
	DM_WAIT_FATAL
};

/* Wait on a device until an event occurs. */
static int _event_wait(struct thread_status *thread)
{
	sigset_t set, old;
	int ret = DM_WAIT_RETRY;
	struct dm_info info;

	/* TODO: audit libdm thread usage */

	/*
	 * This is so that you can break out of waiting on an event,
	 * either for a timeout event, or to cancel the thread.
	 */
	sigemptyset(&old);
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	if (pthread_sigmask(SIG_UNBLOCK, &set, &old) != 0) {
		log_sys_error("pthread_sigmask", "unblock alarm");
		return ret; /* What better */
	}

	if (dm_task_run(thread->wait_task)) {
		thread->current_events |= DM_EVENT_DEVICE_ERROR;
		ret = DM_WAIT_INTR;
		/* Update event_nr */
		if (dm_task_get_info(thread->wait_task, &info))
			dm_task_set_event_nr(thread->wait_task, info.event_nr);
	} else {
		switch (dm_task_get_errno(thread->wait_task)) {
		case ENXIO:
			log_error("%s disappeared, detaching.",
				  thread->device.name);
			ret = DM_WAIT_FATAL;
			break;
		case EINTR:
			thread->current_events |= DM_EVENT_TIMEOUT;
			ret = DM_WAIT_INTR;
			break;
		default:
			log_sys_error("dm_task_run", "waitevent");
		}
	}

	if (pthread_sigmask(SIG_SETMASK, &old, NULL) != 0)
		log_sys_error("pthread_sigmask", "block alarm");

#ifdef DEBUG_SIGNALS
	_print_sigset("dmeventd blocking ", &old);
#endif
	DEBUGLOG("Completed waitevent task for %s.", thread->device.name);

	return ret;
}

/* Register a device with the DSO. */
static int _do_register_device(struct thread_status *thread)
{
	return thread->dso_data->register_device(thread->device.name,
						 thread->device.uuid,
						 thread->device.major,
						 thread->device.minor,
						 &(thread->dso_private));
}

/* Unregister a device with the DSO. */
static int _do_unregister_device(struct thread_status *thread)
{
	return thread->dso_data->unregister_device(thread->device.name,
						   thread->device.uuid,
						   thread->device.major,
						   thread->device.minor,
						   &(thread->dso_private));
}

/* Process an event in the DSO. */
static void _do_process_event(struct thread_status *thread)
{
	struct dm_task *task;

	/* NOTE: timeout event gets status */
	task = (thread->current_events & DM_EVENT_TIMEOUT)
		? _get_device_status(thread) : thread->wait_task;

	if (!task)
		log_error("Lost event in Thr %x.", (int)thread->thread);
	else {
		thread->dso_data->process_event(task, thread->current_events, &(thread->dso_private));
		if (task != thread->wait_task)
			dm_task_destroy(task);
	}
}

static void _thread_unused(struct thread_status *thread)
{
	UNLINK_THREAD(thread);
	LINK(thread, &_thread_registry_unused);
}

/* Thread cleanup handler to unregister device. */
static void _monitor_unregister(void *arg)
{
	struct thread_status *thread = arg, *thread_iter;

	dm_list_iterate_items(thread_iter, &_thread_registry)
		if (thread_iter == thread) {
			/* Relink to _unused */
			_thread_unused(thread);
			break;
		}

	thread->events = 0;	/* Filter is now empty */
	thread->pending = 0;	/* Event pending resolved */
	thread->processing = 1;	/* Process unregistering */

	_unlock_mutex();

	DEBUGLOG("Unregistering monitor for %s.", thread->device.name);
	_unregister_for_timeout(thread);

	if ((thread->status != DM_THREAD_REGISTERING) &&
	    !_do_unregister_device(thread))
		log_error("%s: %s unregister failed.", __func__,
			  thread->device.name);

	DEBUGLOG("Marking Thr %x as DONE and unused.", (int)thread->thread);

	_lock_mutex();
	thread->status = DM_THREAD_DONE; /* Last access to thread memory! */
	_unlock_mutex();
}

/* Device monitoring thread. */
static void *_monitor_thread(void *arg)
{
	struct thread_status *thread = arg;
	int ret;
	sigset_t pendmask;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	pthread_cleanup_push(_monitor_unregister, thread);

	if (!_fill_device_data(thread)) {
		log_error("Failed to fill device data for %s.", thread->device.uuid);
		_lock_mutex();
		goto out;
	}

	if (!_do_register_device(thread)) {
		log_error("Failed to register device %s.", thread->device.name);
		_lock_mutex();
		goto out;
	}

	_lock_mutex();
	thread->status = DM_THREAD_RUNNING;
	thread->processing = 0;

	/* Loop awaiting/analyzing device events. */
	while (thread->events) {

		thread->pending = 0; /* Event is no longer pending...  */

		/*
		 * Check against bitmask filter.
		 *
		 * If there's current events delivered from _event_wait() AND
		 * the device got registered for those events AND
		 * those events haven't been processed yet, call
		 * the DSO's process_event() handler.
		 */
		if (thread->events & thread->current_events) {
			thread->processing = 1;  /* Cannot be removed/signaled */
			_unlock_mutex();

			_do_process_event(thread);
			thread->current_events = 0; /* Current events processed */

			_lock_mutex();
			thread->processing = 0;

			/*
			 * Thread can terminate itself from plugin via SIGALRM
			 * Timer thread will not send signal while processing
			 * TODO: maybe worth API change and return value for
			 *       _do_process_event() instead of this signal solution
			 */
			if (sigpending(&pendmask) < 0)
				log_sys_error("sigpending", "");
			else if (sigismember(&pendmask, SIGALRM))
				break;
		} else {
			_unlock_mutex();

			if ((ret = _event_wait(thread)) == DM_WAIT_RETRY)
				usleep(100); /* Avoid busy loop, wait without mutex */

			_lock_mutex();

			if (ret == DM_WAIT_FATAL)
				break;
		}
	}
out:
	/* ';' fixes gcc compilation problem with older pthread macros
	 * "label at end of compound statement" */
	;

	pthread_cleanup_pop(1);

	return NULL;
}

/* Create a device monitoring thread. */
static int _create_thread(struct thread_status *thread)
{
	return _pthread_create_smallstack(&thread->thread, _monitor_thread, thread);
}

/* Update events - needs to be locked */
static int _update_events(struct thread_status *thread, int events)
{
	int ret = 0;

	if (thread->events == events)
		return 0; /* Nothing has changed */

	thread->events = events;
	thread->pending = DM_EVENT_REGISTRATION_PENDING;

	/* Only non-processing threads can be notified */
	if (!thread->processing) {
		DEBUGLOG("Sending SIGALRM to wakeup Thr %x.", (int)thread->thread);

		/* Notify thread waiting in ioctl (to speed-up) */
		if ((ret = pthread_kill(thread->thread, SIGALRM))) {
			if (ret == ESRCH)
				thread->events = 0;  /* thread is gone */
			else
				log_error("Unable to wakeup thread: %s",
					  strerror(ret));
		}
	}

	/* Threads with no events has to be moved to unused */
	if (!thread->events)
		_thread_unused(thread);

	return -ret;
}

/* Return success on daemon active check. */
static int _active(struct message_data *message_data)
{
	return 0;
}

/*
 * Unregister for an event.
 *
 * Only one caller at a time here as with register_for_event().
 */
static int _unregister_for_event(struct message_data *message_data)
{
	struct thread_status *thread;
	int ret;

	/*
	 * Clear event in bitfield and deactivate
	 * monitoring thread in case bitfield is 0.
	 */
	_lock_mutex();

	if (!(thread = _lookup_thread_status(message_data))) {
		_unlock_mutex();
		return -ENODEV;
	}

	/* AND mask event ~# from events bitfield. */
	ret = _update_events(thread, (thread->events & ~message_data->events_field));

	_unlock_mutex();

	/* If there are no events, thread is later garbage
	 * collected by _cleanup_unused_threads */
	if (message_data->events_field & DM_EVENT_TIMEOUT)
		_unregister_for_timeout(thread);

	DEBUGLOG("Unregistered event for %s.", thread->device.name);

	return ret;
}

/*
 * Register for an event.
 *
 * Only one caller at a time here, because we use
 * a FIFO and lock it against multiple accesses.
 */
static int _register_for_event(struct message_data *message_data)
{
	int ret = 0;
	struct thread_status *thread;
	struct dso_data *dso_data;

	if (!(dso_data = _lookup_dso(message_data)) &&
	    !(dso_data = _load_dso(message_data))) {
		stack;
#ifdef ELIBACC
		ret = ELIBACC;
#else
		ret = ENODEV;
#endif
		return ret;
	}

	_lock_mutex();

	if ((thread = _lookup_thread_status(message_data))) {
		/* OR event # into events bitfield. */
		ret = _update_events(thread, (thread->events | message_data->events_field));
	} else {
		_unlock_mutex();

		/* Only creating thread during event processing
		 * Remaining initialization happens within monitoring thread */
		if (!(thread = _alloc_thread_status(message_data, dso_data))) {
			stack;
			return -ENOMEM;
		}

		if ((ret = _create_thread(thread))) {
			stack;
			_free_thread_status(thread);
			return -ret;
		}

		_lock_mutex();
		/* Note: same uuid can't be added in parallel */
		LINK_THREAD(thread);
	}

	_unlock_mutex();

	/* If creation of timeout thread fails (as it may), we fail
	   here completely. The client is responsible for either
	   retrying later or trying to register without timeout
	   events. However, if timeout thread cannot be started, it
	   usually means we are so starved on resources that we are
	   almost as good as dead already... */
	if ((message_data->events_field & DM_EVENT_TIMEOUT) &&
	    (ret = _register_for_timeout(thread))) {
		stack;
		_unregister_for_event(message_data);
	}

	return -ret;
}

/*
 * Get registered device.
 *
 * Only one caller at a time here as with register_for_event().
 */
static int _registered_device(struct message_data *message_data,
			     struct thread_status *thread)
{
	int r;
	struct dm_event_daemon_message *msg = message_data->msg;

	free(msg->data);

	if ((r = dm_asprintf(&(msg->data), "%s %s %s %u",
			     message_data->id,
			     thread->dso_data->dso_name,
			     thread->device.uuid,
			     thread->events | thread->pending)) < 0)
		return -ENOMEM;

	msg->size = (uint32_t) r;
	DEBUGLOG("Registered %s.", msg->data);

	return 0;
}

static int _want_registered_device(char *dso_name, char *device_uuid,
				   struct thread_status *thread)
{
	/* If DSO names and device paths are equal. */
	if (dso_name && device_uuid)
		return !strcmp(dso_name, thread->dso_data->dso_name) &&
		    !strcmp(device_uuid, thread->device.uuid);

	/* If DSO names are equal. */
	if (dso_name)
		return !strcmp(dso_name, thread->dso_data->dso_name);

	/* If device paths are equal. */
	if (device_uuid)
		return !strcmp(device_uuid, thread->device.uuid);

	return 1;
}

static int _get_registered_dev(struct message_data *message_data, int next)
{
	struct thread_status *thread, *hit = NULL;
	int ret = -ENOENT;

	DEBUGLOG("Get%s dso:%s  uuid:%s.", next ? "" : "Next",
		 message_data->dso_name,
		 message_data->device_uuid);
	_lock_mutex();

	/* Iterate list of threads checking if we want a particular one. */
	dm_list_iterate_items(thread, &_thread_registry)
		if (_want_registered_device(message_data->dso_name,
					    message_data->device_uuid,
					    thread)) {
			hit = thread;
			break;
		}

	/*
	 * If we got a registered device and want the next one ->
	 * fetch next conforming element off the list.
	 */
	if (hit && !next)
		goto reg;

	/*
	 * If we didn't get a match, try the threads waiting to be deleted.
	 * FIXME Do something similar if 'next' is set.
	 */
	if (!hit && !next)
		dm_list_iterate_items(thread, &_thread_registry_unused)
			if (_want_registered_device(message_data->dso_name,
						    message_data->device_uuid, thread)) {
				hit = thread;
				goto reg;
			}

	if (!hit) {
		DEBUGLOG("Get%s not registered", next ? "" : "Next");
		goto out;
	}

	while (1) {
		if (dm_list_end(&_thread_registry, &thread->list))
			goto out;

		thread = dm_list_item(thread->list.n, struct thread_status);
		if (_want_registered_device(message_data->dso_name, NULL, thread)) {
			hit = thread;
			break;
		}
	}

      reg:
	ret = _registered_device(message_data, hit);

      out:
	_unlock_mutex();

	return ret;
}

static int _get_registered_device(struct message_data *message_data)
{
	return _get_registered_dev(message_data, 0);
}

static int _get_next_registered_device(struct message_data *message_data)
{
	return _get_registered_dev(message_data, 1);
}

static int _set_timeout(struct message_data *message_data)
{
	struct thread_status *thread;

	_lock_mutex();
	thread = _lookup_thread_status(message_data);
	_unlock_mutex();

	if (!thread)
		return -ENODEV;

	/* Lets reprogram timer */
	pthread_mutex_lock(&_timeout_mutex);
	thread->timeout = message_data->timeout_secs;
	thread->next_time = 0;
	pthread_cond_signal(&_timeout_cond);
	pthread_mutex_unlock(&_timeout_mutex);

	return 0;
}

static int _get_timeout(struct message_data *message_data)
{
	struct thread_status *thread;
	struct dm_event_daemon_message *msg = message_data->msg;

	_lock_mutex();
	thread = _lookup_thread_status(message_data);
	_unlock_mutex();

	if (!thread)
		return -ENODEV;

	free(msg->data);
	msg->size = dm_asprintf(&(msg->data), "%s %" PRIu32,
				message_data->id, thread->timeout);

	return (msg->data && msg->size) ? 0 : -ENOMEM;
}

static int _open_fifo(const char *path)
{
	struct stat st;
	int fd = -1;
 
 	/*
	 * FIXME Explicitly verify the code's requirement that path is secure:
	 * - All parent directories owned by root without group/other write access unless sticky.
	 */

	/* If path exists, only use it if it is root-owned fifo mode 0600 */
	if ((lstat(path, &st) < 0)) {
		if (errno != ENOENT) {
			log_sys_error("stat", path);
			return -1;
		}
	} else if (!S_ISFIFO(st.st_mode) || st.st_uid ||
		   (st.st_mode & (S_IEXEC | S_IRWXG | S_IRWXO))) {
		log_warn("WARNING: %s has wrong attributes: Replacing.", path);
		if (unlink(path)) {
			log_sys_error("unlink", path);
			return -1;
		}
	}

	/* Create fifo. */
	(void) dm_prepare_selinux_context(path, S_IFIFO);
	if ((mkfifo(path, 0600) == -1) && errno != EEXIST) {
		log_sys_error("mkfifo", path);
		(void) dm_prepare_selinux_context(NULL, 0);
		goto fail;
	}

	(void) dm_prepare_selinux_context(NULL, 0);

	/* Need to open read+write or we will block or fail */
	if ((fd = open(path, O_RDWR)) < 0) {
		log_sys_error("open", path);
		goto fail;
	}

	/* Warn about wrong permissions if applicable */
	if (fstat(fd, &st)) {
		log_sys_error("fstat", path);
		goto fail;
	}

	if (!S_ISFIFO(st.st_mode) || st.st_uid ||
	    (st.st_mode & (S_IEXEC | S_IRWXG | S_IRWXO))) {
		log_error("%s: fifo has incorrect attributes", path);
		goto fail;
	}

	if (fcntl(fd, F_SETFD, FD_CLOEXEC)) {
		log_sys_error("fcntl(FD_CLOEXEC)", path);
		goto fail;
	}

	return fd;

fail:
	if ((fd >= 0) && close(fd))
		log_sys_error("close", path);

	return -1;
}

/* Open fifos used for client communication. */
static int _open_fifos(struct dm_event_fifos *fifos)
{
	/* Create client fifo. */
	if ((fifos->client = _open_fifo(fifos->client_path)) < 0)
		goto fail;

	/* Create server fifo. */
	if ((fifos->server = _open_fifo(fifos->server_path)) < 0)
		goto fail;

	return 1;

fail:
	if (fifos->client >= 0 && close(fifos->client))
		log_sys_error("close", fifos->client_path);

	return 0;
}

/*
 * Read message from client making sure that data is available
 * and a complete message is read.  Must not block indefinitely.
 */
static int _client_read(struct dm_event_fifos *fifos,
			struct dm_event_daemon_message *msg)
{
	struct timeval t;
	unsigned bytes = 0;
	int ret = 0;
	fd_set fds;
	size_t size = 2 * sizeof(uint32_t);	/* status + size */
	uint32_t *header = alloca(size);
	char *buf = (char *)header;

	msg->data = NULL;

	errno = 0;
	while (bytes < size && errno != EOF) {
		/* Watch client read FIFO for input. */
		FD_ZERO(&fds);
		FD_SET(fifos->client, &fds);
		t.tv_sec = 1;
		t.tv_usec = 0;
		ret = select(fifos->client + 1, &fds, NULL, NULL, &t);

		if (!ret && !bytes)	/* nothing to read */
			return 0;

		if (!ret)	/* trying to finish read */
			continue;

		if (ret < 0)	/* error */
			return 0;

		ret = read(fifos->client, buf + bytes, size - bytes);
		bytes += ret > 0 ? ret : 0;
		if (header && (bytes == 2 * sizeof(uint32_t))) {
			msg->cmd = ntohl(header[0]);
			size = msg->size = ntohl(header[1]);
			bytes = 0;
			if (!size)
				break; /* No data -> error */
			buf = msg->data = malloc(msg->size);
			if (!buf)
				break; /* No mem -> error */
			header = 0;
		}
	}

	if (bytes != size) {
		free(msg->data);
		msg->data = NULL;
		return 0;
	}

	return 1;
}

/*
 * Write a message to the client making sure that it is ready to write.
 */
static int _client_write(struct dm_event_fifos *fifos,
			struct dm_event_daemon_message *msg)
{
	uint32_t temp[2];
	unsigned bytes = 0;
	int ret = 0;
	fd_set fds;

	size_t size = 2 * sizeof(uint32_t) + ((msg->data) ? msg->size : 0);
	uint32_t *header = malloc(size);
	char *buf = (char *)header;

	if (!header) {
		/* Reply with ENOMEM message */
		header = temp;
		size = sizeof(temp);
		header[0] = htonl(-ENOMEM);
		header[1] = 0;
	} else {
		header[0] = htonl(msg->cmd);
		header[1] = htonl((msg->data) ? msg->size : 0);
		if (msg->data)
			memcpy(buf + 2 * sizeof(uint32_t), msg->data, msg->size);
	}

	while (bytes < size) {
		do {
			/* Watch client write FIFO to be ready for output. */
			FD_ZERO(&fds);
			FD_SET(fifos->server, &fds);
		} while (select(fifos->server + 1, NULL, &fds, NULL, NULL) != 1);

		if ((ret = write(fifos->server, buf + bytes, size - bytes)) > 0)
			bytes += ret;
		else if (errno == EIO)
			break;
	}

	if (header != temp)
		free(header);

	return (bytes == size);
}

/*
 * Handle a client request.
 *
 * We put the request handling functions into
 * a list because of the growing number.
 */
static int _handle_request(struct dm_event_daemon_message *msg,
			  struct message_data *message_data)
{
	switch (msg->cmd) {
	case DM_EVENT_CMD_REGISTER_FOR_EVENT:
		if (!message_data->events_field)
			return -EINVAL;
		return _register_for_event(message_data);
	case DM_EVENT_CMD_UNREGISTER_FOR_EVENT:
		return _unregister_for_event(message_data);
	case DM_EVENT_CMD_GET_REGISTERED_DEVICE:
		return _get_registered_device(message_data);
	case DM_EVENT_CMD_GET_NEXT_REGISTERED_DEVICE:
		return _get_next_registered_device(message_data);
	case DM_EVENT_CMD_SET_TIMEOUT:
		return _set_timeout(message_data);
	case DM_EVENT_CMD_GET_TIMEOUT:
		return _get_timeout(message_data);
	case DM_EVENT_CMD_ACTIVE:
		return _active(message_data);
	case DM_EVENT_CMD_GET_STATUS:
		return _get_status(message_data);
	/* dmeventd parameters of running dmeventd,
	 * returns 'pid=<pid> daemon=<no/yes> exec_method=<direct/systemd>'
	 * 	pid - pidfile of running dmeventd
	 * 	daemon - running as a daemon or not (foreground)?
	 * 	exec_method - "direct" if executed directly or
	 * 		      "systemd" if executed via systemd
	 */
	case DM_EVENT_CMD_GET_PARAMETERS:
		return _get_parameters(message_data);
	default:
		return -EINVAL;
	}
}

/* Process a request passed from the communication thread. */
static int _do_process_request(struct dm_event_daemon_message *msg)
{
	int ret;
	char *answer;
	struct message_data message_data = { .msg =  msg };

	/* Parse the message. */
	if (msg->cmd == DM_EVENT_CMD_HELLO || msg->cmd == DM_EVENT_CMD_DIE)  {
		ret = 0;
		answer = msg->data;
		if (answer) {
			msg->size = dm_asprintf(&(msg->data), "%s %s %d", answer,
						(msg->cmd == DM_EVENT_CMD_DIE) ? "DYING" : "HELLO",
						DM_EVENT_PROTOCOL_VERSION);
			free(answer);
		}
	} else if (msg->cmd != DM_EVENT_CMD_ACTIVE && !_parse_message(&message_data)) {
		stack;
		ret = -EINVAL;
	} else
		ret = _handle_request(msg, &message_data);

	msg->cmd = ret;
	if (!msg->data)
		msg->size = dm_asprintf(&(msg->data), "%s %s", message_data.id, strerror(-ret));

	_free_message(&message_data);

	return ret;
}

/* Only one caller at a time. */
static void _process_request(struct dm_event_fifos *fifos)
{
	struct dm_event_daemon_message msg = { 0 };
	int cmd;
	/*
	 * Read the request from the client (client_read, client_write
	 * give true on success and false on failure).
	 */
	if (!_client_read(fifos, &msg))
		return;

	cmd = msg.cmd;

	DEBUGLOG(">>> CMD:%s (0x%x) processing...", decode_cmd(cmd), cmd);

	/* _do_process_request fills in msg (if memory allows for
	   data, otherwise just cmd and size = 0) */
	_do_process_request(&msg);

	if (!_client_write(fifos, &msg))
		stack;

	DEBUGLOG("<<< CMD:%s (0x%x) completed (result %d).", decode_cmd(cmd), cmd, msg.cmd);

	free(msg.data);

	if (cmd == DM_EVENT_CMD_DIE) {
		if (unlink(DMEVENTD_PIDFILE))
			log_sys_error("unlink", DMEVENTD_PIDFILE);
		_exit(0);
	}
}

static void _process_initial_registrations(void)
{
	int i;
	char *reg;
	struct dm_event_daemon_message msg = { 0 };

	for (i = 0; (reg = _initial_registrations[i]); ++i) {
		msg.cmd = DM_EVENT_CMD_REGISTER_FOR_EVENT;
		if ((msg.size = strlen(reg))) {
			msg.data = reg;
			_do_process_request(&msg);
		}
	}
}

static void _cleanup_unused_threads(void)
{
	struct dm_list *l;
	struct thread_status *thread;
	int ret;

	_lock_mutex();

	while ((l = dm_list_first(&_thread_registry_unused))) {
		thread = dm_list_item(l, struct thread_status);
		if (thread->status != DM_THREAD_DONE) {
			if (thread->processing)
				break; /* cleanup on the next round */

			/* Signal possibly sleeping thread */
			ret = pthread_kill(thread->thread, SIGALRM);
			if (!ret || (ret != ESRCH))
				break; /* check again on the next round */

			/* thread is likely gone */
		}

		dm_list_del(l);
		_unlock_mutex();

		DEBUGLOG("Destroying Thr %x.", (int)thread->thread);

		if (pthread_join(thread->thread, NULL))
			log_sys_error("pthread_join", "");

		_free_thread_status(thread);
		_lock_mutex();
	}

	_unlock_mutex();
}

static void _sig_alarm(int signum __attribute__((unused)))
{
	/* empty SIG_IGN */;
}

/* Init thread signal handling. */
static void _init_thread_signals(void)
{
	sigset_t my_sigset;
	struct sigaction act = { .sa_handler = _sig_alarm };

	sigaction(SIGALRM, &act, NULL);
	sigfillset(&my_sigset);

	/* These are used for exiting */
	sigdelset(&my_sigset, SIGTERM);
	sigdelset(&my_sigset, SIGINT);
	sigdelset(&my_sigset, SIGHUP);
	sigdelset(&my_sigset, SIGQUIT);

	pthread_sigmask(SIG_BLOCK, &my_sigset, NULL);
}

/*
 * exit_handler
 * @sig
 *
 * Set the global variable which the process should
 * be watching to determine when to exit.
 */
static void _exit_handler(int sig __attribute__((unused)))
{
	_exit_now = DM_SIGNALED_EXIT;
}

#ifdef __linux__
static int _set_oom_adj(const char *oom_adj_path, int val)
{
	FILE *fp;

	if (!(fp = fopen(oom_adj_path, "w"))) {
		log_sys_error("open", oom_adj_path);
		return 0;
	}

	fprintf(fp, "%i", val);

	if (dm_fclose(fp))
		log_sys_error("fclose", oom_adj_path);

	return 1;
}

/*
 * Protection against OOM killer if kernel supports it
 */
static int _protect_against_oom_killer(void)
{
	struct stat st;

	if (stat(OOM_ADJ_FILE, &st) == -1) {
		if (errno != ENOENT)
			log_sys_error("stat", OOM_ADJ_FILE);

		/* Try old oom_adj interface as a fallback */
		if (stat(OOM_ADJ_FILE_OLD, &st) == -1) {
			log_sys_error("stat", OOM_ADJ_FILE_OLD);
			return 1;
		}

		return _set_oom_adj(OOM_ADJ_FILE_OLD, OOM_DISABLE) ||
		       _set_oom_adj(OOM_ADJ_FILE_OLD, OOM_ADJUST_MIN);
	}

	return _set_oom_adj(OOM_ADJ_FILE, OOM_SCORE_ADJ_MIN);
}

static int _handle_preloaded_fifo(int fd, const char *path)
{
	struct stat st_fd, st_path;
	int flags;

	if ((flags = fcntl(fd, F_GETFD)) < 0)
		return 0;

	if (flags & FD_CLOEXEC)
		return 0;

	if (fstat(fd, &st_fd) < 0 || !S_ISFIFO(st_fd.st_mode))
		return 0;

	if (stat(path, &st_path) < 0 ||
	    st_path.st_dev != st_fd.st_dev ||
	    st_path.st_ino != st_fd.st_ino)
		return 0;

	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0)
		return 0;

	return 1;
}

static int _systemd_handover(struct dm_event_fifos *fifos)
{
	const char *e;
	char *p;
	unsigned long env_pid, env_listen_fds;
	int r = 0;

	/* SD_ACTIVATION must be set! */
	if (!(e = getenv(SD_ACTIVATION_ENV_VAR_NAME)) || strcmp(e, "1"))
		goto out;

	/* LISTEN_PID must be equal to our PID! */
	if (!(e = getenv(SD_LISTEN_PID_ENV_VAR_NAME)))
		goto out;

	errno = 0;
	env_pid = strtoul(e, &p, 10);
	if (errno || !p || *p || env_pid <= 0 ||
	    getpid() != (pid_t) env_pid)
		goto out;

	/* LISTEN_FDS must be 2 and the fds must be FIFOSs! */
	if (!(e = getenv(SD_LISTEN_FDS_ENV_VAR_NAME)))
		goto out;

	errno = 0;
	env_listen_fds = strtoul(e, &p, 10);
	if (errno || !p || *p || env_listen_fds != 2)
		goto out;

	/* Check and handle the FIFOs passed in */
	r = (_handle_preloaded_fifo(SD_FD_FIFO_SERVER, DM_EVENT_FIFO_SERVER) &&
	     _handle_preloaded_fifo(SD_FD_FIFO_CLIENT, DM_EVENT_FIFO_CLIENT));

	if (r) {
		fifos->server = SD_FD_FIFO_SERVER;
		fifos->server_path = DM_EVENT_FIFO_SERVER;
		fifos->client = SD_FD_FIFO_CLIENT;
		fifos->client_path = DM_EVENT_FIFO_CLIENT;
	}

out:
	unsetenv(SD_ACTIVATION_ENV_VAR_NAME);
	unsetenv(SD_LISTEN_PID_ENV_VAR_NAME);
	unsetenv(SD_LISTEN_FDS_ENV_VAR_NAME);
	return r;
}

#endif

static void _remove_files_on_exit(void)
{
	if (unlink(DMEVENTD_PIDFILE))
		log_sys_error("unlink", DMEVENTD_PIDFILE);

	if (!_systemd_activation) {
		if (unlink(DM_EVENT_FIFO_CLIENT))
			log_sys_error("unlink", DM_EVENT_FIFO_CLIENT);

		if (unlink(DM_EVENT_FIFO_SERVER))
			log_sys_error("unlink", DM_EVENT_FIFO_SERVER);
	}
}

static void _daemonize(void)
{
	int child_status;
	int fd;
	pid_t pid;
	struct rlimit rlim;
	struct timeval tval;
	sigset_t my_sigset;

	sigemptyset(&my_sigset);
	if (sigprocmask(SIG_SETMASK, &my_sigset, NULL) < 0) {
		fprintf(stderr, "Unable to restore signals.\n");
		exit(EXIT_FAILURE);
	}
	signal(SIGTERM, &_exit_handler);

	switch (pid = fork()) {
	case -1:
		log_sys_error("fork", "");
		exit(EXIT_FAILURE);
	case 0:		/* Child */
		break;

	default:
		/* Wait for response from child */
		while (!waitpid(pid, &child_status, WNOHANG) && !_exit_now) {
			tval.tv_sec = 0;
			tval.tv_usec = 250000;	/* .25 sec */
			select(0, NULL, NULL, NULL, &tval);
		}

		if (_exit_now)	/* Child has signaled it is ok - we can exit now */
			exit(EXIT_SUCCESS);

		/* Problem with child.  Determine what it is by exit code */
		switch (WEXITSTATUS(child_status)) {
		case EXIT_DESC_CLOSE_FAILURE:
		case EXIT_DESC_OPEN_FAILURE:
		case EXIT_FIFO_FAILURE:
		case EXIT_CHDIR_FAILURE:
		default:
			fprintf(stderr, "Child exited with code %d\n", WEXITSTATUS(child_status));
			break;
		}

		exit(WEXITSTATUS(child_status));
	}

	if (chdir("/"))
		exit(EXIT_CHDIR_FAILURE);

	if (getrlimit(RLIMIT_NOFILE, &rlim) < 0)
		fd = 256;	/* just have to guess */
	else
		fd = rlim.rlim_cur;

	for (--fd; fd >= 0; fd--) {
#ifdef __linux__
		/* Do not close fds preloaded by systemd! */
		if (_systemd_activation &&
		    (fd == SD_FD_FIFO_SERVER || fd == SD_FD_FIFO_CLIENT))
			continue;
#endif
		(void) close(fd);
	}

	if ((open("/dev/null", O_RDONLY) < 0) ||
	    (open("/dev/null", O_WRONLY) < 0) ||
	    (open("/dev/null", O_WRONLY) < 0))
		exit(EXIT_DESC_OPEN_FAILURE);

	setsid();
}

static int _reinstate_registrations(struct dm_event_fifos *fifos)
{
	static const char _failed_parsing_msg[] = "Failed to parse existing event registration.\n";
	static const char *_delim = " ";
	struct dm_event_daemon_message msg = { 0 };
	char *endp, *dso_name, *dev_name, *mask, *timeout;
	unsigned long mask_value, timeout_value;
	int i, ret;

	ret = daemon_talk(fifos, &msg, DM_EVENT_CMD_HELLO, NULL, NULL, 0, 0);
	free(msg.data);
	msg.data = NULL;

	if (ret) {
		fprintf(stderr, "Failed to communicate with new instance of dmeventd.\n");
		return 0;
	}

	for (i = 0; _initial_registrations[i]; ++i) {
		if (!(strtok(_initial_registrations[i], _delim)) ||
		    !(dso_name = strtok(NULL, _delim)) ||
		    !(dev_name = strtok(NULL, _delim)) ||
		    !(mask = strtok(NULL, _delim)) ||
		    !(timeout = strtok(NULL, _delim))) {
			fputs(_failed_parsing_msg, stderr);
			continue;
		}

		errno = 0;
		mask_value = strtoul(mask, &endp, 10);
		if (errno || !endp || *endp) {
			fputs(_failed_parsing_msg, stderr);
			continue;
		}

		errno = 0;
		timeout_value = strtoul(timeout, &endp, 10);
		if (errno || !endp || *endp) {
			fputs(_failed_parsing_msg, stderr);
			continue;
		}

		if (daemon_talk(fifos, &msg, DM_EVENT_CMD_REGISTER_FOR_EVENT,
				dso_name,
				dev_name,
				(enum dm_event_mask) mask_value,
				timeout_value))
			fprintf(stderr, "Failed to reinstate monitoring for device %s.\n", dev_name);
	}

	return 1;
}

static void _restart_dmeventd(void)
{
	struct dm_event_fifos fifos = {
		.server = -1,
		.client = -1,
		/* FIXME Make these either configurable or depend directly on dmeventd_path */
		.client_path = DM_EVENT_FIFO_CLIENT,
		.server_path = DM_EVENT_FIFO_SERVER
	};
	struct dm_event_daemon_message msg = { 0 };
	int i, count = 0;
	char *message;
	int version;
	const char *e;

	/* Get the list of registrations from the running daemon. */
	if (!init_fifos(&fifos)) {
		fprintf(stderr, "WARNING: Could not initiate communication with existing dmeventd.\n");
		exit(EXIT_FAILURE);
	}

	if (!dm_event_get_version(&fifos, &version)) {
		fprintf(stderr, "WARNING: Could not communicate with existing dmeventd.\n");
		goto bad;
	}

	if (version < 1) {
		fprintf(stderr, "WARNING: The running dmeventd instance is too old.\n"
				"Protocol version %d (required: 1). Action cancelled.\n",
				version);
		goto bad;
	}

	if (daemon_talk(&fifos, &msg, DM_EVENT_CMD_GET_STATUS, "-", "-", 0, 0))
		goto bad;

	message = strchr(msg.data, ' ') + 1;
	for (i = 0; msg.data[i]; ++i)
		if (msg.data[i] == ';') {
			msg.data[i] = 0;
			++count;
		}

	if (!(_initial_registrations = malloc(sizeof(char*) * (count + 1)))) {
		fprintf(stderr, "Memory allocation registration failed.\n");
		goto bad;
	}

	for (i = 0; i < count; ++i) {
		if (!(_initial_registrations[i] = strdup(message))) {
			fprintf(stderr, "Memory allocation for message failed.\n");
			goto bad;
		}
		message += strlen(message) + 1;
	}
	_initial_registrations[count] = NULL;

	if (version >= 2) {
		if (daemon_talk(&fifos, &msg, DM_EVENT_CMD_GET_PARAMETERS, "-", "-", 0, 0)) {
			fprintf(stderr, "Failed to acquire parameters from old dmeventd.\n");
			goto bad;
		}
		if (strstr(msg.data, "exec_method=systemd"))
			_systemd_activation = 1;
	}
#ifdef __linux__
	/*
	* If the protocol version is old, just assume that if systemd is running,
	* the dmeventd is also run as a systemd service via fifo activation.
	*/
	if (version < 2) {
		/* This check is copied from sd-daemon.c. */
		struct stat st;
		if (!lstat(SD_RUNTIME_UNIT_FILE_DIR, &st) && !!S_ISDIR(st.st_mode))
			_systemd_activation = 1;
	}
#endif

	if (daemon_talk(&fifos, &msg, DM_EVENT_CMD_DIE, "-", "-", 0, 0)) {
		fprintf(stderr, "Old dmeventd refused to die.\n");
		goto bad;
	}

	if (!_systemd_activation &&
	    ((e = getenv(SD_ACTIVATION_ENV_VAR_NAME)) && strcmp(e, "1")))
		_systemd_activation = 1;

	for (i = 0; i < 10; ++i) {
		if ((access(DMEVENTD_PIDFILE, F_OK) == -1) && (errno == ENOENT))
			break;
		usleep(10);
	}

	if (!_systemd_activation) {
		fini_fifos(&fifos);
		return;
	}

	/* Reopen fifos. */
	fini_fifos(&fifos);
	if (!init_fifos(&fifos)) {
		fprintf(stderr, "Could not initiate communication with new instance of dmeventd.\n");
		exit(EXIT_FAILURE);
	}

	if (!_reinstate_registrations(&fifos)) {
		fprintf(stderr, "Failed to reinstate monitoring with new instance of dmeventd.\n");
		goto bad;
	}

	fini_fifos(&fifos);
	exit(EXIT_SUCCESS);
bad:
	fini_fifos(&fifos);
	exit(EXIT_FAILURE);
}

static void _usage(char *prog, FILE *file)
{
	fprintf(file, "Usage:\n"
		"%s [-d [-d [-d]]] [-f] [-h] [-l] [-R] [-V] [-?]\n\n"
		"   -d       Log debug messages to syslog (-d, -dd, -ddd)\n"
		"   -f       Don't fork, run in the foreground\n"
		"   -h       Show this help information\n"
		"   -l       Log to stdout,stderr instead of syslog\n"
		"   -?       Show this help information on stderr\n"
		"   -R       Restart dmeventd\n"
		"   -V       Show version of dmeventd\n\n", prog);
}

int main(int argc, char *argv[])
{
	signed char opt;
	struct dm_event_fifos fifos = {
		.client = -1,
		.server = -1,
		.client_path = DM_EVENT_FIFO_CLIENT,
		.server_path = DM_EVENT_FIFO_SERVER
	};
	time_t now, idle_exit_timeout = DMEVENTD_IDLE_EXIT_TIMEOUT;
	opterr = 0;
	optind = 0;

	while ((opt = getopt(argc, argv, "?fhVdlR")) != EOF) {
		switch (opt) {
		case 'h':
			_usage(argv[0], stdout);
			exit(EXIT_SUCCESS);
		case '?':
			_usage(argv[0], stderr);
			exit(EXIT_SUCCESS);
		case 'R':
			_restart++;
			break;
		case 'f':
			_foreground++;
			break;
		case 'd':
			_debug_level++;
			break;
		case 'l':
			_use_syslog = 0;
			break;
		case 'V':
			printf("dmeventd version: %s\n", DM_LIB_VERSION);
			exit(EXIT_SUCCESS);
		}
	}

	if (!_foreground && !_use_syslog) {
		printf("WARNING: Ignoring logging to stdout, needs options -f\n");
		_use_syslog = 1;
	}
	/*
	 * Switch to C locale to avoid reading large locale-archive file
	 * used by some glibc (on some distributions it takes over 100MB).
	 * Daemon currently needs to use mlockall().
	 */
	if (setenv("LC_ALL", "C", 1))
		perror("Cannot set LC_ALL to C");

	if (_restart)
		_restart_dmeventd();

#ifdef __linux__
	_systemd_activation = _systemd_handover(&fifos);
#endif

	if (!_foreground)
		_daemonize();

	if (_use_syslog)
		openlog("dmeventd", LOG_PID, LOG_DAEMON);

	dm_event_log_set(_debug_level, _use_syslog);
	dm_log_with_errno_init(_libdm_log);

	(void) dm_prepare_selinux_context(DMEVENTD_PIDFILE, S_IFREG);
	if (dm_create_lockfile(DMEVENTD_PIDFILE) == 0)
		exit(EXIT_FAILURE);

	atexit(_remove_files_on_exit);
	(void) dm_prepare_selinux_context(NULL, 0);

	/* Set the rest of the signals to cause '_exit_now' to be set */
	signal(SIGTERM, &_exit_handler);
	signal(SIGINT, &_exit_handler);
	signal(SIGHUP, &_exit_handler);
	signal(SIGQUIT, &_exit_handler);

#ifdef __linux__
	/* Systemd has adjusted oom killer for us already */
	if (!_systemd_activation && !_protect_against_oom_killer())
		log_warn("WARNING: Failed to protect against OOM killer.");
#endif

	_init_thread_signals();

	pthread_mutex_init(&_global_mutex, NULL);

	if (!_systemd_activation && !_open_fifos(&fifos))
		exit(EXIT_FIFO_FAILURE);

	/* Signal parent, letting them know we are ready to go. */
	if (!_foreground)
		kill(getppid(), SIGTERM);

	log_notice("dmeventd ready for processing.");

	_idle_since = time(NULL);

	if (_initial_registrations)
		_process_initial_registrations();

	for (;;) {
		if (_idle_since) {
			if (_exit_now) {
				if (_exit_now == DM_SCHEDULED_EXIT)
					break; /* Only prints shutdown message */
				log_info("dmeventd detected break while being idle "
					 "for %ld second(s), exiting.",
					 (long) (time(NULL) - _idle_since));
				break;
			}
			if (idle_exit_timeout) {
				now = time(NULL);
				if (now < _idle_since)
					_idle_since = now; /* clock change? */
				now -= _idle_since;
				if (now >= idle_exit_timeout) {
					log_info("dmeventd was idle for %ld second(s), "
						 "exiting.", (long) now);
					break;
				}
			}
		} else if (_exit_now == DM_SIGNALED_EXIT) {
			_exit_now = DM_SCHEDULED_EXIT;
			/*
			 * When '_exit_now' is set, signal has been received,
			 * but can not simply exit unless all
			 * threads are done processing.
			 */
			log_info("dmeventd received break, scheduling exit.");
		}
		_process_request(&fifos);
		_cleanup_unused_threads();
	}

	pthread_mutex_destroy(&_global_mutex);

	log_notice("dmeventd shutting down.");

	if (fifos.client >= 0 && close(fifos.client))
		log_sys_error("client close", fifos.client_path);
	if (fifos.server >= 0 && close(fifos.server))
		log_sys_error("server close", fifos.server_path);

	if (_use_syslog)
		closelog();

	_exit_dm_lib();

	exit(EXIT_SUCCESS);
}

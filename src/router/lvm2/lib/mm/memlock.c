/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lib/misc/lib.h"
#include "lib/mm/memlock.h"
#include "lib/config/defaults.h"
#include "lib/config/config.h"
#include "lib/commands/toolcontext.h"

#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <malloc.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#endif

#ifndef DEVMAPPER_SUPPORT

void memlock_inc_daemon(struct cmd_context *cmd)
{
	return;
}

void memlock_dec_daemon(struct cmd_context *cmd)
{
	return;
}

void critical_section_inc(struct cmd_context *cmd, const char *reason)
{
	return;
}

void critical_section_dec(struct cmd_context *cmd, const char *reason)
{
	return;
}

int critical_section(void)
{
	return 0;
}
void memlock_init(struct cmd_context *cmd)
{
	return;
}

void memlock_unlock(struct cmd_context *cmd)
{
	return;
}

void memlock_reset(void)
{
	return;
}

int memlock_count_daemon(void)
{
	return 0;
}

#else				/* DEVMAPPER_SUPPORT */

static size_t _size_stack;
static size_t _size_malloc_tmp;
static size_t _size_malloc = 2000000;

static void *_malloc_mem = NULL;
static int _mem_locked = 0;
static int _priority_raised = 0;
static int _critical_section = 0;
static int _prioritized_section = 0;
static int _memlock_count_daemon = 0;
static int _priority;
static int _default_priority;

/* list of maps, that are unconditionaly ignored */
static const char * const _ignore_maps[] = {
	"[vdso]",
	"[vsyscall]",
	"[vectors]",
};

/* default blacklist for maps */
static const char * const _blacklist_maps[] = {
	"locale/locale-archive",
	"/LC_MESSAGES/",
	"gconv/gconv-modules.cache",
	"/ld-2.",		/* not using dlopen,dlsym during mlock */
	"/libaio.so.",		/* not using aio during mlock */
	"/libattr.so.",		/* not using during mlock (udev) */
	"/libblkid.so.",	/* not using blkid during mlock (udev) */
	"/libbz2.so.",		/* not using during mlock (udev) */
	"/libcap.so.",		/* not using during mlock (systemd) */
	"/libdl-",		/* not using dlopen,dlsym during mlock */
	"/libdw-",		/* not using during mlock (udev) */
	"/libelf-",		/* not using during mlock (udev) */
	"/libgcrypt.so.",	/* not using during mlock (systemd) */
	"/libgpg-error.so.",	/* not using gpg-error during mlock (systemd) */
	"/liblz4.so.",		/* not using lz4 during mlock (systemd) */
	"/liblzma.so.",		/* not using lzma during mlock (systemd) */
	"/libmount.so.",	/* not using mount during mlock (udev) */
	"/libncurses.so.",	/* not using ncurses during mlock */
	"/libpcre.so.",		/* not using pcre during mlock (selinux) */
	"/libpcre2-",		/* not using pcre during mlock (selinux) */
	"/libreadline.so.",	/* not using readline during mlock */
	"/libresolv-",		/* not using during mlock (udev) */
	"/libselinux.so.",	/* not using selinux during mlock */
	"/libsepol.so.",	/* not using sepol during mlock */
	"/libsystemd.so.",	/* not using systemd during mlock */
	"/libtinfo.so.",	/* not using tinfo during mlock */
	"/libudev.so.",		/* not using udev during mlock */
	"/libuuid.so.",		/* not using uuid during mlock (blkid) */
	"/libz.so.",		/* not using during mlock (udev) */
	"/etc/selinux",		/* not using selinux during mlock */
	/* "/libdevmapper-event.so" */
};

typedef enum { LVM_MLOCK, LVM_MUNLOCK } lvmlock_t;

static unsigned _use_mlockall;
static int _maps_fd;
static size_t _maps_len = 8192; /* Initial buffer size for reading /proc/self/maps */
static char *_maps_buffer;
static char _procselfmaps[PATH_MAX] = "";
#define SELF_MAPS "/self/maps"

static size_t _mstats; /* statistic for maps locking */

static void _touch_memory(void *mem, size_t size)
{
	size_t pagesize = lvm_getpagesize();
	char *pos = mem;
	char *end = pos + size - sizeof(long);

	while (pos < end) {
		*(long *) pos = 1;
		pos += pagesize;
	}
}

static void _allocate_memory(void)
{
#ifndef VALGRIND_POOL
	void *stack_mem;
	struct rlimit limit;
	int i, area = 0, missing = _size_malloc_tmp, max_areas = 32, hblks;
	char *areas[max_areas];

	/* Check if we could preallocate requested stack */
	if ((getrlimit (RLIMIT_STACK, &limit) == 0) &&
	    ((_size_stack * 2) < limit.rlim_cur) &&
	    ((stack_mem = alloca(_size_stack))))
		_touch_memory(stack_mem, _size_stack);
	/* FIXME else warn user setting got ignored */

        /*
         *  When a brk() fails due to fragmented address space (which sometimes
         *  happens when we try to grab 8M or so), glibc will make a new
         *  arena. In this arena, the rules for using “direct” mmap are relaxed,
         *  circumventing the MAX_MMAPs and MMAP_THRESHOLD settings. We can,
         *  however, detect when this happens with mallinfo() and try to co-opt
         *  malloc into using MMAP as a MORECORE substitute instead of returning
         *  MMAP'd memory directly. Since MMAP-as-MORECORE does not munmap the
         *  memory on free(), this is good enough for our purposes.
         */
	while (missing > 0) {
		struct mallinfo inf = mallinfo();
		hblks = inf.hblks;

		if ((areas[area] = malloc(_size_malloc_tmp)))
			_touch_memory(areas[area], _size_malloc_tmp);

		inf = mallinfo();

		if (hblks < inf.hblks) {
			/* malloc cheated and used mmap, even though we told it
			   not to; we try with twice as many areas, each half
			   the size, to circumvent the faulty logic in glibc */
			free(areas[area]);
			_size_malloc_tmp /= 2;
		} else {
			++ area;
			missing -= _size_malloc_tmp;
		}

		if (area == max_areas && missing > 0) {
			/* Too bad. Warn the user and proceed, as things are
			 * most likely going to work out anyway. */
			log_warn("WARNING: Failed to reserve memory, %d bytes missing.", missing);
			break;
		}
	}

	if ((_malloc_mem = malloc(_size_malloc)))
		_touch_memory(_malloc_mem, _size_malloc);

	/* free up the reserves so subsequent malloc's can use that memory */
	for (i = 0; i < area; ++i)
		free(areas[i]);
#endif
}

static void _release_memory(void)
{
	free(_malloc_mem);
}

/*
 * mlock/munlock memory areas from /proc/self/maps
 * format described in kernel/Documentation/filesystem/proc.txt
 */
static int _maps_line(const struct dm_config_node *cn, lvmlock_t lock,
		      const char *line, size_t *mstats)
{
	const struct dm_config_value *cv;
	long from, to;
	int pos;
	unsigned i;
	char fr, fw, fx, fp;
	size_t sz;
	const char *lock_str = (lock == LVM_MLOCK) ? "mlock" : "munlock";

	if (sscanf(line, "%lx-%lx %c%c%c%c%n",
		   &from, &to, &fr, &fw, &fx, &fp, &pos) != 6) {
		log_error("Failed to parse maps line: %s", line);
		return 0;
	}

	/* Select readable maps */
	if (fr != 'r') {
		log_debug_mem("%s area unreadable %s : Skipping.", lock_str, line);
		return 1;
	}

	/* always ignored areas */
	for (i = 0; i < DM_ARRAY_SIZE(_ignore_maps); ++i)
		if (strstr(line + pos, _ignore_maps[i])) {
			log_debug_mem("%s ignore filter '%s' matches '%s': Skipping.",
				      lock_str, _ignore_maps[i], line);
			return 1;
		}

	sz = to - from;
	if (!cn) {
		/* If no blacklist configured, use an internal set */
		for (i = 0; i < DM_ARRAY_SIZE(_blacklist_maps); ++i)
			if (strstr(line + pos, _blacklist_maps[i])) {
				log_debug_mem("%s default filter '%s' matches '%s': Skipping.",
					      lock_str, _blacklist_maps[i], line);
				return 1;
			}
	} else {
		for (cv = cn->v; cv; cv = cv->next) {
			if ((cv->type != DM_CFG_STRING) || !cv->v.str[0])
				continue;
			if (strstr(line + pos, cv->v.str)) {
				log_debug_mem("%s_filter '%s' matches '%s': Skipping.",
					      lock_str, cv->v.str, line);
				return 1;
			}
		}
	}

#ifdef HAVE_VALGRIND
	/*
	 * Valgrind is continually eating memory while executing code
	 * so we need to deactivate check of locked memory size
	 */
#ifndef VALGRIND_POOL
	if (RUNNING_ON_VALGRIND)
#endif
		sz -= sz; /* = 0, but avoids getting warning about dead assigment */

#endif
	*mstats += sz;
	log_debug_mem("%s %10ldKiB %12lx - %12lx %c%c%c%c%s", lock_str,
		      ((long)sz + 1023) / 1024, from, to, fr, fw, fx, fp, line + pos);

	if (lock == LVM_MLOCK) {
		if (mlock((const void*)from, sz) < 0) {
			log_sys_error("mlock", line);
			return 0;
		}
	} else {
		if (munlock((const void*)from, sz) < 0) {
			log_sys_error("munlock", line);
			return 0;
		}
	}

	return 1;
}

static int _memlock_maps(struct cmd_context *cmd, lvmlock_t lock, size_t *mstats)
{
	const struct dm_config_node *cn;
	char *line, *line_end;
	size_t len;
	ssize_t n;
	int ret = 1;

	if (_use_mlockall) {
#ifdef MCL_CURRENT
		if (lock == LVM_MLOCK) {
			if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
				log_sys_error("mlockall", "");
				return 0;
			}
		} else {
			if (munlockall()) {
				log_sys_error("munlockall", "");
				return 0;
			}
		}
		return 1;
#else
		return 0;
#endif
	}

	/* Reset statistic counters */
	*mstats = 0;

	/* read mapping into a single memory chunk without reallocation
	 * in the middle of reading maps file */
	for (len = 0;;) {
		if (!_maps_buffer || len >= _maps_len) {
			if (_maps_buffer)
				_maps_len *= 2;
			if (!(line = realloc(_maps_buffer, _maps_len))) {
				log_error("Allocation of maps buffer failed.");
				return 0;
			}
			_maps_buffer = line;
		}
		if (lseek(_maps_fd, 0, SEEK_SET))
			log_sys_error("lseek", _procselfmaps);
		for (len = 0 ; len < _maps_len; len += n) {
			if (!(n = read(_maps_fd, _maps_buffer + len, _maps_len - len)))
				break; /* EOF */
			if (n == -1) {
				log_sys_error("read", _procselfmaps);
				return 0;
			}
		}
		if (len < _maps_len) { /* fits in buffer */
			_maps_buffer[len] = '\0';
			break;
		}
	}

	line = _maps_buffer;
	cn = find_config_tree_array(cmd, activation_mlock_filter_CFG, NULL);

	while ((line_end = strchr(line, '\n'))) {
		*line_end = '\0'; /* remove \n */
		if (!_maps_line(cn, lock, line, mstats))
			ret = 0;
		line = line_end + 1;
	}

	log_debug_mem("%socked %ld bytes",
		      (lock == LVM_MLOCK) ? "L" : "Unl", (long)*mstats);

	return ret;
}

#ifdef DEBUG_MEMLOCK
/*
 * LVM is not supposed to use mmap while devices are suspended.
 * This code causes a core dump if gets called."
 */
#  ifdef __i386__
#    define ARCH_X86
#  endif /* __i386__ */
#  ifdef __x86_64__
#    ifndef ARCH_X86
#      define ARCH_X86
#    endif /* ARCH_X86 */
#  endif /* __x86_64__ */

#endif /* DEBUG_MEMLOCK */

#ifdef ARCH_X86
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
static const unsigned char _instruction_hlt = 0x94;
static char _mmap_orig;
static unsigned char *_mmap_addr;
#ifdef __i386__
static char _mmap64_orig;
static unsigned char *_mmap64_addr;
#endif /* __i386__ */
#endif /* ARCH_X86 */

static int _disable_mmap(void)
{
#ifdef ARCH_X86
	volatile unsigned char *abs_addr;

	if (!_mmap_addr) {
		_mmap_addr = (unsigned char *) dlsym(RTLD_NEXT, "mmap");
		if (_mmap_addr[0] == 0xff && _mmap_addr[1] == 0x25) { /* plt */
#ifdef __x86_64__
			abs_addr = _mmap_addr + 6 + *(int32_t *)(_mmap_addr + 2);
#endif /* __x86_64__ */
#ifdef __i386__
			abs_addr = *(void **)(_mmap_addr + 2);
#endif /* __i386__ */
			_mmap_addr = *(void **)abs_addr;
		} else
			log_debug_mem("Can't find PLT jump entry assuming -fPIE linkage.");
		if (mprotect((void *)((unsigned long)_mmap_addr & ~4095UL), 4096, PROT_READ|PROT_WRITE|PROT_EXEC)) {
			log_sys_error("mprotect", "");
			_mmap_addr = NULL;
			return 0;
		}
		_mmap_orig = *_mmap_addr;
	}
	log_debug_mem("Remapping mmap entry %02x to %02x.", _mmap_orig, _instruction_hlt);
	*_mmap_addr = _instruction_hlt;

#ifdef __i386__
	if (!_mmap64_addr) {
		_mmap64_addr = (unsigned char *) dlsym(RTLD_NEXT, "mmap64");
		if (_mmap64_addr[0] == 0xff && _mmap64_addr[1] == 0x25) {
			abs_addr = *(void **)(_mmap64_addr + 2);
			_mmap64_addr = *(void **)abs_addr;
		} /* Can't find PLT jump entry assuming -fPIE linkage */
		if (mprotect((void *)((unsigned long)_mmap64_addr & ~4095UL), 4096, PROT_READ|PROT_WRITE|PROT_EXEC)) {
			log_sys_error("mprotect", "");
			_mmap64_addr = NULL;
			return 0;
		}
		_mmap64_orig = *_mmap64_addr;
	}
	*_mmap64_addr = INSTRUCTION_HLT;
#endif /* __i386__ */
#endif /* ARCH_X86 */
	return 1;
}

static int _restore_mmap(void)
{
#ifdef ARCH_X86
	if (_mmap_addr)
		*_mmap_addr = _mmap_orig;
#ifdef __i386__
	if (_mmap64_addr)
		*_mmap64_addr = _mmap64_orig;
#endif /* __i386__ */
	log_debug_mem("Restored mmap entry.");
#endif /* ARCH_X86 */
	return 1;
}
static void _raise_priority(struct cmd_context *cmd)
{
	if (_priority_raised)
		return;

	_priority_raised = 1;
	errno = 0;
	if (((_priority = getpriority(PRIO_PROCESS, 0)) == -1) && errno)
		log_sys_debug("getpriority", "");
	else if (_default_priority < _priority) {
		if (setpriority(PRIO_PROCESS, 0, _default_priority) == 0)
			log_debug_activation("Raised task priority %d -> %d.",
					     _priority, _default_priority);
		else
			log_warn("WARNING: setpriority %d failed: %s.",
				 _default_priority, strerror(errno));
	}
}

static void _restore_priority_if_possible(struct cmd_context *cmd)
{
	if (!_priority_raised || _critical_section || _memlock_count_daemon)
		return;

	if (setpriority(PRIO_PROCESS, 0, _priority) == 0)
		log_debug_activation("Restoring original task priority %d.", _priority);
	else
		log_warn("WARNING: setpriority %u failed: %s.",
			 _priority, strerror(errno));

	_priority_raised = 0;
}

/* Stop memory getting swapped out */
static void _lock_mem(struct cmd_context *cmd)
{
	_allocate_memory();
	(void)strerror(0);		/* Force libc.mo load */
	(void)dm_udev_get_sync_support(); /* udev is initialized */
	log_very_verbose("Locking memory");

	/*
	 * For daemon we need to use mlockall()
	 * so even future adition of thread which may not even use lvm lib
	 * will not block memory locked thread
	 * Note: assuming _memlock_count_daemon is updated before _memlock_count
	 */
	_use_mlockall = _memlock_count_daemon ? 1 :
		find_config_tree_bool(cmd, activation_use_mlockall_CFG, NULL);

	if (!_use_mlockall) {
		if (!*_procselfmaps &&
		    dm_snprintf(_procselfmaps, sizeof(_procselfmaps),
				"%s" SELF_MAPS, cmd->proc_dir) < 0) {
			log_error("proc_dir too long");
			return;
		}

		if (!(_maps_fd = open(_procselfmaps, O_RDONLY))) {
			log_sys_error("open", _procselfmaps);
			return;
		}

		if (!_disable_mmap())
			stack;
	}

	if (!_memlock_maps(cmd, LVM_MLOCK, &_mstats))
		stack;
}

static void _unlock_mem(struct cmd_context *cmd)
{
	size_t unlock_mstats;

	log_very_verbose("Unlocking memory");

	if (!_memlock_maps(cmd, LVM_MUNLOCK, &unlock_mstats))
		stack;

	if (!_use_mlockall) {
		_restore_mmap();
		if (close(_maps_fd))
			log_sys_error("close", _procselfmaps);
		free(_maps_buffer);
		_maps_buffer = NULL;
		if (_mstats < unlock_mstats) {
			if ((_mstats + lvm_getpagesize()) < unlock_mstats)
				log_error(INTERNAL_ERROR
					  "Reserved memory (%ld) not enough: used %ld. Increase activation/reserved_memory?",
					  (long)_mstats, (long)unlock_mstats);
			else
				/* FIXME Believed due to incorrect use of yes_no_prompt while locks held */
				log_debug_mem("Suppressed internal error: Maps lock %ld < unlock %ld, a one-page difference.",
					      (long)_mstats, (long)unlock_mstats);
		}
	}

	_restore_priority_if_possible(cmd);

	_release_memory();
}

static void _lock_mem_if_needed(struct cmd_context *cmd)
{
	log_debug_mem("Lock:   Memlock counters: prioritized:%d locked:%d critical:%d daemon:%d suspended:%d",
		      _priority_raised, _mem_locked, _critical_section, _memlock_count_daemon, dm_get_suspended_counter());
	if (!_mem_locked &&
	    ((_critical_section + _memlock_count_daemon) == 1)) {
		_mem_locked = 1;
		_lock_mem(cmd);
	}
}

static void _unlock_mem_if_possible(struct cmd_context *cmd)
{
	log_debug_mem("Unlock: Memlock counters: prioritized:%d locked:%d critical:%d daemon:%d suspended:%d",
		      _priority_raised, _mem_locked, _critical_section, _memlock_count_daemon, dm_get_suspended_counter());
	if (_mem_locked &&
	    !_critical_section &&
	    !_memlock_count_daemon) {
		_unlock_mem(cmd);
		_mem_locked = 0;
	}
}

/*
 * Critical section is only triggered with suspending reason.
 * Other reasons only raise process priority so the table manipulation
 * remains fast.
 *
 * Memory stays locked until 'memlock_unlock()' is called so when possible
 * it may stay locked across multiple crictical section entrances.
 */
void critical_section_inc(struct cmd_context *cmd, const char *reason)
{
	if (!_critical_section &&
	    (strcmp(reason, "suspending") == 0)) {
		/*
		 * Profiles are loaded on-demand so make sure that before
		 * entering the critical section all needed profiles are
		 * loaded to avoid the disk access later.
		 */
		(void) load_pending_profiles(cmd);
		_critical_section = 1;
		log_debug_activation("Entering critical section (%s).", reason);
		_lock_mem_if_needed(cmd);
	} else
		log_debug_activation("Entering prioritized section (%s).", reason);

	_raise_priority(cmd);
	_prioritized_section++;
}

void critical_section_dec(struct cmd_context *cmd, const char *reason)
{
	if (_critical_section && !dm_get_suspended_counter()) {
		_critical_section = 0;
		log_debug_activation("Leaving critical section (%s).", reason);
	} else
		log_debug_activation("Leaving section (%s).", reason);

	if (_prioritized_section > 0)
		_prioritized_section--;
}

int critical_section(void)
{
	return _critical_section;
}

int prioritized_section(void)
{
	return _prioritized_section;
}

/*
 * The memlock_*_daemon functions will force the mlockall() call that we need
 * to stay in memory, but they will have no effect on device scans (unlike
 * normal critical_section_inc/dec). Memory is kept locked as long as either
 * of critical_section or memlock_daemon is in effect.
 */

void memlock_inc_daemon(struct cmd_context *cmd)
{
	++_memlock_count_daemon;
	if (_memlock_count_daemon == 1 && _critical_section > 0)
		log_error(INTERNAL_ERROR "_memlock_inc_daemon used in critical section.");
	log_debug_mem("memlock_count_daemon inc to %d", _memlock_count_daemon);
	_lock_mem_if_needed(cmd);
	_raise_priority(cmd);
}

void memlock_dec_daemon(struct cmd_context *cmd)
{
	if (!_memlock_count_daemon)
		log_error(INTERNAL_ERROR "_memlock_count_daemon has dropped below 0.");
	--_memlock_count_daemon;
	log_debug_mem("memlock_count_daemon dec to %d", _memlock_count_daemon);
	_unlock_mem_if_possible(cmd);
}

void memlock_init(struct cmd_context *cmd)
{
	/* When threaded, caller already limited stack size so just use the default. */
	_size_stack = 1024ULL * (cmd->threaded ? DEFAULT_RESERVED_STACK :
				 find_config_tree_int(cmd, activation_reserved_stack_CFG, NULL));
	_size_malloc_tmp = find_config_tree_int(cmd, activation_reserved_memory_CFG, NULL) * 1024ULL;
	_default_priority = find_config_tree_int(cmd, activation_process_priority_CFG, NULL);
}

void memlock_reset(void)
{
	log_debug_mem("memlock reset.");
	_mem_locked = 0;
	_priority_raised = 0;
	_critical_section = 0;
	_prioritized_section = 0;
	_memlock_count_daemon = 0;
}

void memlock_unlock(struct cmd_context *cmd)
{
	_unlock_mem_if_possible(cmd);
	_restore_priority_if_possible(cmd);
}

int memlock_count_daemon(void)
{
	return _memlock_count_daemon;
}

#endif

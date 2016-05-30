/* src/vm/os.cpp - system (OS) functions

   Copyright (C) 2007, 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#if defined(__DARWIN__)
# include <mach/mach.h>
# include <mach/mach_host.h>
# include <mach/host_info.h>
#endif

/* this should work on BSD */
/* #include <sys/sysctl.h> */

#include "mm/memory.hpp"
#include "toolbox/logging.hpp"
#include "vm/os.hpp"
#include "vm/vm.hpp"


/**
 * Prints an error message and aborts the VM.
 *
 * @param text Error message to print.
 */
void os::abort(const char* text, ...)
{
	va_list ap;

	// Print the log message.
	log_start();

	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);

	log_finish();

	// Print a backtrace.
	os::print_backtrace();

	// Now abort the VM.
	os::abort();
}

/**
 * Prints a should not reach message and aborts the VM.
 *
 * @param text Error message to print.
 */
void os::shouldnotreach(const char* text, ...)
{
	va_list ap;

	// Print the log message.
	log_start();

	log_print("should not reach: ");
	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);

	log_finish();

	// Print a backtrace.
	os::print_backtrace();

	// Now abort the VM.
	os::abort();
}

/**
 * Prints an unimplemented message and aborts the VM.
 *
 * @param text Error message to print.
 */
void os::unimplemented(const char* text, ...)
{
	va_list ap;

	// Print the log message.
	log_start();

	log_print("not implemented yet: ");
	va_start(ap, text);
	log_vprint(text, ap);
	va_end(ap);

	log_finish();

	// Print a backtrace.
	os::print_backtrace();

	// Now abort the VM.
	os::abort();
}

/**
 * Common code for both os::abort_errnum and os::abort_errno.
 */
static void abort_verrnum(int errnum, const char* text, va_list ap)
{
	// Print the log message.
	log_start();

	log_vprint(text, ap);

	// Print the strerror-message of errnum.
	log_print(": %s", os::strerror(errnum));

	log_finish();

	// Print a backtrace.
	os::print_backtrace();

	// Now abort the VM.
	os::abort();
}

/**
 * Prints an error message, appends ":" plus the strerror-message of
 * errnum and aborts the VM.
 *
 * @param errnum Error number.
 * @param text   Error message to print.
 */
void os::abort_errnum(int errnum, const char* text, ...)
{
	va_list ap;

	va_start(ap, text);
	abort_verrnum(errnum, text, ap);
	va_end(ap);
}


/**
 * Equal to abort_errnum, but uses errno to get the error number.
 *
 * @param text Error message to print.
 */
void os::abort_errno(const char* text, ...)
{
	va_list ap;

	va_start(ap, text);
	abort_verrnum(errno, text, ap);
	va_end(ap);
}


/**
 * Return the current working directory.
 *
 * @return Pointer to a char array allocated by MNEW, or
 *         NULL if memory could not be allocated.
 */
char* os::getcwd(void)
{
	int32_t size = 1024;

	char* buf = MNEW(char, size);

	while (buf != NULL) {
		if (getcwd(buf, size) != NULL)
			return buf;

		MFREE(buf, char, size);

		/* too small buffer or a more serious problem */

		if (errno != ERANGE)
			abort_errno("os::getcwd: getcwd failed");

		/* double the buffer size */

		size *= 2;

		buf = MNEW(char, size);
	}

	return NULL;
}


/**
 * Maps anonymous memory, even on systems not defining
 * MAP_ANON(YMOUS).
 *
 * @param ...
 */
void* os::mmap_anonymous(void *addr, size_t len, int prot, int flags)
{
	void* p;

#if defined(MAP_ANON) || defined(MAP_ANONYMOUS)
	p = mmap(addr, len, prot,
# if defined(MAP_ANON)
			 MAP_ANON | flags,
# else
			 MAP_ANONYMOUS | flags,
# endif
			 -1, 0);
#else
	int fd;

	fd = open("/dev/zero", O_RDONLY, 0);

	if (fd == -1)
		os::abort_errno("os::mmap_anonymous: open failed");

	p = mmap(addr, len, prot, flags, fd, 0);
#endif

#if defined(MAP_FAILED)
	if (p == MAP_FAILED)
#else
	if (p == (void *) -1)
#endif
		os::abort_errno("os::mmap_anonymous: mmap failed");

	return p;
}


/**
 * Print a C backtrace.
 */
void os::print_backtrace()
{
#define BACKTRACE_SIZE 100
	void** array = new void*[SIZEOF_VOID_P * BACKTRACE_SIZE];

	// Get the backtrace.
	int size = backtrace(array, BACKTRACE_SIZE);

	// Resolve the symbols.
	char** strings = backtrace_symbols(array, size);

	log_println("Backtrace (%d stack frames):", size);

	for (int i = 0; i < size; i++)
		log_println("%s", strings[i]);

	// We have to free the strings.
	free(strings);
}


/**
 * Returns the number of online processors in the system.
 *
 * @return Number of online processors.
 */
int os::processors_online(void)
{
#if defined(_SC_NPROC_ONLN)

	return (int) sysconf(_SC_NPROC_ONLN);

#elif defined(_SC_NPROCESSORS_ONLN)

	return (int) sysconf(_SC_NPROCESSORS_ONLN);

#elif defined(__DARWIN__)

	host_basic_info_data_t hinfo;
	mach_msg_type_number_t hinfo_count = HOST_BASIC_INFO_COUNT;
	kern_return_t rc;

	rc = host_info(mach_host_self(), HOST_BASIC_INFO,
				   (host_info_t) &hinfo, &hinfo_count);
 
	if (rc != KERN_SUCCESS) {
		return -1;
	}

	/* XXX michi: according to my infos this should be
	   hinfo.max_cpus, can someone please confirm or deny that? */
	return (int) hinfo.avail_cpus;

#elif defined(__FREEBSD__)
# error IMPLEMENT ME!

	/* this should work in BSD */
	/*
	int ncpu, mib[2], rc;
	size_t len;

	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	len = sizeof(ncpu);
	rc = sysctl(mib, 2, &ncpu, &len, NULL, 0);

	return (int32_t) ncpu;
	*/

#else

	return 1;

#endif
}


// Legacy C interface.

extern "C" {
	void*  os_mmap_anonymous(void *addr, size_t len, int prot, int flags) { return os::mmap_anonymous(addr, len, prot, flags); }

	int    os_atoi(const char* nptr) { return os::atoi(nptr); }
	int    os_getpagesize(void) { return os::getpagesize(); }
	void*  os_memcpy(void* dest, const void* src, size_t n) { return os::memcpy(dest, src, n); }
	void*  os_memset(void* s, int c, size_t n) { return os::memset(s, c, n); }
	char*  os_strdup(const char* s) { return os::strdup(s); }
	int    os_strlen(const char* s) { return os::strlen(s); }

}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */

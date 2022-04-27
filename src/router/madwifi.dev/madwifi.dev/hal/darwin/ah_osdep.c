/*-
 * Copyright (c) 2002-2004 Sam Leffler, Errno Consulting, Atheros
 * Communications, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the following conditions are met:
 * 1. The materials contained herein are unmodified and are used
 *    unmodified.
 * 2. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following NO
 *    ''WARRANTY'' disclaimer below (''Disclaimer''), without
 *    modification.
 * 3. Redistributions in binary form must reproduce at minimum a
 *    disclaimer similar to the Disclaimer below and any redistribution
 *    must be conditioned upon including a substantially similar
 *    Disclaimer requirement for further binary redistribution.
 * 4. Neither the names of the above-listed copyright holders nor the
 *    names of any contributors may be used to endorse or promote
 *    product derived from this software without specific prior written
 *    permission.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT,
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
 * FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES.
 *
 * $Id: //depot/sw/branches/sam_hal/darwin/ah_osdep.c#1 $
 */
#include "opt_ah.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/sysctl.h>
#include <sys/malloc.h>
#include <sys/proc.h>

#include <libkern/libkern.h>
#include <libkern/OSAtomic.h>

#include <net/ethernet.h>		/* XXX for ether_sprintf */

#include "ah.h"

extern void IOLog(const char *format, ...);

extern	void ath_hal_printf(struct ath_hal *, const char*, ...)
	__printflike(2,3);
extern	void ath_hal_vprintf(struct ath_hal *, const char*, __va_list)
	__printflike(2, 0);
extern	const char* ath_hal_ether_sprintf(const u_int8_t *mac);
extern	void *ath_hal_malloc(size_t);
extern	void ath_hal_free(void *);

#ifdef AH_ASSERT
extern	void ath_hal_assert_failed(const char* filename,
				   int lineno, const char* msg);
#endif

#ifdef AH_DEBUG
extern	void HALDEBUG(struct ath_hal *ah, const char* fmt, ...);
extern	void HALDEBUGn(struct ath_hal *ah, u_int level, const char* fmt, ...);
#endif /* AH_DEBUG */


int	ath_hal_dma_beacon_response_time = 2;	/* in TU's */
int	ath_hal_sw_beacon_response_time = 10;	/* in TU's */
int	ath_hal_additional_swba_backoff = 0;	/* in TU's */
static	int ath_hal_debug = 0;


void*
ath_hal_malloc(size_t size)
{
	char *ptr;

	/*
	 * align to word boundary and add an extra int for
	 * size storage.
	 */
	size = (size + ((sizeof(int)) << 1)) & ~(sizeof(int)); 
	ptr = (char *) IOMalloc(size);
	if (ptr) {
		*((int *)ptr) = size; 
		ptr += sizeof(int);
		memset(ptr, 0, size-sizeof(int));
	}
	return((void *) ptr);
}

void
ath_hal_free(void* p)
{
	p = (char *)p - sizeof(int);
	IOFree(p,*(int *)p);
}


void
ath_hal_vprintf(struct ath_hal *ah, const char* fmt, va_list ap)
{
	char buf[256];		/* enough? */
	vsnprintf(buf, sizeof(buf), fmt, ap);
	IOLog("%s",buf);
}

void
ath_hal_printf(struct ath_hal *ah, const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	ath_hal_vprintf(ah, fmt, ap);
	va_end(ap);
}

void __ahdecl
ath_hal_printstr(struct ath_hal *ah, const char *str)
{
	ath_hal_printf("%s", str);
}

const char*
ath_hal_ether_sprintf(const u_int8_t *mac)
{
	return (const char *) ether_sprintf(mac);
}

#ifdef AH_DEBUG
void
HALDEBUG(struct ath_hal *ah, const char* fmt, ...)
{
	if (ath_hal_debug) {
		__va_list ap;
		va_start(ap, fmt);
		ath_hal_vprintf(ah, fmt, ap);
		va_end(ap);
	}
}

void
HALDEBUGn(struct ath_hal *ah, u_int level, const char* fmt, ...)
{
	if (ath_hal_debug >= level) {
		__va_list ap;
		va_start(ap, fmt);
		ath_hal_vprintf(ah, fmt, ap);
		va_end(ap);
	}
}
#endif /* AH_DEBUG */

#if defined(AH_DEBUG) || defined(AH_REGOPS_FUNC)
/*
 * Memory-mapped device register read/write.  These are here
 * as routines when debugging support is enabled and/or when
 * explicitly configured to use function calls.  The latter is
 * for architectures that might need to do something before
 * referencing memory (e.g. remap an i/o window).
 *
 * NB: see the comments in ah_osdep.h about byte-swapping register
 *     reads and writes to understand what's going on below.
 */


#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#error("Unknown byte order")
#endif

#if defined(__LITTLE_ENDIAN__) && defined(__BIG_ENDIAN__)
#error("Two byte orders defined?")
#endif

void
ath_hal_reg_write(struct ath_hal *ah, u_int32_t reg, u_int32_t val)
{
	OSSynchronizeIO();
	*((volatile u_int32_t *)(ah->ah_sh + reg)) = val;
	OSSynchronizeIO();
}
§
u_int32_t
ath_hal_reg_read(struct ath_hal *ah, u_int32_t reg)
{
	volatile u_int32_t val = 0;

	OSSynchronizeIO();
	val = *((volatile u_int32_t *)(ah->ah_sh + reg));
	OSSynchronizeIO();
	return val;
}

#endif /* AH_DEBUG || AH_REGOPS_FUNC */

#ifdef AH_ASSERT
void
ath_hal_assert_failed(const char* filename, int lineno, const char *msg)
{
	printf("Atheros HAL assertion failure: %s: line %u: %s\n",
		filename, lineno, msg);
	panic("ath_hal_assert");
}
#endif /* AH_ASSERT */

/*
 * Delay n microseconds.
 */
void
ath_hal_delay(int n)
{
	IODelay(n);
} 

u_int32_t
ath_hal_getuptime(struct ath_hal *ah)
{
	struct timeval tv;
	microuptime(&tv);
	return tv.tv_sec * 1000 + (tv.tv_usec / 100);
}

void
ath_hal_memzero(void *dst, size_t n)
{
	bzero(dst, n);
}

void *
ath_hal_memcpy(void *dst, const void *src, size_t n)
{
	return memcpy(dst, src, n);
}

int ath_hal_memcmp(const void *a, const void *b, size_t n)
{
	return memcmp(a, b, n);
}

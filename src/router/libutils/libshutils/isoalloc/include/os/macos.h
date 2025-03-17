/* macos.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

#include <libkern/OSByteOrder.h>
#include <mach/vm_statistics.h>
#include <pthread.h>
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)
#define ENVIRON NULL

/*
 * On pthread_exit, it expects to be dealing with libmalloc
 * rather than isoalloc at releasing time
 */
__attribute__((weak)) void _pthread_tsd_cleanup(pthread_t);
void _pthread_tsd_cleanup(pthread_t p)
{
    (void)p;
}

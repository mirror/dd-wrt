/* linux.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

#include <sys/prctl.h>
#include <sys/auxv.h>
#include <byteswap.h>
/* Get linux kernel version */
#include <linux/version.h>

#if defined(CPU_PIN) && defined(_GNU_SOURCE) && defined(__linux__)
#include <sched.h>
#endif

/* In Linux kernel versions greater than 5.17.0, it is also possible 
 * to name anonymous VMA. See
 * https://kernelnewbies.org/Linux_5.17#Support_giving_names_to_anonymous_memory */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
/* Use this macro can allow for compatibility with older versions while 
 * introducing new functionality */
#define KERNEL_VERSION_SEQ_5_17 1
/* Same as android.h. */
#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#endif
#ifndef PR_SET_VMA_ANON_NAME
#define PR_SET_VMA_ANON_NAME 0
#endif
#endif

#define ENVIRON environ

#if (__clang_major__ >= 12 && defined(__aarch64__) && !defined(__ILP32__)) && ARM_MTE == 1
/* TODO: these belong in an aarch specific header */
#ifndef PROT_MTE
#define PROT_MTE 0x20
#endif
#ifndef PR_SET_TAGGED_ADDR_CTRL
#define PR_SET_TAGGED_ADDR_CTRL 54
#endif
#ifndef PR_GET_TAGGED_ADDR_CTRL
#define PR_GET_TAGGED_ADDR_CTRL 56
#endif
#ifndef PR_TAGGED_ADDR_ENABLE
#define PR_TAGGED_ADDR_ENABLE (1UL << 0)
#endif
#ifndef PR_MTE_TCF_SHIFT
#define PR_MTE_TCF_SHIFT 1
#endif
#ifndef PR_MTE_TAG_SHIFT
#define PR_MTE_TAG_SHIFT 3
#endif
#ifndef PR_MTE_TCF_NONE
#define PR_MTE_TCF_NONE (0UL << PR_MTE_TCF_SHIFT)
#endif
#ifndef PR_MTE_TCF_SYNC
#define PR_MTE_TCF_SYNC (1UL << PR_MTE_TCF_SHIFT)
#endif
#ifndef PR_MTE_TCF_MASK
#define PR_MTE_TCF_MASK (3UL << PR_MTE_TCF_SHIFT)
#endif
#endif

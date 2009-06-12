/*
 ***************************************************************************
 *  triscend_a7.h
 *
 *  Copyright (c) 2000-2003 Triscend Corporation. All rights reserved.
 *
 ***************************************************************************
 */

#if defined(A7VL) || defined(A7VE) || defined(A7VC) || defined(A7VT)
#ifdef CONFIG_USE_A7HAL
#include <asm/arch/a7hal/triscend_a7v.h>
#else
#include <asm/arch/triscend_a7v.h>
#endif
#else
#ifdef CONFIG_USE_A7HAL
#include <asm/arch/a7hal/triscend_a7s.h>
#else
#include <asm/arch/triscend_a7s.h>
#endif
#endif

#ifndef INLINE
#ifdef _DIAB
#define INLINE
#elif defined( __ARM__ )
#define INLINE	__inline
#else
#define INLINE	__inline__
#endif
#endif

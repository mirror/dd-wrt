#ifndef CYGONCE_HAL_DIAG_H
#define CYGONCE_HAL_DIAG_H

/*=============================================================================
//
//      hal_diag.h
//
//      HAL Support for Kernel Diagnostic Routines
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_if.h>

#define HAL_DIAG_INIT() hal_if_diag_init()
#if 0
#define HAL_DIAG_WRITE_CHAR(_c_) hal_if_diag_write_char(_c_)
#else
#define HAL_DIAG_WRITE_CHAR(_c_) \
do { \
	int _lsr; \
	do { \
		HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_UART_LSR), _lsr); \
	} while ((_lsr & 0x20) == 0); \
	HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_UART_RBR), _c_); \
} while(0); 
#endif
#define HAL_DIAG_READ_CHAR(_c_) hal_if_diag_read_char(&_c_)

//-----------------------------------------------------------------------------
// end of hal_diag.h
#endif // CYGONCE_HAL_DIAG_H

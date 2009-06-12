;;delay_us

#include <pkgconf/hal.h>	

#include <cyg/hal/arch.inc>	
#include <cyg/hal/basetype.h>
#include <cyg/hal/mod_regs_tmr.h>

#define DELAY_COUNT (CYGHWR_HAL_H8300_PROCESSOR_SPEED/1000000)/8*25
	
//	.file	"delay_us.S"
	.h8300s
	.text

	.global	CYG_LABEL_DEFN(hal_delay_us)
CYG_LABEL_DEFN(hal_delay_us):
	shlr.l	er0
	shlr.l	er0
	shlr.l	er0
	mov.l	er0,er0
1:	
	ble	4f
	dec.l	#1,er0
	bra	1b
4:	
	rts

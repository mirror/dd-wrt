;;delay_us

#include <pkgconf/hal.h>	

#include <cyg/hal/arch.inc>	
#include <cyg/hal/basetype.h>
#include <cyg/hal/mod_regs_tmr.h>

#define DELAY_COUNT (CYGHWR_HAL_H8300_PROCESSOR_SPEED/1000000)/8*25
	
//	.file	"delay_us.S"
	.h8300h
	.text

	.global	CYG_LABEL_DEFN(hal_delay_us)
CYG_LABEL_DEFN(hal_delay_us):
	mov.b	#0,r1l
	mov.b	r1l,@CYGARC_8TCNT2:8
	mov.b	#1,r1l
	mov.b	r1l,@CYGARC_8TCR2:8
	mov.b	#0,r2h
	sub.w	e1,e1
1:
	mov.l	er0,er0
	ble	6f
2:	
	cmp.w	#DELAY_COUNT,e1
	bgt	5f
	mov.b	#0,r1h
	mov.b	@CYGARC_8TCNT2:8,r2l
3:	
	mov.b	@CYGARC_8TCNT2:8,r1l
	cmp.b	r1l,r2l
	beq	3b
	bcs	4f
	inc.b	r1h
4:
	sub.w	r2,r1
	add.w	r1,e1
	bra	2b
5:
	mov.w	e1,r1
	mov.b	#DELAY_COUNT,r2l
	divxs	r2l,r1
	sub.l	er2,er2
	mov.b	r1l,r2l
	sub.w	r2,e1
	dec.l	#1,er0
	bra	1b
6:
	mov.b	#0,r0l
	mov.b	r0l,@CYGARC_8TCR2
	rts

#ifdef __H8300S__
	.h8300s
#else
	.h8300h
#endif
	.text
	
.global __setjmp
.global ___sigsetjmp
.global ___longjmp

__setjmp:
___sigsetjmp:
	add.l	#20,er0
	mov.l	er6,@er0
	mov.l	er5,@-er0
	mov.l	er4,@-er0
	mov.l	er3,@-er0
	mov.l	er2,@-er0
	mov.l	er1,@-er0
	mov.l	sp,@(24,er0)
	mov.l	@sp,er1
	mov.l	er1,@(28,er0)
	sub.l	er0,er0
	rts

___longjmp:
	mov.l	er1,er1
	bne	1f
	sub.l	er1,er1
	inc.l	#1,er1
1:
	adds	#4,er0
	adds	#4,er0
	mov.l	@er0+,er3
	mov.l	@er0+,er4
	mov.l	@er0+,er5
	mov.l	@er0+,er6
	mov.l	@er0+,sp
	mov.l	@er0,er2
	mov.l	er2,@sp
	mov.l	er1,er2
	mov.l	er0,er1
	mov.l	er2,er0
	sub.l	#28,er1
	mov.l	@(4,er1),er2
	mov.l	@er1,er1
	rts


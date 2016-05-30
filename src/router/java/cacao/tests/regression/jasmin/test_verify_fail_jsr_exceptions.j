.class public test_verify_fail_jsr_exceptions
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static checkI(I)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	iload_0
	invokevirtual java/io/PrintStream/println(I)V
	return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 3
	.limit locals 3

	.catch java/lang/Exception from test_start to test_end using handler

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------
	jsr sbr_1
	ldc 111
	invokestatic test_verify_fail_jsr_exceptions/checkI(I)V
	return

continue_1:
	jsr sbr_2
	ldc 222
	invokestatic test_verify_fail_jsr_exceptions/checkI(I)V
	return

test_start:
continue_2:

; ERROR: VerifyError
; This fails, because the "astore 1" is _after_ the exception range boundary

	astore 1
	ldc 2
	ldc 0
	idiv
	pop

	astore 1
	ldc 2
	ldc 0
	idiv
	pop
	return

test_end:
	; --------------------------------------------------

force_basic_block_boundary:

	return
	
sbr_1:
	goto continue_1

sbr_2:
	goto continue_2

handler:
	pop
	ret 1

.end method


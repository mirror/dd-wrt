.class public test_verify_ok_jsr_handler_in_sub2
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
	.limit stack 2
	.limit locals 3

	.catch java/lang/Exception from test_start to test_end using handler

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	jsr sbr_1
	; OUTPUT: 48
	iload 1
	invokestatic test_verify_ok_jsr_handler_in_sub2/checkI(I)V
	; OUTPUT: 35

	ldc 1234
	istore 1

	jsr sbr_1
	; OUTPUT: 48
	iload 1
	invokestatic test_verify_ok_jsr_handler_in_sub2/checkI(I)V
	; OUTPUT: 1234

	; --------------------------------------------------

force_basic_block_boundary:

	return
	
sbr_1:
	astore 2
	ldc 2
	ldc 0
test_start:
	idiv
test_end:
	pop
	ldc 666
	invokestatic test_verify_ok_jsr_handler_in_sub2/checkI(I)V
	return

handler:
	pop
	ldc 48
	invokestatic test_verify_ok_jsr_handler_in_sub2/checkI(I)V
	ret 2

.end method

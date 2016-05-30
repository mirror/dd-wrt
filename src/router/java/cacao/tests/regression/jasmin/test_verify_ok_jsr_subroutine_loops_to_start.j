.class public test_verify_ok_jsr_subroutine_loops_to_start
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static check(I)V
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

	iconst_0
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	jsr sbr_1
	; OUTPUT: 111
	; OUTPUT: 222

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_verify_ok_jsr_subroutine_loops_to_start/check(I)V
	; OUTPUT: 1

	return

sbr_1:
	dup
	astore 2

	iload 1
	ifeq first_time

second_time:
	pop
	ldc 222
	invokestatic test_verify_ok_jsr_subroutine_loops_to_start/check(I)V
	ret 2

first_time:
	ldc 111
	invokestatic test_verify_ok_jsr_subroutine_loops_to_start/check(I)V
	iinc 1 1
	goto sbr_1

.end method


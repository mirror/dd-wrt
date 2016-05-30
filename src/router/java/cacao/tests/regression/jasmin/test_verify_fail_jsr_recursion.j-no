.class public test_verify_fail_jsr_recursion
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
	.limit locals 4

	ldc 0
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	jsr sbr_1
	jsr sbr_1

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_verify_fail_jsr_recursion/check(I)V

	return

sbr_1:
	astore 2
	iload 1
	invokestatic test_verify_fail_jsr_recursion/check(I)V
	iload 1
	ifne  second_time
	iinc 1 1
	jsr sbr_1
	; ERROR: VerifyError

second_time:
	ret 2

.end method


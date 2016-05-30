.class public test_verify_fail_jsr_called_with_different_stackdepths
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

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	jsr jsr_1

	ldc 123
	jsr jsr_1
	; ERROR: VerifyError
	pop

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_verify_fail_jsr_called_with_different_stackdepths/check(I)V

	return

jsr_1:
	astore 2
	iload 1
	invokestatic test_verify_fail_jsr_called_with_different_stackdepths/check(I)V
	iinc 1 1
	ret 2

.end method

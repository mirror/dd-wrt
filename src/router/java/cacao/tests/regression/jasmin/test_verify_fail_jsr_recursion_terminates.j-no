.class public test_verify_fail_jsr_recursion_terminates
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

.method public static check(Ljava/lang/String;)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	aload_0
	invokevirtual java/io/PrintStream/println(Ljava/lang/String;)V
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
	invokestatic test_verify_fail_jsr_recursion_terminates/check(I)V

	return

sbr_1:
	ldc "entry"
	invokestatic test_verify_fail_jsr_recursion_terminates/check(Ljava/lang/String;)V
	iload 1
	ifne second_time

	astore 2
	ldc "first"
	invokestatic test_verify_fail_jsr_recursion_terminates/check(Ljava/lang/String;)V
	iinc 1 1
	jsr sbr_1
	; ERROR: VerifyError
	ret 2

second_time:
	astore 3
	ldc "second"
	invokestatic test_verify_fail_jsr_recursion_terminates/check(Ljava/lang/String;)V
	ret 3

.end method


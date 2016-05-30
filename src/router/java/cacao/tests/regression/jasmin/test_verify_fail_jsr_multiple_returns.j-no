.class public test_verify_fail_jsr_multiple_returns
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
	invokestatic test_verify_fail_jsr_multiple_returns/check(I)V

	return

sbr_1:
	astore 2
	ldc "one"
	invokestatic test_verify_fail_jsr_multiple_returns/check(Ljava/lang/String;)V
	jsr sbr_2
	ldc "one-B"
	invokestatic test_verify_fail_jsr_multiple_returns/check(Ljava/lang/String;)V
	iinc 1 1
	ret 2

sbr_2:
	astore 3
	ldc "two"
	invokestatic test_verify_fail_jsr_multiple_returns/check(Ljava/lang/String;)V
	iload 1
	ifne second_time
	ret 3

second_time:
	ret 2
	; ERROR: VerifyError

.end method


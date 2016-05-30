.class public test_verify_fail_long_local
.super java/lang/Object

; long local overwrites int local in second half

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

	ldc 42
	istore 2

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc2_w 1234567890987654321
	lstore 1

	; --------------------------------------------------

force_basic_block_boundary:

	iload 2
	; ERROR: VerifyError
	invokestatic test_verify_fail_long_local/check(I)V

	return
.end method

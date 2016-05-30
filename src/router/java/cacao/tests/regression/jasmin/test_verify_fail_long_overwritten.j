.class public test_verify_fail_long_overwritten
.super java/lang/Object

; test: second half of long is overwritten by an int

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static check(J)V
	.limit locals 2
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	lload_0
	invokevirtual java/io/PrintStream/println(J)V
	return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 3

	ldc2_w 1234567890987654321
	lstore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc 42
	istore 2

	; --------------------------------------------------

force_basic_block_boundary:

	lload 1
	; ERROR: VerifyError
	invokestatic test_verify_fail_long_overwritten/check(J)V

	return
.end method

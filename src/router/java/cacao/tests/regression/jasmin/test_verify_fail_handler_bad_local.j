.class public test_verify_fail_handler_bad_local
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
	.limit locals 2

	.catch java/lang/Throwable from start_range to end_range using handler

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

start_range:
	ldc 42
	istore 1
	iload 1
	invokestatic test_verify_fail_handler_bad_local/check(I)V
	; reaches handler

	; comment these two instructions to make it typesafe
	aconst_null
	astore 1

	ldc 123
	invokestatic test_verify_fail_handler_bad_local/check(I)V
	; reaches handler
	; ERROR: VerifyError
end_range:

	; --------------------------------------------------

force_basic_block_boundary:

	return

handler:
	pop
	iload 1
	invokestatic test_verify_fail_handler_bad_local/check(I)V
	ldc 8888
	invokestatic test_verify_fail_handler_bad_local/check(I)V
	return

.end method


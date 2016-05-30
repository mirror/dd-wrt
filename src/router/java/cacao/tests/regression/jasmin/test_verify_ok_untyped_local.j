.class public test_verify_ok_untyped_local
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
	.limit locals 2

	ldc "will it work?"
	astore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc 42
	istore 1

	iload 1
	invokestatic test_verify_ok_untyped_local/check(I)V
	; OUTPUT: 42

	ldc "wow, it works!"
	astore 1

	; --------------------------------------------------

force_basic_block_boundary:

	aload 1
	invokestatic test_verify_ok_untyped_local/check(Ljava/lang/String;)V
	; OUTPUT: wow, it works!

	return
.end method


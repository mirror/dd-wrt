.class public test_dup
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
	.limit stack 3
	.limit locals 2

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc 32
	ldc 91
	dup
	isub
	invokestatic test_dup/check(I)V
	; OUTPUT: 0
	invokestatic test_dup/check(I)V
	; OUTPUT: 32

	; --------------------------------------------------

force_basic_block_boundary:

	return
.end method

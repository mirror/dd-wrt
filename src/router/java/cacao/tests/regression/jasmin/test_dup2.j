.class public test_dup2
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
	.limit stack 5
	.limit locals 2

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc 32
	ldc 91
	dup2 ; 32 91 32 91 (top)
	isub ; 32 91 -59 (top)
	invokestatic test_dup2/check(I)V
	; OUTPUT: -59
	invokestatic test_dup2/check(I)V
	; OUTPUT: 91
	invokestatic test_dup2/check(I)V
	; OUTPUT: 32

	ldc 32
	ldc2_w 91
	dup2 ; 32 91 91 (top)
	lsub ; 32 91 0 (top)
	invokestatic test_dup2/check(J)V
	; OUTPUT: 0
	invokestatic test_dup2/check(I)V
	; OUTPUT: 32

	; --------------------------------------------------

force_basic_block_boundary:

	return
.end method

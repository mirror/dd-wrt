.class public test_dup_x1
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static checkI(I)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	iload_0
	invokevirtual java/io/PrintStream/println(I)V
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
	dup_x1  ; 91 32 91
	isub
	invokestatic test_dup_x1/checkI(I)V
	; OUTPUT: -59
	invokestatic test_dup_x1/checkI(I)V
	; OUTPUT: 91

	; --------------------------------------------------

force_basic_block_boundary:

	return
.end method

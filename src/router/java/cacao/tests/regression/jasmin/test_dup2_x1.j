.class public test_dup2_x1
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
	.limit stack 6
	.limit locals 2

	ldc 35
	istore 1

	ldc 1032
	ldc 1091
	ldc 1047

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	dup2_x1 ; 1091 1047 1032 1091 1047

	aload 0
	ifnull force_basic_block_boundary2

	; --------------------------------------------------

	isub
	invokestatic test_dup2_x1/checkI(I)V
	; OUTPUT: 44
	isub
	invokestatic test_dup2_x1/checkI(I)V
	; OUTPUT:  15
	invokestatic test_dup2_x1/checkI(I)V
	; OUTPUT:  1091

	ldc 32
	ldc 91
	ldc 47
	dup2_x1 ; 91 47 32 91 47
	isub
	invokestatic test_dup2_x1/checkI(I)V
	; OUTPUT: 44
	isub
	invokestatic test_dup2_x1/checkI(I)V
	; OUTPUT:  15
	invokestatic test_dup2_x1/checkI(I)V
	; OUTPUT:  91

	return

	; --------------------------------------------------

force_basic_block_boundary:
	return

force_basic_block_boundary2:
	return

.end method

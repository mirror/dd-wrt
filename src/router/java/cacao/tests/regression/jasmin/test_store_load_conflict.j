.class public test_store_load_conflict
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
	.limit stack 2
	.limit locals 3

	ldc 35
	istore 1
	ldc 777
	istore 2

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	iload 1   ; loads 35
	ldc 42
	istore 1  ; stores 42
	ldc 100
	iadd      ; 35 + 100 = 135
	istore 2

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_store_load_conflict/checkI(I)V
	; OUTPUT: 42

	iload 2
	invokestatic test_store_load_conflict/checkI(I)V
	; OUTPUT: 135

	return
.end method

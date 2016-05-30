.class public test_load_store_conflict_via_swap
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
	.limit stack 4
	.limit locals 3

	ldc 35
	istore 1
	ldc 777
	istore 2

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	iload 1  ; loads 35
	ldc 42
	swap     ; stack is now 42 35 (top)
	istore 2 ; stores 35 to L2
	istore 1 ; stores 42 to L2

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_load_store_conflict_via_swap/checkI(I)V
	; OUTPUT: 42

	iload 2
	invokestatic test_load_store_conflict_via_swap/checkI(I)V
	; OUTPUT: 35

	return
.end method

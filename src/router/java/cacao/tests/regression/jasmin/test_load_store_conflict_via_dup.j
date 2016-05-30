.class public test_load_store_conflict_via_dup
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
	dup2     ; stack is now 35 42 35 42 (top)
	iadd     ; stack is now 35 42 77
	istore 2
	istore 1
	pop

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_load_store_conflict_via_dup/checkI(I)V
	; OUTPUT: 42

	iload 2
	invokestatic test_load_store_conflict_via_dup/checkI(I)V
	; OUTPUT: 77

	return
.end method

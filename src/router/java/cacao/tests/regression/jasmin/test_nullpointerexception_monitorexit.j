.class public test_nullpointerexception_monitorexit
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

	.catch java/lang/NullPointerException from ex_start to ex_end using handler

	ldc 42
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

ex_start:
	aconst_null
	monitorexit
ex_end:

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_nullpointerexception_monitorexit/check(I)V
	return

handler:
	
	ldc 123
	invokestatic test_nullpointerexception_monitorexit/check(I)V
	; OUTPUT: 123
	return

.end method

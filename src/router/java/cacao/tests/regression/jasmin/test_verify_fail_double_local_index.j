.class public test_verify_fail_double_local_index
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 2

	ldc 42
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc2_w 123.456
	dstore 1
	; ERROR: VerifyError

	; --------------------------------------------------

force_basic_block_boundary:

	return
.end method

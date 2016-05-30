.class public test_verify_fail_aload_retaddress
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
	.limit locals 3

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	jsr sbr_1

	; --------------------------------------------------

force_basic_block_boundary:

	return
	
sbr_1:
	astore 2
	aload 2 
	pop
	ret 2
	; ERROR: VerifyError

.end method

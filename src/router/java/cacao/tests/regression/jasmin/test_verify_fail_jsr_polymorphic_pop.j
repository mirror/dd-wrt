.class public test_verify_fail_jsr_polymorphic_pop
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
	.limit locals 3

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc 123
	jsr sbr_1

	aconst_null
	jsr sbr_1
	; ERROR: VerifyError

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_verify_fail_jsr_polymorphic_pop/check(I)V

	return

sbr_1:
	astore 2
	pop
	iload 1
	invokestatic test_verify_fail_jsr_polymorphic_pop/check(I)V
	iinc 1 1
	ret 2

.end method


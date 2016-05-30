.class public test_verify_fail_jsr_merge_subroutines_via_stack
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

	jsr sbr_1

	jsr sbr_2

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_verify_fail_jsr_merge_subroutines_via_stack/check(I)V

	return

sbr_1:
	iload 1
	invokestatic test_verify_fail_jsr_merge_subroutines_via_stack/check(I)V
	goto common_ret
	; ERROR: VerifyError

sbr_2:
	ldc 42
	invokestatic test_verify_fail_jsr_merge_subroutines_via_stack/check(I)V

common_ret:
	astore 2
	iinc 1 1
	ret 2

.end method


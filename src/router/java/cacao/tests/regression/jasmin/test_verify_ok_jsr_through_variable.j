.class public test_verify_ok_jsr_through_variable
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

.method public static check(J)V
	.limit locals 2
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	lload_0
	invokevirtual java/io/PrintStream/println(J)V
	return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 6

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	ldc 56
	istore 3
	jsr sbr_1
	; OUTPUT: 35
	iload 3
	invokestatic test_verify_ok_jsr_through_variable/check(I)V
	; OUTPUT: 56

	ldc2_w 123456789123456789
	lstore 2
	jsr sbr_1
	; OUTPUT: 35
	lload 2
	invokestatic test_verify_ok_jsr_through_variable/check(J)V
	; OUTPUT: 123456789123456789

	; --------------------------------------------------

force_basic_block_boundary:

	return
	
sbr_1:
	astore 5
	iload 1
	invokestatic test_verify_ok_jsr_through_variable/check(I)V
	ret 5

.end method

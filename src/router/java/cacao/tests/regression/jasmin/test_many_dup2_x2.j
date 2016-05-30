.class public test_many_dup2_x2
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
	.limit stack 64
	.limit locals 1

	ldc 100
	ldc 101
	ldc 42
	ldc -1

	; 30 times

	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2

	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2

	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2

	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 42
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 101
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 100

	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 42

	; pop 56 slots

	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2

	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2

	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2
	pop2

	; Play it again, Sam!

	ldc 100
	ldc 101

	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2

	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2

	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2
	dup2_x2

	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 101
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 100
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 42

	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 101
	invokestatic test_many_dup2_x2/check(I)V
	; OUTPUT: 100

	return
.end method

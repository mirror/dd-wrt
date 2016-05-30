.class public test_many_dup2
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
	.limit stack 62
	.limit locals 2

	ldc 42
	ldc -1

	; 30 times

	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2

	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2

	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2

	invokestatic test_many_dup2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2/check(I)V
	; OUTPUT: 42

	invokestatic test_many_dup2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2/check(I)V
	; OUTPUT: 42

	invokestatic test_many_dup2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2/check(I)V
	; OUTPUT: 42

	; pop 27 times

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

	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2

	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2

	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2
	dup2

	invokestatic test_many_dup2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2/check(I)V
	; OUTPUT: 42

	invokestatic test_many_dup2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2/check(I)V
	; OUTPUT: 42

	invokestatic test_many_dup2/check(I)V
	; OUTPUT: -1
	invokestatic test_many_dup2/check(I)V
	; OUTPUT: 42

	return
.end method

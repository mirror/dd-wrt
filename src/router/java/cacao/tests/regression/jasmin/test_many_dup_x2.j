.class public test_many_dup_x2
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
	.limit stack 33
	.limit locals 2

	ldc 100
	ldc 101
	ldc 42

	; 30 times

	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2

	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2

	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2

	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 42
	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 101
	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 100

	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 42
	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 42

	; pop 27 slots

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

	pop
	pop2
	pop2
	pop2

	; Play it again, Sam!

	; 42
	ldc 100
	ldc 101

	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2

	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2

	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2
	dup_x2

	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 101
	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 100
	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 42

	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 101
	invokestatic test_many_dup_x2/check(I)V
	; OUTPUT: 101

	return
.end method

.class public test_many_dup
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
	.limit stack 31
	.limit locals 2

	ldc 42

	; 30 times

	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup

	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup

	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup

	invokestatic test_many_dup/check(I)V
	; OUTPUT: 42
	invokestatic test_many_dup/check(I)V
	; OUTPUT: 42

	; pop 28 slots

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

	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup

	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup

	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup
	dup

	invokestatic test_many_dup/check(I)V
	; OUTPUT: 42
	invokestatic test_many_dup/check(I)V
	; OUTPUT: 42

	return
.end method

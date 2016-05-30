.class public test_many_swap
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

	ldc 42
	ldc -1

	; 31 times

	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap

	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap

	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap
	swap

	swap

	invokestatic test_many_swap/check(I)V
	; OUTPUT: 42
	invokestatic test_many_swap/check(I)V
	; OUTPUT: -1

	return
.end method

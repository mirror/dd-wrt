.class public test_swap_locals
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static checkI(I)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	iload_0
	invokevirtual java/io/PrintStream/println(I)V
	return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 3
	.limit locals 4

	ldc 4
	istore 1

	ldc 18
	istore 2
	ldc 100
	istore 3

	; --------------------------------------------------

loop:
	iload 2
	iload 3
	swap
	istore 3
	istore 2
	iinc 1 -1
	iload 1
	ifge loop

	; --------------------------------------------------

	iload 2
	invokestatic test_swap_interface_slots/checkI(I)V
	; OUTPUT: 100
	iload 3
	invokestatic test_swap_interface_slots/checkI(I)V
	; OUTPUT: 18

	ldc 7
	istore 1

	ldc 28
	istore 2
	ldc 200
	istore 3

	; --------------------------------------------------

loop2:
	iinc 1 -1
	iload 2
	iload 3
	swap
	istore 3
	istore 2
	iload 1
	ifge loop2

	; --------------------------------------------------

	iload 2
	invokestatic test_swap_interface_slots/checkI(I)V
	; OUTPUT: 28
	iload 3
	invokestatic test_swap_interface_slots/checkI(I)V
	; OUTPUT: 200

	return
.end method

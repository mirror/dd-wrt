.class public test_dup_x1_interface_slots
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
	.limit locals 2

	ldc 4
	istore 1

	ldc 18
	ldc 100

	; --------------------------------------------------

loop:
	dup_x1
	iinc 1 -1
	pop
	iload 1
	ifge loop

	; --------------------------------------------------

	invokestatic test_dup_x1_interface_slots/checkI(I)V
	; OUTPUT: 18
	invokestatic test_dup_x1_interface_slots/checkI(I)V
	; OUTPUT: 100

	ldc 7
	istore 1

	ldc 28
	ldc 200

	; --------------------------------------------------

loop2:
	iinc 1 -1
	dup_x1
	pop
	iload 1
	ifge loop2

	; --------------------------------------------------

	invokestatic test_dup_x1_interface_slots/checkI(I)V
	; OUTPUT: 200
	invokestatic test_dup_x1_interface_slots/checkI(I)V
	; OUTPUT: 28

	return
.end method

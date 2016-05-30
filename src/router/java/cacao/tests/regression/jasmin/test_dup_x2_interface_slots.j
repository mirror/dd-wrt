.class public test_dup_x2_interface_slots
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
	.limit stack 4
	.limit locals 2

	ldc 4
	istore 1

	ldc 18
	ldc 100
	ldc 91

	; stack is 18 100 91 (top)

	; --------------------------------------------------

	; "ROT3" performed 5 times
loop:
	dup_x2
	iinc 1 -1
	pop
	iload 1
	ifge loop

	; --------------------------------------------------

	; stack should be 100 91 18 (top)

	invokestatic test_dup_x2_interface_slots/checkI(I)V
	; OUTPUT: 18
	invokestatic test_dup_x2_interface_slots/checkI(I)V
	; OUTPUT: 91
	invokestatic test_dup_x2_interface_slots/checkI(I)V
	; OUTPUT: 100

	ldc 6
	istore 1

	ldc 28
	ldc 200
	ldc 291

	; stack is 28 200 291 (top)

	; --------------------------------------------------

	; "ROT3" performed 7 times
loop2:
	iinc 1 -1
	dup_x2
	pop
	iload 1
	ifge loop2

	; --------------------------------------------------

	; stack should be 291 28 200

	invokestatic test_dup_x2_interface_slots/checkI(I)V
	; OUTPUT: 200
	invokestatic test_dup_x2_interface_slots/checkI(I)V
	; OUTPUT: 28
	invokestatic test_dup_x2_interface_slots/checkI(I)V
	; OUTPUT: 291

	return
.end method


.class public test_many_monitors
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

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
	.limit stack 2
	.limit locals 2

	ldc 567
	istore 1

	new test
	dup
	invokespecial test/<init>()V

	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter
	dup
	monitorenter

	iload 1
	invokestatic test_many_monitors/checkI(I)V
	iinc 1 2
	; OUTPUT: 567

	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit
	dup
	monitorexit

	iload 1
	invokestatic test_many_monitors/checkI(I)V
	; OUTPUT: 569

	return
.end method

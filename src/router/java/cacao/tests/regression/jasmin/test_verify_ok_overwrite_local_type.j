.class public test_verify_ok_overwrite_local_type
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
	.limit stack 2
	.limit locals 2

	ldc "a string"
	astore 1

	ldc 42
	istore 1

	iload 1
	invokestatic test_verify_ok_overwrite_local_type/checkI(I)V
	; OUTPUT: 42

	return
.end method

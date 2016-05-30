.class public test_verify_fail_invoke_return_basic_type
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

.method public static foo()Ljava/lang/String;
	.limit locals 0
	.limit stack 1
	ldc "value"
	areturn
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 2

	invokestatic test_verify_fail_invoke_return_basic_type/foo()Ljava/lang/String;

	invokestatic test_verify_fail_invoke_return_basic_type/checkI(I)V
	; ERROR: VerifyError

	return
.end method

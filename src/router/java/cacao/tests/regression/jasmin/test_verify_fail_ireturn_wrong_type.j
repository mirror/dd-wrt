.class public test_verify_fail_ireturn_wrong_type
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static check(Ljava/lang/String;)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	aload_0
	invokevirtual java/io/PrintStream/println(Ljava/lang/String;)V
	return
.end method

.method public static foo()Ljava/lang/String;
	.limit locals 0
	.limit stack 1
	ldc 123
	ireturn
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 2

	invokestatic test_verify_fail_ireturn_wrong_type/foo()Ljava/lang/String;

	invokestatic test_verify_fail_ireturn_wrong_type/check(Ljava/lang/String;)V
	; ERROR: VerifyError

	return
.end method

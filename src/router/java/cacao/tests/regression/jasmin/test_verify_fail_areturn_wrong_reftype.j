.class public test_verify_fail_areturn_wrong_reftype
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static check(Ljava/lang/Object;)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	aload_0
	invokevirtual java/io/PrintStream/println(Ljava/lang/Object;)V
	return
.end method

.method public static foo()Ljava/lang/String;
	.limit locals 0
	.limit stack 2
	new java/lang/Object
	dup
	invokespecial java/lang/Object/<init>()V
	areturn
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 2

	invokestatic test_verify_fail_areturn_wrong_reftype/foo()Ljava/lang/String;

	invokestatic test_verify_fail_areturn_wrong_reftype/check(Ljava/lang/Object;)V
	; ERROR: VerifyError\|LinkageError

	return
.end method

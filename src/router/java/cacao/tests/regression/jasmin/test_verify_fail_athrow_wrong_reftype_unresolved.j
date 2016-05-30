.class public test_verify_fail_athrow_wrong_reftype_unresolved
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
	.limit stack 3

	new java/lang/String
	dup
	ldc "a string"
	invokespecial java/lang/String/<init>(Ljava/lang/String;)V
	athrow
	; ERROR: VerifyError\|LinkageError

	aconst_null
	areturn
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 2

	invokestatic test_verify_fail_athrow_wrong_reftype_unresolved/foo()Ljava/lang/String;

	invokestatic test_verify_fail_athrow_wrong_reftype_unresolved/check(Ljava/lang/Object;)V

	return
.end method

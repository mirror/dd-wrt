.class public test_verify_fail_init_nullpointer
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 3

	ldc 1
	istore 1

	aconst_null
	dup
	invokespecial test_verify_fail_init_nullpointer/<init>()V
	; ERROR: VerifyError

	getstatic java/lang/System/out Ljava/io/PrintStream;
	swap
	invokevirtual java/io/PrintStream/println(Ljava/lang/Object;)V

	return
.end method

.class public test_verify_fail_merge_different_new_objects
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

	aload 0
	ifnull branch1

	new test_verify_fail_merge_different_new_objects
	goto branch2

branch1:
	new test_verify_fail_merge_different_new_objects

branch2:
	dup
	invokespecial test_verify_fail_merge_different_new_objects/<init>()V
	; ERROR: VerifyError

	getstatic java/lang/System/out Ljava/io/PrintStream;
	swap
	invokevirtual java/io/PrintStream/println(Ljava/lang/Object;)V

	return
.end method

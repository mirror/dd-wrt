.class public test_verify_fail_backward_with_new_on_stack
.super java/lang/Object

; The check against backward branches with uninitialized objects on the stack
; is unnecessary. In this case there is a VerifyError because the type merge
; leads to the _|_ (bottom) type in a stack slot, which is forbidden.
;
; Reference:
;     Alessandro Coglio
;     Improving the official specification of Java bytecode verification
;     Proceedings of the 3rd ECOOP Workshop on Formal Techniques for Java Programs
;     June 2001
;     citeseer.ist.psu.edu/article/coglio03improving.html

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

	; aconst_null
	new test_verify_fail_backward_with_new_on_stack

backward:
	pop
	new test_verify_fail_backward_with_new_on_stack

	iload 1
	ifeq backward
	; ERROR: VerifyError

	dup
	invokespecial test_verify_fail_backward_with_new_on_stack/<init>()V

	getstatic java/lang/System/out Ljava/io/PrintStream;
	swap
	invokevirtual java/io/PrintStream/println(Ljava/lang/Object;)V

	return
.end method

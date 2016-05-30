.class public test_verify_unspecced_ok_backward_with_new_in_local
.super java/lang/Object

; The check against backward branches with uninitialized objects is unnecessary
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

.method public toString()Ljava/lang/String;
	.limit stack 1
	.limit locals 1

	ldc "it's me"
	areturn

.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 2
	.limit locals 3

	ldc 1
	istore 1

backward:
	new test_verify_unspecced_ok_backward_with_new_in_local
	astore 2

	iload 1
	ifeq backward
	; this is ok

	aload 2
	dup
	invokespecial test_verify_unspecced_ok_backward_with_new_in_local/<init>()V

	getstatic java/lang/System/out Ljava/io/PrintStream;
	swap
	invokevirtual java/io/PrintStream/println(Ljava/lang/Object;)V
	; OUTPUT: it's me

	return
.end method

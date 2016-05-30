.class public test_verify_unspecced_ok_new_in_local_within_try
.super java/lang/Object

; The check against uninitialized objects in locals within try blocks is
; unnecessary.
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

	.catch java/lang/Throwable from start_try to end_try using handler

	ldc 1
	istore 1

	new test_verify_unspecced_ok_new_in_local_within_try
start_try:
	astore 2
	; this is ok

	aload 2
	dup
	invokespecial test_verify_unspecced_ok_new_in_local_within_try/<init>()V
end_try:

	getstatic java/lang/System/out Ljava/io/PrintStream;
	swap
	invokevirtual java/io/PrintStream/println(Ljava/lang/Object;)V
	; OUTPUT: it's me

	return

handler:
	return
.end method


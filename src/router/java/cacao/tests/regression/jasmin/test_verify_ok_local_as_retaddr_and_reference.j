.class public test_verify_ok_local_as_retaddr_and_reference
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static check(I)V
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
	.limit locals 3

	ldc 35
	istore 1

	; --------------------------------------------------

	; load local 2 with a reference to a string
    ldc "foo"
	astore 2

	; perform some bogus instructions
	aload 0
	ifnull branch_a

	nop
branch_a:

    ; use the string in local 2
	getstatic java/lang/System/out Ljava/io/PrintStream;
	aload_2
	invokevirtual java/io/PrintStream/println(Ljava/lang/String;)V
	; OUTPUT: foo

	; --------------------------------------------------

	aload 0
	ifnull branch_b

	jsr sbr_1
	goto merge_point
	; OUTPUT: 35

branch_b:
	jsr sbr_2
	goto merge_point

merge_point:
    ; here local 2 becomes VOID, as two incompatible returnAddresses
	; are merged

	; --------------------------------------------------

force_basic_block_boundary:

	iload 1
	invokestatic test_verify_ok_local_as_retaddr_and_reference/check(I)V
	; OUTPUT: 35

	return

sbr_1:
	astore 2
	iload 1
	invokestatic test_verify_ok_local_as_retaddr_and_reference/check(I)V
	ret 2

sbr_2:
	astore 2
	ldc 42
	invokestatic test_verify_ok_local_as_retaddr_and_reference/check(I)V
	ret 2

.end method


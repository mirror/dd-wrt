.class public test_load_store_conflict_different_types
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

.method public static checkString(Ljava/lang/String;)V
	.limit locals 1
	.limit stack 10
	getstatic java/lang/System/out Ljava/io/PrintStream;
	aload_0
	invokevirtual java/io/PrintStream/println(Ljava/lang/String;)V
	return
.end method

; ======================================================================

.method public static main([Ljava/lang/String;)V
	.limit stack 3
	.limit locals 3

	ldc 35
	istore 1
    ldc 42
    istore 2

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	iload 1  ; loads 35
	ldc "Sepp"
	astore 1
	istore 2 ; stores 35

	; --------------------------------------------------

	aload 1
	invokestatic test_load_store_conflict_different_types/checkString(Ljava/lang/String;)V
	; OUTPUT: Sepp

	iload 2
	invokestatic test_load_store_conflict_different_types/checkI(I)V
	; OUTPUT: 35

force_basic_block_boundary:

	return
.end method

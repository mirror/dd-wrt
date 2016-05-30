.class public testlcmp
.super java/lang/Object

; ======================================================================

.method public <init>()V
   aload_0
   invokenonvirtual java/lang/Object/<init>()V
   return
.end method

; ======================================================================

.method public static compare(JJ)I
	.limit locals 4
	.limit stack 4
	lload_0
	lload_2
	lcmp
	ireturn
.end method

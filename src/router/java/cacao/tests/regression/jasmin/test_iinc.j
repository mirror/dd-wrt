.class public test_iinc
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
	.limit locals 2

	ldc 35
	istore 1

	aload 0
	ifnull force_basic_block_boundary

	; --------------------------------------------------

	iinc 1 0

	; positive, 8bit

	iinc 1 1
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 36
	iinc 1 2
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 38
	iinc 1 3
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 41
	iinc 1 4
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 45
	iinc 1 7
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 52
	iinc 1 8
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 60
	iinc 1 9
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 69
	iinc 1 15
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 84
	iinc 1 16
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 100
	iinc 1 17
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 117
	iinc 1 31
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 148
	iinc 1 32
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 180
	iinc 1 33
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 213
	iinc 1 63
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 276
	iinc 1 64
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 340
	iinc 1 65
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 405
	iinc 1 127
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 532

	; positive, 16 bit

	iinc 1 128
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 660
	iinc 1 129
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 789
	iinc 1 255
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 1044
	iinc 1 256
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 1300
	iinc 1 257
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 1557
	iinc 1 511
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 2068
	iinc 1 512
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 2580
	iinc 1 513
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 3093
	iinc 1 1023
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT:  4116
	iinc 1 1024
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 5140
	iinc 1 1025
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 6165
	iinc 1 2047
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 8212
	iinc 1 2048
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 10260
	iinc 1 2049
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 12309
	iinc 1 4095
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 16404
	iinc 1 4096
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 20500
	iinc 1 4097
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 24597
	iinc 1 8191
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 32788
	iinc 1 8192
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 40980
	iinc 1 8193
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 49173
	iinc 1 16383
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 65556
	iinc 1 16384
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 81940
	iinc 1 16385
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 98325
	iinc 1 32767
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131092

	; negative, 8bit

	iinc 1 -1
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131091
	iinc 1 -2
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131089
	iinc 1 -3
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131086
	iinc 1 -4
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131082
	iinc 1 -7
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131075
	iinc 1 -8
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131067
	iinc 1 -9
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131058
	iinc 1 -15
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131043
	iinc 1 -16
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131027
	iinc 1 -17
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 131010
	iinc 1 -31
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130979
	iinc 1 -32
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130947
	iinc 1 -33
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130914
	iinc 1 -63
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130851
	iinc 1 -64
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130787
	iinc 1 -65
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130722
	iinc 1 -127
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130595
	iinc 1 -128
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130467

	; negative, 16 bit

	iinc 1 -129
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130338
	iinc 1 -255
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 130083
	iinc 1 -256
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 129827
	iinc 1 -257
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 129570
	iinc 1 -511
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 129059
	iinc 1 -512
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 128547
	iinc 1 -513
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 128034
	iinc 1 -1023
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 127011
	iinc 1 -1024
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 125987
	iinc 1 -1025
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 124962
	iinc 1 -2047
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 122915
	iinc 1 -2048
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 120867
	iinc 1 -2049
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 118818
	iinc 1 -4095
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 114723
	iinc 1 -4096
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 110627
	iinc 1 -4097
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 106530
	iinc 1 -8191
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 98339
	iinc 1 -8192
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 90147
	iinc 1 -8193
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 81954
	iinc 1 -16383
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 65571
	iinc 1 -16384
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 49187
	iinc 1 -16385
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 32802
	iinc 1 -32767
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: 35
	iinc 1 -32768
		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: -32733

	; --------------------------------------------------

force_basic_block_boundary:

		iload 1
		invokestatic test_iinc/check(I)V
		; OUTPUT: -32733

	return
.end method

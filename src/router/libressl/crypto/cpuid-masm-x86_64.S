; 1 "crypto/cpuid-masm-x86_64.S.tmp"
; 1 "<built-in>" 1
; 1 "<built-in>" 3
; 343 "<built-in>" 3
; 1 "<command line>" 1
; 1 "<built-in>" 2
; 1 "crypto/cpuid-masm-x86_64.S.tmp" 2
OPTION	DOTNAME

; 1 "./crypto/x86_arch.h" 1


; 16 "./crypto/x86_arch.h"









; 40 "./crypto/x86_arch.h"













































; 3 "crypto/cpuid-masm-x86_64.S.tmp" 2
EXTERN	OPENSSL_cpuid_setup:NEAR

.CRT$XCU	SEGMENT READONLY ALIGN(8)
		DQ	OPENSSL_cpuid_setup

EXTERN	OPENSSL_ia32cap_P:NEAR


.CRT$XCU	ENDS
.text$	SEGMENT ALIGN(64) 'CODE'

PUBLIC	OPENSSL_ia32_cpuid

ALIGN	16
OPENSSL_ia32_cpuid	PROC PUBLIC
	mov	r8,rbx

	xor	eax,eax
	cpuid
	mov	r11d,eax

	xor	eax,eax
	cmp	ebx,0756e6547h
	setne	al
	mov	r9d,eax
	cmp	edx,049656e69h
	setne	al
	or	r9d,eax
	cmp	ecx,06c65746eh
	setne	al
	or	r9d,eax
	jz	$L$intel

	cmp	ebx,068747541h
	setne	al
	mov	r10d,eax
	cmp	edx,069746E65h
	setne	al
	or	r10d,eax
	cmp	ecx,0444D4163h
	setne	al
	or	r10d,eax
	jnz	$L$intel


	mov	eax,080000000h
	cpuid
	cmp	eax,080000001h
	jb	$L$intel
	mov	r10d,eax
	mov	eax,080000001h
	cpuid
	or	r9d,ecx
	and	r9d,(1 SHL 11)
	or	r9d,1

	cmp	r10d,080000008h
	jb	$L$intel

	mov	eax,080000008h
	cpuid
	movzx	r10,cl
	inc	r10

	mov	eax,1
	cpuid
	bt	edx,28
	jnc	$L$generic
	shr	ebx,16
	cmp	bl,r10b
	ja	$L$generic
	xor	edx,(1 SHL 28)
	jmp	$L$generic

$L$intel::
	cmp	r11d,4
	mov	r10d,-1
	jb	$L$nocacheinfo

	mov	eax,4
	mov	ecx,0
	cpuid
	mov	r10d,eax
	shr	r10d,14
	and	r10d,0fffh

$L$nocacheinfo::
	mov	eax,1
	cpuid

	and	edx,(NOT((1 SHL 20) OR (1 SHL 30)))
	cmp	r9d,0
	jne	$L$notintel

	or	edx,(1 SHL 30)
	and	ah,15
	cmp	ah,15
	jne	$L$notintel

	or	edx,(1 SHL 20)
$L$notintel::
	bt	edx,28
	jnc	$L$generic
	xor	edx,(1 SHL 28)
	cmp	r10d,0
	je	$L$generic

	or	edx,(1 SHL 28)
	shr	ebx,16
	cmp	bl,1
	ja	$L$generic
	xor	edx,(1 SHL 28)

$L$generic::
	and	r9d,(1 SHL 11)
	and	ecx,(NOT(1 SHL 11))
	or	r9d,ecx

	mov	r10d,edx
	bt	r9d,27
	jnc	$L$clear_avx
	xor	ecx,ecx
DB	00fh,001h,0d0h		
	and	eax,6
	cmp	eax,6
	je	$L$done
$L$clear_avx::
	mov	eax,(NOT((1 SHL 28) OR (1 SHL 12) OR (1 SHL 11)))
	and	r9d,eax
$L$done::
	shl	r9,32
	mov	eax,r10d
	mov	rbx,r8
	or	rax,r9
	DB	0F3h,0C3h		;repret
OPENSSL_ia32_cpuid	ENDP

.text$	ENDS
END


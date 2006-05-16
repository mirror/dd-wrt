; /*** DO NOT EDIT - GENERATED AUTOMATICALLY ***/
; /*** Copyright (C) 1996-2002 Markus F.X.J. Oberhumer ***/

.386p
.model flat

.code
public _lzo1c_decompress_asm_safe

_lzo1c_decompress_asm_safe:
                push    ebp
                push    edi
                push    esi
                push    ebx
                push    ecx
                push    edx
                sub     esp,0000000cH
                cld
                mov     esi,+28H[esp]
                mov     edi,+30H[esp]
                mov     ebp,00000003H
                lea     eax,-3H[esi]
                add     eax,+2cH[esp]
                mov     +4H[esp],eax
                mov     eax,edi
                mov     edx,+34H[esp]
                add     eax,[edx]
                mov     [esp],eax
                lea     esi,+0H[esi]
L3:             xor     eax,eax
                mov     al,[esi]
                inc     esi
                cmp     al,20H
                jae     L6
                or      al,al
                je      L7
                mov     ecx,eax
L4:             lea     ebx,[edi+ecx]
                cmp     [esp],ebx
                jb      L18
                lea     ebx,[esi+ecx]
                cmp     +4H[esp],ebx
                jb      L17
                repe    movsb
L5:             mov     al,[esi]
                inc     esi
                cmp     al,20H
                jb      L9
L6:             cmp     al,40H
                jb      L10
                mov     ecx,eax
                and     al,1fH
                lea     edx,-1H[edi]
                shr     ecx,05H
                sub     edx,eax
                mov     al,[esi]
                inc     esi
                shl     eax,05H
                sub     edx,eax
                inc     ecx
                xchg    esi,edx
                cmp     esi,+30H[esp]
                jb      L19
                lea     ebx,[edi+ecx]
                cmp     [esp],ebx
                jb      L18
                repe    movsb
                mov     esi,edx
                jmp     L3
                lea     esi,+0H[esi]
L7:             mov     al,[esi]
                inc     esi
                lea     ecx,+20H[eax]
                cmp     al,0f8H
                jb      L4
                mov     ecx,00000118H
                sub     al,0f8H
                je      L8
                xchg    eax,ecx
                xor     al,al
                shl     eax,cl
                xchg    eax,ecx
L8:             lea     ebx,[edi+ecx]
                cmp     [esp],ebx
                jb      L18
                lea     ebx,[esi+ecx]
                cmp     +4H[esp],ebx
                jb      L17
                repe    movsb
                jmp     L3
                lea     esi,+0H[esi]
L9:             lea     edx,-1H[edi]
                sub     edx,eax
                mov     al,[esi]
                inc     esi
                shl     eax,05H
                sub     edx,eax
                xchg    esi,edx
                cmp     esi,+30H[esp]
                jb      L19
                lea     ebx,+4H[edi]
                cmp     [esp],ebx
                jb      L18
                movsb
                movsb
                movsb
                mov     esi,edx
                movsb
                xor     eax,eax
                jmp     L5
L10:            and     al,1fH
                mov     ecx,eax
                jne     L13
                mov     cl,1fH
L11:            mov     al,[esi]
                inc     esi
                or      al,al
                jne     L12
                add     ecx,000000ffH
                jmp     L11
                lea     esi,+0H[esi]
L12:            add     ecx,eax
L13:            mov     al,[esi]
                inc     esi
                mov     ebx,eax
                and     al,3fH
                mov     edx,edi
                sub     edx,eax
                mov     al,[esi]
                inc     esi
                shl     eax,06H
                sub     edx,eax
                cmp     edx,edi
                je      L14
                xchg    edx,esi
                lea     ecx,+3H[ecx]
                cmp     esi,+30H[esp]
                jb      L19
                lea     eax,[edi+ecx]
                cmp     [esp],eax
                jb      L18
                repe    movsb
                mov     esi,edx
                xor     eax,eax
                shr     ebx,06H
                mov     ecx,ebx
                jne     L4
                jmp     L3
L14:            cmp     ecx,00000001H
                setne   al
                cmp     edi,[esp]
                ja      L18
                mov     edx,+28H[esp]
                add     edx,+2cH[esp]
                cmp     esi,edx
                ja      L17
                jb      L16
L15:            sub     edi,+30H[esp]
                mov     edx,+34H[esp]
                mov     [edx],edi
                neg     eax
                add     esp,0000000cH
                pop     edx
                pop     ecx
                pop     ebx
                pop     esi
                pop     edi
                pop     ebp
                ret
                mov     eax,00000001H
                jmp     L15
L16:            mov     eax,00000008H
                jmp     L15
L17:            mov     eax,00000004H
                jmp     L15
L18:            mov     eax,00000005H
                jmp     L15
L19:            mov     eax,00000006H
                jmp     L15

end

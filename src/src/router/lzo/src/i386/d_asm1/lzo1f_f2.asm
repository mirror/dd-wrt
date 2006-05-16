; /*** DO NOT EDIT - GENERATED AUTOMATICALLY ***/
; /*** Copyright (C) 1996-2002 Markus F.X.J. Oberhumer ***/

.386p
.model flat

.code
public _lzo1f_decompress_asm_fast_safe

_lzo1f_decompress_asm_fast_safe:
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
                cmp     al,1fH
                ja      L9
                or      al,al
                mov     ecx,eax
                jne     L6
L4:             mov     al,[esi]
                inc     esi
                or      al,al
                jne     L5
                add     ecx,000000ffH
                jmp     L4
L5:             lea     ecx,+1fH[eax+ecx]
L6:             lea     ebx,[edi+ecx]
                cmp     [esp],ebx
                jb      L21
                lea     ebx,[esi+ecx]
                cmp     +4H[esp],ebx
                jb      L20
                mov     al,cl
                shr     ecx,02H
                repe    movsd
                and     al,03H
                je      L7
                mov     ebx,[esi]
                add     esi,eax
                mov     [edi],ebx
                add     edi,eax
L7:             mov     al,[esi]
                inc     esi
L8:             cmp     al,1fH
                jbe     L13
L9:             cmp     al,0dfH
                ja      L16
                mov     ecx,eax
                shr     eax,02H
                lea     edx,-1H[edi]
                and     al,07H
                shr     ecx,05H
                mov     ebx,eax
                mov     al,[esi]
                lea     eax,[ebx+eax*8]
                inc     esi
L10:            sub     edx,eax
                add     ecx,00000002H
                xchg    edx,esi
                cmp     esi,+30H[esp]
                jb      L22
                lea     ebx,[edi+ecx]
                cmp     [esp],ebx
                jb      L21
                cmp     ecx,00000006H
                jb      L11
                cmp     eax,00000004H
                jb      L11
                mov     al,cl
                shr     ecx,02H
                repe    movsd
                and     al,03H
                mov     cl,al
L11:            repe    movsb
                mov     esi,edx
L12:            mov     cl,-2H[esi]
                and     ecx,00000003H
                je      L3
                mov     eax,[esi]
                add     esi,ecx
                mov     [edi],eax
                add     edi,ecx
                xor     eax,eax
                mov     al,[esi]
                inc     esi
                jmp     L8
L13:            lea     edx,+3H[edi]
                cmp     [esp],edx
                jb      L21
                shr     eax,02H
                lea     edx,-801H[edi]
                mov     ecx,eax
                mov     al,[esi]
                inc     esi
                lea     eax,[ecx+eax*8]
                sub     edx,eax
                cmp     edx,+30H[esp]
                jb      L22
                mov     eax,[edx]
                mov     [edi],eax
                add     edi,00000003H
                jmp     L12
L14:            mov     al,[esi]
                inc     esi
                or      al,al
                jne     L15
                add     ecx,000000ffH
                jmp     L14
L15:            lea     ecx,+1fH[eax+ecx]
                jmp     L17
                lea     esi,+0H[esi]
L16:            and     al,1fH
                mov     ecx,eax
                je      L14
L17:            mov     edx,edi
                mov     ax,[esi]
                add     esi,00000002H
                shr     eax,02H
                jne     L10
                cmp     ecx,00000001H
                setne   al
                cmp     edi,[esp]
                ja      L21
                mov     edx,+28H[esp]
                add     edx,+2cH[esp]
                cmp     esi,edx
                ja      L20
                jb      L19
L18:            sub     edi,+30H[esp]
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
                jmp     L18
L19:            mov     eax,00000008H
                jmp     L18
L20:            mov     eax,00000004H
                jmp     L18
L21:            mov     eax,00000005H
                jmp     L18
L22:            mov     eax,00000006H
                jmp     L18
                lea     esi,+0H[esi]

end

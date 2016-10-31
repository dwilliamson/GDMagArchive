;----------------------------------------------------------------------------
;
; Subdividing affine inner loop.
;
;  Things we could do to make this code faster (but I ran out of time):
;   - fix all the @todo notes
;   - use the float->int trick to remove the scales/fistps
;   - do the texture delta multiply in the FPU
;   - peephole optimize the whole function
;   - pipeline things better
;   - overlap integer and floating point operations better
;
; This material is Copyright 1995, 1996 Chris Hecker, All Rights Reserved.
; It's for you to read and learn from, not to put in your own articles
; or books or on your website, etc.  Thank you.
;
; Chris Hecker
; checker@netcom.com
;
;----------------------------------------------------------------------------


        .386

        option language:c

        assume ds:FLAT,es:FLAT,ss:FLAT
        assume fs:nothing,gs:nothing

        OPTION OLDSTRUCTS               ; old masm structure access

        .xlist

        include stdcall.inc             ;calling convention cmacros

        .list

dib_info struc
    pBits dd ?
    DibWidth dd ?
    DibHeight dd ?
    DeltaScan dd ?
dib_info ends

gradients struc
    aOneOverZ dd 3 dup (?)
    aUOverZ dd 3 dup (?)
    aVOverZ dd 3 dup (?)
    dOneOverZdX dd ?
    dOneOverZdX8 dd ?
    dOneOverZdY dd ?
    dUOverZdX dd ?
    dUOverZdX8 dd ?
    dUOverZdY dd ?
    dVOverZdX dd ?
    dVOverZdX8 dd ?
    dVOverZdY dd ?
    dUdXModifier dd ?
    dVdXModifier dd ?
gradients ends

edge struc
    X dd ?
    XStep dd ?
    Numerator dd ?
    Denominator dd ?
    ErrorTerm dd ?
    Y dd ?
    Height dd ?
    OneOverZ dd ?
    OneOverZStep dd ?
    OneOverZStepExtra dd ?
    UOverZ dd ?
    UOverZStep dd ?
    UOverZStepExtra dd ?
    VOverZ dd ?
    VOverZStep dd ?
    VOverZStepExtra dd ?
edge ends   


_DATA   SEGMENT DWORD USE32 PUBLIC 'DATA'


FixedScale dd 65536.0
FixedScale8 dd 8192.0 ; 2^16 / 8
One dd 1.0
FloatTemp dd ?

; @todo should align this in the same cache line?
UVintVfracStepVCarry dd ?
UVintVfracStepVNoCarry dd ?
UVintVfracStep equ UVintVfracStepVNoCarry

DeltaVFrac dd ?

_DATA   ends

_TEXT$01   SEGMENT PARA USE32 PUBLIC 'CODE'
           ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;----------------------------------------------------------------------------
;
; extern "C" void DrawScanLine_suba_asm( dib_info const *pDest,
;                   gradients_fx_fl_asm const *pGradients,
;                   edge_fx_fl_asm *pLeft, edge_fx_fl_asm *pRight,
;                   dib_info const *pTexture );
;
; definition for asm function...I really wish there was a nice way to
; integrate asm and C++, but most compiler vendors are too worried about
; the latest GUI IDE to bother with something simple that real programmers
; could use
;
; as a result of this situation, we get to do the type checking and management
; of parameter passing ourselves
;
;----------------------------------------------------------------------------

cProc DrawScanLine_suba_asm,20,<\
    pDest : dword,\
    pGradients : dword,\
    pLeft : dword,\
    pRight : dword,\
    pTexture : dword>

;----------locals----------

    local Subdivisions : dword
    local WidthModLength : dword

    local DeltaU : dword
    local DeltaV : dword
    local DeltaUFrac : dword
    local TextureDeltaScan : dword
    local pTextureBits : dword

    local UFixed : dword
    local VFixed : dword

    local LocaldUdXModifier : dword
    local LocaldVdXModifier : dword

    local FPUCW : word
    local OldFPUCW : word

;--------------------------

    push    edi                         ; save Win32 stdcall registers
    push    esi
    push    ebx

    ; put the FPU in 32 bit mode
    ; @todo move this out of here!
    fstcw   [OldFPUCW]                  ; store copy of CW
    mov     ax,OldFPUCW                 ; get it in ax
    and     eax,NOT 1100000000y         ; 24 bit precision
    mov     [FPUCW],ax                  ; store it
    fldcw   [FPUCW]                     ; load the FPU

    mov     esi,pDest                   ; get dest dib
    mov     ebx,pLeft                   ; ebx = left edge pointer
    mov     eax,[ebx].X                 ; eax = start x
    mov     ecx,pRight                  ; get right edge
    mov     ecx,[ecx].X                 ; ecx = end x
    sub     ecx,eax                     ; ecx = width
    jle     Return                      ; no pixels to draw, get out

    mov     edi,[esi].pBits             ; edi = dest pointer
    mov     edx,[ebx].Y                 ; get this scanline's Y
    imul    edx,[esi].DeltaScan         ; get the scanline offset in dib
    add     edi,edx                     ; calc scaline offset pointer
    add     edi,eax                     ; add in x start offset

    ; edi = pointer to start pixel in dest dib
    ; ecx = spanwidth

    mov     eax,ecx                     ; eax and ecx = width
    shr     ecx,3                       ; ecx = width / subdivision length
    and     eax,7                       ; eax = width mod subdivision length
    jnz     @f                          ; any leftover?
    dec     ecx                         ; no, so special case last span
    mov     eax,8                       ; it's 8 pixels long
@@:
    mov     [Subdivisions],ecx          ; store widths
    mov     [WidthModLength],eax

    mov     ebx,pTexture                ; get texture pointer
    mov     esi,[ebx].pBits             ; get texture bits pointer
    mov     [pTextureBits],esi          ; store local copy
    mov     ebx,[ebx].DeltaScan         ; get scanline step
    mov     [TextureDeltaScan],ebx      ; store local copy
    
    mov     ebx,pLeft                   ; get left edge pointer
    mov     edx,pGradients              ; get gradients pointer

    ; calculate ULeft and VLeft         ; FPU Stack (ZL = ZLeft)
                                        ; st0  st1  st2  st3  st4  st5  st6  st7
    fld     [ebx].VOverZ                ; V/ZL 
    fld     [ebx].UOverZ                ; U/ZL V/ZL 
    fld     [ebx].OneOverZ              ; 1/ZL U/ZL V/ZL 
    fld1                                ; 1    1/ZL U/ZL V/ZL 
    fdiv    st,st(1)                    ; ZL   1/ZL U/ZL V/ZL 
    fld     st                          ; ZL   ZL   1/ZL U/ZL V/ZL 
    fmul    st,st(4)                    ; VL   ZL   1/ZL U/ZL V/ZL 
    fxch    st(1)                       ; ZL   VL   1/ZL U/ZL V/ZL 
    fmul    st,st(3)                    ; UL   VL   1/ZL U/ZL V/ZL 

    fstp    st(5)                       ; VL   1/ZL U/ZL V/ZL UL
    fstp    st(5)                       ; 1/ZL U/ZL V/ZL UL   VL

    ; calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

    fadd    [edx].dOneOverZdX8          ; 1/ZR U/ZL V/ZL UL   VL
    fxch    st(1)                       ; U/ZL 1/ZR V/ZL UL   VL
    fadd    [edx].dUOverZdX8            ; U/ZR 1/ZR V/ZL UL   VL
    fxch    st(2)                       ; V/ZL 1/ZR U/ZR UL   VL
    fadd    [edx].dVOverZdX8            ; V/ZR 1/ZR U/ZR UL   VL
    
    ; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

    fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
    ; @todo overlap this guy
    fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL
    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    test    ecx,ecx                     ; check for any full spans
    jz      HandleLeftoverPixels
    
SpanLoop:

    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    ; convert left side coords

    fld     st(5)                       ; UL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    [FixedScale]                ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fistp   [UFixed]                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    [FixedScale]                ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
    fistp   [VFixed]                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
    fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
    fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
    fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

    fmul    [FixedScale8]               ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
    fistp   [DeltaV]                    ; UR   V/ZR 1/ZR U/ZR dU   VR

    fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
    fmul    [FixedScale8]               ; dU8  V/ZR 1/ZR U/ZR UR   VR
    fistp   [DeltaU]                    ; V/ZR 1/ZR U/ZR UR   VR

    ; increment terms for next span     ; st0  st1  st2  st3  st4  st5  st6  st7
    ; Right terms become Left terms---->; V/ZL 1/ZL U/ZL UL   VL

    mov     edx,pGradients
    fadd    [edx].dVOverZdX8            ; V/ZR 1/ZL U/ZL UL   VL
    fxch    st(1)                       ; 1/ZL V/ZR U/ZL UL   VL
    fadd    [edx].dOneOverZdX8          ; 1/ZR V/ZR U/ZL UL   VL
    fxch    st(2)                       ; U/ZL V/ZR 1/ZR UL   VL
    fadd    [edx].dUOverZdX8            ; U/ZR V/ZR 1/ZR UL   VL
    fxch    st(2)                       ; 1/ZR V/ZR U/ZR UL   VL
    fxch    st(1)                       ; V/ZR 1/ZR U/ZR UL   VL

    ; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

    fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
    fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL

    ; set up affine registers

    ; setup delta values
    
    mov     eax,[DeltaV]                ; get v 16.16 step
    mov     ebx,eax                     ; copy it
    sar     eax,16                      ; get v int step
    shl     ebx,16                      ; get v frac step
    mov     [DeltaVFrac],ebx            ; store it
    imul    eax,[TextureDeltaScan]      ; calculate texture step for v int step

    mov     ebx,[DeltaU]                ; get u 16.16 step
    mov     ecx,ebx                     ; copy it
    sar     ebx,16                      ; get u int step
    shl     ecx,16                      ; get u frac step
    mov     [DeltaUFrac],ecx            ; store it
    add     eax,ebx                     ; calculate uint + vint step
    mov     [UVintVfracStepVNoCarry],eax; save whole step in non-v-carry slot
    add     eax,[TextureDeltaScan]      ; calculate whole step + v carry
    mov     [UVintVfracStepVCarry],eax  ; save in v-carry slot

    ; setup initial coordinates
    
    mov     esi,[UFixed]                ; get u 16.16 fixedpoint coordinate
    add     esi,[edx].dUdXModifier      ; fixup rounding rule
    mov     ebx,esi                     ; copy it
    sar     esi,16                      ; get integer part
    shl     ebx,16                      ; get fractional part
    
    mov     ecx,[VFixed]                ; get v 16.16 fixedpoint coordinate
    add     ecx,[edx].dVdXModifier      ; fixup rounding rule
    mov     edx,ecx                     ; copy it
    sar     edx,16                      ; get integer part
    shl     ecx,16                      ; get fractional part
    imul    edx,[TextureDeltaScan]      ; calc texture scanline address
    add     esi,edx                     ; calc texture offset
    add     esi,[pTextureBits]          ; calc address

    mov     edx,[DeltaUFrac]            ; get register copy
    push    ebp                         ; free up another register

    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************

    ; 8 pixel span code
    ; edi = dest dib bits at current pixel
    ; esi = texture pointer at current u,v
    ; ebx = u fraction 0.32
    ; ecx = v fraction 0.32
    ; edx = u frac step
    ; ebp = v carry scratch

    mov     al,[edi]                    ; preread the destination cache line

    mov     al,[esi]                    ; get texture pixel 0

    add     ecx,[DeltaVFrac]            ; increment v fraction
    sbb     ebp,ebp                     ; get -1 if carry
    add     ebx,edx                     ; increment u fraction

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+0],al                  ; store pixel 0

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 1

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+1],al                  ; store pixel 1

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 2

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+2],al                  ; store pixel 2

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 3

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+3],al                  ; store pixel 3

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 4

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+4],al                  ; store pixel 4

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 5

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+5],al                  ; store pixel 5

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 6

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries
    add     ecx,[DeltaVFrac]            ; increment v fraction

    sbb     ebp,ebp                     ; get -1 if carry
    mov     [edi+6],al                  ; store pixel 6

    add     ebx,edx                     ; increment u fraction
    mov     al,[esi]                    ; get texture pixel 7

    adc     esi,[4*ebp+UVintVfracStep]  ; add in step ints & carries

    mov     [edi+7],al                  ; store pixel 7

    pop     ebp                         ; restore access to stack frame
    
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************


    ; the fdiv is done, finish right    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    add     edi,8                       ; increment to next span
    dec     [Subdivisions]              ; decrement span count
    jnz     SpanLoop                    ; loop back

HandleLeftoverPixels:

    mov     esi,[pTextureBits]          ; load texture pointer

    ; edi = dest dib bits
    ; esi = texture dib bits
    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    ; inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     [WidthModLength],0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    mov     ebx,pRight                  ; get right edge pointer
    mov     edx,pGradients              ; get gradients pointer

    ; convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    [FixedScale]                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   [UFixed]                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    [FixedScale]                ; VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   [VFixed]                    ; inv. inv. inv. inv. inv. UL   VL

    dec     [WidthModLength]            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    ; calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    ; r -> R+1

    ; @todo rearrange things so we don't need these two instructions
    fstp    [FloatTemp]                 ; inv. inv. inv. inv. UL   VL
    fstp    [FloatTemp]                 ; inv. inv. inv. UL   VL

    fld     [ebx].VOverZ                ; V/Zr inv. inv. inv. UL   VL
    fsub    [edx].dVOverZdX             ; V/ZR inv. inv. inv. UL   VL
    fld     [ebx].UOverZ                ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    [edx].dUOverZdX             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     [ebx].OneOverZ              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    [edx].dOneOverZdX           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   [One]                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   [WidthModLength]            ; dv   UR   inv. inv. inv. dU   VR
    fmul    [FixedScale]                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   [DeltaV]                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   [WidthModLength]            ; du   inv. inv. inv. UR   VR
    fmul    [FixedScale]                ; du16 inv. inv. inv. UR   VR
    fistp   [DeltaU]                    ; inv. inv. inv. UR   VR

    ; @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR

OnePixelSpan:

    mov     ebx,[UFixed]                ; get starting coordinates
    mov     ecx,[VFixed]                ; for span
    add     ebx,[edx].dUdXModifier      ; fixup rounding rule
    add     ecx,[edx].dVdXModifier

    ; leftover pixels loop
    ; edi = dest dib bits
    ; esi = texture dib bits

    ; ebx = u 16.16
    ; ecx = v 16.16

LeftoverLoop:
    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,[TextureDeltaScan]      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,DeltaU                  ; increment u coordinate
    add     ecx,DeltaV                  ; increment v coordinate

    dec     [WidthModLength]            ; decrement loop count
    jge     LeftoverLoop                ; finish up

FPUReturn:

    ; busy FPU registers:               ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; xxx  xxx  xxx  xxx  xxx  xxx  xxx
    ffree   st(0)
    ffree   st(1)
    ffree   st(2)
    ffree   st(3)
    ffree   st(4)
    ffree   st(5)
    ffree   st(6)

Return:

    fldcw   [OldFPUCW]                  ; restore the FPU

    pop     ebx
    pop     esi
    pop     edi
    cRet    DrawScanLine_suba_asm

endProc DrawScanLine_suba_asm

_TEXT$01   ends

end


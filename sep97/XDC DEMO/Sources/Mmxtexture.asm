;/**************************************************************************
;
;Mixed Rendering
;
; **************************************************************************/
;/***************************************************************
;*
;*       This program has been developed by Intel Corporation.  
;*		You have Intel's permission to incorporate this code 
;*       into your product, royalty free.  Intel has various 
;*	    intellectual property rights which it may assert under
;*       certain circumstances, such as if another manufacturer's
;*       processor mis-identifies itself as being "GenuineIntel"
;*		when the CPUID instruction is executed.
;*
;*       Intel specifically disclaims all warranties, express or
;*       implied, and all liability, including consequential and
;*		other indirect damages, for the use of this code, 
;*		including liability for infringement of any proprietary
;*		rights, and including the warranties of merchantability
;*		and fitness for a particular purpose.  Intel does not 
;*		assume any responsibility for any errors which may 
;*		appear in this code nor any responsibility to update it.
;*
;*  * Other brands and names are the property of their respective
;*    owners.
;*
;*  Copyright (c) 1995, Intel Corporation.  All rights reserved.
;***************************************************************/
TITLE marble and wood textures using MMX(tm) technology

;prevent listing of iammx.inc file
.nolist
INCLUDE iammx.inc
.list
.586 
.model FLAT

;***********************************************************************
;     Data Segment Declarations
;***********************************************************************
;.DATA
DSEG SEGMENT PARA


extrn _sinTable : ptr sword
extrn _woodTable : ptr dword
extrn _sqrtTable : ptr dword
extrn _turbulenceTbl : ptr dword


;Variables, u, v, du, dv  each contain parameters for two
;texels.  Since u, v, ...  are 64 bit, then each texel parameter is
;32 bit.  (32 bit per texel * two texels = 64 bits).  This enables us
;to work with two pixels at one time using MMX technology.
ALIGN 8
du		    QWORD ?
dv		   QWORD ?
result	dd 0


;Various masks.  Set up to filter out unwanted bits in MMX registers.
ALIGN 8
mask_quad_green	           QWORD 0800080008000800h
mask_quad_10                 QWORD 000a000a000a000ah
mask_quad_15                 QWORD 000f000f000f000fh
mask_quad_735               QWORD 002df02df02df02dfh
mask_quad_1500             QWORD 05dc05dc05dc05dch
mask_FFFF_Minus_High  QWORD 0e000e000e000e000h
mask_FFFF_Minus_High_Wood QWORD 0e890e890e890e890h
mask_all_1					      QWORD 0ffffffffffffffffh
mask_clear_byte_1           QWORD 0000000000000ffffh
mask_high_words             QWORD 00000ffff0000ffffh
mask_low_words     QWORD 0ffff0000ffff0000h

DSEG ENDS

;***********************************************************************
;     Constant Segment Declarations
;***********************************************************************
.const

;***********************************************************************
;     Code Segment Declarations
;***********************************************************************
.code



; MMX_Marb uses the contents of turbulence_buffer which was filled 
; before by MMX_Octave with num_octaves of noise.
; Our marble approx is marb(x) = sin(x + turb(x)), we use a pre-computed
; sine table to accelerate it and to able to use MMX tech.
; In each iteration 4 pixles are calculated, 'num_pixels' is a mutiply of 4.


MMX_Marb PROC NEAR C USES eax ecx ebx edi ,
		u_init:DWORD, du_init:DWORD, num_pixels:DWORD

MOV	               ECX  ,   num_pixels              ; number of pixels in scanline  
LEA	                EDI   ,  _turbulenceTbl         ; allready calculated turbulence
MOVD             MM2 ,  du_init                      ; mm2 = 0:du

MOVD             MM0 ,  u_init                        ; mm0 = 0:u
PSLLQ            MM2 ,  32                             ; mm2 = du:0

SHR	               ECX, 2									; ECX= # of times to draw 4 pixels at once
PUNPCKLDQ   MM0, MM0						   ; p1 = u          , p0 = u

PADDD	         MM0, MM2							; p1 = u + du  , p0 = u
PUNPCKHDQ  MM2, MM2							  ; du               , du

MOVQ			 MM1, MM0							; p3 = u +du  ,  p2 = u  
PSLLD            MM2 , 1								; 2du             , 2du       

PADDD	         MM1 , MM2						   ; p3 = u + 3du, p2 = u + 2du
PSLLD             MM2 , 1								; 4du ,4du       

MOVQ              MM7,  DWORD PTR mask_high_words ; mm7 = 0000:ffff:0000:ffff


marb_loop:

MOVQ       MM5, [EDI]      ; mm5 = turb3:turb2:turb1:turb0
MOVQ       MM3, MM0      ; mm3 = u1:u0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;; we have to do packUsdw but there is no such an instruction
;;;;;;;;; so we do :  Shift Left by 16 , and then Shift Right Arithmetic by 16.
;;;;;;;;; The 16 bits shift left is done by 2 bits shift left instead of  14 bits shift right.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MOVQ      MM4, MM1      ; mm4 = u3:u2
PSLLD	  MM3, 2 	        ; shift left by 2 (16 -14)  

 PMULLW	 MM5 , DWORD PTR mask_quad_10       ; turb = 10 * turb
 PSLLD	    MM4, 2 	        ;  shift left by 2 (16 -14)  

;PSRLD	   MM4, 14 	       ; Convert from 10.22 to 10.8  ;no need done by pslld mm4 , 2
;PSRLD	   MM3, 14 	       ; Convert from 10.22 to 10.8  ;no need done by pslld mm3 , 2

 PADDD	MM0, MM2        ; inc u1:u0 for next iteration
 PSRAD  MM3,16             ; extend sign bit for PACKSSDW

 PSRAD  MM4,16             ; extend sign bit for PACKSSDW
 ADD       EDI,8                 ; inc edi for next iteration

PSUBW        MM5 , DWORD PTR mask_quad_1500    ;  turb = turb - 1500
PACKSSDW  MM3, MM4      ;mm3 = (u3:u2:u1:u0) >> 14 and "packUsdw"

PADDD     MM1, MM2     ; inc u3:u2 for next iteration
PADDW    MM3 , MM5    ; marble indexes are:  (u_init >> 14) + (10 * turb) - 1500

MOVQ     MM6,MM3	    ; mm6 = indx3:indx2:indx1:indx0        ;;1	
PAND      MM3,MM7        ; mm3 = 0:indx2:0:indx0 

MOVD      EAX,MM3        ; eax = 0:indx0							       ;;2,3
PSRLD    MM6,16           ; mm6=0:indx3:0:indx1

MOVD     EBX,MM6         ; ebx=0:indx1									  ;;4,5
PSRLQ    MM3,32           ;  mm3=0:0:0:indx2 

MOVD     MM4  ,  [ _sinTable + eax*2]   ;pixel0			    ;; 6

MOVD               EAX,MM3       ; eax = indx2								;;7
PSRLQ			  MM6,32          ; mm6=0:0:0:indx3

PUNPCKLWD   MM4  ,  [ _sinTable  +   ebx*2]   ; pixel1	          ;; 8
MOVD               EBX   , MM6							     ; ebx=0:indx1	;; 9
MOVD               MM5  , [ _sinTable  + eax*2]      ; pixel2              ;; 10
PUNPCKLWD   MM5  , [ _sinTable  + ebx*2]      ; pixel3              ;; 11

PUNPCKLDQ    MM4  , MM5			 ; mm4 = p3:p2:p1:p0
MOVQ	           [EDI-8] , MM4	    ; store the 4 pixels to turb_buffer

DEC     ECX
JNZ  marb_loop

EMMS                       ; Clear out the MMX registers and set approp flags.

RET                        ; end of function
MMX_Marb ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;MOVD      EAX,MM3   ; eax = indx1:indx0												; 1,2
;MOVQ      MM6,MM3  ; now we read the pixles' color from the sine table       
;AND         EAX, 0ffffh																				; 3
;PSRLD	  MM3, 16
;MOVD      MM4  , DWORD PTR [esi + eax*2] ;pixel0							   ; 4,5
;PSRLQ     MM6, 32
;MOVD      EBX,MM6  ; ebx = indx3:indx2												  ;  6
;PSRLD	  MM6, 16
;MOVD              EAX,MM3   ; eax = indx1												    ; 7
;AND                 EBX,0ffffh   ; ebx = indx2													;  8
;PUNPCKLWD    MM4 , DWORD PTR [esi + eax*2]   ;pixel1					;  9
;MOVD              MM5  , DWORD PTR [esi  + ebx*2]   ;pixel2				   ; 10
;MOVD              EBX,MM6									  ;ebx = indx3				   ; 11
;PUNPCKLWD   MM5 , DWORD PTR [esi][ebx*2]    ;pixel3					 ; 12

;MOVD				EAX,MM3   ; eax = indx1:indx0						;1,2
;PSRLQ				MM3,32     ; now we read the pixles' color from the sine table       
;MOV				  EDX , EAX														   ;3
;AND				   EAX , 0ffffh  
;MOVD				 EBX,mm3  ; ebx = indx3:indx2						 ;4
;MOVD				 MM4  , DWORD PTR [esi + eax*2] ;pixel0      ;5
;MOV				   EAX , EBX														;6
;AND                   EBX , 0ffffh ;ebx =indx2
;SHR                   EDX   , 16 ; edx = indx1                                   ; 7
;SHR                   EAX   , 16 ; eax = indx3									  ; 8
;MOVD                MM5  , DWORD PTR [esi  + ebx*2]   ;pixel2   ; 9
;PUNPCKLWD    MM4  , DWORD PTR [esi   + edx*2]   ;pixel1  ; 10 
;PUNPCKLWD    MM5  , DWORD PTR [esi   + eax*2]   ;pixel3  ; 11 

; PSLLD   MM3,16          ; no need done by : pslld mm3 , 2
; PSLLD   MM4,16          ; no need done by : pslld mm4 , 2
; The following adds a bias of 1 to the color so that real BLACK is
; never actually written to the sphere (thus it works nice for a chroma BLT
; paddusw			MM4, DWORD PTR mask_quad_green  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

MMX_Wood PROC NEAR C USES edi  ecx eax edx,
		u_init:DWORD, v_init:DWORD,
		du_init:DWORD ,dv_init:DWORD,
		num_pixels:DWORD

MOV	              ECX ,   num_pixels 
LEA	                EDI  , _turbulenceTbl

MOVD             MM2 , du_init  ;  0:du 
SHR	               ECX , 1	          ; ECX= # of times to draw 2 pixels at once

MOVD             MM0, u_init    ;  0:u
PSLLQ            MM2, 32         ;  du:0

PUNPCKLDQ   MM0, MM0    ;  u:u

MOVD              MM3, dv_init  ;  0:v
PADDD	         MM0, MM2	  ; u + du:u

MOVD			  MM1, v_init    ;  0:v  
PUNPCKHDQ   MM2, MM2     ;  du:du

PUNPCKLDQ   MM1, MM1	;  v:v

PSLLQ            MM3, 32         ;  dv:0 

PADDD	         MM1 , MM3	  ;  v + dv:v 
PUNPCKHDQ   MM3, MM3	; dv:dv

PSLLD             MM2 , 1          ; 2du:2du       

PSLLD             MM3 , 1          ; 2dv:2dv       

wood_loop:

MOVQ       MM5, [EDI]       ;   turbulence
MOVQ       MM4, MM0       ;   u1 : u0

MOVQ       MM6, MM1       ;   v1 : v0
PSRLD	   MM4, 14 	         ; Convert from 10.22 to 10.8

; need to be checked
;PSLLD	   mm6 , 2 	          ; Convert from 10.22 to 10.8 (2 bits left instead of 14 bits right !!!)
;PAND       mm6 , dword ptr mask_low_words     
;POR         mm4 , mm6

PMADDWD	 MM4 , MM4   ;  u1*u1: u0*u0
PSRLD	     MM6, 14 	     ;   Convert from 10.22 to 10.8

PMADDWD	 MM6 , MM6   ;  v1*v1      : v0*v0
PADDD	     MM0, MM2     ;  u1 + 2du : u0 + 2du

PMULLW    MM5 ,  DWORD PTR mask_quad_15   ; turb  = 15 * turb
PADDD	    MM1  , MM3      ; v1 + 2dv : v0 + 2dv

PADDD       MM4, MM6        ; res1 = (u1*u1  + v1*v1) :  res0 = (u0*u0 + v0*v0)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;; we have to do packUsdw but there is no such an instruction
;;;;;;;;; so we do :  shift left by 16 , and then shift right Arithmetic by 16.
;;;;;;;;; the 16 bits shift left is done by 6 bits shift left instead of 10 bits shift right.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PSLLD           MM4 ,   6 
PSRAD           MM4 ,  16
PACKSSDW   MM4 ,  MM4

;;; now we clip the values against the range [0 : 2000h ] 
;;; which is the size of our sqrt table
PADDUSW     MM4 , dword ptr mask_FFFF_Minus_High
PSUBUSW     MM4 , dword ptr mask_FFFF_Minus_High

MOVD              EAX  ,  MM4
MOV                EDX  ,  EAX
AND			        EAX  ,  0ffffh							   ; eax =  res0
SHR                 EDX ,   16								  ; edx =  res1
 
MOVD              MM7 ,  [ _sqrtTable +  eax*2]  ; read from the sqrt table
PUNPCKLWD  MM7 ,  [ _sqrtTable +  edx*2]  ; 0:0:sqrt(res1):sqrt(res0) 

PMULLW         MM7  , DWORD PTR mask_quad_10   ; 10 * (0:0:sqrt(res1);sqrt(res0)) 
ADD                 EDI    , 4

PADDW           MM7  ,  MM5												; wood_indx      = 10 * sqrt(res) + 15 * turbulence
PSRLW           MM7  ,  2												    ; wood_indx  >>=  2
PSUBW          MM7  ,  DWORD PTR mask_quad_735     ; wood_indx    -= 735

PADDUSW     MM7 , dword ptr mask_FFFF_Minus_High_Wood
PSUBUSW     MM7 , dword ptr mask_FFFF_Minus_High_Wood



MOVD              EAX  , MM7												; 0:0:wood_indx1:wood_indx0
MOV                 EDX  , EAX
AND                 EAX  , 0ffffh												 ; eax = indx0 
SHR                 EDX  , 16												    ; edx = indx1    
MOVD              MM6 ,  [ _woodTable + eax*2]                  ;  read wood colors from table
PUNPCKLWD  MM6 ,  [ _woodTable + edx*2]                  ; 0:0:wood1:wood0

; need to be checked bias 
;paddusw			MM6, DWORD PTR mask_quad_green  

MOVD             EAX     , MM6
MOV	              [EDI-4] , EAX	   ; store the colors into turb_buffer

 DEC     ECX
JNZ  wood_loop

EMMS                    ; Clear out the MMX registers and set approp flags.

RET                        ; end of function
MMX_Wood ENDP


MMX_Wood_1 PROC NEAR C USES edi  ecx eax edx,
		u_init:DWORD, v_init:DWORD,
		du_init:DWORD ,dv_init:DWORD,
		num_pixels:DWORD

MOV	              ECX ,   num_pixels 
LEA	                EDI  , _turbulenceTbl

MOVD             MM2 , du_init  ;  0:du 
SHR	               ECX , 1	          ; ECX= # of times to draw 2 pixels at once

MOVD             MM0, u_init    ;  0:u
PSLLQ            MM2, 32         ;  du:0

PUNPCKLDQ   MM0, MM0    ;  u:u

MOVD              MM3, dv_init  ;  0:v
PADDD	         MM0, MM2	  ; u + du:u

MOVD			  MM1, v_init    ;  0:v  
PUNPCKHDQ   MM2, MM2     ;  du:du

PUNPCKLDQ   MM1, MM1	;  v:v

PSLLQ            MM3, 32         ;  dv:0 

PADDD	         MM1 , MM3	  ;  v + dv:v 
PUNPCKHDQ   MM3, MM3	; dv:dv

PSLLD             MM2 , 1          ; 2du:2du       

PSLLD             MM3 , 1          ; 2dv:2dv       

wood_loop:

MOVQ       MM4, MM0       ;   u1 : u0
MOVQ       MM6, MM1       ;   v1 : v0

PSLLD	    MM6 , 2 	      ; Convert from 10.22 to 10.8 (2 bits left instead of 14 bits right !!!)
PADDD	   MM0, MM2     ;  u1 + 2du : u0 + 2du

PAND         MM6 , dword ptr mask_low_words     
PSRLD	    MM4 , 14 	    ; Convert from 10.22 to 10.8

POR           MM4 , MM6
PADDD	    MM1  , MM3      ; v1 + 2dv : v0 + 2dv

MOVQ          MM5, [EDI]       ;  turbulence
PMADDWD	 MM4 , MM4     ;  res1 = (u1*u1  + v1*v1) :  res0 = (u0*u0 + v0*v0)

PMULLW    MM5 ,  DWORD PTR mask_quad_15   ; turb  = 15 * turb

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;; we have to do packUsdw but there is no such an instruction
;;;;;;;;; so we do :  shift left by 16 , and then shift right Arithmetic by 16.
;;;;;;;;; the 16 bits shift left is done by 6 bits shift left instead of 10 bits shift right.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PSLLD           MM4 ,   6 
PSRAD           MM4 ,  16
PACKSSDW   MM4 ,  MM4

;;; now we clip the values against the range [0 : 2000h ] 
;;; which is the size of our sqrt table
PADDUSW     MM4 , dword ptr mask_FFFF_Minus_High
PSUBUSW     MM4 , dword ptr mask_FFFF_Minus_High

MOVD              EAX  ,  MM4
MOV                EDX  ,  EAX
AND			        EAX  ,  0ffffh							   ; eax =  res0
SHR                 EDX ,   16								  ; edx =  res1
 
MOVD              MM7 ,  [ _sqrtTable +  eax*2]  ; read from the sqrt table
PUNPCKLWD  MM7 ,  [ _sqrtTable +  edx*2]  ; 0:0:sqrt(res1):sqrt(res0) 

PMULLW         MM7  , DWORD PTR mask_quad_10   ; 10 * (0:0:sqrt(res1);sqrt(res0)) 
ADD                 EDI    , 4

PADDW           MM7  ,  MM5												; wood_indx      = 10 * sqrt(res) + 15 * turbulence
PSRLW           MM7  ,  2												    ; wood_indx  >>=  2
PSUBW          MM7  ,  DWORD PTR mask_quad_735     ; wood_indx    -= 735

PADDUSW     MM7 , dword ptr mask_FFFF_Minus_High_Wood
PSUBUSW     MM7 , dword ptr mask_FFFF_Minus_High_Wood

MOVD              EAX  , MM7												; 0:0:wood_indx1:wood_indx0
MOV                 EDX  , EAX
AND                 EAX  , 0ffffh												 ; eax = indx0 
SHR                 EDX  , 16												    ; edx = indx1    
MOVD              MM6 ,  [ _woodTable + eax*2]                  ;  read wood colors from table
PUNPCKLWD  MM6 ,  [ _woodTable + edx*2]                  ; 0:0:wood1:wood0

; need to be checked bias 
;paddusw			MM6, DWORD PTR mask_quad_green  

MOVD             EAX     , MM6
MOV	              [EDI-4] , EAX	   ; store the colors into turb_buffer

 DEC     ECX
JNZ  wood_loop

EMMS                    ; Clear out the MMX registers and set approp flags.

RET                        ; end of function
MMX_Wood_1 ENDP


MMX_Wood_4_Pix PROC NEAR C USES edi  ecx eax edx,
		u_init:DWORD, v_init:DWORD,
		du_init:DWORD ,dv_init:DWORD,
		num_pixels:DWORD

MOV	              ECX ,   num_pixels 
LEA	                EDI  , _turbulenceTbl

MOVD             MM4 , du_init  ;  0:du 
SHR	               ECX , 2	          ; ECX= # of times to draw 4 pixels at once

MOVD             MM0, u_init    ;  0:u
PSLLQ            MM4, 32         ;  du:0

PUNPCKLDQ   MM0, MM0    ;  u:u

MOVD              MM5, dv_init  ;  0:v
PADDD	         MM0, MM4	  ; u + du:u

MOVD			  MM1, v_init    ;  0:v  
PUNPCKHDQ   MM4, MM4     ;  du:du

PUNPCKLDQ   MM1, MM1	;  v:v

PSLLQ            MM5, 32         ;  dv:0 

PADDD	         MM1 , MM5	  ;  v + dv:v 
PUNPCKHDQ   MM5, MM5	; dv:dv

MOVQ              MM2 , MM0    ; u + du:u
MOVQ              MM3 , MM1    ; v + dv:v 

PSLLD             MM4 , 1          ; 2du:2du       
PSLLD             MM5 , 1          ; 2dv:2dv       

PADDD            MM2 , MM4   ; u + 3du:u+2du
PADDD            MM3 , MM5   ; v + 3dv:v+2dv 

PSLLD             MM4 , 1          ; 4du:4du       
PSLLD             MM5 , 1          ; 4dv:4dv       

MOVQ           dword ptr du , mm4
MOVQ           dword ptr dv , mm5

wood_loop:

MOVQ       MM5, [EDI]       ;   turbulence
MOVQ       MM4, MM0       ;   u1 : u0

MOVQ       MM6, MM1       ;   v1 : v0
PSRLD	   MM4, 14 	         ; Convert from 10.22 to 10.8

PMADDWD	 MM4 , MM4   ;  u1*u1: u0*u0
PSRLD	      MM6, 14 	     ;   Convert from 10.22 to 10.8

PMADDWD	 MM6 , MM6   ;  v1*v1      : v0*v0
PADDD	     MM0, dword ptr du ; MM2     ;  u1 + 2du : u0 + 2du

PMULLW    MM5 ,  DWORD PTR mask_quad_15   ; turb  = 15 * turb
PADDD	    MM1  , dword ptr dv ; MM3      ; v1 + 2dv : v0 + 2dv

PADDD       MM4, MM6        ; res1 = (u1*u1  + v1*v1) :  res0 = (u0*u0 + v0*v0)

MOVQ          MM7 , MM2       ; u3:u2
MOVQ          MM6 , MM3       ; v3:v2
PSRLD	      MM7  , 14 	     ; Convert from 10.22 to 10.8
PSRLD	      MM6  , 14 	     ; Convert from 10.22 to 10.8
PMADDWD	 MM7 , MM7       ;  u3*u3: u2*u2
PMADDWD	 MM6 , MM6       ;  v3*v3: v2*v2
PADDD         MM2 , DWORD PTR du
PADDD         MM3 , DWORD PTR dv
PADDD         MM6 , MM7        ; res1 = (u3*u3  + v3*v3) :  res0 = (u2*u2 + v2*v2)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;; we have to do packUsdw but there is no such an instruction
;;;;;;;;; so we do :  shift left by 16 , and then shift right Arithmetic by 16.
;;;;;;;;; the 16 bits shift left is done by 6 bits shift left instead of 10 bits shift right.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PSLLD           MM4 ,   6 
PSRAD          MM4 ,  16

PSLLD           MM6 ,   6
PSRAD          MM6 ,  16 

PACKSSDW   MM4 ,  MM6

;;; now we clip the values against the range [0 : 2000h ] 
;;; which is the size of our sqrt table
PADDUSW     MM4 , dword ptr mask_FFFF_Minus_High
PSUBUSW     MM4 , dword ptr mask_FFFF_Minus_High

MOVD              EAX  ,  MM4
MOV                EDX  ,  EAX
AND			        EAX  ,  0ffffh							   ; eax =  res0
SHR                 EDX ,   16								  ; edx =  res1
MOVD              MM7 ,  [ _sqrtTable +  eax*2]  ; read from the sqrt table
PUNPCKLWD  MM7 ,  [ _sqrtTable +  edx*2]  ; 0:0:sqrt(res1):sqrt(res0) 

PSRLQ            MM4 , 32
MOVD              EAX  ,  MM4
MOV                EDX  ,  EAX
AND			        EAX  ,  0ffffh							   ; eax =  res2
SHR                 EDX ,   16								  ; edx =  res3
MOVD              MM6 ,  [ _sqrtTable +  eax*2]  ; read from the sqrt table
PUNPCKLWD  MM6 ,  [ _sqrtTable +  edx*2]  ; 0:0:sqrt(res3):sqrt(res2) 
PUNPCKLDQ   MM7 , MM6							   ; sqrt(res3):sqrt(res2):sqrt(res1):sqrt(res0)   

PMULLW         MM7  , DWORD PTR mask_quad_10   ; 10 * (sqrt(res3):sqrt(res2):sqrt(res1):sqrt(res0)) 
ADD                 EDI    , 8

PADDW           MM7  ,  MM5												; wood_indx      = 10 * sqrt(res) + 15 * turbulence
PSRLW           MM7  ,  2												    ; wood_indx  >>=  2
PSUBW          MM7  ,  DWORD PTR mask_quad_735     ; wood_indx    -= 735

PADDUSW     MM7 , dword ptr mask_FFFF_Minus_High_Wood
PSUBUSW     MM7 , dword ptr mask_FFFF_Minus_High_Wood



MOVD              EAX  , MM7												; 0:0:wood_indx1:wood_indx0
MOV                 EDX  , EAX
AND                 EAX  , 0ffffh												 ; eax = indx0 
SHR                 EDX  , 16												    ; edx = indx1    
MOVD              MM6 ,  [ _woodTable + eax*2]                  ;  read wood colors from table
PUNPCKLWD  MM6 ,  [ _woodTable + edx*2]                  ; 0:0:wood1:wood0

PSRLQ            MM7 , 32
MOVD              EAX  , MM7												; 0:0:wood_indx1:wood_indx0
MOV                EDX  , EAX
AND                 EAX  , 0ffffh												 ; eax = indx0 
SHR                 EDX  , 16												    ; edx = indx1    
MOVD              MM7 ,  [ _woodTable + eax*2]                  ;  read wood colors from table
PUNPCKLWD  MM7 ,  [ _woodTable + edx*2]                  ;  0:0:wood3:wood2
PUNPCKLDQ   MM6 , MM7											    ;  wood3:wood2:wood1:wood0

;MOVD             EAX     , MM6
MOVQ              [EDI-8] , MM6	   ; store the colors into turb_buffer

 DEC     ECX
JNZ  wood_loop

EMMS                    ; Clear out the MMX registers and set approp flags.

RET                        ; end of function
MMX_Wood_4_Pix ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MMX_Zbuffer PROC NEAR C USES edi esi ebx ecx eax,
		z_init:   DWORD         , 
		dz_init: DWORD         ,
		num_pixels: DWORD  , 
        z_line:        PTR SWORD  ,   color_line: PTR SWORD,
		z_buffer:    PTR SWORD  ,   color_buffer: PTR SWORD 

MOVD   MM4,  z_init
MOVD   MM5, dz_init
MOVD   MM1, dz_init   

PSLLQ            MM1, 16
PUNPCKLWD  MM4, MM4	  ;0:0:z:z

PADDSW	      MM4, MM1	  ;0:0:z + dz:z
PUNPCKLWD  MM5, MM5	  ;0:0:dz:dz

MOVQ			MM3, MM4   ;0:0:z +dz:z  
PSLLW           MM5 ,1         ;0:0:2dz:2dz       

PSLLQ            MM3 ,  32     ;z +dz:z:0:0 
PSLLQ            MM5 ,  32     ;2dz:2dz:0:0 

PADDSW        MM3 , MM5   ;z+3z:z+2dz:0:0 
PUNPCKHDQ  MM5 , MM5   ;2dz:2dz:2dz:2dz

POR               MM4 , MM3   ;z+3z:z+2dz:z + dz:z
PSLLW           MM5 , 1         ;4dz:4dz:4dz:4dz        

MOV eax ,  z_buffer
MOV ebx ,  z_line

MOV ecx ,  color_buffer
MOV edi  ,  color_line

MOV esi  ,  num_pixels
SHR  esi ,   2

zLoop:
   MOVQ			MM0 , [eax]  ; mm0 = Za,Za,Za,Za (load)
   MOVQ			MM1 , MM4  ; [ebx] ; mm1 = Zb,Zb,Zb,Zb (load)
   
   MOVQ			MM2 , MM0  ; mm2 = Za,Za,Za,Za (wil be the mask)
   PADDSW     MM4 , MM5     
   
   PCMPGTW  MM2 , MM1   ; mm2 = mask of 0000 or fffff (4 times) 
   MOVQ         MM3 , MM2   ; (after pxor) mm3 = ~mm2 (mm2 xor fffffffffffffff)
   PXOR          MM3 , DWORD PTR  mask_all_1 
    
   PAND          MM1 ,  MM2   ; mm1 = only the Zb's which are less then the Za's
   PAND          MM0 ,  MM3   ; mm0 = the Z'a which are less or EQUAL the Zb's
   POR            MM0 ,  MM1   ; mm0 = the wanted Z's
   MOVQ         [eax] , MM0    ; (store)
   
   MOVQ         MM0 ,  [ecx]   ; mm0 = Ca,Ca,Ca,Ca
   MOVQ         MM1 ,  [edi]    ; mm1 = Cb,Cb,Cb,Cb
   
   PAND           MM1 , MM2   ; mm1 = the Ca's of the 'Good' Za's
   PAND           MM0 , MM3   ; mm0 = the Cb's of the 'Good' Zb's
   POR             MM0 , MM1  ; the wanted C's
   MOVQ          [edi]  , MM0  ; (store)
  
  ADD            eax , 8
  ADD            ebx , 8
  ADD            ecx , 8
  ADD            edi , 8
  
  DEC             esi 
  JNZ			   zLoop
EMMS
RET
MMX_Zbuffer ENDP

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; z is calculated along the scan line z = z_init + i * dz_init
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MMX_INCZbuffer PROC NEAR C USES edi esi ecx eax ebx,
		z_init:   DWORD           , 
		dz_init: DWORD           ,
		num_pixels: DWORD   , 
        z_line:        PTR SWORD   ,   color_line: PTR SWORD,
		z_buffer:    PTR SWORD   ,   color_buffer: PTR SWORD 

MOVD   MM1, dz_init   
MOVD   MM4,  z_init

MOVD   MM5, dz_init
PSLLD   MM1, 16               ; 0:0:dz:0

PAND     MM4 , DWORD PTR mask_clear_byte_1

MOVQ            MM6, DWORD PTR  mask_all_1 
PUNPCKLWD  MM4, MM4	  ;0:0:z:z

PADDSW	      MM4, MM1	  ;0:0:z + dz:z
PUNPCKLWD  MM5, MM5	  ;0:0:dz:dz

MOVQ			MM3, MM4   ;0:0:z +dz:z  
PSLLW           MM5 ,1         ;0:0:2dz:2dz       

PSLLQ            MM3 ,  32     ;z +dz:z:0:0 
MOV               eax   ,   z_buffer

PSLLQ            MM5 ,  32     ;2dz:2dz:0:0 
MOV               edi    ,  color_line

PADDSW        MM3 , MM5   ;z+3z:z+2dz:0:0 
PUNPCKHDQ  MM5 , MM5   ;2dz:2dz:2dz:2dz

POR               MM4 , MM3   ;z+3z:z+2dz:z + dz:z
PSLLW           MM5 , 1         ;4dz:4dz:4dz:4dz        

MOV ecx ,  color_buffer
MOV esi  ,  num_pixels

SHR  esi ,   2

zLoop:
   MOVQ			MM0 , [eax]  ; mm0 = Za,Za,Za,Za (load)
   MOVQ			MM1 , MM4  ; [ebx] ; mm1 = Zb,Zb,Zb,Zb (load)
   
   MOVQ			MM2 , MM0  ; mm2 = Za,Za,Za,Za (wil be the mask)
   PADDSW     MM4 , MM5     
   
   PCMPGTW  MM2 , MM1   ; mm2 = mask of 0000 or fffff (4 times) 
   ADD            eax , 8
   
   MOVQ         MM3 , MM2   ; (after pxor) mm3 = ~mm2 (mm2 xor fffffffffffffff)
   PAND          MM1 ,  MM2   ; mm1 = only the Zb's which are less then the Za's
   
   PXOR          MM3 , MM6  ;DWORD PTR  mask_all_1 
   ADD            ecx , 8

   PAND          MM0 ,  MM3   ; mm0 = the Z'a which are less or EQUAL the Zb's
   ADD            edi , 8

   POR            MM0 ,  MM1   ; mm0 = the wanted Z's
   MOVQ         [eax-8] , MM0    ; (store Z's)
   
   MOVQ         MM0 ,  [ecx-8]   ; mm0 = Ca,Ca,Ca,Ca
   MOVQ         MM1 ,  [edi-8]    ; mm1 = Cb,Cb,Cb,Cb
   
   PAND           MM1 , MM2   ; mm1 = the Ca's of the 'Good' Za's
   PAND           MM0 , MM3   ; mm0 = the Cb's of the 'Good' Zb's
   
   POR             MM0 , MM1  ; the wanted C's
   MOVQ          [edi-8]  , MM0  ; (store)
   DEC             esi 
  JNZ			     zLoop
EMMS
RET
MMX_INCZbuffer ENDP

PUBLIC	_my_ftol
;	COMDAT _my_ftol
_TEXT	SEGMENT
_d$ = 4
_my_ftol PROC NEAR					; COMDAT
     
	fld	     DWORD PTR _d$[esp]
    fistp	DWORD PTR result
    mov	  eax , DWORD PTR result
	ret	0
_my_ftol ENDP
_TEXT	ENDS 




END

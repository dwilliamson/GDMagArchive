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
TITLE Modified form of Perlin's Noise Basis function using MMX(tm) technology

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

;KEY for comments
;P0, P1, P# = Pixel number 0, Pixel number 1, Pixel number # respectively.
;Pix        = Pixel
;DU         = Derivative of the variable U.
;DDU        = Derivative of the variable DU.
;Texel      = A point in the texture to be mapped onto the screen.  Given by U, V.

;Note: Even though the assembly writes four pixel values through each pass of the
;inner loop, only two of the pixels are directly calculated.  The other two pixels
;are averaged from neighboring pixels.  According to the current scheme, 
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MMX = | Pixel #0     | Pixel #1     | Pixel #2    | Pixel #3      |
;      +-----------------------------------------------------------+
;Pixels #1 and #3 are directly calculated.  Pixel #2 is averaged from Pixel #1 and
;pixel #3.  Pixel #0 is averaged from Pixel #1 and the previous pixel before #0.
;
;Also, the programmer realizes that the pixels are labeled from 0, 1, 2, 3 instead
;of 3, 2, 1, 0 as follows the conventional format of Intel Architecture.  This was 
;an oversite and not realized until it was to late.

;Variables, u, v, du, dv, ddu, ddv each contain parameters for two
;texels.  Since u, v, ..., ddv are 64 bit, then each texel parameter is
;32 bit.  (32 bit per texel * two texels = 64 bits).  This enables us
;to work with two pixels at one time using MMX technology.
ALIGN 8
u		    QWORD ?
du		   QWORD ?
ddu		  QWORD ?

v		    QWORD ?
dv		   QWORD ?
ddv		  QWORD ?

firstU     QWORD ?
firstV     QWORD ?

   

;Since the program only calculates odd pixel values, the even pixel values
;must be averaged.  Therefore, for each pass through the inner loop, four
;pixels will be drawn.	In order to draw the first pixel, the pixel before
;it must be known for the averaging.  This pixel color is contained here.
prev_color        DWORD 255
octShift			 DWORD  0,0
turbShift            DWORD  0,0

;Various masks.  Set up to filter out unwanted bits in MMX registers.
ALIGN 8
mask_32_to_15	  QWORD 00007FFF00007FFFh
mask_quad_1	  QWORD 0001000100010001h
mask_quad_255	  QWORD 00FF00FF00FF00FFh
mask_quad_256	  QWORD 0100010001000100h
mask_quad_510	  QWORD 01FE01FE01FE01FEh
mask_quad_511	  QWORD 01FF01FF01FF01FFh
mask_quad_1536	  QWORD 0600060006000600h
mask_double_255   QWORD 000000FF000000FFh
mask_double_FFFF  QWORD 0000FFFF0000FFFFh
mask_double_65536 QWORD 0001000000010000h
mask_four_255        QWORD 00FF00FF00FF00FFh


DSEG ENDS

;***********************************************************************
;     Constant Segment Declarations
;***********************************************************************
.const

;***********************************************************************
;     Code Segment Declarations
;***********************************************************************
.code

COMMENT^
void MMX_Octave(unsigned long u_init, unsigned long v_init, 
				            long du_init, long dv_init,
	                        unsigned long Num_Pix,
                            unsigned _int16* turb_buffer,
							unsigned long num_octaves);
^

MMX_Octave PROC NEAR C USES  ebx ecx edi esi,
			u_init:DWORD			, v_init:DWORD, 
			du_init:DWORD		   , dv_init:DWORD,
			num_pixels:DWORD    , turb_buffer:DWORD,
			num_octaves:DWORD 

;Initialization

MOVD	       MM0, u_init

MOVD		   MM1, v_init
PUNPCKLDQ   MM0, MM0	  ;U p1 = u, p3 = u

MOVD			 MM2, du_init
PUNPCKLDQ   MM1, MM1	  ;V p1 = v, p3 = v

MOVD	    MM3, dv_init
PADDD	    MM0, MM2	  ;U p1 = u, p3 = u + du

PADDD	    MM1, MM3	  ;V p1 = v, p3 = v + dv
PADDD	    MM0, MM2	  ;U p1 = u, p3 = u + 2du

PADDD	         MM1, MM3	  ;V p1 = v, p3 = v + 2dv
PUNPCKLDQ   MM2, MM2

PUNPCKLDQ   MM3, MM3
PADDD	         MM0, MM2	  ;U p1 = u + du, p3 = u + 3du

MOV        [turbShift]  , 0    ; turbShift is the octave number 0,1,2,....
XOR        ESI,ESI               

MOVQ	   DWORD PTR firstU , MM0
PADDD	   MM1, MM3	  ;V p1 = v + dv, p3 = v + 3dv

MOV          [octShift]   , 14  ; octshift is (14 - esi (octave number))
PSLLD	    MM2, 2	  ;DU p1 = 4du, p3 = 4du

MOVQ	   DWORD PTR firstV,  MM1
PSLLD	    MM3, 2   ;DU p1 = 4dv, p3 = 4dv

MOVQ	    DWORD PTR du, MM2

MOVQ	    DWORD PTR dv, MM3


start_octave : 
MOV	    EBX, prev_color
MOV	    EDI,  turb_buffer ;EDI will always be pointer to screen buffer

MOV	    ECX, num_pixels
SUB	     EDI, 8

;Get the UV parameters in MMX(tm) technology form.
;Note: UV texel values are stored in 10.22 fixed integer format.
;This sets up the U parameters for pixels 1 and 3 in MM0 register and
;V parameter in MM1 register.  After setup, the registers will contain:
;      |--------- 32 bit ------------|
;      +-------------------------------------------------------------------+
;MM0 = | U texel for pix #1 = u + du | U texel for pix #3 = u + 3du + 3ddu |
;      +-------------------------------------------------------------------+
;      +-------------------------------------------------------------------+
;MM1 = | V texel for pix #1 = v + dv | V texel for pix #3 = v + 3dv + 3ddv |
;      +-------------------------------------------------------------------+
;This is because the first four pixels drawn on the screen will have the
;U and V texel values of:
;Pixel #0 = u
;Pixel #1 = u + du
;Pixel #2 = u + 2du + ddu
;Pixel #3 = u + 3du + 3ddu
;We are only interested in pixels #1 and #3 because pixels #0 and #2 are averaged.

movq          mm0,     DWORD PTR firstU
movq          mm1,     DWORD PTR firstV
movq          DWORD PTR u , mm0     
movq          DWORD PTR v , mm1




;Get the du dv parameters in MMX(tm) technology form
;Note: du dv texel values are stored in 10.22 fixed integer format.
;This sets up the du parameters for pixels 1 and 3 in MM0 register and
;dv parameter in MM1 register.	After setup, the registers will contain:
;      |--------- 32 bit --------------|
;      +---------------------------------------------------------------+
;MM0 = | DU texel for p1 = 4du + 10ddu | DU texel for p3 = 4du + 18ddu |
;      +---------------------------------------------------------------+
;      +---------------------------------------------------------------+
;MM1 = | DV texel for p1 = 4dv + 10ddv | DV texel for p3 = 4dv + 18ddv |
;      +---------------------------------------------------------------+
;This is because after the first four pixels are drawn on the screen, the
;loop repeats to draw the next four pixels.  In order to get the next u, v
;texel coordinates, appropriate du, dv values need to be summed to u and v.
;The correct starting values of du and dv are:
;Pixel #0 = 4du	+  6ddu  ;Note: these have been mathematically proven.
;Pixel #1 = 4du + 10ddu
;Pixel #2 = 4du + 14ddu
;Pixel #3 = 4du + 18ddu
;We are only interested in pixels #1 and #3 because pixels #0 and #2 are averaged.

;nop
;nop
;nop
;nop
;nop
start_scan_line:
;First, the program converts the u and v texel coordinates
;from 10.22 format to 8.8 format.  10.22 format is used for
;decimal accuracy but only 16 of the 32 bits are actually used.
;Because the final format will fit in a 16 bit result, u and v
;values are converted from 4, 32 bit packed values
;to 4, 16 bit packed values that will fit in one MMX register.	Output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM0 = | U texel - p1 | U texel - p3 | V texel - p1 | V texel - p3 |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;u_16bit = u_init >> 14;
;v_16bit = v_init >> 14;

MOVQ	    MM1, DWORD PTR u;

MOVQ       MM3, DWORD PTR octShift

MOVQ	    MM0, DWORD PTR v;
PSRLD	    MM1,MM3							;Convert from 10.22 to 10.8

MOVQ	    MM2, DWORD PTR mask_32_to_15 ;Uses 15 instead of 16 because of signed saturation.
PSRLD	    MM0,MM3                           ;Convert from 10.22 to 10.8

;PSRLD	    MM1, [octShift] 	  
;PSRLD	    MM0, [octShift]	   

PAND	    MM1, MM2	   ;Convert from 10.8 to 7.8 integer format
PAND	    MM0, MM2	   ;Convert from 10.8 to 7.8 integer format

MOVQ	        MM3, DWORD PTR mask_quad_1
PACKSSDW    MM0, MM1	   ;Pack the result into one register

;Calculation of the bx0, by0, bx1, by1 values for both pixels.	Output:
;             |-8 bit-|
;      +-----------------------------------------------------------+
;MM2 = |      |BX0 p1 |      |BX0 p3 |      |BY0 p1 |      |BY0 p3 |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM3 = |      |BX1 p1 |      |BX1 p3 |      |BY1 p1 |      |BY1 p3 |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;bx0 = u_16bit >> 8;
;by0 = v_16bit >> 8;
;bx1 = bx0 + 1;
;by1 = by0 + 1;

MOVQ	    MM1, DWORD PTR u	    ;Used for incrementing u for next 4 pix.
MOVQ	    MM2, MM0

PSRLW	    MM2, 8

;PADDD	      MM1, MM4	    ;Used for incrementing u for next 4 pix.
PADDD	      MM1,  DWORD PTR du	    ;Used for incrementing u for next 4 pix.
PADDUSB     MM3, MM2   ;mm3 = 0:BX1(1):0:BX1(3):0:BY1(1):0:BY1(3)


;Calculation of the rx0, ry0 values for both pixels.  Final output:
;	      |-8 bit-|
;      +-----------------------------------------------------------+
;MM0 = |      |RX0 p1 |      |RX0 p3 |	    |RY0 p1 |	   |RY0 p3 |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;rx0 = u_16bit & 255;
;ry0 = v_16bit & 255;

PSLLW	    MM0, 8  
MOVQ	    MM4, MM3

MOVQ	    MM6, DWORD PTR mask_quad_1
PUNPCKHWD   MM4, MM2    ;mm4 = 0:BX0(1):0:BX1(1):0:BX0(3):0:BX1(3)

PUNPCKLWD   MM3, MM2    ;mm3  = 0:BY0(1):0:BY1(1):0:BY0(3):0:BY1(3)
PMULLW	    MM4, MM4       ;mm4  = BX0^2(1):BX1^2(1):BX0^2(3):BX1^2(3)
 
PSRLW	    MM0, 8	   ;MM0 = rx0 and ry0 param for pix 1, 3
;This section includes calculation of b00, b01, b10, b11.  Output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM4 = | b01 for p1   | b11 for p1   | b01 for p3   | b11 for p3   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM5 = | b00 for p1   |	b10 for p1   | b00 for p3   | b10 for p3   |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;b00 = random1((random1(bx0) + by0));
;b01 = random1((random1(bx0) + by1));
;b10 = random1((random1(bx1) + by0));
;b11 = random1((random1(bx1) + by1));
MOVQ	    MM2, MM3        
 
PUNPCKLDQ   MM3, MM3    ;mm3  = 0:BY0(3):0:BY1(3):0:BY0(3):0:BY1(3)

PUNPCKHDQ   MM2, MM2    ;mm2  = 0:BY0(1):0:BY1(1):0:BY0(1):0:BY1(1)
MOVQ	         MM5, MM4

MOVQ	         DWORD PTR u, MM1	   ;Used for incrementing u for next 4 pix.
PUNPCKLWD   MM4, MM4  ;mm4  = BX0^2(3):BX0^2(3):BX1^2(3):BX1^2(3)

PUNPCKHWD   MM5, MM5  ;mm5  = BX0^2(1):BX0^2(1):BX1^2(1):BX1^2(1)
PADDW	    MM4, MM3


PADDW	    MM5, MM2
;This section calculates g_b00_0, b_b01_0, g_b10_0, g_b11_0 for pix 1 and 3.
;Output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM2 = | g_b00_1 p3   | g_b01_1 p3   | g_b10_1 p3   | g_b11_1 p3   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM3 = | g_b00_1 p1   |	g_b01_1 p1   | g_b10_1 p1   | g_b11_1 p1   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM4 = | g_b00_0 p3   | g_b01_0 p3   | g_b10_0 p3   | g_b11_0 p3   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM5 = | g_b00_0 p1   |	g_b01_0 p1   | g_b10_0 p1   | g_b11_0 p1   |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;g_b00_0 = (random2(b00) & 511) - 256;
;g_b01_0 = (random2(b01) & 511) - 256;
;g_b10_0 = (random2(b10) & 511) - 256;
;g_b11_0 = (random2(b11) & 511) - 256;
;g_b00_1 = (random2(b00 + 1) & 511) - 256;
;g_b01_1 = (random2(b01 + 1) & 511) - 256;
;g_b10_1 = (random2(b10 + 1) & 511) - 256;
;g_b11_1 = (random2(b11 + 1) & 511) - 256;
PMULLW	    MM4, MM4  ;random1

PMULLW	    MM5, MM5  ;random1
MOVQ	      MM2, MM6

MOVQ	     MM3, MM6
PADDUSW   MM2, MM4

PMULLW	    MM2, MM2     ;random2
PADDUSW    MM3, MM5

MOVQ	    MM1, DWORD PTR mask_quad_256
PMULLW	   MM3, MM3     ;random2

MOVQ	    MM7, DWORD PTR mask_quad_511
PMULLW	   MM4, MM4    ;random2


PMULLW	    MM5, MM5   ;random2
PSRLW	      MM2, 2

PSRLW	    MM3, 2
PAND	    MM2, MM7

PSRLW	    MM4, 2
PAND	    MM3, MM7

PSRLW	    MM5, 2
PAND	    MM4, MM7

PAND	    MM5, MM7
PSUBW	    MM2, MM1	   ;MM2 = g_b##_1 for pixel #3

PSUBW	    MM3, MM1	   ;MM3 = g_b##_1 for pixel #1
PSUBW	    MM4, MM1	   ;MM4 = g_b##_0 for pixel #3

PSUBW	    MM5, MM1	   ;MM5 = g_b##_0 for pixel #1

;Take above data for g_b00_0, b_b01_0, g_b10_0, g_b11_0 for pix 1 and 3
;and rearrange the packed values in the MMX registers.
;Output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM2 = | g_b00_0 p3   | g_b00_1 p3   | g_b01_0 p3   | g_b01_1 p3   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM3 = | g_b00_0 p1   |	g_b00_1 p1   | g_b01_0 p1   | g_b01_1 p1   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM6 = | g_b10_0 p3   | g_b10_1 p3   | g_b11_0 p3   | g_b11_1 p3   |
;      +-----------------------------------------------------------+
;      +-----------------------------------------------------------+
;MM7 = | g_b10_0 p1   |	g_b10_1 p1   | g_b11_0 p1   | g_b11_1 p1   |
;      +-----------------------------------------------------------+

MOVQ	    MM6, MM2

MOVQ	    MM7, MM3
PUNPCKHWD   MM2, MM4	   ;MM2 = g_b00_# and g_b01_# for pix #3

PUNPCKLWD   MM6, MM4	   ;MM6 = g_b10_# and g_b11_# for pix #3

PUNPCKHWD   MM3, MM5	   ;MM3 = g_b00_# and g_b01_# for pix #1
MOVQ	    MM4, MM0	   ;Preparing for rx1 and ry1 calculation

PUNPCKLWD   MM7, MM5	   ;MM7 = g_b10_# and g_b11_# for pix #1

;Calculation of the rx1, ry1 values for both pixels.  Final output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM4 = |       RX1 p1 |       RX1 p3 |	     RY1 p1 |	    RY1 p3 |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;rx1 = rx0 - 256;
;ry1 = ry0 - 256;

PSUBW	    MM4, MM1	   ;MM4 = rx1 and ry1 parameters


;Setup for the calculation of u1 and u2 for pix #1.  Final output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM1 = |       RX0 p1 |	     RY0 p1 |	     RX0 p1 |	    RY1 p1 |
;      +-----------------------------------------------------------+

MOVQ	    MM5, MM0
MOVQ	    MM1, MM4

PSRLD	    MM5, 16

PSRAD	    MM1, 16

PSLLQ	    MM1, 32

PUNPCKHDQ   MM1, MM5

PACKSSDW    MM1, MM1

PACKSSDW    MM5, MM5

PUNPCKLDQ   MM1, MM5

;Calculation for U1 and U2 for pixel #1 -> After multiplication... Output:
;      |--------- 32 bit ---------|
;      +-----------------------------------------------------+
;MM3 = | U1 for pixel #1	  | U2 for pixel #1	     |
;      +-----------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;u1 = rx0 * g_b00_0 + ry0 * g_b00_1;
;u2 = rx0 * g_b01_0 + ry1 * g_b01_1;
PMADDWD     MM3, MM1       ;43u, MM3 = u1 and u2 for pixel #1

;Setup for the calculation of v1 and v2 for pix #1.  Final output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM5 = |       RX1 p1 |	     RY0 p1 |	     RX1 p1 |	    RY1 p1 |
;      +-----------------------------------------------------------+

MOVQ	    MM5, MM4

PSRAD	    MM5, 16
MOVQ	    MM1, MM0

PSRLD	    MM1, 16

PSLLQ	    MM1, 32

PUNPCKHDQ   MM1, MM5

PACKSSDW    MM1, MM1

PACKSSDW    MM5, MM5

PUNPCKLDQ   MM5, MM1

;Calculation for V1 and V2 for pixel #1 -> After multiplication... Output:
;      |--------- 32 bit ---------|
;      +-----------------------------------------------------+
;MM7 = | V1 for pixel #1	  | V2 for pixel #1	     |
;      +-----------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;v1 = rx1 * g_b00_0 + ry0 * g_b00_1;
;v2 = rx1 * g_b01_0 + ry1 * g_b01_1;

PMADDWD     MM7, MM5	   ;MM7 = v1 and v2 for pixel #1

;Setup for the calculation of u1 and u2 for pix #3.  Final output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM1 = |       RX0 p3 |	     RY0 p3 |	     RX0 p3 |	    RY1 p3 |
;      +-----------------------------------------------------------+

MOVQ	    MM5, MM0

PSLLD	    MM5, 16

PSRLD	    MM5, 16
MOVQ	    MM1, MM4

PSLLD	    MM1, 16

PSRAD	    MM1, 16

PUNPCKLDQ   MM1, MM1

PUNPCKHDQ   MM1, MM5

PACKSSDW    MM1, MM1

PACKSSDW    MM5, MM5

PUNPCKLDQ   MM1, MM5

;Calculation for U1 and U2 for pixel #3 -> After multiplication... Output:
;      |--------- 32 bit ---------|
;      +-----------------------------------------------------+
;MM2 = | U1 for pixel #3	  | U2 for pixel #3	     |
;      +-----------------------------------------------------+
PMADDWD     MM2, MM1	   ;MM2 = u1 and u2 for pixel #3


;Setup for the calculation of v1 and v2 for pix #3.  Final output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM4 = |       RX1 p3 |	     RY0 p3 |	     RX1 p3 |	    RY1 p3 |
;      +-----------------------------------------------------------+

PSLLD	    MM4, 16

PSRAD	    MM4, 16
MOVQ	    MM5, MM0

PSLLD	    MM5, 16

PSRAD	    MM5, 16

PUNPCKLDQ   MM5, MM5

PUNPCKHDQ   MM5, MM4

PACKSSDW    MM5, MM5

PACKSSDW    MM4, MM4

PUNPCKLDQ   MM4, MM5

;Calculation for V1 and V2 for pixel #3 -> After multiplication... Output:
;      |--------- 32 bit ---------|
;      +-----------------------------------------------------+
;MM6 = | V1 for pixel #3	  | V2 for pixel #3	     |
;      +-----------------------------------------------------+
PMADDWD     MM6, MM4	   ;MM6 = v1 and v2 for pixel #2

;Calculation for SX and SY for pixels #1 and #3, Output:
;      |--- 16 bit ---|
;      +-----------------------------------------------------------+
;MM1 = |       SX  p1 |	     SX  p3 |	     SY  p1 |	    SY	p3 |
;      +-----------------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;sx = (((rx0 * rx0) >> 1) * ((1536 - (rx0 << 2)))) >> 16;
;sy = (((ry0 * ry0) >> 1) * ((1536 - (ry0 << 2)))) >> 16;
MOVQ	    MM5, MM0

PMULLW	    MM5, MM5
MOVQ	    MM4, MM0

MOVQ	    MM1, DWORD PTR mask_quad_1536
PSLLW	    MM4, 2

PSUBD	    MM6, MM2	   ;V1 - U1 and V2 - U2 for P3
PSUBD	    MM7, MM3	   ;V1 - U1 and V2 - U2 for P1

PSUBW	    MM1, MM4
PSRLW	    MM5, 1

PMULHW	    MM1, MM5	   ;MM1 = sx and sy param for pix 1, 3

;Calculation of A and B for pixel #1 and #3. Output:
;      |--------- 32 bit ---------|
;      +-----------------------------------------------------+
;MM7 = | A for pixel #1		  | B for pixel #1	     |
;      +-----------------------------------------------------+
;      +-----------------------------------------------------+
;MM6 = | A for pixel #3 	  | B for pixel #3	     |
;      +-----------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;a = u1 + sx * ((v1 - u1) >> 8);
;b = u2 + sx * ((v2 - u2) >> 8);
PSRAD	    MM7, 8

PSRAD	    MM6, 8

MOVQ	    MM4, MM1
MOVQ	    MM5, MM1

PSRLQ	    MM4, 16

PUNPCKLWD   MM1, MM1

PUNPCKHDQ   MM4, MM4

PMADDWD     MM7, MM4
PSLLD	    MM5, 16

MOVQ	    MM4, DWORD PTR v	    ;Used for incrementing v for next 4 pix
PSRLD	    MM5, 16

PUNPCKHDQ   MM5, MM5
;PADDD	    MM4, MM0	    ;Used for incrementing v for next 4 pix
PADDD	    MM4, DWORD PTR dv	    ;Used for incrementing v for next 4 pix

PADDD	    MM7, MM3	    ;MM7 = a and b parameter for pix #1
PMADDWD     MM6, MM5

MOVQ	    MM3, DWORD PTR mask_double_65536
PSRLD	    MM1, 16

MOVQ	    DWORD PTR v, MM4	    ;Used for incrementing v for next 4 pix

;Calculation of color indexes for pixel #1 and #3. Output:
;      |--------- 32 bit ---------|
;      +-----------------------------------------------------+
;MM7 = | Color index for pixel #1 | Color index for pixel #3 |
;      +-----------------------------------------------------+
;This code correlates to the following "C" code in the "C_Noise()" function.
;color = (a + 65536 + sy * ((b - a) >> 8)) >> 9;
PADDD	    MM6, MM2	    ;MM6 = a and b parameter for pix #3

MOVQ	    MM4, DWORD PTR mask_quad_510
MOVQ	    MM2, MM6

PUNPCKLDQ   MM6, MM7

MOVD	          MM0, ebx	    ;Move the last color written into MM2
PUNPCKHDQ   MM2, MM7

PADDD	    MM3, MM2
PSUBD	    MM6, MM2

PSRAD	    MM6, 8

PMADDWD     MM6, MM1

PADDD	    MM6, MM3

PSRLD	    MM6, 9	    ;MM6 = color for pix #1 and #3

;Since the color values have been calculated for pixels 1 and 3,
;pixels 0 and 2 still need to be determined.  Pixel 0 is calculated by
;(prev_pixel + pixel #1) / 2 and pixel 2 is calculated by (pixel #1 +
;pixel #3) / 2.  Output:
;      |--- 16 bit ----|
;      +-----------------------------------------------------------------+
;MM3 = |Color p0 index | Color p1 index | Color p2 index | Color p3 index|
;      +-----------------------------------------------------------------+

MOVD	    MM4, DWORD PTR mask_double_255
PACKSSDW    MM6, MM6

MOVQ	    MM7, MM6
MOVQ	    MM3, MM6

PSRLD	    MM7, 16

PUNPCKLWD   MM7, MM0

PADDW	    MM6, MM7

PSRLW	    MM6, 1

PUNPCKLWD   MM3, MM6
ADD	        EDI, 8

;Now that MM3 contains the 4 memory indexes in packed format, we need
;to unpack them in order to get the precomputed color values from the 256
;element color array.  Output:
;      |--- 16 bit ---|
;      +--------------------------------------------------------------+
;MM1 = | Color p3     | Color p2      | Color p1      | Color p0      |
;      +--------------------------------------------------------------+

;Write the 4 pixel colors to the backbuffer.
;Decrease the counter and loop back to draw four more pixels if necessary.
;The looping construct may look strange but it is done to allow for the
;calculation of the pixel colors at the end of the scan line.

;Or : divide(right shift) by the octave index and add to the prev ones

MOVD	  EBX ,MM3

PSRLW    MM3,[turbShift]

PADDW    MM3,[EDI]      
MOVQ	  [EDI], MM3	   ;Write out the 4 pix to video memory.

DEC	   ECX
JNZ	   start_scan_line

INC    ESI
;MOV  prev_color, EBX       ;EBX is the color index of pixel #3. Store it.
INC     [turbShift]  
DEC    [octShift]  

CMP  ESI, num_octaves
JNZ	   start_octave

MOV  prev_color, EBX       ;EBX is the color index of pixel #3. Store it.

;end_scan_line:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; here we rearrange the turb buffer 
;; buffer[i] = p0:p1:p2:p3 --> buffer[i] = p3:p2:p1:p0

MOV   EDI , turb_buffer
MOV	  ECX, num_pixels

flipLoop: 
MOVQ       MM5, [EDI]      

MOVQ	         MM4, MM5
PUNPCKHDQ   MM5,MM5  ; mm5 = p0:p1:p0:p1

MOVQ            MM7,MM5   ; mm7 = p0:p1:p0:p1
PSRLD            MM5,16

MOVQ	        MM6, MM4
PUNPCKLWD  MM5,MM7  ; mm5  =  *:*:p1:p0

PSRLQ           MM6,16       ; mm6 =   0:p0:p1:p2

PUNPCKLWD  MM6,MM4  ; mm6  =  *:*:p3:p2

PUNPCKLDQ   MM5, MM6 ; mm5 = p3:p2:p1:p0

MOVQ	  [EDI], MM5	   
ADD	               EDI, 8
DEC         ECX
JNZ          flipLoop
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EMMS                       ; Clear out the MMX registers and set approp flags.

RET                        ; end of function

MMX_Octave ENDP
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

END

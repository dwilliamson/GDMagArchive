#line 1 "ScatterVS10.vsh"
; Scattering Vertex shader
; (c) Preetham - ATI Research, 2002.
;
; 

vs.1.0

#line 1 "TerrainVShaderConstants.h"
























































#line 58 "TerrainVShaderConstants.h"
#line 9 "ScatterVS10.vsh"

; 
; V - View direction
; L - Sun direction
; Theta - Scattering angle
; s - Distance
; E - Total extinction (including reflectance).

; Transformation.
m4x4 oPos, v0, c[0]	; Transform position to viewport

; Texture Coordinates.
; 0 - Normal/Horizon map
; 1 - Terrain texture
; 2 - Cloud texture
mad oT0.xy, v0.xz, c[16].xy, c[16].zw
mad oT1.xy, v0.xz, c[16].xy, c[16].zw
mad oT2.xy, v0.xz, c[18].xy, c[18].zw


; Calculate V
m4x4 r1, v0, c[8]	        ; Transform position to world space
sub r1, c[22], r1	        ; V = eye - position
dp3 r1.w, r1, r1                        ; Normalize V.
rsq r1.w, r1.w							
mul r1, r1, r1.w



; Angle (theta) between sun direction (L) and view direction (V).
dp3 r0.x, r1, c[12]       ; r0.x = [cos(theta)] = V.L
mad r0.y, r0.x, r0.x, c[31].x	; r0.y = [1+cos^2(theta)] = Phase1(theta)


; Distance (s)
m4x4 r1, v0, c[4]           ; r1.z = s
mov r0.z, r1.z							; store in r0.z for future use.

; Terms used in the scattering equation.
; r0 = [cos(theta), 1+cos^2(theta), s] 

; Extinction term E

mul r1, c[24], -r0.z       ; -(beta_1+beta_2) * s
mul r1, r1, c[31].y           ; -(beta_1+beta_2) * s * log_2 e
exp r1.x, r1.x					
exp r1.y, r1.y					
exp r1.z, r1.z                          ; r1 = e^(-(beta_1 + beta_2) * s) = E1


; Apply Reflectance to E to get total net effective E
mul r3, r1, c[13]   ;r3 = E (Total extinction) 


; Phase2(theta) = (1-g^2)/(1+g-2g*cos(theta))^(3/2)
; theta is 180 - actual theta (this corrects for sign)
; c[28] = [1-g^2, 1+g, 2g]
mad r4.x, c[28].z, r0.x, c[28].y; 


rsq r4.x, r4.x						
mul r4.y, r4.x, r4.x			
mul r4.x, r4.y, r4.x;
mul r0.w, r4.x, c[28].x              ; r0.w = Phase2(theta)


; Inscattering (I) = (Beta'_1 * Phase_1(theta) + Beta'_2 * Phase_2(theta)) * 
;        [1-exp(-Beta_1*s).exp(-Beta_2*s)] / (Beta_1 + Beta_2)

mul r4, c[27], r0.y 
mul r5, c[26], r0.w  
sub r6, c[31].x, r1
mov r7, c[24]

add r4, r4, r5
mul r4, r4, r6
mul r4, r4, c[23]	; r4 = I (inscattering)


; Apply Inscattering contribution factors.

mul r4, r4, c[25].y


; Scale with Sun color & intesity.
mul r4, r4, c[15]		
mul r4, r4, c[15].w	

mul r3, r3, c[15]		
mul r3, r3, c[15].w	

; Outputs.
mov oD0, r3                             ; Extinction
mov oD1, r4                             ; Inscattering

; Cartoon-Style Vertex Blending Shader
;
; Vertex Data
; v0	vertex position
; v1	matrix weights (packed in byte form
; v2	matrix indices (packed in byte form
; v3	normal
; v4	texture coordinate (only x used)
;
; Shader Constants
; c0-3  Transformation and Projection matrix
; c4-6  Inverse Transpose Matrix used to transform normal
; c7    Light vector
; c8	( 1.0, -1.0) used for weight calc
; c9-96 Used to store the bone deformation matrices
;		3 registers per bone
; Registers used
; r0	Accumulated vertex position
; r1	temp vertex position
; r2	Accumulated normal position
; r3	temp vertex normal
; r4	weight 4 storage
;
; Declare the shader version number
vs.1.1

mov a0.x, v2.x				; get the first matrix index
dp4 r0.x, v0, c[a0.x + 0]	; multiply by matrix 1
dp4 r0.y, v0, c[a0.x + 1]
dp4 r0.z, v0, c[a0.x + 2]
mul r0.xyz,v1.x, r0			; factor weight and store in reg 0

dp4 r2.x, v3, c[a0.x + 0]	; multiply normal by matrix 1
dp4 r2.y, v3, c[a0.x + 1]
dp4 r2.z, v3, c[a0.x + 2]
mul r2.xyz,v1.x, r2			; factor weight and store

	mov a0.x, v2.y				; get the second matrix index
	dp4 r1.x, v0, c[a0.x + 0]	; multiply by matrix 2
	dp4 r1.y, v0, c[a0.x + 1]
	dp4 r1.z, v0, c[a0.x + 2]
	mad r0.xyz,v1.y, r1, r0		; multiple weight and accumulate result

	dp4 r3.x, v3, c[a0.x + 0]	; multiply normal by matrix 2
	dp4 r3.y, v3, c[a0.x + 1]
	dp4 r3.z, v3, c[a0.x + 2]
	mad r2.xyz,v1.y, r3, r2		; multiple weight and accumulate result

		mov a0.x, v2.z				; get the third matrix index
		dp4 r1.x, v0, c[a0.x + 0]	; multiply by the bone matrix
		dp4 r1.y, v0, c[a0.x + 1]
		dp4 r1.z, v0, c[a0.x + 2]
		mad r0.xyz,v1.z, r1, r0		; multiple weight and accumulate result

		dp4 r3.x, v3, c[a0.x + 0]	; multiply normal by matrix 2
		dp4 r3.y, v3, c[a0.x + 1]
		dp4 r3.z, v3, c[a0.x + 2]
		mad r2.xyz,v1.z, r3, r2		; multiple weight and accumulate result

			mov r4,v1					; store off the weights temporarily
			dp4 r4.w, r4, c[8].yyyx		; calculate w4 = (1 - (w1 + w2 + w3))

			mov a0.x, v2.w				; get the last matrix index
			dp4 r1.x, v0, c[a0.x + 0]	; multiply by the bone matrix
			dp4 r1.y, v0, c[a0.x + 1]
			dp4 r1.z, v0, c[a0.x + 2]
			mad r0.xyz,r4.w, r1, r0		; multiple weight and accumulate result

			dp4 r3.x, v3, c[a0.x + 0]	; multiply normal by matrix 2
			dp4 r3.y, v3, c[a0.x + 1]
			dp4 r3.z, v3, c[a0.x + 2]
			mad r2.xyz,r4.w, r3, r2		; multiple weight and accumulate result

			mov r0.w, v0.w				; set vertex w to initial w
			mov r2.w, v2.w				; set vertex w to initial w

dp4 oPos.x, r0, c[0]		; Transform to screen space
dp4 oPos.y, r0, c[1]
dp4 oPos.z, r0, c[2]
dp4 oPos.w, r0, c[3]

dp3 r0.x, r2, c[4]			; Transform the normal
dp3 r0.y, r2, c[5]
dp3 r0.z, r2, c[6]

; Compute the Dot product of the light and normal and 
; set the output texture coordinate
dp3 oT0.x, r0, c[7]


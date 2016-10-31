; Cartoon-Style Vertex Shader
;
; Vertex Data
; v0	vertex position
; v1	normal
; v2	texture coordinate (only x used)
;
; Shader Constants
; c0-3  Transformation and Projection matrix
; c4-7  Inverse Transpose Matrix used to transform normal
; c8    Light vector

; Declare the shader version number
vs.1.0

; Transform the vertex position
dp4 oPos.x, v0, c[0]
dp4 oPos.y, v0, c[1]
dp4 oPos.z, v0, c[2]
dp4 oPos.w, v0, c[3]

; Transform the normal
dp3 r0.x, v1, c[4]
dp3 r0.y, v1, c[5]
dp3 r0.z, v1, c[6]

; Compute the Dot product of the light and normal and 
; set the output texture coordinate
dp3 oT0.x, r0, c[8]

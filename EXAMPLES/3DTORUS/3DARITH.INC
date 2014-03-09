	.code
;Gets normal of two vectors
;SI=src1,BX=src2,DI=dest
;
; d1=y1*z2-z1*y2
; d2=z1*x2-x1*x2
; d3=x1*y2-y1*x2

vNormal:movsx eax,w z[si]
	movsx ebp,w y[bx]
	cdq
	imul ebp
	mov ecx,eax
	movsx eax,w y[si]
	movsx ebp,w z[bx]
	cdq
	imul ebp
	sub eax,ecx
	stosw
	;
	movsx eax,w x[si]
	movsx ebp,w x[bx]
	cdq
	imul ebp
	mov ecx,eax
	movsx eax,w z[si]
	cdq
	movsx ebp,w x[bx]
	imul ebp
	sub eax,ecx
	stosw
	;
	movsx eax,w y[si]
	cdq
	movsx ebp,w x[bx]
	imul ebp
	mov ecx,eax
	movsx eax,w x[si]
	cdq
	movsx ebp,w y[bx]
	imul ebp
	sub eax,ecx
	stosw
	ret

vDegree:movsx eax,w x[si]
	cdq
	imul eax
	mov ecx,eax
	movsx eax,w y[si]
	cdq
	imul eax
	add ecx,eax
	movsx eax,w z[si]
	cdq
	imul eax
	add eax,ecx
	mov esi,eax
	call vSqroot
	ret

;get square root
vSqroot:xor ebx,ebx
	mov bl,1
l1:	mov ebp,ebx	;x+1=(x+n/x)*0.5
	mov eax,esi
	cdq
	idiv ebx
	add eax,ebx
	shr eax,1
	inc eax
	mov ebx,eax
	mov eax,ebp
	sub eax,ebx
	cmp eax,1	;x=oldx?
	ja  l1		;nope, repeat
	ret
	
;Get scalar product of two vectors
vScalar:movsx eax,w x[si]
	cdq
	imul d x[bx]
	mov ecx,eax
	movsx eax,w y[si]
	cdq
	imul d y[bx]
	add ecx,eax
	movsx eax,w z[si]
	cdq
	imul d z[bx]
	add eax,ecx
	ret

vDelta: mov ax,x[si]
	sub ax,x[bx]
	stosw
	mov ax,y[si]
	sub ax,y[bx]
	stosw
	mov ax,z[si]
	sub ax,z[bx]
	stosw
	ret

; Get brightness of plane
vLight: push esi,edi
	mov edi,tmp_delta1
	call vDelta
	pop ebx,esi
	mov edi,tmp_delta2
	call vDelta

	mov esi,tmp_delta1
	mov ebx,tmp_delta2
	mov edi,tmp_normal
	push edi
	;push edi
	call vNormal

	pop esi
	call vDegree
	mov tmp_nbetr,ebx

;	 pop esi
;	 mov ebx,vec_light
;	 call vScalar

;	 mov ebx,deg_light
;	 cdq
;	 idiv ebx
;	 shl eax,8
;	 idiv d tmp_nbetr
	ret

;Initializes light position
init_light:
	mov esi,vec_light
	call vDegree
	mov deg_light,ebx
	ret

vec_light:	dw 0,0,-100	;Light vector
	
	.data
deg_light:	dd ?		;is updated by init_light
tmp_delta1:	dw ?,?,?
tmp_delta2:	dw ?,?,?
tmp_normal:	dw ?,?,?
tmp_nbetr	dd ?

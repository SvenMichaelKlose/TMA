; 3D rotation (c)1997 Ba'al shem

	.code

screen_width=320
screen_height=200

; SI	Source pointstruc
; DI	Dest
;    Yt = Y*COS(Xan) - Z*SIN(Xan)
;    Zt = Y*SIN(Xan) + Z*COS(Xan)
;rotate_x:
rotate: push si
	mov ecx,angle_x
	db 66h,0fh,0bfh,4
;	 movsx eax,w x[si]
	mov rot_x,eax
	;
       db 66h,0fh,0bfh,44h,2
;	 movsx eax,w y[si]
	push eax
	mov ebx,cos[ecx]
	cdq
	imul ebx
	mov ebp,eax
	db 66h,0fh,0bfh,44h,4
;	 movsx eax,w z[si]
	mov esi,eax
	mov ebx,sin[ecx]
	cdq
	imul ebx
	neg eax
	add eax,ebp
	sar eax,16
	mov rot_y,eax
	;
	pop eax
	mov ebx,sin[ecx]
	cdq
	imul ebx
	mov ebp,eax
	mov eax,esi
	mov ebx,cos[ecx]
	cdq
	imul ebx
	add eax,ebp
	sar eax,16
	mov rot_z,eax

;    Xt  = X*COS(Yan) - Z*SIN(Yan)
;    Zt  = X*SIN(Yan) + Z*COS(Yan)
;rotate_y:
	mov ecx,angle_y
	mov eax,rot_x
	push eax
	mov ebx,cos[ecx]
	cdq
	imul ebx
	mov ebp,eax
	mov eax,rot_z
	mov esi,eax
	mov ebx,sin[ecx]
	cdq
	imul ebx
	neg eax
	add eax,ebp
	sar eax,16
	mov rot_x,eax
	;
	pop eax
	mov ebx,sin[ecx]
	cdq
	imul ebx
	mov ebp,eax
	mov eax,esi
	mov ebx,cos[ecx]
	cdq
	imul ebx
	add eax,ebp
	sar eax,16
	mov rot_z,eax

;    Xt  = X*COS(Zan) - Y*SIN(Zan)
;    Yt  = X*SIN(Zan) + Y*COS(Zan)
rotate_z:
	mov ecx,angle_z
	mov eax,rot_x
	push eax
	mov ebx,cos[ecx]
	cdq
	imul ebx
	mov ebp,eax
	mov eax,rot_y
	mov esi,eax
	mov ebx,sin[ecx]
	cdq
	imul ebx
	neg eax
	add eax,ebp
	sar eax,16
	shl eax,7
	mov ebx,rot_z
	add ebx,trans_z
	cdq
	or ebx,ebx
	if nz idiv ebx
	add ax,trans_x
	add ax,screen_width/2
	stosw
	;
	pop eax
	mov ebx,sin[ecx]
	cdq
	imul ebx
	mov ebp,eax
	mov eax,esi
	mov ebx,cos[ecx]
	cdq
	imul ebx
	add eax,ebp
	sar eax,16
	shl eax,7
	mov ebx,rot_z
	add ebx,trans_z
	cdq
	or ebx,ebx
	if nz idiv ebx
	add ax,screen_height/2
	stosw
	
	mov ax,rot_z
	stosw
	xor ax,ax
	stosw
	pop si
	add si,pointsize
	ret

	.data
angle_x dd ?
angle_y dd ?
angle_z dd ?

rot_x	dd ?
rot_y	dd ?
rot_z	dd ?

trans_x dd ?
trans_y dd ?
trans_z dd ?


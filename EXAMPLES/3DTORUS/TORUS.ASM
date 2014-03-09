; 3D torus
; (c)1997 S.Klose

	.code
torus:	jmp start

	include"3dmain.inc"

	.code
start:	mov ax,13h
	int 10h
	cld
	mov d planelimit,8
	mov ax,cs
	add ax,1000h
;mov ax,0a000h
	mov scrbuffer,ax
	call init_sine
	;
;	 call setnewint
	mov d trans_x,0
	mov d trans_y,0
	mov d trans_z,30128
	mov cx,1500
l1:	push cx
	xor eax,eax
	mov al,cl ;b[rotation]
	shl ax,2
	and ax,1111111100b
	mov angle_x,eax
	shl eax,1
	and eax,1020
	mov angle_y,eax
	mov angle_z,eax
	call retrace
	mov es,w[scrbuffer]
	mov cx,16000
	mov eax,0f0f0f0fh
	xor di,di
	rep stosd
	call draw_world
	mov es,0a000h
	push ds
	mov ds,[scrbuffer]
	mov cx,16000
	xor si,si
	xor di,di
	rep movsd
	pop ds
	pop cx
	cmp d trans_z,tzoom
	if a sub d trans_z,70
	cmp d trans_z,tzoom
	if l mov d trans_z,tzoom
	add d planelimit,1
	mov ah,1
	int 16h
	jnz >e1
	dec cx
	jnz l1
;	 call restoreint
e1:	mov ax,3
	int 10h
	mov dx,bye
	mov ah,9
	int 21h
	ret

newint1ch:
	dec w cs:rotation
	iret

setnewint:
	push es
	mov es,0
	mov ax,cs
	shl eax,16
	mov ax,newint1ch
	xchg eax,[es:1ch*4]
	mov oldint1ch,eax
	pop es
	ret

restoreint:
	push es
	mov es,0
	mov eax,[oldint1ch]
	mov [es:1ch*4],eax
	pop es
	ret

; ---------------------------------------------------------------------------

	.code

;pointlist:  
	dw 24
	dw -500,-500,500
	dw 500,-500,500
	dw -500,500,500
	dw 500,500,500
	
	dw -500,-500,-500
	dw 500,-500,-500
	dw -500,500,-500
	dw 500,500,-500

	dw 500,-500,-500
	dw 500,500,-500
	dw 500,-500,500
	dw 500,500,500

	dw -500,-500,-500
	dw -500,500,-500
	dw -500,-500,500
	dw -500,500,500

	dw -500,500,-500
	dw 500,500,-500
	dw -500,500,500
	dw 500,500,500

	dw -500,-500,-500
	dw 500,-500,-500
	dw -500,-500,500
	dw 500,-500,500

;planelist:
	dw 12
	dw 0,1,2,1,0
	dw 1,2,3,2,0

	dw 4,5,6,3,0
	dw 5,6,7,4,0

	dw 8,9,10,5,0
	dw 9,10,11,6,0

	dw 12,13,14,7,0
	dw 13,14,15,8,0

	dw 16,17,18,9,0
	dw 17,18,19,10,0

	dw 20,21,22,11,0
	dw 21,22,23,12,0

	include"torus.vec"

bye:	db"This file is a part of the TMA macro assembler.",10,13,"$"
	
oldint1ch: dd ?
rotation: dw ?


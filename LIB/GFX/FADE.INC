;Color fading
	.code
SetVideoMode:
	pusha
	push es,ds
	mov ax,data
	mov ds,ax
	mov es,ax
	mov ax,768
	mov Fades,ax
	call retrace
	call GetColors
  
	call ClearNewColors
	call retrace
	call fullfade

	mov ax,19	;VGA 320*200*256
	int 10h
  
	mov ax,768
	mov w[Fades],ax
  
	call retrace
	mov di,OldColors
	mov cx,[Fades]
	xor al,al
	rep stosb
  
	pop ds,es
	popa
	ret

Clearoldcolors:
        mov ax,data
	mov ds,ax
	mov es,ax
	mov di,OldColors
	mov cx,w[Fades]
	xor al,al
	rep stosb
	ret

ClearNewColors:        
	push ax,cx,di,es,ds
	mov ax,data
	mov ds,ax
	mov es,ax
	mov di,NewColors
	mov cx,w[Fades]
	xor al,al
	rep stosb
	pop ds,es,di,cx,ax
	ret

BlankScreen:
	push ax,cx,dx
	mov dx,3c8h
	xor al,al
	mov cx,100
	out dx,al
	inc dl
svm3:	out dx,al
	out dx,al
	out dx,al
	dec cx
	jnz svm3
	pop dx,cx,ax
	ret

fullfade:	 
	pusha
	mov ds,data
	mov b fadespctrl,0
	mov bx,64
	mov al,FadeSpeed
	xor ah,ah
	xor dx,dx
	mul bx
	mov cx,ax

svm2:	call Retrace
	call Fade
	loop svm2
	popa
	ret

Fade:	pusha
	pushf
	push ds,es
	mov ax,data
	mov ds,ax
	mov es,ax
	cld
	inc b FadeSpCtrl
	mov al,FadeSpCtrl
	cmp al,FadeSpeed
	jne FadeBye
	xor al,al
	mov FadeSpCtrl,al
	mov si,OldColors
	push si
	mov dx,3c8h	      ;1.Farbwert angeben
	xor al,al
	out dx,al
	inc dx	
	mov cx,Fades
	rep outsb
	pop si
	mov di,si
	mov bx,NewColors

	mov cx,Fades
FadeLoop:
	lodsb
	mov dl,[bx]
	cmp al,dl
	je NoFading
	cmp al,dl
	jb fadein
	dec al
	jmp nofading
Fadein: inc al
NoFading:
	stosb
	inc bx
	dec cx
	jnz FadeLoop

FadeBye:pop es,ds
	popf
	popa
	ret

GetColors:
	push cx,dx,di
	mov ax,data
	mov ds,ax
	mov es,ax
	mov di,Oldcolors
	mov dx,3c7h
	xor al,al
	out dx,al
	add dl,2
	mov cx,fades
	call Retrace
	rep insb
	pop di,dx,cx
	ret

	.data
FadeSpeed	db ?	;Geschwindigkeit
FadeSpCtrl	db ?	;(intern)
Fades		dw ?	; Anzahl der Farben

OldColors:	db 768 dup ?
NewColors:	db 768 dup ?
;(c)1995 S.Klose

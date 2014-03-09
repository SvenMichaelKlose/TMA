; ********************
; *** Faderoutinen ***
; ********************
	.code
SetVideoMode:
	pusha
	push es,ds,cs,cs
	pop ds,es
;	 mov w[Fades],16*3
	call retrace
	call Getcolors
  
	call ClearNewCols
	call retrace	      ;Schnee bei Tastendruck verhindern
	call fullfade

	mov ax,19	;VGA 320*200*256
	int 10h
  
	mov w[Fades],768
  
	call retrace
	mov di,Oldcolors
	mov cx,w[Fades]
	xor al,al
	rep stosb
  
	pop ds,es
	popa
	ret

ClearNewCols:        
	push ax,cx,di,es
	push cs
	pop es
	mov di,Newcolors
	mov cx,[cs:Fades]
	xor al,al
	rep stosb
	pop es,di,cx,ax
	ret

BlankScreen:
	push ax,cx,dx
	mov cx,100
svm3:   mov dx,3c8h
	mov al,cl
	dec al
	out dx,al
	inc dx
	xor al,al
	out dx,al
	out dx,al
	out dx,al
	loop svm3
	pop dx,cx,ax
	ret

fullfade:        
	pusha
	mov bx,64
	mov al,[cs:FadeSpeed]
	xor ah,ah
	xor dx,dx
	mul bx
	mov cx,ax

svm2:   call Retrace
	call Fade
	loop svm2
	popa
	ret

FadeSpeed:db 1          ;Geschwindigkeit
FadeSpCtrl:db 0         ;(intern)
Fades:dw 256            ; Anzahl der Farben

Fade:   pusha
	pushf
	push ds,es,cs,cs
	pop ds,es
	cld
	inc b[FadeSpCtrl]
	mov al,[FadeSpCtrl]
	cmp al,[FadeSpeed]
	jne FadeBye
	mov b[FadeSpCtrl],0
	mov si,Oldcolors
	push si
	mov dx,3c8h           ;1.Farbwert angeben
	xor al,al
	out dx,al
	inc dx  
	mov cx,[Fades]
	rep outsb
	pop si
	mov di,si
	mov bx,Newcolors

	mov cx,[Fades]
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
	loop FadeLoop

FadeBye:pop es,ds
	popf
	popa
	ret

Getcolors:
	push cx,dx,di
	push cs
	pop es
	mov di,Oldcolors
	mov dx,3c7h
	xor al,al
	out dx,al
	add dl,2
	mov cx,[cs:fades]
	rep insb
	pop di,dx,cx
	ret

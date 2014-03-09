; ************************************************
; *** Zeichnet Kreis mit Bresenham-Algorithmus ***
; ************************************************
; ES=Zielsegment (256*256) Punkte
; RADIUS is klar
; POSX & POY = Zentrum
	.code
DrawCircle:
; ----- Initialisierung
	mov ds,data
	mov ax,[radius] 	;y=radius
	mov di,ax		;y
	xor si,si		;x
	mov bp,3		;d=3-2*radius
	add ax,ax
	sub bp,ax
; ----- Hauptteil
dc3:	call dcpoint
	mov ax,si
	cmp ax,di
	jge dcEnd
	test bp,32*1024
	jz dc6
	mov ax,si		;d=4x+6
	shl ax,2
	add ax,6
	add bp,ax
	inc si
	jmp dc3
dc6:	mov ax,si
	sub ax,di
	shl ax,2
	add ax,10
	add bp,ax
	dec di
	inc si
	jmp dc3

; ----- Punkte in den Quadranten zeichnen
dcEnd:	mov ax,si
	cmp ax,di
	jne dcEnd2
	call dcpoint
dcend2: ret

dcpoint:mov bx,posx
	mov cx,si
	shr cx,1
	mov dx,di
	shr dx,1
	mov ax,bx
	add ax,cx
	mov xxcp,ax
	mov ax,bx
	sub ax,cx
	mov xxcm,ax
	mov ax,bx
	add ax,dx
	mov xycp,ax
	mov ax,bx
	sub ax,dx
	mov xycm,ax
	mov bx,posy
	mov ax,bx
	add ax,cx
	mov yxcp,ax
	mov ax,bx
	sub ax,cx
	mov yxcm,ax
	mov ax,bx
	add ax,dx
	mov yycp,ax
	mov ax,bx
	sub ax,dx
	mov yycm,ax
	mov al,[CircleCol]
	mov bl,xxcp
	call wp
	mov bl,xxcm
	call wp
	mov bl,xycp
	call wp2
	mov bl,xycm
	call wp2
	ret

wp:	mov bh,yycp
	mov b[es:bx],al
	mov bh,yycm
	mov b[es:bx],al
	ret
wp2:	mov bh,yxcp
	mov b[es:bx],al
	mov bh,yxcm
	mov b[es:bx],al
	ret

	.data
	align 4
Radius: dw ?
PosX	dw ?
PosY	dw ?
xxcp	dw ?
xycp	dw ?
xxcm	dw ?
xycm	dw ?
yxcp	dw ?
yycp	dw ?
yxcm	dw ?
yycm	dw ?


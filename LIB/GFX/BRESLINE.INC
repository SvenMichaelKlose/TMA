; ***********************************************
; *** Zeichnet Linie nach Bresenham-Verfahren ***
; ***********************************************
; AX/BX=Quellposition
; CX/DX=Zielposition
	.code
DrawLine:
	mov x1,ax
	mov y1,bx
	mov x2,cx
	mov y2,dx
	;
	mov ax,x2	;ax=xdif
	sub ax,x1
	jns dl1
	neg ax
	;
dl1:	mov bx,y2	;bx=ydif
	sub bx,y1
	jns dl2
	neg bx
	;
dl2:	cmp ax,bx
	jl dl20
	;
	mov cx,x1	;xdif>=ydif
	cmp cx,x2	;x-Richtung nach CX
	jg dl4
	mov cx,1
	jmp dl5
dl4:	mov cx,-1
	;
dl5:	mov dx,y1	;y-Richtung nach DX
	cmp dx,y2
	jg dl6
	mov dx,1
	jmp dl7
dl6:	mov dx,-1
	;
dl7:	mov s1,cx	;steps speichern
	mov s2,dx
	add bx,bx	;doppelte H”he nach s3
	mov s3,bx
	;
	sub bx,ax	;ydif*2-xdif nach bx
	mov cx,bx
	sub cx,ax	;s4=ydif*2+xdif*2
	mov s4,cx
	;
	mov cx,x1	;Anfangspunkt zeichnen
	mov dx,y1
	call Point
	;
dl8:	cmp cx,x2	;x=Zielposition?
	jz dl3		;Ja, Ende.
	add cx,s1	;X einen Schritt weiter
	or bx,bx	;bx negativ?
	jns dl10
	add bx,s3	;Nein, bx=+ydif*2
	jmp dl11
dl10:	add bx,s4	;Ja, bx=+ydif*2-xdif*2
	add dx,s2	;Einen Schritt weiter auf der Y-Achse
dl11:	call point	;Punkt zeichnen
	jmp dl8

dl20:	mov cx,y1
	cmp cx,y2
	jg dl12
	mov cx,1
	jmp dl13

dl12:	mov cx,-1

dl13:	mov dx,x1
	cmp dx,x2
	jg dl14
	mov dx,1
	jmp dl15
dl14:	mov dx,-1
dl15:	mov s1,cx
	mov s2,dx
	add ax,ax
	mov s3,ax
	sub ax,bx
	mov cx,ax
	sub cx,bx
	mov s4,cx
	mov bx,ax
	mov cx,x1
	mov dx,y1
	call point
dl16:	cmp dx,y2
	jz dl3
	add dx,s1
	or bx,bx
	jns dl18
	add bx,s3
	jmp dl19
dl18:	add bx,s4
	add cx,s2
dl19:	call point
	jmp dl16

dl3:	ret

	.data
x1	dw ?
x2	dw ?
y1	dw ?
y2	dw ?
s1	dw ?
s2	dw ?
s3	dw ?
s4	dw ?


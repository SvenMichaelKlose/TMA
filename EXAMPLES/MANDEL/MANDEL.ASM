; Mandelbrot found in the FIDO-Net. Very nice.

start:	mov ax,13h
	int 10h 	; set 320x200 256 color mode
	mov ax,0a000h
	mov ds,ax	; load ds with video segment
	xor di,di	; zero out screen offset
	mov bp,-200	; load y with -(screen height)
l1:	mov si,-320	; load x with -(screen width)
l2:	push di 	; save screen offset
	xor bx,bx	; val1 = 0
	xor di,di	; val2 = 0
l3:	push bx
	lea ax,[bx+di]	; ax = val1 + val2
	sub bx,di	; bx = val1 - val2
	imul bx 	; ans = val1^2 - val2^2
	mov bl,ah
	mov bh,dl
	lea bx,[bx+si-64]   ; val1 = (ans/256) + x - 64
	pop ax
	imul di 	; ans = tval1 * val2
	mov al,ah
	mov ah,dl
	add ax,ax
	xchg ax,di
	lea di,[bp+di-56] ; val2 = (ans/128) + y - 56
	cmp bh,4
	jg draw 	; if val1 > 1024 then draw point
	inc cl		; increment color
	jne l3
draw:	pop di		; restore screen offset
	xchg [di],cl	; store color, and make color = 0
	inc di		; increment screen offset
	inc si		; increment x
	jne l2		; if x <> 0 then continue lp2
	inc bp		; increment y
	jne l1		; if y <> 0 then continue lp1
	xor ax,ax
	int 16h
	mov ax,3	
	int 10h 	; set text mode
	ret		; exit program

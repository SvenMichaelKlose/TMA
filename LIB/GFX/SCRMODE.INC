	.code
;Switch to Mode X
SetScrollMode:
	mov ax,11
	int 10h
	mov ax,13h	;320*200
	int 10h
	mov dx,03c4h	;chain-4-adr off
	mov al,4
	out dx,al
	inc dx
	in al,dx
	and al,(not 8)
	out dx,al
	mov dx,3d4h	;double word off
	push dx
	mov al,14h
	out dx,al
	inc dx
	in al,dx
	and al,(not 40h)
	out dx,al
	dec dx		;byte mode on
	mov al,17h
	out dx,al
	inc dx
	in al,dx
	or al,40h
	out dx,al
	pop dx		;width
	mov al,13h
	out dx,al
	inc dx
	mov al,640/8
	out dx,al

	mov dx,3ceh	;Bildschirm l”schen
	mov ax,0f00h	;Alle Ebenen aktivieren
	out dx,ax
	dec cx
	inc al
	out dx,al
	mov ax,0a000h
	mov es,ax
	xor di,di
	xor eax,eax
	mov cx,4000h
	rep stosd
	ret


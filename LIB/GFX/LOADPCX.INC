; *************************************
; *** LÑdt PCX-Bild und Farbpalette ***
; *************************************
; ES = Zielsegment
; DS:DX=Name der Datei, Farben kommen nach NewColours
	.code
LoadPCXfileError:
	stc
	ret

LoadPCXimage:
	push es,ds
	mov ds,data
	mov ax,3d00h	  ;Datei îffnen
	int 21h
	jc LoadPCXFileError
	mov bx,ax
	push bx
  
	mov ah,3fh		;PCX-Header laden
	mov cx,128
	mov dx,PCXheader
	int 21h
	jc LoadPCXfileError

	mov ax,w[PCXheader+8]	;Breite der Graphik holen
	sub ax,w[PCXheader+5]
	inc ax
	mov w[PCXwidth],ax
  
	mov ax,w[PCXheader+10]	;Hoehe der Graphik holen
	sub ax,w[PCXheader+6]
	inc ax
	mov w[PCXheight],ax

	xor di,di
LoadPCXnb:
	mov ah,3fh		;Graphikdaten holen
        mov cx,-1
	mov ds,data
	mov ds,[LoadSeg]      ;Zielsegment Åbergeben
	xor dx,dx
	int 21h
	jc LoadPCXfileError
	inc ax
	mov w[cs:LoadSize],ax
	xor si,si
	xor di,di
	mov cx,w[cs:PCXheight]
l1:	push cx
	mov bx,w[cs:PCXwidth]	;BX ist BreitenzÑhler
PCXwloop:
	lodsb			;Farbkette ?
	cmp al,0c0h
	jb PCXnoString		;Nein,weiter ...
	sub al,0c0h
	mov cl,al
	xor ch,ch
	sub bx,cx
	lodsb
	rep stosb		;Farbkette schreiben
	jmp PCXwloop2		;Weiter ...
PCXnoString:
	stosb			;Farbwert schreiben
	dec bx
PCXwloop2:
	or bx,bx		;Ende der Zeile ?
	ja PCXwLoop		;Nein,weiter
	pop cx			;NÑchste Zeile ...	  
	loop l1

; ----- Farbtabelle ab DS:SI
PCX3:	lodsb			;12 ueberspringen
	cmp al,0ch
	jne PCX3
  
	mov cx,256		;256 Farben setzen
	mov di,NewColors
	mov es,cs
LoPCXcol2:
	lodsw
	shr al,2
	shr ah,2
	stosw
	lodsb
	shr al,2
	stosb
	loop LoPCXcol2
  
	pop bx
	mov ah,3eh		;Datei schliessen
	int 21h
	jc LoadPCXfileError2
	pop ds,es
	clc
	ret

LoadPCXfileError2:
	jmp LoadPCXfileError

	.data
PCXheader:db 1024 dup ?
PCXheight:dw ?
PCXwidth:dw ?
LoadSize:dw ?

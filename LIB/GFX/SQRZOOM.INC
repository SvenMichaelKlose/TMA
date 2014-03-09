	code segment

; ====================
; >>> Scale bitmap <<<
; ====================
  ; In:
  ; AX destination X
  ; BX destination Y

  ; ES source segment
  ; BP Destination width, height will be assumed automatically.
  ; DI Heigt of source image; it must be 320 pixels wide.
DrawSquare:
	push ds,es
	xor si,si
	push di 	      ;Hîhe retten
	mov bh,bl	      ;Position der FlÑche auf dem Bildschirm
	xor bl,bl
	mov di,bx
	shr bx,2
	add di,bx
	add di,ax

	mov w[cs:sqrhelp],bp

	mov ax,320 shl 7      ;Senkrechte Dehnung nach BP
	xor dx,dx
	div bp		      ;320/BP
	mov bp,ax
	xor dx,dx	      ;DX ist ZÑhler
	mov w[DrawCounter],dx
	pop bx		      ;Hîhe wiederholen

	push 0a000h
	push es     ;Datenzeiger auf Bildschirm
	pop ds
	pop es
DrSQR1:
	push si,di,bx
	mov bx,w[cs:sqrhelp]  ;Adresse der Einsprungtabelle berechnen
	shl bx,2
DrawSQRJump:
	cli
	call far cs:[squarejumptab+bx] ;Linie zeichnen
	sti
	pop bx,di,si
	add di,320
	add dx,bp	      ;1 Zeile weiter springen ?
	add w[cs:DrawCounter],bp
DrSQR2:
	cmp dx,80h
	jb DrSQRmagnify
	add si,320	      ;nÑchste Zeile
	sub dx,80h
	jmp DrSqr2
DrSQRmagnify:
	mov ax,w[cs:DrawCounter]
	shr ax,7
	cmp ax,bx	      ;Fertig ?
	jb DrSQR1	      ;Nein,nÑchste Zeile ...
	pop es,ds
	ret

DrawCounter:dw ?
sqrhelp:dw ?

; =================================
; >>> Precalculate line zoomers <<<
; =================================
; x86 instruction opcodes
CM_retf = 0cbh		 ;RETF
CM_add_si =0c683h
CM_add_si_word = 0c681h
CM_movsb = 0a4h
CM_lodsb = 0ach
CM_stosb = 0aah        ;26h=es:
cm_inc_si = 046h

CreateSquareTab:
	mov bp,1	      ;ZÑhler fÅr Linienbreite
	mov ax,w[NewSeg]      ;Zeiger auf neue Routinen
	mov es,ax
	xor di,di	      ;Zeiger auf Routinenbuffer (Anfang)
	mov bx,offset SquareJumpTab

SQUentry:
	mov ax,di	      ;Eintrag in Sprungtabelle
	mov word ptr[bx],ax
	push es
	pop ax
	mov word ptr[bx+2],ax
	add bx,4

	mov ax,320 shl 7	;Dehnungswert berechnen
	push bx 		;1/128 Genauigkeit
	xor dx,dx
	mov bx,bp
	div bx
	pop bx
	mov dx,ax	      ;in DX speichern
 
	xor ax,ax
	mov w[SquareCounter],ax ;PunktezÑhler
	xor cx,cx	      ;CX als DehnungszÑhler

SQUdecide:		;Sprungwert berechnen
	add w[SquareCounter],dx
	add cx,dx
 
SQUloop1:
	cmp cx,80h            ;Auf "Vorkommastelle" prÅfen
	ja SQUsmall           ;Linie verkleinern
	je SQUmove
	mov al,CM_lodsb       ;Line vergrîssern
	stosb
	mov al,CM_stosb
	stosb
SQUloop2:
	add w[SquareCounter],dx
	add cx,dx
	cmp cx,80h
	jae SQUloop1
	stosb
	jmp SQUloop2

SQUmove:
	xor cx,cx	      ;ZÑhler zurÅcksetzen
SQUmove2:        
	mov al,CM_movsb       ;normale Verschiebung
	stosb
	jmp SQUcompsi         ;weiter

SQUsmall:
	mov ax,cx             ;Anzahl der BytesprÅnge
	shr ax,7
	dec ax                ;Wert korrigieren
	and cx,1111111b       ;CX auf <1 setzen
	cmp al,1              ;inc si ?
	je SQUinc_si          ;ja,springen
	test al,128           ;wenn Sprungwert>127, dann Word
	jnz SQUWordAdd
	push ax
	mov ax,CM_add_si
	stosw
	pop ax
	stosb
	jmp SQUmove2
SQUWordAdd:
	push ax
	mov ax,CM_add_si_word
	stosw
	pop ax
	stosw
	jmp SQUmove2
SQUinc_SI:
	mov ax,CM_inc_si
	stosb
	jmp SQUmove2
SQUcompSI:
	mov ax,di
	mov al,ah
	xor ah,ah
	cmp ax,0f0h
	jg SQUnewES
	mov ax,320            ;PrÅfen, ob Zeile fertig ist.
	push bx
	mov bx,w[SquareCounter]
	shr bx,7
	cmp bx,ax
	jb SQUnoNewLine
	pop bx
	inc bp
	mov al,CM_retf
	stosb
	cmp bp,321
	je SQUend
	jmp SQUentry
SQUnoNewLine:
	pop bx
	jmp SQUdecide
SQUnewES:
	sub bx,4
	push es
	pop ax
	add ax,1000h
	push ax
	pop es
	xor di,di
	jmp SQUentry

SQUend:
	mov bx,di       ;Ende des Segments
	shr bx,4        ;/16
	mov ax,es
	add ax,bx
	inc ax
	mov w[NewSeg],ax
	ret
	code ends

	data segment
SquareCounter:
	dw 0,0
SurfaceSeg:dw 0
SquareJumpTab:dd 320 dup ?
	data ends

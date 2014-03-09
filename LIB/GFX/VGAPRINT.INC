;Hier ist die Printroutine aus der alten ANIMED-Version.
;Ein kleiner Vergleich mit TP kuriert den „rgsten Zweifler:
	.code
; Gibt Text im VGA-Mode 13h aus.
; Zeichensatz muss vorher geladen sein.

PrintText:
p3:	lodsb			;Zeichen holen ?
	or al,al		;Ende des Strings 0
	jz >e1			;Ja, Ende
	cmp al,1		;N„chste Zeile ?
	jne >n1 		;Nein, Zeichen ausgeben
	mov b[cs:Cursor_x],0	;Anfang
	inc b[cs:Cursor_y]	;der n„chsten Zeile
	jmp p3
	;
n1:	call PrintChar
	jmp p3
	;
e1:	pop ds
	ret

PrintChar:
	pusha			;Erstmal alles retten
	push ds,es		;Segmente retten
	push ax
; ----- Bildschirmadresse brerechnen
PrintPlane2:
	mov al,b[cs:Cursor_Y]	;Zieladresse des Buchstabens berechnen
	xor ah,ah
	shl ax,9
	mov di,ax
	shl ax,2
	add di,ax
	mov al,b[cs:Cursor_X]
	xor ah,ah
	shl ax,3
	add di,ax
	pop ax			;Adresse des Buchstabens im Zeichensatz
	xor ah,ah
	shl ax,3
	add ax,w[cs:CharSetAdr]
	mov bp,ax
	mov bx,w[cs:PrintColor] ;Low-Byte=Vordergrundfarbe
				;High-Byte=Hintergrundfarbe
	;
	mov ax,w[cs:CharSetSeg] ;DS mit Segment der
	mov ds,ax
	mov ax,0a000h
	mov es,ax
	;
	mov dh,8		;8 Zeilen ausgeben
p2:	mov cx,8		;8 Spalten schreiben
	mov al,[ds:bp]		;Zeile aus Zeichensatz holen
p3:	mov dl,bh
	shl al,1		;Ist das Bit gesetzt ?
	jnc >p4 		;Nein, Hintergrundfarbe benutzen
	mov dl,bl		;Ja,Vordergrundfarbe benutzen
p4:	mov b[es:di],dl
	inc di			;Zeiger auf n„chsten Bildschirmpunkt
        dec cx
	jnz p3
	inc bp			;Zeiger auf n„chste Zeile
	add di,320-8		;(auch im Bildschirm)
	dec dh
	jnz p2			;Weiter bis alle Zeilen voll sind
	inc b[cs:Cursor_x]
	pop es,ds
	popa
	ret


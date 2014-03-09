; This function write a characters to a SVGA screen (256 colors) and should
; way in any VESA mode. There's no background color, those pixels won't be
; drawn.
SVGA_PrintChar:
        pusha
        push ds,es
        push ax
; ----- Bildschirmadresse brerechnen
        mov ax,cs
        mov ds,ax
        mov ax,w[Cursor_y]
        shl ax,5
        mov di,ax
        shl ax,2
        add di,ax
        mov ax,w[Cursor_x]
        mov bh,al               ;Aktuelle Banknummer
        shr ax,2
        add di,ax
	pop ax
        xor ah,ah
        shl ax,3
        add ax,w[CharSetAdr]
	mov bp,ax
        mov bl,b[cs:PrintColor]
	;
        mov ax,w[CharSetSeg]
        mov ds,ax
        mov ax,0a000h
        mov es,ax
        ;
        mov cx,4                ;4 Planes
        mov dx,3c4h             ;plane setzen
p3:     push cx,bp,di
        push cx
        and bh,3
        mov ax,102h
        mov cl,bh
        shl ah,cl
        out dx,ax
        pop ax
        mov cl,4
	sub cl,al
        mov ch,8                ;8 Zeilen ausgeben
p2:     mov al,[ds:bp]          ;Zeile aus Zeichensatz holen
        shl al,cl
	test al,128		;Ist das Bit gesetzt ?
        jz >b1                  ;Nein, weiter
	mov b[es:di],bl
        ;
b1:     inc di
        test al,8               ;Ist das Bit gesetzt ?
        jz >b1                  ;Nein, weiter
        mov b[es:di],bl
        ;
b1:     inc bp                  ;Zeiger auf n„chste Zeile
        add di,160-1
        dec ch
        jnz p2
        pop di,bp,cx
        inc bh
        test bh,4
        jz >b1
        inc di
b1:     dec cx
	jnz p3
        add w[cs:Cursor_x],8
        pop es,ds
        popa
        ret

;(c)1996 S.Klose

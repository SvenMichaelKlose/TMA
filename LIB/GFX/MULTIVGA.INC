	.code
; Create conversion table for mixing
; In:
; ES = 64k segment for conversion table
scr256_create_convtab:        
        xor di,di
        mov cx,100h     ;Farbumwandlungstabelle berechnen
l1:     push cx
        xor ah,ah
        sub ah,cl
        mov cx,100h
l2:     push ax
	xor al,al
        sub al,cl
        mov bl,al       ;Grundfarben erniedrigen (alte Farbe)
        mov bh,al
        shr al,6        ;rot
        shr bl,3        ;grÅn
        and bl,7
        and bh,7        ;blau
        mov dl,ah       ;neue Farbwerte aufteilen
        mov dh,ah
        and dh,7        ;blau
        shr ah,6        ;rot
	shr dl,3	;gruen
        and dl,7
        add al,ah
        add bl,dl
        add bh,dh
        shr al,1        ;und den Durchschnitt berechnen
        shr bl,1
        shr bh,1
        shl al,6        ;Grundfarben wieder zusammenfÅgen
        shl bl,3
	or al,bl
	or al,bh
        stosb
        pop ax
        loop l2
        pop cx
        loop l1
        ret

; Converts indexed bitmap to multicolor screen
; In:
; DS:BP = Color table of currently used picture
; FS    = Segment of indexed bitmap
; ES	= Destination segment (same size/may be equal to FS)
std256_convbmp:
        xor si,si
        xor di,di
        mov cx,0ffffh
l1:     fs:lodsb        ;Farbwert holen
        xor ah,ah       ;und seine Adresse in der Farbpalette berechnen
        mov bx,ax
        add ax,ax
        add bx,ax
	add bx,bp
	mov al,b[bx]	;rot
        shl al,2
        and al,11000000b
        mov dl,b[bx+1]  ;grÅn
        and dl,111000b
        or al,dl
        mov dl,b[bx+2]  ;blau wie ich
        shr dl,3
        or al,dl
        stosb           ;und neuen Farbwert retour
        loop l1
        ret

; Create color palette for multicolor screen.
; In:
; ES:DI = Memory for new palette
std256_create_colors:        
        mov cx,100h
l1:     xor al,al
        sub al,cl
        mov ah,al
        and al,11000000b
	shr al,2
	mov dl,al
        shl dl,2
        or al,dl
        shl dl,2
        or al,dl
l2:     stosb
        mov al,ah
        and al,111000b
        mov dl,al
        shr dl,3
        or al,dl
l2:     stosb
	mov al,ah
        and al,111b
        mov dl,al
        shl al,3
        or al,dl
l2:     stosb
        loop l1
        ret

        .data
scr256colorseg  dw ?


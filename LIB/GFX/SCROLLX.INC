; Mode X scrolling
; (c) 1995 Sven Klose

	.code
; ===================
; >>> Scroll left <<<
; ===================
scrollx_left:
	pusha
	mov ds,data
	dec w Scrollx_OffsetX
	mov ax,Scrollx_OffsetX ;Bildschirmadresse
	push ax,ax    ;Write enable register setzen
	mov cl,al
	and cl,1111b
	and cl,3
	mov ah,1
	shl ah,cl
	mov al,2
	mov dx,3c4h
	out dx,ax
	
	pop ax
	xor ah,ah     ;Offset der Blockspalte
	push ax       
	shr al,2
	and al,3
	mov bx,ax
	pop ax
	and al,3
	shl al,2
	or bl,al
  
	pop ax        ;x+position (1st push)
	shr ax,2
	mov di,ax
	add di,scoff
	mov ah,scrollx_MapOffsetY ;Adresse auf Karte
	mov cx,scrollx_OffsetX
	shr cx,4
	mov al,cl
	add al,MapOffsetX
	mov si,ax
  
	mov ds,data
	mov ds,scrollx_map_segment ;Segment der Levelkarte
	mov bp,15
	mov cx,24     ;24 Bl”cke senkrecht
ScLe2:  lodsb         ;Block holen
	mov ah,al
	xor al,al
	;Zeilenoffset des Blocks
	push si,ds,cx
	mov ds,data
	mov ds,scrollx_tile_segment
	mov es,0a000h
	mov si,ax     ;Blocklinie zeichnen
	add si,bx     ;(Offset fr Spalte)
	mov cx,159
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di+79],al
	add di,cx
	pop cx,ds,si
	add si,255    ;n„chster Block auf Karte
	loop ScLe2b
	popa
	mov ds,cs
	ret
ScLe2b: jmp ScLe2


; ====================
; >>> Scroll right <<<
; ====================
scrollx_right:
	pusha
	mov ds,cs
	inc w scrollx_OffsetX
	mov ax,scrollx_OffsetX;Bildschirmadresse
	add ax,320-1
  
	push ax,ax    ;Write enable register setzen
	mov cl,al
	and cl,1111b
	and cl,3
	mov ah,1
	shl ah,cl
	mov al,2
	mov dx,3c4h
	out dx,ax
  
	pop ax
	xor ah,ah     ;Offset der Blockspalte
	push ax       
	shr al,2
	and al,3
	mov bx,ax
	pop ax
	and al,3
	shl al,2
	or bl,al
  
	pop ax        ;x+position (1st push)
	shr ax,2
	mov di,ax
	add di,scoff
	mov ah,scrollx_MapOffsetY;Adresse auf Karte
	mov cx,scrollx_OffsetX
	add cx,319
	shr cx,4
	mov al,cl
	add al,scrollx_MapOffsetX
	mov si,ax
  
	mov ds,cs:scrollx_map_segment ;Segment der Levelkarte
	mov bp,15
	mov cx,24     ;24 Bl”cke senkrecht
ScRi2:  lodsb         ;Block holen
	mov ah,al
	xor al,al
	;Zeilenoffset des Blocks
	push si,ds,cx
	mov ds,cs:scrollx_tile_segment
	mov es,0a000h
	mov si,ax     ;Blocklinie zeichnen
	add si,bx     ;(Offset fr Spalte)
	mov cx,159
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	add si,bp
	lodsb
	stosb
	mov b[es:di-81],al
	add di,cx
	pop cx,ds,si
	add si,255    ;n„chster Block auf Karte
	loop scRi2b
	popa
	mov ds,cs
	ret
scri2b: jmp scri2

; ===================
; >>> Scroll down <<<
; ===================
scrollx_down:
	pusha
	mov ds,cs
	inc w scrollx_OffsetY
	mov ax,scrollx_OffsetY ;Bildschirmadresse
	add ax,12*16-1
	push ax
	mov bl,al     ;Blockzeile
	and bl,15
	xor bh,bh
	shl bx,4      ;(offset) 1 Zeile=16 bytes

	xchg al,ah    ;Zeile*160
	ror ax,1
	mov di,ax
	shr ax,2
	add di,ax
	add di,scoff
	pop ax        ;Adresse auf Karte (1st push Ypos)
	shr ax,4
	mov ah,al
	add ah,scrollx_MapOffsetY
	mov al,scrollx_MapOffsetX
	mov si,ax
  
	mov dx,3c4h
	mov es,0a000h ;Bildschirmsegment
	mov cx,40     ;40 Bl”cke horizontal
ScDo2:  mov ds,cs:scrollx_map_segment ;Segment der Levelkarte
	lodsb         ;Block holen
	mov ah,al
	xor al,al
	add ax,bx     ;Zeilenoffset des Blocks...
  
	push si,ds
	mov ds,cs:scrollx_tile_segment
	mov si,ax     ;... nach SI
	mov bp,di     ;Zielposition retten
  
	mov ax,0102h
	out dx,ax
	lodsd
	stosd
	mov di,bp
	mov [es:di-(12*16*160)],eax
  
	mov ax,0202h
	out dx,ax
	lodsd
	stosd
	mov di,bp
	mov [es:di-(12*16*160)],eax
  
	mov ax,0402h
	out dx,ax
	lodsd
	stosd
	mov di,bp
	mov [es:di-(12*16*160)],eax
  
	mov ax,0802h
	out dx,ax
	lodsd
	stosd
	mov w[es:di-(12*16*160)-4],eax
  
	pop ds,si
	loop scDo2
	popa
	ret


; =================
; >>> Scroll up <<<
; =================
scrollx_up:
	pusha
	mov ds,cs
	dec w scrollx_OffsetY
	mov ax,scrollx_OffsetY ;Bildschirmadresse
	push ax
	mov bl,al     ;Blockzeile
	and bl,15
	xor bh,bh
	shl bx,4      ;(offset) 1 Zeile=16 bytes

	xchg al,ah    ;Zeile*160
	ror ax,1
	mov di,ax
	shr ax,2
	add di,ax
	add di,scoff
	pop ax        ;Adresse auf Karte (1st push Ypos)
	shr ax,4
	mov ah,al
	add ah,scrollx_MapOffsetY
	mov al,scrollx_MapOffsetX
	mov si,ax
	 
	mov dx,3c4h
	push 0a000    ;Bildschirmsegment
	pop es
	mov cx,40     ;40 Bl”cke horizontal
ScUp2:  mov ds,data
	mov ds,scrollx_map_segment ;Segment der Levelkarte
	lodsb         ;Block holen
	mov ah,al
	xor al,al
	add ax,bx     ;Zeilenoffset des Blocks...
  
	push si,ds
	mov ds,data
	mov ds,scrollx_tile_segment
	mov si,ax     ;... nach SI
	mov bp,di     ;Zielposition retten
  
	mov ax,0102h
	out dx,ax
	lodsd
	stosd
	mov di,bp
	mov [es:di+(12*16*160)],eax
  
	mov ax,0202h
	out dx,ax
	lodsd
	stosd
	mov di,bp
	mov [es:di+(12*16*160)],eax
  
	mov ax,0402h
	out dx,ax
	lodsd
	stosd
	mov di,bp
	mov [es:di+(12*16*160)],eax
  
	mov ax,0802h
	out dx,ax
	lodsd
	stosd
	mov [es:di+(12*16*160)-4],eax
  
	pop ds,si
	loop scup2
	popa
	ret

; ===========================
; >>> Set window position <<<
; ===========================
; BX=Zeile, CX=Spalte
scrollx_setwindow:
	mov bx,Scrollx_OffsetY
	mov cx,Scrollx_OffsetX
	push bx,cx
	xchg bl,bh    ;Zeile berechnen
	ror bx,1
	mov ax,bx
	shr ax,2
	add bx,ax
	shr cx,2
	add bx,cx
	add bx,scoff

	mov dx,3d4h
	mov ah,bh
	mov al,0ch
	out dx,ax
	mov ah,bl
	inc al
	out dx,ax
	;Panning  
	mov dx,3c0h     
	mov al,13h
	out dx,al
	pop cx
	push cx
	mov al,cl
	and al,3
	shl al,1
	out dx,al
	mov al,20h
	out dx,al
	pop cx,bx
	ret

; ============================
; >>> Draw background tile <<<
; ============================
; CX/DX=Zieladresse;AX=Nummer des Blocks
scrollx_draw_tile:
	push ds,dx,cx,si,bp
	mov ah,al     ;Adresse des Blocks im GFXsegment
	xor al,al
	mov si,ax
	shl cx,4
	shl dx,2
	xchg cl,ch    ;Zeile*160 nach BP
	ror cx,1
	mov bp,cx
	shr cx,2
	add bp,cx
	add bp,dx
	add bp,scoff
	mov dx,3c4h   ;Ebene selektieren

	mov ds,data
	mov ds,scrollx_tile_segment
	mov es,0a000h
	mov cx,16     ;(sp„ter NLT)
  
DrBl1:  mov ax,102h
	out dx,ax
	mov di,bp
	movsd
	mov di,bp
	mov ax,202h
	out dx,ax
	movsd
	mov di,bp
	mov ax,402h
	out dx,ax
	movsd
	mov di,bp
	mov ax,802h
	out dx,ax
	movsd
	add bp,160
	loop DrBl1
	pop bp,si,cx,dx,ds
	ret

; ==================================
; >>> Draw level (redraw screen) <<<
; ==================================
scrollx_redraw:
	pusha
	push ds
	mov ds,data
	mov ds,scrollx_tile_segment
	xor bl,bl
	mov cx,96
	call SetBackPal
	mov ds,data
	mov ds,scrollx_map_segment
	xor si,si
	mov cx,25             ;Anzahl der Zeilen
DrLe1:  push cx
	mov ax,25
	sub ax,cx
	mov cx,ax
	xor dx,dx
DrLe2:  lodsb
	call DrawBlock
	inc dx
	cmp dx,40
	jne DrLe2
	pop cx
	add si,256-40
	loop DrLe1
	pop ds
	popa
	ret

	.data
; Datenbereich
scrollx_MapOffsetX      dw ?
scrollx_MapOffsetY      dw ?
scrollx_OffsetX         dw ?
scrollx_OffsetY         dw ?
scrollx_Switched        db ?    ;Flag, wenn Ausschnitt Rand berschritt
scrollx_map_segment     dw ?
scrollx_tile_segment    dw ?


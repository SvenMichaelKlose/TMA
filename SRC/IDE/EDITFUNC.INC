;TMA macro assembler
;Line functions
; (c)1996-1997 S.Klose

	.code
compon	macro
	mov b flag_nocomp,1	;Kompression abschalten
	endm

compoff macro
	mov b flag_nocomp,0    ;Kompression einschalten
	endm

set_marker:
	test bl,_shift
	je >p4
	test b oldstate,_shift
	if e mov marked_start,ax
	mov dx,crsr_addr
	mov marked_end,dx
p3:	mov oldstate,bl
	ret
	;
p4:	xor ax,ax
	xor dx,dx
	mov marked_start,ax
	mov marked_end,ax
	jmp p3

; =================================================
; >>> Springt an den Anfang der aktuellen Zeile <<<
; =================================================
go_pos1:mov cx,crsr_pos
l3:	call cursor_left
	loop l3
	ret

; ===============================================
; >>> Springt an das Ende der aktuellen Zeile <<<
; ===============================================
go_end:
l4:	mov cx,bufferlen	;Abstand zur Endposition berechnen
	sub cx,crsr_pos
	sub cx,2
	or cx,cx
	je >l5
	jg >l3
	call go_pos1		;Hinter der Position, erst an den Anfang der
	jmp l4			;Zeile, dann nochmal versuchen.
l3:	call cursor_right
	loop l3
l5:	ret

; ==================================================
; >>> FÅgt ein Zeichen in die aktuelle Zeile ein <<<
; ==================================================
insert_char:
	mov bx,maxbufferlen	;PrÅfen, ob die Zeile voll ist
	cmp bx,bufferlen
	je >x3			;Zeile ist voll, nix einfÅgen
	mov bx,bufferlen	;Zeichen in Zeile Åbertragen
	sub bx,2		;cr+lf abziehen
	cmp bx,crsr_pos 	;Zeilenende hinter Cursorposition ?
	ja >l2			;Ja, weiter
	mov cx,crsr_pos
	sub cx,bx
	add bufferlen,cx
	inc cx
	mov di,bx
	add di,buffer
	mov al," "
	rep stosb
	mov al,cr
	stosb
	mov al,lf
	stosb
	jmp >l3
	;
l2:	mov si,bufferlen	;LÑnge des zu verschiebenden StÅckes berechnen
	mov cx,si
	sub cx,crsr_pos
	add si,buffer-1
	mov di,si
	inc di
	std
	rep movsb		;Freies Byte fÅr Zeichen schaffen
	cld
l3:	mov bx,crsr_pos
	mov [bx+buffer],ah	;Zeichen ablegen
	inc w bufferlen
	cmp ah,tab
	jne >x1
	call buffer2line
	call line2buffer
x2:	call cursor_right
	test b crsr_pos,111b
	jnz x2
x3:	ret
	;
x1:	call cursor_right
	ret

; ==================================================
; >>> Lîscht ein Zeichen aus der aktuellen Zeile <<<
; ==================================================
delete_char:
	mov cx,bufferlen
	sub cx,crsr_pos
	cmp cx,2
	jl >l1
	ja >l2	;ne
;	 jl >l1 		 ;Nix zu lîschen
	cmp b input_mode,textmode      ; NÑchste Zeile an die aktuelle anhÑngen
	jne >l1 		;Geht nicht, Eingabezeile ist aktiv
	push crsr_addr
	call buffer2line
	mov bx,ofs crsr_addr	;Ist dies die letzte Zeile ?
	call get_next_line
	jc >l10 		;Ja, nichts machen
	compon
	call line2buffer	;und nÑchste Zeile dekomprimieren
	call buffer2line
	pop crsr_addr

	call line2buffer	;cr+lf von aktueller Zeile abschneiden...
	sub w bufferlen,2
	call buffer2line	;und in den Text Åbernehmen (also nichts)
	call line2buffer
	compoff
	ret
	;
l2:	;jcxz >l1
	mov si,crsr_pos 	;nachfolgende Zeichen Åber zu aktuelles
	add si,buffer		; Zeichen kopieren.
	mov di,si
	inc si
	rep movsb
	dec w bufferlen
	ret
	;
l10:	pop crsr_addr
	call line2buffer
l1:	ret

; ================================
; >>> Kopiert Zeile zum Puffer <<<
; ================================
line2buffer:
	push ds
	push es
	mov ax,cs
	mov es,ax
	mov si,crsr_addr
	mov ds,seg_text
	mov di,buffer
	xor cx,cx
	xor dx,dx
l1:	lodsb
	cmp al,tab
	je >t1
	cmp al,cr
	je >p2
l3:	stosb
	inc cx
	inc dx
	cmp cx,0feh
	je >p2
	jmp l1

t1:	mov al," "
	stosb
	inc cx
	test cx,111b	;an Tabulatorposition ?
	jnz t1
	inc dx
	jmp l1

p2:	mov ax,lf by cr
	stosw
	add cx,2		;cr+lf
	add dx,2
	mov cs:bufferlen,cx
	mov cs:oldbufferlen,dx
	pop es
	pop ds
	ret

; =============================================
; >>> Ersetzt alte Zeile durch Pufferinhalt <<<
; =============================================
buffer2line:
	push ds
	push es
	pusha
	call compress_line
	mov ax,cs:seg_text
	mov ds,ax
	mov es,ax
	;
	mov si,cs:crsr_addr
	mov di,si
	mov ax,cs:bufferlen
	add di,ax
	add si,cs:oldbufferlen
	mov cx,cs:filesize
	sub cx,si
	mov bx,cs:oldbufferlen
	cmp ax,bx
	je >n1
	jb >l1
	std
	add si,cx
	add di,cx
	mov cs:filesize,di
	dec si
	dec di
	rep movsb
	cld
	jmp >n1
	;
l1:	rep movsb
	mov cs:filesize,di
n1:	mov si,buffer
	mov cx,cs:bufferlen
	mov di,cs:crsr_addr
	mov ax,cs
	mov ds,ax
	jcxz >l1		;Nix zu kopieren
	rep movsb
l1:	popa
	pop es
	pop ds
	ret

compress_line:
	mov ax,cs
	mov ds,ax
	cmp b flag_nocomp,1
	je >l5
	mov es,ax
	mov si,buffer
	mov di,si
	xor dx,dx
	mov cx,bufferlen
	jcxz >l5
l1:	lodsb
	;
	cmp al,"'"	;Nicht komprimieren in AnfÅhrungszeichen
	je >l10
	cmp al,'"'
	jne >l7
l10:	mov ah,al
	stosb
	inc dx
	dec cx
	jz >l2
l8:	lodsb
	inc dx
	cmp al,ah
	je >l2
	stosb
	loop l8
	jmp >l9
	;
l7:	inc dx
	cmp al,0dh
	je >l6
	cmp al,0ah
	jne >l7
l6:	xor dx,dx
	jmp >l2
l7:	cmp al," "
	jne >l2
	push cx
	push si
	push dx
	test dx,111b
	jz >k2
l4:	lodsb
	cmp al," "
	jne >k2
	inc dx
	dec cx
	test dx,111b
	jnz l4
	;
	pop ax
	pop ax
	pop ax
	mov al,9
l2:	stosb
	loop l1
l9:	sub di,buffer
	mov bufferlen,di
l5:	ret

k2:	pop dx
	pop si
	pop cx
	mov al," "
	jmp l2

	
; ==========================================
; >>> Routinen zur Steuerung des Cursors <<<
; ==========================================
cursor_down:
	push bx
	inc w crsry	       ;nÑchste Zeile
	mov ax,w crsry	       ;öber den Bildschirm hinaus ?
	cmp ax,screenheight
	jne >o1
	dec w crsry	       ;Ja, Zeile beibehalten
	mov bx,ofs crsr_addr
	call get_next_line
	jc >l2
	mov bx,ofs screen_addr	;Screen nach oben scrollen
	call get_next_line
	jc >l2			;!!! if nc 
	inc w disp_line
	jmp >l1
	;
o1:	mov bx,ofs crsr_addr	;Neue Adresse des Cursors holen
	call get_next_line
	jnc >l1
	dec w crsry	       ;Cursor geht nicht Åber Text hinaus
	stc	;clc
	jmp >l2
l1:	inc w actual_line
l2:	pop bx
	call check_xmspos
	ret

cursor_up:
	push bx
	dec w crsry
	mov ax,crsry
	cmp ax,-1
	jne >o1
	inc w crsry
	mov bx,ofs screen_addr
	call get_last_line
	if nc dec w disp_line
o1:	mov bx,ofs crsr_addr
	call get_last_line
	jnc >l1
	xor ax,ax
	mov [crsry],ax
	stc
l1:	if nc dec w actual_line
	pop bx
	call check_xmspos
	ret

cursor_left:
	mov ax,crsrx
	or ax,ax
	jnz >l3
	mov ax,coloffset
	or ax,ax
	jz >l1
	dec w coloffset
	dec w crsr_pos
	clc
	ret
	;
l3:	dec w crsrx
	dec w crsr_pos
	clc
	ret
	;
l1:	stc
	ret

cursor_right:
	mov ax,crsrx
	inc ax
	cmp ax,screenwidth
	jne k3
	inc w coloffset
	inc w crsr_pos
	ret
	;
k3:	inc w crsrx
	inc w crsr_pos
	ret

; =======================================
; >>> Sucht nach der vorherigen Zeile <<<
; =======================================
; Input : Adresse des Zeigers in BX
get_last_line:
	pusha
	push ds
	mov ds,cs:seg_text
	mov si,[cs:bx]		;Pointer auf Anfang der aktuellen Zeile
	or si,si
	jz >e1			;Zeiger befindet sich bereits am Anfang
	std
	dec si
l1:	lodsb			;Endmarkierung der vorherigen Z. Åberspringen
	cmp si,-1
	je >f1
	cmp al,13
	je >l2
	cmp al," "
	jb l1
	jmp >l3
	;
l2:	lodsb			;Bei erneuter Endmarkierung abbrechen
	cmp si,-1
	je >f1
	cmp al,13
	je >l4			;Leerzeile
	cmp al,tab		;bei normalen Zeichen, weiter in der Schleife
	je l2
	cmp al," "
	jnb l2
	;
l3:	inc si
l4:	cld
d3:	lodsb
	cmp al,13
	je >l1
	cmp al,tab
	je >l1
	cmp al," "
	jb d3
l1:	dec si
	cmp si,-1
	jne >l1
f1:	inc si
l1:	mov [cs:bx],si
	cld
	pop ds
	popa
	clc
	ret

e1:	cld
	pop ds
	popa
	stc
	ret

; =====================================
; >>> Sucht nach der nÑchsten Zeile <<<
; =====================================
; Input : Adresse des Zeigers in BX
get_next_line:
	pusha
	push ds
	mov ds,cs:seg_text
	mov si,[cs:bx]
	cmp si,cs:filesize
	jae >e1
l2:	lodsb		;Zeichen holen
	cmp al,13	;Ende der Zeile ?
	je >l1		;Yep, Anfang der nÑchsten suchen
	cmp al,tab
	je l2
	cmp al,lf
	je l2
	cmp al," "	;Legale Zeichen Åberspringen
	jnb l2
	;
l1:	lodsb		;Anfang der nÑchsten suchen.
	cmp al,13	;Leerzeile !
	je >l2
	cmp al,tab
	je >l2
	cmp al," "
	jb l1
l2:	dec si
	;
l1:	cmp si,cs:filesize ;öber das Ende des Files hinaus ?
	jae >e1 	;Yepp, neuen Zeiger nicht speichern
	mov [cs:bx],si
	pop ds
	popa
	clc
	ret
	;
e1:	pop ds
	popa
	stc
	ret

check_xmspos:
	ret


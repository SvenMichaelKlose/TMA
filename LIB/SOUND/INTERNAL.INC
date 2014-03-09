; *** Internal speaker playroutine
; *** (c)1994-1996 Sven Klose
	.code
spkcorrup=2
spkcorrdown=2
GetInternal:
	mov ds,_data
	mov w[sdev_start],startinternal
	mov w[sdev_stop],stopinternal
	mov b[dms_sur],0
	mov b[sdev_bits],8	;8-bit, mono
	mov eax,1234dch 	   ;wirkliche Samplingrate einstellen
	xor ebx,ebx
	mov bx,[cs:DMSresolution]
	xor edx,edx
	div ebx
	mov ebx,eax
	mov eax,1234dch
	xor edx,edx
	div ebx
	mov [cs:DMSresolution],ax
	mov bx,MODintcalls	;Zeitliche Korrektur des alten Interrupts
	xor dx,dx
	div bx
	mov [spkrtimecorr],ax
	mov w[sdev_maxrate],62798
	mov dx,TXT_internal
	mov ah,9
	int 21h
	clc			;Speaker ist immer da !
	ret

; *** Schaltet Playroutine ein
StartInternal:
	cli
	push ds
	mov ax,data
	mov ds,ax
	mov es,ax
	
	mov ax,[DMAbufferseg]
	mov [spkradr],ax
	mov ax,spkrtimingcheck
	mov [MODtimingcheck],ax
	mov b[spkref],80h
	push ds
	xor ax,ax
	mov ds,ax
	mov ax,data
	shl eax,16
	mov ax,spkrint		;und auf eigene Routine richten
	xchg eax,[4*8]
	pop ds
	mov [cs:spkroldoff],eax
	mov eax,1234dch
	xor ebx,ebx
	mov bx,[DMSresolution]
	xor edx,edx
	div ebx
	mov bx,ax
	mov al,00110100b	;Samplingrate ca. 22KHz
	out 43h,al
	mov al,bl
	out 40h,al		;(INT 08H)
	mov al,bh
	out 40h,al
	mov ax,[DMSresolution]
	mov bx,MODintcalls
	xor dx,dx
	div bx
	mov [spkroldintcnt],ax
	pop ds
	sti
	ret

; ********************************
; *** Schaltet Playroutinen ab ***
; ********************************
stopinternal:
	cli
	push ds
	mov ds,data
	mov al,110100b		;Interne Uhr korrigieren
	out 43h,al
	xor ax,ax
	out 40h,al
	out 40h,al
	push ds
	xor ax,ax
	mov ds,ax
	mov eax,[cs:spkroldoff]
	mov [8*4],eax
	pop ds
	in al,61h	;Lautsprecher aus
	and al,-3
	out 61h,al
	mov al,20h	;EOI fr letzten int..
	out 20h,al
	pop ds
	ret

; **************************************************
; *** Playing interrupt for the internal speaker ***
; **************************************************
	align 20h
spkrint:cli
	pusha
	push ds,fs
	mov ds,data
	mov fs,bufseg
	mov si,[spkradr]	;Zeiger auf aktruellen Wert holen
	mov dx,61h		;Lautsprecherstatus auslesen
	in al,dx
	mov bl,[fs:si]		;Wert holen
	inc si			;Zeiger inkrementieren und...
	mov [spkradr],si     ;...zurckschreiben.
	cmp bl,[spkref]      ;Wert mit Referenz vergleichen
	jb >l1			;Kleiner...
	or al,2 		;Gr”sser, Lautsprecher
	out dx,al
	add b[spkref],spkcorrup
	dec w[spkroldintcnt]
	jz coldint		;Alten Interrupt aufrufen
	mov al,20h
	out 20h,al
	pop fs,ds
	popa
	iret

	align 20h
l1:	and al,0fch
	out dx,al
	sub b[spkref],spkcorrdown
	dec w[spkroldintcnt]
	jz coldint		;Alten Interrupt aufrufen
l2:	mov al,20h
	out 20h,al
	pop fs,ds
	popa
	iret

coldint:mov ax,[spkrtimecorr]
	mov w[spkroldintcnt],ax
	mov al,20h
	out 20h,al
	pop fs,ds
	popa
	sti
	jmpf [cs:spkroldoff] ;alte Routine aufrufen

; =======================================
; >>> framecheck for playing routines <<<
; =======================================
spkrtimingcheck:
	mov bx,[spkradr]
	mov ax,[modoldmixpos]
	sub ax,bx
	mov bx,[DMSoutlen]
	cmp ax,bx
	ja >l1
	clc	;no frame to be played
	ret
l1:	stc	;frame needed
	ret

spkroldoff:	dw ?	 ;old interrupt vector 08h

#if !_mdltiny
_text	segment
#endif

TXT_internal:
	db"Internal speaker (dithered burst) - Let your pet out first !",10,13,"$"

; *******************************
; *** Daten fuer Playroutinen ***
; *******************************
	.data
align 4
spkrtimecorr:	dw ?
spkradr:	dw ?
spkroldseg:	dw ?
spkroldintcnt:	dd ?	 ;usually word
spkref: 	db ?


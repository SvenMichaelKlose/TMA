; *******************
; *** Datei laden ***
; *******************
; DS:DX=Dateiname, ES:DI=Ziel
	.code
LoadFile:
	mov ax,3d00h	  ;Datei �ffnen
	int 21h
	mov bx,ax	   ;Handle �bergeben
	mov ah,3fh	   ;Block einlesen
	mov cx,32000	;Soviel wie m�glich laden
	push ds    ;Zielsegment �bergeben
	mov ds,es
	mov dx,di	   ;Zieladresse
	int 21h    ;Daten laden
	pop ds
	mov ah,3eh	   ;Datei schliessen
	int 21h
	ret

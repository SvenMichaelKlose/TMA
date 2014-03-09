; *******************
; *** Datei laden ***
; *******************
; DS:DX=Dateiname, ES:DI=Ziel
	.code
LoadFile:
	mov ax,3d00h	  ;Datei îffnen
	int 21h
	mov bx,ax	   ;Handle Åbergeben
	mov ah,3fh	   ;Block einlesen
	mov cx,32000	;Soviel wie mîglich laden
	push ds    ;Zielsegment Åbergeben
	mov ds,es
	mov dx,di	   ;Zieladresse
	int 21h    ;Daten laden
	pop ds
	mov ah,3eh	   ;Datei schliessen
	int 21h
	ret

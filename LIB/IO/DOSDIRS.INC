	.code

nme_nextfile=DTA+1eh

find_first:
	push dx
	mov ah,1ah		;Neue DTA festlegen
	mov dx,DTA
	int 21h
	mov ah,4eh		;1. Datei suchen
	pop dx
	int 21h
	ret

;Sucht n„chste Eingabedatei
find_next:
	mov ah,4fh		;N„chsten Dateinamen holen
	int 21h
	cmp al,12h
	je >e1
	clc
	ret
e1:	stc
	ret

	.data
dta:		db 256 dup ?


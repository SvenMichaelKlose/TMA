; TetraByte TSR clock by Spaceman
	jmp start
	;
	;Interruptroutine
intcall:pushad			;SÑmtliche Register retten.
	mov di,144		;Zielposition im Bildschirm nach DS:DI.
	mov eax,[6ch]		;Zeit holen, DS zeigt bereits auf BIOS-Daten
	mov ds,0b800h
	mov ch,1eh		;Bildschirmfarbe.
	;
	xor edx,edx		;Stundenzahl berechnen.
	mov ebx,10008h
	call nrout
	call dbldot		;Doppelpunkt ausgeben.
	mov bx,444h		;Minuten ausgeben.
	call nrout2
	call dbldot		;Doppelpunkt...
	shl ax,4		;Genauigkeit erhîhen...
	mov bx,123h		;...und Sekunden ausgeben.
	call nrout2
	popad			;Register wiederholen...
	iret			;...und Interrupt verlassen.

	;Stunde, Minute oder Sekunde ausgeben
nrout:	rand:			;EAX/EBX=EAX; Rest in EDX.
nrout2: div bx			;AX/BX=AX; Rest in DX.
	push dx 		;Rest retten.
	mov bl,10		;Wert durch 10 teilen; Rest in AH.
	div bl
	call charout		;1. Ziffer ausgeben.
	mov al,ah		;Rest nach AL holen...
	call charout		;...und 2. Ziffer ausgeben.
	pop ax			;Rest der aktuellen Zeit nach AX.
	xor dx,dx		;DX fÅr nÑchste Division lîschen.
	ret

	;Ziffer ausgeben
charout:mov cl,al		;Wert der Ziffer nach CL...
	add cl,"0"		;...und in ASCII umwandeln...
s5:	mov [di],cx		;...und in den Bildschirm schreiben.
	add di,2		;NÑchste Bildschirmposition.
	ret

	;Doppelpunkt ausgeben
dbldot: mov cl,":"		;CL mit Doppelpunkt laden...
	jmp s5			;...und ausgeben.

	;Initialisierung
start:	mov dx,msg_standard	 ;Meldung ausgeben.
	mov ah,9
	int 21h
	mov dx,intcall		;Int 1ch auf neue Routine legen.
	mov ax,251ch
	int 21h
	mov dx,start		;Programm bis Label "start" im Speicher...
	int 27h 		;...behalten und beenden.

msg_standard:db"TetraByte TSR clock activated.",10,13,"$"


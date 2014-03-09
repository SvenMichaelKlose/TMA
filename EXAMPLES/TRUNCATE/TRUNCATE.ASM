; *** Truncate Utility - (c)1995 Sven Klose
	.code
	mov di,datastart
	mov cx,dataend-datastart
	xor al,al
	rep stosb
	mov dx,TXT_standard
	mov ah,9
	int 21h
	call OpenFiles
	mov ax,cs
	add ax,4k
	mov InputSeg,ax
	mov w[DestHandle],0
	mov b[FileNum],"0"
; ----- Neuen Dateinamen erstellen
	mov si,SourceName
	mov di,DestName
	mov ax,":A"
	stosw
	mov al,"\"
	stosb
CopyNotDone:
	lodsb
	stosb
	or al,al
	jnz copynotdone
	sub di,2
	mov FileNumAdr,di

; ----- Ende der Initialisierung
	call CreateNewPart

ReadSourceBlock:
	mov dx,TXT_reading
	mov ah,9
	int 21h
	mov bx,SourceHandle
	mov ah,3fh      ;Block einlesen
	mov cx,32k      ;Soviel wie mîglich laden
	push ds,Inputseg;Zielsegment Åbergeben
	pop ds
	xor dx,dx       ;Zieladresse
	int 21h         ;Daten laden
	pop ds
	or ax,ax        ;Ende der Datei ?
	jz Eof
	mov InputSize,ax
	xor dx,dx
WriteLoop:
	push dx
	mov dx,TXT_writing
	mov ah,9
	int 21h
	pop dx
	mov ah,40h
	mov bx,DestHandle
	mov cx,InputSize
	push dx,ds,InputSeg
	pop ds
	int 21h
	pop ds
	sub [InputSize],ax
	jz ReadSourceBlock
	add dx,ax
	call CreateNewPart
	jmp WriteLoop

Eof:    call CloseFiles
	mov dx,TXT_done
	mov ah,9
	int 21h
	mov ax,4c00h
	int 21h

ErrorBye:
	mov ah,9
	int 21h
	mov ax,4cffh
	int 21h

CreateNewPart:
	push dx
	mov bx,DestHandle
	or bx,bx
	jz DontClose
	push bx
	mov dx,TXT_closing
	mov ah,9
	int 21h
	pop bx
	mov ah,3eh
	int 21h
DontClose:
	inc b[FileNum]
	mov al,FileNum
	mov bx,FileNumAdr
	mov b[bx],al
	mov dx,TXT_newdisk
	mov ah,9
	int 21h
	xor ax,ax
	int 16h
	mov dx,TXT_creating
	mov ah,9
	int 21h
	mov ah,3ch
	xor cx,cx
	mov dx,DestName
	int 21h
	mov DestHandle,ax
	pop dx
	ret

; ********************************************************
; *** Holt Dateinamen aus Kommandozeile und îffnet Sie ***
; ********************************************************
OpenFiles:
	mov si,81h      ;Dateinamen suchen
	mov di,SourceName
	call GetEnvName
	mov w[SourceLength],cx
	
	mov dx,SourceName
	mov ax,3d00h      ;Datei îffnen
	int 21h
	jnc Open4       ;Kein Fehler,weiter
	mov dx,ERR_SourceNotOpen
	jmp ErrorBye
Open4:  mov bx,ax       ;Handle Åbergeben
	mov w[SourceHandle],ax
	ret

; *********************************************
; *** Holt Dateinamen aus der Kommandozeile ***
; *********************************************
; Aufruf: DS:SI=Quellstring DS:DI=Ziel
GetEnvName:
	push cs,cs
	pop ds,es
OpenLoop0:
	lodsb
	cmp al,' '
	je OpenLoop0
	cmp al,0dh      ;Ende der Zeile ?
	jne Open2       ;Nein,weiter ...
	mov dx,ERR_NoName
	jmp ErrorBye
Open2:  dec si
	xor cx,cx
OpenLoop3:
debug
	es:lodsb
	stosb
	inc cx
	cmp al,' '
	je Open3
	cmp al,0dh
	jne OpenLoop3
Open3:  dec cx
	dec si
	dec di
	xor al,al       ;Ende des Namens kennzeichnen
	stosb
	ret

; ******************************
; *** Schliesst alle Dateien ***
; ******************************
CloseFiles:
	mov bx,w[SourceHandle]
	mov ah,3eh      ;Datei schliessen
	int 21h
	mov bx,w[DestHandle]
	mov ah,3eh
	int 21h
	ret

TXT_standard:
	db"Truncate Utility - (c)1995 by Spaceman/Tetrabyte",10,13
	db"================================================",10,13,"$"
TXT_creating:
	db"Creating file...                           ",13,"$"
TXT_reading:
	db"Reading file...                            ",13,"$"
TXT_writing:
	db"Writing to disk...                         ",13,"$"
TXT_closing:
	db"Closing part...                            ",13,"$"
TXT_done:
	db"Done.                                      ",10,13,"$"
TXT_newdisk:
	db"Please insert a new disk and press a key...",13,"$"
ERR_sourceNotOpen:
	db"Sorry, couldn't open your desired source.",10,13,"$"
ERR_noname:
	db"Sorry, you'll have to type in",10,13,10
	db"TRUNCATE <YourFile>",10,13,10
	db"This program will ask you for disks in drive A:",10,13
	db"then it will extend the filenames with",10,13
	db"numbers....",10,13
	db"Ask the author for the rest <giggle>",10,13,"$"

	db"Ceterum censeo Cannabis legalisandum esse !"

	.data
datastart:
SourceHandle    dw ?
DestHandle      dw ?
SourceLength    dw ?
InputSeg        dw ?
InputSize       dw ?
FileNumAdr      dw ?
FileNum         db ?
SourceName:     db 64 dup ?
DestName:       db 64 dup ?
dataend:

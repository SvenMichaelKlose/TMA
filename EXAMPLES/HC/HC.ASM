; *** Tetrabyte's Horrorcrypt
; *** (c)1995 Sven Klose/Tetrabyte
	.code
Horrorcrypt:
	mov dx,TXT_standard
	mov ah,9
	int 21h
	call line
	mov dx,TXT_standard2
	mov ah,9
	int 21h
	call line
	mov di,datastart
	mov cx,dataend-datastart
	xor al,al
	rep stosw
	call OpenFiles
	cmp b[comc],0
	je bullshit
	mov ax,cs
	add ax,1000h
	mov InputSeg,ax
	add ax,1000h
	mov DestSeg,ax
	mov dx,txt_converting
	mov ah,9
	int 21h

Reload: mov bx,cs:SourceHandle
	mov ah,3fh	;Block einlesen
	mov cx,8000h	;Soviel wie mîglich laden
	mov ds,cs:Inputseg ;Zielsegment Åbergeben
	xor dx,dx	;Zieladresse
	int 21h 	;Daten laden
	jc doserrorroutine
	or ax,ax	;Ende der Datei ?
	jz Eof
	mov cx,ax
	xor si,si
	xor di,di
	mov al,"."
	int 29h
	mov es,cs:destseg
mloop:	cmp b[cs:chstack],0
	je nost
	mov al,b[cs:chstack]
	mov b[cs:chstack],0
	inc cx
	jmp stck

nost:   lodsb

stck:   call ansifilter
	jc ansiorjmp
f2:     cmp b[cs:fnogerm],1
	jne f3
	call german
f3:
	cmp b[cs:frecfr],1
	jne norec
	call reconstruct
norec:
	cmp b[cs:fnobeg],1
	jne nobeg
	cmp b[cs:lastchar]," "
	jle didboth
nobeg:  cmp b[cs:ftocap],1
	je docap
	cmp b[cs:ftosmall],1
	je nocap
	cmp b[cs:ftoboth],1
	jne didboth
	mov ah,b[cs:changeflag]
	add b[cs:change],ah
	test b[cs:change],1
	jz nocap
docap:  call tocapital
	jmp didboth

ansiorjmp:
	cmp b[cs:ftoascii],1    ;ansi-zeichen gefunden, filtern
	je ignore               ;ja, weiter
	jmp nofreak             ;nein, nicht behandeln

nocap:	call tosmall
didboth:cmp b[cs:fconvbrdr],1
	jne nocb
	call convertbrdr
nocb:   cmp b[cs:ftoFreaky],1
	jne nofreak
	call tosymbols
nofreak:stosb
	mov cs:lastchar,al
ignore: mov cs:lastchar2,al
	loop mloop2
	mov cx,di
	mov ah,40h
	mov bx,cs:DestHandle
	xor dx,dx
	mov ds,cs:destseg
	int 21h
	jc doserrorroutine
	jmp reload
mloop2:jmp mloop
Eof:    push cs
	pop ds
	call CloseFiles
	mov dx,TXT_done
	mov ah,9
	int 21h
	mov ax,4c00h
	int 21h

ErrorBye:
bullshit:
	mov dx,ERR_noname
	mov ah,9
	int 21h
	call line
	mov ax,4cffh
	int 21h


; ********************************************************
; *** Holt Dateinamen aus Kommandozeile und îffnet Sie ***
; ********************************************************
OpenFiles:
	mov si,81h      ;Dateinamen suchen
	call GetComs
	mov di,SourceName
	call GetEnvName
	jc bullshit
	mov SourceLength,cx
	call GetComs
	mov di,Destname
	call GetEnvName
	mov DestLength,cx
	pushf
	call Getcoms
	popf
	jnc name2
	cmp b[ftofreaky],0
	je an2
	mov dx,efrk
	jmp addext
an2:    cmp b[frecfr],0
	je an3
	mov dx,erec
	jmp addext
an3:    cmp b[ftoascii],0
	je addext
	mov dx,easc
addext: call chgext
name2:	mov al,b[ftosmall]
	and al,b[ftocap]
	jz gx1
	mov dx,ERR_notboth
	mov ah,9
	int 21h
	mov ax,4cffh
	int 21h
gx1:	cmp b[comc],0
	je bullshit
; ----- Print filename that'll be created !
	mov dx,TXT_creating
	mov ah,9
	int 21h
	mov cx,destlength
	mov si,Destname
wrn:    lodsb
	int 29h
	loop wrn
	mov dx,TXT_cr
	mov ah,9
	int 21h
; -----
	mov dx,SourceName
	mov ax,3d00h      ;Datei îffnen
	int 21h
	jc Doserrorroutine
	mov SourceHandle,ax
; -----
	mov dx,DestName
	mov ah,5bh      ;Neue Datei erstellen
	xor cx,cx
	int 21h
	jnc notexisting
	cmp al,80
	jne doserrorroutine
	cmp b[fyes],1
	je overwrite
	mov dx,WAR_exists
	mov ah,9
	int 21h
	xor ax,ax
	int 16h
	call tocapital
	cmp al,"Y"
	je overwrite
	mov dx,txt_skipped
	mov ah,9
	int 21h
	call Closefile2
	mov ax,4c01h
	int 21h
overwrite:
	mov dx,DestName
	mov ah,3ch
	xor cx,cx
	int 21h
	jc doserrorroutine
notexisting:
	mov DestHandle,ax
	ret

; *********************************************
; *** Holt Dateinamen aus der Kommandozeile ***
; *********************************************
; Aufruf: DS:SI=Quellstring DS:DI=Ziel
GetEnvName:
	push cs
	push cs
	pop ds
	pop es
OpenLoop0:
	es:lodsb
	cmp al,' '
	je OpenLoop0
	or al,al
	je nn2
	cmp al,0dh	;Ende der Zeile ?
	jne Open2       ;Nein,weiter ...
nn2:    stc             ;No name found !
	ret
Open2:  dec si
	xor cx,cx
OpenLoop3:
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
	xor al,al	;Ende des Namens kennzeichnen
	stosb
	clc
	ret

; Forces new extension to destination name
; cs:dx=new name
ChgExt: mov si,Sourcename       ;copy name
	mov di,Destname
	mov cx,Sourcelength
	mov bx,cx
ce3:    lodsb
	call tocapital
	stosb
	loop ce3
	;
	mov si,Destname
	add si,bx
	mov cx,bx
	std
	dec si
ce2:	lodsb
	cmp al,"\"
	je slash
	cmp al,"."
	jne nopoint
	inc si
	mov di,si
	jmp putext
nopoint:loop ce2
slash:  mov di,Destname ;end of name, add ext
	add di,bx
putext: mov si,dx
	cld
	mov al,"."
	stosb
	movsw
	movsb
	xor al,al       ;mark end
	stosb
	sub di,destname+1
	mov destlength,di
	ret

; ******************************
; *** Schliesst alle Dateien ***
; ******************************
CloseFiles:
	mov bx,w[DestHandle]
	mov ah,3eh
	int 21h
	jc doserrorroutine
CloseFile2:
	mov bx,SourceHandle
	mov ah,3eh      ;Datei schliessen
	int 21h
	jc doserrorroutine
	ret

; **********************************************
; *** Gibt DOS-Fehler auf dem Bildschirm aus ***
; **********************************************
; AL=DOS-Fehlernummer
DOSerrorRoutine:
	push cs
	pop ds
	push ax                 ;Fehlermeldung ausgeben
	mov dx,TXT_cr
	mov ah,9
	int 21h
	mov ax,83 by "="
	call printstring
	mov ax,18 by " "
	call PrintString
	mov dx,TXT_DOSerr
	mov ah,9
	int 21h
	mov ax,19 by " "
	call Printstring
	mov ax,83 by "="
	call Printstring
	pop ax
	push ax
	cmp ax,27h
	jb DOSerrrt1
	mov ax,27h
DOSerrRt1:
	mov cx,ax
	mov si,TXT_doserrors
DOSErrRt2:
	lodsb
	cmp al,"$"
	jne Doserrrt2
	dec cx
	jnz Doserrrt2
	mov dx,si
	mov ah,9
	int 21h
	mov dx,TXT_cr
	mov ah,9
	int 21h
	pop ax

	mov ah,4ch              ;und mit Fehlercode beenden
	int 21h

; Prints string
Printstring:
	mov cl,ah
	xor ch,ch
p1:     int 29h
	loop p1
	ret

tocapital:
	cmp al,"a"
	jb noletter
	cmp al,"z"
	ja noletter
	sub al,"a"-"A"
noletter:
	ret

tosmall:cmp al,"A"
	jb noletter
	cmp al,"Z"
	ja noletter2
	sub al,"A"-"a"
noletter2:
	ret
leavebit7:
	or al,127
	ret

testonalph:
	cmp al,"A"
	jb toa2
	cmp al,"Z"
	jbe toa3
	cmp al,"a"
	jb toa2
	cmp al,"z"
	ja toa2
toa3:   stc
	ret
toa2:   clc
	ret

convertbrdr:
	mov dx,bordercontab
	call translate
	and al,127              ;leave bit 7 off
	ret

german: cmp al,"·"
	jne nodbls
	mov al,"s"
	mov b[cs:chstack],al
	ret
nodbls: mov dx,germancontab
	call translate
	jnc noger
	mov b[cs:chstack],"e"
noger:  ret

tosymbols:
	cmp al,"A"
	jb noletter3
	mov bx,capitals-"A"
	cmp al,"Z"
	jna nosin
	cmp al,"a"
	jb noletter3
	cmp al,"z"
	ja noletter3
	mov bx,singles-"a"
nosin:  xor ah,ah
	add bx,ax
	mov al,[cs:bx]
noletter3:
	ret

ansifilter:
	cmp b[cs:ansijump],0
	jne jumpnow
	cmp b[cs:ansiflag],0       ;ansi active ?
	je testonansi
	cmp al,"["
	je cansi
	cmp al,"0"
	jnae nocount
	cmp al,"9"
	ja nocount
	push ax
	mov bl,b[cs:ansinum]
	shl bl,1
	mov bh,bl
	shl bl,2
	add bl,bh
	sub al,"0"
	add bl,al
	mov b[cs:ansinum],bl
	pop ax
	jmp cansi
nocount:cmp al,";"
	jne nosem
	mov b[cs:ansinum],0
	jmp cansi
nosem:  cmp al,"?"
	jne noqm
	cmp b[cs:lastchar2],"["
	je cansi
noqm:   cmp b[cs:ftoascii],1
	jne nojump
	cmp al,"C"
	jne nojump
	push ax
	mov al,b[cs:ansinum]
	or al,al
	jnz notonce
	inc al
notonce:mov b[cs:ansijump],al
	pop ax
nojump: mov b[cs:ansiflag],0
cansi:	stc
	ret
testonansi:
	cmp al,7
	jb cansi
	cmp al,27
	jne noansi
	mov b[cs:ansiflag],1
	mov b[cs:ansinum],0
	jmp cansi
noansi: clc
	ret
jumpnow:mov al," "
	dec b[cs:ansijump]
	inc cx
	dec si
	clc
	ret

translate:
	mov ah,al
	push si
	mov si,dx
cb2:	mov bp,si
	cmp b[cs:si],0
	je ca
	inc si
next:	cs:lodsb
	or al,al
	je cb2
	cmp al,ah
	jne next
	cs:mov al,b[bp]
	stc
	pop si
	ret
ca:     clc
	pop si
	mov al,ah
	ret

reconstruct:
	cmp al,"0"              ;zero could be oh
	jne nozero
	mov dl,al
	mov al,cs:lastchar
	call testonalph
	jc alph
	mov al,b[si]
	call nozero
	call testonalph
	jnc noalph
alph:	mov dl,"o"
noalph: mov al,dl
nofreak3:
	ret
nozero: cmp al,"0"
	je nofreak3
	push cx
	push di
	push es
	push cs
	pop es
	mov di,singles
	mov cx,26*2
	cld
	repnz scasb
	or cx,cx
	jz nofreak2
	sub di,singles+1
	xor ah,ah
	mov al,"a"
	cmp di,25
	jna smallone
	mov al,"A"-26
smallone:
	add ax,di
nofreak2:
	pop es
	pop di
	pop cx
	ret

l1:     ret
getcoms:lodsb
	cmp al," "
	je getcoms
	cmp al,13
	je l1
	cmp al,"-"
	jne nocom
gcloop: lodsb
	cmp al,13
	je nocom
	cmp al," "
	je getcoms
	call tosmall
	mov cx,comend-coms
	mov di,coms
	push cs
	pop es
	cld
	repnz scasb
	or cx,cx
	jz notlegal
	sub di,coms+1
	shl di,1
	add di,comadrs
	mov bx,w[di]
	mov b[bx],1
	inc b[comc]
	jmp gcloop
nocom:  dec si
	ret
notlegal:
	push ax
	mov dx,ERR_notlegal
	mov ah,9
	int 21h
	pop ax
	int 29h
	mov dx,ERR_notlegal2
	jmp bullshit

Line:   mov cx,80
l1:     mov al,196
	int 29h
	loop l1
	ret

TXT_standard:
	db"       Tetrabyte's Horror Crypt & Convert - Text conversion utility v1.0",10,13,"$"
TXT_standard2:
        db"              (c)1995 Spaceman/Tetrabyte - MAILWARE ! - COPY !!!",10,13,"$"


TXT_cr: db 10,13,"$"

coms:	db"fvcbsn7guy"  ;legal commands
comend:
comadrs:dw ftoFreaky,ftoascii,ftocap,fnobeg
	dw ftosmall,ftoboth,fconvbrdr,fnogerm,frecfr,fyes

ERR_noname:
ERR_nocom:
	db"Don't know what to do...",10,13
	db"Usage: HC -<options> sourcefile [destination]",10,13,10
	db" -f   Convert letters into freaky codes",10,13
	db" -v   Convert into ASCII format with ASC ending of filename",10,13
	db" -c   Convert into capital letters",10,13
	db" -cb  Convert into capital letters except beginnings of words",10,13
	db" -s   Convert into small letters",10,13
	db" -sb  Convert into small letters except beginnnings of words",10,13
	db" -n   Convert into capital AND small letters",10,13
	db" -7   Convert IBM special codes into codes <128 and leave bit 7 off",10,13
	db" -g   Convert the German letters Ñ,î,Å and · into usual letters",10,13
	db" -u   Reconvert from freaky codes into readable format ;-) ",10,13
        db" -y   Force YES to any query",10,13,10
        db"Please visit our homepage http://www.aol.com/members/tetraweb/start.htm",10,13
	db"$"
TXT_creating:
	db 13,"                                           ",13
	db"Creating file $"
ERR_notlegal:
	db 7,"Illegal option used ! '-$"
ERR_notlegal2:
	db"'",10,13,"$"
ERR_notboth:
	db"Huh ! Stupid get...you can't use both -s and -c option !",10,13,"$"
WAR_exists:
	db"File already exists ! Overwrite (Y/N)?:$"
TXT_skipped:
	db 13,"     -=*< C0ÔVÓRSI0Ô SKIPPÓD ! >*=-    ",10,13,"$"
TXT_converting:
	db 13,"                                           ",13
	db"Converting...$"
TXT_done:
	db"done.",10,13,"$"
chstack:db 0
TXT_doserrors:
	db"$Invalid function number !",10,13,"$"
	db"File not found.$"
	db"Directory not found.$"
	db"Too many open files.$"
	db"Access to file denied.$"
	db"Invalid file handle.$"
	db"Memory control blocks are destroyed.$"
	db"Not enough memory to start program.",10,13
	db"See the doc file(s) to solve this problem.$"
        db"Can't allocate memory block. Invalid adress or segment used.$"
	db"Invalid user environment.$"
	db"Unknown file format.$"
	db"Invalid file access code.$"
	db"Invalid data for access on device.$"
	db"DOS: Internal error. Description not possible.",10,13
	db"You must be using MS-DOS (tm), don't you (y/y)?$"
	db"Invalid drive specification.$"
	db"Can't delete actual directory. Get back to the parent one and try again.$"
	db"You will have to specify the same drive for this operation.$"
	db"Can't find file.$"
	db"Disk is write protected.$"
	db"Error at access on disk drive. Bad disk unit.$"
	db"Drive is not ready.$"
	db"Invalid disk command at access on drive.$"
	db"CRC at reading from drive.$"
	db"Disk access with invalid request structure length.$"
	db"Couldn't find track on disk. Seek error.$"
	db"Not a DOS disk present in accessed drive.$"
	db"Couldn't find sector on accessed drive.$"
	db"Apply some paper to your printer.$"
	db"Error writing data block to disk.$"
	db"Error reading data block from disk.$"
	db"Device is not present.$"
	db"File sharing voilation occured.$"
	db"File sharing lock violation occured.$"
	db"Wrong disk. Can't access.$"
	db"File control block is unavailable.$"
	db"File sharing buffer overflow.$"
	db"DOS: Internal error. Description not possible.$"
	db"Couldn't finsish file access.$"
	db"A network error occured.$"
TXT_DOSerr:
	db"DOS function error - Program aborted.$"
TXT_retry:
	db"Retry.",0
TXT_cancel:
	db"Cancel.",0

singles:db"‡bõÎÓüghijklmÔ0pqÁstuvwxÊz"
capitals:
	db"íBCD‰üGHIJKLMÔ0PQRS‚UVWXùZ"

Bordercontab:
	db"|≥¥µ∂π√≈∆«Ã◊ÿ∫",0
	db"-ƒ¬¡ Àœ—",0
	db"=Õ",0
	db"+∑∏ªºΩæø¿»…”‘’÷Ÿ⁄Œ",0
	db"@€",0
	db"#≤",0
	db"*±",0
	db"+∞",0
	db"_‹",0
	db"~ﬂ",0
	db"!›ﬁ",0
	db 0
germancontab:
	db"Aé",0
	db"aÑ",0
	db"Oô",0
	db"oî",0
	db"Uö",0
	db"uÅ",0
	db 0

erec:	db"REC"
easc:	db"ASC"
efrk:	db"FRK"

changeflag:db 1

lessonstart:
	db 1," Ceterum censeo Cannabis legalisandum esse ! ",1

	.data
datastart:
ftoFreaky	db ?
ftoascii:	db ?
ftocap: 	db ?
fnobeg: 	db ?
ftosmall:	db ?
ftoboth:	db ?
fconvbrdr:	db ?
fnogerm:        db ?
frecfr:         db ?
fyes:           db ?
lastchar        db ?
lastchar2       db ?
comc:		db ?
change:         db ?
gerflag:        db ?
ansiflag:       db ?
ansinum:        db ?
ansijump:       db ?
DestHandle      dw ?
SourceHandle    dw ?
SourceLength    dw ?
Destlength      dw ?
InputSeg        dw ?
destseg 	dw ?
InputSize       dw ?
FileNumAdr:     dw ?
FileNum:        db ?
SourceName:	db 64 dup ?
DestName:       db 64 dup ?
dataend:
